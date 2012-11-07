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
#include "libpla_io.h"


void 
libpla_io_read_node_state_stream_s(FILE *file, struct key_info *issuer, 
    struct key_info *subject, struct cert_info *ci) 
{
    // TODO: handle errors in libpla_io_read_node_state_stream somehow (invalid input)

	char tmp[2000];
	char *ptr = tmp;
	char *cur;	

	memset(tmp, 0, sizeof(tmp));
	fread(tmp, sizeof(tmp), 1, file);

	strsep(&ptr, "\"");  /* Skip headers */

	cur = strsep(&ptr, "\"");

	/* read x-coordinate and a compession bit of issuer e_y */
	cur = libpla_io_string_hex_to_elem(cur, &issuer->e_y.x) + 1;

	int tmpb;
	sscanf(cur, "%d\n", &tmpb);
	issuer->e_y_b8 = tmpb;
	cur += 2;

	/* read x-coordinate and a compession bit of issuer rho */
	cur = libpla_io_string_hex_to_elem(cur, &issuer->rho) + 1;

	sscanf(cur, "%d\n", &tmpb);
	issuer->b8 = tmpb;
	cur += 2;
	
	/* decompress point & initialize z-coordinate of e_y */
	ec_decompress_point(&issuer->e_y, issuer->e_y.x, issuer->e_y_b8);
	libpla_io_string_hex_to_elem(INITIALZ, &issuer->e_y.z);

/*
	fprintf(stderr, "Issuer's point after decompression:\n");
	fprintf(stderr, "e_y.x: "); elem_print_hex(issuer->e_y.x);
	fprintf(stderr, "e_y.y: "); elem_print_hex(issuer->e_y.y);
	fprintf(stderr, "e_y.z: "); elem_print_hex(issuer->e_y.z);
	fprintf(stderr, "\n");
*/

	cur = strsep(&ptr, "\"");  /* Locator */
	cur = strsep(&ptr, "\"");  /* Locator value */

	/* Subject starts */
	cur = strsep(&ptr, "\"");  /* Pubkey */
	cur = strsep(&ptr, "\"");  /* Pubkey value */


	/* same as above for subject */
	cur = libpla_io_string_hex_to_elem(cur, &subject->e_y.x) + 1;

	sscanf(cur, "%d\n", &tmpb);
	subject->e_y_b8 = tmpb;
	cur += 2;

	cur = libpla_io_string_hex_to_elem(cur, &subject->rho) + 1;
	sscanf(cur, "%d\n", &tmpb);
	subject->b8 = tmpb;
	cur += 2;

	/* decompress point & initialize z-coordinate of e_y */
        ec_decompress_point(&subject->e_y, subject->e_y.x, subject->e_y_b8);
        libpla_io_string_hex_to_elem(INITIALZ, &subject->e_y.z);

/*
        fprintf(stderr, "Subject's point after decompression:\n");
        fprintf(stderr, "e_y.x: "); elem_print_hex(subject->e_y.x);
        fprintf(stderr, "e_y.y: "); elem_print_hex(subject->e_y.y);
        fprintf(stderr, "e_y.z: "); elem_print_hex(subject->e_y.z);
        fprintf(stderr, "\n");
*/

	/* Rid, identity, deleg, time, signature starts */
	cur = strsep(&ptr, "\"");
	cur = strsep(&ptr, "\"");

    if (strlen(cur) < RID_LEN) { /* Rid is empty */
        ci->rid = NULL;
    } else {
        ci->rid = (pursuit_rid_t*) calloc(sizeof(uint8_t), RID_LEN);
        libpla_io_string_hexread(cur, (unsigned char *)ci->rid, RID_LEN);
    }

    uint32_t tmpi;
    cur = strsep(&ptr, "\""); /* Identity */
	cur = strsep(&ptr, "\"");  
	sscanf(cur, "%d\n", &tmpi);
	ci->identity = tmpi;

	cur = strsep(&ptr, "\"");  
	struct tm tm_nb;
	bzero(&tm_nb, sizeof(struct tm));

	if (!strptime(ptr, "%Y-%m-%d %H:%M:%S", &tm_nb)) {
		fprintf(stderr, "Invalid time for not-before: %s\n", ptr);
		exit(EXIT_FAILURE);
	}
	ci->not_before = mktime(&tm_nb);
	

	cur = strsep(&ptr, "\"");  
	cur = strsep(&ptr, "\"");  

	struct tm tm_na;
	bzero(&tm_na, sizeof(struct tm));
	if (!strptime(ptr, "%Y-%m-%d %H:%M:%S", &tm_na)) {
		fprintf(stderr, "Invalid time for not-after: %s\n", ptr);
		exit(EXIT_FAILURE);
	}
	ci->not_after = mktime(&tm_na);

