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
#include <signal.h>
#include <blackadder.hpp>
#include <nb_blackadder.hpp>

NB_Blackadder *ba;

int payload_size = 1400;


using namespace std;

void callback(Event *ev) {
    char *payload;
    char *end_payload;
    if (ev->type == START_PUBLISH) {
        cout << "start publishing " << endl;
        for (int i = 0; i < 1000000; i++) {
            //cout << "publishing data for ID " << chararray_to_hex(ev.id) << endl;
            payload = (char *) malloc(payload_size);
            memset(payload, 'A', payload_size);
            ba->publish_data(ev->id, DOMAIN_LOCAL, NULL, 0, payload, payload_size);
        }
        for (int i = 0; i < 100; i++) {
            //cout << "publishing end flag for ID " << chararray_to_hex(ev.id) << endl;
            end_payload = (char *) malloc(payload_size);
            memset(end_payload, 'B', payload_size);
            ba->publish_data(ev->id, DOMAIN_LOCAL, NULL, 0, end_payload, payload_size);
        }
        delete ev;
        delete ba;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int user_or_kernel = atoi(argv[1]);
        if (user_or_kernel == 0) {
            ba = NB_Blackadder::Instance(true);
        } else {
            ba = NB_Blackadder::Instance(false);
        }
    } else {
        /*By Default I assume blackadder is running in user space*/
        ba = NB_Blackadder::Instance(true);
    }
    ba->setCallback(callback);
    cout << "Process ID: " << getpid() << endl;
    string id = "1111111111111111";
    string prefix_id = string();
    string bin_id = hex_to_chararray(id);
    string bin_prefix_id = hex_to_chararray(prefix_id);
    ba->publish_scope(bin_id, prefix_id, DOMAIN_LOCAL, NULL, 0);

    id = "1111111111111111";
    prefix_id = "1111111111111111";
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);

    ba->publish_info(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);

    sleep(1);
    ba->join();
    ba->disconnect();
    return 0;
}
