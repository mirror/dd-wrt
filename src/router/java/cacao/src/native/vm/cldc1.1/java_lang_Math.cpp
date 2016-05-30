/* src/native/vm/cldc1.1/java_lang_Math.cpp

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

#include "fdlibm/fdlibm.h"

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/java_lang_Math.h"
#endif

#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/Math
 * Method:    ceil
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL Java_java_lang_Math_ceil(JNIEnv *env, jclass clazz, jdouble a)
{
	return ceil(a);
}


/*
 * Class:     java/lang/Math
 * Method:    cos
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL Java_java_lang_Math_cos(JNIEnv *env, jclass clazz, jdouble a)
{
	return cos(a);
}


/*
 * Class:     java/lang/Math
 * Method:    floor
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL Java_java_lang_Math_floor(JNIEnv *env, jclass clazz, jdouble a)
{
	return floor(a);
}


/*
 * Class:     java/lang/Math
 * Method:    sin
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL Java_java_lang_Math_sin(JNIEnv *env, jclass clazz, jdouble a)
{
	return sin(a);
}


/*
 * Class:     java/lang/Math
 * Method:    sqrt
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL Java_java_lang_Math_sqrt(JNIEnv *env, jclass clazz, jdouble a)
{
	return sqrt(a);
}


/*
 * Class:     java/lang/Math
 * Method:    tan
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL Java_java_lang_Math_tan(JNIEnv *env, jclass clazz, jdouble a)
{
	return tan(a);
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "ceil",  (char*) "(D)D", (void*) (uintptr_t) &Java_java_lang_Math_ceil  },
	{ (char*) "cos",   (char*) "(D)D", (void*) (uintptr_t) &Java_java_lang_Math_cos   },
	{ (char*) "floor", (char*) "(D)D", (void*) (uintptr_t) &Java_java_lang_Math_floor },
	{ (char*) "sin",   (char*) "(D)D", (void*) (uintptr_t) &Java_java_lang_Math_sin   },
	{ (char*) "sqrt",  (char*) "(D)D", (void*) (uintptr_t) &Java_java_lang_Math_sqrt  },
	{ (char*) "tan",   (char*) "(D)D", (void*) (uintptr_t) &Java_java_lang_Math_tan   },
};
 
 
/* _Jv_java_lang_Math_init *****************************************************
 
   Register native functions.
 
*******************************************************************************/

void _Jv_java_lang_Math_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/Math");

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
