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
#include "libpla.h"
#include "libpla_io.h"
#include "libpla_connections.h"
#include "libpla_cert.h"

struct list_head connection_list;

uint8_t crypto_type;
pla_certificate_list_item_t *default_certificate;

LIST_HEAD(pla_certificate_list, pla_certificate_list_item) pla_certificate_list_head;

/**
 * Initializes libpla
 *
 * @param cert_file	 path to the certificate file (can be NULL)
 * @param crypto		cryptographic solution to use for signatures
 */
void
libpla_init(char *cert_file_path, uint8_t crypto) 
{
	ec_init();

	libpla_connections_init();
	LIST_INIT(&pla_certificate_list_head);

	default_certificate = (pla_certificate_list_item_t *) malloc(sizeof(pla_certificate_list_item_t));

	if ((crypto != PLA_CRYPTO_NULL) && (crypto != PLA_CRYPTO_SW) &&
		(crypto != PLA_CRYPTO_HW)) {
		fprintf(stderr, "Error: invalid crypto type\n");
		exit(EXIT_FAILURE);
	}

	if (crypto == PLA_CRYPTO_HW) {
		fprintf(stderr, "Error: hardware crypto not implemented yet\n");
		exit(EXIT_FAILURE);
	}

	crypto_type = crypto;

	/* Read certificate information from file and set default values */
	default_certificate = libpla_read_certificate(cert_file_path);
}


void
libpla_cleanup()
{
	pla_certificate_list_item_t *plc, *plc1;

	libpla_connections_cleanup();

/*
	// testing:
	LIST_FOREACH(plc, &pla_certificate_list_head, entries) {
		printf("cleanup: pla_hdr is: %lu\n", (uint64_t)plc->pla_header);
		//printf("seqnum is: %lu\n", plc->pla_header->sequence_number);
	}
*/

	/* Delete linked list */
	LIST_FOREACH_SAFE(plc, &pla_certificate_list_head, entries, plc1) {
		LIST_REMOVE(plc, entries);
		if (plc->rid != NULL)
			free(plc->rid);
		mpz_clear(plc->private_key);
		free(plc->pla_header);
		free(plc);
	}

}

/**
 * Receive PLA packet and perform security checks (Yi-Ching Liao)
 *
 * @param	pla_header the pla header
 * @param	data		signed packet content (including PLA header, but excluding 
 *					  Ethernet header, forwarding identifier and other fields 
 *					  which are not included into signature calculation)
 * @param	length	  total length of data
 * @param	verify_mask   denotes if function should exit early if some check fails,
 *					  this parameter is a combination of PLA_xxx_OK bitmasks. 
 *					  If exit_mask is 0, all checks will be performed
 *
 * @return	verification result (see libpla_configs.h)
 */
uint8_t
libpla_pla_full_verify(struct pla_hdr *pla_header, unsigned char *data, uint32_t length)
{	
	uint8_t ret=0;

	/* 1. Check the timestamp */
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	if (abs((uint32_t)tv.tv_sec - pla_header->timestamp) <= PLA_TIMESTAMP_DIFF) {
		ret |= PLA_TIMESTAMP_OK;
	} else {	
		fprintf(stderr, "Verification error: invalid timestamp, time difference is: %d\n", (uint32_t)tv.tv_sec - pla_header->timestamp);
		free(pla_header);
		return ret;
	}

	/* 2. Check the Sequence number... */
	if (libpla_check_sequence_number(pla_header->implicit_certificate, pla_header->sequence_number)) {
		ret |= PLA_SEQ_NUM_OK;
	} else {
		fprintf(stderr, "Verification error: invalid sequence number: %d\n", pla_header->sequence_number);
		free(pla_header);
		return ret;
	}

	/* 3. Check TTP certificate validity time*/
	if ((uint32_t)tv.tv_sec - pla_header->ttp_not_before_time >= 0 && pla_header->ttp_not_after_time - (uint32_t)tv.tv_sec >= 0) {
		ret |= PLA_TTP_CERT_OK;
	} else {
		fprintf(stderr, "Verification error: invalid TTP certificate validity time\n");
		free(pla_header);
		return ret;
	}

	/* 3.1 Check TTP certificate validity rights*/
	if (pla_header->ttp_rights & PLA_TTP_CONTROL_RIGHT) {
		ret |= PLA_TTP_LOW_PRIORITY_OK;
	} else {
		fprintf(stderr, "Verification error: invalid TTP certificate validity rights\n");
		free(pla_header);
		return ret;
	}
	if (pla_header->ttp_rights & PLA_TTP_TRAFFIC_RIGHT) {
		ret |= PLA_TTP_HIGH_PRIORITY_OK;
	} else {
		fprintf(stderr, "Verification error: invalid TTP certificate validity rights\n");
		free(pla_header);
		return ret;
	}

	/* 4. Extract sender's PK and check signature... */
	elem_t elem;
	bzero(&elem, sizeof(elem_t));
	ecselfsig_t sig;
	bzero(&sig, sizeof(ecselfsig_t));

	uint8_t b;
	libpla_io_bin_to_public_key(pla_header->implicit_certificate, sig.rho, &sig.b);
	libpla_io_bin_to_public_key(pla_header->ttp_public_key, elem, &b);

	/* Decompress TTP PK to e.y */
	ec_decompress_point(&e.y, elem, b);
	libpla_io_string_hex_to_elem(INITIALZ, &e.y.z);

	ec_io_bin_to_sig2(pla_header->signature, &sig);

	/* Calculate hash over the packet ignoring the signature */
	RIPEMD160_CTX ctx;
	RIPEMD160_Init(&ctx);
	RIPEMD160_Update(&ctx, data, PLA_HEADER_SIG_OFFSET);
	RIPEMD160_Update(&ctx, data + PLA_HEADER_SIG_OFFSET + PLA_SIG_LEN, length - PLA_HEADER_SIG_OFFSET - PLA_SIG_LEN);
	unsigned char hash[PLA_HASH_LENGTH + 1];// Workaround for EC bug.
	hash[PLA_HASH_LENGTH] = 0;			  // Workaround for EC bug.
	RIPEMD160_Final(hash, &ctx);

	/* Calculate id hash over TTP certificate fields, this assumes that identify hash fields 
	   start after the signature field, CHANGE this if the structure of the header changes */
	RIPEMD160_Init(&ctx);
	RIPEMD160_Update(&ctx, data + PLA_HEADER_ID_HASH_OFFSET, PLA_ID_HASH_LEN);
	unsigned char id_hash[PLA_HASH_LENGTH + 1];
	id_hash[PLA_HASH_LENGTH] = 0;
	RIPEMD160_Final(id_hash, &ctx);

	if(libpla_verify(&sig, hash, id_hash)) {
		ret |= PLA_SIG_OK;
	} else {
		fprintf(stderr, "Verification error: invalid signature\n");
	}

	free(pla_header);
	return ret;
}

