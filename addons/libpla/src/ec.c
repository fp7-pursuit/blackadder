/*
* Copyright (c) 2008, Billy Brumley, TKK, <billy.brumley@tkk.fi>,
* Dmitrij Lagutin, HIIT, <dmitrij.lagutin@hiit.fi>
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
#include "ec.h"
#include "ec_hw.h"

static int hw_sock = -1;

ecurve_t e;
ecpoint_t infinity;
ecpoint_t infinity_ld;
gmp_randstate_t texas;

int ec_open(void)
{
	if (ec_hw_open(&hw_sock) == HW_ERROR) {
		fprintf(stderr, "Unable to connect to hardware\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

void ec_close(void)
{
	if (hw_sock >= 0)
		ec_hw_close(hw_sock);

	return;
}

/**
 * initialization for calculating the tau-NAF.
 * sets the order e.r and the variables e.s0, e.s1, e.Vm
 */
void tau_init() {
  int i;

  mpz_t *U;
  mpz_t *V;
  U = (__mpz_struct (*)[1]) malloc(sizeof(mpz_t) * (DEGREE+4));
  V = (__mpz_struct (*)[1])malloc(sizeof(mpz_t) * (DEGREE+4));
  
  for(i=DEGREE+4; i--;) {
    mpz_init(U[i]);
    mpz_init(V[i]);
  }

  mpz_set_ui(U[0],0);
  mpz_set_ui(U[1],1);

  /* v_0 = 2, v_1 = mu (1 here) */
  mpz_set_ui(V[0],2);
  mpz_set_ui(V[1],1);

  for(i=1; i<DEGREE+3; i++) {
    mpz_mul_2exp(U[i+1],U[i-1],1);
    mpz_sub(U[i+1],U[i],U[i+1]);

    mpz_mul_2exp(V[i+1],V[i-1],1);
    mpz_sub(V[i+1],V[i],V[i+1]);
  }

  /* s0 = 1/2 * (1 - U_115) */
  mpz_init_set_ui(e.s0,1);
  mpz_sub(e.s0,e.s0,U[DEGREE+3-1-0]);
  mpz_fdiv_q_2exp(e.s0,e.s0,1);

  /* s1 = -1/2 * (1 - U_114) */
  mpz_init_set_ui(e.s1,1);
  mpz_sub(e.s1,e.s1,U[DEGREE+3-1-1]);
  mpz_fdiv_q_2exp(e.s1,e.s1,1);
  mpz_neg(e.s1,e.s1);

//  printf("size: %d s0: %s\n",mpz_sizeinbase(e.s0,2),mpz_get_str(NULL,10,e.s0));
//  printf("size: %d s1: %s\n",mpz_sizeinbase(e.s1,2),mpz_get_str(NULL,10,e.s1));

  mpz_init_set_ui(e.r,1);
  mpz_mul_2exp(e.r,e.r,DEGREE);
  mpz_add_ui(e.r,e.r,1);
  mpz_sub(e.r,e.r,V[DEGREE]);
  mpz_fdiv_q_2exp(e.r,e.r,1);
//  printf("size: %d order: %s\n",mpz_sizeinbase(e.r,2),mpz_get_str(NULL,10,e.r));

  mpz_init_set(e.Vm,V[DEGREE]);
//  printf("size: %d vm: %s\n",mpz_sizeinbase(e.Vm,2),mpz_get_str(NULL,10,e.Vm));

  /* free it all */
  for(i=DEGREE+4; i--; ) {
    mpz_clear(U[i]);
    mpz_clear(V[i]);
  }
  free((void *)U);
  free((void *)V);

}

/**
 * a simple rounding function. rounds towards largest decimal portion.
 * Solinas, section 5
 *
 * @param a      big integer to hold the rounded value
 * @param lambda big float to be rounded
 */
void tau_round1(mpz_t a, mpf_t lambda) {
  mpf_t ret;
  mpf_init2(ret,256);
  mpf_set_d(ret,0.5);
  mpf_add(ret,ret,lambda);
  mpf_floor(ret,ret);
  mpz_set_f(a,ret);
  mpf_clear(ret);
}

/**
 * rounding in Z[tau].
 * Solinas p 149 routine 60
 *
 * @param q0 big integer to hold the rounded lambda0
 * @param q1 big integer to hold the rounded lambda1
 * @param q0 big float to be rounded
 * @param q1 big float to be rounded
 */
void tau_round(mpz_t q0, mpz_t q1, mpf_t lambda0, mpf_t lambda1) {
  mpz_t f0, f1;
  double eta, eta0, eta1;

  mpz_init(f0);
  mpz_init(f1);

  /* line 1-2 */
  tau_round1(f0,lambda0);
  tau_round1(f1,lambda1);

  /* line 3 */
  mpf_t t0, t1;
  mpf_init2(t0,256);
  mpf_set_z(t0,f0);
  mpf_sub(t0,lambda0,t0);
  eta0 = mpf_get_d(t0);

  /* line 4 */
  mpf_init2(t1,256);
  mpf_set_z(t1,f1);
  mpf_sub(t1,lambda1,t1);
  eta1 = mpf_get_d(t1);

  mpf_clear(t0);
  mpf_clear(t1);

  /* line 5-6 */
  int h0, h1;
  h0 = 0;
  h1 = 0;

  /* line 7 */
  eta = 2.0 * eta0 + eta1; /* mu is 1 here since a is 1 */

  if(eta >= 1.0)
    if(eta0 - 3.0*eta1 < -1.0) h1 = 1; /* mu is 1 */
    else h0 = 1;
  else if(eta0 + 4.0*eta1 >= 2.0) h1 = 1; /* mu is 1 */
  
  if(eta < -1.0)
    if(eta0 - 3.0*eta1 >= 1.0) h1 = -1; /* mu is 1 */
    else h0 = -1.0;
  else if(eta0 + 4.0*eta1 < -2.0) h1 = -1;  /* mu is 1 */
  
  mpz_set_si(q0,h0);
  mpz_add(q0,q0,f0);
  mpz_set_si(q1,h1);
  mpz_add(q1,q1,f1);

  mpz_clear(f0);
  mpz_clear(f1);
}

