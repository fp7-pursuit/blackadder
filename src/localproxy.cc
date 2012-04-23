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
#include "localproxy.hh"
#include "helper.hh"
#include "ba_bitvector.hh"

CLICK_DECLS

LocalProxy::LocalProxy() {
}

LocalProxy::~LocalProxy() {
    click_chatter("LocalProxy: destroyed!");
}

int LocalProxy::configure(Vector<String> &conf, ErrorHandler *errh) {
    gc = (GlobalConf *) cp_element(conf[0], this);
    //click_chatter("LocalProxy: configured!");
    return 0;
}

int LocalProxy::initialize(ErrorHandler *errh) {
    //click_chatter("LocalProxy: initialized!");
    return 0;
}

void LocalProxy::cleanup(CleanupStage stage) {
    int size = 0;
    if (stage >= CLEANUP_ROUTER_INITIALIZED) {
        size = local_pub_sub_Index.size();
        PubSubIdxIter it1 = local_pub_sub_Index.begin();
        for (int i = 0; i < size; i++) {
            delete (*it1).second;
            it1 = local_pub_sub_Index.erase(it1);
        }
        size = activePublicationIndex.size();
        ActivePubIter it2 = activePublicationIndex.begin();
        for (int i = 0; i < size; i++) {
            delete (*it2).second;
            it2 = activePublicationIndex.erase(it2);
        }
        size = activeSubscriptionIndex.size();
        ActiveSubIter it3 = activeSubscriptionIndex.begin();
        for (int i = 0; i < size; i++) {
            delete (*it3).second;
            it3 = activeSubscriptionIndex.erase(it3);
        }
    }
    click_chatter("LocalProxy: Cleaned Up!");
}

void LocalProxy::push(int in_port, Packet * p) {
    int descriptor, index;
    int type_of_publisher;
    bool forward;
    unsigned char type, numberOfIDs, IDLength /*in fragments of PURSUIT_ID_LEN each*/, prefixIDLength /*in fragments of PURSUIT_ID_LEN each*/, strategy;
    Vector<String> IDs;
    LocalHost *_localhost;
    BABitvector RVFID;
    BABitvector FID_to_subscribers;
    String ID, prefixID;
    index = 0;
    if (in_port == 2) {
        /*from port 2 I receive publications from the network*/
        index = 0;
        /*read the "header"*/
        numberOfIDs = *(p->data());
        /*Read all the identifiers*/
        for (int i = 0; i < (int) numberOfIDs; i++) {
            IDLength = *(p->data() + sizeof (numberOfIDs) + index);
            IDs.push_back(String((const char *) (p->data() + sizeof (numberOfIDs) + sizeof (IDLength) + index), IDLength * PURSUIT_ID_LEN));
            index = index + sizeof (IDLength) + IDLength * PURSUIT_ID_LEN;
        }
        if ((IDs.size() == 1) && (IDs[0].compare(gc->notificationIID) == 0)) {
            /*a special case here: Got back an RV/TM event...it was published using the ID /FFFFFFFFFFFFFFFD/MYNODEID*/
            /*remove the header*/
            p->pull(sizeof (numberOfIDs) + index);
            handleRVNotification(p);
            p->kill();
        } else {
            /*a regular network publication..I will look for local subscribers*/
            /*Careful: I will not kill the packet - I will reuse it one way or another, so....get rid of everything except the data*/
            /*remove the header*/
            p->pull(sizeof (numberOfIDs) + index);
            handleNetworkPublication(IDs, p);
        }
    } else {
        /*the request comes from the IPC element or from a click Element. The descriptor here may be the netlink ID of an application or the click port of an Element*/
        if (in_port == 0) {
            /*The packet came from the FromNetlink Element. An application sent it*/
            descriptor = p->anno_u32(0);
            type_of_publisher = LOCAL_PROCESS;
        } else {
            /*anything else is from a Click Element (e.g. the LocalRV Element)*/
            descriptor = in_port;
            type_of_publisher = CLICK_ELEMENT;
        }
        _localhost = getLocalHost(type_of_publisher, descriptor);
        type = *(p->data());
        if (type == DISCONNECT) {
            disconnect(_localhost);
            p->kill();
            return;
        } else if (type == PUBLISH_DATA) {
            /*this is a publication coming from an application or a click element*/
            IDLength = *(p->data() + sizeof (type));
            ID = String((const char *) (p->data() + sizeof (type) + sizeof (IDLength)), IDLength * PURSUIT_ID_LEN);
            strategy = *(p->data() + sizeof (type) + sizeof (IDLength) + ID.length());
            if (strategy == IMPLICIT_RENDEZVOUS) {
                FID_to_subscribers = BABitvector(FID_LEN * 8);
                memcpy(FID_to_subscribers._data, p->data() + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (strategy), FID_LEN);
                /*Careful: I will not kill the packet - I will reuse it one way or another, so....get rid of everything except the data and will see*/
                p->pull(sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (strategy) + FID_LEN);
                if ((ID.compare(gc->notificationIID) == 0)) {
                    /*A special case here: The locaRV element published data using the blackadder API. This data is an RV notification*/
                    handleRVNotification(p);
                    p->kill();
                } else {
                    handleUserPublication(ID, FID_to_subscribers, p, _localhost);
                }
            } else if (strategy == LINK_LOCAL) {
                FID_to_subscribers = BABitvector(FID_LEN * 8);
                memcpy(FID_to_subscribers._data, p->data() + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (strategy), FID_LEN);
                //click_chatter("publish link_local using LID %s", FID_to_subscribers.to_string().c_str());
                /*Careful: I will not kill the packet - I will reuse it one way or another, so....get rid of everything except the data and will see*/
                p->pull(sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (strategy) + FID_LEN);
                handleUserPublication(ID, FID_to_subscribers, p, _localhost);
            } else if (strategy == BROADCAST_IF) {
                FID_to_subscribers = BABitvector(FID_LEN * 8);
                FID_to_subscribers.negate();
                /*Careful: I will not kill the packet - I will reuse it one way or another, so....get rid of everything except the data and will see*/
                p->pull(sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (strategy));
                handleUserPublication(ID, FID_to_subscribers, p, _localhost);
            } else {
                /*Careful: I will not kill the packet - I will reuse it one way or another, so....get rid of everything except the data*/
                p->pull(sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (strategy));
                handleUserPublication(ID, p, _localhost);
            }
        } else {
            /*This is a pub/sub request*/
            /*read user request*/
            IDLength = *(p->data() + sizeof (type));
            ID = String((const char *) (p->data() + sizeof (type) + sizeof (IDLength)), IDLength * PURSUIT_ID_LEN);
            prefixIDLength = *(p->data() + sizeof (type) + sizeof (IDLength) + ID.length());
            prefixID = String((const char *) (p->data() + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (prefixIDLength)), prefixIDLength * PURSUIT_ID_LEN);
            strategy = *(p->data() + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (prefixIDLength) + prefixID.length());
            RVFID = BABitvector(FID_LEN * 8);
            switch (strategy) {
                case NODE_LOCAL:
                    break;
                case LINK_LOCAL:
                    /*don't do anything here..just a placeholder to remind us about that strategy...subscriptions will recorded only locally. No publication will be sent to the RV (wherever that is)*/
                    break;
                case BROADCAST_IF:
                    /*don't do anything here..just a placeholder to remind us about that strategy...subscriptions will recorded only locally. No publication will be sent to the RV (wherever that is)*/
                    break;
                case DOMAIN_LOCAL:
                    RVFID = gc->defaultRV_dl;
                    break;
                case IMPLICIT_RENDEZVOUS:
                    /*don't do anything here..just a placeholder to remind us about that strategy...subscriptions will recorded only locally. No publication will be sent to the RV (wherever that is)*/
                    break;
                default:
                    click_chatter("LocalProxy: a weird strategy that I don't know of --- FATAL");
                    break;
            }
            forward = handleLocalRequest(type, _localhost, ID, prefixID, strategy, RVFID);
            if (forward) {
                publishReqToRV(p, RVFID);
            } else {
                p->kill();
            }
        }
    }
}

