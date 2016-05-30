/* src/native/vm/gnuclasspath/java_lang_management_VMManagementFactory.cpp

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

#include <stdlib.h>

#include "vm/types.hpp"

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/java_lang_management_VMManagementFactory.h"
#endif

#include "toolbox/logging.hpp"

#include "vm/array.hpp"
#include "vm/globals.hpp"
#include "vm/vm.hpp"

#include "vm/jit/builtin.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/management/VMManagementFactory
 * Method:    getMemoryPoolNames
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_management_VMManagementFactory_getMemoryPoolNames(JNIEnv *env, jclass clazz)
{
	log_println("Java_java_lang_management_VMManagementFactory_getMemoryPoolNames: IMPLEMENT ME!");

	ObjectArray oa(0, class_java_lang_String);

	return oa.get_handle();
}


/*
 * Class:     java/lang/management/VMManagementFactory
 * Method:    getMemoryManagerNames
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_management_VMManagementFactory_getMemoryManagerNames(JNIEnv *env, jclass clazz)
{
	log_println("Java_java_lang_management_VMManagementFactory_getMemoryManagerNames: IMPLEMENT ME!");

	ObjectArray oa(0, class_java_lang_String);

	return oa.get_handle();
}


/*
 * Class:     java/lang/management/VMManagementFactory
 * Method:    getGarbageCollectorNames
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_management_VMManagementFactory_getGarbageCollectorNames(JNIEnv *env, jclass clazz)
{
	log_println("Java_java_lang_management_VMManagementFactory_getGarbageCollectorNames: IMPLEMENT ME!");

	ObjectArray oa(0, class_java_lang_String);

	return oa.get_handle();
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "getMemoryPoolNames",       (char*) "()[Ljava/lang/String;", (void*) (uintptr_t) &Java_java_lang_management_VMManagementFactory_getMemoryPoolNames       },
	{ (char*) "getMemoryManagerNames",    (char*) "()[Ljava/lang/String;", (void*) (uintptr_t) &Java_java_lang_management_VMManagementFactory_getMemoryManagerNames    },
	{ (char*) "getGarbageCollectorNames", (char*) "()[Ljava/lang/String;", (void*) (uintptr_t) &Java_java_lang_management_VMManagementFactory_getGarbageCollectorNames },
};


/* _Jv_java_lang_management_VMManagementFactory_init ***************************

   Register native functions.

*******************************************************************************/

void _Jv_java_lang_management_VMManagementFactory_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/management/VMManagementFactory");

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
