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

#ifndef PUBSUBPAYLOADAPI_HPP
#define PUBSUBPAYLOADAPI_HPP

#include "MESSAGE_CODES.hpp"
#include "Transport.hpp"

typedef struct{
	string RId;
	string SId;
}infoId;

class PubSubWPayloadAPI:public Transport, CommChannel{
	public:
		/**
		 * Instance will create a new object by calling the private constructor 
		 * and assign it to the m_pInstance value ONLY the first time it is called. 
		 * All other times it will return the m_pInstance pointer.
		 * @param user_space if it is true, the netlink is created so that 
		 * it can communicate with blackadder running in user space. 
		 * if it is false, the netlink is created so that it can 
		 * communicate with blackadder running in kernel space.
		 * @param strategy The blackadder strategy (NODE_LOCAL,LINK_LOCAL,DOMAIN_LOCAL)
		 * @return 
		 */
		static PubSubWPayloadAPI* Instance(bool user_space, char bastrategy);
		/**
		 * It publishes a scope with payload
		 * @param payloadRVSId The SId to which the higher level RV expect messages
		 * @param id The scope ID
		 * @param prefix_id The scope path
		 * @param payload The payload
		 * @param size The size of the payload
		 * @param strategy  BlackAdder API strategy
		 * @param LID BlackAdder API LID (usually NULL)
		 */ 
		void publish_scope_wpl(std::string payloadRVSId, std::string &id, std::string &prefix_id, char* payload, int size, char strategy);
		
		/**
		 * It publishes info with payload
		 * @param payloadRVSId The SId to which the higher level RV expect messages
		 * @param id The publication ID
		 * @param prefix_id The scope path
		 * @param payload The payload
		 * @param size The size of the payload
		 * @param strategy  BlackAdder API strategy
		 * @param LID BlackAdder API LID (usually NULL)
		 */ 
		void publish_info_wpl(std::string payloadRVSId,std::string &id, std::string &prefix_id, char* payload, int size, char strategy);
		
		/**
		 * It subscribers to info with payload
		 * @param payloadRVSId The SId to which the higher level RV expect messages
		 * @param id The publication ID
		 * @param prefix_id The scope path
		 * @param payload The payload
		 * @param size The size of the payload
		 * @param strategy  BlackAdder API strategy
		 * @param LID BlackAdder API LID (usually NULL)
		 */ 
		void subscribe_info_wpl(std::string payloadRVSId, std::string &id, std::string &prefix_id, char* payload, int size, char strategy);
		/**
		 * It sets the underlay SId user to communicate with the RV
		 * @param RVSId, the SId
		 * @param uplayer The upper layer that will expects events from this RVSId 
		 */
		 void setRVSId(std::string RVSId, CommChannel* uplayer); 
		/**
		 * It receives events from the underlay layer
		 * @param ev The event
		 */
		 void fromLowerLayer(RVEvent &ev); 
		/**
		 * Inherited from Transport.hpp
		 */
		void publish_data(const std::string &id, char strategy, void *str_opt, unsigned int str_opt_len, void *data, int data_len);
		/**
		 * Inherited from Tranport.hpp
		 */ 
		void publish_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
		/**
		 * Inherited from Tranport.hpp
		 */
		void unpublish_info(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
		/**
		 * Inherited from Tranport.hpp
		 */
		void unsubscribe_info(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len);
	protected:
		/**
		 * Generates a random RId
		 * @param length The length of the RId
		 * @return The RId
		 */ 
		std::string rndRId(int length);
		/**Creates a response RId based on a given RId
		 * @param rid The RId
		 * @return The response RId
		 */
		 std::string getResponseRId(std::string rid);  
		 
		/**
		 * The SId of the RV with Payload support
		 */
		std::string payloadRVSId;
		/**
		 * The user may use multiple payloadRVSId
		 */
		std::map<std::string,CommChannel*> RVSIds; 
		 
		map<std::string,publication> buffer;
		/**
		 * Constructor
		 */
		 PubSubWPayloadAPI(bool user_space,char strategy);
	private:
		/**
		 * It stores a map from Application level
		 * SId, RId to lower level SId, RId
		 */
		 map<std::string,infoId> AppIDtoLowerID; 
		 /**
		 * It stores a map from lower level
		 * SId, RId to Application SId, RId
		 */
		 map<std::string,std::string> LowerIDtoAppId; 
		 /**A link to the upper layer
		  * 
		  */
		 CommChannel* uplayer;  
		/**
		  * Is this a local machine application 
		  * or a network application?
		  */
		 char bastrategy; 
		  /**@brief the single static PubSubWPayloadAPI object an application can access.
		   * 		
		   */
		 static PubSubWPayloadAPI* pbAPIInstance;
		
};

#endif
