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

#include "Transport.hpp"
Transport* Transport::tranInstance = NULL;
void *nbThreadGetEvent(void* pThis){
	Transport *tp = static_cast<Transport*>(pThis);
	while(true){
		tp->listenForEvents();
	}
}

void Transport::listenForEvents(){
	Event ev;
    Blackadder:getEvent(ev);
    eventHandler(ev);
}

Transport::Transport(bool user_space, CommChannel* uplayer):Blackadder(user_space){
	this->uplayer = uplayer;
	pthread_create( &eventThread, NULL, &nbThreadGetEvent, (void*)this);
}; 

Transport* Transport::Instance(bool user_space, CommChannel* uplayer){
	if(!tranInstance){
		tranInstance = new Transport(user_space,uplayer);
	}
	return tranInstance;
}


void Transport::publish_file(string &id, char strategy, void *str_opt, unsigned int str_opt_len, char* filename){
		FILE * pFile;
		char * buffer = new char[MAX_PACKET_SIZE-2];
		size_t result;
		int offset = 1;
		pFile = fopen ( filename , "rb" );
		if (pFile==NULL) return;
		// copy the file into the buffer:
		result = fread (buffer,1,MAX_PACKET_SIZE-2,pFile);
		while(result == MAX_PACKET_SIZE-2){
			publish_data(id,strategy,str_opt,str_opt_len,buffer, MAX_PACKET_SIZE-2);
			//move file pointer
			fseek (pFile , offset*(MAX_PACKET_SIZE-2) , SEEK_SET );
			free (buffer);
			offset++;
			buffer = new char[MAX_PACKET_SIZE-2];
			result = fread (buffer,1,MAX_PACKET_SIZE-2,pFile);
		}
		if(result > 0){
			char * payload = new char[result];
			memcpy(payload,buffer,result);
			free (buffer);
			publish_data(id,strategy,str_opt,str_opt_len,payload,result);
			free(payload);
		}else{//rare case, send empty packet
			publish_data(id,strategy,str_opt,str_opt_len,buffer,0);
		}
	  fclose (pFile);
	  
}

void Transport::eventHandler(Event &ev){
	//If event is PUBLISHED_DATA
	if(ev.type == PUBLISHED_DATA){
		//check if the ev.id is in the buffer
		//if it is not create a new one
		packetBuffer::iterator bufferIter = buffer.find(ev.id);
		if(bufferIter==buffer.end())//this is the first time the packet appears
			buffer[ev.id] =packet();
		//fist add the packet
		
		if(ev.data_len>2){
			buffer[ev.id].data_len = buffer[ev.id].data_len+ev.data_len-2;
			//remove the first two bytes of the data, they are the packet sec number
			buffer[ev.id].data.append(((const char*)ev.data)+2,ev.data_len-2);
		}
		
		
		//if this is  notify the subscriber
		if( ev.data_len < MAX_PACKET_SIZE ){
			RVEvent tev;
			tev.id = ev.id;
			tev.type = PUBLISHED_DATA;
			tev.data = (void*)(buffer[ev.id].data.c_str());
			tev.buffer = NULL;
			tev.data_len = buffer[ev.id].data_len;
			uplayer->fromLowerLayer(tev);
			buffer.erase(ev.id);
		}
		
		
	}else{//for any other event just notify
		uplayer->fromLowerLayer((RVEvent&)ev);
	}
}


 void Transport::publish_data(const string &id, char strategy, void *str_opt, unsigned int str_opt_len, void *data, int data_len){
	//Check if the data feets in one packet
	//The first 2 bytes of the packet are used for signalling
	//That's why -2
	char firstbyte = 0;//packet counter
	char secondbyte =1;//packet counter
	int offset = 0;//for memory copying
	std::string sdata = string((char*)data);
	while (data_len >= MAX_PACKET_SIZE-2){
		//create a new data array with more two bytes for control
		std::string tmpdata;;
		//set the first two bytes 0,1 to show this is the first packet
		tmpdata.append(1,firstbyte);
		tmpdata.append(1,secondbyte);
		tmpdata.append(sdata,(offset*(MAX_PACKET_SIZE-2)),MAX_PACKET_SIZE-2);
		Blackadder::publish_data(id, strategy, str_opt,str_opt_len, (char*)tmpdata.c_str(), MAX_PACKET_SIZE);
		//adjust the offset
		offset++;
		//adjust the remaining data len
		data_len-=MAX_PACKET_SIZE-2;
		//adjust the packet counter
		secondbyte++;
		if(secondbyte==256){
			secondbyte = 0;
			firstbyte++;
			if(firstbyte==256)//reset counter
			firstbyte=0;
		}
	}
	
	//Send the rest of the packet. if data_len == 0 send an 
	//empty packet in oder the other party to understand that this
	//is the end of the stream
	if(data_len>=0){
		//create a new data array with more two bytes for control
		std::string tmpdata="";
		//set the first two bytes 0,1 to show this is the first packet
		tmpdata.append(1,firstbyte);
		tmpdata.append(1,secondbyte);
		//copy the rest of the packet
		if(data_len > 0)
			tmpdata.append(sdata,(offset*(MAX_PACKET_SIZE-2)),data_len);
		Blackadder::publish_data(id, strategy, str_opt,str_opt_len, (char*)tmpdata.c_str(), data_len+2);
	}
 }
