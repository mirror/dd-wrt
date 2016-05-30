/* src/native/vm/gnuclasspath/java_lang_reflect_VMMethod.cpp

   Copyright (C) 1996-2013
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

#if defined(ENABLE_ANNOTATIONS)
#include "vm/vm.hpp"
#endif

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/java_lang_reflect_VMMethod.h"
#endif

#include "native/vm/reflection.hpp"

#include "vm/access.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/exceptions.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/initialize.hpp"
#include "vm/javaobjects.hpp"
#include "vm/method.hpp"
#include "vm/resolve.hpp"
#include "vm/string.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/reflect/VMMethod
 * Method:    getModifiersInternal
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_java_lang_reflect_VMMethod_getModifiersInternal(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMMethod rvmm(_this);
	methodinfo* m = rvmm.get_method();
	return m->flags;
}


/*
 * Class:     java/lang/reflect/VMMethod
 * Method:    getReturnType
 * Signature: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_java_lang_reflect_VMMethod_getReturnType(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMMethod rvmm(_this);
	methodinfo* m = rvmm.get_method();
	classinfo*  c = method_returntype_get(m);

	return (jclass) LLNI_classinfo_wrap(c);
}


/*
 * Class:     java/lang/reflect/VMMethod
 * Method:    getParameterTypes
 * Signature: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_VMMethod_getParameterTypes(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMMethod rvmm(_this);
	methodinfo* m = rvmm.get_method();
	java_handle_objectarray_t* oa = method_get_parametertypearray(m);
	return (jobjectArray) oa;
}


/*
 * Class:     java/lang/reflect/VMMethod
 * Method:    getExceptionTypes
 * Signature: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_VMMethod_getExceptionTypes(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMMethod rvmm(_this);
	methodinfo* m = rvmm.get_method();
	java_handle_objectarray_t* oa = method_get_exceptionarray(m);
	return (jobjectArray) oa;
}


/*
 * Class:     java/lang/reflect/VMMethod
 * Method:    invoke
 * Signature: (Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_java_lang_reflect_VMMethod_invoke(JNIEnv *env, jobject _this, jobject o, jobjectArray args)
{
	java_lang_reflect_VMMethod jlrvmm(_this);
	java_lang_reflect_Method jlrm(jlrvmm.get_m());

	java_handle_t* result = jlrm.invoke((java_handle_t*) o, (java_handle_objectarray_t*) args);

	return (jobject) result;
}


/*
 * Class:     java/lang/reflect/VMMethod
 * Method:    getSignature
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_lang_reflect_VMMethod_getSignature(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMMethod rvmm(_this);
	methodinfo* m = rvmm.get_method();

	if (m->signature == NULL)
		return NULL;

	java_handle_t* s = JavaString::from_utf8(m->signature);

	/* in error case o is NULL */

	return (jstring) s;
}


#if defined(ENABLE_ANNOTATIONS)
/*
 * Class:     java/lang/reflect/VMMethod
 * Method:    getDefaultValue
 * Signature: ()Ljava/lang/Object;
 *
 * Parses the annotation default value and returnes it (boxed, if it's a primitive).
 */
