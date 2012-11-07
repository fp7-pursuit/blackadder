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

#include "BaseRV.hpp"

BaseRV::BaseRV(CommChannel* upperLayer,bool user_space, char bastrategy){
	this->uplayer = upperLayer;
	srand ( time(NULL) );
	isListening = false;
	tp = PubSubWPayloadAPI::Instance(user_space,DOMAIN_LOCAL);
	this->bastrategy = bastrategy;
}
void BaseRV::listen(std::string payloadRVSId){
	if(isListening) return; //already listening
	isListening = true;
	this->payloadRVSId = payloadRVSId;
	//publish the scope
	std::string rootSId = std::string();
	tp->setRVSId(payloadRVSId,this);
    tp->publish_scope(payloadRVSId, rootSId, bastrategy, NULL,0);
    tp->publish_scope(ACK_SCOPEID, payloadRVSId, bastrategy, NULL,0);
	//listen to the scope
	tp->subscribe_scope(payloadRVSId,rootSId, bastrategy, NULL,0);
}

void BaseRV::fromLowerLayer(RVEvent &ev){
	if(ev.type == START_PUBLISH){
		if(buffer.find(ev.id)==buffer.end())//this should not happen
			return;
		tp->publish_data(ev.id,
								buffer[ev.id].strategy,
								NULL,0,
								(char*)(buffer[ev.id].payload.c_str()),
								buffer[ev.id].size);
		if(!buffer[ev.id].immutable){
			tp->unpublish_info(buffer[ev.id].RId, 
									payloadRVSId, 
									buffer[ev.id].strategy, 
									NULL,0);
			buffer.erase(ev.id);//remove the data from the buffer
		}
				
	} 
	if(ev.type==PUBLISHED_DATA){
		std::string data = (char*)ev.data;
		int si=0;//help variable to split data
		int ei=0;//help variable to split data
		ei = data.find(";",si);
		std::string type = data.substr(si,ei-si);
		si = ei+1;
		ei = data.find(";",si);
		std::string cId = data.substr(si,ei-si);
		si = ei+1;
		ei = data.find(";",si);
		std::string prefix = data.substr(si,ei-si);
		si = ei+1;
		std::string payload = data.substr(si);
		std::string responseRId = getResponseRId(extractRId(ev.id));
		RVEvent tempev;
		tempev.id = ev.id;
		tempev.data = (void*)payload.c_str();
		tempev.data_len = payload.size();
		tempev.buffer = NULL;
		tempev.RIdtoEP = responseRId;
		tempev.OPId = cId;
		tempev.prefix = prefix;
		if (type.compare(PUBLISH_SCOPE_WITH_PAYLOAD)==0){	
			tempev.type = NEW_PUB_SCOPE;			
			uplayer->fromLowerLayer(tempev);			
		}else if (type.compare(PUBLISH_INFO_WITH_PAYLOAD)==0){
			tempev.type = NEW_PUB_INFO;
			uplayer->fromLowerLayer(tempev);
		}else if (type.compare(SUBSCRIBE_INFO_WITH_PAYLOAD)==0){
			tempev.type = NEW_SUB_INFO;
			uplayer->fromLowerLayer(tempev);
		}			
	}
}

void BaseRV::sendACK(std::string RIdtoEP, std::string msg){
	publication ack;
	ack.strategy = bastrategy;
	ack.size = msg.size();
	ack.immutable = false;
	ack.payload = msg;
	ack.RId = RIdtoEP;
	std::string keys;
	keys.append(payloadRVSId+ACK_SCOPEID);
	keys.append(RIdtoEP);
	buffer[keys] = ack;
	tp->publish_info(RIdtoEP,payloadRVSId+ACK_SCOPEID,bastrategy,NULL,0);
}




void BaseRV::notifyPubSub(std::string RId, std::string prefix,std::string RIdtoPub, std::string RIdtoSub,std::string cSId,
							std::string cRId,std::string msgToPub,std::string msgToSub){

	std::string rootSId = std::string();
	if(cSId!="" && cRId!=""){
		tp->publish_scope(cSId, rootSId , bastrategy, NULL,0);
	}
	std::string pubnotificationPayload;
	pubnotificationPayload.append(PUBLICATION_MATCH);//The PUBLICATION_MATCH command
	pubnotificationPayload.append(";");
	pubnotificationPayload.append(cSId+";");//Sid, RId used for lower level communication
	pubnotificationPayload.append(cRId+";");
	pubnotificationPayload.append(prefix);//The application SId, RId
	pubnotificationPayload.append(RId+";");
	pubnotificationPayload.append(msgToPub);//The notification message
	//add the message to the buffer, in order to send it when notified
	publication pub;
	pub.strategy = bastrategy;
	pub.size = pubnotificationPayload.size();
	pub.immutable = false;
	pub.payload = pubnotificationPayload.c_str();
	pub.RId = RIdtoPub;
	std::string keyp;	
	keyp.append(payloadRVSId);
	keyp.append(RIdtoPub);
	buffer[keyp] = pub;
	tp->publish_info(RIdtoPub,payloadRVSId,bastrategy,NULL,0);
	
	//notify subscriber
	std::string subnotificationPayload;
	subnotificationPayload.append(SUBSCRIPTION_MATCH);//The SUBSCRIPTION_MATCH command
	subnotificationPayload.append(";");
	subnotificationPayload.append(cSId+";");//Sid, RId used for lower level communication
	subnotificationPayload.append(cRId+";");
	subnotificationPayload.append(prefix);//The application SId, RId
	subnotificationPayload.append(RId+";");
	subnotificationPayload.append(msgToSub);//The notification message
	//add the message to the buffer, in order to send it when notified
	publication sub;
	sub.strategy = bastrategy;
	sub.size = subnotificationPayload.size();
	sub.immutable = false;
	sub.payload = subnotificationPayload;
	sub.RId = RIdtoSub;
	std::string keys;
	keys.append(payloadRVSId);
	keys.append(RIdtoSub);
	buffer[keys] = sub;
	tp->publish_info(RIdtoSub,payloadRVSId,bastrategy,NULL,0);
								
}
std::string BaseRV::extractRId(std::string Id){
	return Id.substr(payloadRVSId.size());
}

std::string BaseRV::getResponseRId(std::string rid){
	//simply reverse it
	return std::string(rid.rbegin(),rid.rend());
}

std::string BaseRV::rndRId(int length){
	char* rid = new char[length];
	static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < length; ++i) {
        rid[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
	std::string result = std::string(rid,length);
	delete[] rid;
	rid=NULL;
	return result;
}
