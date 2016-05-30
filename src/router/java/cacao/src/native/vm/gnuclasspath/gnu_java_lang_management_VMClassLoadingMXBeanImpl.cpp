/* src/native/vm/gnuclasspath/gnu_java_lang_management_VMClassLoadingMXBeanImpl.cpp

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

#include "mm/gc.hpp"

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/gnu_java_lang_management_VMClassLoadingMXBeanImpl.h"
#endif

#include "toolbox/logging.hpp"

#include "vm/classcache.hpp"
#include "vm/options.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     gnu/java/lang/management/VMClassLoadingMXBeanImpl
 * Method:    getLoadedClassCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_gnu_java_lang_management_VMClassLoadingMXBeanImpl_getLoadedClassCount(JNIEnv *env, jclass clazz)
{
	int32_t count;

	count = classcache_get_loaded_class_count();

	return count;
}


/*
 * Class:     gnu/java/lang/management/VMClassLoadingMXBeanImpl
 * Method:    getUnloadedClassCount
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_gnu_java_lang_management_VMClassLoadingMXBeanImpl_getUnloadedClassCount(JNIEnv *env, jclass clazz)
{
	int32_t count;

	// XXX Fix this once we support class unloading!
	count = 0;

	return count;
}


/*
 * Class:     gnu/java/lang/management/VMClassLoadingMXBeanImpl
 * Method:    isVerbose
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_gnu_java_lang_management_VMClassLoadingMXBeanImpl_isVerbose(JNIEnv *env, jclass clazz)
{
	return opt_verboseclass;
}


/*
 * Class:     gnu/java/lang/management/VMClassLoadingMXBeanImpl
 * Method:    setVerbose
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_gnu_java_lang_management_VMClassLoadingMXBeanImpl_setVerbose(JNIEnv *env, jclass clazz, jboolean verbose)
{
	opt_verboseclass = verbose;
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "getLoadedClassCount",   (char*) "()I",  (void*) (uintptr_t) &Java_gnu_java_lang_management_VMClassLoadingMXBeanImpl_getLoadedClassCount   },
	{ (char*) "getUnloadedClassCount", (char*) "()J",  (void*) (uintptr_t) &Java_gnu_java_lang_management_VMClassLoadingMXBeanImpl_getUnloadedClassCount },
	{ (char*) "isVerbose",             (char*) "()Z",  (void*) (uintptr_t) &Java_gnu_java_lang_management_VMClassLoadingMXBeanImpl_isVerbose             },
	{ (char*) "setVerbose",            (char*) "(Z)V", (void*) (uintptr_t) &Java_gnu_java_lang_management_VMClassLoadingMXBeanImpl_setVerbose            },
};


/* _Jv_gnu_java_lang_management_VMClassLoadingMXBeanImpl_init ******************

   Register native functions.

*******************************************************************************/

void _Jv_gnu_java_lang_management_VMClassLoadingMXBeanImpl_init(void)
{
	Utf8String u = Utf8String::from_utf8("gnu/java/lang/management/VMClassLoadingMXBeanImpl");

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
