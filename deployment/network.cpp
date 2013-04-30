/*
 * Copyright (C) 2010-2011  George Parisis and Dirk Trossen
 * All rights reserved.
 *
 * PlanetLab Deployment support By Dimitris Syrivelis
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of
 * the BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

#include <map>
#include <sstream>
#include <iomanip> 
#include "network.hpp"

Domain::Domain() {
    TM_node = NULL;
    RV_node = NULL;
    number_of_nodes = 0;
    number_of_connections = 0;
    number_of_pl_nodes = 0;
    number_of_p_connections = 0;
}

void Domain::printDomainData() {
    int no_nodes;
    no_nodes = network_nodes.size();
    cout << "RV NODE: " << RV_node->label << endl;
    cout << "TM NODE: " << TM_node->label << endl;
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        cout << "****************************" << endl;
        cout << "Node " << nn->label << endl;
        if (overlay_mode.compare("mac_ml") == 0) {
            cout << "Node_Type " << nn->type << endl;
        }
        cout << "testbed_ip " << nn->testbed_ip << endl;
        cout << "iLid " << nn->iLid.to_string() << endl;
        cout << "isRV " << nn->isRV << endl;
        cout << "isTM " << nn->isTM << endl;
        cout << "FID_to_RV " << nn->FID_to_RV.to_string() << endl;
        cout << "FID_to_TM " << nn->FID_to_TM.to_string() << endl;
        cout << "****************************" << endl;
        for (size_t j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            cout << "*****src_label " << nc->src_label << endl;
            cout << "*****dst_label " << nc->dst_label << endl;
            cout << "*****src_if " << nc->src_if << endl;
            cout << "*****dst_if " << nc->dst_if << endl;
            cout << "*****src_ip " << nc->src_ip << endl;
            cout << "*****dst_ip " << nc->dst_ip << endl;
            cout << "*****src_mac " << nc->src_mac << endl;
            cout << "*****dst_mac " << nc->dst_mac << endl;
            cout << "*****LID " << nc->LID.to_string() << endl;
            if (overlay_mode.compare("mac_ml") == 0) {
                cout << "*****in_pt " << nc->in_pt << endl;
                cout << "*****out_pt "<< nc->out_pt << endl;
                cout << "*****lnk_type "<< nc->lnk_type << endl;
            }
        }
    }
}

bool exists(vector<Bitvector> &LIDs, Bitvector &LID) {
    for (size_t i = 0; i < LIDs.size(); i++) {
        if (LIDs[i] == LID) {
            //cout << "duplicate LID" << endl;
            return true;
        }
    }
    return false;
}

NetworkNode *Domain::findNode(string label) {
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        if (nn->label.compare(label) == 0) {
            return nn;
        }
    }
    return NULL;
}

void Domain::calculateLID(vector<Bitvector> &LIDs, int index) {
    int bit_position;
    int number_of_bits = (index / (fid_len * 8)) + 1;
    Bitvector LID;
    do {
        LID = Bitvector(fid_len * 8);
        for (int i = 0; i < number_of_bits; i++) {
            /*assign a bit in a random position*/
            bit_position = rand() % (fid_len * 8);
            LID[bit_position] = true;
        }
    } while (exists(LIDs, LID));
    LIDs[index] = LID;
}

void Domain::assignLIDs() {
    int LIDCounter = 0;
    srand(0);
    //srand(66000);
    /*first calculated how many LIDs should I calculate*/
    int totalLIDs;
    bool mac_ml = (overlay_mode.compare("mac_ml") == 0);
    if (mac_ml) {
        cout << "Number of iLIDs is: " << number_of_pl_nodes << ", Number of LIDs is: " << number_of_p_connections << ", Number of Total connections is: " << number_of_connections << endl;
        totalLIDs = number_of_pl_nodes/*the iLIDs*/ + number_of_p_connections;
        // here to add pn node and pp connection count to eleminate the assignment of Lids for the oo links and oe-links
    } else {
        totalLIDs = number_of_nodes/*the iLIDs*/ + number_of_connections;
    }
    vector<Bitvector> LIDs(totalLIDs);
    for (int i = 0; i < totalLIDs; i++) {
        calculateLID(LIDs, i);
    }
#if 0
    for (int i = 0; i < totalLIDs; i++) {
        cout << "LID " << i << " : " << LIDs.at(i).to_string() << endl;
    }
#endif
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        if (mac_ml) {
            if (nn->type == "PN") { /* packet layer node */
                nn->iLid = LIDs[LIDCounter];
                LIDCounter++;
            }
        } else {
            nn->iLid = LIDs[LIDCounter];
            LIDCounter++;
        }
        for (size_t j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            if (mac_ml) {
                if ((nc->lnk_type == "pp")) { /* packet-to-packet connection */
                    nc->LID = LIDs[LIDCounter];
                    LIDCounter++;
                } else {
                    nc->LID = Bitvector(fid_len * 8, 0);
                }
            } else {
                nc->LID = LIDs[LIDCounter];
                LIDCounter++;
            }
        }
    }
    for (int i = 0; i < LIDCounter; i++) {
        cout << "LID " << i << " : " << LIDs.at(i).to_string() << endl;
    }
}

string Domain::getTestbedIPFromLabel(string label) {
    NetworkNode *nn = findNode(label);
    if (nn)
        return nn->testbed_ip;
    return "";
}

/* Values for target machines running Linux: */
#define HWADDR_LABEL   "HWaddr"
#define HWADDR_OFFSET  21

#define HWADDR_LABEL_FREEBSD   "ether"
#define HWADDR_OFFSET_FREEBSD  19

