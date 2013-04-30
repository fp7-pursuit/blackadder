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

#include "tm_igraph.hpp"
extern int EF_ALLOW_MALLOC_0;
#include <sstream>


TMIgraph::TMIgraph() {
    igraph_i_set_attribute_table(&igraph_cattribute_table);
    //igraph_empty(&graph, 0, IGRAPH_DIRECTED); // No, read from file instead
}

TMIgraph::~TMIgraph() {
    map<string, Bitvector *>::iterator nodeID_iLID_iter;
    map<int, Bitvector *>::iterator edge_LID_iter;
    for (nodeID_iLID_iter = nodeID_iLID.begin(); nodeID_iLID_iter != nodeID_iLID.end(); nodeID_iLID_iter++) {
        delete (*nodeID_iLID_iter).second;
    }
    for (edge_LID_iter = edge_LID.begin(); edge_LID_iter != edge_LID.end(); edge_LID_iter++) {
        delete (*edge_LID_iter).second;
    }
    igraph_i_attribute_destroy(&graph);
    igraph_destroy(&graph);
}

int TMIgraph::readTopology(const char *file_name) {
    int ret;
    Bitvector *lid;
    Bitvector *ilid;
    ifstream infile;
    string str;
    size_t found_5, found_6, first, second;
    FILE *instream;
    infile.open(file_name, ifstream::in);
    if (infile.fail()) {
        return -1;
    }
    /*first the Global graph attributes - c igraph does not do it!!*/
    while (infile.good()) {
        getline(infile, str);
        found_5 = str.find("<data key=\"FID_LEN\">");
        found_6 = str.find("<data key=\"g_FID_LEN\">");
        if ((found_5 != string::npos) || ((found_6 != string::npos))) {
            first = str.find(">");
            second = str.find("<", first);
            sscanf(str.substr(first + 1, second - first - 1).c_str(), "%d", &fid_len);
        }
        found_5 = str.find("<data key=\"TM\">");
        found_6 = str.find("<data key=\"g_TM\">");
        if ((found_5 != string::npos) || ((found_6 != string::npos))) {
            first = str.find(">");
            second = str.find("<", first);
            nodeID = str.substr(first + 1, second - first - 1);
        }
        found_5 = str.find("<data key=\"RV\">");
        found_6 = str.find("<data key=\"g_RV\">");
        if ((found_5 != string::npos) || ((found_6 != string::npos))) {
            first = str.find(">");
            second = str.find("<", first);
            RVnodeID = str.substr(first + 1, second - first - 1);
        }
        found_5 = str.find("<data key=\"TM_MODE\">");
        found_6 = str.find("<data key=\"g_TM_MODE\">");
        if ((found_5 != string::npos) || ((found_6 != string::npos))) {
            first = str.find(">");
            second = str.find("<", first);
            mode = str.substr(first + 1, second - first - 1);
        }
    }
    infile.close();
    instream = fopen(file_name, "r");
    ret = igraph_read_graph_graphml(&graph, instream, 0);
    fclose(instream);
    if (ret < 0) {
        return ret;
    }
    cout << "TM: " << igraph_vcount(&graph) << " nodes" << endl;
    cout << "TM: " << igraph_ecount(&graph) << " edges" << endl;
    for (int i = 0; i < igraph_vcount(&graph); i++) {
        string nID = string(igraph_cattribute_VAS(&graph, "NODEID", i));
        string iLID = string(igraph_cattribute_VAS(&graph, "iLID", i));
        reverse_node_index.insert(pair<string, int>(nID, i));
        ilid = new Bitvector(iLID);
        nodeID_iLID.insert(pair<string, Bitvector *>(nID, ilid));
        vertex_iLID.insert(pair<int, Bitvector *>(i, ilid));
        //cout << "node " << i << " has NODEID " << nID << endl;
        //cout << "node " << i << " has ILID " << ilid->to_string() << endl;
    }
    for (int i = 0; i < igraph_ecount(&graph); i++) {
        string LID = string(igraph_cattribute_EAS(&graph, "LID", i));
        reverse_edge_index.insert(pair<string, int>(LID, i));
        lid = new Bitvector(LID);
        edge_LID.insert(pair<int, Bitvector *>(i, lid));
		igraph_integer_t head;
		igraph_integer_t tail;
		igraph_edge(&graph, i,&head,&tail);
        cout << "edge " << i 
	     <<" "<<head<<"->"<<tail<<" has LID  " 
	     << lid->to_string() << endl;
    }
    return ret;
}
int& TMIgraph::get_reverse_node_index(string &nodeid){
  return (*reverse_node_index.find(nodeid)).second;
}


