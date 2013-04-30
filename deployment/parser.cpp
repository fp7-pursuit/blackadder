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


#include "parser.hpp"

Parser::Parser(char *_file, Domain *_dm) {
    file = _file;
    dm = _dm;
    dm->number_of_connections = 0;
    dm->number_of_p_connections = 0;
}

Parser::~Parser() {
}

int Parser::parseConfiguration() {
    try {
        cfg.readFile(file);
        cout << "I read configuration " << file << endl;
        return 0;
    } catch (const FileIOException &fioex) {
        std::cerr << "I/O error while reading file." << std::endl;
        return -1;
    } catch (const ParseException &pex) {
	std::cerr << "Parse error while reading file. " << std::endl;
        return -1;
    }
}

int Parser::getGlobalDomainParameters() {
    try {
        dm->ba_id_len = cfg.lookup("BLACKADDER_ID_LENGTH");
        //cout << "BLACKADDER_ID_LENGTH: " << ba_id_len << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option BLACKADDER_ID_LENGTH is missing" << endl;
        return -1;
    }
    try {
        dm->fid_len = cfg.lookup("LIPSIN_ID_LENGTH");
        //cout << "LIPSIN_ID_LENGTH: " << fid_len << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option LIPSIN_ID_LENGTH is missing" << endl;
        return -1;
    }
    try {
        cfg.lookupValue("CLICK_HOME", dm->click_home);
        if (dm->click_home[dm->click_home.length() - 1] != '/') {
            dm->click_home.append("/"); /* XXX: default separator assumed */
        }
        cout << "Click Home: " << dm->click_home << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option CLICK_HOME is missing" << endl;
        return -1;
    }
    try {
        cfg.lookupValue("WRITE_CONF", dm->write_conf);
        if (dm->write_conf[dm->write_conf.length() - 1] != '/') {
            dm->write_conf.append("/"); /* XXX: default separator assumed */
        }
        if (dm->write_conf.find_first_of('\"') != string::npos) {
            /* XXX: this is only a very basic sanity check */
            cerr << "bad write_conf string" << endl;
            return -1;
        }
        string dir_exists_cmd = "/usr/bin/env test -d \"" + dm->write_conf + "\"";
        int dir_exists = system(dir_exists_cmd.c_str());
        if (dir_exists != 0) {
            cerr << "output directory " << dm->write_conf << " doesn't exist or is not a directory" << endl;
            return -1;
        }
        cout << "Write conf files in: " << dm->write_conf << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option WRITE_CONF is missing" << endl;
        return -1;
    }
    try {
        cfg.lookupValue("USER", dm->user);
        cout << "User: " << dm->user << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option USER is missing" << endl;
        return -1;
    }
    try {
        cfg.lookupValue("SUDO", dm->sudo);
        cout << "Sudo: " << dm->sudo << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option SUDO is missing" << endl;
        return -1;
    }
    try {
        cfg.lookupValue("OVERLAY_MODE", dm->overlay_mode);
        cout << "OVERLAY_MODE: " << dm->overlay_mode << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option OVERLAY_MODE is missing" << endl;
        return -1;
    }
    return 0;
}

int Parser::getNS3GlobalDomainParameters() {
    try {
        dm->ba_id_len = cfg.lookup("BLACKADDER_ID_LENGTH");
        cout << "BLACKADDER_ID_LENGTH: " << dm->ba_id_len << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option BLACKADDER_ID_LENGTH is missing" << endl;
        return -1;
    }
    try {
        dm->fid_len = cfg.lookup("LIPSIN_ID_LENGTH");
        cout << "LIPSIN_ID_LENGTH: " << dm->fid_len << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option LIPSIN_ID_LENGTH is missing" << endl;
        return -1;
    }
    try {
        cfg.lookupValue("WRITE_CONF", dm->write_conf);
        cout << "Write conf files in: " << dm->write_conf << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option WRITE_CONF is missing" << endl;
        return -1;
    }
    return 0;
}

