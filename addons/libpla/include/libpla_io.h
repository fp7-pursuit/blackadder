/*
* Copyright (c) 2008, Dmitrij Lagutin, HIIT, <dmitrij.lagutin@hiit.fi>,
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
#ifndef _LIBPLA_IO_H
#define _LIBPLA_IO_H

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "libpla_configs.h"
#include "ec.h"

/* Initial value of z-coordinate for elliptic curves */
#define INITIALZ "ffffffff ffffffff ffffffff ffffffff ffffffff e0000000"


// TODO: should this overhauled? we don't really need rho and e_y at the same time...
struct key_info
{
    elem_t rho;             /**< Implicit certificate from TTP (used for calculating real
                                 public key) */
    uint8_t b8;             /**< Compression bit of above */

    ecpoint_t e_y;          /**< Public key (used for expressing TTP's public key) */
    uint8_t e_y_b8;         /**< Compression bit of public key */
};

struct cert_info
{
    pursuit_rid_t *rid;     /**< Rendezvous id, for cases where the certificate 
                                 is tied to the specific Rid, can be NULL */

    uint32_t identity;      /**< 32bit unique identity number given by TTP */

    uint8_t deleg;          /**< Delegation bits */
    uint8_t rights;         /**< Rights bits */

    time_t not_before;      /**< Validity time limits */
    time_t not_after;

    mpz_t sigma;            /**< Private key */

    unsigned char r[PLA_HASH_LENGTH]; /**< Signature of the certificate (r+s) */
    mpz_t s;
};

void libpla_io_read_node_state_stream_s(FILE *file, struct key_info *issuer, 
    struct key_info *subject, struct cert_info *ci);
void libpla_io_write_node_state_s(FILE *file, struct key_info *issuer, 
    struct key_info *subject, struct cert_info *ci);

char* libpla_io_string_hex_to_elem(char *source, elem_t *dst);
char* libpla_io_string_hexread(char *source, unsigned char *target, unsigned int len);
void libpla_io_hexdump(FILE *file, unsigned char *buf, unsigned int len);

void libpla_io_fprint_private_key(FILE *file, struct cert_info *node);
void libpla_io_fprint_public_key(FILE *file, struct key_info *node);

int libpla_io_elem_to_hex(char *dst, elem_t src);
int libpla_io_ecpoint_t_sprintf(char *cp, ecpoint_t *point);

void libpla_io_public_key_to_bin(char *target, elem_t e, uint8_t b);
void libpla_io_bin_to_public_key(char *src, elem_t e, uint8_t *b);

void libpla_io_key_info_init(struct key_info *ki);
void libpla_io_key_info_uninit(struct key_info *ki);
void libpla_io_cert_info_init(struct cert_info *si);
void libpla_io_cert_info_uninit(struct cert_info *si);

#endif /* _LIBPLA_IO_H */
