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

#include "CDN.hpp"

CDN::CDN(std::string payloadRVSId,bool user_space, char bastrategy){
	brv = new BaseRV(this,user_space,bastrategy);
	brv->listen(payloadRVSId);
}

void CDN::new_pub_scope(std::string RIdtoPub,std::string SId, std::string prefix, char* payload, int size){
		cerr << "\nNew publish_scope with SId " << SId << "\nPrefix " << prefix;
		cerr << "\nPayload " << payload; 
}

void CDN::new_pub_info(std::string RIdtoPub,std::string RId, std::string prefix, char* payload, int size){
		cerr << "\nNew publish_info with RId " << RId << "\nPrefix " << prefix;
		cerr << "\nPayload " << payload;
		//add the publisher and the ISPID to the proper structure
		pubisp[RIdtoPub] = payload;
		//add the publisher to the publication list
		string key = prefix + RId;
		if(pubpub.find(key)==pubpub.end())//this is the first publisher
			pubpub[key]= vector<string>();
		pubpub[key].push_back(RIdtoPub);
}

void CDN::new_sub_info(std::string RIdtoSub,std::string RId, std::string prefix, char* payload, int size){
		cerr << "\nNew subscribe_info to info with RId " << RId << "\nPrefix " << prefix;
		cerr << "\nPayload " << payload;
		cerr << "\nFindind the closer publisher";
		string key = prefix + RId;
		vector<string> publishers =pubpub[key];
		string RIdtoPub =  publishers[0]; //If I don't find a publisher 
										  //close to the subscriber I will notify the first
		 for (int i=0; i<publishers.size(); i++)
			if (pubisp[publishers.at(i)]==payload)
				RIdtoPub = publishers.at(i);
		brv->notifyPubSub(RId, prefix,RIdtoPub, RIdtoSub,brv->rndRId(8),brv->rndRId(8));
		
}

void CDN::fromLowerLayer(RVEvent &ev){
	if(ev.type == NEW_PUB_SCOPE){
		new_pub_scope(ev.RIdtoEP,ev.OPId, ev.prefix, (char*)ev.data, ev.data_len);
	}else if(ev.type == NEW_PUB_INFO){
		new_pub_info(ev.RIdtoEP,ev.OPId, ev.prefix, (char*)ev.data, ev.data_len);
	}else if(ev.type == NEW_SUB_INFO){
		new_sub_info(ev.RIdtoEP,ev.OPId, ev.prefix, (char*)ev.data, ev.data_len);
	}
	 
}

int main(int argc, char* argv[]){
	CDN rv = CDN("SCOPEWAL",true,DOMAIN_LOCAL);
	
	//wait forever
	while(true){
		sleep(1); // to release some cpu but not sure if this is good
	}
}
