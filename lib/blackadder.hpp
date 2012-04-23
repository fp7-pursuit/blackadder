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

#ifndef BLACKADDER_HPP
#define BLACKADDER_HPP

#ifdef __linux__
#define HAVE_USE_NETLINK 1
#endif

#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <sys/socket.h>
#if HAVE_USE_NETLINK
#include <linux/netlink.h>
#else  /* __FreeBSD__, __APPLE__, etc. */
#include <sys/un.h>
#include <sys/ioctl.h>
#endif
#include <errno.h>
#include <vector>
#include <sstream> 
#include <iostream>

#include "blackadder_defs.h"


using namespace std;

/**@relates Blackadder
 * @brief converts a string of size 2 * PURSUIT_ID_LEN in hex to a string of size PURSUIT_ID_LEN in binary.
 * 
 * @param hexstr a string of size 2 * PURSUIT_ID_LEN in hex
 * @return the converted string
 */
string hex_to_chararray(const string &hexstr);
/**@relates Blackadder
 * @brief converts a string of size PURSUIT_ID_LEN in binary to a string of size 2 * PURSUIT_ID_LEN in hex.
 * 
 * @param str a string of size PURSUIT_ID_LEN in binary
 * @return the converted string
 */
string chararray_to_hex(const string &str);

class Event;

/**@brief (User Library) This is the wrapper class that makes the service model available to all applications. 
 * 
 * blackadder expects requests to be sent in its netlink socket. Therefore the wrapper class just exports some human-friendly methods for creating service model compliant buffers that are sent to blackadder.
 * blackadder implements the Singleton Pattern. A single blackadder object can be created by a single process using the <b>public</b> Instance method. The Constructor is <b>protected</b>.
 * 
 * @note All service request related methods enforce some rules regarding the size of the identifiers so that blackadder is not confused.
 */
class Blackadder {
public:
    /**@brief Destructor: It closes the socket.
     * 
     */
    ~Blackadder();
    /**@brief the Instance method is the only way to construct a blackadder object. It is impossible to construct multiple objects since Instance will return the already constructed one.
     * 
     * Instance will create a new object by calling the protected constructor and assign it to the m_pInstance value ONLY the first time it is called. All other times it will return the m_pInstance pointer.
     * @param user_space if it is true, the netlink is created so that it can communicate with blackadder running in user space. 
     * if it is false, the netlink is created so that it can communicate with blackadder running in kernel space.
     * @return 
     */
    static Blackadder* Instance(bool user_space);
    /**@brief this method will send a PUBLISH_SCOPE request to blackadder. 
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
    /**@brief this method will send a PUBLISH_INFO request to blackadder. 
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
    void publish_info(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a UNPUBLISH_SCOPE request to blackadder. 
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
    void unpublish_scope(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a UNPUBLISH_INFO request to blackadder.
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
    void unpublish_info(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a SUBSCRIBE_SCOPE request to blackadder.
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
    void subscribe_scope(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a SUBSCRIBE_INFO request to blackadder.
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
    void subscribe_info(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a UNSUBSCRIBE_SCOPE request to blackadder.
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
    void unsubscribe_scope(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a UNSUBSCRIBE_INFO request to blackadder.
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
    void unsubscribe_info(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
    /**@brief this method will send a PUBLISH_DATA request to blackadder.
     * 
     * @param id the full identifier of the information item for which data is published.
     * @param strategy the dissemination strategy assigned to the request.
     * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
     * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
     * @param data a bucket of data that is published.
     * @param data_len the size of the published data.
     */
    void publish_data(const string&id, unsigned char strategy, void *str_opt, unsigned int str_opt_len, void *data, unsigned int data_len);
    /**@brief This method blocks until an event is received from blackadder.
     * 
     * @param ev a reference to an Event which will be updated accordingly. An application can read the Event (and the data when the event is PUBLISHED_DATA) when the method unblocks.
     */
    void getEvent(Event &ev);
    /**@brief This method will send a disconnect signal to Blackadder. 
     * 
     * In user space this is required so that Blackadder can then undo all requests the application has previously sent.
     * In kernel this is not required since Blackadder can monitor applications.
     */
    void disconnect();
protected:
    /**@brief Constructor: It creates the netlink socket, binds it and construct the appropriate sockaddr_nl structures for sending requests to Blackadder.
     * 
     * @param user_space Whether Blackadder runs in user or kernel space.
     */
    Blackadder(bool user_space);
private:
    /**@brief create_and_send_buffers is called by all other service model related methods. It creates and sends the buffers to Blackadder.
     * 
     * @param type as passed by a request method.
     * @param id as passed by a request method.
     * @param prefix_id as passed by a request method.
     * @param strategy as passed by a request method.
     * @param str_opt as passed by a request method.
     * @param str_opt_len as passed by a request method.
     */
    int create_and_send_buffers(unsigned char type, const string &id, const string &prefix_id, char strategy, void *str_opt, unsigned int str_opt_len);
    /** @brief The netlink socket file descriptor.
     */
    int sock_fd;
    /**@brief the netlink socket source and destination sockaddr_nl structures. They stay the same as long as the application runs.
     */
#if HAVE_USE_NETLINK
    struct sockaddr_nl s_nladdr, d_nladdr;
#else
    struct sockaddr_un s_nladdr, d_nladdr;
#endif
    /**@brief a dummy buffer for peeking the actual expected buffer so that we can learn its size.
     */
    char fake_buf[1];
    /**@brief the single static Blackadder object an application can access.
     */
    static Blackadder* m_pInstance;
};

