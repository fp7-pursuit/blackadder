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

#ifndef ACRV_HPP
#define ACRV_HPP

#include "BaseRV.hpp"
#include "iostream"
#include <string>
#include <map>

/*
 * Implements a rendezvous point with access control
 */ 
class ACRV:public CommChannel{
	public:
		/**
		 * Constructor
		 * @param std::string payloadRVSId the SId to which the BaseRV receives messages 
		 * @param user_space The blackadder user space
		 * @param strategy The blackadder strategy (NODE_LOCAL,LINK_LOCAL,DOMAIN_LOCAL) 
		 */
		 ACRV(std::string payloadRVSId,bool user_space, char bastrategy);
		 
		 /**Callback function when a new event occurs
		  * @param ev The event
		  */ 
		  void fromLowerLayer(RVEvent &ev);
	
	private:
		BaseRV* brv;
		/**
		 * It is invoked when a NEW_PUB_INFO event occurs
		 * @param RIdtoPub An RId to which the publisher has subscribed and expects a control message from the RV
		 * @param RId The RId of the publication
		 * @param prefix The prefix of the publication
		 * @param payload The payload that was sent by the publisher
		 * @param size The size of the payload
		 */ 
		void new_pub_info(std::string RIdtoPub,std::string RId, std::string prefix, char* payload, int size);
		 /**
		 * It is invoked when a NEW_SUB_INFO event occurs
		 * @param RIdtoSub An RId to which the subscriber has subscribed and expects a control message from the RV
		 * @param RId The RId of the subscription
		 * @param prefix The prefix of the subscription
		 * @param payload The payload that was sent by the subscriber
		 * @param size The size of the payload
		 */
		void new_sub_info(std::string RIdtoSub,std::string RId, std::string prefix, char* payload, int size);
		/**
		 * It is invoked when a NEW_PUB_SCOPE event occurs
		 * @param RIdtoPub An RId to which the publisher has subscribed and expects a control message from the RV
		 * @param RId The RId of the scope
		 * @param prefix The prefix of the scope
		 * @param payload The payload that was sent by the publisher
		 * @param size The size of the payload
		 */
		void new_pub_scope(std::string RIdtoPub,std::string SId, std::string prefix, char* payload, int size);
		/**
		 * a structure that is used to store publication IDs, password pairs
		 */
		 map<std::string,std::string> pubpass;
		 /**
		 * a structure that is used to store publication IDs, publishers pairs
		 */
		 map<std::string,std::string> pubpub;
		  
};
#endif
