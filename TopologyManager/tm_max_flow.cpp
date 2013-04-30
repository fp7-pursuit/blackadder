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

#include "tm_max_flow.hpp" // XXX: Include this first
#include <float.h>
#include <boost/graph/copy.hpp>
using namespace boost;

inline double Graph_mf::calcD() {
  using namespace boost;
  double sum =0.0;
  graph_traits < NetGraph >::edge_iterator ei, eend;
  for(tie(ei,eend) = edges(gdual); ei != eend; ei++) {
    double l = get(edge_weight,gdual,*ei);
    double c = get(edge_capacity,gdual,*ei);
    sum += l*c;
  }
  return sum;
}

double Graph_mf::assign_gflow(std::vector<mf_demand> &demands) {
  graph_traits < NetGraph >::edge_iterator ei, eend;
  for(tie(ei,eend) = edges(gflow); ei != eend; ei++) {
    put(edge_weight,gflow,*ei,0.0);
  }
  std::vector<mf_demand>::iterator di;

  for(di=demands.begin(); di != demands.end(); di++) {
    std::map<const std::list<Vertex>,double>::iterator mi;
    for(mi=(*di).path_flow_map.begin() ; mi != (*di).path_flow_map.end(); mi++) {
      std::list<Vertex>::const_iterator lh=(mi->first).begin();
      std::list<Vertex>::const_iterator lt=(mi->first).begin();
      lh++;
      for(;lh != mi->first.end(); lh++, lt++) {
	// remember flow paths are stored in reverse order
	std::pair<Edge, bool> ed = edge(*lh,*lt,gflow);
	double w =get(edge_weight,gflow,ed.first);
	put(edge_weight,gflow,ed.first,w+mi->second);
      }
    }
  }
  gamma = DBL_MAX;
  for(tie(ei,eend) = edges(gflow); ei != eend; ei++) {
    double w = get(edge_weight,gflow,*ei);
    double c = get(edge_capacity,gflow,*ei);
    gamma = (c-w)/c < gamma ? (c-w)/c : gamma;
  }
  return(gamma);
}

inline double Graph_mf::calcLambda(std::vector<mf_demand> &demands) {
  lambda = DBL_MAX;
  gamma = 0;
  std::vector<mf_demand>::iterator di;
  for(di=demands.begin(); di != demands.end(); di++) {
    lambda = di->flow / di->demand < lambda ? di->flow / di->demand : lambda;
  }
  gamma= 1.0 - 1.0/lambda;
  return(lambda);
}
void Graph_mf:: sp_concurrent_flow(std::vector<mf_demand> &demands) {

  std::vector<mf_demand>::iterator di;
  for(di=demands.begin(); di != demands.end(); di++) {
    di->flow = 0;
    di->path_flow_map.erase(di->path_flow_map.begin(),di->path_flow_map.end());
  }

  int num_dem = demands.size();
  gflow.clear();
  boost::copy_graph((NetGraph)*this,gflow);
  gdual.clear();
  boost::copy_graph((NetGraph)*this,gdual);
  for(int i=0 ; i<num_dem; i++) {
    demands[i].flow = 0;
  }
  int N = num_vertices(*this);
  number_flows = 0;

  graph_traits < NetGraph >::edge_iterator ei, eend;
  // assign unit weigths to all edges
  for(tie(ei,eend) = edges(gdual); ei != eend; ei++) {
    int s = source(*ei,gdual);
    int t = target(*ei,gdual);
    std::pair<Edge, bool> e = edge(vertex(s,gdual),
				   vertex(t,gdual),gdual);
    put(edge_weight,gdual,e.first,1.0);
    e = edge(vertex(s,gflow),
	     vertex(t,gflow),gflow);
    put(edge_weight,gflow,e.first,0.0);
  }
  // for each demand
  for(int i=0; i<num_dem;i++) {
    mf_demand demand = demands[i];

    std::vector<Vertex> penult(N);
    std::vector<double> dist(N);
    std::vector<mf_demand>::iterator vi,ve;
    Vertex source = demand.source;
    Vertex sink = demand.sink;
    Vertex f,p;

    // calculated shortest path
    dijkstra_shortest_paths(gdual, source,
			    predecessor_map(&penult[0]).distance_map(&dist[0]));
    // record the path (in reverse as its easier from penult)
    // and add to the weight (flow) in gflow
    std::list<Vertex> path;

    f = sink;
    p = penult[f];

    std::pair<Edge, bool> ed = edge(p,f,gdual);

    path.push_front(f);
    path.push_front(p);
    f = p;
    p = penult[p];
    while(f != source) {
      ed = edge(p,f,gflow);
      double w =get(edge_weight,gflow,ed.first);
      put(edge_weight,gflow,ed.first,w+demand.demand);
      path.push_front(p);
      f = p;
      p = penult[p];
    }
    
    demands[i].path_flow_map[path] = demand.demand;
    demands[i].flow = demand.demand;
  }

  // calculate lambda
  lambda = DBL_MAX;
  gamma = 0;
  for(tie(ei,eend) = edges(gflow); ei != eend; ei++) {
    double w = get(edge_weight,gflow,*ei);
    double c = get(edge_capacity,gflow,*ei);
    lambda = c/w < lambda ? c/w : lambda;
  }
  gamma= 1.0 - 1.0/lambda;
}

