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

#ifndef TM_IGRAPH_HH
#define TM_IGRAPH_HH

#include <map>
#include <set>
#include <vector>
#include <string>
#include <igraph/igraph.h>
#include <climits>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <bitvector.hpp>
#include <blackadder_defs.h>

#include "tm_graph.hpp"



/* XXX: Try to figure out igraph version based on very simple assumptions. */
#define IGRAPH_V_0_6   60
#define IGRAPH_V_0_5   50
#ifdef IGRAPH_VERSION
# define IGRAPH_V IGRAPH_V_0_6
#else /* !defined(IGRAPH_VERSION) */
# define IGRAPH_V IGRAPH_V_0_5
#endif

using namespace std;

/**@brief (Topology Manager) This is a representation of the network topology (using the iGraph library) for the Topology Manager.
 */
class TMIgraph : public TMgraph {
public:
    /**@brief Contructor: creates an empty iGraph graph.
     */
    TMIgraph();
    /**@brief destroys the graph and frees some meomry here and there.
     * 
     */
    virtual ~TMIgraph();
    /**
     * @brief reads the graphml file that describes the network topology and creates some indexes for making the operation faster.
     * 
     * Currently iGraph cannot read the Global Graph variables and, therefore, this methid does it manually.
     * 
     * @param name the /graphML file name
     * @return <0 if there was a problem reading the file
     */
    virtual int readTopology(const char *name);
        /**@brief it calculates a LIPSIN identifier from source to destination using the shortest path.
     * 
     * @param source the node label of the source node.
     * @param destination the node label of the destination node.
     * @return a pointer to a bitvector that represents the LIPSIN identifier.
     */
    Bitvector *calculateFID(string &source, string &destination);
    /**@brief it calculates LIPSIN identifiers from a set of publishers to a set of subscribers using the shortest paths.
     * 
     * @param publishers a reference to a set of node labels, representing the source nodes.
     * @param subscribers a reference to a set of node labels, representing the destination nodes.
     * @param result a reference to a map where the method will put node labels representing source nodes mapped to LIPSIN identifiers. Note that some of these identifiers may be NULL.
     * @param path_vectors a reference to a map of information items and their corresponding delivery paths
     */
    void calculateFID(set<string> &publishers, set<string> &subscribers, map<string, Bitvector *> &result, map<string, set<string> > &path_vectors);
    /**@brief used internally by the above method.
     * 
     * @param source
     * @param destination
     * @param resultFID
     * @param numberOfHops
     * @param path a reference to the string of the result path vector
     */
    void calculateFID(string &source, string &destination, Bitvector &resultFID, unsigned int &numberOfHops, string &path);

    //FIXME: Messy implementation...
    void calculateFID_weighted(string &source, string &destination, Bitvector &resultFID, unsigned int &numberOfHops, string &path, const igraph_vector_t *weights);
    void calculateFID_weighted(set<string> &publishers, set<string> &subscribers, map<string, Bitvector *> &result, map<string, set<string> > &path_vectors, const igraph_vector_t *weights);
    //     recompute the graph after removing edges    
    /**
     * @brief update the graph object when a notification about a link-status change arrives (i.e. add or remove edges)
     * 
     * @param srouce source of the edge (notification publisher)
     * @param destination the node on the other side of the link
     * @param net transport network type (single biderctional link / edge or two unidirectional links / edge)
     * @param update_op the type of the update operation (i.e. ADD or RMV)
     */
    virtual void updateGraph(string &source, string &destination, string &net, bool update_op);
    
    /**
      * @brief Update the links status whenever a LSM update arrives
      * 
      * @param lid The link identifier
      * @param status The latest known link status
      * 
      * Note: Consider updating all the links of a node at the same time with an Overloaded
      * updateLinkState(const LSMPacket &ptk)
      */
    virtual void updateLinkState(const string &lid, const QoSList &status);


public:
   
    /**
     * @brief checks if the source node has any path towards the destination
     */
    int checkisolation(string &source,string& destination);
    
    
    int& get_reverse_node_index(string &nodeid);

    /**@brief the igraph graph
     */
    igraph_t graph;

    /**@brief number of connections in the graph.
     */
    int number_of_connections;
    /**@brief number of nodes in the graph.
     */
    int number_of_nodes;
    /**@brief an index that maps node labels to igraph vertex ids.
     */
    map<string, int> reverse_node_index;
    /**@brief an index that maps node labels to igraph edge ids.
     */
    map<string, int> reverse_edge_index;
    /**@brief an index that maps node labels to their internal link Identifiers.
     */
    map<string, Bitvector *> nodeID_iLID;
    /**@brief an index that maps igraph vertex ids to internal link Identifiers.
     */
    map<int, Bitvector *> vertex_iLID;
    /**@brief an index that maps igraph edge ids to link Identifiers.
     */
    map<int, Bitvector *> edge_LID;
    /**@breif an index that maps flowing information items to their delivering paths
     */
    map<string, set<string> > path_info;
    /**@breif an index that maps removed edges (defined by src/des) to thier corresponding LIDs
     */
    map<string, Bitvector *> rmv_edge_lid;
    /**@breif an index that maps information items to their 'Alternative' Publisher (Standby Pubs)
     */
    map<string, set<string> > ex_pubs;
    
    
protected:
  
    /**
     * @brief Create a new "line" into the QoSLinkWeightMap... 
     * 
     * Each "line"/entry represents the network plane for a specific queueing priority
     */
    virtual void createNewQoSLinkPrioMap(const uint8_t & prio);
    
    
};

#endif
