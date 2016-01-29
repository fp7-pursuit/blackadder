/*
* Copyright (c) 2008, Billy Brumley, TKK, <billy.brumley@tkk.fi>,
* Janne Lundberg <janne.lundberg@iki.fi>,
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
#ifndef GNB_H_
#define GNB_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <openssl/evp.h>

#define W 32                                 /* size of an unsigned long */
// 113, 163, or 359
#define DEGREE 163                           /* exponent of the underlying field's size  */
#define MARGIN ((W - (DEGREE % W)) % W)      /* number of unused bits on the right */
#define NUMWORDS ((DEGREE + MARGIN) / W)     /* number of computer words of a field element */
#define CURVE_TYPE 4                         /* type of the curve */
#define P (CURVE_TYPE * DEGREE + 1)          /* P value for the curve */

/* jlu XXX: These blow up on a 64-bit architecture. */
#if 0
typedef unsigned long ulong_t;               /* lazyness */
typedef unsigned long elem_t[NUMWORDS];      /* type to represent a field element */
typedef unsigned char byte_t;
#else
typedef uint32_t ulong_t;                    /* lazyness */
typedef uint32_t elem_t[NUMWORDS];           /* type to represent a field element */
typedef unsigned char byte_t;
#endif

extern int32_t **R;
extern ulong_t *TA;                        /* ditto */
extern ulong_t *TB;                        /* ditto */

extern elem_t gnb_zero;
extern elem_t gnb_one;

extern const byte_t BitReverseTable256[256];

int rand_int(int n);

int elem_getcoef(const elem_t a, int i);
void elem_setcoef(elem_t a, int i);
void elem_clearcoef(elem_t a, int i);
void elem_flipcoef(elem_t a, int i);
void elem_convert_endian(elem_t a);
void elem_import_bytes(elem_t a, const byte_t *b);
void elem_export_bytes(byte_t *b, const elem_t a);
void elem_print_binary(const elem_t a);
void elem_print_hex(const elem_t a);
void elem_chomp(elem_t a);
void elem_random(elem_t a);
void elem_lrotate(elem_t a);
void elem_rrotate(elem_t a);
void elem_reverse_words(elem_t a, int s, int n);
void elem_rotate_words(elem_t a, int k);
void elem_rrotate_n(elem_t a, int offset);

int gnb_modpow(int b, int e, int m);
int gnb_compute_order(int g);
int gnb_get_u();
void gnb_init();
void gnb_add(elem_t c, const elem_t a, const elem_t b);
void gnb_and(elem_t c, const elem_t a, const elem_t b);
void gnb_square(elem_t a);
void gnb_multiply(elem_t c, const elem_t at, const elem_t bt);
void gnb_inverse(elem_t eta, const elem_t beta);

#endif /* GNB_H_ */
