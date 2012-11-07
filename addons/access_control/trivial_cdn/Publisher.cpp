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

#include "Publisher.hpp"

void Publisher::startPulish(std::string payload){
	ba = new BaseEP("SCOPEWAL",this,true,DOMAIN_LOCAL);
	std::string SId = "CDN Scope"; 
	std::string scope_payload="";
    std::string rootSId = std::string();
    cout <<"\n Publishing Scope ";
    ba->publish_scope_wpl(SId, rootSId , (char*)scope_payload.c_str(), scope_payload.size());
    std::string RId="Blog Post 15-06-2011";
    std::string publication_payload=payload;
    cout <<"\n Publishing Info to the CDN";
    ba->publish_info_wpl(RId, SId,(char*)publication_payload.c_str(), publication_payload.size());
 
}

void Publisher::fromLowerLayer(RVEvent &ev){
	if(ev.type == START_PUBLISH){
		cerr << "\nSomeone subscribed to " << ev.id;
		std::string messagetosend = "Hello this is a publication served from the CDN";
		ba->publish_data(ev.id,(char*)messagetosend.c_str(), messagetosend.size());
	}
	 
}



int main(int argc, char* argv[]){
	Publisher pub;
	if(argc ==2){
		cout << "\n*******Start publishing*********";
		pub.startPulish(argv[1]);
		//The following while is needed 
		//without is the programme will exit immediateley
		//as the code is non-blocking
		//The cout and the sleep are simply to demonstrate
		//that the code is non-blocking
		//probably is not best practice to add sleep
		cout << "\n I am waiting subscriber.....";
		while(!pub.IHaveFinish){
			sleep(1);
		}
	}else{
		cout<<"\nPlease provide the ISP id by running ./Publisher <ISP ID>\n";
	}
	return 0;
}
