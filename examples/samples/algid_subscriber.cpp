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

#include <blackadder.hpp>
#include <nb_blackadder.hpp>
#include <bitvector.hpp>
#include <signal.h>
#include <pthread.h>
#include <map>
#include <vector>
#include <math.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <map>
#include <openssl/sha.h>

using namespace std;

class ExpectedFragmentSequence;

NB_Blackadder *ba;
string scope_identifier = "0000000000000000";
string item_identifier = "00000000000000001111111111111111";
string bin_scope_identifier = hex_to_chararray(scope_identifier);
string bin_item_identifier = hex_to_chararray(item_identifier);
map<string, bool> expectedInfo;
string algid1, algid2;
string backchannel_id;
//pthread_t event_listener;
pthread_t timeout_thread;
map<string, string> retransmission_channel_map;

pthread_mutex_t global_mutex;
pthread_cond_t global_cond;
ExpectedFragmentSequence *efs;

int sequence_number = 0;
int counter = 0;
struct timezone tz;
struct timeval start_tv;
struct timeval end_tv;
struct timeval duration;
int payload_size = 1420;
char *payload = (char *) malloc(payload_size);
char *end_payload = (char *) malloc(payload_size);
bool experiment_started = false;

class ExpectedFragmentSequence {
public:
    string firstID;
    Bitvector fragments_map;
    int number_of_fragments;
    int fragments_so_far;
    string s_to_p_channel;
    string p_to_s_channel;
    int time_beat;

    void printEFS() {
        cout << "****** expected fragment sequence********" << endl;
        cout << "efs->firstID  " << chararray_to_hex(firstID) << endl;
        cout << "efs->number_of_fragments  " << number_of_fragments << endl;
        cout << "efs->fragments_so_far  " << fragments_so_far << endl;
        cout << "efs->s_to_p_channel  " << chararray_to_hex(s_to_p_channel) << endl;
        cout << "efs->p_to_s_channel  " << chararray_to_hex(p_to_s_channel) << endl;
    }

    void printResult() {
        cout << "received ALL fragments " << endl;
        gettimeofday(&end_tv, &tz);
        printf("END TIME: %ld,%ld \n", end_tv.tv_sec, end_tv.tv_usec);
        duration.tv_sec = end_tv.tv_sec - start_tv.tv_sec;
        if (end_tv.tv_usec - start_tv.tv_usec > 0) {
            duration.tv_usec = end_tv.tv_usec - start_tv.tv_usec;
        } else {
            duration.tv_usec = end_tv.tv_usec + 1000000 - start_tv.tv_usec;
            duration.tv_sec--;
        }
        printf("duration: %ld seconds and %d microseconds\n\n", duration.tv_sec, duration.tv_usec);
        float left = number_of_fragments * ((float) payload_size / (float) (1024 * 1024));
        float right = ((float) ((duration.tv_sec * 1000000) + duration.tv_usec)) / 1000000;
        cout << "counter: " << number_of_fragments << endl;
        cout << "payload_size: " << payload_size << endl;
        float throughput = (left / right);
        printf("Throughput: %f MB/sec \n\n", throughput);
        cout << "THAT'S ALL FOLKS" << endl;
    }
};

void find_next(string &fragment_id) {
    for (int i = fragment_id.length() - 1; i >= 0; i--) {
        if (fragment_id.at(i) != -1) {
            if (fragment_id.at(i) != 127) {
                fragment_id.at(i)++;
                //cout << "i: " << i << "  fragment.at  " << (int) fragment_id.at(i) << endl;
                break;
            } else {
                fragment_id.at(i) = -128;
                //cout << "i: " << i << "  fragment.at  " << (int) fragment_id.at(i) << endl;
                break;
            }
        } else {
            fragment_id.at(i) = 0;
        }
    }
}

void find_previous(string &fragment_id) {
    for (int i = fragment_id.length() - 1; i >= 0; i--) {
        if (fragment_id.at(i) != 0) {
            if (fragment_id.at(i) != -128) {
                fragment_id.at(i)--;
                //cout << "i: " << i << "  fragment.at  " << (int) fragment_id.at(i) << endl;
                break;
            } else {
                fragment_id.at(i) = 127;
                //cout << "i: " << i << "  fragment.at  " << (int) fragment_id.at(i) << endl;
                break;
            }
        } else {
            fragment_id.at(i) = -1;
        }
    }
}

/*very very bad implementation*/
void calculate_fragment_from_int(string &fragment_id, int offset) {
    for (int i = 0; i < offset - 1; i++) {
        find_next(fragment_id);
    }
}

int calculate_number_of_fragments(string &start_id, string &end_id) {
    int result = 1;
    if (start_id.compare(end_id) > 0) {
        return -1;
    }
    for (string::size_type i = 0; i < start_id.size(); i++) {
        unsigned short first = (unsigned short) (unsigned char) start_id[i];
        unsigned short last = (unsigned short) (unsigned char) end_id[i];
        result = result + pow(256, start_id.size() - i - 1)*(last - first);
    }
    return result;
}

