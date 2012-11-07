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
#include "gnb.h"


int32_t **R;
ulong_t *TA;                        
ulong_t *TB;                        

elem_t gnb_zero;
elem_t gnb_one;

const byte_t BitReverseTable256[256] = 
{
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};


/****************************************************************
 * assorted utility functions that don't really fit anywhere    *
 ****************************************************************/

/* a simple random function. OK for precomputation, NOT for crypto purposes */
int rand_int(int n) { return (int)((double)rand() / ((double)RAND_MAX + 1) * n); }



/****************************************************************
 *  field element manipulation and informational functions      *
 ****************************************************************/

/* these are pretty self-explanatory. remember the bit ordering */
int elem_getcoef(const elem_t a, int i) { return ((a[i / W] >> (W-1-(i % W))) & 1); }
void elem_setcoef(elem_t a, int i) { a[i / W] |= 1 << (W-1-(i % W)); }
void elem_clearcoef(elem_t a, int i) { a[i / W] &= ~(1 << (W-1-(i % W))); }
void elem_flipcoef(elem_t a, int i) { a[i / W] ^= 1 << (W-1-(i % W)); }

/**
 * reverse the bit ordering of a field element
 *
 * @param b element to reverse
 */
void elem_convert_endian(elem_t a) {
  int i;
  elem_reverse_words(a,0,NUMWORDS);
  for(i=NUMWORDS; i--; ) {
    ulong_t c;
    byte_t * p = (byte_t *) &a[i];
    byte_t * q = (byte_t *) &c;
    q[3] = BitReverseTable256[p[0]]; 
    q[2] = BitReverseTable256[p[1]]; 
    q[1] = BitReverseTable256[p[2]]; 
    q[0] = BitReverseTable256[p[3]];
    a[i] = c;
  }
}

/**
 * creates a field element from a byte array.
 * reads the byte array just like it would a field element,
 * so the bit orderings should be the same.
 * NOTE: should chomp after this, not done automatically
 *       because some data (compression point) might
 *       want to be attached
 *
 * @param a space for the new field element
 * @param b byte array to create the field element from
 */
void elem_import_bytes(elem_t a, const byte_t *b) {
  int i;
  for(i=0; i<NUMWORDS; i++, b+=4)
    a[i] = ntohl(*(ulong_t*)(b));
}

/**
 * fills a byte array from a field element.
 * writes the byte array just like it would a field element,
 * so the bit orderings should be the same.
 *
 * @param b byte array to write the field element to
 * @param a field element to export
 */
void elem_export_bytes(byte_t *b, const elem_t a) {
  int i;
  for(i=0; i<NUMWORDS; i++, b+=4)
    *(ulong_t*)(b) = htonl(a[i]);
}

/**
 * prints the field element a in binary format
 *
 * @param a field element to print
 */
void elem_print_binary(const elem_t a)
{
  int i, j;
  for(i=0; i < NUMWORDS; i++) {
    for(j=31; j >= 0; j--) {
      printf("%lu", (long unsigned int)((a[i] >> j) & 1));
    }
    printf("|");
  }
  printf("\n");
}

/**
 * prints the field element a in hex format
 *
 * @param a field element to print
 */
void elem_print_hex(const elem_t a)
{
  int i;
  for(i=0; i<NUMWORDS; i++) fprintf(stderr,"%08lx ", (long unsigned int)a[i]);
  fprintf(stderr, "\n");
}

/**
 * removes MARGIN bits from the end of the element.
 * useful after shifting
 *
 * @param a field element to chomp
 */
void elem_chomp(elem_t a) {
  a[NUMWORDS-1] &= (0xFFFFFFFF << MARGIN);
}

/**
 * loads random bits into a field element
 *
 * @param a field element to hold the randomized bits
 */
void elem_random(elem_t a)
{
  FILE *devrandom;
  if ((devrandom = fopen("/dev/urandom","r")) == NULL) {
    /* error out, or use some alternate method. */
    int i;
    elem_t b;
    /* maybe this is random enough for a backup. */
    for(i=0; i<NUMWORDS; i++) {
      a[i] = rand_int(RAND_MAX);
      b[i] = rand_int(RAND_MAX);
    }
    elem_lrotate(b);
    gnb_add(a,a,b);
  } else {
    fread(a,sizeof(elem_t),1,devrandom);
    fclose(devrandom);
  }
  elem_chomp(a);
}

