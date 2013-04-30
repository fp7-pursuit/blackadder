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

#ifndef META_DATA_PACKET 
#define META_DATA_PACKET

#include <inttypes.h>
#include <iostream>
#include <string>
#include <map>

#include "blackadder_defs.h"
#include "blackadder.hpp"
#include "bitvector.hpp"

#include "qos_structs.hpp"
#include "bytearray.hpp"


#define QoS_METADATA 108

using namespace std;

class MetaDataPacket{
  protected:
    const static int  CONST_HDR_LEN = (1+1+1); // Type + ID Len ... NumOfItems
    ByteArray data_;
  
  public:
  
    /**
     * Create an empty packet with allocated space for ID,
     * correct type/id length and 0 items
     */ 
    MetaDataPacket(const uint8_t & idlen);
    
    /**
     * Create empty packet and pre-alloc space
     */ 
    MetaDataPacket(uint32_t datalen);
    virtual ~MetaDataPacket();
    /**
     * Create Packet form data
     */ 
    MetaDataPacket(uint8_t * data, int size);
    
    
    // Replace all MetaDataPacket  data
    /**
     * Replace the packet's data
     */ 
    void setData(uint8_t * data, int size);
    
    /**
     * Print in hex
     */ 
    void debugPrint();
    
    /**
     * Get all data in a ByteArray
     */ 
    ByteArray getAllData(){return data_;}
    uint16_t getSize(){return data_.getSize();}
    
    // Packet Header
    
    // Getters
    /** Return the Type */
    uint8_t getType();    
    uint8_t getIDLen();
    string getID();
    string getID_RAW();
    uint8_t getItemNum();
    pair<uint8_t, uint16_t> getItem(int index);
    /** Make the functionality similar to the LSM Packets */
    QoSList getIIStatus();
    uint8_t * getData(){return data_.data;}
    
    // Setters
    void setType(uint8_t type);
    void setIDLen(uint8_t len);
    void setID(string id);
    void setItemNum(uint8_t num);
    void setItem(int index, uint8_t key, uint16_t val);
    /** Make the functionality similar to the LSM Packets */
    void appendIIStatus(QoSList q);
    
    // debug
    friend std::ostream & operator<<(std::ostream & out, MetaDataPacket & pkt);
};

#endif

