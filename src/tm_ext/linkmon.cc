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
#include "linkmon.hh"


//#include <iostream>

using namespace std;

CLICK_DECLS



int LinkMon::configure(Vector<String>& conf, ErrorHandler*){
  // Get config...
  gc = (GlobalConf *) cp_element(conf[0], this);
  
  // Get Forwarder
  fw = (Forwarder *) cp_element(conf[1], this);
  
  // Initalize Scope
  LSM_SCOPE="";
  for (uint8_t j = 0; j < PURSUIT_ID_LEN-1; j++) {
      LSM_SCOPE += (char) 0xff;
  }
  LSM_SCOPE += (char) TM_LSM_SCOPE_SUFFIX;
  
  // Active
  active=false;
  
  // Timers
  reportTimer.initialize(this);
  reportTimer.assign(this);
  cp_integer(conf[3], &reportInt);
  
  pollTimer.initialize(this);
  pollTimer.assign(this);
  cp_integer(conf[2], &pollInt);
  
  click_chatter("LinkMon: Scheduling report after: %d sec while polling every %d", reportInt, pollInt);
  reportTimer.schedule_after_sec(reportInt);
  pollTimer.schedule_after_sec(pollInt);
  
  // Copy the rest of the priorities
  for (int i=4; i<conf.size()-1; i++){
    int tmp = 0;
    cp_integer(conf[i], &tmp);
    confPrio.push_back(tmp);
  }
  
  return 0;
}


int LinkMon::initialize(ErrorHandler *){
  
  // Initialize Links
  FwTable fwt = &fw->fwTable;
  
  // Count sub-interfaces per interface
  map<string, int> ifcount;
  
  /*
   * Assuming that the links are going to be parsed from 0-N,
   * the lower the portid the higher the link priority. This 
   * is virtually calculated later...
   * 
   * Note: The link priority is need in order for the topology
   * manager to know the fast-path links
   */
  for (int i=0; i<fwt->size(); i++){
    Link l;
    l.lid = *(fwt->at(i)->LID);
    l.fw_port = fwt->at(i)->port;
    
    // A. IN Counter
    // Get element connected to 1st port
    l.inC = fw->output(l.fw_port).element();
    if (!l.inC) {
       click_chatter("LinkMon: *** Error: No Element Connected at fw[%d]",l.fw_port);
       continue;
    }
    
    // Get the handler...
    l.ich = (Handler *)router()->handler(l.inC,"bit_rate");
    if (l.inC && l.ich == NULL){
      click_chatter("LinkMon: *** Error: Element Name: %s does not have 'bit_rate' handler, SKIPPING...",
		    l.inC->name().c_str());
      continue;
    }
    
    // B. Queue
    // Navigate to queue module!
    l.queue = l.inC->output(0).element();
    if (!l.queue) {
       click_chatter("LinkMon: *** Error: No Element Connected at %s[0]",l.inC->name().c_str());
       continue;
    }
    l.qh = (Handler *)router()->handler(l.queue,"length");
    // Make sure it is a queue!
    if (l.qh == NULL){
      click_chatter("LinkMon: *** Error: Element Name: %s does not have 'length' handler, SKIPPING...",
		    l.queue->name().c_str());
      continue;
    }
    
    /**
     * EVERYTHING AFTER THE QUEUE IS A PULL ELEMENT! (Q is Push2Pull). Therefore we cannot
     * follow the links based on the ports. So we parse the Q element's name and figure out
     * the rest of the names!
     */
    vector<string> parts;
    strExplode(l.queue->name().c_str(), "_", &parts);
    if (parts.size()!=3) {
      click_chatter("Unknown/Wrong Element Naming Format %s. Cannot decide the ifname and sub-interface",l.queue->name().c_str());
      continue;
    }
    
    string ifname = parts.at(1);
    string subif = parts.at(2);
    
    // Store parent interface
    l.parent_if=ifname;
    
    // Update the map
    if (ifcount.count(ifname)) ifcount[ifname]=ifcount[ifname]++;
    else ifcount[ifname]=1;
    
    
    /*
     * C. Bandwidth Shaper!
     */
    l.bs=cp_element((string("BS_")+ifname+"_"+subif).c_str(), this);
    if (!l.bs) {
       click_chatter("LinkMon: *** Error: No Element Connected at %s[0].  Cannot find: %s",l.queue->name().c_str(),(string("BS_")+ifname+"_"+subif).c_str());
       continue;
    }
    l.bsh = (Handler *)router()->handler(l.bs,"rate");
    // Make sure it is a queue!
    if (l.bsh == NULL){
      click_chatter("LinkMon: *** Error: Element Name: %s does not have 'rate' handler, SKIPPING...",
		    l.bsh->name().c_str());
      continue;
    }
    
    // D. OUT Counter
    // Navigate again: to the 4th element... 
    l.outC = cp_element((string("outC_")+ifname+"_"+subif).c_str(), this);
    if (!l.outC) {
       click_chatter("LinkMon: *** Error: No Element Connected at %s[0]",l.bs->name().c_str());
       continue;
    }
    l.och = (Handler *)router()->handler(l.outC,"bit_rate");
    // Make sure it is a counter and an element!
    if (l.outC && l.och == NULL){
      click_chatter("LinkMon: *** Error: Element Name: %s does not have 'bit_rate' handler, SKIPPING...",
		    l.outC->name().c_str());
      continue;
    }
    
    // FIXME:TODO: Assumed 100M ETH
    l.status[QoS_NETTYPE] = NT_ETH;
    l.status[QoS_CAPACITY_M] = 100;
    
    // Priority
    l.status[QoS_PRIO] = confPrio[i];
    
    // Add it!
    click_chatter("LinkMon: Adding monitored link fw[%d] with %s -> %s -> %s -> %s prio=%d", 
		  l.fw_port, l.inC->name().c_str(), l.queue->name().c_str(), l.bs->name().c_str(), l.outC->name().c_str(), l.status[QoS_PRIO]);
    
    links.push_back(l);
  }
  
  /*
   * Here we can calculate the link priority for each registered link
   NOTE: NO! It should be a parameter
  // ifcase counting starts from 1!
  int subifcount=1;
  // last parent interface
  string tmp_last_pif="";
  
  for (int i=0; i<links.size(); i++){
    
    // If it is the last interface, it is the BE=0
    if (subifcount == ifcount[links[i].parent_if]){
      links[i].status[QoS_PRIO] = 0;
      // Reset the count for the next interface
      subifcount=1;
      continue;
    }
      
    // Set non normalized priority
    links[i].status[QoS_PRIO]=100 - subifcount;
    subifcount++;
    
  }
  */
  
  // Publish!
  WritablePacket *p1 = bapi_publish_scope( "", LSM_SCOPE, DOMAIN_LOCAL);
  WritablePacket *p2 = bapi_publish_info( gc->nodeID, LSM_SCOPE, DOMAIN_LOCAL);
  
  output(0).push(p1);
  output(0).push(p2);
  
  return 0;
}