double Graph_mf::calcBeta(std::vector<mf_demand> &demands) {
  double Alpha=0;
  int N = num_vertices(*this);

  std::vector<mf_demand>::iterator di;
  std::vector<Vertex> penult(N);
  std::vector<double> dist(N);

  for(di=demands.begin(); di != demands.end(); di++) {
    dijkstra_shortest_paths(gdual, di->source,
			    predecessor_map(&penult[0]).distance_map(&dist[0]));
    Alpha += (*di).demand * dist[di->sink];
  }
  double D= calcD();
  return(D/Alpha);
}


void Graph_mf::rescale_demands(std::vector<mf_demand> &demands,double scale) {
  std::vector<mf_demand>::iterator di;
  std::map<const std::list<Vertex>,double>::iterator mi;
  for(di=demands.begin(); di != demands.end(); di++) {
    (*di).demand *= scale;
  }
}

void Graph_mf::rescale_demands_flows(std::vector<mf_demand> &demands,double scale) {

  std::vector<mf_demand>::iterator di;
  std::map<const std::list<Vertex>,double>::iterator mi;
  for(di=demands.begin(); di != demands.end(); di++) {
    (*di).flow *= scale;
    for(mi=(*di).path_flow_map.begin() ; mi != (*di).path_flow_map.end(); mi++) {
      (*mi).second *= scale;
    }
  }
}

void Graph_mf:: max_concurrent_flow_prescaled(std::vector<mf_demand> &demands,
					      double e) {

  std::vector<mf_demand> save_demands = demands;
  long num_dem=demands.size();
  sp_concurrent_flow(demands);
  rescale_demands(demands,lambda);
  // Beta might be very high so obtain 2-approximate solution
  // (1+w) = 2 = (1-e)^-3
  double e2= 1- pow(2.0,-1.0/3.0);
  max_concurrent_flow(demands,e2);
  rescale_demands(demands,beta/2);

  max_concurrent_flow(demands,e);

  for(int i=0; i<num_dem; i++) {
    demands[i].demand = save_demands[i].demand;
  }
  lambda = calcLambda(demands);
  beta = calcBeta(demands);
  assign_gflow(demands);
}

void Graph_mf:: min_congestion_flow(std::vector<mf_demand> &demands,
				     double e) {
  max_concurrent_flow_prescaled(demands,e);
  rescale_demands_flows(demands,1/lambda);
  assign_gflow(demands);
}

