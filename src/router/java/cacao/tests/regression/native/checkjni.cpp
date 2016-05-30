/* src/tests/native/checkjni.c - for testing JNI stuff

   Copyright (C) 1996-2005, 2006, 2008
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#include "config.h"

#include <stdio.h>
#include <string.h>

#include INCLUDE_JNI_MD_H
#include INCLUDE_JNI_H

extern "C" {

JNIEXPORT jboolean JNICALL Java_checkjni_IsAssignableFrom(JNIEnv *env, jclass clazz, jclass sub, jclass sup)
{
  return env->IsAssignableFrom(sub, sup);
}

JNIEXPORT jboolean JNICALL Java_checkjni_IsInstanceOf(JNIEnv *env, jclass clazz, jobject obj, jclass c)
{
  return env->IsInstanceOf(obj, c);
}

JNIEXPORT jboolean JNICALL Java_checkjni_IsSameObject(JNIEnv *env, jclass clazz, jobject obj1, jobject obj2)
{
  return env->IsSameObject(obj1, obj2);
}

JNIEXPORT jint     JNICALL Java_checkjni_PushLocalFrame(JNIEnv *env, jclass clazz, jint capacity)
{
  return env->PushLocalFrame(capacity);
}

JNIEXPORT void     JNICALL Java_checkjni_Throw(JNIEnv *env, jclass clazz)
{
	jclass c = env->FindClass("java/lang/Exception");
	env->ThrowNew(c, "Exception from JNI");
}

JNIEXPORT jclass   JNICALL Java_checkjni_GetObjectClass(JNIEnv *env, jclass clazz, jobject obj)
{
	return env->GetObjectClass(obj);
}

#define CHECKJNI_GET_FIELD(type, name, fieldname, sig)                                            \
JNIEXPORT type     JNICALL Java_checkjni_Get##name##Field(JNIEnv *env, jclass clazz, jobject obj) \
{                                                                                                 \
	jclass c    = env->GetObjectClass(obj);                                                       \
	jfieldID id = env->GetFieldID(c, fieldname, sig);                                             \
                                                                                                  \
	if (env->ExceptionCheck()) {                                                                  \
		return 0;                                                                                 \
	} else {                                                                                      \
		return env->Get##name##Field(obj, id);                                                    \
	}                                                                                             \
}
CHECKJNI_GET_FIELD(jint,    Int,    "jfI", "I")
CHECKJNI_GET_FIELD(jobject, Object, "jfL", "Ljava/lang/Object;")

#define CHECKJNI_GET_STATIC_FIELD(type, name, fieldname, sig)                              \
JNIEXPORT type     JNICALL Java_checkjni_GetStatic##name##Field(JNIEnv *env, jclass clazz) \
{                                                                                          \
	jfieldID id = env->GetStaticFieldID(clazz, fieldname, sig);                            \
                                                                                           \
	if (env->ExceptionCheck()) {                                                           \
		return 0;                                                                          \
	} else {                                                                               \
		return env->GetStatic##name##Field(clazz, id);                                     \
	}                                                                                      \
}
CHECKJNI_GET_STATIC_FIELD(jint,    Int,    "jsfI", "I")
CHECKJNI_GET_STATIC_FIELD(jobject, Object, "jsfL", "Ljava/lang/Object;")

#define CHECKJNI_SET_STATIC_FIELD(type, name, fieldname, sig)                                    \
JNIEXPORT type JNICALL Java_checkjni_SetStatic##name##Field(JNIEnv *env, jclass clazz, type val) \
{                                                                                                \
	jfieldID id = env->GetStaticFieldID(clazz, fieldname, sig);                                  \
	                                                                                             \
	if (env->ExceptionCheck()) {                                                                 \
		return 0;                                                                                \
	} else {                                                                                     \
		env->SetStatic##name##Field(clazz, id, val);                                             \
		return val;                                                                              \
	}                                                                                            \
}
CHECKJNI_SET_STATIC_FIELD(jint,    Int,    "jsfI", "I")
CHECKJNI_SET_STATIC_FIELD(jobject, Object, "jsfL", "Ljava/lang/Object;")

#define CHECKJNI_NEW_ARRAY(type, name) \
JNIEXPORT type##Array JNICALL Java_checkjni_New##name##Array(JNIEnv *env, jclass clazz, jint size) \
{                                                                                                  \
	jboolean iscopy;                                                                               \
                                                                                                   \
	type##Array  a = env->New##name##Array(size);                                                  \
	type        *p = env->Get##name##ArrayElements(a, &iscopy);                                    \
                                                                                                   \
	for (int i = 0; i < size; i++) p[i] = i;                                                       \
                                                                                                   \
	env->Release##name##ArrayElements(a, p, 0);                                                    \
	return a;                                                                                      \
}
CHECKJNI_NEW_ARRAY(jint,    Int)
CHECKJNI_NEW_ARRAY(jlong,   Long)

JNIEXPORT jstring  JNICALL Java_checkjni_NewString(JNIEnv *env, jclass clazz, jint type)
{
	if (type == 1)
		return env->NewString((jchar *) "Test String from JNI", (jsize) strlen("Test String from JNI"));
	else
		return env->NewStringUTF("Test String from JNI with UTF");
}

} // extern "C"

