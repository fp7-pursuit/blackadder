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

#include "tm_qos.hpp"

IIMetaCache::IIMetaCache(){
  last_non_lazy_update=time(NULL);
}

bool IIMetaCache::exists(const string & ii){
  IIMetaDataMap::iterator it = ii_meta.find(ii);
  return (it!=ii_meta.end());
}


bool IIMetaCache::quering(const string & ii){
  IIMetaDataMap::iterator it = ii_meta.find(ii);
  
  // II does not exist!
  if (it==ii_meta.end()) return false;
  
  return (it->second.querying);
}
  
void IIMetaCache::cleanAllExpired(Blackadder *ba){
  IIMetaDataMap::iterator it = ii_meta.begin();
  time_t now = time(NULL);
  for (; it!=ii_meta.end();/* ++it*/){
    
    // Check creation time
    if (it->second.created>0){
      if (now-it->second.created > EXPIRATION_TIME){
	cout<<" - Cleaned (outdated)"<<chararray_to_hex(it->first)<<endl;
	ii_meta.erase(it++);
      }
      else {
	++it;
      }
    }
    
    // Check quering ones
    else if (it->second.querying){
      if (now-it->second.querytime > QUERY_EXP_TIME){
	cout<<" - Cleaned  (querying)"<<chararray_to_hex(it->first)<<endl;
	
	string item_id = it->first.substr(it->first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
	string item_pre = it->first.substr(0,it->first.length() - PURSUIT_ID_LEN);
	ba->unsubscribe_info(item_id, item_pre, DOMAIN_LOCAL, NULL, 0);
	ii_meta.erase(it++);
      }else {
	++it;
      }
    }else{
      ++it;
    }
    
  } // End of IIs
}


void IIMetaCache::checkClean(const string & ii, Blackadder *ba){
  cout<<" - checkClean for: "<<chararray_to_hex(ii)<<endl;
  IIMetaDataMap::iterator it = ii_meta.find(ii);
  
  // II does not exist!
  if (it==ii_meta.end()) return;
  
  time_t now = time(NULL);
  
  // Check/Clean
  if (it->second.created>0){
    if (now-it->second.created > EXPIRATION_TIME){
      cout<<" - Cleaned (outdated)"<<chararray_to_hex(it->first)<<endl;
      ii_meta.erase(it);
    }
  }
  // Check quering ones
  else if (it->second.querying){
    if (now-it->second.querytime > QUERY_EXP_TIME){
      cout<<" - Cleaned (querying)"<<chararray_to_hex(it->first)<<endl;
      string item_id = ii.substr(ii.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
      string item_pre = ii.substr(0,ii.length() - PURSUIT_ID_LEN);
      ba->unsubscribe_info(item_id, item_pre, DOMAIN_LOCAL, NULL, 0);
      ii_meta.erase(it);
    }
  }
  
}

 
string IIMetaCache::getQoSID(const string & bin_item_identifier){
      string bin_item_last = bin_item_identifier.substr(bin_item_identifier.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
      unsigned char * qos_id_p = (unsigned char *) malloc(SHA_DIGEST_LENGTH);
      SHA1((const unsigned char*) bin_item_last.c_str(), bin_item_last.length(), qos_id_p);
      string qos_id_tmp = string((const char *) qos_id_p, PURSUIT_ID_LEN);
      delete []qos_id_p;
      return qos_id_tmp;
  }


QoSList * IIMetaCache::getIIMeta(const string & ii, Blackadder *ba) {
  
  cout<<" - getIIMeta for: "<<chararray_to_hex(ii)<<endl;
  // Check the non-lazy
  time_t now = time(NULL);
  if (now - last_non_lazy_update > NON_LAZY_INT)
    cleanAllExpired(ba);
  
  // First of all try to clean it ... if it is needed
  checkClean(ii,ba);
  
  // Now try to find it
  IIMetaDataMap::iterator it = ii_meta.find(ii);
  
  // II does not exist!
  if (it==ii_meta.end()) return NULL;
  
  return &it->second.qos;
}


uint16_t IIMetaCache::getIIQoSPrio(const string & ii, Blackadder *ba){
  cout<<" - getIIQoSPrio for: "<<chararray_to_hex(ii)<<endl;
  // Check the non-lazy
  time_t now = time(NULL);
  if (now - last_non_lazy_update > NON_LAZY_INT)
    cleanAllExpired(ba);
  
  // First of all try to clean it ... if it is needed
  checkClean(ii,ba);
  
  // Now try to find it
  IIMetaDataMap::iterator it = ii_meta.find(ii);
  
  
  // II is queried now...
  if (it->second.querying) return DEFAULT_QOS_PRIO;
  
  // II does not exist!
  if (it==ii_meta.end()) return DEFAULT_QOS_PRIO;
  // Not set
  QoSList::iterator qit = it->second.qos.find(QoS_PRIO);
  if (qit==it->second.qos.end()) return DEFAULT_QOS_PRIO;
  
  // QIT is now set
  return qit->second;
}


bool IIMetaCache::sendQueryIfNeeded(const string & ii, Blackadder *ba){
  // First of all try to clean it ... if it is needed
  checkClean(ii,ba);
  
  // Now we are sure that if it exists, is valid!
  if (exists(ii)) return false;
  
  // Add it...
  MetaDataContent mc;
  mc.querying = true;
  
  string qos_id = getQoSID(ii);
  string qos_prefix = ii.substr(0, ii.length() - PURSUIT_ID_LEN);
  
  // Before subscribing create a fast path for the meta data
  //  1. We subscribe
  //  2. We get a MATCH for the meta data II
  //  3. We must have its priority set!
  addMetaItem(qos_prefix+qos_id);
  cout<<" - sendQueryIfNeeded: subscribing to "<<chararray_to_hex(qos_prefix+qos_id)<<endl;
  
  ba->subscribe_info(qos_id, qos_prefix, DOMAIN_LOCAL, NULL, 0);
  mc.querytime = time(NULL);
  ii_meta[ii] = mc;
  
  return true;
}

void IIMetaCache::update(const string & ii, QoSList meta, Blackadder * ba){
  if (!exists(ii)) {
    cerr<<"Got update for an Item that does not exist! (Ignoring)"<<endl;
    return;
  }
  
  ii_meta[ii].qos = meta;
  ii_meta[ii].querying = false;
  ii_meta[ii].created = time(NULL);
  
  
  // Unsubscribe!
  string qos_id = getQoSID(ii);
  string qos_prefix = ii.substr(0, ii.length() - PURSUIT_ID_LEN);
  ba->unsubscribe_info(qos_id, qos_prefix, DOMAIN_LOCAL, NULL, 0);
  
  // Remove MetaItem
  IIMetaDataMap::iterator it = ii_meta.find(qos_prefix+qos_id);
  if (it!=ii_meta.end()) ii_meta.erase(it);
}


void IIMetaCache::addMetaItem(const string & ii){
  cout<<" - addMetaItem FAKE: "<<chararray_to_hex(ii)<<endl;
  ii_meta[ii] = MetaDataContent();
  ii_meta[ii].qos[QoS_PRIO] = 99;
  ii_meta[ii].querying = false;
  ii_meta[ii].created = time(NULL);
}