	cur = strsep(&ptr, "\"");  

	cur = strsep(&ptr, "\"");  /* rights */
	cur = strsep(&ptr, "\"");  /* rights value */
	{
		uint8_t tmp_rights;
		sscanf(cur, "%d", (int *)&tmp_rights);
		ci->rights = tmp_rights;
	}

	cur = strsep(&ptr, "\"");  /* deleg */
	cur = strsep(&ptr, "\"");  /* deleg value */
	{
		uint8_t tmp_deleg;
		sscanf(cur, "%d", (int *)&tmp_deleg);
		ci->deleg = tmp_deleg;
	}

	strsep(&ptr, "\"");
	cur = strsep(&ptr, "\"");
	mpz_set_str(ci->sigma, cur, 16);

	//cur = strsep(&ptr, "\"");  /* ttp priv key */
	//cur = strsep(&ptr, "\"");  /* ttp priv key value */
	//mpz_set_str(ci->ttp_sigma, cur, 16);

	cur = strsep(&ptr, "\"");  /* signature value */
	cur = strsep(&ptr, "\"");  /* signature value */

	char *tmpcur = cur;
	tmpcur = libpla_io_string_hexread(tmpcur, ci->r, PLA_HASH_LENGTH);
	mpz_set_str(ci->s, tmpcur, 16);

	return;
}

void 
libpla_io_write_node_state_s(FILE *file, struct key_info *issuer, 
    struct key_info *subject, struct cert_info *ci)
{
	fprintf(file, "(cert \n");

	fprintf(file, " (issuer\n");
	fprintf(file, "  (pub-key\n");
	libpla_io_fprint_public_key(file, issuer);
	fprintf(file, "   locator ");
	fprintf(file, "\"NULL\"");
	fprintf(file, "\n");
	fprintf(file, "  )\n");
	fprintf(file, " )\n");

	fprintf(file, " (subject\n");
	fprintf(file, "  (pub-key\n");
	libpla_io_fprint_public_key(file, subject);
	fprintf(file, "  )\n");
	fprintf(file, " )\n");

	char nb[50];
	char na[50];
	struct tm tm_nb;
	struct tm tm_na;

	localtime_r(&ci->not_before, &tm_nb);
	localtime_r(&ci->not_after, &tm_na);

	strftime(nb, sizeof(nb), "%Y-%m-%d %H:%M:%S", &tm_nb);
	strftime(na, sizeof(na), "%Y-%m-%d %H:%M:%S", &tm_na);

    fprintf(file, " (rid \""); /* Rendezvous id field */
    if (ci->rid == NULL) {
        fprintf(file, "NULL");
    } else {
        libpla_io_hexdump(file, (unsigned char *)ci->rid, RID_LEN);	
    }
    fprintf(file, "\")\n");

	fprintf(file, " (identity \"%u\")\n", ci->identity);
	fprintf(file, " (valid not-before \"%s\" not-after \"%s\")\n", nb, na);
	fprintf(file, " (rights \"%d\")\n", ci->rights);
	fprintf(file, " (deleg \"%d\")\n", ci->deleg);

	//fprintf(file, " (rights \"%x\")\n", ci->rights);
	//fprintf(file, " (deleg \"%x\")\n", ci->deleg);

	fprintf(file, " (private-key \"");
	libpla_io_fprint_private_key(file, ci);
	fprintf(file, "\")\n");

/*
	fprintf(file, " (ttp-private-key \"");
	{
		char *str = mpz_get_str(NULL, 16, ci->ttp_sigma);
		if(!str) {
			fprintf(stderr, "Out of memory on ttp_sigma print\n");
			exit(EXIT_FAILURE);
		}
		fprintf(file, "%s", str);
		free(str);
	}
	fprintf(file, "\")\n");
*/

	fprintf(file, " (signature\n\"");
	libpla_io_hexdump(file, ci->r, PLA_HASH_LENGTH);	
	fprintf(file, "\n");
	mpz_out_str(file, 16, ci->s);
	fprintf(file, "\"\n )\n");

	fprintf(file, ")\n");

	return;
}

char* 
libpla_io_string_hex_to_elem(char *source, elem_t *dst)
{
	char *cur = source;
	uint32_t *ptr = (uint32_t*) &dst[0];

	for (int i=0; i < NUMWORDS; i++) {
		if (sscanf(cur, "%08x", &ptr[i]) < 0) {
			fprintf(stderr, "Failure in hex_to_elem\n");
			exit(EXIT_FAILURE);
		}
		cur += 8+1;   /* Each field is 8+1 bytes long */
	}
	return cur;
}