uint8_t
libpla_pla_lightweight_verify(struct pla_hdr *pla_header)
{	
	uint8_t ret=0;

	/* 1. Check the timestamp */
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	if (abs((uint32_t)tv.tv_sec - pla_header->timestamp) <= PLA_TIMESTAMP_DIFF) {
		ret |= PLA_TIMESTAMP_OK;
	} else {	
		fprintf(stderr, "Verification error: invalid timestamp, time difference is: %d\n", (uint32_t)tv.tv_sec - pla_header->timestamp);
		free(pla_header);
		return ret;
	}

	/* 2. Check the Sequence number... */
	if (libpla_check_sequence_number(pla_header->implicit_certificate, pla_header->sequence_number)) {
		ret |= PLA_SEQ_NUM_OK;
	} else {
		fprintf(stderr, "Verification error: invalid sequence number: %d\n", pla_header->sequence_number);
		free(pla_header);
		return ret;
	}

	/* 3. Check TTP certificate validity time*/
	if ((uint32_t)tv.tv_sec - pla_header->ttp_not_before_time >= 0 && pla_header->ttp_not_after_time - (uint32_t)tv.tv_sec >= 0) {
		ret |= PLA_TTP_CERT_OK;
	} else {
		fprintf(stderr, "Verification error: invalid TTP certificate validity time\n");
		free(pla_header);
		return ret;
	}

	/* 3.1 Check TTP certificate validity rights*/
	if (pla_header->ttp_rights & PLA_TTP_CONTROL_RIGHT) {
		ret |= PLA_TTP_LOW_PRIORITY_OK;
	} else {
		fprintf(stderr, "Verification error: invalid TTP certificate validity rights\n");
		free(pla_header);
		return ret;
	}
	if (pla_header->ttp_rights & PLA_TTP_TRAFFIC_RIGHT) {
		ret |= PLA_TTP_HIGH_PRIORITY_OK;
	} else {
		fprintf(stderr, "Verification error: invalid TTP certificate validity rights\n");
		free(pla_header);
		return ret;
	}

	return ret;
}
/**
 * Receive PLA packet and perform security checks
 *
 * @param	data		signed packet content (including PLA header, but excluding 
 *					  Ethernet header, forwarding identifier and other fields 
 *					  which are not included into signature calculation)
 * @param	length	  total length of data
 * @param	offset	  offset in bytes where the PLA header starts
 * @param   sid		 Scope id (can be null), if sid is given,
 *					  sid = Hash(TTP public key) check will be performed
 * @param   rid		 Rendezvous id (can be null), if rid is given, it will be
 *					  included into identity hash calculation
 * @param	exit_mask   denotes if function should exit early if some check fails,
 *					  this parameter is a combination of PLA_xxx_OK bitmasks. 
 *					  If exit_mask is 0, all checks will be performed
 *
 * @return	verification result (see libpla_configs.h)
 */
