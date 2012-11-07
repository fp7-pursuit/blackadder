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

#include "Subscriber.hpp"

void Subscriber::startSubscribe(std::string payload){
	ba = new BaseEP("SCOPEWAL",this,true,DOMAIN_LOCAL);
	IHaveFinish = false; //just a hack to finish normally
	std::string SId = "CDN Scope"; 
	std::string RId="Blog Post 15-06-2011";
	std::string sub_payload= payload;
    ba->subscribe_info_wpl(RId, SId,(char*)sub_payload.c_str(), sub_payload.size());
}

void Subscriber::fromLowerLayer(RVEvent &ev){
	if(ev.type == PUBLISHED_DATA){
		cerr << "\nI received :\n";
		cerr << (char*)ev.data;
		cerr << "\n";
		IHaveFinish = true; //just a hack to finish normally
	}

}

int main(int argc, char* argv[]){
	Subscriber sub;
	cout << "\n*******Start subscription*********";
	if(argc == 2){
		sub.startSubscribe(argv[1]);
		//The following while is needed 
		//without is the programme will exit immediateley
		//as the code is non-blocking
		cout << "\n I am waiting data.....";
		while(!sub.IHaveFinish){
			sleep(1);
		}
	}else{
		cout<<"\nPlease provide the ISP id by running ./Subscriber <ISP ID>\n";
	}
	exit(0);
}
