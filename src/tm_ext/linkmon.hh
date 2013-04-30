/*
 * Copyright (C) 2012-2013  Andreas Bontozoglou
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

#ifndef CLICK_LINKMON_HH
#define CLICK_LINKMON_HH

#include <vector> // XXX: Should use Click's Vector instead

// Standard includes: Order matters
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/element.hh>
#include <click/string.hh>
#include <click/vector.hh>
#include <click/router.hh>

#include <click/timer.hh>

// DEBUG
//#include <iostream>

using std::vector;

#include "../helper.hh"
#include "../globalconf.hh"
#include "../forwarder.hh"

#include "ba_api.hh"
#include "qos_structs.hh"
#include "lsmpacket.hh"

#define TM_LSM_SCOPE_SUFFIX 0xfb

CLICK_DECLS

/**
 * A link to be monitored. Update currently assumes normal (Not RED)
 * queue so it pulls only the length. Extension is needed in the 
 * forwarder module configuration to get the device type and capacity.
 * 
 * (The same job could be done with a script in user space (ifconfig/iwconfig))
 */
typedef struct _links{
  BABitvector lid;
  int fw_port;
  
  // Parent if
  string parent_if;

  
  /// Pointer to attributes
  // Q
  Handler * qh;
  Element * queue;
  // In Counter
  Handler * ich;
  Element * inC;
  // Bandwidth Shaper
  Element * bs;
  Handler * bsh;
  // Out Counter
  Handler * och;
  Element * outC;
  
  /// Link Status 
  QoSList status;
  
  void inline update(){
    if (qh!=NULL) status[QoS_QUEUELEN] = atoi(qh->call_read(queue).c_str());
    if (ich!=NULL) {
      String bw=ich->call_read(inC);
      status[QoS_INRATE_K] = atoi(bw.c_str())/1000;
    }
    // Update current limit and conver the MBps
    if (bsh!=NULL) {
      String bw=bsh->call_read(bs);
      
      // The bandwidth figure in Kbps (or it will be in a while)
      double bwval = atof(bw.c_str());
      
      // Convert Unit
      if (bw.find_left("MBps")!=-1) bwval *= 8*1000;
      else if (bw.find_left("Gbps")!=-1) bwval *= 1000*1000;
      else if (bw.find_left("Mbps")!=-1) bwval *= 1000;
      else if (bw.find_left("kbps")!=-1) {/*nothing here*/};
      
      // uint16_t max (values up to 65Gbps)
      if (bwval<65536) {
	// Report in Kbps
	status[QoS_BS_LIM_K] = bwval;
	
	// Remove Mbps if in the map...
	QoSList::iterator it = status.find(QoS_BS_LIM_M);
	if (it!=status.end()) status.erase(it);
	
      } else {
	// Report in Mbps
	status[QoS_BS_LIM_M] = bwval/1000;
	
	// Remove Mbps if in the map...
	QoSList::iterator it = status.find(QoS_BS_LIM_K);
	if (it!=status.end()) status.erase(it);
	
      }
      
      
    }
    if (och!=NULL) {
      String bw=och->call_read(outC);
      status[QoS_OUTRATE_K] = atoi(bw.c_str())/1000;
    }
  }
  
} Link;

typedef Vector<Link> LinkTable;

/**
 * Monitor the links and send updates to the proper scope. This
 * module should be configured after the forwarder and after the
 * queue modules!
 */ 
class LinkMon : public Element {
  protected:
    GlobalConf *gc;
    Forwarder * fw;
    Timer reportTimer;
    Timer pollTimer;
    /// Scope to publish to
    String LSM_SCOPE;
    
    /// Publication status
    bool active;
    
    /// All the links
    LinkTable links;
    /// Reporting interval
    int reportInt;
    /// Polling interval
    int pollInt;
    
    Vector<int> confPrio;
    
    // Pointer to fwtable
    typedef Vector<ForwardingEntry *> * FwTable;
    
  public:
    
    virtual const char *class_name() const {return "LinkMon";}
    virtual const char *port_count() const {return "-/-";}
    virtual const char *processing() const {return PUSH;}
    virtual int configure_phase() const {return 501;}
    
    virtual int configure(Vector<String>&, ErrorHandler*);
    virtual int initialize(ErrorHandler *errh);
    virtual void cleanup(CleanupStage stage);
    virtual void push(int, Packet *);
    
    virtual void run_timer(Timer *rTimer);
    virtual void report();
    virtual void poll();
    
    void static strExplode(string str, string separator, vector<string>* results);
  
};
CLICK_ENDDECLS
#endif
