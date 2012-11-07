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

#include "pub-sub-application.h"

NS_LOG_COMPONENT_DEFINE("PubSubApplication");

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(PubSubApplication);

    TypeId PubSubApplication::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::PubSubApplication")
                .SetParent<Application > ()
                .AddConstructor<PubSubApplication > ();
        return tid;
    }

    PubSubApplication::PubSubApplication() {
        m_cb = MakeCallback(&PubSubApplication::EventHandler, this);
    }

    PubSubApplication::~PubSubApplication() {
    }

    void PubSubApplication::DoStart(void) {
        NS_LOG_FUNCTION(this);
        Application::DoStart();
    }

    void PubSubApplication::DoDispose() {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    void PubSubApplication::StartApplication(void) {
        NS_LOG_FUNCTION(this);
        /*I must get a reference (a Ptr) to the service model*/
        servModel = GetNode()->GetObject<ServiceModel > ();
        NS_ASSERT(servModel != 0);
        /*I must register this application with the service model (like having a pid with the netlink socket)*/
        m_pid = servModel->RegisterApplication(this, m_cb);
    }

    void PubSubApplication::StopApplication() {
        NS_LOG_FUNCTION(this);
        servModel->UnregisterApplication(m_pid);
    }
    
    void PubSubApplication::publish_scope(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        servModel->publish_scope(m_pid, id, prefix_id, strategy, str_opt, str_opt_len);
    }

    void PubSubApplication::publish_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        servModel->publish_info(m_pid, id, prefix_id, strategy, str_opt, str_opt_len);
    }

    void PubSubApplication::unpublish_scope(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        servModel->unpublish_scope(m_pid, id, prefix_id, strategy, str_opt, str_opt_len);
    }

    void PubSubApplication::unpublish_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        servModel->unpublish_info(m_pid, id, prefix_id, strategy, str_opt, str_opt_len);
    }

    void PubSubApplication::subscribe_scope(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        servModel->subscribe_scope(m_pid, id, prefix_id, strategy, str_opt, str_opt_len);
    }

    void PubSubApplication::subscribe_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        servModel->subscribe_info(m_pid, id, prefix_id, strategy, str_opt, str_opt_len);
    }

    void PubSubApplication::unsubscribe_scope(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        servModel->unsubscribe_scope(m_pid, id, prefix_id, strategy, str_opt, str_opt_len);
    }

    void PubSubApplication::unsubscribe_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        servModel->unsubscribe_info(m_pid, id, prefix_id, strategy, str_opt, str_opt_len);
    }

    void PubSubApplication::publish_data(const std::string&id, unsigned char strategy, void *str_opt, unsigned int str_opt_len, void *data, unsigned int data_len) {
        NS_LOG_FUNCTION(this);
        servModel->publish_data(m_pid, id, strategy, str_opt, str_opt_len, data, data_len);
    }

    std::string PubSubApplication::hex_to_chararray(const std::string &hexstr) {
        NS_LOG_FUNCTION(this);
        return servModel->hex_to_chararray(hexstr);
    }

    std::string PubSubApplication::chararray_to_hex(const std::string &str) {
        NS_LOG_FUNCTION(this);
        return servModel->chararray_to_hex(str);
    }

    void PubSubApplication::EventHandler(Ptr<Event> ev) {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("Default EventHandler - I am not doing anything - You MUST declare one in the actual application to override this one");
    }
} // namespace ns3