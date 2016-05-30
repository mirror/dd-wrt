/* src/native/vm/cldc1.1/java_lang_Double.c

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
# include "native/include/java_lang_Double.h"
#endif

#include "vm/vm.hpp"

#include "vm/jit/builtin.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/Double
 * Method:    doubleToLongBits
 * Signature: (D)J
 */
JNIEXPORT jlong JNICALL Java_java_lang_Double_doubleToLongBits(JNIEnv *env, jclass clazz, jdouble doubleValue)
{
	jvalue val;
	s8  e, f;
	val.d = doubleValue;

#if defined(__IEEE_BYTES_LITTLE_ENDIAN)
	/* On little endian ARM processors when using FPA, word order of
	   doubles is still big endian. So take that into account here. When
	   using VFP, word order of doubles follows byte order. */

#define SWAP_DOUBLE(a)    (((a) << 32) | (((a) >> 32) & 0x00000000ffffffff))

	val.j = SWAP_DOUBLE(val.j);
#endif

	e = val.j & 0x7ff0000000000000LL;
	f = val.j & 0x000fffffffffffffLL;

	if (e == DBL_POSINF && f != 0L)
		val.j = DBL_NAN;

	return val.j;
}


/*
 * Class:     java/lang/Double
 * Method:    longBitsToDouble
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_java_lang_Double_longBitsToDouble(JNIEnv *env, jclass clazz, jlong longValue)
{
	jvalue val;
	val.j = longValue;

#if defined(__IEEE_BYTES_LITTLE_ENDIAN)
	val.j = SWAP_DOUBLE(val.j);
#endif

	return val.d;
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "doubleToLongBits", (char*) "(D)J", (void*) (uintptr_t) &Java_java_lang_Double_doubleToLongBits },
	{ (char*) "longBitsToDouble", (char*) "(J)D", (void*) (uintptr_t) &Java_java_lang_Double_longBitsToDouble },
};
 
 
/* _Jv_java_lang_Double_init ***************************************************
 
   Register native functions.
 
*******************************************************************************/

void _Jv_java_lang_Double_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/Double");

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
