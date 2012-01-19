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
     * Note: This Python callback function is static, not specific to
     *       a Blackadder instance.
     */
    static PyObject *_py_callback = NULL;
    static void _cxx_py_callback(Event *ev) {
        if (_py_callback) {
            SWIG_PYTHON_THREAD_BEGIN_BLOCK;
            PyObject *args = PyTuple_New(1);
            PyObject *evobj = SWIG_NewPointerObj(SWIG_as_voidptr(ev), SWIGTYPE_p_Event, SWIG_POINTER_NEW |  0 );
            Py_XINCREF(evobj); /* XXX */
            PyTuple_SetItem(args, 0, evobj);

            PyObject *result = PyObject_CallObject(_py_callback, args);

            if (result == NULL && PyErr_Occurred() != NULL) {
                PyErr_WriteUnraisable(_py_callback);
            }
            Py_XDECREF(result);
            Py_XDECREF(args);
            SWIG_PYTHON_THREAD_END_BLOCK;
        }
        delete ev; /* XXX */
    }
%}

%extend NB_Blackadder {
    void setPyCallback(PyObject *py_callback) {
        if (!PyCallable_Check(py_callback)) {
            /* XXX: Raise exception here. */
            return;
        }
        Py_XINCREF(py_callback);
        Py_XDECREF(_py_callback);
        _py_callback = py_callback;
        $self->setCallback(_cxx_py_callback);
    }

    void testPyCallback(Event *ev) {
        $self->cf(ev);
    }
}
