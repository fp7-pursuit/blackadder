/*-
 * Copyright (C) 2012  Oy L M Ericsson Ab, NomadicLab
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of the
 * BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

/* Blackadder domain and socket implementation for FreeBSD kernels. */

#include <sys/param.h>
#include <sys/domain.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/vnet.h>

#include <click/config.h>

#include "bsd_ba_socket.h"

/* Control block */
struct bapcb {
    struct socket     *ba_socket;
    struct mtx	      mtx;
    LIST_ENTRY(bapcb) socks;
    pid_t             pid;
};

#define sotobapcb(so) ((struct bapcb *)(so)->so_pcb)

/* Buffer space */
static u_long ba_sendspace = 16 * 1024; /* really max datagram size */
static u_long ba_recvspace = 32 * 1024;

/* List of all sockets (for netstat -f netgraph) */
static LIST_HEAD(, bapcb) _ba_socklist, *ba_socklist = &_ba_socklist;
static struct mtx _ba_socklist_mtx, *ba_socklist_mtx = &_ba_socklist_mtx;

static void (*_from_socket)(struct mbuf *m) = NULL;

/*
 * Called when a socket is attached.
 */
static int
ba_socket_attach(struct socket *so, int proto, struct thread *td)
{
    struct bapcb *pcbp = sotobapcb(so);
    int error;

    if (pcbp != NULL)
        return (EISCONN);

    /* Standard socket setup stuff. */
    error = soreserve(so, ba_sendspace, ba_recvspace);
    if (error)
        return (error);

    /* Allocate the pcb. */
    pcbp = malloc(sizeof(struct bapcb), M_PCB, M_WAITOK | M_ZERO);

    /* Link the pcb and the socket. */
    so->so_pcb = (caddr_t)pcbp;
    pcbp->ba_socket = so;

    /* Initialize mutex. */
    mtx_init(&pcbp->mtx, "ba_socket", NULL, MTX_DEF);

    /* Add the socket to linked list */
    mtx_lock(ba_socklist_mtx);
    LIST_INSERT_HEAD(ba_socklist, pcbp, socks);
    mtx_unlock(ba_socklist_mtx);

    /* Set PID (XXX). */
    pcbp->pid = td->td_proc->p_pid;

    return (0);
}

/*
 * Called when a socket is detached.
 */
static void
ba_socket_detach(struct socket *so)
{
    struct bapcb *const pcbp = sotobapcb(so);

    if (pcbp == NULL)
        return;

    mtx_lock(&pcbp->mtx);
    sbrelease(&so->so_snd, so); /* XXX */
    sbrelease(&so->so_rcv, so); /* XXX */
    mtx_destroy(&pcbp->mtx);

    pcbp->ba_socket->so_pcb = NULL;

    mtx_lock(ba_socklist_mtx);
    LIST_REMOVE(pcbp, socks);
    mtx_unlock(ba_socklist_mtx);

    free(pcbp, M_PCB);
}

/*
 * Called when a packet is sent from userlevel.
 */
static int
ba_socket_send(struct socket *so, int flags, struct mbuf *m, struct sockaddr *addr,
         struct mbuf *control, struct thread *td)
{
        struct bapcb *const pcbp = sotobapcb(so);
        int error;

        if ((pcbp == NULL) || (control != NULL)) {
            error = EINVAL;
            goto release;
        }

        if (_from_socket == NULL) {
            error = EPROTO; /* XXX */
            goto release;
        }

        _from_socket(m); /* callback takes care of mbuf */
        error = 0;
        goto release1;

 release:
        if (m != NULL)
            m_freem(m);
 release1:
        if (control != NULL)
            m_freem(control);
        return (error);
}

static int
dummy_disconnect(struct socket *so)
{
    return (0);
}

int
ba_to_socket(struct mbuf *m, pid_t pid)
{
    struct bapcb *pcbp = NULL;
    struct socket *so = NULL;
    struct sockaddr sa = { 0, 0, { 0 } };
    int error = ESRCH;

    if (m == NULL)
        return EINVAL;

    mtx_lock(ba_socklist_mtx);
    LIST_FOREACH(pcbp, ba_socklist, socks) {
        /* XXX: This linked list implies relatively poor scalability. */
        if (pcbp->pid == pid) {
            so = pcbp->ba_socket;
            if (sbappendaddr(&so->so_rcv, (struct sockaddr *)&sa, m, NULL) == 0) {
                error = ENOBUFS;
            } else {
                sorwakeup(so);
                error = 0;
            }
            break;
        }
    }
    mtx_unlock(ba_socklist_mtx);

    if (error != 0)
        m_free(m);

    return error;
}

static struct pr_usrreqs ba_usrreqs = {
    .pru_abort =      NULL,
    .pru_attach =     ba_socket_attach,
    .pru_bind =       NULL, /*ba_bind,*/
    .pru_connect =    NULL, /*ba_connect,*/
    .pru_detach =     ba_socket_detach,
    .pru_disconnect = dummy_disconnect,
    .pru_peeraddr =   NULL,
    .pru_send =       ba_socket_send,
    .pru_shutdown =   NULL,
    .pru_sockaddr =   NULL, /*ba_getsockaddr,*/
    .pru_sopoll =     sopoll_generic,
    .pru_soreceive =  soreceive_generic,
    .pru_sosend =     sosend_generic,
    .pru_close =      NULL, /*ba_close,*/
};

/*
 * Definitions of protocols supported in the BLACKADDER domain.
 */

extern struct domain badomain; /* stop compiler warnings */

static struct protosw basw[] = {
{
    .pr_type =      SOCK_RAW,
    .pr_domain =    &badomain,
    .pr_protocol =  PROTO_BLACKADDER,
    .pr_flags =	    PR_ATOMIC | PR_ADDR /* | PR_RIGHTS */,
    .pr_usrreqs =   &ba_usrreqs
},
};

struct domain badomain = {
    .dom_family =          AF_BLACKADDER,
    .dom_name =            "blackadder",
    .dom_protosw =         basw,
    .dom_protoswNPROTOSW = &basw[sizeof(basw) / sizeof(basw[0])]
};

int
ba_socket_init(void (*from_socket)(struct mbuf *))
{
    mtx_init(ba_socklist_mtx, "ba_socklist", NULL, MTX_DEF);
    _from_socket = from_socket;

    domain_init(&badomain);
    domain_add(&badomain);

    return 0;
}

extern struct mtx dom_mtx;

int
ba_socket_cleanup(void)
{
    int error = 0;

    mtx_lock(ba_socklist_mtx);
    if (!LIST_EMPTY(ba_socklist)) {
        mtx_unlock(ba_socklist_mtx);
        printf("ba_socklist not empty");
	return EBUSY;
    }
    mtx_destroy(ba_socklist_mtx);

    _from_socket = NULL;

    mtx_lock(&dom_mtx);
    if (domains != &badomain) {
	error = EBUSY;
	goto done;
    }

    domains = domains->dom_next;
    error = 0;

    //vnet_domain_uninit(&badomain);

 done:
    mtx_unlock(&dom_mtx);

    return error;
}

//VNET_DOMAIN_SET(ba);

ELEMENT_PROVIDES(bsd_ba_socket)
ELEMENT_REQUIRES(bsdmodule)
