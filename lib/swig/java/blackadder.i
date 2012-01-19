/*-
 * Copyright (C) 2011  Oy L M Ericsson Ab, NomadicLab
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of the
 * BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

%module BA
%{
#include "blackadder.hpp"
//#include "nb_blackadder.hpp"
#include "bitvector.hpp"
%}

typedef unsigned long  uint64_t;
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
typedef long  int64_t;
typedef int   int32_t;
typedef short int16_t;
typedef char  int8_t;

%apply int { unsigned char strategy }; // the corresponding constants are ints

%include <various.i> // typemaps for BYTE
%include bytestr.i // additional typemaps for BYTE (and BYTESTR and BYTES)

// Use byte arrays as ID strings (due to encoding issues)
%apply const std::string &BYTESTR { const std::string &id, const std::string &prefix_id };

// Apply typemap for (void *BYTES, unsigned int LEN) to str_opt and data
%apply (void *BYTES, unsigned int LEN) { (void *data, unsigned int data_len), (void *str_opt, unsigned int str_opt_len) };
// Apply typemap for "char *BYTE" to (e.g.) Event.data
%apply char *BYTE { void *data };

%include "blackadder_defs.h"
%include "blackadder.hpp"
//%include "nb_blackadder.hpp"
%include "bitvector.hpp"
