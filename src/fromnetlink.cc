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
#include "fromnetlink.hh"

#if HAVE_USE_UNIX
#include <click/cxxprotect.h>
CLICK_CXX_PROTECT
#include <sys/ioctl.h>
CLICK_CXX_UNPROTECT
#include <click/cxxunprotect.h>
#endif

CLICK_DECLS

#if CLICK_LINUXMODULE || CLICK_BSDMODULE
static Vector<Packet *> *down_queue;
static struct mutex down_mutex;
static Task *_from_netlink_task;
static unsigned long _from_netlink_element_state;
#else
char fake_buf[1];
#endif


#if CLICK_LINUXMODULE || CLICK_BSDMODULE

# if CLICK_LINUXMODULE
void nl_callback(struct sk_buff *skb)
# else
static void nl_callback(struct mbuf *m)
# endif
{
    int type, flags, nlmsglen, skblen;
    struct nlmsghdr *nlh;

# if CLICK_LINUXMODULE
    skblen = skb->len;
    if (skblen < sizeof (*nlh)) {
        return;
    }
    nlh = nlmsg_hdr(skb);
# else /* CLICK_BSDMODULE */
    Packet *p = Packet::make(m);
    skblen = p->length(); /* XXX */
    if (skblen < sizeof(*nlh)) {
        p->kill();
        return;
    }
    nlh = (struct nlmsghdr *)p->data();
# endif
    nlmsglen = nlh->nlmsg_len;
    if (nlmsglen < sizeof (*nlh) || skblen < nlmsglen) {
        return;
    }
    flags = nlh->nlmsg_flags;
    type = nlh->nlmsg_type;
# if CLICK_LINUXMODULE
#if HAVE_SKB_DST_DROP
    skb_dst_drop(skb);
#else
    if (skb->dst) {
        dst_release(skb->dst);
        skb->dst = 0;
    }
#endif
    skb_orphan(skb);
    skb = skb_realloc_headroom(skb, 50);
    Packet *p = Packet::make(skb);
# endif

    /*pull the netlink header*/
    p->pull(sizeof (nlmsghdr));
    p->set_anno_u32(0, nlh->nlmsg_pid);

    mutex_lock(&down_mutex);
    down_queue->push_front(p);
    _from_netlink_task->reschedule();
# if CLICK_LINUXMODULE
    set_bit(TASK_IS_SCHEDULED, &_from_netlink_element_state);
# else
    atomic_set_long((volatile u_long *)(&_from_netlink_element_state), 1);
# endif
    mutex_unlock(&down_mutex);
}
#endif

FromNetlink::FromNetlink() {
}

FromNetlink::~FromNetlink() {
    click_chatter("FromNetlink: destroyed!");
}

int FromNetlink::configure(Vector<String> &conf, ErrorHandler */*errh*/) {
    netlink_element = (Netlink *) cp_element(conf[0], this);
    //click_chatter("FromNetlink: configured!");
    return 0;
}

#if CLICK_LINUXMODULE || CLICK_BSDMODULE
int FromNetlink::initialize(ErrorHandler *errh) {
    down_queue = new Vector<Packet *>();
# if CLICK_LINUXMODULE
    netlink_element->nl_sk = netlink_kernel_create(&init_net, NETLINK_BADDER, 0, nl_callback, NULL, THIS_MODULE);
    if (netlink_element->nl_sk == NULL) {
        delete down_queue;
        return -1;
    }
# else
    ba_socket_init(nl_callback);
# endif
    _task = new Task(this);
    _from_netlink_task = _task;
    _from_netlink_element_state = 0;
# if CLICK_LINUXMODULE
    mutex_init(&down_mutex);
# else
    mtx_init(&down_mutex, "ba_down_mutex", NULL, MTX_DEF);
# endif
    ScheduleInfo::initialize_task(this, _task, errh);
    //click_chatter("FromNetlink: initialized!");
    return 0;
}
#else
int FromNetlink::initialize(ErrorHandler */*errh*/) {
    add_select(netlink_element->fd, SELECT_READ);
    //click_chatter("FromNetlink: initialized!");
    return 0;
}
#endif

