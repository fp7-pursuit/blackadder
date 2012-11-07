/*
 * Copyright (C) 2010-2011  George Parisis and Dirk Trossen
 * All rights reserved.
 * PlanetLab Deployment support By Dimitris Syrivelis
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

#include <vector>

#include "graph_representation.hpp"

GraphRepresentation::GraphRepresentation(Domain *_dm, bool autogenerate) {
    dm = _dm;
    if (autogenerate) {
        igraph_i_set_attribute_table(&igraph_cattribute_table);
#if IGRAPH_V >= IGRAPH_V_0_6
        igraph_barabasi_game(&igraph, dm->number_of_nodes, 1, 1,
                             NULL, false, 1, true,
                             IGRAPH_BARABASI_BAG, NULL);
#else
        igraph_barabasi_game(&igraph, dm->number_of_nodes, 1,
                             NULL, false, true);
#endif
    }
}

void GraphRepresentation::buildIGraphTopology() {
    int source_vertex_id, destination_vertex_id;
    //int egress_id, ingress_id;
    igraph_i_set_attribute_table(&igraph_cattribute_table);
    igraph_empty(&igraph, 0, true);
    //cout << "iGraph: number of nodes: " << dm->number_of_nodes << endl;
    //cout << "iGraph: number number_of_connections nodes: " << dm->number_of_connections << endl;
    for (size_t i = 0; i < dm->network_nodes.size(); i++) {
        NetworkNode *nn = dm->network_nodes[i];
        bool unconnected = (nn->connections.size() == 0); /* XXX */
        if (unconnected) {
            /* XXX: Create a temporary pseudo-connection for a node without any connections (only iLID).
             * We do this so that the data structures for this node get initialized (below) in the same way as for connected nodes.
             * Note that this probably makes sense mainly in a single-node scenario. */
            cerr << "Warning: " << nn->label << " not connected" << endl;
            NetworkConnection *nc = new NetworkConnection();
            nc->src_label = nn->label;
            nc->dst_label = nn->label;
            nn->connections.push_back(nc);
            unconnected = true;
        }
        for (size_t j = 0; j < nn->connections.size(); j++) {
            NetworkConnection *nc = nn->connections[j];
            map <string, int>::iterator index_it;
            /*find that node - if exists*/
            index_it = reverse_node_index.find(nc->src_label);
            /*check if the source node exists in the reverse index*/
            if (index_it == reverse_node_index.end()) {
                /*it does not exist...add it*/
                source_vertex_id = igraph_vcount(&igraph);
                igraph_add_vertices(&igraph, 1, 0);
                reverse_node_index.insert(pair<string, int>(nc->src_label, source_vertex_id));
                igraph_cattribute_VAS_set(&igraph, "NODEID", source_vertex_id, nc->src_label.c_str());
                //igraph_cattribute_VAS_set(&igraph, "NODENAME", source_vertex_id, nn->name.c_str());
            } else {
                source_vertex_id = (*index_it).second;
            }
            index_it = reverse_node_index.find(nc->dst_label);
            /*check if the destination node exists in the reverse index*/
            if (index_it == reverse_node_index.end()) {
                /*it does not exist...add it*/
                destination_vertex_id = igraph_vcount(&igraph);
                igraph_add_vertices(&igraph, 1, 0);
                reverse_node_index.insert(pair<string, int>(nc->dst_label, destination_vertex_id));
                igraph_cattribute_VAS_set(&igraph, "NODEID", destination_vertex_id, nc->dst_label.c_str());
                for (size_t j = 0; j < dm->network_nodes.size(); j++) {
                    if (dm->network_nodes[j]->label == nc->dst_label) {
                        //igraph_cattribute_VAS_set(&igraph, "NODENAME", destination_vertex_id, dm->network_nodes[j]->name.c_str());
                        break;
                    }
                }
                //cout << "added node " << nc->dst_label << " in the igraph" << endl;
            } else {
                destination_vertex_id = (*index_it).second;
            }
            if (!unconnected) {
                /*add an edge in the graph*/
                igraph_add_edge(&igraph, source_vertex_id, destination_vertex_id);
                igraph_cattribute_EAS_set(&igraph, "LID", igraph_ecount(&igraph) - 1, nc->LID.to_string().c_str());
                if (dm->overlay_mode.compare("mac_ml") == 0) {
                    igraph_cattribute_EAS_set(&igraph, "LTYPE", igraph_ecount(&igraph) - 1, nc->lnk_type.c_str());
                }
            }
        }
        if (unconnected) {
            /* Remove temorary pseudo-connection. */
            NetworkConnection *nc = nn->connections.back();
            nn->connections.pop_back();
            delete nc;
        }
    }
    for (size_t i = 0; i < dm->network_nodes.size(); i++) {
        NetworkNode *nn = dm->network_nodes[i];
        map<string,int>::iterator index_it = reverse_node_index.find(nn->label);
        if (index_it == reverse_node_index.end()) {
            cerr << "Warning: " << nn->label << " not found in reverse node index map" << endl;
            continue;
        }
        int vertex_index = index_it->second;
        igraph_cattribute_VAS_set(&igraph, "iLID", vertex_index, nn->iLid.to_string().c_str());
        if (dm->overlay_mode.compare("mac_ml") == 0) {
            igraph_cattribute_VAS_set(&igraph, "NTYPE", vertex_index, nn->type.c_str());
        }
    }
}


