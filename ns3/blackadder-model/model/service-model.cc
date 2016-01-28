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

#include "service-model.h"

NS_LOG_COMPONENT_DEFINE("ServiceModel");

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(ServiceModel);

    TypeId ServiceModel::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::ServiceModel")
                .SetParent<Object > ()
                .AddConstructor<ServiceModel > ();
        return tid;
    }

    ServiceModel::ServiceModel() : m_pid_generator(0, 65536) {
    }

    ServiceModel::~ServiceModel() {
    }

    void ServiceModel::DoInitialize(void) {
        NS_LOG_FUNCTION(this);
        Ptr<ClickBridge> click_bridge = GetObject<Node > ()->GetObject<ClickBridge > ();
        click_bridge->SetUpcall(ns3::MakeCallback(&ns3::ServiceModel::ClickUpcall, this));
    }

    void ServiceModel::DoDispose() {
        NS_LOG_FUNCTION(this);
    }

    uint32_t ServiceModel::RegisterApplication(Ptr<Application> app, EventListener el) {
        NS_LOG_FUNCTION(this);
        uint32_t pid = 0;
        if (m_eventListeners.find(app) != m_eventListeners.end()) {
            NS_LOG_INFO("Application is already registered");
            return -1;
        } else {
            m_eventListeners.insert(std::pair<Ptr<Application>, EventListener > (app, el));
            do {
                pid = m_pid_generator.GetValue();
            } while (m_applications.find(pid) != m_applications.end());
            m_applications.insert(std::pair<uint32_t, Ptr<Application> > (pid, app));
        }
        NS_LOG_INFO("PID: " << pid);
        return pid;
    }

    void ServiceModel::UnregisterApplication(uint32_t pid) {
        NS_LOG_FUNCTION(this);
        Ptr<Application> app;
        /*send a "disconnect" message to Blackadder*/
        disconnect(pid);
        app = (*(m_applications.find(pid))).second;
        m_eventListeners.erase(app);
        m_applications.erase(pid);
    }

    void ServiceModel::ClickUpcall(int ifid, const unsigned char *data, int len) {
        NS_LOG_FUNCTION(this);
        Ptr<Application> receiverApp;
        uint32_t pid = 0;
        unsigned char id_len;
        Ptr<Event> ev = Create<Event > ();
        memcpy(&pid, data, sizeof (pid));
        NS_LOG_INFO("PID: " << pid);
        memcpy(&ev->type, data + sizeof (pid), sizeof (ev->type));
        memcpy(&id_len, data + sizeof (pid) + sizeof (ev->type), sizeof (id_len));
        ev->id = std::string((char *) data + sizeof (pid) + sizeof (ev->type) + sizeof (id_len), ((int) id_len) * PURSUIT_ID_LEN);
        NS_LOG_INFO("EVENT FOR ID: " << chararray_to_hex(ev->id));
        if (ev->type == PUBLISHED_DATA) {
            ev->data_len = len - (sizeof (pid) + sizeof (ev->type) + sizeof (id_len) + ((int) id_len) * PURSUIT_ID_LEN);
            ev->data = (unsigned char *) malloc(ev->data_len);
            memcpy(ev->data, data + sizeof (pid) + sizeof (ev->type) + sizeof (id_len) + ((int) id_len) * PURSUIT_ID_LEN, ev->data_len);
        } else {
            ev->data = NULL;
            ev->data_len = 0;
        }
        receiverApp = (*m_applications.find(pid)).second;
        EventListener evListener = (*m_eventListeners.find(receiverApp)).second;
        evListener(ev);
    }

    void ServiceModel::disconnect(uint32_t pid) {
        NS_LOG_FUNCTION(this);
        unsigned char type = DISCONNECT;
        Ptr<ClickBridge> click_bridge = GetObject<Node > ()->GetObject<ClickBridge > ();
        NS_ASSERT(click_bridge != 0);
        /*serialize to a char * buffer and send it to Blackadder via ClickBridge*/
        uint32_t len = sizeof (uint32_t) + 1 /*type*/;
        uint8_t *buf = new uint8_t [len];
        memcpy(buf, &pid, sizeof (uint32_t));
        memcpy(buf + sizeof (uint32_t), &type, 1);
        click_bridge->SendPacketToClick(0, SIMCLICK_PTYPE_UNKNOWN, buf, len);
    }

    void ServiceModel::publish_scope(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        if (id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send PUBLISH_SCOPE request - wrong ID size");
        } else if (prefix_id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library : Could not send PUBLISH_SCOPE request - wrong prefix_id size");
        } else if (id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send PUBLISH_SCOPE request - id cannot be empty");
        } else {

            create_and_send_buffers(pid, PUBLISH_SCOPE, id, prefix_id, strategy, str_opt, str_opt_len);
        }
    }

    void ServiceModel::publish_info(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        if (id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send PUBLISH_INFO request - wrong ID size");
        } else if (prefix_id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send PUBLISH_INFO request - wrong prefix_id size");
        } else if (prefix_id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send PUBLISH_INFO request - prefix_id cannot be empty");
        } else if (prefix_id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send PUBLISH_INFO request - prefix_id cannot be empty");
        } else {

            create_and_send_buffers(pid, PUBLISH_INFO, id, prefix_id, strategy, str_opt, str_opt_len);
        }
    }

    void ServiceModel::unpublish_scope(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        if (id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNPUBLISH_SCOPE request - wrong ID size");
        } else if (prefix_id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNPUBLISH_SCOPE request - wrong prefix_id size");
        } else if (id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNPUBLISH_SCOPE request - id cannot be empty");
        } else if (id.length() / PURSUIT_ID_LEN > 1) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNPUBLISH_SCOPE request - id cannot consist of multiple fragments");
        } else {

            create_and_send_buffers(pid, UNPUBLISH_SCOPE, id, prefix_id, strategy, str_opt, str_opt_len);
        }
    }

    void ServiceModel::unpublish_info(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        if (id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNPUBLISH_INFO request - wrong ID size");
        } else if (prefix_id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNPUBLISH_INFO request - wrong prefix_id size");
        } else if (id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNPUBLISH_INFO request - id cannot be empty");
        } else if (prefix_id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNPUBLISH_INFO request - prefix_id cannot be empty");
        } else if (id.length() / PURSUIT_ID_LEN > 1) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNPUBLISH_INFO request - id cannot consist of multiple fragments");
        } else {

            create_and_send_buffers(pid, UNPUBLISH_INFO, id, prefix_id, strategy, str_opt, str_opt_len);
        }
    }

    void ServiceModel::subscribe_scope(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        if (id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send SUBSCRIBE_SCOPE request - wrong ID size");
        } else if (prefix_id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send SUBSCRIBE_SCOPE request - wrong prefix_id size");
        } else if (id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send SUBSCRIBE_SCOPE request - id cannot be empty");
        } else if (id.length() / PURSUIT_ID_LEN > 1) {
            NS_LOG_ERROR("Blackadder Library: Could not send SUBSCRIBE_SCOPE request - id cannot consist of multiple fragments");
        } else {

            create_and_send_buffers(pid, SUBSCRIBE_SCOPE, id, prefix_id, strategy, str_opt, str_opt_len);
        }
    }

    void ServiceModel::subscribe_info(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        if (id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send SUBSCRIBE_INFO request - wrong ID size");
        } else if (prefix_id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send SUBSCRIBE_INFO request - wrong prefix_id size");
        } else if (id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send SUBSCRIBE_INFO request - id cannot be empty");
        } else if (prefix_id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send SUBSCRIBE_INFO request - prefix_id cannot be empty");
        } else if (id.length() / PURSUIT_ID_LEN > 1) {
            NS_LOG_ERROR("Blackadder Library: Could not send SUBSCRIBE_INFO request - id cannot consist of multiple fragments");
        } else {

            create_and_send_buffers(pid, SUBSCRIBE_INFO, id, prefix_id, strategy, str_opt, str_opt_len);
        }
    }

    void ServiceModel::unsubscribe_scope(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        if (id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNSUBSCRIBE_SCOPE request - wrong ID size");
        } else if (prefix_id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNSUBSCRIBE_SCOPE request - wrong prefix_id size");
        } else if (id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNSUBSCRIBE_SCOPE request - id cannot be empty");
        } else if (id.length() / PURSUIT_ID_LEN > 1) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNSUBSCRIBE_SCOPE request - id cannot consist of multiple fragments");
        } else {

            create_and_send_buffers(pid, UNSUBSCRIBE_SCOPE, id, prefix_id, strategy, str_opt, str_opt_len);
        }
    }

    void ServiceModel::unsubscribe_info(uint32_t pid, const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        if (id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNSUBSCRIBE_INFO request - wrong ID size");
        } else if (prefix_id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNSUBSCRIBE_INFO request - wrong prefix_id size");
        } else if (id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNSUBSCRIBE_INFO request - id cannot be empty");
        } else if (prefix_id.length() == 0) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNSUBSCRIBE_INFO request - prefix_id cannot be empty");
        } else if (id.length() / PURSUIT_ID_LEN > 1) {
            NS_LOG_ERROR("Blackadder Library: Could not send UNSUBSCRIBE_INFO request - id cannot consist of multiple fragments");
        } else {

            create_and_send_buffers(pid, UNSUBSCRIBE_INFO, id, prefix_id, strategy, str_opt, str_opt_len);
        }
    }

    void ServiceModel::publish_data(uint32_t pid, const std::string&id, unsigned char strategy, void *str_opt, unsigned int str_opt_len, void *data, unsigned int data_len) {
        NS_LOG_FUNCTION(this);
        uint32_t len;
        Ptr<ClickBridge> click_bridge = GetObject<Node > ()->GetObject<ClickBridge > ();
        if (id.length() % PURSUIT_ID_LEN != 0) {
            NS_LOG_ERROR("Service Model: Could not send  - wrong ID size");
        } else {
            unsigned char type = PUBLISH_DATA;
            unsigned char id_len = id.length() / PURSUIT_ID_LEN;
            if (str_opt == NULL) {
                len = sizeof (uint32_t) + 1 /*type*/ + 1 /*for id length*/ + id.length() + sizeof (strategy) + data_len;
            } else {
                len = sizeof (uint32_t) + 1 /*type*/ + 1 /*for id length*/ + id.length() + sizeof (strategy) + str_opt_len + data_len;
            }
            uint8_t *buf = new uint8_t [len];
            memcpy(buf, &pid, sizeof (uint32_t));
            memcpy(buf + sizeof (uint32_t), &type, 1);
            memcpy(buf + sizeof (uint32_t) + 1, &id_len, 1);
            memcpy(buf + sizeof (uint32_t) + 1 + 1, id.c_str(), id.length());
            memcpy(buf + sizeof (uint32_t) + 1 + 1 + id.length(), &strategy, sizeof (strategy));
            if (str_opt == NULL) {
                memcpy(buf + sizeof (uint32_t) + 1 + 1 + id.length() + sizeof (strategy), data, data_len);
            } else {
                memcpy(buf + sizeof (uint32_t) + 1 + 1 + id.length() + sizeof (strategy), str_opt, str_opt_len);
                memcpy(buf + sizeof (uint32_t) + 1 + 1 + id.length() + sizeof (strategy) + str_opt_len, data, data_len);
            }
            click_bridge->SendPacketToClick(0, SIMCLICK_PTYPE_UNKNOWN, buf, len);
        }
    }

    void ServiceModel::create_and_send_buffers(uint32_t pid, unsigned char type, const std::string &id, const std::string &prefix_id, char strategy, void *str_opt, unsigned int str_opt_len) {
        NS_LOG_FUNCTION(this);
        unsigned char id_len = id.length() / PURSUIT_ID_LEN;
        unsigned char prefix_id_len = prefix_id.length() / PURSUIT_ID_LEN;
        Ptr<ClickBridge> click_bridge = GetObject<Node > ()->GetObject<ClickBridge > ();
        NS_ASSERT(click_bridge != 0);
        /*serialize to a char * buffer and send it to Blackadder via ClickBridge*/
        uint32_t len = sizeof (uint32_t) + 1 /*type*/ + 1 /*for id length*/ + id.length() + 1 /*for prefix_id length*/ + prefix_id.length() + 1 /*strategy*/ + str_opt_len;

        uint8_t *buf = new uint8_t [len];
        memcpy(buf, &pid, sizeof (uint32_t));
        memcpy(buf + sizeof (uint32_t), &type, 1);
        memcpy(buf + sizeof (uint32_t) + 1, &id_len, 1);
        memcpy(buf + sizeof (uint32_t) + 1 + 1, id.c_str(), id.length());
        memcpy(buf + sizeof (uint32_t) + 1 + 1 + id.length(), &prefix_id_len, 1);
        memcpy(buf + sizeof (uint32_t) + 1 + 1 + id.length() + 1, prefix_id.c_str(), prefix_id.length());
        memcpy(buf + sizeof (uint32_t) + 1 + 1 + id.length() + 1 + prefix_id.length(), &strategy, 1);
        memcpy(buf + sizeof (uint32_t) + 1 + 1 + id.length() + 1 + prefix_id.length() + 1, str_opt, str_opt_len);

        click_bridge->SendPacketToClick(0, SIMCLICK_PTYPE_UNKNOWN, buf, len);
    }

    std::string ServiceModel::hex_to_chararray(const std::string &hexstr) {
        std::vector<unsigned char> bytes = std::vector<unsigned char>();
        for (std::string::size_type i = 0; i < hexstr.size() / 2; ++i) {
            std::istringstream iss(hexstr.substr(i * 2, 2));
            unsigned int n;
            iss >> std::hex >> n;
            bytes.push_back(static_cast<unsigned char> (n));
        }
        bytes.data();
        std::string result = std::string((const char *) bytes.data(), bytes.size());
        return result;
    }

    std::string ServiceModel::chararray_to_hex(const std::string &str) {
        std::ostringstream oss;
        for (std::string::size_type i = 0; i < str.size(); ++i) {
            if ((unsigned short) (unsigned char) str[i] > 15) {
                oss << std::hex << (unsigned short) (unsigned char) str[i];
            } else {
                oss << std::hex << '0';
                oss << std::hex << (unsigned short) (unsigned char) str[i];
            }
        }
        return oss.str();
    }
} // namespace ns3