/*
 * Copyright (C) 2012-2013  Andreas Bontozoglou
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of
 * the BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

#include <blackadder.hpp>
#include <nb_blackadder.hpp>
#include <signal.h>
#include <string.h>
#include <inttypes.h>
#include <openssl/sha.h>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/time.h>

#include <metadatapacket.hpp>

using namespace std;

#define ITEM_ID "0000000000000000deadbeefdeadbeef"
#define MAX_BUFFER 1500
#define TG_PKT_SIZE 1000


uint64_t RATE=1000000;

NB_Blackadder *ba;
pthread_t publisher;
// pthread_mutex_t queue_mutex;
// pthread_cond_t queue_cond;
string bin_item_identifier = hex_to_chararray(ITEM_ID);
string qos_id;
string bin_qos_id;
string config;
QoSList qos;

// Stats
uint64_t pkt_send;

void sigterm(int sig) {
    (void) signal(SIGINT, SIG_DFL);
    cout << "App Terminating..." <<endl;
    cout<<"\n--- Stats ---\n"
	<<"Rate="<<RATE<<" bps\n"
	<<"Pkts="<<pkt_send<<"\n";
    
    ba->disconnect();
    delete ba;
    exit(0);
}

/// Simple string split function
vector<string> split(const char *str, char c=' ')
{
    vector<string> result;
    while(1)
    {
        const char *begin = str;
        while(*str != c && *str)
                str++;

        result.push_back(string(begin, str));
        if(0 == *str++)
                break;
    }
    return result;
}


void sendMetaData(){
  cout<<" - Sending MetaData..."<<endl;
  
  // Create the packet...
  MetaDataPacket pkt((uint8_t)2);
  pkt.setID(bin_item_identifier);
  pkt.appendIIStatus(qos);
  
  
  // ...
  uint8_t * data = new uint8_t[pkt.getSize()];
  memcpy(data, pkt.getData(), pkt.getSize());
  ba->publish_data(bin_qos_id, DOMAIN_LOCAL, 0, 0, data, pkt.getSize());
}


/**
 * This is an infinite loop generating traffic
 * on the published II.
 */
void *publisher_loop(void *arg) {
    
    // Calculate packet inter-leave interval
    int pkt_interval = ((double)TG_PKT_SIZE*8*1000000)/RATE; // in us 
    cout<<"TG pkt Interval="<<pkt_interval<<"us"<<endl;
    
    // data...
    uint8_t * data;
    
    // Temp time measurement vars... (pub takes 3-4us)
//     unsigned long time_in_micros1,time_in_micros2;
//     struct timeval tv;
      
    // Keep on TxIng
    while (1){
      
      //gettimeofday(&tv,NULL);
      //time_in_micros1 = 1000000 * tv.tv_sec + tv.tv_usec;
      
      // Alloc
      data = (uint8_t *) malloc(TG_PKT_SIZE);
      
      //cout<<"before: "<<time_in_micros1<<"us"<<endl;
      
      ba->publish_data(bin_item_identifier, DOMAIN_LOCAL, 0, 0, data, TG_PKT_SIZE);
      pkt_send++;
      
      //gettimeofday(&tv,NULL);
      //time_in_micros2 = 1000000 * tv.tv_sec + tv.tv_usec;
      //cout<<"after: "<<time_in_micros2<<"us diff="<<time_in_micros2-time_in_micros1<<endl;
      //int diff=(time_in_micros2-time_in_micros1);
      //if (diff <= pkt_interval){
	//usleep(pkt_interval-diff);
      //}
      
      usleep(pkt_interval);
    }
    
    return NULL;
}

/**
 * Conditionaly start the publisher, if it is not running already...
 */
void startPublisher(){
    if (publisher) {
      cerr<<"Not Creating Publisher... already there..."<<endl;
      return;
    }
    
    // Create the publisher
    pthread_create(&publisher, NULL, publisher_loop, NULL);
}

void callback(Event *ev) {
  // Main loop
  switch (ev->type) {
      case SCOPE_PUBLISHED:
	  cout << "SCOPE_PUBLISHED: " << chararray_to_hex(ev->id) << endl;
	  break;
      case SCOPE_UNPUBLISHED:
	  cout << "SCOPE_UNPUBLISHED: " << chararray_to_hex(ev->id) << endl;
	  break;
      case START_PUBLISH:
      {
	  string spid = chararray_to_hex(ev->id);
	  cout << "START_PUBLISH: " << spid << endl;
	  string item_last = spid.substr(spid.length() - PURSUIT_ID_LEN*2, PURSUIT_ID_LEN*2);

	  if (spid==ITEM_ID) {
	    startPublisher();
	  }else if (item_last == chararray_to_hex(qos_id)){
	    sendMetaData();
	  }else{
	    cout<<"topology manager... (UnKnown ID!)"<<endl;
	  }
	  
	  break;
      }
      case STOP_PUBLISH:
      {
	  cout << "STOP_PUBLISH: " << chararray_to_hex(ev->id) << endl;
	  string spid = chararray_to_hex(ev->id);
	  if (spid==ITEM_ID) {
	    pthread_cancel(publisher);
	    publisher=0;
	  }
	  break;
      }
      case PUBLISHED_DATA:
	  cout << "PUBLISHED_DATA: " << chararray_to_hex(ev->id) << endl;
	  break;
  }
}

