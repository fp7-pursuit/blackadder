/*
* Copyright (c) 2008, Dmitrij Lagutin, HIIT, <dmitrij.lagutin@hiit.fi>,
* Janne Lundberg <janne.lundberg@iki.fi>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* Alternatively, this software may be distributed under the terms of BSD
* license.
*
* See LICENSE and COPYING for more details.
*/
#ifndef _LIBPLA_CONNECTIONS_H
#define _LIBPLA_CONNECTIONS_H

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <sys/param.h>
#include <sys/queue.h>

#include "libpla_configs.h"

struct list_head 
{
    struct list_head *next, *prev;
};

//LIST_HEAD(connection_list, pla_connection) connection_list_head;

struct pla_connection 
{
    struct list_head list_head;
    //LIST_ENTRY(pla_connection) entries;

    uint64_t window_low;            /**< Smallest acceptable sequence number */
    uint16_t low_index;             /**< Index of the low point */

    char window[PLA_WINDOW_SIZE];
    char key[PLA_PUBKEY_LEN];
};

void libpla_connections_init(void);
void libpla_connections_cleanup(void);

void list_add(struct list_head *new_item, struct list_head *head);
void list_del(struct list_head *entry);

struct pla_connection* libpla_connection_find(char *key);
void libpla_connections_flush();

extern struct list_head connection_list;
#endif /* _LIBPLA_CONNECTIONS_H */
