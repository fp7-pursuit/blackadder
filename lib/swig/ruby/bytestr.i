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

// String/char-related typemaps for Ruby


// (void *BYTES, unsigned int LEN)
// buffer -> (void *, unsigned int)

%typemap(in) (void *BYTES, unsigned int LEN) {
    // in: (void *BYTES, unsigned int LEN)
    if ($input == Qnil) {
        $1 = NULL;
        $2 = 0;
    }
    else {
        $1 = ($1_type)(RSTRING($input))->ptr;
        $2 = ($2_type)(RSTRING($input))->len;
    }    
}


// char *BYTE
%typemap(in) char *BYTE {
    // in: char *BYTE
    if ($input == Qnil) {
        $1 = NULL;
    }
    else {
        $1 = ($1_type)(RSTRING($input))->ptr;
    }
}

%typemap(memberin) char *BYTE {
    // memberin: char *BYTE
    if ($input)
        memcpy($1, $input, (RSTRING(res1))->len); /* XXX: res1 */
}

%typemap(out) char *BYTE {
    // out: char *BYTE
    /* XXX: rb_str_new() allocates and copies memory. */
    /* XXX: We shouldn't use data_len explicitly.     */
    VALUE v = rb_str_new((char *)$1, arg1->data_len); /* XXX: arg1 */
    $result = SWIG_Ruby_AppendOutput($result, v);
}
