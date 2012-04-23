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
#include <arpa/inet.h>
#include <openssl/sha.h>

#include <map>
#include <list>
#include <math.h>

using namespace std;

NB_Blackadder *ba;
string item_identifier = "00000000000000001111111111111111";
string bin_item_identifier = hex_to_chararray(item_identifier);
string algid1, algid2;
string s_to_p;
string p_to_s;
int number_of_fragments;
int data_len = 150000 * 1420;
int fragment_size = 1420;

class IDRange {
public:

    IDRange(string _prefix, string _first_fragment, int _number_of_fragments) {
        prefix = _prefix;
        first_fragment = _first_fragment;
        number_of_fragments = _number_of_fragments;
    }
    string prefix;
    string first_fragment;
    int number_of_fragments;
};

list<IDRange *> fragmentsToPublish;

void find_next(string &fragment_id);
void find_previous(string &fragment_id);

pthread_mutex_t queue_mutex;
pthread_cond_t queue_cond;

map<string, pair<string, bool> > retransmission_channels;

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

void *publisher_loop(void *arg) {
    list<IDRange *>::iterator iter;
    char *payload;
    IDRange *range;
    int counter = 0;
    while (true) {
        if (counter == 20) {
            usleep(100);
            counter = 0;
        } else {
            counter++;
        }
        pthread_mutex_lock(&queue_mutex);
        if (fragmentsToPublish.empty()) {
            //cout << "WAITING ON THE CONDITION VARIABLE" << endl;
            pthread_cond_wait(&queue_cond, &queue_mutex);
            pthread_mutex_unlock(&queue_mutex);
        } else {
            iter = fragmentsToPublish.begin();
            while (iter != fragmentsToPublish.end()) {
                range = (*iter);
                payload = (char *) malloc(fragment_size);
                ba->publish_data(range->prefix + range->first_fragment, DOMAIN_LOCAL, NULL, 0, payload, fragment_size);
                //cout << "Publishing fragment: " << chararray_to_hex(range->prefix + range->first_fragment) << endl;
                find_next(range->first_fragment);
                range->number_of_fragments--;
                //cout << range->number_of_fragments << endl;
                if (range->number_of_fragments == 0) {
                    iter = fragmentsToPublish.erase(iter);
                    delete range;
                } else {
                    iter++;
                }
            }
            pthread_mutex_unlock(&queue_mutex);
        }
    }
}

