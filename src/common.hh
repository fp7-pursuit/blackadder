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

#ifndef CLICK_COMMON_HH
#define CLICK_COMMON_HH

#include "ba_bitvector.hh"

#include <click/string.hh>
#include <click/hashtable.hh>

CLICK_DECLS

class LocalHost;
class RemoteHost;
class Scope;
class InformationItem;
class ActivePublication;
class ActiveSubscription;

/** @brief (blackadder Core) A click-compatible way of implementing a set of LocalHost (see localhost.hh)
 * 
 * LocalHostSetItem represents a LocalHost that can be inserted in a set (Click HashTable)
 */
struct LocalHostSetItem {
    /**@brief pointer to LocalHost.
     */
    LocalHost * _lhpointer;

    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef LocalHost * key_type;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef LocalHost * key_const_reference;

    /**@brief required by Click to implement Sets using a HashTable.
     */
    key_const_reference hashkey() const {
        return _lhpointer;
    }

    /**@brief required by Click to implement Sets using a HashTable.
     */
    LocalHostSetItem(LocalHost * lhp) : _lhpointer(lhp) {
    }
};

/** @brief (blackadder Core) A click-compatible way of implementing a set of RemoteHost (see remotehost.hh)
 * 
 * RemoteHostSetItem represents a RemoteHost that can be inserted in a set (Click HashTable)
 */
struct RemoteHostSetItem {
    /**@brief pointer to RemoteHost.
     */
    RemoteHost * _rhpointer;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef RemoteHost * key_type;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef RemoteHost * key_const_reference;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    key_const_reference hashkey() const {
        return _rhpointer;
    }
    /**@brief required by Click to implement Sets using a HashTable.
     */
    RemoteHostSetItem(RemoteHost * rhp) : _rhpointer(rhp) {
    }
};

/** @brief (blackadder Core) A click-compatible way of implementing a set of String (Click's string)
 * 
 * StringSetItem represents a String that can be inserted in a set (Click HashTable)
 */
struct StringSetItem {
    /**@brief a String.
     */
    String _strData;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef String key_type;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef String key_const_reference;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    key_const_reference hashkey() const {
        return _strData;
    }
    /**@brief required by Click to implement Sets using a HashTable.
     */
    StringSetItem(String str) : _strData(str) {

    }
};

/** @brief (blackadder Core) A click-compatible way of implementing a set of InformationItem (see informationitem.hh)
 * 
 * InformationItemSetItem represents an InformationItem that can be inserted in a set (Click HashTable)
 */
struct InformationItemSetItem {
    /**@brief a pointer to an InformationItem.
     */
    InformationItem * _iipointer;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef InformationItem * key_type;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef InformationItem * key_const_reference;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    key_const_reference hashkey() const {
        return _iipointer;
    }
    /**@brief required by Click to implement Sets using a HashTable.
     */
    InformationItemSetItem(InformationItem * iip) : _iipointer(iip) {
    }
};

/** @brief (blackadder Core) A click-compatible way of implementing a set of Scope (see scope.hh)
 * 
 * ScopeSetItem represents a Scope that can be inserted in a set (Click HashTable)
 */
struct ScopeSetItem {
    /**@brief a pointer to a Scope.
     */
    Scope * _scpointer;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef Scope * key_type;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    typedef Scope * key_const_reference;
    /**@brief required by Click to implement Sets using a HashTable.
     */
    key_const_reference hashkey() const {
        return _scpointer;
    }
    /**@brief required by Click to implement Sets using a HashTable.
     */
    ScopeSetItem(Scope * scp) : _scpointer(scp) {
    }
};
/** @brief A set (implemented as a Click's HashTable) of applications and click elements (see localhost.hh).
 */
typedef HashTable<LocalHostSetItem> LocalHostSet;
/** @brief An iterator to a set (implemented as a Click's HashTable) of applications and click elements (see localhost.hh).
 */
typedef LocalHostSet::iterator LocalHostSetIter;
/** @brief A set (implemented as a Click's HashTable) of Remote Hosts (see remotehost.hh).
 */
typedef HashTable<RemoteHostSetItem> RemoteHostSet;
/** @brief An iterator to a set (implemented as a Click's HashTable) of Remote Hosts (see remotehost.hh).
 */