int Parser::addConnection(const Setting &connection, NetworkNode *nn) {
    NetworkConnection *nc = new NetworkConnection();
    string in_pt;
    string out_pt;
    string src_if;
    string src_ip;
    string dst_if;
    string dst_ip;
    string src_mac;
    string dst_mac;
    nc->src_label = nn->label;
    /***********Parse the destination label**************/
    string dst_label;
    if (connection.lookupValue("to", dst_label)) {
        if (dst_label.length() != (size_t)dm->ba_id_len) {
            cerr << "destination label in the configuration file has BAD label size " << dst_label.length() << endl;
            return -1;
        } else {
            nc->dst_label = dst_label;
        }
    } else {
        cout << "missing mandatory destination label parameter for connection of node " << nn->label << endl;
        return -1;
    }
    /***********Parse the connection type****************/
    /***Parse the src_if or src_ip or out_pt depending on mode****/
    if (dm->overlay_mode.compare("mac") == 0 || dm->overlay_mode.compare("mac_qos") == 0) {
        if (connection.lookupValue("src_if", src_if)) {
            nc->src_if = src_if;
        } else {
            cout << "missing mandatory src_if parameter for connection of node " << nn->label << endl;
            return -1;
        }
    } else if (dm->overlay_mode.compare("mac_ml") == 0) {
        if (connection.lookupValue("out_pt", out_pt)) {
            nc->out_pt = out_pt;
        } else if (connection.lookupValue("src_if", src_if)) {
            nc->src_if=src_if;
        } else {
            cout << "missing mandatory in_pt or src_if parameter for connection of node " << nn->label << endl;
            return -1;
        } 
    } else {
        if (connection.lookupValue("src_ip", src_ip)) {
            nc->src_ip = src_ip;
        } else {
            cout << "missing mandatory src_ip parameter for connection of node " << nn->label << endl;
            return -1;
        }
    }
    /***Parse the dst_if or dst_ip depending on mode****/
    if (dm->overlay_mode.compare("mac") == 0 || dm->overlay_mode.compare("mac_qos") == 0) {
        if (connection.lookupValue("dst_if", dst_if)) {
            nc->dst_if = dst_if;
        } else {
            cout << "missing mandatory dst_if parameter for connection of node " << nn->label << endl;
            return -1;
        }
    } else if (dm->overlay_mode.compare("mac_ml") == 0) {
        if (connection.lookupValue("in_pt", in_pt)) {
            nc->in_pt = in_pt;
        } else {
            if (connection.lookupValue("dst_if",dst_if)) {
                nc->dst_if = dst_if;
            }
        }
    } else {
        if (connection.lookupValue("dst_ip", dst_ip)) {
            nc->dst_ip = dst_ip;
        } else {
            cout << "missing mandatory dst_ip parameter for connection of node " << nn->label << endl;
            return -1;
        }
    }
    /*check if a mac address was hardcoded - e.g. for laptops in the testbed*/
    if (dm->overlay_mode.compare("mac") == 0 || dm->overlay_mode.compare("mac_qos") == 0) {
        if (connection.lookupValue("src_mac", src_mac)) {
            nc->src_mac = src_mac;
        }
        if (connection.lookupValue("dst_mac", dst_mac)) {
            nc->dst_mac = dst_mac;
        }
    }
    /*set the type of the link according to the connection ends: pure overlay layer "oo", pure packet layer "pp", or cross layer "cl"*/
    if (dm->overlay_mode.compare("mac_ml") == 0) {
        //adding the type parameter to the link
        if (((nc->src_if != "") && (nc->dst_if != "")) || ((nc->src_mac != "") && (nc->dst_mac != ""))) {
            nc->lnk_type ="pp";
            dm->number_of_p_connections++;
        } else if ((nc->out_pt != "") && (nc->in_pt != "")) {
            nc->lnk_type="oo";
        } else {
            nc->lnk_type = "cl";
        }
        // check if a mac address was hardcoded
        if (connection.lookupValue("src_mac", src_mac)) {
            nc->src_mac = src_mac;
        }
        if (connection.lookupValue("dst_mac", dst_mac)) {
            nc->dst_mac = dst_mac;
        }
    }
    
    // get the link priority for scheduling
    nc->priority=0; // default - BE
    connection.lookupValue("priority", nc->priority);
    
    // get link rate limit
    nc->rate_lim=1250000000;
    connection.lookupValue("rate_lim", nc->rate_lim);
    
    nn->connections.push_back(nc);
    dm->number_of_connections++;
    return 0;
}

