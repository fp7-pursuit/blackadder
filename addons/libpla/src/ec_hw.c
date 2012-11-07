/*
* Copyright (c) 2008, Juha Forsten <juha.forsten@hut.fi>, Kimmo Järvinen <kimmo.jarvinen@hut.fi>, TKK
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
#include "ec_hw.h"

/**
 * Functions for communicating with cryptographic hardware
 */
int ec_hw_open(int *sock)
{
  struct sockaddr_in hw_board;

  /* Create the TCP socket */
  if ((*sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("HW: Failed to create socket");
    return(HW_ERROR);
  }
  
  /* Construct the server sockaddr_in structure */
  memset(&hw_board, 0, sizeof(hw_board));       /* Clear struct */
  hw_board.sin_family = AF_INET;                  /* Internet/IP */
  hw_board.sin_addr.s_addr = inet_addr(HW_HOST);  /* IP address */
  hw_board.sin_port = htons(atoi(HW_PORT));       /* server port */

  /* Establish connection */
  if (connect(*sock,(struct sockaddr *) &hw_board,sizeof(hw_board)) < 0) {
    perror("HW: Failed to connect with Hardware board");
    return(HW_ERROR);
  }

  return 0;
}

int ec_hw_close(int sock)
{
  uint32_t quit[6] = {0,0,0,0,0,0xffffffff};


  /* Send QUIT-packet to the server */
  
  if (send(sock, quit, 72, 0) != 72) {
    perror("HW: Mismatch in number of sent bytes");
    return(HW_ERROR);
  }
  close(sock);

  return 0;
}

int ec_hw_multiply(int sock, ecpoint_t *p1, ecpoint_t *p0, mpz_t k)
{

  unsigned long k_int[6];
  //uint32_t k_int[6];

  char tmp[48];
  size_t size;

  char out_buffer[HW_BUFFSIZE];
  char in_buffer[HW_BUFFSIZE];

  int i, bytes;

  bzero(k_int, 24);
  bzero(tmp, 48);

  mpz_export(k_int,&size,-1,sizeof(ulong_t),-1,0,k); 
  //mpz_export(k_int,&size,-1,sizeof(uint32_t),-1,0,k); 
 
  /* Send the data to the server */  
  memcpy(out_buffer, &p0->x, 24);
  memcpy(out_buffer+24, &p0->y, 24);
  memcpy(out_buffer+48, k_int, 24);

  if ((i=send(sock, out_buffer, 72, 0)) != 72) {
    perror("HW: Mismatch in number of sent bytes");
    return(HW_ERROR);
  }
 
#ifdef HW_DEBUG
  printf("\n");
  for(i=0;i<6;i++) {
    //printf("HW: Sent Px[%02d]: %08lx\n",i, *((unsigned long *) ((out_buffer+i*4))) );
    printf("HW: Sent Px[%02d]: %08x\n",i, *((uint32_t *) ((out_buffer+i*4))) );
  }
  printf("\n");
  for(i=0;i<6;i++) {
    printf("HW: Sent Py[%02d]: %08x\n",i, *((uint32_t *) ((out_buffer+i*4+24))) );
  }
  printf("\n");
  for(i=0;i<6;i++) {
    printf("HW: Sent  k[%02d]: %08x\n",i, *((uint32_t *) ((out_buffer+i*4+48))) );
  }
  printf("\n");
#endif
 
  /* Receive the data back from the server */
  if ((bytes = recv(sock, in_buffer, 48, 0)) < 1) {
    perror("HW: Failed to receive bytes from server");
    return(HW_ERROR);
  }

  /* Copy the received data to the point struct */  
  memcpy(&p1->x, in_buffer, 24);
  memcpy(&p1->y, in_buffer+24, 24);


#ifdef HW_DEBUG
  for(i=0;i<6;i++) {
    printf("HW: Received Qx[%02d]: %x\n",i, *((uint32_t *) ((in_buffer+i*4)))  ); 
  }
  printf("\n");
  for(i=0;i<6;i++) {
    printf("HW: Received Qy[%02d]: %x\n",i, *((uint32_t *) ((in_buffer+i*4+24))) ); 
  }
 
  fprintf(stdout, "\n");
#endif

  return 0;
}
