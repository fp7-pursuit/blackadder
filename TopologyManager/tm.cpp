/*
 * Copyright (C) 2010-2011  George Parisis and Dirk Trossen
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

#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <set>
#include <blackadder.hpp>

#include "tm_graph.hpp"
#include "tm_igraph.hpp"
#include "te_graph_mf.hpp"

// QoS Stuff
#include <qos_structs.hpp>
#include <lsmpacket.hpp>
#include "tm_qos.hpp"
#include <metadatapacket.hpp>

using namespace std;
extern pthread_t _te_thread, *te_thread;

Blackadder *ba = NULL;
//TMIgraph *tm_igraph = NULL;
TMgraph *tm_igraph = NULL;
pthread_t _event_listener, *event_listener = NULL;
sig_atomic_t listening = 1;

string tm_rv_brk_id = string(PURSUIT_ID_LEN-1, '0') + "1"; // "000000001"

string req_id = string(PURSUIT_ID_LEN*2-1, 'F') + "E"; // "FF..FFFFFFFFFFFFFE"
string req_prefix_id = string();
string req_bin_id = hex_to_chararray(req_id);
string req_bin_prefix_id = hex_to_chararray(req_prefix_id);

string resp_id = string();
string resp_prefix_id = string(PURSUIT_ID_LEN*2-1, 'F') + "D"; // "FF..FFFFFFFFFFFFFD"
string resp_bin_id = hex_to_chararray(resp_id);
string resp_bin_prefix_id = hex_to_chararray(resp_prefix_id);

string resl_id = string(PURSUIT_ID_LEN*2-1, 'F') + "0"; // "FF..FFFFFFFFFFFFF0";
string resl_prefix_id = string();
string resl_bin_id = hex_to_chararray(resl_id);
string resl_bin_prefix_id = hex_to_chararray(resl_prefix_id);

string resl_resp_id = string(PURSUIT_ID_LEN*2-1, 'F') + "1"; // "FF..FFFFFFFFFFFFF1"
string resl_resp_prefix_id = string();
string resl_resp_bin_id = hex_to_chararray(resl_resp_id);
string resl_resp_bin_prefix_id = hex_to_chararray(resl_resp_prefix_id);

string resl_req_id = string(PURSUIT_ID_LEN*2-1, 'F') + "2"; // "FF..FFFFFFFFFFFFF2"
string resl_req_bin_id = hex_to_chararray(resl_req_id);
 
string lsn_id = string(PURSUIT_ID_LEN*2-1, 'F') + "A"; // "FF..FFFFFFFFFFFFFA"
string lsn_prefix_id = string();
string lsn_bin_id = hex_to_chararray(lsn_id);
string lsn_bin_prefix_id = hex_to_chararray(lsn_prefix_id);

// Link state monitoring scope (Status Updates - QoS)
string lsm_scope = string(PURSUIT_ID_LEN*2-1, 'F') + "B"; // "FF..FFFFFFFFFFFFFB"
// Binary of Link State Monitoring Scope
string lsm_bin_scope = hex_to_chararray(lsm_scope);

// Global II metadata cache
IIMetaCache metaCache;

void handleIIMetaData(char *request, int request_len){
  cout<<"Got Meta Data!!!"<<endl;
  MetaDataPacket pkt((uint8_t *)request, request_len);
  metaCache.update(pkt.getID_RAW(), pkt.getIIStatus(), ba);
  
  // TODO: We have to re-route this II
}

void handleRequest(char *request, int request_len) {
    cout<<"---------------- REQUEST --------------------"<<endl;
    unsigned char request_type;
    unsigned char no_publishers;
    unsigned char no_alt_publishers;
    unsigned char no_subscribers;
    unsigned char no_pathvectors;
    unsigned char no_ids;
    unsigned char IDLen;
    string pathvectors_Field = string();
    map<string, set<string> > pathvectors;
    map<string, set<string> >::iterator tmp_map_it;
    string nodeID;
    set<string> publishers;
    string alt_publishers;
    set<string> subscribers;
    map<string, Bitvector *> result = map<string, Bitvector *>();
    map<string, Bitvector *>::iterator map_iter;
    unsigned char response_type;
    int idx = 0;
    unsigned char strategy;
    memcpy(&request_type, request, sizeof (request_type));
    memcpy(&strategy, request + sizeof (request_type), sizeof (strategy));
    // Early break for II metadata
    if (request_type==QoS_METADATA){
      handleIIMetaData(request, request_len);
      return;
    }
    if (request_type == MATCH_PUB_SUBS) {
	no_alt_publishers = 0;
	no_pathvectors = 0;
	/*this a request for topology formation*/
        memcpy(&no_publishers, request + sizeof (request_type) + sizeof (strategy), sizeof (no_publishers));
        cout << "Publishers: ";
        for (int i = 0; i < (int) no_publishers; i++) {
            nodeID = string(request + sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + idx, PURSUIT_ID_LEN);
            cout << nodeID << " ";
            idx += PURSUIT_ID_LEN;
            publishers.insert(nodeID);
        }
        cout << endl;
        cout << "Subscribers: ";
        memcpy(&no_subscribers, request + sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + idx, sizeof (no_subscribers));
        for (int i = 0; i < (int) no_subscribers; i++) {
            nodeID = string(request + sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + sizeof (no_subscribers) + idx, PURSUIT_ID_LEN);
            cout << nodeID << " ";
            idx += PURSUIT_ID_LEN;
            subscribers.insert(nodeID);
        }
        cout << endl;
	// MOVED ON TOP OF CALC + BUG FIX??
	// extract the Information IDs for this MATCH_PUB_SUBS request...
	int ids_size= request_len- sizeof(request_type) - sizeof(strategy) - sizeof(no_publishers) - (int)no_publishers * PURSUIT_ID_LEN - sizeof(no_subscribers) - (int)no_subscribers * PURSUIT_ID_LEN ;
	char *ids =(char *) malloc (ids_size);
	memcpy(&no_ids, request + sizeof(request_type) + sizeof(strategy) + sizeof(no_publishers) + (int)no_publishers * PURSUIT_ID_LEN + sizeof(no_subscribers) + (int)no_subscribers * PURSUIT_ID_LEN , sizeof(no_ids));
	memcpy(&IDLen, request + sizeof(request_type) + sizeof(strategy) + sizeof(no_publishers) + (int)no_publishers * PURSUIT_ID_LEN + sizeof(no_subscribers) + (int)no_subscribers * PURSUIT_ID_LEN + sizeof(no_ids), sizeof(IDLen));
	memcpy (ids, request + sizeof(request_type) + sizeof(strategy) + sizeof(no_publishers) + (int)no_publishers * PURSUIT_ID_LEN + sizeof(no_subscribers) + (int)no_subscribers * PURSUIT_ID_LEN + sizeof(no_ids) + sizeof(IDLen), ids_size);
	string ids_str = string((const char *)(ids) , (int)IDLen * PURSUIT_ID_LEN);
	//finished extracting the information IDs of this Request.......
	
	// Use QoS map if: (a) enabled and (b) is initialised 
	if (tm_igraph->getExten(QOS) && tm_igraph->isQoSMapOk()) {
	  // before doing anything... check that we do not need to subscribe to the 
	  // MetaData of that item...
	  cout<<"Request is for II="<<chararray_to_hex(ids_str)<<endl;
	  bool added = metaCache.sendQueryIfNeeded(ids_str, ba);
	  // Get the priority
	  int prio = DEFAULT_QOS_PRIO;
	  // Try to avoid messing with the cache while quering
	  // IT IS NOT THREAD-SAFE!
	  if (!added)
	    prio = metaCache.getIIQoSPrio(ids_str, ba);
	  
	  // Get the available network class (ie. map II priority 98
	  // to net 95 for a net that supports 0,95 and 99)
	  uint16_t netprio = tm_igraph->getWeightKeyForIIPrio(prio);
	  cout<<"TM: QoS: II Priority: "<<prio;
	  cout<<"Mappend on NET Priority Class: "<<netprio<<endl;
	  tm_igraph->calculateFID_weighted(publishers, subscribers, result, pathvectors, &tm_igraph->qlwm[netprio]);
	}else{
	  // Core calc. here...
	  tm_igraph->calculateFID(publishers, subscribers, result, pathvectors);
	}
        /*notify publishers*/
        for (map_iter = result.begin(); map_iter != result.end(); map_iter++) {
            if ((*map_iter).second == NULL) {
                cout << "Publisher " << (*map_iter).first << ", FID: NULL" << endl;
                response_type = STOP_PUBLISH;
                int response_size = request_len - sizeof(strategy) - sizeof (no_publishers) - no_publishers * PURSUIT_ID_LEN - sizeof (no_subscribers) - no_subscribers * PURSUIT_ID_LEN;
                char *response = (char *) malloc(response_size);
                memcpy(response, &response_type, sizeof (response_type));
                int ids_index = sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + no_publishers * PURSUIT_ID_LEN + sizeof (no_subscribers) + no_subscribers * PURSUIT_ID_LEN;
                memcpy(response + sizeof (response_type), request + ids_index, request_len - ids_index);
                /*find the FID to the publisher*/
                string destination = (*map_iter).first;
                Bitvector *FID_to_publisher = tm_igraph->calculateFID(tm_igraph->getNodeID(), destination);
                string response_id = resp_bin_prefix_id + (*map_iter).first;
                ba->publish_data(response_id, IMPLICIT_RENDEZVOUS, (char *) FID_to_publisher->_data, FID_LEN, response, response_size);
		/*When resiliency support is in effect, construct a string of the alternative publishers, which will be sent to the TM_RV_BRK*/
		if(tm_igraph->getExten(RESI)){
		 (int)no_alt_publishers++;
		 alt_publishers.append((*map_iter).first);
		}
                delete FID_to_publisher;
                free(response);
            } else {
                cout << "Publisher " << (*map_iter).first << ", FID: " << (*map_iter).second->to_string() << endl;
                response_type = START_PUBLISH;
                int response_size = request_len - sizeof(strategy) - sizeof (no_publishers) - no_publishers * PURSUIT_ID_LEN - sizeof (no_subscribers) - no_subscribers * PURSUIT_ID_LEN + FID_LEN;
                char *response = (char *) malloc(response_size);
                memcpy(response, &response_type, sizeof (response_type));
                int ids_index = sizeof (request_type) + sizeof (strategy) + sizeof (no_publishers) + no_publishers * PURSUIT_ID_LEN + sizeof (no_subscribers) + no_subscribers * PURSUIT_ID_LEN;
                memcpy(response + sizeof (response_type), request + ids_index, request_len - ids_index);
                memcpy(response + sizeof (response_type) + request_len - ids_index, (*map_iter).second->_data, FID_LEN);
                /*find the FID to the publisher*/
                string destination = (*map_iter).first;
                Bitvector *FID_to_publisher = tm_igraph->calculateFID(tm_igraph->getNodeID(), destination);
                string response_id = resp_bin_prefix_id + (*map_iter).first;
                ba->publish_data(response_id, IMPLICIT_RENDEZVOUS, (char *) FID_to_publisher->_data, FID_LEN, response, response_size);
                delete (*map_iter).second;
                delete FID_to_publisher;
                free(response);
		/*When resiliency support is in effect, construct a string of the path vectors, which will be sent to the TM_RV_BRK*/
		if(tm_igraph->getExten(RESI)){
		  tmp_map_it = pathvectors.find((*map_iter).first);
		  for(set<string>::iterator path_it = tmp_map_it->second.begin(); path_it != tmp_map_it->second.end() ; path_it++){
		    unsigned char pathvector_Size;
		    pathvector_Size = (*path_it).size();
		    pathvectors_Field += pathvector_Size;
		    pathvectors_Field += (*path_it);
		    (int)no_pathvectors++;
		  }
		}
            }
        }
    /*If resiliency support is in effect, create a notification of this delivery update and publish it to the TM_RV_BRK*/    
    if(tm_igraph->getExten(RESI)){
	  response_type = UPDATE_DELIVERY;
	  int pathvector_FieldSize = 0;
	  if(no_pathvectors > 0)
	    pathvector_FieldSize = pathvectors_Field.size();
	  int alt_publishers_size = alt_publishers.size();
	  int update_response_size = sizeof(response_type) + sizeof(no_ids) + sizeof(IDLen) + ((int)no_ids * (int)IDLen * PURSUIT_ID_LEN) + sizeof(no_pathvectors) + pathvector_FieldSize + sizeof(no_alt_publishers) + alt_publishers_size * PURSUIT_ID_LEN;
	  char * response = (char *) malloc(update_response_size);
	  memcpy(response, &response_type, sizeof(response_type));
	  memcpy(response + sizeof(response_type), &no_ids, sizeof(no_ids));
	  memcpy(response + sizeof(response_type) + sizeof(no_ids), &IDLen, sizeof(IDLen));
	  memcpy(response + sizeof(response_type) + sizeof(no_ids) + sizeof(IDLen), ids , ids_size);
	  memcpy(response + sizeof(response_type) + sizeof(no_ids) + sizeof(IDLen) + (int)IDLen * PURSUIT_ID_LEN, &no_pathvectors, sizeof(no_pathvectors));
  	  memcpy(response + sizeof(response_type) + sizeof(no_ids) + sizeof(IDLen) + (int)IDLen * PURSUIT_ID_LEN + sizeof(no_pathvectors) , (char *)pathvectors_Field.c_str() , pathvector_FieldSize);
	  memcpy(response + sizeof(response_type) + sizeof(no_ids) + sizeof(IDLen) + (int)IDLen * PURSUIT_ID_LEN + sizeof(no_pathvectors) + pathvector_FieldSize , &no_alt_publishers , sizeof(no_alt_publishers));
	  memcpy(response + sizeof(response_type) + sizeof(no_ids) + sizeof(IDLen) + (int)IDLen * PURSUIT_ID_LEN + sizeof(no_pathvectors) + pathvector_FieldSize  + sizeof(no_alt_publishers) , (char *)alt_publishers.c_str() , (alt_publishers_size * PURSUIT_ID_LEN));
	  cout << "TM: info_id: " << chararray_to_hex(ids_str) << endl;
	  string response_id = resl_bin_id + tm_rv_brk_id;
	  Bitvector *FID_to_publisher = tm_igraph->calculateFID(tm_igraph->getNodeID(), tm_rv_brk_id);
	  ba->publish_data(response_id, IMPLICIT_RENDEZVOUS, (char *) FID_to_publisher->_data, FID_LEN,response, update_response_size);
	  delete FID_to_publisher;
	  free(response);
	}
	free(ids);
    } else if ((request_type == SCOPE_PUBLISHED) || (request_type == SCOPE_UNPUBLISHED)) {
        /*this a request to notify subscribers about a new scope*/
        memcpy(&no_subscribers, request + sizeof (request_type) + sizeof (strategy), sizeof (no_subscribers));
        for (int i = 0; i < (int) no_subscribers; i++) {
            nodeID = string(request + sizeof (request_type) + sizeof (strategy) + sizeof (no_subscribers) + idx, PURSUIT_ID_LEN);
            Bitvector *FID_to_subscriber = tm_igraph->calculateFID(tm_igraph->getNodeID(), nodeID);
            int response_size = request_len - sizeof(strategy) - sizeof (no_subscribers) - no_subscribers * PURSUIT_ID_LEN + FID_LEN;
            int ids_index = sizeof (request_type) + sizeof (strategy) + sizeof (no_subscribers) + no_subscribers * PURSUIT_ID_LEN;
            char *response = (char *) malloc(response_size);
            string response_id = resp_bin_prefix_id + nodeID;
            memcpy(response, &request_type, sizeof (request_type));
            memcpy(response + sizeof (request_type), request + ids_index, request_len - ids_index);
            //cout << "PUBLISHING NOTIFICATION ABOUT NEW OR DELETED SCOPE to node " << nodeID << " using FID " << FID_to_subscriber->to_string() << endl;
            ba->publish_data(response_id, IMPLICIT_RENDEZVOUS, FID_to_subscriber->_data, FID_LEN, response, response_size);
            idx += PURSUIT_ID_LEN;
            delete FID_to_subscriber;
            free(response);
        }
    } else {
      cout<<"UNKNOWN operation!\n";
    }
    cout<<"---------------- EoR --------------------\n"<<endl;
}

