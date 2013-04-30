/*
 * Copyright (C) 2012-2013  Mays AL-Naday	mfhaln@essex.ac.uk
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

#include <signal.h>
#include <arpa/inet.h>
#include <set>
#include <map>
#include <vector>
#include <blackadder.hpp>
#include "tm_igraph.hpp"
#include <sstream>
#include <iostream>


using namespace std;

Blackadder *ba = NULL;
TMIgraph *tm_igraph = NULL;
pthread_t _event_listener, *event_listener = NULL;
sig_atomic_t listening = 1;

string tm_id = string(PURSUIT_ID_LEN-1, '0') + "1"; // "00000001"

string req_id = string(PURSUIT_ID_LEN*2-1, 'F') + "E"; // "FF..FFFFFFFFFFFFFE"
string req_prefix_id = string();
string req_bin_id = hex_to_chararray(req_id);
string req_bin_prefix_id = hex_to_chararray(req_prefix_id);

string resp_id = string();
string resp_prefix_id = string(PURSUIT_ID_LEN*2-1, 'F') + "D"; // "FF..FFFFFFFFFFFFFD"
string resp_bin_id = hex_to_chararray(resp_id);
string resp_bin_prefix_id = hex_to_chararray(resp_prefix_id);

//Resiliency scope, when the id is this one, use a broadcast FID and LINK_LOCAL strategy, used for reliability and network resiliency
string resl_id = string(PURSUIT_ID_LEN*2-1, 'F') + "0"; // "FF..FFFFFFFFFFFFF0";
string resl_prefix_id = string();
string resl_bin_id = hex_to_chararray(resl_id);
string resl_bin_prefix_id = hex_to_chararray(resl_prefix_id);

string resl_resp_id = string(PURSUIT_ID_LEN*2-1, 'F') + "1"; // "FF..FFFFFFFFFFFFF1"
string resl_resp_prefix_id = string();
string resl_resp_bin_id = hex_to_chararray(resl_resp_id);
string resl_resp_bin_prefix_id = hex_to_chararray(resl_resp_prefix_id);

string resl_req_id = string(PURSUIT_ID_LEN*2-1, 'F') + "2"; // "FF..FFFFFFFFFFFFF2"
string resl_req_prefix_id = string();
string resl_req_bin_id = hex_to_chararray(resl_req_id);
string resl_req_bin_prefix_id = hex_to_chararray(resl_req_prefix_id);

//The Resiliency functions
void update_info_delivery (char *request, int request_len){
  string request_str = string (request, request_len);
  unsigned char request_type;
  unsigned char no_ids;
  unsigned char IDlen;
  unsigned char no_paths;
  unsigned char path_len;
  unsigned char no_alt_pubs;
  int path_idx = 0;
  int alt_pub_idx = 0;
  string alt_publisher;
  set<string> alternative_publishers;
  string path_v;
  set<string> path_vecs;
  memcpy(&request_type, request, sizeof(request_type));
  if(request_type == UPDATE_DELIVERY){
    memcpy(&no_ids, request + sizeof(request_type), sizeof(no_ids));
    memcpy(&IDlen, request + sizeof(request_type) + sizeof(no_ids), sizeof(IDlen));
    int ids_size= (int)IDlen * PURSUIT_ID_LEN + sizeof(no_ids) + sizeof(IDlen);
    char *ids = (char *) malloc (ids_size);
    memcpy(ids, request + sizeof(request_type) , ids_size);
    string info_id = string((const char *)(ids) , ids_size);
    cout << "TM_RV_BRK: New Status of: " << chararray_to_hex(info_id) << endl;
    /* Extract the list of Path Vectors */
    memcpy(&no_paths, request + sizeof(request_type) + ids_size, sizeof(no_paths));
    cout << "Paths: " << (int)no_paths << endl;
    if((int)no_paths > 0){
      path_idx = sizeof(request_type) + ids_size + sizeof(no_paths);
      for(int i = 1; i <= (int)no_paths ; i++){
      memcpy(&path_len, request + path_idx, sizeof(path_len));
      path_idx += sizeof(path_len);
      path_v = string((const string&)request_str, path_idx , (int)path_len);
      /*print out the path vectors*/
      cout << path_v << endl;
      path_vecs.insert(path_v);
      path_idx += (int)path_len;
      }
    } else {
      cout << "TM_RV_BRK: Delivery Finished of : " << chararray_to_hex(info_id) << endl;
      path_idx = sizeof(request_type) + ids_size + sizeof(no_paths);
    }
    /* Update the Global Data Structure of Path Vectors which are currently utilized to deliver Data (path_info) */
    if(tm_igraph->path_info.find(info_id) == tm_igraph->path_info.end()){
	  if((!path_vecs.empty())){
	    tm_igraph->path_info.insert(make_pair(info_id, path_vecs));
	    path_vecs.clear();
	  }
    }else{
      tm_igraph->path_info.erase(info_id);
      if((!path_vecs.empty()))
	tm_igraph->path_info.insert(make_pair(info_id, path_vecs));
    }
    
    /*Extract the list of Alternative Publishers*/
    memcpy(&no_alt_pubs, request + path_idx, sizeof(no_alt_pubs));
    alt_pub_idx += sizeof(no_alt_pubs);
    cout << "Alternative Publishers: " << endl;
    for (int j = 0 ; j < (int) no_alt_pubs ; j ++){
      alt_publisher = string(request + path_idx + alt_pub_idx, PURSUIT_ID_LEN);
      cout << alt_publisher << endl;
      alternative_publishers.insert(alt_publisher);
      alt_pub_idx += PURSUIT_ID_LEN;
    }
    /*Update the Global data structure of Alternative Publishers (ex_pub)*/
    if(tm_igraph->ex_pubs.find(info_id) != tm_igraph->ex_pubs.end())
      tm_igraph->ex_pubs.erase(info_id);
    tm_igraph->ex_pubs.insert(pair<string, set<string> >(info_id, alternative_publishers));
    free(ids);
  }
  
}

