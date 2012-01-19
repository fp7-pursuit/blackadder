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

/* Simple text publisher example. */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <blackadder_c.h>

static void print_hex(const char * const,
                      const char * const,
                      const unsigned int);

int main(int argc, char *argv[])
{
    const char sid[] = { 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B };
    const char rid[] = { 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D };
    char id[sizeof(sid)+sizeof(rid)];
    unsigned char strategy = NODE_LOCAL;
    ba_handle ba;

    memcpy(id, sid, sizeof(sid));
    memcpy(id+sizeof(sid), rid, sizeof(rid));

    if (argc >= 2)
        strategy = (unsigned char)atoi(argv[1]);

    ba = ba_instance(1);

    ba_publish_scope(ba, sid, sizeof(rid), "", 0,
                     strategy, (void *)0, 0);
    ba_publish_info(ba, rid, sizeof(rid), sid, sizeof(sid),
                    strategy, (void *)0, 0);

    do {
        ba_event ev = ba_event_new();
        unsigned char *_type = NULL, type;
        ba_get_event(ba, ev);
        ba_event_type(ev, &_type);
        type = *_type;
        ba_event_delete(ev);
        if (type == 0)
            goto disconnect;
        if (type == START_PUBLISH)
            break;
    } while (1);

    do {
        char buf[101];
        int nitems;
        unsigned int len;
        printf("What shall I publish, Sir/Madam? (ctrl-d to quit)\n");
        nitems = scanf("%100s", buf); /* XXX */
        if (nitems == EOF)
            break;
        len = strlen(buf);
        print_hex(NULL, buf, len);
        ba_publish_data(ba, id, sizeof(id), strategy, (void *)0, 0, buf, len);
    } while (1);

 disconnect:
    ba_disconnect(ba);
    ba_delete(ba);

    return 0;
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
