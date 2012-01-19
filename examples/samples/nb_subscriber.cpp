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
#include <sys/time.h>

NB_Blackadder *nb_ba;

int counter = 0;
struct timezone tz;

struct timeval start_tv;
struct timeval end_tv;
struct timeval duration;

int payload_size = 1450;
bool experiment_started = false;

void eventHandler(Event *ev) {
    char *data = (char *) ev->data;
    if (ev->type == PUBLISHED_DATA) {
        if (data[0] == 'A') {
            if (experiment_started == false) {
                experiment_started = true;
                gettimeofday(&start_tv, &tz);
                printf("START TIME: %ld,%ld \n", start_tv.tv_sec, start_tv.tv_usec);
            }
            //cout << "received published data for ID " << chararray_to_hex(str_id) << "!! size: " << (total_buf_size - (sizeof (struct nlmsghdr) + sizeof (unsigned char) + sizeof (unsigned char) +str_id.size())) << endl;
            counter++;
        }
        if (data[0] == 'B') {
            gettimeofday(&end_tv, &tz);
            duration.tv_sec = end_tv.tv_sec - start_tv.tv_sec;
            if (end_tv.tv_usec - start_tv.tv_usec > 0) {
                duration.tv_usec = end_tv.tv_usec - start_tv.tv_usec;
            } else {
                duration.tv_usec = end_tv.tv_usec + 1000000 - start_tv.tv_usec;
                duration.tv_sec--;
            }
            printf("duration: %ld seconds and %d microseconds\n\n", duration.tv_sec, duration.tv_usec);
            float left = counter * ((float) payload_size / (float) (1024 * 1024));
            float right = ((float) ((duration.tv_sec * 1000000) + duration.tv_usec)) / 1000000;
            cout << "counter: " << counter << endl;
            cout << "payload_size: " << payload_size << endl;
            float throughput = (left / right);
            printf("Throughput: %f MB/sec \n\n", throughput);
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
    id = "1111111111111111";
    prefix_id = string();
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);
    nb_ba->subscribe_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    nb_ba->join();
    nb_ba->disconnect();
    delete nb_ba;
    cout << "exiting...." << endl;
    return 0;
}
