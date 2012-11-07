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

#include "PubSubWPayloadAPI.hpp"
PubSubWPayloadAPI* PubSubWPayloadAPI::pbAPIInstance = NULL;

PubSubWPayloadAPI::PubSubWPayloadAPI(bool user_space,char strategy):Transport(user_space,this)
{
	bastrategy = strategy;
	srand ( time(NULL) );
}

PubSubWPayloadAPI* PubSubWPayloadAPI::Instance(bool user_space, char strategy){
	if (!pbAPIInstance) {
        pbAPIInstance = new PubSubWPayloadAPI(user_space,strategy);
    }
    return pbAPIInstance;
		
}

void PubSubWPayloadAPI::publish_scope_wpl(std::string payloadRVSId, std::string &id, std::string &prefix_id, char* payload, int size, char strategy){
	std::string header;
	header.append(PUBLISH_SCOPE_WITH_PAYLOAD);
	header.append(";");
	header.append(id);
	header.append(";");
	header.append(prefix_id);
	header.append(";");
	int headersize = header.length();
	const char* headerc = header.c_str();
	publication pub;
	pub.strategy = strategy;
	pub.size = headersize+size;
	pub.payload.append(headerc);
	pub.payload.append(payload);
	pub.immutable =false; //Unpublish policy when send to avoid collisions 
	std::string key;
	std::string RId = rndRId(8);
	pub.RId = RId;
	pub.SId = payloadRVSId;
	key.append(payloadRVSId);
	key.append(RId);
	buffer[key] = pub;
	//create an RId to which we expect responses from the RV
	std::string responseRId = getResponseRId(RId);
	//Subscribe to the response
	Transport::subscribe_info(responseRId,payloadRVSId,strategy,NULL,0);
	//Subscribe to possible ack	
	Transport::subscribe_info(responseRId,payloadRVSId +ACK_SCOPEID,strategy,NULL,0);
	//Publish the scope advertisement
	Transport::publish_info(RId,payloadRVSId,strategy,NULL,0);
	

}

void PubSubWPayloadAPI::publish_info_wpl(std::string payloadRVSId, std::string &id, std::string &prefix_id, char* payload, int size, char strategy){
	std::string header;
	header.append(PUBLISH_INFO_WITH_PAYLOAD);
	header.append(";");
	header.append(id);
	header.append(";");
	header.append(prefix_id);
	header.append(";");
	int headersize = header.length();
	const char* headerc = header.c_str();
	publication pub;
	pub.strategy = strategy;
	pub.size = headersize+size;
	pub.payload.append(headerc);
	pub.payload.append(payload);;
	pub.immutable =false; //Unpublish policy when send to avoid collisions 
	std::string key;
	std::string RId = rndRId(8);//generate a random RId
	pub.RId = RId;
	pub.SId = payloadRVSId;
	key.append(payloadRVSId);
	key.append(RId);
	buffer[key] = pub;
	
	//create an RId to which we expect responses from the RV
	std::string responseRId = getResponseRId(RId);
	//Subscribe to possible acck
	Transport::subscribe_info(responseRId,payloadRVSId+ACK_SCOPEID,strategy,NULL,0);
	//Subscribe to the response
	Transport::subscribe_info(responseRId,payloadRVSId,strategy,NULL,0);
	//Create the publication
	Transport::publish_info(RId,payloadRVSId,strategy,NULL,0);

}

void PubSubWPayloadAPI::subscribe_info_wpl(std::string payloadRVSId, std::string &id, std::string &prefix_id, char* payload, int size, char strategy){
	std::string header;
	header.append(SUBSCRIBE_INFO_WITH_PAYLOAD);
	header.append(";");
	header.append(id);
	header.append(";");
	header.append(prefix_id);
	header.append(";");
	int headersize = header.length();
	const char* headerc = header.c_str();
	publication pub;
	pub.strategy = strategy;
	pub.size = headersize+size;
	pub.payload.append(headerc);
	pub.payload.append(payload);
	pub.immutable =false; //Unpublish policy when send to avoid collisions 
	std::string key;
	std::string RId = rndRId(8);//generate a random RId
	pub.RId = RId;
	pub.SId = payloadRVSId;
	key.append(payloadRVSId);
	key.append(RId);
	buffer[key] = pub;
	//create an RId to which we expect responses from the RV
	std::string responseRId = getResponseRId(RId);
	//Subscribe to possible acck
	Transport::subscribe_info(responseRId,payloadRVSId+ACK_SCOPEID,strategy,NULL,0);
	//Subscribe to the response
	Transport::subscribe_info(responseRId,payloadRVSId,strategy,NULL,0);
	//Send the subscription message
	Transport::publish_info(RId,payloadRVSId,strategy,NULL,0);

}

