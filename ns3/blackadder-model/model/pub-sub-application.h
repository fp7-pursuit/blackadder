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

#ifndef PUB_SUB_APPLICATION_H
#define PUB_SUB_APPLICATION_H

#include <ns3/log.h>
#include <ns3/application.h>

#include "service-model.h"

namespace ns3 {

    /**@brief (blackadder Simulation) This Class is basically a wrapper Class to hide some implementation details of ServiceModel from NS3 applications.
     * 
     * All NS3 applications MUST extend this Class.
     */
    class PubSubApplication : public Application {
    public:
        static TypeId GetTypeId(void);
        PubSubApplication();
        virtual ~PubSubApplication();
        /**@brief a wrapper for the same method in ServiceModel that hides the process identifier from the NS3 application.
         */
        void publish_scope(const std::string &id, const std::string &prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**@brief a wrapper for the same method in ServiceModel that hides the process identifier from the NS3 application.
         */
        void publish_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**@brief a wrapper for the same method in ServiceModel that hides the process identifier from the NS3 application.
         */
        void unpublish_scope(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**@brief a wrapper for the same method in ServiceModel that hides the process identifier from the NS3 application.
         */
        void unpublish_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**@brief a wrapper for the same method in ServiceModel that hides the process identifier from the NS3 application.
         */
        void subscribe_scope(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**@brief a wrapper for the same method in ServiceModel that hides the process identifier from the NS3 application.
         */
        void subscribe_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**@brief a wrapper for the same method in ServiceModel that hides the process identifier from the NS3 application.
         */
        void unsubscribe_scope(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**@brief a wrapper for the same method in ServiceModel that hides the process identifier from the NS3 application.
         */
        void unsubscribe_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
        /**@brief a wrapper for the same method in ServiceModel that hides the process identifier from the NS3 application.
         */
        void publish_data(const std::string&id, unsigned char strategy, void *str_opt, unsigned int str_opt_len, void *data, unsigned int data_len);
        std::string hex_to_chararray(const std::string &hexstr);
        std::string chararray_to_hex(const std::string &str);
        /**@brief the default EventListener. It does NOTHING and it must be overridden by an NS3 application.
         * 
         * @param ev
         */
        void EventHandler(Ptr<Event> ev);
    protected:
        void DoStart(void);
        void DoDispose(void);
        /**@brief Called by the simulator when the time has come for the NS3 application to start. It gets a reference of the ServiceModel Class and registers the application with the ServiceModel.
         * The process identifier is acquired and used with all service model calls.
         */
        void StartApplication(void);
        /**@brief Called by the simulator when the time has come for the NS3 application to stop. It unregisters the application from the ServiceModel Class.
         */
        void StopApplication();
        /*members*/
        /**@brief the EventListener passed by an NS3 application.
         */
        EventListener m_cb;
    private:
        PubSubApplication& operator =(const PubSubApplication &);
        PubSubApplication(const PubSubApplication &);
        Ptr<ServiceModel> servModel;
        uint32_t m_pid;
    };
}

#endif
