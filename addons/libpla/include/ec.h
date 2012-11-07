/*
* Copyright (c) 2008, Billy Brumley, TKK, <billy.brumley@tkk.fi>,
* Dmitrij Lagutin, HIIT, <dmitrij.lagutin@hiit.fi>
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
#ifndef EC_H_
#define EC_H_

#include <stdint.h>
#include <gmp.h>

#include "gnb.h"

#define H 20 /* number of bytes in the hash function */

typedef struct {
	elem_t x, y, z;
} ecpoint_t;

typedef struct {
	ecpoint_t g, y;
	mpz_t s0, s1, r, Vm;
} ecurve_t;

typedef struct {
	mpz_t c, d;
} ecsig_t;

typedef struct {
	elem_t rho;
	byte_t r[20];
	mpz_t s;
	uint8_t b;
} ecselfsig_t;

extern ecurve_t e;
extern ecpoint_t infinity;
extern ecpoint_t infinity_ld;
extern gmp_randstate_t texas;

int ec_open(void);
void ec_close(void);

void tau_init();
void tau_round1(mpz_t a, mpf_t lambda);
void tau_round(mpz_t q0, mpz_t q1, mpf_t lambda0, mpf_t lambda1);
void tau_approx_div(mpf_t lambdap, mpz_t n, mpz_t si);
void tau_red_mod(mpz_t r0, mpz_t r1, mpz_t n);

void elem_from_hex(elem_t a, char *s);
void elem_to_int(mpz_t i, elem_t a);

void ec_init();
void ec_copy(ecpoint_t *p1, ecpoint_t *p2);
int ec_equals(ecpoint_t *p1, ecpoint_t *p2);
void ec_add(ecpoint_t *p2, ecpoint_t *p0, ecpoint_t *p1);
void ec_sub(ecpoint_t *p2, ecpoint_t *p0, ecpoint_t *p1);
void ec_copy_ld(ecpoint_t *p1, ecpoint_t *p2);
int ec_equals_ld(ecpoint_t *p1, ecpoint_t *p2);
void ec_add_ld(ecpoint_t *p2, ecpoint_t *p0, ecpoint_t *p1);
void ec_sub_ld(ecpoint_t *p2, ecpoint_t *p0, ecpoint_t *p1);
void ec_normalize(ecpoint_t *q, ecpoint_t *p);
void ec_multiply(ecpoint_t *p1, ecpoint_t *p0, mpz_t k);
void ec_multiply_binary(ecpoint_t *p1, ecpoint_t *p0, mpz_t k);
void ec_multiply_addsub(ecpoint_t *p1, ecpoint_t *p0, mpz_t k);
void ec_multiply_tau(ecpoint_t *p1, ecpoint_t *p0, mpz_t k);
void ec_keygen(ecpoint_t *w, mpz_t s);
void ec_sign_nr(ecsig_t *cd, mpz_t s, byte_t *f);
void ec_verify_nr(byte_t *f, ecsig_t *cd, ecpoint_t *w);
void ec_sign_dsa(ecsig_t *cd, mpz_t s, byte_t *f);
void ec_verify_dsa(byte_t *f, ecsig_t *cd, ecpoint_t *w);
void ec_solve_quad(elem_t z, elem_t beta);
void ec_random_point(ecpoint_t *p);
void ec_is_point_on_curve(ecpoint_t *p);
void ec_random_point_largeorder(ecpoint_t *g);
void ec_decompress_point(ecpoint_t *p, elem_t x, int y);
void ec_compress_point(int *y, ecpoint_t *p);
int ec_verify_self(ecselfsig_t *sig, byte_t *m, byte_t *id);
void ec_sign_self(ecselfsig_t *sig, byte_t *m, mpz_t sigma);
//void ec_verify_nrself(ecselfsig_t *sig);
//void ec_sign_nrself(ecselfsig_t *sig, byte_t *m, mpz_t sigma);
void ec_create_key(elem_t *impl_cert, uint8_t *impl_cert_b, mpz_t *private_key,
	unsigned char *id_hash, mpz_t *ttp_private_key);

#endif /* EC_H_ */
