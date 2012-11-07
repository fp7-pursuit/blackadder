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

#ifndef COMCHANNEL_HPP
#define COMCHANNEL_HPP

#include <blackadder.hpp>
#include "MESSAGE_CODES.hpp"
/**
 * Used to hold RV events, new events have been added (in 
 * MESSAGE_CODES.hpp
 */ 
class RVEvent: public Event{
	public:
		/**
		 * AN RId that can be used to send data back to the
		 * endpoint that created the event
		 */  
		std::string RIdtoEP;
		/**
		 * The RId of a new publication or subscription or
		 * the SId of a new scope
		 */ 
		std::string OPId;
		/**
		 * The prefix of OPId
		 */ 
		std::string prefix;
		/**
		 * Constructors of event
		 */
		//RVEvent():Event(){}
		//RVEvent(RVEvent &ev):Event((Event&) ev){}
		/**
		 * Destructor
		 */
		//~RVEvent():~Event(){}  
		
};

class CommChannel{
	public:
		/**
		 * It is called whenever a blackadder event is received from the lower layer
		 * @param ev The event
		 */
		virtual void fromLowerLayer(RVEvent &ev)=0;
};

#endif