#define HWADDR_LABEL_DARWIN   "ether"
#define HWADDR_OFFSET_DARWIN  20

static void getHwaddrLabel(const string &os,
                           string &hwaddr_label, int &hwaddr_offset) {
    if (os.compare("Linux") == 0) {
        hwaddr_label  = HWADDR_LABEL;
        hwaddr_offset = HWADDR_OFFSET;
    } else if (os.compare("FreeBSD") == 0) {
        hwaddr_label  = HWADDR_LABEL_FREEBSD;
        hwaddr_offset = HWADDR_OFFSET_FREEBSD;
    } else if (os.compare("Darwin") == 0) {
        hwaddr_label  = HWADDR_LABEL_DARWIN;
        hwaddr_offset = HWADDR_OFFSET_DARWIN;
    } else {
        hwaddr_label  = HWADDR_LABEL;
        hwaddr_offset = HWADDR_OFFSET;
    }
}

void Domain::discoverMacAddresses(bool no_remote) {
    map<string, string> mac_addresses;
    string testbed_ip;
    string line;
    string mac_addr;
    FILE *fp_command;
    char response[1035];
    string command;
    string hwaddr_label;
    int hwaddr_offset;
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        for (size_t j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            if (overlay_mode.compare("mac") == 0
                || overlay_mode.compare("mac_ml") == 0
                || overlay_mode.compare("mac_qos") == 0) {
                if (overlay_mode.compare("mac_ml") == 0
                    && (nn->type != "PN" || nc->src_if == "")) {
                    continue;
                }
                if (mac_addresses.find(nc->src_label + nc->src_if) == mac_addresses.end()) {
                    /*get source mac address*/
                    if (nc->src_mac.length() == 0 && !no_remote) {
                        NetworkNode *src_node = findNode(nc->src_label);
                        if (src_node) {
                            getHwaddrLabel(src_node->operating_system, hwaddr_label, hwaddr_offset);
                            testbed_ip = src_node->testbed_ip;
                        }
                        if (sudo) {
                            command = "ssh " + user + "@" + testbed_ip + " -t \"sudo ifconfig " + nc->src_if + " | grep " + hwaddr_label + "\"";
                        } else {
                            command = "ssh " + user + "@" + testbed_ip + " -t \"ifconfig " + nc->src_if + " | grep " + hwaddr_label + "\"";
                        }
                        cout << command << endl;
                        fp_command = popen(command.c_str(), "r");
                        if (fp_command == NULL) {
                            printf("Failed to run command\n");
                            goto close_src;
                        }
                        /* Read the output a line at a time - output it. */
                        if (fgets(response, sizeof (response) - 1, fp_command) == NULL) {
                            cout << "Error or empty response." << endl;
                            goto close_src;
                        }
                        line = string(response);
                        mac_addr = line.substr(line.length() - hwaddr_offset, 17);
                        cout << mac_addr << endl;
                        nc->src_mac = mac_addr;
                        /* close */
                    close_src:
                        pclose(fp_command);
                    } else if (nc->src_mac.length() != 0) {
                        cout << nn->label << ": I already know the src mac address (" << nc->src_mac << ") for this connection (" << nc->src_label << ":" << nc->src_if << " -> " << nc->dst_label << ":" << nc->dst_if  << ")...it was hardcoded in the configuration file" << endl;
                    } else {
                        nc->src_mac = "00:00:00:00:00:00"; /* XXX */
                    }
                    mac_addresses[nc->src_label + nc->src_if] = nc->src_mac;
                } else {
                    nc->src_mac = mac_addresses[nc->src_label + nc->src_if];
                    //cout << "I learned this mac address: " << nc->src_label << ":" << nc->src_if << " - " << mac_addresses[nc->src_label + nc->src_if] << endl;
                }
                if (overlay_mode.compare("mac_ml") == 0
                    && (nn->type != "PN" || nc->dst_if == "")) {
                    continue;
                }
                if (mac_addresses.find(nc->dst_label + nc->dst_if) == mac_addresses.end()) {
                    /*get destination mac address*/
                    if (nc->dst_mac.length() == 0 && !no_remote) {
                        NetworkNode *dst_node = findNode(nc->dst_label);
                        if (dst_node) {
                            getHwaddrLabel(dst_node->operating_system, hwaddr_label, hwaddr_offset);
                            testbed_ip = dst_node->testbed_ip;
                        }
                        if (sudo) {
                            command = "ssh " + user + "@" + testbed_ip + " -t \"sudo ifconfig " + nc->dst_if + " | grep "+ hwaddr_label + "\"";
                        } else {
                            command = "ssh " + user + "@" + testbed_ip + " -t \"ifconfig " + nc->dst_if + " | grep " + hwaddr_label + "\"";
                        }
                        cout << command << endl;
                        fp_command = popen(command.c_str(), "r");
                        if (fp_command == NULL) {
                            printf("Failed to run command\n");
                            goto close_dst;
                        }
                        /* Read the output a line at a time - output it. */
                        if (fgets(response, sizeof (response) - 1, fp_command) == NULL) {
                            cout << "Error or empty response." << endl;
                            goto close_dst;
                        }
                        line = string(response);
                        mac_addr = line.substr(line.length() - hwaddr_offset, 17);
                        cout << mac_addr << endl;
                        nc->dst_mac = mac_addr;
                        /* close */
                    close_dst:
                        pclose(fp_command);
                    } else if (nc->dst_mac.length() != 0) {
                        cout << nn->label << ": I already know the dst mac address (" << nc->dst_mac << ") for this connection (" << nc->src_label << ":" << nc->src_if << " -> " << nc->dst_label << ":" << nc->dst_if  << ")...it was hardcoded in the configuration file" << endl;
                    } else {
                        nc->dst_mac = "00:00:00:00:00:00"; /* XXX */
                    }
                    mac_addresses[nc->dst_label + nc->dst_if] = nc->dst_mac;
                } else {
                    nc->dst_mac = mac_addresses[nc->dst_label + nc->dst_if];
                    //cout << "I learned this mac address: " << nc->dst_label << ":" << nc->dst_if << " - " << mac_addresses[nc->src_label + nc->src_if] << endl;
                }
            } else {
                //cout << "no need to discover mac addresses - it's a connection over raw IP sockets" << endl;
            }
        }
    }
}

