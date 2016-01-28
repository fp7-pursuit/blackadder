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

#ifndef TOPOLOGY_MANAGER_H
#define TOPOLOGY_MANAGER_H

#include <ns3/ptr.h>

#include <ns3/string.h>

#include <map>
#include <set>

#include "pub-sub-application.h"
#include "bit-vector.h"
#include "tm_igraph.h"


namespace ns3 {
    /**@brief (blackadder Simulation) This Class implements a NS3 application that implements the Topology Management core network function. As any other NS3 application, it extends the PubSubApplication Class.
     */
    class TopologyManager : public PubSubApplication {
    public:
        /**@brief as mandated by NS3. The only attribute is the filename. The default value is topology.graphml. The attribute can be set in the NS3 C++ topology code.
         */
        static TypeId GetTypeId(void);
        /**@brief Constructor
         */
        TopologyManager();
        /**@brief Destructor
         */
        virtual ~TopologyManager();
        /**@brief the callback function registered to receive events. It is only interested about PUBLISHED_DATA events because the TM only receives topology formation requests as published data by an RV node.
         * 
         * @param ev a smart pointer to an Event.
         */
        void EventHandler(Ptr<Event> ev);
    protected:
        /**@brief Creates a callback function that will be called when a new event is pushed from Blackadder.
         */
        void DoInitialize(void);
        virtual void DoDispose(void);
    private:
        /**@brief When the Topology Manager is started, the topology file is read and a subscription with IMPLICIT_RENDEZVOUSstrategy is sent to blackadder so that topology requests can be later received.
         */
        virtual void StartApplication(void);
        /*@brief the nothing happens here. The PubSubApplication Class is responsible for unregistering the application from the ServiceModel which in turn sends a "disconnect" message to Blackadder so that state can be released.
         */
        virtual void StopApplication(void);
        /**@brief it handles a request for topology formation sent by a rendezvous node. Just like in a real Blackadder deployment.
         * 
         * @param request request payload
         * @param request_len request length
         */
        void handleRequest(char *request, int request_len);

        std::string m_req_id;
        std::string m_req_prefix_id;
        std::string m_resp_id;
        std::string m_resp_prefix_id;
        std::string m_req_bin_id;
        std::string m_req_bin_prefix_id;
        std::string m_resp_bin_id;
        std::string m_resp_bin_prefix_id;

        std::string m_filename;
        TMIgraph m_tm_igraph;
    };

} // namespace ns3

#endif
