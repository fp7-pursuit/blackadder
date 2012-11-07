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

#include "BaseEP.hpp"

BaseEP::BaseEP(std::string payloadRVSId,CommChannel* uplayer,bool user_space, char bastrategy){
	this->uplayer = uplayer;
	this->payloadRVSId=payloadRVSId;
	this->bastrategy = bastrategy;
	tp = PubSubWPayloadAPI::Instance(user_space,bastrategy);
}

void BaseEP::fromLowerLayer(RVEvent &ev){
	//just forward everything to the uplayer
	uplayer->fromLowerLayer(ev);	
	
}

void BaseEP::publish_scope_wpl(std::string &id, std::string &prefix_id, char* payload, int size){
	//simply call the function of the PubSubWPayloadAPI 
	tp->setRVSId(this->payloadRVSId,this);
	tp->publish_scope_wpl(this->payloadRVSId,id,prefix_id,payload,size,bastrategy);
}

void BaseEP::publish_info_wpl(std::string &id, std::string &prefix_id, char* payload, int size){
	//simply call the function of the PubSubWPayloadAPI 
	tp->setRVSId(this->payloadRVSId,this);
	tp->publish_info_wpl(this->payloadRVSId,id,prefix_id,payload,size,bastrategy);
}

void BaseEP::subscribe_info_wpl(std::string &id, std::string &prefix_id, char* payload, int size){
	//simply call the function of the PubSubWPayloadAPI 
	tp->setRVSId(this->payloadRVSId,this);
	tp->subscribe_info_wpl(this->payloadRVSId,id,prefix_id,payload,size,bastrategy);
}

BaseEP::~BaseEP()
{
	tp->disconnect();
}
