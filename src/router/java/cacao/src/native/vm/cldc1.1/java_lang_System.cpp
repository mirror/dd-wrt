/* src/native/vm/cldc1.1/java_lang_System.cpp

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

#include "mm/memory.hpp"

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/java_lang_System.h"
#endif

#include "vm/jit/builtin.hpp"
#include "vm/javaobjects.hpp"
#include "vm/properties.hpp"
#include "vm/string.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/System
 * Method:    arraycopy
 * Signature: (Ljava/lang/Object;ILjava/lang/Object;II)V
 */
JNIEXPORT void JNICALL Java_java_lang_System_arraycopy(JNIEnv *env, jclass clazz, jobject src, jint srcStart, jobject dest, jint destStart, jint len)
{
	builtin_arraycopy((java_handle_t *) src, srcStart,
					  (java_handle_t *) dest, destStart, len);
}


/*
 * Class:     java/lang/System
 * Method:    getProperty0
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_java_lang_System_getProperty0(JNIEnv *env, jclass clazz, jstring s)
{
	java_handle_t *so;
	char*          key;
	const char*    value;
	java_handle_t *result;

	so = (java_handle_t *) s;

	/* build an ASCII string out of the java/lang/String passed */

	key = JavaString(so).to_chars();

	/* get the property from the internal table */

	value = VM::get_current()->get_properties().get(key);

	/* release the memory allocated in JavaString::to_chars */

	MFREE(key, char, 0);

	if (value == NULL)
		return NULL;

	result = JavaString::from_utf8(value);

	return (jstring) result;
}

/*
 * Class:     java/lang/System
 * Method:    identityHashCode
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_java_lang_System_identityHashCode(JNIEnv *env, jclass clazz, jobject obj)
{
        java_lang_Object o(obj);

        return o.get_hashcode();
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "arraycopy",    (char*) "(Ljava/lang/Object;ILjava/lang/Object;II)V", (void*) (uintptr_t) &Java_java_lang_System_arraycopy    },
	{ (char*) "getProperty0", (char*) "(Ljava/lang/String;)Ljava/lang/String;",     (void*) (uintptr_t) &Java_java_lang_System_getProperty0 },
	{ (char*) "identityHashCode", (char*) "(Ljava/lang/Object;)I",                  (void*) (uintptr_t) &Java_java_lang_System_identityHashCode }
};


/* _Jv_java_lang_System_init ***************************************************
 
   Register native functions.
 
*******************************************************************************/
 
void _Jv_java_lang_System_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/System");
 
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