int Parser::addNS3Connection(const Setting &connection, NetworkNode *nn) {
    NetworkConnection *nc = new NetworkConnection();
    nc->src_label = nn->label;
    /***********Parse the destination label**************/
    string dst_label;
    if (connection.lookupValue("to", dst_label)) {
        if (dst_label.length() != (size_t)dm->ba_id_len) {
            cerr << "destination label in the configuration file has BAD label size " << dst_label.length() << endl;
            return -1;
        } else {
            nc->dst_label = dst_label;
        }
    } else {
        cout << "missing mandatory destination label parameter for connection of node " << nn->label << endl;
        return -1;
    }
    if (!connection.lookupValue("Mtu", nc->mtu)) {
        cout << "missing mandatory NS3-related Mtu value " << nn->label << endl;
        return -1;
    }
    if (!connection.lookupValue("DataRate", nc->rate)) {
        cout << "missing mandatory NS3-related DataRate value " << nn->label << endl;
        return -1;
    }
    if (!connection.lookupValue("Delay", nc->delay)) {
        cout << "missing mandatory NS3-related Delay value " << nn->label << endl;
        return -1;
    }
    nn->connections.push_back(nc);
    dm->number_of_connections++;
    return 0;
}

int Parser::addNS3Application(const Setting &application, NetworkNode *nn) {
    string attributes;
    NS3Application *app = new NS3Application();
    if (!application.lookupValue("name", app->name)) {
        cout << "missing mandatory name for NS3 application " << nn->label << endl;
        return -1;
    }
    if (!application.lookupValue("start", app->start)) {
        cout << "missing mandatory start for NS3 application " << nn->label << endl;
        return -1;
    }
    if (!application.lookupValue("stop", app->stop)) {
        cout << "missing mandatory stop for NS3 application " << nn->label << endl;
        return -1;
    }
    nn->applications.push_back(app);
    if (application.lookupValue("attributes", attributes)) {
        int startindex = 0;
        int index1 = 0;
        int index2 = 0;
        do {
            index1 = attributes.find('=', startindex);
            index2 = attributes.find(',', startindex);
            string name = attributes.substr(startindex, index1 - startindex);
            string value = attributes.substr(index1 + 1, index2 - index1 - 1);
            NS3ApplicationAttribute *attr = new NS3ApplicationAttribute();
            attr->name = name;
            attr->value = value;
            app->attributes.push_back(attr);
            startindex = index2 + 1;
        } while (((size_t)index1 != string::npos) && ((size_t)index2 != string::npos));
    }
    return 0;
}

