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
/*
 * Copy data_len bytes from data to a newly allocated buffer. This
 * buffer can be given to the publish_data() function in the
 * non-blocking API. The buffer will be freed (asynchronously)
 * by the API library once the packet has been sent.
 */
PyObject *to_malloc_buffer(void *data, unsigned int data_len)
{
    void *buf;

    buf = _to_malloc_buffer(data, data_len);
    if (buf == NULL)
        return NULL; /* XXX */

    return PyBuffer_FromReadWriteMemory(buf, data_len);
}

/* Allocate new empty buffer. */
PyObject *new_malloc_buffer(unsigned int data_len)
{
    return to_malloc_buffer(NULL, data_len);
}

/* Free unsent data. */
void free_malloc_buffer(PyObject *obj)
{
    void *buf = NULL;
    ssize_t ssize;
    int res;

    if (obj == Py_None)
        return; /* XXX */

    if (!PyObject_CheckReadBuffer(obj))
        return; /* XXX */

    res = PyObject_AsReadBuffer(obj, (const void **)&buf, &ssize);
    if (!SWIG_IsOK(res))
        return; /* XXX */

    free(buf);
}
%}
