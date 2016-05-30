/* src/native/vm/cldc1.1/java_lang_Float.cpp

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

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/java_lang_Float.h"
#endif

#include "vm/vm.hpp"

#include "vm/jit/builtin.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/Float
 * Method:    floatToIntBits
 * Signature: (F)I
 */
JNIEXPORT jint JNICALL Java_java_lang_Float_floatToIntBits(JNIEnv *env, jclass clazz, jfloat value)
{
	imm_union val;
	int e, f;

	val.f = value;

	e = val.i & 0x7f800000;
	f = val.i & 0x007fffff;

	if (e == FLT_POSINF && f != 0)
		return FLT_NAN;

	return val.i;
}

/*
 * Class:     java/lang/Float
 * Method:    intBitsToFloat
 * Signature: (I)F
 */
JNIEXPORT jfloat JNICALL Java_java_lang_Float_intBitsToFloat(JNIEnv *env, jclass clazz, jint value)
{
        imm_union val;
        val.i = value;
        return val.f;
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "floatToIntBits", (char*) "(F)I", (void*) (uintptr_t) &Java_java_lang_Float_floatToIntBits },
	{ (char*) "intBitsToFloat", (char*) "(I)F", (void*) (uintptr_t) &Java_java_lang_Float_intBitsToFloat }
};
 
 
/* _Jv_java_lang_Float_init ****************************************************
 
   Register native functions.
 
*******************************************************************************/

void _Jv_java_lang_Float_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/Float");

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
