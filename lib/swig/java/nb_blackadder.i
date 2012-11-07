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
     * Note: This Java callback function is static, not specific to
     *       a Blackadder instance.
     */
    static JavaVM *_java_cb_vm = NULL;
    static jclass _java_cb_cls = NULL;
    static jmethodID _java_cb_mid = NULL;
    static void _cxx_java_callback(Event *ev) {
        if (_java_cb_vm && _java_cb_cls && _java_cb_mid) {
            JNIEnv *env = NULL;
            _java_cb_vm->AttachCurrentThread((void **)&env, NULL);

            jclass cls = _java_cb_cls;
            jmethodID mid = _java_cb_mid;
            env->CallStaticVoidMethod(cls, mid, (jlong)ev);

            _java_cb_vm->DetachCurrentThread();
        } else {
            delete ev; /* XXX */
        }
    }
%}

%typemap(in, numinputs=0) (JNIEnv *_ba_env) {
    $1 = jenv; /* XXX */
}

%extend NB_Blackadder {
    void setJavaCallback(jobject obj, const char *mname, JNIEnv *_ba_env) {
        JavaVM *vm  = NULL;
        _ba_env->GetJavaVM(&vm);
        if (vm == NULL)
            return;

        jclass cls = _ba_env->GetObjectClass(obj);
        if (cls == NULL)
            return;
        jmethodID mid = _ba_env->GetStaticMethodID(cls, mname, "(J)V");
        if (mid == NULL)
            return;

        _java_cb_vm = vm;
        _java_cb_cls = cls;
        _java_cb_mid = mid;

        $self->setCallback(_cxx_java_callback);
    }

    void testJavaCallback(Event *ev) {
        $self->cf(ev);
    }
}
