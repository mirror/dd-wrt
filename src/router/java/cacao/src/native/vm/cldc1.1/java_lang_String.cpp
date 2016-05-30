/* src/native/vm/cldc1.1/java_lang_String.cpp

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
#include <string.h>

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/java_lang_String.h"
#endif

#include "vm/array.hpp"
#include "vm/javaobjects.hpp"
#include "vm/string.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/String
 * Method:    hashCode
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_java_lang_String_hashCode(JNIEnv *env, jstring _this)
{
	java_lang_String jls(_this);

	CharArray value(jls.get_value());

	int32_t offset = jls.get_offset();
	int32_t count  = jls.get_count();

	int32_t hash = 0;

	for (int32_t i = 0; i < count; i++) {
		hash = (31 * hash) + value.get_element(offset + i);
	}

	return hash;
}


/*
 * Class:     java/lang/String
 * Method:    indexOf
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_java_lang_String_indexOf__I(JNIEnv *env, jstring _this, jint ch)
{
	java_lang_String jls(_this);

	CharArray value(jls.get_value());

	int32_t offset = jls.get_offset();
	int32_t count  = jls.get_count();

	for (int32_t i = 0; i < count; i++) {
		if (value.get_element(offset + i) == ch) {
			return i;
		}
	}

	return -1;
}


/*
 * Class:     java/lang/String
 * Method:    indexOf
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_java_lang_String_indexOf__II(JNIEnv *env, jstring _this, jint ch, jint fromIndex)
{
	java_lang_String jls(_this);

	CharArray value(jls.get_value());

	int32_t offset = jls.get_offset();
	int32_t count  = jls.get_count();

	if (fromIndex < 0) {
		fromIndex = 0;
	}
	else if (fromIndex >= count) {
		// Note: fromIndex might be near -1>>>1.
		return -1;
	}

	for (int32_t i = fromIndex ; i < count ; i++) {
		if (value.get_element(offset + i) == ch) {
			return i;
		}
	}

	return -1;
}


/*
 * Class:     java/lang/String
 * Method:    lastIndexOf
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_java_lang_String_lastIndexOf__II(JNIEnv *env, jstring _this, jint ch, jint fromIndex)
{
	java_lang_String jls(_this);

	CharArray value(jls.get_value());

	int32_t offset = jls.get_offset();
	int32_t count  = jls.get_count();

	int32_t start = ((fromIndex >= count) ? count - 1 : fromIndex);

	for (int32_t i = start; i >= 0; i--) {
		if (value.get_element(offset + i) == ch) {
			return i;
		}
	}

	return -1;
}


/*
 * Class:     java/lang/String
 * Method:    lastIndexOf
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_java_lang_String_lastIndexOf__I(JNIEnv *env, jstring _this, jint ch)
{
	java_lang_String jls(_this);
	
	return Java_java_lang_String_lastIndexOf__II(env, _this, ch, jls.get_count() - 1);
}


/*
 * Class:     java/lang/String
 * Method:    intern
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_lang_String_intern(JNIEnv *env, jstring _this)
{
	java_lang_String jls(_this);

	if (jls.is_null())
		return NULL;

	return (jstring) JavaString(jls.get_handle()).intern();
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "hashCode",    (char*) "()I",                    (void*) (uintptr_t) &Java_java_lang_String_hashCode        },
	{ (char*) "indexOf",     (char*) "(I)I",                   (void*) (uintptr_t) &Java_java_lang_String_indexOf__I      },
	{ (char*) "indexOf",     (char*) "(II)I",                  (void*) (uintptr_t) &Java_java_lang_String_indexOf__II     },
	{ (char*) "lastIndexOf", (char*) "(II)I",                  (void*) (uintptr_t) &Java_java_lang_String_lastIndexOf__II },
	{ (char*) "lastIndexOf", (char*) "(I)I",                   (void*) (uintptr_t) &Java_java_lang_String_lastIndexOf__I  },
#if 0
	{ (char*) "equals",      (char*) "(Ljava/lang/Object;)Z;", (void*) (uintptr_t) &Java_java_lang_String_equals          },
#endif
	{ (char*) "intern",      (char*) "()Ljava/lang/String;",   (void*) (uintptr_t) &Java_java_lang_String_intern          },
};


/* _Jv_java_lang_String_init ***************************************************
 
   Register native functions.
 
*******************************************************************************/
 
void _Jv_java_lang_String_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/String");
 
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
