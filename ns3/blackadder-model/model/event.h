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

#ifndef EVENT_H
#define EVENT_H

#include <ns3/simple-ref-count.h>
#include <ns3/log.h>
#include <string.h>


namespace ns3 {

    /**@brief (blackadder Simulation) This Class defines the Events that can be sent to a Simulated Application
     * 
     * NS3 integration with Blackadder is inherently asynchronous. Therefore, all applications can asynchronously receive Events (instances of the Event Class).
     * The Events have the same structure as the ones received in a non-simulated Blackadder deployment.
     */
    class Event : public SimpleRefCount<Event> {
    public:
        Event();
        virtual ~Event();
        Event(Event &ev);
        Event& operator=(const Event &ev);
        /**@brief The type of the Event.
         * 
         * It can be: 
         * 
         * START_PUBLISH 
         * 
         * STOP_PUBLISH 
         * 
         * SCOPE_PUBLISHED 
         * 
         * SCOPE_UNPUBLISHED 
         * 
         * PUBLISHED_DATA 
         * 
         * Events are handled by applications arbitrarily (e.g. one could publish a million buffers after receiving a START_PUBLISH or one could subscribe to the scope after receiving a SCOPE_PUBLISHED Event).
         */
        unsigned char type;
        /**@brief The full identifier of the scope or information item reffered by the event.
         */
        std::string id;
        /**@brief The data that accompany a PUBLISHED_DATA Event
         */
        unsigned char *data;
        /**@brief The data length of the data that accompany a PUBLISHED_DATA Event
         */
        uint32_t data_len;
    };
}
#endif