/**
 * approximate division by r
 * Solinas p 163 Algorithm 6
 *
 * @param lambdap big integer to hold the result
 * @param n       the big integer numerator
 * @param si      big integer (a per curve-parameter)
 */
void tau_approx_div(mpf_t lambdap, mpz_t n, mpz_t si) {
  int K, C;
  mpz_t np, t1, gp, hp, jp, lp;
  mpf_t t1f;
  
  C = 32;
  K = (DEGREE + 5) / 2 + C;

  /* line 1 */
  mpz_init(np);
  mpz_fdiv_q_2exp(np,n,DEGREE-K-2+1); // a = 1

  /* line 2 */
  mpz_init(gp);
  mpz_mul(gp,si,np);

  /* line 3 */
  mpz_init(hp);
  mpz_fdiv_q_2exp(hp,gp,DEGREE);

  /* line 4 */
  mpz_init(jp);
  mpz_mul(jp,e.Vm,hp);

  /* line 5 */
  mpz_init(t1);
  mpz_add(t1,gp,jp);
  mpf_init2(t1f,256);
  mpf_set_z(t1f,t1);
  mpf_div_2exp(t1f,t1f,K-C);
  mpz_init(lp);
  tau_round1(lp,t1f);

  /* line 6 */
  mpf_set_z(lambdap,lp);
  mpf_div_2exp(lambdap,lambdap,C);

  mpz_clear(np);
  mpz_clear(gp);
  mpz_clear(hp);
  mpz_clear(jp);
  mpz_clear(lp);
  mpz_clear(t1);
  mpf_clear(t1f);
}

/**
 * Partial reduction. Outputs integers specifying
 * r_0 + r_1 tau = n mod(tau^m-1)/(tau-1)
 * Solinas p 155 Routine 74
 *
 * @param r0 big integer to hold the result 
 * @param r1 big integer to hold the result
 * @param n  big integer to be reduced
 */
void tau_red_mod(mpz_t r0, mpz_t r1, mpz_t n) {
	mpz_t d0, q0, q1, t1;
	mpz_init(q0);
	mpz_init(q1);
	mpz_init(t1);
	mpz_init_set(d0,e.s0);
	mpz_add(d0,d0,e.s1); // mu is 1
	
	mpf_t lambda0, lambda1, rf;
	mpf_init2(lambda0,256);
	mpf_init2(lambda1,256);
	mpf_init2(rf,256);
	mpf_set_z(rf,e.r);
	mpf_clear(rf);
	
	/* use this if you want to do real division */
	/*
	mpz_mul(t1,s0,n);
	mpf_set_z(lambda0,t1);
	mpf_div(lambda0,lambda0,rf);
	mpz_mul(t1,s1,n);
	mpf_set_z(lambda1,t1);
	mpf_div(lambda1,lambda1,rf);
	*/

	/* use this if you want to do approximate division */
	tau_approx_div(lambda0,n,e.s0);
	tau_approx_div(lambda1,n,e.s1);

	tau_round(q0,q1,lambda0,lambda1);
	mpf_clear(lambda0);
	mpf_clear(lambda1);

	mpz_mul(r0,d0,q0);
	mpz_clear(d0);
	mpz_sub(r0,n,r0);
	mpz_mul(t1,e.s1,q1);
	mpz_mul_2exp(t1,t1,1);
	mpz_sub(r0,r0,t1);

	mpz_mul(r1,e.s1,q0);
	mpz_mul(t1,e.s0,q1);
	mpz_sub(r1,r1,t1);
	mpz_clear(q0);
	mpz_clear(q1);
	mpz_clear(t1);
}



/* elliptic curve stuff */

/**
 * Hex string to field element converter.
 * this is antiquated and needs revision for the new bit ordering.
 * as written, takes a hex string in big endian  b_(m-1), b_(m-2), ..., b_1, b_0
 * to a field element in little endian b_0, b_1, ..., b_(m-1)
 * 
 * SO if we want to be able to import our own hex strings, we should do:
 * take a hex string in little endian b_0, b_1, ..., b_(m-1)
 * to a field element in little endian b_0, b_1, ..., b_(m-1)
 *
 * maybe its not antiquated, just misleading. they both have their uses.
 * if you think of the hex string as an integer, the first is logical.
 * as a field element, the second is logical.
 * maybe another converter elem_from_hex_int? or a boolean parameter to specify..
 *
 * @param a field element to hold the value
 * @param s hex string representation of the field element
 */
void elem_from_hex(elem_t a, char *s) {
  mpz_t z;
  size_t size;
  memset(a,0,sizeof(elem_t));
  mpz_init(z);
  mpz_set_str(z,s,16);
  mpz_export(a,&size,-1,sizeof(ulong_t),-1,0,z);
  elem_reverse_words(a,0,NUMWORDS);
  elem_convert_endian(a);
  mpz_clear(z);
}

/**
 * initialization for elliptic curve domain parameters.
 * sets the base point e.g and infinity. calls the other initializers.
 */
