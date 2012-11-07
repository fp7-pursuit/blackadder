/*
* Copyright (c) 2008, Billy Brumley, TKK, <billy.brumley@tkk.fi>,
* Dmitrij Lagutin, HIIT, <dmitrij.lagutin@hiit.fi>,
* Janne Lundberg <janne.lundberg@iki.fi>
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
#include "ec_io.h"

/**
 * ECC related conversion and print functions
 */


/**
 * prints an elliptic curve point
 *
 * @param p1    point to print
 * @param label name of the point
 */
void ec_io_print_point(ecpoint_t *p1, char *label) {
  printf("%s_x : ",label);
  elem_print_hex(p1->x);
  printf("%s_y : ",label);
  elem_print_hex(p1->y);
}


/**
 * prints an elliptic curve point
 *
 * @param p1    point to print
 * @param label name of the point
 */
void ec_io_print_point_ld(ecpoint_t *p1, char *label) {
  printf("%s_x : ",label);
  elem_print_hex(p1->x);
  printf("%s_y : ",label);
  elem_print_hex(p1->y);
  printf("%s_z : ",label);
  elem_print_hex(p1->z);
}


/**
 * prints a self-certified signature
 *
 * @param sig signature to print
 */
void ec_io_print_selfsig(ecselfsig_t *sig) {
  int i;
  printf("rho : "); elem_print_hex(sig->rho);
  printf("r   : ");
  for(i=0; i<H; i++) printf("%02x ",sig->r[i]); printf("\n");
  printf("s   : %s\n",mpz_get_str(NULL,16,sig->s));
  printf("b   : %d\n",sig->b);
}


/**
 * Binary certificate to parameters
 * Returns: length of consumed data
 */
int ec_io_bin_to_cert(char *source, elem_t *rho, int8_t *b)
{
	char *cur = source;

	/* import rho */
	memcpy(rho, cur, sizeof(elem_t));
	cur += sizeof(elem_t);
	
	/* import b */
	memcpy(b, cur, sizeof(*b));
	cur += sizeof(*b);

	return cur - source;
}

/**
 * Returns the length of the created certificate.
 */
int ec_io_cert_to_bin(char *target, elem_t rho, int8_t b)
{
	char *cur = target;

	/* export rho */
	memcpy(cur, rho, sizeof(elem_t));
	cur += sizeof(elem_t);
	
	/* export b */
	memcpy(cur, &b, sizeof(b));
	cur += sizeof(b);

	return cur - target;
}


/**
 * Returns length of consumed data.
 */
int ec_io_bin_to_privkey(char *source, mpz_t *sigma)
{
	char *cur = source;
	u_int8_t count;

	/* import sigma. Client private key */
	memcpy(&count, cur, sizeof(count));
	cur += sizeof(count);

	mpz_import(*sigma, count, 1, sizeof(u_int32_t), 1, 0, cur);
//	mpz_out_str(stderr, 10, *sigma); fprintf(stderr, "\n");
	cur += sizeof(u_int32_t) * count;

	return cur - source;
}

/* 
 */
int ec_io_bin_to_ttppubkey(char *source, ecpoint_t *e_y)
{
	char *cur = source;
	
	/* import e.y. (TTP public key) */
	memcpy(e_y, cur, sizeof(ecpoint_t));
	cur += sizeof(ecpoint_t);
	
	return cur - source;
}

int ec_io_privkey_to_bin(char *target, mpz_t sigma)
{
	char *cur = target;
	size_t countp;

	/* export sigma (private key) */
//	mpz_out_str(stderr, 10, sigma);

	u_int32_t tmpbuf[100];
	mpz_export(&tmpbuf, &countp, 1, sizeof(tmpbuf[0]), 1, 0, sigma);
	
	*cur = countp;
	cur += 1;
	
	memcpy(cur, tmpbuf, sizeof(tmpbuf[0]) * countp);
	cur += sizeof(tmpbuf[0]) * countp;

	return cur - target;
}


/**
 * Convert certificate variables to binary.
 * Returns length of result.
 */
int ec_io_params_to_bin(char *target, elem_t rho, mpz_t sigma, int b, ecpoint_t ey)
{
	char *cur = target;

	/* Export certificate */
	cur += ec_io_cert_to_bin(cur, rho, b);

	/* Export private key */
	cur += ec_io_privkey_to_bin(cur, sigma);

	/* export e.y (TTP public key) */
	memcpy(cur, &e.y, sizeof(e.y));
	cur += sizeof(e.y);

	return cur - target;
}