NetworkNode *Domain::getNode(string label) {
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        if (nn->label.compare(label) == 0) {
            return nn;
        }
    }
    return NULL;
}

NetworkConnection *NetworkNode::getConnection(string src_label, string dst_label) {
    for (size_t i = 0; i < connections.size(); i++) {
        NetworkConnection *nc = connections[i];
        if ((nc->src_label.compare(src_label) == 0) && (nc->dst_label.compare(dst_label) == 0)) {
            return nc;
        }
    }
    return NULL;
}
///*check if I have already considered this connection (the other way) before*/
//            for (int k = 0; k < network_nodes.size(); k++) {
//                NetworkNode *neighbourNode = network_nodes[k];
//                if(neighbourNode->label.compare())
//            }

string Domain::getNextMacAddress() {
    uint8_t m_address[6];
    string mac_str;
    std::stringstream ss;
    m_address[0] = (id >> 40) & 0xff;
    m_address[1] = (id >> 32) & 0xff;
    m_address[2] = (id >> 24) & 0xff;
    m_address[3] = (id >> 16) & 0xff;
    m_address[4] = (id >> 8) & 0xff;
    m_address[5] = (id >> 0) & 0xff;
    id++;
    ss.setf(std::ios::hex, std::ios::basefield);

    ss.fill('0');
    for (uint8_t i = 0; i < 5; i++) {
        ss << std::setw(2) << (uint32_t) m_address[i] << ":";
    }
    // Final byte not suffixed by ":"
    ss << std::setw(2) << (uint32_t) m_address[5];
    ss.setf(std::ios::dec, std::ios::basefield);
    ss.fill(' ');
    mac_str = ss.str();
    return mac_str;
}

/*probably the most inefficient way of doing it!!*/
void Domain::assignDeviceNamesAndMacAddresses() {
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        cout << "Looking at Node " << nn->label << ", Device Offset " << nn->deviceOffset << endl;
        for (size_t j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            cout << "Looking at Connection " << nc->src_label << " -> " << nc->dst_label << endl;
            /*check if I have already considered this connection (the other way) before*/
            if ((nc->src_if.empty()) && (nc->dst_if.empty())) {
                std::stringstream out1;
                out1 << nn->deviceOffset;
                nc->src_if = "eth" + out1.str();
                nc->src_mac = getNextMacAddress();
                std::stringstream out2;
                NetworkNode *neighbour = getNode(nc->dst_label);
                out2 << neighbour->deviceOffset;
                nc->dst_if = "eth" + out2.str();
                nc->dst_mac = getNextMacAddress();
                cout << "Assigning " << nn->label << ", device offset " << nn->deviceOffset << ", device name " << nc->src_if << ", MAC Address " << nc->src_mac << endl;
                cout << "Assigning " << neighbour->label << ", device offset " << neighbour->deviceOffset << ", device name " << nc->dst_if << ", MAC Address " << nc->dst_mac << endl;
                nn->deviceOffset++;
                neighbour->deviceOffset++;
                /*find the connection from the neighbour node and assign the same interfaces*/
                NetworkConnection *neighbourConnection = neighbour->getConnection(nc->dst_label, nc->src_label);
                neighbourConnection->src_if = nc->dst_if;
                neighbourConnection->dst_if = nc->src_if;
                neighbourConnection->src_mac = nc->dst_mac;
                neighbourConnection->dst_mac = nc->src_mac;
            } else {
                cout << "Already checked that connection" << endl;
                cout << "device name " << nc->src_if << ", MAC Address " << nc->src_mac << endl;
                cout << "device name " << nc->dst_if << ", MAC Address " << nc->dst_mac << endl;
            }
        }
    }
}

