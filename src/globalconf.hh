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
#ifndef CLICK_GLOBALCONF_HH
#define CLICK_GLOBALCONF_HH

#include <click/config.h>
#include <click/element.hh>
#include <click/confparse.hh>
#include <click/error.hh>

#include "helper.hh"
#include "common.hh"

CLICK_DECLS

/**@brief (blackadder Core) GlobalConf is a Click Element that contains information that need to be shared across multiple Blackadder Elements.
 * 
 * See the configure method for a detailed description of this information.
 *
 */
class GlobalConf : public Element {
public:
    /** 
     * @brief @brief Constructor: it does nothing - as Click suggests
     */
    GlobalConf();
    /**
     * @brief Destructor: it does nothing - as Click suggests
     */
    ~GlobalConf();

    /**
     * @brief the class name - required by Click
     * @return 
     */
    const char *class_name() const {return "GlobalConf";}
    /**
     * @brief the port count - required by Click - GlobalConf has no ports. It is passed as an argument to other elements so that some global information can be shared,
     */
    const char *port_count() const {return "0/0";}
    /**
     * @brief an AGNOSTIC Element.
     * @return AGNOSTIC
     */
    const char *processing() const {return AGNOSTIC;}
    /**
     * @brief GlobalConf configuration...see details.
     * 
     * This Configuration is shared across multiple Blackadder's elements, like the LocalProxy, LocalRV and the Forwarder.
     * 
     * MODE: a String representing the mode in which Blackadder will overlay. It is "mac" and "ip" for overlaying on top of Ethernet and raw IP sockets, respectively.
     * 
     * NODEID: The (statistically unique) node label.
     * 
     * DEFAULTRV: a String representing the LIPSIN identifier to the domain's rendezvous node. It is wrapped to a BitVector.
     * 
     * iLID:      a String representing the internal Link Identifier. It is wrapped to a BitVector.
     * 
     * TMFID:     a String representing the LIPSIN identifier to the domain's Topology Manager. It is wrapped to a BitVector.
     */
    int configure(Vector<String>&, ErrorHandler*);
    /**
     * @brief The GlobalConf Element MUST be initialized and configured first.
     * 
     * @return it returns 100, the smallest returned value among all Blackadder's elements and, therefore, it starts FIRST.
     */
    int configure_phase() const {return 100;}
    /**@brief It does nothing since the GlobalConf Element does pretty much nothing.
     * 
     * @param errh
     * @return 
     */
    int initialize(ErrorHandler *errh);
    /**@brief It does nothing since nothing is dynamically allocated.
     */
    void cleanup(CleanupStage stage);
    /** @brief the Blackadder's node label.
     * 
     * This label should be statistically unique and it is self-assigned by the node itself.
     * The size of the label MUST be NODEID_LEN (see helper.hh).
     * Right now the deployment application writes the Blackadder Click configuration and all node labels are read by the topology configuration.
     */
    String nodeID;
    /**
     * @brief the default LIPSIN identifier to the domain rendez-vous node.
     * 
     * This LISPIN identifier includes the internal link assigned to the Blackadder node that is the domain's rendezvous point.
     * Right now it is calculated by the deployment application utility.
     */
    BABitvector defaultRV_dl;
    /**@brief The internal Link Identifier of this Blackadder node.
     * 
     * Right now it is calculated by the deployment application utility.
     */
    BABitvector iLID;
    /**@brief a LIPSIN identifier that points to the Blackadder node that runs the Topology Manager.
     * 
     * This LISPIN identifier includes the internal link assigned to the Blackadder node that runs the Topology Manager.
     * Right now it is calculated by the deployment application utility.
     */
    BABitvector TMFID;
    /**@brief This boolean variable denotes the mode in which this Blackadder node runs.
     * 
     * True for overlaying over Ethernet.
     * False for overlaying over raw IP sockets.
     * Right now it is assigned by the deployment application utility using the MODE configuration parameter.
     */
    bool use_mac;
    /** @brief That's the scope where the localRV Element subscribes using the implicit rendezvous strategy.
     * 
     * It is hardcoded to be the /FFFFFFFFFFFFFFFF
     */
    String RVScope;
    /** @brief That's the scope where the TopologyManager subscribes using the implicit rendezvous strategy.
     * 
     * It is hardcoded to be the /FFFFFFFFFFFFFFFE
     */
    String TMScope;
    /**@brief the Information identifier using which this Blackadder node will publish all RV requests.
     * 
     * It is hardcoded to be the /FFFFFFFFFFFFFFFF/NODEID. 
     * 
     * Note that the node label is the same size as the information labels (see helper.hh).
     */
    String nodeRVScope;
    /**@brief the Information identifier using which this Blackadder node will publish all TM requests.
     * 
     * It is hardcoded to be the /FFFFFFFFFFFFFFFE/NODEID. 
     * 
     * Note that the node label is the same size as the information labels (see helper.hh).
     */
    String nodeTMScope;
    /**@brief the Information identifier to which this Blackadder node (the LocalProxy) will subscribe to receive all RV/TM notifications.
     * 
     * It is hardcoded to be the /FFFFFFFFFFFFFFFD/NODEID. 
     * 
     * Note that the node label is the same size as the information labels (see helper.hh).
     */
    String notificationIID;
};

CLICK_ENDDECLS

#endif