Bitvector GraphRepresentation::calculateFID(string &source, string &destination) {
    int vertex_id;
    Bitvector result(dm->fid_len * 8);
    igraph_vs_t vs;
    igraph_vector_ptr_t res;
    igraph_vector_t to_vector;
    igraph_vector_t *temp_v;
    igraph_integer_t eid;
    const char *Ntype;
    /*find the vertex id in the reverse index*/
    map<string,int>::iterator index_it = reverse_node_index.find(source);
    if (index_it == reverse_node_index.end()) {
        cerr << "Warning: " << source << " not found in reverse node index map (source)" << endl;
        return result; /* XXX */
    }
    int from = index_it->second;
    if (dm->overlay_mode.compare("mac_ml") != 0) {
//      cout << "Creating RVFIDs and TMFIDs for single layer topology..." << endl;
    } else {
        // compute RVFID and TMFID only to PN nodes not ON nodes
        Ntype = igraph_cattribute_VAS(&igraph, "NTYPE", from);
        //cout << "NTYPE IS: " << Ntype[0] << endl << endl;
        if (Ntype[0] != 'P') {
            for (int k = 0; k < dm->fid_len * 8; k++) {
                (result)[dm->fid_len * 8 - k - 1].operator |= (false);
            }
            return result;
        }
        cout << "Creating RVFIDs and TMFIDs for multilayer topology..." << endl;
    }
    index_it = reverse_node_index.find(destination);
    if (index_it == reverse_node_index.end()) {
        cerr << "Warning: " << destination << " not found in reverse node index map (destination)" << endl;
        return result; /* XXX */
    }
    igraph_vector_init(&to_vector, 1);
    VECTOR(to_vector)[0] = index_it->second;  // find vertex id of the destination node label
    /*initialize the sequence*/
    igraph_vs_vector(&vs, &to_vector);
    /*initialize the vector that contains pointers*/
    igraph_vector_ptr_init(&res, 1);
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    temp_v = (igraph_vector_t *) malloc(sizeof (igraph_vector_t));
    VECTOR(res)[0] = temp_v;
    igraph_vector_init(temp_v, 1);
    /*run the shortest path algorithm from "from"*/
#if IGRAPH_V >= IGRAPH_V_0_6
    igraph_get_shortest_paths(&igraph, &res, NULL, from, vs, IGRAPH_OUT);
#else
    igraph_get_shortest_paths(&igraph, &res, from, vs, IGRAPH_OUT);
#endif
    /*check the shortest path to each destination*/
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    //click_chatter("Shortest path from %s to %s", igraph_cattribute_VAS(&graph, "NODEID", from), igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[igraph_vector_size(temp_v) - 1]));

    /*now let's "or" the FIDs for each link in the shortest path*/
    for (int j = 0; j < igraph_vector_size(temp_v) - 1; j++) {
#if IGRAPH_V >= IGRAPH_V_0_6
        igraph_get_eid(&igraph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true, true);
#else
        igraph_get_eid(&igraph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true);
#endif
        //click_chatter("node %s -> node %s", igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[j]), igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[j + 1]));
        //click_chatter("link: %s", igraph_cattribute_EAS(&graph, "LID", eid));
        string LID(igraph_cattribute_EAS(&igraph, "LID", eid), dm->fid_len * 8);
        for (int k = 0; k < dm->fid_len * 8; k++) {
            if (LID[k] == '1') {
                (result)[ dm->fid_len * 8 - k - 1].operator |=(true);
            }
        }
        //click_chatter("FID of the shortest path: %s", result.to_string().c_str());
    }
    /*now for all destinations "or" the internal linkID*/
    vertex_id = (*reverse_node_index.find(destination)).second;
    string iLID(igraph_cattribute_VAS(&igraph, "iLID", vertex_id));
    //click_chatter("internal link for node %s: %s", igraph_cattribute_VAS(&graph, "NODEID", vertex_id), iLID.c_str());
    for (int k = 0; k < dm->fid_len * 8; k++) {
        if (iLID[k] == '1') {
            (result)[ dm->fid_len * 8 - k - 1].operator |=(true);
        }
    }
    igraph_vector_destroy((igraph_vector_t *) VECTOR(res)[0]);
    igraph_vector_destroy(&to_vector);
    igraph_vector_ptr_destroy_all(&res);
    igraph_vs_destroy(&vs);
    return result;
}

