/*
* Copyright (c) 2008, Dmitrij Lagutin, HIIT, <dmitrij.lagutin@hiit.fi>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* Alternatively, this software may be distributed under the terms of BSD
* license.
*
* See LICENSE and COPYING for more details.
*/
#include <stdlib.h>

#include "libpla.h"
#include "libpla_io.h"

struct timezone tz;
struct timeval start_tv, end_tv;

void start_eval() {
	gettimeofday(&start_tv, &tz);
}

void stop_eval(int message) {
	gettimeofday(&end_tv, &tz);
	long int duration = end_tv.tv_sec * 1000000 + end_tv.tv_usec - start_tv.tv_sec * 1000000  - start_tv.tv_usec;
	switch (message) {
	case 0:
		fprintf(stderr, "SIGN %ld\n", duration);
		break;
	case 1:
		fprintf(stderr, "VERIFY %ld\n", duration);
		break;
	case 2:
		fprintf(stderr, "VERIFY-FAIL %ld\n", duration);
		break;
	}
}

int main(void)
{
	int payload_size = 1000;
	char *payload;
	struct pla_hdr *pla_header;
	pla_certificate_list_item_t *pla_cert;
	unsigned char psirp_header[17] = "abcdefghij";
	unsigned char *data;
	uint8_t ret;
	FILE *pFile;
	long file_size;
	int i;

	struct timezone tz;
	struct timeval start_tv;
	struct timeval end_tv;
	struct timeval duration;

	libpla_init(NULL, PLA_CRYPTO_SW); /* No rid present */

	/* Create a new PLA header */
	pla_header = (struct pla_hdr *) malloc(sizeof(struct pla_hdr));
	
	/* Test creation of a new certificate */
	char ttp_cert_path[]="ttp.cert";
	char new_cert_path[]="user.cert";
	pla_cert = libpla_create_certificate(ttp_cert_path, new_cert_path, time(NULL), time(NULL)+100000000, 31, 31, NULL);

	pFile = fopen("/home/pursuit/blackadder/trunk/api.pdf", "r");		
	if (pFile != NULL) {
		/*obtain file size*/
		fseek(pFile, 0, SEEK_END);
		file_size = ftell(pFile);
		rewind(pFile);
		for (i = 0; i <= int (file_size/payload_size); i++) {
			payload = (char *) malloc(payload_size);
			fread(payload, 1, payload_size, pFile);
			/*sign the packet with the certificate*/
start_eval();
			libpla_pla_header_add(pla_header, pla_cert, psirp_header, (int)sizeof(psirp_header)-1, (unsigned char*) payload, (int)sizeof(payload)-1);
stop_eval(0);
			/* Try to verify that with pla_receive function */
			data = (unsigned char *) malloc(sizeof(struct pla_hdr)+((int)sizeof(psirp_header)-1)+((int)sizeof(payload)-1));
			memcpy(data, psirp_header, (int)sizeof(psirp_header)-1);
			memcpy(data+(int)sizeof(psirp_header)-1, pla_header, sizeof(struct pla_hdr));
			memcpy(data+(int)sizeof(psirp_header)-1+sizeof(struct pla_hdr), payload, (int)sizeof(payload)-1);		

			/*verify the packet*/
start_eval();
			ret = libpla_pla_receive(data, (int)sizeof(psirp_header)-1+sizeof(struct pla_hdr)+(int)sizeof(payload)-1, (int)sizeof(psirp_header)-1, NULL, NULL, 0);
			if (!ret&PLA_SIG_OK) {
				printf("PLA not verified");
stop_eval(2);
			}
stop_eval(1);
		}
	}
	libpla_print_certificates(stdout);
	fclose(pFile);
	free(pla_header);
	free(data);
	libpla_cleanup();		
	return 0;
}
