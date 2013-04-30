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
/**
 * Internal Blackadder API! To avoid messy code and packet creation 
 * with memcpy.
 * 
 * The functions bellow can be used from any module connected to 
 * local proxy (port>2) that acts as an application running in the 
 * router.
 * 
 * They could (in theory) be used for any packet creation within a
 * BA node.
 */

#ifndef CLICK_BAAPI_HH
#define CLICK_BAAPI_HH

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/string.hh>
#include <click/vector.hh>
#include <click/packet.hh>

#include "../helper.hh"

CLICK_DECLS

/**
 * @brief: Create a BA packet that will not contain any data (pub/sub instructions)
 */
WritablePacket * bapi_makepacket(uint8_t type,String prefix, String id, uint8_t strategy);
/**
 * @brief: Create a BA data packet from the given data...
 */
WritablePacket * bapi_publish_data(String prefix, String id, uint8_t strategy, char * data, uint16_t datalen);

// Detailed
/**
 * @brief: Helper function to publish a new scope (handles type)
 */
WritablePacket * bapi_publish_scope(String prefix, String id, uint8_t strategy);
/**
 * @brief: Helper function to publish a new information item (handles type)
 */
WritablePacket * bapi_publish_info(String prefix, String id, uint8_t strategy);
/**
 * @brief: Helper function to unpublish a scope (handles type)
 */
WritablePacket * bapi_unpublish_scope(String prefix, String id, uint8_t strategy);
/**
 * @brief: Helper function to unpublish an information item (handles type)
 */
WritablePacket * bapi_unpublish_info(String prefix, String id, uint8_t strategy);



CLICK_ENDDECLS
#endif