void TMIgraph::updateLinkState(const string &lid, const QoSList &status){
  // Get the edge ID from the map
  igraph_integer_t eid = reverse_edge_index[lid];
  //cout<<"lid="<<lid<<" eid="<<eid<<endl;
  
  // Parse and store
  QoSList::const_iterator it = status.begin();
  for (; it!=status.end(); ++it){
    
    // Store on the edges (in case we pring/export the GraphML)
    stringstream ss;
    ss<<"QoS_"<<(int)it->first;
    //cout<<" Adding "<<ss.str().c_str()<<endl;
    SETEAN(&graph, ss.str().c_str(), eid, it->second);
  }
  
  
  // Update the QoSLinkWeightMap of the TM
  // 1. Check that this link level exists
  uint8_t lp = (status.find(QoS_PRIO))->second;
  QoSLinkWeightMap::iterator mit = qlwm.find(lp);
  if (mit==qlwm.end()) {
    createNewQoSLinkPrioMap(lp);
    //return; Do not return... we have to fix all the other qos weights also
  }
  
  // 2. If there, update the link stats
  // - map iterator, mit, points to the proper line...
  // - eid points to the edges id: ASSUMPION: the weight vector is indexed by edge id
  igraph_vector_t * vec = &qlwm[lp];
  // way 2: igraph_vector_t * vec = &mit->second;
  int vsize = igraph_vector_size(vec);
  if (eid >= vsize){
    cerr<<"ABORT eid >= vsize"<<endl;
    return;
  }
  
  // Update all the eid positions in the QoS map
  mit = qlwm.begin();
  for (; mit!=qlwm.end(); ++mit){
    // Set the vector pointer
    vec = &mit->second;
    if (mit->first < lp){
      // For any priority lower than this edge's... avoid using this edge
      igraph_vector_set(vec, eid, UCHAR_MAX);
    }
    else {
      // For all the others keep the real weight
      igraph_vector_set(vec, eid, (MAX_PRIO-lp));
    }
  }
  
}

void TMIgraph::createNewQoSLinkPrioMap(const uint8_t & map_prio){
  cout<<"Creating new priority plane: "<<(int)map_prio<<endl;
  // Create the line as a c array!
  int numedges=igraph_ecount(&graph);
  // cout<<"  # of Edges: "<<numedges<<endl;
  igraph_real_t * data = new igraph_real_t[numedges];
  
  map<int, Bitvector *>::iterator it;
  for (it = edge_LID.begin(); it != edge_LID.end(); ++it) {
    // Get edge ID
    int eid = it->first;
    
    // Get edge priority from the graph attr
    stringstream ss;
    ss<<"QoS_"<<(int)QoS_PRIO;
    uint8_t ep = EAN(&graph, ss.str().c_str(), eid);
    
    
    uint8_t weight = UCHAR_MAX;
    // If the current edge prio is the same of more that 
    // keep the exact weight
    if (map_prio >= ep) weight = MAX_PRIO-ep; // P=98 -> weight=2
     
    // Assign the weight
    data[eid] = weight;
    //cout<<"  - Edge id="<<eid<<" Prio="<<(int)ep<<" Weight="<<(int)weight<<endl;
  }
  
  // Port data to vector
  igraph_vector_t vec;
  igraph_vector_view(&vec, data, numedges);
  
  // Add plane to the map
  qlwm[map_prio]=vec;

}

