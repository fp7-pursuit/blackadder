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

#include <ns3/ethernet-header.h>
#include <ns3/packet.h>

#include "click-bridge.h"

NS_LOG_COMPONENT_DEFINE("ClickBridge");

#define INTERFACE_ID_KERNELTAP 0
#define INTERFACE_ID_FIRST 1
#define INTERFACE_ID_FIRST_DROP 33

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(ClickBridge);
    std::map < simclick_node_t *, Ptr<ClickBridge> > ClickBridge::m_clickInstanceFromSimNode;

    TypeId ClickBridge::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::ClickBridge")
                .SetParent<Object > ()
                .AddConstructor<ClickBridge > ();
        return tid;
    }

    ClickBridge::ClickBridge() {
    }

    ClickBridge::~ClickBridge() {
    }

    void ClickBridge::DoStart(void) {
        NS_LOG_FUNCTION(this);
        RegisterReceiver();
        uint32_t id = GetObject<Node > ()->GetId();
        std::stringstream name;
        name << "Node" << id;
        m_nodeName = name.str();
        m_simNode = new simclick_node_t;
        timerclear(&m_simNode->curtime);
        AddSimNodeToClickMapping();
        NS_ASSERT(m_clickFile.length() > 0);

        // Even though simclick_click_create() will halt programme execution
        // if it is unable to initialise a Click router, we play safe
        if (simclick_click_create(m_simNode, m_clickFile.c_str()) >= 0) {
            NS_LOG_DEBUG(m_nodeName << " has initialised a Click Router");
            m_clickInitialised = true;
        } else {
            NS_LOG_DEBUG("Click Router Initialisation failed for " << m_nodeName);
            m_clickInitialised = false;
        }

        NS_ASSERT(m_clickInitialised == true);
        simclick_click_run(m_simNode);
    }

    void ClickBridge::DoDispose() {
        NS_LOG_FUNCTION(this);
        if (m_clickInitialised) {
            simclick_click_kill(m_simNode);
        }
        delete m_simNode;
    }

    void ClickBridge::SetUpcall(ReceiveCallback cb) {
        NS_LOG_FUNCTION(this);
        m_upcall = cb;
    }

    void ClickBridge::RegisterReceiver(void) {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("Registered Receive Callback to Node " << GetObject<Node > ()->GetId());
        GetObject<Node > ()->RegisterProtocolHandler(ns3::MakeCallback(&ns3::ClickBridge::Receive, this), 0,0,  false);
    }

    void ClickBridge::Receive(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType) {
        NS_LOG_FUNCTION(this);
        int len;
        uint8_t *buf;
        uint32_t ifid;
        EthernetHeader ethHeader;
        Ptr<Packet> packetToClick;
        for (ifid = 0; ifid < GetObject<Node > ()->GetNDevices(); ifid++) {
            Ptr<NetDevice> device = GetObject<Node > ()->GetDevice(ifid);
            if (Mac48Address::ConvertFrom(device->GetAddress()) == Mac48Address::ConvertFrom(to)) {
                break;
            }
        }
        /*For now I will assume that all devices will give me a packet that has no link layer header*/
        /*The forwarder expects ethernet frames so I will add one using the MAC addresses of the devices*/
        Mac48Address fromMacAddr = Mac48Address::ConvertFrom(from);
        Mac48Address toMacAddr = Mac48Address::ConvertFrom(to);
        ethHeader.SetSource(fromMacAddr);
        ethHeader.SetDestination(toMacAddr);
        len = packet->GetSize();
        ethHeader.SetLengthType(len);
        packetToClick = packet->Copy();
        packetToClick->AddHeader(ethHeader);
        len = packetToClick->GetSize();
        buf = new uint8_t [len];
        packetToClick->CopyData(buf, len);
        /*Push the packet to Click - use the right network interface*/
        SendPacketToClick(++ifid, SIMCLICK_PTYPE_ETHER, buf, len);
    }

    Ptr<ClickBridge> ClickBridge::GetClickInstanceFromSimNode(simclick_node_t *simnode) {
        return m_clickInstanceFromSimNode[simnode];
    }

    std::string ClickBridge::GetNodeName() {
        return m_nodeName;
    }

    void ClickBridge::AddSimNodeToClickMapping() {
        m_clickInstanceFromSimNode.insert(std::make_pair(m_simNode, this));
    }

    void ClickBridge::SetClickFile(std::string clickfile) {
        m_clickFile = clickfile;
    }

    struct timeval ClickBridge::GetTimevalFromNow() const {
        struct timeval curtime;
        uint64_t remainder = 0;

        curtime.tv_sec = Simulator::Now().GetSeconds();
        curtime.tv_usec = Simulator::Now().GetMicroSeconds() % 1000000;

        switch (Simulator::Now().GetResolution()) {
            case Time::NS:
                remainder = Simulator::Now().GetNanoSeconds() % 1000;
                break;
            case Time::PS:
                remainder = Simulator::Now().GetPicoSeconds() % 1000000;
                break;
            case Time::FS:
                remainder = Simulator::Now().GetFemtoSeconds() % 1000000000;
                break;
            default:
                break;
        }

        if (remainder) {
            ++curtime.tv_usec;
            if (curtime.tv_usec == 1000000) {
                ++curtime.tv_sec;
                curtime.tv_usec = 0;
            }
        }

        return curtime;
    }

    void ClickBridge::RunClickEvent() {
        m_simNode->curtime = GetTimevalFromNow();
        //NS_LOG_DEBUG("RunClickEvent at " << m_simNode->curtime.tv_sec << " " << m_simNode->curtime.tv_usec << " " << Simulator::Now());
        simclick_click_run(m_simNode);
    }

    void ClickBridge::HandleScheduleFromClick(const struct timeval *when) {
        //NS_LOG_DEBUG("HandleScheduleFromClick at " << when->tv_sec << " " << when->tv_usec << " " << Simulator::Now());
        Time simtime = Time::FromInteger(when->tv_sec, Time::S) + Time::FromInteger(when->tv_usec, Time::US);
        Time simdelay = simtime - Simulator::Now();
        Simulator::Schedule(simdelay, &ClickBridge::RunClickEvent, this);
    }

    void ClickBridge::HandlePacketFromClick(int ifid, int ptype, const unsigned char* data, int len) {
        NS_LOG_FUNCTION(this << ifid);
        // Figure out packet's destination here:
        // If ifid == 0, then the packet's going up
        // else, the packet's going down
        //free((void *) data);
        if (ifid == 0) {
            NS_LOG_DEBUG("Received an event that goes to an application");
            m_upcall(ifid, data, len);
        } else if (ifid) {
            NS_LOG_DEBUG("Incoming packet from eth" << ifid - 1 << " of type " << ptype << ". Sending packet down the stack.");
            Ptr<Packet> packet = Create<Packet > ((uint8_t const *) data, len);
            EthernetHeader ethHeader;
            packet->RemoveHeader(ethHeader);
            NS_LOG_INFO("SOURCE MAC ADDRESS: " << ethHeader.GetSource());
            NS_LOG_INFO("DESTINATION MAC ADDRESS: " << ethHeader.GetDestination());
            Ptr<NetDevice> nd = GetObject<Node > ()->GetDevice(ifid - 1);
            nd->Send(packet, ethHeader.GetDestination(), 0x800);
        }
    }

    void ClickBridge::SendPacketToClick(int ifid, int ptype, const uint8_t *buf, int len) {
        NS_LOG_FUNCTION(this << ifid);
        m_simNode->curtime = GetTimevalFromNow();
        // Since packets in ns-3 don't have global Packet ID's and Flow ID's, we
        // feed dummy values into pinfo. This avoids the need to make changes in the Click code
        simclick_simpacketinfo pinfo;
        pinfo.id = 0;
        pinfo.fid = 0;
        simclick_click_send(m_simNode, ifid, ptype, buf, len, &pinfo);
        delete [] buf;
    }

    std::string ClickBridge::ReadHandler(std::string elementName, std::string handlerName) {
        char *handle = simclick_click_read_handler(m_simNode, elementName.c_str(), handlerName.c_str(), 0, 0);
        std::string ret(handle);
        // This is required because Click does not free
        // the memory allocated to the return string
        // from simclick_click_read_handler()
        free(handle);
        return ret;
    }

    int ClickBridge::WriteHandler(std::string elementName, std::string handlerName, std::string writeString) {
        int r = simclick_click_write_handler(m_simNode, elementName.c_str(), handlerName.c_str(), writeString.c_str());
        // Note: There are probably use-cases for returning
        // a write handler's error code, so don't assert.
        // For example, the 'add' handler for IPRouteTable
        // type elements fails if the route to be added
        // already exists.
        return r;
    }

    int ClickBridge::GetInterfaceId(const char *ifname) {
        int retval = -1;
        Ptr< Node > node;
        // The below hard coding of interface names follows the
        // same approach as used in the original nsclick code for
        // ns-2. The interface names map directly to what is to
        // be used in the Click configuration files.
        // Thus eth0 will refer to the first network device of
        // the node, and is to be named so in the Click graph.
        // This function is called by Click during the intialisation
        // phase of the Click graph, during which it tries to map
        // interface IDs to interface names. The return value
        // corresponds to the interface ID that Click will use.

        // Tap/tun devices refer to the kernel devices
        if (strstr(ifname, "tap") || strstr(ifname, "tun")) {
            retval = 0;
        } else if (const char *devname = strstr(ifname, "eth")) {
            while (*devname && !isdigit((unsigned char) *devname)) {
                devname++;
            }
            if (*devname) {
                retval = atoi(devname) + INTERFACE_ID_FIRST;
            }
        } else if (const char *devname = strstr(ifname, "drop")) {
            while (*devname && !isdigit((unsigned char) *devname)) {
                devname++;
            }
            if (*devname) {
                retval = atoi(devname) + INTERFACE_ID_FIRST_DROP;
            }
        }
        return retval;
    }

    std::string ClickBridge::GetMacAddressFromInterfaceId(int ifid) {
        std::stringstream addr;
        Ptr< Node > node = GetObject<Node > ();
        Ptr<NetDevice> device = node->GetDevice(ifid);
        Address devAddr = device->GetAddress();
        addr << Mac48Address::ConvertFrom(devAddr);
        return addr.str();
    }
} // namespace ns3