void create_random_ID(string &id) {
    for (int i = 0; i < PURSUIT_ID_LEN; i++) {
        id = id + ' ';
        id[i] = rand() % 255;
    }
}

void *timeout_handler(void *arg) {
    int i, htonl_i;
    char *retransmission_payload;
    int required_fragments;
    unsigned char IDlen;
    string id = efs->firstID;
    while (true) {
        //cout << "timeout" << endl;
        usleep(90000);
        pthread_mutex_lock(&global_mutex);
        if (efs == NULL) {
            cout << "NO EFS - NULL - NADA" << endl;
            pthread_mutex_unlock(&global_mutex);
            break;
        }
        efs->time_beat--;
        if (efs->time_beat == 0) {
            /*find all non received fragments and send multiple requests for sequences of fragments*/
            i = 0;
            while (i < efs->fragments_map.size()) {
                required_fragments = 0;
                while ((Bitvector::Bit::unspecified_bool_type(efs->fragments_map[i]) != false) && (i < efs->fragments_map.size())) {
                    i++;
                }
                if (i == efs->fragments_map.size()) {
                    /*I reached the end of the publication*/
                    break;
                }
                htonl_i = htonl(i);
                id.replace(id.length() - sizeof (int), sizeof (int), (const char *) &htonl_i, sizeof (int));
                retransmission_payload = (char *) malloc(sizeof (IDlen) + id.length() + sizeof (required_fragments) + sizeof (sequence_number));
                IDlen = (unsigned char) id.length() / PURSUIT_ID_LEN;
                memcpy(retransmission_payload, &IDlen, sizeof (IDlen));
                memcpy(retransmission_payload + sizeof (IDlen), id.c_str(), id.length());
                while ((Bitvector::Bit::unspecified_bool_type(efs->fragments_map[i]) == false) && (i < efs->fragments_map.size())) {
                    required_fragments++;
                    i++;
                }
                memcpy(retransmission_payload + sizeof (IDlen) + id.length(), &required_fragments, sizeof (required_fragments));
                memcpy(retransmission_payload + sizeof (IDlen) + id.length() + sizeof (required_fragments), &sequence_number, sizeof (sequence_number));
                //cout << "will publish a retransmission request" << endl;
                ba->publish_data(efs->s_to_p_channel, DOMAIN_LOCAL, NULL, 0, retransmission_payload, sizeof (IDlen) + id.length() + sizeof (required_fragments) + sizeof (sequence_number));
                sequence_number++;
                //free(retransmission_payload);
            }
            efs->time_beat = 3;
        } else if (efs->time_beat < 0) {
            cout << "that should not happen" << endl;
        }
        pthread_mutex_unlock(&global_mutex);
    }
}

