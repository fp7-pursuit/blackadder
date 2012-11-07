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
#include "libpla_connections.h"


/**
 * Keeps track of connections (based on sender's key), useful for tracking
 * sequence numbers
 */


/**
 * Linked list related operations
 */
void 
list_add(struct list_head *new_item, struct list_head *head)
{
    struct list_head *next = head->next;

    next->prev = new_item;
    new_item->next = next;
    new_item->prev = head;
    head->next = new_item;
}


void
list_del(struct list_head *entry)
{
    struct list_head *prev, *next;

    prev = entry->prev;
    next = entry->next;

    next->prev = prev;
    prev->next = next;
}


void
libpla_connections_init(void)
{
    (&connection_list)->next = &connection_list;
    (&connection_list)->prev = &connection_list;
}


void 
libpla_connections_cleanup(void)
{
    libpla_connections_flush();
}


/**
 * Finds pla_connection from connection_list based on key
 *
 * @param key	key (e.g. user's implicit certificate)
 * @return pla_connection entry
 */
struct pla_connection*
libpla_connection_find(char *key)
{
    struct list_head *cursor;

    for (cursor = (&connection_list)->next; cursor->next, cursor != (&connection_list);
        cursor = cursor->next) {
        struct pla_connection *this_entry = (struct pla_connection*) cursor;

        if (memcmp(this_entry->key, key, sizeof(this_entry->key) ) == 0) {
            return this_entry;
        }
    }

    return NULL;
}


/**
 * Frees connection_list
 */
void 
libpla_connections_flush() 
{
    struct list_head *cursor, *next;

    cursor = (&connection_list)->next;

    if (cursor == NULL)
        return;

    for(next = cursor->next; cursor != (&connection_list); cursor = next, next = cursor->next) {

            if ((&connection_list)->next != NULL) {
                struct pla_connection *this_entry = (struct pla_connection *) cursor;
                list_del(cursor);
                free(this_entry);
            }
        }

    return;
}