void ec_init() {
  /* initialize PRNGs */
  srand( (unsigned)time( NULL ) );

  elem_t a;
  mpz_t seed;

  mpz_init(seed);
  gmp_randinit(texas,(gmp_randalg_t) 0,128);

  elem_random(a);
  elem_to_int(seed,a);

  gmp_randseed(texas, seed);

  /* set infinity */
  memcpy(infinity.x,gnb_zero,sizeof(elem_t));
  memcpy(infinity.y,gnb_zero,sizeof(elem_t));

  /* set projective infinity */
  memcpy(infinity_ld.x,gnb_one,sizeof(elem_t));
  memcpy(infinity_ld.y,gnb_zero,sizeof(elem_t));
  memcpy(infinity_ld.z,gnb_zero,sizeof(elem_t));

  if(DEGREE==163) {
    char x[] = "0 5679b353 caa46825 fea2d371 3ba450da 0c2a4541";
    char y[] = "2 35b7c671 00506899 06bac3d9 dec76a83 5591edb2";
    elem_from_hex(e.g.x,x);
    elem_from_hex(e.g.y,y);
    elem_convert_endian(e.g.x);
    elem_convert_endian(e.g.y);
    int i;
    for(i=0; i<NUMWORDS-1; i++) {
      e.g.x[i] = (e.g.x[i] << MARGIN) | (e.g.x[i+1] >> (W-MARGIN));
      e.g.y[i] = (e.g.y[i] << MARGIN) | (e.g.y[i+1] >> (W-MARGIN));
    }
    e.g.x[NUMWORDS-1] <<= MARGIN;
    e.g.y[NUMWORDS-1] <<= MARGIN;
    
  }
  else if(DEGREE==113) {
    char x[] = "16c803abc107c188da15867fa62d4";
    char y[] = "de1e607f7da58e4a1858a8ba7c60";
    elem_from_hex(e.g.x,x);
    elem_from_hex(e.g.y,y);
  }
  else if(DEGREE==359) {
    char x[] = "36 76e5ba96 8e136f68 594557b7 b35c62df 069c55a6 9ef206e7 bc737667 039eed2c 907473f0 237b5d77 22f2629a";
    char y[] = "58 4162fd81 80a096f1 15b61a91 ea1f56aa a0e52dcd 0b63c644 eb64872f a5bfd99c 0fca39f8 d947bf3b f37f5b6e";
    elem_from_hex(e.g.x,x);
    elem_from_hex(e.g.y,y);
  }

  memcpy(e.g.z,gnb_one,sizeof(elem_t));
//  ec_io_print_point(&(e.g),"g");
  gnb_init();
  tau_init();
  ec_is_point_on_curve(&(e.g));
}


/**
 * comparison operation for points.
 *
 * @param p1 first point to compare
 * @param p2 point to compare p1 with
 * @return 1 if equal, else 0
 */
int ec_equals(ecpoint_t *p1, ecpoint_t *p2) {
  if(!memcmp(p1->x,p2->x,sizeof(elem_t)) 
     && !memcmp(p1->y,p2->y,sizeof(elem_t))) return 1;
  return 0;
}

/**
 * comparison operation for points.
 *
 * @param p1 first point to compare
 * @param p2 point to compare p1 with
 * @return 1 if equal, else 0
 */
int ec_equals_ld(ecpoint_t *p1, ecpoint_t *p2) {
  if(!memcmp(p1->x,p2->x,sizeof(elem_t))
     && !memcmp(p1->y,p2->y,sizeof(elem_t))
     && !memcmp(p1->z,p2->z,sizeof(elem_t))) return 1;
  return 0;
}


/**
 * deep copies point p2 to point p1
 *
 * @param p1 target point
 * @param p2 source point
 */
void ec_copy(ecpoint_t *p1, ecpoint_t *p2) {
	memcpy(p1->x,p2->x,sizeof(elem_t));
	memcpy(p1->y,p2->y,sizeof(elem_t));
}

/**
 * deep copies point p2 to point p1
 *
 * @param p1 target point
 * @param p2 source point
 */
void ec_copy_ld(ecpoint_t *p1, ecpoint_t *p2) {
	memcpy(p1->x,p2->x,sizeof(elem_t));
	memcpy(p1->y,p2->y,sizeof(elem_t));
	memcpy(p1->z,p2->z,sizeof(elem_t));
}

/**
 * addition of two affine points. outputs p2 = p0 + p1
 * P1363 A.10.2 p 128 "Full Addition and Subtraction (binary case)"
 *
 * @param p2 resulting point
 * @param p0 point zero
 * @param p1 point one
 */
void ec_add(ecpoint_t *p2, ecpoint_t *p0, ecpoint_t *p1) {
	if( ec_equals(p0,&infinity) ) {
		ec_copy(p2,p1);
		return;
	}
	if( ec_equals(p1,&infinity) ) {
		ec_copy(p2,p0);
		return;
	}
	/* a should be from domain parameters */
	elem_t lambda, a, x2, y2;
	memcpy(a,gnb_one,sizeof(elem_t));
	memcpy(x2,p2->x,sizeof(elem_t));
	memcpy(y2,p2->y,sizeof(elem_t));
	if(memcmp(p0->x,p1->x,sizeof(elem_t))) {
		gnb_add(y2,p0->y,p1->y);
		gnb_add(x2,p0->x,p1->x);
		gnb_inverse(lambda,x2);
		gnb_multiply(lambda,lambda,y2);
		gnb_add(x2,a,lambda);
		gnb_add(x2,x2,p0->x);
		gnb_add(x2,x2,p1->x);
		memcpy(y2,lambda,sizeof(elem_t));
		gnb_square(y2);
		gnb_add(x2,x2,y2);
	}
	else {
		if(!memcmp(p1->x,gnb_zero,sizeof(elem_t))) {
			ec_copy(p2,&infinity);
			return;
		}
		gnb_inverse(x2,p1->x);
		gnb_multiply(lambda,p1->y,x2);
		gnb_add(lambda,p1->x,lambda);
		gnb_add(x2,a,lambda);
		memcpy(y2,lambda,sizeof(elem_t));
		gnb_square(y2);
		gnb_add(x2,x2,y2);
	}
	gnb_add(y2,p1->x,x2);
	gnb_multiply(y2,y2,lambda);
	gnb_add(y2,y2,x2);
	gnb_add(y2,y2,p1->y);
	/* x2 and y2 are p2 */
	memcpy(p2->x,x2,sizeof(elem_t));
	memcpy(p2->y,y2,sizeof(elem_t));
}

/**
 * subtraction of two affine points.
 * P1363 A.10.2 p 128 "Full Addition and Subtraction (binary case)"
 * "To subtract the point P=(x,y), one adds the point -P=(x,x+y)."
 *
 * @param p2 resulting point
 * @param p0 point zero
 * @param p1 point one
 */
void ec_sub(ecpoint_t *p2, ecpoint_t *p0, ecpoint_t *p1) {
	ecpoint_t p1n;
	ec_copy(&p1n,p1);
	gnb_add(p1n.y,p1->x,p1->y);
	ec_add(p2,p0,&p1n);
}