void GraphRepresentation::calculateRVFIDs() {
    string RVLabel = dm->RV_node->label;
    for (size_t i = 0; i < dm->network_nodes.size(); i++) {
        NetworkNode *nn = dm->network_nodes[i];
        nn->FID_to_RV = calculateFID(nn->label, RVLabel);
    }
}

void GraphRepresentation::calculateTMFIDs() {
    string TMLabel = dm->TM_node->label;
    for (size_t i = 0; i < dm->network_nodes.size(); i++) {
        NetworkNode *nn = dm->network_nodes[i];
        nn->FID_to_TM = calculateFID(nn->label, TMLabel);
    }
}

//AutoGenerated Graph support starts here

string GraphRepresentation::ProduceStringLabelsFromIds(int id) {

    string reply;
    stringstream ss;

    ss << (id + 1);

    if (ss.str().size() <= 8) {
        string label(8 - ss.str().size(), '0');
        reply = label + ss.str();
    } else {
        std::cout << "Cannot build 8 character label";
    }
    return reply;

}

void GraphRepresentation::addGeneratedMappedConnection(NetworkNode *nn1, NetworkNode *nn2) {

    NetworkConnection *nc = new NetworkConnection();

    nc->src_label = nn1->label;
    nc->dst_label = nn2->label;

    nc->src_ip = nn1->testbed_ip;
    nc->dst_ip = nn2->testbed_ip;

    nn1->connections.push_back(nc);
    dm->number_of_connections++;

    return;

}

