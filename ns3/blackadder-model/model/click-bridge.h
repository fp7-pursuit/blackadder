/*
 * Copyright (C) 2010-2012  George Parisis and Dirk Trossen
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

#ifndef CLICK_BRIDGE_H
#define CLICK_BRIDGE_H

#include <ns3/node.h>
#include <ns3/ptr.h>
#include <ns3/log.h>
#include <ns3/packet.h>
#include <ns3/mac48-address.h>
#include <ns3/object.h>
#include <ns3/test.h>
#include <ns3/simulator.h>
#include <ns3/ethernet-header.h>
#include <ns3/callback.h>

#include <sys/time.h>
#include <sys/types.h>

#include <click/simclick.h>

#include <map>
#include <string>
#include <string.h>
#include <stdarg.h>


namespace ns3 {

    typedef Callback<void, int, const unsigned char*, int> ReceiveCallback;
    /**@brief ClickBridge Class implements the necessary functionality (exported by Click) so that NS3 can communicate with Click and Blackadder.
     * 
     * It basically replaces (and is heavily based on) the original integration code that is dependant on IP routing tables.
     */
    class ClickBridge : public Object {
    public:
        /**
         * @brief As mandated by NS3...no attributes.
         * @return 
         */
        static TypeId GetTypeId(void);
        ClickBridge();
        virtual ~ClickBridge();
        /**@brief It is called when simclick_sim_send is called by Click. i.e. when a packet is exiting from a ToSimDevice in Blackadder. It can exit towards the network or the ServiceModel Class (i.e. towards applications).
         * 
         * @param ifid the interface identifier that this packet has to be routed to.If it is zero it has to be pushed to the ServiceModel Class. In any other case it has to be pushed to the ifid-1 network interface. 
         * In Blackadder configuration this interface is ToSimDevice(eth(ifid-1)) and in NS3 it's the respective PointToPointNetDevice.
         * @param ptype ??don't remember :).
         * @param data the actual packet pushed from Click (an Event or an Ethernet Frame).
         * @param len the length of the packet.
         */
        void HandlePacketFromClick(int ifid, int ptype, const unsigned char* data, int len);
        /**@brief It gets the NS3 identifier of the node this ClickBridge.object is instantiated.
         * 
         * @return the node name
         */
        std::string GetNodeName();
        /**@brief for NS3-Click housekeeping.
         * 
         */
        void AddSimNodeToClickMapping();
        /**@brief Given a pointer to simclick_node_t it will return the actual instance of the ClickBridge so that further actions can be taken.
         * 
         * @param simnode a pointer to a simclick_node_t
         * @return 
         */
        static Ptr<ClickBridge> GetClickInstanceFromSimNode(simclick_node_t *simnode);
        /**@brief It sets the Click configuration file name for this NS3 node.
         * 
         * @param clickfile the file name
         */
        void SetClickFile(std::string clickfile);
        /**@brief It will return a struct timeval representing the current simulation time.
         * 
         * @return 
         */
        struct timeval GetTimevalFromNow() const;
        /**@brief this method is run when an event previously scheduled by Click expires. It basically calls simclick_click_run.. 
         * 
         */
        void RunClickEvent();
        /**@brief it basically schedules the RunClickEvent to run after when timeval.
         * 
         * @param when when?
         */
        void HandleScheduleFromClick(const struct timeval *when);
        /**@brief it is used to send packets from NS3 to Click/Blackadder.
         * 
         * @param ifid the interface from which Click will receive the packet (e.g. 0 for tap0, 1 for eth0, 2 for eth1)
         * @param ptype don't remember
         * @param buf the payload of the Click packet (there are no annotations!!!! - the process identifier is appended in the beginning of the packet)
         * @param len the length of the packet.
         */
        void SendPacketToClick(int ifid, int ptype, const uint8_t *buf, int len);
        /**@brief Not implemented!!
         * 
         * @param elementName
         * @param handlerName
         * @return 
         */
        std::string ReadHandler(std::string elementName, std::string handlerName);
        /**@brief Not implemented!!
         * 
         * @param elementName
         * @param handlerName
         * @return 
         */
        int WriteHandler(std::string elementName, std::string handlerName, std::string writeString);
        /**@brief Given the interface name (e.g. tap0 or eth1), it will return the ifid used by Click.
         * 
         * @param ifname
         * @return 
         */
        int GetInterfaceId(const char *ifname);
        /**@brief Given an ifid (from Click), this method will return the Mac Address of the respective NS3 PointToPointNetDevice (it has a MAC address!).
         * 
         * @param ifid the interface id requested by Click.
         * @return 
         */
        std::string GetMacAddressFromInterfaceId(int ifid);
        /**@brief It sets the callback that will be called when events from Blackadder must be sent to one or more NS3 applications. Called by ServiceModel Class.
         * 
         * @param cb the callback method.
         */
        void SetUpcall(ReceiveCallback cb);
    private:
        ClickBridge& operator =(const ClickBridge &);
        ClickBridge(const ClickBridge &);
        /**
         * @brief Called when the object is started by the simulator. It registers a callback for receiving all Packets that arrive to ALL PointToPointNetDevices.
         * The Click node (basically Blackadder) is initialized.
         * Some housekeeping is also done so that NS3 can keep track of the active click nodes in the simulation.
         */
        void DoStart(void);
        /**@brief The click node is destroyed.
         * 
         */
        void DoDispose(void);
        /** @brief It will register the Receive callback to all network devices..It must be called after all network devices are added to the Node
         * 
         */
        void RegisterReceiver(void);
        /** @brief The callback function that will be called each time a packet is received from a network device..
         * 
         * The ClickBridge will push the packet to Click that runs a Blackadder configuration.
         * 
         * @param device the network device
         * @param p the packet
         * @param protocol the protocol type
         * @param from a MAC? address
         * @param to a MAC? address
         * @param packetType see packet types
         */
        void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);

        /*members*/
        /**@brief this is a map intended for NS3 internal housekeeping related to Click instances.
         */
        static std::map < simclick_node_t *, Ptr<ClickBridge> > m_clickInstanceFromSimNode;
        /**@brief the name of the Click configuration file used by this node to instantiate a Blackadder node. 
         * It is set by SetClickFile in the NS3 C++ topology code.
         */
        std::string m_clickFile;
        /**@brief a string representing the NS3 node identifier - NOT the Blackadder node identifier.
         */
        std::string m_nodeName;
        /**@brief defined in Click-NS3 interface code (in Click).
         */
        simclick_node_t *m_simNode;
        /**@brief for housekeeping.
         */
        bool m_clickInitialised;
        /**@brief This is the Callback function registered by the ServiceModel so that all Blackadder Event can be routed to the interested applications.
         */
        ReceiveCallback m_upcall;

    };
}

#endif
