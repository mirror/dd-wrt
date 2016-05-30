/* src/native/vm/gnuclasspath/gnu_java_lang_management_VMMemoryMXBeanImpl.cpp

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

#include "toolbox/logging.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/gnu_java_lang_management_VMMemoryMXBeanImpl.h"
#endif

#include "vm/class.hpp"
#include "vm/global.hpp"
#include "vm/javaobjects.hpp"
#include "vm/options.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     gnu/java/lang/management/VMMemoryMXBeanImpl
 * Method:    getHeapMemoryUsage
 * Signature: ()Ljava/lang/management/MemoryUsage;
 */
JNIEXPORT jobject JNICALL Java_gnu_java_lang_management_VMMemoryMXBeanImpl_getHeapMemoryUsage(JNIEnv *env, jclass clazz)
{
	// Get values from the VM.
	// XXX If we ever support more than one VM, change this.
	int64_t init     = opt_heapstartsize;
	int64_t used     = gc_get_total_bytes();
	int64_t commited = gc_get_heap_size();
	int64_t maximum  = gc_get_max_heap_size();

	// Construct a new java.lang.management.MemoryUsage object.
	java_lang_management_MemoryUsage jlmmu(init, used, commited, maximum);

	return jlmmu.get_handle();
}


/*
 * Class:     gnu/java/lang/management/VMMemoryMXBeanImpl
 * Method:    getNonHeapMemoryUsage
 * Signature: ()Ljava/lang/management/MemoryUsage;
 */
JNIEXPORT jobject JNICALL Java_gnu_java_lang_management_VMMemoryMXBeanImpl_getNonHeapMemoryUsage(JNIEnv *env, jclass clazz)
{
	log_println("Java_gnu_java_lang_management_VMMemoryMXBeanImpl_getNonHeapMemoryUsage: IMPLEMENT ME!");

	return NULL;
}


/*
 * Class:     gnu/java/lang/management/VMMemoryMXBeanImpl
 * Method:    getObjectPendingFinalizationCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_gnu_java_lang_management_VMMemoryMXBeanImpl_getObjectPendingFinalizationCount(JNIEnv *env, jclass clazz)
{
	log_println("Java_gnu_java_lang_management_VMMemoryMXBeanImpl_getObjectPendingFinalizationCount: IMPLEMENT ME!");

	return 0;
}


/*
 * Class:     gnu/java/lang/management/VMMemoryMXBeanImpl
 * Method:    isVerbose
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_gnu_java_lang_management_VMMemoryMXBeanImpl_isVerbose(JNIEnv *env, jclass clazz)
{
	return opt_verbosegc;
}


/*
 * Class:     gnu/java/lang/management/VMMemoryMXBeanImpl
 * Method:    setVerbose
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_gnu_java_lang_management_VMMemoryMXBeanImpl_setVerbose(JNIEnv *env, jclass clazz, jboolean verbose)
{
	opt_verbosegc = verbose;
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "getHeapMemoryUsage",                (char*) "()Ljava/lang/management/MemoryUsage;", (void*) (uintptr_t) &Java_gnu_java_lang_management_VMMemoryMXBeanImpl_getHeapMemoryUsage                },
	{ (char*) "getNonHeapMemoryUsage",             (char*) "()Ljava/lang/management/MemoryUsage;", (void*) (uintptr_t) &Java_gnu_java_lang_management_VMMemoryMXBeanImpl_getNonHeapMemoryUsage             },
	{ (char*) "getObjectPendingFinalizationCount", (char*) "()I",                                  (void*) (uintptr_t) &Java_gnu_java_lang_management_VMMemoryMXBeanImpl_getObjectPendingFinalizationCount },
	{ (char*) "isVerbose",                         (char*) "()Z",                                  (void*) (uintptr_t) &Java_gnu_java_lang_management_VMMemoryMXBeanImpl_isVerbose                         },
	{ (char*) "setVerbose",                        (char*) "(Z)V",                                 (void*) (uintptr_t) &Java_gnu_java_lang_management_VMMemoryMXBeanImpl_setVerbose                        },
};


/* _Jv_gnu_java_lang_management_VMMemoryMXBeanImpl_init ************************

   Register native functions.

*******************************************************************************/

void _Jv_gnu_java_lang_management_VMMemoryMXBeanImpl_init(void)
{
	Utf8String u = Utf8String::from_utf8("gnu/java/lang/management/VMMemoryMXBeanImpl");

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
