/* src/native/vm/cldc1.1/java_lang_Object.c

   Copyright (C) 2006-2013
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
#include <stdlib.h>

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/java_lang_Object.h"
#endif

#include "threads/lock.hpp"

#include "vm/exceptions.hpp"
#include "vm/javaobjects.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/Object
 * Method:    getClass
 * Signature: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_java_lang_Object_getClass(JNIEnv *env, jobject obj)
{
	if (obj == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	java_lang_Object o(obj);

	return (jclass) LLNI_classinfo_wrap(o.get_Class());
}


/*
 * Class:     java/lang/Object
 * Method:    hashCode
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_java_lang_Object_hashCode(JNIEnv *env, jobject _this)
{
	java_lang_Object o(_this);

	return o.get_hashcode();
}


/*
 * Class:     java/lang/Object
 * Method:    notify
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_Object_notify(JNIEnv *env, jobject _this)
{
	lock_notify_object((java_handle_t *) _this);
}


/*
 * Class:     java/lang/Object
 * Method:    notifyAll
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_Object_notifyAll(JNIEnv *env, jobject _this)
{
	lock_notify_all_object((java_handle_t *) _this);
}


/*
 * Class:     java/lang/Object
 * Method:    wait
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_java_lang_Object_wait(JNIEnv *env, jobject _this, jlong timeout)
{
	lock_wait_for_object((java_handle_t *) _this, timeout, 0);
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "getClass",  (char*) "()Ljava/lang/Class;", (void*) (uintptr_t) &Java_java_lang_Object_getClass  },
	{ (char*) "hashCode",  (char*) "()I",                 (void*) (uintptr_t) &Java_java_lang_Object_hashCode  },
	{ (char*) "notify",    (char*) "()V",                 (void*) (uintptr_t) &Java_java_lang_Object_notify    },
	{ (char*) "notifyAll", (char*) "()V",                 (void*) (uintptr_t) &Java_java_lang_Object_notifyAll },
	{ (char*) "wait",      (char*) "(J)V",                (void*) (uintptr_t) &Java_java_lang_Object_wait      },
};
 
 
/* _Jv_java_lang_Object_init ***************************************************
 
   Register native functions.
 
*******************************************************************************/
 
void _Jv_java_lang_Object_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/Object");

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