void PubSubWPayloadAPI::fromLowerLayer(RVEvent &ev){
	if(ev.type == STOP_PUBLISH){
		//notify the application
		Event tempev;
		tempev.type = START_PUBLISH;
		tempev.id = LowerIDtoAppId[ev.id];
		tempev.data = NULL;
		tempev.buffer = NULL;
		(*RVSIds.begin()).second->fromLowerLayer((RVEvent&)tempev);
		return;
	}
	if(ev.type == START_PUBLISH){//This happens when the RV asks for the payload
		if(buffer.find(ev.id)==buffer.end()){//this is for a higher level publication
			//notify the application
			Event tempev;
			tempev.type = START_PUBLISH;
			tempev.id = LowerIDtoAppId[ev.id];
			tempev.data = NULL;
			tempev.buffer = NULL;
			(*RVSIds.begin()).second->fromLowerLayer((RVEvent&)tempev);
			return;
		}
		
		Transport::publish_data(ev.id,
								buffer[ev.id].strategy,
								NULL,0,
								(char*)(buffer[ev.id].payload.c_str()),
								buffer[ev.id].size);
		if(!buffer[ev.id].immutable){
			Transport::unpublish_info(buffer[ev.id].RId, 
									buffer[ev.id].SId, 
									buffer[ev.id].strategy, 
									NULL,0);
			buffer.erase(ev.id);//remove the data from the buffer
		}			
	}else if(ev.type==PUBLISHED_DATA){
		std::string data =(char*)ev.data;
		//check if the message is from the RV
		std::string evsid = ev.id.substr(0,payloadRVSId.size());
		if(RVSIds.find(evsid)!=RVSIds.end()){//A command received from A RV
			int si=0;//help variable to split data
			int ei=0;//help variable to split data
			ei = data.find(";",si);
			std::string type = data.substr(si,ei-si);
			//Check the command	
			if (type.compare(PUBLICATION_MATCH)==0){//A subscriber has been found
				//get the SId, RId created in order to send there the publication
				si = ei+1;
				ei = data.find(";",si);
				std::string SId = data.substr(si,ei-si);
				si = ei+1;
				ei = data.find(";",si);
				std::string RId = data.substr(si,ei-si);
				//get the application layer SId, RId
				si = ei+1;
				ei = data.find(";",si);
				std::string AppSIdRId = data.substr(si,ei-si);
				//get the message to publisher
				si = ei+1;
				ei = data.find(";",si);
				std::string msgToHL = data.substr(si,ei-si);
				if(SId!="" && RId!=""){
					//update the AppId to SId, RId map
					AppIDtoLowerID[AppSIdRId].SId= SId;
					AppIDtoLowerID[AppSIdRId].RId= RId;
					LowerIDtoAppId[SId+RId] = AppSIdRId;
					//Store which uplayer should be notified for this match
					RVSIds[SId+RId] = RVSIds[evsid];
					Transport::publish_info(RId,SId,bastrategy,NULL,0);
				}
				if(msgToHL!=""){
					Event tempev;
					tempev.type = PUBLISHED_DATA;
					tempev.id = LowerIDtoAppId[ev.id];
					tempev.data = (void*)msgToHL.c_str();
					tempev.buffer = NULL;
					RVSIds[evsid]->fromLowerLayer((RVEvent&)tempev);
				}
			}else if (type.compare(SUBSCRIPTION_MATCH)==0){//A publisher has been found
				//get the SId, RId created in order to receive form there the publication
				si = ei+1;
				ei = data.find(";",si);
				std::string SId = data.substr(si,ei-si);
				si = ei+1;
				ei = data.find(";",si);
				std::string RId = data.substr(si,ei-si);
				//get the application layer SId, RId
				si = ei+1;
				ei = data.find(";",si);
				std::string AppSIdRId = data.substr(si,ei-si);
				//get the message to subscriber
				si = ei+1;
				ei = data.find(";",si);
				std::string msgToHL = data.substr(si,ei-si);
				if(SId!="" && RId!=""){
					//update the AppId to SId, RId map
					AppIDtoLowerID[AppSIdRId].SId= SId;
					AppIDtoLowerID[AppSIdRId].RId= RId;
					LowerIDtoAppId[SId+RId] = AppSIdRId;
					//Store which uplayer should be notified for this match
					RVSIds[SId+RId] = RVSIds[evsid];
					Transport::subscribe_info(RId,SId,bastrategy,NULL,0);
				}
				if(msgToHL!=""){
					Event tempev;
					tempev.type = PUBLISHED_DATA;
					tempev.id = LowerIDtoAppId[ev.id];
					tempev.data = (void*)msgToHL.c_str();
					tempev.buffer = NULL;
					RVSIds[evsid]->fromLowerLayer((RVEvent&)tempev);
				}
			}else{//message to the higher layer
				Event tempev;
				tempev.type = PUBLISHED_DATA;
				tempev.id = ev.id;
				tempev.data = (void*)data.c_str();
				tempev.buffer = NULL;
				RVSIds[evsid]->fromLowerLayer((RVEvent&)tempev);
			}
		}else{//Data received from a publisher
			//notify the application
			Event tempev;
			tempev.type = PUBLISHED_DATA;
			tempev.id = LowerIDtoAppId[ev.id];
			tempev.data = (void*)data.c_str();
			tempev.buffer = NULL;
			RVSIds[ev.id]->fromLowerLayer((RVEvent&)tempev);
		}
	}
}