int Parser::addNode(const Setting &node) {
    int ret;
    NetworkNode *nn = new NetworkNode();
    string node_label;
    string testbed_ip;
    string running_mode;
    string operating_system;
    //string node_name;
    string node_type; /*overlay or packet node*/   
    /**********************************Parse the Node Label*****************************************/
    if (!node.lookupValue("label", node_label)) {
        cout << "I will create a random label for a node in the configuration file (not supported yet)" << endl;
    } else {
        if (node_label.length() != (size_t)dm->ba_id_len) {
            cerr << "Node in the configuration file has BAD label size " << node_label.length() << endl;
            return -1;
        } else {
            nn->label = node_label;
            if (node.lookupValue("type", node_type)) {
                nn->type = node_type;
            }
        }
    }
    /*********************Parse the IP (for ssh'ing in the testbed only!!)**************************/
    if (node.lookupValue("testbed_ip", testbed_ip)) {
        /*I will assume that the IP is valid*/
        nn->testbed_ip = testbed_ip;
    } else {
        cerr << "testbed_ip conf parameter is mandatory for all nodes...missing from node " << node_label << endl;
        return -1;
    }
    if (node.lookupValue("running_mode", running_mode)) {
        /*I will assume that the IP is valid*/
        nn->running_mode = running_mode;
        if ((running_mode.compare("kernel") == 0) && (dm->overlay_mode.compare("mac") != 0)) {
            cerr << "Cannot run over IP in kernel mode" << endl;
            return -1;
        }
    } else {
        cerr << "running_mode conf parameter is mandatory for all nodes...missing from node " << node_label << endl;
        return -1;
    }
    if (node.lookupValue("operating_system", operating_system)) {
        nn->operating_system = operating_system;
        if (operating_system.compare("Linux") != 0
            && operating_system.compare("FreeBSD") != 0
            && operating_system.compare("Darwin") != 0) {
            cerr << "Unrecognized operating system for node " << node_label << endl;
            return -1;
        }
        if ((operating_system.compare("FreeBSD") == 0
             || operating_system.compare("Darwin") == 0)
            && (dm->overlay_mode.compare("mac") != 0)) {
            cerr << "Cannot run over IP in FreeBSD or Darwin" << endl;
            return -1;
        }
        if (operating_system.compare("Darwin") == 0
            && (nn->running_mode.compare("user") != 0)) {
            cerr << "Cannot run in kernel mode in Darwin" << endl;
            return -1;
        }
    } else {
        operating_system = "Linux";
        nn->operating_system = operating_system;
    }
#if 0
    if (node.lookupValue("name", node_name)) {
        nn->name = node_name;
    } else {
        node_name = nn->label;
        nn->name = node_name;
    }
#endif
    /***********Parse the roles..no role or role = []; mean no special functionality****************/
    /*role = ["TM", "RV"]; for both roles*/
    int number_of_roles;
    try {
        number_of_roles = node["role"].getLength();
    } catch (const SettingNotFoundException &nfex) {
        number_of_roles = 0;
    }
    if (number_of_roles <= 2) {
        nn->isRV = false;
        nn->isTM = false;
        if (number_of_roles == 0) {
            cout << "node " << node_label << " is a regular network node" << endl;
        } else {
            for (int role_counter = 0; role_counter < number_of_roles; role_counter++) {
                string role = node["role"][role_counter];
                if (role.compare("RV") == 0) {
                    nn->isRV = true;
                    if (dm->RV_node == NULL) {
                        cout << "node " << node_label << " is the RV node" << endl;
                        dm->RV_node = nn;
                    } else {
                        cerr << "multiple RV nodes are assigned - this is currently wrong" << endl;
                        return -1;
                    }
                }
                if (role.compare("TM") == 0) {
                    nn->isTM = true;
                    if (dm->TM_node == NULL) {
                        cout << "node " << node_label << " is the TM node" << endl;
                        dm->TM_node = nn;
                    } else {
                        cerr << "multiple TM nodes are assigned - this is currently wrong" << endl;
                        return -1;
                    }
                }
            }
        }
    } else {
        cerr << "More than two roles is wrong!" << endl;
        return -1;
    }
    /**********************************Parse the Connections*****************************************/
    try {
        const Setting &connections = node["connections"];
        int number_of_connections = connections.getLength();
        if (number_of_connections > 0) {
// 	    cout << "addNode: number_of_connections is: "<< number_of_connections << endl;
            for (int connections_counter = 0; connections_counter < number_of_connections; connections_counter++) {
                const Setting &connection = connections[connections_counter];
                ret = addConnection(connection, nn);
                if (ret < 0) {
//		    cout << "addConnection: ret is: "<< ret << endl;
                    return -1;
                }
            }
        }
        dm->network_nodes.push_back(nn);
    } catch (SettingNotFoundException) {
        cout << "no connections for node " << nn->label << endl;
        dm->network_nodes.push_back(nn);
        return 0;
    }
    return 0;
}

