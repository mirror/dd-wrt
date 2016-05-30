/* src/native/vm/cldc1.1/com_sun_cldchi_jvm_JVM.cpp

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
# include "native/include/com_sun_cldchi_jvm_JVM.h"
#endif

#include "vm/exceptions.hpp"
#include "vm/string.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     com/sun/cldchi/jvm/JVM
 * Method:    loadLibrary
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_sun_cldchi_jvm_JVM_loadLibrary(JNIEnv *env, jclass clazz, jstring libName)
{
	if (libName == NULL) {
		exceptions_throw_nullpointerexception();
		return;
	}

	// REMOVEME When we use Java-strings internally.
	Utf8String name = JavaString((java_handle_t*) libName).to_utf8();

	NativeLibrary nl(name);
	bool result = nl.load(env);

	// Check for error and throw an exception in case.
	if (result == false) {
		exceptions_throw_unsatisfiedlinkerror(name);
	}
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "loadLibrary", (char*) "(Ljava/lang/String;)V", (void*) (uintptr_t) &Java_com_sun_cldchi_jvm_JVM_loadLibrary },
};


/* _Jv_com_sun_cldchi_jvm_JVM_init *********************************************
 
   Register native functions.
 
*******************************************************************************/
 
void _Jv_com_sun_cldchi_jvm_JVM_init(void)
{
	Utf8String u = Utf8String::from_utf8("com/sun/cldchi/jvm/JVM");

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
