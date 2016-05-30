/* src/native/vm/gnuclasspath/java_lang_VMThrowable.cpp

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
#include <assert.h>

#include "vm/types.hpp"

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/java_lang_VMThrowable.h"
#endif

#include "vm/array.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/exceptions.hpp"
#include "vm/globals.hpp"
#include "vm/javaobjects.hpp"
#include "vm/loader.hpp"
#include "vm/string.hpp"
#include "vm/vm.hpp"

#include "vm/jit/code.hpp"
#include "vm/jit/linenumbertable.hpp"
#include "vm/jit/stacktrace.hpp"



// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/VMThrowable
 * Method:    fillInStackTrace
 * Signature: (Ljava/lang/Throwable;)Ljava/lang/VMThrowable;
 */
JNIEXPORT jobject JNICALL Java_java_lang_VMThrowable_fillInStackTrace(JNIEnv *env, jclass clazz, jobject t)
{
	java_handle_t* h;
	java_handle_bytearray_t* ba;

	h = native_new_and_init(class_java_lang_VMThrowable);

	if (h == NULL)
		return NULL;

	java_lang_VMThrowable vmt(h);

	ba = stacktrace_get_current();

	if (ba == NULL)
		return NULL;

	vmt.set_vmdata(ba);

	return (jobject) vmt.get_handle();
}


/*
 * Class:     java/lang/VMThrowable
 * Method:    getStackTrace
 * Signature: (Ljava/lang/Throwable;)[Ljava/lang/StackTraceElement;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_VMThrowable_getStackTrace(JNIEnv *env, jobject _this, jobject t)
{
	java_lang_VMThrowable vmt(_this);

	// Get the stacktrace from the VMThrowable object.

	ByteArray ba(vmt.get_vmdata());

	// XXX Critical GC section?
	stacktrace_t* st = (stacktrace_t*) ba.get_raw_data_ptr();

	assert(st != NULL);

	return stacktrace_get_StackTraceElements(st);
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "fillInStackTrace", (char*) "(Ljava/lang/Throwable;)Ljava/lang/VMThrowable;",        (void*) (uintptr_t) &Java_java_lang_VMThrowable_fillInStackTrace },
	{ (char*) "getStackTrace",    (char*) "(Ljava/lang/Throwable;)[Ljava/lang/StackTraceElement;", (void*) (uintptr_t) &Java_java_lang_VMThrowable_getStackTrace    },
};


/* _Jv_java_lang_VMThrowable_init **********************************************

   Register native functions.

*******************************************************************************/

void _Jv_java_lang_VMThrowable_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/VMThrowable");

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