void Domain::createNS3SimulationCode() {
    ofstream ns3_code;
    ns3_code.open((write_conf + "topology.cpp").c_str());
    /*First write all required stuff before starting to build the actual topology*/
    ns3_code << "#include <ns3/core-module.h>" << endl;
    ns3_code << "#include <ns3/network-module.h>" << endl;
    ns3_code << "#include <ns3/point-to-point-module.h>" << endl;
    ns3_code << "#include <ns3/blackadder-module.h>" << endl;
    ns3_code << "using namespace ns3;" << endl;
    ns3_code << "NS_LOG_COMPONENT_DEFINE(\"topology\");" << endl;
    ns3_code << "int main(int argc, char *argv[]) {" << endl;
    /*create all nodes now*/
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        ns3_code << "   Ptr<Node> node" << i << " = CreateObject<Node>();" << endl;
        std::stringstream out2;
        out2 << "node" << i;
        label_to_node_name.insert(pair<string, string > (nn->label, out2.str()));
        for (size_t j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            ns3_code << "   Ptr<PointToPointNetDevice> dev" << i << "_" << j << " = Create<PointToPointNetDevice>();" << endl;
            ns3_code << "   dev" << i << "_" << j << "->SetAddress (Mac48Address(\"" << nc->src_mac << "\"));" << endl;
            ns3_code << "   dev" << i << "_" << j << "->SetDataRate (DataRate(\"" << nc->rate << "\"));" << endl;
            ns3_code << "   dev" << i << "_" << j << "->SetMtu (" << nc->mtu << ");" << endl;
            std::stringstream out1;
            out1 << "dev" << i << "_" << j;
            address_to_device_name.insert(pair<string, string > (nc->src_mac, out1.str()));
            ns3_code << "   node" << i << "->AddDevice(dev" << i << "_" << j << ");" << endl;
            /*The type of queue can be a parameter in the initial topology file that is parsed (along with its attributes)*/
            ns3_code << "   Ptr<DropTailQueue> queue" << i << "_" << j << " = CreateObject<DropTailQueue > ();" << endl;
            ns3_code << "   dev" << i << "_" << j << "->SetQueue(queue" << i << "_" << j << ");" << endl;
        }
        ns3_code << endl;
    }
    int channel_counter = 0;
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        for (size_t j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            if ((NS3devices_in_connections.find(nc->src_mac) == NS3devices_in_connections.end()) && (NS3devices_in_connections.find(nc->dst_mac) == NS3devices_in_connections.end())) {
                ns3_code << "   Ptr<PointToPointChannel> channel" << channel_counter << " = CreateObject<PointToPointChannel>();" << endl;
                ns3_code << "   channel" << channel_counter << "->SetAttribute(\"Delay\", StringValue(\"" << nc->delay << "\"));" << endl;
                ns3_code << "   " << (*address_to_device_name.find(nc->src_mac)).second << "->Attach(channel" << channel_counter << ");" << endl;
                ns3_code << "   " << (*address_to_device_name.find(nc->dst_mac)).second << "->Attach(channel" << channel_counter << ");" << endl;
                NS3devices_in_connections.insert(nc->src_mac);
                NS3devices_in_connections.insert(nc->dst_mac);
                channel_counter++;
                ns3_code << endl;
            }
        }
    }
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        ns3_code << "   Ptr<ClickBridge> click" << i << " = CreateObject<ClickBridge > ();" << endl;
        ns3_code << "   node" << i << "->AggregateObject(click" << i << ");" << endl;
        ns3_code << "   click" << i << "->SetClickFile(\"" << write_conf << "/" << nn->label << ".conf\");" << endl;
        ns3_code << "   Ptr<ServiceModel> servModel" << i << " = CreateObject<ServiceModel > (); " << endl;
        ns3_code << "   node" << i << "->AggregateObject(servModel" << i << "); " << endl;
    }
    ns3_code << endl;

    /*Start the topology manager application to the respective network node*/
    ns3_code << "   Ptr<TopologyManager> tm = CreateObject<TopologyManager > ();" << endl;
    ns3_code << "   tm->SetStartTime(Seconds(0.)); " << endl;
    ns3_code << "   tm->SetAttribute(\"Topology\", StringValue(\"" << write_conf << "topology.graphml\"));" << endl;
    ns3_code << "   " << (*label_to_node_name.find(TM_node->label)).second << "->AddApplication(tm); " << endl;
    ns3_code << endl;
    /*add applications and attributes*/
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        for (size_t j = 0; j < nn->applications.size(); j++) {
            NS3Application *app = nn->applications[j];
            ns3_code << "   Ptr<" << app->name << "> app" << i << "_" << j << " = CreateObject<" << app->name << "> ();" << endl;
            ns3_code << "   " << "app" << i << "_" << j << "->SetStartTime(Seconds(" << app->start << ")); " << endl;
            ns3_code << "   " << "app" << i << "_" << j << "->SetStopTime(Seconds(" << app->stop << ")); " << endl;
            for (size_t k = 0; k < app->attributes.size(); k++) {
                NS3ApplicationAttribute *attr = app->attributes[k];
                ns3_code << "   " << "app" << i << "_" << j << "->SetAttribute(\"" << attr->name << "\", " << attr->value << ");" << endl;
            }
            ns3_code << "   node" << i << "->AddApplication(app" << i << "_" << j << ");" << endl;
            ns3_code << endl;
        }
    }
    ns3_code << "   Simulator::Run();" << endl;
    ns3_code << "   Simulator::Destroy();" << endl;
    ns3_code << "   return 0;" << endl;

    ns3_code << "}" << endl;
    ns3_code.close();
}

int findOffset(vector<string> &unique, string &str) {
    for (size_t i = 0; i < unique.size(); i++) {
        if (unique[i].compare(str) == 0) {
            return i;
        }
    }
    return -1;
}

