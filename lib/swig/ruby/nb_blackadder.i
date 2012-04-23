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

%{
    /*
     * Note: This Ruby callback function is static, not specific to
     *       a Blackadder instance.
     */
    static VALUE _rb_cb_obj = NULL;
    static ID _rb_cb_method = NULL;
    static void _cxx_rb_callback(Event *ev) {
        if (_rb_cb_obj && _rb_cb_method) {
            VALUE evobj = SWIG_NewPointerObj(SWIG_as_voidptr(ev), SWIGTYPE_p_Event, 0 |  0 );

            rb_funcall(_rb_cb_obj, _rb_cb_method, 1, evobj);
            // ...
        }
        delete ev; /* XXX */
    }
%}

%extend NB_Blackadder {
    void setRubyCallback(VALUE rb_cb_obj, VALUE rb_cb_method) {
        ID method = rb_intern(RSTRING_PTR(rb_cb_method));
        if (!rb_respond_to(rb_cb_obj, method)) {
            /* XXX: Raise exception here. */
            printf("%ld does not respond to %s\n", rb_cb_obj,
                   RSTRING_PTR(rb_cb_method));
            return;
        }
        _rb_cb_obj = rb_cb_obj;
        _rb_cb_method = method;
        $self->setCallback(_cxx_rb_callback);
    }

    void testRubyCallback(Event *ev) {
        $self->cf(ev);
    }
}

%{
void *_to_malloc_buffer(void *data, unsigned int data_len)
{
    void *buf;

    if (data_len < 0)
        return NULL;

    buf = malloc(data_len);
    if (buf == NULL)
        return NULL; /* XXX */

    if (data != NULL)
        memcpy(buf, data, data_len);
    else
        memset(buf, 0, data_len);

    return buf;
}
%}

%inline %{
struct Malloc_buffer {
    void *data;
    int data_len;
};
typedef struct Malloc_buffer malloc_buffer_t;

/*
 * Copy data_len bytes from data to a newly allocated buffer. This
 * buffer can be given to the publish_data() function in the
 * non-blocking API. The buffer will be freed (asynchronously)
 * by the API library once the packet has been sent.
 */
malloc_buffer_t to_malloc_buffer(void *data, unsigned int data_len)
{
    void *buf;
    malloc_buffer_t mb = { NULL, 0 };

    buf = _to_malloc_buffer(data, data_len);
    if (buf != NULL) {
        mb.data = buf;
        mb.data_len = data_len;
    }

    return mb;
}

/* Free unsent data */
void malloc_buffer_free(malloc_buffer_t mb)
{
    if (mb.data != NULL) {
        free(mb.data); /* XXX */
        mb.data = NULL;
        mb.data_len = 0;
    }
}
%}
