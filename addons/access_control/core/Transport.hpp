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

#ifndef TRANSPORT_HPP
#define TRANSPORT_HPP

#include "CommChannel.hpp"
#include <map>
#include <list>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <blackadder.hpp>
#include <pthread.h>

/**The maximum packet size. It should beconfigured to the same value in 
 * both publisher and subscriber.
 */
#define MAX_PACKET_SIZE 1000

typedef struct{
	int data_len;
	string data;
}packet;
 
typedef  std::map<std::string,packet > packetBuffer; 

//This structure is used by higher layers to create a buffer of publications
//so to know what to send when a PUBLISH_DATA event occurs
typedef struct{
	std::string RId;
	std::string SId;
	std::string prefix_id;
	string payload;
	char strategy;
	int size;
	bool immutable;
}publication;
/**
 * It extends BlackAdder by overriding
 * getEvent.
 */
class Transport:public Blackadder{
	public:
		/**
		 * Instance will create a new object by calling the private constructor 
		 * and assign it to the m_pInstance value ONLY the first time it is called. 
		 * All other times it will return the m_pInstance pointer.
		 * @param user_space if it is true, the netlink is created so that 
		 * it can communicate with blackadder running in user space. 
		 * if it is false, the netlink is created so that it can 
		 * communicate with blackadder running in kernel space.
		 * @param uplayer A pointer to the upper layer
		 * @return 
		 */
		static Transport* Instance(bool user_space,CommChannel* uplayer);
		/**
		 * It performs the fragmentatios if
		 * data_len is > than PACKET_SIZE -2 then it splits to more.
		 * -2 is because the first two bytes of the packer will be used 
		 * for signalling purpose. If the packet size is == PACKET_SIZE -2
		 * it will lead to generation of another empty packet
		 * if isfile==false when finishing sending it will also send an empty 
		 * packet to indicate the end of the stream, if isfile==true it
		 * will not do it as it expects more data to be send
		 * @param id the full identifier of the information item for which data is published.
		 * @param strategy the dissemination strategy assigned to the request.
         * @param str_opt a bucket of bytes that are strategy specific. When the IMPLICIT_RENDEZVOUS strategy is used this bucket contains a LIPSIN identifier.
         * @param str_opt_len the size of the provided bucket of bytes. When the IMPLICIT_RENDEZVOUS strategy is used str_opt_len should be FID_LEN.
         * @param data a bucket of data that is published.
         * @param data_len the size of the published data.
         */
        void publish_data(const string &id, char strategy, void *str_opt, unsigned int str_opt_len, void *data, int data_len);
		/**
		 * It publishes a file
		 * @param id data id
		 * @param strategy The strategy to be used as defined in blackadder.h
		 * @param LID The zFilter
		 * @param filename The file to be send
		 */ 
		void publish_file(string &id, char strategy, void *str_opt, unsigned int str_opt_len, char* filename);
		 /**
		  * It listens for events and it notifies the higher layer
		  * it is invoked by a separate thread
		  */
		 void listenForEvents();  
		  
    protected:
		/**Constructor
		 * @param user_space the user_space parameter
		 * @param uplayer A pointer to the upper layer
		 * of blackadder contstructor
		 */
		Transport(bool user_space,CommChannel* uplayer);
	private:
		 
		 /**
		 * This map is the buffer. The key is the id of the publication
		 */
		 packetBuffer buffer;
		 /**
		  * This the communication channel to the upper layer
		  */ 
		 CommChannel* uplayer; 
		 /**
		  * The event hanlder thread
		  */
		 pthread_t eventThread; 
		 /**The event handler function
		  * @param The event
		  */
		 void eventHandler(Event &ev);  
		 
		 static Transport* tranInstance;
};


#endif
