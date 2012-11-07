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
#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

#include <net/if.h>
#include <openssl/ripemd.h>

#include "libpla_configs.h"

#include "ec.h"
#include "ec_io.h"
#include "libpla_io.h"

#define PLA_TTP_CONF_FILE "/etc/pla.conf"

FILE *output_file = NULL;
FILE *certificate_file = NULL;
FILE *key_file = NULL;

/*
static int shexdump(char *target, unsigned char *buf, unsigned int len)
{
	char *orig = target;

        while (len--) {
                target += sprintf(target, "%02x", *buf++);
	}

	target += sprintf(target, "\n");

	return target - orig;
}
*/

void id_create(unsigned char *id_hash)
{
	int i;

	/* jlu XXX: Yes, this does it in the dumbest possible way. */
	srandom(time(NULL));
	for (i=0; i<PLA_HASH_LENGTH; i++)
		id_hash[i] = random();

	return;
}

#if 0
void create_key(elem_t *rho, mpz_t *sigma, uint8_t *b8, unsigned char *client_id_hash, mpz_t *ttp_private)
{
	/* Create new key for the client */
	mpz_t k;
	mpz_t rhoi;
	ecpoint_t kgd;
	
	mpz_init(k);
	ec_keygen(&kgd,k);
	
	int b;
	
	ec_compress_point(&b,&kgd);
	*b8 = b;
	elem_t hid;
	elem_import_bytes(hid, (unsigned char *)client_id_hash);
	elem_chomp(hid); /* hash of the ID */
	gnb_add(*rho,hid,kgd.x);
	
	mpz_init(rhoi);
	mpz_init(*sigma);
	elem_to_int(rhoi,*rho);
	mpz_fdiv_r(rhoi,rhoi,e.r);
	mpz_mul(*sigma,rhoi,*ttp_private);
	mpz_add(*sigma,k,*sigma);
	mpz_fdiv_r(*sigma,*sigma,e.r);
	
	return;
}
#endif