/**
 * Keep it here so the /lib does not have the SHA dep... should tho be in there...
 */
string getQoSID(const string & bin_item_identifier){
    string bin_item_last = bin_item_identifier.substr(bin_item_identifier.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
    unsigned char * qos_id_p = (unsigned char *) malloc(SHA_DIGEST_LENGTH);
    SHA1((const unsigned char*) bin_item_last.c_str(), bin_item_last.length(), qos_id_p);
    string qos_id_tmp = string((const char *) qos_id_p, PURSUIT_ID_LEN);
    delete []qos_id_p;
    return qos_id_tmp;
}

/// Configuration! --------------
void parseConfig(){

  ifstream fin(config.c_str());
  if (!fin.is_open()){
    cerr<<"Cannot Open Config \""<<config<<"\" ... Exiting"<<endl;
    exit(-2);
  }
	
  string line="";
  while (getline(fin, line)){
    if (line[0]=='#') continue;
    vector<string> parts = split(line.c_str());
    // Basic Error Check
    if (parts.size()%2!=0) {
      cerr<<"WRONG Config Format for \""<<config<<"\"... Exiting"<<endl;
      exit(-3);
    }
    
    // Build MetaData QoS
    if (parts.size()==0)
	return;
    
    for (uint i=0; i<parts.size(); i+=2){
      qos.insert (pair<uint8_t,uint16_t>(
	  atoi(parts[i].c_str()),
	  atoi(parts[i+1].c_str())
	));
      
      if (atoi(parts[i].c_str())==QoS_BW_M)
	RATE=1000000*atoi(parts[i+1].c_str());
      else if (atoi(parts[i].c_str())==QoS_BW_K)
	RATE=1000*atoi(parts[i+1].c_str());
    }
  }
    
}

int main(int argc, char* argv[]) {
    
    
    bool kernel = false;
    pkt_send=0;
    
    if (argc > 1 && atoi(argv[1])==1) kernel=true;
    // Config
    if (argc > 2) config=argv[2];
    else {
      cerr<<"USAGE: "<<argv[0]<<" <0|1>(user|kernel) <config> \n\
      \nI need a configuration file in the format:\n\
<uint8_t> <uint16_t> <uint8_t> <uint16_t> <uint8_t> <uint16_t> <uint8_t> <uint16_t> ...\n\
\nCurrently Supported Attributes\n\
/// The link or flow priority (may or may not change dynamically: currently it doesn't) \n\
#define QoS_PRIO	15 \n\
/// Bandiwdth/data rate needed (mean) \n\
#define QoS_BW_K	50 \n\
#define QoS_BW_M	51 \n\
/// More information/characteristics about the flow \n\
#define QoS_FLOW	52\n\
/// Avg. packet size\n\
#define QoS_PKTSIZE	53\n\
/// Max. Time of a burst (ms)\n\
#define QoS_BURSTSIZE	54\n\
/// Peak rate on a burst\n\
#define QoS_PEAKRATE_K	55\n\
#define QoS_PEAKRATE_M	56\n";
	    return -1;
    }
    
    // Before everything... check n load config
    parseConfig();
    
    ba = NB_Blackadder::Instance(!kernel);
    ba->setCallback(callback);
    

    //pthread_mutex_init(&queue_mutex, NULL);
    //pthread_cond_init(&queue_cond, NULL);
    
    // Create the algorithmic QoS item...
    qos_id = getQoSID(bin_item_identifier);
    bin_qos_id=bin_item_identifier.substr(0, PURSUIT_ID_LEN) + qos_id;
    
    cout<<"Publishing stuff..."<<endl;
    // Publish Scopes/Items
    ba->publish_scope(bin_item_identifier.substr(0, PURSUIT_ID_LEN), string(), DOMAIN_LOCAL, NULL, 0);
    ba->publish_info(bin_item_identifier.substr(bin_item_identifier.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN), bin_item_identifier.substr(0, PURSUIT_ID_LEN), DOMAIN_LOCAL, NULL, 0);
    // ... publish under the same scope, with the algorithmic id
    ba->publish_info(qos_id,  bin_item_identifier.substr(0, PURSUIT_ID_LEN), DOMAIN_LOCAL, NULL, 0);
    
    // Join the callback thread...
    signal(SIGINT, sigterm);
    ba->join();
    
    // Clean
    cout<<"Cleaning (no KILL)..."<<endl;
    ba->disconnect();
    delete ba;
    return 0;
}
