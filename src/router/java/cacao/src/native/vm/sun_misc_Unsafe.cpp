/* src/native/vm/sun_misc_Unsafe.cpp - sun/misc/Unsafe

   Copyright (C) 2006-2014
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

#include <stdint.h>
#include <unistd.h>

#include "threads/atomic.hpp"
#include "threads/thread.hpp"

#include "mm/memory.hpp"

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/sun_misc_Unsafe.h"
#endif

#include "vm/array.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/exceptions.hpp"
#include "vm/initialize.hpp"
#include "vm/javaobjects.hpp"
#include "vm/os.hpp"
#include "vm/string.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     sun/misc/Unsafe
 * Method:    registerNatives
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_registerNatives(JNIEnv *env, jclass clazz)
{
	/* The native methods of this function are already registered in
	   _Jv_sun_misc_Unsafe_init() which is called during VM
	   startup. */
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getInt
 * Signature: (Ljava/lang/Object;J)I
 */
JNIEXPORT jint JNICALL Java_sun_misc_Unsafe_getInt__Ljava_lang_Object_2J(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putInt
 * Signature: (Ljava/lang/Object;JI)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putInt__Ljava_lang_Object_2JI(JNIEnv *env, jobject _this, jobject o, jlong offset, jint x)
{
	FieldAccess::set(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getObject
 * Signature: (Ljava/lang/Object;J)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_sun_misc_Unsafe_getObject(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get<jobject>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putObject
 * Signature: (Ljava/lang/Object;JLjava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putObject(JNIEnv *env, jobject _this, jobject o, jlong offset, jobject x)
{
	FieldAccess::set(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getBoolean
 * Signature: (Ljava/lang/Object;J)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_misc_Unsafe_getBoolean(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putBoolean
 * Signature: (Ljava/lang/Object;JZ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putBoolean(JNIEnv *env, jobject _this, jobject o, jlong offset, jboolean x)
{
	FieldAccess::set(o, offset, (int32_t) x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getByte
 * Signature: (Ljava/lang/Object;J)B
 */
JNIEXPORT jbyte JNICALL Java_sun_misc_Unsafe_getByte__Ljava_lang_Object_2J(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putByte
 * Signature: (Ljava/lang/Object;JB)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putByte__Ljava_lang_Object_2JB(JNIEnv *env, jobject _this, jobject o, jlong offset, jbyte x)
{
	FieldAccess::set(o, offset, (int32_t) x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getShort
 * Signature: (Ljava/lang/Object;J)S
 */
JNIEXPORT jshort JNICALL Java_sun_misc_Unsafe_getShort__Ljava_lang_Object_2J(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putShort
 * Signature: (Ljava/lang/Object;JS)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putShort__Ljava_lang_Object_2JS(JNIEnv *env, jobject _this, jobject o, jlong offset, jshort x)
{
	FieldAccess::set(o, offset, (int32_t) x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getChar
 * Signature: (Ljava/lang/Object;J)C
 */
JNIEXPORT jchar JNICALL Java_sun_misc_Unsafe_getChar__Ljava_lang_Object_2J(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putChar
 * Signature: (Ljava/lang/Object;JC)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putChar__Ljava_lang_Object_2JC(JNIEnv *env, jobject _this, jobject o, jlong offset, jchar x)
{
	FieldAccess::set(o, offset, (int32_t) x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getLong
 * Signature: (Ljava/lang/Object;J)J
 */
JNIEXPORT jlong JNICALL Java_sun_misc_Unsafe_getLong__Ljava_lang_Object_2J(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get<int64_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putLong
 * Signature: (Ljava/lang/Object;JJ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putLong__Ljava_lang_Object_2JJ(JNIEnv *env, jobject _this, jobject o, jlong offset, jlong x)
{
	FieldAccess::set(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getFloat
 * Signature: (Ljava/lang/Object;J)F
 */
JNIEXPORT jfloat JNICALL Java_sun_misc_Unsafe_getFloat__Ljava_lang_Object_2J(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get<float>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putFloat
 * Signature: (Ljava/lang/Object;JF)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putFloat__Ljava_lang_Object_2JF(JNIEnv *env, jobject _this, jobject o, jlong offset, jfloat x)
{
	FieldAccess::set(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getDouble
 * Signature: (Ljava/lang/Object;J)D
 */
JNIEXPORT jdouble JNICALL Java_sun_misc_Unsafe_getDouble__Ljava_lang_Object_2J(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get<double>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putDouble
 * Signature: (Ljava/lang/Object;JD)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putDouble__Ljava_lang_Object_2JD(JNIEnv *env, jobject _this, jobject o, jlong offset, jdouble x)
{
	FieldAccess::set(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getByte
 * Signature: (J)B
 */
JNIEXPORT jbyte JNICALL Java_sun_misc_Unsafe_getByte__J(JNIEnv *env, jobject _this, jlong address)
{
	int8_t *p;
	int8_t  value;

	p = (int8_t *) (intptr_t) address;

	value = *p;

	return (int32_t) value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putByte
 * Signature: (JB)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putByte__JB(JNIEnv *env, jobject _this, jlong address, jbyte value)
{
	int8_t *p;

	p = (int8_t *) (intptr_t) address;

	*p = (int8_t) value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getShort
 * Signature: (J)S
 */
JNIEXPORT jshort JNICALL Java_sun_misc_Unsafe_getShort__J(JNIEnv *env, jobject _this, jlong address)
{
	int16_t *p;
	int16_t  value;

	p = (int16_t *) (intptr_t) address;

	value = *p;

	return (int32_t) value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putShort
 * Signature: (JS)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putShort__JS(JNIEnv *env, jobject _this, jlong address, jshort value)
{
	int16_t *p;

	p = (int16_t *) (intptr_t) address;

	*p = (int16_t) value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getChar
 * Signature: (J)C
 */
JNIEXPORT jchar JNICALL Java_sun_misc_Unsafe_getChar__J(JNIEnv *env, jobject _this, jlong address)
{
	uint16_t *p;
	uint16_t  value;

	p = (uint16_t *) (intptr_t) address;

	value = *p;

	return (int32_t) value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putChar
 * Signature: (JC)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putChar__JC(JNIEnv *env, jobject _this, jlong address, jchar value)
{
	uint16_t *p;

	p = (uint16_t *) (intptr_t) address;

	*p = (uint16_t) value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getInt
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_sun_misc_Unsafe_getInt__J(JNIEnv *env, jobject _this, jlong address)
{
	int32_t *p;
	int32_t  value;

	p = (int32_t *) (intptr_t) address;

	value = *p;

	return value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putInt
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putInt__JI(JNIEnv *env, jobject _this, jlong address, jint value)
{
	int32_t *p;

	p = (int32_t *) (intptr_t) address;

	*p = value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getLong
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_sun_misc_Unsafe_getLong__J(JNIEnv *env, jobject _this, jlong address)
{
	int64_t *p;
	int64_t  value;

	p = (int64_t *) (intptr_t) address;

	value = *p;

	return value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putLong
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putLong__JJ(JNIEnv *env, jobject _this, jlong address, jlong value)
{
	int64_t *p;

	p = (int64_t *) (intptr_t) address;

	*p = value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getFloat
 * Signature: (J)F
 */
JNIEXPORT jfloat JNICALL Java_sun_misc_Unsafe_getFloat__J(JNIEnv *env, jobject _this, jlong address)
{
	float *p;
	float  value;

	p = (float *) (intptr_t) address;

	value = *p;

	return value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putFloat
 * Signature: (JF)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putFloat__JF(JNIEnv *env, jobject _this, jlong address, jfloat value)
{
	float* p;

	p = (float*) (intptr_t) address;

	*p = value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getDouble
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_sun_misc_Unsafe_getDouble__J(JNIEnv *env, jobject _this, jlong address)
{
	double *p;
	double  value;

	p = (double*) (intptr_t) address;

	value = *p;

	return value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putDouble
 * Signature: (JD)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putDouble__JD(JNIEnv *env, jobject _this, jlong address, jdouble value)
{
	double* p;

	p = (double*) (intptr_t) address;

	*p = value;
}


/*
 * Class:     sun_misc_Unsafe
 * Method:    getAddress
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_sun_misc_Unsafe_getAddress (JNIEnv *env, jobject _this, jlong address)
{
	uintptr_t *p;
	uintptr_t  value;

	p = (uintptr_t *) (intptr_t) address;

	value = *p;

	return value;
}


/*
 * Class:     sun_misc_Unsafe
 * Method:    putAddress
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putAddress (JNIEnv *env, jobject _this, jlong address, jlong value)
{
	uintptr_t *p;

	p = (uintptr_t *) (intptr_t) address;

	*p = value;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    objectFieldOffset
 * Signature: (Ljava/lang/reflect/Field;)J
 */
JNIEXPORT jlong JNICALL Java_sun_misc_Unsafe_objectFieldOffset(JNIEnv *env, jobject _this, jobject field)
{
#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

	java_lang_reflect_Field rf(field);
	java_lang_reflect_VMField rvmf(rf.get_f());
	fieldinfo* f = rvmf.get_field();

#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

	java_lang_reflect_Field rf(field);
	fieldinfo* f = rf.get_field();

#else
# error unknown configuration
#endif

	return (jlong) f->offset;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    allocateMemory
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_sun_misc_Unsafe_allocateMemory(JNIEnv *env, jobject _this, jlong bytes)
{
	size_t  length;
	void   *p;

	length = (size_t) bytes;

	if ((length != (uint64_t) bytes) || (bytes < 0)) {
		exceptions_throw_illegalargumentexception();
		return 0;
	}

	p = MNEW(uint8_t, length);

	return (int64_t) (intptr_t) p;
}


/* OpenJDK 7 */

/*
 * Class:     sun/misc/Unsafe
 * Method:    setMemory
 * Signature: (Ljava/lang/Object;JJB)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_setMemory_jdk7(JNIEnv *env, jobject _this, jobject o, jlong offset, jlong bytes, jbyte value)
{
	size_t  length;
	void   *p;

	length = (size_t) bytes;

	if ((length != (uint64_t) bytes) || (bytes < 0)) {
		exceptions_throw_illegalargumentexception();
		return;
	}

	/* XXX Missing LLNI: we need to unwrap _this object. */

	p = (void *) (((uint8_t *) o) + offset);

	/* XXX Not sure this is correct. */

	os::memset(p, value, length);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    copyMemory
 * Signature: (Ljava/lang/Object;JLjava/lang/Object;JJ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_copyMemory_jdk7(JNIEnv *env, jobject _this, jobject srcBase, jlong srcOffset, jobject destBase, jlong destOffset, jlong bytes)
{
	size_t  length;
	void   *src;
	void   *dest;

	if (bytes == 0)
		return;

	length = (size_t) bytes;

	if ((length != (uint64_t) bytes) || (bytes < 0)) {
		exceptions_throw_illegalargumentexception();
		return;
	}

	/* XXX Missing LLNI: We need to unwrap these objects. */

	src  = (void *) (((uint8_t *) srcBase) + srcOffset);
	dest = (void *) (((uint8_t *) destBase) + destOffset);

	os::memcpy(dest, src, length);
}

/*
 * Class:     sun/misc/Unsafe
 * Method:    setMemory
 * Signature: (JJB)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_setMemory_jdk6(JNIEnv *env, jobject _this, jlong address, jlong bytes, jbyte value)
{
	size_t  length;
	void   *p;

	length = (size_t) bytes;

	if ((length != (uint64_t) bytes) || (bytes < 0)) {
		exceptions_throw_illegalargumentexception();
		return;
	}

	p = (void *) (intptr_t) address;

	/* XXX Not sure this is correct. */

	os::memset(p, value, length);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    copyMemory
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_copyMemory_jdk6(JNIEnv *env, jobject _this, jlong srcAddress, jlong destAddress, jlong bytes)
{
	size_t  length;
	void   *src;
	void   *dest;

	if (bytes == 0)
		return;

	length = (size_t) bytes;

	if ((length != (uint64_t) bytes) || (bytes < 0)) {
		exceptions_throw_illegalargumentexception();
		return;
	}

	src  = (void *) (intptr_t) srcAddress;
	dest = (void *) (intptr_t) destAddress;

	os::memcpy(dest, src, length);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    freeMemory
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_freeMemory(JNIEnv *env, jobject _this, jlong address)
{
	void *p;

	p = (void *) (intptr_t) address;

	if (p == NULL)
		return;

	/* we pass length 1 to trick the free function */

	MFREE(p, uint8_t, 1);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    staticFieldOffset
 * Signature: (Ljava/lang/reflect/Field;)J
 */
JNIEXPORT jlong JNICALL Java_sun_misc_Unsafe_staticFieldOffset(JNIEnv *env, jobject _this, jobject f)
{
	/* The offset of static fields is 0. */

	return 0;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    staticFieldBase
 * Signature: (Ljava/lang/reflect/Field;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_sun_misc_Unsafe_staticFieldBase(JNIEnv *env, jobject _this, jobject field)
{
#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

	java_lang_reflect_Field rf(field);
	java_lang_reflect_VMField rvmf(rf.get_f());
	fieldinfo* f = rvmf.get_field();

#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

	java_lang_reflect_Field rf(field);
	fieldinfo* f = rf.get_field();

#else
# error unknown configuration
#endif

	return (jobject) (f->value);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    ensureClassInitialized
 * Signature: (Ljava/lang/Class;)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_ensureClassInitialized(JNIEnv *env, jobject _this, jclass clazz)
{
	classinfo *c;

	c = LLNI_classinfo_unwrap(clazz);

	if (!(c->state & CLASS_INITIALIZED))
		initialize_class(c);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    arrayBaseOffset
 * Signature: (Ljava/lang/Class;)I
 */
JNIEXPORT jint JNICALL Java_sun_misc_Unsafe_arrayBaseOffset(JNIEnv *env, jobject _this, jclass arrayClass)
{
	classinfo       *c;
	arraydescriptor *ad;

	c  = LLNI_classinfo_unwrap(arrayClass);
	ad = c->vftbl->arraydesc;

	if (ad == NULL) {
		/* XXX does that exception exist? */
		exceptions_throw_internalerror("java/lang/InvalidClassException");
		return 0;
	}

	return ad->dataoffset;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    arrayIndexScale
 * Signature: (Ljava/lang/Class;)I
 */
JNIEXPORT jint JNICALL Java_sun_misc_Unsafe_arrayIndexScale(JNIEnv *env, jobject _this, jclass arrayClass)
{
	classinfo       *c;
	arraydescriptor *ad;

	c  = LLNI_classinfo_unwrap(arrayClass);
	ad = c->vftbl->arraydesc;

	if (ad == NULL) {
		/* XXX does that exception exist? */
		exceptions_throw_internalerror("java/lang/InvalidClassException");
		return 0;
	}

	return ad->componentsize;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    addressSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_sun_misc_Unsafe_addressSize(JNIEnv *env, jobject _this)
{
	return SIZEOF_VOID_P;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    pageSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_sun_misc_Unsafe_pageSize(JNIEnv *env, jobject _this)
{
	int sz;

	sz = os::getpagesize();

	return sz;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    defineClass
 * Signature: (Ljava/lang/String;[BIILjava/lang/ClassLoader;Ljava/security/ProtectionDomain;)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_sun_misc_Unsafe_defineClass__Ljava_lang_String_2_3BIILjava_lang_ClassLoader_2Ljava_security_ProtectionDomain_2(JNIEnv *env, jobject _this, jstring name, jbyteArray b, jint off, jint len, jobject loader, jobject protectionDomain)
{
	classloader_t *cl;
	Utf8String     utfname;
	classinfo     *c;

	cl = loader_hashtable_classloader_add((java_handle_t *) loader);

	/* check if data was passed */

	if (b == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	/* check the indexes passed */

	ByteArray ba(b);

	if ((off < 0) || (len < 0) || ((off + len) > ba.get_length())) {
		exceptions_throw_arrayindexoutofboundsexception();
		return NULL;
	}

	if (name != NULL) {
		/* convert '.' to '/' in java string */

		utfname = JavaString((java_handle_t*) name).to_utf8_dot_to_slash();
	}
	else {
		utfname = NULL;
	}

	/* define the class */

	uint8_t* ptr = ((uint8_t*) ba.get_raw_data_ptr()) + off;
	c = class_define(utfname, cl, len, ptr,
					 (java_handle_t *) protectionDomain);

	if (c == NULL)
		return NULL;

	java_handle_t* h = LLNI_classinfo_wrap(c);

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
	// Set ProtectionDomain.
	java_lang_Class jlc(h);
	jlc.set_pd(protectionDomain);
#endif

	return (jclass) h;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    allocateInstance
 * Signature: (Ljava/lang/Class;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_sun_misc_Unsafe_allocateInstance(JNIEnv *env, jobject _this, jclass cls)
{
	classinfo     *c;
	java_handle_t *o;

	c = LLNI_classinfo_unwrap(cls);

	o = builtin_new(c);

	return (jobject ) o;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    throwException
 * Signature: (Ljava/lang/Throwable;)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_throwException(JNIEnv *env, jobject _this, jobject ee)
{
	java_handle_t *o;

	o = (java_handle_t *) ee;

	exceptions_set_exception(o);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    compareAndSwapObject
 * Signature: (Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_misc_Unsafe_compareAndSwapObject(JNIEnv *env, jobject _this, jobject o, jlong offset, jobject expected, jobject x)
{
	void **p;
	void           *result;

	/* XXX Use LLNI */

	p = (void **) (((uint8_t *) o) + offset);

	result = Atomic::compare_and_swap(p, (void *) expected, (void *) x);
#if defined(CAS_PROVIDES_FULL_BARRIER) && CAS_PROVIDES_FULL_BARRIER
	Atomic::instruction_barrier();
#else
	Atomic::memory_barrier();
#endif

	if (result == expected)
		return true;

	return false;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    compareAndSwapInt
 * Signature: (Ljava/lang/Object;JII)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_misc_Unsafe_compareAndSwapInt(JNIEnv *env, jobject _this, jobject o, jlong offset, jint expected, jint x)
{
	uint32_t *p;
	uint32_t  result;

	/* XXX Use LLNI */

	p = (uint32_t *) (((uint8_t *) o) + offset);

	result = Atomic::compare_and_swap(p, (uint32_t) expected, (uint32_t) x);
#if defined(CAS_PROVIDES_FULL_BARRIER) && CAS_PROVIDES_FULL_BARRIER
	Atomic::instruction_barrier();
#else
	Atomic::memory_barrier();
#endif

	if (result == (uint32_t) expected)
		return true;

	return false;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    compareAndSwapLong
 * Signature: (Ljava/lang/Object;JJJ)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_misc_Unsafe_compareAndSwapLong(JNIEnv *env, jobject _this, jobject o, jlong offset, jlong expected, jlong x)
{
	uint64_t *p;
	uint64_t  result;

	/* XXX Use LLNI */

	p = (uint64_t *) (((uint8_t *) o) + offset);

	result = Atomic::compare_and_swap(p, (uint64_t) expected, (uint64_t) x);
#if defined(CAS_PROVIDES_FULL_BARRIER) && CAS_PROVIDES_FULL_BARRIER
	Atomic::instruction_barrier();
#else
	Atomic::memory_barrier();
#endif

	if (result == (uint64_t) expected)
		return true;

	return false;
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getObjectVolatile
 * Signature: (Ljava/lang/Object;J)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_sun_misc_Unsafe_getObjectVolatile(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get_volatile<jobject>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putObjectVolatile
 * Signature: (Ljava/lang/Object;JLjava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putObjectVolatile(JNIEnv *env, jobject _this, jobject o, jlong offset, jobject x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getBooleanVolatile
 * Signature: (Ljava/lang/Object;J)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_misc_Unsafe_getBooleanVolatile(JNIEnv* env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get_volatile<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putBooleanVolatile
 * Signature: (Ljava/lang/Object;JZ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putBooleanVolatile (JNIEnv *env, jobject _this, jobject o, jlong offset, jboolean x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getByteVolatile
 * Signature: (Ljava/lang/Object;J)B
 */
JNIEXPORT jbyte JNICALL Java_sun_misc_Unsafe_getByteVolatile(JNIEnv* env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get_volatile<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putByteVolatile
 * Signature: (Ljava/lang/Object;JB)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putByteVolatile (JNIEnv *env, jobject _this, jobject o, jlong offset, jbyte x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getShortVolatile
 * Signature: (Ljava/lang/Object;J)S
 */
JNIEXPORT jshort JNICALL Java_sun_misc_Unsafe_getShortVolatile(JNIEnv* env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get_volatile<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putShortVolatile
 * Signature: (Ljava/lang/Object;JS)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putShortVolatile (JNIEnv *env, jobject _this, jobject o, jlong offset, jshort x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getCharVolatile
 * Signature: (Ljava/lang/Object;J)C
 */
JNIEXPORT jchar JNICALL Java_sun_misc_Unsafe_getCharVolatile(JNIEnv* env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get_volatile<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putCharVolatile
 * Signature: (Ljava/lang/Object;JC)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putCharVolatile (JNIEnv *env, jobject _this, jobject o, jlong offset, jchar x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getIntVolatile
 * Signature: (Ljava/lang/Object;J)I
 */
JNIEXPORT jint JNICALL Java_sun_misc_Unsafe_getIntVolatile(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get_volatile<int32_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putIntVolatile
 * Signature: (Ljava/lang/Object;JI)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putIntVolatile(JNIEnv *env, jobject _this, jobject o, jlong offset, jint x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getLongVolatile
 * Signature: (Ljava/lang/Object;J)J
 */
JNIEXPORT jlong JNICALL Java_sun_misc_Unsafe_getLongVolatile(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get_volatile<int64_t>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putLongVolatile
 * Signature: (Ljava/lang/Object;JJ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putLongVolatile(JNIEnv *env, jobject _this, jobject o, jlong offset, jlong x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getFloatVolatile
 * Signature: (Ljava/lang/Object;J)F
 */
JNIEXPORT jfloat JNICALL Java_sun_misc_Unsafe_getFloatVolatile(JNIEnv* env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get_volatile<float>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putFloatVolatile
 * Signature: (Ljava/lang/Object;JF)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putFloatVolatile (JNIEnv *env, jobject _this, jobject o, jlong offset, jfloat x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getDoubleVolatile
 * Signature: (Ljava/lang/Object;J)D
 */
JNIEXPORT jdouble JNICALL Java_sun_misc_Unsafe_getDoubleVolatile(JNIEnv *env, jobject _this, jobject o, jlong offset)
{
	return FieldAccess::get_volatile<double>(o, offset);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putDoubleVolatile
 * Signature: (Ljava/lang/Object;JD)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putDoubleVolatile (JNIEnv *env, jobject _this, jobject o, jlong offset, jdouble x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putOrderedObject
 * Signature: (Ljava/lang/Object;JLjava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putOrderedObject(JNIEnv *env, jobject _this, jobject o, jlong offset, jobject x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putOrderedInt
 * Signature: (Ljava/lang/Object;JI)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putOrderedInt(JNIEnv *env, jobject _this, jobject o, jlong offset, jint x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    putOrderedLong
 * Signature: (Ljava/lang/Object;JJ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_putOrderedLong(JNIEnv *env, jobject _this, jobject o, jlong offset, jlong x)
{
	FieldAccess::set_volatile(o, offset, x);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    unpark
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_unpark(JNIEnv *env, jobject _this, jobject thread)
{
	java_handle_t *h = (java_handle_t *) thread;
	threadobject *t;

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
	h = java_lang_Thread(h).get_vmThread();
#endif
	t = thread_get_thread(h);

	threads_unpark(t);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    park
 * Signature: (ZJ)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Unsafe_park(JNIEnv *env, jobject _this, jboolean isAbsolute, jlong time)
{
	threads_park(isAbsolute, time);
}


/*
 * Class:     sun/misc/Unsafe
 * Method:    getLoadAverage
 * Signature: ([DI)I
 */
JNIEXPORT jint JNICALL Java_sun_misc_Unsafe_getLoadAverage(JNIEnv *env, jobject _this, jdoubleArray loadavg, jint nelem)
{
	DoubleArray da(loadavg);

#define MAX_SAMPLES 3

	// Check the passed number of samples.
	if ((nelem < 0) || (nelem > da.get_length()) || nelem > MAX_SAMPLES) {
		exceptions_throw_arrayindexoutofboundsexception();
		return -1;
	}

	// Actually retrieve samples.
	double values[MAX_SAMPLES];
	int result = os::getloadavg(values, nelem);

	// Save samples into the given array.
	for (int i = 0; i < result; i++) {
		da.set_element(i, values[i]);
	}

	return result;
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "registerNatives",        (char*) "()V",                                                        (void*) (uintptr_t) &Java_sun_misc_Unsafe_registerNatives                  },
	{ (char*) "getInt",                 (char*) "(Ljava/lang/Object;J)I",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getInt__Ljava_lang_Object_2J     },
	{ (char*) "putInt",                 (char*) "(Ljava/lang/Object;JI)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putInt__Ljava_lang_Object_2JI    },
	{ (char*) "getObject",              (char*) "(Ljava/lang/Object;J)Ljava/lang/Object;",                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_getObject                        },
	{ (char*) "putObject",              (char*) "(Ljava/lang/Object;JLjava/lang/Object;)V",                   (void*) (uintptr_t) &Java_sun_misc_Unsafe_putObject                        },
	{ (char*) "getBoolean",             (char*) "(Ljava/lang/Object;J)Z",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getBoolean                       },
	{ (char*) "putBoolean",             (char*) "(Ljava/lang/Object;JZ)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putBoolean                       },
	{ (char*) "getByte",                (char*) "(Ljava/lang/Object;J)B",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getByte__Ljava_lang_Object_2J    },
	{ (char*) "putByte",                (char*) "(Ljava/lang/Object;JB)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putByte__Ljava_lang_Object_2JB   },
	{ (char*) "getShort",               (char*) "(Ljava/lang/Object;J)S",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getShort__Ljava_lang_Object_2J   },
	{ (char*) "putShort",               (char*) "(Ljava/lang/Object;JS)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putShort__Ljava_lang_Object_2JS  },
	{ (char*) "getChar",                (char*) "(Ljava/lang/Object;J)C",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getChar__Ljava_lang_Object_2J    },
	{ (char*) "putChar",                (char*) "(Ljava/lang/Object;JC)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putChar__Ljava_lang_Object_2JC   },
	{ (char*) "getLong",                (char*) "(Ljava/lang/Object;J)J",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getLong__Ljava_lang_Object_2J    },
	{ (char*) "putLong",                (char*) "(Ljava/lang/Object;JJ)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putLong__Ljava_lang_Object_2JJ   },
	{ (char*) "getFloat",               (char*) "(Ljava/lang/Object;J)F",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getFloat__Ljava_lang_Object_2J   },
	{ (char*) "putFloat",               (char*) "(Ljava/lang/Object;JF)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putFloat__Ljava_lang_Object_2JF  },
	{ (char*) "getDouble",              (char*) "(Ljava/lang/Object;J)D",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getDouble__Ljava_lang_Object_2J  },
	{ (char*) "putDouble",              (char*) "(Ljava/lang/Object;JD)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putDouble__Ljava_lang_Object_2JD },
	{ (char*) "getByte",                (char*) "(J)B",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_getByte__J                       },
	{ (char*) "putByte",                (char*) "(JB)V",                                                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_putByte__JB                      },
	{ (char*) "getShort",               (char*) "(J)S",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_getShort__J                      },
	{ (char*) "putShort",               (char*) "(JS)V",                                                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_putShort__JS                     },
	{ (char*) "getChar",                (char*) "(J)C",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_getChar__J                       },
	{ (char*) "putChar",                (char*) "(JC)V",                                                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_putChar__JC                      },
	{ (char*) "getInt",                 (char*) "(J)I",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_getInt__J                        },
	{ (char*) "putInt",                 (char*) "(JI)V",                                                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_putInt__JI                       },
	{ (char*) "getLong",                (char*) "(J)J",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_getLong__J                       },
	{ (char*) "putLong",                (char*) "(JJ)V",                                                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_putLong__JJ                      },
	{ (char*) "getFloat",               (char*) "(J)F",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_getFloat__J                      },
	{ (char*) "putFloat",               (char*) "(JF)V",                                                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_putFloat__JF                     },
	{ (char*) "getDouble",              (char*) "(J)D",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_getDouble__J                     },
	{ (char*) "putDouble",              (char*) "(JD)V",                                                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_putDouble__JD                    },
	{ (char*) "getAddress",             (char*) "(J)J",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_getAddress                       },
	{ (char*) "putAddress",             (char*) "(Ljava/lang/Object;JJ)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putAddress                       },
	{ (char*) "objectFieldOffset",      (char*) "(Ljava/lang/reflect/Field;)J",                               (void*) (uintptr_t) &Java_sun_misc_Unsafe_objectFieldOffset                },
	{ (char*) "allocateMemory",         (char*) "(J)J",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_allocateMemory                   },
	// next two methods: OpenJDK 7
	{ (char*) "setMemory",              (char*) "(Ljava/lang/Object;JJB)V",                                   (void*) (uintptr_t) &Java_sun_misc_Unsafe_setMemory_jdk7                   },
	{ (char*) "copyMemory",             (char*) "(Ljava/lang/Object;JLjava/lang/Object;JJ)V",                 (void*) (uintptr_t) &Java_sun_misc_Unsafe_copyMemory_jdk7                  },
	// next two methods: OpenJDK 6
	{ (char*) "setMemory",              (char*) "(JJB)V",                                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_setMemory_jdk6                   },
	{ (char*) "copyMemory",             (char*) "(JJJ)V",                                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_copyMemory_jdk6                  },
	{ (char*) "freeMemory",             (char*) "(J)V",                                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_freeMemory                       },
	{ (char*) "staticFieldOffset",      (char*) "(Ljava/lang/reflect/Field;)J",                               (void*) (uintptr_t) &Java_sun_misc_Unsafe_staticFieldOffset                },
	{ (char*) "staticFieldBase",        (char*) "(Ljava/lang/reflect/Field;)Ljava/lang/Object;",              (void*) (uintptr_t) &Java_sun_misc_Unsafe_staticFieldBase                  },
	{ (char*) "ensureClassInitialized", (char*) "(Ljava/lang/Class;)V",                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_ensureClassInitialized           },
	{ (char*) "arrayBaseOffset",        (char*) "(Ljava/lang/Class;)I",                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_arrayBaseOffset                  },
	{ (char*) "arrayIndexScale",        (char*) "(Ljava/lang/Class;)I",                                       (void*) (uintptr_t) &Java_sun_misc_Unsafe_arrayIndexScale                  },
	{ (char*) "addressSize",            (char*) "()I",                                                        (void*) (uintptr_t) &Java_sun_misc_Unsafe_addressSize                      },
	{ (char*) "pageSize",               (char*) "()I",                                                        (void*) (uintptr_t) &Java_sun_misc_Unsafe_pageSize                         },
	{ (char*) "defineClass",            (char*) "(Ljava/lang/String;[BIILjava/lang/ClassLoader;Ljava/security/ProtectionDomain;)Ljava/lang/Class;", (void*) (uintptr_t) &Java_sun_misc_Unsafe_defineClass__Ljava_lang_String_2_3BIILjava_lang_ClassLoader_2Ljava_security_ProtectionDomain_2 },
	{ (char*) "allocateInstance",       (char*) "(Ljava/lang/Class;)Ljava/lang/Object;",                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_allocateInstance                 },
	{ (char*) "throwException",         (char*) "(Ljava/lang/Throwable;)V",                                   (void*) (uintptr_t) &Java_sun_misc_Unsafe_throwException                   },
	{ (char*) "compareAndSwapObject",   (char*) "(Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z", (void*) (uintptr_t) &Java_sun_misc_Unsafe_compareAndSwapObject             },
	{ (char*) "compareAndSwapInt",      (char*) "(Ljava/lang/Object;JII)Z",                                   (void*) (uintptr_t) &Java_sun_misc_Unsafe_compareAndSwapInt                },
	{ (char*) "compareAndSwapLong",     (char*) "(Ljava/lang/Object;JJJ)Z",                                   (void*) (uintptr_t) &Java_sun_misc_Unsafe_compareAndSwapLong               },
	{ (char*) "getObjectVolatile",      (char*) "(Ljava/lang/Object;J)Ljava/lang/Object;",                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_getObjectVolatile                },
	{ (char*) "putObjectVolatile",      (char*) "(Ljava/lang/Object;JLjava/lang/Object;)V",                   (void*) (uintptr_t) &Java_sun_misc_Unsafe_putObjectVolatile                },
	{ (char*) "getBooleanVolatile",     (char*) "(Ljava/lang/Object;J)Z",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getBooleanVolatile               },
	{ (char*) "putBooleanVolatile",     (char*) "(Ljava/lang/Object;JZ)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putBooleanVolatile               },
	{ (char*) "getByteVolatile",        (char*) "(Ljava/lang/Object;J)B",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getByteVolatile                  },
	{ (char*) "putByteVolatile",        (char*) "(Ljava/lang/Object;JB)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putByteVolatile                  },
	{ (char*) "getShortVolatile",       (char*) "(Ljava/lang/Object;J)S",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getShortVolatile                 },
	{ (char*) "putShortVolatile",       (char*) "(Ljava/lang/Object;JS)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putShortVolatile                 },
	{ (char*) "getCharVolatile",        (char*) "(Ljava/lang/Object;J)C",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getCharVolatile                  },
	{ (char*) "putCharVolatile",        (char*) "(Ljava/lang/Object;JC)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putCharVolatile                  },
	{ (char*) "getIntVolatile",         (char*) "(Ljava/lang/Object;J)I",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getIntVolatile                   },
	{ (char*) "putIntVolatile",         (char*) "(Ljava/lang/Object;JI)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putIntVolatile                   },
	{ (char*) "getLongVolatile",        (char*) "(Ljava/lang/Object;J)J",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getLongVolatile                  },
	{ (char*) "putLongVolatile",        (char*) "(Ljava/lang/Object;JJ)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putLongVolatile                  },
	{ (char*) "getFloatVolatile",       (char*) "(Ljava/lang/Object;J)F",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getFloatVolatile                 },
	{ (char*) "putFloatVolatile",       (char*) "(Ljava/lang/Object;JF)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putFloatVolatile                 },
	{ (char*) "getDoubleVolatile",      (char*) "(Ljava/lang/Object;J)D",                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getDoubleVolatile                },
	{ (char*) "putDoubleVolatile",      (char*) "(Ljava/lang/Object;JD)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putDoubleVolatile                },
	{ (char*) "putOrderedObject",       (char*) "(Ljava/lang/Object;JLjava/lang/Object;)V",                   (void*) (uintptr_t) &Java_sun_misc_Unsafe_putOrderedObject                 },
	{ (char*) "putOrderedInt",          (char*) "(Ljava/lang/Object;JI)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putOrderedInt                    },
	{ (char*) "putOrderedLong",         (char*) "(Ljava/lang/Object;JJ)V",                                    (void*) (uintptr_t) &Java_sun_misc_Unsafe_putOrderedLong                   },
	{ (char*) "unpark",                 (char*) "(Ljava/lang/Object;)V",                                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_unpark                           },
	{ (char*) "park",                   (char*) "(ZJ)V",                                                      (void*) (uintptr_t) &Java_sun_misc_Unsafe_park                             },
	{ (char*) "getLoadAverage",         (char*) "([DI)I",                                                     (void*) (uintptr_t) &Java_sun_misc_Unsafe_getLoadAverage                   },
};


/* _Jv_sun_misc_Unsafe_init ****************************************************

   Register native functions.

*******************************************************************************/

void _Jv_sun_misc_Unsafe_init(void)
{
	Utf8String u = Utf8String::from_utf8("sun/misc/Unsafe");

	NativeMethods& nm = VM::get_current()->get_nativemethods();
	nm.register_methods(u, methods, NATIVE_METHODS_COUNT);
}


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
