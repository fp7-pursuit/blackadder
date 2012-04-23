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
#ifndef CLICK_LOCALPROXY_HH
#define CLICK_LOCALPROXY_HH

#include "globalconf.hh"
#include "localhost.hh"
#include "activepub.hh"
#include <click/router.hh>

CLICK_DECLS

class LocalHost;

/**@brief (blackadder Core) The LocalProxy Element is the core element in a Blackadder Node.
 * 
 * All Click packets received by the Core component are annotated with an application identifier by the FromNetlink Element. 
 * Click packets received by other Click elements are annotated with the Click port with which the LocaLproxy element is connected.
 * A further role is that of providing a proxy function to all publishers and subscribers. 
 * Rendezvous nodes (even the one running in the same node) do not know about individual application identifiers or click elements.
 * Instead, a statistically unique node label that identifies the network node from which a request was sent is stored by the LocalProxy.
 */
class LocalProxy : public Element {
public:
    /**
     * @brief Constructor: it does nothing - as Click suggests
     * @return 
     */
    LocalProxy();
    /**
     * @brief Destructor: it does nothing - as Click suggests
     * @return 
     */
    ~LocalProxy();
    /**
     * @brief the class name - required by Click
     * @return 
     */
    const char *class_name() const {return "LocalProxy";}
    /**
     * @brief the port count - required by Click - it can have multiple input and output ports that are connected with the: FromNetlink, ToNetlink, Forwarder, LocalRV, and other extra Click Elements.
     * @return 
     */
    const char *port_count() const {return "-/-";}
    /**
     * @brief a PUSH Element.
     * @return PUSH
     */
    const char *processing() const {return PUSH;}
    /**
     * @brief Element configuration. LocalProxy needs only a pointer to the GlovalConf Element so that it can read the Global Configuration.
     */
    int configure(Vector<String>&, ErrorHandler*);
    /**@brief This Element must be configured AFTER the GlobalConf Element
     * @return the correct number so that it is configured afterwards
     */
    int configure_phase() const {return 200;}
    /**
     * @brief This method is called by Click when the Element is about to be initialized. There is nothing that needs initialization though.
     * @param errh
     * @return 
     */
    int initialize(ErrorHandler *errh);
    /**@brief Cleanups everything. 
     * 
     * If stage >= CLEANUP_ROUTER_INITIALIZED (i.e. the Element was initialized) LocalProxy will delete all stored ActivePublication, ActiveSubscription and LocalHost.
     */
    void cleanup(CleanupStage stage);
    /**@brief This method is called by Click whenever a packet is pushed to the LocalProxy by some other Element.
     * 
     * We distinct the following cases:
     * 
     * 1) A Packet is pushed by the ToNetlink Element. An application has sent a pub/sub request. See helper.hh for possible cases.
     * LocalProxy first finds or creates the respective LocalHost object and then inspects the type of the request:
     * 
     *  a) DISCONNECT: calls the disconnect method, kills the packet and returns.
     * 
     *  b) PUBLISH_DATA: Calls handleUserPublication() method. A special case is when the LocalRV publishes an RV/TM notification (e.g. in a NODE_LOCAL case or in a DOMAIN_LOCAL in the domain's rendezvous node).
     * 
     *  c) All other requests: Reads all fields exported to the API and calls the handleLocalRequest() method. 
     * Then, depending on the output of this method it may publish the user request to a remote Blackadder node (a rendezvous point) using the publishReqToRV() method or not (the packet is killed).
     * 
     * 
     * 2) A Packet is pushed by another Click Element (e.g. the LocalRV Element). This case is the same as the 1st. The only difference is that the LocalHost is a Click Element. 
     * In both of these cases requests are formatted according to the exported service model are expected.
     * 
     * 3) A Packet is pushed by the Forwarder Element. This is a network publication.
     * Network publications may have multiple information identifiers. LocalProxy reads them all and pulls everything from the packet except from the payload. Then, it calls the handleNetworkPublication() method.
     * A special case is when the publication identifier is the /FFFFFFFFFFFFFFFD/NODEID. 
     * This publications are RV/TM notifications, therefore the processRVNotification() method is called (after pulling everything from the packet except from the payload, which in this case is the notification).
     * Then the Packet is deleted (p->kill()).
     * 
     * @param port the port from which the packet was pushed
     * @param p a pointer to the packet
     */
    void push(int port, Packet *p);
    /**@brief It looks into the local_pub_sub_Index for a LocalHost (a publisher or subscriber).
     * 
     * @param type_of_publisher the type of the publisher. LOCAL_PROCESS for an application or kernel module and CLICK_ELEMENT for a Click Element.
     * @param publisher_identifier the identifier of the publisher. See LocalHost for details.
     * @return an existing LocalHost or a constructed object.
     */
    LocalHost * getLocalHost(int type_of_publisher, int publisher_identifier);
    /**@brief On behalf of the LocalHost (application or Click element) the LocalProxy will undo all AcivePublication and ActiveSubscription.
     * 
     * This has to be done in a way that after this method call, _localhost is like it never existed.
     * 
     * @param _localhost
     */
    void disconnect(LocalHost *_localhost);
    /** @brief It handles a pub/sub request sent by a LocalHost. Depending on the request it calls storeActivePublication(), removeActivePublication(), storeActiveSubscription() and removeActiveSubscription() methods.
     * 
     * These methods do not care about the information hierarchy so they accept a single full Identifier that is constructed appropriately.
     * @param type The type of the request. Can be one of the PUBLISH_SCOPE, PUBLISH_INFO, UNPUBLISH_SCOPE, UNPUBLISH_INFO, SUBSCRIBE_SCOPE, SUBSCRIBE_INFO, UNSUBSCRIBE_SCOPE and UNSUBSCRIBE_INFO.
     * @param _localhost A pointer to the LocalHost object
     * @param ID This identifier can either be a single fragment, identifying a scope or an information item in the context of the scope identified by prefixID, or a full identifier (e.g. when republishing scopes and information items).
     * @param prefixID The Identifier of the parent scope
     * @param strategy The assigned dissemination strategy.
     * @param RVFID The LIPSIN identifier to the rendezvous node. Note that it may be the internal link identifier (e.g. in a NODE_LOCAL strategy).
     * @return a boolean value denoting if this request should be sent to the rendezvous node or not. For instance, if another application has previously sent the same request, the rendezvous node is already aware. 
     * Note that the rendezvous node does NOT know about applications or Click elements. It only knows about Blackadder node labels. So two applications requesting the same thing does not concern the rendezvous point.
     */
    bool handleLocalRequest(unsigned char &type, LocalHost *_localhost, String &ID, String &prefixID, unsigned char &strategy, BABitvector &RVFID);
    /** @brief The behaviour of this method is strategy specific. It should store to the LocalProxy some state about the fact that an ActivePublication is assigned to a publisher (LocalHost).
     * 
     * The ActivePublication is potentially allocated and the publisher is assigned to it. Depending on the strategy, the request may be forwarded to a rendezvous node (even locally) or not.
     * @param _localhost A pointer to the LocalHost object.
     * @param fullID a reference to the full identifier of the ActivePublication.
     * @param strategy the dissemination strategy.
     * @param RVFID The LIPSIN identifier to the rendezvous node. Note that it may be the internal link identifier (e.g. in a NODE_LOCAL strategy).
     * @param isScope a boolean value denoting if the ActivePublications refers to a scope or not.
     * @return True if the request must be forwarded to the rendezvous node. The result is strategy specific.
     */
    bool storeActivePublication(LocalHost *_localhost, String &fullID, unsigned char strategy, BABitvector & RVFID, bool isScope);
    /** @brief The behaviour of this method is strategy specific. It should store to the LocalProxy some state about the fact that an ActiveSubscription is assigned to a subscriber (LocalHost).
     * 
     * The ActiveSubscription is potentially allocated and the subscriber is assigned to it. Depending on the strategy, the request may be forwarded to a rendezvous node (even locally) or not.
     * @param _localhost A pointer to the LocalHost object.
     * @param fullID a reference to the full identifier of the ActiveSubscription.
     * @param strategy the dissemination strategy.
     * @param RVFID The LIPSIN identifier to the rendezvous node. Note that it may be the internal link identifier (e.g. in a NODE_LOCAL strategy).
     * @param isScope a boolean value denoting if the ActiveSubscription refers to a scope or not.
     * @return True if the request must be forwarded to the rendezvous node. The result is strategy specific.
     */
    bool storeActiveSubscription(LocalHost *_localhost, String &fullID, unsigned char strategy, BABitvector & RVFID, bool isScope);
    /** @brief The behaviour of this method is strategy specific. It should remove by the LocalProxy some state about the fact that an ActivePublication is no more assigned to a publisher (LocalHost).
     * 
     * The ActivePublication is potentially deleted and the publisher is removed from it. Depending on the strategy, the request may be forwarded to a rendezvous node (even locally) or not.
     * @param _localhost A pointer to the LocalHost object.
     * @param fullID a reference to the full identifier of the ActivePublication.
     * @param strategy the dissemination strategy.
     * @return True if the request must be forwarded to the rendezvous node. The result is strategy specific.
     */
    bool removeActivePublication(LocalHost *_localhost, String &fullID, unsigned char strategy);
    /** @brief The behaviour of this method is strategy specific. It should remove by the LocalProxy some state about the fact that an ActiveSubscription is no more assigned to a subscriber (LocalHost).
     * 
     * The ActiveSubscription is potentially deleted and the subscriber is removed from it. Depending on the strategy, the request may be forwarded to a rendezvous node (even locally) or not.
     * @param _localhost A pointer to the LocalHost object.
     * @param fullID a reference to the full identifier of the ActiveSubscription.
     * @param strategy the dissemination strategy.
     * @return True if the request must be forwarded to the rendezvous node. The result is strategy specific.
     */
    bool removeActiveSubscription(LocalHost *_localhost, String &fullID, unsigned char strategy);
    /**@brief  Handles a notification sent by a rendezvous point.
     * 
     * This method is called in two different cases. An RV notification can be published by the localRV Element running locally in this Blackadder node or a by a remote RV.
     * In the first case, LocalProxy first handles it as a regular PUBLISH_DATA request where the /FFFFFFFFFFFFFFFD/NODEID has been used.
     * In the second case it is a network publication published using the same identifier as before.
     * An RV notification can be START_PUBLISH, STOP_PUBLISH, SCOPE_PUBLISHED or SCOPE_UNPUBLISHED.
     * The LocalProxy finds all subscribers and then uses the sendNotificationLocally() method to notify all interested LocalHost. Note that behaviour is strategy specific. 
     * The default behaviour for START_PUBLISH is to notify a random publisher only when this is the first time the LocalProxy receives such notification.
     * Respectively, a STOP_PUBLISH notification is sent to the publisher what was notified when the first START_PUBLISH had arrived.
     */
    void handleRVNotification(Packet *p);
    /**
     * @brief This method is called whenever a LocalHost (application or Click Element) sends a publish_data request.
     * 
     * The LocalProxy first finds the respective ActivePublication. It checks if the assigned FID (to subscribers) is zero or the internal Link identifier. In that case there are no remote subscribers.
     * Then it looks for any local subscribers by calling the findLocalSubscribers() method. Note that an ActivePublication may be known with multiple identifiers. That method looks for all of them.
     * 
     * Finally, if necessary, it copies the packet and pushes it to all local subscribers and to the network using the stored FID.
     * @param ID A reference to the full identifier of the published information item.
     * @param p A Click packet containing ONLY some headroom and the DATA to be published.
     * @param _localhost The LocalHost sent the request.
     */
    void handleUserPublication(String &ID, Packet *p, LocalHost *_localhost);
    /**
     * @brief This method is similar to the above. It is called whenever the IMPLICIT_RENDEZVOUS strategy is used.
     * 
     * As in the previous method, it looks for local subscribers. However, it does not look if there is an assigned FID for remote subscribers.
     * It pushes the publication to all local subscribers (if any exists) and to the networkusing the provided FID_to_subscribers.
     * 
     * @param ID A reference to the full identifier of the published information item.
     * @param FID_to_subscribers A LIPSIN identifier provided by the LocalHost. It will be used to push the publication to the network.
     * @param p A Click packet containing ONLY some headroom and the DATA to be published.
     * @param _localhost The LocalHost sent the request.
     */
    void handleUserPublication(String &ID, BABitvector &FID_to_subscribers, Packet *p, LocalHost *_localhost);
    /**@brief This method is called whenever a network publication is pushed to the LocalProxy.
     * 
     * It looks if any local subscribers exist and pushes the data to each one of them
     * 
     * @param IDs A network publication may have multiple identifiers. IDs is a vector containing all the identifiers.
     * @param p A Click packet containing ONLY some headroom and the published DATA.
     */
    void handleNetworkPublication(Vector<String> &IDs, Packet *p);
    /** This method is called whenever a pub/sub request must be published to the rendezvous point.
     * 
     * If the rendezvous node is running locally (e.g. node-local strategy or domain-local when the application runs locally), a packet is created according to the exported API (PUBLISHED_DATA event) and sent to the LocalRV Element.
     * If the rendezvous node runs somewhere else, a network publication is created and the default LIPSIN identifier to the domain RV is added in the packet.
     * 
     * @param p A Click packet containing ONLY some headroom and the pub/sub request (this data is not touched by this method - they are considered as payload).
     * @param FID The LIPSIN identifier to the rendezvous point. It can be the default FID to the domain's RV or the internal LID.
     */
    void publishReqToRV(Packet *p, BABitvector &FID);
    /**@brief It sends a PUBLISHED_DATA event to a LocalHost (an application or a Click Element)
     * 
     * If LocalHost is an application the packet is annotated with the application identifier (the respective netlink socket) and pushed to the ToNetlink socket.
     * If LocalHost is a Click Element the packet is pushed to the respective Element output.
     * 
     * @param _localhost The LocalHost to send the event (along with the data).
     * @param ID The information identifier included in the PUBLISHED_DATA event.
     * @param p A Click packet containing ONLY some headroom and the DATA to be published.
     */
    void pushDataToLocalSubscriber(LocalHost *_localhost, String &ID, Packet *p);
    /**@brief This method is similar to the previous one. It is called when the IMPLICIT_RENDEZVOUS strategy is used.
     * 
     * The LocalProxy will use the provided FID_to_subscribers to forward the packet to the network.
     * 
     * @param IDs a reference to a vector that contains all information identifiers that should be copied in the network publication.
     * @param FID_to_subscribers The LIPSIN identifier that will be used for sending the packet.
     * @param p
     */
    void pushDataToRemoteSubscribers(Vector<String> &IDs, BABitvector &FID_to_subscribers, Packet *p);
    /**@brief This method is called only by one of the deleteAll* methods. It creates and sends a packet that is then published to a rendezvous point.
     * 
     * Depending on where is the rendezvous point, the packet will be pushed either locally to the LocalRV Element or to a remote rendezvous point using the provided RVFID.
     * @param type the type of the pub/sub request.
     * @param IDLength The length of the ID (as in the API) in fragments.
     * @param ID The ID (as in the API).
     * @param prefixIDLength The length of the prefixID (as in the API) in fragments.
     * @param prefixID The prefixID (as in the API).
     * @param RVFID The LISPIN identifier to the rendezvous point. It can be the internal LID or the default FID to the domain's RV.
     * @param strategy The assigned dissemination strategy.
     */
    void createAndSendPacketToRV(unsigned char type, unsigned char IDLength, String &ID, unsigned char prefixIDLength, String &prefixID, BABitvector &RVFID, unsigned char strategy);
    /**@brief This method is called on behalf of a LocalHost (an application or a Click Element) that has exited (when disconnect() is called).
     * 
     * The basic idea is that all previous pub/sub requests must be undone. First all Information Items previously advertised by the _localhost must be unpublished.
     * This method creates a Click packet for each request and sends it to the right rendezvous point using the FID in each ActivePublication.
     * @param _localhost the LocalHost on behalf of which items will be unpublished.
     */
    void deleteAllActiveInformationItemPublications(LocalHost * _localhost);
    /**@brief This method is called on behalf of a LocalHost (an application or a Click Element) that has exited (when disconnect() is called).
     * 
     * For all Information Items to which _localhost has previously subscribed an unsubscribe request must be sent. 
     * This method creates a Click packet for each request and sends it to the right rendezvous point using the FID in each ActiveSubscription.
     * @param _localhost the LocalHost on behalf of which unsubscribe requests will be sent.
     */
    void deleteAllActiveInformationItemSubscriptions(LocalHost * _localhost);
    /**This method is called on behalf of a LocalHost (an application or a Click Element) that has exited (when disconnect() is called).
     * 
     * All Scopes previously published by the _localhost will be unpublished.
     * This method creates a Click packet for each request and sends it to the right rendezvous point using the FID in each ActivePublication.
     * @param _localhost
     */
    void deleteAllActiveScopePublications(LocalHost * _localhost);
    /**@brief This method is called on behalf of a LocalHost (an application or a Click Element) that has exited (when disconnect() is called).
     * 
     * For all Scopes to which _localhost has previously subscribed an unsubscribe request must be sent.
     * This method creates a Click packet for each request and sends it to the right rendezvous point using the FID in each ActiveSubscription.
     * @param _localhost the LocalHost on behalf of which unsubscribe requests will be sent.
     */
    void deleteAllActiveScopeSubscriptions(LocalHost * _localhost);
    /**@brief This method looks for ActiveSubscriptions for the provided bector of identifier.
     * 
     * It looks in the ActiveSubscription index. For each identifier in the vector it looks in the index for this identifier as well as for the father identifier (i.e. without the last fragment).
     * For each ActiveSubscription, it stores all LocalHost subscribers in the provided LocalHostStringHashMap.
     * 
     * @param IDs a reference to a vector of identifiers for which the LocalProxy will seek for an ActiveSubscription.
     * @param _localSubscribers a reference to a HashTable that maps pointers to LocalHost to information identifiers for which the LocalHost is subscribed.
     * @return true if at least a subscriber was found.
     */
    bool findLocalSubscribers(Vector<String> &IDs, LocalHostStringHashMap & _localSubscribers);
    /**@brief It looks for local subscribers to father item of the one identified by the ID.
     * 
     * @todo It should be renamed or something
     * 
     * @param ID a reference to an information (or scope) identifier.
     * @param local_subscribers_to_notify a reference to a set of LocalHost.
     */
    void findActiveSubscriptions(String &ID, LocalHostSet &local_subscribers_to_notify);
    /**@brief This method creates a Click Packet, copies a pub/sub notification to a LocalHost (SCOPE_PUBLISHED, SCOPE_UNPUBLISHED, START_PUBLISH, STOP_PUBLISH) and sends it to the right Click Element (e.g. ToNetlink or LocalRV).
     * 
     * @param type the type of the notification.
     * @param _localhost The LocalHost to send the notification.
     * @param ID a reference to an information identifier that will be included in the notification.
     */
    void sendNotificationLocally(unsigned char type, LocalHost *_localhost, String ID);
    /**@brief A pointer to the GlobalConf Element so that LocalProxy can access the node's Global Configuration.
     */
    GlobalConf *gc;
    /**@brief A HashTable that maps Publishers' and Subscribers' identifiers to pointers of LocalHost.
     * 
     * A publisher or subscriber can be an application or a Click element. Check LocalHost documentation.
     */
    PubSubIdx local_pub_sub_Index;
    /**@brief A HashTable that maps an ActivePublication identifier (full ID from a root of a graph) to a pointer of ActivePublication.
     */
    ActivePub activePublicationIndex;
    /**@brief A HashTable that maps an ActiveSubscription identifier (full ID from a root of a graph) to a pointer of ActiveSubscription.
     */
    ActiveSub activeSubscriptionIndex;
};

CLICK_ENDDECLS
#endif
