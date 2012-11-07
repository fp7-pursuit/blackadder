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
#include <openssl/ripemd.h>

#include "libpla.h"
#include "libpla_io.h"
#include "ec.h"


#define COUNT 5000


/**
 * Performance testing of ECC signature generations and verifications
 */
int main(void)
{
	elem_t rho;
	uint8_t b;

    unsigned char hash[PLA_HASH_LENGTH + 1];
    hash[PLA_HASH_LENGTH] = 0;
    unsigned char id_hash[PLA_HASH_LENGTH + 1];
    id_hash[PLA_HASH_LENGTH] = 0;

	unsigned char c = 'x';
	unsigned char c2 = 'y';
	int i;

	time_t initial, final;

	ec_init();

	/* Some hashes */
    RIPEMD160(&c, 1, hash);
	RIPEMD160(&c2, 1, id_hash);

	/* Generate a new key */
	mpz_t tmp_sigma; mpz_init(tmp_sigma);
	mpz_t sigma; mpz_init(sigma);
	ec_keygen(&(e.y), tmp_sigma);
	ec_create_key(&rho, &b, &sigma, id_hash, &tmp_sigma);

	/* Test signing */
	ecselfsig_t sig;

	initial = time(&initial);
	for (i=0; i<COUNT; i++)
		ec_sign_self(&sig, hash, sigma);


	final = time(&final);
	printf("Elapsed: %d seconds for %d generations (%.2f generations/second)\n",
		(int)(final-initial), COUNT, COUNT/(float)(final-initial));

	/* Test verifying */
	sig.b = b;
	memcpy(&sig.rho, &rho, sizeof(elem_t));

	initial = time(&initial);
	for (i=0; i<COUNT; i++)
		ec_verify_self(&sig, hash, id_hash);
	final = time(&final);
	printf("Elapsed: %d seconds for %d verifications (%.2f verifications/second)\n",
		 (int)(final-initial), COUNT, COUNT/(float)(final-initial));

	return 0;
}
