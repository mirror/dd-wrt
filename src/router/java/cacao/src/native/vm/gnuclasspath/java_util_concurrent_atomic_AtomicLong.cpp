/* src/native/vm/gnuclasspath/java_util_concurrent_atomic_AtomicLong.cpp

   Copyright (C) 2007-2013
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
# include "native/include/java_util_concurrent_atomic_AtomicLong.h"
#endif

#include "vm/utf8.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/util/concurrent/atomic/AtomicLong
 * Method:    VMSupportsCS8
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_java_util_concurrent_atomic_AtomicLong_VMSupportsCS8(JNIEnv *env, jclass clazz)
{
	// IMPLEMENT ME
	return 0;
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "VMSupportsCS8", (char*) "()Z", (void*) (uintptr_t) &Java_java_util_concurrent_atomic_AtomicLong_VMSupportsCS8 },
};


/* _Jv_java_util_concurrent_atomic_AtomicLong_init *****************************

   Register native functions.

*******************************************************************************/

void _Jv_java_util_concurrent_atomic_AtomicLong_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/util/concurrent/atomic/AtomicLong");

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
