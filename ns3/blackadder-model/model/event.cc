/*
 * Copyright (C) 2010-2012  George Parisis and Dirk Trossen
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

#include "event.h"

NS_LOG_COMPONENT_DEFINE("Event");

namespace ns3 {

    Event::Event() : type(0), id(), data(NULL), data_len(0) {
    }

    Event::~Event() {
        if (data != NULL) {
            free(data);
        }
    }

    Event::Event(Event &ev) {
        type = ev.type;
        id = ev.id;
        data_len = ev.data_len;
        data = (unsigned char *) malloc(data_len);
        memcpy(data, ev.data, data_len);
    }
}