uint8_t
libpla_pla_receive(unsigned char *data, uint32_t length, uint32_t offset, pursuit_rid_t *sid, pursuit_rid_t *rid, uint8_t exit_mask)
{	
	unsigned char hash[PLA_HASH_LENGTH + 1];// Workaround for EC bug.
	hash[PLA_HASH_LENGTH] = 0;			  // Workaround for EC bug.

	unsigned char id_hash[PLA_HASH_LENGTH + 1];
	id_hash[PLA_HASH_LENGTH] = 0;

	/* 1. Extract PLA header and its fields */
	struct pla_hdr *pla_header = (struct pla_hdr *) malloc(sizeof(struct pla_hdr));
	memcpy(pla_header, data+offset*sizeof(char), sizeof(struct pla_hdr));

	/* 2. Check sequence number and timestamp */
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	uint8_t ret=0;
	if (abs((uint32_t)tv.tv_sec - pla_header->timestamp) <= PLA_TIMESTAMP_DIFF) {
		ret |= PLA_TIMESTAMP_OK;
	} else {	
		fprintf(stderr, "Verification error: invalid timestamp, time difference is: %d\n", (uint32_t)tv.tv_sec - pla_header->timestamp);
		if (exit_mask & PLA_TIMESTAMP_OK) {
			free(pla_header);
			return ret;
		}
	}
	/* Sequence number... */
	if (libpla_check_sequence_number(pla_header->implicit_certificate, 
		pla_header->sequence_number) == 1) {
		ret |= PLA_SEQ_NUM_OK;
	} else {
		fprintf(stderr, "Verification error: invalid sequence number: %d\n", pla_header->sequence_number);
		if (exit_mask & PLA_SEQ_NUM_OK) {
			free(pla_header);
			return ret;
		}
	}

	/* 3. Check TTP certificate validity time and rights... */
	if (pla_header->ttp_not_before_time <= time(NULL) && 
		pla_header->ttp_not_after_time >= time(NULL)) {
		ret |= PLA_TTP_CERT_OK;
	} else {
		fprintf(stderr, "Verification error: invalid TTP certificate validity time\n");
		if (exit_mask & PLA_TTP_CERT_OK) {
			free(pla_header);
			return ret;
		}
	}
	/* Rights... */
	if (pla_header->ttp_rights & PLA_TTP_CONTROL_RIGHT) {
		ret |= PLA_TTP_LOW_PRIORITY_OK;
	} else {
		if (exit_mask & PLA_TTP_LOW_PRIORITY_OK) {
			free(pla_header);
			return ret;
		}
	}
	if (pla_header->ttp_rights & PLA_TTP_TRAFFIC_RIGHT) {
		ret |= PLA_TTP_HIGH_PRIORITY_OK;
	} else {
		if (exit_mask & PLA_TTP_HIGH_PRIORITY_OK) {
			free(pla_header);
			return ret;
		}
	}

	/* 3.1 Perform sid = Hash(TTP public key) check if sid is given */
	RIPEMD160_CTX ctx;
	if (sid != NULL) {
		RIPEMD160_Init(&ctx);
		RIPEMD160_Update(&ctx, pla_header->ttp_public_key, PLA_PUBKEY_LEN);
		RIPEMD160_Final(hash, &ctx);

		if (memcmp(hash, sid, PLA_HASH_LENGTH) == 0) {
			ret |= PLA_TTP_PK_HASH_SID_OK;
		} else {
			fprintf(stderr, "Verification error: sid != Hash(TTP_PK)\n");
			if (exit_mask & PLA_TTP_PK_HASH_SID_OK) {
				free(pla_header);
				return ret;
			}
		}
		bzero(hash, PLA_HASH_LENGTH + 1); /* Empty hash just in case */
	}

	/* 4. Extract sender's PK and check signature... */
	elem_t elem;
	bzero(&elem, sizeof(elem_t));
	ecselfsig_t sig;
	bzero(&sig, sizeof(ecselfsig_t));

	uint8_t b;
	libpla_io_bin_to_public_key(pla_header->implicit_certificate, sig.rho, &sig.b);
	libpla_io_bin_to_public_key(pla_header->ttp_public_key, elem, &b);

	/* Decompress TTP PK to e.y */
	ec_decompress_point(&e.y, elem, b);
	libpla_io_string_hex_to_elem(INITIALZ, &e.y.z);

	ec_io_bin_to_sig2(pla_header->signature, &sig);

	/* Calculate hash over the packet ignoring the signature */
	RIPEMD160_Init(&ctx);
	RIPEMD160_Update(&ctx, data, offset + PLA_HEADER_SIG_OFFSET);
	RIPEMD160_Update(&ctx, data + offset + PLA_HEADER_SIG_OFFSET + PLA_SIG_LEN, 
		length - offset - PLA_HEADER_SIG_OFFSET - PLA_SIG_LEN);
	RIPEMD160_Final(hash, &ctx);

	/* Calculate id hash over TTP certificate fields, this assumes that identify hash fields 
	   start after the signature field, CHANGE this if the structure of the header changes */
	RIPEMD160_Init(&ctx);
	if (rid != NULL) {
		RIPEMD160_Update(&ctx, rid, RID_LEN);
	}
	RIPEMD160_Update(&ctx, data + offset + PLA_HEADER_ID_HASH_OFFSET, PLA_ID_HASH_LEN);
	RIPEMD160_Final(id_hash, &ctx);

	if(libpla_verify(&sig, hash, id_hash) == 1) {
		ret |= PLA_SIG_OK;
	} else {
		fprintf(stderr, "Verification error: invalid signature\n");
	}

	free(pla_header);
	return ret;
}