std::string PubSubWPayloadAPI::rndRId(int length){
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

void PubSubWPayloadAPI::setRVSId(std::string RVSId,CommChannel* uplayer)
{
	RVSIds[RVSId]=uplayer;
	this->payloadRVSId = RVSId;
}

void PubSubWPayloadAPI::publish_data(const string &id, char strategy, void *str_opt, unsigned int str_opt_len, void *data, int data_len){
	if(AppIDtoLowerID.find(id)==AppIDtoLowerID.end())
		Transport::publish_data(id,strategy,str_opt,str_opt_len,data,data_len);
	else{
		Transport::publish_data(AppIDtoLowerID[id].SId+AppIDtoLowerID[id].RId,strategy,str_opt,str_opt_len,data,data_len);
	}
}

void PubSubWPayloadAPI::unpublish_info(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len){
	std::string key;
	key.append(prefix_id);
	key.append(id);
	if(AppIDtoLowerID.find(key)==AppIDtoLowerID.end())
		Transport::unpublish_info(id,prefix_id,strategy,str_opt,str_opt_len);
	else{
		Transport::unpublish_info(AppIDtoLowerID[key].RId,AppIDtoLowerID[id].SId,strategy,str_opt,str_opt_len);
	}
}

void PubSubWPayloadAPI::unsubscribe_info(const string&id, const string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len){
	std::string key;
	key.append(prefix_id);
	key.append(id);
	if(AppIDtoLowerID.find(key)==AppIDtoLowerID.end())
		Transport::unpublish_info(id,prefix_id,strategy,str_opt,str_opt_len);
	else{
		Transport::unpublish_info(AppIDtoLowerID[key].RId,AppIDtoLowerID[key].SId,strategy,str_opt,str_opt_len);
	}
}
void PubSubWPayloadAPI::publish_info(const std::string&id, const std::string&prefix_id, unsigned char strategy, void *str_opt, unsigned int str_opt_len){
	std::string key;
	key.append(prefix_id);
	key.append(id);
	LowerIDtoAppId[key] = key;
	Transport::publish_info(id,prefix_id ,strategy,str_opt,str_opt_len);
}
std::string PubSubWPayloadAPI::getResponseRId(std::string rid){
	//simply reverse it
	return std::string(rid.rbegin(),rid.rend());
}
