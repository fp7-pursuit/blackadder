/*
 * Copyright (C) 2010-2012  Yi-Ching Liao
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

#include "pla.hh"

CLICK_DECLS

PLA::PLA() {
}

PLA::~PLA() {
	click_chatter("PLA: destroyed!");
}

#if HAVE_USE_PLA
int PLA::configure(Vector<String> &conf, ErrorHandler *errh) {
	click_chatter("*****PLA CONFIGURATION*****");
	String _gc, _forwarder_element;
	VER_PROB = 0;
	LIGHT_VER_PROB = 0;	
	SIGN_PLA = true;
	CRYPTO = true;
	_id = 0;
	int reverse_proto;
	cp_integer(String("0x080a"), 16, &reverse_proto);
	proto_type = htons(reverse_proto);
	if (Args(conf, this, errh)
		.read_mp("GLOBAL", _gc)
		.read_mp("FORWARDER", _forwarder_element)
		.read_p("SIGN_PLA", SIGN_PLA)
		.read_p("LIGHT_VER_PROB", FixedPointArg(VERIFYING_SHIFT), LIGHT_VER_PROB)
		.read_p("VER_PROB", FixedPointArg(VERIFYING_SHIFT), VER_PROB)
		.read_p("CRYPTO", CRYPTO)
		.complete() < 0)
		return -1;
	gc = (GlobalConf *) cp_element(_gc, this);
	forwarder_element = (Forwarder *) cp_element(_forwarder_element, this);
	if (LIGHT_VER_PROB > (1 << LIGHT_VERIFYING_SHIFT)) {
		return errh->error("lightweight verification probability must be between 0 and 1");
	}
	if (VER_PROB > (1 << VERIFYING_SHIFT)) {
		return errh->error("verification probability must be between 0 and 1");
	}
	/*Initialize libpla*/
	if (CRYPTO) {				
		libpla_init(NULL, PLA_CRYPTO_SW);
	} else { 
		libpla_init(NULL, PLA_CRYPTO_NULL);
	}
	if (gc->use_mac) {
		HD_LEN = sizeof(click_ether); 
	} else {
		HD_LEN = sizeof(click_udp) + sizeof(click_ip);
	}
	PLA_LEN = sizeof(struct pla_hdr);
	return 0;
}

int PLA::initialize(ErrorHandler *errh) {
	return 0;
}

void PLA::cleanup(CleanupStage stage) {
}

