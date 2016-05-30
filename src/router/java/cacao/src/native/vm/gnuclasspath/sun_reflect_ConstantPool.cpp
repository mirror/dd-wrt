/* src/native/vm/gnuclasspath/sun_reflect_ConstantPool.cpp

   Copyright (C) 2007-2013
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

/*******************************************************************************

   XXX: The Methods in this file are very redundant to thouse in
        src/native/vm/sun/jvm.c Unless someone has a good idea how to cover
        such redundancy I leave it how it is.

  The ConstantPool class represents an interface to the constant pool of a
  class and is used by the annotations parser (sun.reflect.annotation.
  AnnotationParser) to get the values of the constants refered by the
  annotations.

*******************************************************************************/

#include "config.h"

#include <assert.h>
#include <stdint.h>

#include "mm/memory.hpp"

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/native.hpp"

// FIXME
//#include "native/include/sun_reflect_ConstantPool.h"

#include "native/vm/reflection.hpp"

#include "toolbox/logging.hpp"

#include "vm/class.hpp"
#include "vm/exceptions.hpp"
#include "vm/javaobjects.hpp"
#include "vm/resolve.hpp"
#include "vm/string.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getSize0
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_sun_reflect_ConstantPool_getSize0(JNIEnv *env, jobject _this, jobject jcpool)
{
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);
	return cls->cpcount;
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getClassAt0
 * Signature: (Ljava/lang/Object;I)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_sun_reflect_ConstantPool_getClassAt0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	constant_classref *ref;
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	ref = (constant_classref*)class_getconstant(
		cls, index, CONSTANT_Class);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	return (jclass) LLNI_classinfo_wrap(resolve_classref_eager(ref));
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getClassAtIfLoaded0
 * Signature: (Ljava/lang/Object;I)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_sun_reflect_ConstantPool_getClassAtIfLoaded0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	constant_classref *ref;
	classinfo *c = NULL;
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	ref = (constant_classref*)class_getconstant(
		cls, index, CONSTANT_Class);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}
	
	if (!resolve_classref(NULL,ref,resolveLazy,true,true,&c)) {
		return NULL;
	}

	if (c == NULL || !(c->state & CLASS_LOADED)) {
		return NULL;
	}
	
	return (jclass) LLNI_classinfo_wrap(c);
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getMethodAt0
 * Signature: (Ljava/lang/Object;I)Ljava/lang/reflect/Member;
 */
JNIEXPORT jobject JNICALL Java_sun_reflect_ConstantPool_getMethodAt0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	constant_FMIref *ref;
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);
	
	ref = (constant_FMIref*)class_getconstant(
		cls, index, CONSTANT_Methodref);
	
	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	/* XXX: is that right? or do I have to use resolve_method_*? */
	java_lang_reflect_Method jlrm(ref->p.method);

	return (jobject) jlrm.get_handle();
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getMethodAtIfLoaded0
 * Signature: (Ljava/lang/Object;I)Ljava/lang/reflect/Member;
 */
JNIEXPORT jobject JNICALL Java_sun_reflect_ConstantPool_getMethodAtIfLoaded0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	constant_FMIref *ref;
	classinfo *c = NULL;
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	ref = (constant_FMIref*)class_getconstant(
		cls, index, CONSTANT_Methodref);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	if (!resolve_classref(NULL,ref->p.classref,resolveLazy,true,true,&c)) {
		return NULL;
	}

	if (c == NULL || !(c->state & CLASS_LOADED)) {
		return NULL;
	}

	java_lang_reflect_Method jlrm(ref->p.method);

	return (jobject) jlrm.get_handle();
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getFieldAt0
 * Signature: (Ljava/lang/Object;I)Ljava/lang/reflect/Field;
 */
JNIEXPORT jobject JNICALL Java_sun_reflect_ConstantPool_getFieldAt0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	constant_FMIref *ref;
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	ref = (constant_FMIref*) class_getconstant(cls, index, CONSTANT_Fieldref);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	// Create a new java.lang.reflect.Field Java object.
	java_lang_reflect_Field jlrf(ref->p.field);

	return (jobject) jlrf.get_handle();
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getFieldAtIfLoaded0
 * Signature: (Ljava/lang/Object;I)Ljava/lang/reflect/Field;
 */
