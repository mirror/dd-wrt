/* src/native/vm/gnuclasspath/gnu_java_lang_management_VMThreadMXBeanImpl.cpp

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
# include "native/vm/include/gnu_java_lang_management_VMThreadMXBeanImpl.h"
#endif

#include "threads/threadlist.hpp"

#include "toolbox/logging.hpp"

#include "vm/classcache.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     gnu/java/lang/management/VMThreadMXBeanImpl
 * Method:    findMonitorDeadlockedThreads
 * Signature: ()[J
 */
JNIEXPORT jlongArray JNICALL Java_gnu_java_lang_management_VMThreadMXBeanImpl_findMonitorDeadlockedThreads(JNIEnv *env, jclass clazz)
{
	log_println("Java_gnu_java_lang_management_VMThreadMXBeanImpl_findMonitorDeadlockedThreads: IMPLEMENT ME!");

	return NULL;
}


/*
 * Class:     gnu/java/lang/management/VMThreadMXBeanImpl
 * Method:    getCurrentThreadCpuTime
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_gnu_java_lang_management_VMThreadMXBeanImpl_getCurrentThreadCpuTime(JNIEnv *env, jclass clazz)
{
	log_println("Java_gnu_java_lang_management_VMThreadMXBeanImpl_getCurrentThreadCpuTime: IMPLEMENT ME!");

	return 0;
}


/*
 * Class:     gnu/java/lang/management/VMThreadMXBeanImpl
 * Method:    getCurrentThreadUserTime
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_gnu_java_lang_management_VMThreadMXBeanImpl_getCurrentThreadUserTime(JNIEnv *env, jclass clazz)
{
	log_println("Java_gnu_java_lang_management_VMThreadMXBeanImpl_getCurrentThreadUserTime: IMPLEMENT ME!");

	return 0;
}


/*
 * Class:     gnu/java/lang/management/VMThreadMXBeanImpl
 * Method:    getPeakThreadCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_gnu_java_lang_management_VMThreadMXBeanImpl_getPeakThreadCount(JNIEnv *env, jclass clazz)
{
	return ThreadList::get()->get_peak_of_active_java_threads();
}


/*
 * Class:     gnu/java/lang/management/VMThreadMXBeanImpl
 * Method:    getThreadCpuTime
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_gnu_java_lang_management_VMThreadMXBeanImpl_getThreadCpuTime(JNIEnv *env, jclass clazz, jlong id)
{
	log_println("Java_gnu_java_lang_management_VMThreadMXBeanImpl_getThreadCpuTime: IMPLEMENT ME!");

	return 0;
}


/*
 * Class:     gnu/java/lang/management/VMThreadMXBeanImpl
 * Method:    getThreadInfoForId
 * Signature: (JI)Ljava/lang/management/ThreadInfo;
 */
JNIEXPORT jobject JNICALL Java_gnu_java_lang_management_VMThreadMXBeanImpl_getThreadInfoForId(JNIEnv *env, jclass clazz, jlong id, jint maxDepth)
{
	log_println("Java_gnu_java_lang_management_VMThreadMXBeanImpl_getThreadInfoForId: IMPLEMENT ME!");

	return NULL;
}


/*
 * Class:     gnu/java/lang/management/VMThreadMXBeanImpl
 * Method:    getThreadUserTime
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_gnu_java_lang_management_VMThreadMXBeanImpl_getThreadUserTime(JNIEnv *env, jclass clazz, jlong par1)
{
	log_println("Java_gnu_java_lang_management_VMThreadMXBeanImpl_getThreadUserTime: IMPLEMENT ME!");

	return 0;
}


/*
 * Class:     gnu/java/lang/management/VMThreadMXBeanImpl
 * Method:    getTotalStartedThreadCount
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_gnu_java_lang_management_VMThreadMXBeanImpl_getTotalStartedThreadCount(JNIEnv *env, jclass clazz)
{
	return ThreadList::get()->get_number_of_started_java_threads();
}


/*
 * Class:     gnu/java/lang/management/VMThreadMXBeanImpl
 * Method:    resetPeakThreadCount
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_gnu_java_lang_management_VMThreadMXBeanImpl_resetPeakThreadCount(JNIEnv *env, jclass clazz)
{
	return ThreadList::get()->reset_peak_of_active_java_threads();
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "findMonitorDeadlockedThreads", (char*) "()[J",                                  (void*) (uintptr_t) &Java_gnu_java_lang_management_VMThreadMXBeanImpl_findMonitorDeadlockedThreads },
	{ (char*) "getCurrentThreadCpuTime",      (char*) "()J",                                   (void*) (uintptr_t) &Java_gnu_java_lang_management_VMThreadMXBeanImpl_getCurrentThreadCpuTime      },
	{ (char*) "getCurrentThreadUserTime",     (char*) "()J",                                   (void*) (uintptr_t) &Java_gnu_java_lang_management_VMThreadMXBeanImpl_getCurrentThreadUserTime     },
	{ (char*) "getPeakThreadCount",           (char*) "()I",                                   (void*) (uintptr_t) &Java_gnu_java_lang_management_VMThreadMXBeanImpl_getPeakThreadCount           },
	{ (char*) "getThreadCpuTime",             (char*) "(J)J",                                  (void*) (uintptr_t) &Java_gnu_java_lang_management_VMThreadMXBeanImpl_getThreadCpuTime             },
	{ (char*) "getThreadInfoForId",           (char*) "(JI)Ljava/lang/management/ThreadInfo;", (void*) (uintptr_t) &Java_gnu_java_lang_management_VMThreadMXBeanImpl_getThreadInfoForId           },
	{ (char*) "getThreadUserTime",            (char*) "(J)J",                                  (void*) (uintptr_t) &Java_gnu_java_lang_management_VMThreadMXBeanImpl_getThreadUserTime            },
	{ (char*) "getTotalStartedThreadCount",   (char*) "()J",                                   (void*) (uintptr_t) &Java_gnu_java_lang_management_VMThreadMXBeanImpl_getTotalStartedThreadCount   },
	{ (char*) "resetPeakThreadCount",         (char*) "()V",                                   (void*) (uintptr_t) &Java_gnu_java_lang_management_VMThreadMXBeanImpl_resetPeakThreadCount         },
};


/* _Jv_gnu_java_lang_management_VMThreadMXBeanImpl_init ************************

   Register native functions.

*******************************************************************************/

void _Jv_gnu_java_lang_management_VMThreadMXBeanImpl_init(void)
{
	Utf8String u = Utf8String::from_utf8("gnu/java/lang/management/VMThreadMXBeanImpl");

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
