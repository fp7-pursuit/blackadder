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

#include "topology-manager.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("TopologyManager");
    NS_OBJECT_ENSURE_REGISTERED(TopologyManager);

    TypeId TopologyManager::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::TopologyManager")
                .SetParent<PubSubApplication > ()
                .AddConstructor<TopologyManager > ()
                .AddAttribute("Topology",
                "The file that describes the topology that will be used by the Topology Manager.",
                StringValue("topology.graphml"),
                MakeStringAccessor(&TopologyManager::m_filename),
                MakeStringChecker());
        return tid;
    }

    TopologyManager::TopologyManager() {
    }

    TopologyManager::~TopologyManager() {
    }

    void TopologyManager::DoStart(void) {
        NS_LOG_FUNCTION(this);
        m_cb = MakeCallback(&TopologyManager::EventHandler, this);
        PubSubApplication::DoStart();
    }

    void TopologyManager::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        PubSubApplication::DoDispose();
    }

    void TopologyManager::StartApplication(void) {
        NS_LOG_FUNCTION(this);
        PubSubApplication::StartApplication();
        /*read the graphML file that describes the topology*/
        NS_LOG_INFO("FILENAME: " << m_filename);
        NS_ASSERT_MSG(m_tm_igraph.readTopology(m_filename.c_str()) >= 0, "TM: couldn't read topology file...aborting");
        NS_LOG_INFO("Blackadder Node: " << m_tm_igraph.nodeID);
        /***************************************************/
        /*I should write a read hander that reads the Node Identifier (and other useful information)*/
        NS_LOG_INFO("Started Topology Manager");
        m_req_id = "FFFFFFFFFFFFFFFE";
        m_req_prefix_id = std::string();
        m_req_bin_id = hex_to_chararray(m_req_id);
        m_req_bin_prefix_id = hex_to_chararray(m_req_prefix_id);
        m_resp_id = std::string();
        m_resp_prefix_id = "FFFFFFFFFFFFFFFD";
        m_resp_bin_id = hex_to_chararray(m_resp_id);
        m_resp_bin_prefix_id = hex_to_chararray(m_resp_prefix_id);
        subscribe_scope(m_req_bin_id, m_req_bin_prefix_id, IMPLICIT_RENDEZVOUS, NULL, 0);
    }

    void TopologyManager::StopApplication() {
        NS_LOG_FUNCTION(this);
        PubSubApplication::StopApplication();
    }

    void TopologyManager::EventHandler(Ptr<Event> ev) {
        NS_LOG_FUNCTION(this);
        switch (ev->type) {
            case SCOPE_PUBLISHED:
                NS_LOG_INFO("SCOPE_PUBLISHED: " << chararray_to_hex(ev->id) << "..NOT SUPPORTED EVENT");
                break;
            case SCOPE_UNPUBLISHED:
                NS_LOG_INFO("SCOPE_UNPUBLISHED: " << chararray_to_hex(ev->id) << "..NOT SUPPORTED EVENT");
                break;
            case START_PUBLISH:
                NS_LOG_INFO("START_PUBLISH: " << chararray_to_hex(ev->id) << "..NOT SUPPORTED EVENT");
                break;
            case STOP_PUBLISH:
                NS_LOG_INFO("STOP_PUBLISH: " << chararray_to_hex(ev->id) << "..NOT SUPPORTED EVENT");
                break;
            case PUBLISHED_DATA:
                handleRequest((char *) ev->data, ev->data_len);
                break;
        }
    }

    void TopologyManager::handleRequest(char *request, int request_len) {
        NS_LOG_FUNCTION(this);
        unsigned char request_type;
        unsigned char no_publishers;
        unsigned char no_subscribers;
        std::string nodeID;
        std::set<std::string> publishers;
        std::set<std::string> subscribers;
        std::map<std::string, Ptr<BitVector> > result = std::map<std::string, Ptr<BitVector> >();
        std::map<std::string, Ptr<BitVector> >::iterator map_iter;
        unsigned char response_type;
        int idx = 0;
        unsigned char strategy;
        memcpy(&request_type, request, sizeof (request_type));
        memcpy(&strategy, request + sizeof (request_type), sizeof (strategy));
        if (request_type == MATCH_PUB_SUBS) {
            /*this a request for topology formation*/
            memcpy(&no_publishers, request + sizeof (request_type) + sizeof (strategy), sizeof (no_publishers));
            //NS_LOG_INFO("Publishers: ");
            for (int i = 0; i < (int) no_publishers; i++) {
                nodeID = std::string(request + sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + idx, PURSUIT_ID_LEN);
                //NS_LOG_INFO(nodeID << " ");
                idx += PURSUIT_ID_LEN;
                publishers.insert(nodeID);
            }
            //NS_LOG_INFO("");
            //NS_LOG_INFO("Subscribers: ");
            memcpy(&no_subscribers, request + sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + idx, sizeof (no_subscribers));
            for (int i = 0; i < (int) no_subscribers; i++) {
                nodeID = std::string(request + sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + sizeof (no_subscribers) + idx, PURSUIT_ID_LEN);
                //NS_LOG_INFO(nodeID << " ");
                idx += PURSUIT_ID_LEN;
                subscribers.insert(nodeID);
            }
            //NS_LOG_INFO("");

            m_tm_igraph.calculateFID(publishers, subscribers, result);

            /*notify publishers*/
            for (map_iter = result.begin(); map_iter != result.end(); map_iter++) {
                if ((*map_iter).second == NULL) {
                    //NS_LOG_INFO("Publisher " << (*map_iter).first << ", FID: NULL");
                    response_type = STOP_PUBLISH;
                    int response_size = request_len - sizeof (strategy) - sizeof (no_publishers) - no_publishers * PURSUIT_ID_LEN - sizeof (no_subscribers) - no_subscribers * PURSUIT_ID_LEN;
                    char *response = (char *) malloc(response_size);
                    memcpy(response, &response_type, sizeof (response_type));
                    int ids_index = sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + no_publishers * PURSUIT_ID_LEN + sizeof (no_subscribers) + no_subscribers * PURSUIT_ID_LEN;
                    memcpy(response + sizeof (response_type), request + ids_index, request_len - ids_index);
                    /*find the FID to the publisher*/
                    std::string destination = (*map_iter).first;
                    Ptr<BitVector> FID_to_publisher = m_tm_igraph.calculateFID(m_tm_igraph.nodeID, destination);
                    std::string response_id = m_resp_bin_prefix_id + (*map_iter).first;
                    publish_data(response_id, IMPLICIT_RENDEZVOUS, (char *) FID_to_publisher->_data, FID_LEN, response, response_size);
                    free(response);
                } else {
                    //NS_LOG_INFO("Publisher " << (*map_iter).first << ", FID: " << (*map_iter).second->to_string());
                    response_type = START_PUBLISH;
                    int response_size = request_len - sizeof (strategy) - sizeof (no_publishers) - no_publishers * PURSUIT_ID_LEN - sizeof (no_subscribers) - no_subscribers * PURSUIT_ID_LEN + FID_LEN;
                    char *response = (char *) malloc(response_size);
                    memcpy(response, &response_type, sizeof (response_type));
                    int ids_index = sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + no_publishers * PURSUIT_ID_LEN + sizeof (no_subscribers) + no_subscribers * PURSUIT_ID_LEN;
                    memcpy(response + sizeof (response_type), request + ids_index, request_len - ids_index);
                    memcpy(response + sizeof (response_type) + request_len - ids_index, (*map_iter).second->_data, FID_LEN);
                    /*find the FID to the publisher*/
                    std::string destination = (*map_iter).first;
                    Ptr<BitVector> FID_to_publisher = m_tm_igraph.calculateFID(m_tm_igraph.nodeID, destination);
                    std::string response_id = m_resp_bin_prefix_id + (*map_iter).first;
                    publish_data(response_id, IMPLICIT_RENDEZVOUS, (char *) FID_to_publisher->_data, FID_LEN, response, response_size);
                    free(response);
                }
            }
            result.clear();
        } else if ((request_type == SCOPE_PUBLISHED) || (request_type == SCOPE_UNPUBLISHED)) {
            /*this a request to notify subscribers about a new scope*/
            memcpy(&no_subscribers, request + sizeof (request_type) + sizeof (strategy), sizeof (no_subscribers));
            for (int i = 0; i < (int) no_subscribers; i++) {
                nodeID = std::string(request + sizeof (request_type) + sizeof (strategy) + sizeof (no_subscribers) + idx, PURSUIT_ID_LEN);
                Ptr<BitVector> FID_to_subscriber = m_tm_igraph.calculateFID(m_tm_igraph.nodeID, nodeID);
                int response_size = request_len - sizeof (strategy) - sizeof (no_subscribers) - no_subscribers * PURSUIT_ID_LEN + FID_LEN;
                int ids_index = sizeof (request_type) + sizeof (strategy) + sizeof (no_subscribers) + no_subscribers * PURSUIT_ID_LEN;
                char *response = (char *) malloc(response_size);
                std::string response_id = m_resp_bin_prefix_id + nodeID;
                memcpy(response, &request_type, sizeof (request_type));
                memcpy(response + sizeof (request_type), request + ids_index, request_len - ids_index);
                //NS_LOG_INFO("PUBLISHING NOTIFICATION ABOUT NEW OR DELETED SCOPE to node " << nodeID << " using FID " << FID_to_subscriber->to_string());
                publish_data(response_id, IMPLICIT_RENDEZVOUS, FID_to_subscriber->_data, FID_LEN, response, response_size);
                idx += PURSUIT_ID_LEN;
                free(response);
            }
        }
    }

} // Namespace ns3