uint8_t
libpla_pla_header_sign(struct pla_hdr *pla_header, pla_certificate_list_item_t *pla_cert, unsigned char *payload, uint32_t payload_len) 
{
	RIPEMD160_CTX ctx;
	struct timeval tv;
	struct timezone tz;

	ecselfsig_t sig;
	uint32_t length;

	unsigned char *data;
	unsigned char hash[PLA_HASH_LENGTH + 1];// Workaround for EC bug
	hash[PLA_HASH_LENGTH] = 0;			  // Workaround for EC bug

	if (pla_cert != NULL) { /* Use pla_header from pla_cert */
		++pla_cert->pla_header->sequence_number;
		memcpy(pla_header, pla_cert->pla_header, sizeof(struct pla_hdr));
	} else {
		++default_certificate->pla_header->sequence_number;
		memcpy(pla_header, default_certificate->pla_header, sizeof(struct pla_hdr));
	}

	/* Get current time */
	gettimeofday(&tv, &tz);
	// FIXME: maybe htonl/htons need to be added at the later stage
	pla_header->timestamp = (uint32_t)tv.tv_sec;

	/* Combine all data together, calculate hash, and sign it */
	length = sizeof(struct pla_hdr) + payload_len;
	data = (unsigned char *) malloc(length);

	memcpy(data, pla_header, sizeof(struct pla_hdr));
	memcpy(data + sizeof(struct pla_hdr), 
		payload, payload_len);

	/* Calculate hash ignoring signature field */
	RIPEMD160_Init(&ctx);
	RIPEMD160_Update(&ctx, data, PLA_HEADER_SIG_OFFSET);
	RIPEMD160_Update(&ctx, data + PLA_HEADER_SIG_OFFSET+PLA_SIG_LEN, 
		sizeof(struct pla_hdr) - PLA_HEADER_SIG_OFFSET - PLA_SIG_LEN + payload_len);
	RIPEMD160_Final(hash, &ctx);

	/* Sign and copy signature to the header */
	if (pla_cert != NULL) { /* Use private key from pla_cert */
		libpla_sign(&sig, hash, pla_cert->private_key);
	} else {
		libpla_sign(&sig, hash, default_certificate->private_key);
	}

	ec_io_sig_to_bin2(pla_header->signature, &sig);

	free(data);
	return 0;
}

/**
 * Creates a new PLA header based on the payload
 *
 * @param   pla_header		  the pla_header to be created
 * @param   pla_cert			contains certificate struct to be used to sign 
 *							  packets, can be NULL
 * @param   psirp_header		PSIRP header which resides in front of the pla_header
 * @param   psirp_header_len	length of the PSIRP header
 * @param   payload			 payload of the packet
 * @param   payload_len		 length of the payload
 *
 * @return  status (FIXME) 
 */
uint8_t
libpla_pla_header_add(struct pla_hdr *pla_header, pla_certificate_list_item_t *pla_cert, 
	unsigned char *psirp_header, uint32_t psirp_header_len, 
	unsigned char *payload, uint32_t payload_len) 
{
	RIPEMD160_CTX ctx;
	struct timeval tv;
	struct timezone tz;

	ecselfsig_t sig;
	uint32_t length;

	unsigned char *data;
	unsigned char hash[PLA_HASH_LENGTH + 1];// Workaround for EC bug
	hash[PLA_HASH_LENGTH] = 0;			  // Workaround for EC bug

	if (pla_cert != NULL) { /* Use pla_header from pla_cert */
		++pla_cert->pla_header->sequence_number;
		memcpy(pla_header, pla_cert->pla_header, sizeof(struct pla_hdr));
	} else {
		++default_certificate->pla_header->sequence_number;
		memcpy(pla_header, default_certificate->pla_header, sizeof(struct pla_hdr));
	}

	/* Get current time */
	gettimeofday(&tv, &tz);
	// FIXME: maybe htonl/htons need to be added at the later stage
	pla_header->timestamp = (uint32_t)tv.tv_sec;

	/* Combine all data together, calculate hash, and sign it */
	length = sizeof(struct pla_hdr) + psirp_header_len + payload_len;
	data = (unsigned char *) malloc(length);

	memcpy(data, psirp_header, psirp_header_len);
	memcpy(data + psirp_header_len, pla_header, sizeof(struct pla_hdr));
	memcpy(data + psirp_header_len + sizeof(struct pla_hdr), 
		payload, payload_len);

	/* Calculate hash ignoring signature field */
	RIPEMD160_Init(&ctx);
	RIPEMD160_Update(&ctx, data, psirp_header_len + PLA_HEADER_SIG_OFFSET);
	RIPEMD160_Update(&ctx, data + psirp_header_len + PLA_HEADER_SIG_OFFSET+PLA_SIG_LEN, 
		sizeof(struct pla_hdr) - PLA_HEADER_SIG_OFFSET - PLA_SIG_LEN + payload_len);
	RIPEMD160_Final(hash, &ctx);

	/* Sign and copy signature to the header */
	if (pla_cert != NULL) { /* Use private key from pla_cert */
		libpla_sign(&sig, hash, pla_cert->private_key);
	} else {
		libpla_sign(&sig, hash, default_certificate->private_key);
	}

	ec_io_sig_to_bin2(pla_header->signature, &sig);

	free(data);
	return 0;
}


