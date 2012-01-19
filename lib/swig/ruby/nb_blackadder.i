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
            printf("%ld does not respond to %s\n", rb_cb_obj, RSTRING(rb_cb_method)->ptr);
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
