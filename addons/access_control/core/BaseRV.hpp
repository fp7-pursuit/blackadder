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

#ifndef BaseRV_HPP
#define BaseRV_HPP

#include "CommChannel.hpp"
#include "PubSubWPayloadAPI.hpp"
#include "MESSAGE_CODES.hpp"
#include <set>
#include <map>

class BaseRV:public CommChannel{
	public:		
		/**
		 * Constructor
		 * @param upperLayer The actual implementation
		 * @param user_space The blackadder user_space parameter 
		 * @param strategy The blackadder strategy (NODE_LOCAL,LINK_LOCAL,DOMAIN_LOCAL) 
		 */
		 BaseRV(CommChannel* upperLayer,bool user_space, char bastrategy);
		 
		 /**Start listening  for events
		  * @param acrvSId The acrvSId to listen for events
		  */ 
		  void listen(std::string payloadRVSId); 
		 
		 /**Callback function when a new event occurs
		  * @param ev The event
		  */ 
		  void fromLowerLayer(RVEvent &ev);
		 
		 /**
		  * It notifies publisher and subscriber that a publication-subscription
		  * match has taken place. This will result in publisher starting sending the
		  * publication data
		  * @param RId The RId of the publication for which the match took place
		  * @param prefix The prefix of the publication for which the match took place
		  * @param RIdtoPub The RId to which the publisher expects the notification
		  * @param RIdtoSub The RId to which the subscriber expects the notification
		  * @param cSId The SId which the publisher and the subscriber will use to communicate
		  * @param cRId The RId which the publisher and the subscriber will use to communicate
		  * @param msgToPub an optional message to the publisher
		  * @param msgToSub an optional message to the subscriber
		  */
		  void notifyPubSub(std::string RId, std::string prefix, std::string RIdtoPub, std::string RIdtoSub,std::string cSId,
							std::string cRId,std::string msgToPub="",std::string msgToSub=""); 
	   /**
		* Generates a random RId
		* @param length The length of the RId
		* @return The RId
		*/ 
		std::string rndRId(int length);
		/**
		 * It sends an ack message. This is an optional step which should be handled
		 * by the application.
		 * @param RIdtoEP The RId in which the endpoint expects the ack
		 * @param msg The message
		 */
		 void  sendACK(std::string RIdtoEP, std::string msg);
	protected:
		/**
		 * The scopeId to which it listens
		 */ 
		std::string payloadRVSId;
		/**
		 * Access to Transport API
		 */
		PubSubWPayloadAPI *tp;
		/**
		 * Indicated is already listening
		 */
		bool isListening;  
		/**Extracts the RId out of n ev.id
		 * @param Id The ev.id
		 * @return The RId
		 */
		 std::string extractRId(std::string Id);  
		 /**Creates a response RId based on a given RId
		 * @param rid The RId
		 * @return The response RId
		 */
		 std::string getResponseRId(std::string rid); 
		 
	private:
		
		
		/**
		 * A publication buffer. For the publication struct
		 * see Transport.hpp
		 */
		 map<std::string,publication> buffer; 
		 /**
		  * Is this a local machine application 
		  * or a network application?
		  */
		 char bastrategy; 
		 /**
		  * A pointer to the upper layer
		  */ 
		CommChannel* uplayer;
};

#endif
