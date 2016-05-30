/* src/native/vm/gnuclasspath/gnu_classpath_VMStackWalker.cpp

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

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/gnu_classpath_VMStackWalker.h"
#endif

#include "vm/class.hpp"
#include "vm/global.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"

#include "vm/jit/stacktrace.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     gnu/classpath/VMStackWalker
 * Method:    getClassContext
 * Signature: ()[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_gnu_classpath_VMStackWalker_getClassContext(JNIEnv *env, jclass clazz)
{
	java_handle_objectarray_t *oa;

	oa = stacktrace_getClassContext();

	return (jobjectArray) oa;
}


/*
 * Class:     gnu/classpath/VMStackWalker
 * Method:    getCallingClass
 * Signature: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_gnu_classpath_VMStackWalker_getCallingClass(JNIEnv *env, jclass clazz)
{
	classinfo *c;

	c = stacktrace_get_caller_class(2);

	return (jclass) c;
}


/*
 * Class:     gnu/classpath/VMStackWalker
 * Method:    getCallingClassLoader
 * Signature: ()Ljava/lang/ClassLoader;
 */
JNIEXPORT jobject JNICALL Java_gnu_classpath_VMStackWalker_getCallingClassLoader(JNIEnv *env, jclass clazz)
{
	classinfo     *c;
	classloader_t *cl;

	c  = stacktrace_get_caller_class(2);
	cl = class_get_classloader(c);

	return (jobject) cl;
}


/*
 * Class:     gnu/classpath/VMStackWalker
 * Method:    firstNonNullClassLoader
 * Signature: ()Ljava/lang/ClassLoader;
 */
JNIEXPORT jobject JNICALL Java_gnu_classpath_VMStackWalker_firstNonNullClassLoader(JNIEnv *env, jclass clazz)
{
	classloader_t *cl;

	cl = stacktrace_first_nonnull_classloader();

	return (jobject) cl;
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "getClassContext",         (char*) "()[Ljava/lang/Class;",      (void*) (uintptr_t) &Java_gnu_classpath_VMStackWalker_getClassContext         },
	{ (char*) "getCallingClass",         (char*) "()Ljava/lang/Class;",       (void*) (uintptr_t) &Java_gnu_classpath_VMStackWalker_getCallingClass         },
	{ (char*) "getCallingClassLoader",   (char*) "()Ljava/lang/ClassLoader;", (void*) (uintptr_t) &Java_gnu_classpath_VMStackWalker_getCallingClassLoader   },
	{ (char*) "firstNonNullClassLoader", (char*) "()Ljava/lang/ClassLoader;", (void*) (uintptr_t) &Java_gnu_classpath_VMStackWalker_firstNonNullClassLoader },
};


/* _Jv_gnu_classpath_VMStackWalker_init ****************************************

   Register native functions.

*******************************************************************************/

void _Jv_gnu_classpath_VMStackWalker_init(void)
{
	Utf8String u = Utf8String::from_utf8("gnu/classpath/VMStackWalker");

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