void TMIgraph::updateGraph(string &source, string &destination, string &net, bool update_op){
  igraph_vector_t v;
  igraph_es_t es;
  igraph_vector_t eids;
  int src_v = (*reverse_node_index.find(source)).second;
  int dst_v = (*reverse_node_index.find(destination)).second;
  string pairs[2];
  pairs[0]=source + "/" + destination;
  pairs[1]=destination + "/" + source;
     if(net != "e"){
      igraph_vector_init(&v, 2);
      igraph_vector_init(&eids,1);
      VECTOR(v)[0]=src_v;
      VECTOR(v)[1]=dst_v;
    } else {
      igraph_vector_init(&v, 4);
      igraph_vector_init(&eids,2);
      VECTOR(v)[0]=src_v;
      VECTOR(v)[1]=dst_v;
      VECTOR(v)[2]=dst_v;
      VECTOR(v)[3]=src_v;
    }
    if(update_op){
      if(rmv_edge_lid.find(pairs[0]) != rmv_edge_lid.end()){
	igraph_add_edge(&graph, src_v, dst_v);
	Bitvector *lid=new Bitvector();
	lid = (*rmv_edge_lid.find(pairs[0])).second;
	igraph_cattribute_EAS_set(&graph, "LID", igraph_ecount(&graph) - 1, lid->to_string().c_str());
	rmv_edge_lid.erase(pairs[0]);
	if((net=="e")&&(rmv_edge_lid.find(pairs[1]) != rmv_edge_lid.end())){
	  igraph_add_edge(&graph,dst_v , src_v);
	  Bitvector *lid=new Bitvector();
	  lid = (*rmv_edge_lid.find(pairs[1])).second;
	  igraph_cattribute_EAS_set(&graph, "LID", igraph_ecount(&graph) - 1, lid->to_string().c_str());
	  rmv_edge_lid.erase(pairs[1]);
	}
      }
    }else {
      if(net =="e"){
      if ((rmv_edge_lid.find(pairs[0]) != rmv_edge_lid.end()) || (rmv_edge_lid.find(pairs[1]) != rmv_edge_lid.end())){
	cout << "TM: Reported Edges are alreday disconnected" << endl;
      } else {
	igraph_es_pairs(&es, &v, true);
#if IGRAPH_V >= IGRAPH_V_0_6
	igraph_get_eids(&graph, &eids, &v, NULL, true, false);
# else 
	igraph_get_eids(&graph, &eids, &v, true);
#endif
	for (int j = 0; j <= igraph_vector_size(&eids) - 1; j++) {
	  Bitvector *lid = (*edge_LID.find(VECTOR(eids)[j])).second;
	  rmv_edge_lid.insert(pair<string, Bitvector *>(pairs[j],lid));
	}
	igraph_delete_edges(&graph, es);
      }
      } else if (net=="o"){
	if(rmv_edge_lid.find(pairs[0]) != rmv_edge_lid.end()){
	  cout << "TM: Reproted Edge is already disconnected" << endl;
	} else {
	    igraph_es_pairs(&es, &v, true);
#if IGRAPH_V >= IGRAPH_V_0_6
	    igraph_get_eids(&graph, &eids, &v, NULL, true, false);
# else 
	    igraph_get_eids(&graph, &eids, &v, true);
#endif
	    for (int j = 0; j <= igraph_vector_size(&eids) - 1; j++) {
	      Bitvector *lid = (*edge_LID.find(VECTOR(eids)[j])).second;
	      rmv_edge_lid.insert(pair<string, Bitvector *>(pairs[j],lid));
	    }
	    igraph_delete_edges(&graph, es);
	  }
	}
      }
    //mjreed MEMORY LEAK HERE
    reverse_edge_index.clear();
    edge_LID.clear();
    for (int i = 0; i < igraph_ecount(&graph); i++) {
      string LID = string(igraph_cattribute_EAS(&graph, "LID", i));
      reverse_edge_index.insert(pair<string, int>(LID, i));
      Bitvector* lid = new Bitvector(LID);
      edge_LID.insert(pair<int, Bitvector *>(i, lid));
      //cout << "edge " << i << " has LID  " << lid->to_string() << endl;
    }
}
Bitvector *TMIgraph::calculateFID(string &source, string &destination) {
    int vertex_id;
    Bitvector *result = new Bitvector(FID_LEN * 8);
    igraph_vs_t vs;
    igraph_vector_ptr_t res;
    igraph_vector_t to_vector;
    igraph_vector_t *temp_v;
    igraph_integer_t eid;

    /*find the vertex id in the reverse index*/
    int from = (*reverse_node_index.find(source)).second;
    igraph_vector_init(&to_vector, 1);
    VECTOR(to_vector)[0] = (*reverse_node_index.find(destination)).second;
    /*initialize the sequence*/
    igraph_vs_vector(&vs, &to_vector);
    /*initialize the vector that contains pointers*/
    igraph_vector_ptr_init(&res, 1);
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    temp_v = (igraph_vector_t *) malloc(sizeof (igraph_vector_t));
    VECTOR(res)[0] = temp_v;
    igraph_vector_init(temp_v, 1);
    /*run the shortest path algorithm from "from"*/
#if IGRAPH_V >= IGRAPH_V_0_6
    igraph_get_shortest_paths(&graph, &res, NULL, from, vs, IGRAPH_OUT);
#else
    igraph_get_shortest_paths(&graph, &res, from, vs, IGRAPH_OUT);
#endif
    /*check the shortest path to each destination*/
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    //click_chatter("Shortest path from %s to %s", igraph_cattribute_VAS(&graph, "NODEID", from), igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[igraph_vector_size(temp_v) - 1]));
    /*now let's "or" the FIDs for each link in the shortest path*/
    for (int j = 0; j < igraph_vector_size(temp_v) - 1; j++) {
#if IGRAPH_V >= IGRAPH_V_0_6
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true, true);
#else
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true);
#endif
        //click_chatter("node %s -> node %s", igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[j]), igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[j + 1]));
        //click_chatter("link: %s", igraph_cattribute_EAS(&graph, "LID", eid));
        string LID(igraph_cattribute_EAS(&graph, "LID", eid), FID_LEN * 8);
        for (int k = 0; k < FID_LEN * 8; k++) {
            if (LID[k] == '1') {
                (*result)[ FID_LEN * 8 - k - 1].operator |=(true);
            }
        }
        //click_chatter("FID of the shortest path: %s", result.to_string().c_str());
    }
    /*now for all destinations "or" the internal linkID*/
    vertex_id = (*reverse_node_index.find(destination)).second;
    string iLID(igraph_cattribute_VAS(&graph, "iLID", vertex_id));
    //click_chatter("internal link for node %s: %s", igraph_cattribute_VAS(&graph, "NODEID", vertex_id), iLID.c_str());
    for (int k = 0; k < FID_LEN * 8; k++) {
        if (iLID[k] == '1') {
            (*result)[ FID_LEN * 8 - k - 1].operator |=(true);
        }
    }
    igraph_vector_destroy((igraph_vector_t *) VECTOR(res)[0]);
    igraph_vector_destroy(&to_vector);
    igraph_vector_ptr_destroy_all(&res);
    igraph_vs_destroy(&vs);
    return result;
}