/**
 * performs a left bitwise rotation by 1 on a field element
 * same as a sqrt in normal basis
 *
 * @param a field element to rotate left
 */
void elem_lrotate(elem_t a) {

  int i;
  ulong_t c = a[0] >> (W-1);
  for(i=0; i<NUMWORDS-1; i++)
    a[i] = (a[i] << 1) | (a[i+1] >> (W-1));
  a[NUMWORDS-1] = (a[NUMWORDS-1] << 1) | (c << MARGIN);
}

/**
 * performs a right bitwise rotation by 1 on a field element
 * same as a squaring in normal basis
 *
 * @param a field element to rotate right
 */
void elem_rrotate(elem_t a) {
  int i;
  for(i=NUMWORDS-1; i>0; i--)
    a[i] = (a[i] >> 1) | (a[i-1] << (W-1));
  a[0] = (a[0] >> 1) | (a[NUMWORDS-1] << (W-MARGIN));
  elem_chomp(a);
}

/**
 * reverses n words in a starting from index s
 * e.g. word indices 1 2 3 4 -> 4 3 2 1
 *
 * @param a field element to perform reverse in
 * @param s starting index for the reverse
 * @param n number of words to reverse
 */
void elem_reverse_words(elem_t a, int s, int n) {
  int i, j = (n >> 1);
  n--;
  for(i=0; i<j; i++) {
    ulong_t tmp = a[i+s];
    a[i+s] = a[s+n-i];
    a[s+n-i] = tmp;
  }
}

/**
 * left rotation of the words in a by k positions
 * performed in-place via 3 in-place reversals
 * reads and writes each word twice
 *
 * @param a field element to rotate left
 * @param k number of positions to rotate
 */
void elem_rotate_words(elem_t a, int k) {
  if(k<0 || k>=DEGREE) k = (k+DEGREE)%DEGREE;
  if(k==0) return;

  elem_reverse_words(a,0,k);
  elem_reverse_words(a,k,NUMWORDS-k);
  elem_reverse_words(a,0,NUMWORDS);
}

/**
 * performs a right bitwise rotation by offset on a field element
 * same as a a^(2^offset) in normal basis
 *
 * @param a      field element to rotate right by offset
 * @param offset rotation amount
 */
void elem_rrotate_n(elem_t a, int offset) {
  int i;
  if(offset<0 || offset >= DEGREE) offset %= DEGREE;
  if(offset < 0) offset += DEGREE;
  if(offset == 0) return;

  if(offset>=W) {
    elem_rotate_words(a,NUMWORDS-offset/W);
    for(i=offset/W-1; i>=0; i--)
      a[i] = (a[i] >> MARGIN) | (a[(i-1+NUMWORDS)%NUMWORDS] << (W-MARGIN));
    elem_chomp(a);
  }
  if(offset%=W) {
    while(offset>MARGIN) {
      elem_rrotate_n(a,MARGIN);
      offset -= MARGIN;
    }
    for(i=NUMWORDS-1; i>0; i--)
      a[i] = (a[i] >> offset) | (a[i-1] << (W-offset));
    a[0] = (a[0] >> offset) | (a[NUMWORDS-1] << (W-MARGIN));
    elem_chomp(a);
  }
}



/****************************************************************
 *  field arithmetic and operations for gaussian normal basis   *
 ****************************************************************/

/**
 * simple modular exponentiation. calculates b^e mod m
 * used only in the process of deriving a multiplication rule
 * for the gaussian normal basis
 * P1363 A.2.1 p. 84 (but right to left instead)
 *
 * @param b the base
 * @param e the exponent
 * @param m the modulus
 * @return b^e mod m
 */
int gnb_modpow(int32_t b, int32_t e, int32_t m)
{
  int result;
  result = 1;
  while(e) {
    if(e & 1) result = (result * b) % m;
    e >>= 1;
    b = (b * b) % m;
  }
  return result;
}

/**
 * computes the order of a given integer modulo P
 * used only in the process of deriving a multiplication rule
 * for the gaussian normal basis
 * P1363 A.2.7 p. 87
 *
 * @param g integer to compute the order of
 * @return order of g modulo P
 */
int gnb_compute_order(int32_t g)
{
  int32_t b, j;
  b = g;
  j = 1;
  while(b > 1) {
    b = g * b % P;
    j++;
  }
  return j;
}

/**
 * constructs an integer of order T modulo P
 * used only in the process of deriving a multiplication rule
 * for the gaussian normal basis
 * P1363 A.2.8 p. 87
 *
 * @return integer u of order T modulo P
 */
