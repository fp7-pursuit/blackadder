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

#include "ACRV.hpp"

ACRV::ACRV(std::string payloadRVSId,bool user_space, char bastrategy){
	brv = new BaseRV(this,user_space,bastrategy);
	brv->listen(payloadRVSId);
}

void ACRV::new_pub_scope(std::string RIdtoPub,std::string SId, std::string prefix, char* payload, int size){
		cerr << "\nNew publish_scope with SId " << SId << "\nPrefix " << prefix;
		cerr << "\nPayload " << payload; 
}

void ACRV::new_pub_info(std::string RIdtoPub,std::string RId, std::string prefix, char* payload, int size){
		cerr << "\nNew publish_info with RId " << RId << "\nPrefix " << prefix;
		cerr << "\nPayload " << payload;
		//add the publication and the password to the proper structure
		string key = prefix + RId;
		pubpass[key] = payload;
		//add the publisher
		pubpub[key] = RIdtoPub;
}

void ACRV::new_sub_info(std::string RIdtoSub,std::string RId, std::string prefix, char* payload, int size){
		cerr << "\nNew subscribe_info to info with RId " << RId << "\nPrefix " << prefix;
		cerr << "\nPayload " << payload;
		cerr << "\nChecking if password is correct";
		string key = prefix + RId;
		if (pubpass[key]==payload){//password correct
			cerr <<"\n Password correct sending notifications";
			brv->notifyPubSub(RId, prefix,pubpub[key], RIdtoSub,brv->rndRId(8),brv->rndRId(8));
		}else{//password did not match
			cerr << "\nWrong password, notifying subscriber";
			brv->sendACK(RIdtoSub,"The password was wrong");
		}
}

void ACRV::fromLowerLayer(RVEvent &ev){
	if(ev.type == NEW_PUB_SCOPE){
		new_pub_scope(ev.RIdtoEP,ev.OPId, ev.prefix, (char*)ev.data, ev.data_len);
	}else if(ev.type == NEW_PUB_INFO){
		new_pub_info(ev.RIdtoEP,ev.OPId, ev.prefix, (char*)ev.data, ev.data_len);
	}else if(ev.type == NEW_SUB_INFO){
		new_sub_info(ev.RIdtoEP,ev.OPId, ev.prefix, (char*)ev.data, ev.data_len);
	}
	 
}

int main(int argc, char* argv[]){
	ACRV rv = ACRV("ACRVAka1",true,DOMAIN_LOCAL);
	
	//wait forever
	while(true){
		sleep(1); // to release some cpu but not sure if this is good
	}
}