void Domain::writeClickFiles(bool montoolstub, bool dump_supp) {
    ofstream click_conf;
    ofstream write_TMFID;
    for (size_t i = 0; i < network_nodes.size(); i++) {
        vector<string> unique_ifaces;
        vector<string> unique_srcips;
        // Map Interface to ports for QoS (e.g.: eth0: 1,2)
        IfPortsMap ifmap;
        NetworkNode *nn = network_nodes[i];
        click_conf.open((write_conf + nn->label + ".conf").c_str());
        write_TMFID.open((write_conf+ nn->label +"_TMFID.txt").c_str());
        if (montoolstub && (nn->running_mode.compare("user") == 0)) {
            click_conf << "require(blackadder); \n\nControlSocket(\"TCP\",55000);\n\n " << endl << endl;
        } else {
            click_conf << "require(blackadder);" << endl << endl;
        }
        /*Blackadder Elements First*/
        click_conf << "globalconf::GlobalConf(MODE " << overlay_mode << ", NODEID " << nn->label << "," << endl;
        click_conf << "DEFAULTRV " << nn->FID_to_RV.to_string() << "," << endl;
        click_conf << "TMFID     " << nn->FID_to_TM.to_string() << "," << endl;
        write_TMFID << nn->FID_to_TM.to_string()<<endl;
        click_conf << "iLID      " << nn->iLid.to_string() << ");" << endl << endl;

        click_conf << "localRV::LocalRV(globalconf);" << endl;
        click_conf << "netlink::Netlink();" << endl << "tonetlink::ToNetlink(netlink);" << endl << "fromnetlink::FromNetlink(netlink);" << endl << endl;
        click_conf << "proxy::LocalProxy(globalconf);" << endl << endl;
        click_conf << "fw::Forwarder(globalconf," << nn->connections.size() << "," << endl;
        for (size_t j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            int offset;
            if (overlay_mode.compare("mac") == 0
                || (overlay_mode.compare("mac_ml") == 0
                    && nn->type.compare("PN") == 0)) {
                if (overlay_mode.compare("mac_ml") == 0
                    && nc->lnk_type != "pp") {
                    nc->dst_mac = "00:00:00:00:00:00";
                }
                
                // Default MAC behaviour
                if ((offset = findOffset(unique_ifaces, nc->src_if)) == -1) {
                    unique_ifaces.push_back(nc->src_if);
                    click_conf << unique_ifaces.size() << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string();
                } else {
                    click_conf << offset + 1 << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string();
                }
                
                // Do not ignore duplicates and use a new port!
            } else if (overlay_mode.compare("mac_qos") == 0) {         
                // Add the interface either way! (Do not count uniques)
                // Add to the map
                IfMapE tmp_ime;
                tmp_ime.fwport = j+1;
                tmp_ime.prio = nc->priority;
                tmp_ime.rate_lim = nc->rate_lim;
                ifmap[nc->src_if].push_back(tmp_ime);
                // Config...
                click_conf << j + 1 << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string();
            } else {
                if ((offset = findOffset(unique_srcips, nc->src_ip)) == -1) {
                    unique_srcips.push_back(nc->src_ip);
                    //cout << "PUSHING BACK " << nc->src_ip << endl;
                    //cout << unique_srcips.size() << endl;
                    click_conf << unique_srcips.size() << "," << nc->src_ip << "," << nc->dst_ip << "," << nc->LID.to_string();
                } else {
                    click_conf << offset + 1 << "," << nc->src_ip << "," << nc->dst_ip << "," << nc->LID.to_string();
                }
            }
            if (j < nn->connections.size() - 1) {
                /* We assume that all if and else clauses above print a line */
                click_conf << "," << endl;
            }
        }
        click_conf << ");" << endl << endl;
        if ((overlay_mode.compare("mac") == 0) || ((overlay_mode.compare("mac_ml") == 0))) {
            for (size_t j = 0; j < unique_ifaces.size(); j++) {
                click_conf << "tsf" << j << "::ThreadSafeQueue(1000);" << endl;
                if (nn->running_mode.compare("user") == 0) {
                    click_conf << "fromdev" << j << "::FromDevice(" << unique_ifaces[j] << ");" << endl << "todev" << j << "::ToDevice(" << unique_ifaces[j] << ");" << endl;
                } else {
                    click_conf << "fromdev" << j << "::FromDevice(" << unique_ifaces[j] << ", BURST 8);" << endl << "todev" << j << "::ToDevice(" << unique_ifaces[j] << ", BURST 8);" << endl;
                }
            }
            /*Necessary Click Elements*/
        } else if (overlay_mode.compare("mac_qos") == 0) {
            // write down the map + the scheduler
            
            // For each interface (PHY in theory)
            IfPortsMap::iterator it = ifmap.begin();
            // Build priority list for the LinkMon module
            stringstream priolist;
            for (; it!=ifmap.end(); ++it){
                // For each port on the FW module (and device)
                for (size_t i = 0; i < it->second.size(); i++){
                    // Add a counter (incoming rate / demand)
                    click_conf << "inC_" << it->first<<"_"<<i+1<<"::Counter();"<<endl;
                    // Add the Q in the middle 
                    click_conf << "tsQ_" << it->first<<"_"<<i+1<<"::ThreadSafeQueue(1000);"<<endl;
                    // Add the outgoing counter (outgoing rate / utilization)
                    click_conf << "outC_" << it->first<<"_"<<i+1<<"::Counter();"<<endl;
                    
                    // Add the outgoing (ofcource) rate limiter!
                    click_conf << "BS_" << it->first<<"_"<<i+1<<"::BandwidthShaper("<<it->second[i].rate_lim<<");"<<endl;
                    
                    priolist<<it->second[i].prio<<",";
                    //if (i<it->second.size()-1) priolist<<",";
                }
                
                // Add the sceduler for this device!
                click_conf << "sc_"<<it->first <<"::SimplePrioSched();"<<endl;
                
                // Add the classifier for this device!
                click_conf << endl << "cf_" << it->first  << "::Classifier(12/080a);" << endl;
                
                // Finally add the device (unique cause it is a map)
                if (nn->running_mode.compare("user") == 0)
                    click_conf << "fromdev_" << it->first << "::FromDevice(" << it->first << ");" << endl << "todev_" << it->first << "::ToDevice(" << it->first << ");" << endl;
                else 
                    click_conf << "fromdev_" << it->first << "::FromDevice(" << it->first << ", BURST 8);" << endl << "todev_" << it->first << "::ToDevice(" << it->first << ", BURST 8);" << endl;
            } // Eo Interfaces
            
            // Add the monitoring element
            // TODO: Pass the intervals as parameters to avoid recompiling
            // Terminate the priolist with a dummy -1 (to avoid removing comma :) sry...)
            click_conf << "lnmon::LinkMon(globalconf,fw, 1, 10,"<<priolist.str()<<"-1);" << endl;
        } else {
            /*raw sockets here*/
            click_conf << "tsf" << "::ThreadSafeQueue(1000);" << endl;
            if (nn->running_mode.compare("user") == 0) {
                click_conf << "rawsocket" << "::RawSocket(UDP, 55555)" << endl;
                //click_conf << "classifier::IPClassifier(dst udp port 55000 and src udp port 55000)" << endl;
            } else {
                cerr << "Something is wrong...I should not build click config using raw sockets for node " << nn->label << "that will run in kernel space" << endl;
            }
        }

        /*Now link all the elements appropriately*/
        click_conf << endl << endl << "proxy[0]->tonetlink;" << endl << "fromnetlink->[0]proxy;" << endl << "localRV[0]->[1]proxy[1]->[0]localRV;" << endl << "proxy[2]-> [0]fw[0] -> [2]proxy;" << endl;
        if ((overlay_mode.compare("mac") == 0) || ((overlay_mode.compare("mac_ml") == 0))) {
            for (size_t j = 0; j < unique_ifaces.size(); j++) {
                if (nn->running_mode.compare("kernel") == 0) {
                    click_conf << endl << "classifier" << j << "::Classifier(12/080a,-);" << endl;
                    if (montoolstub && j == 0 && (nn->running_mode.compare("user") == 0)) {
                        click_conf << "fw[" << (j + 1) << "]->tsf" << j << "->outc::Counter()->todev" << j << ";" << endl;
                        click_conf << "fromdev" << j << "->classifier" << j << "[0]->inc::Counter()  -> [" << (j + 1) << "]fw;" << endl;
                    } else {
                        click_conf << "fw[" << (j + 1) << "]->tsf" << j << "->todev" << j << ";" << endl;
                        click_conf << "fromdev" << j << "->classifier" << j << "[0]->[" << (j + 1) << "]fw;" << endl;
                    }
                    click_conf << "classifier" << j << "[1]->ToHost()" << endl;
                } else {
                    click_conf << endl << "classifier" << j << "::Classifier(12/080a);" << endl;
                    if (montoolstub && j == 0 && (nn->running_mode.compare("user") == 0)) {
                        click_conf << "fw[" << (j + 1) << "]->tsf" << j << "->outc::Counter()->todev" << j << ";" << endl;
                        click_conf << "fromdev" << j << "->classifier" << j << "[0]->inc::Counter()  -> [" << (j + 1) << "]fw;" << endl;
                    } else {
                        click_conf << "fw[" << (j + 1) << "]->tsf" << j << "->todev" << j << ";" << endl;
                        click_conf << "fromdev" << j << "->classifier" << j << "[0]->[" << (j + 1) << "]fw;" << endl;
                    }
                }
            }
        } else if (overlay_mode.compare("mac_qos") == 0) {
            
            // For each interface 
            IfPortsMap::iterator it = ifmap.begin();
            int it_count = 1; // :) 
            for (; it!=ifmap.end(); ++it){
                // For each port on the FW module (and device)
                for (size_t i = 0; i < it->second.size(); i++) {
                    stringstream ss;
                    //ss << it->first << "_" << (it->second[i].fwport);
                    ss << it->first << "_" << (i+1);
                    string ifpname = ss.str();
                    // Connect the fw->inC->Q->BS->outC->scheduler!
                    click_conf << "fw["<<it->second[i].fwport<<"] -> inC_" << ifpname << " -> "<< " tsQ_" << ifpname <<
			" -> BS_" << it->first<<"_"<<((i+1)/* *it_count*/)<<" -> outC_" << ifpname << " -> ["<<i<<"]sc_"<<it->first<<";"<<endl;
                }
                
                // Finally connect scheduler<->Interface
                click_conf << "sc_"<<it->first << " -> " << "todev_"<< it->first << ";" << endl;
                click_conf << "fromdev_"<< it->first << " -> cf_" << it->first << "[0] ->inc_"<< it->first <<"::Counter()  -> [" << (it_count++) << "]fw;" << endl;
            } // Eo Interfaces
            
            // Connect the link monitoring module
            click_conf << "lnmon[0]->[3]proxy[3]->[0]lnmon;" << endl;
        } else {
            /*raw sockets here*/
            if (montoolstub) {
                click_conf << "fw[1] -> tsf -> outc::Counter() -> rawsocket -> IPClassifier(dst udp port 55555 and src udp port 55555)[0]-> inc::Counter()  -> [1]fw" << endl;
            } else {
                click_conf << "fw[1] ->  tsf -> rawsocket -> IPClassifier(dst udp port 55555 and src udp port 55555)[0] -> [1]fw" << endl;
            }
        }
        if (dump_supp && nn->isRV && nn->running_mode.compare("user") == 0) {
            click_conf << "\ncs :: ControlSocket(TCP, 55500);" << endl;
        }
        click_conf.close();
        write_TMFID.close();
    }
}