void PLA::push(int in_port, Packet *p) {
	WritablePacket *newPacket;
	ForwardingEntry *fe;
	Vector<ForwardingEntry *> out_links;
	BABitvector FID(FID_LEN * 8);
	BABitvector andVector(FID_LEN * 8);
	Vector<ForwardingEntry *>::iterator out_links_it;
	struct pla_hdr *pla_header = (struct pla_hdr *) malloc(PLA_LEN);
	uint8_t pla_len = 0;
	int i;
	click_ip *ip;
	click_udp *udp;
	unsigned csum;
	uint8_t ret = 0;
	uint16_t len;

	if (in_port == 0) {
		start_eval();
		memcpy(FID._data, p->data() + HD_LEN, FID_LEN);
		/*Check all entries in my forwarding table and forward appropriately*/
		for (i = 0; i < forwarder_element->fwTable.size(); i++) {
			fe = forwarder_element->fwTable[i];
			andVector = (FID)&(*fe->LID);
			if (andVector == (*fe->LID)) {
				if (SIGN_PLA) {
					/*remove original header, reserve headroom for the PLA header*/
					newPacket = Packet::make(HD_LEN + PLA_LEN, p->data() + HD_LEN, p->length() - HD_LEN, 0);
					/*sign the packet with the certificate*/
					libpla_pla_header_sign(pla_header, NULL, (unsigned char*) newPacket->data(), newPacket->length());
					newPacket = newPacket->push(PLA_LEN);
					memcpy(newPacket->data(), pla_header, PLA_LEN);
					if (gc->use_mac) {
						/*push back the original mac header*/
						newPacket = newPacket->push_mac_header(HD_LEN);
						memcpy(newPacket->data(), p->data(), HD_LEN);						
					} else {
						/*push back the original IP header*/
						newPacket = newPacket->push(HD_LEN);
						memcpy(newPacket->data(), p->data(), HD_LEN);
						ip = reinterpret_cast<click_ip *> (newPacket->data());
						udp = reinterpret_cast<click_udp *> (ip + 1);
						/*change the length and checksum field*/
						ip->ip_len = htons(newPacket->length());
						ip->ip_sum = click_in_cksum((unsigned char *) ip, sizeof (click_ip));
						newPacket->set_ip_header(ip, sizeof (click_ip));
						/*change the length and checksum field*/
						len = newPacket->length() - sizeof (click_ip);
						udp->uh_ulen = htons(len);
						csum = click_in_cksum((unsigned char *) udp, len);
						udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
					}
					/*push the packet to the appropriate ToDevice Element*/
					output(fe->port).push(newPacket);
				} else {
					output(fe->port).push(p);
				}
				stop_eval();
				print_eval(0);
			}
		}
	} else if (in_port >= 1) {
	/**a packet has been pushed by the underlying network.**/
		start_eval();
		/*verify PLA header*/		
		if (p->length() - HD_LEN - FID_LEN - PLA_LEN > 0) {
			memcpy(pla_header, p->data() + HD_LEN, PLA_LEN);
			if (pla_header->header_type == PLA_HEADER_TYPE || pla_header->header_type == PLA_RID_HEADER_TYPE) {							
				/*verify the packet according to the verification probability*/
				if ((click_random() & VERIFYING_MASK) < VER_PROB) {
					ret = libpla_pla_full_verify(pla_header, (unsigned char*) p->data() + HD_LEN, p->length() - HD_LEN);
					if ( !(ret&PLA_SIG_OK) ) {
						p->kill();
						stop_eval();
						print_eval(2);
						return;
					} else {
						pla_len = PLA_LEN;
					}
				/*verify the packet according to the lightweight verification probability*/
				} else if ((click_random() & LIGHT_VERIFYING_MASK) < LIGHT_VER_PROB) {			
					ret = libpla_pla_lightweight_verify(pla_header);
					if ( !((ret&PLA_TTP_HIGH_PRIORITY_OK)>>1)) {
						p->kill();
						stop_eval();
						print_eval(5);
						return;
					} else {
						pla_len = PLA_LEN;
					}
				} else {
					/*skip the verification*/
					ret = 0xFF;
					pla_len = PLA_LEN;
				}
			}
		}

		/*check if it needs to be forwarded*/
		memcpy(FID._data, p->data() + HD_LEN + pla_len, FID_LEN);
		BABitvector testFID(FID);
		testFID.negate();
		if (!testFID.zero()) {
			/*Check all entries in my forwarding table and forward appropriately*/
			for (i = 0; i < forwarder_element->fwTable.size(); i++) {
				fe = forwarder_element->fwTable[i];
				andVector = (FID)&(*fe->LID);
				if (andVector == (*fe->LID)) {
					out_links.push_back(fe);
				}
			}
		}
		/*check if the packet must be pushed locally*/
		andVector = FID & gc->iLID;
		if (andVector == gc->iLID) {
			if (pla_len > 0) {
				/*remove the PLA header*/
				newPacket = Packet::make(HD_LEN, p->data() + HD_LEN + pla_len, p->length() - HD_LEN - pla_len, 0);
				newPacket = newPacket->push(HD_LEN);
				memcpy(newPacket->data(), p->data(), HD_LEN);
				output(0).push(newPacket);
			} else {
				output(0).push(p);
			}
			stop_eval();
			if (ret == 0xFF) {
				print_eval(3);
			} else if  (pla_len == 0) {
				print_eval(4);
			} else if ( ret&PLA_SIG_OK ) {
				print_eval(1);
			} else if ( (ret&PLA_TTP_HIGH_PRIORITY_OK)>>1 ) {
				print_eval(6);
			}
		}
		if (!testFID.zero()) {
			for (out_links_it = out_links.begin(); out_links_it != out_links.end(); out_links_it++) {
				newPacket = p->uniqueify();
				fe = *out_links_it;
				/*change the destination MAC/IP address and related fields*/
				if (gc->use_mac) {
					memcpy(newPacket->data(), fe->dst->data(), MAC_LEN);
					memcpy(newPacket->data() + MAC_LEN, fe->src->data(), MAC_LEN);					
				} else {
					ip = reinterpret_cast<click_ip *> (newPacket->data());
					ip->ip_src = fe->src_ip->in_addr();
					ip->ip_dst = fe->dst_ip->in_addr();
					ip->ip_tos = 0;
					ip->ip_off = 0;
					ip->ip_ttl = 250;
					ip->ip_sum = 0;
					ip->ip_sum = click_in_cksum((unsigned char *) ip, sizeof (click_ip));
					udp = reinterpret_cast<click_udp *> (ip + 1);
					len = p->length() - sizeof (click_ip); 
					udp->uh_sum = 0;
					csum = click_in_cksum((unsigned char *) udp, len);
					udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
				}
				/*push the packet to the appropriate ToDevice Element*/
				output(fe->port).push(newPacket);
				stop_eval();
				if (ret == 0xFF) {
					print_eval(3);
				} else if  (pla_len == 0) {
					print_eval(4);
				} else if ( ret&PLA_SIG_OK ) {
					print_eval(1);
				} else if ( (ret&PLA_TTP_HIGH_PRIORITY_OK)>>1 ) {
					print_eval(6);
				}
			}
		}

	}
}

void PLA::start_eval() {
	gettimeofday(&start_tv, &tz);
}

void PLA::stop_eval() {
	gettimeofday(&end_tv, &tz);
}

void PLA::print_eval(int message) {
	long int duration = end_tv.tv_sec * 1000000 + end_tv.tv_usec - start_tv.tv_sec * 1000000  - start_tv.tv_usec;
	if (message == 0) {
		click_chatter("SIGN %ld\n", duration);
	} else if (message == 1) {
		click_chatter("VERIFY-PLA %ld\n", duration);
	} else if (message == 2) {
		click_chatter("VERIFY-FAIL %ld\n", duration);
	} else if (message == 3) {
		click_chatter("VERIFY-SKIP %ld\n", duration);
	} else if (message == 4) {
		click_chatter("VERIFY-NOPLA %ld\n", duration);
	} else if (message == 5) {
		click_chatter("VERIFY-LIGHT-FAIL %ld\n", duration);
	} else if (message == 6) {
		click_chatter("VERIFY-LIGHT-PLA %ld\n", duration);
	}
}
#else /* !HAVE_USE_PLA */
#warning PLA support not enabled
#endif /* HAVE_USE_PLA */

CLICK_ENDDECLS
ELEMENT_REQUIRES(userlevel)
EXPORT_ELEMENT(PLA)
//ELEMENT_LIBS(-L../libpla/include -lpla)