LocalHost * LocalProxy::getLocalHost(int type, int id) {
    LocalHost *_localhost;
    String ID;
    _localhost = local_pub_sub_Index.get(id);
    if (_localhost == local_pub_sub_Index.default_value()) {
        _localhost = new LocalHost(type, id);
        local_pub_sub_Index.set(id, _localhost);
    }
    return _localhost;
}

void LocalProxy::disconnect(LocalHost *_localhost) {
    /*there is a bug here...I have to rethink how to correctly delete all entries in the right sequence*/
    if (_localhost != NULL) {
        click_chatter("LocalProxy: Entity %s disconnected...cleaning...", _localhost->localHostID.c_str());
        /*I know whether we talk about a scope or an information item from the isScope boolean value*/
        deleteAllActiveInformationItemPublications(_localhost);
        deleteAllActiveInformationItemSubscriptions(_localhost);
        deleteAllActiveScopePublications(_localhost);
        deleteAllActiveScopeSubscriptions(_localhost);
        local_pub_sub_Index.erase(_localhost->id);
        delete _localhost;
    }
}

/*Handle application or click element request..the RVFID is NULL except from link-local cases where the application has specified one*/
bool LocalProxy::handleLocalRequest(unsigned char &type, LocalHost *_localhost, String &ID, String &prefixID, unsigned char &strategy, BABitvector &RVFID) {
    bool forward = false;
    String fullID;
    /*create the fullID*/
    if (ID.length() == PURSUIT_ID_LEN) {
        /*a single fragment*/
        fullID = prefixID + ID;
    } else {
        /*multiple fragments*/
        fullID = prefixID + ID.substring(ID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
    }
    switch (type) {
        case PUBLISH_SCOPE:
            //click_chatter("LocalProxy: received PUBLISH_SCOPE request: %s, %s, %s, %d", _localhost->localHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
            forward = storeActivePublication(_localhost, fullID, strategy, RVFID, true);
            break;
        case PUBLISH_INFO:
            //click_chatter("LocalProxy: received PUBLISH_INFO request: %s, %s, %s, %d", _localhost->localHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
            forward = storeActivePublication(_localhost, fullID, strategy, RVFID, false);
            break;
        case UNPUBLISH_SCOPE:
            //click_chatter("LocalProxy: received UNPUBLISH_SCOPE request: %s, %s, %s, %d", _localhost->localHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
            forward = removeActivePublication(_localhost, fullID, strategy);
            break;
        case UNPUBLISH_INFO:
            //click_chatter("LocalProxy: received UNPUBLISH_INFO request: %s, %s, %s, %d", _localhost->localHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
            forward = removeActivePublication(_localhost, fullID, strategy);
            break;
        case SUBSCRIBE_SCOPE:
            //click_chatter("LocalProxy: received SUBSCRIBE_SCOPE request: %s, %s, %s, %d", _localhost->localHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
            forward = storeActiveSubscription(_localhost, fullID, strategy, RVFID, true);
            break;
        case SUBSCRIBE_INFO:
            //click_chatter("LocalProxy: received SUBSCRIBE_INFO request: %s, %s, %s, %d", _localhost->localHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
            forward = storeActiveSubscription(_localhost, fullID, strategy, RVFID, false);
            break;
        case UNSUBSCRIBE_SCOPE:
            //click_chatter("LocalProxy: received UNSUBSCRIBE_SCOPE request: %s, %s, %s, %d", _localhost->localHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
            forward = removeActiveSubscription(_localhost, fullID, strategy);
            break;
        case UNSUBSCRIBE_INFO:
            //click_chatter("LocalProxy: received UNSUBSCRIBE_INFO request: %s, %s, %s, %d", _localhost->localHostID.c_str(), ID.quoted_hex().c_str(), prefixID.quoted_hex().c_str(), (int) strategy);
            forward = removeActiveSubscription(_localhost, fullID, strategy);
            break;
        default:
            //click_chatter("LocalProxy: unknown request - skipping request - this should be something FATAL!");
            break;
    }
    return forward;
}

/*store the remote scope for the _publisher..forward the message to the RV point only if this is the first time the scope is published.
 If not, the RV point already knows about this node's publication...Note that RV points know only about network nodes - NOT for processes or click modules*/
bool LocalProxy::storeActivePublication(LocalHost *_publisher, String &fullID, unsigned char strategy, BABitvector &RVFID, bool isScope) {
    ActivePublication *ap;
    if ((strategy == NODE_LOCAL) || (strategy == DOMAIN_LOCAL)) {
        ap = activePublicationIndex.get(fullID);
        if (ap == activePublicationIndex.default_value()) {
            /*create the active scope's publication entry*/
            ap = new ActivePublication(fullID, strategy, isScope);
            ap->RVFID = RVFID;
            /*add the active scope's publication to the index*/
            activePublicationIndex.set(fullID, ap);
            /*update the local publishers of that active scope's publication*/
            ap->publishers.find_insert(_publisher, STOP_PUBLISH);
            /*update the active scope publications for this publsher*/
            _publisher->activePublications.find_insert(StringSetItem(fullID));
            //click_chatter("LocalProxy: store Active Scope Publication %s for local publisher %s", fullID.quoted_hex().c_str(), _publisher->publisherID.c_str());
            return true;
        } else {
            if (ap->strategy == strategy) {
                /*update the publishers of that remote scope*/
                ap->publishers.find_insert(_publisher, STOP_PUBLISH);
                /*update the published remote scopes for this publsher*/
                _publisher->activePublications.find_insert(StringSetItem(fullID));
                //click_chatter("LocalProxy: Active Scope Publication %s exists...updated for local publisher %s", fullID.quoted_hex().c_str(), _publisher->publisherID.c_str());
            } else {
                //click_chatter("LocalProxy: LocalRV: error while trying to update list of publishers for active publication %s..strategy mismatch", ap->fullID.quoted_hex().c_str());
            }
        }
    } else {
        //click_chatter("I am not doing anything with %s..the strategy is not NODE_LOCAL or DOMAIN_LOCAL", fullID.quoted_hex().c_str());
    }
    return false;
}

/*delete the remote publication for the _publisher..forward the message to the RV point only if there aren't any other publishers or subscribers for this scope*/
bool LocalProxy::removeActivePublication(LocalHost *_publisher, String &fullID, unsigned char strategy) {
    ActivePublication *ap;
    if ((strategy == NODE_LOCAL) || (strategy == DOMAIN_LOCAL)) {
        ap = activePublicationIndex.get(fullID);
        if (ap != activePublicationIndex.default_value()) {
            if (ap->strategy == strategy) {
                _publisher->activePublications.erase(fullID);
                ap->publishers.erase(_publisher);
                //click_chatter("LocalProxy: deleted publisher %s from Active Scope Publication %s", _publisher->publisherID.c_str(), fullID.quoted_hex().c_str());
                if (ap->publishers.size() == 0) {
                    //click_chatter("LocalProxy: delete Active Scope Publication %s", fullID.quoted_hex().c_str());
                    delete ap;
                    activePublicationIndex.erase(fullID);
                    return true;
                }
            } else {
                //click_chatter("LocalProxy: error while trying to delete active publication %s...strategy mismatch", ap->fullID.quoted_hex().c_str());
            }
        } else {
            //click_chatter("LocalProxy:%s is not an active publication", fullID.quoted_hex().c_str());
        }
    } else {
        //click_chatter("I am not doing anything with %s..the strategy is not NODE_LOCAL or DOMAIN_LOCAL", fullID.quoted_hex().c_str());
    }
    return false;
}

/*store the active scope for the _subscriber..forward the message to the RV point only if this is the first subscription for this scope.
 If not, the RV point already knows about this node's subscription...Note that RV points know only about network nodes - NOT about processes or click modules*/
bool LocalProxy::storeActiveSubscription(LocalHost *_subscriber, String &fullID, unsigned char strategy, BABitvector &RVFID, bool isScope) {
    ActiveSubscription *as;
    as = activeSubscriptionIndex.get(fullID);
    if (as == activeSubscriptionIndex.default_value()) {
        as = new ActiveSubscription(fullID, strategy, isScope);
        as->RVFID = RVFID;
        /*add the remote scope to the index*/
        activeSubscriptionIndex.set(fullID, as);
        /*update the subscribers of that remote scope*/
        as->subscribers.find_insert(LocalHostSetItem(_subscriber));
        /*update the subscribed remote scopes for this publsher*/
        _subscriber->activeSubscriptions.find_insert(StringSetItem(fullID));
        //click_chatter("LocalProxy: store Active Subscription %s for local subscriber %s", fullID.quoted_hex().c_str(), _subscriber->localHostID.c_str());
        if ((strategy != IMPLICIT_RENDEZVOUS) && (strategy != LINK_LOCAL) && (strategy != BROADCAST_IF)) {
            return true;
        } else {
            //click_chatter("I am not forwarding subscription for %s...strategy is %d", fullID.quoted_hex().c_str(), (int) strategy);
        }
    } else {
        if (as->strategy == strategy) {
            /*update the subscribers of that remote scope*/
            as->subscribers.find_insert(LocalHostSetItem(_subscriber));
            /*update the subscribed remote scopes for this publsher*/
            _subscriber->activeSubscriptions.find_insert(StringSetItem(fullID));
            //click_chatter("LocalProxy: Active Subscription %s exists...updated for local subscriber %s", fullID.quoted_hex().c_str(), _subscriber->localHostID.c_str());
        } else {
            //click_chatter("LocalProxy: error while trying to update list of subscribers for Active Subscription %s..strategy mismatch", as->fullID.quoted_hex().c_str());
        }
    }
    return false;
}

/*delete the remote scope for the _subscriber..forward the message to the RV point only if there aren't any other publishers or subscribers for this scope*/
bool LocalProxy::removeActiveSubscription(LocalHost *_subscriber, String &fullID, unsigned char strategy) {
    ActiveSubscription *as;
    as = activeSubscriptionIndex.get(fullID);
    if (as != activeSubscriptionIndex.default_value()) {
        if (as->strategy == strategy) {
            _subscriber->activeSubscriptions.erase(fullID);
            as->subscribers.erase(_subscriber);
            //click_chatter("LocalProxy: deleted subscriber %s from Active Subscription %s", _subscriber->localHostID.c_str(), fullID.quoted_hex().c_str());
            if (as->subscribers.size() == 0) {
                //click_chatter("LocalProxy: delete Active Subscription %s", fullID.quoted_hex().c_str());
                delete as;
                activeSubscriptionIndex.erase(fullID);
                if ((strategy != IMPLICIT_RENDEZVOUS) && (strategy != LINK_LOCAL) && (strategy != BROADCAST_IF)) {
                    return true;
                } else {
                    //click_chatter("I am not forwarding subscription for %s...strategy is %d", fullID.quoted_hex().c_str(), (int) strategy);
                }
            }
        } else {
            //click_chatter("LocalProxy: error while trying to delete Active Subscription %s...strategy mismatch", as->fullID.quoted_hex().c_str());
        }
    } else {
        //click_chatter("LocalProxy: no active subscriptions %s", fullID.quoted_hex().c_str());
    }
    return false;
}

void LocalProxy::handleRVNotification(Packet *p) {
    unsigned char type, numberOfIDs, IDLength/*in fragments of PURSUIT_ID_LEN each*/;
    unsigned int index = 0;
    Vector<String> IDs;
    ActivePublication *ap;
    bool shouldBreak = false;
    BABitvector FID;
    type = *(p->data());
    numberOfIDs = *(p->data() + sizeof (type));
    for (int i = 0; i < (int) numberOfIDs; i++) {
        IDLength = *(p->data() + sizeof (type) + sizeof (numberOfIDs) + index);
        IDs.push_back(String((const char *) (p->data() + sizeof (type) + sizeof (numberOfIDs) + sizeof (IDLength) + index), IDLength * PURSUIT_ID_LEN));
        index = index + sizeof (IDLength) + IDLength*PURSUIT_ID_LEN;
    }
    switch (type) {
        case SCOPE_PUBLISHED:
            //click_chatter("Received notification about new scope");
            /*Find the applications to forward the notification*/
            for (int i = 0; i < (int) numberOfIDs; i++) {
                /*check the active scope subscriptions for that*/
                /*prefix-match checking here*/
                /*I should create a set of local subscribers to notify*/
                LocalHostSet local_subscribers_to_notify;
                findActiveSubscriptions(IDs[i], local_subscribers_to_notify);
                for (LocalHostSetIter set_it = local_subscribers_to_notify.begin(); set_it != local_subscribers_to_notify.end(); set_it++) {
                    //click_chatter("LocalProxy: notifying Subscriber: %s", (*set_it)._lhpointer->localHostID.c_str());
                    /*send the message*/
                    sendNotificationLocally(SCOPE_PUBLISHED, (*set_it)._lhpointer, IDs[i]);
                }
            }
            break;
        case SCOPE_UNPUBLISHED:
            //click_chatter("Received notification about a deleted scope");
            /*Find the applications to forward the notification*/
            for (int i = 0; i < (int) numberOfIDs; i++) {
                /*check the active scope subscriptions for that*/
                /*prefix-match checking here*/
                /*I should create a set of local subscribers to notify*/
                LocalHostSet local_subscribers_to_notify;
                findActiveSubscriptions(IDs[i], local_subscribers_to_notify);
                for (LocalHostSetIter set_it = local_subscribers_to_notify.begin(); set_it != local_subscribers_to_notify.end(); set_it++) {
                    //click_chatter("LocalProxy: notifying Subscriber: %s", (*set_it)._lhpointer->localHostID.c_str());
                    /*send the message*/
                    sendNotificationLocally(SCOPE_UNPUBLISHED, (*set_it)._lhpointer, IDs[i]);
                }
            }
            break;
        case START_PUBLISH:
            FID = BABitvector(FID_LEN * 8);
            memcpy(FID._data, p->data() + sizeof (type) + sizeof (numberOfIDs) + index, FID_LEN);
            //click_chatter("LocalProxy: RECEIVED FID:%s\n", FID.to_string().c_str());
            for (int i = 0; i < (int) numberOfIDs; i++) {
                ap = activePublicationIndex.get(IDs[i]);
                if (ap != activePublicationIndex.default_value()) {
                    /*copy the IDs vector to the allKnownIDs vector of the ap*/
                    ap->allKnownIDs = IDs;
                    /*this item exists*/
                    ap->FID_to_subscribers = FID;
                    /*iterate once to see if any of the publishers for this item (which may be represented by many ids) is already notified*/
                    for (PublisherHashMapIter publishers_it = ap->publishers.begin(); publishers_it != ap->publishers.end(); publishers_it++) {
                        if ((*publishers_it).second == START_PUBLISH) {
                            //click_chatter("/*hmmm...this publisher has been previously notified*/");
                            shouldBreak = true;
                            break;
                        }
                    }
                    if (shouldBreak) {
                        break;
                    }
                }
            }
            if (!shouldBreak) {
                //click_chatter("/*none of the publishers has been previously notified*/");
                /*notify the first you find*/
                for (int i = 0; i < (int) numberOfIDs; i++) {
                    ap = activePublicationIndex.get(IDs[i]);
                    if (ap != activePublicationIndex.default_value()) {
                        /*iterate once to see if any of the publishers for this item (which may be represented by many ids) is already notified*/
                        for (PublisherHashMapIter publishers_it = ap->publishers.begin(); publishers_it != ap->publishers.end(); publishers_it++) {
                            (*publishers_it).second = START_PUBLISH;
                            sendNotificationLocally(START_PUBLISH, (*publishers_it).first, IDs[i]);
                            shouldBreak = true;
                            break;
                        }
                    }
                    if (shouldBreak) {
                        break;
                    }
                }
            }
            break;
        case STOP_PUBLISH:
            //click_chatter("LocalProxy: Received NULL FID");
            for (int i = 0; i < (int) numberOfIDs; i++) {
                ap = activePublicationIndex.get(IDs[i]);
                if (ap != activePublicationIndex.default_value()) {
                    ap->allKnownIDs = IDs;
                    /*update the FID to the all zero FID*/
                    ap->FID_to_subscribers = BABitvector(FID_LEN * 8);
                    /*iterate once to see if any the publishers for this item (which may be represented by many ids) is already notified*/
                    for (PublisherHashMapIter publishers_it = ap->publishers.begin(); publishers_it != ap->publishers.end(); publishers_it++) {
                        if ((*publishers_it).second == START_PUBLISH) {
                            (*publishers_it).second = STOP_PUBLISH;
                            sendNotificationLocally(STOP_PUBLISH, (*publishers_it).first, IDs[i]);
                        }
                    }
                }
            }
            break;
        default:
            //click_chatter("LocalProxy: FATAL - didn't understand the RV notification");
            break;
    }
}

void LocalProxy::pushDataToLocalSubscriber(LocalHost *_localhost, String &ID, Packet *p /*p contains only the data and has some headroom as well*/) {
    unsigned char IDLength;
    unsigned char type = PUBLISHED_DATA;
    WritablePacket *newPacket;
    IDLength = ID.length() / PURSUIT_ID_LEN;
    //click_chatter("pushing data to subscriber %s", _localhost->localHostID.c_str());
    newPacket = p->push(sizeof (unsigned char) + sizeof (unsigned char) +ID.length());
    memcpy(newPacket->data(), &type, sizeof (unsigned char));
    memcpy(newPacket->data() + sizeof (unsigned char), &IDLength, sizeof (unsigned char));
    memcpy(newPacket->data() + sizeof (unsigned char) + sizeof (unsigned char), ID.c_str(), ID.length());
    if (_localhost->type == CLICK_ELEMENT) {
        output(_localhost->id).push(newPacket);
    } else {
        newPacket->set_anno_u32(0, _localhost->id);
        output(0).push(newPacket);
    }
}

/*this method is quite different from the one above
it will forward the data using the provided FID
Here, we only know about a single ID.We do not care if there are multiple IDs*/
void LocalProxy::pushDataToRemoteSubscribers(Vector<String> &IDs, BABitvector &FID_to_subscribers, Packet *p) {
    WritablePacket *newPacket;
    unsigned char IDLength = 0;
    int index;
    unsigned char numberOfIDs;
    int totalIDsLength = 0;
    Vector<String>::iterator it;
    numberOfIDs = (unsigned char) IDs.size();
    for (it = IDs.begin(); it != IDs.end(); it++) {
        totalIDsLength = totalIDsLength + (*it).length();
    }
    newPacket = p->push(FID_LEN + sizeof (numberOfIDs) /*number of ids*/+((int) numberOfIDs) * sizeof (unsigned char) /*id length*/ +totalIDsLength);
    memcpy(newPacket->data(), FID_to_subscribers._data, FID_LEN);
    memcpy(newPacket->data() + FID_LEN, &numberOfIDs, sizeof (numberOfIDs));
    index = 0;
    it = IDs.begin();
    for (int i = 0; i < (int) numberOfIDs; i++) {
        IDLength = (unsigned char) (*it).length() / PURSUIT_ID_LEN;
        memcpy(newPacket->data() + FID_LEN + sizeof (numberOfIDs) + index, &IDLength, sizeof (IDLength));
        memcpy(newPacket->data() + FID_LEN + sizeof (numberOfIDs) + index + sizeof (IDLength), (*it).c_str(), (*it).length());
        index = index + sizeof (IDLength) + (*it).length();
        it++;
    }
    output(2).push(newPacket);
}

void LocalProxy::handleNetworkPublication(Vector<String> &IDs, Packet *p /*the packet has some headroom and only the data which hasn't been copied yet*/) {
    LocalHostStringHashMap localSubscribers;
    int counter = 1;
    //click_chatter("received data for ID: %s", IDs[0].quoted_hex().c_str());
    bool foundLocalSubscribers = findLocalSubscribers(IDs, localSubscribers);
    int localSubscribersSize = localSubscribers.size();
    if (foundLocalSubscribers) {
        for (LocalHostStringHashMapIter localSubscribers_it = localSubscribers.begin(); localSubscribers_it != localSubscribers.end(); localSubscribers_it++) {
            if (counter == localSubscribersSize) {
                /*don't clone the packet since this is the last subscriber*/
                pushDataToLocalSubscriber((*localSubscribers_it).first, (*localSubscribers_it).second, p);
            } else {
                pushDataToLocalSubscriber((*localSubscribers_it).first, (*localSubscribers_it).second, p->clone()->uniqueify());
            }
            counter++;
        }
    } else {
        p->kill();
    }
}

void LocalProxy::handleUserPublication(String &ID, Packet *p /*the packet has some headroom and only the data which hasn't been copied yet*/, LocalHost *__localhost) {
    int localSubscribersSize;
    int counter = 1;
    bool remoteSubscribersExist = true;
    bool useFatherFID = false;
    Vector<String> IDs;
    LocalHostStringHashMap localSubscribers;
    ActivePublication *ap = activePublicationIndex.get(ID);
    if (ap == activePublicationIndex.default_value()) {
        /*check a FID is assigned to the father item - used for fragmentation*/
        ap = activePublicationIndex.get(ID.substring(0, ID.length() - PURSUIT_ID_LEN));
        useFatherFID = true;
        IDs.push_back(ID);
    }
    if (ap != activePublicationIndex.default_value()) {
        if ((ap->FID_to_subscribers.zero()) || (ap->FID_to_subscribers == gc->iLID)) {
            remoteSubscribersExist = false;
        }
        /*I have to find any subscribers that exist locally*/
        /*Careful: I will use all known IDs of the ap and check for each one (findLocalSubscribers() does that)*/
        if (useFatherFID == true) {
            findLocalSubscribers(IDs, localSubscribers);
        } else {
            findLocalSubscribers(ap->allKnownIDs, localSubscribers);
        }
        localSubscribers.erase(__localhost);
        localSubscribersSize = localSubscribers.size();

        /*Now I know if I should send the packet to the Network and how many local subscribers exist*/
        /*I should be able to minimise packet copy*/
        if ((localSubscribersSize == 0) && (!remoteSubscribersExist)) {
            p->kill();
        } else if ((localSubscribersSize == 0) && (remoteSubscribersExist)) {
            /*no need to clone..packet will be sent only to the network*/
            if (useFatherFID == true) {
                pushDataToRemoteSubscribers(IDs, ap->FID_to_subscribers, p);
            } else {
                pushDataToRemoteSubscribers(ap->allKnownIDs, ap->FID_to_subscribers, p);
            }
        } else if ((localSubscribersSize > 0) && (!remoteSubscribersExist)) {
            /*only local subscribers exist*/
            for (LocalHostStringHashMapIter localSubscribers_it = localSubscribers.begin(); localSubscribers_it != localSubscribers.end(); localSubscribers_it++) {
                LocalHost *_localhost = (*localSubscribers_it).first;
                if (counter == localSubscribersSize) {
                    /*don't clone the packet since this is the last subscriber*/
                    pushDataToLocalSubscriber(_localhost, ID, p);
                } else {
                    pushDataToLocalSubscriber(_localhost, ID, p->clone()->uniqueify());
                }
                counter++;
            }
        } else {
            /*local and remote subscribers exist*/
            if (useFatherFID == true) {
                pushDataToRemoteSubscribers(IDs, ap->FID_to_subscribers, p->clone()->uniqueify());
            } else {
                pushDataToRemoteSubscribers(ap->allKnownIDs, ap->FID_to_subscribers, p->clone()->uniqueify());
            }
            for (LocalHostStringHashMapIter localSubscribers_it = localSubscribers.begin(); localSubscribers_it != localSubscribers.end(); localSubscribers_it++) {
                LocalHost *_localhost = (*localSubscribers_it).first;
                if (counter == localSubscribersSize) {
                    /*don't clone the packet since this is the last subscriber*/
                    pushDataToLocalSubscriber(_localhost, ID, p);
                } else {
                    pushDataToLocalSubscriber(_localhost, ID, p->clone()->uniqueify());
                }
                counter++;
            }
        }
    } else {
        p->kill();
    }
}

/*this method is quite different...I will check if there is any active subscription for that item.
 * if there is one, I will also forward the data to the local subscribers
 * If not I will forward the data to the network using the application provided FID*/
void LocalProxy::handleUserPublication(String &ID, BABitvector &FID_to_subscribers, Packet *p, LocalHost *__localhost) {
    int counter = 1;
    int localSubscribersSize;
    LocalHostStringHashMap localSubscribers;
    Vector<String> IDs;
    ActivePublication *ap = activePublicationIndex.get(ID.substring(0, ID.length() - PURSUIT_ID_LEN));
    /*i will augment the IDs vector using my father publication*/
    if (ap != activePublicationIndex.default_value()) {
        for (int i = 0; i < ap->allKnownIDs.size(); i++) {
            String knownID = ap->allKnownIDs[i] + ID.substring(ID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
            if (knownID.compare(ID) != 0) {
                IDs.push_back(knownID);
            }
        }
    }
    IDs.push_back(ID);
    /*I have to find any subscribers that exist locally*/
    findLocalSubscribers(IDs, localSubscribers);
    localSubscribers.erase(__localhost);
    localSubscribersSize = localSubscribers.size();
    if (localSubscribersSize == 0) {
        /*no need to clone..packet will be sent only to the network*/
        pushDataToRemoteSubscribers(IDs, FID_to_subscribers, p);
    } else {
        /*local and remote subscribers exist*/
        pushDataToRemoteSubscribers(IDs, FID_to_subscribers, p->clone()->uniqueify());
        for (LocalHostStringHashMapIter localSubscribers_it = localSubscribers.begin(); localSubscribers_it != localSubscribers.end(); localSubscribers_it++) {
            LocalHost *_localhost = (*localSubscribers_it).first;
            if (counter == localSubscribersSize) {
                //click_chatter("/*don't clone the packet since this is the last subscriber*/");
                pushDataToLocalSubscriber(_localhost, ID, p);
            } else {
                pushDataToLocalSubscriber(_localhost, ID, p->clone()->uniqueify());
            }
            counter++;
        }
    }
}

/*sends the pub/sub request to the local or remote RV*/
void LocalProxy::publishReqToRV(Packet *p, BABitvector &RVFID) {
    WritablePacket *p1, *p2;
    if ((RVFID.zero()) || (RVFID == gc->iLID)) {
        /*this should be a request to the RV element running locally*/
        /*This node is the RV point for this request*/
        /*interact using the API - differently than below*/
        /*these events are going to be PUBLISHED_DATA*/
        unsigned char typeOfAPIEvent = PUBLISHED_DATA;
        unsigned char IDLengthOfAPIEvent = gc->nodeRVScope.length() / PURSUIT_ID_LEN;
        /***********************************************************/
        p1 = p->push(sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + gc->nodeRVScope.length());
        memcpy(p1->data(), &typeOfAPIEvent, sizeof (typeOfAPIEvent));
        memcpy(p1->data() + sizeof (typeOfAPIEvent), &IDLengthOfAPIEvent, sizeof (IDLengthOfAPIEvent));
        memcpy(p1->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent), gc->nodeRVScope.c_str(), gc->nodeRVScope.length());
        output(1).push(p1);
    } else {
        /*wrap the request to a publication to /FFFFFFFF/NODE_ID  */
        /*Format: numberOfIDs = (unsigned char) 1, numberOfFragments1 = (unsigned char) 2, ID1 = /FFFFFFFF/NODE_ID*/
        /*this should be a request to the RV element running in some other node*/
        //click_chatter("I will send the request to the domain RV using the FID: %s", RVFID.to_string().c_str());
        unsigned char numberOfIDs = 1;
        unsigned char numberOfFragments = 2;
        /*push the "header" - see above*/
        p1 = p->push(sizeof (unsigned char) + 1 * sizeof (unsigned char) + 2 * PURSUIT_ID_LEN);
        memcpy(p1->data(), &numberOfIDs, sizeof (unsigned char));
        memcpy(p1->data() + sizeof (unsigned char), &numberOfFragments, sizeof (unsigned char));
        memcpy(p1->data() + sizeof (unsigned char) + sizeof (unsigned char), gc->nodeRVScope.c_str(), gc->nodeRVScope.length());
        p2 = p1->push(FID_LEN);
        memcpy(p2->data(), RVFID._data, FID_LEN);
        output(2).push(p2);
    }
}

void LocalProxy::findActiveSubscriptions(String &ID, LocalHostSet &local_subscribers_to_notify) {
    ActiveSubscription *as;
    LocalHostSetIter set_it;
    as = activeSubscriptionIndex.get(ID.substring(0, ID.length() - PURSUIT_ID_LEN));
    if (as != activeSubscriptionIndex.default_value()) {
        if (as->isScope) {
            for (set_it = as->subscribers.begin(); set_it != as->subscribers.end(); set_it++) {
                local_subscribers_to_notify.find_insert(*set_it);
            }
        }
    }
}

bool LocalProxy::findLocalSubscribers(Vector<String> &IDs, LocalHostStringHashMap & _localSubscribers) {
    bool foundSubscribers;
    String knownID;
    LocalHostSetIter set_it;
    Vector<String>::iterator id_it;
    ActiveSubscription *as;
    foundSubscribers = false;
    /*prefix-match checking here for all known IDS of aiip*/
    for (id_it = IDs.begin(); id_it != IDs.end(); id_it++) {
        knownID = *id_it;
        /*check for local subscription for the specific information item*/
        as = activeSubscriptionIndex.get(knownID);
        if (as != activeSubscriptionIndex.default_value()) {
            for (set_it = as->subscribers.begin(); set_it != as->subscribers.end(); set_it++) {
                _localSubscribers.set((*set_it)._lhpointer, knownID);
                foundSubscribers = true;
            }
        }
        as = activeSubscriptionIndex.get(knownID.substring(0, knownID.length() - PURSUIT_ID_LEN));
        if (as != activeSubscriptionIndex.default_value()) {
            for (set_it = as->subscribers.begin(); set_it != as->subscribers.end(); set_it++) {
                _localSubscribers.set((*set_it)._lhpointer, knownID);
                foundSubscribers = true;
            }
        }
    }
    return foundSubscribers;
}

void LocalProxy::sendNotificationLocally(unsigned char type, LocalHost *_localhost, String ID) {
    WritablePacket *p;
    unsigned char IDLength;
    p = Packet::make(30, NULL, sizeof (unsigned char) /*type*/ + sizeof (unsigned char) /*id length*/ +ID.length() /*id*/, 0);
    IDLength = ID.length() / PURSUIT_ID_LEN;
    memcpy(p->data(), &type, sizeof (char));
    memcpy(p->data() + sizeof (unsigned char), &IDLength, sizeof (unsigned char));
    memcpy(p->data() + sizeof (unsigned char) + sizeof (unsigned char), ID.c_str(), IDLength * PURSUIT_ID_LEN);
    if (_localhost->type == CLICK_ELEMENT) {
        output(_localhost->id).push(p);
    } else {
        /*set the annotation for the to_netlink element*/
        p->set_anno_u32(0, _localhost->id);
        //click_chatter("setting annotation: %d", _localhost->id);
        output(0).push(p);
    }
}

void LocalProxy::createAndSendPacketToRV(unsigned char type, unsigned char IDLength /*in fragments of PURSUIT_ID_LEN each*/, String &ID, unsigned char prefixIDLength /*in fragments of PURSUIT_ID_LEN each*/, String &prefixID, BABitvector &RVFID, unsigned char strategy) {
    WritablePacket *p;
    unsigned char numberOfIDs = 1;
    unsigned char numberOfFragments = 2;
    if ((RVFID.zero()) || (RVFID == gc->iLID)) {
        unsigned char typeOfAPIEvent = PUBLISHED_DATA;
        unsigned char IDLengthOfAPIEvent = gc->nodeRVScope.length() / PURSUIT_ID_LEN;
        /***********************************************************/
        p = Packet::make(50, NULL, sizeof (numberOfIDs) + 1 * sizeof (numberOfFragments) + 2 * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength) + IDLength * PURSUIT_ID_LEN + sizeof (prefixIDLength) + prefixIDLength * PURSUIT_ID_LEN + sizeof (strategy), 50);
        /*the local RV should always be in output port 1*/
        memcpy(p->data(), &typeOfAPIEvent, sizeof (typeOfAPIEvent));
        memcpy(p->data() + sizeof (typeOfAPIEvent), &IDLengthOfAPIEvent, sizeof (IDLengthOfAPIEvent));
        memcpy(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent), gc->nodeRVScope.c_str(), gc->nodeRVScope.length());
        memcpy(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + gc->nodeRVScope.length(), &type, sizeof (type));
        memcpy(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + gc->nodeRVScope.length() + sizeof (type), &IDLength, sizeof (IDLength));
        memcpy(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + gc->nodeRVScope.length() + sizeof (type) + sizeof (IDLength), ID.c_str(), ID.length());
        memcpy(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + gc->nodeRVScope.length() + sizeof (type) + sizeof (IDLength) + ID.length(), &prefixIDLength, sizeof (prefixIDLength));
        memcpy(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + gc->nodeRVScope.length() + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (prefixIDLength), prefixID.c_str(), prefixID.length());
        memcpy(p->data() + sizeof (typeOfAPIEvent) + sizeof (IDLengthOfAPIEvent) + gc->nodeRVScope.length() + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (prefixIDLength) + prefixID.length(), &strategy, sizeof (strategy));
        output(1).push(p);
    } else {
        p = Packet::make(50, NULL, FID_LEN + sizeof (numberOfIDs) + 1 * sizeof (numberOfFragments) + 2 * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength) + IDLength * PURSUIT_ID_LEN + sizeof (prefixIDLength) + prefixIDLength * PURSUIT_ID_LEN + sizeof (strategy), 0);
        memcpy(p->data(), RVFID._data, FID_LEN);
        memcpy(p->data() + FID_LEN, &numberOfIDs, sizeof (unsigned char));
        memcpy(p->data() + FID_LEN + sizeof (unsigned char), &numberOfFragments, sizeof (unsigned char));
        memcpy(p->data() + FID_LEN + sizeof (unsigned char) + sizeof (unsigned char), gc->nodeRVScope.c_str(), 2 * PURSUIT_ID_LEN);
        memcpy(p->data() + FID_LEN + sizeof (unsigned char) + sizeof (unsigned char) + 2 * PURSUIT_ID_LEN, &type, sizeof (type));
        memcpy(p->data() + FID_LEN + sizeof (unsigned char) + sizeof (unsigned char) + 2 * PURSUIT_ID_LEN + sizeof (type), &IDLength, sizeof (IDLength));
        memcpy(p->data() + FID_LEN + sizeof (unsigned char) + sizeof (unsigned char) + 2 * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength), ID.c_str(), ID.length());
        memcpy(p->data() + FID_LEN + sizeof (unsigned char) + sizeof (unsigned char) + 2 * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength) + ID.length(), &prefixIDLength, sizeof (prefixIDLength));
        memcpy(p->data() + FID_LEN + sizeof (unsigned char) + sizeof (unsigned char) + 2 * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (prefixIDLength), prefixID.c_str(), prefixID.length());
        memcpy(p->data() + FID_LEN + sizeof (unsigned char) + sizeof (unsigned char) + 2 * PURSUIT_ID_LEN + sizeof (type) + sizeof (IDLength) + ID.length() + sizeof (prefixIDLength) + prefixID.length(), &strategy, sizeof (strategy));
        output(2).push(p);
    }
}

