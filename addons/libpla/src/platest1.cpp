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

#define COUNT 1069

int main(void)
{
	struct pla_hdr *pla_header;
	pla_certificate_list_item_t *pla_cert;
	unsigned char payload[1000] = "123456789";
	unsigned char psirp_header[17] = "abcdefghij";
	unsigned char *data;
	uint8_t ret;

	time_t initial, final;

	libpla_init(NULL, PLA_CRYPTO_SW); /* No rid present */

	/* Create a new PLA header */
	pla_header = (struct pla_hdr *) malloc(sizeof(struct pla_hdr));
	
	/* Test creation of a new certificate */
	char ttp_cert_path[]="ttp.cert";
	char new_cert_path[]="user.cert";
	pla_cert = libpla_create_certificate(ttp_cert_path, new_cert_path, time(NULL), time(NULL)+100000000, 31, 31, NULL);

	data = (unsigned char *) malloc(sizeof(struct pla_hdr)+((int)sizeof(psirp_header)-1)+((int)sizeof(payload)-1));
	fprintf(stderr,"Size of the psirp_header is: %d\n", (int)sizeof(psirp_header)-1);
	fprintf(stderr,"Size of the PLA header is: %d\n", (int)sizeof(struct pla_hdr));
	fprintf(stderr,"Size of the payload is: %d\n", (int)sizeof(payload)-1);

	/* Check in loop */
	initial = time(&initial);
	int i;
	for (i=0; i<COUNT; i++) {
		/* Sign packet with the certificate */
		libpla_pla_header_add(pla_header, pla_cert, psirp_header, (int)sizeof(psirp_header)-1, payload, (int)sizeof(payload)-1);

		/* Try to verify that with pla_receive function */
		memcpy(data, psirp_header, (int)sizeof(psirp_header)-1);
		memcpy(data+(int)sizeof(psirp_header)-1, pla_header, sizeof(struct pla_hdr));
		memcpy(data+(int)sizeof(psirp_header)-1+sizeof(struct pla_hdr), payload, (int)sizeof(payload)-1);
		
		/* No rid present in the certificate */
		ret = libpla_pla_receive(data, (int)sizeof(psirp_header)-1+sizeof(struct pla_hdr)+(int)sizeof(payload)-1, (int)sizeof(psirp_header)-1, NULL, NULL, 0);

		//fprintf(stderr, "PLA receive returned: %d\n", ret&PLA_SIG_OK);
	}
	final = time(&final);
	printf("Elapsed: %d seconds for %d generations + verifications (%.2f generations + verifications/second)\n", 
        (int)(final-initial), COUNT, COUNT/(float)(final-initial));
	/* Print certificate information */
	//libpla_print_certificates(stdout);
	
	free(pla_header);
	free(data);
	libpla_cleanup();
		
	return 0;
}