void handle_lsn_request(char *request, int request_len){
  unsigned char request_type;
  string net_type;
  string info_id;
  unsigned char lsn_publisher_vtx ;
  unsigned char lsn_neighbour_vtx;
  int affected_pub;
  int affected_sub;
  unsigned char no_publishers;
  unsigned char no_subscribers;
  unsigned char response_type;
  unsigned char strategy;
  set<string> publishers;
  set<string> subscribers;
  vector<int> path_vertices;
  bool Recover = false;
  map<string, set<string> >::iterator info_selector;
  int field_offset = 0;
  memcpy(&request_type, request, sizeof(request_type));
  field_offset += sizeof(request_type);
  if(request_type == LSN){
    net_type=string(request, 1);
    field_offset += net_type.size();
    memcpy(&lsn_publisher_vtx, request + field_offset , sizeof(lsn_publisher_vtx));
    field_offset += sizeof(lsn_publisher_vtx);
    memcpy(&lsn_neighbour_vtx, request + field_offset , sizeof(lsn_neighbour_vtx));
    field_offset += sizeof(lsn_neighbour_vtx);
    if(!(tm_igraph->path_info.empty())){
      //examin current information delivery
      for(map<string, set<string> >::iterator info_iter = tm_igraph->path_info.begin(); info_iter!=tm_igraph->path_info.end(); info_iter++ ){ //path_iter++
	info_id=(*info_iter).first;
	for(set<string>::iterator path_iter = info_iter->second.begin(); path_iter != info_iter->second.end(); path_iter++){
	  string path = (*path_iter);
	  path_vertices = split_int_string(path, '/');
	  for(size_t i = 0; i < path_vertices.size()-1 ; i++){
	    //findout if the broken edge is part of the current path
	    if (((path_vertices[i] == (int)lsn_publisher_vtx) && (path_vertices[i+1] == (int)lsn_neighbour_vtx)) || ((path_vertices[i+1] == (int)lsn_publisher_vtx) && (path_vertices[i] == (int)lsn_neighbour_vtx))) {
	      cout<<"TM_RV_BRK: Information "<< chararray_to_hex(info_id) <<" on Path " << (*path_iter) <<  " Has Been Cut Between: " << path_vertices[i+1]<<" and " << path_vertices[i]<< endl;
	      affected_pub = path_vertices.front();
	      for (map<string, int>::iterator pub_pos = tm_igraph->reverse_node_index.begin(); pub_pos != tm_igraph->reverse_node_index.end(); pub_pos++){
		if ((*pub_pos).second == affected_pub){
		  publishers.insert((*pub_pos).first);
		  break;
		}
	      }
	      affected_sub = path_vertices.back();
	      for (map<string, int>::iterator sub_pos = tm_igraph->reverse_node_index.begin(); sub_pos != tm_igraph->reverse_node_index.end(); sub_pos++){
		if ((*sub_pos).second == affected_sub){
		  subscribers.insert((*sub_pos).first);
		  break;
		}
	      }
	      break;
	    }
	  }
	/*path loop end*/
	}
	/*if publishers is not empty then there are broken delivery tree*/
	if(!publishers.empty()){
	  Recover = true;
	  info_selector=tm_igraph->ex_pubs.find(info_id);
	  for(set<string>::iterator alt_pub_it = info_selector->second.begin(); alt_pub_it != info_selector->second.end(); alt_pub_it++){
	    publishers.insert((*alt_pub_it));
	  }
	  /**
	   *construct a Re-MATCH_PUB_SUBS packet that is to be sent to the TM for the broken paths
	   */
	  response_type = MATCH_PUB_SUBS;
	  strategy = IMPLICIT_RENDEZVOUS;	//strategy at this point is irrelevant to the communication between TM and TM_RV_BRK, however it has been considered to fill in the corresponding field in the packet and it may be used later on
	  no_publishers = publishers.size();
	  no_subscribers = subscribers.size();
	  int response_size = sizeof(response_type) + sizeof(strategy) + sizeof(no_publishers) + (int)no_publishers * PURSUIT_ID_LEN + sizeof(no_subscribers) + (int)no_subscribers * PURSUIT_ID_LEN + info_id.length();
	  char * response = (char *) malloc (response_size);
	  /*create the payload*/
	  memcpy(response , &response_type , sizeof(response_type));
	  memcpy(response + sizeof(response_type) , &strategy , sizeof(strategy));
	  memcpy(response + sizeof(response_type) + sizeof(strategy) , &no_publishers , sizeof(no_publishers));
	  int pubsub_idx = 0;
	  /*fill in the publishers*/
	  for(set<string>::iterator pub = publishers.begin(); pub != publishers.end(); pub++){
	    memcpy(response + sizeof(response_type) + sizeof(strategy) + sizeof(no_publishers) + pubsub_idx , (*pub).c_str() , (*pub).length());
	    pubsub_idx += (*pub).length();
	  }
	  memcpy(response + sizeof(response_type) + sizeof(strategy) + sizeof(no_publishers) + pubsub_idx , &no_subscribers , sizeof(no_subscribers));
	  /*fill in the subscribers*/
	   for(set<string>::iterator sub = subscribers.begin(); sub != subscribers.end(); sub++){
	     memcpy(response + sizeof(response_type) + sizeof(strategy) + sizeof(no_publishers) + pubsub_idx + sizeof(no_subscribers),(*sub).c_str() , (*sub).length());
	     pubsub_idx += (*sub).length();
	  }
	  memcpy(response + sizeof(response_type) + sizeof(strategy) + sizeof(no_publishers) + pubsub_idx + sizeof(no_subscribers), info_id.c_str(), info_id.length());
	  string response_id = resl_resp_bin_id + tm_igraph->nodeID;
	  Bitvector *FID_to_tm = tm_igraph->calculateFID(tm_igraph->nodeID, tm_id);
	  ba->publish_data(response_id, IMPLICIT_RENDEZVOUS, (char *) FID_to_tm->_data, FID_LEN, response, response_size);
	  delete FID_to_tm;
	  publishers.clear();
	  subscribers.clear();
	  path_vertices.clear();
	}
      /*info loop end*/
      }
    }
   if(!Recover)
     cout << "TM_RV_BRK: no Paths have been affected..." << endl;
  }else{
    cout << "TM_RV_BRK: Unkown Publication Type..." <<endl;
  }
  
}