void handle_lsn_request(char *request, int request_len, char * request_pub) {
    cout<<"---------------- LSN REQUEST--------------------"<<endl;
    string lsn_type;	//ADD or RMV link
    string net_type;
    unsigned char notification_type;
    unsigned char lsn_publisher_vtx;
    unsigned char lsn_neighbour_vtx;	//triggering (publishing) node and disconnected/recovered node, will be used as the string form of the reverse_node_index
    string lsn_publisher=string(request_pub);
    string lsn_neighbour=string(request, request_len-PURSUIT_ID_LEN, PURSUIT_ID_LEN);
    lsn_type= string(request,request_len-(PURSUIT_ID_LEN + 1), 1);
    net_type=string(request, request_len-(PURSUIT_ID_LEN + 2),1);
    //update_op flag to indicate whether its add or remove edge operation...
    bool lsn_op=(lsn_type =="1");
    lsn_publisher_vtx = tm_igraph->get_reverse_node_index(lsn_publisher);
    lsn_neighbour_vtx = tm_igraph->get_reverse_node_index(lsn_neighbour);
    tm_igraph->updateGraph(lsn_neighbour, lsn_publisher, net_type, lsn_op);
    if(!lsn_op){
      cout<< "TM: Link Failure: " << lsn_publisher << " - " << lsn_neighbour << endl;
      Bitvector *FID_to_neighbour = tm_igraph->calculateFID(tm_igraph->getNodeID(), lsn_neighbour);
      Bitvector *FID_neighbour_TM = tm_igraph->calculateFID(lsn_neighbour,tm_igraph->getNodeID());
      Bitvector *FID_neighbour_RV = tm_igraph->calculateFID(lsn_neighbour,tm_igraph->getRVNodeID());
      string response_id = resp_bin_prefix_id + lsn_neighbour;
      //update the nieghbour node (disconnected node) with a new TMFID and RVFID...
      notification_type = UPDATE_BA;
      int update_size = sizeof(notification_type) + (2 * FID_LEN);
      char * update_response = (char*)malloc(update_size);
      memcpy(update_response, &notification_type, sizeof(notification_type));
      memcpy(update_response + sizeof(notification_type), FID_neighbour_RV->_data, FID_LEN);
      memcpy(update_response + sizeof(notification_type) + FID_LEN, FID_neighbour_TM->_data, FID_LEN);
      //update the neighbour of the lsn publisher (i.e. disconnected node) with new to_TM and to_RV FIDs, RV node is assumed to be the same as TM node
      ba->publish_data(response_id, IMPLICIT_RENDEZVOUS, (char *) FID_to_neighbour->_data, FID_LEN, update_response,  update_size);
      cout <<"TM: New TMFID to Node " << lsn_neighbour <<  ", "  << FID_neighbour_TM->to_string() << endl;
      cout <<"TM: New RVFID to Node " << lsn_neighbour <<  ", "  << FID_neighbour_RV->to_string() << endl;
      notification_type = LSN;
      int lsn_msg_size = sizeof(notification_type) + sizeof(unsigned char) + sizeof(lsn_publisher_vtx) + sizeof(lsn_neighbour_vtx);
      char * lsn_msg = (char *) malloc (lsn_msg_size);
      memcpy(lsn_msg, &notification_type, sizeof(notification_type));
      memcpy(lsn_msg + sizeof(notification_type) , (char *)net_type.c_str(), sizeof(unsigned char));
      memcpy(lsn_msg + sizeof(notification_type) + sizeof(unsigned char), &lsn_publisher_vtx , sizeof(lsn_publisher_vtx));
      memcpy(lsn_msg + sizeof(notification_type) + sizeof(unsigned char) + sizeof(lsn_publisher_vtx) , &lsn_neighbour_vtx, sizeof(lsn_neighbour_vtx));
      response_id = resl_req_bin_id + tm_rv_brk_id;
      Bitvector *FID_to_TMRVBRK = tm_igraph->calculateFID(tm_igraph->getNodeID(), tm_rv_brk_id);
      ba->publish_data(response_id, IMPLICIT_RENDEZVOUS, (char *) FID_to_TMRVBRK->_data, FID_LEN, lsn_msg, lsn_msg_size);
      cout << "TM: Request Delivery Check from TM_RV_BRK..." << endl;
      delete FID_to_TMRVBRK;
      free(update_response);
      free(lsn_msg);
    } else {
      cout <<"TM: Link Establishement: " << lsn_publisher << " - " << lsn_neighbour << endl;
    }
    cout<<"---------------- EoR --------------------"<<endl;
}

