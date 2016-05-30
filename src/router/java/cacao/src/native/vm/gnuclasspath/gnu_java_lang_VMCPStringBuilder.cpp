/* src/native/vm/gnuclasspath/gnu_java_lang_VMCPStringBuilder.cpp

   Copyright (C) 2008-2013
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
#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/gnu_java_lang_VMCPStringBuilder.h"
#endif

#include "vm/array.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/exceptions.hpp"
#include "vm/globals.hpp"
#include "vm/javaobjects.hpp"
#include "vm/vm.hpp"

/*
 * Class:     gnu/java/lang/VMCPStringBuilder
 * Method:    toString
 * Signature: ([CII)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_gnu_java_lang_VMCPStringBuilder_toString(JNIEnv *env, jclass clazz, jcharArray value, jint startIndex, jint count)
{
	/* This is a native version of
	   java.lang.String.<init>([CIIZ)Ljava/lang/String; */

	if (startIndex < 0) {
/* 		exceptions_throw_stringindexoutofboundsexception("offset: " + offset); */
		exceptions_throw_stringindexoutofboundsexception();
		return NULL;
	}

	if (count < 0) {
/* 		exceptions_throw_stringindexoutofboundsexception("count: " + count); */
		exceptions_throw_stringindexoutofboundsexception();
		return NULL;
	}

    /* equivalent to: offset + count < 0 || offset + count > data.length */

	CharArray ca(value);

	if (ca.get_length() - startIndex < count) {
/* 		exceptions_throw_stringindexoutofboundsexception("offset + count: " + (offset + count)); */
		exceptions_throw_stringindexoutofboundsexception();
		return NULL;
	}

	return (jstring) (java_handle_t*) JavaString::from_array(ca.get_handle(), count, startIndex);
}


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "toString", (char*) "([CII)Ljava/lang/String;", (void*) (uintptr_t) &Java_gnu_java_lang_VMCPStringBuilder_toString },
};


/* _Jv_gnu_java_lang_VMCPStringBuilder *****************************************

   Register native functions.

*******************************************************************************/

void _Jv_gnu_java_lang_VMCPStringBuilder_init(void)
{
	Utf8String u = Utf8String::from_utf8("gnu/java/lang/VMCPStringBuilder");

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
