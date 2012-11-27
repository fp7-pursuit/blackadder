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

#include "tm_igraph.hpp"

TMIgraph::TMIgraph() {
    igraph_i_set_attribute_table(&igraph_cattribute_table);
    //igraph_empty(&graph, 0, IGRAPH_DIRECTED); // No, read from file instead
}

TMIgraph::~TMIgraph() {
    map<string, Bitvector *>::iterator nodeID_iLID_iter;
    map<int, Bitvector *>::iterator edge_LID_iter;
    for (nodeID_iLID_iter = nodeID_iLID.begin(); nodeID_iLID_iter != nodeID_iLID.end(); nodeID_iLID_iter++) {
        delete (*nodeID_iLID_iter).second;
    }
    for (edge_LID_iter = edge_LID.begin(); edge_LID_iter != edge_LID.end(); edge_LID_iter++) {
        delete (*edge_LID_iter).second;
    }
    igraph_i_attribute_destroy(&graph);
    igraph_destroy(&graph);
}

int TMIgraph::readTopology(char *file_name) {
    int ret;
    Bitvector *lid;
    Bitvector *ilid;
    ifstream infile;
    string str;
    size_t found_5, found_6, first, second;
    FILE *instream;
    infile.open(file_name, ifstream::in);
    if (infile.fail()) {
        return -1;
    }
    /*first the Global graph attributes - c igraph does not do it!!*/
    while (infile.good()) {
        getline(infile, str);
        found_5 = str.find("<data key=\"FID_LEN\">");
        found_6 = str.find("<data key=\"g_FID_LEN\">");
        if ((found_5 != string::npos) || ((found_6 != string::npos))) {
            first = str.find(">");
            second = str.find("<", first);
            sscanf(str.substr(first + 1, second - first - 1).c_str(), "%d", &fid_len);
        }
        found_5 = str.find("<data key=\"TM\">");
        found_6 = str.find("<data key=\"g_TM\">");
        if ((found_5 != string::npos) || ((found_6 != string::npos))) {
            first = str.find(">");
            second = str.find("<", first);
            nodeID = str.substr(first + 1, second - first - 1);
        }

        found_5 = str.find("<data key=\"TM_MODE\">");
        found_6 = str.find("<data key=\"g_TM_MODE\">");
        if ((found_5 != string::npos) || ((found_6 != string::npos))) {
            first = str.find(">");
            second = str.find("<", first);
            mode = str.substr(first + 1, second - first - 1);
        }
    }
    infile.close();
    instream = fopen(file_name, "r");
    ret = igraph_read_graph_graphml(&graph, instream, 0);
    fclose(instream);
    if (ret < 0) {
        return ret;
    }
    cout << "TM: " << igraph_vcount(&graph) << " nodes" << endl;
    cout << "TM: " << igraph_ecount(&graph) << " edges" << endl;
    for (int i = 0; i < igraph_vcount(&graph); i++) {
        string nID = string(igraph_cattribute_VAS(&graph, "NODEID", i));
        string iLID = string(igraph_cattribute_VAS(&graph, "iLID", i));
        reverse_node_index.insert(pair<string, int>(nID, i));
        ilid = new Bitvector(iLID);
        nodeID_iLID.insert(pair<string, Bitvector *>(nID, ilid));
        vertex_iLID.insert(pair<int, Bitvector *>(i, ilid));
        //cout << "node " << i << " has NODEID " << nID << endl;
        //cout << "node " << i << " has ILID " << ilid->to_string() << endl;
    }
    for (int i = 0; i < igraph_ecount(&graph); i++) {
        string LID = string(igraph_cattribute_EAS(&graph, "LID", i));
        reverse_edge_index.insert(pair<string, int>(LID, i));
        lid = new Bitvector(LID);
        edge_LID.insert(pair<int, Bitvector *>(i, lid));
        //cout << "edge " << i << " has LID  " << lid->to_string() << endl;
    }
    return ret;
}