/**
 *  Creates a new certificate with given parameters
 *
 *  @param  ttp_cert_path	   location of TTP certificate 
 *							  (which is used to sign new certificate)
 *  @param  new_cert_path	   location where the certificate will be saved
 *							  (can be NULL)
 *  @param  not_before_time	 starting validity time of the certificate
 *  @param  not_after_time	  end of validity time
 *  @param  rights			  rights
 *  @param  deleg_rights		delegatable rights
 *  @param  rid				 rid (can be NULL)
 *
 *  @return pointer to the pla_certificate_list_item_t containing static 
 *	  PLA header information, private key and rid
 */
pla_certificate_list_item_t* 
libpla_create_certificate(char *ttp_cert_path, char *new_cert_path,
	uint32_t not_before_time, uint32_t not_after_time, uint8_t rights,
	uint8_t deleg_rights, pursuit_rid_t *rid) 
{
	struct key_info tmp_null, issuer, subject;
	struct cert_info ttp_cert, new_cert;
	FILE *ttp_cert_file = NULL, *output_file = NULL;
	pla_certificate_list_item_t *pla_cert;

	pla_cert = (pla_certificate_list_item_t *) malloc(sizeof(pla_certificate_list_item_t));

	unsigned char hash[PLA_HASH_LENGTH + 1];
	hash[PLA_HASH_LENGTH] = 0;
	unsigned char id_hash[PLA_HASH_LENGTH + 1];
	id_hash[PLA_HASH_LENGTH] = 0;

	//ENTER();

	libpla_io_key_info_init(&tmp_null);
	libpla_io_key_info_init(&issuer);
	libpla_io_key_info_init(&subject);
	libpla_io_cert_info_init(&ttp_cert);
	libpla_io_cert_info_init(&new_cert);	

	/* 1. Read key information from the TTP certificate file */
	if (ttp_cert_path != NULL)
		ttp_cert_file = fopen(ttp_cert_path, "r");
	else
		ttp_cert_file = fopen(PLA_TTP_FILE, "r");

	if (!ttp_cert_file) {
		fprintf(stderr, "Error opening TTP certificate file: %s\n", ttp_cert_path);
		exit(EXIT_FAILURE);
	}
	libpla_io_read_node_state_stream_s(ttp_cert_file, &tmp_null, &issuer, &ttp_cert);
	fclose(ttp_cert_file);

	/* 2. Create a new ceritificate */
	/* Generate identity, FIXME: identity should be unique per TTP, Each TTP should 
	   maintain a list of given identities */
	srandom(time(NULL));
	new_cert.identity = random();

	new_cert.not_before = not_before_time;
	new_cert.not_after = not_after_time;
	new_cert.rights = rights;
	new_cert.deleg = deleg_rights;

	if (rid != NULL) {
		new_cert.rid = (pursuit_rid_t*) calloc(sizeof(unsigned char), RID_LEN);
		memcpy(new_cert.rid, rid, RID_LEN);

		/* Insert Rid also to the pla_cert */
		pla_cert->rid = (pursuit_rid_t *) calloc(sizeof(unsigned char), RID_LEN);
		memcpy(pla_cert->rid, rid, RID_LEN);
	} else {
		new_cert.rid = NULL;
		pla_cert->rid = NULL;
	}

	/* 3. Create a new key & sign the certificate */
	/* Calculate identity hash from newly created TTP-certificate fields */
	libpla_cert_calculate_id_hash(id_hash, &new_cert);

	/* Create a new key for the node */
	ec_create_key(&subject.rho, &subject.b8, &new_cert.sigma, id_hash, &ttp_cert.sigma);

	/* Calculate a real TTP's public key from it's private key */
	ec_multiply(&issuer.e_y, &(e.g), ttp_cert.sigma);
	ec_compress_point((int *)&issuer.e_y_b8, &issuer.e_y);

	/* Empty TTP's implicit certificate, it is not really needed */
	memset(issuer.rho, 0, sizeof(issuer.rho));
	issuer.b8 = 0;

	/* Sign the new certificate */
	ecselfsig_t sig;

	libpla_cert_calculate_cert_hash(hash, &issuer, &subject, &new_cert);
	ec_sign_self(&sig, hash, new_cert.sigma);

	/* Copy the signature to the new certificate */
	mpz_set(new_cert.s, sig.s); 
	memcpy(new_cert.r, sig.r, PLA_HASH_LENGTH);

	/* 4. Save new certificate (if new_cert_path != NULL) */
	if (new_cert_path != NULL) {
		output_file = fopen(new_cert_path, "w");

		if (!output_file) {
			fprintf(stderr, "Error opening ouput certificate file: %s for writing\n",
				 new_cert_path);
			exit(EXIT_FAILURE);
		}
		libpla_io_write_node_state_s(output_file, &issuer, &subject, &new_cert);
		fclose(output_file);
	}

	/* 5. Save new base PLA header into linked list and return pointer to it */
	/* Copy static header information to pla_cert->pla_header */
	pla_cert->pla_header = (struct pla_hdr *) calloc(1, sizeof(struct pla_hdr));
	//pla_cert->pla_header->next_header_type = 0;

	if (rid != NULL) {
		pla_cert->pla_header->header_type = PLA_RID_HEADER_TYPE; /* Certificate tied to Rid */
	} else {
		pla_cert->pla_header->header_type = PLA_HEADER_TYPE;
	}

	/* Get key information */
	libpla_io_public_key_to_bin(pla_cert->pla_header->ttp_public_key, issuer.e_y.x, issuer.e_y_b8);
	libpla_io_public_key_to_bin(pla_cert->pla_header->implicit_certificate, subject.rho, subject.b8);	
	//pla_cert->pla_header->next_header_type = 0; 

	/* Get other fields from TTP certificate */
	pla_cert->pla_header->ttp_identity = new_cert.identity;
	pla_cert->pla_header->ttp_rights = new_cert.rights;
	pla_cert->pla_header->ttp_deleg_rights = new_cert.deleg;
	pla_cert->pla_header->ttp_not_before_time = new_cert.not_before;
	pla_cert->pla_header->ttp_not_after_time = new_cert.not_after;
	pla_cert->pla_header->sequence_number = 0;

	/* Copy private key */
	mpz_init(pla_cert->private_key);
	mpz_set(pla_cert->private_key, new_cert.sigma);

	/* Insert certificate into linked list */
	LIST_INSERT_HEAD(&pla_certificate_list_head, pla_cert, entries);

	libpla_io_key_info_uninit(&issuer);
	libpla_io_key_info_uninit(&subject);
	libpla_io_key_info_uninit(&tmp_null);
	libpla_io_cert_info_uninit(&ttp_cert);
	libpla_io_cert_info_uninit(&new_cert);

	return pla_cert;
}