void Domain::writeNS3ClickFiles() {
    ofstream click_conf;
    for (size_t i = 0; i < network_nodes.size(); i++) {
        vector<string> unique_ifaces;
        NetworkNode *nn = network_nodes[i];
        click_conf.open((write_conf + nn->label + ".conf").c_str());
        /*Blackadder Elements First*/
        click_conf << "globalconf::GlobalConf(MODE mac, NODEID " << nn->label << "," << endl;
        click_conf << "DEFAULTRV " << nn->FID_to_RV.to_string() << "," << endl;
        click_conf << "TMFID     " << nn->FID_to_TM.to_string() << "," << endl;
        click_conf << "iLID      " << nn->iLid.to_string() << ");" << endl << endl;
        click_conf << "localRV::LocalRV(globalconf);" << endl;
        click_conf << "toApps::ToSimDevice(tap0);" << endl << "fromApps::FromSimDevice(tap0);" << endl;
        click_conf << "proxy::LocalProxy(globalconf);" << endl << endl;
        click_conf << "fw::Forwarder(globalconf," << nn->connections.size() << "," << endl;
        for (size_t j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            int offset;
            if ((offset = findOffset(unique_ifaces, nc->src_if)) == -1) {
                unique_ifaces.push_back(nc->src_if);
                if (j == nn->connections.size() - 1) {
                    click_conf << unique_ifaces.size() << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string() << ");" << endl << endl;
                } else {
                    click_conf << unique_ifaces.size() << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string() << "," << endl;
                }
            } else {
                if (j == nn->connections.size() - 1) {
                    click_conf << offset + 1 << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string() << ");" << endl << endl;
                } else {
                    click_conf << offset + 1 << "," << nc->src_mac << "," << nc->dst_mac << "," << nc->LID.to_string() << "," << endl;
                }
            }
        }
        for (size_t j = 0; j < unique_ifaces.size(); j++) {
            click_conf << "tsf" << j << "::ThreadSafeQueue(1000);" << endl;
            click_conf << "fromsimdev" << j << "::FromSimDevice(" << unique_ifaces[j] << ");" << endl << "tosimdev" << j << "::ToSimDevice(" << unique_ifaces[j] << ");" << endl;
        }
        /*Now link all the elements appropriately*/
        click_conf << "proxy[0]->toApps;" << endl << "fromApps->[0]proxy;" << endl << "localRV[0]->[1]proxy[1]->[0]localRV;" << endl << "proxy[2]-> [0]fw[0] -> [2]proxy;" << endl;
        for (size_t j = 0; j < unique_ifaces.size(); j++) {
            click_conf << "fw[" << (j + 1) << "]->tsf" << j << "->tosimdev" << j << ";" << endl;
            click_conf << "fromsimdev" << j << "->[" << (j + 1) << "]fw;" << endl;
        }
        click_conf.close();
    }
}