Bitvector *TMIgraph::calculateFID(string &source, string &destination) {
    int vertex_id;
    Bitvector *result = new Bitvector(FID_LEN * 8);
    igraph_vs_t vs;
    igraph_vector_ptr_t res;
    igraph_vector_t to_vector;
    igraph_vector_t *temp_v;
    igraph_integer_t eid;

    /*find the vertex id in the reverse index*/
    int from = (*reverse_node_index.find(source)).second;
    igraph_vector_init(&to_vector, 1);
    VECTOR(to_vector)[0] = (*reverse_node_index.find(destination)).second;
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
    igraph_get_shortest_paths(&graph, &res, NULL, from, vs, IGRAPH_OUT);
#else
    igraph_get_shortest_paths(&graph, &res, from, vs, IGRAPH_OUT);
#endif
    /*check the shortest path to each destination*/
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    //click_chatter("Shortest path from %s to %s", igraph_cattribute_VAS(&graph, "NODEID", from), igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[igraph_vector_size(temp_v) - 1]));
    /*now let's "or" the FIDs for each link in the shortest path*/
    for (int j = 0; j < igraph_vector_size(temp_v) - 1; j++) {
#if IGRAPH_V >= IGRAPH_V_0_6
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true, true);
#else
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true);
#endif
        //click_chatter("node %s -> node %s", igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[j]), igraph_cattribute_VAS(&graph, "NODEID", VECTOR(*temp_v)[j + 1]));
        //click_chatter("link: %s", igraph_cattribute_EAS(&graph, "LID", eid));
        string LID(igraph_cattribute_EAS(&graph, "LID", eid), FID_LEN * 8);
        for (int k = 0; k < FID_LEN * 8; k++) {
            if (LID[k] == '1') {
                (*result)[ FID_LEN * 8 - k - 1].operator |=(true);
            }
        }
        //click_chatter("FID of the shortest path: %s", result.to_string().c_str());
    }
    /*now for all destinations "or" the internal linkID*/
    vertex_id = (*reverse_node_index.find(destination)).second;
    string iLID(igraph_cattribute_VAS(&graph, "iLID", vertex_id));
    //click_chatter("internal link for node %s: %s", igraph_cattribute_VAS(&graph, "NODEID", vertex_id), iLID.c_str());
    for (int k = 0; k < FID_LEN * 8; k++) {
        if (iLID[k] == '1') {
            (*result)[ FID_LEN * 8 - k - 1].operator |=(true);
        }
    }
    igraph_vector_destroy((igraph_vector_t *) VECTOR(res)[0]);
    igraph_vector_destroy(&to_vector);
    igraph_vector_ptr_destroy_all(&res);
    igraph_vs_destroy(&vs);
    return result;
}

/*main function for rendezvous*/
void TMIgraph::calculateFID(set<string> &publishers, set<string> &subscribers, map<string, Bitvector *> &result) {
    set<string>::iterator subscribers_it;
    set<string>::iterator publishers_it;
    string bestPublisher;
    Bitvector resultFID(FID_LEN * 8);
    Bitvector bestFID(FID_LEN * 8);
    unsigned int numberOfHops = 0;
    /*first add all publishers to the hashtable with NULL FID*/
    for (publishers_it = publishers.begin(); publishers_it != publishers.end(); publishers_it++) {
        string publ = *publishers_it;
        result.insert(pair<string, Bitvector *>(publ, NULL));
    }
    for (subscribers_it = subscribers.begin(); subscribers_it != subscribers.end(); subscribers_it++) {
        /*for all subscribers calculate the number of hops from all publishers (not very optimized...don't you think?)*/
        unsigned int minimumNumberOfHops = UINT_MAX;
        for (publishers_it = publishers.begin(); publishers_it != publishers.end(); publishers_it++) {
            resultFID.clear();
            string str1 = (*publishers_it);
            string str2 = (*subscribers_it);
            calculateFID(str1, str2, resultFID, numberOfHops);
            if (minimumNumberOfHops > numberOfHops) {
                minimumNumberOfHops = numberOfHops;
                bestPublisher = *publishers_it;
                bestFID = resultFID;
            }
        }
        //cout << "best publisher " << bestPublisher << " for subscriber " << (*subscribers_it) << " -- number of hops " << minimumNumberOfHops - 1 << endl;
        if ((*result.find(bestPublisher)).second == NULL) {
            /*add the publisher to the result*/
            //cout << "FID1: " << bestFID.to_string() << endl;
            result[bestPublisher] = new Bitvector(bestFID);
        } else {
            //cout << "/*update the FID for the publisher*/" << endl;
            Bitvector *existingFID = (*result.find(bestPublisher)).second;
            /*or the result FID*/
            *existingFID = *existingFID | bestFID;
        }
    }
}

void TMIgraph::calculateFID(string &source, string &destination, Bitvector &resultFID, unsigned int &numberOfHops) {
    igraph_vs_t vs;
    igraph_vector_ptr_t res;
    igraph_vector_t to_vector;
    igraph_vector_t *temp_v;
    igraph_integer_t eid;

    /*find the vertex id in the reverse index*/
    int from = (*reverse_node_index.find(source)).second;
    igraph_vector_init(&to_vector, 1);
    VECTOR(to_vector)[0] = (*reverse_node_index.find(destination)).second;
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
    igraph_get_shortest_paths(&graph, &res, NULL, from, vs, IGRAPH_OUT);
#else
    igraph_get_shortest_paths(&graph, &res, from, vs, IGRAPH_OUT);
#endif
    /*check the shortest path to each destination*/
    temp_v = (igraph_vector_t *) VECTOR(res)[0];

    /*now let's "or" the FIDs for each link in the shortest path*/
    for (int j = 0; j < igraph_vector_size(temp_v) - 1; j++) {
#if IGRAPH_V >= IGRAPH_V_0_6
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true, true);
#else
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true);
#endif
        Bitvector *lid = (*edge_LID.find(eid)).second;
        (resultFID) = (resultFID) | (*lid);
    }
    numberOfHops = igraph_vector_size(temp_v);

    /*now for the destination "or" the internal linkID*/
    Bitvector *ilid = (*nodeID_iLID.find(destination)).second;
    (resultFID) = (resultFID) | (*ilid);
    //cout << "FID of the shortest path: " << resultFID.to_string() << endl;
    igraph_vector_destroy((igraph_vector_t *) VECTOR(res)[0]);
    igraph_vector_destroy(&to_vector);
    igraph_vector_ptr_destroy_all(&res);
    igraph_vs_destroy(&vs);
}
