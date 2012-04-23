/*
 * Copyright (C) 2010-2011  George Parisis and Dirk Trossen
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of
 * the BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

#ifndef NB_BLACKADDER_HPP
#define NB_BLACKADDER_HPP

#include "blackadder.hpp"

#include <signal.h>
#include <queue>
#include <fcntl.h>

class Event;

/**@relates NB_Blackadder
 * @brief a type definition for the pointer to the callback method.
 */
typedef void (*callbacktype)(Event *);

/**@brief (User Library) This is the wrapper class that makes the service model available to all applications in a Non-Blocking manner. 
 * 
 * blackadder expects requests to be sent in its netlink socket. Therefore the wrapper class just exports some human-friendly methods for creating service model compliant buffers that are asynchronously sent to blackadder.
 * NB_Blackadder implements the Singleton Pattern. A single NB_Blackadder object can be created by a single process using the <b>public</b> Instance method. The Constructor is <b>protected</b>.
 * 
 * NB_Blackadder uses two threads. A selector thread reads events when the netlink socket is readable and passes them to the worker thread. 
 * In the context of the worker thread, the callback method that <b>must be provided by the applications</b> is called with a reference to the received Event.
 * 
 * The selector thread is also notified (using a pipe) to register the netlink socket for write when requests are to be forwarded to blackadder.
 * 
 * @note All service request related methods enforce some rules regarding the size of the identifiers so that blackadder is not confused.
 */
