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

// String/char-related typemaps for Python


// (void *BYTES, unsigned int LEN)
// buffer -> (void *, unsigned int)

%typemap(in) (void *BYTES, unsigned int LEN) {
    // in: (void *BYTES, unsigned int LEN)
    void *buf = NULL;
    ssize_t ssize;
    int res;

    if ($input == Py_None) {
        $1 = NULL;
        $2 = 0;
    }
    else {
        if (PyObject_CheckReadBuffer($input))
            res = PyObject_AsReadBuffer($input, (const void **)&buf, &ssize);
        else
            res = -1;
        if (!SWIG_IsOK(res)) {
            %argument_fail(res, "(void *BYTES, unsigned int LEN)", $symname, $argnum);
        }
        $1 = (uint8_t *)buf;
        $2 = ($2_type)ssize;
    }
}


// char *BYTE
// buffer <-> char *

%typemap(in) char *BYTE {
    // in: char *BYTE
    void *buf = NULL;
    ssize_t ssize = 0;
    int res;

    if ($input == Py_None)
        $1 = NULL;
    else {
        if (PyObject_CheckReadBuffer($input))
            res = PyObject_AsReadBuffer($input, (const void **)&buf, &ssize);
        else
            res = -1;
        if (!SWIG_IsOK(res)) {
            %argument_fail(res, "char *BYTE", $symname, $argnum);
        }
        $1 = ($1_type)buf;
    }
}

%typemap(memberin) char *BYTE {
    // memberin: char *BYTE
    if ($input)
        memcpy($1, $input, PyObject_Length(obj1)); /* XXX: obj1 */
}

%typemap(out) char *BYTE {
    // out: char *BYTE
    /* Note: We return a buffer that can be used more or less like a string. */
    /* XXX:  We shouldn't use data_len explicitly. */
    PyObject *obj = PyBuffer_FromMemory($1, arg1->data_len); /* XXX: arg1 */
    $result = SWIG_Python_AppendOutput($result, obj);
}
