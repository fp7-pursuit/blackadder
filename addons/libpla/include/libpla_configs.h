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

#ifndef _LIBPLA_CONFIGS_H
#define _LIBPLA_CONFIGS_H

/* Default paths */
#define PLA_CONF_FILE "/etc/pla.conf"   /**< Certificate for signing outgoing packets */
#define PLA_TTP_FILE "/etc/ttp.cert"    /**< TTP certificate for creating new certificates */

/* Lenghts of various fields */
#define PLA_PUBKEY_LEN      (168/8)     /**< In bytes */
#define PLA_PRIVKEY_LEN     (32)        /**< Must be multiple of 8 */

#define PLA_CERT_LEN        (32)        /**< Must be multiple of 8, NOT needed? */
#define PLA_SIG_LEN         (328/8)     /**< In bytes: 326 bits + padding */
#define PLA_HASH_LENGTH     (160/8)     /**< In bytes */
#define PLA_LOCATOR_LEN     (128/8)     /**< In bytes */

#define PLA_HEADER_SIG_OFFSET (4 + 2*PLA_PUBKEY_LEN) 
                    /**< CHANGE this if the structure of the header changes */
#define PLA_ID_HASH_LEN (4+4+4+1+1)     /**< CHANGE this if the structure of the header changes */
#define PLA_HEADER_ID_HASH_OFFSET (4 + 2*PLA_PUBKEY_LEN + PLA_SIG_LEN)
                    /**< CHANGE this if the structure of the header changes */

#define PLA_TIMESTAMP_DIFF  5          /**< Maximum difference between timestamp and reality */
#define PLA_WINDOW_SIZE     128         /**< Window for acceptable sequence numbers */

#define PLA_HEADER_TYPE     0xFF        /**< Header type used by PLA */
#define PLA_RID_HEADER_TYPE 0xFE        /**< Header type when Rid is tied to the TTP certificate */

/* Bitmask values for packet verify results */
#define PLA_TIMESTAMP_OK    0x20        /**< Timestamp is ok */
#define PLA_SEQ_NUM_OK      0x10        /**< Sequence number is ok */
#define PLA_TTP_CERT_OK     0x8         /**< TTP certificate validity time is valid */
#define PLA_TTP_LOW_PRIORITY_OK    0x4  /**< TTP certificate contains control traffic right */
#define PLA_TTP_HIGH_PRIORITY_OK   0x2 /**< TTP certificate contains full traffic right */
#define PLA_SIG_OK          0x1         /**< Signature is valid */

#define PLA_TTP_PK_HASH_SID_OK  0x80        /**< Sid = Hash(TTP public key) */
//#define PLA_TTP_OK          0x40         /**< TTP is trusted */

/* Cryptographic options */
#define PLA_CRYPTO_NULL     0
#define PLA_CRYPTO_SW       1
#define PLA_CRYPTO_HW       2           /**< NOT supported yet */

/* Rights of TTP certificates */
#define PLA_TTP_DELEG_RIGHT     0x1     /**< Right to delegate other rigths */
#define PLA_TTP_TRAFFIC_RIGHT   0x2     /**< Right to send traffic with a high priority */
#define PLA_TTP_CONTROL_RIGHT   0x4     /**< Right to send control traffic (low priority) */

/* Pub/sub-related stuff */
#define RID_LEN 32
struct pursuit_rid {
    u_int8_t id[RID_LEN];
};
typedef struct pursuit_rid pursuit_rid_t;

#endif /* _LIBPLA_CONFIGS_H */
