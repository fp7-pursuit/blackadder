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
#ifndef _LIBPLA_CERT_H
#define _LIBPLA_CERT_H

#include <openssl/ripemd.h>

#include "libpla_io.h"
#include "ec_io.h"

void libpla_cert_calculate_id_hash(unsigned char *hash, struct cert_info *ci);
void libpla_cert_calculate_cert_hash(unsigned char *digest, struct key_info *issuer, 
                    struct key_info *subject, struct cert_info *ci);
int libpla_cert_key_info_to_bin(char *target, struct key_info *info);
int libpla_cert_to_bin(unsigned char *target, int target_len, struct key_info *issuer, 
    struct key_info *subject, struct cert_info *ci);

#endif /* _LIBPLA_CERT_H */
