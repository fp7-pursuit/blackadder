/*
    Copyright (C) 2012 Martin J Reed              martin@reednet.org.uk
    University of Essex, Colchester, UK

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <stdlib.h>
#include "te_graph_mf.hpp"

extern int EF_ALLOW_MALLOC_0;

pthread_mutex_t te_mutex;
pthread_t _te_thread, *te_thread;

TEgraphMF::TEgraphMF() {
  igraph_i_set_attribute_table(&igraph_cattribute_table);
  cacheValid=false;
  e=0.1;
  recalculationDelay=60;
  defaultBW=1;
  te_thread= NULL;
}

void TEgraphMF::initialise(int Delay,double eval,double dBW){
  recalculationDelay = Delay;
  e=eval;
  defaultBW=dBW;

}

void* te_loop(void *arg);

TEgraphMF::~TEgraphMF() {
  if (te_thread)
    pthread_cancel(*te_thread);
  reverse_node_index.clear();
  reverse_edge_index.clear();
  vertex_iLID.clear();
  nodeID_iLID.clear();
  edge_LID.clear();
  igraph_i_attribute_destroy(&graph);
  igraph_destroy(&graph);
  if(cacheValid)
    delete(demandMapApplied);
}

int& TEgraphMF::get_reverse_node_index(string &nodeid){
  return (*reverse_node_index.find(nodeid)).second;
}
int TEgraphMF::readTopology(const char *file_name) {
  int ret = 0;
  Bitvector* lid;
  Bitvector* ilid;
  ifstream infile;
  string str;
  size_t found, first, second;
  FILE *instream;
  infile.open(file_name, ifstream::in);
  /*first the Global graph attributes - c igraph does not do it!!*/
  while (infile.good()) {
    getline(infile, str);
    found = str.find("<data key=\"FID_LEN\">");
    if (found != string::npos) {
      first = str.find(">");
      second = str.find("<", first);
      sscanf(str.substr(first + 1, second - first - 1).c_str(), "%d", &fid_len);
    }
    found = str.find("<data key=\"TM\">");
    if (found != string::npos) {
      first = str.find(">");
      second = str.find("<", first);
      nodeID = str.substr(first + 1, second - first - 1);
    }
    found = str.find("<data key=\"RV\">");
    if (found != string::npos) {
      first = str.find(">");
      second = str.find("<", first);
      RVnodeID = str.substr(first + 1, second - first - 1);
    }
    found = str.find("<data key=\"TM_MODE\">");
    if (found != string::npos) {
      first = str.find(">");
      second = str.find("<", first);
      mode = str.substr(first + 1, second - first - 1);
    }
  }
  infile.close();
  instream = fopen(file_name, "r");
  if (instream == NULL) {
    return -1;
  }
  //EF_ALLOW_MALLOC_0=1;
  ret = igraph_read_graph_graphml(&graph, instream, 0);
  //EF_ALLOW_MALLOC_0=0;

  fclose(instream);
  if (ret < 0) {
    return ret;
  }
  //cout << "TM: " << igraph_vcount(&graph) << " nodes" << endl;
  //cout << "TM: " << igraph_ecount(&graph) << " edges" << endl;
  for (int i = 0; i < igraph_vcount(&graph); i++) {
    string nID = string(igraph_cattribute_VAS(&graph, "NODEID", i));
    string iLID = string(igraph_cattribute_VAS(&graph, "iLID", i));
    reverse_node_index.insert(pair<string, int>(nID, i));
    ilid = new Bitvector(iLID);
    nodeID_iLID.insert(pair<string, Bitvector* >(nID, ilid));
    vertex_iLID.insert(pair<int, Bitvector* >(i, ilid));
    cout<<"node "<<i<<" has NODEID"<<nID<<endl;
    cout<<"node "<<i<<" has ILID"<<ilid->to_string()<<endl;
  }
  for (int i = 0; i < igraph_ecount(&graph); i++) {
    string LID = string(igraph_cattribute_EAS(&graph, "LID", i));
    reverse_edge_index.insert(pair<string, int>(LID, i));
    lid = new Bitvector(LID);
    edge_LID.insert(pair<int, Bitvector* >(i, lid));

    igraph_integer_t head;
    igraph_integer_t tail;
    igraph_edge(&graph, i,&head,&tail);
    cout << "edge " << i 
	 <<" "<<head<<"->"<<tail<<" has LID  " 
	 << lid->to_string() << endl;
  }

  std::vector<int> edgepairs;
  std::vector<double> capacities;
  igraph_eit_t ieit;
  igraph_eit_create(&graph,igraph_ess_all(IGRAPH_EDGEORDER_ID),&ieit);
  while(!IGRAPH_EIT_END(ieit)) {
    igraph_integer_t edgeid = IGRAPH_EIT_GET(ieit);
    igraph_integer_t head;
    igraph_integer_t tail;
    // WARNING all edge capacities are give the same value
    // this needs to come from deployment script
    capacities.push_back(defaultBW);
    igraph_edge(&graph, edgeid,&head,&tail);
    cout<<"edge"<<head<<"->"<<tail<<endl;
    edgepairs.push_back(head);
    edgepairs.push_back(tail);
    IGRAPH_EIT_NEXT(ieit);
  }
  igraph_eit_destroy(&ieit);

  // create an initial dmand matrix assuming equal traffic
  // between all node pairs - unlikely to be correct but
  // as booststrap we do not know any better
  // THIS WILL NEED TO BE DYNAMICALLY UPDATED LATER
  for (int i = 0; i < igraph_vcount(&graph); i++) {
    for (int j = 0; j < igraph_vcount(&graph); j++) {
      if( i == j) continue;
      mf_demand demand;
      demand.source = i;
      demand.sink = j;
      // WARNING HARDCODED VALUE, ok for initial boostrap
      // as it is all relative. It should be obtained from
      // the deployment script as an initial demand.
      demand.demand = 1.0;
      demandMapMeasured.insert(pair<intpair,mf_demand>(intpair(i,j),
						       demand));
			       
					  
    }
  }

  graphMF = Graph_mf((int)igraph_vcount(&graph),edgepairs,capacities);

  // now demands are set to half the maximum flow when 
  // using shortest paths assuming equal flow between
  // all pairs - enough for boostrapping
  // initial demand matrix done!
  //update_paths();


     
  preCalculateFids();
  return ret;
}

