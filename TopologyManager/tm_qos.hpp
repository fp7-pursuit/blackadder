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
 * QoS structs for the topology manager. This file contains everything that
 * is NOT related to the network graph...
 */

#include <qos_structs.hpp>
#include <map>
#include <ctime>
#include <string>
#include <openssl/sha.h>

#include <blackadder.hpp>

using namespace std; // XXX


/// expiration time for an entry (in seconds - default 10m)
#define EXPIRATION_TIME 600

/// QUERY expiration time for an entry (in seconds - default 1)
#define QUERY_EXP_TIME 1

/// NON-LAZY INTERVAL (seconds)
#define NON_LAZY_INT 3600

/// Default QoS flow priority (BE)
#define DEFAULT_QOS_PRIO 0

typedef struct __meta_data_cont {
  QoSList qos;
  bool querying;
  time_t querytime;
  time_t created;
  
  // Constructor to ensure 0 times
  __meta_data_cont(){
    created=querytime=0;
    querying=false;
  }
  
} MetaDataContent;

typedef map<string, MetaDataContent> IIMetaDataMap;

/**
 * Wrapper class to help on managing meta data cache and do
 * lazy clean up...
 */
class IIMetaCache {
protected:
  IIMetaDataMap ii_meta;
  time_t last_non_lazy_update;
  
public:
  IIMetaCache();
  
  /**
   * Check if an II exists (true if it does)
   */
  bool exists(const string & ii);
  
  /**
   * Check if it is in quering mode
   */
  bool quering(const string & ii);
  
  /**
   * Check and clean an II. This will be called from getIIMeta on lazy
   * mode in order to remove the II if it is expired, before returning 
   * it to the caller
   */
  void checkClean(const string & ii, Blackadder *ba);
  
  /**
   * Clean all expired II. To be called on non-lazy mode
   * if ever implemented...
   */
  void cleanAllExpired(Blackadder *ba);
  
  /**
   * Get the QoS of an II. This returns:
   * - NULL in case the Item is expired<br>
   * - NULL in case the Item does not exist<br>
   * - The QoSList of the item if it is valid and exits<br>
   */
  QoSList * getIIMeta(const string & ii, Blackadder *ba);
  
  /**
   * Get the QoS of an II. This returns:
   * - DEFAULT in case the Item is expired<br>
   * - DEFAULT in case the Item does not exist<br>
   * - DEFAULT in case QoS_PRIO is not set<br>
   * - DEFAULT when querying<br>
   * - The Priority in any other case...<br>
   */
  uint16_t getIIQoSPrio(const string & ii, Blackadder *ba);
  
  /**
   * Add II if needed...
   * 
   * Blackadder is needed in order to subscribe to the proper scope. 
   * Returns true if the II didn't exist and it had to subscribe to 
   * the proper scope. Returns false if the II was there and not added...
   */
  bool sendQueryIfNeeded(const string & ii, Blackadder *ba);
  
  /**
   * Update Item
   */
  void update(const string & ii, QoSList meta, Blackadder * ba);
  
  
  /**
   * Force add an High Priority II. This is used for the 
   * Subscription on the MetaData publication...
   */
  void addMetaItem(const string & ii);
  
  
  static string getQoSID(const string & bin_item_identifier);
};