void Domain::scpTMConfiguration(string TM_conf) {
    FILE *scp_command;
    string command;
    command = "scp " + write_conf + TM_conf + " " + user + "@" + TM_node->testbed_ip + ":" + write_conf;
    cout << command << endl;
    scp_command = popen(command.c_str(), "r");
    /* close */
    pclose(scp_command);
}

void Domain::scpClickFiles() {
    FILE *scp_command;
    string command, fidtm_cmd;
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        if (overlay_mode.compare("mac_ml") == 0 && nn->type.compare("PN") != 0) {
            //avoid scp to optical nodes
            continue;
        }
        command = "scp " + write_conf + nn->label + ".conf" + " " + user + "@" + nn->testbed_ip + ":" + write_conf;
        fidtm_cmd= "scp " + write_conf + nn->label + "_TMFID.txt" + " " + user + "@" + nn->testbed_ip + ":" + write_conf;
        cout << command << endl;
        scp_command = popen(command.c_str(), "r");
        if (scp_command == NULL) {
            cerr << "Failed to scp click file to node " << nn->label << endl;
        }
        pclose(scp_command);
        cout << fidtm_cmd << endl;
        scp_command = popen(fidtm_cmd.c_str(), "r");
        if (scp_command == NULL) {
            cerr << "Failed to scp TMFID file to node " << nn->label << endl;
        }
        /* close */
        pclose(scp_command);
    }
}

void Domain::startClick() {
    FILE *ssh_command;
    string command;
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];
        if (overlay_mode.compare("mac_ml") == 0 && nn->type.compare("PN") != 0) {
            // avoid starting Click in optical nodes
            continue;
        }
        /*kill click first both from kernel and user space*/
        if (sudo) {
            command = "ssh " + user + "@" + nn->testbed_ip + " -t \"sudo pkill -9 click\"";
        } else {
            command = "ssh " + user + "@" + nn->testbed_ip + " -t \"pkill -9 click\"";
        }
        cout << command << endl;
        ssh_command = popen(command.c_str(), "r");
        if (ssh_command == NULL) {
            cerr << "Failed to stop click at node " << nn->label << endl;
        }
        pclose(ssh_command);
        if (sudo) {
            command = "ssh " + user + "@" + nn->testbed_ip + " -t \"sudo " + click_home + "sbin/click-uninstall\"";
        } else {
            command = "ssh " + user + "@" + nn->testbed_ip + " -t \"" + click_home + "sbin/click-uninstall \"";
        }
        cout << command << endl;
        ssh_command = popen(command.c_str(), "r");
        if (ssh_command == NULL) {
            cerr << "Failed to stop click at node " << nn->label << endl;
        }
        pclose(ssh_command);
        /*now start click*/
        if (nn->running_mode.compare("user") == 0) {
            if (sudo) {
                command = "ssh " + user + "@" + nn->testbed_ip + " \"sudo " + click_home + "bin/click " + write_conf + nn->label + ".conf > /dev/null 2>&1 &\"";
            } else {
                command = "ssh " + user + "@" + nn->testbed_ip + " \"" + click_home + "bin/click " + write_conf + nn->label + ".conf > /dev/null 2>&1 &\"";
            }
            cout << command << endl;
            ssh_command = popen(command.c_str(), "r");
            if (ssh_command == NULL) {
                cerr << "Failed to start click at node " << nn->label << endl;
            }
            pclose(ssh_command);
        } else {
            if (sudo) {
                command = "ssh " + user + "@" + nn->testbed_ip + " \"sudo " + click_home + "sbin/click-install " + write_conf + nn->label + ".conf > /dev/null 2>&1 &\"";
            } else {
                command = "ssh " + user + "@" + nn->testbed_ip + " \"" + click_home + "sbin/click-install " + write_conf + nn->label + ".conf > /dev/null 2>&1 &\"";
            }
            cout << command << endl;
            ssh_command = popen(command.c_str(), "r");
            if (ssh_command == NULL) {
                cerr << "Failed to start click at node " << nn->label << endl;
            }
            pclose(ssh_command);
        }
    }
}

