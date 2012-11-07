/*
* Copyright (c) 2008, Juha Forsten <juha.forsten@hut.fi>, Kimmo Järvinen <kimmo.jarvinen@hut.fi>,  TKK
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

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "ec.h"

#define HW_BUFFSIZE 256
#define HW_HOST "10.0.0.3"
#define HW_PORT "30"

#define HW_ERROR 1
#define HW_OK 0

#define HW_DEBUG 0

int ec_hw_open(int *sock);
int ec_hw_close(int sock);

int ec_hw_multiply(int sock, ecpoint_t *p1, ecpoint_t *p0, mpz_t k);