char* 
libpla_io_string_hexread(char *source, unsigned char *target, unsigned int len)
{
	//unsigned char *orig = target;
	unsigned remaining = len;

	while (remaining--) {
		sscanf(source, "%02x", (int *)target);
		target += 1;
		source += 2;
	}

	return source;
}

void 
libpla_io_hexdump(FILE *file, unsigned char *buf, unsigned int len)
{
        while (len--)
                fprintf(file, "%02x", *buf++);
}

void 
libpla_io_fprint_private_key(FILE *file, struct cert_info *node)
{
	char *str = mpz_get_str(NULL, 16, node->sigma);
	if(!str) {
		fprintf(stderr, "Out of memory on fprint_private_key\n");
		exit(EXIT_FAILURE);
	}
	fprintf(file, "%s", str);
	free(str);

	return;
}

void 
libpla_io_fprint_public_key(FILE *file, struct key_info *node)
{
	char tmp[1000];
	char *fp = tmp;

	fp += libpla_io_ecpoint_t_sprintf(fp, &node->e_y);
	fp += sprintf(fp, "%d\n", node->e_y_b8);
	fp += libpla_io_elem_to_hex(fp, node->rho);
	fp += sprintf(fp, "%d", node->b8);

	fprintf(file, "\"%s\"\n", tmp);

	return;
}

int 
libpla_io_elem_to_hex(char *dst, elem_t src)
{
	int i;
	char *tmp_ptr = dst;

	for (i=0; i < NUMWORDS; i++)
		tmp_ptr += sprintf(tmp_ptr, "%08x ",src[i]);

	tmp_ptr += sprintf(tmp_ptr, "\n");

	return tmp_ptr - dst;
}

int 
libpla_io_ecpoint_t_sprintf(char *cp, ecpoint_t *point)
{
	char *cp_orig = cp;

	cp += libpla_io_elem_to_hex(cp, point->x);

	/* DL: no need to store y,z-coordinates in the file... */
	//cp += jlu_elem_to_hex(cp, point->y);
	//cp += jlu_elem_to_hex(cp, point->z);

	return cp - cp_orig;
}

void 
libpla_io_key_info_init(struct key_info *ki)
{
	memset(ki, 0, sizeof(struct key_info));
//	mpz_init(ki->sigma);

	return;
}

void 
libpla_io_key_info_uninit(struct key_info *ki)
{
//	mpz_clear(ki->sigma);

	return;
}

void 
libpla_io_cert_info_init(struct cert_info *si)
{
	memset(si, 0, sizeof(struct cert_info));
	mpz_init(si->s);

	mpz_init(si->sigma);
	//mpz_init(si->ttp_sigma);

	return;
}

void 
libpla_io_cert_info_uninit(struct cert_info *si)
{
	//mpz_clear(si->ttp_sigma);
	mpz_clear(si->sigma);

	mpz_clear(si->s);

	return;
}


/**
 * Converts public key information to binary format which can included in the header
 *
 * @param	target    has length of PLA_PUBKEY_LEN (168 bits)
 * @param	e         key
 * @param	b         compression bit
 *
 */
void
libpla_io_public_key_to_bin(char *target, elem_t e, uint8_t b)
{
    // FIXME: should we return something here?
    int i, tmp_int;

    for (i=0; i < NUMWORDS-1; i++)
        memcpy(target+sizeof(uint32_t)*i, &e[i], sizeof(uint32_t));

    /* Get a byte from last elemenet */
    tmp_int = e[NUMWORDS-1];
    tmp_int = tmp_int >> 24; /* Ignore first 3*8 = 24 bits */

    /* Add a compression bit to it */
    tmp_int += b;

    memcpy(target+sizeof(uint32_t)*(NUMWORDS-1), &tmp_int, sizeof(char));
}


/**
 * Extracts key information from binary format
 *
 * @param	src   key in binary format
 * @param	e     key info
 * @param	b     compression bit
 *
 */
void
libpla_io_bin_to_public_key(char *src, elem_t e, uint8_t *b)
{
    int i, tmp_int;

    for (i=0; i < NUMWORDS-1; i++)
        memcpy(&e[i], src+sizeof(uint32_t)*i, sizeof(uint32_t));


    /* Get a byte from the last element */
    memcpy(&tmp_int, src+sizeof(uint32_t)*i, sizeof(int));
    *b = tmp_int & 1; /* Set the compression bit */

    /* Restore the original element */
    tmp_int -= *b;
    tmp_int = tmp_int << 24;

    memcpy(&e[i], &tmp_int, sizeof(int));
}
