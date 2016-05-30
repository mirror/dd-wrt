/* src/native/vm/cldc1.1/java_lang_Runtime.cpp

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

#include "mm/gc.hpp"

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/java_lang_Runtime.h"
#endif

#include "vm/utf8.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/Runtime
 * Method:    exitInternal
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_java_lang_Runtime_exitInternal(JNIEnv *env, jobject _this, jint status)
{
	vm_shutdown(status);
}


/*
 * Class:     java/lang/Runtime
 * Method:    freeMemory
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_java_lang_Runtime_freeMemory(JNIEnv *env, jobject _this)
{
	return gc_get_free_bytes();
}


/*
 * Class:     java/lang/Runtime
 * Method:    totalMemory
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_java_lang_Runtime_totalMemory(JNIEnv *env, jobject _this)
{
	return gc_get_heap_size();
}


/*
 * Class:     java/lang/Runtime
 * Method:    gc
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_Runtime_gc(JNIEnv *env, jobject _this)
{
	gc_call();
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "exitInternal", (char*) "(I)V", (void*) (uintptr_t) &Java_java_lang_Runtime_exitInternal },
	{ (char*) "freeMemory",   (char*) "()J",  (void*) (uintptr_t) &Java_java_lang_Runtime_freeMemory   },
	{ (char*) "totalMemory",  (char*) "()J",  (void*) (uintptr_t) &Java_java_lang_Runtime_totalMemory  },
	{ (char*) "gc",           (char*) "()V",  (void*) (uintptr_t) &Java_java_lang_Runtime_gc           },
};
 
 
/* _Jv_java_lang_Runtime_init **************************************************
 
   Register native functions.
 
*******************************************************************************/
 
void _Jv_java_lang_Runtime_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/Runtime");

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
