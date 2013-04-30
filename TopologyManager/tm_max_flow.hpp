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

#ifndef MAX_FLOW_HPP
#define MAX_FLOW_HPP
#include <vector>
#include <list>
#include <utility>

// This required to stop warning about depcricated header in Boost Graph library
#define BOOST_NO_HASH

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

class mf_demand;

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
			      boost::property<boost::vertex_color_t, 
					      boost::default_color_type>,
			      boost::property<boost::edge_weight_t, double,
					      boost::property<boost::edge_capacity_t, 
							      double > > > NetGraph;

class Graph_mf: public NetGraph {

public:
  typedef NetGraph::graph_property_type graph_property_type;
  typedef NetGraph::vertices_size_type vertices_size_type;
  typedef NetGraph::edges_size_type edges_size_type;
  
  long number_flows;
   Graph_mf()
    : NetGraph() { }
   Graph_mf(const graph_property_type& p)
  : NetGraph(p) { }
   Graph_mf(const NetGraph& x)
  : NetGraph(x) { }
   Graph_mf(vertices_size_type num_vertices)
  : NetGraph(num_vertices) { }
   Graph_mf(vertices_size_type num_vertices,
		  const graph_property_type& p)
    : NetGraph(num_vertices, p) { }

   Graph_mf(int num_vertices,
		  const std::vector< int > &edges,
		  const std::vector<double> &capacities)
    : NetGraph(num_vertices)
  {
    int NE = edges.size()/2;
    for (int i = 0, j=0; i < NE ;  i++, j+=2 ) {
      std::pair<edge_descriptor, bool> e =
	boost::add_edge(edges[j], edges[j+1],
			0.0, *this);
      boost::put(boost::edge_capacity,*this,e.first,capacities[i]);
    }
  } 
  void max_concurrent_flow(std::vector<mf_demand> &demands,
		      double e=0.1);
  void sp_concurrent_flow(std::vector<mf_demand> &demands);

  void max_concurrent_flow_prescaled(std::vector<mf_demand> &demands,
				     double e);
  void min_congestion_flow(std::vector<mf_demand> &demands,
			   double e);



  NetGraph gdual;
  NetGraph gflow;
  long totalphases;
  double gamma;
  double lambda;
  double beta;
private:
  double calcD();
  void rescale_demands(std::vector<mf_demand> &demands,double scale);
  void rescale_demands_flows(std::vector<mf_demand> &demands,double scale);
  double calcBeta(std::vector<mf_demand> &demands);
  double calcLambda(std::vector<mf_demand> &demands);
  double assign_gflow(std::vector<mf_demand> &demands);
};

typedef boost::graph_traits < NetGraph >::edge_descriptor Edge;
typedef boost::graph_traits < NetGraph >::vertex_descriptor Vertex;

class mf_demand {
 public:
  double flow;
  Vertex source;
  Vertex sink;
  double demand;
  std::map<const std::list<Vertex>,double> path_flow_map;
};
#endif