void ec_normalize(ecpoint_t *q, ecpoint_t *p) {
	gnb_inverse(p->z,p->z);
	gnb_multiply(q->x,p->x,p->z);

	elem_rrotate(p->z);
	gnb_multiply(q->y,p->y,p->z);
	/*
	elem_t t1;
	gnb_inverse(t1,p->z);
	gnb_multiply(q->x,p->x,t1);

	memcpy(t1,p->z,sizeof(elem_t));
	elem_rrotate(t1);
	gnb_inverse(t1,t1);
	gnb_multiply(q->y,p->y,t1);
	*/
	memcpy(q->z,gnb_one,sizeof(elem_t));
}

void ec_double_ld(ecpoint_t *q, ecpoint_t *p) {
  if(ec_equals_ld(p,&infinity_ld)) {
    ec_copy_ld(q,&infinity_ld);
    return;
  }
  elem_t t1, t2;
  memcpy(t1,p->z,sizeof(elem_t));
  elem_rrotate(t1);
  memcpy(t2,p->x,sizeof(elem_t));
  elem_rrotate(t2);
  gnb_multiply(q->z,t1,t2);
  memcpy(q->x,t2,sizeof(elem_t));
  elem_rrotate(q->x);
  elem_rrotate(t1);
  //gnb_multiply(t2,t1,b); b is one
  gnb_add(q->x,q->x,t2);
  memcpy(t1,p->y,sizeof(elem_t));
  elem_rrotate(t1);
  gnb_add(t1,t1,q->z); // a is one
  gnb_add(t1,t1,t2);
  gnb_multiply(q->y,q->x,t1);
  gnb_multiply(t1,t2,q->z);
  gnb_add(q->y,q->y,t1);
}

// hankerson et al Alg 3.25 pp 116 (95)
void ec_add_ld(ecpoint_t *p2, ecpoint_t *p, ecpoint_t *q) {
	if(ec_equals_ld(q,&infinity_ld)) {
		ec_copy_ld(p2,p);
		return;
	}
	if (ec_equals_ld(p,&infinity_ld)) {
		ec_copy_ld(p2,q);
		return;
	}
	register elem_t t1, t2, t3;
	gnb_multiply(t1,p->z,q->x);
	memcpy(t2,p->z,sizeof(elem_t));
	elem_rrotate(t2);
	gnb_add(p2->x,p->x,t1);
	gnb_multiply(t1,p->z,p2->x);
	gnb_multiply(t3,t2,q->y);
	gnb_add(p2->y,p->y,t3);
	if(!memcmp(p2->x,gnb_zero,sizeof(elem_t))) {
		if(!memcmp(p2->y,gnb_zero,sizeof(elem_t)))
			ec_double_ld(p2,q);
		else ec_copy_ld(p2,&infinity_ld);
		return;
	}
	memcpy(p2->z,t1,sizeof(elem_t));
	elem_rrotate(p2->z);
	gnb_multiply(t3,t1,p2->y);
	// ok a is 1...
	gnb_add(t1,t1,t2);
	memcpy(t2,p2->x,sizeof(elem_t));
	elem_rrotate(t2);
	gnb_multiply(p2->x,t2,t1);
	memcpy(t2,p2->y,sizeof(elem_t));
	elem_rrotate(t2);
	gnb_add(p2->x,p2->x,t2);
	gnb_add(p2->x,p2->x,t3);
	gnb_multiply(t2,q->x,p2->z);
	gnb_add(t2,t2,p2->x);
	memcpy(t1,p2->z,sizeof(elem_t));
	elem_rrotate(t1);
	gnb_add(t3,t3,p2->z);
	gnb_multiply(p2->y,t3,t2);
	gnb_add(t2,q->x,q->y);
	gnb_multiply(t3,t1,t2);
	gnb_add(p2->y,p2->y,t3);
}

void ec_sub_ld(ecpoint_t *p2, ecpoint_t *p0, ecpoint_t *p1) {
	ecpoint_t p1n;
	ec_copy_ld(&p1n,p1);
	gnb_add(p1n.y,p1->x,p1->y);
	ec_add_ld(p2,p0,&p1n);
}

/* wrapper for scalar multiplication */
void ec_multiply(ecpoint_t *p1, ecpoint_t *p0o, mpz_t ko) {

	/* Use HW if available */
	if (hw_sock >= 0) {
		ec_hw_multiply(hw_sock, p1, p0o, ko);
		return;
	}

//printf("P0->X:\n%x%x%x%x%x%x\n", p0o->x[0], p0o->x[1], p0o->x[2], p0o->x[3], p0o->x[4], p0o->x[5]);
//printf("P0->Y:\n%x%x%x%x%x%x\n", p0o->y[0], p0o->y[1], p0o->y[2], p0o->y[3], p0o->y[4], p0o->y[5]);
//mpz_out_str(stdout, 16, ko);
//printf("\n");

	/* to ensure n < r/2: n(x,y) = (r-n)(x,x+y) */
	register mpz_t rover2, k;
	register ecpoint_t p0;
	mpz_init_set(rover2,e.r);
	mpz_init_set(k,ko);
	ec_copy(&p0,p0o);
	
	memcpy(p0.z,gnb_one,sizeof(elem_t)); /* p0 must be affine */
	
	mpz_fdiv_r(k,k,e.r); /* reduce k mod r  */
	mpz_fdiv_q_2exp(rover2,rover2,1);
	
	/* check if k > r/2 (we want smallest binary representation) */
	if(mpz_cmp(k,rover2) > 0) {
		//printf("NOTICE: taking smallest k...\n");
		mpz_sub(k,e.r,k);
		elem_t y2;
		gnb_add(y2,p0.x,p0.y);
		memcpy(p0.y,y2,sizeof(elem_t));
	}
	//else printf("NOTICE: k is ALREADY small...\n");
	mpz_clear(rover2);	
	ec_multiply_tau(p1,&p0,k); /* do the mult */
	mpz_clear(k);	
	ec_normalize(p1,p1); /* now normalize the point... */

//printf("P1->X:\n%x%x%x%x%x%x\n", p1->x[0], p1->x[1], p1->x[2], p1->x[3], p1->x[4], p1->x[5]);
//printf("P1->Y:\n%x%x%x%x%x%x\n", p1->y[0], p1->y[1], p1->y[2], p1->y[3], p1->y[4], p1->y[5]);
}

