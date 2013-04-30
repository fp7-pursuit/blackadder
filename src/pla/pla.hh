/*
* Copyright (C) 2010-2012  Yi-Ching Liao
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

#ifndef CLICK_PLA_HH
#define CLICK_PLA_HH

#include "../globalconf.hh"
#include "../forwarder.hh"

#include <clicknet/ether.h>
#include <click/args.hh>
#include <sys/time.h>
#if HAVE_USE_PLA
#include "../../addons/libpla/include/libpla.h"
#include "../../addons/libpla/include/libpla_io.h"
#endif /* HAVE_USE_PLA */

CLICK_DECLS

/**@brief (Blackadder Core) The PLA Element implements the PLA function. Currently it supports the basic LIPSIN mechanism.
 * 
 * It can work in two modes. In a MAC mode it expects ethernet frames from the network devices. It checks the LIPSIN identifiers and pushes packets to another Ethernet interface or to the Forwarder.
 * In IP mode, the PLA expects raw IP sockets as the underlying network. Note that a mixed mode is currently not supported. Some lines must be written.
 */
class PLA : public Element {
public:
	/**
	 * @brief Constructor: it does nothing - as Click suggests
	 * @return 
	 */
	PLA();
	/**
	 * @brief Destructor: it does nothing - as Click suggests
	 * @return 
	 */
	~PLA();
	/**
	 * @brief the class name - required by Click
	 * @return 
	 */
	const char *class_name() const {return "PLA";}
#if !HAVE_USE_PLA
	const char *port_count() const {return PORTS_0_0;}
#else /* HAVE_USE_PLA */
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
	 * @brief Element configuration. PLA needs a pointer to the GlovalConf Element so that it can read the Global Configuration.
	 * Then, there is the forwarder element. 
	 * SIGN_PLA: Indicates signing the packet with the certificate or not.
	 * LIGHT_VER_PROB: Sets the lightweight verification probability to a number between 0 and 1.
	 * VER_PROB: Sets the verification probability to a number between 0 and 1.
	 * CRYPTO: Indicates applying cryptographic solution or not.
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
	 */
	void cleanup(CleanupStage stage);
	/**@brief This method is called whenever a packet is received from the network (and pushed to the PLA by a "network" Element) or whenever the Forwarder pushes a packet to the PLA element.
	 * 
	 * Forwarder pushes packets to the 0 port of the PLA. If PLA is active, PLA signs the packet with the certificate.
	 * 
	 * When a packet is pushed from the network, the PLA verifies the packet according to the verification probability and pushes the packet to the Forwarder elements.
	 * 
	 * @param port the port from which the packet was pushed. 0 for Forwarder, 1 for network elements
	 * @param p a pointer to the packet
	 */
	void push(int port, Packet *p);
	/**@brief A pointer to the GlobalConf Element for reading some global node configuration.
	 */
	GlobalConf *gc;
	/**@brief It is used for filling the ip_id field in the IP packet when sending over raw sockets. It is increased for every sent packet.
	 */
	atomic_uint32_t _id;
	/** @brief a pointer to the Forwarder Element.
	 */
	Forwarder *forwarder_element;
	/**@brief This boolean variable denotes signing the packet with the certificate or not.
	 */
	bool SIGN_PLA;
	/**@brief This boolean variable denotes applying cryptographic solution or not.
	 */
	bool CRYPTO;
		/**@brief This variable denotes the PLA verification probability. 
	 * 
	 * One out of 1/VER_PROB packets are verified. The remaining packets are sent without verification.
	 */
	uint32_t VER_PROB;
		/**@brief This variable denotes the PLA verification probability for timestamp and sequence number. 
	 * 
	 * One out of 1/LIGHT_VER_PROB packets are verified. The remaining packets are sent without verification.
	 */
	uint32_t LIGHT_VER_PROB;

private:

	enum { VERIFYING_SHIFT = 28 };
	enum { VERIFYING_MASK = (1 << VERIFYING_SHIFT) - 1 };
	enum { LIGHT_VERIFYING_SHIFT = 28 };
	enum { LIGHT_VERIFYING_MASK = (1 << LIGHT_VERIFYING_SHIFT) - 1 };
	struct timezone tz;
	struct timeval start_tv, end_tv;
	void start_eval();
	void stop_eval();
	void print_eval(int message);
	int proto_type;
	/**@brief the original Forwarding Element header length
	 */
	uint8_t HD_LEN;
	uint8_t PLA_LEN;
#endif /* HAVE_USE_PLA */
};

CLICK_ENDDECLS
#endif /* CLICK_PLA_HH */
