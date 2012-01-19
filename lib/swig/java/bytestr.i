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

// String/char-related typemaps for Java
// See also: <std_string.i> and <various.i>


namespace std {

%naturalvar string;

class string;

// const string &BYTESTR
// byte[] <-> const string &

%typemap(jni) const string &BYTESTR "jbyteArray"
%typemap(jtype) const string &BYTESTR "byte[]"
%typemap(jstype) const string &BYTESTR "byte[]"
%typemap(javadirectorin) const string &BYTESTR "$jniinput"
%typemap(javadirectorout) const string &BYTESTR "$javacall"

%typemap(in) const string &BYTESTR {
    // in: const string &BYTESTR
    if(!$input) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException,
                                "null byte[]");
        return $null;
    }
    char *$1_pstr = (char *)JCALL2(GetByteArrayElements, jenv, $input, NULL);
    ssize_t $1_length = JCALL1(GetArrayLength, jenv, $input);
    std::string *$1_str = new std::string($1_pstr, $1_length);
    $1 = $1_str;
    JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte *)$1_pstr, JNI_ABORT);
}

%typemap(argout) const string &BYTESTR {
    // argout: const string &BYTESTR
}

%typemap(freearg) const string &BYTESTR {
    // freearg: const string &BYTESTR
    if ($1) {
        delete $1;
        $1 = NULL;
    }
}

%typemap(out) const string &BYTESTR {
    // out: const string &BYTESTR
    $result = JCALL1(NewByteArray, jenv, $1->length());
    char *$1_pstr = (char *)JCALL2(GetByteArrayElements, jenv, $result, NULL);
    memcpy($1_pstr, $1->c_str(), $1->length());
    JCALL3(ReleaseByteArrayElements, jenv, $result, (jbyte *)$1_pstr, (jint)0);
}

%typemap(directorout, warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG) const string &
{
    // directorout: const string &BYTESTR
    if(!$input) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null std::string");
        return $null;
    }
    char *$1_pstr = (char *)JCALL2(GetByteArrayElements, jenv, $input, NULL);
    ssize_t $1_length = JCALL1(GetArrayLength, jenv, $input);
    static std::string $1_str; /* XXX: static - not thread-safe */
    $1_str.assign($1_pstr, $1_length);
    $result = &$1_str;
    JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte *)$1_pstr, JNI_ABORT);
}

%typemap(directorin, descriptor="[B") const string & {
    // directorin: const string &BYTESTR
    $input = JCALL1(NewByteArray, jenv, $1.length());
    char *$1_pstr = (char *)JCALL2(GetByteArrayElements, jenv, $input, NULL);
    memcpy($1_pstr, $1.c_str(), $1.length());
    JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte *)$1_pstr, (jint)0);
}

%typemap(javain) const string &BYTESTR "$javainput"

%typemap(javaout) const string &BYTESTR {
    return $jnicall;
}

} /* namespace */


// (void *BYTES, unsigned int LEN)
// byte[] <-> (void *, unsigned int)

/* XXX: Use ByteBuffer instead of byte[] in order to avoid copying? */

%typemap(jni) (void *BYTES, unsigned int LEN) "jbyteArray"
%typemap(jtype) (void *BYTES, unsigned int LEN) "byte[]"
%typemap(jstype) (void *BYTES, unsigned int LEN) "byte[]"

%typemap(in) (void *BYTES, unsigned int LEN) {
    // in: (void *BYTES, unsigned int LEN)
    if(!$input) {
        $1 = NULL;
        $2 = 0;
    }
    else {
        $1 = ($1_type)JCALL2(GetByteArrayElements, jenv, $input, NULL);
        $2 = JCALL1(GetArrayLength, jenv, $input);
    }
}

%typemap(argout) (void *BYTES, unsigned int LEN) {
    // argout: (void *BYTES, unsigned int LEN)
    if ($1) {
        JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte *)$1, (jint)0);
        $1 = NULL;
    }
}

%typemap(javain) (void *BYTES, unsigned int LEN) "$javainput"

%typemap(javaout) (void *BYTES, unsigned int LEN) {
    return $jnicall;
}


// char *BYTE
// byte[] <-> char *
// XXX: Additional typemaps defined in <various.i>

%typemap(javadirectorin) char *BYTE "$jniinput"
%typemap(javadirectorout) char *BYTE "$javacall"

%typemap(memberin) char *BYTE {
    // memberin: char *BYTE
    jsize length = JCALL1(GetArrayLength, jenv, j$input); /* XXX: j$input */
    memcpy($1, $input, length);
}

%typemap(out) char *BYTE {
    // out: char *BYTE
    /* XXX: We shouldn't use data_len explicitly. */
    size_t _data_len = arg1->data_len; /* XXX: arg1 */
    $result = JCALL1(NewByteArray, jenv, _data_len);
    char *$1_pstr = (char *)JCALL2(GetByteArrayElements, jenv, $result, NULL);
    memcpy($1_pstr, $1, _data_len);
    JCALL3(ReleaseByteArrayElements, jenv, $result, (jbyte *)$1_pstr, (jint)0);
}

%typemap(javaout) char *BYTE {
    return $jnicall;
}