/**
 * Handle link state update... This comes from each forwarder and contains 
 * a list with the links of it and their status. Currently working only on
 * mac_qos mode. The update has (for each outgoing QUEUE): <br>
 * - Priority (from config - but could change dynamically in the future) <br>
 * - Incoming rate (to the queue): represents the bandwidth demand <br>
 * - Outgoint rate (from the queue): represents the utilized bandwidth (in case 
 *   the next is zer0, and for debuging the shaper to confirm it limits correctly) <br>
 * - The queue backlog in packets ? (FIXME:may be bytes)<br>
 * 
 * Additionally, there are some static (currently) parameters that should be
 * added into the link configuration. These are: <br>
 * - The link capacity (hard-coded to 100Mbps - could be used for ns3 too) <br>
 * - The link type (hard-coded to NT_ETH - Ethernet) <br>
 * 
 * For more info refer to qos_structs.hpp
 */
void handleLSMUpdate(uint8_t * data, int data_len, const string & request_pub){
  //cout<<"Got a LSM update!! Node="<<request_pub<<endl;
  LSMPacket pkt(data, data_len);
  // Update the graph for each link
  for (uint i=0; i<pkt.getLinksLen(); i++){
    tm_igraph->updateLinkState(pkt.getLidStr(i), pkt.getLinkStatus(i));
    QoSList l=pkt.getLinkStatus(i);
  }
  // DEBUG: Print the qos link weight map
  // cout<<(tm_igraph->qlwm)<<endl;
}

