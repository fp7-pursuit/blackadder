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

#ifndef CLICK_FORWARDER_HH
#define CLICK_FORWARDER_HH
#define MAC_LEN 6

#include "globalconf.hh"

#include <click/etheraddress.hh>
#include <clicknet/udp.h>

CLICK_DECLS

/**@brief (blackadder Core) a forwarding_entry represents an entry in the forwarding table of this Blackadder node.
 * 
 * Depending on the network mode in which Blackadder runs in this node, a forwarding entru may have an src and dst EtherAddress, or a src_ip and dst_ip IP address.
 */
class ForwardingEntry {
public:
    ForwardingEntry();
    ~ForwardingEntry();
    /**@brief the source MAC address for this entry (or unused).
     */
    EtherAddress *src;
    /**@brief the destination MAC address for this entry (or unused).
     */
    EtherAddress *dst;
    /**@brief the source IP address for this entry (or unused).
     */
    IPAddress *src_ip;
    /**@brief the destination IP address for this entry (or unused).
     */
    IPAddress *dst_ip;
    /**@brief the output port for the network element that where packets should be forwarded when this entry is used.
     */
    int port;
    /**@brief A bitvector that represents the Link Identifier.
     */
    BABitvector *LID;
};


/**@brief (blackadder Core) The Forwarder Element implements the forwarding function. Currently it supports the basic LIPSIN mechanism.
 * 
 * It can work in two modes. In a MAC mode it expects ethernet frames from the network devices. It checks the LIPSIN identifiers and pushes packets to another Ethernet interface or to the LocalProxy.
 * In IP mode, the Forwarder expects raw IP sockets as the underlying network. Note that a mixed mode is currently not supported. Some lines must be written.
 */
class Forwarder : public Element {
public:
    /**
     * @brief Constructor: it does nothing - as Click suggests
     * @return 
     */
    Forwarder();
    /**
     * @brief Destructor: it does nothing - as Click suggests
     * @return 
     */
    ~Forwarder();
    /**
     * @brief the class name - required by Click
     * @return 
     */
    const char *class_name() const {return "Forwarder";}
    /**
     * @brief the port count - required by Click - it can have multiple output ports that are connected with Click "network" Elements, like ToDevice and raw sockets. 
     * It can have multiple input ports from multiple "network" devices, like FromDevice or raw sockets.
     * @return 
     */
    const char *port_count() const {return "-/-";}
    /**
     * @brief a PUSH Element.
     * @return PUSH
     */
    const char *processing() const {return PUSH;}
    /**
     * @brief Element configuration. Forwarder needs a pointer to the GlovalConf Element so that it can read the Global Configuration.
     * Then, there is the number of (LIPSIN) links. 
     * For each such link the Forwarder reads the outgoing port (to a "network" Element), the source and destination Ethernet or IP addresses (depending on the network mode) as well as the Link identifier (FID_LEN size see helper.hh).
     */
    int configure(Vector<String>&, ErrorHandler*);
    /**@brief This Element must be configured AFTER the GlobalConf Element
     * @return the correct number so that it is configured afterwards
     */
    int configure_phase() const{return 200;}
    /**
     * @brief This method is called by Click when the Element is about to be initialized. There is nothing that needs initialization though.
     * @param errh
     * @return 
     */
    int initialize(ErrorHandler *errh);
    /**@brief Cleanups everything. 
     * 
     * If stage >= CLEANUP_CONFIGURED (i.e. the Element was configured), Forwarder will delete all stored ForwardingEntry.
     */
    void cleanup(CleanupStage stage);
    /**@brief This method is called whenever a packet is received from the network (and pushed to the Forwarder by a "network" Element) or whenever the LocalProxy pushes a packet to the Forwarder.
     * 
     * LocalProxy pushes packets to the 0 port of the Forwarder. The first FID_LEN bytes are the forwarding identifier assigned by the LocalProxy.
     * The Forwarder checks with all its ForwardingEntry by oring the LIPSIN identifier with each Link identifier and pushes the packet to the right "network" elements. 
     * NOTE that the Forwarder will not push back a packet to the LocalProxy if the LIPSIN identifier matches the internal link identifier. This has to be checked before pushing the packet.
     * 
     * When a packet is pushed by the network the Forwarder checks with all its entries as well as the internal Link identifier and pushes the packet accordingly.
     * 
     * In general, if now entries that match are found the packet is killed. Moreover the packet is copied only as required by the number of entries that match the LIPSIN identifier.
     * @param port the port from which the packet was pushed. 0 for LocalProxy, >0 for network elements
     * @param p a pointer to the packet
     */
    void push(int port, Packet *p);
    /**@brief A pointer to the GlobalConf Element for reading some global node configuration.
     */
    GlobalConf *gc;
    /**@brief It is used for filling the ip_id field in the IP packet when sending over raw sockets. It is increased for every sent packet.
     */
    atomic_uint32_t _id;
    /**@brief The number of links in the forwarding table.
     */
    int number_of_links;
    /**@brief The Ethernet protocol type (hardcoded to be 0x080a)
     */
    int proto_type;
    /**@brief A vector containing all ForwardingEntry.
     */
    Vector<ForwardingEntry *> fwTable;
};

CLICK_ENDDECLS
#endif
