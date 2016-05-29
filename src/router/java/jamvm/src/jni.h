/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __JNI_H__
#define __JNI_H__
#include <stdio.h>
#include <stdarg.h>

#define JNI_VERSION_1_1 0x00010001
#define JNI_VERSION_1_2 0x00010002
#define JNI_VERSION_1_4 0x00010004
#define JNI_VERSION_1_6 0x00010006

#define JNI_FALSE 0
#define JNI_TRUE 1

#define JNI_OK 0
#define JNI_ERR (-1)
#define JNI_EDETACHED (-2)
#define JNI_EVERSION (-3)

#define JNI_COMMIT 1
#define JNI_ABORT 2

#define JNIEXPORT
#define JNICALL

typedef int             jint;
typedef long long       jlong;
typedef signed char     jbyte;   
typedef unsigned char   jboolean;
typedef unsigned short  jchar;
typedef short           jshort;
typedef float           jfloat;
typedef double          jdouble;
typedef jint            jsize;

typedef void *jobject;
typedef jobject jclass;
typedef jobject jthrowable;
typedef jobject jstring;
typedef jobject jarray;
typedef jarray jbooleanArray;
typedef jarray jbyteArray;
typedef jarray jcharArray;
typedef jarray jshortArray;
typedef jarray jintArray;
typedef jarray jlongArray;
typedef jarray jfloatArray;
typedef jarray jdoubleArray;
typedef jarray jobjectArray;

typedef jobject jweak;

typedef union jvalue {
    jboolean z;
    jbyte    b;
    jchar    c;
    jshort   s;
    jint     i;
    jlong    j;
    jfloat   f;
    jdouble  d;
    jobject  l;
} jvalue;

typedef struct {
    char *name;
    char *signature;
    void *fnPtr;
} JNINativeMethod;

typedef void *jfieldID;
typedef void *jmethodID;

struct _JNINativeInterface;
typedef const struct _JNINativeInterface *JNIEnv;

struct _JNIInvokeInterface;
typedef const struct _JNIInvokeInterface *JavaVM;

enum _jobjectRefType
{
  JNIInvalidRefType    = 0,
  JNILocalRefType      = 1,
  JNIGlobalRefType     = 2,
  JNIWeakGlobalRefType = 3 
};

typedef enum _jobjectRefType jobjectRefType;

