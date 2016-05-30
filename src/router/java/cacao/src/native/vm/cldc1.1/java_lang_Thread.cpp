/* src/native/vm/cldc1.1/java_lang_Thread.cpp

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
# include "native/include/java_lang_Thread.h"
#endif

#include "threads/thread.hpp"
#include "threads/threadlist.hpp"

#include "toolbox/logging.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/javaobjects.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/Thread
 * Method:    currentThread
 * Signature: ()Ljava/lang/Thread;
 */
JNIEXPORT jobject JNICALL Java_java_lang_Thread_currentThread(JNIEnv *env, jclass clazz)
{
	return (jobject) thread_get_current_object();
}


/*
 * Class:     java/lang/Thread
 * Method:    setPriority0
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_java_lang_Thread_setPriority0(JNIEnv *env, jobject _this, jint oldPriority, jint newPriority)
{
	java_lang_Thread jlt(_this);
	threadobject    *t = jlt.get_vm_thread();

	// The threadobject is null when a thread is created in Java. The
	// priority is set later during startup.
	if (t == NULL)
		return;

	threads_set_thread_priority(t, newPriority);
}


/*
 * Class:     java/lang/Thread
 * Method:    sleep
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_java_lang_Thread_sleep(JNIEnv *env, jclass clazz, jlong millis)
{
	threads_sleep(millis, 0);
}


/*
 * Class:     java/lang/Thread
 * Method:    start0
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_Thread_start0(JNIEnv *env, jobject _this)
{
	java_lang_Thread jlt(_this);
	threads_thread_start(jlt.get_handle());
}


/*
 * Class:     java/lang/Thread
 * Method:    isAlive
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_Thread_isAlive(JNIEnv *env, jobject _this)
{
	java_lang_Thread jlt(_this);
	threadobject    *t = jlt.get_vm_thread();

	if (t == NULL)
		return 0;

	return threads_thread_is_alive(t);
}


/*
 * Class:     java/lang/Thread
 * Method:    activeCount
 * Signature: ()I
 */
JNIEXPORT s4 JNICALL Java_java_lang_Thread_activeCount(JNIEnv *env, jclass clazz)
{
	return ThreadList::get()->get_number_of_non_daemon_threads();
}


/*
 * Class:     java/lang/Thread
 * Method:    interrupt0
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_Thread_interrupt0(JNIEnv *env, jobject _this)
{
	java_lang_Thread jlt(_this);
	threadobject    *t = jlt.get_vm_thread();

	threads_thread_interrupt(t);
}

#if 0
/*
 * Class:     java/lang/Thread
 * Method:    internalExit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_Thread_internalExit(JNIEnv *env, jobject _this)
{
}
#endif


/*
 * Class:     java/lang/Thread
 * Method:    yield
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_Thread_yield(JNIEnv *env, jclass clazz)
{
	threads_yield();
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "currentThread", (char*) "()Ljava/lang/Thread;", (void*) (uintptr_t) &Java_java_lang_Thread_currentThread },
	{ (char*) "setPriority0",  (char*) "(II)V",                (void*) (uintptr_t) &Java_java_lang_Thread_setPriority0  },
	{ (char*) "sleep",         (char*) "(J)V",                 (void*) (uintptr_t) &Java_java_lang_Thread_sleep         },
	{ (char*) "start0",        (char*) "()V",                  (void*) (uintptr_t) &Java_java_lang_Thread_start0        },
	{ (char*) "isAlive",       (char*) "()Z",                  (void*) (uintptr_t) &Java_java_lang_Thread_isAlive       },
	{ (char*) "activeCount",   (char*) "()I",                  (void*) (uintptr_t) &Java_java_lang_Thread_activeCount   },
	{ (char*) "setPriority0",  (char*) "(II)V",                (void*) (uintptr_t) &Java_java_lang_Thread_setPriority0  },
	{ (char*) "interrupt0",    (char*) "()V",                  (void*) (uintptr_t) &Java_java_lang_Thread_interrupt0    },
#if 0
	{ (char*) "internalExit",  (char*) "()V",                  (void*) (uintptr_t) &Java_java_lang_Thread_internalExit  },
#endif
	{ (char*) "yield",         (char*) "()V",                  (void*) (uintptr_t) &Java_java_lang_Thread_yield         },
};


/* _Jv_java_lang_Thread_init ***************************************************
 
   Register native functions.
 
*******************************************************************************/
 
void _Jv_java_lang_Thread_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/Thread");

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