/**@brief (User Library) An event is what can be always expected by Blackaddder. Events are sent to applications asynchronously in respect with their initial pub/sub requests.
 * 
 * An Event contains the type of the Event, the full identifier of the Scope or Information Item (depending on the Event) and potentially the data (when the Event is PUBLISHED_DATA).
 * 
 */
class Event {
public:
    /**@brief Constructor: Usually applications create an Event using the default constructor and pass a reference to the getEvent() method.
     */
    Event();
    /**
     * @brief Destructor: The destructor should delete the buffer. The buffer is not accessed by applications. data is somewhere in this buffer so a free(data) is NOT ALLOWED.
     */
    ~Event();
    /**@brief Copy Constructor: The buffer must be copied so that it can be then freed safely.
     * 
     * @param ev
     */
    Event(Event &ev);
    /**@brief Overloading operator=: The buffer must be copied so that it can be then freed safely.
     * 
     * @param ev
     */
    Event& operator=(const Event &ev);
    /**@brief The type of the Event.
     * 
     * It can be: 
     * 
     * START_PUBLISH 
     * 
     * STOP_PUBLISH 
     * 
     * SCOPE_PUBLISHED 
     * 
     * SCOPE_UNPUBLISHED 
     * 
     * PUBLISHED_DATA 
     * 
     * Events are handled by applications arbitrarily (e.g. one could publish a million buffers after receiving a START_PUBLISH or one could subscribe to the scope after receiving a SCOPE_PUBLISHED Event).
     */
    unsigned char type;
    /**@brief The full identifier of the scope or information item reffered by the event.
     */
    string id;
    /**@brief The data that accompany a PUBLISHED_DATA Event
     */
    void *data;
    /**@brief The data length of the data that accompany a PUBLISHED_DATA Event
     */
    unsigned int data_len;
    /**@brief a buffer containing all the above.
     */
    void *buffer; /*do not use that...only the destructor uses it to delete the whole buffer once*/
};

#ifndef __LINUX_NETLINK_H
extern "C" {

    struct nlmsghdr {
        uint32_t nlmsg_len;
        uint16_t nlmsg_type;
        uint16_t nlmsg_flags;
        uint32_t nlmsg_seq;
        uint32_t nlmsg_pid;
    };
}
#endif

#if !HAVE_USE_NETLINK
#define ba_id2path(path, id)  snprintf((path), 100, "/tmp/blackadder.%05u", (id))	
#if defined(__FreeBSD__) || defined(__OpenBSD__)

inline int
ba_path2id(const char *path) {
    const char *errstr = NULL;
    int num = (int) strtonum((path) + 16, 0, 99999/*PID_MAX*/, &errstr);
    return (errstr == NULL) ? num : -1;
}
#else
#define ba_path2id(path) atoi((path) + 16)
#endif
#endif
#endif
