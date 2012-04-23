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

int ToNetlink::configure(Vector<String> &conf, ErrorHandler *errh) {
    netlink_element = (Netlink *) cp_element(conf[0], this);
    //click_chatter("ToNetlink: configured!");
    return 0;
}

int ToNetlink::initialize(ErrorHandler *errh) {
#if CLICK_LINUXMODULE
    _task = new Task(this);
    _to_netlink_element_state = 0;
    mutex_init(&up_mutex);
    ScheduleInfo::initialize_task(this, _task, errh);
#endif
    //click_chatter("ToNetlink: initialized!");
    return 0;
}

void ToNetlink::cleanup(CleanupStage stage) {
    if (stage >= CLEANUP_INITIALIZED) {
#if CLICK_LINUXMODULE
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
        mutex_unlock(&up_mutex);
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
#if CLICK_LINUXMODULE
    nlh->nlmsg_pid = 0;
    mutex_lock(&up_mutex);
    up_queue.push_front(final_p);
    mutex_unlock(&up_mutex);
    _task->reschedule();
    set_bit(TASK_IS_SCHEDULED, &_to_netlink_element_state);
#else
    nlh->nlmsg_pid = 9999;
    netlink_element->out_buf_queue.push(final_p);
    add_select(netlink_element->fd, SELECT_WRITE);
#endif
}

#if CLICK_LINUXMODULE

bool ToNetlink::run_task(Task *t) {
    int ret;
    Packet *up_packet = NULL;
    int pid;
    clear_bit(TASK_IS_SCHEDULED, &_to_netlink_element_state);
    mutex_lock(&up_mutex);
    while (up_queue.size() > 0) {
        up_packet = up_queue.at(up_queue.size() - 1);
        up_queue.pop_back();
        pid = up_packet->anno_u32(0);
        ret = netlink_unicast(netlink_element->nl_sk, up_packet->skb(), pid, MSG_WAITALL);
    }
    mutex_unlock(&up_mutex);
    if (test_bit(TASK_IS_SCHEDULED, &_to_netlink_element_state) == 1) {
        t->fast_reschedule();
    }
    return true;
}
#else

void ToNetlink::selected(int fd, int mask) {
    WritablePacket *newPacket;
    int bytes_written;
#if HAVE_USE_NETLINK
    struct sockaddr_nl d_nladdr;
#else
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
#else
            d_nladdr.sun_len = sizeof (d_nladdr);
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