/**
 * basic, left to right binary elliptic scalar multiplication.
 * Hankerson, Algorithm 3.27 p 97
 *
 * @param p1 resulting point
 * @param p0 point to be multiplied
 * @param k  big integer scalar to multiply by
 */
void ec_multiply_binary(ecpoint_t *p1, ecpoint_t *p0, mpz_t k) {
  int i;
  ecpoint_t q;
  ec_copy(&q,&infinity);
  for(i=mpz_sizeinbase(k,2);  i--;) {
    ec_add(&q,&q,&q);
    if(mpz_tstbit(k,i))
      ec_add(&q,&q,p0);
  }
  ec_copy(p1,&q);
}

/**
 * addition-subtraction, NAF, goes by many names.
 * but it's a little better than the normal elliptic
 * scalar multiplication.
 * Solinas, Routine 6, p. 129
 *
 * @param p1 resulting point
 * @param p0 point to be multiplied
 * @param k  big integer scalar to multiply by
 */
void ec_multiply_addsub(ecpoint_t *p1, ecpoint_t *p0, mpz_t k) {
  int u;
  ecpoint_t q, p;
  mpz_t c;
  mpz_init_set(c,k);
  ec_copy(&q,&infinity);
  ec_copy(&p,p0);

  int ops = 0;
  while(mpz_sgn(c) > 0) {
    if(mpz_odd_p(c)) {
      u = mpz_fdiv_ui(c,4);
      u = 2 - u;
      if(u == 1) {
	mpz_sub_ui(c,c,1);
	ec_add(&q,&q,&p);
	ops++;
      }
      else if(u == -1) {
	mpz_add_ui(c,c,1);
	ec_sub(&q,&q,&p);
	ops++;
      }
    }
    mpz_fdiv_q_2exp(c,c,1);
    ec_add(&p,&p,&p);
  }
  ec_copy(p1,&q);
}

/**
 * the tau-adic method for elliptic scalar multiplication.
 * Solinas, Algorithm 3, p. 227
 *
 * @param p1 resulting point
 * @param p0 point to be multiplied
 * @param k  big integer scalar to multiply by
 */
void ec_multiply_tau(ecpoint_t *p1, ecpoint_t *p0, mpz_t k) {
	/* to ensure n < r/2: n(x,y) = (r-n)(x,x+y) */
	ecpoint_t q, p;
	//ec_copy(&q,&infinity);
	ec_copy_ld(&q,&infinity_ld);
	ec_copy_ld(&p,p0);

	mpz_t r0, r1, t1;
	mpz_init(r0);
	mpz_init(r1);
	mpz_init(t1);	
	tau_red_mod(r0,r1,k);

	int u;
	while((mpz_sgn(r0) != 0) || (mpz_sgn(r1) != 0)) {
		if(mpz_odd_p(r0)) {
			mpz_mul_2exp(t1,r1,1);
			mpz_sub(t1,r0,t1);
			u = 2 - mpz_fdiv_ui(t1,4);
			if (u == 1) {
				mpz_sub_ui(r0,r0,1);
				//ec_add(&q,&q,&p);
				ec_add_ld(&q,&q,&p);
			}
			else if (u == -1) {
				mpz_add_ui(r0,r0,1);
				//ec_sub(&q,&q,&p);
				ec_sub_ld(&q,&q,&p);
			}
		}
		elem_rrotate(p.x);
		elem_rrotate(p.y);
		elem_rrotate(p.z);

		mpz_fdiv_q_2exp(t1,r0,1);
		mpz_fdiv_q_2exp(r0,r0,1);
		mpz_add(r0,r1,r0);
		mpz_neg(r1,t1);
	}

	/* q holds the solution */
	ec_copy_ld(p1,&q);
	mpz_clear(r0);
	mpz_clear(r1);
	mpz_clear(t1);
}

/**
 * generates a public key W and a private key s, 0 < s < r
 * P1363, A.16.9, p. 170
 *
 * @param w  public key W to be generated
 * @param s  space for private key (big integer)
 */
void ec_keygen(ecpoint_t *w, mpz_t s) {
  //printf("---------START KEYGEN\n");
  /* generate the private key s */
  /*
  do {
    elem_random(w->x);
    elem_to_int(s,w->x);
  } while (mpz_sgn(s) < 1 || mpz_cmp(s,e.r) >= 0);
  */

  do {
    mpz_urandomm(s,texas,e.r);
  } while (mpz_sgn(s) < 1);

  /* now mulitply times the base point */
  ec_multiply(w,&(e.g),s);
  //printf("final secret key s : %s\n",mpz_get_str(NULL,16,s));
  //ec_io_print_point(w,"final public key w");
  //printf("---------END KEYGEN\n");

}

/**
 * field element to integer routine.
 * takes a field element in little endian b_0, b_1, b_2 ... b_(m-1)
 * to an integer in big endian b_(m-1), b_(m-2), ..., b_1, b_0
 *
 * @param i  the resulting big integer
 * @param ao the field element to be converted
 */
void elem_to_int(mpz_t i, elem_t ao) {
	elem_t a;
	memcpy(a,ao,sizeof(elem_t));
	elem_convert_endian(a);
	mpz_import(i,NUMWORDS,1,sizeof(ulong_t),0,0,a);
}

/**
 * "Solving Quadratic Equations over GF(2^m)" P1363, A.4.7 p 101
 * solves the equation z^2 + z = beta. This is the obvious way,
 * but there is probably a more efficient way...
 *
 * @param z    solution to the equation (field element)
 * @param beta coefficient for the equation
 */
void ec_solve_quad(elem_t z, elem_t beta) {
  int i;
  memset(z,0,sizeof(elem_t));
  int prevbit = 0;
  for(i=1; i<DEGREE; i++) {
    prevbit ^= elem_getcoef(beta,i);
    if(prevbit) elem_setcoef(z,i);
  }

  /* check the solution */
  elem_t gamma;
  memcpy(gamma,z,sizeof(elem_t));
  elem_rrotate(gamma);
  gnb_add(gamma,gamma,z);

  if(memcmp(beta,gamma,sizeof(elem_t)) != 0)
    memset(z,0,sizeof(elem_t));
}