/**
 *  Reads certificate information from a file and stores it to the plh list
 *
 *  @param  cert_file_path	   location of the certificate file
 *
 *  @return pointer to the pla_certificate_list_item_t struct containing static 
 *	  PLA header information, private key and rid
 */
pla_certificate_list_item_t*
libpla_read_certificate(char *cert_file_path) 
{
	pla_certificate_list_item_t *pla_cert;
	FILE *certificate_file;
	struct key_info issuer, subject;
	struct cert_info ci;

	/* Read key information from the certificate file */
	if (cert_file_path != NULL)
		certificate_file = fopen(cert_file_path, "r");
	else
		certificate_file = fopen(PLA_CONF_FILE, "r");

	if (!certificate_file) {
		fprintf(stderr, "Error opening certificate file: %s\n", cert_file_path);
		exit(EXIT_FAILURE);
	}

	libpla_io_key_info_init(&issuer);
	libpla_io_key_info_init(&subject);
	libpla_io_cert_info_init(&ci);	

	libpla_io_read_node_state_stream_s(certificate_file, &issuer, &subject, &ci);
	fclose(certificate_file);


	/* Copy static header information to pla_cert->pla_header */
	//pla_cert = malloc(sizeof(struct pla_certificate));
	pla_cert = (pla_certificate_list_item_t *) malloc(sizeof(pla_certificate_list_item_t));
	pla_cert->pla_header = (struct pla_hdr *) calloc(1, sizeof(struct pla_hdr));

	/* Get key information */
	libpla_io_public_key_to_bin(pla_cert->pla_header->ttp_public_key, issuer.e_y.x, issuer.e_y_b8);
	libpla_io_public_key_to_bin(pla_cert->pla_header->implicit_certificate, subject.rho, subject.b8);	

	/* Get other fields from TTP certificate */
	// FIXME: maybe htonl/htons need to be added at the later stage
	pla_cert->pla_header->ttp_identity = ci.identity;
	pla_cert->pla_header->ttp_rights = ci.rights;
	pla_cert->pla_header->ttp_deleg_rights = ci.deleg;
	pla_cert->pla_header->ttp_not_before_time = ci.not_before;
	pla_cert->pla_header->ttp_not_after_time = ci.not_after;
	pla_cert->pla_header->sequence_number = 0;

	/* Copy private key */
	mpz_init(pla_cert->private_key);
	mpz_set(pla_cert->private_key, ci.sigma);

	/* Check if we have Rid */
	if (ci.rid != NULL) {
		pla_cert->rid = (pursuit_rid_t *) calloc(sizeof(unsigned char), RID_LEN);
		memcpy(pla_cert->rid, ci.rid, RID_LEN);
		pla_cert->pla_header->header_type = PLA_RID_HEADER_TYPE; /* Certificate tied to Rid */
	} else {
		pla_cert->rid = NULL;
		pla_cert->pla_header->header_type = PLA_HEADER_TYPE;
	}

	/* Insert certificate into linked list */
	LIST_INSERT_HEAD(&pla_certificate_list_head, pla_cert, entries);

	libpla_io_key_info_uninit(&issuer);
	libpla_io_key_info_uninit(&subject);
	libpla_io_cert_info_uninit(&ci);

	return pla_cert;
}


/**
 *  Searches certificate list and returns the first matching certificate
 *  Search order: 1. rid, 2. implicit certificate
 *
 *  @param  rid		 rid of the certificate
 *  @param  impl_cert   implicit certificate part of the certificate
 *
 *  @return pointer to pla_certificate structure containing certificate information,
 *		  NULL if not found
 */
