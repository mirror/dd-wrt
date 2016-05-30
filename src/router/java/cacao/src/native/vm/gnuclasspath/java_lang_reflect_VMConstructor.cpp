/* src/native/vm/gnuclasspath/java_lang_reflect_VMConstructor.cpp

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

#include <assert.h>
#include <stdlib.h>

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/java_lang_reflect_VMConstructor.h"
#endif

#include "native/vm/reflection.hpp"

#include "vm/javaobjects.hpp"
#include "vm/string.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/reflect/VMConstructor
 * Method:    getModifiersInternal
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_java_lang_reflect_VMConstructor_getModifiersInternal(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMConstructor rvmc(_this);
	methodinfo* m = rvmc.get_method();
	return m->flags;
}


/*
 * Class:     java/lang/reflect/VMConstructor
 * Method:    getParameterTypes
 * Signature: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_VMConstructor_getParameterTypes(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMConstructor rvmc(_this);
	methodinfo* m = rvmc.get_method();

	java_handle_objectarray_t* hoa = method_get_parametertypearray(m);

	return (jobjectArray) hoa;
}


/*
 * Class:     java/lang/reflect/VMConstructor
 * Method:    getExceptionTypes
 * Signature: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_VMConstructor_getExceptionTypes(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMConstructor rvmc(_this);
	methodinfo* m = rvmc.get_method();

	java_handle_objectarray_t* hoa = method_get_exceptionarray(m);

	return (jobjectArray) hoa;
}


/*
 * Class:     java/lang/reflect/VMConstructor
 * Method:    construct
 * Signature: ([Ljava/lang/Object;Ljava/lang/Class;I)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_java_lang_reflect_VMConstructor_construct(JNIEnv *env, jobject _this, jobjectArray args)
{
	java_lang_reflect_VMConstructor jlrvmc(_this);
	java_lang_reflect_Constructor jlrc(jlrvmc.get_cons());

	java_handle_t* o = jlrc.new_instance((java_handle_objectarray_t*) args);

	return (jobject) o;
}


/*
 * Class:     java/lang/reflect/VMConstructor
 * Method:    getSignature
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_lang_reflect_VMConstructor_getSignature(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMConstructor rvmc(_this);
	methodinfo* m = rvmc.get_method();
	java_handle_t *o;

	if (m->signature == NULL)
		return NULL;

	o = JavaString::from_utf8(m->signature);

	/* In error case o is NULL. */

	return (jstring) o;
}


#if defined(ENABLE_ANNOTATIONS)
/*
 * Class:     java/lang/reflect/VMConstructor
 * Method:    declaredAnnotations
 * Signature: ()Ljava/util/Map;
 *
 * Parses the annotations (if they aren't parsed yet) and stores them into
 * the declaredAnnotations map and return this map.
 */
JNIEXPORT jobject JNICALL Java_java_lang_reflect_VMConstructor_declaredAnnotations(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMConstructor rvmc(_this);

	java_handle_t* declaredAnnotations = rvmc.get_declaredAnnotations();

	/* are the annotations parsed yet? */
	if (declaredAnnotations == NULL) {
		java_handle_bytearray_t* annotations    = rvmc.get_annotations();
		classinfo*               declaringClass = rvmc.get_clazz();
		classinfo*               referer        = rvmc.get_Class();

		declaredAnnotations = Reflection::get_declaredannotations(annotations, declaringClass, referer);

		rvmc.set_declaredAnnotations(declaredAnnotations);
	}

	return (jobject) declaredAnnotations;
}


/*
 * Class:     java/lang/reflect/VMConstructor
 * Method:    getParameterAnnotations
 * Signature: ()[[Ljava/lang/annotation/Annotation;
 *
 * Parses the parameter annotations and returns them in an 2 dimensional array.
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_VMConstructor_getParameterAnnotations(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMConstructor rvmc(_this);

	java_handle_bytearray_t* parameterAnnotations = rvmc.get_parameterAnnotations();
	methodinfo* m = rvmc.get_method();
	classinfo* referer = rvmc.get_Class();

	java_handle_objectarray_t* oa = Reflection::get_parameterannotations(parameterAnnotations, m, referer);

	return (jobjectArray) oa;
}
#endif

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "getModifiersInternal",    (char*) "()I",                                     (void*) (uintptr_t) &Java_java_lang_reflect_VMConstructor_getModifiersInternal    },
	{ (char*) "getParameterTypes",       (char*) "()[Ljava/lang/Class;",                    (void*) (uintptr_t) &Java_java_lang_reflect_VMConstructor_getParameterTypes       },
	{ (char*) "getExceptionTypes",       (char*) "()[Ljava/lang/Class;",                    (void*) (uintptr_t) &Java_java_lang_reflect_VMConstructor_getExceptionTypes       },
	{ (char*) "construct",               (char*) "([Ljava/lang/Object;)Ljava/lang/Object;", (void*) (uintptr_t) &Java_java_lang_reflect_VMConstructor_construct               },
	{ (char*) "getSignature",            (char*) "()Ljava/lang/String;",                    (void*) (uintptr_t) &Java_java_lang_reflect_VMConstructor_getSignature            },
#if defined(ENABLE_ANNOTATIONS)
	{ (char*) "declaredAnnotations",     (char*) "()Ljava/util/Map;",                       (void*) (uintptr_t) &Java_java_lang_reflect_VMConstructor_declaredAnnotations     },
	{ (char*) "getParameterAnnotations", (char*) "()[[Ljava/lang/annotation/Annotation;",   (void*) (uintptr_t) &Java_java_lang_reflect_VMConstructor_getParameterAnnotations },
#endif
};


/* _Jv_java_lang_reflect_VMConstructor_init ************************************

   Register native functions.

*******************************************************************************/

void _Jv_java_lang_reflect_VMConstructor_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/reflect/VMConstructor");

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