void *event_listener_loop(void *arg) {
    Blackadder *ba = (Blackadder *) arg;
    string scope_id, full_id, node_id;
    while (listening) {
        Event ev;
        ba->getEvent(ev);
        if (ev.type == UNDEF_EVENT) {
            if (!listening)
                cout << "TM: final event" << endl;
            return NULL;
        }
	scope_id = string(ev.id,0, PURSUIT_ID_LEN);
	node_id = string(ev.id, PURSUIT_ID_LEN, PURSUIT_ID_LEN);
	if (ev.type == PUBLISHED_DATA) {
	  if ((scope_id == lsn_bin_id) && (tm_igraph->getExten(RESI))) {	//resiliency
	    handle_lsn_request((char *) ev.data, ev.data_len, (char *)node_id.c_str());
          } else if ((scope_id==lsm_bin_scope) && (tm_igraph->getExten(QOS))) { 	// mac_qos
            // Call the handler for LSM... This should update the internal link
            // status structure and the Graph's edge weight map...
            handleLSMUpdate( (uint8_t *) ev.data, ev.data_len, node_id);
	  } else {
	      handleRequest((char *) ev.data, ev.data_len);
          }
	} else {
            cout << "TM: I am not expecting any other notification...FATAL" << endl;
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
  opterr = 0;
  bool te=false;
  bool resi = false;
  bool qos = false;
  long te_delay=60;
  double te_e=0.1;
  double defaultBW=1e9;

  char c;
  while ((c = getopt (argc, argv, "rqtd:")) != -1){
    switch (c)
      {
      case 't':
	te = true;
	break;
      case 'd':
	sscanf(optarg,"%ld",&te_delay);
	break;
      case 'b':
	sscanf(optarg,"%lg",&defaultBW);
	break;
      case 'e':
	sscanf(optarg,"%lg",&te_e);
	break;
      case 'r':
	resi = true;
	break;
      case 'q':
	qos = true;
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

  if (optind >= argc) {
    fprintf(stderr, "The topolgy file is missing\n");
    exit(EXIT_FAILURE);
  }

  (void) signal(SIGINT, sigfun);
  cout << "TM: starting - process ID: " << getpid() << endl;

  if (te) {
    tm_igraph = new TEgraphMF();
    tm_igraph->setExten(TE);
    cout<<"Setting delay to "<<te_delay<<endl;
    ((TEgraphMF*)tm_igraph)->initialise(te_delay,te_e,defaultBW);
    cout << "TM: TE Support is in effect" << endl;
  } else {
    tm_igraph = new TMIgraph();
  }
  
  
  
  if(resi){
    tm_igraph->setExten(RESI);
    cout << "TM: Resiliency Support is in effect" << endl;
  }
  if(qos){
    tm_igraph->setExten(QOS);
    cout << "TM: QoS Support is in effect" << endl;
  }
  
  /*read the graphML file that describes the topology*/
  if (tm_igraph->readTopology(argv[optind]) < 0) {
    cout << "TM: couldn't read topology file...aborting" << endl;
    exit(0);
  }
  cout << "Blackadder Node: " << tm_igraph->getNodeID() << endl;
  
  /***************************************************/
  if (tm_igraph->getMode().compare("kernel") == 0) {
      ba = Blackadder::Instance(false);
  } else {
      ba = Blackadder::Instance(true);
  }
  
  
  if(tm_igraph->getExten(TE)) {
    cout << "TE extension detected: Spawning..." << endl;
    pthread_create(&_te_thread, NULL, te_loop,(void*)tm_igraph);
    te_thread = &_te_thread;
  }
  
  pthread_create(&_event_listener, NULL, event_listener_loop, (void *) ba);
  event_listener = &_event_listener;
  ba->subscribe_scope(req_bin_id, req_bin_prefix_id, IMPLICIT_RENDEZVOUS, NULL, 0);
  ba->subscribe_scope(lsn_bin_id, lsn_bin_prefix_id, IMPLICIT_RENDEZVOUS, NULL, 0);
  ba->subscribe_scope(resl_resp_bin_id, resl_resp_bin_prefix_id, IMPLICIT_RENDEZVOUS, NULL, 0);

  // Subscribe to the LSM scope to get updates
  ba->subscribe_scope(lsm_bin_scope, "", DOMAIN_LOCAL, NULL, 0);
  
  sleep(5);
  
    
  pthread_join(*event_listener, NULL);
  cout << "TM: disconnecting" << endl;

    
    ba->disconnect();
    delete ba;
    delete tm_igraph;
    cout << "TM: exiting" << endl;
    return 0;
}
