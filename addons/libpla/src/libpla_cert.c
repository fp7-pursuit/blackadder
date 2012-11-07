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
#include "libpla_cert.h"

/**
 * Functions for certificate management 
 */

/**
 * Calculates identity hash based on the TTP-certificate fields 
 * hash = H(identity + rights + deleg. rights + validity time)
 * FIXME: maybe htonl/htons need to be added at the later stage
 * FIXME: change the order of the fields, if the structure of the PLA header changes
 *
 * @param	hash  hash string
 * @param	ci    cert_info struct containing certificate information
 */
void 
libpla_cert_calculate_id_hash(unsigned char *hash, struct cert_info *ci) 
{
    char bin_data[1000];
    char *cur = bin_data;
    uint32_t *tmp32;
    uint8_t *tmp8;
    int len;

    bzero(bin_data, 1000);

    /* Include Rid into identity hash if it exists in the certificate */
    if (ci->rid != NULL) {
        //fprintf(stderr, "Including Rid into id_hash calcucation\n");
        memcpy(cur, ci->rid, RID_LEN);
        cur += RID_LEN;
    }

    /* Convert identity */
    tmp32 = ((uint32_t *)cur);
    //*tmp32 = htonl(ci->identity);
    *tmp32 = ci->identity;
    tmp32++;
    cur = ((char *)tmp32);

    /* Convert not-before */
    tmp32 = ((uint32_t *)cur);
    //*tmp32 = htonl(ci->not_before);
    *tmp32 = ci->not_before;
    tmp32++;
    cur = ((char *)tmp32);

    /* Not-after */
    tmp32 = ((uint32_t *)cur);
    //*tmp32 = htonl(ci->not_after);
    *tmp32 = ci->not_after;
    tmp32++;
    cur = ((char *)tmp32);

    /* Convert rights bits */
    tmp8 = ((uint8_t *)cur);
    //*tmp8 = htons(ci->rights);
    *tmp8 = ci->rights;
    tmp8++;
    cur = ((char *)tmp8);

    /* Convert delegation bits */
    tmp8 = ((uint8_t *)cur);
    //*tmp8 = htons(ci->deleg);
    *tmp8 = ci->deleg;
    tmp8++;
    cur = ((char *)tmp8);

    /* Calculate hash */
    len = cur - bin_data;
    RIPEMD160((unsigned char *)bin_data, len, hash);
}


void 
libpla_cert_calculate_cert_hash(unsigned char *hash, struct key_info *issuer, 
            struct key_info *subject, struct cert_info *ci)
{
    unsigned char bin_cert[1000];
    int len = libpla_cert_to_bin(bin_cert, sizeof(bin_cert), issuer, subject, ci);

    RIPEMD160(bin_cert, len, hash);

    return;
}

int
libpla_cert_key_info_to_bin(char *target, struct key_info *info)
{
    char *cur = target;

    // e_y
    // e_y_b8
    cur += ec_io_cert_to_bin(cur, info->e_y.x, info->e_y_b8);

    // rho
    // b
    cur += ec_io_cert_to_bin(cur, info->rho, info->b8);

    return cur - target;
}


// FIXME: maybe htonl/htons need to be added at the later stage
int
libpla_cert_to_bin(unsigned char *target, int target_len, struct key_info *issuer, 
            struct key_info *subject, struct cert_info *ci)
{
    unsigned char *cur = target;	
    uint32_t *tmp32;
    uint8_t *tmp8;

    memset(target, 0, target_len);
    cur += libpla_cert_key_info_to_bin((char *)cur, issuer);
    cur += libpla_cert_key_info_to_bin((char *)cur, subject);

    /* Convert identity */
    tmp32 = ((uint32_t *) cur);
    //*tmp32 = htonl(ci->identity);
    *tmp32 = ci->identity;
    tmp32++;
    cur = ((unsigned char *) tmp32);

    /* Convert rights bits */
    tmp8 = ((uint8_t *) cur);
    *tmp8 = ci->rights;
    tmp8++;
    cur = ((unsigned char *) tmp8);

    /* Convert delegation bits */
    tmp8 = ((uint8_t *) cur);
    *tmp8 = ci->deleg;
    tmp8++;
    cur = ((unsigned char *) tmp8);

    /* Convert not-before */
    tmp32 = ((uint32_t *) cur);
    //*tmp32 = htonl(ci->not_before);
    *tmp32 = ci->not_before;
    tmp32++;
    cur = ((unsigned char *) tmp32);

    /* Not-after */
    tmp32 = ((uint32_t *) cur);
    //*tmp32 = htonl(ci->not_after);
    *tmp32 = ci->not_after;
    tmp32++;
    cur = ((unsigned char *) tmp32);

    return cur - target;
}