int ec_io_sig_to_bin(char *target, ecselfsig_t *sig)
{
	char *cur = target;

	memcpy(cur, sig->rho, sizeof(sig->rho));
	cur += sizeof(sig->rho);

	memcpy(cur, sig->r, sizeof(sig->r));
	cur += sizeof(sig->r);

	memcpy(cur, &sig->b, sizeof(sig->b));
	cur += sizeof(sig->b);

	/* Finally, the mpz... grr...*/
	u_int32_t tmpbuf[100];
	size_t countp;
	mpz_export(&tmpbuf, &countp, 1, sizeof(tmpbuf[0]), 1, 0, sig->s);
	*cur = countp;
	cur += 1;
	memcpy(cur, tmpbuf, sizeof(tmpbuf[0]) * countp);
	cur += sizeof(tmpbuf[0]) * countp;

	return cur - target;
}


/**
 * Copies only signature parts to the source (sig.r + sig.s), encodes
 * countp value to the last byte
 *
 * @param	target	buffer where the signature will be stored
 * @param	sig		points to ecselfsig_t struct
 *
 * @return	the amount of bytes used from target
 */
int ec_io_sig_to_bin2(char *target, ecselfsig_t *sig)
{
	char *cur = target;

	memcpy(cur, sig->r, sizeof(sig->r));
	cur += sizeof(sig->r);

	u_int8_t tmpbuf[100];
	size_t countp;
    bzero(tmpbuf, 100);
	mpz_export(&tmpbuf, &countp, -1, sizeof(tmpbuf[0]), 1, 0, sig->s);

	/* Encode countp information (countp - 18) in two most significant bits, which should 
	   not be touched by signature */
	tmpbuf[PLA_SIG_LEN - PLA_HASH_LENGTH - 1] |= 
		(countp - (PLA_SIG_LEN - PLA_HASH_LENGTH - 3)) << 6;

    /* Copy always 21 bytes (PLA_SIG_LEN - PLA_HASH_LENGTH), 
        since the last byte contains countp information */
	memcpy(cur, tmpbuf, sizeof(tmpbuf[0]) * (PLA_SIG_LEN - PLA_HASH_LENGTH));

	cur += sizeof(tmpbuf[0]) * countp;

	return cur - target;
}


int ec_io_bin_to_sig(char *source, ecselfsig_t *sig)
{
	char *cur = source;

	memcpy(sig->rho, cur, sizeof(sig->rho));
	cur += sizeof(sig->rho);

	memcpy(sig->r, cur, sizeof(sig->r));
	cur += sizeof(sig->r);

	memcpy(&sig->b, cur, sizeof(sig->b));
	cur += sizeof(sig->b);

	/* Finally, the mpz... grr...*/
	u_int32_t tmpbuf[100];
	size_t countp;

	countp = *cur;
	cur += 1;

	memcpy(tmpbuf, cur, sizeof(tmpbuf[0]) * countp);

	mpz_init(sig->s);
	mpz_import(sig->s, countp, 1, sizeof(u_int32_t), 1, 0, tmpbuf);

	cur += sizeof(tmpbuf[0]) * countp;

	return cur - source;
}


/**
 * Copies only signature parts from the source (sig.r + sig.s), decodes
 * countp value from the last byte
 *
 * @param	source	buffer containing the signature
 * @param	sig		points to ecselfsig_t struct
 *
 * @return	the amount of bytes read from source
 */
int ec_io_bin_to_sig2(char *source, ecselfsig_t *sig)
{
	char *cur = source;

	memcpy(sig->r, cur, sizeof(sig->r));
	cur += sizeof(sig->r);

	/* Decode countp value from two most significant bits of the last byte */
	size_t countp = (uint8_t)source[PLA_SIG_LEN - 1] >> 6;
	countp += (PLA_SIG_LEN - PLA_HASH_LENGTH - 3);

	/* Remove countp value from the last byte */
	source[PLA_SIG_LEN - 1] &= 63; /* ignore first two bits */

	u_int8_t tmpbuf[100];
	memcpy(tmpbuf, cur, sizeof(tmpbuf[0]) * countp);

	mpz_init(sig->s);
	mpz_import(sig->s, countp, -1, sizeof(u_int8_t), 1, 0, tmpbuf);

	cur += sizeof(tmpbuf[0]) * countp;

	return cur - source;
}