/*main function for rendezvous*/
void TMIgraph::calculateFID(set<string> &publishers, set<string> &subscribers, map<string, Bitvector *> &result, map<string, set<string> > &path_vectors) {
    set<string>::iterator subscribers_it;
    set<string>::iterator publishers_it;
    string bestPublisher;
    Bitvector resultFID(FID_LEN * 8);
    Bitvector bestFID(FID_LEN * 8);
    unsigned int numberOfHops = 0;
    set<string> paths_per_pub;
    string best_path;
    /*first add all publishers to the hashtable with NULL FID*/
    for (publishers_it = publishers.begin(); publishers_it != publishers.end(); publishers_it++) {
        string publ = *publishers_it;
        result.insert(pair<string, Bitvector *>(publ, NULL));
	path_vectors.insert(pair<string, set<string> >(publ, paths_per_pub));
    }
    for (subscribers_it = subscribers.begin(); subscribers_it != subscribers.end(); subscribers_it++) {
        /*for all subscribers calculate the number of hops from all publishers (not very optimized...don't you think?)*/
        unsigned int minimumNumberOfHops = UINT_MAX;
        for (publishers_it = publishers.begin(); publishers_it != publishers.end(); publishers_it++) {
	    string curr_path;
            resultFID.clear();
            string str1 = (*publishers_it);
            string str2 = (*subscribers_it);
	    calculateFID(str1, str2, resultFID, numberOfHops, curr_path);
            if (minimumNumberOfHops > numberOfHops) {
		minimumNumberOfHops = numberOfHops;
		bestPublisher = *publishers_it;
		bestFID = resultFID;
		if(extension[RESI])		//resiliency
		  best_path = curr_path;
	    }
        }
        /*When the resiliency support is in effect, return the set of path vectors for each publisher*/
	if(extension[RESI]){
	  cout<<"Best path is: "<<best_path<<endl;
	  if(!(path_vectors.empty())){
	    for(map<string, set<string> >::iterator b_b_it = path_vectors.begin(); b_b_it != path_vectors.end(); b_b_it++){
	      if ((*b_b_it).first == bestPublisher){
		b_b_it->second.insert(best_path);
	      }
	    }
	  } else{
	    set<string> add_b_path;
	    add_b_path.insert(best_path);
	    path_vectors.insert(pair<string, set<string> >(bestPublisher, add_b_path));
	  }
	}
        //cout << "best publisher " << bestPublisher << " for subscriber " << (*subscribers_it) << " -- number of hops " << minimumNumberOfHops - 1 << endl;
        if ((*result.find(bestPublisher)).second == NULL || minimumNumberOfHops == UINT_MAX) {
            /*add the publisher to the result*/
            //cout << "FID1: " << bestFID.to_string() << endl;
            result[bestPublisher] = new Bitvector(bestFID);
        } else {
            //cout << "/*update the FID for the publisher*/" << endl;
            Bitvector *existingFID = (*result.find(bestPublisher)).second;
            /*or the result FID*/
            *existingFID = *existingFID | bestFID;
        }
    }
}