void LinkMon::cleanup(CleanupStage ){
  
  WritablePacket *p2 = bapi_unpublish_info( gc->nodeID, LSM_SCOPE, DOMAIN_LOCAL);
  output(0).push(p2);
}
void LinkMon::push(int, Packet * p){
  uint8_t type = p->data()[0];
  
  if (type==START_PUBLISH) {
    active=true;
    click_chatter("LinkMon: Clear to send, START_PUBLISH received");
  }else if (type==STOP_PUBLISH) {
    active=false;
    click_chatter("LinkMon: STOP_PUBLISH received... pausing");
  }else
    click_chatter("LinkMon: UnKnown TYPE: %d!!!", type);
  
  p->kill();
}

void LinkMon::report(){
  //click_chatter("LinkMon: Time to report ;-)");
  if (!active) {
    click_chatter("LinkMon: We are NOT ACTIVE... re-publishing (where is the TM oeo?)");
    // Try to re publish 
    // The following may not be needed...
    WritablePacket *p0 = bapi_unpublish_info( gc->nodeID, LSM_SCOPE, DOMAIN_LOCAL);
    output(0).push(p0);
    
    WritablePacket *p1 = bapi_publish_scope( "", LSM_SCOPE, DOMAIN_LOCAL);
    WritablePacket *p2 = bapi_publish_info( gc->nodeID, LSM_SCOPE, DOMAIN_LOCAL);
    output(0).push(p1);
    output(0).push(p2);
    
    reportTimer.schedule_after_sec(reportInt);
    return;
  }
  
  if (links.size()==0){
    click_chatter("LinkMon: No Links on this router!!!");
    reportTimer.schedule_after_sec(reportInt);
    return;
  }
  
  // Create packet
  LSMPacket pkt;
  string nodeid=gc->nodeID.c_str();
  pkt.setNodeId(nodeid);
  
  // For each link
  for (int i=0; i<links.size(); i++){
    pkt.appendLinkStatus(links[i].lid, links[i].status);
  }
  
  // DEBUG
//   cout<<pkt<<endl;
//   pkt.debugPrint();
  
  WritablePacket *p = bapi_publish_data( LSM_SCOPE, gc->nodeID, DOMAIN_LOCAL, (char *)pkt.getAllData().data, pkt.getSize());
  output(0).push(p);
  
  // Schedule the next one
  reportTimer.schedule_after_sec(reportInt);
}

void LinkMon::poll(){
  if (links.size()==0){
    click_chatter("LinkMon: No Links on this router!!!");
    pollTimer.schedule_after_sec(pollInt);
    return;
  }
  

  // For each link
  for (int i=0; i<links.size(); i++){
    // Finally do the job
    links[i].update();
  }
  
  pollTimer.schedule_after_sec(pollInt);
}

void LinkMon::run_timer(Timer *rTimer){
 if (rTimer == &reportTimer) report();
 else if (rTimer == &pollTimer) poll();  
}

/// ---- Tools (static, TODO: move) ----
void LinkMon::strExplode(string str, string separator, vector<string>* results){
    unsigned long found;
    found = str.find_first_of(separator);
    while(found != string::npos){
        if(found > 0){
            results->push_back(str.substr(0,found));
        }
        str = str.substr(found+1);
        found = str.find_first_of(separator);
    }
    if(str.length() > 0){
        results->push_back(str);
    }
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(userlevel)
EXPORT_ELEMENT(LinkMon) // For elements
// ELEMENT_PROVIDES(LinkMon) // For non-elements