pla_certificate_list_item_t* 
libpla_search_certificate(pursuit_rid_t *rid, unsigned char *impl_cert)
{
	pla_certificate_list_item_t *pla_cert;

	if ((rid == NULL) && (impl_cert == NULL)) { // no search terms given
		return NULL;
	}

	// TODO: code could be written more nicely...
	LIST_FOREACH(pla_cert, &pla_certificate_list_head, entries) {
		if ((rid != NULL) && (pla_cert->rid != NULL)) {
			if (!memcmp(rid, pla_cert->rid, RID_LEN)) { // Rid matches
				if (impl_cert != NULL) { // check also impl_cert
					if (!memcmp(impl_cert, pla_cert->pla_header->implicit_certificate, PLA_PUBKEY_LEN)) {
						return pla_cert;
					}
				} else {
					return pla_cert;
				}
			}
		} else if (impl_cert != NULL) { // Rid is null, check only impl_cert
			if (!memcmp(impl_cert, pla_cert->pla_header->implicit_certificate, PLA_PUBKEY_LEN)) {
				return pla_cert;
			}
		}
	}

	return NULL; // not found
}


/**
 *  Searches certificate list and deletes the first matching certificate
 *  Search order: 1. rid, 2. implicit certificate
 *
 *  @param  rid		 rid of the certificate
 *  @param  impl_cert   implicit certificate part of the certificate
 *
 *  @return 1 if deleted, 0 otherwise
 */
uint8_t 
libpla_delete_certificate(pursuit_rid_t *rid, unsigned char *impl_cert)
{
	pla_certificate_list_item_t *pla_cert, *plc;

	if ((rid == NULL) && (impl_cert == NULL)) { // no search terms given
		return 0;
	}

	// TODO: code could be written more nicely...
	LIST_FOREACH_SAFE(pla_cert, &pla_certificate_list_head, entries, plc) {
		if ((rid != NULL) && (pla_cert->rid != NULL)) {
			if (!memcmp(rid, pla_cert->rid, RID_LEN)) { // Rid matches
				if (impl_cert != NULL) { // check also impl_cert
					if (!memcmp(impl_cert, pla_cert->pla_header->implicit_certificate, PLA_PUBKEY_LEN)) {
				LIST_REMOVE(pla_cert, entries);
				if (pla_cert->rid != NULL)
   					free(pla_cert->rid);
				free(pla_cert->pla_header);
   				free(pla_cert);
   				return 1;
					}
				} else {
			LIST_REMOVE(pla_cert, entries);
			if (pla_cert->rid != NULL)
							free(pla_cert->rid);
						free(pla_cert->pla_header);
						free(pla_cert);
						return 1;
				}
			}
		} else if (impl_cert != NULL) { // Rid is null, check only impl_cert
			if (!memcmp(impl_cert, pla_cert->pla_header->implicit_certificate, PLA_PUBKEY_LEN)) {
		LIST_REMOVE(pla_cert, entries);
		if (pla_cert->rid != NULL)
					free(pla_cert->rid);
				free(pla_cert->pla_header);
				free(pla_cert);
				return 1;
			}
		}
	}
	return 0;
}


/**
 * Wrapper function for signature generation
 *
 * @param sig   pointer to space for the signature
 * @param m	 pointer to the hash of the message
 * @param sigma pointer to the signers private key
 */
void 
libpla_sign(ecselfsig_t *sig, byte_t *m, mpz_t sigma) 
{

	switch (crypto_type) {
	case PLA_CRYPTO_SW:
		ec_sign_self(sig, m, sigma);
		break;

	case PLA_CRYPTO_NULL:
		/* Empty signature */
		bzero(sig->r, PLA_HASH_LENGTH);
		bzero(sig->s, sizeof(mpz_t));
		break;

	case PLA_CRYPTO_HW:
		fprintf(stderr, "Error: hardware crypto not implemented yet\n");
		break;

	default:
		fprintf(stderr, "Error: invalid crypto type\n");
		exit(EXIT_FAILURE);
	}
}


/**
 * Wrapper function for signature verification
 *
 * @param sig   pointer to signature to verify
 * @param m	 pointer to the hash of the message
 * @param id	pointer to the hash of the signer's ID
 *
 * @return 1 if verification is successful, else 0
 */
