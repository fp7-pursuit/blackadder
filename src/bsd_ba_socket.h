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

/**
 * @file bsd_ba_socket.h
 * @brief Blackadder domain and socket implementation for FreeBSD kernels.
 */

#ifndef CLICK_BSD_BA_SOCKET_H
#define CLICK_BSD_BA_SOCKET_H

#if CLICK_BSDMODULE
#define mutex_lock mtx_lock
#define mutex_unlock mtx_unlock
#define mutex mtx /* XXXXXX */
#define pid proc /* XXXXXX */
#define task_struct proc /* XXXXXX */
#endif

/** @brief Blackadder address family. */
#define AF_BLACKADDER 134 /* XXX */
#define PROTO_BLACKADDER 1 /* XXX */

/**
 * @brief Send packets to userlevel.
 *
 * Sends packets to a userlevel socket.
 *
 * @param  m mbuf to send.
 * @param  pid ID of destination process.
 * @return Error code. 0 on success, errno on failure.
 */
int ba_to_socket(struct mbuf *m, pid_t pid);

/**
 * @brief Initialization function.
 *
 * Initializes the socket implementation.
 * Intended to be called when the module is loaded.
 *
 * @param  from_socket Callback function for packets sent from userlevel.
 * @return Error code. 0 on success, errno on failure.
 */
int ba_socket_init(void (*from_socket)(struct mbuf *));

/**
 * @brief Cleanup function.
 *
 * Performs cleanup operations.
 * Intended to be called when the module is unloaded.
 *
 * @return Error code. 0 on success, errno on failure.
 */
int ba_socket_cleanup(void);

#endif /* CLICK_BSD_BA_SOCKET_H */