int key_info_to_bin(char *target, struct key_info *info)
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
static int cert_to_bin(unsigned char *target, int target_len, struct key_info *issuer, struct key_info *subject, struct cert_info *ci)
{
	unsigned char *cur = target;	
	uint32_t *tmp32;
	uint8_t *tmp8;

	memset(target, 0, target_len);
	cur += key_info_to_bin((char *)cur, issuer);
	cur += key_info_to_bin((char *)cur, subject);

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

void cert_to_digest(unsigned char *digest, struct key_info *issuer, struct key_info *subject, struct cert_info *ci)
{
	unsigned char bin_cert[1000];
	int len = cert_to_bin(bin_cert, sizeof(bin_cert), issuer, subject, ci);

	RIPEMD160(bin_cert, len, digest);

//fprintf(stderr, "cert hash: ");
//libpla_io_hexdump(stderr, digest, PLA_HASH_LENGTH); fprintf(stderr, "\n");

	return;
}


/**
 * Calculates identity hash based on the TTP-certificate fields 
 * hash = H(identity + rights + deleg. rights + validity time)
 * FIXME: maybe htonl/htons need to be added at the later stage
 * FIXME: change the order of the fields, if the structure of the PLA header changes
 *
 * @param	hash  hash string
 * @param	ci    cert_info struct containing certificate information
 */
void calculate_id_hash(unsigned char *hash, struct cert_info *ci) 
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

/*
int set_interfaces(char **argv, int argc, int optind)
{
	int if_in, if_out;
	char* policystr;
	int policynum;

	if_in = if_nametoindex(argv[optind]);
	if (if_in == 0) {
		fprintf(stderr, "No such interface: %s\n", argv[optind]);
		exit(EXIT_FAILURE);
	}
	optind++;

	if_out = if_nametoindex(argv[optind]);
	if (if_out == 0) {
		fprintf(stderr, "No such interface: %s\n", argv[optind]);
		exit(EXIT_FAILURE);
	}
	optind++;

	policystr = argv[optind];
	if (strcmp(policystr, "drop") == 0)
		policynum = PLA_FORWARD_POLICY_DROP;
	else if (strcmp(policystr, "add") == 0)
		policynum = PLA_FORWARD_POLICY_SIGN;
	else if (strcmp(policystr, "verify") == 0)
		policynum = PLA_FORWARD_POLICY_VERIFY;
	else if (strcmp(policystr, "strip") == 0)
		policynum = PLA_FORWARD_POLICY_STRIP;
	else {
		fprintf(stderr, "Invalid policy string: %s\n",  policystr);
		exit(EXIT_FAILURE);
	}

	if (if_out == 0 && strcmp(argv[optind], "local") != 0) {
		fprintf(stderr, "No such interface: %s\n", argv[optind]);
		exit(EXIT_FAILURE);
	}
	optind++;

	fprintf(stderr, "Setting policy: %d\n", policynum);

	struct pla_netlink_set_interface if_set = {
		.if_in  = if_in,
		.if_out = if_out,
		.policy = policynum,
		};

	
	fprintf(stderr, "Indexes: %d %d\n", if_set.if_in, if_set.if_out);

	//send_netlink(&if_set, sizeof(if_set), NLMSG_PLA_SET_INTERFACES );

	return 0;
}
*/

void set_time_in_ci(struct cert_info *ci, char *not_before, char *not_after)
{
	struct tm tm_na;  /* Not after */
	struct tm tm_nb;  /* Not before */

	bzero(&tm_na, sizeof(struct tm));
	bzero(&tm_nb, sizeof(struct tm));

	if (!strptime(not_before, "%Y-%m-%d %H:%M:%S", &tm_nb)) {
		fprintf(stderr, "Invalid time for not-before: %s\n", not_before);
		exit(EXIT_FAILURE);
	}

	if (!strptime(not_after, "%Y-%m-%d %H:%M:%S", &tm_na)) {
		fprintf(stderr, "Invalid time for not-before: %s\n", not_after);
		exit(EXIT_FAILURE);
	}

	ci->not_before = mktime(&tm_nb);
	ci->not_after = mktime(&tm_na);

	return;
}


int ttp_create(char **args, int argc, int optind)
{
	struct key_info issuer;
	struct key_info subject;
	struct cert_info ci;

	libpla_io_key_info_init(&issuer);
	libpla_io_key_info_init(&subject);
	libpla_io_cert_info_init(&ci);

	unsigned char digest[PLA_HASH_LENGTH + 1]; // Workaround for EC bug.
	digest[PLA_HASH_LENGTH] = 0;      	// Workaround for EC bug.

	unsigned char id_hash[PLA_HASH_LENGTH + 1];
	id_hash[PLA_HASH_LENGTH] = 0;

	/* Create a new TTP */
	mpz_t tmp_sigma; mpz_init(tmp_sigma);

	ec_keygen(&(e.y), tmp_sigma); /* e.y is domain public key of TTP */
	memcpy(&issuer.e_y, &e.y, sizeof(issuer.e_y));

	/* Compress point to save space */
	ec_compress_point((int *)&issuer.e_y_b8, &issuer.e_y);

	/* Set other fields of the TTP certificate */
	srandom(time(NULL));
	ci.identity = random();

	ci.deleg = 255;
	ci.rights = 255;

	set_time_in_ci(&ci, args[optind], args[optind+1]);
	optind += 2;

	/* Create a private key for the TTP */
	calculate_id_hash(id_hash, &ci);
	ec_create_key(&subject.rho, &subject.b8, &ci.sigma, id_hash, &tmp_sigma);

	/* Sign the new certificate */
	ecselfsig_t sig;

	/* Real public key is derived from the implicit certificate and identity hash
	   thus we can actually sign some dummy data with a new private key */
	cert_to_digest(digest, &issuer, &subject, &ci);
	ec_sign_self(&sig, digest, ci.sigma);

	/* Copy signature to cert_info struct */
	mpz_set(ci.s, sig.s); 
	memcpy(ci.r, sig.r, PLA_HASH_LENGTH);


#if 0
// verify this certificate
sig.b = subject.b8;
memcpy(&sig.rho, &subject.rho, sizeof(elem_t));
fprintf(stderr, "Verify: %d\n", ec_verify_self(&sig, digest, id_hash));
#endif

	/* Write certificate information into a file */
	libpla_io_write_node_state_s(output_file, &issuer, &subject, &ci);

	libpla_io_key_info_uninit(&subject);
	libpla_io_key_info_uninit(&issuer);
	libpla_io_cert_info_uninit(&ci);


	return 0;
}


int cert_verify(void)
{
	int ret = -1;

	struct key_info issuer;
	struct key_info subject;
	struct cert_info si;

	libpla_io_key_info_init(&issuer);
	libpla_io_key_info_init(&subject);
	libpla_io_cert_info_init(&si);

	unsigned char digest[PLA_HASH_LENGTH + 1]; // Workaround for EC bug.
	digest[PLA_HASH_LENGTH] = 0;      	// Workaround for EC bug.

	unsigned char id_hash[PLA_HASH_LENGTH+1];
	id_hash[PLA_HASH_LENGTH] = 0;

	if (!certificate_file) {
		fprintf(stderr, "No certificate file name given.\n");
		exit(EXIT_FAILURE);
	}
	libpla_io_read_node_state_stream_s(certificate_file, &issuer, &subject, &si);

	/* Check not_before and not_after */
	if (si.not_before > time(NULL) || si.not_after < time(NULL)) {
		fprintf(stderr, "Failed verify: Time.\n");
		exit(2);
	}
	
	/* Verify it. */
	ecselfsig_t sig;

	memcpy(&e.y, &issuer.e_y, sizeof(e.y));
	
	memcpy(&sig.rho, &subject.rho, sizeof(subject.rho));
	sig.b = subject.b8;
	
	mpz_init(sig.s);
	mpz_set(sig.s, si.s); 
	memcpy(sig.r, si.r, PLA_HASH_LENGTH);
	
	cert_to_digest(digest, &issuer, &subject, &si);


	/* Calculate hash over identity fields of the certficate */
	calculate_id_hash(id_hash, &si);

#if 0
fprintf(stderr, "e.y.x: "); elem_print_hex(e.y.x);
fprintf(stderr, "e.y.y: "); elem_print_hex(e.y.y);
fprintf(stderr, "e.y.z: "); elem_print_hex(e.y.z);
libpla_io_hexdump(stderr, (unsigned char*) &sig.rho, sizeof(sig.rho)); fprintf(stderr, " b: %d\n", sig.b);
libpla_io_hexdump(stderr, digest, 20); fprintf(stderr, "\n");
libpla_io_hexdump(stderr, id_hash, 20);fprintf(stderr, "\n");
libpla_io_hexdump(stderr, sig.r, PLA_HASH_LENGTH); fprintf(stderr, "\n");
mpz_out_str(stderr, 16, sig.s);fprintf(stderr, "\n");
#endif
	
	if(ec_verify_self(&sig, (unsigned char *)digest, id_hash) == 1) {
		ret = 1;
		fprintf(stderr, "Successful verify\n");
	} else {
		ret = 2;
		fprintf(stderr, "Failed verify: Signature.\n");
	}

	libpla_io_key_info_uninit(&subject);
	libpla_io_key_info_uninit(&issuer);
	libpla_io_cert_info_uninit(&si);

	return ret;
}


int id_sign(char **args, int argc, int optind)
{
	unsigned char digest[PLA_HASH_LENGTH + 1];
	digest[PLA_HASH_LENGTH] = 0;
	unsigned char id_hash[PLA_HASH_LENGTH + 1];
	id_hash[PLA_HASH_LENGTH] = 0;

	struct cert_info input_cert;
	struct cert_info ci;
	
	struct key_info tmp_null;
	struct key_info subject;
	struct key_info issuer;

	libpla_io_key_info_init(&tmp_null);
	libpla_io_key_info_init(&subject);
	libpla_io_key_info_init(&issuer);
	libpla_io_cert_info_init(&input_cert);
	libpla_io_cert_info_init(&ci);

	mpz_t sig_sigma;
	mpz_init(sig_sigma);
	

	if (!certificate_file) {
		fprintf(stderr, "No certificate file given. Trying " PLA_TTP_CONF_FILE "\n");
		certificate_file = fopen(PLA_TTP_CONF_FILE, "r");
		if (!certificate_file) {
			fprintf(stderr, "Error opening file\n");
			exit(EXIT_FAILURE);
		}
	}

	libpla_io_read_node_state_stream_s(certificate_file, &tmp_null, &issuer, &input_cert);

	/* Generate identity, FIXME: identity should be unique per TTP, Each TTP should 
	   maintain a list of given identities */
    srandom(time(NULL));
    ci.identity = random();

	/* Rights */
	if (optind >= argc) {
		fprintf(stderr, "Rights parameter missing\n");
		exit(EXIT_FAILURE);
	}
	uint32_t tmp_rights = strtol(args[optind], NULL, 10);
	if (errno == ERANGE) {
		fprintf(stderr, "Invalid rights string\n");
		exit(EXIT_FAILURE);
	}
	ci.rights = tmp_rights;
	optind++;

	/* Delegation */
	if (optind >= argc) {
		fprintf(stderr, "Delegation parameter missing\n");
		exit(EXIT_FAILURE);
	}
	ci.deleg = strtol(args[optind], NULL, 10);
	if (errno == ERANGE) {
		fprintf(stderr, "Invalid delegation string\n");
		exit(EXIT_FAILURE);
	}
	optind++;

	/* Not before, not after */
	if (optind + 1 >= argc) {
		fprintf(stderr, "Date parameters missing.\n");
		exit(EXIT_FAILURE);
	}
	set_time_in_ci(&ci, args[optind], args[optind+1]);
	optind += 2;

    if (optind < argc) { /* Rid present, read it */
        //fprintf(stderr, "strlen of param is: %d\n", strlen(args[optind]));
        if (strlen(args[optind]) == 2*RID_LEN) {
            ci.rid = (pursuit_rid_t*) calloc(sizeof(unsigned char), RID_LEN);
            libpla_io_string_hexread(args[optind], (unsigned char *)ci.rid, RID_LEN);
        } else {
            fprintf(stderr, "Invalid Rid length, %d bytes (%d characters) expected\n",
                RID_LEN, 2*RID_LEN);
        }
    }

	/* Calculate identity hash from newly created TTP-certificate fields */
	calculate_id_hash(id_hash, &ci);

	/* Create a new key for the node */
	ec_create_key(&subject.rho, &subject.b8, &ci.sigma, id_hash, &input_cert.sigma);

	/* Calculate a real TTP's public key from it's private key */
	ec_multiply(&issuer.e_y, &(e.g), input_cert.sigma);
	ec_compress_point((int *)&issuer.e_y_b8, &issuer.e_y);

	/* Empty TTP's implicit certificate, it is not really needed */
	memset(issuer.rho, 0, sizeof(issuer.rho));
	issuer.b8 = 0;

	/* Calculate public key from the private key, this is not really needed */
	//ec_keygen(&subject.e_y, ci.sigma);
	//ec_compress_point((int *)&subject.e_y_b8, &subject.e_y);


	/* Sign the new certificate */
	ecselfsig_t sig;

	cert_to_digest(digest, &issuer, &subject, &ci);
	ec_sign_self(&sig, digest, ci.sigma);
	
	/* Copy the signature to the ci */
	mpz_set(ci.s, sig.s); 
	memcpy(ci.r, sig.r, PLA_HASH_LENGTH);

#if 0
// test
memcpy(&e.y, &issuer.e_y, sizeof(e.y));
memcpy(&sig.rho, &subject.rho, sizeof(subject.rho));
sig.b = subject.b8;

libpla_io_hexdump(stderr, (unsigned char*) &sig.rho, sizeof(sig.rho)); fprintf(stderr, " b: %d\n", sig.b);
fprintf(stderr, "e.y.x: "); elem_print_hex(e.y.x);
fprintf(stderr, "e.y.y: "); elem_print_hex(e.y.y);
fprintf(stderr, "e.y.z: "); elem_print_hex(e.y.z);
libpla_io_hexdump(stderr, digest, 20); fprintf(stderr, "\n");
libpla_io_hexdump(stderr, id_hash, 20);fprintf(stderr, "\n");
libpla_io_hexdump(stderr, sig.r, PLA_HASH_LENGTH); fprintf(stderr, "\n");
mpz_out_str(stderr, 16, sig.s);fprintf(stderr, "\n");

fprintf(stderr, "Verify returned: %d\n", ec_verify_self(&sig, digest, id_hash));
#endif

	libpla_io_write_node_state_s(output_file, &issuer, &subject, &ci);

	libpla_io_cert_info_uninit(&ci);
	libpla_io_cert_info_uninit(&input_cert);
	libpla_io_key_info_uninit(&issuer);
	libpla_io_key_info_uninit(&subject);		
	libpla_io_key_info_uninit(&tmp_null);
	
	return 0;
}


void kernelconfig(char **args, int argc, int optind)
{
	if (optind >= argc) {
		fprintf(stderr, "Insufficient parameters for command: kernelconfig.\n");
		exit(EXIT_FAILURE);
	}

	if (strcmp(args[optind], "help") == 0) {
		optind++;
		goto out_err;
	}

	if (strcmp(args[optind], "cryptotarget") == 0) {
		optind++;

		//set_userspace_target(args[optind]);

		return;
	}

	if (strcmp(args[optind], "setinterface") == 0) {
		optind++;

		//set_interfaces(args, argc, optind);
		return;
	}

	if (strcmp(args[optind], "addcert") == 0) {
		optind++;

		//add_cert(args, argc, optind);
		return;
	}

	if (strcmp(args[optind], "configid") == 0) {
		optind++;

		struct key_info issuer, subject;
		struct cert_info ci;

		libpla_io_key_info_init(&issuer);
		libpla_io_key_info_init(&subject);
		libpla_io_cert_info_init(&ci);

		if (!certificate_file) {
			fprintf(stderr, "No certificate file set\n");
			exit(EXIT_FAILURE);
		}

		libpla_io_read_node_state_stream_s(certificate_file, &issuer, &subject, &ci);
		
		if (mpz_cmp_ui(ci.sigma, 0) == 0) {
			fprintf(stderr, "Error: Trying to configure kernel id without a private key.\n");
			exit(EXIT_FAILURE);
		}
		
#if 1
		fprintf(stderr, "Configuring rho: ");
		libpla_io_hexdump(stderr, (unsigned char*) &subject.rho, sizeof(subject.rho));
		fprintf(stderr, "\n");
#endif

		//send_id_netlink(&issuer.e_y, &ci.ttp_sigma, &subject.rho, &subject.b8, subject.id_hash);
		
		libpla_io_cert_info_uninit(&ci);
		libpla_io_key_info_uninit(&subject);
		libpla_io_key_info_uninit(&issuer);

		return;
	}

	fprintf(stderr, "Unknown command after kernelconfig\n");
	goto out_err;

	return;

out_err:
	fprintf(stdout, "\nUsage: placonf [OPTIONS] kernelconfig {cryptotarget|setinterface|configid|help}\n"); 
	exit(EXIT_FAILURE);

	return;
}

int id(char **args, int argc, int optind)
{
	if (optind >= argc) {
		fprintf(stderr, "Insufficient parameters for command: id.\n");
		goto out_err;
	}

	if (strcmp(args[optind], "sign") == 0) {
		optind++;
		return id_sign(args, argc, optind);
	}

	if (strcmp(args[optind], "create") == 0) {
		optind++;

		struct key_info ki;
		struct cert_info ci;

		libpla_io_key_info_init(&ki);
		libpla_io_cert_info_init(&ci);

		//id_create(ki.id_hash);

		libpla_io_write_node_state_s(output_file, &ki, &ki, &ci);

		libpla_io_cert_info_uninit(&ci);
		libpla_io_key_info_uninit(&ki);
		return 0;
	}

	fprintf(stderr, "Unknown command after id\n");

out_err:
	fprintf(stdout, "\nUsage: placonf [OPTIONS] id {create|sign}\n"); 

	return -1;
}


int cert(char **args, int argc, int optind)
{
	if (optind >= argc) {
		fprintf(stderr, "Insufficient parameters for command: cert.\n");
		goto out_err;
	}

	if (strcmp(args[optind], "verify") == 0) {
		optind++;
		return cert_verify();
	}

#if 0
	if (strcmp(args[optind], "chain") == 0) {
		optind++;
		chain(args, argc, optind);
		return;
	}
#endif

	fprintf(stderr, "Unknown command after cert.\n");
	goto out_err;

	return 0;

out_err:
	fprintf(stdout, "\nUsage: placonf [OPTIONS] cert verify\n"); 

	return -1;
}

int ttp(char **args, int argc, int optind)
{
	if (optind >= argc) {
		fprintf(stderr, "Insufficient parameters for command: ttp.\n");
		goto out_err;
	}

	if (strcmp(args[optind], "create") == 0) {
		optind++;
		return ttp_create(args, argc, optind);
	}

	fprintf(stderr, "Unknown command after ttp\n");
	goto out_err;

out_err:
	fprintf(stdout, "\nUsage: placonf [OPTIONS] ttp create\n"); 

	return -1;
}

int main(int argc, char **argv) 
{
	int ret = -1;

	int c;
	int use_hw = 0;

	output_file = stdout;

	ec_init();

	while (1) {
		c = getopt_long (argc, argv, "o:C:K:H", NULL, NULL);
		if (c == -1)
			break;
		
		switch (c) {
		case 'H':
			use_hw = 1;
			break;

		case 'o':
			output_file = fopen(optarg, "w");
			if (!output_file) {
				perror("fopen");
				exit(EXIT_FAILURE);
			}
			break;

		case 'K':
			key_file = fopen(optarg, "r");
			if (!key_file) {
				perror("fopen");
				exit(EXIT_FAILURE);
			}
			break;

		case 'C':
			certificate_file = fopen(optarg, "r");
			if (!certificate_file) {
				perror("fopen");
				exit(EXIT_FAILURE);
			}
			break;

		case '?':
			exit(EXIT_FAILURE);
			
		default:
			fprintf (stderr, "?? getopt returned character code 0%o ??\n", c);
			exit(EXIT_FAILURE);

		}
	}
	
	if (use_hw) 
		ec_open();

	if (optind < argc) {

		if (strcmp(argv[optind], "help") == 0) {
			fprintf(stdout, "Usage: placonf [OPTIONS] COMMAND\n");
			fprintf(stdout, "Options:\n");
			fprintf(stdout, "         -o output_file\n");
			fprintf(stdout, "         -C certificate_file\n");
			fprintf(stdout, "         -K key_file\n");
			fprintf(stdout, "         -H (Use crypto accelerator)\n");
			fprintf(stdout, "Commands:\n");
			fprintf(stdout, "         help | cert | ttp | id | kernelconfig\n");
			goto out;
		}

		if (strcmp(argv[optind], "cert") == 0) {
			++optind;
			cert(argv, argc, optind);
			goto out;
		}

		if (strcmp(argv[optind], "ttp") == 0) {
			++optind;
			ret = ttp(argv, argc, optind);
			goto out;
		}

		if (strcmp(argv[optind], "id") == 0) {
			++optind;
			ret = id(argv, argc, optind);
			goto out;
		}

		if (strcmp(argv[optind], "kernelconfig") == 0) {
			++optind;
			kernelconfig(argv, argc, optind);
			ret = 0;
			goto out;
		}

		fprintf(stderr, "Unknown command: %s\n", argv[optind]);
		exit(EXIT_FAILURE);
	}

out:
	fclose(output_file);  /* This always exists */

	if (key_file)
		fclose(key_file);

	if (certificate_file)
		fclose(certificate_file);

	ec_close();

	return ret;
}
