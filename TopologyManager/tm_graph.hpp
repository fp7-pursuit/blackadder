/*
 * Copyright (C) 2012-2013  Mays Al-naday, Andreas Bontozoglou and Martin Reed
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

#ifndef TM_GRAPH_HH
#define TM_GRAPH_HH

#include <map>
#include <set>
#include <string>

#include <igraph/igraph.h>
#include <climits>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <inttypes.h>

#include "tm_max_flow.hpp"

#include <bitvector.hpp>
#include <blackadder_defs.h>

// QoS part
#include "qos_structs.hpp"
typedef std::map<uint8_t, igraph_vector_t> QoSLinkWeightMap;

/* XXX: Try to figure out igraph version based on very simple assumptions. */
#define IGRAPH_V_0_6   60
#define IGRAPH_V_0_5   50
#ifdef IGRAPH_VERSION
# define IGRAPH_V IGRAPH_V_0_6
#else /* !defined(IGRAPH_VERSION) */
# define IGRAPH_V IGRAPH_V_0_5
#endif

#define SUPPORT_SIZE	4
#define TE	0
#define QOS	1
#define RESI	2
#define ML	3

#define UPDATE_BA 55
#define UPDATE_DELIVERY 66
#define LSN 77

using namespace std; // XXX


/** @brief This is a (abstract) generic helper class to carry out TE functions
    it should be inherited from.
*/
class TMgraph {
 public:
   
  /** @brief Constructs a traffic engineering instance of the graph
   */
  TMgraph();
  /** 
   * @brief destroys the TE instance
   */
  virtual ~TMgraph();
  virtual void initialise();
  
  /**
   * @brief reads the graphml file that describes the network topology and creates some indexes for making the operation faster.
   * 
   * Currently iGraph cannot read the Global Graph variables and, therefore, this methid does it manually.
   * 
   * @param name the /graphML file name
   * @return <0 if there was a problem reading the file
   * 
   * 
   */
  virtual int readTopology(const char *name)=0;
  virtual string& getNodeID();
  virtual string& getMode();
  virtual bool getExten(int idx);
  virtual void setExten(int idx);
  virtual string& getRVNodeID();
  
  virtual void calculateFID(string &source, string &destination,
			    Bitvector &resultFID,
			    unsigned int &numberOfHops, string &path)=0;

  virtual Bitvector* calculateFID(string &source, string &destination)=0;
    
  virtual void calculateFID(set<string> &publishers, set<string> &subscribers, map<string, Bitvector* > &result, map<string, set<string> > &path_vectors)=0;
  virtual void updateGraph(string &source, string &destination, string &net, bool update_op) = 0;


  virtual int& get_reverse_node_index(string &nodeid) = 0;
  
  
  //------ QoS --------------------------------------------------------------
  /**
    * @brief A Map that keeps the weight set for each QoS level. 
    * 
    * This efficiently (without using copies of the graph) slices the network 
    * and maps each flow to the best available QoS queues...
    * 
    * TODO: Document
    */
  QoSLinkWeightMap qlwm;
  
  /**
   * @brief Check if the QoS map is OK (initialised). 
   */
  virtual bool isQoSMapOk();
    
  // Virtual Function to calculateFIDs weighted... FIXME: Messy, sub-class!!! 
  virtual void calculateFID_weighted(string &source, string &destination, Bitvector &resultFID, unsigned int &numberOfHops, string &path, const igraph_vector_t *weights)=0;
  virtual void calculateFID_weighted(set<string> &publishers, set<string> &subscribers, map<string, Bitvector *> &result, map<string, set<string> > &path_vectors, const igraph_vector_t *weights)=0;
    
  /**
    * @brief Returns the correct network weights for given II priority
    * 
    * If the II priority does not exist in the network, this function will 
    * downgrade it to the closest/higher available (but smaller than the II prio).
    * Default priority is considered 0 (zero) and it should exist in all networks
    * as BE...
    * 
    */
  virtual uint16_t getWeightKeyForIIPrio(uint8_t prio);
  
  /**
    * @brief Update the links status whenever a LSM update arrives
    * 
    * @param lid The link identifier
    * @param status The latest known link status
    * 
    * Note: Consider updating all the links of a node at the same time with an Overloaded
    * updateLinkState(const LSMPacket &ptk)
    */
  virtual void updateLinkState(const string &lid, const QoSList &status)=0;
  
  /**
   * @brief the set of extension features which can be supported in the TM:
   * currently there are: TE, QoS, Resiliency, ML
   */
  bool extension[SUPPORT_SIZE]; //{TE, qos, resiliency, Multilayer}
  /**
   * @brief the length in bytes of the LIPSIN identifier.
   */
  int fid_len;
  /**
   * @brief mode in which this Blackadder node runs - the TM must create the appropriate netlink socket.
   */
  string mode;
  /**
   * @brief the node label of this Blackadder node.
   */
  string nodeID;
  /**
   * @brief the node label of the RV node
   */
  string RVnodeID;
    


};


// debugging...
std::ostream& operator<<(std::ostream& out, const QoSLinkWeightMap & qm);

#endif