int gnb_get_u()
{
  int32_t g, k;
  k = T +1;
  while(k % T) {
    g = rand_int(P - 2) + 2;
    k = gnb_compute_order(g);
  }
  return gnb_modpow(g,k/T,P);
}

/**
 * precomputation for multiplication for a gaussian normal basis
 * from many sources, all noted below
 */
void gnb_init()
{
  int i, j;
  int32_t w, n, u;
  int32_t *F;
  //byte_t **M;
  int32_t **M;

  TA = (ulong_t*) malloc(DEGREE*2*sizeof(ulong_t));
  TB = (ulong_t*) malloc(DEGREE*2*sizeof(ulong_t));
  F = (int32_t*) malloc(P*sizeof(int32_t));
  memset(F,0,P*sizeof(int32_t));

  /* compute f's (P1363, A.3.7 p. 93) */
  u = gnb_get_u();
  w = 1;
  for(j=0; j < T; j++) {
    n = w;
    for(i=0; i < DEGREE; i++) {
      F[n] = i;
      n = 2 * n % P;
    }
    w = u * w % P;
  }
  /*
  printf("fp:\n");
  for(i=1; i<P; i++) printf("%d ",F[i]);
  printf("\n");
  */

  M = (int32_t**) malloc(DEGREE*sizeof(int32_t*));
  R = (int32_t**) malloc(DEGREE*sizeof(int32_t*));
  for(i=0; i<DEGREE; i++) {
    M[i] = (int32_t*) malloc(DEGREE*sizeof(int32_t));
    memset(M[i],0,DEGREE * sizeof(int32_t));
    R[i] = (int32_t*) malloc((T+1)*sizeof(int32_t));
    memset(R[i],0,(T+1) * sizeof(int32_t));
  }


  /* build the multiplication matrix M */
  for(i=1; i<P-1; i++) {
    int ir = F[P-i];
    int ic = F[i+1];
    M[ir][ic] = (M[ir][ic]+1)%2;
  }

  /*
  for(i=0;i<DEGREE; i++) {
    for(j=0; j<DEGREE; j++) {
      printf("%d",M[i][j]);
    }
    printf("\n");
  }
  */

  /*
    M is sparse (no more than T entries per row), so build
    a matrix R DEGREExT which holds the indices for the 1 entries
  */
  for(i=0; i<DEGREE; i++) {
    int ones = 0;
    for(j=0; j<DEGREE; j++) {
      //printf("%d",M[i][j]);
      if(M[i][j] == 1){
	ones++;
	R[i][ones] = j;
      }
    }
    //printf(" :%d\n",ones);
  }

  /*
  for(i=1; i<DEGREE; i++) {
    for(j=1; j<=T; j++) {
      printf("%d ",R[i][j]);
    }
    printf("\n");
  }
  */

  /* free these temporary matrices */
  free((void *)F);
  for(i = 0; i < DEGREE; i++)
    free((void *)M[i]);
  free((void *)M);

  /* set zero and one */
  memset(gnb_zero,0,sizeof(elem_t));
  memset(gnb_one,-1,sizeof(elem_t));
  elem_chomp(gnb_one);
}

/**
 * adds two field elements (just an XOR)
 * P1363 A.3.3 p. 91
 *
 * @param c field element to store the sum a + b
 * @param a field element
 * @param b field element
 */
void gnb_add(elem_t c, const elem_t a, const elem_t b)
{
  int i;
  for(i=0; i < NUMWORDS; i++) *c++ = *a++ ^ *b++;
}

/* ditto for ANDing...used in multiplication */
void gnb_and(elem_t c, const elem_t a, const elem_t b)
{
  int i;
  for(i=0; i < NUMWORDS; i++) *c++ = *a++ & *b++;
}

/**
 * squares a field element (just a right rotation)
 * P1363 A.4.1 p. 96
 *
 * @param a field element to square
 */
void gnb_square(elem_t a) { elem_rrotate(a); }

/**
 * multiplies two field elements in normal basis
 *
 * @param c  field element to store the product at*bt
 * @param at field element
 * @param bt field element
 */
