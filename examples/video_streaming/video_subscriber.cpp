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
#include <openssl/sha.h>
#include <spawn.h>
#include <arpa/inet.h>
#include <blackadder.hpp>

Blackadder *ba;

string video_stream;

int port_sequence_number = 1500;

int getPort() {
    if (port_sequence_number < 50000) {
        return port_sequence_number++;
    } else {
        port_sequence_number = 1500;
        return port_sequence_number;
    }
}

void *vlc_thread_loop(void *arg) {
    string str_random_port = string((char *) arg);
    cout << "random_port " << str_random_port << endl;
    string command = string("/usr/bin/vlc rtp://@:") + string(str_random_port);
    system(command.c_str());
    ba->disconnect();
    delete ba;
    exit(0);
}

void *event_listener_loop(void *arg) {
    int sock;
    struct sockaddr_in server;
    char str_random_port[30];
    pthread_t vlc_thread;
    Blackadder *ba = (Blackadder *) arg;
    /*create a random port*/
    int random_port = (rand() % 10000 + 3000);
    snprintf(str_random_port, sizeof (str_random_port), "%d", random_port);
    pthread_create(&vlc_thread, NULL, vlc_thread_loop, str_random_port);

    /*create the UDP loopback sender*/
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(random_port);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
    }
    while (true) {
        Event ev;
        ba->getEvent(ev);
        if (ev.type == PUBLISHED_DATA) {
            sendto(sock, ev.data, ev.data_len, 0, (struct sockaddr *) &server, sizeof (server));
        } else {
            cout << "weird" << endl;
        }
    }
}

void sigfun(int sig) {
    (void) signal(SIGINT, SIG_DFL);
    ba->disconnect();
    delete ba;
    exit(0);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    string id;
    string prefix_id;
    string bin_id;
    string bin_prefix_id;
    Event ev;
    pthread_t event_listener;
    (void) signal(SIGINT, sigfun);
    if (argc > 1) {
        int user_or_kernel = atoi(argv[1]);
        if (user_or_kernel == 0) {
            ba = Blackadder::Instance(true);
        } else {
            ba = Blackadder::Instance(false);
        }
    } else {
        /*By Default I assume blackadder is running in user space*/
        ba = Blackadder::Instance(true);
    }
    cout << "subscribing to the video catalogue information item" << endl;
    id = "0000000000000000";
    prefix_id = "0000000000000000";
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);
    ba->subscribe_info(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    ba->getEvent(ev);
    if (ev.type == PUBLISHED_DATA) {
        if (ev.id.compare(bin_prefix_id + bin_id) == 0) {
            cout << "received Video Catalogue" << endl;
            ba->unsubscribe_info(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
            string video_catalogue((char *) ev.data, ev.data_len);
            cout << video_catalogue << endl;
            cout << "Please select the video stream you want to join" << endl;
            cin >> video_stream;
            cout << "You selected to watch the stream with title " << video_stream << " ...soon it will be here :)" << endl;
            unsigned char *c_channel_id = (unsigned char *) malloc(256);
            /*publish the channel for this info*/
            SHA1((const unsigned char *) video_stream.c_str(), video_stream.length(), c_channel_id);
            string channel_id = string((const char *) c_channel_id).substr(0, PURSUIT_ID_LEN);
            cout << "Subscribing to video channel with ID " << id + chararray_to_hex(channel_id) << endl;
            prefix_id = "0000000000000000";
            bin_prefix_id = hex_to_chararray(prefix_id);
            ba->subscribe_info(channel_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
        } else {
            cout << "received data for ID " << chararray_to_hex(ev.id) << ". That's weird. aborting..." << endl;
            goto error;
        }
    } else {
        cout << "something wrong has happened. aborting...." << endl;
        goto error;
    }
    pthread_create(&event_listener, NULL, event_listener_loop, (void *) ba);
    pthread_join(event_listener, NULL);
    cout << "disconnecting" << endl;
    sleep(1);
error:
    ba->disconnect();
    delete ba;
    return 0;
}