int Parser::addNS3Node(const Setting &node) {
    int ret;
    NetworkNode *nn = new NetworkNode();
    string node_label;
    /**********************************Parse the Node Label*****************************************/
    if (!node.lookupValue("label", node_label)) {
        cout << "I will create a random label for a node in the configuration file (not supported yet)" << endl;
    } else {
        if (node_label.length() != (size_t)dm->ba_id_len) {
            cerr << "Node in the configuration file has BAD label size " << node_label.length() << endl;
            return -1;
        } else {
            nn->label = node_label;
        }
    }
    /***********Parse the roles..no role or role = []; mean no special functionality****************/
    /*role = ["TM", "RV"]; for both roles*/
    int number_of_roles;
    try {
        number_of_roles = node["role"].getLength();
    } catch (const SettingNotFoundException &nfex) {
        number_of_roles = 0;
    }
    if (number_of_roles <= 2) {
        nn->isRV = false;
        nn->isTM = false;
        if (number_of_roles == 0) {
            cout << "node " << node_label << " is a regular network node" << endl;
        } else {
            for (int role_counter = 0; role_counter < number_of_roles; role_counter++) {
                string role = node["role"][role_counter];
                if (role.compare("RV") == 0) {
                    nn->isRV = true;
                    if (dm->RV_node == NULL) {
                        cout << "node " << node_label << " is the RV node" << endl;
                        dm->RV_node = nn;
                    } else {
                        cerr << "multiple RV nodes are assigned - this is currently wrong" << endl;
                        return -1;
                    }
                }
                if (role.compare("TM") == 0) {
                    nn->isTM = true;
                    if (dm->TM_node == NULL) {
                        cout << "node " << node_label << " is the TM node" << endl;
                        dm->TM_node = nn;
                    } else {
                        cerr << "multiple TM nodes are assigned - this is currently wrong" << endl;
                        return -1;
                    }
                }
            }
        }
    } else {
        cerr << "More than two roles is wrong!" << endl;
        return -1;
    }
    /**********************************Parse the Connections*****************************************/
    try {
        const Setting &connections = node["connections"];
        int number_of_connections = connections.getLength();
        if (number_of_connections > 0) {
            for (int connections_counter = 0; connections_counter < number_of_connections; connections_counter++) {
                const Setting &connection = connections[connections_counter];
                ret = addNS3Connection(connection, nn);
                if (ret < 0) {
                    return -1;
                }
            }
        }
        dm->network_nodes.push_back(nn);
    } catch (SettingNotFoundException) {
        cout << "no connections for this node" << endl;
        dm->network_nodes.push_back(nn);
        return 0;
    }
    try {
        const Setting &applications = node["applications"];
        int number_of_applications = applications.getLength();
        if (number_of_applications > 0) {
            for (int applications_counter = 0; applications_counter < number_of_applications; applications_counter++) {
                const Setting &application = applications[applications_counter];
                ret = addNS3Application(application, nn);
                if (ret < 0) {
                    return -1;
                }
            }
        }
    } catch (SettingNotFoundException) {
        cout << "no applications" << endl;
    }
    return 0;
}

int Parser::buildNetworkDomain() {
    int ret;
    ret = parseConfiguration();
    if (ret < 0) {
        cerr << "could not read configuration file " << file << endl;
        return ret;
    }
    ret = getGlobalDomainParameters();
    if (ret < 0) {
        cerr << "could not get global parameters from " << file << endl;
        return ret;
    }
    const Setting& root = cfg.getRoot();
//    cout << "root is:" << &root << endl; //Checkup point to see if root is returning something
    
    if (dm->overlay_mode.compare("mac_ml") != 0) {
        try {
            const Setting &nodes = root["network"]["nodes"];
            dm->number_of_nodes = nodes.getLength();
            cout << "Number of nodes in the topology: " << dm->number_of_nodes << endl;
            for (unsigned int node_counter = 0; node_counter < dm->number_of_nodes; ++node_counter) {
                const Setting &node = nodes[node_counter];
                ret = addNode(node);
                if (ret < 0) {
                    return -1;
                }
            }
        } catch (const SettingNotFoundException &nfex) {
            return -1;
            cerr << "SettingNotFoundException" << endl;
        }
    } else {
        // Parse a multilayer configuration file
        try {
            const Setting &ol_nodes = root["network"]["ol_nodes"];
            const Setting &pl_nodes = root ["network"]["pl_nodes"];
            dm->number_of_pl_nodes = pl_nodes.getLength();
            dm->number_of_nodes = ol_nodes.getLength() + pl_nodes.getLength();
            int number_of_ol_nodes = dm->number_of_nodes - dm->number_of_pl_nodes;
            
            //here we need to add dm->number_of_en_nodes = el_nodes.getLength()
            
            cout << "Number of nodes in the topology: " << dm->number_of_nodes << endl;
            cout<< "Number of Electronic nodes in the topology: " << pl_nodes.getLength() << endl;
            for (int node_counter = 0; node_counter < number_of_ol_nodes; ++node_counter) {
                const Setting &ol_node = ol_nodes[node_counter];
                ret = addNode(ol_node);
                cout <<"parser ret is :" << ret << endl;
                if (ret < 0) {
                    return -1;
                }
            }
            for (unsigned int node_counter = 0; node_counter < dm->number_of_pl_nodes; ++node_counter) {
                const Setting &pl_node = pl_nodes[node_counter];
                ret = addNode(pl_node);
                cout <<"parser ret is :" << ret << endl;
                if (ret < 0) {
                    return -1;
                }
            }        
        } catch (const SettingNotFoundException &nfex) {
            cerr << "SettingNotFoundException" << endl;
            return 0;			// XXX: this should go back to -1 not 0
        }
    }
    return 0;
}