void* te_loop(void *arg) {
  TEgraphMF* graph=(TEgraphMF*)arg;
  while(true) {
    cout<<"recalculating"<<endl;
    //sleep(10);
    graph->update_paths();
    sleep(graph->recalculationDelay);

  }
  return NULL;
}
//resiliency function for use later when supporting resiliency inside TE 
void TEgraphMF::updateGraph(string &source, string &destination, string &net, bool update_op){
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
	cout<<"TM: Link Recovery: " << source << " - " << destination <<endl;
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
	    igraph_get_eids(&graph, &eids, &v,NULL, true, false);
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

void TEgraphMF::update_paths() {

  Bitvector* lid;
  // make deep copy of demandsMapMeasured
  vector<mf_demand> tmpdemands;
  te_mf_demandMap* tmpdemandMap = new te_mf_demandMap;

  pthread_mutex_lock (&te_mutex);

  cout<<"New demands measured\n";
  BOOST_FOREACH(demandMapMeasured_type::value_type &demandEntry,
		demandMapMeasured) {
    mf_demand &demand = demandEntry.second;
    cout<<demand.source<<"->"<<demand.sink<<"="<<demand.demand<<endl;

    mf_demand d;
    d.flow =0;
    d.source = demand.source;
    d.sink = demand.sink;
    // if we have had no demands in this last time period
    // set it to 1 request, else it will not create a path!
    if(demand.demand == 0) {
      d.demand = 1.0;
    } else{
      d.demand = demand.demand;
    }
    // do not cpy path_flow_map
    tmpdemands.push_back(d);
    demand.demand=0;
  }
  pthread_mutex_unlock (&te_mutex);

  // scale the demands as they represent number of calls
  // not actual bit rates
  graphMF.sp_concurrent_flow(tmpdemands);

  BOOST_FOREACH(mf_demand &demand,
		tmpdemands) {
    demand.demand = demand.demand * graphMF.lambda /2.0;
  }
  
  cout<<"Computing Min Congestion Flow"<<endl;
  graphMF.min_congestion_flow(tmpdemands,e);
  cout<<"Finished computing Min Congestion Flow"<<endl;
  std::vector<mf_demand>::iterator di;

  cout<<"Computed flows\n";
  // create the demandMap from the results of the min_congestion_flow
  for(di=tmpdemands.begin(); di != tmpdemands.end(); di++) {
    //cout<<"Demand "<<di->source<<"->"<<di->sink<<endl;
    te_mf_demand te_demand;
    te_demand.source = di->source;
    te_demand.sink = di->sink;
    te_demand.demand = di->demand;
    te_demand.flow = di->flow;
    std::vector<std::vector<Vertex> > paths;
    std::vector<double> flows;
    std::vector<double> probs;
    std::vector<Bitvector> fids;

    std::map<const std::list<Vertex>,double>::iterator mi;
    for(mi=(*di).path_flow_map.begin() ; 
	mi != (*di).path_flow_map.end(); mi++) {
      // first insert the sink internal LID
      Bitvector resultFID(FID_LEN * 8);
      Bitvector* ilid = 
	(*vertex_iLID.find(di->sink)).second;
      (resultFID) = (resultFID) | (*ilid);



      //cout<<"Assigning ilinkID "<<di->sink<<" "<<
      //ilid->to_string()<<endl;
      //cout<<endl;
      std::list<Vertex>::const_iterator vi=(mi->first).begin();
      int head=*vi;
      int tail;
      vi++;
      igraph_integer_t eid;

      std::vector<Vertex> path((mi->first).size());
      int i = 0;
      path[i]=head;
      i++;
      for(;vi != (mi->first).end(); vi++,i++) {
	tail=*vi;
	path[i]=tail;
#if IGRAPH_V >= IGRAPH_V_0_6
	igraph_get_eid(&graph, &eid,head,tail,true, false);
#else
	igraph_get_eid(&graph, &eid,head,tail,true);
#endif
	lid = (*edge_LID.find(eid)).second;
	(resultFID) = (resultFID) | (*lid);
	head=tail;
      }
      te_demand.paths.push_back(path);
      te_demand.flows.push_back(mi->second);
      te_demand.probs.push_back(mi->second / di->flow);
      te_demand.fids.push_back(resultFID);
      //cout<<"assigned num flows="<<te_demand.flows.size()<<endl;
      tmpdemandMap->insert_demand(te_demand);
    }
  }

  pthread_mutex_lock (&te_mutex);
  if(cacheValid) {
    delete demandMapApplied;
  }
  demandMapApplied = tmpdemandMap;
  pthread_mutex_unlock (&te_mutex);

  cacheValid=true;
  
  // THIS BLOCK ONLY FOR DEBUGGING DELETE
  
  demandMapApplied_type mymap = demandMapApplied->demand_map;
  BOOST_FOREACH(const demandMapApplied_type::value_type &te_demandm,
		mymap) {
    te_mf_demand te_demand = te_demandm.second;
    cout<<"Demand "<<te_demand.source+1<<"->"<<te_demand.sink+1
	<< " demand="<<te_demand.demand<<endl;
    for(unsigned int i=0;i<te_demand.paths.size();i++) {
      string spath;
      BOOST_FOREACH(Vertex &v,te_demand.paths[i]) {
	spath += "->" + boost::lexical_cast<string>(v);
      }
      cout<<" path "<<spath<<"="<<te_demand.flows[i]
	  << ", factor="<<te_demand.flows[i]/te_demand.flow<<endl;
    }
  }
  // END THIS BLOCK ONLY FOR DEBUGGING DELETE


}


Bitvector& te_mf_demandMap::get_fid(int source, int sink) {

  //select the FID probablistically from the set of paths
  // use inverse transfrom of CDF
  cout<<"calculateFID for "<<source<<"->"<<sink<<endl;
  double ran = drand48();
  double cumul = 0;
  if(demand_map.find(pair<int,int>(source,sink)) == 
     demand_map.end()) {
    cout<<"WARNING no demand_map entry found\n";

    Bitvector *resultFID = new Bitvector(FID_LEN * 8);
    return *resultFID;
  }
  te_mf_demand* te_demand = 
    &(demand_map.find(pair<int,int>(source,sink))->second);
  unsigned int i=0;
  for(; i<te_demand->flows.size() - 1; i++) {
    cumul += te_demand->probs[i];
    if (ran <= cumul) {
      break;
    }
  }

  cout<<"using path ";
  const std::vector<Vertex>& path = te_demand->paths[i];
  BOOST_FOREACH(const Vertex &v,
		path) {
    cout<<v<<",";
  }
  //cout<<"returning FID:"<<te_demand->fids[i].to_string()<<endl;
  cout<<endl;
  return te_demand->fids[i];
}
void te_mf_demandMap::insert_demand(te_mf_demand &demand) {
  demand_map[std::pair<int,int>(demand.source,demand.sink)]=demand;
}
  
void TEgraphMF::calculateFID(string &source, string &destination,
			     Bitvector &resultFID,
			     unsigned int &numberOfHops, string &path) {
  if(cacheValid) {
    pthread_mutex_lock (&te_mutex);
    resultFID = 
      demandMapApplied->get_fid(reverse_node_index.find(source)->second,
				reverse_node_index.find(destination)->second);
    pthread_mutex_unlock (&te_mutex);
    // this is from shortest hop, the path selected may be longer than this
    numberOfHops = distanceMap.find(pair<string,string>(source,destination))->
      second;
  } else {
    uncachedCalculateFID(source,destination,resultFID,numberOfHops);
  }
}
  
Bitvector* TEgraphMF::calculateFID(string &source, string &destination) {
  Bitvector* resultFID = new Bitvector(FID_LEN * 8);;
  if(cacheValid) {
    pthread_mutex_lock (&te_mutex);
    *resultFID = 
      demandMapApplied->get_fid(reverse_node_index.find(source)->second,
				reverse_node_index.find(destination)->second);
    pthread_mutex_unlock (&te_mutex);
    //*resultFID = fidMap.find(pair<string,string>(source,destination))->
    //	    second;
  } else {
    resultFID = uncachedCalculateFID(source,destination);
  }
  return(resultFID);
}
  
void TEgraphMF::calculateFID(set<string> &publishers, 
			     set<string> &subscribers, 
			     map<string, Bitvector* > &result, map<string, set<string> > &path_vectors) {
  set<string>::iterator subscribers_it;
  set<string>::iterator publishers_it;
  string bestPublisher;
  Bitvector resultFID(FID_LEN * 8);
  Bitvector bestFID(FID_LEN * 8);
  unsigned int numberOfHops = 0;
  //path variable will be used later when supporting resiliency insider TE, now it is empty
  string path;
  cout<<"MATCHING "<< publishers.size() << " publishers and " 
      << subscribers.size() << " subscibers\n";

  /*first add all publishers to the hashtable with NULL FID*/
  for (publishers_it = publishers.begin(); 
       publishers_it != publishers.end(); 
       publishers_it++) {
    string publ = *publishers_it;
    result.insert(pair<string, Bitvector* >(publ, NULL));
  }
  for (subscribers_it = subscribers.begin();
       subscribers_it != subscribers.end();
       subscribers_it++) {
    /*for all subscribers calculate the number of hops from all publishers (not very optimized...don't you think?)*/
    unsigned int minimumNumberOfHops = UINT_MAX;
    for (publishers_it = publishers.begin();
	 publishers_it != publishers.end();
	 publishers_it++) {
      resultFID.clear();
      string str1 = (*publishers_it);
      string str2 = (*subscribers_it);
      calculateFID(str1, str2, resultFID, numberOfHops, path);
      if (minimumNumberOfHops > numberOfHops) {
	minimumNumberOfHops = numberOfHops;
	bestPublisher = *publishers_it;
	bestFID = resultFID;
      }
    }

    cout << "best publisher " << bestPublisher 
	 << " for subscriber " << (*subscribers_it) 
	 << " -- number of hops " << minimumNumberOfHops - 1 << endl;
    int source = reverse_node_index.find(bestPublisher)->second;
    int sink = reverse_node_index.find(*subscribers_it)->second;
    pthread_mutex_lock(&te_mutex);

    mf_demand* demand = 
      &(demandMapMeasured.find(pair<int,int>(source,sink))->second);
    demand->demand++;

    pthread_mutex_unlock(&te_mutex);

    if ((*result.find(bestPublisher)).second == NULL) {
      /*add the publisher to the result*/
      //cout << "FID1: " << bestFID.to_string() << endl;
      result[bestPublisher] = new Bitvector(bestFID);
    } else {
      cout << "/*update the FID for the publisher*/" << endl;
      Bitvector* existingFID = (*result.find(bestPublisher)).second;
      /*or the result FID*/
      *existingFID = *existingFID | bestFID;
    }
  }

}

void TEgraphMF::preCalculateFids() {
  //pre-calculate the routing tables, fids and distances
  for (int i = 0; i < igraph_vcount(&graph); i++) {
    string iID = string(igraph_cattribute_VAS(&graph, "NODEID", i));
    for (int j = 0; j < igraph_vcount(&graph); j++) {
      Bitvector* pFID;
      Bitvector FID;
      unsigned int hops;
      string jID = string(igraph_cattribute_VAS(&graph, "NODEID", j));
      uncachedCalculateFID(iID, jID, FID, hops);
      cout<<"TM Calc = " << iID << "->" << jID << " hop count="<<hops<<endl;
      distanceMap.insert( pair< pair< string, string > ,int > 
			  (pair<string,string>(iID,jID), hops));
      pFID = new Bitvector(FID);
      fidMap.insert( pair< pair < string, string > , Bitvector* > 
		     (pair<string,string>(iID,jID), pFID));
    }
  }

}

Bitvector* TEgraphMF::uncachedCalculateFID(string &source, 
					   string &destination) {
  int vertex_id;
  Bitvector* result = new Bitvector(FID_LEN * 8);
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
  //NULL added by Mays to support Igraphv6
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
    igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true, false);
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
  cout<<"TM Calc:  " << source << "->" << destination 
      << " fid="<<result->to_string()<<endl;
  cout<<"TM stored:" << source << "->" << destination 
      << " fid="<<
    fidMap.find(pair<string,string>(source,destination))->
    second->to_string()<<endl;
    
  return result;
}

