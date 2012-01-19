/*
 * Copyright (C) 2011  Christos Tsilopoulos, Mobile Multimedia Laboratory, 
 * Athens University of Economics and Business 
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of
 * the BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

#include "eu_pursuit_client_BlackadderWrapper.h"
#include <blackadder.hpp>
#include <string>
#include <stdio.h>

using std::string;

void print_contents(char *data_ptr, int length){
	for(int i=0; i<length; i++){
		printf("%d ", data_ptr[i]);	
	}
	printf("\n");
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    create_new_ba
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_eu_pursuit_client_BlackadderWrapper_create_1new_1ba
  (JNIEnv *, jobject, jint userspace){
	bool user = userspace? true : false;
	printf("create new instance\n");
	Blackadder *ba_ptr = Blackadder::Instance(user);
	return (jlong) ba_ptr;
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    delete_ba
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_delete_1ba
  (JNIEnv *, jobject, jlong ba_ptr){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;
	ba->disconnect();
	delete ba;
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_publish_scope
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1publish_1scope
  (JNIEnv *env, jobject obj, jlong ba_ptr, jbyteArray scope, jbyteArray scope_prefix, jbyte strategy, jbyteArray jstr_opt){

	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * scope_ptr = (*env).GetByteArrayElements(scope, &copy);
	unsigned int scope_length = (*env).GetArrayLength(scope);
        const string scope_str ((char *)scope_ptr, scope_length);

	jbyte *scope_prefix_ptr = (*env).GetByteArrayElements(scope_prefix, &copy);
	unsigned int scope_prefix_length = (*env).GetArrayLength(scope_prefix);
        const string scope_prefix_str ((char *)scope_prefix_ptr, scope_prefix_length);

	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}

	/*
	 * the call to blackadder 
	 */
	ba->publish_scope(scope_str, scope_prefix_str, (char)strategy, str_opt, str_opt_len);

        /*
         * XXX: Because the ID strings are expected to be constant,
         *      ReleaseByteArrayElements() could be called with
         *      JNI_ABORT instead of 0.
         */
	(*env).ReleaseByteArrayElements(scope, scope_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(scope_prefix, scope_prefix_ptr, (jint)0);	
	
	if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_publish_item
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1publish_1item
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray rid, jbyteArray sid, jbyte strategy, jbyteArray jstr_opt){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * rid_ptr = (*env).GetByteArrayElements(rid, &copy);
	int rid_length = (*env).GetArrayLength(rid);	
        const string rid_str ((char *)rid_ptr, rid_length);

	jbyte *sid_ptr = (*env).GetByteArrayElements(sid, &copy);
	int sid_length = (*env).GetArrayLength(sid);
        const string sid_str ((char *)sid_ptr, sid_length);

	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}

	/*
	 * the call to blackadder 
	 */
	ba->publish_info(rid_str, sid_str, (char)strategy, str_opt, str_opt_len);

	(*env).ReleaseByteArrayElements(rid, rid_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(sid, sid_ptr, (jint)0);	
	
	if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_unpublish_scope
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1unpublish_1scope
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray scope, jbyteArray scope_prefix, jbyte strategy, jbyteArray jstr_opt){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * scope_ptr = (*env).GetByteArrayElements(scope, &copy);
	int scope_length = (*env).GetArrayLength(scope);
        const string scope_str ((char *)scope_ptr, scope_length);

	jbyte *scope_prefix_ptr = (*env).GetByteArrayElements(scope_prefix, &copy);
	int scope_prefix_length = (*env).GetArrayLength(scope_prefix);
        const string scope_prefix_str ((char *)scope_prefix_ptr, scope_prefix_length);

	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}
	
	/*
	 * the call to blackadder 
	 */
	ba->unpublish_scope(scope_str, scope_prefix_str, (char)strategy, str_opt, str_opt_len);

	(*env).ReleaseByteArrayElements(scope, scope_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(scope_prefix, scope_prefix_ptr, (jint)0);	
	
	if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_unpublish_item
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1unpublish_1item
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray rid, jbyteArray sid, jbyte strategy, jbyteArray jstr_opt){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * rid_ptr = (*env).GetByteArrayElements(rid, &copy);
	int rid_length = (*env).GetArrayLength(rid);
	//string id = rid_length == 0? "" : std::string((char *)rid_ptr, rid_length);
        const string rid_str ((char *)rid_ptr, rid_length);

	jbyte *sid_ptr = (*env).GetByteArrayElements(sid, &copy);
	int sid_length = (*env).GetArrayLength(sid);
	//string prefix = sid_length == 0? "" : std::string((char *)sid_ptr, sid_length);
        const string sid_str ((char *)sid_ptr, sid_length);

	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}

	/*
	 * the call to blackadder 
	 */
	ba->unpublish_info(rid_str, sid_str, (char)strategy, str_opt, str_opt_len);

	(*env).ReleaseByteArrayElements(rid, rid_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(sid, sid_ptr, (jint)0);	
	
	if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_subscribe_scope
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1subscribe_1scope
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray scope, jbyteArray scope_prefix, jbyte strategy, jbyteArray jstr_opt){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * scope_ptr = (*env).GetByteArrayElements(scope, &copy);
	int scope_length = (*env).GetArrayLength(scope);
        const string scope_str ((char *)scope_ptr, scope_length);

	jbyte *scope_prefix_ptr = (*env).GetByteArrayElements(scope_prefix, &copy);
	int scope_prefix_length = (*env).GetArrayLength(scope_prefix);
        const string scope_prefix_str ((char *)scope_prefix_ptr, scope_prefix_length);

	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}
	
	/*
	 * the call to blackadder 
	 */
	ba->subscribe_scope(scope_str, scope_prefix_str, (char)strategy, str_opt, str_opt_len);

	(*env).ReleaseByteArrayElements(scope, scope_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(scope_prefix, scope_prefix_ptr, (jint)0);	
	
	if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_subscribe_item
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1subscribe_1item
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray rid, jbyteArray sid, jbyte strategy, jbyteArray jstr_opt){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * rid_ptr = (*env).GetByteArrayElements(rid, &copy);
	int rid_length = (*env).GetArrayLength(rid);
        const string rid_str ((char *)rid_ptr, rid_length);

	jbyte *sid_ptr = (*env).GetByteArrayElements(sid, &copy);
	int sid_length = (*env).GetArrayLength(sid);
        const string sid_str ((char *)sid_ptr, sid_length);

	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}

	/*
	 * the call to blackadder 
	 */
	ba->subscribe_info(rid_str, sid_str, (char)strategy, str_opt, str_opt_len);

	(*env).ReleaseByteArrayElements(rid, rid_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(sid, sid_ptr, (jint)0);	
	
	if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_unsubscribe_scope
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1unsubscribe_1scope
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray scope, jbyteArray scope_prefix, jbyte strategy, jbyteArray jstr_opt){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * scope_ptr = (*env).GetByteArrayElements(scope, &copy);
	int scope_length = (*env).GetArrayLength(scope);
        const string scope_str ((char *)scope_ptr, scope_length);

	jbyte *scope_prefix_ptr = (*env).GetByteArrayElements(scope_prefix, &copy);
	int scope_prefix_length = (*env).GetArrayLength(scope_prefix);
        const string scope_prefix_str ((char *)scope_prefix_ptr, scope_prefix_length);

	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}
	
	/*
	 * the call to blackadder 
	 */
	ba->unsubscribe_scope(scope_str, scope_prefix_str, (char)strategy, str_opt, str_opt_len);

	(*env).ReleaseByteArrayElements(scope, scope_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(scope_prefix, scope_prefix_ptr, (jint)0);	
	
	if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_unsubscribe_item
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1unsubscribe_1item
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray rid, jbyteArray sid, jbyte strategy, jbyteArray jstr_opt){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * rid_ptr = (*env).GetByteArrayElements(rid, &copy);
	int rid_length = (*env).GetArrayLength(rid);
	//string id = rid_length == 0? "" : std::string((char *)rid_ptr, rid_length);
        const string rid_str ((char *)rid_ptr, rid_length);

	jbyte *sid_ptr = (*env).GetByteArrayElements(sid, &copy);
	int sid_length = (*env).GetArrayLength(sid);
	//string prefix = sid_length == 0? "" : std::string((char *)sid_ptr, sid_length);
        const string sid_str ((char *)sid_ptr, sid_length);

	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}

	/*
	 * the call to blackadder 
	 */
	ba->unsubscribe_info(rid_str, sid_str, (char)strategy, str_opt, str_opt_len);

	(*env).ReleaseByteArrayElements(rid, rid_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(sid, sid_ptr, (jint)0);	
	
	if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_publish_data_direct
 * Signature: (J[BB[BLjava/nio/ByteBuffer;I)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1publish_1data_1direct
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray name, jbyte strategy, jbyteArray jstr_opt, jobject jbytebuffer, jint length){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;


	jboolean copy = (jboolean)false;
	jbyte *name_ptr = (*env).GetByteArrayElements(name, &copy);
	int name_length = (*env).GetArrayLength(name);
        const string name_str ((char *)name_ptr, name_length);
	
	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}

	char *data_ptr = (char *)(*env).GetDirectBufferAddress(jbytebuffer);	
	
	ba->publish_data(name_str, (char)strategy, str_opt, str_opt_len, (char *)data_ptr, (int)length);
	
	(*env).ReleaseByteArrayElements(name, name_ptr, (jint)0);
	
        if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_publish_data
 * Signature: (J[BB[B[BI)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1publish_1data
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray name, jbyte strategy, jbyteArray jstr_opt, jbyteArray data, jint datalen){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte *name_ptr = (*env).GetByteArrayElements(name, &copy);
	int name_length = (*env).GetArrayLength(name);
        const string name_str ((char *)name_ptr, name_length);
	
	void *str_opt;
	str_opt = 0;
	unsigned int str_opt_len = 0;

	if(jstr_opt != NULL && (str_opt_len = (*env).GetArrayLength(jstr_opt)) > 0) {
		str_opt = (void *) (*env).GetByteArrayElements(jstr_opt, &copy);		
	}

	jbyte *data_ptr = (*env).GetByteArrayElements(data, &copy);
//	print_contents((char *)data_ptr, (int)datalen);	

	ba->publish_data(name_str, (char)strategy, str_opt, str_opt_len, (char *)data_ptr, (int)datalen);

	(*env).ReleaseByteArrayElements(name, name_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(data, data_ptr, (jint)0);
	
        if(jstr_opt != 0){	
		(*env).ReleaseByteArrayElements(jstr_opt, (jbyte *)str_opt, (jint)0);		
	}	
}


/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_nextEvent_direct
 * Signature: (JLeu/pursuit/client/EventInternal;)J
 */
JNIEXPORT jlong JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1nextEvent_1direct
  (JNIEnv *env, jobject, jlong ba_ptr, jobject obj_EventInternal){

	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;
	Event *ev = new Event();
        ba->getEvent(*ev);

	static jclass cls = (*env).GetObjectClass(obj_EventInternal);
	
	static jfieldID evTypeField = (*env).GetFieldID(cls, "type", "B");
	if (evTypeField == NULL) {
		printf("JNI: error getting field id for Event.type\n");
		return 0;
	}

	static	jfieldID evIdField = (*env).GetFieldID(cls, "id", "[B");
	if (evTypeField == NULL) {
		printf("JNI: error getting field id for Event.id\n");
		return 0;
	}

	static	jfieldID evDataField = (*env).GetFieldID(cls, "data", "Ljava/nio/ByteBuffer;");
	if (evDataField == NULL) {
		printf("JNI: error getting field id for Event.data\n");
		return 0;
	}	

	//copy type byte
	(*env).SetByteField(obj_EventInternal, evTypeField, (jbyte) ev->type);

	//copy id array
	const char *id_ptr = ev->id.c_str();	
	jbyteArray idArray = (*env).NewByteArray(ev->id.length());
	(*env).SetByteArrayRegion(idArray, 0, ev->id.length(), (jbyte *) id_ptr);
	(*env).SetObjectField(obj_EventInternal, evIdField, idArray);

	jobject jbuff = (*env).NewDirectByteBuffer((void*)ev->data, (jlong) ev->data_len); 
	(*env).SetObjectField(obj_EventInternal, evDataField, jbuff);
	
	return (jlong)ev;
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_delete_event
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1delete_1event
  (JNIEnv *, jobject, jlong ba_ptr, jlong ev_ptr){
	Event *event = (Event *)ev_ptr;
	delete event;
}


