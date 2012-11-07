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
#include "tonetlink.hh"

CLICK_DECLS

ToNetlink::ToNetlink() {
}

ToNetlink::~ToNetlink() {
    click_chatter("ToNetlink: destroyed!");
}

int ToNetlink::configure(Vector<String> &conf, ErrorHandler */*errh*/) {
    netlink_element = (Netlink *) cp_element(conf[0], this);
    //click_chatter("ToNetlink: configured!");
    return 0;
}

#if CLICK_LINUXMODULE || CLICK_BSDMODULE
int ToNetlink::initialize(ErrorHandler *errh) {
    _task = new Task(this);
    _to_netlink_element_state = 0;
# if CLICK_LINUXMODULE
    mutex_init(&up_mutex);
# else
    mtx_init(&up_mutex, "ba_up_mutex", NULL, MTX_DEF);
# endif
    ScheduleInfo::initialize_task(this, _task, errh);
    //click_chatter("ToNetlink: initialized!");
    return 0;
}
#else
int ToNetlink::initialize(ErrorHandler */*errh*/) {
    //click_chatter("ToNetlink: initialized!");
    return 0;
}
#endif

void ToNetlink::cleanup(CleanupStage stage) {
    if (stage >= CLEANUP_INITIALIZED) {
#if CLICK_LINUXMODULE || CLICK_BSDMODULE
        /*empty the queues first*/
        int up_queue_size;
        /*empty the up_queue*/
        mutex_lock(&up_mutex);
        /*unschedule and delete the task*/
        _task->unschedule();
        up_queue_size = up_queue.size();
        for (int i = 0; i < up_queue_size; i++) {
            up_queue.at(up_queue.size() - 1)->kill();
            up_queue.pop_back();
        }
        delete _task;
# if CLICK_LINUXMODULE
        mutex_unlock(&up_mutex);
# else
        mtx_destroy(&up_mutex);
# endif
#endif
    }
    click_chatter("ToNetlink: Cleaned Up!");
}

void ToNetlink::push(int, Packet *p) {
    WritablePacket *final_p;
    struct nlmsghdr *nlh;
    /*LocalProxy pushed a packet to be sent to an application*/
    final_p = p->push(sizeof (struct nlmsghdr));
    /*Now it is ready - I have to create a netlink header and put it in the up_queue*/
    nlh = (struct nlmsghdr *) final_p->data();
    nlh->nlmsg_len = sizeof (final_p->length());
    nlh->nlmsg_type = 0;
    nlh->nlmsg_flags = 1;
    nlh->nlmsg_seq = 0;
#if CLICK_LINUXMODULE || CLICK_BSDMODULE
    nlh->nlmsg_pid = 0;
    mutex_lock(&up_mutex);
    up_queue.push_front(final_p);
    mutex_unlock(&up_mutex);
    _task->reschedule();
# if CLICK_LINUXMODULE
    set_bit(TASK_IS_SCHEDULED, &_to_netlink_element_state);
# else
    atomic_set_long((volatile u_long *)(&_to_netlink_element_state), 1);
# endif
#else
    nlh->nlmsg_pid = 9999;
    netlink_element->out_buf_queue.push(final_p);
    add_select(netlink_element->fd, SELECT_WRITE);
#endif
}

#if CLICK_LINUXMODULE || CLICK_BSDMODULE

bool ToNetlink::run_task(Task *t) {
    int ret;
    Packet *up_packet = NULL;
    int pid;
# if CLICK_LINUXMODULE
    clear_bit(TASK_IS_SCHEDULED, &_to_netlink_element_state);
# else
    atomic_clear_long((volatile u_long *)(&_to_netlink_element_state), 1);
# endif
    mutex_lock(&up_mutex);
    while (up_queue.size() > 0) {
        up_packet = up_queue.at(up_queue.size() - 1);
        up_queue.pop_back();
        pid = up_packet->anno_u32(0);
# if CLICK_LINUXMODULE
        ret = netlink_unicast(netlink_element->nl_sk, up_packet->skb(), pid, MSG_WAITALL);
# else
        struct mbuf *m = up_packet->steal_m();
        ret = ba_to_socket(m, pid);
# endif
    }
    mutex_unlock(&up_mutex);
# if CLICK_LINUXMODULE
    if (test_bit(TASK_IS_SCHEDULED, &_to_netlink_element_state) == 1) {
        t->fast_reschedule();
    }
# else
    if ((*(volatile u_long *)(&_to_netlink_element_state)) & 1) {
        t->fast_reschedule();
    }
# endif
    return true;
}
#else

void ToNetlink::selected(int fd, int mask) {
    WritablePacket *newPacket;
    int bytes_written;
#if HAVE_USE_NETLINK
    struct sockaddr_nl d_nladdr;
#elif HAVE_USE_UNIX
    struct sockaddr_un d_nladdr;
#endif
    struct msghdr msg;
    struct iovec iov[1];
    if ((mask & SELECT_WRITE) == SELECT_WRITE) {
        if (!netlink_element->out_buf_queue.empty()) {
            newPacket = netlink_element->out_buf_queue.front();
            memset(&d_nladdr, 0, sizeof (d_nladdr));
#if HAVE_USE_NETLINK
            d_nladdr.nl_family = AF_NETLINK;
            d_nladdr.nl_pad = 0;
            d_nladdr.nl_pid = newPacket->anno_u32(0);
#elif HAVE_USE_UNIX
# ifndef __linux__
            d_nladdr.sun_len = sizeof (d_nladdr);
# endif
            d_nladdr.sun_family = PF_LOCAL;
            ba_id2path(d_nladdr.sun_path, newPacket->anno_u32(0));
#endif
            iov[0].iov_base = newPacket->data();
            iov[0].iov_len = newPacket->length();
            memset(&msg, 0, sizeof (msg));
            msg.msg_name = (void *) &d_nladdr;
            msg.msg_namelen = sizeof (d_nladdr);
            msg.msg_iov = iov;
            msg.msg_iovlen = 1;
            bytes_written = sendmsg(fd, &msg, MSG_WAITALL);
            /*remove buffer from queue and free it*/
            newPacket->kill();
            netlink_element->out_buf_queue.pop();
            if (netlink_element->out_buf_queue.empty()) {
                remove_select(fd, SELECT_WRITE);
            }
        } else {
            remove_select(fd, SELECT_WRITE);
            add_select(fd, SELECT_READ);
        }
    }
}
#endif

CLICK_ENDDECLS
EXPORT_ELEMENT(ToNetlink)
ELEMENT_REQUIRES(Netlink)
