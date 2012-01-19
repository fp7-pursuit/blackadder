/*-
 * Copyright (C) 2011  Oy L M Ericsson Ab, NomadicLab
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

/* Simple text subscriber example (non-blocking). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nb_blackadder_c.h>

static void event_handler(ba_event);
static void print_hex(const char * const,
                      const char * const,
                      const unsigned int);

int main(int argc, char *argv[])
{
    const char sid[] = { 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B };
    const char rid[] = { 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D };
    unsigned char strategy = NODE_LOCAL;
    nb_ba_handle ba;

    if (argc >= 2)
        strategy = (unsigned char)atoi(argv[1]);

    ba = nb_ba_instance(1);
    nb_ba_set_callback(ba, event_handler);

    nb_ba_subscribe_info(ba, rid, sizeof(rid), sid, sizeof(sid),
                         NODE_LOCAL, (void *)0, 0);

    nb_ba_join(ba);
    nb_ba_disconnect(ba);
    nb_ba_delete(ba);

    return 0;
}

static void event_handler(ba_event ev)
{
    unsigned char *type = NULL;
    const char *id = NULL;
    unsigned int id_len = 0;
    void *data = NULL;
    unsigned int *data_len = NULL;

    if (ev == NULL) {
        fprintf(stderr, "Event is NULL");
    }

    ba_event_type(ev, &type);
    ba_event_id(ev, &id, &id_len);
    ba_event_data(ev, &data, &data_len);

    printf("type=%u\n", *type);
    print_hex("id", id, id_len);

    if (*type == PUBLISHED_DATA)
        print_hex(NULL, data, *data_len);

    ba_event_delete(ev);
}

static void print_hex(const char * const label,
                      const char * const bin,
                      const unsigned int len)
{
    unsigned int i;

    printf("%s%s\"", (label != NULL) ? label : "", (label != NULL) ? "=" : "");
    for (i = 0; i < len; i++) {
        printf("%02x", bin[i]);
    }
    printf("\"\n");
}