/**
 * "Finding a Random Point on an Elliptic Curve (binary case)" P1363, A.11.2 p 137
 * gets a random point on the curve (surprise).
 *
 * @param p storage for the random point
 */
void ec_random_point(ecpoint_t *p) {
  elem_t alpha, beta, z;

  do {
    do {
      /* line 1 */
      elem_random(p->x);
      
      /* line 3 */
      memcpy(alpha,p->x,sizeof(elem_t));
      elem_rrotate(alpha);
      memcpy(beta,alpha,sizeof(elem_t));
      gnb_multiply(alpha,alpha,p->x);
      gnb_add(alpha,alpha,beta);
      gnb_add(alpha,alpha,gnb_one);
    } while(!memcmp(p->x,gnb_zero,sizeof(elem_t)) 
	    || !memcmp(alpha,gnb_zero,sizeof(elem_t)));
    
    /* line 5 */
    gnb_inverse(beta,beta); // beta is already x^2
    gnb_multiply(beta,beta,alpha);
    
    /* line 6 */
    ec_solve_quad(z,beta);
  } while(memcmp(z,gnb_zero,sizeof(elem_t)) == 0); // line 7

  /* generate a random bit mu and set y = (z+mu)x */
  if(rand_int(2)) /* that should be random enough. returns 0 or 1 */
     gnb_add(z,z,gnb_one);

  gnb_multiply(p->y,z,p->x);

  //ec_is_point_on_curve(p);
}

/**
 * a sanity check for making sure a point actually is on the curve,
 * y^2 + xy = x^3 + ax^2 + b (a, b = 1)
 *
 * @param p point to check
 */
void ec_is_point_on_curve(ecpoint_t *p) {
//  printf("---------START IS ON CURVE\n");
  elem_t left, right, t1;
  memcpy(t1,p->y,sizeof(elem_t));
  elem_rrotate(t1);
  gnb_multiply(left,p->x,p->y);
  gnb_add(left,left,t1);

  memcpy(t1,p->x,sizeof(elem_t));
  elem_rrotate(t1);
  memcpy(right,t1,sizeof(elem_t));
  gnb_multiply(right,right,p->x);
  gnb_add(right,right,t1);
  gnb_add(right,right,gnb_one);

//  printf("left : "); elem_print_hex(left);
//  printf("right: "); elem_print_hex(right);
  
//  if(!memcmp(left,right,sizeof(elem_t))) printf("PASSED\n");
//  else printf("FAILED\n");
//  printf("---------END IS ON CURVE\n");
}

/**
 * "Finding a Point of Large Prime Order" P1363, A.11.3 p 137
 * "If the order #E(GF(q)) = u of an elliptic curve is nearly prime,
 * the following algorithm efficiently produces a random point on E
 * whose order is the large prime factor r of u = kr."
 * For K-163, the cofactor is 2. (k)
 *
 * @param g storage for the random point of large prime order
 */
void ec_random_point_largeorder(ecpoint_t *g) {
  //printf("---------START RANDOM POINT LARGEORDER\n");
  ecpoint_t p, q;
  do {
    ec_random_point(&p);
    /* cofactor is 2, so we can just double p. If it was 4, double twice.. */
    ec_add(g,&p,&p);
  } while(ec_equals(g,&infinity));
  /* dumb, any self-respecting scalar mult will take
     smallest binary rep mod r anyways..but OK */
  ec_multiply(&q,g,e.r);
  if(!ec_equals(&q,&infinity)) printf("FAILED, not infinity\n");
  //else printf("PASSED, is infinity\n");
  //ec_io_print_point(g,"generated g");
  //printf("---------END RANDOM POINT LARGEORDER\n");
}

/**
 * 7.2.5 ECSP-NR (signature primitive, Nyberg-Rueppel) P1363 p 41
 *
 * @param cd holds the resulting signature (c,d)
 * @param s  signers long-term private key s (big integer)
 * @param f  message representative (byte hash of message)
 */
void ec_sign_nr(ecsig_t *cd, mpz_t s, byte_t *f) {
  printf("---------START NR SIGN\n");
  ecpoint_t v;
  mpz_t u,i;
  mpz_init(u);
  mpz_init(i);
  mpz_init(cd->c);
  mpz_init(cd->d);

  do{
    ec_keygen(&v,u);
    elem_to_int(i,v.x);
    mpz_import(cd->c,11,1,sizeof(byte_t),0,0,f);
    printf("f as int %d : %s\n", (int)mpz_sizeinbase(cd->c,2),mpz_get_str(NULL,16,cd->c));
    mpz_add(cd->c,i,cd->c);
    mpz_fdiv_r(cd->c,cd->c,e.r);
  } while(!mpz_sgn(cd->c));
  mpz_mul(cd->d,s,cd->c);
  mpz_sub(cd->d,u,cd->d);
  mpz_fdiv_r(cd->d,cd->d,e.r);
  mpz_clear(u);
  mpz_clear(i);
  printf("c: %s\n",mpz_get_str(NULL,16,cd->c));
  printf("d: %s\n",mpz_get_str(NULL,16,cd->d));
  printf("---------END NR SIGN\n");
}

/**
 * 7.2.6 ECVP-NR (verification primitive, Nyberg-Rueppel) P1363 p 42
 *
 * @param f  message representative (byte hash of message)
 * @param cd signature (c,d) to be verified
 * @param w  signers public key
 * @return nothing yet but should be true or false
 */
void ec_verify_nr(byte_t *f, ecsig_t *cd, ecpoint_t *w) {
  printf("---------START NR VERIFY\n");
  /* TODO: check sig ranges */
  ecpoint_t dg, cw, p;
  mpz_t i;
  mpz_init(i);

  ec_multiply(&dg,&(e.g),cd->d);
  ec_multiply(&cw,w,cd->c);

  ec_add(&p,&dg,&cw);
  elem_to_int(i,p.x);

  mpz_t fi, forig;
  mpz_init(fi);
  mpz_init(forig);
  mpz_sub(fi,cd->c,i);
  mpz_fdiv_r(fi,fi,e.r);

  mpz_import(forig,11,1,sizeof(byte_t),0,0,f);
  printf("fi    : %s\n",mpz_get_str(NULL,16,fi));
  printf("forig : %s\n",mpz_get_str(NULL,16,forig));
  if(mpz_cmp(fi,forig) != 0) printf("FAILED\n");
  else printf("PASSED\n");
  printf("---------END NR VERIFY\n");
  mpz_clear(i);
  mpz_clear(fi);
  mpz_clear(forig);
  /* needs to return something... */
}

