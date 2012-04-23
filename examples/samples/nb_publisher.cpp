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

#include <nb_blackadder.hpp>

#include <sstream> 
#include <iostream>

NB_Blackadder *nb_ba;

void eventHandler(Event *ev) {
    cout << "Received Event" << endl;
    cout << "Type: " << (int) ev->type << endl;
    cout << "Information Identifier: " << chararray_to_hex(ev->id) << endl;
    char *payload;
    char * end_payload;
    int payload_size = 1450;
    if (ev->type == START_PUBLISH) {
        cout << "start publishing " << endl;
        for (int i = 0; i < 50000; i++) {
            payload = (char *) malloc(payload_size);
            memset(payload, 'A', payload_size);
            nb_ba->publish_data(ev->id, DOMAIN_LOCAL, NULL, 0, payload, payload_size);
        }
        for (int i = 0; i < 100; i++) {
            end_payload = (char *) malloc(payload_size);
            memset(end_payload, 'B', payload_size);
            nb_ba->publish_data(ev->id, DOMAIN_LOCAL, NULL, 0, end_payload, payload_size);
        }
    }
    delete ev;
}

int main(int argc, char* argv[]) {
    string id;
    string prefix_id;
    string bin_id;
    string bin_prefix_id;
    if (argc > 1) {
        int user_or_kernel = atoi(argv[1]);
        if (user_or_kernel == 0) {
            nb_ba = NB_Blackadder::Instance(true);
        } else {
            nb_ba = NB_Blackadder::Instance(false);
        }
    } else {
        /*By Default I assume blackadder is running in user space*/
        nb_ba = NB_Blackadder::Instance(true);
    }
    /*Set the callback function*/
    nb_ba->setCallback(eventHandler);
    /***************************/
    cout << "Process ID: " << getpid() << endl;
    /*****************************publish root scope /0000000000000000 ************************************/
    id = "1111111111111111";
    prefix_id = string();
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);
    nb_ba->publish_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    id = "1111111111111111";
    prefix_id = "1111111111111111";
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);
    nb_ba->publish_info(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    nb_ba->join();
    nb_ba->disconnect();
    delete nb_ba;
    cout << "exiting...." << endl;
    return 0;
}