void Graph_mf::  max_concurrent_flow(std::vector<mf_demand> &demands,
				     double e) {

  std::vector<mf_demand>::iterator di;
  for(di=demands.begin(); di != demands.end(); di++) {
    di->flow = 0;
    di->path_flow_map.erase(di->path_flow_map.begin(),di->path_flow_map.end());
  }

  int num_dem = demands.size();
  gflow.clear();
  boost::copy_graph((NetGraph)*this,gflow);
  gdual.clear();
  boost::copy_graph((NetGraph)*this,gdual);
  for(int i=0 ; i<num_dem; i++) {
    demands[i].flow = 0;
  }
  int N = num_vertices(*this);
  int m = num_edges(*this);
  number_flows = 0;
  double delta = pow(double(m) / (1.0 - e),-1.0/e);

  graph_traits < NetGraph >::edge_iterator ei, eend;

  for(tie(ei,eend) = edges(gdual); ei != eend; ei++) {
    int s = source(*ei,gdual);
    int t = target(*ei,gdual);
    std::pair<Edge, bool> e = edge(vertex(s,gdual),
				   vertex(t,gdual),gdual);
    double c = get(edge_capacity,gdual,e.first);
    put(edge_weight,gdual,e.first,delta/c);
  }
	

  double D;
  D=calcD();
	
  // assuming doubreq is about the maximum number of phases
  // then we want to only update the progress bar every 1%
  //int doubreq =  2*int(ceil(1.0/e * log(m/(1-e))/log(1+e)));
  //int updatepb = int(ceil(doubreq / 100.0));
  int phases =0;
  totalphases =0;
  
  std::vector<Vertex> penult(N);
  std::vector<double> dist(N);
  std::vector<mf_demand>::iterator vi,ve;


  std::vector<int> demand_index(num_dem);
  for(int i=0; i<num_dem; i++) {
    demand_index[i]=i;
  }

  // phases
  while(D < 1.0) {
    // this doubling could be included but it needs "undoubling" for
    // demands and flows at the end if prescaling is done this should
    // not be necessary
    /*
    if(phases > doubreq) {
      for(vi=demands.begin(); vi < demands.end(); vi++) {
	vi->demand = vi->demand * 2;
      }
      phases = 0;
      }*/
    // steps
    random_shuffle(demand_index.begin(),demand_index.end());
	  
    for(int j=0; j<num_dem;j++) {
      int i= demand_index[j];
      mf_demand demand=demands[i];
      Vertex source = demand.source;
      Vertex sink = demand.sink;
      Vertex f,p;

      //iterations
      while( D < 1.0 && demand.demand > 0) {
	dijkstra_shortest_paths(gdual, source,
				predecessor_map(&penult[0]).distance_map(&dist[0]));

	// go through the path (backwards) and find minimum capacity
	f = sink;
	p = penult[f];
	std::pair<Edge, bool> ed = edge(p,f,gdual);

	double mincap=get(edge_capacity,gdual,ed.first);
	f = p;
	p = penult[p];
	double w =get(edge_weight,gdual,ed.first);
		  
	while(f != source) {
	  ed = edge(p,f,gdual);
	  double cap =get(edge_capacity,gdual,ed.first);
	  mincap = cap < mincap? cap : mincap;
	  f = p;
	  p = penult[p];
	}

	// now we have the maximum flow we can push through this
	// step, and update demand (will add flow later)
	mincap = demand.demand < mincap ? demand.demand : mincap;
	demand.demand = demand.demand - mincap;

	// update each edge length = length (1 + (e*mincap) / capacity_e)
	// again go though the path backwards
	f = sink;
	p = penult[f];
	ed = edge(p,f,gdual);
		  
	double c=get(edge_capacity,gdual,ed.first);
	w =get(edge_weight,gdual,ed.first);
	w = w * (1 + (e * mincap) / c);
	put(edge_weight,gdual,ed.first,w);

	// note this records the path backwards! (slightly faster
	// when creating the path vector so no real reason to change
	std::list<Vertex> path;
	path.push_front(f);
	path.push_front(p);

	f = p;
	p = penult[p];

				 
	while(f != source) {
	  ed = edge(p,f,gdual);
	  c =get(edge_capacity,gdual,ed.first);
	  w =get(edge_weight,gdual,ed.first);
	  w = w * (1 + (e * mincap) / c);
	  put(edge_weight,gdual,ed.first,w);
	  path.push_front(p);
	  f = p;
	  p = penult[p];
	}
	
	if (demands[i].path_flow_map.find(path) == 
	    demands[i].path_flow_map.end()) {
	  demands[i].path_flow_map[path] = mincap;
	  number_flows++;
	} else {
	  demands[i].path_flow_map[path] += mincap;
	}


	demands[i].flow += mincap;
	D=calcD();
      }
    }
    phases++;
    totalphases++;
  }
  
  double scalef = 1.0 / (log(1.0/delta) / log(1+e) );
  rescale_demands_flows(demands,scalef);
  lambda = calcLambda(demands);
  beta = calcBeta(demands);
  assign_gflow(demands);
  
}