void TEgraphMF::uncachedCalculateFID(string &source, string &destination,
				     Bitvector &resultFID, 
				     unsigned int &numberOfHops) {
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
  //added by Mays to support igraph_v06
#if IGRAPH_V >= IGRAPH_V_0_6
  igraph_get_shortest_paths(&graph, &res, NULL, from, vs, IGRAPH_OUT);
#else
  igraph_get_shortest_paths(&graph, &res, from, vs, IGRAPH_OUT);
#endif
  /*check the shortest path to each destination*/
  temp_v = (igraph_vector_t *) VECTOR(res)[0];

  /*now let's "or" the FIDs for each link in the shortest path*/
  for (int j = 0; j < igraph_vector_size(temp_v) - 1; j++) {
#if IGRAPH_V >= IGRAPH_V_0_6
    igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true, false);
#else    
    igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true);
#endif
    Bitvector* lid = (*edge_LID.find(eid)).second;
    (resultFID) = (resultFID) | (*lid);
  }
  numberOfHops = igraph_vector_size(temp_v);

  /*now for the destination "or" the internal linkID*/
  Bitvector* ilid = (*nodeID_iLID.find(destination)).second;
  (resultFID) = (resultFID) | (*ilid);
  //cout << "FID of the shortest path: " << resultFID.to_string() << endl;
  igraph_vector_destroy((igraph_vector_t *) VECTOR(res)[0]);
  igraph_vector_destroy(&to_vector);
  igraph_vector_ptr_destroy_all(&res);
  igraph_vs_destroy(&vs);
    
}





