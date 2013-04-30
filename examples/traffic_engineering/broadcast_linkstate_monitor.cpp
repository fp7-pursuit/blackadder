/*
 * Copyright (C) 2012-2013  Mays AL-Naday
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
#include <bitvector.hpp>
#include <signal.h>
#include <map>
#include <iostream>
#include <fstream>

Blackadder *ba;
string FID_str;
Bitvector *FID_to_TM; 
string net_type = "e";
time_t live,last_lsu, lsu;
double lsu_diff;
map<string, time_t> connectivity;
map<string, time_t>::iterator con_itr;
pthread_t _event_listener, *event_listener = NULL;
bool operation = true;

int payload_size=sizeof(live);
char *payload = (char *) malloc(payload_size);
string id = string();
string prefix_id = "0" + string(PURSUIT_ID_LEN*2-2, '1') + "0"; // "0111111111111110";
string bin_prefix_id = hex_to_chararray(prefix_id);
string bin_id;

string resp_prefix_id = string(PURSUIT_ID_LEN*2-1, 'F') + "D"; // "FF..FFFFFFFFFFFFFD"
string bin_resp_prefix_id = hex_to_chararray(resp_prefix_id);

string lsn_prefix_id = string(PURSUIT_ID_LEN*2-1, 'F') + "A"; // "FF..FFFFFFFFFFFFFA"
string root_prefix_id = string();
string bin_lsn_prefix_id = hex_to_chararray(lsn_prefix_id);
string bin_root_prefix_id = hex_to_chararray(root_prefix_id);


using namespace std;


void sigfun(int sig) {
    (void) signal(SIGINT, SIG_DFL); 
    operation=false;
    if (event_listener)
    pthread_cancel(*event_listener);
    ba->disconnect();
    delete ba;
    free(payload);
    exit(0);
}
void *event_listener_loop(void *arg) {
  string full_id, rec_prefix_id, node_id, bin_node_id;
  string ev_add_rmv;
  string ev_response=string();
  Blackadder *ba = (Blackadder *) arg;
  while(operation){
      Event ev;
        ba->getEvent(ev);
	switch (ev.type) {
	  case PUBLISHED_DATA:
	    full_id=chararray_to_hex(ev.id);
	    bin_node_id=full_id.substr(PURSUIT_ID_LEN*2, PURSUIT_ID_LEN*2);
	    node_id=hex_to_chararray(bin_node_id);
	    rec_prefix_id=string (full_id, 0, PURSUIT_ID_LEN*2);
   	   // cout<<"Published prefix_id is: "<<rec_prefix_id <<" while node id is: "<< node_id<<endl;
// 	    cout << "ev.id : " << full_id<<endl;
	   if(ev.id==(resp_prefix_id + id)){
	    FID_to_TM = new Bitvector((char *)ev.data);
	    cout <<"Updated FID to TM: "<< FID_to_TM->to_string()<<endl;
	    } else if(rec_prefix_id == prefix_id){
		if ((connectivity.find(node_id)) == connectivity.end()) { 
		  connectivity.insert(pair<string, time_t>(node_id, time(&lsu)));
		  ev_add_rmv="1";
		  ev_response+=net_type;
		  ev_response+=ev_add_rmv;
		  ev_response+=node_id;
		  cout<<"FID_to_TM is: " << FID_to_TM->to_string()<<endl;
		  ba->publish_data(bin_lsn_prefix_id + bin_id, IMPLICIT_RENDEZVOUS, (char *)FID_to_TM->_data, FID_LEN, (char *)ev_response.c_str(), ev_response.size());
		} else {
		  connectivity[node_id]= time(&lsu);
		}
		cout<<"NEIGHBOUR NODEID: "<<node_id<<endl;
                break;
        }
      }
   }
   delete FID_to_TM;

   return NULL;
}

int main(int argc, char* argv[]) {
    (void) signal(SIGINT, sigfun);
    char c;
    int user_or_kernel = 0;
    char * net_arg = (char *) malloc(1);
    char * id_arg = (char *) malloc(PURSUIT_ID_LEN);
    while ((c = getopt (argc, argv, "nki:")) != -1){
      switch (c)
	{
	case 'n':
	  sscanf(optarg,"%s", net_arg);
	  net_type = string((const char *) (net_arg) , 1);
	  break;
	case 'k':
	  user_or_kernel = 1;
	  break;
	case 'i':
	  sscanf(optarg,"%s", id_arg);
	  id = string((const char *) (id_arg) , PURSUIT_ID_LEN);
	  break;
	case '?':
	  if (isprint (optopt))
	    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	  else
	    fprintf (stderr,
		    "Unknown option character `\\x%x'.\n",
		    optopt);
	  return 1;
	default:
	  abort ();
	}
    }
      free(net_arg);
      free(id_arg);
      if (user_or_kernel == 0) {
	  ba = Blackadder::Instance(true);
      } else {
	  ba = Blackadder::Instance(false);
      }
    cout << "Process ID: " << getpid() << endl;
    if(id == ""){
      cout<<"Please Enter your NodeID: "<<endl;
      getline(cin, id);
    }
    if (id.size()==16){
      bin_id = hex_to_chararray(id);
    } else if (id.size()==8){
      bin_id=id;
    }
    string id_to_broadcast = bin_prefix_id + bin_id;
    time_t countt;
    countt=time(& countt);
    string add_rmv;
    string response;
    string dis_node=string();
    ifstream getfid;
    string file_name = "/tmp/";
    file_name+=bin_id;
    file_name+="_TMFID.txt";
    getfid.open(file_name.c_str());
    if (getfid.is_open()){
      getline(getfid, FID_str);
    }
    FID_to_TM = new Bitvector(FID_str);
    cout<<"Net type is: "<< net_type << endl << "Initial FID_to_TM is:" << FID_to_TM->to_string()<<endl;
    pthread_create(&_event_listener, NULL, event_listener_loop, (void *) ba);
    event_listener = &_event_listener;
    ba->subscribe_scope(bin_prefix_id, bin_root_prefix_id, BROADCAST_IF, NULL, 0);
    while(operation) {
//       cout << "sleeping for a while..., cycle: "<<countt << endl;
      sleep(1);
      time(&live);
      payload= ctime(&countt);
      ba->publish_data(id_to_broadcast, BROADCAST_IF, NULL, 0, payload, payload_size);
      time(&countt);
      for(con_itr=connectivity.begin(); con_itr!=connectivity.end();){
      last_lsu = (*con_itr).second;
      lsu_diff = difftime(live, last_lsu);
      printf("difference is : %.2lf\n", lsu_diff);
      if (lsu_diff > 7) {
	dis_node=string((*con_itr).first);
	connectivity.erase(con_itr++);
	add_rmv="0";
	response+=net_type;
	response+=add_rmv;
	response+=dis_node;
	size_t response_size = response.size();
	ba->publish_data(bin_lsn_prefix_id + bin_id, IMPLICIT_RENDEZVOUS,(char *)FID_to_TM->_data, FID_LEN, (char *)response.c_str(), response_size);
	response="";
      }else{
	con_itr++;
      }
	}      
    }
    pthread_join(*event_listener, NULL);
    return 0;
}
