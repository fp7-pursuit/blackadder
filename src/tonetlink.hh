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
#ifndef CLICK_TONETLINK_HH
#define CLICK_TONETLINK_HH

#include "netlink.hh"

CLICK_DECLS

/**@brief (blackadder Core) The ToNetlink Element is the Element that sends packets to applications.
 * 
 * The LocalProxy pushes annotated packets to the ToNetlink element, which then sends them to the right applications using the provided packet annotation.
 */
class ToNetlink : public Element {
public:
    /**
     * @brief Constructor: it does nothing - as Click suggests
     * @return 
     */
    ToNetlink();
    /**
     * @brief Destructor: it does nothing - as Click suggests
     * @return 
     */
    ~ToNetlink();
    /**
     * @brief the class name - required by Click
     * @return 
     */
    const char *class_name() const {return "ToNetlink";}
    /**
     * @brief the port count - required by Click - there is no output and a single input where  LocalProxy pushes packets.
     * @return 
     */
    const char *port_count() const {return "1/0";}
    /**
     * @brief a PUSH Element.
     * @return PUSH
     */
    const char *processing() const {return PUSH;}
    /**
     * @brief Element configuration - the base Netlink socket is passed as the only parameter (in the Click configuration file)
     */
    int configure(Vector<String>&, ErrorHandler*);
    /**@brief This Element must be configured AFTER the base Netlink Element
     * @return the correct number so that it is configured afterwards
     */
    int configure_phase() const {return 301;}
    /**
     * @brief This method is called by Click when the Element is about to be initialized.
     * 
     * In kernel space it allocates and initializes the task that is later scheduled when packets are pushed from the LocalProxy.
     * It also allocates the mutex that protects the up_queue.
     * @param errh
     * @return 
     */
    int initialize(ErrorHandler *errh);
    /**@brief Cleanups everything.
     * 
     * If the stage is before CLEANUP_INITIALIZED (i.e. the element was never initialized), then it does nothing.
     * In the opposite case it unschedules the Task, empties the up_queue and deletes all packets in it.
     * @param stage passed by Click
     */
    void cleanup(CleanupStage stage);
    /**@brief the push method is called by the element connected to the ToNetlink element (i.e. the LocalProxy) and pushes a packet.
     * 
     * This method pushes some space in the packet so that netlink header can fit. It then adds the header.
     * In user space the nlh->nlmsg_pid is assigned to 9999 whereas in kernel space is assigned to 0.
     * In user space the packet is pushed in the out_buf_queue and socket is registered for writing using the add_select.
     * In kernel space it is pushed in the up_queue and the Task is rescheduled.
     * @param port the port from which the packet was pushed
     * @param p a pointer to the packet
     */
    void push(int port, Packet *p);
#if CLICK_LINUXMODULE
    /**@brief This Click Task is executed whenever a packet is pushed in the up_queue (in kernel space only).
     * 
     * It tries to send all packets in the queue and if, in the meanwhile, another packet was pushed in the queue, it is fastly rescheduled.
     */
    bool run_task(Task *t);
#else
    /**@brief The selected method is called by Click whenever the socket is writable and one or more packets have been previously put in the out_buf_queue (in iser space only).
     * 
     * It tries to send a packet to an application and if it succeeds it removes the packet and deletes it (kill).
     * If more packets exist the socket is registered for writing using the add_select method.
     * @param fd
     * @param mask
     */
    void selected(int fd, int mask);
#endif
    /** @brief a pointer to the Base Netlink Element.
     */
    Netlink *netlink_element;
#if CLICK_LINUXMODULE
    /**@brief the Click Task.
     */
    Task *_task;
    Vector<Packet *> up_queue;
    struct mutex up_mutex;
    unsigned long _to_netlink_element_state;
#endif
    
};

CLICK_ENDDECLS
#endif