#define VIRTUAL_METHOD(type, native_type)                                                   \
native_type (*Call##type##Method)(JNIEnv *env, jobject obj, jmethodID mID, ...);            \
native_type (*Call##type##MethodV)(JNIEnv *env, jobject obj, jmethodID mID, va_list jargs); \
native_type (*Call##type##MethodA)(JNIEnv *env, jobject obj, jmethodID mID, jvalue *jargs);

#define NONVIRTUAL_METHOD(type, native_type)                                                \
native_type (*CallNonvirtual##type##Method)(JNIEnv *env, jobject obj, jclass clazz,         \
                jmethodID methodID, ...);                                                   \
native_type (*CallNonvirtual##type##MethodV)(JNIEnv *env, jobject obj, jclass clazz,        \
                jmethodID methodID, va_list jargs);                                         \
native_type (*CallNonvirtual##type##MethodA)(JNIEnv *env, jobject obj, jclass clazz,        \
                jmethodID methodID, jvalue *jargs);

#define STATIC_METHOD(type, native_type)                                                    \
native_type (*CallStatic##type##Method)(JNIEnv *env, jclass clazz, jmethodID methodID, ...);\
native_type (*CallStatic##type##MethodV)(JNIEnv *env, jclass clazz, jmethodID methodID,     \
                va_list jargs);                                                             \
native_type (*CallStatic##type##MethodA)(JNIEnv *env, jclass clazz, jmethodID methodID,     \
                jvalue *jargs);

#define VOID_VIRTUAL_METHOD                                                                 \
void (*CallVoidMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);                  \
void (*CallVoidMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list jargs);       \
void (*CallVoidMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *jargs);       \

#define VOID_NONVIRTUAL_METHOD                                                              \
void (*CallNonvirtualVoidMethod)(JNIEnv *env, jobject obj, jclass clazz,                    \
                jmethodID methodID, ...);                                                   \
void (*CallNonvirtualVoidMethodV)(JNIEnv *env, jobject obj, jclass clazz,                   \
                jmethodID methodID, va_list jargs);                                         \
void (*CallNonvirtualVoidMethodA)(JNIEnv *env, jobject obj, jclass clazz,                   \
                jmethodID methodID, jvalue *jargs);

#define VOID_STATIC_METHOD                                                                  \
void (*CallStaticVoidMethod)(JNIEnv *env, jclass clazz, jmethodID methodID, ...);           \
void (*CallStaticVoidMethodV)(JNIEnv *env, jclass clazz, jmethodID methodID, va_list jargs);\
void (*CallStaticVoidMethodA)(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *jargs);

#define CALL_METHOD(access)         \
access##_METHOD(Object, jobject);   \
access##_METHOD(Boolean, jboolean); \
access##_METHOD(Byte, jbyte);       \
access##_METHOD(Char, jchar);       \
access##_METHOD(Short, jshort);     \
access##_METHOD(Int, jint);         \
access##_METHOD(Long, jlong);       \
access##_METHOD(Float, jfloat);     \
access##_METHOD(Double, jdouble);   \
VOID_##access##_METHOD;


#define NEW_PRIM_ARRAY(type, native_type, array_type) \
native_type##Array (*New##type##Array)(JNIEnv *env, jsize length);

#define GET_ELEMENTS_PRIM_ARRAY(type, native_type, array_type) \
native_type *(*Get##type##ArrayElements)(JNIEnv *env, native_type##Array array, jboolean *isCopy);

#define RELEASE_ELEMENTS_PRIM_ARRAY(type, native_type, array_type) \
void (*Release##type##ArrayElements)(JNIEnv *env, native_type##Array array, native_type *elems, jint mode);

#define GET_REGION_PRIM_ARRAY(type, native_type, array_type) \
void (*Get##type##ArrayRegion)(JNIEnv *env, native_type##Array array, jsize start, jsize len, native_type *buf);

#define SET_REGION_PRIM_ARRAY(type, native_type, array_type) \
void (*Set##type##ArrayRegion)(JNIEnv *env, native_type##Array array, jsize start, jsize len, native_type *buf);

#define PRIM_ARRAY_OP(op)                      \
op##_PRIM_ARRAY(Boolean, jboolean, T_BOOLEAN); \
op##_PRIM_ARRAY(Byte, jbyte, T_BYTE);          \
op##_PRIM_ARRAY(Char, jchar, T_CHAR);          \
op##_PRIM_ARRAY(Short, jshort, T_SHORT);       \
op##_PRIM_ARRAY(Int, jint, T_INT);             \
op##_PRIM_ARRAY(Long, jlong, T_LONG);          \
op##_PRIM_ARRAY(Float, jfloat, T_FLOAT);       \
op##_PRIM_ARRAY(Double, jdouble, T_DOUBLE);


#define GET_FIELD(type, native_type) \
native_type (*Get##type##Field)(JNIEnv *env, jobject obj, jfieldID fieldID);

#define SET_FIELD(type, native_type) \
void (*Set##type##Field)(JNIEnv *env, jobject obj, jfieldID fieldID, native_type value);

#define GET_STATIC_FIELD(type, native_type) \
native_type (*GetStatic##type##Field)(JNIEnv *env, jclass clazz, jfieldID fieldID);

#define SET_STATIC_FIELD(type, native_type) \
void (*SetStatic##type##Field)(JNIEnv *env, jclass clazz, jfieldID fieldID, native_type value);

#define FIELD_OP(op)           \
op##_FIELD(Object, jobject);   \
op##_FIELD(Boolean, jboolean); \
op##_FIELD(Byte, jbyte);       \
op##_FIELD(Char, jchar);       \
op##_FIELD(Short, jshort);     \
op##_FIELD(Int, jint);         \
op##_FIELD(Long, jlong);       \
op##_FIELD(Float, jfloat);     \
op##_FIELD(Double, jdouble);


struct _JNINativeInterface {
    void *reserved0;
    void *reserved1;
    void *reserved2;
    void *reserved3;

    jint (*GetVersion)(JNIEnv *env);

    jclass (*DefineClass)(JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize len);
    jclass (*FindClass)(JNIEnv *env, const char *name);

    jmethodID (*FromReflectedMethod)(JNIEnv *env, jobject method);
    jfieldID (*FromReflectedField)(JNIEnv *env, jobject field);
    jobject (*ToReflectedMethod)(JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic);

    jclass (*GetSuperclass)(JNIEnv *env, jclass sub);
    jboolean (*IsAssignableFrom)(JNIEnv *env, jclass sub, jclass sup);

    jobject (*ToReflectedField)(JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic);

    jint (*Throw)(JNIEnv *env, jthrowable obj);
    jint (*ThrowNew)(JNIEnv *env, jclass clazz, const char *msg);

    jthrowable (*ExceptionOccurred)(JNIEnv *env);
    void (*ExceptionDescribe)(JNIEnv *env);
    void (*ExceptionClear)(JNIEnv *env);
    void (*FatalError)(JNIEnv *env, const char *msg);

    jint (*PushLocalFrame)(JNIEnv *env, jint capacity);
    jobject (*PopLocalFrame)(JNIEnv *env, jobject result);

    jobject (*NewGlobalRef)(JNIEnv *env, jobject obj);
    void (*DeleteGlobalRef)(JNIEnv *env, jobject obj);
    void (*DeleteLocalRef)(JNIEnv *env, jobject obj);
    jboolean (*IsSameObject)(JNIEnv *env, jobject obj1, jobject obj2);

    jobject (*NewLocalRef)(JNIEnv *env, jobject obj);
    jint (*EnsureLocalCapacity)(JNIEnv *env, jint capacity);

    jobject (*AllocObject)(JNIEnv *env, jclass clazz);
    jobject (*NewObject)(JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jobject (*NewObjectV)(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jobject (*NewObjectA)(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jclass (*GetObjectClass)(JNIEnv *env, jobject obj);
    jboolean (*IsInstanceOf)(JNIEnv *env, jobject obj, jclass clazz);

    jmethodID (*GetMethodID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);

    CALL_METHOD(VIRTUAL);
    CALL_METHOD(NONVIRTUAL);

    jfieldID (*GetFieldID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);

    FIELD_OP(GET);
    FIELD_OP(SET);

    jmethodID (*GetStaticMethodID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);

    CALL_METHOD(STATIC);

    jfieldID (*GetStaticFieldID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);

    FIELD_OP(GET_STATIC);
    FIELD_OP(SET_STATIC);

    jstring (*NewString)(JNIEnv *env, const jchar *unicode, jsize len);
    jsize (*GetStringLength)(JNIEnv *env, jstring str);
    const jchar *(*GetStringChars)(JNIEnv *env, jstring str, jboolean *isCopy);
    void (*ReleaseStringChars)(JNIEnv *env, jstring str, const jchar *chars);
  
    jstring (*NewStringUTF)(JNIEnv *env, const char *utf);
    jsize (*GetStringUTFLength)(JNIEnv *env, jstring str);
    const char* (*GetStringUTFChars)(JNIEnv *env, jstring str, jboolean *isCopy);
    void (*ReleaseStringUTFChars)(JNIEnv *env, jstring str, const char* chars);
  
    jsize (*GetArrayLength)(JNIEnv *env, jarray array);

    jobjectArray (*NewObjectArray)(JNIEnv *env, jsize len, jclass clazz, jobject init);
    jobject (*GetObjectArrayElement)(JNIEnv *env, jobjectArray array, jsize index);
    void (*SetObjectArrayElement)(JNIEnv *env, jobjectArray array, jsize index, jobject val);

    PRIM_ARRAY_OP(NEW);
    PRIM_ARRAY_OP(GET_ELEMENTS);
    PRIM_ARRAY_OP(RELEASE_ELEMENTS);
    PRIM_ARRAY_OP(GET_REGION);
    PRIM_ARRAY_OP(SET_REGION);

    jint (*RegisterNatives)(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods);
    jint (*UnregisterNatives)(JNIEnv *env, jclass clazz);

    jint (*MonitorEnter)(JNIEnv *env, jobject obj);
    jint (*MonitorExit)(JNIEnv *env, jobject obj);
 
    jint (*GetJavaVM)(JNIEnv *env, JavaVM **vm);

    void (*GetStringRegion)(JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf);
    void (*GetStringUTFRegion)(JNIEnv *env, jstring str, jsize start, jsize len, char *buf);

    void *(*GetPrimitiveArrayCritical)(JNIEnv *env, jarray array, jboolean *isCopy);
    void (*ReleasePrimitiveArrayCritical)(JNIEnv *env, jarray array, void *carray, jint mode);

    const jchar *(*GetStringCritical)(JNIEnv *env, jstring string, jboolean *isCopy);
    void (*ReleaseStringCritical)(JNIEnv *env, jstring string, const jchar *cstring);

    jweak (*NewWeakGlobalRef)(JNIEnv *env, jobject obj);
    void (*DeleteWeakGlobalRef)(JNIEnv *env, jweak obj);

    jboolean (*ExceptionCheck)(JNIEnv *env);
    jobject (*NewDirectByteBuffer)(JNIEnv *env, void *addr, jlong capacity);
    void* (*GetDirectBufferAddress)(JNIEnv *env, jobject buffer);
    jlong (*GetDirectBufferCapacity)(JNIEnv *env, jobject buffer);
    jobjectRefType (*GetObjectRefType)(JNIEnv *env, jobject obj);
};

struct _JNIInvokeInterface {
    void *reserved0;
    void *reserved1;
    void *reserved2;

    jint (*DestroyJavaVM)(JavaVM *vm);
    jint (*AttachCurrentThread)(JavaVM *vm, void **penv, void *args);
    jint (*DetachCurrentThread)(JavaVM *vm);
    jint (*GetEnv)(JavaVM *vm, void **penv, jint version);
    jint (*AttachCurrentThreadAsDaemon)(JavaVM *vm, void **penv, void *args);
};

typedef struct JavaVMOption {
    char *optionString;
    void *extraInfo;
} JavaVMOption;

typedef struct JavaVMInitArgs {
    jint version;
    jint nOptions;
    JavaVMOption *options;
    jboolean ignoreUnrecognized;
} JavaVMInitArgs;

typedef struct JavaVMAttachArgs {
    jint version;
    char *name;
    jobject group;
} JavaVMAttachArgs;

#undef VIRTUAL_METHOD
#undef NONVIRTUAL_METHOD
#undef STATIC_METHOD
#undef VOID_VIRTUAL_METHOD
#undef VOID_NONVIRTUAL_METHOD
#undef VOID_STATIC_METHOD
#undef CALL_METHOD
#undef NEW_PRIM_ARRAY
#undef GET_ELEMENTS_PRIM_ARRAY
#undef RELEASE_ELEMENTS_PRIM_ARRAY
#undef GET_REGION_PRIM_ARRAY
#undef SET_REGION_PRIM_ARRAY
#undef PRIM_ARRAY_OP
#undef GET_FIELD
#undef SET_FIELD
#undef GET_STATIC_FIELD
#undef SET_STATIC_FIELD
#undef FIELD_OP
#endif
