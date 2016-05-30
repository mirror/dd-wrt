/* src/native/vm/gnuclasspath/java_lang_VMThread.cpp

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
#include <stdint.h>

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/native.hpp"

#include "toolbox/logging.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/java_lang_VMThread.h"
#endif

#include "threads/lock.hpp"
#include "threads/thread.hpp"

#include "vm/exceptions.hpp"
#include "vm/javaobjects.hpp"
#include "vm/string.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/VMThread
 * Method:    countStackFrames
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_java_lang_VMThread_countStackFrames(JNIEnv *env, jobject _this)
{
	log_println("Java_java_lang_VMThread_countStackFrames: Deprecated.  Not implemented.");

	return 0;
}


/*
 * Class:     java/lang/VMThread
 * Method:    start
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMThread_start(JNIEnv *env, jobject _this, jlong stacksize)
{
	java_lang_VMThread jlvmt(_this);

	threads_thread_start(jlvmt.get_thread());
}


/*
 * Class:     java/lang/VMThread
 * Method:    interrupt
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_VMThread_interrupt(JNIEnv *env, jobject _this)
{
	thread_handle_interrupt((java_handle_t *) _this);
}


/*
 * Class:     java/lang/VMThread
 * Method:    isInterrupted
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMThread_isInterrupted(JNIEnv *env, jobject _this)
{
	return thread_handle_is_interrupted((java_handle_t *) _this);
}


/*
 * Class:     java/lang/VMThread
 * Method:    suspend
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_VMThread_suspend(JNIEnv *env, jobject _this)
{
	log_println("Java_java_lang_VMThread_suspend: Deprecated.  Not implemented.");
}


/*
 * Class:     java/lang/VMThread
 * Method:    resume
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_VMThread_resume(JNIEnv *env, jobject _this)
{
	log_println("Java_java_lang_VMThread_resume: Deprecated.  Not implemented.");
}


/*
 * Class:     java/lang/VMThread
 * Method:    nativeSetPriority
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMThread_nativeSetPriority(JNIEnv *env, jobject _this, jint priority)
{
	thread_handle_set_priority((java_handle_t *) _this, priority);
}


/*
 * Class:     java/lang/VMThread
 * Method:    nativeStop
 * Signature: (Ljava/lang/Throwable;)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMThread_nativeStop(JNIEnv *env, jobject _this, jobject t)
{
	log_println("Java_java_lang_VMThread_nativeStop: Deprecated.  Not implemented.");
}


/*
 * Class:     java/lang/VMThread
 * Method:    currentThread
 * Signature: ()Ljava/lang/Thread;
 */
JNIEXPORT jobject JNICALL Java_java_lang_VMThread_currentThread(JNIEnv *env, jclass clazz)
{
	return (jobject) thread_get_current_object();
}


/*
 * Class:     java/lang/VMThread
 * Method:    yield
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_VMThread_yield(JNIEnv *env, jclass clazz)
{
	threads_yield();
}


/*
 * Class:     java/lang/VMThread
 * Method:    sleep
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMThread_sleep(JNIEnv *env, jclass clazz, int64_t ms, int32_t ns)
{
	threads_sleep(ms, ns);
}


/*
 * Class:     java/lang/VMThread
 * Method:    interrupted
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMThread_interrupted(JNIEnv *env, jclass clazz)
{
	threadobject *t           = thread_get_current();
	int32_t       interrupted = thread_is_interrupted(t);

	if (interrupted)
		thread_set_interrupted(t, false);

	return interrupted;
}


/*
 * Class:     java/lang/VMThread
 * Method:    holdsLock
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMThread_holdsLock(JNIEnv *env, jclass clazz, jobject o)
{
	java_handle_t *h = (java_handle_t *) o;

	if (h == NULL) {
		exceptions_throw_nullpointerexception();
		return 0;
	}

	return lock_is_held_by_current_thread(h);
}


/*
 * Class:     java/lang/VMThread
 * Method:    getState
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_lang_VMThread_getState(JNIEnv *env, jobject _this)
{
	java_handle_t *h     = (java_handle_t *) _this;
	int            state = thread_handle_get_state(h);
	
	switch (state) {
	case THREAD_STATE_NEW:
		return (jstring) JavaString::from_utf8("NEW");
	case THREAD_STATE_RUNNABLE:
		return (jstring) JavaString::from_utf8("RUNNABLE");
	case THREAD_STATE_BLOCKED:
		return (jstring) JavaString::from_utf8("BLOCKED");
	case THREAD_STATE_WAITING:
		return (jstring) JavaString::from_utf8("WAITING");
	case THREAD_STATE_TIMED_WAITING:
		return (jstring) JavaString::from_utf8("TIMED_WAITING");
	case THREAD_STATE_PARKED:
		return (jstring) JavaString::from_utf8("PARKED");
	case THREAD_STATE_TIMED_PARKED:
		return (jstring) JavaString::from_utf8("TIMED_PARKED");
	case THREAD_STATE_TERMINATED:
		return (jstring) JavaString::from_utf8("TERMINATED");
	default:
		vm_abort("Java_java_lang_VMThread_getState: unknown thread state %d", state);
		return NULL; /* Keep compiler happy. */
	}
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "countStackFrames",  (char*) "()I",                      (void*) (uintptr_t) &Java_java_lang_VMThread_countStackFrames  },
	{ (char*) "start",             (char*) "(J)V",                     (void*) (uintptr_t) &Java_java_lang_VMThread_start             },
	{ (char*) "interrupt",         (char*) "()V",                      (void*) (uintptr_t) &Java_java_lang_VMThread_interrupt         },
	{ (char*) "isInterrupted",     (char*) "()Z",                      (void*) (uintptr_t) &Java_java_lang_VMThread_isInterrupted     },
	{ (char*) "suspend",           (char*) "()V",                      (void*) (uintptr_t) &Java_java_lang_VMThread_suspend           },
	{ (char*) "resume",            (char*) "()V",                      (void*) (uintptr_t) &Java_java_lang_VMThread_resume            },
	{ (char*) "nativeSetPriority", (char*) "(I)V",                     (void*) (uintptr_t) &Java_java_lang_VMThread_nativeSetPriority },
	{ (char*) "nativeStop",        (char*) "(Ljava/lang/Throwable;)V", (void*) (uintptr_t) &Java_java_lang_VMThread_nativeStop        },
	{ (char*) "currentThread",     (char*) "()Ljava/lang/Thread;",     (void*) (uintptr_t) &Java_java_lang_VMThread_currentThread     },
	{ (char*) "yield",             (char*) "()V",                      (void*) (uintptr_t) &Java_java_lang_VMThread_yield             },
	{ (char*) "sleep",             (char*) "(JI)V",                    (void*) (uintptr_t) &Java_java_lang_VMThread_sleep             },
	{ (char*) "interrupted",       (char*) "()Z",                      (void*) (uintptr_t) &Java_java_lang_VMThread_interrupted       },
	{ (char*) "holdsLock",         (char*) "(Ljava/lang/Object;)Z",    (void*) (uintptr_t) &Java_java_lang_VMThread_holdsLock         },
	{ (char*) "getState",          (char*) "()Ljava/lang/String;",     (void*) (uintptr_t) &Java_java_lang_VMThread_getState          },
};


/* _Jv_java_lang_VMThread_init *************************************************

   Register native functions.

*******************************************************************************/

void _Jv_java_lang_VMThread_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/VMThread");

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
