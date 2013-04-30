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

#include <metadatapacket.hpp>

using namespace std;


typedef struct __meta_db_item{
  QoSList qos;
  string bin_ii_id;
} MetaDBItem;

typedef map<string, MetaDBItem> MetaDataMap;
MetaDataMap meta_db;


uint64_t RATE=1000000;

Blackadder *ba;
string config;




void sigterm(int sig) {
    (void) signal(SIGINT, SIG_DFL);
    cout << "Meta Provider Terminating..." <<endl;
    
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

/**
 * Keep it here so the /lib does not have the SHA dep... should tho be in there...
 */
string getFullQoSID(const string & bin_item_identifier){
    string bin_item_last = bin_item_identifier.substr(bin_item_identifier.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
    unsigned char * qos_id_p = (unsigned char *) malloc(SHA_DIGEST_LENGTH);
    SHA1((const unsigned char*) bin_item_last.c_str(), bin_item_last.length(), qos_id_p);
    string qos_id_tmp = string((const char *) qos_id_p, PURSUIT_ID_LEN);
    delete []qos_id_p;
    return  bin_item_identifier.substr(0,bin_item_identifier.length() - PURSUIT_ID_LEN)+qos_id_tmp;
}

/**
 * Parse Configuration and create the meta_db
 * 
 * Could in the future be any database or file format
 */
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
    if (parts.size()%2==0 || parts.size()==0) {
      cerr<<"WRONG Config Format for \""<<config<<"\"... Exiting"<<endl;
      exit(-3);
    }
    
    // Create item (check if exists in the future for longer DBs)
    string bin_ii_id = hex_to_chararray(parts[0]);
    string bin_qos_item_id = getFullQoSID(bin_ii_id);
    
    // Build MetaData Item
    MetaDBItem dbi;
    
    // Store the original II since it is needed in the metadata packet
    dbi.bin_ii_id = bin_ii_id;
    
    // Store QoSList
    for (uint i=1; i<parts.size(); i+=2){
      dbi.qos.insert (pair<uint8_t,uint16_t>(
	  atoi(parts[i].c_str()),
	  atoi(parts[i+1].c_str())
	));
    }
    
    cout<<"Adding item='"<<parts[0]<<"' with qos hash='"<<chararray_to_hex(bin_qos_item_id)<<"'"<<endl;
    // Use QoS II (bin) ID in order to avoid re-hashing later (on sending meta, cause start pub. comes with the hash as ev.id)
    meta_db[bin_qos_item_id]=dbi;
    
    // Publish scopes/items
    int path_items = bin_ii_id.length()/PURSUIT_ID_LEN;
    string tmp_scope="";
    for (int i=0; i<path_items-1; i++){
      string piece = bin_ii_id.substr(i*PURSUIT_ID_LEN, PURSUIT_ID_LEN);
      cout<<" - Publishing scope "<<chararray_to_hex(piece)<<" under '"<<chararray_to_hex(tmp_scope)<<"' (if empty==root)"<<endl;
      ba->publish_scope(piece, tmp_scope, DOMAIN_LOCAL, NULL, 0);
      tmp_scope+=piece;
    }
    cout<<" - Publishing QoS item under "<<chararray_to_hex(tmp_scope)<<endl;
    ba->publish_info(bin_qos_item_id.substr(bin_qos_item_id.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN), tmp_scope, DOMAIN_LOCAL, NULL, 0);
    
  }
    
}


void sendMetaData(const string & bin_qos_ii){
  cout<<" - Searching MetaData for "<<chararray_to_hex(bin_qos_ii)<<endl;
  
  MetaDataMap::iterator it = meta_db.find(bin_qos_ii);
  if (it==meta_db.end()){
    cout<<" - No meta-data for this item"<<endl;
    return;
  }
  
  // Create the packet...
  MetaDataPacket pkt((uint8_t)2);
  pkt.setID(it->second.bin_ii_id);
  pkt.appendIIStatus(it->second.qos);
  
//   cout<<pkt<<endl;
//   pkt.debugPrint();
  
  // ...
  uint8_t * data = new uint8_t[pkt.getSize()];
  memcpy(data, pkt.getData(), pkt.getSize());
  ba->publish_data(bin_qos_ii, DOMAIN_LOCAL, 0, 0, data, pkt.getSize());
}


int main(int argc, char* argv[]) {
    
    
    bool kernel = false;
    
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
    
    
    
    // Join the callback thread...
    signal(SIGINT, sigterm);
    ba = Blackadder::Instance(!kernel);
    
    parseConfig();
    
    while (true) {
        Event ev;
        ba->getEvent(ev);
        if (ev.type == START_PUBLISH) {
	  
	  cout<<"* START_PUBLISH for: "<<chararray_to_hex(ev.id)<<endl;
	  sendMetaData(ev.id);
	  
        } else if (ev.type == STOP_PUBLISH) {
            
        } else {
            cout << "I am not expecting anything else than Start or Stop Publishing" << endl;
        }
    }
    
    
    // Clean
    cout<<"Cleaning (no KILL)..."<<endl;
    ba->disconnect();
    delete ba;
    return 0;
} 

