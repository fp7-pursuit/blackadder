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

#ifndef SERVICE_MODEL_H
#define SERVICE_MODEL_H

#include <ns3/object.h>
#include <ns3/log.h>
#include <ns3/application.h>
#include <ns3/callback.h>
#include <ns3/random-variable.h>

#include <vector>
#include <map>
#include <sstream> 
#include <iostream>

#include "click-bridge.h"
#include "event.h"

namespace ns3 {
    /**********************************/
#define PURSUIT_ID_LEN 8 //in bytes
#define FID_LEN 32 //in bytes
#define NODEID_LEN PURSUIT_ID_LEN //in bytes
    /****some strategies*****/
#define NODE_LOCAL          0
#define LINK_LOCAL          1
#define DOMAIN_LOCAL        2
#define IMPLICIT_RENDEZVOUS 3
#define BROADCAST_IF        4
    /************************/
    /*intra and inter click message types*/
#define PUBLISH_SCOPE 0
#define PUBLISH_INFO 1
#define UNPUBLISH_SCOPE 2
#define UNPUBLISH_INFO 3
#define SUBSCRIBE_SCOPE 4
#define SUBSCRIBE_INFO 5
#define UNSUBSCRIBE_SCOPE 6
#define UNSUBSCRIBE_INFO 7
#define PUBLISH_DATA  8 //the request
#define CONNECT 12
#define DISCONNECT 13
    /*****************************/
#define UNDEF_EVENT 0
#define START_PUBLISH 100
#define STOP_PUBLISH 101
#define SCOPE_PUBLISHED 102
#define SCOPE_UNPUBLISHED 103
#define PUBLISHED_DATA 104
#define MATCH_PUB_SUBS 105
#define RV_RESPONSE 106	
#define NETLINK_BADDER 20

    typedef Callback<void, Ptr<Event> > EventListener;

    /**@brief (blackadder Simulation) This Class implements the service model that is exported to all simulated applications.
     * 
     * In order to make the exported API identical to the one used by real blackadder applications we use the PubSubApplication Class. The ServiceModel Class is the one 
     * "communicating" with Blackadder. From the NS3 code, it is not possible to annotate packets using the Process identifier, as it is done in a real Blackadder deployment.
     * Moreover, there is no process identifier because applications are simulated.
     * Therefore, the process identifier, which is randomly selected, is passed in the exported API, although this is hidden to NS3 applications.
     * 
     */
    class ServiceModel : public Object {
    public:
        /**
         * @brief GetTypeId is implemented as mandated by NS3. There are no attributes..
         * @return the type identifier of this Class that extends the NS3 Object Class.
         */
        static TypeId GetTypeId(void);
        /**
         * @brief The constructor initializes the uniform number generator that is used to produce unique, random process identifiers to NS3 applications.
         */
        ServiceModel();
        /**
         * @brief The destructor
         */
        virtual ~ServiceModel();
        /**
         * @brief it registers an NS3 application.
         * 
         * A unique process identifier is calculated and an event listener, passed by an NS3 application, is registered so that it can be called when events from Blackadder must be pushed to the application.
         * It is be called by the PubSubApplication and is hidden by the actual NS3 application.
         * @param app a smart pointer to an application
         * @param el the callback function
         * @return the process identifier for the registered application or -1 if the application is already registered.
         */
        uint32_t RegisterApplication(Ptr<Application> app, EventListener el);
        /**
         * @brief It unregisters an NS3 application. It is be called by the PubSubApplication and is hidden by the actual NS3 application.
         * @param pid the process identifier of the application to be unregistered by the ServiceModel.
         */
        void UnregisterApplication(uint32_t pid);
        /**@A callback that is registered with the ClickBridge (see DoStart) and is called whenever a Blackadder Event needs to be routed to an NS3 application.
         * 
         * @param ifid
         * @param data the data received by Blackadder (from which an Event will be constructed)
         * @param len the length of the received data
         */
        void ClickUpcall(int ifid, const unsigned char *data, int len);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void disconnect(uint32_t pid);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void publish_scope(uint32_t pid, const std::string &id, const std::string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void publish_info(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void unpublish_scope(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void unpublish_info(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void subscribe_scope(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void subscribe_info(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void unsubscribe_scope(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void unsubscribe_info(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void publish_data(uint32_t pid, const std::string&id, unsigned char strategy, void *str_opt, unsigned int str_opt_len, void *data, unsigned int data_len);
        /**
         * @brief The same call is in Blackadder's library
         */
        std::string hex_to_chararray(const std::string &hexstr);
        /**
         * @brief The same call is in Blackadder's library
         */
        std::string chararray_to_hex(const std::string &str);
    private:
        ServiceModel& operator =(const ServiceModel &);
        ServiceModel(const ServiceModel &);
        /**
         * @brief It is called when the simulated object is started. It registers a callback to ClickBridge to receive all events, that must be routed to NS3 applications, by Blackadder.
         */
        void DoStart(void);
        void DoDispose(void);
        /**
         * @brief The same call is in Blackadder's library. The only difference is the process identifier which must be explicitly passed as an argument
         * @param pid the random, unique process identifier
         */
        void create_and_send_buffers(uint32_t pid, unsigned char type, const std::string &id, const std::string &prefix_id, char strategy, void *str_opt, unsigned int str_opt_len);
        /*members*/
        /**@brief a map that stores process identifiers mapped to smart pointers to applications.
         */
        std::map<uint32_t, Ptr<Application> > m_applications;
        /**@brief a map that stores smart pointers to applications mapped to Event Listeners (callback methods registered by NS3 applications)
         */
        std::map<Ptr<Application>, EventListener > m_eventListeners;
        /**@brief a process identifier uniform number generator.
         */
        UniformVariable m_pid_generator;
    };
}

#endif