void LocalProxy::deleteAllActiveInformationItemPublications(LocalHost * _publisher) {
    unsigned char type, IDLength /*in fragments of PURSUIT_ID_LEN each*/, prefixIDLength /*in fragments of PURSUIT_ID_LEN each*/;
    String ID, prefixID;
    bool shouldNotify = false;
    WritablePacket *newPacket;
    int size = _publisher->activePublications.size();
    StringSetIter it = _publisher->activePublications.begin();
    for (int i = 0; i < size; i++) {
        shouldNotify = false;
        ActivePublication *ap = activePublicationIndex.get((*it)._strData);
        if (!ap->isScope) {
            it = _publisher->activePublications.erase(it);
            if (ap->publishers.get(_publisher) != STOP_PUBLISH) {
                shouldNotify = true;
            }
            ap->publishers.erase(_publisher);
            //click_chatter("LocalProxy: deleted publisher %s from Active Information Item Publication %s", _publisher->localHostID.c_str(), ap->fullID.quoted_hex().c_str());
            if (ap->publishers.size() == 0) {
                //click_chatter("LocalProxy: delete Active Information item Publication %s", ap->fullID.quoted_hex().c_str());
                activePublicationIndex.erase(ap->fullID);
                /*notify the RV Function - depending on strategy*/
                type = UNPUBLISH_INFO;
                IDLength = 1;
                prefixIDLength = (ap->fullID.length() - PURSUIT_ID_LEN) / PURSUIT_ID_LEN;
                ID = ap->fullID.substring(ap->fullID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                prefixID = ap->fullID.substring(0, ap->fullID.length() - PURSUIT_ID_LEN);
                createAndSendPacketToRV(type, IDLength, ID, prefixIDLength, prefixID, ap->RVFID, ap->strategy);
                delete ap;
            } else {
                /*there are other local publishers...check the state of the deleted local publisher and potentially notify one of the other local publishers*/
                if (shouldNotify) {
                    /*None of the available local publishers has been previously notified*/
                    (*ap->publishers.begin()).second = START_PUBLISH;
                    IDLength = ap->fullID.length() / PURSUIT_ID_LEN;
                    newPacket = Packet::make(30, NULL, sizeof (unsigned char) /*type*/ + sizeof (unsigned char) /*id length*/ +ap->fullID.length() /*id*/, 0);
                    newPacket->set_anno_u32(0, (*ap->publishers.begin()).first->id);
                    type = START_PUBLISH;
                    memcpy(newPacket->data(), &type, sizeof (char));
                    memcpy(newPacket->data() + sizeof (unsigned char), &IDLength, sizeof (unsigned char));
                    memcpy(newPacket->data() + sizeof (unsigned char) + sizeof (unsigned char), ap->fullID.c_str(), ap->fullID.length());
                    if ((*ap->publishers.begin()).first->type == CLICK_ELEMENT) {
                        output((*ap->publishers.begin()).first->id).push(newPacket);
                    } else {
                        output(0).push(newPacket);
                    }
                }
            }
        } else {
            it++;
        }
    }
}

void LocalProxy::deleteAllActiveInformationItemSubscriptions(LocalHost * _subscriber) {
    unsigned char type, IDLength /*in fragments of PURSUIT_ID_LEN each*/, prefixIDLength /*in fragments of PURSUIT_ID_LEN each*/;
    String ID, prefixID;
    int size = _subscriber->activeSubscriptions.size();
    StringSetIter it = _subscriber->activeSubscriptions.begin();
    for (int i = 0; i < size; i++) {
        ActiveSubscription *as = activeSubscriptionIndex.get((*it)._strData);
        if (!as->isScope) {
            it = _subscriber->activeSubscriptions.erase(it);
            as->subscribers.erase(_subscriber);
            //click_chatter("LocalProxy: deleted subscriber %s from Active Information Item Publication %s", _subscriber->localHostID.c_str(), as->fullID.quoted_hex().c_str());
            if (as->subscribers.size() == 0) {
                //click_chatter("LocalProxy: delete Active Information item Subscription %s", as->fullID.quoted_hex().c_str());
                activeSubscriptionIndex.erase(as->fullID);
                if ((as->strategy != IMPLICIT_RENDEZVOUS) && (as->strategy != LINK_LOCAL) && (as->strategy != BROADCAST_IF)) {
                    /*notify the RV Function - depending on strategy*/
                    type = UNSUBSCRIBE_INFO;
                    IDLength = 1;
                    prefixIDLength = (as->fullID.length() - PURSUIT_ID_LEN) / PURSUIT_ID_LEN;
                    ID = as->fullID.substring(as->fullID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                    prefixID = as->fullID.substring(0, as->fullID.length() - PURSUIT_ID_LEN);
                    createAndSendPacketToRV(type, IDLength, ID, prefixIDLength, prefixID, as->RVFID, as->strategy);
                }
                delete as;
            }
        } else {
            it++;
        }
    }
}

void LocalProxy::deleteAllActiveScopePublications(LocalHost * _publisher) {
    int max_level = 0;
    int temp_level;
    unsigned char type, IDLength/*in fragments of PURSUIT_ID_LEN each*/, prefixIDLength/*in fragments of PURSUIT_ID_LEN each*/;
    String ID, prefixID;
    StringSetIter it;
    for (it = _publisher->activePublications.begin(); it != _publisher->activePublications.end(); it++) {
        ActivePublication *ap = activePublicationIndex.get((*it)._strData);

        if (ap->isScope) {
            String temp_id = (*it)._strData;
            temp_level = temp_id.length() / PURSUIT_ID_LEN;
            if (temp_level > max_level) {
                max_level = temp_level;
            }
        }
    }
    for (int i = max_level; i > 0; i--) {
        it = _publisher->activePublications.begin();
        int size = _publisher->activePublications.size();
        for (int j = 0; j < size; j++) {
            String fullID = (*it)._strData;
            it++;
            if (fullID.length() / PURSUIT_ID_LEN == i) {
                ActivePublication *ap = activePublicationIndex.get(fullID);
                if (ap->isScope) {
                    _publisher->activePublications.erase(fullID);
                    ap->publishers.erase(_publisher);
                    //click_chatter("LocalProxy: deleted publisher %s from Active Scope Publication %s", _publisher->localHostID.c_str(), ap->fullID.quoted_hex().c_str());
                    if (ap->publishers.size() == 0) {
                        //click_chatter("LocalProxy: delete Active Scope Publication %s", ap->fullID.quoted_hex().c_str());
                        activePublicationIndex.erase(ap->fullID);
                        /*notify the RV Function - depending on strategy*/
                        type = UNPUBLISH_SCOPE;
                        IDLength = 1;
                        prefixIDLength = (ap->fullID.length() - PURSUIT_ID_LEN) / PURSUIT_ID_LEN;
                        ID = ap->fullID.substring(ap->fullID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                        prefixID = ap->fullID.substring(0, ap->fullID.length() - PURSUIT_ID_LEN);
                        createAndSendPacketToRV(type, IDLength, ID, prefixIDLength, prefixID, ap->RVFID, ap->strategy);
                        delete ap;
                    }
                }
            }
        }
    }
}

void LocalProxy::deleteAllActiveScopeSubscriptions(LocalHost * _subscriber) {
    int max_level = 0;
    int temp_level;
    unsigned char type, IDLength /*in fragments of PURSUIT_ID_LEN each*/, prefixIDLength /*in fragments of PURSUIT_ID_LEN each*/;
    String ID, prefixID;
    StringSetIter it;
    for (it = _subscriber->activeSubscriptions.begin(); it != _subscriber->activeSubscriptions.end(); it++) {
        ActiveSubscription *as = activeSubscriptionIndex.get((*it)._strData);
        if (as->isScope) {
            String temp_id = (*it)._strData;
            temp_level = temp_id.length() / PURSUIT_ID_LEN;
            if (temp_level > max_level) {
                max_level = temp_level;
            }
        }
    }
    for (int i = max_level; i > 0; i--) {
        it = _subscriber->activeSubscriptions.begin();
        int size = _subscriber->activeSubscriptions.size();
        for (int j = 0; j < size; j++) {
            String fullID = (*it)._strData;
            it++;
            if (fullID.length() / PURSUIT_ID_LEN == i) {
                ActiveSubscription *as = activeSubscriptionIndex.get(fullID);
                if (as->isScope) {
                    _subscriber->activeSubscriptions.erase(fullID);
                    as->subscribers.erase(_subscriber);
                    //click_chatter("LocalProxy: deleted subscriber %s from Active Scope Subscription %s", _subscriber->localHostID.c_str(), as->fullID.quoted_hex().c_str());
                    if (as->subscribers.size() == 0) {
                        //click_chatter("LocalProxy: delete Active Scope Subscription %s", as->fullID.quoted_hex().c_str());
                        activeSubscriptionIndex.erase(as->fullID);
                        /*notify the RV Function - depending on strategy*/
                        if ((as->strategy != IMPLICIT_RENDEZVOUS) && (as->strategy != LINK_LOCAL) && (as->strategy != BROADCAST_IF)) {
                            type = UNSUBSCRIBE_SCOPE;
                            IDLength = 1;
                            prefixIDLength = (as->fullID.length() - PURSUIT_ID_LEN) / PURSUIT_ID_LEN;
                            ID = as->fullID.substring(as->fullID.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                            prefixID = as->fullID.substring(0, as->fullID.length() - PURSUIT_ID_LEN);
                            createAndSendPacketToRV(type, IDLength, ID, prefixIDLength, prefixID, as->RVFID, as->strategy);
                        }
                        delete as;
                    }
                }
            }
        }
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(LocalProxy)
