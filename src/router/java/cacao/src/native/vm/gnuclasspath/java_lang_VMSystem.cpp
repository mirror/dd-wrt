/* src/native/vm/gnuclasspath/java_lang_VMSystem.cpp - java/lang/VMSystem

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
#include <string.h>

#include "mm/gc.hpp"

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vminclude/java_lang_VMSystem.h"
#endif

#include "vm/jit/builtin.hpp"
#include "vm/javaobjects.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/VMSystem
 * Method:    arraycopy
 * Signature: (Ljava/lang/Object;ILjava/lang/Object;II)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMSystem_arraycopy(JNIEnv *env, jclass clazz, jobject src, jint srcStart, jobject dest, jint destStart, jint len)
{
	builtin_arraycopy((java_handle_t *) src, srcStart,
					  (java_handle_t *) dest, destStart, len);
}


/*
 * Class:     java/lang/VMSystem
 * Method:    identityHashCode
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_java_lang_VMSystem_identityHashCode(JNIEnv *env, jclass clazz, jobject obj)
{
	java_lang_Object o(obj);

	return o.get_hashcode();
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "arraycopy",        (char*) "(Ljava/lang/Object;ILjava/lang/Object;II)V", (void*) (uintptr_t) &Java_java_lang_VMSystem_arraycopy },
	{ (char*) "identityHashCode", (char*) "(Ljava/lang/Object;)I",                      (void*) (uintptr_t) &Java_java_lang_VMSystem_identityHashCode },
};


/* _Jv_java_lang_VMSystem_init *************************************************

   Register native functions.

*******************************************************************************/

void _Jv_java_lang_VMSystem_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/VMSystem");

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
