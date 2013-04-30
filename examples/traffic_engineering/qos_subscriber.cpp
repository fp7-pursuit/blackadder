/*
 * Copyright (C) 2012-2013  Andreas Bontozoglou
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
#include <signal.h>
#include <map>
#include <ctime>
#include <inttypes.h>
#include <cmath>
#include <sys/time.h>

Blackadder *ba;

using namespace std;

typedef struct __stats{
  uint64_t pkt_recv;
  uint64_t bytes_recv;
  uint32_t bps_recv;
  double inter_arrival_sum;
  double jitter_sum;
  
  // Remember 1st packet to calc mean rate
  timeval ii_start;
  
  // Packet timers
  timeval t1;
  
  __stats(){
    t1.tv_sec=t1.tv_usec=0;
    bps_recv=bytes_recv=pkt_recv=0;
    jitter_sum=inter_arrival_sum=0.0;
    
  }
  
} Stats;
typedef map<string, Stats> IIStats;

string bin_id;
string bin_prefix_id;
IIStats stats;

void sigfun(int sig) {
    (void) signal(SIGINT, SIG_DFL);
    //ba->unsubscribe_info(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    ba->disconnect();
    delete ba;
    
    timeval t2;
    gettimeofday(&t2,NULL);
  
    double elapsedTime;
  
    
    cout<<"\n --- Stats ---\n";
    IIStats::iterator it = stats.begin();
    for (; it!=stats.end(); ++it){
	cout<<"II: "<<chararray_to_hex(it->first)<<endl;
	
	// Get time diff
	elapsedTime = (t2.tv_sec - it->second.ii_start.tv_sec);      // sec to ms
	elapsedTime += (t2.tv_usec - it->second.ii_start.tv_usec) / 1000000.0;   // us to s
	
	cout<<"Time Running="<<elapsedTime<<"s"<<endl;
	cout<<"Pkts="<<it->second.pkt_recv<<endl;
	cout<<"Bytes="<<it->second.bytes_recv<<endl;
	
	cout<<"Mean Rate="<<((it->second.bytes_recv*8)/elapsedTime)<<"bps"<<endl;
	cout<<"Mean Interarrival="<<(it->second.inter_arrival_sum/it->second.pkt_recv)<<"ms"<<endl;
	cout<<"Mean Jitter="<<(it->second.jitter_sum/it->second.pkt_recv)<<"ms"<<endl;
	
	cout<<"\n";
    }
    
    exit(0);
}

void calcStats(const string & ii, const int & len){
  IIStats::iterator it = stats.find(ii);
  
  // First packet
  if (it==stats.end()) {
    // Indicate that RX started
    cout << "PUBLISHED_DATA: " << chararray_to_hex(ii) << endl;
    Stats s;
    stats[ii]=s;
    gettimeofday(&stats[ii].t1,NULL);
    gettimeofday(&stats[ii].ii_start,NULL);
    stats[ii].pkt_recv++;
    stats[ii].bytes_recv+=len;
    return;
  }
  
  // Item exists
  it->second.pkt_recv++;
  it->second.bytes_recv+=len;
  
  timeval t2;
  gettimeofday(&t2,NULL);
  
  double elapsedTime;
  elapsedTime = (t2.tv_sec - it->second.t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - it->second.t1.tv_usec) / 1000.0;   // us to ms
  
  // Total Inter arrival time
  it->second.inter_arrival_sum+=elapsedTime;
  // Mean inter arrival time
  double m_ia = it->second.inter_arrival_sum/(it->second.pkt_recv-1);
  // Jitter sum
  it->second.jitter_sum+=fabs(m_ia-elapsedTime);
  
  memcpy(&it->second.t1, &t2, sizeof(timeval));
}

int main(int argc, char* argv[]) {
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
        //ba = Blackadder::Instance(true);
        // get usage instead... 
        cout<<"Usage: "<<argv[0]<<" <0|1>(user|kernel) [II ID]\n";
	return -1;
    }
    cout << "Process ID: " << getpid() << endl;
    string id = "0000000000000000";
    string prefix_id;
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);
    
    // Added optional item ID on the cmd line
    if (argc>2){
      bin_prefix_id = bin_id;
      bin_id = hex_to_chararray(argv[2]);
      ba->subscribe_info(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    }else{
      ba->subscribe_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    }
    
    while (true) {
        Event ev;
        ba->getEvent(ev);
        switch (ev.type) {
            case SCOPE_PUBLISHED:
                cout << "SCOPE_PUBLISHED: " << chararray_to_hex(ev.id) << endl;
                bin_id = ev.id.substr(ev.id.length() - PURSUIT_ID_LEN, PURSUIT_ID_LEN);
                bin_prefix_id = ev.id.substr(0, ev.id.length() - PURSUIT_ID_LEN);
                ba->subscribe_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
                break;
            case SCOPE_UNPUBLISHED:
                cout << "SCOPE_UNPUBLISHED: " << chararray_to_hex(ev.id) << endl;
                break;
            case START_PUBLISH:
                cout << "START_PUBLISH: " << chararray_to_hex(ev.id) << endl;
                break;
            case STOP_PUBLISH:
                cout << "STOP_PUBLISH: " << chararray_to_hex(ev.id) << endl;
                break;
            case PUBLISHED_DATA:
		calcStats(ev.id, ev.data_len);
                break;
        }
    }
    ba->disconnect();
    delete ba;
    return 0;
}
