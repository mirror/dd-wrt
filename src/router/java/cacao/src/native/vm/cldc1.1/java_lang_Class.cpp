/* src/native/vm/cldc1.1/java_lang_Class.cpp

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
#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/java_lang_Class.h"
#endif

#include "vm/exceptions.hpp"
#include "vm/initialize.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/Class
 * Method:    forName
 * Signature: (Ljava/lang/String;)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_java_lang_Class_forName(JNIEnv *env, jclass clazz, jstring name)
{
	Utf8String ufile;
	Utf8String uname;
	JavaString sname = name;
	classinfo *c;
	char*      pos;
	int32_t    i;

	/* illegal argument */

	if (name == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	/* create utf string in which '.' is replaced by '/' */

	ufile = sname.to_utf8_dot_to_slash();
	uname = sname.to_utf8();

	/* name must not contain '/' (mauve test) */

	// FIXME Move this check into a function.
	for (const char *it = uname.begin(), *end = uname.end(); it != end; ++it) {
		if (*it == '/') {
			exceptions_throw_classnotfoundexception(uname);
			return NULL;
		}
	}

	/* try to load, ... */

	c = load_class_bootstrap(ufile);

	if (c == NULL)
	    return NULL;

	/* link, ... */

	if (!link_class(c))
		return NULL;
	
	/* ...and initialize it. */

	if (!initialize_class(c))
		return NULL;

	return (jclass) LLNI_classinfo_wrap(c);
}


/*
 * Class:     java/lang/Class
 * Method:    newInstance
 * Signature: ()Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_java_lang_Class_newInstance(JNIEnv *env, jclass _this)
{
	classinfo     *c;
	java_handle_t *o;

	c = LLNI_classinfo_unwrap(_this);

	o = native_new_and_init(c);

	return (jobject) o;
}


/*
 * Class:     java/lang/Class
 * Method:    isInstance
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_Class_isInstance(JNIEnv *env, jclass _this, jobject obj)
{
	classinfo     *c;
	java_handle_t *h;

	c = LLNI_classinfo_unwrap(_this);
	h = (java_handle_t *) obj;

	return class_is_instance(c, h);
}


/*
 * Class:     java/lang/Class
 * Method:    isAssignableFrom
 * Signature: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_Class_isAssignableFrom(JNIEnv *env, jclass _this, jclass cls)
{
	classinfo *to;
	classinfo *from;

	to   = LLNI_classinfo_unwrap(_this);
	from = LLNI_classinfo_unwrap(cls);

	if (from == NULL) {
		exceptions_throw_nullpointerexception();
		return 0;
	}

	return class_is_assignable_from(to, from);
}


/*
 * Class:     java/lang/Class
 * Method:    isInterface
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_Class_isInterface(JNIEnv *env, jclass _this)
{
	classinfo *c;

	c = LLNI_classinfo_unwrap(_this);

	return class_is_interface(c);
}


/*
 * Class:     java/lang/Class
 * Method:    isArray
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_Class_isArray(JNIEnv *env, jclass _this)
{
	classinfo *c;

	c = LLNI_classinfo_unwrap(_this);

	return class_is_array(c);
}


/*
 * Class:     java/lang/Class
 * Method:    getName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_lang_Class_getName(JNIEnv *env, jclass _this)
{
	classinfo *c;

	c = LLNI_classinfo_unwrap(_this);

	return (jstring) class_get_classname(c);
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "forName",          (char*) "(Ljava/lang/String;)Ljava/lang/Class;",(void*) (uintptr_t) &Java_java_lang_Class_forName          },
	{ (char*) "newInstance",      (char*) "()Ljava/lang/Object;",                 (void*) (uintptr_t) &Java_java_lang_Class_newInstance      },
	{ (char*) "isInstance",       (char*) "(Ljava/lang/Object;)Z",                (void*) (uintptr_t) &Java_java_lang_Class_isInstance       },
	{ (char*) "isAssignableFrom", (char*) "(Ljava/lang/Class;)Z",                 (void*) (uintptr_t) &Java_java_lang_Class_isAssignableFrom },
	{ (char*) "isInterface",      (char*) "()Z",                                  (void*) (uintptr_t) &Java_java_lang_Class_isInterface      },
	{ (char*) "isArray",          (char*) "()Z",                                  (void*) (uintptr_t) &Java_java_lang_Class_isArray          },
	{ (char*) "getName",          (char*) "()Ljava/lang/String;",                 (void*) (uintptr_t) &Java_java_lang_Class_getName          },
};


/* _Jv_java_lang_Class_init ****************************************************
 
   Register native functions.
 
*******************************************************************************/

void _Jv_java_lang_Class_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/Class");

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