// Copy/Pasting to avoid conflicts (FIXME: Messy)
void TMIgraph::calculateFID_weighted(set<string> &publishers, set<string> &subscribers, map<string, Bitvector *> &result, map<string, set<string> > &path_vectors, const igraph_vector_t *weights) {
    set<string>::iterator subscribers_it;
    set<string>::iterator publishers_it;
    string bestPublisher;
    Bitvector resultFID(FID_LEN * 8);
    Bitvector bestFID(FID_LEN * 8);
    unsigned int numberOfHops = 0;
    string curr_path="";
    set<string> paths_per_pub;
    string best_path;
    /*first add all publishers to the hashtable with NULL FID*/
    for (publishers_it = publishers.begin(); publishers_it != publishers.end(); publishers_it++) {
        string publ = *publishers_it;
        result.insert(pair<string, Bitvector *>(publ, NULL));
	path_vectors.insert(pair<string, set<string> >(publ, paths_per_pub));
    }
    for (subscribers_it = subscribers.begin(); subscribers_it != subscribers.end(); subscribers_it++) {
        /*for all subscribers calculate the number of hops from all publishers (not very optimized...don't you think?)*/
        unsigned int minimumNumberOfHops = UINT_MAX;
        for (publishers_it = publishers.begin(); publishers_it != publishers.end(); publishers_it++) {
            resultFID.clear();
            string str1 = (*publishers_it);
            string str2 = (*subscribers_it);
	    calculateFID_weighted(str1, str2, resultFID, numberOfHops, curr_path, weights);

 	    cout<<"Testing distance from "<<str1<<"->"<<str2<< "="<<numberOfHops<<endl;
            // Urban: BUG? local pub/sub didn't work without the || part bellow
            //  || numberOfHops==UINT_MAX
            if (minimumNumberOfHops > numberOfHops) {
                minimumNumberOfHops = numberOfHops;
                bestPublisher = *publishers_it;
                bestFID = resultFID;
		best_path=curr_path;
	    }
	    curr_path="";
        }
        
	cout<<"Best path is: "<<best_path<<endl;
	if(!(path_vectors.empty())){
	  for(map<string, set<string> >::iterator b_b_it = path_vectors.begin(); b_b_it != path_vectors.end(); b_b_it++){
	    if ((*b_b_it).first == bestPublisher){
	      b_b_it->second.insert(best_path);
	    }
	  }
	} else{
	  set<string> add_b_path;
	  add_b_path.insert(best_path);
	  path_vectors.insert(pair<string, set<string> >(bestPublisher, add_b_path));
	}
        //cout << "best publisher " << bestPublisher << " for subscriber " << (*subscribers_it) << " -- number of hops " << minimumNumberOfHops - 1 << endl;
        if ((*result.find(bestPublisher)).second == NULL || minimumNumberOfHops == UINT_MAX) {
            /*add the publisher to the result*/
            //cout << "FID1: " << bestFID.to_string() << endl;
            result[bestPublisher] = new Bitvector(bestFID);
        } else {
            //cout << "/*update the FID for the publisher*/" << endl;
            Bitvector *existingFID = (*result.find(bestPublisher)).second;
            /*or the result FID*/
            *existingFID = *existingFID | bestFID;
        }
    }
}

