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
#ifndef _LIBPLA_H
#define _LIBPLA_H

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>

#ifdef __linux__
#include <sys/time.h>
#endif /* __linux__ */

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <openssl/ripemd.h>

#include "libpla_configs.h"
#include "pla_header.h"
#include "ec.h"
#include "ec_io.h"


/* LIST_FOREACH_SAFE is not supported by Linux's sys/queue.h */
#ifdef __linux__

#define LIST_FOREACH_SAFE(var, head, field, tvar)                       \
        for ((var) = LIST_FIRST((head));                                \
            (var) && ((tvar) = LIST_NEXT((var), field), 1);             \
            (var) = (tvar))

#endif /* __linux__ */


struct pla_certificate_list_item {
    LIST_ENTRY(pla_certificate_list_item) entries;
    struct pla_hdr *pla_header;
    mpz_t private_key;
    pursuit_rid_t *rid;
};

typedef struct pla_certificate_list_item pla_certificate_list_item_t;


void libpla_init(char *cert_file_path, uint8_t crypto);
void libpla_cleanup();
uint8_t libpla_pla_full_verify(struct pla_hdr *pla_header, unsigned char *data, uint32_t length);
uint8_t libpla_pla_lightweight_verify(struct pla_hdr *pla_header);
uint8_t libpla_pla_receive(unsigned char *data, uint32_t length, uint32_t offset, 
    pursuit_rid_t *sid, pursuit_rid_t *rid, uint8_t exit_mask);
uint8_t libpla_pla_header_add(struct pla_hdr *pla_header, 
    pla_certificate_list_item_t *pla_cert, unsigned char *psirp_header, 
    uint32_t psirp_header_len, unsigned char *payload, uint32_t payload_len);
uint8_t libpla_pla_header_sign(struct pla_hdr *pla_header, pla_certificate_list_item_t *pla_cert, unsigned char *payload, uint32_t payload_len);

void libpla_sign(ecselfsig_t *sig, byte_t *m, mpz_t sigma);
uint8_t libpla_verify(ecselfsig_t *sig, byte_t *m, byte_t *id);
uint8_t libpla_check_sequence_number(char *impl_cert, uint64_t seqnum);
void libpla_get_subject_pk(uint8_t **pk_ret, uint32_t *len_ret);

//TODO: maybe certificate management functions to libpla_cert.h file?
pla_certificate_list_item_t* libpla_create_certificate(char *ttp_cert_path, 
    char *new_cert_path, uint32_t not_before_time, uint32_t not_after_time, 
    uint8_t rights, uint8_t deleg_rights, pursuit_rid_t *rid);
pla_certificate_list_item_t* libpla_read_certificate(char *cert_file_path); 

pla_certificate_list_item_t* libpla_search_certificate(pursuit_rid_t *rid, unsigned char *impl_cert);
uint8_t libpla_delete_certificate(pursuit_rid_t *rid, unsigned char *impl_cert);

void libpla_print_certificates(FILE *stream);
void libpla_print_certificate_information(FILE *stream, pla_certificate_list_item_t *pla_cert);

#endif /* _LIBPLA_H */