void GraphRepresentation::ChooseTheBestTMRVNode(void) {

    int numofvertices, i, j;
    igraph_matrix_t res;
    unsigned int cur_total = 99999999;
    unsigned int cur_vid = 0;
    unsigned int tmp;

    numofvertices = igraph_vcount(&igraph);
    igraph_matrix_init(&res, numofvertices, numofvertices);
#if IGRAPH_V >= IGRAPH_V_0_6
    igraph_shortest_paths(&igraph, &res, igraph_vss_all(), igraph_vss_all(), IGRAPH_OUT);
#else
    igraph_shortest_paths(&igraph, &res, igraph_vss_all(), IGRAPH_OUT);
#endif

    for (i = 0; i < numofvertices; i++) {
        tmp = 0;
        for (j = 0; j < numofvertices; j++) {
            tmp += MATRIX(res, i, j);
        }
        if (tmp < cur_total) {
            cur_total = tmp;
            cur_vid = i;
        }
    }
    NetworkNode * nn = dm->network_nodes[cur_vid];
    nn->isRV = true;
    nn->isTM = true;
    dm->RV_node = nn;
    dm->TM_node = nn;
    cout << "Autogenerated Info: chose as the RV and TM: " << cur_vid + 1 << " with total hops " << cur_total << endl;

}

void GraphRepresentation::OutputLeafVertices(string filename) {
    int degree;
    ofstream degreeonefile;
    NetworkNode *nn = NULL;

    igraph_vector_t vid_res;
    igraph_vector_init(&vid_res, 1);
    igraph_degree(&igraph, &vid_res, igraph_vss_all(), IGRAPH_OUT, false);

    degreeonefile.open((dm->write_conf + filename).c_str());
    degreeonefile << "network = { \n\n nodes = (";
    for (size_t i = 0; i < dm->network_nodes.size(); i++) {
        nn = dm->network_nodes[i];
        degree = VECTOR(vid_res)[(*reverse_node_index.find(nn->label)).second];
        if (degree == 1) {
            degreeonefile << "\n{\n  testbed_ip=\"" << nn->testbed_ip << "\";\n  }," << endl;
        }
    }
    degreeonefile << " \n  );\n};\n";
    degreeonefile.close();
    igraph_vector_destroy(&vid_res);
}

void GraphRepresentation::BuildInputMap() {

    int i, numofvertices;
    string labelid;
    igraph_vector_t edge_vector;
    NetworkNode * nn1 = NULL;
    NetworkNode * nn2 = NULL;

    /**STEP 1: Assign Names to Vertices and put them in the lookup hash table
     */
    numofvertices = igraph_vcount(&igraph);
    for (i = 0; i < numofvertices; i++) {
        labelid = ProduceStringLabelsFromIds(i);
        /**Create a straight map
         */
        node_index.insert(pair<int, string > (i, labelid));
        /**Create the inverse map
         */
        reverse_node_index.insert(pair<string, int>(labelid, i));
        /**update the network nodes
         */
        NetworkNode *nn = dm->network_nodes[i];
        nn->label = labelid;
        nn->running_mode = "user";
        /**update graph info
         */
        igraph_cattribute_VAS_set(&igraph, "NODEID", i, labelid.c_str());
    }

    /**STEP 2: Get the vertex pair for each edge in the array and update the domain map
     */
    igraph_vector_init(&edge_vector, 1);
    igraph_get_edgelist(&igraph, &edge_vector, true);
    int z = igraph_ecount(&igraph);
    for (unsigned int i = 0; i < dm->number_of_nodes; i++) {
        cout << "Node " << dm->network_nodes[i]->label << " -- IP :" << dm->network_nodes[i]->testbed_ip << endl;
        ;
    }
    for (int i = 0; i < z; i++) {
        string dst_label;
        nn1 = dm->network_nodes[(int) VECTOR(edge_vector)[i]];
        nn2 = dm->network_nodes[(int) VECTOR(edge_vector)[z + i]];
        cout << "Connection " << (int) VECTOR(edge_vector)[i] << " " << (int) VECTOR(edge_vector)[z + i] << endl;
        addGeneratedMappedConnection(nn1, nn2);
        //Add the reverse edge on the graph in case you load it again later
        igraph_add_edge(&igraph, (int) VECTOR(edge_vector)[z + i], (int) VECTOR(edge_vector)[i]);
        addGeneratedMappedConnection(nn2, nn1);
    }
    igraph_vector_destroy(&edge_vector);

}

