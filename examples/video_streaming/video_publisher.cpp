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
#include <map>
#include <spawn.h>
#include <arpa/inet.h>
#include <blackadder.hpp>

Blackadder *ba;

string video_catalogue;
int port_sequence_number = 1500;

int getPort() {
    if (port_sequence_number < 50000) {
        return port_sequence_number++;
    } else {
        port_sequence_number = 1500;
        return port_sequence_number;
    }
}

class StreamingInfo {
public:

    StreamingInfo() {
    };
    /*members*/
    Blackadder *ba;
    int sock;
    struct sockaddr_in server;
    int port;
    /*the channel ID*/
    pid_t pid_vlc;
    string channelID;
    string video_name;
    /*the loopback socket receiver thread*/
    pthread_t socket_receiver;
};

map <string, StreamingInfo *> channelsMap;
map <string, string> channel_to_name;

void *udp_socket_listener(void *arg) {
    StreamingInfo *stream_info = (StreamingInfo *) arg;
    int bytes;
    struct sockaddr_in from;
    char buffer[1500];
    int length;
    stream_info->server.sin_family = AF_INET;
    stream_info->server.sin_addr.s_addr = inet_addr("127.0.0.1");
    stream_info->server.sin_port = htons(stream_info->port);
    stream_info->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (stream_info->sock < 0) {
        perror("socket");
    }
    if (bind(stream_info->sock, (struct sockaddr *) &stream_info->server, sizeof (struct sockaddr_in)) < 0) {
        perror("binding");
    }
    while (true) {
        bytes = recvfrom(stream_info->sock, buffer, 1500, 0, (struct sockaddr *) &from, (socklen_t*) & length);
        if (bytes < 0) {
            break;
        } else {
            ba->publish_data(stream_info->channelID, NODE_LOCAL, NULL, 0, buffer, bytes);
        }
    }
}

void *event_listener_loop(void *arg) {
    string catalogue_id = hex_to_chararray("00000000000000000000000000000000");
    Blackadder *ba = (Blackadder *) arg;
    while (true) {
        Event ev;
        ba->getEvent(ev);
        if (ev.type == START_PUBLISH) {
            if (ev.id.compare(catalogue_id) == 0) {
                /*publish once the catalogue data*/
                cout << "publishing the video catalogue using ID " << chararray_to_hex(ev.id) << endl;
                ba->publish_data(ev.id, (char) DOMAIN_LOCAL, NULL, 0, (void *) video_catalogue.c_str(), video_catalogue.length());
            } else {
                /*start publishing a video channel...use a separate thread*/
                StreamingInfo *stream_info = new StreamingInfo();
                stream_info->ba = ba;
                stream_info->channelID = ev.id;
                stream_info->video_name = channel_to_name[ev.id];
                stream_info->port = getPort();
                channelsMap.insert(pair <string, StreamingInfo *>(ev.id, stream_info));
                int pid_vlc;
                string full_video_path = string("/home/pursuit/") + stream_info->video_name;
                char str_int[30];
                snprintf(str_int, sizeof (str_int), "%d", stream_info->port);
                string long_argument = string(":sout=#rtp{dst=127.0.0.1,port=") + string(str_int) + string(",mux=ts}");
                char * _ExecutablePath[] = {(char *)"/usr/bin/cvlc", (char *) full_video_path.c_str(), (char *) long_argument.c_str(), (char *)":no-sout-rtp-sap", (char *)":no-sout-standard-sap", (char *)":sout-keep", (char *)"--loop", NULL};
                /*create thread that will receive all loopback rtp packets*/
                pthread_create(&stream_info->socket_receiver, NULL, udp_socket_listener, (void *) stream_info);
                /**********************************************************/
                posix_spawn(&pid_vlc, _ExecutablePath[0], NULL, NULL, _ExecutablePath, NULL);
                stream_info->pid_vlc = pid_vlc;
            }
        } else if (ev.type == STOP_PUBLISH) {
            if (ev.id.compare(catalogue_id) == 0) {
                cout << "No subscribers for the catalogue ID - Don't do anything" << endl;
            } else {
                /*stop publishing a video channel...find the thread and kill it*/
                cout << "No more subscribers for channel " << chararray_to_hex(ev.id) << "...stopping vlc " << endl;
                StreamingInfo *stream_info = channelsMap[ev.id];
                char str_pid[33];
                snprintf(str_pid, sizeof (str_pid), "%d", stream_info->pid_vlc);
                pthread_cancel(stream_info->socket_receiver);
                int forget_pid;
                char * _ExecutablePath[] = {(char *)"/bin/kill", str_pid, NULL};
                posix_spawn(&forget_pid, _ExecutablePath[0], NULL, NULL, _ExecutablePath, NULL);
                cout << "closing the respective loopback socket" << endl;
                close(stream_info->sock);
                delete stream_info;
                channelsMap.erase(ev.id);
            }
        } else {
            cout << "I am not expecting anything else than Start or Stop Publishing" << endl;
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
    /*this buffer contains the list of videos in ASCII.*/
    /* Each video title is written in a separate line*/
    string id;
    string prefix_id;
    string bin_id;
    string bin_prefix_id;
    string input;
    pthread_t event_listener;
    /*override the signal handler*/
    (void) signal(SIGINT, sigfun);
    /*check the arguments and initialize blackadder client accordingly*/
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

    id = "0000000000000000";
    prefix_id = string();
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);
    cout << "Publishing the root scope where all videos are going to be advertised" << endl;
    ba->publish_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);

    cout << "Please type the video titles available in /videos folder...A GUI would be better of course" << endl;
    while (true) {
        cin >> input;
        if (input.compare(".") != 0) {
            video_catalogue += (input + "\n");
            unsigned char *c_channel_id = (unsigned char *) malloc(256);
            /*publish the channel for this info*/
            SHA1((const unsigned char *) input.c_str(), input.length(), c_channel_id);
            string channel_id = string((const char *) c_channel_id).substr(0, PURSUIT_ID_LEN);
            cout << "advertising video channel using the ID: " << id + chararray_to_hex(channel_id) << endl;
            prefix_id = "0000000000000000";
            bin_prefix_id = hex_to_chararray(prefix_id);
            ba->publish_info(channel_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
            channel_to_name.insert(pair<string, string > (bin_prefix_id + channel_id, input));
            /***********************************/
            cout << "type another title" << endl;
            free(c_channel_id);
        } else {
            break;
        }
    }
    cout << "Catalogue length: " << video_catalogue.length() << endl;
    cout << video_catalogue << endl;
    cout << "advertising the video catalogue under the root scope" << endl;
    prefix_id = "0000000000000000";
    id = "0000000000000000";
    bin_prefix_id = hex_to_chararray(prefix_id);
    bin_id = hex_to_chararray(id);
    /*This information item is the catalogue of available video channels*/
    ba->publish_info(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    /*start the event listener*/
    pthread_create(&event_listener, NULL, event_listener_loop, (void *) ba);
    pthread_join(event_listener, NULL);
    cout << "disconnecting" << endl;
    sleep(1);
    ba->disconnect();
    delete ba;
    return 0;
}
