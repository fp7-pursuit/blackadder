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
#ifndef LSM_PACKET_H
#define LSM_PACKET_H

#include <inttypes.h>
#include <iostream>
#include <string>

#include "../helper.hh"
#include "../ba_bitvector.hh"

#include "bytearray.hh"
#include "qos_structs.hh"

#define QoS_REPORT 107

using std::string;

CLICK_DECLS

/**
 * Create the Link State Monitoring packet without any FID
 */
class LSMPacket{
  private:
    ByteArray data_;
    
  public:
    ///  Type + NodeID + ListLen
    const static int  CONST_HDR_LEN = (1+NODEID_LEN+1);
    
    LSMPacket();
    virtual ~LSMPacket();
    LSMPacket(uint8_t * data, int size);
    
    // Replace all LSMPackets packets data
    void setData(uint8_t * data, int size);
    void debugPrint();
    
    
    // Packet Header
    
    // Getters
    /** Return the Type */
    uint8_t getType();
    /** Return the ID Length */
    uint8_t getRepLen();
    string getNodeId();
    
    BABitvector getLid(uint index);
    QoSList getLinkStatus(uint index);
    int getStatusListLen(uint index);
    int getLinkOffset(uint index);
    
    ByteArray getAllData(){return data_;}

    
    // Setters
    void setType(uint8_t type);
    void setRepLen(uint8_t len);
    void setNodeId(string & nodeID);
    
    
    
    void appendLinkStatus(const BABitvector & bv,  QoSList & q);
    
    int getSize(){return data_.getSize();}
    
    
    // debug
    friend std::ostream & operator<<(std::ostream & out, LSMPacket & pkt);
    
  
};
CLICK_ENDDECLS
#endif