typedef RemoteHostSet::iterator RemoteHostSetIter;
/** @brief A set (implemented as a Click's HashTable) of Information Items (see informationitem.hh).
 */
typedef HashTable<InformationItemSetItem> InformationItemSet;
/** @brief An iterator to a set (implemented as a Click's HashTable) of Information Items (see informationitem.hh).
 */
typedef InformationItemSet::iterator InformationItemSetIter;
/** @brief A set (implemented as a Click's HashTable) of Scopes (see scope.hh).
 */
typedef HashTable<ScopeSetItem> ScopeSet;
/** @brief An iterator to a set (implemented as a Click's HashTable) of Scopes (see scope.hh).
 */
typedef ScopeSet::iterator ScopeSetIter;
/** @brief A set (implemented as a Click's HashTable) of Click's Strings.
 */
typedef HashTable<StringSetItem> StringSet;
/** @brief An iterator to a set (implemented as a Click's HashTable) of Click's Strings.
 */
typedef StringSet::iterator StringSetIter;
/** @brief A Click's Pair of remotehosts.
 */
typedef Pair<RemoteHostSet, RemoteHostSet> RemoteHostPair;
/** @brief A Click's HashTable of Click's Strings mapped to Pair of set of RemoteHosts.
 */
typedef HashTable<String, RemoteHostPair *> IdsHashMap;
/** @brief An iterator to a Click's HashTable of Click's Strings mapped to Pair of set of RemoteHosts.
 */
typedef IdsHashMap::iterator IdsHashMapIter;
/** @brief A Click's HashTable of Click's Strings mapped to a RemoteHost.
 */
typedef HashTable<String, RemoteHost *> RemoteHostHashMap;
/** @brief An iterator to a Click's HashTable of Click's Strings mapped to a RemoteHost.
 */
typedef RemoteHostHashMap::iterator RemoteHostHashMapIter;
/** @brief A Click's HashTable of Click's Strings mapped to an InformationItem.
 */
typedef HashTable<String, InformationItem *> IIHashMap;
/** @brief An iterator to a Click's HashTable of Click's Strings mapped to an InformationItem.
 */
typedef IIHashMap::iterator IIHashMapIter;
/** @brief A Click's HashTable of Click's Strings mapped to pointers to Scopes.
 */
typedef HashTable<String, Scope *> ScopeHashMap;
/** @brief An iterator Click's HashTable of Click's Strings mapped to pointers to Scopes.
 */
typedef ScopeHashMap::iterator ScopeHashMapIter;
/** @brief A Click's HashTable of pointers to LocalHost mapped to Click's Strings.
 */
typedef HashTable<LocalHost *, String> LocalHostStringHashMap;
/** @brief An iterator to a Click's HashTable of pointers to LocalHost mapped to Click's Strings.
 */
typedef LocalHostStringHashMap::iterator LocalHostStringHashMapIter;
/**@brief  A Click's HashTable of pointers to LocalHost mapped to unsigned char.
 */
typedef HashTable<LocalHost *, unsigned char> PublisherHashMap;
/**@brief  An iterator to a Click's HashTable of pointers to LocalHost mapped to unsigned char.
 */
typedef PublisherHashMap::iterator PublisherHashMapIter;
/**A Click's HashTable of pointers to RemoteHost mapped to a pointer to BABitvector.
 */
typedef Pair<RemoteHost *, BABitvector *> RemoteHostBitVectorPair;
/** @brief A Click's HashTable of integers mapped to pointers of LocalHost.
 */
typedef HashTable<int, LocalHost *> PubSubIdx;
/** @brief An iterator to a Click's HashTable of integers mapped to pointers of LocalHost.
 */
typedef PubSubIdx::iterator PubSubIdxIter;
/** @brief A Click's HashTable of Click's Strings mapped to an ActivePublication.
 */
typedef HashTable<String, ActivePublication *> ActivePub;
/** @brief An iterator to a Click's HashTable of Click's Strings mapped to an ActivePublication.
 */
typedef ActivePub::iterator ActivePubIter;
/** @brief A Click's HashTable of Click's Strings mapped to an ActiveSubscription.
 */
typedef HashTable<String, ActiveSubscription *> ActiveSub;
/** @brief An iterator to a Click's HashTable of Click's Strings mapped to an ActiveSubscription.
 */
typedef ActiveSub::iterator ActiveSubIter;

CLICK_ENDDECLS
#endif