void *event_listener_loop(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
    string scope_id, full_id, node_id;
    while (listening) {
        Event ev;
        ba->getEvent(ev);
	scope_id=string(ev.id,0, PURSUIT_ID_LEN);
	if(scope_id == resl_bin_id) {
	  cout <<"TM_RV_BRK: Update Delivery Status" <<endl;
	  update_info_delivery((char *) ev.data, ev.data_len);
	} else if (scope_id == resl_req_bin_id) {
            cout << "\nTM_RV_BRK: Request for checking delivery status upon a link failure" << endl;
            handle_lsn_request((char *) ev.data, ev.data_len);
        } else if (ev.type == UNDEF_EVENT && !listening) {
            cout << "TM: final event" << endl;
        }
    }
    return NULL;
}

void sigfun(int sig) {
    listening = 0;
    if (event_listener)
        pthread_cancel(*event_listener);
    (void) signal(SIGINT, SIG_DFL);
}

int main(int argc, char* argv[]) {
    (void) signal(SIGINT, sigfun);
    cout << "TM_RV_BRK: starting - process ID: " << getpid() << endl;
    if (argc < 2) {
        cout << "TM_RV_BRK: the topology file is missing" << endl;
        exit(0);
    }
    tm_igraph = new TMIgraph();
    /*read the graphML file that describes the topology*/
    if (tm_igraph->readTopology(argv[1]) < 0) {
        cout << "TM: couldn't read topology file...aborting" << endl;
        exit(0);
    }
    cout << "TM Node: " << tm_igraph->nodeID << endl;
    /***************************************************/
    if (tm_igraph->mode.compare("kernel") == 0) {
        ba = Blackadder::Instance(false);
    } else {
        ba = Blackadder::Instance(true);
    }
  
    pthread_create(&_event_listener, NULL, event_listener_loop, (void *) ba);
    event_listener = &_event_listener;
    ba->subscribe_scope(req_bin_id, req_bin_prefix_id, IMPLICIT_RENDEZVOUS, NULL, 0);
    ba->subscribe_scope(resl_bin_id, resl_bin_prefix_id, IMPLICIT_RENDEZVOUS, NULL, 0);
    ba->subscribe_scope(resl_req_bin_id, resl_req_bin_prefix_id, IMPLICIT_RENDEZVOUS, NULL, 0);
    
    pthread_join(*event_listener, NULL);
    cout << "TM: disconnecting" << endl;
    ba->disconnect();
    delete ba;
    delete tm_igraph;
    cout << "TM: exiting" << endl;
    return 0;
}
