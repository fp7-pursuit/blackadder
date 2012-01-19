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
#ifndef CLICK_SCOPE_HH
#define CLICK_SCOPE_HH

#include "informationitem.hh"
#include "helper.hh"

CLICK_DECLS

class Scope;
class RemoteHost;
class InformationItem;

/**
 * @brief (blackadder Core) A Scope represents a scope that exists somewhere in the information graph managed by a rendezvous element.
 * 
 * It is not the same as an ActivePublication. InformationItem and Scope are managed by the rendezvous element, which knows the whole graph structure.
 * So a Scope keeps track of its place in the graph by having pointers to its father scopes, to its children scopes and information items. Note that only RemoteHost objects are used here. 
 * The rendezvous element does not know about LocalHost objects (applications and click elements).
 * @param _strategy
 * @param father_scope
 */
class Scope {
public:
    /**
     * @brief Constructor: It creates a Scope and links it under a single father scope. Other father scopes are added later as a result of pub/sub requests.
     * 
     * The father scope is also updated to store a pointer to its child Scope. Initially childrenScopes nd informationitems are empty.
     * The publisher or subscriber initiated the construction is later added using the updatePublishers method (the same method actually stores the scope identifier for this item).
     * @param _strategy The assigned dissemination strategy
     * @param father_scope A pointer to the father Scope
     */
    Scope(unsigned char _strategy, Scope *father_scope);
    /**
     * @brief Destructor: deletes the pairs of sets of publishers and subscribers for each full information identifier (ids).
     */
    ~Scope();
    /**
     * @brief Update the set of publishers by adding the provided RemoteHost (publisher) for the provided identifier. Note that we keep a pair of such sets for each different scope identifier.
     * 
     * @param ID the information identifier for which the set of publishers will be updated.
     * @param _publisher A pointer to a RemoteHost representing a publisher (a Blackadder node identified by a statistically unique label).
     * @return False if the publisher was already in the set for the provided ID. True if it wasn't or if the identifier wasn't there (a republication under a new scope). 
     */
    bool updatePublishers(String ID, RemoteHost *_publisher);
    /**
     * @brief Update the set of subscribers by adding the provided RemoteHost (subscriber) for the provided identifier. Note that we keep a pair of such sets for each different scope identifier.
     * 
     * @param ID the information identifier for which the set of subscribers will be updated.
     * @param _subscriber A pointer to a RemoteHost representing a subscriber (a Blackadder node identified by a statistically unique label).
     * @return False if the subscriber was already in the set for the provided ID. True if it wasn't or if the identifier wasn't there. 
     */
    bool updateSubscribers(String ID, RemoteHost *_subscriber);
    /**
     * @brief All ids must be updated because of the publication or the republication. For instance the scope may have two father scopes, each one being identified by multiple identifiers. 
     * 
     * This has to be recursively done for all subscopes and information items that reside under this scope (for the cases of republication).
     * For ALL these unique identifiers this method adds respective identifiers using the provided suffix. The indexed provided (which are the Scope and InformationItem index of the rendezvous element) are also updated.
     * @param scopeIndex a reference to a HashTable where ids are mapped to Scope pointers (the rendezvous index for all scopes identifiers).
     * @param pubIndex a reference to a HashTable where ids are mapped to InformationItem pointers (the rendezvous index for all information item identifiers).
     * @param suffixID the last fragment of the identifier of the added Scope.
     */
    void recursivelyUpdateIDs(ScopeHashMap &scopeIndex, IIHashMap &pubIndex, String suffixID);
    /**
     * @brief Checks if publishers or subscribers for this scope exist ONLY under the provided fatherScope.
     * 
     * @param fatherScope the father Scope under which the method will look for publishers or subscribers. Note that we keep a pair of such sets for each different scope identifier.
     * So the fatherScope will give us the clue about what pair to look.
     * @return True if publishers or subscribers exist. False if not.
     */
    bool checkForOtherPubSub(Scope *fatherScope);
    /**
     * @brief Updates the provided set of subscribers with the subscribers (for all ids) for this scope. The method does NOT look into father scopes.
     * 
     * @param subscribers a reference to a set of subscribers.
     */
    void getSubscribers(RemoteHostSet &subscribers);
    /**
     * @brief Updates the provided set of information items with all the items that are children (NOT all descendant items in the subgraph) of this scope.
     * 
     * @param _informationitems a reference to a set of information items.
     */
    void getInformationItems(InformationItemSet &_informationitems);
    /**
     * @brief Updates the provided set of scopes with all the scopes that are children (NOT all descendant scopes in the subgraph) of this scope.
     * 
     * @param _subscopes a reference to a set of scopes
     */
    void getSubscopes(ScopeSet & _subscopes);
    /**
     * 
     * @return 
     */
    void getIDs(StringSet &_ids);
    /**
     * @brief Returns the first identifier of this InformationItem in a binary format. It has to be quoted_hex() for printing in the usual hex format.
     * 
     * @return a string of the binary representation of the first InformationItem's identifier.
     */
    String printID();
    /**
     * @brief Returns the ith identifier of this InformationItem in a binary format. It has to be quoted_hex() for printing in the usual hex format.
     * 
     * @return a string of the binary representation of the ith InformationItem's identifier.
     */
    String printID(unsigned int i);
    /** @brief The information identifiers of this Scope. 
     * 
     *  They are represented as a HashTable of strings (binary representation of an identifier) mapped to a Pair of set of RemoteHosts, containing the publishers and subscribers, respectively.
     */
    IdsHashMap ids;
    /** @brief A boolean variable stating whether this Scope is a root scope
     */
    bool isRoot;
    /** @brief The set of father scopes
     */
    ScopeSet fatherScopes;
    /** @brief The set of children scopes
     */
    ScopeSet childrenScopes;
    /** @brief The set of children information items
     */
    InformationItemSet informationitems;
    /** @brief The assigned dissemination strategy
     */
    unsigned char strategy;
};

CLICK_ENDDECLS
#endif