void callback(Event *ev) {
    map<string, bool>::iterator iter;
    string hex_id;
    string actual_identifier;
    int ntohl_number_of_fragments;
    int distance;
    hex_id = chararray_to_hex(ev->id);
    switch (ev->type) {
        case SCOPE_PUBLISHED:
            cout << "SCOPE_PUBLISHED: " << hex_id << endl;
            delete ev;
            break;
        case SCOPE_UNPUBLISHED:
            cout << "SCOPE_UNPUBLISHED: " << hex_id << endl;
            delete ev;
            break;
        case START_PUBLISH:
            cout << "START_PUBLISH: " << hex_id << endl;
            /*I don't need to do anything here..The publisher will receive the backpath "hello" message implicitly when the first request for retransmission will be sent*/
            delete ev;
            break;
        case STOP_PUBLISH:
            cout << "STOP_PUBLISH: " << hex_id << endl;
            delete ev;
            break;
        case PUBLISHED_DATA:
            //cout << "PUBLISHED_DATA: " << hex_id << endl;
            pthread_mutex_lock(&global_mutex);
            iter = expectedInfo.find(ev->id.substr(0, ev->id.length() - PURSUIT_ID_LEN));
            if (iter != expectedInfo.end()) {
                /*this is not a retransmitted fragment*/
                if ((*iter).second == false) {
                    (*iter).second = true;
                    /*this is the the first fragment I receive*/
                    /*start measuring*/
                    gettimeofday(&start_tv, &tz);
                    printf("START TIME: %ld,%ld \n", start_tv.tv_sec, start_tv.tv_usec);
                    create_random_ID(backchannel_id);
                    ba->publish_info(backchannel_id, ev->id.substr(0, ev->id.length() - 2 * PURSUIT_ID_LEN) + algid1, DOMAIN_LOCAL, NULL, 0);
                    ba->subscribe_info(backchannel_id, ev->id.substr(0, ev->id.length() - 2 * PURSUIT_ID_LEN) + algid2, DOMAIN_LOCAL, NULL, 0);
                    efs = new ExpectedFragmentSequence();
                    efs->firstID = ev->id.substr(0, ev->id.length() - PURSUIT_ID_LEN) + string((const char *) ev->id.substr(ev->id.length() - PURSUIT_ID_LEN, sizeof (int)).c_str(), sizeof (int)) + hex_to_chararray("00000000");
                    memcpy(&ntohl_number_of_fragments, ev->id.substr(ev->id.length() - PURSUIT_ID_LEN, sizeof (int)).c_str(), sizeof (int));
                    efs->number_of_fragments = ntohl(ntohl_number_of_fragments);
                    efs->fragments_so_far = 1;
                    efs->fragments_map = Bitvector(efs->number_of_fragments);
                    distance = calculate_number_of_fragments(efs->firstID, ev->id) - 1; //e.g. from 0 to 1 distance = 1
                    efs->fragments_map[distance] = true;
                    efs->time_beat = 3;
                    efs->p_to_s_channel = ev->id.substr(0, ev->id.length() - 2 * PURSUIT_ID_LEN) + algid2 + backchannel_id;
                    efs->s_to_p_channel = ev->id.substr(0, ev->id.length() - 2 * PURSUIT_ID_LEN) + algid1 + backchannel_id;
                    retransmission_channel_map.insert(pair<string, string > (efs->p_to_s_channel, bin_item_identifier));
                    efs->printEFS();
                    //cout << efs->fragments_map.to_string().c_str() << endl;
                    pthread_create(&timeout_thread, NULL, timeout_handler, NULL);
                } else {
                    distance = calculate_number_of_fragments(efs->firstID, ev->id) - 1; //e.g. from 0 to 1 distance = 1
                    if (Bitvector::Bit::unspecified_bool_type(efs->fragments_map[distance]) == false) {
                        efs->fragments_so_far++;
                        efs->fragments_map[distance] = true;
                        efs->time_beat = 3;
                        //cout << efs->fragments_map.to_string().c_str() << endl;
                        if (efs->fragments_so_far == efs->number_of_fragments) {
                            pthread_cancel(timeout_thread);
                            efs->printResult();
                            delete efs;
                            efs = NULL;
                            delete ev;
                            ba->disconnect();
                            delete ba;
                            break;
                        }
                    } else {
                        cout << "Received a duplicate fragment: " << hex_id << endl;
                    }
                }
            } else {
                /*A retransmission*/
                actual_identifier = (*retransmission_channel_map.find(ev->id.substr(0, ev->id.length() - PURSUIT_ID_LEN))).second + ev->id.substr(ev->id.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                distance = calculate_number_of_fragments(efs->firstID, actual_identifier) - 1; //e.g. from 0 to 1 distance = 1
                if (Bitvector::Bit::unspecified_bool_type(efs->fragments_map[distance]) == false) {
                    efs->fragments_so_far++;
                    efs->fragments_map[distance] = true;
                    efs->time_beat = 3;
                    //cout << efs->fragments_map.to_string().c_str() << endl;
                    if (efs->fragments_so_far == efs->number_of_fragments) {
                        pthread_cancel(timeout_thread);
                        efs->printResult();
                        delete efs;
                        efs = NULL;
                        delete ev;
                        ba->disconnect();
                        delete ba;
                        break;
                    }
                } else {
                    cout << "Received a duplicate fragment: " << hex_id << endl;
                }
            }
            pthread_mutex_unlock(&global_mutex);
            delete ev;
            break;
    }
}

int main(int argc, char* argv[]) {
    unsigned char *p_algid1, *p_algid2;
    srand(time(NULL) / getpid());
    if (argc > 1) {
        int user_or_kernel = atoi(argv[1]);
        if (user_or_kernel == 0) {
            ba = NB_Blackadder::Instance(true);
        } else {
            ba = NB_Blackadder::Instance(false);
        }
    } else {
        ba = NB_Blackadder::Instance(true);
    }
    ba->setCallback(callback);
    ba->publish_scope(bin_scope_identifier, string(), DOMAIN_LOCAL, NULL, 0);
    p_algid1 = (unsigned char *) malloc(SHA_DIGEST_LENGTH);
    SHA1((const unsigned char*) bin_item_identifier.c_str(), bin_item_identifier.length(), p_algid1);
    algid1 = string((const char *) p_algid1, PURSUIT_ID_LEN);
    p_algid2 = (unsigned char *) malloc(SHA_DIGEST_LENGTH);
    SHA1((const unsigned char*) p_algid1, SHA_DIGEST_LENGTH, p_algid2);
    algid2 = string((const char *) p_algid2, PURSUIT_ID_LEN);
    pthread_mutex_init(&global_mutex, NULL);
    pthread_cond_init(&global_cond, NULL);
    //ret = pthread_create(&event_listener, NULL, event_listener_loop, (void *) ba);
    expectedInfo.insert(pair<string, bool>(bin_item_identifier, false));
    ba->subscribe_info(bin_item_identifier.substr(bin_item_identifier.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN), bin_item_identifier.substr(0, PURSUIT_ID_LEN), DOMAIN_LOCAL, NULL, 0);
    //pthread_join(event_listener, NULL);
    ba->join();
    return 0;
}