int Parser::buildNS3NetworkDomain() {
    int ret;
    ret = parseConfiguration();
    if (ret < 0) {
        cerr << "could not read configuration file " << file << endl;
        return ret;
    }
    ret = getNS3GlobalDomainParameters();
    if (ret < 0) {
        cerr << "could not get global parameters for the NS3 simulation" << file << endl;
        return ret;
    }
    const Setting& root = cfg.getRoot();
    try {
        const Setting &nodes = root["network"]["nodes"];
        dm->number_of_nodes = nodes.getLength();
        cout << "Number of nodes in the topology: " << dm->number_of_nodes << endl;
        for (unsigned int node_counter = 0; node_counter < dm->number_of_nodes; ++node_counter) {
            const Setting &node = nodes[node_counter];
            ret = addNS3Node(node);
            if (ret < 0) {
                return -1;
            }
        }
    } catch (const SettingNotFoundException &nfex) {
        return -1;
        cerr << "SettingNotFoundException" << endl;
    }
    return 0;
}

//PlanetLab Support Starts Here

int Parser::addPlanetLabNode(const Setting &node) {
    cout << "Building planetlab node" << endl;
    NetworkNode *nn = new NetworkNode();
    string testbed_ip;
    //For now only testbed IPs for planetlab nodes should be added
    if (node.lookupValue("testbed_ip", testbed_ip)) {
        /*I will assume that the IP is valid*/
        nn->testbed_ip = testbed_ip;
    } else {
        cerr << "testbed_ip conf parameter is mandatory for all Planetlab nodes...missing from node " << endl;
        return -1;
    }
    /***********Parse the roles..no role or role = []; mean no special functionality****************/
    /*role = ["TM", "RV"]; for both roles*/
    int number_of_roles;
    try {
        number_of_roles = node["role"].getLength();
    } catch (const SettingNotFoundException &nfex) {
        number_of_roles = 0;
    }
    if (number_of_roles <= 2) {
        nn->isRV = false;
        nn->isTM = false;
        if (number_of_roles == 0) {
            cout << "node  is a regular network node" << endl;
        } else {
            for (int role_counter = 0; role_counter < number_of_roles; role_counter++) {
                string role = node["role"][role_counter];
                if (role.compare("RV") == 0) {
                    nn->isRV = true;
                    if (dm->RV_node == NULL) {
                        cout << "node  is the RV node" << endl;
                        dm->RV_node = nn;
                    } else {
                        cerr << "multiple RV nodes are assigned - this is currently wrong" << endl;
                        return -1;
                    }
                }
                if (role.compare("TM") == 0) {
                    nn->isTM = true;
                    if (dm->TM_node == NULL) {
                        cout << "node  is the TM node" << endl;
                        dm->TM_node = nn;
                    } else {
                        cerr << "multiple TM nodes are assigned - this is currently wrong" << endl;
                        return -1;
                    }
                }
            }
        }
    } else {
        cerr << "More than two roles is wrong!" << endl;
        return -1;
    }
    dm->network_nodes.push_back(nn);
    return 0;
}

int Parser::buildPlanetLabDomain() {
    int ret;
    ret = parseConfiguration();
    if (ret < 0) {
        cerr << "could not read configuration file " << file << endl;
        return ret;
    }
    const Setting& root = cfg.getRoot();
    try {
        const Setting &nodes = root["network"]["nodes"];
        dm->number_of_nodes = nodes.getLength();
        cout << "Number of nodes in the topology: " << dm->number_of_nodes << endl;
        /*For each node I will create an entry in the iGraph*/
        /*if an ID (with correct size according to the BLACKADDER_ID_LENGTH) is provided I will use that one. In any other case, I will create a randomID*/
        for (unsigned int node_counter = 0; node_counter < dm->number_of_nodes; ++node_counter) {
            const Setting &node = nodes[node_counter];
            ret = addPlanetLabNode(node);
            if (ret < 0) {
                return -1;
            }
        }
    } catch (const SettingNotFoundException &nfex) {
        return -1;
        cerr << "SettingNotFoundException" << endl;
    }
    return 0;

}

