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

#include "publisher.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("Publisher");
    NS_OBJECT_ENSURE_REGISTERED(Publisher);

    TypeId Publisher::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::Publisher")
                .SetParent<PubSubApplication > ()
                .AddConstructor<Publisher > ();
        return tid;
    }

    Publisher::Publisher() {
        m_Event = EventId();
    }

    Publisher::~Publisher() {
    }

    void Publisher::DoStart(void) {
        NS_LOG_FUNCTION(this);
        m_cb = MakeCallback(&Publisher::EventHandler, this);
        PubSubApplication::DoStart();
    }

    void Publisher::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        PubSubApplication::DoDispose();
    }

    void Publisher::StartApplication(void) {
        NS_LOG_FUNCTION(this);
        PubSubApplication::StartApplication();
        std::string id = "0000000000000000";
        std::string prefix_id;
        std::string bin_id = hex_to_chararray(id);
        std::string bin_prefix_id = hex_to_chararray(prefix_id);
        publish_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
        m_Event = Simulator::Schedule(Seconds(0.), &Publisher::publish, this);
    }

    void Publisher::StopApplication() {
        NS_LOG_FUNCTION(this);
        Simulator::Cancel(m_Event);
        PubSubApplication::StopApplication();
    }

    void Publisher::publish(void) {
        NS_LOG_FUNCTION(this);
        NS_ASSERT(m_Event.IsExpired());
        std::string prefix_id = "0000000000000000";
        UniformVariable part1;
        double d1;
        char *testID = (char *) malloc(8);
        d1 = part1.GetValue();
        memcpy(testID, &d1, sizeof (double));
        std::string FINALID(testID, sizeof (double));
        std::string bin_prefix_id = hex_to_chararray(prefix_id);
        publish_info(FINALID, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
        free(testID);
        m_Event = Simulator::Schedule(MilliSeconds(500.), &Publisher::publish, this);
    }

    void Publisher::EventHandler(Ptr<Event> ev) {
        char *payload;
        switch (ev->type) {
            case SCOPE_PUBLISHED:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": SCOPE_PUBLISHED: " << chararray_to_hex(ev->id));
                break;
            case SCOPE_UNPUBLISHED:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": SCOPE_UNPUBLISHED: " << chararray_to_hex(ev->id));
                break;
            case START_PUBLISH:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": START_PUBLISH: " << chararray_to_hex(ev->id));
                payload = (char *) malloc(1000);
                memset(payload, 'A', 1000);
                publish_data(ev->id, DOMAIN_LOCAL, NULL, 0, payload, 1000);
                free(payload);
                break;
            case STOP_PUBLISH:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": STOP_PUBLISH: " << chararray_to_hex(ev->id));
                break;
            case PUBLISHED_DATA:
                NS_LOG_INFO("Node " << GetNode()->GetId() << ": PUBLISHED_DATA: " << chararray_to_hex(ev->id));
                break;
        }
    }

} // Namespace ns3
