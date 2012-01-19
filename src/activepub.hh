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

#ifndef CLICK_ACTIVEPUB_HH
#define CLICK_ACTIVEPUB_HH

#include "common.hh"
#include <click/vector.hh>

CLICK_DECLS

class LocalHost;

/**
 * @brief (blackadder Core) ActivePublication represents an active publication of an application or click element or another Linux module.
 
 * It can be either a scope or an information item. The LocalProxy only handles active publications.
 */
class ActivePublication {
public:
    /**@brief Constructor: It constructs an ActivePublication object using the provided values.
     * 
     * @param _fullID the full identifier of the published scope or item, which has size of PURSUIT_ID_LEN * number of fragments in the graph. 
     * Therefore it is the identifier starting from a root of the information graph.
     * LocalProxy does not know the information graph. Only RV elements know the graphs for which they are rendezvous points.
     * @param _strategy the dissemination strategy assigned to this scope or item.
     * @param _isScope is it a scope? 
     * @return 
     */
    ActivePublication(String _fullID, unsigned char _strategy, bool _isScope);
    /**
     * @brief Destructor: there is nothing dynamically allocated so it is the default destructor
     */
    ~ActivePublication();
    /** @brief The identifier of the scope or information item starting from the root of the information graph
     */
    String fullID;
    /** @brief The assigned dissemination strategy
     */
    unsigned char strategy;
    /** @brief A Click HashTable<LocalHost *, unsigned char> mapping each publisher (application or click element) to a START_PUBLISH or STOP_PUBLISH byte. 
     * 
     *  That way the LocalProxy knows which publisher is notified for each active publication. Only one publisher can be notified at a specific time.
     */
    PublisherHashMap publishers;
    /**@brief A vector of Strings with which this scope or information item is known to this LocalProxy. Initially each item is only known by the identifier provided by the application.
     * 
     * As events regarding this scope or item arrive (e.g. start/stop publish or new/deleted scope) the LocalProxy assigns more IDs to this (if there are any). 
     * The Local Proxy does not know the structure so it only learns by pub/sub events.
     */
    Vector<String> allKnownIDs;
    /** @brief This is the LIPSIN identifier to the subscribers assigned to this item or scope. 
     * 
     *  Usually, the FID is initially NULL unless the implicit rendezvous strategy is selected and the application provided a FID, which is directly assigned to the item.
     */
    BABitvector FID_to_subscribers;
    /** @brief The LIPSIN identifier to the node that is the rendevous point in respect to the assigned strategy. 
     * 
     *  It can be the internal Link Identifier is the strategy is NODE_LOCAL or a preconfigured FID to the domain's rendezvous.
     */
    BABitvector RVFID;
    /**@brief Does this publication refere to a scope or an information item?
     */
    bool isScope;
};

/**
 * @brief (blackadder Core) ActiveSubscription represents an active subscription of an application or click element or another Linux module.
 * 
 * It can be either a scope or an information item. The LocalProxy only handles active subscriptions.
 */
class ActiveSubscription {
public:
    /**@brief Constructor: It constructs an ActiveSubscription object using the provided values.
     * @param _fullID the full identifier of the subscribed scope or item, which has size of PURSUIT_ID_LEN * number of fragments in the graph. 
     * Therefore it is the identifier starting from a root of the information graph.
     * LocalProxy does not know the information graph. Only RV elements know the graphs for which they are rendezvous points.
     * @param _strategy the dissemination strategy assigned to this subscription.
     * @param _isScope is it a scope? 
     */
    ActiveSubscription(String _fullID, unsigned char _strategy, bool _isScope);
    /**
     * @brief Destructor: there is nothing dynamically allocated so it is the default destructor
     */
    ~ActiveSubscription();
    /** @brief The identifier of the scope or information item starting from the root of the information graph
     */
    String fullID;
    /** @brief The assigned dissemination strategy
     */
    unsigned char strategy;
    /** @brief A Click HashTable<LocalHostSetItem> (see common.hh). It is the way sets can be implemented using click-compatible code. 
     * 
     *  It contains all applications and click elements that are subscribed to this scope or item.
     */
    LocalHostSet subscribers;
    /**@brief A vector of Strings with which this scope or information item is known to this LocalProxy. Initially each item is only known by the identifier provided by the application.
     * 
     * As events regarding this scope or item arrive (e.g. start/stop publish or new/deleted scope) the LocalProxy assigns more IDs to this (if there are any). 
     * The Local Proxy does not know the structure so it only learns identifiers by pub/sub events.
     */
    Vector<String> allKnownIDs;
    /** @brief The LIPSIN identifier to the node that is the rendevous point in respect to the assigned strategy. 
     * 
     *  It can be the internal Link Identifier is the strategy is NODE_LOCAL or a preconfigured FID to the domain's rendezvous.
     */
    BABitvector RVFID;
    /**@brief Does this publication refere to a scope or an information item?
     */
    bool isScope;
};

CLICK_ENDDECLS

#endif