uint8_t 
libpla_verify(ecselfsig_t *sig, byte_t *m, byte_t *id) {
	switch (crypto_type) {
	case PLA_CRYPTO_SW:
		return ec_verify_self(sig, m, id);

	case PLA_CRYPTO_NULL:
		return 1;

	case PLA_CRYPTO_HW:
		fprintf(stderr, "Error: hardware crypto not implemented yet\n");
		return 0;

	default:
		fprintf(stderr, "Error: invalid crypto type\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}


/**
 * Verifies sequence number (that packet is not a duplicate, and has not been 
 * execessively delayed)
 * TODO: Since impl_cert changes with a new TTP certificate, we may also use 
 *  TTP PK + identity number to identify senders....
 *
 * @param impl_cert pointer to implicit certificate
 * @param seqnum	sequence number
 *
 * @return 1 if sequence number is ok, else 0
 */
uint8_t libpla_check_sequence_number(char *impl_cert, uint64_t seqnum) 
{
	/* Find entry. If not available, create it. */
	struct pla_connection *entry;
	if (!(entry = libpla_connection_find(impl_cert))) {
		entry = (struct pla_connection *) malloc(sizeof(struct pla_connection));
		if (!entry) {
			return -ENOMEM;
		}
		memcpy(entry->key, impl_cert, sizeof(entry->key));
		memset(entry->window, 0, sizeof(entry->window));
		entry->window_low = 0;
		entry->low_index = 0;

		list_add(&entry->list_head, &connection_list);
	}

	/* Seqnum is below window: drop packet */
	if (seqnum < entry->window_low) {
		return 0;
	}

	int window_high = entry->window_low + PLA_WINDOW_SIZE - 1;   /* Highest sequence number that can be put in window */

	/* Seqnum within window: */
	if (seqnum <= window_high) {
		int entry_index = (entry->low_index + (seqnum-entry->window_low)) % PLA_WINDOW_SIZE;

		if (entry->window[entry_index]) {
			return 0;	/* Drop duplicate */
		} else {
			entry->window[entry_index] = 1;
			return 1;	/* Accept new packet */
		}
		
	}

	/* Seqnum is above window: advance window and accept packet */
	int advance = seqnum - window_high;

	/* Zero part of window */
	int nulled_entries = MIN(PLA_WINDOW_SIZE, advance);
	for (int i=nulled_entries; i--; )
		entry->window[(entry->low_index + i) % PLA_WINDOW_SIZE] = 0;

	entry->low_index = (entry->low_index + advance) % PLA_WINDOW_SIZE;
	entry->window_low += advance;
	entry->window[(entry->low_index + (seqnum-entry->window_low)) % PLA_WINDOW_SIZE] = 1;
	
	return 1;
}


/**
 * Prints all certificate information
 *
 *  @param stream	   output stream
 *
 */
void libpla_print_certificates(FILE *stream) {

	pla_certificate_list_item_t *pla_cert;
	int i = 0;

	fprintf(stream, "================== Certificate list ==================\n");
	LIST_FOREACH(pla_cert, &pla_certificate_list_head, entries) {
		libpla_print_certificate_information(stream, pla_cert);
		i++;
	}
	fprintf(stream, "================ Found %d certificates ================\n\n", i);
}


/**
 * Prints certificate information
 *
 *  @param stream	   output stream
 *  @param pla_hdr	  pla_header information
 *  @param private_key  private key
 */
void 
libpla_print_certificate_information(FILE *stream, pla_certificate_list_item_t *pla_cert)
{
	char nb[50]; char na[50];
	struct tm tm_nb; struct tm tm_na;

	time_t temp, temp2;
	temp = pla_cert->pla_header->ttp_not_before_time;
	temp2 = pla_cert->pla_header->ttp_not_after_time;

	/* Get TTP certifiate validity time */
	localtime_r(&temp, &tm_nb);
	localtime_r(&temp2, &tm_na);

	strftime(nb, sizeof(nb), "%Y-%m-%d %H:%M:%S", &tm_nb);
	strftime(na, sizeof(na), "%Y-%m-%d %H:%M:%S", &tm_na);


	// TODO: print in the placonf format?
	fprintf(stream, "===== Certificate information =====\n");

	fprintf(stream, "Implicit certificate:\n");
	libpla_io_hexdump(stderr, 
		(unsigned char *)pla_cert->pla_header->implicit_certificate, PLA_PUBKEY_LEN);
	fprintf(stderr,"\n\n");

	fprintf(stream, "TTP public key:\n");
	libpla_io_hexdump(stderr, 
		(unsigned char *)pla_cert->pla_header->ttp_public_key, PLA_PUBKEY_LEN);
	fprintf(stderr,"\n\n");

	fprintf(stream, "Private key:\n");
	mpz_out_str(stderr, 16, pla_cert->private_key);fprintf(stderr, "\n\n");

	fprintf(stream, "Rid:\t");
	if (pla_cert->rid == NULL) {
		fprintf(stream, "NULL");
	} else {
		libpla_io_hexdump(stream, (unsigned char *)pla_cert->rid, RID_LEN);	
	}
	fprintf(stream, "\n\n");

	fprintf(stream, "TTP certificate information\n");
	fprintf(stream, "\tIdentity: %d\n", pla_cert->pla_header->ttp_identity);
	fprintf(stream, "\tValidity: \"%s\" - \"%s\"\n", nb, na);
	fprintf(stream, "\tRights: %d\n", pla_cert->pla_header->ttp_rights);
	fprintf(stream, "\tDelegatable rights: %d\n", pla_cert->pla_header->ttp_deleg_rights);
	fprintf(stream, "\n");

	fprintf(stream, "Current sequence number: %lu\n", pla_cert->pla_header->sequence_number);
	fprintf(stream, "\n");
}


/**
 * Returns user's implicit certificate (which is used for calculating public key)
 *
 * FIXME: now this returns only default certificate....
 */
void
libpla_get_subject_pk(uint8_t **pk_ret, uint32_t *len_ret) 
{

	//*pk_ret = (uint8_t *)pla_hdr_base->implicit_certificate;
	*pk_ret = (uint8_t *)default_certificate->pla_header->implicit_certificate;
	*len_ret = PLA_PUBKEY_LEN;
	//*pk_ret = (uint8_t*)subject.rho; 
	//*len_ret = sizeof(subject.rho);
}
