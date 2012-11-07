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

#include "subscriber.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("Subscriber");
    NS_OBJECT_ENSURE_REGISTERED(Subscriber);

    TypeId Subscriber::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::Subscriber")
                .SetParent<PubSubApplication > ()
                .AddConstructor<Subscriber > ();
        return tid;
    }

    Subscriber::Subscriber() {
        m_Event = EventId();
    }

    Subscriber::~Subscriber() {
    }

    void Subscriber::DoStart(void) {
        NS_LOG_FUNCTION(this);
        m_cb = MakeCallback(&Subscriber::EventHandler, this);
        PubSubApplication::DoStart();
    }

    void Subscriber::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        PubSubApplication::DoDispose();
    }

    void Subscriber::StartApplication(void) {
        NS_LOG_FUNCTION(this);
        PubSubApplication::StartApplication();
        ScheduleTransmit(Seconds(0.));
    }

    void Subscriber::StopApplication() {
        NS_LOG_FUNCTION(this);
        Simulator::Cancel(m_Event);
        PubSubApplication::StopApplication();
    }

    void Subscriber::ScheduleTransmit(Time dt) {
        NS_LOG_FUNCTION(this);
        m_Event = Simulator::Schedule(dt, &Subscriber::Send, this);
    }

    void Subscriber::Send(void) {
        NS_LOG_FUNCTION(this);
        NS_ASSERT(m_Event.IsExpired());
        std::string id = "0000000000000000";
        std::string prefix_id;
        std::string bin_id = hex_to_chararray(id);
        std::string bin_prefix_id = hex_to_chararray(prefix_id);
        subscribe_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    }

    void Subscriber::EventHandler(Ptr<Event> ev) {
        switch (ev->type) {
            case SCOPE_PUBLISHED:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": SCOPE_PUBLISHED: " << chararray_to_hex(ev->id));
                break;
            case SCOPE_UNPUBLISHED:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": SCOPE_UNPUBLISHED: " << chararray_to_hex(ev->id));
                break;
            case START_PUBLISH:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": START_PUBLISH: " << chararray_to_hex(ev->id));
                break;
            case STOP_PUBLISH:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": STOP_PUBLISH: " << chararray_to_hex(ev->id));
                break;
            case PUBLISHED_DATA:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": PUBLISHED_DATA: " << chararray_to_hex(ev->id));
                //NS_LOG_INFO("data size: " << ev->data_len);
                //NS_LOG_INFO(std::string((char *)ev->data, ev->data_len));
                break;
        }
    }

} // Namespace ns3
