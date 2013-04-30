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

#include "lsmpacket.hh"

using namespace std;

CLICK_DECLS

LSMPacket::LSMPacket(){
  data_.extend(CONST_HDR_LEN);
  setType(QoS_REPORT);
  setRepLen(0);
}

LSMPacket::~LSMPacket(){
}

LSMPacket::LSMPacket(uint8_t * data, int size){
  setData(data, size);
}

void LSMPacket::setData(uint8_t * data, int size){
  if (size<0) return;
  if (data_.getSize() > 0)
    data_.clear();
  data_.extend(size);
  data_.setBytes(data,size,0,0);
}

void LSMPacket::debugPrint(){
  cout<<data_<<endl;
}


uint8_t LSMPacket::getType() {
  // First byte
  return (uint8_t)data_.data[0];
}

uint8_t LSMPacket::getRepLen(){
  // Second byte
  return (uint8_t)data_.data[NODEID_LEN+1];
}



// Setters
void LSMPacket::setType(uint8_t type){
  data_.data[0] = type;
}
void LSMPacket::setRepLen(uint8_t len){
  data_.data[NODEID_LEN+1] = len;
}

void LSMPacket::setNodeId(string & nodeID){
  data_.setBits(nodeID,1,0);
}

string LSMPacket::getNodeId(){
  return data_.getStrBits(NODEID_LEN,1,0);
}

int LSMPacket::getLinkOffset(uint index){
  if (index>getRepLen()) return -1;
  
  uint16_t offset=CONST_HDR_LEN; //+1 for report len
  
  for (uint i=0; i<index; i++){
    
    // LID size
    offset+=FID_LEN;
    // + #of Items * Item size
    offset+=data_.data[offset]*QoS_ITEM_SIZE;
    
    // +1 for the list length
    ++offset;
  }
  
  return offset;
}

int LSMPacket::getStatusListLen(uint index){
 int offset = getLinkOffset(index);
 if (offset==-1) return -1;
 
 // For this now link we have LID|len|items
 return (int)data_.data[offset+FID_LEN];
}


void LSMPacket::appendLinkStatus(const BABitvector & bv, QoSList & q){
  uint8_t len = getRepLen();
  uint32_t bytepos = data_.getSize();
  uint32_t bitpos = 0;
  data_.extend(FID_LEN + 1 + q.size()*QoS_ITEM_SIZE );
  
  // Copy bv to bytearray
  uint8_t byte = 0;
  for (int i=0; i<bv.size(); i++){
    bool bit = bv[i];
    byte<<=1;
    byte|=bit;
    
    bitpos++;
    if (bitpos==8) {
      bitpos=0; 
      data_.data[bytepos] = byte;
      bytepos++;
    }
  }
  
  // QoSList size
  data_.data[bytepos++]=q.size();
  
  // Copy QoS into bytearray
  QoSList::iterator it = q.begin();
  for (; it!=q.end(); ++it){
    data_.data[bytepos++] = it->first;
    data_.setBits((uint64_t)it->second,16,bytepos++,0);
    bytepos++;
  }
  setRepLen(len+1);
  
}

BABitvector LSMPacket::getLid(uint index){
  if (index > getRepLen()) return BABitvector();
  
  BABitvector bv(FID_LEN*8);
  
  uint32_t bytepos = getLinkOffset(index);
  int bitpos=0;
  
  /**
   * TODO: FIXME:
   * 
   * Currently done bit-by-bit instead of words.
   */
  for (int i=0; i<FID_LEN*8; i++){
    
    bv[bitpos]=data_.getBits8_ORLESS(1,bytepos,bitpos%8);
    
    bitpos++;
    if ((i+1)%8 == 0) bytepos++;
  }
  
  return bv;
}

QoSList LSMPacket::getLinkStatus(uint index){
  if (index > getRepLen()) return QoSList();
  
  QoSList q;
  
  uint32_t offset = getLinkOffset(index) + FID_LEN + 1;// list len
  int len = getStatusListLen(index);
 
  
  if (len==0) return QoSList();
  
  for (int i=0; i<len; i++){
    q[data_.data[offset]] = data_.getBits(16,offset+1,0);
    offset+=QoS_ITEM_SIZE;
  }

  return q;
}



ostream & operator<<(ostream & out, LSMPacket & pkt){
  out<<"Type: \t\t"<<(int)pkt.getType()<<endl;
  out<<"Pkt Size: \t"<<pkt.getSize()<<endl;
  out<<"Node ID: \t"<<pkt.getNodeId()<<endl;
  out<<"Report Len: \t"<<(int)pkt.getRepLen()<<endl;
  
  for (uint i=0; i<pkt.getRepLen(); i++){
    
    out<<" #"<<i<<" LID: "<<pkt.getLid(i).to_string().c_str()<<endl;
    
    QoSList l = pkt.getLinkStatus(i);
    out<<" #"<<i<<" ListLen: "<<l.size()<<endl;
    for (QoSList::iterator it = l.begin(); it!=l.end(); ++it)
      out<<" QoS id="<<(int)it->first<<" val="<<(int)it->second<<" | ";
    
    out<<endl;
  }
  
  return out;
}


CLICK_ENDDECLS
ELEMENT_REQUIRES(userlevel)
ELEMENT_PROVIDES(LSMPacket)
