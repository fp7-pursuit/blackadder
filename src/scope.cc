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
#include "scope.hh"

CLICK_DECLS

/*Constructor*/
Scope::Scope(unsigned char _strategy, Scope *_father_scope) {
    if (_father_scope == NULL) {
        /*a root scope*/
        strategy = _strategy;
        isRoot = true;
    } else {
        strategy = _strategy;
        isRoot = false;
        fatherScopes.find_insert(ScopeSetItem(_father_scope));
        _father_scope->childrenScopes.find_insert(ScopeSetItem(this));
    }
}

/*Destructor*/
Scope::~Scope() {
    for (IdsHashMapIter it = ids.begin(); it != ids.end(); it++) {
        RemoteHostPair * pair = it.value();
        delete pair;
    }
}

/*update the publishers' set with _publisher. Create a new ID entry in the ids hashtable if it does not exist.
 * return true if the publisher did not exist*/
bool Scope::updatePublishers(String fullID, RemoteHost *_publisher) {
    RemoteHostPair * pair = NULL;
    pair = ids.get(fullID);
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

/*Update the subscribers' set with _subscriber.
 * Create a new ID entry in the ids hashtable if it does not exist*/
bool Scope::updateSubscribers(String fullID, RemoteHost *_subscriber) {
    RemoteHostPair * pair = NULL;
    pair = ids.get(fullID);
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

/*if there are multiple paths to this scope the scopeIndex and pubIndex should be updated (do that recursively for all subscopes and InformationItems)*/
void Scope::recursivelyUpdateIDs(ScopeHashMap &scopeIndex, IIHashMap &pubIndex, String suffixID) {
    Scope *fatherScope;
    for (ScopeSetIter father_it = fatherScopes.begin(); father_it != fatherScopes.end(); father_it++) {
        fatherScope = (*father_it)._scpointer;
        for (IdsHashMapIter it = fatherScope->ids.begin(); it != fatherScope->ids.end(); it++) {
            String fullID = (*it).first + suffixID;
            RemoteHostPair * pair = ids.get(fullID);
            if (pair == ids.default_value()) {
                //click_chatter("adding to the index scope ID %s", fullID.quoted_hex().c_str());
                scopeIndex.set(fullID, this);
                pair = new RemoteHostPair();
                ids.set(fullID, pair);
            }
        }
    }
    /*This is recursive here but it is OK - it has to be recursive*/
    for (ScopeSetIter child_it = childrenScopes.begin(); child_it != childrenScopes.end(); child_it++) {
        String tempSuffix = (*(*child_it)._scpointer->ids.begin()).first.substring((*(*child_it)._scpointer->ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
        (*child_it)._scpointer->recursivelyUpdateIDs(scopeIndex, pubIndex, tempSuffix);
    }
    InformationItemSet::iterator pub_it;
    for (pub_it = informationitems.begin(); pub_it != informationitems.end(); pub_it++) {
        String tempSuffix = (*(*pub_it)._iipointer->ids.begin()).first.substring((*(*pub_it)._iipointer->ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
        (*pub_it)._iipointer->updateIDs(pubIndex, tempSuffix);
    }
}

/*Return true if there are active publishers or subscribers for that Scope under the specific fatherScope*/
bool Scope::checkForOtherPubSub(Scope * fatherScope) {
    if (fatherScope != NULL) {
        String suffixID = (*ids.begin()).first.substring((*ids.begin()).first.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
        for (IdsHashMapIter it = fatherScope->ids.begin(); it != fatherScope->ids.end(); it++) {
            RemoteHostPair * pair = ids.get(((*it).first + suffixID));
            if ((!pair->first.empty()) || (!pair->second.empty())) {
                return true;
            }
        }
        return false;
    } else {
        /*A ROOT Scope*/
        for (IdsHashMapIter it = ids.begin(); it != ids.end(); it++) {
            RemoteHostPair * pair = it.value();
            if ((!pair->first.empty()) || (!pair->second.empty())) {
                return true;
            }
        }
        return false;
    }
}

void Scope::getSubscribers(RemoteHostSet & subscribers) {
    /*add the subscribers of this scope for all ids*/
    for (IdsHashMapIter id_it = ids.begin(); id_it != ids.end(); id_it++) {
        for (RemoteHostSetIter subscriber_it = (*id_it).second->second.begin(); subscriber_it != (*id_it).second->second.end(); subscriber_it++) {
            subscribers.find_insert((*subscriber_it));
        }
    }
}

void Scope::getInformationItems(InformationItemSet & _informationitems) {
    for (InformationItemSetIter pub_it = informationitems.begin(); pub_it != informationitems.end(); pub_it++) {
        _informationitems.find_insert(*pub_it);
    }
}

void Scope::getSubscopes(ScopeSet & _subscopes) {
    for (ScopeSetIter subscope_it = childrenScopes.begin(); subscope_it != childrenScopes.end(); subscope_it++) {
        _subscopes.find_insert(*subscope_it);
    }
}

void Scope::getIDs(StringSet &_ids) {
    IdsHashMapIter it ;
    for(it= ids.begin(); it != ids.end();it++) {
        _ids.find_insert((*it).first);
    }
}

String Scope::printID() {
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

String Scope::printID(unsigned int index) {
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
ELEMENT_PROVIDES(Scope)
