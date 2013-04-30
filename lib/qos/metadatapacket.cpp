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

#include "metadatapacket.hpp"

MetaDataPacket::MetaDataPacket(uint32_t datalen){
  data_.extend(CONST_HDR_LEN+datalen);
  setType(QoS_METADATA);
}

MetaDataPacket::~MetaDataPacket(){
}

MetaDataPacket::MetaDataPacket(const uint8_t & idlen){
  data_.extend(idlen*PURSUIT_ID_LEN+CONST_HDR_LEN);
  // Packet Type
  data_.data[0]=QoS_METADATA;
  // ID len.
  data_.data[1]=idlen;
  // Number of Items
  data_.data[PURSUIT_ID_LEN*idlen+2]=0;
}

MetaDataPacket::MetaDataPacket(uint8_t * data, int size){
  setData(data, size);
}

void MetaDataPacket::setData(uint8_t * data, int size){
  if (size<0) return;
  if (data_.getSize() > 0)
    data_.clear();
  data_.extend(size);
  data_.setBytes(data,size,0,0);
}

void MetaDataPacket::debugPrint(){
  cout<<data_<<endl;
}


uint8_t MetaDataPacket::getType(){
  return (uint8_t)data_.data[0];
}

uint8_t MetaDataPacket::getIDLen(){
  return (uint8_t)data_.data[1];
}

string MetaDataPacket::getID(){
  return chararray_to_hex(getID_RAW());
}

string MetaDataPacket::getID_RAW(){
  return data_.getStrBytes(getIDLen()*PURSUIT_ID_LEN, 2,0);
}

uint8_t MetaDataPacket::getItemNum(){
  return (uint8_t)data_.data[CONST_HDR_LEN+getIDLen()*PURSUIT_ID_LEN-1];
}

pair<uint8_t, uint16_t>
MetaDataPacket::getItem(int index){
  pair<uint8_t, uint16_t> item;
  item.first=0;
  
  if (index<0 || index>getItemNum()) return item;
  
  uint16_t offset = CONST_HDR_LEN+getIDLen()*PURSUIT_ID_LEN + (index*QoS_ITEM_SIZE);
  
  item.first = (uint8_t)data_.data[offset++];
  item.second = (uint16_t)data_.getBits(16,offset, 0);
  
  return item;
}

QoSList MetaDataPacket::getIIStatus(){  
  QoSList q;
  
  uint32_t offset = CONST_HDR_LEN+getIDLen()*PURSUIT_ID_LEN;
  int len = getItemNum();
  
  if (len==0) return QoSList();
  
  for (int i=0; i<len; i++){
    q[data_.data[offset]] = data_.getBits(16,offset+1,0);
    offset+=QoS_ITEM_SIZE;
  }

  return q;
}

void MetaDataPacket::setType(uint8_t type){
  data_.data[0] = type;
}

void MetaDataPacket::setIDLen(uint8_t len){
  data_.data[1] = len;
}

void MetaDataPacket::setID(string id){
  setIDLen(id.size()/PURSUIT_ID_LEN);
  data_.setBytes(id,2,0);
}

void MetaDataPacket::setItemNum(uint8_t num){
  data_.data[CONST_HDR_LEN+getIDLen()*PURSUIT_ID_LEN-1] = num;
}

void MetaDataPacket::setItem(int index, uint8_t key, uint16_t val){
  if (index<0 || index>getItemNum()) return;
  
  uint16_t offset = CONST_HDR_LEN+getIDLen()*PURSUIT_ID_LEN + (index*QoS_ITEM_SIZE);
  
  data_.data[offset++] = key;
  data_.setBits((uint64_t)val,16,offset,0);
  
}

void MetaDataPacket::appendIIStatus(QoSList q){
  uint32_t bytepos = CONST_HDR_LEN+getIDLen()*PURSUIT_ID_LEN - 1;
  
  data_.extend(q.size()*QoS_ITEM_SIZE );
  
  // QoSList size
  data_.data[bytepos++]=q.size();
  
  // Copy QoS into bytearray
  for (QoSList::iterator it = q.begin(); it!=q.end(); ++it){    
    data_.data[bytepos++] = it->first;
    data_.setBits((uint64_t)it->second,16,bytepos++,0);
    bytepos++;
  }
}

ostream & operator<<(ostream & out, MetaDataPacket & pkt){
  out<<"Total Size:\t"<<pkt.getSize()<<endl;
  out<<"Type: \t\t"<<(int)pkt.getType()<<endl;
  
  int idlen=pkt.getIDLen();
  out<<"Pkt ID Len:\t"<<idlen<<endl;
  out<<"ID: \t\t"<<pkt.getID()<<endl;
  
  out<<"QoS Items: "<<(int)pkt.getItemNum()<<endl;
  for (uint i=0; i<pkt.getItemNum(); ++i){
    pair<uint8_t,uint16_t> item = pkt.getItem(i);
    out<<" Item #"<<i<<" Key="<<(int)item.first<<" Val="<<(int)item.second<<endl;
  }
  
  
  return out;
}