/**
 * 7.2.7 ECSP-DSA (signature primitive, DSA) P1363 p 43
 *
 * @param cd holds the resulting signature (c,d)
 * @param s  signers long-term private key s (big integer)
 * @param f  message representative (byte hash of message)
 */
void ec_sign_dsa(ecsig_t *cd, mpz_t s, byte_t *f) {
  printf("---------START DSA SIGN\n");
  ecpoint_t v;
  mpz_init(cd->c);
  mpz_init(cd->d);

  do{
    mpz_t u, t1, t2, fi;
    mpz_init(u);
    mpz_init(t1);
    mpz_init(t2);
    mpz_init(fi);

    ec_keygen(&v,u);
    elem_to_int(cd->c,v.x);
    mpz_fdiv_r(cd->c,cd->c,e.r);
    mpz_mul(t1,s,cd->c);
    mpz_import(fi,11,1,sizeof(byte_t),0,0,f);
    mpz_add(t1,fi,t1);
    mpz_invert(t2,u,e.r);
    mpz_mul(cd->d,t2,t1);
    mpz_fdiv_r(cd->d,cd->d,e.r);

    mpz_clear(u);
    mpz_clear(t1);
    mpz_clear(t2);
    mpz_clear(fi);
  } while(!mpz_sgn(cd->c) || !mpz_sgn(cd->d) );

  printf("c: %s\n",mpz_get_str(NULL,16,cd->c));
  printf("d: %s\n",mpz_get_str(NULL,16,cd->d));
  printf("---------END DSA SIGN\n");
}

/**
 * 7.2.8 ECVP-DSA (verification primitive, DSA) P1363 p 43
 *
 * @param f  message representative (byte hash of message)
 * @param cd signature (c,d) to be verified
 * @param w  signers public key
 * @return nothing yet but should be true or false
 */
void ec_verify_dsa(byte_t *f, ecsig_t *cd, ecpoint_t *w) {
  printf("---------START DSA VERIFY\n");
  /* TODO: check sig ranges */

  mpz_t h, h1, h2, fi;
  mpz_init(h);
  mpz_init(h1);
  mpz_init(h2);
  mpz_init(fi);

  mpz_invert(h,cd->d,e.r);
  mpz_import(fi,11,1,sizeof(byte_t),0,0,f);
  mpz_mul(h1,fi,h);
  mpz_mul(h2,cd->c,h);
  mpz_fdiv_r(h1,h1,e.r);
  mpz_fdiv_r(h2,h2,e.r);

  ecpoint_t h1g, h2w, p;

  ec_multiply(&h1g,&(e.g),h1);
  ec_multiply(&h2w,w,h2);
  ec_add(&p,&h1g,&h2w);

  mpz_t i;
  mpz_init(i);
  elem_to_int(i,p.x);
  mpz_fdiv_r(i,i,e.r);
  printf("c: %s\n",mpz_get_str(NULL,16,cd->c));
  printf("i: %s\n",mpz_get_str(NULL,16,i));
  if(mpz_cmp(cd->c,i) != 0) printf("FAILED\n");
  else printf("PASSED\n");
  printf("---------END DSA VERIFY\n");
  mpz_clear(h);
  mpz_clear(h1);
  mpz_clear(h2);
  mpz_clear(fi);
  mpz_clear(i);
}


/**
 * signature primitive for PLA, schnorr type
 *
 * @param sig   pointer to space for the signature
 * @param m     pointer to the hash of the message
 * @param sigma pointer to the signers private key
 */
void ec_sign_self(ecselfsig_t *sig, byte_t *m, mpz_t sigma){
  ecpoint_t v;
  mpz_t k;
  mpz_init(k);

  /* kG_d */
  ec_keygen(&v,k);

  /* m is h(m) (hash of the mesage) */

  /* r = H_1(H_2(m) || [kG_D]_x) */

  /* init the hash */
  unsigned int md_len;
  EVP_MD_CTX mdctx;
  EVP_MD_CTX_init(&mdctx);
  //EVP_DigestInit_ex(&mdctx,EVP_sha1(),NULL);
  EVP_DigestInit_ex(&mdctx,EVP_ripemd160(),NULL);

  /* run H_2(m) through H_1 */
  EVP_DigestUpdate(&mdctx,m,H);
  /* run v_x through H_1 */
  EVP_DigestUpdate(&mdctx,(byte_t *) &(v.x[0]),sizeof(elem_t));
  EVP_DigestFinal_ex(&mdctx,sig->r,&md_len);
  EVP_MD_CTX_cleanup(&mdctx);


  /* s = (k-r*sigma) mod e.r */
  mpz_t ri;
  mpz_init(ri);
  mpz_import(ri,H,1,sizeof(byte_t),0,0,sig->r);
  /*
  int i;
  for(i=0; i<H; i++) printf("%02x ",sig->r[i]); printf("\n");
  printf("ri   : %s\n",mpz_get_str(NULL,16,ri));
  */
  mpz_fdiv_r(ri,ri,e.r);
  mpz_init(sig->s);
  mpz_mul(sig->s,ri,sigma);
  mpz_sub(sig->s,k,sig->s);
  mpz_fdiv_r(sig->s,sig->s,e.r);
  mpz_clear(ri);
}

/**
 * verification primitive for PLA, schnorr type
 *
 * @param sig pointer to signature to verify
 * @param m   pointer to the hash of the message
 * @param id  pointer to the hash of the signer's ID
 * @return 1 if it verifies, else 0
 */
