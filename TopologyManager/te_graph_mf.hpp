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

#ifndef TE_GRAPH_MF_HH
#define TE_GRAPH_MF_HH
#include "tm_graph.hpp"
#define BOOST_NO_HASH

/** @brief This is a data structure to hold a TE routing demand and
    pick a path from a multiple path set. There is one demand for
    each source/sink in the network ie this is an aggregate demand
    not a microflow demand.
*/
class te_mf_demand : public mf_demand {
 public:
  // these flows will be stored in reverse order (from sink to source)
  std::vector<std::vector<Vertex> > paths;
  std::vector<double> flows;
  std::vector<double> probs;
  std::vector<Bitvector> fids;
  int pick_flow();
};
  
/** @brief A collection of te_mf_demand objects indexed
    by source/sink pairs.
*/
class te_mf_demandMap {
 public:
  std::vector< std::vector<int> > get_paths(int source, int sink);
  std::vector<double> get_probs(int source, int sink);
  std::vector<double> get_flows(int source, int sink);
  std::vector<Bitvector> get_fids(int source, int sink);
  Bitvector& get_fid(int source, int sink);
  void insert_demand(te_mf_demand &demand);
  std::map< std::pair<int,int> , te_mf_demand> demand_map;
  
};


/** @brief This is a helper class to carry out TE routing
 */
class TEgraphMF : public TMgraph {
 public:
  /** @brief Constructs a traffic engineering instance of the graph
   */
  TEgraphMF();
  /** @brief destroys the TE instance
   */
  ~TEgraphMF();

  /** @brief sets parameters 
      @param recalctime seconds between recalculations, default 60
      @param e optimisation parameter, defualt 0.1 (must be 0 < e < 1)
      @param defbw default bandwidth in bits per second, default 10^8
   */
  void initialise(int recalctime,double e,double defbw);

  void calculateFID(string &source, string &destination,
		    Bitvector &resultFID,
		    unsigned int &numberOfHops, string &path);

  Bitvector* calculateFID(string &source, string &destination);
    
  void calculateFID(set<string> &publishers, set<string> &subscribers, 
		    map<string, Bitvector* > &result, map<string, set<string> > &path_vectors);
  void updateGraph(string &source, string &destination, string &net, bool update_op);

  
  /**@brief reads the graphml file that describes the network topology and creates some indexes for making the operation faster.
   * 
   * Currently iGraph cannot read the Global Graph variables and,
   * therefore, this method does it manually.
   * 
   * @param name the /graphML file name
   * @return <0 if there was a problem reading the file
   */
  int readTopology(const char *name);
  // Canceled out QoS members... not supported by this class...
  virtual void calculateFID_weighted(string &source, string &destination, Bitvector &resultFID, unsigned int &numberOfHops, string &path, const igraph_vector_t *weights){}
  virtual void calculateFID_weighted(set<string> &publishers, set<string> &subscribers, map<string, Bitvector *> &result, map<string, set<string> > &path_vectors, const igraph_vector_t *weights){}
  virtual void updateLinkState(const string &lid, const QoSList &status){}

  /**@brief the node label of this Blackadder node.
     In the abscence of any bandwidth information on links
     this is the value used.
   */
  double defaultBW;

  /**@brief value for the approximation limit in the mx-flow in range (0,1.0)     value of 0.1 will givea about 10% away from optimum, 0.01 better but
     quite slow. Default is 0.1
  */
  double e;

  void update_paths();

  /**@brief time between end of one max-flow calculation and 
     the start of the other
   */
  int recalculationDelay;

  /**@breif an index that maps removed edges (defined by src/des) to thier corresponding LIDs
  */
  map<string, Bitvector *> rmv_edge_lid;
  
 private:
  /**@brief it calculates a LIPSIN identifier from source to destination using the shortest path.
   * 
   * @param source the node label of the source node.
   * @param destination the node label of the destination node.
   * @return a pointer to a Bitvector that represents the LIPSIN identifier.
   */
  Bitvector* uncachedCalculateFID(string &source, string &destination);

  /**@brief calculate FID and number of hops (not using cache).
   * 
   * @param source
   * @param destination
   * @param resultFID
   * @param numberOfHops
   */
  void uncachedCalculateFID(string &source, string &destination, Bitvector &resultFID, unsigned int &numberOfHops);
      
    int& get_reverse_node_index(string &nodeid);
    
    void preCalculateFids();
  
    bool cacheValid;
  /**@brief the igraph graph
   */
  igraph_t graph;
  /**@brief the Graph_mf graph
   */
  Graph_mf graphMF;

  /**@brief number of connections in the graph.
   */
  int number_of_connections;
  /**@brief number of nodes in the graph.
   */
  int number_of_nodes;
  /**@brief map storing hop count between nodes by node labels
   */
  map< pair< string,string > , int > distanceMap;
  /**@brief map storing hop count between nodes by node labels
   */
  map< pair< string,string > , Bitvector* > fidMap;
  /**@brief an index that maps node labels to igraph vertex ids.
   */
  map<string, int> reverse_node_index;
  /**@brief an index that maps node labels to igraph edge ids.
   */
  map<string, int> reverse_edge_index;
  /**@brief an index that maps node labels to their internal link Identifiers.
   */
  map<string, Bitvector* > nodeID_iLID;
  /**@brief an index that maps igraph vertex ids to internal link Identifiers.
   */
  map<int, Bitvector* > vertex_iLID;
  /**@brief an index that maps igraph edge ids to link Identifiers.
   */


  map<int, Bitvector* > edge_LID;
 
  /**@brief the vector of demands used by the Max Flow algorithm
   */

  typedef std::map< std::pair<int,int>, te_mf_demand> 
    demandMapApplied_type;
  typedef std::map< std::pair<int,int>, mf_demand> 
    demandMapMeasured_type;

  typedef std::pair<int,int> intpair;
  map< pair<int,int>, mf_demand> demandMapMeasured;
  te_mf_demandMap *demandMapApplied;

};


void* te_loop(void *arg);

#endif