JNIEXPORT jobject JNICALL Java_java_lang_reflect_VMMethod_getDefaultValue(JNIEnv *env, jobject _this)
{
	static methodinfo *m_parseAnnotationDefault   = NULL; /* parser method (will be chached, therefore static) */
	Utf8String         utf_parseAnnotationDefault = NULL; /* parser method name                                */
	Utf8String         utf_desc        = NULL;            /* parser method descriptor (signature)              */

	if (_this == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	// TODO Use a constructor.
	java_handle_t* h = native_new_and_init(class_sun_reflect_ConstantPool);

	if (h == NULL)
		return NULL;

	sun_reflect_ConstantPool cp(h);
	
	java_lang_reflect_VMMethod rvmm(_this);
	classinfo* declaringClass = rvmm.get_clazz();
	cp.set_constantPoolOop(declaringClass);

	/* only resolve the parser method the first time */
	if (m_parseAnnotationDefault == NULL) {
		utf_parseAnnotationDefault = Utf8String::from_utf8("parseAnnotationDefault");
		utf_desc = Utf8String::from_utf8(
			"(Ljava/lang/reflect/Method;[BLsun/reflect/ConstantPool;)"
			"Ljava/lang/Object;");

		if (utf_parseAnnotationDefault == NULL || utf_desc == NULL) {
			/* out of memory */
			return NULL;
		}

		classinfo *referer = rvmm.get_Class();

		m_parseAnnotationDefault = class_resolveclassmethod(
			class_sun_reflect_annotation_AnnotationParser,
			utf_parseAnnotationDefault,
			utf_desc,
			referer,
			true);

		if (m_parseAnnotationDefault == NULL) {
			/* method not found */
			return NULL;
		}
	}

	java_lang_reflect_Method rm(rvmm.get_m());
	java_handle_bytearray_t* annotationDefault = rvmm.get_annotationDefault();

	java_handle_t* result = vm_call_method(m_parseAnnotationDefault, NULL, rm.get_handle(), annotationDefault, cp.get_handle());

	return (jobject) result;
}


/*
 * Class:     java/lang/reflect/VMMethod
 * Method:    declaredAnnotations
 * Signature: ()Ljava/util/Map;
 */
JNIEXPORT jobject JNICALL Java_java_lang_reflect_VMMethod_declaredAnnotations(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMMethod rvmm(_this);
	java_handle_t* declaredAnnotations = rvmm.get_declaredAnnotations();

	// Are the annotations parsed yet?
	if (declaredAnnotations == NULL) {
		java_handle_bytearray_t* annotations    = rvmm.get_annotations();
		classinfo*               declaringClass = rvmm.get_clazz();
		classinfo*               referer        = rvmm.get_Class();

		declaredAnnotations = Reflection::get_declaredannotations(annotations, declaringClass, referer);

		rvmm.set_declaredAnnotations(declaredAnnotations);
	}

	return (jobject) declaredAnnotations;
}


/*
 * Class:     java/lang/reflect/VMMethod
 * Method:    getParameterAnnotations
 * Signature: ()[[Ljava/lang/annotation/Annotation;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_VMMethod_getParameterAnnotations(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMMethod rvmm(_this);
	java_handle_bytearray_t* parameterAnnotations = rvmm.get_parameterAnnotations();
	methodinfo* m = rvmm.get_method();
	classinfo* referer = rvmm.get_Class();

	java_handle_objectarray_t* oa = Reflection::get_parameterannotations(parameterAnnotations, m, referer);
	return (jobjectArray) oa;
}
#endif

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "getModifiersInternal",    (char*) "()I",                                                       (void*) (uintptr_t) &Java_java_lang_reflect_VMMethod_getModifiersInternal    },
	{ (char*) "getReturnType",           (char*) "()Ljava/lang/Class;",                                       (void*) (uintptr_t) &Java_java_lang_reflect_VMMethod_getReturnType           },
	{ (char*) "getParameterTypes",       (char*) "()[Ljava/lang/Class;",                                      (void*) (uintptr_t) &Java_java_lang_reflect_VMMethod_getParameterTypes       },
	{ (char*) "getExceptionTypes",       (char*) "()[Ljava/lang/Class;",                                      (void*) (uintptr_t) &Java_java_lang_reflect_VMMethod_getExceptionTypes       },
	{ (char*) "invoke",                  (char*) "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;", (void*) (uintptr_t) &Java_java_lang_reflect_VMMethod_invoke                  },
	{ (char*) "getSignature",            (char*) "()Ljava/lang/String;",                                      (void*) (uintptr_t) &Java_java_lang_reflect_VMMethod_getSignature            },
#if defined(ENABLE_ANNOTATIONS)
	{ (char*) "getDefaultValue",         (char*) "()Ljava/lang/Object;",                                      (void*) (uintptr_t) &Java_java_lang_reflect_VMMethod_getDefaultValue         },
	{ (char*) "declaredAnnotations",     (char*) "()Ljava/util/Map;",                                         (void*) (uintptr_t) &Java_java_lang_reflect_VMMethod_declaredAnnotations     },
	{ (char*) "getParameterAnnotations", (char*) "()[[Ljava/lang/annotation/Annotation;",                     (void*) (uintptr_t) &Java_java_lang_reflect_VMMethod_getParameterAnnotations },
#endif
};


/* _Jv_java_lang_reflect_VMMethod_init *****************************************

   Register native functions.

*******************************************************************************/

void _Jv_java_lang_reflect_VMMethod_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/reflect/VMMethod");

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
 */