int ec_verify_self(ecselfsig_t *sig, byte_t *m, byte_t *id) {

	/* sG_D */
	ecpoint_t sgd;
	ec_multiply(&sgd,&(e.g),sig->s);

	/* extract the public key eta */
	mpz_t rhoi;
	mpz_init(rhoi);
	elem_to_int(rhoi,sig->rho);
	mpz_fdiv_r(rhoi,rhoi,e.r);
	ecpoint_t rhoyd;
	ec_multiply(&rhoyd,&(e.y),rhoi); /* rhoY_D */
	mpz_clear(rhoi);

	elem_t x, hid;
	memcpy(x,sig->rho,sizeof(elem_t));
	//elem_from_hex(hid,id); /* hash of ID! */
	elem_import_bytes(hid,id); elem_chomp(hid);
	gnb_add(x,x,hid);
	ecpoint_t eta;
	ec_decompress_point(&eta,x,sig->b); /* DEC(rho-h(ID),b) */
	//ec_add(&eta,&eta,&rhoyd); /* eta = DEC(rho-h(ID),b) + rhoY_D */
	ec_sub(&eta,&eta,&rhoyd); /* eta = DEC(rho-h(ID),b) - rhoY_D */

#if 0
fprintf(stderr, "Extracted user's public key:\n");
fprintf(stderr, "x: "); elem_print_hex(eta.x);
fprintf(stderr, "y: "); elem_print_hex(eta.y);
fprintf(stderr, "z: "); elem_print_hex(eta.z);
fprintf(stderr, "\n");
#endif
	mpz_t ri;
	mpz_init(ri);
	mpz_import(ri,H,1,sizeof(byte_t),0,0,sig->r);
	mpz_fdiv_r(ri,ri,e.r);
	ecpoint_t reta;
	ec_multiply(&reta,&eta,ri); /* rEta */
	mpz_clear(ri);

	ec_add(&sgd,&sgd,&reta); /* sG_D + rEta */

	/* init the hash */	
	EVP_MD_CTX mdctx;
	EVP_MD_CTX_init(&mdctx);
	//EVP_DigestInit_ex(&mdctx,EVP_sha1(),NULL);
	EVP_DigestInit_ex(&mdctx,EVP_ripemd160(),NULL);

	/* run H_2(m) through H_1 */
	EVP_DigestUpdate(&mdctx,m,H);
	/* run sG_D + rEta through H_1 */
	EVP_DigestUpdate(&mdctx,(byte_t *) &(sgd.x[0]),sizeof(elem_t));
	/* the result should be r if sig is valid */
	unsigned int md_len;
	byte_t result[H];
	EVP_DigestFinal_ex(&mdctx,result,&md_len);
	EVP_MD_CTX_cleanup(&mdctx);

	if(!memcmp(sig->r,result,H)) {
//		printf("PASSED, hash match\n");
		return 1;
	}
//	printf("FAILED, hash mismatch\n");
	return 0;
}

/**
 * A.12.9 "Decompression of y Coordinates (binary case)" P1363 p 146
 * recovers the y coordinate of a point given its x coordinate and compression bit
 *
 * @param p holds the resulting decompressed point
 * @param x x-coordinate of the point
 * @param y compression bit of the y-coordinate
 */
void ec_decompress_point(ecpoint_t *p, elem_t x, int y) {
	/* TODO: check if x = 0 */

	/* line 2 */
	elem_t alpha, beta, z;
	memcpy(beta,x,sizeof(elem_t));
	elem_rrotate(beta);
	memcpy(alpha,beta,sizeof(elem_t));
	gnb_multiply(alpha,alpha,x);
	gnb_add(alpha,alpha,beta);
	gnb_add(alpha,alpha,gnb_one);

	/* line 3 */
	gnb_inverse(beta,beta);
	gnb_multiply(beta,alpha,beta);

	/* line 4 */
	ec_solve_quad(z,beta);

	/* line 5-6 rightmost bit of z ... least sig? */
	if(elem_getcoef(z,0) ^ y)
		gnb_add(z,z,gnb_one);
	gnb_multiply(p->y,z,x);
	memcpy(p->x,x,sizeof(elem_t));
}

/**
 * A.9.6 "Representation of Points" P1363 p 127
 * computes the compression bit of a point
 * rightmost bit of y*x^-1
 *
 * @param y compression bit of the y-coordinate
 * @param p point to be compressed
 */
void ec_compress_point(int *y, ecpoint_t *p){
  /* TODO: check for x = 0 */
  elem_t a;
  gnb_inverse(a,p->x);
  gnb_multiply(a,a,p->y);
  /* bit is rightmost bit of a...least sig? */
  *y = elem_getcoef(a,0);
}


/**
 * Creates a new key for the user using implicit certificate mechanism
 *
 * @param	impl_cert		implicit certificate given by TTP
 * @param	impl_cert_b		compression bit of the implicit certificate
 * @param	private_key		user's private key
 * @param	id_hash			user's identity hash
 * @param	ttp_private_key	TTP's private key
 */
void ec_create_key(elem_t *impl_cert, uint8_t *impl_cert_b, mpz_t *private_key,
	unsigned char *id_hash, mpz_t *ttp_private_key)
{
	mpz_t k, kT, ru_tmp, si;
	ecpoint_t kG, kTG, r;
	elem_t hid;

	mpz_init(k); mpz_init(kT); mpz_init(ru_tmp); mpz_init(si); 

	/* User generates kG and sends it to TTP */
	ec_keygen(&kG, k);

	/* TTP generates kT... */
	ec_keygen(&kTG, kT);

	/* ...and calculates: (r.x, impl_cert_b) = COMPRESS(kG + kTG) */
	ec_add(&r, &kG, &kTG);
	ec_compress_point((int *)impl_cert_b, &r);

	/* ...and: impl_cert = r.x + H(id_hash) */
	elem_import_bytes(hid, id_hash);
	elem_chomp(hid); /* hash of the ID */
	gnb_add(*impl_cert, hid, r.x);

	/* ...and: si = kT - impl_cert*ttp_private_key (mod r) */
	elem_to_int(ru_tmp, *impl_cert);
	mpz_mul(si, ru_tmp, *ttp_private_key);
	mpz_sub(si, kT, si); 
	mpz_fdiv_r(si, si, e.r);

	/* TTP sends its implicit certificate and signature ((impl_cert, impl_cert_b), si) 
		back to the user, which calculates his private key: private_key = k + si (mod r) */
	mpz_add(*private_key, k, si);
	mpz_fdiv_r(*private_key, *private_key, e.r);
}