JNIEXPORT jobject JNICALL Java_sun_reflect_ConstantPool_getFieldAtIfLoaded0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	constant_FMIref *ref;
	classinfo *c;
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	ref = (constant_FMIref*) class_getconstant(cls, index, CONSTANT_Fieldref);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	if (!resolve_classref(NULL,ref->p.classref,resolveLazy,true,true,&c)) {
		return NULL;
	}

	if (c == NULL || !(c->state & CLASS_LOADED)) {
		return NULL;
	}

	// Create a new java.lang.reflect.Field Java object.
	java_lang_reflect_Field jlrf(ref->p.field);

	return (jobject) jlrf.get_handle();
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getMemberRefInfoAt0
 * Signature: (Ljava/lang/Object;I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_sun_reflect_ConstantPool_getMemberRefInfoAt0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	log_println("Java_sun_reflect_ConstantPool_getMemberRefInfoAt0(env=%p, jcpool=%p, index=%d): IMPLEMENT ME!", env, jcpool, index);
	return NULL;
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getIntAt0
 * Signature: (Ljava/lang/Object;I)I
 */
JNIEXPORT jint JNICALL Java_sun_reflect_ConstantPool_getIntAt0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	int32_t *ref = (int32_t*) class_getconstant(cls, index, CONSTANT_Integer);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return 0;
	}

	return *ref;
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getLongAt0
 * Signature: (Ljava/lang/Object;I)J
 */
JNIEXPORT jlong JNICALL Java_sun_reflect_ConstantPool_getLongAt0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	int64_t *ref = (int64_t*) class_getconstant(cls, index, CONSTANT_Long);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return 0;
	}

	return *ref;
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getFloatAt0
 * Signature: (Ljava/lang/Object;I)F
 */
JNIEXPORT float JNICALL Java_sun_reflect_ConstantPool_getFloatAt0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	float *ref = (float*) class_getconstant( cls, index, CONSTANT_Float);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return 0;
	}

	return *ref;
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getDoubleAt0
 * Signature: (Ljava/lang/Object;I)D
 */
JNIEXPORT double JNICALL Java_sun_reflect_ConstantPool_getDoubleAt0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	double *ref = (double*) class_getconstant(cls, index, CONSTANT_Double);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return 0;
	}

	return *ref;
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getStringAt0
 * Signature: (Ljava/lang/Object;I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_reflect_ConstantPool_getStringAt0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	Utf8String ref;
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);
	
	ref = (utf*)class_getconstant(cls, index, CONSTANT_String);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	/* XXX: I hope literalstring_new is the right Function. */
	return (jstring) JavaString::literal(ref);
}


/*
 * Class:     sun/reflect/ConstantPool
 * Method:    getUTF8At0
 * Signature: (Ljava/lang/Object;I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_reflect_ConstantPool_getUTF8At0(JNIEnv *env, jobject _this, jobject jcpool, jint index)
{
	Utf8String ref;
	classinfo *cls = LLNI_classinfo_unwrap(jcpool);

	ref = (utf*)class_getconstant(cls, index, CONSTANT_Utf8);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	/* XXX: I hope literalstring_new is the right Function. */
	return (jstring) JavaString::literal(ref);
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "getSize0",             (char*) "(Ljava/lang/Object;I)I",                          (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getSize0             },
	{ (char*) "getClassAt0",          (char*) "(Ljava/lang/Object;I)Ljava/lang/Class;",          (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getClassAt0          },
	{ (char*) "getClassAtIfLoaded0",  (char*) "(Ljava/lang/Object;I)Ljava/lang/Class;",          (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getClassAtIfLoaded0  },
	{ (char*) "getMethodAt0",         (char*) "(Ljava/lang/Object;I)Ljava/lang/reflect/Member;", (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getMethodAt0         },
	{ (char*) "getMethodAtIfLoaded0", (char*) "(Ljava/lang/Object;I)Ljava/lang/reflect/Member;", (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getMethodAtIfLoaded0 },
	{ (char*) "getFieldAt0",          (char*) "(Ljava/lang/Object;I)Ljava/lang/reflect/Field;",  (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getFieldAt0          },
	{ (char*) "getFieldAtIfLoaded0",  (char*) "(Ljava/lang/Object;I)Ljava/lang/reflect/Field;",  (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getFieldAtIfLoaded0  },
	{ (char*) "getMemberRefInfoAt0",  (char*) "(Ljava/lang/Object;I)[Ljava/lang/String;",        (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getMemberRefInfoAt0  },
	{ (char*) "getIntAt0",            (char*) "(Ljava/lang/Object;I)I",                          (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getIntAt0            },
	{ (char*) "getLongAt0",           (char*) "(Ljava/lang/Object;I)J",                          (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getLongAt0           },
	{ (char*) "getFloatAt0",          (char*) "(Ljava/lang/Object;I)F",                          (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getFloatAt0          },
	{ (char*) "getDoubleAt0",         (char*) "(Ljava/lang/Object;I)D",                          (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getDoubleAt0         },
	{ (char*) "getStringAt0",         (char*) "(Ljava/lang/Object;I)Ljava/lang/String;",         (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getStringAt0         },
	{ (char*) "getUTF8At0",           (char*) "(Ljava/lang/Object;I)Ljava/lang/String;",         (void*) (uintptr_t) &Java_sun_reflect_ConstantPool_getUTF8At0           },	
};


/* _Jv_sun_reflect_ConstantPool_init ******************************************

   Register native functions.

*******************************************************************************/

void _Jv_sun_reflect_ConstantPool_init(void)
{
	Utf8String u = Utf8String::from_utf8("sun/reflect/ConstantPool");

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
