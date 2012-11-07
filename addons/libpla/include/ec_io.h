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
#ifndef _EC_IO_H
#define _EC_IO_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "libpla_configs.h"
#include "ec.h"

void ec_io_print_point(ecpoint_t *p1, char *label);
void ec_io_print_point_ld(ecpoint_t *p1, char *label);
void ec_io_print_selfsig(ecselfsig_t *sig);

int ec_io_bin_to_cert(char *source, elem_t *rho, int8_t *b);

int ec_io_bin_to_privkey(char *source, mpz_t *sigma);
int ec_io_bin_to_ttppubkey(char *source, ecpoint_t *e_y);
int ec_io_cert_to_bin(char *target, elem_t rho, int8_t b);
int ec_io_privkey_to_bin(char *target, mpz_t sigma);
int ec_io_params_to_bin(char *target, elem_t rho, mpz_t sigma, int b, ecpoint_t ey);

int ec_io_sig_to_bin2(char *target, ecselfsig_t *sig);
int ec_io_bin_to_sig2(char *source, ecselfsig_t *sig);

#endif /* _EC_IO_H */