void TMIgraph::calculateFID_weighted(string &source, string &destination, Bitvector &resultFID, unsigned int &numberOfHops, string &path, const igraph_vector_t *weights) {
    igraph_vs_t vs;
    igraph_vector_ptr_t res;
#if IGRAPH_V >= IGRAPH_V_0_6
    igraph_vector_ptr_t edges;
#endif
    igraph_vector_t to_vector;
    igraph_vector_t *temp_v;
    igraph_integer_t eid;
    stringstream ss_curr_path;
   /*find the vertex id in the reverse index*/
    int from = (*reverse_node_index.find(source)).second;
    igraph_vector_init(&to_vector, 1);
    VECTOR(to_vector)[0] = (*reverse_node_index.find(destination)).second;
    /*initialize the sequence*/
    igraph_vs_vector(&vs, &to_vector);
    /*initialize the vector that contains pointers*/
    igraph_vector_ptr_init(&res, 1);
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    temp_v = (igraph_vector_t *) malloc(sizeof (igraph_vector_t));
    VECTOR(res)[0] = temp_v;
    igraph_vector_init(temp_v, 1);
    /*run the shortest path algorithm from "from"*/
#if IGRAPH_V >= IGRAPH_V_0_6
    cout<<"v0.6 Init"<<endl;
    // Init a vector ptr
    igraph_vector_ptr_init(&edges, 1);
    VECTOR(edges)[0]=calloc(1, sizeof(igraph_vector_t));
    igraph_vector_init((igraph_vector_t*)VECTOR(edges)[0],1);
    cout<<"v0.6 Calculation..."<<endl;
    int rc=igraph_get_shortest_paths_dijkstra(&graph, &res, &edges, from, vs, weights, IGRAPH_OUT);
    cout<<"rc="<<rc<<endl;
#else
    igraph_get_shortest_paths_dijkstra(&graph, &res, from, vs, weights, IGRAPH_OUT);
#endif
    /*construct a string of the current path represented by nodes*/
    for (int j = 0; j <= igraph_vector_size(temp_v)-1; j++) {
      ss_curr_path << VECTOR(*temp_v)[j];
      path+=ss_curr_path.str();
      path+="/";
      ss_curr_path.str("");
    }
    /*check the shortest path to each destination*/
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    /*now let's "or" the FIDs for each link in the shortest path*/
    
#if IGRAPH_V >= IGRAPH_V_0_6
  cout<<"Constructing FID based on edges..."<<endl;
  igraph_vector_t *temp_ev = (igraph_vector_t *)VECTOR(edges)[0];
  for (int j = 0; j < igraph_vector_size(temp_ev); j++) {
        eid = VECTOR(*temp_ev)[j];
#else
  for (int j = 0; j < igraph_vector_size(temp_v) - 1; j++) {
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true);
#endif
        Bitvector *lid = (*edge_LID.find(eid)).second;
        (resultFID) = (resultFID) | (*lid);
    }
    
#if IGRAPH_V >= IGRAPH_V_0_6
    numberOfHops = igraph_vector_size(temp_ev);
#else
    numberOfHops = igraph_vector_size(temp_v);
#endif
    
   if(numberOfHops == 0) numberOfHops=UINT_MAX;
   // Fix: Cause it seems that edges without weight are getting ignored from dijkstra
   // and these are the iLIDs
   if (source == destination) numberOfHops=1;
   
    /*now for the destination "or" the internal linkID*/
    Bitvector *ilid = (*nodeID_iLID.find(destination)).second;
    (resultFID) = (resultFID) | (*ilid);
    //cout << "FID of the shortest path: " << resultFID.to_string() << endl;
    igraph_vector_destroy((igraph_vector_t *) VECTOR(res)[0]);
//     igraph_vector_destroy((igraph_vector_t *) VECTOR(edge)[0]);
    igraph_vector_destroy(&to_vector);
    igraph_vector_ptr_destroy_all(&res);
    igraph_vs_destroy(&vs);
}

