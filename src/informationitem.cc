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

#include "informationitem.hh"

CLICK_DECLS

/*Constructor*/
InformationItem::InformationItem(unsigned char _strategy, Scope *_father_scope) {
    fatherScopes.find_insert(ScopeSetItem(_father_scope));
    _father_scope->informationitems.find_insert(InformationItemSetItem(this));
    strategy = _strategy;
}

/*Destructor - Ensure that all pairs are deleted*/
InformationItem::~InformationItem() {
    for (IdsHashMapIter it = ids.begin(); it != ids.end(); it++) {
        RemoteHostPair * pair = it.value();
        delete pair;
    }
}

/*if there are multiple paths to this InformationItem the pubIndex should be updated*/
void InformationItem::updateIDs(IIHashMap &pubIndex, String suffixID) {
    Scope *fatherScope;
    for (ScopeSetIter father_it = fatherScopes.begin(); father_it != fatherScopes.end(); father_it++) {
        fatherScope = (*father_it)._scpointer;
        for (IdsHashMapIter it = fatherScope->ids.begin(); it != fatherScope->ids.end(); it++) {
            String fullID = it.key() + suffixID;
            RemoteHostPair * pair = ids.get(fullID);
            if (pair == ids.default_value()) {
                //click_chatter("adding to the index InformationItem ID %s", fullID.quoted_hex().c_str());
                pubIndex.set(fullID, this);
                pair = new RemoteHostPair();
                ids.set(fullID, pair);
            }
        }
    }
}

/*update the publishers' set with _publisher. Create a new ID entry in the ids hashtable if it does not exist*/
bool InformationItem::updatePublishers(String fullID, RemoteHost *_publisher) {
    RemoteHostPair * pair = ids.get(fullID);
    if (pair != ids.default_value()) {
        if(pair->first.find(_publisher) == pair->first.end()) {
            pair->first.find_insert(_publisher);
        } else {
            return false;
        }
    } else {
        pair = new RemoteHostPair();
        pair->first.find_insert(_publisher);
        ids.set(fullID, pair);
    }
    return true;
}

/*update the subscribers' set with _subscriber
Create a new ID entry in the ids hashtable if it does not exist*/
bool InformationItem::updateSubscribers(String fullID, RemoteHost *_subscriber) {
    RemoteHostPair * pair = ids.get(fullID);
    if (pair != ids.default_value()) {
        if(pair->second.find(_subscriber) == pair->second.end()) {
            pair->second.find_insert(_subscriber);
        } else {
            return false;
        }
    } else {
        pair = new RemoteHostPair();
        pair->second.find_insert(_subscriber);
        ids.set(fullID, pair);
    }
    return true;
}

/*Return true if there are active publishers or subscribers for that InformationItem under the specific fatherScope*/
bool InformationItem::checkForOtherPubSub(Scope *fatherScope) {
    String suffixID = (*ids.begin()).first.substring((*ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
    for (IdsHashMapIter it = fatherScope->ids.begin(); it != fatherScope->ids.end(); it++) {
        RemoteHostPair * pair = ids.get(((*it).first + suffixID));
        if ((!pair->first.empty()) || (!pair->second.empty())) {
            return true;
        }
    }
    return false;
}

void InformationItem::getSubscribers(RemoteHostSet &subscribers) {
    /*add the subscribers of this information item for all ids*/
    for (IdsHashMapIter id_it = ids.begin(); id_it != ids.end(); id_it++) {
        RemoteHostSetIter subscriber_it;
        for (subscriber_it = (*id_it).second->second.begin(); subscriber_it != (*id_it).second->second.end(); subscriber_it++) {
            subscribers.find_insert(*subscriber_it);
        }
    }
}

void InformationItem::getPublishers(RemoteHostSet &publishers) {
    /*add the publishers of this information item for all ids*/
    for (IdsHashMapIter id_it = ids.begin(); id_it != ids.end(); id_it++) {
        RemoteHostSetIter publisher_it;
        for (publisher_it = (*id_it).second->first.begin(); publisher_it != (*id_it).second->first.end(); publisher_it++) {
            publishers.find_insert((*publisher_it));
        }
    }
}

/*Print the first ID in the ids HashTable*/
String InformationItem::printID() {
    String toReturn;
    const char hex_digits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    int i = 0;
    StringAccum sa;
    int fragments;
    char *buf;
    fragments = (*ids.begin()).first.length() / PURSUIT_ID_LEN;
    buf = sa.extend((*ids.begin()).first.length() * 2 + fragments);
    *buf++ = '/';
    const uint8_t *e = reinterpret_cast<const uint8_t*> ((*ids.begin()).first.end());
    for (const uint8_t *x = reinterpret_cast<const uint8_t*> ((*ids.begin()).first.begin()); x < e; x++) {
        *buf++ = hex_digits[(*x >> 4) & 0xF];
        *buf++ = hex_digits[*x & 0xF];
        i++;
        if (i % PURSUIT_ID_LEN * 2 == 0) {
            *buf++ = '/';
        }
    }
    toReturn = sa.take_string();
    return toReturn;
}

/*Print the index th ID in the ids HashTable*/
String InformationItem::printID(unsigned int index) {
    String toReturn;
    const char hex_digits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    int i = 0;
    StringAccum sa;
    int fragments;
    char *buf;
    if (index < ids.size()) {
        IdsHashMapIter it = ids.begin();
        for (unsigned int j = 0; j < index; j++) {
            it++;
        }
        String ID = (*it).first;
        fragments = ID.length() / PURSUIT_ID_LEN;
        buf = sa.extend(ID.length() * 2 + fragments);
        *buf++ = '/';
        const uint8_t *e = reinterpret_cast<const uint8_t*> (ID.end());
        for (const uint8_t *x = reinterpret_cast<const uint8_t*> (ID.begin()); x < e; x++) {
            *buf++ = hex_digits[(*x >> 4) & 0xF];
            *buf++ = hex_digits[*x & 0xF];
            i++;
            if (i % PURSUIT_ID_LEN * 2 == 0) {
                *buf++ = '/';
            }
        }
        toReturn = sa.take_string();
        return toReturn;
    } else {
        return String("");
    }
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(InformationItem)