void FromNetlink::cleanup(CleanupStage stage) {
    if (stage >= CLEANUP_INITIALIZED) {
#if CLICK_LINUXMODULE || CLICK_BSDMODULE
        int down_queue_size;
        mutex_lock(&down_mutex);
        _task->unschedule();
        down_queue_size = down_queue->size();
        for (int i = 0; i < down_queue_size; i++) {
            down_queue->at(down_queue->size() - 1)->kill();
            down_queue->pop_back();
        }
        /*delete queue*/
        delete down_queue;
        delete _task;
# if CLICK_LINUXMODULE
        mutex_unlock(&down_mutex);
# else
        mtx_destroy(&down_mutex);
# endif
#endif
    }
    click_chatter("FromNetlink: Cleaned Up!");
}

#if CLICK_LINUXMODULE || CLICK_BSDMODULE

bool FromNetlink::run_task(Task *t) {
    Packet *down_packet = NULL;
# if CLICK_LINUXMODULE
    clear_bit(TASK_IS_SCHEDULED, &_from_netlink_element_state);
# else
    atomic_clear_long((volatile u_long *)(&_from_netlink_element_state), 1);
# endif
    mutex_lock(&down_mutex);
    while (down_queue->size() > 0) {
        down_packet = down_queue->at(down_queue->size() - 1);
        down_queue->pop_back();
        output(0).push(down_packet);
    }
# if CLICK_LINUXMODULE
    if (test_bit(TASK_IS_SCHEDULED, &_from_netlink_element_state) == 1) {
        t->fast_reschedule();
    }
# else
    if ((*(volatile u_long *)(&_from_netlink_element_state)) & 1) {
        t->fast_reschedule();
    }
# endif
    mutex_unlock(&down_mutex);
    return true;
}
#else

void FromNetlink::selected(int fd, int mask) {
    WritablePacket *newPacket;
    int total_buf_size = -1;
    int bytes_read;
    if ((mask & SELECT_READ) == SELECT_READ) {
        /*read from the socket*/
# ifdef __linux__
        total_buf_size = recv(fd, fake_buf, 1, MSG_PEEK | MSG_TRUNC | MSG_WAITALL);
# else
#  ifdef __APPLE__
        socklen_t _option_len = sizeof(total_buf_size);
        if (recv(fd, fake_buf, 1, MSG_PEEK | MSG_DONTWAIT) < 0 || getsockopt(fd, SOL_SOCKET, SO_NREAD, &total_buf_size, &_option_len) < 0)
#  else
        if (recv(fd, fake_buf, 1, MSG_PEEK | MSG_DONTWAIT) < 0 || ioctl(fd, FIONREAD, &total_buf_size) < 0)
#  endif
        {
            click_chatter("recv/ioctl: %d", errno);
            return;
        }
# endif
        if (total_buf_size < 0) {
            click_chatter("Hmmm");
            return;
        }
        newPacket = Packet::make(100, NULL, total_buf_size, 100);
        bytes_read = recv(fd, newPacket->data(), newPacket->length(), MSG_WAITALL);
        if (bytes_read > 0) {
            if ((uint32_t)bytes_read < newPacket->length()) {
                /* truncate to actual length (if total_buf_size > bytes_read
                   despite MSG_WAITALL) */
                newPacket->take(newPacket->length() - bytes_read);
            }
            struct nlmsghdr *nlh = (struct nlmsghdr *) newPacket->data();
            /*pull the netlink header*/
            newPacket->pull(sizeof (nlmsghdr));
            newPacket->set_anno_u32(0, nlh->nlmsg_pid);
            output(0).push(newPacket);
        } else {
            /* recv() returns -1 on error. We treat 0 and -N similarly. */
            click_chatter("recv returned %d", bytes_read);
            newPacket->kill();
        }
    }
}
#endif

CLICK_ENDDECLS
EXPORT_ELEMENT(FromNetlink)
ELEMENT_REQUIRES(Netlink)