void Domain::startTM() {
    FILE *ssh_command;
    string command;
    /*kill the topology manager first*/
    command = "ssh " + user + "@" + TM_node->testbed_ip + " -t \"pkill -9 tm\"";
    cout << command << endl;
    ssh_command = popen(command.c_str(), "r");
    if (ssh_command == NULL) {
        cerr << "Failed to stop Topology Manager at node " << TM_node->label << endl;
    }
    pclose(ssh_command);
    /*now start the TM*/
    command = "ssh " + user + "@" + TM_node->testbed_ip + " -t \"/home/" + user + "/blackadder/TopologyManager/tm " + write_conf + "topology.graphml > /dev/null 2>&1 &\"";
    cout << command << endl;
    ssh_command = popen(command.c_str(), "r");
    if (ssh_command == NULL) {
        cerr << "Failed to start Topology Manager at node " << TM_node->label << endl;
    }
    pclose(ssh_command);
}

void Domain::scpClickBinary(string tgzfile) {
    string command;
    for (size_t i = 0; i < network_nodes.size(); i++) {
        NetworkNode *nn = network_nodes[i];

        command = "scp ./" + tgzfile + "  " + user + "@" + nn->testbed_ip + ":";
        cout << command << endl;
        system(command.c_str());

        command = "ssh -X " + user + "@" + nn->testbed_ip + " -t 'tar zxvf ~/" + tgzfile + "'";
        cout << command << endl;
        system(command.c_str());

    }
}



// Planetlab Support starts Here

string Domain::writeConfigFile(string filename) {

    ofstream configfile;

    for (size_t i = 0; i < network_nodes.size(); i++) {
        configfile.open((write_conf + filename).c_str());
        //Domain Global stuff here
        configfile << "BLACKADDER_ID_LENGTH = " << ba_id_len << ";\n";
        configfile << "LIPSIN_ID_LENGTH = " << fid_len << ";\n";
        configfile << "CLICK_HOME = \"" << click_home << "\";\n";
        configfile << "WRITE_CONF = \"" << write_conf << "\";\n";
        configfile << "USER = \"" << user << "\";\n";
        if (sudo == 1) {
            configfile << "SUDO = true;\n";
        } else {
            configfile << "SUDO = false;\n";
        }
        configfile << "OVERLAY_MODE = \"" << overlay_mode << "\";\n\n\n";
        //network
        configfile << "network = {\n";
        configfile << "    nodes = (\n";
        if (overlay_mode.compare("ip") == 0) {
            for (size_t i = 0; i < network_nodes.size(); i++) {
                NetworkNode *nn = network_nodes[i];
                configfile << "     {\n";
                configfile << "       testbed_ip = \"" << nn->testbed_ip << "\";\n";
                configfile << "       running_mode = \"" << nn->running_mode << "\";\n";
                configfile << "       label = \"" << nn->label << "\";\n";
                if ((nn->isRV == true) && (nn->isTM == true)) {
                    configfile << "       role = [\"RV\",\"TM\"];\n";
                } else if (nn->isRV == true) {
                    configfile << "       role = [\"RV\"];\n";
                } else if (nn->isTM == true) {
                    configfile << "       role = [\"TM\"];\n";
                }
                configfile << "       connections = (\n";
                for (size_t j = 0; j < nn->connections.size(); j++) {
                    NetworkConnection *nc = nn->connections[j];
                    configfile << "           {\n";
                    configfile << "               to = \"" << nc->dst_label << "\";\n";
                    configfile << "               src_ip = \"" << nc->src_ip << "\";\n";
                    configfile << "               dst_ip = \"" << nc->dst_ip << "\";\n";
                    if (j != (nn->connections.size() - 1)) {
                        configfile << "           },\n";
                    } else {
                        configfile << "           }\n";
                    }
                }
                configfile << "        );\n";
                if (i != (network_nodes.size() - 1)) {
                    configfile << "     },\n";
                } else {
                    configfile << "     }\n";
                }
            }

        } else {
#if 0
            for (size_t i = 0; i < network_nodes.size(); i++) {
                NetworkNode *nn = network_nodes[i];

                for (size_t j = 0; j < nn->connections.size(); j++) {
                    NetworkConnection *nc = nn->connections[j];
                }
            }
#endif
        }
        configfile << "    );\n};";
    }

    configfile.close();
    return (write_conf + filename);
}


