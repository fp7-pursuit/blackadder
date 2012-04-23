/*-
 * Copyright (C) 2011-2012  Oy L M Ericsson Ab, NomadicLab
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
    else if (TYPE($input) == T_STRING) {
        $1 = ($1_type)(RSTRING_PTR($input));
        $2 = ($2_type)(RSTRING_LEN($input));
    }
    else if (SWIG_CheckConvert($input, SWIGTYPE_p_Malloc_buffer) != 0) {
        malloc_buffer_t *mb;
        int res;
        res = SWIG_ConvertPtr($input, (void **)&mb,
                              SWIGTYPE_p_Malloc_buffer, 0);
        if (!SWIG_IsOK(res)) {
            %argument_fail(res, "(void *BYTES, unsigned int LEN)",
                           $symname, $argnum);
        }
        $1 = ($1_type)mb->data;
        $2 = ($2_type)mb->data_len;
    }
    else {
        %argument_fail(SWIG_TypeError, "(void *BYTES, unsigned int LEN)",
                       $symname, $argnum);
    }
}


// char *BYTE
%typemap(in) char *BYTE {
    // in: char *BYTE
    if ($input == Qnil) {
        $1 = NULL;
    }
    else {
        $1 = ($1_type)(RSTRING_PTR($input));
    }
}

%typemap(memberin) char *BYTE {
    // memberin: char *BYTE
    if ($input)
        memcpy($1, $input, (RSTRING_LEN(res1))); /* XXX: res1 */
}

%typemap(out) char *BYTE {
    // out: char *BYTE
    /* XXX: rb_str_new() allocates and copies memory. */
    /* XXX: We shouldn't use data_len explicitly.     */
    VALUE v = rb_str_new((char *)$1, arg1->data_len); /* XXX: arg1 */
    $result = SWIG_Ruby_AppendOutput($result, v);
}