static int simstrlcpy(char *buf, int len, const std::string &s) {
    if (len) {
        len--;

        if ((unsigned) len > s.length()) {
            len = s.length();
        }

        s.copy(buf, len);
        buf[len] = '\0';
    }
    return 0;
}

/**@brief Sends a Packet from Click to the Simulator: Defined in simclick.h. Click calls these methods.*/
int simclick_sim_send(simclick_node_t *simnode, int ifid, int type, const unsigned char* data, int len, simclick_simpacketinfo *pinfo) {
    NS_LOG_DEBUG("simclick_sim_send called at " << ns3::Simulator::Now().GetSeconds() << ": " << ifid << " " << len);
    if (simnode == NULL) {
        return -1;
    }
    ns3::Ptr<ns3::ClickBridge> clickInstance = ns3::ClickBridge::GetClickInstanceFromSimNode(simnode);
    clickInstance->HandlePacketFromClick(ifid, type, data, len);
    return 0;
}

/**@brief Click Service Methods: Defined in simclick.h*/
int simclick_sim_command(simclick_node_t *simnode, int cmd, ...) {
    va_list val;
    va_start(val, cmd);
    int retval = 0;

    ns3::Ptr<ns3::ClickBridge> clickInstance = ns3::ClickBridge::GetClickInstanceFromSimNode(simnode);
    switch (cmd) {
        case SIMCLICK_VERSION:
        {
            retval = 0;
            break;
        }
        case SIMCLICK_SUPPORTS:
        {
            int othercmd = va_arg(val, int);
            retval = (othercmd >= SIMCLICK_VERSION && othercmd <= SIMCLICK_GET_NODE_ID);
            break;
        }
        case SIMCLICK_IFID_FROM_NAME:
        {
            const char *ifname = va_arg(val, const char *);
            retval = clickInstance->GetInterfaceId(ifname);
            //NS_LOG_DEBUG(clickInstance->GetNodeName() << " SIMCLICK_IFID_FROM_NAME: " << ifname << " " << retval);
            break;
        }
        case SIMCLICK_MACADDR_FROM_NAME:
        {
            const char *ifname = va_arg(val, const char *);
            char *buf = va_arg(val, char *);
            int len = va_arg(val, int);
            int ifid = clickInstance->GetInterfaceId(ifname);
            if (ifid >= 0) {
                retval = simstrlcpy(buf, len, clickInstance->GetMacAddressFromInterfaceId(ifid));
            } else {
                retval = -1;
            }
            //NS_LOG_DEBUG(clickInstance->GetNodeName() << " SIMCLICK_MACADDR_FROM_NAME: " << ifname << " " << buf << " " << len);
            break;
        }
        case SIMCLICK_SCHEDULE:
        {
            const struct timeval *when = va_arg(val, const struct timeval *);
            clickInstance->HandleScheduleFromClick(when);
            retval = 0;
            //NS_LOG_DEBUG(clickInstance->GetNodeName() << " SIMCLICK_SCHEDULE at " << when->tv_sec << "s and " << when->tv_usec << "usecs.");
            break;
        }
        case SIMCLICK_GET_NODE_NAME:
        {
            char *buf = va_arg(val, char *);
            int len = va_arg(val, int);
            retval = simstrlcpy(buf, len, clickInstance->GetNodeName());
            //NS_LOG_DEBUG(clickInstance->GetNodeName() << " SIMCLICK_GET_NODE_NAME: " << buf << " " << len);
            break;
        }
        case SIMCLICK_IF_PROMISC:
        {
            //int ifid = va_arg(val, int);
            retval = 0;
            //NS_LOG_DEBUG(clickInstance->GetNodeName() << " SIMCLICK_IF_PROMISC: " << ifid << " " << ns3::Simulator::Now());
            break;
        }
        case SIMCLICK_IF_READY:
        {
            //int ifid = va_arg(val, int);
            retval = true;
            //NS_LOG_DEBUG(clickInstance->GetNodeName() << " SIMCLICK_IF_READY: " << ifid << " " << ns3::Simulator::Now());
            break;
        }
        case SIMCLICK_TRACE:
        {
            // Used only for tracing
            //NS_LOG_DEBUG(clickInstance->GetNodeName() << " Received a call for SIMCLICK_TRACE");
            break;
        }
        case SIMCLICK_GET_NODE_ID:
        {
            // Used only for tracing
            //NS_LOG_DEBUG(clickInstance->GetNodeName() << " Received a call for SIMCLICK_GET_NODE_ID");
            break;
        }
    }
    return retval;
}
