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
#ifndef CLICK_FROMNETLINK_HH
#define CLICK_FROMNETLINK_HH

#include "netlink.hh"
#include "helper.hh"

CLICK_DECLS

/**
 * @brief (blackadder Core) The FromNetlink Element receives packets from applications, annotates and pushes them to the LocalProxy Element.
 * 
 * In kernel space it also keeps track of the applications (assuming that the netlink port used by each application is the process ID). 
 * When an application terminates, it sends a disconnection message to the LocalProxy on behalf of the application.
 * In user-space the application must send the message by itself before it terminates (or crashes?How?).
 * 
 * FromNetlink uses a kernel mutex when running in kernel space since packets are received in the context of the process and then are processed by a Click Task. 
 * A queue that is secured with the mutex is used for that transition.
 */
class FromNetlink : public Element {
public:
    /**
     * @brief Constructor: it does nothing - as Click suggests
     * @return 
     */
    FromNetlink();
    /**
     * @brief Destructor: it does nothing - as Click suggests
     * @return 
     */
    ~FromNetlink();
    /**
     * @brief the class name - required by Click
     * @return 
     */
    const char *class_name() const {return "FromNetlink";}
    /**
     * @brief the port count - required by Click - there is no input and single output to the LocalProxy.
     * @return 
     */
    const char *port_count() const {return "0/1";}
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
     * 
     * @return the correct number so that it is configured afterwards
     */
    int configure_phase() const {return 301;}
    /**
     * @brief This method is called by Click when the Element is about to be initialized.
     * 
     * In user space this method calls the add_select method to denote that the selected method should be called whenever the socket is readable.
     * In kernel space, it initializes the kernel socket (netlink port 0 and netlink protocol NETLINK_BLACKADDER) and the mutex that protects the down_queue.
     * In kernel space it also initializes the Task and the Timer and schedules the timer. The Task is scheduled only when packets arrive from the socket.
     * @param errh
     * @return 
     */
    int initialize(ErrorHandler *errh);
    /**@brief Cleanups everything.
     * 
     * If the stage is before CLEANUP_INITIALIZED (i.e. the element was never initialized), then it does nothing.
     * In the opposite case it unschedules the Task, clears and deletes the Timer, empties the down_queue and deletes all packets in it.
     * @param stage stage passed by Click
     */
    void cleanup(CleanupStage stage);
#if CLICK_LINUXMODULE
    /**@brief This task is fastly rescheduled when more packets exist in the down_queue.
     * Each time it runs, it acquires the mutex and pushes all pending packets to the LocalProxy Element.
     * During that time processes sending packets to their netlink socket are blocked in that mutex.
     */
    bool run_task(Task *t);
    /**@brief The Timer runs periodically and checks if a previously recorded process (by the callback method) ended.
     *  For each ended process it pushes a disconnection packet to the Local Proxy and reschedules itself.
     */
    void run_timer(Timer *_timer);
#else
    /**@brief The selected method overrides Click Element's selected method (User-Space only).
     *  The netlink socket is always marked as readable. Whenever it is, the selected method is called.
     *  It reads a packet from the socket buffer (if possible), annotates it using the source netlink port and pushes it to the LocalProxy.
     */
    void selected(int fd, int mask);
#endif
    /**@brief A pointer to the base Netlink Element.
     */
    Netlink *netlink_element;
#if CLICK_LINUXMODULE
    /**@brief A Click Task used for processing all packets from the down_queue (Kernel-space Only).
     * 
     * Packets are immediately put there by the netlink callback function that runs is the process context.
     */
    Task *_task;
    /** @brief A Click Timer used for checking for terminated applications. It runs periodically.
     * 
     * If a processes is detected as terminated a disconnect message is sent to the LocalProxy.
     */
    Timer *_timer;
#endif
};

CLICK_ENDDECLS
#endif
