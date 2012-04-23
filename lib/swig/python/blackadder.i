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

%module blackadder_py
%{
#include "blackadder.hpp"
#include "nb_blackadder.hpp"
#include "bitvector.hpp"
%}

%include <std_string.i>
// Apply typemaps for "const std::string" to "std::string"
%apply const std::string & { std::string & };
%apply const std::string { std::string };

%include bytestr.i
// Apply typemap for (void *BYTES, unsigned int LEN) to str_opt and data
%apply (void *BYTES, unsigned int LEN) { (void *str_opt, unsigned int str_opt_len), (void *data, unsigned int data_len) };
%apply (void *BYTES, unsigned int LEN) { (const unsigned char *data, unsigned int data_len) };
// Apply typemap for char *BYTE to (e.g.) Event.data
%apply char *BYTE { void *data };

// Python methods for NB_Blackadder class
%include nb_blackadder.i

// Python methods for Bitvector class
%include bitvector.i

%include "blackadder_defs.h"
%include "blackadder.hpp"
%include "nb_blackadder.hpp"
%include "bitvector.hpp"

%inline %{
// Convenience function for creating a read-write buffer object
PyObject *rwbuffer(int len)
{
    return PyBuffer_New(len);
}
%}
