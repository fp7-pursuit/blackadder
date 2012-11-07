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
#ifndef _PLA_HEADER_H
#define _PLA_HEADER_H

#include "libpla_configs.h"

struct pla_hdr
{
    uint32_t header_type;
    //uint8_t next_header_type;

    char implicit_certificate[PLA_PUBKEY_LEN];
    char ttp_public_key[PLA_PUBKEY_LEN];
    char signature[PLA_SIG_LEN];
    //char ttp_locator[PLA_LOCATOR_LEN];	/* Not needed for now, maybe will be scopeid+rid? */

    uint32_t ttp_identity;
    uint32_t ttp_not_before_time;
    uint32_t ttp_not_after_time;
    uint8_t ttp_rights;
    uint8_t ttp_deleg_rights;

    uint32_t timestamp;
    uint64_t sequence_number;
} __attribute__((__packed__));

#endif /* _PLA_HEADER_H */