void callback(Event *ev) {
    string hex_id;
    string first_fragment;
    IDRange *frags;
    unsigned char IDlen;
    string ID_to_retransmit;
    int no_of_fragments;
    int sequence_number;
    int htonl_number_of_fragments;
    hex_id = chararray_to_hex(ev->id);
    switch (ev->type) {
        case SCOPE_PUBLISHED:
            //cout << "SCOPE_PUBLISHED: " << hex_id << endl;
            delete ev;
            break;
        case SCOPE_UNPUBLISHED:
            //cout << "SCOPE_UNPUBLISHED: " << hex_id << endl;
            delete ev;
            break;
        case START_PUBLISH:
            cout << "START_PUBLISH: " << hex_id << endl;
            if (ev->id.compare(bin_item_identifier) == 0) {
                first_fragment = hex_to_chararray("0000000000000000");
                htonl_number_of_fragments = htonl(number_of_fragments);
                first_fragment.replace(0, 4, (const char *) &htonl_number_of_fragments, sizeof (int));
                frags = new IDRange(bin_item_identifier, first_fragment, number_of_fragments);
                pthread_mutex_lock(&queue_mutex);
                fragmentsToPublish.push_back(frags);
                pthread_cond_signal(&queue_cond);
                pthread_mutex_unlock(&queue_mutex);
            }
            delete ev;
            break;
        case STOP_PUBLISH:
            cout << "STOP_PUBLISH: " << hex_id << endl;
            delete ev;
            break;
        case PUBLISHED_DATA:
            //cout << "PUBLISHED_DATA: " << hex_id << endl;
            if (ev->id.compare(0, 2 * PURSUIT_ID_LEN, (bin_item_identifier.substr(0, PURSUIT_ID_LEN) + algid1)) == 0) {
                s_to_p = ev->id;
                p_to_s = bin_item_identifier.substr(0, PURSUIT_ID_LEN) + algid2 + s_to_p.substr(s_to_p.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                if (retransmission_channels.find(s_to_p) == retransmission_channels.end()) {
                    /*that's the first retransmission request from a subscriber*/
                    /*I will advertise a channel so that I can retransmit to the subscriber - This MUST be reliable*/
                    retransmission_channels.insert(pair<string, pair<string, bool > >(s_to_p, pair<string, bool >(p_to_s, false)));
                    ba->publish_info(p_to_s.substr(p_to_s.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN), p_to_s.substr(0, p_to_s.length() - PURSUIT_ID_LEN), DOMAIN_LOCAL, NULL, 0);
                }
                memcpy(&IDlen, ev->data, sizeof (IDlen));
                ID_to_retransmit = string((char *) ev->data + sizeof (IDlen), IDlen * PURSUIT_ID_LEN);
                memcpy(&no_of_fragments, (char *) ev->data + sizeof (IDlen) + IDlen * PURSUIT_ID_LEN, sizeof (int));
                memcpy(&sequence_number, (char *) ev->data + sizeof (IDlen) + IDlen * PURSUIT_ID_LEN + sizeof (int), sizeof (sequence_number));
                //cout << "retransmission request for " << chararray_to_hex(ID_to_retransmit) << ", number of fragments: " << no_of_fragments << ", sequence number: " << sequence_number << endl;
                if (no_of_fragments > 0) {
                    frags = new IDRange(p_to_s, ID_to_retransmit.substr(ID_to_retransmit.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN), no_of_fragments);
                    pthread_mutex_lock(&queue_mutex);
                    fragmentsToPublish.push_back(frags);
                    pthread_cond_signal(&queue_cond);
                    pthread_mutex_unlock(&queue_mutex);
                } else {
                    cout << "that's a bug on the subscriber's side" << endl;
                }
            }
            delete ev;
            break;
    }
}

int main(int argc, char* argv[]) {
    pthread_t publisher;
    unsigned char *p_algid1, *p_algid2;
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
    p_algid1 = (unsigned char *) malloc(SHA_DIGEST_LENGTH);
    SHA1((const unsigned char*) bin_item_identifier.c_str(), bin_item_identifier.length(), p_algid1);
    algid1 = string((const char *) p_algid1, PURSUIT_ID_LEN);
    p_algid2 = (unsigned char *) malloc(SHA_DIGEST_LENGTH);
    SHA1((const unsigned char*) p_algid1, SHA_DIGEST_LENGTH, p_algid2);
    algid2 = string((const char *) p_algid2, PURSUIT_ID_LEN);

    if (data_len % fragment_size == 0) {
        number_of_fragments = data_len / fragment_size;
    } else {
        number_of_fragments = (data_len / fragment_size) + 1;
    }

    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_cond, NULL);

    //    pthread_create(&event_listener, NULL, event_listener_loop, NULL);
    pthread_create(&publisher, NULL, publisher_loop, NULL);

    ba->publish_scope(bin_item_identifier.substr(0, PURSUIT_ID_LEN), string(), DOMAIN_LOCAL, NULL, 0);
    ba->publish_info(bin_item_identifier.substr(bin_item_identifier.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN), bin_item_identifier.substr(0, PURSUIT_ID_LEN), DOMAIN_LOCAL, NULL, 0);
    /*subscribe to the scope with the algorithmically calculated ID ALGID(/0000000000000000/1111111111111111) - will be used by the subscribers to send retransmission requests*/
    ba->subscribe_scope(algid1, bin_item_identifier.substr(0, PURSUIT_ID_LEN), DOMAIN_LOCAL, NULL, 0);
    /*publish scope with the algorithmically calculated ID ALGID(ALGID(/0000000000000000/1111111111111111))*/
    ba->publish_scope(algid2, bin_item_identifier.substr(0, PURSUIT_ID_LEN), DOMAIN_LOCAL, NULL, 0);
    ba->join();
    ba->disconnect();
    delete ba;
    return 0;
}

