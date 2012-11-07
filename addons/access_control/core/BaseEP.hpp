/*-
 * Copyright (C) 2012  Mobile Multimedia Laboratory, AUEB
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of the
 * BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

#ifndef BaseEP_HPP
#define BaseEP_HPP

#include "CommChannel.hpp"
#include "PubSubWPayloadAPI.hpp"
#include "MESSAGE_CODES.hpp"
#include <set>
#include <map>


class BaseEP:public CommChannel{
	private:
		/**
		 * The scopeId to which it listens
		 */ 
		std::string payloadRVSId;
		/**
		 * Access to Transport API
		 */
		PubSubWPayloadAPI *tp;
		/**
		 * The higher layer
		 */ 
		CommChannel* uplayer;
		/**
		 * The blackadder strategy
		 */ 
		char bastrategy;
	public:		
		/**
		 * Constructor
		 * @param payloadRVSId The SId to to which the higher layer RV listens for events 
		 * @param upperLayer A channel to the upper layer
		 * @param user_space if it is true, the netlink is created so that 
		 * it can communicate with blackadder running in user space. 
		 * if it is false, the netlink is created so that it can 
		 * communicate with blackadder running in kernel space.
		 * @param strategy The blackadder strategy (NODE_LOCAL,LINK_LOCAL,DOMAIN_LOCAL)
		 */
		 BaseEP(std::string payloadRVSId,CommChannel* upperLayer,bool user_space, char bastrategy);
		 
		 /**Callback function when a new event occurs
		  * @param ev The event
		  */ 
		  void fromLowerLayer(RVEvent &ev);
		  
		/**
		 * It publishes a scope with payload
		 * @param id The scope ID
		 * @param prefix_id The scope path
		 * @param payload The payload
		 * @param size The size of the payload
		 */ 
		void publish_scope_wpl(std::string &id, std::string &prefix_id, char* payload, int size);
		
		/**
		 * It publishes info with payload
		 * @param id The publication ID
		 * @param prefix_id The scope path
		 * @param payload The payload
		 * @param size The size of the payload
		 */ 
		void publish_info_wpl(std::string &id, std::string &prefix_id, char* payload, int size);
		
		/**
		 * It subscribers to info with payload
		 * @param id The publication ID
		 * @param prefix_id The scope path
		 * @param payload The payload
		 * @param size The size of the payload
		 */ 
		void subscribe_info_wpl( std::string &id, std::string &prefix_id, char* payload, int size);
		/**
		 * calls the repspective function of PubSubWPayloadAPI
		 */
		void publish_data(const std::string &id, void *data, int data_len)
		{tp->publish_data(id, bastrategy, NULL, 0, data,data_len);}
		/**
		 * calls the repspective function of PubSubWPayloadAPI
		 */ 
		void publish_info(const std::string&id, const std::string&prefix_id)
		{tp->publish_info(id, prefix_id, bastrategy, NULL, 0);}
		/**
		 * calls the repspective function of PubSubWPayloadAPI
		 */ 
		 void unpublish_info(const string&id, const string&prefix_id)
		 {tp->unpublish_info(id,prefix_id, bastrategy, NULL, 0);}
		 /**
		 * calls the repspective function of PubSubWPayloadAPI
		 */
		 void unsubscribe_info(const string&id, const string&prefix_id)
		 {tp->unsubscribe_info(id,prefix_id,bastrategy, NULL, 0);}
		/**
		 * Destructor
		 * it disconnects the underlay
		 */ 
		~BaseEP();
};
	

#endif