class NB_Blackadder {
public:
    /**@brief Destructor: It closes the socket.
     * 
     */
    ~NB_Blackadder();
    /**@brief the Instance method is the only way to construct a NB_Blackadder object. It is impossible to construct multiple objects since Instance will return the already constructed one.
     * 
     * Instance will create a new object by calling the protected constructor and assign it to the m_pInstance value ONLY the first time it is called. All other times it will return the m_pInstance pointer.
     * @param user_space if it is true, the netlink is created so that it can communicate with blackadder running in user space. 
     * if it is false, the netlink is created so that it can communicate with blackadder running in kernel space.
     * @return 
     */
    static NB_Blackadder* Instance(bool user_space);
    /**@brief this method will send a PUBLISH_SCOPE request to blackadder. <b>It won't block. Instead the request buffer will be put in a queue and the selector thread will be notified to send the request to blackadder.</b>
     * 
     * If prefix_id is an empty string, the request is about a root scope.
     * 
     * If id is a single fragment, the request is about publishing a new scope.
     * 
     * If id consists of multiple fragments, the request is about republishing an existing scope under an existing scope.
     * 
     * @param id the identifier of a scope. It can be a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param prefix_id the identifier of the father scope. It can be an empty string, a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     */
    void publish_scope(const string &id, const string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a PUBLISH_INFO request to blackadder. <b>It won't block. Instead the request buffer will be put in a queue and the selector thread will be notified to send the request to blackadder.</b>
     * 
     * prefix_id CANNOT be an empty string.
     * 
     * If id is a single fragment, the request is about publishing a new information item.
     * 
     * If id consists of multiple fragments, the request is about republishing an existing information item under an existing scope.
     * 
     * Implicitly registers to START_PUBLISH and STOP_PUBLISH events for the corresponding information item. START_PUBLISH tells the application that it has at least one subscriber to the corresponding information item and STOP_PUBLISH means that it has no subscribers.
     *
     * @param id the identifier of an information item. It can be a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param prefix_id the identifier of the father scope. It can be a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     */
    void publish_info(const string &id, const string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a UNPUBLISH_SCOPE request to blackadder. <b>It won't block. Instead the request buffer will be put in a queue and the selector thread will be notified to send the request to blackadder.</b>
     * 
     * If prefix_id is the empty string, the request is about a root scope.
     * 
     * If id is a single fragment, the request is about unpublishing a scope.
     * 
     * id CANNOT consist of multiple fragments.
     * 
     * @param id the identifier of a scope. It can be a single fragment with size PURSUIT_ID_LEN.
     * @param prefix_id the identifier of the father scope. It can be an empty string, a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     */
    void unpublish_scope(const string &id, const string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a UNPUBLISH_INFO request to blackadder. <b>It won't block. Instead the request buffer will be put in a queue and the selector thread will be notified to send the request to blackadder.</b>
     * 
     * prefix_id CANNOT be an empty string.
     * 
     * If id is a single fragment, the request is about unpublishing an information item.
     * 
     * id CANNOT consist of multiple fragments.
     * 
     * @param id the identifier of an information item. It can be a single fragment with size PURSUIT_ID_LEN.
     * @param prefix_id the identifier of the father scope. It can be a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     */
    void unpublish_info(const string &id, const string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a SUBSCRIBE_SCOPE request to blackadder. <b>It won't block. Instead the request buffer will be put in a queue and the selector thread will be notified to send the request to blackadder.</b>
     * 
     * If prefix_id is the empty string, the request is about a root scope.
     * 
     * If id is a single fragment, the request is about unpublishing a scope.
     * 
     * id CANNOT consist of multiple fragments. 
     *
     * Implicitly registers to SCOPE_PUBLISHED and SCOPE_UNPUBLISHED events that provide notifications about new or deleted subscopes, as well as to PUBLISHED_DATA events for all items directly under the scope.
     * 
     * @param id id the identifier of a scope. It can be a single fragment with size PURSUIT_ID_LEN.
     * @param prefix_id prefix_id the identifier of the father scope. It can be an empty string, a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     */
    void subscribe_scope(const string &id, const string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a SUBSCRIBE_INFO request to blackadder. <b>It won't block. Instead the request buffer will be put in a queue and the selector thread will be notified to send the request to blackadder.</b>
     *
     * 
     * prefix_id CANNOT be the empty string.
     * 
     * If id is a single fragment, the request is about unpublishing a scope.
     * 
     * id CANNOT consist of multiple fragments. 
     *
     * Implicitly registers to PUBLISHED_DATA events for the corresponding information item.
     *
     * @param id id the identifier of an information item. It can be a single fragment with size PURSUIT_ID_LEN.
     * @param prefix_id prefix_id the identifier of the father scope. It can be a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     */
    void subscribe_info(const string &id, const string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a UNSUBSCRIBE_SCOPE request to blackadder. <b>It won't block. Instead the request buffer will be put in a queue and the selector thread will be notified to send the request to blackadder.</b>
     * 
     * If prefix_id is the empty string, the request is about a root scope.
     * 
     * If id is a single fragment, the request is about unpublishing a scope.
     * 
     * id CANNOT consist of multiple fragments. 
     * 
     * @param id id the identifier of a scope. It can be a single fragment with size PURSUIT_ID_LEN.
     * @param prefix_id prefix_id the identifier of the father scope. It can be an empty string, a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     */
    void unsubscribe_scope(const string &id, const string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a UNSUBSCRIBE_INFO request to blackadder. <b>It won't block. Instead the request buffer will be put in a queue and the selector thread will be notified to send the request to blackadder.</b>
     *
     * 
     * prefix_id CANNOT be the empty string.
     * 
     * If id is a single fragment, the request is about unpublishing a scope.
     * 
     * id CANNOT consist of multiple fragments.
     *  
     * @param id id the identifier of an information item. It can be a single fragment with size PURSUIT_ID_LEN.
     * @param prefix_id prefix_id the identifier of the father scope. It can be a single fragment with size PURSUIT_ID_LEN or multiple fragments PURSUIT_ID_LEN each.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     */
    void unsubscribe_info(const string &id, const string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a PUBLISH_DATA request to blackadder. <b>It won't block. Instead the request buffer will be put in a queue and the selector thread will be notified to send the request to blackadder.</b>
     * 
     * @param id the full identifier of the information item for which data is published.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     * @param data a bucket of data that is published.
     * @param data_len the size of the published data.
     */
    void publish_data(const string &id, char strategy, void *str_opt, unsigned int str_opt_len, void *data, unsigned int data_len);
    /**@brief This method will send a disconnect signal to Blackadder. 
     * 
     * In user space this is required so that Blackadder can then undo all requests the application has previously sent.
     * In kernel this is not required since Blackadder can monitor applications.
     */
    void disconnect();
    /**@brief this method registers a user provided call back with NB_Blackadder
     * 
     * @param t a pointer to the callback function (of type callbacktype)
     */
    void setCallback(callbacktype t);
    /**@brief the selector thread execution method.
     * 
     * @param arg
     */
    static void *selector(void *arg);
    /**@brief The worker thread execution method.
     * 
     * @param arg
     */
    static void *worker(void *arg);
    /**@brief the signal handler.
     * 
     * 
     * @param sig
     */
    static void signal_handler(int sig);
    /**@brief This method MUST be called by the application so that the main function will not end before the NB_Blackadder threads end.
     * 
     * it calls pthread_join for the worker and selector threads.
     */
    void join();
    /**@brief The selector Thread (see details).
     * 
     * The selector thread always blocks to the select() method. If the netlink socket is only registered for reading, select() unblocks only when an Event is sent by blackadder.
     * When a request is ready to be sent to blackadder, a dummy message is sent to the pipe in which the selector thread always includes it in the reading fd_set. AT that point the netlink socket is also registered for writing.
     * When the socket will unblock again it will send the message to blackadder.
     */
    static pthread_t selector_thread;
    /**@brief a mutex used to synchronize NB_Blackadder threads.
     */
    static pthread_mutex_t selector_mutex;
    /**@brief this condition is used for putting a limit to the pending requests.
     */
    static pthread_cond_t queue_overflow_cond;
    /**@brief the worker thread (see details).
     * 
     * The worker thread always blocks in the worker_cond. The selector thread signals that condition whenever a new event from blackadder was read.
     * The worker thread unblocks, reads the event from the event_queue and calls the application-defined callback method.
     */
    static pthread_t worker_thread;
    /**@brief a mutex used to synchronize NB_Blackadder threads.
     */
    static pthread_mutex_t worker_mutex;
    /**@brief the condition for which the worker thread constantly waits. It is only signaled by the selector thread.
     */
    static pthread_cond_t worker_cond;
    /**@brief the set of file descriptions that are registered for reading. Only the netlink socket sock_fd and the pipefds are used.
     */
    static fd_set read_set;
    /**@brief the set of file descriptions that are registered for writing. Only the netlink socket sock_fd and the pipefds are used.
     */
    static fd_set write_set;
    /**@brief the file descriptors of the bidirectional pipe.
     */
    static int pipe_fds[2];
    /**@brief the netlink socket file descriptor.
     */
    static int sock_fd;
    /**@brief the queue where all service model related methods put their messages that are later sent to blackadder by the selector thread.
     */
    static queue <struct msghdr> output_queue;
    /**@brief the queue where the selector thread puts all Events sent by blackadder that are later processed by the worker thread.
     */
    static queue <Event *> event_queue;
    /**@brief a dummy buffer for getting a dummy message from the pipe and interrupting the select() call.
     */
    static char pipe_buf[1];
    /**@brief a dummy buffer for peeking to the actual netlink buffers.
     */
    static char fake_buf[1];
    /**@brief the Callback function registered with NB_Blackadder. The user must override the default by calling the setCallback() method.
     */
    static callbacktype cf;

protected:
    /**@brief Constructor: It initiates the netlink socket appropriately. It initiates all mutexes and condition variables and starts the worker and selector threads.
     * 
     * @param user_space
     */
    NB_Blackadder(bool user_space);
private:
    /**brief push a message in the queue and notify selector thread to send it to blackadder.
     * 
     * @param type as passed by a request method.
     * @param id as passed by a request method.
     * @param prefix_id as passed by a request method.
     * @param strategy as passed by a request method.
     * @param str_opt as passed by a request method.
     * @param str_opt_len as passed by a request method.
     */
    void push(unsigned char type, const string &id, const string &prefix_id, char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief the single static NB_Blackadder object an application can access.
     */
    static NB_Blackadder* m_pInstance;
    /**@brief the netlink socket source and destination sockaddr_nl structures. They stay the same as long as the application runs.
     */
#if HAVE_USE_NETLINK
    struct sockaddr_nl s_nladdr, d_nladdr;
#else	
    static struct sockaddr_un s_nladdr, d_nladdr;
#endif
};
#endif