void TMIgraph::calculateFID(string &source, string &destination, Bitvector &resultFID, unsigned int &numberOfHops, string &path) {
    igraph_vs_t vs;
    igraph_vector_ptr_t res;
    igraph_vector_t to_vector;
    igraph_vector_t *temp_v;
    igraph_integer_t eid;

    /*find the vertex id in the reverse index*/
    int from = (*reverse_node_index.find(source)).second;
    igraph_vector_init(&to_vector, 1);
    VECTOR(to_vector)[0] = (*reverse_node_index.find(destination)).second;
    /*initialize the sequence*/
    igraph_vs_vector(&vs, &to_vector);
    /*initialize the vector that contains pointers*/
    igraph_vector_ptr_init(&res, 1);
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    temp_v = (igraph_vector_t *) malloc(sizeof (igraph_vector_t));
    VECTOR(res)[0] = temp_v;
    igraph_vector_init(temp_v, 1);
    /*run the shortest path algorithm from "from"*/
#if IGRAPH_V >= IGRAPH_V_0_6
    igraph_get_shortest_paths(&graph, &res, NULL, from, vs, IGRAPH_OUT);
#else
    igraph_get_shortest_paths(&graph, &res, from, vs, IGRAPH_OUT);
#endif
    /*"When the Resiliency mode is in effect, construct a string of the current path represented by nodes*/
//     if(support.compare("resiliency") == 0){
    if(extension[RESI]){
      stringstream ss_curr_path;
      for (int j = 0; j <= igraph_vector_size(temp_v)-1; j++) {
	ss_curr_path << VECTOR(*temp_v)[j];
	path+=ss_curr_path.str();
	path+="/";
	ss_curr_path.str("");
      }
    }
    /*check the shortest path to each destination*/
    temp_v = (igraph_vector_t *) VECTOR(res)[0];

    /*now let's "or" the FIDs for each link in the shortest path*/
    for (int j = 0; j < igraph_vector_size(temp_v) - 1; j++) {
#if IGRAPH_V >= IGRAPH_V_0_6
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true, true);
#else
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true);
#endif
        Bitvector *lid = (*edge_LID.find(eid)).second;
        (resultFID) = (resultFID) | (*lid);
    }
    numberOfHops = igraph_vector_size(temp_v);
    if(numberOfHops == 0)
      numberOfHops=UINT_MAX;
    /*now for the destination "or" the internal linkID*/
    Bitvector *ilid = (*nodeID_iLID.find(destination)).second;
    (resultFID) = (resultFID) | (*ilid);
    //cout << "FID of the shortest path: " << resultFID.to_string() << endl;
    igraph_vector_destroy((igraph_vector_t *) VECTOR(res)[0]);
    igraph_vector_destroy(&to_vector);
    igraph_vector_ptr_destroy_all(&res);
    igraph_vs_destroy(&vs);
}