void gnb_multiply(elem_t c, const elem_t at, const elem_t bt)
{
  int i,j;
  elem_t a,b;
  ulong_t sb;
  memcpy(a,at,sizeof(elem_t));
  memcpy(b,bt,sizeof(elem_t));

  //precompute TA and TB...
  for(i=0; i<DEGREE; i++) {
    TA[i] = TA[i+DEGREE] = a[0];
    TB[i] = TB[i+DEGREE] = b[0];
    elem_lrotate(a);
    elem_lrotate(b);
  }

  /*
  ulong_t *TA2, *TB2;
  TA2 = malloc(DEGREE*2*sizeof(ulong_t));
  TB2 = malloc(DEGREE*2*sizeof(ulong_t));
  */
  /*
  for(i=0; i<NUMWORDS; i++) {
    TA[i*W] = TA[i*W+DEGREE] = at[i];
    TB[i*W] = TB[i*W+DEGREE] = bt[i];
  }
  TA[(NUMWORDS-1)*W] |= ( TA[0] >> (W-MARGIN) );
  TA[(NUMWORDS-1)*W+DEGREE] = TA[(NUMWORDS-1)*W];
  TB[(NUMWORDS-1)*W] |= ( TB[0] >> (W-MARGIN) );
  TB[(NUMWORDS-1)*W+DEGREE] = TB[(NUMWORDS-1)*W];

  for(i=1; i<W; i++) {
    for(j=0; (i+j*W)<DEGREE; j++) {
      TA[i+j*W] = (TA[i-1+j*W] << 1) | (TA[i-1+(j+1)*W] >> (W-1));
      TA[i+j*W+DEGREE] = TA[i+j*W];
      TB[i+j*W] = (TB[i-1+j*W] << 1) | (TB[i-1+(j+1)*W] >> (W-1));
      TB[i+j*W+DEGREE] = TB[i+j*W];
    }
  }

  for(i=(NUMWORDS-1)*W; i<DEGREE; i++) {
    TA[i] = (TA[i-1] << 1) | (TA[i+(W-1)] >> (W-1));
    TA[i+DEGREE] = TA[i];
    TB[i] = (TB[i-1] << 1) | (TB[i+(W-1)] >> (W-1));
    TB[i+DEGREE] = TB[i];
  }
  */
  /*
  for(i=0; i<2*DEGREE; i++) {
    printf("a_%d: %08x\t%08x",i,TA[i],TA2[i]);
    if(TA[i] != TA2[i]) printf(" FAILED\n");
    else printf(" passed\n");
  }

  for(i=0; i<2*DEGREE; i++) {
    printf("b_%d: %08x\t%08x",i,TB[i],TB2[i]);
    if(TB[i] != TB2[i]) printf(" FAILED\n");
    else printf(" passed\n");
  }
  */

  for(j=0; j<NUMWORDS; j++)
    c[j] = TA[j*W] & TB[1+j*W];
  
  for(i=1; i<DEGREE; i++) {
    for(j=0; j<NUMWORDS; j++) {
      //sb=0;
      //for(int k=1; k<=T; k++)
      //sb ^= TB[ R[i][k] + j*W ];
      sb = TB[ R[i][1] + j*W ];
      sb ^= TB[ R[i][2] + j*W ];
      sb ^= TB[ R[i][3] + j*W ];
      sb ^= TB[ R[i][4] + j*W ];
      c[j] = c[j] ^ (TA[i + j*W] & sb);
    }
  }
  elem_chomp(c);
}

/**
 * inverts a field element in normal basis
 * computes beta^-1 = beta^(2^DEGREE-2)
 * this is left to right, probably better to rewrite right to left
 * rewrite to use the arbitrary rotation as well
 * P1363, A.4.4, p. 99
 *
 * @param eta field element to store the inverse
 * @param beta field element to invert
 */
void gnb_inverse(elem_t eta, const elem_t b) {
  int mm1, i, k, r;
  elem_t mu, beta;
  memcpy(beta,b,sizeof(elem_t));
  mm1 = DEGREE - 1;
  r = -1;
  while(mm1) {
    mm1 >>= 1;
    r++;
  }
  mm1 = DEGREE - 1;

  memcpy(eta,beta,sizeof(elem_t));
  k = 1;
  for(i=r-1; i >= 0; i--) {
    memcpy(mu,eta,sizeof(elem_t));
    elem_rrotate_n(mu,k);
    gnb_multiply(eta,mu,eta);
    k <<= 1;
    if((mm1 >> i) & 1) {
      gnb_square(eta);
      gnb_multiply(eta,eta,beta);
      k++;
    }
  }
  gnb_square(eta);
}
