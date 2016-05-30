/* src/native/vm/gnuclasspath/java_lang_VMClass.cpp

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
# include "native/vm/include/java_lang_VMClass.h"
#endif

#include "vm/class.hpp"
#include "vm/exceptions.hpp"
#include "vm/globals.hpp"
#include "vm/initialize.hpp"
#include "vm/javaobjects.hpp"
#include "vm/string.hpp"
#include "vm/utf8.hpp"

#if defined(ENABLE_ANNOTATIONS)
#include "vm/annotation.hpp"
#include "vm/vm.hpp"
#endif


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/VMClass
 * Method:    isInstance
 * Signature: (Ljava/lang/Class;Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMClass_isInstance(JNIEnv *env, jclass clazz, jclass klass, jobject o)
{
	classinfo     *c;
	java_handle_t *h;

	c = LLNI_classinfo_unwrap(klass);
	h = (java_handle_t *) o;

	return class_is_instance(c, h);
}


/*
 * Class:     java/lang/VMClass
 * Method:    isAssignableFrom
 * Signature: (Ljava/lang/Class;Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMClass_isAssignableFrom(JNIEnv *env, jclass clazz, jclass klass, jclass c)
{
	classinfo *to;
	classinfo *from;

	to   = LLNI_classinfo_unwrap(klass);
	from = LLNI_classinfo_unwrap(c);

	if (from == NULL) {
		exceptions_throw_nullpointerexception();
		return 0;
	}

	return class_is_assignable_from(to, from);
}


/*
 * Class:     java/lang/VMClass
 * Method:    isInterface
 * Signature: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMClass_isInterface(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo *c;

	c = LLNI_classinfo_unwrap(klass);

	return class_is_interface(c);
}


/*
 * Class:     java/lang/VMClass
 * Method:    isPrimitive
 * Signature: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMClass_isPrimitive(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo *c;

	c = LLNI_classinfo_unwrap(klass);

	return class_is_primitive(c);
}


/*
 * Class:     java/lang/VMClass
 * Method:    getName
 * Signature: (Ljava/lang/Class;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_lang_VMClass_getName(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo* c;

	c = LLNI_classinfo_unwrap(klass);

	return (jstring) class_get_classname(c);
}


/*
 * Class:     java/lang/VMClass
 * Method:    getSuperclass
 * Signature: (Ljava/lang/Class;)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_java_lang_VMClass_getSuperclass(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo *c;
	classinfo *super;

	c = LLNI_classinfo_unwrap(klass);

	super = class_get_superclass(c);

	return (jclass) LLNI_classinfo_wrap(super);
}


/*
 * Class:     java/lang/VMClass
 * Method:    getInterfaces
 * Signature: (Ljava/lang/Class;)[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_VMClass_getInterfaces(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo                 *c;
	java_handle_objectarray_t *oa;

	c = LLNI_classinfo_unwrap(klass);

	oa = class_get_interfaces(c);

	return (jobjectArray) oa;
}


/*
 * Class:     java/lang/VMClass
 * Method:    getComponentType
 * Signature: (Ljava/lang/Class;)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_java_lang_VMClass_getComponentType(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo *c;
	classinfo *component;
	
	c = LLNI_classinfo_unwrap(klass);
	
	component = class_get_componenttype(c);

	return (jclass) LLNI_classinfo_wrap(component);
}


/*
 * Class:     java/lang/VMClass
 * Method:    getModifiers
 * Signature: (Ljava/lang/Class;Z)I
 */
JNIEXPORT jint JNICALL Java_java_lang_VMClass_getModifiers(JNIEnv *env, jclass clazz, jclass klass, jboolean ignoreInnerClassesAttrib)
{
	classinfo *c;
	int32_t    flags;

	c = LLNI_classinfo_unwrap(klass);

	flags = class_get_modifiers(c, ignoreInnerClassesAttrib);

	return flags;
}


/*
 * Class:     java/lang/VMClass
 * Method:    getDeclaringClass
 * Signature: (Ljava/lang/Class;)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_java_lang_VMClass_getDeclaringClass(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo *c;
	classinfo *dc;

	c = LLNI_classinfo_unwrap(klass);

	dc = class_get_declaringclass(c);

	return (jclass) LLNI_classinfo_wrap(dc);
}


/*
 * Class:     java/lang/VMClass
 * Method:    getDeclaredClasses
 * Signature: (Ljava/lang/Class;Z)[Ljava/lang/Class;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_VMClass_getDeclaredClasses(JNIEnv *env, jclass clazz, jclass klass, jboolean publicOnly)
{
	classinfo                 *c;
	java_handle_objectarray_t *oa;

	c = LLNI_classinfo_unwrap(klass);

	oa = class_get_declaredclasses(c, publicOnly);

	return (jobjectArray) oa;
}


/*
 * Class:     java/lang/VMClass
 * Method:    getDeclaredFields
 * Signature: (Ljava/lang/Class;Z)[Ljava/lang/reflect/Field;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_VMClass_getDeclaredFields(JNIEnv *env, jclass clazz, jclass klass, jboolean publicOnly)
{
	classinfo                 *c;
	java_handle_objectarray_t *oa;

	c = LLNI_classinfo_unwrap(klass);

	oa = class_get_declaredfields(c, publicOnly);

	return (jobjectArray) oa;
}


/*
 * Class:     java/lang/VMClass
 * Method:    getDeclaredMethods
 * Signature: (Ljava/lang/Class;Z)[Ljava/lang/reflect/Method;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_VMClass_getDeclaredMethods(JNIEnv *env, jclass clazz, jclass klass, jboolean publicOnly)
{
	classinfo                 *c;
	java_handle_objectarray_t *oa;

	c = LLNI_classinfo_unwrap(klass);

	oa = class_get_declaredmethods(c, publicOnly);

	return (jobjectArray) oa;
}


/*
 * Class:     java/lang/VMClass
 * Method:    getDeclaredConstructors
 * Signature: (Ljava/lang/Class;Z)[Ljava/lang/reflect/Constructor;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_VMClass_getDeclaredConstructors(JNIEnv *env, jclass clazz, jclass klass, jboolean publicOnly)
{
	classinfo                 *c;
	java_handle_objectarray_t *oa;

	c = LLNI_classinfo_unwrap(klass);

	oa = class_get_declaredconstructors(c, publicOnly);

	return (jobjectArray) oa;
}


/*
 * Class:     java/lang/VMClass
 * Method:    getClassLoader
 * Signature: (Ljava/lang/Class;)Ljava/lang/ClassLoader;
 */
JNIEXPORT jobject JNICALL Java_java_lang_VMClass_getClassLoader(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo     *c;
	classloader_t *cl;

	c  = LLNI_classinfo_unwrap(klass);
	cl = class_get_classloader(c);

	return (jobject) cl;
}


/*
 * Class:     java/lang/VMClass
 * Method:    forName
 * Signature: (Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_java_lang_VMClass_forName(JNIEnv *env, jclass clazz, jstring name, jboolean initialize, jobject loader)
{
	classloader_t *cl;
	JavaString     sname;
	Utf8String     ufile;
	Utf8String     uname;
	classinfo     *c;

	sname = name;

	cl = loader_hashtable_classloader_add((java_handle_t *) loader);

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

	c = load_class_from_classloader(ufile, cl);

	if (c == NULL)
	    return NULL;

	/* link, ... */

	if (!link_class(c))
		return NULL;
	
	/* ...and initialize it, if required */

	if (initialize)
		if (!initialize_class(c))
			return NULL;

	return (jclass) LLNI_classinfo_wrap(c);
}


/*
 * Class:     java/lang/VMClass
 * Method:    isArray
 * Signature: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMClass_isArray(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo *c;

	c = LLNI_classinfo_unwrap(klass);

	return class_is_array(c);
}


/*
 * Class:     java/lang/VMClass
 * Method:    throwException
 * Signature: (Ljava/lang/Throwable;)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMClass_throwException(JNIEnv *env, jclass clazz, jobject t)
{
	java_handle_t *o;

	o = (java_handle_t *) t;

	exceptions_set_exception(o);
}


#if defined(ENABLE_ANNOTATIONS)
/*
 * Class:     java/lang/VMClass
 * Method:    getDeclaredAnnotations
 * Signature: (Ljava/lang/Class;)[Ljava/lang/annotation/Annotation;
 */
JNIEXPORT jobjectArray JNICALL Java_java_lang_VMClass_getDeclaredAnnotations(JNIEnv *env, jclass clazz, jclass klass)
{
	static methodinfo* m_parseAnnotationsIntoArray = NULL;

	if (klass == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}
	
	classinfo* c = LLNI_classinfo_unwrap(klass);

	/* get annotations: */
	java_handle_bytearray_t* annotations = class_get_annotations(c);

	java_handle_t* h = native_new_and_init(class_sun_reflect_ConstantPool);
	
	if (h == NULL)
		return NULL;

	sun_reflect_ConstantPool cp(h);
	cp.set_constantPoolOop(klass);

	/* only resolve the parser method the first time */
	if (m_parseAnnotationsIntoArray == NULL) {
		Utf8String utf_parseAnnotationsIntoArray = Utf8String::from_utf8("parseAnnotationsIntoArray");
		Utf8String utf_desc = Utf8String::from_utf8("([BLsun/reflect/ConstantPool;Ljava/lang/Class;)"
		                                           "[Ljava/lang/annotation/Annotation;");

		if (utf_parseAnnotationsIntoArray == NULL || utf_desc == NULL) {
			/* out of memory */
			return NULL;
		}

		m_parseAnnotationsIntoArray = class_resolveclassmethod(
			class_sun_reflect_annotation_AnnotationParser,
			utf_parseAnnotationsIntoArray,
			utf_desc,
			class_java_lang_Class,
			true);

		if (m_parseAnnotationsIntoArray == NULL) {
			/* method not found */
			return NULL;
		}
	}

	java_handle_objectarray_t* oa = (java_handle_objectarray_t*) vm_call_method(m_parseAnnotationsIntoArray, NULL, annotations, cp.get_handle(), klass);
	return (jobjectArray) oa;
}
#endif


/*
 * Class:     java/lang/VMClass
 * Method:    getEnclosingClass
 * Signature: (Ljava/lang/Class;)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_java_lang_VMClass_getEnclosingClass(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo *c;
	classinfo *result;

	c = LLNI_classinfo_unwrap(klass);

	result = class_get_enclosingclass(c);

	return (jclass) LLNI_classinfo_wrap(result);
}


/*
 * Class:     java/lang/VMClass
 * Method:    getEnclosingConstructor
 * Signature: (Ljava/lang/Class;)Ljava/lang/reflect/Constructor;
 */
JNIEXPORT jobject JNICALL Java_java_lang_VMClass_getEnclosingConstructor(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo*     c;
	java_handle_t* h;

	c = LLNI_classinfo_unwrap(klass);
	h = class_get_enclosingconstructor(c);

	return (jobject) h;
}


/*
 * Class:     java/lang/VMClass
 * Method:    getEnclosingMethod
 * Signature: (Ljava/lang/Class;)Ljava/lang/reflect/Method;
 */
JNIEXPORT jobject JNICALL Java_java_lang_VMClass_getEnclosingMethod(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo*     c;
	java_handle_t* h;

	c = LLNI_classinfo_unwrap(klass);
	h = class_get_enclosingmethod(c);

	return (jobject) h;
}


/*
 * Class:     java/lang/VMClass
 * Method:    getClassSignature
 * Signature: (Ljava/lang/Class;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_lang_VMClass_getClassSignature(JNIEnv *env, jclass clazz, jclass klass)
{
	classinfo *c = LLNI_classinfo_unwrap(klass);
	Utf8String u = class_get_signature(c);

	/* in error case return NULL */

	if (u == NULL)
		return NULL;

	return (jstring) JavaString::from_utf8(u);
}


/*
 * Class:     java/lang/VMClass
 * Method:    isAnonymousClass
 * Signature: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMClass_isAnonymousClass(JNIEnv *env, jclass clazz, jclass klass)
{
	return class_is_anonymousclass(LLNI_classinfo_unwrap(klass));
}


/*
 * Class:     java/lang/VMClass
 * Method:    isLocalClass
 * Signature: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMClass_isLocalClass(JNIEnv *env, jclass clazz, jclass klass)
{
	return class_is_localclass(LLNI_classinfo_unwrap(klass));
}


/*
 * Class:     java/lang/VMClass
 * Method:    isMemberClass
 * Signature: (Ljava/lang/Class;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_VMClass_isMemberClass(JNIEnv *env, jclass clazz, jclass klass)
{
	return class_is_memberclass(LLNI_classinfo_unwrap(klass));
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "isInstance",              (char*) "(Ljava/lang/Class;Ljava/lang/Object;)Z",                        (void*) (uintptr_t) &Java_java_lang_VMClass_isInstance              },
	{ (char*) "isAssignableFrom",        (char*) "(Ljava/lang/Class;Ljava/lang/Class;)Z",                         (void*) (uintptr_t) &Java_java_lang_VMClass_isAssignableFrom        },
	{ (char*) "isInterface",             (char*) "(Ljava/lang/Class;)Z",                                          (void*) (uintptr_t) &Java_java_lang_VMClass_isInterface             },
	{ (char*) "isPrimitive",             (char*) "(Ljava/lang/Class;)Z",                                          (void*) (uintptr_t) &Java_java_lang_VMClass_isPrimitive             },
	{ (char*) "getName",                 (char*) "(Ljava/lang/Class;)Ljava/lang/String;",                         (void*) (uintptr_t) &Java_java_lang_VMClass_getName                 },
	{ (char*) "getSuperclass",           (char*) "(Ljava/lang/Class;)Ljava/lang/Class;",                          (void*) (uintptr_t) &Java_java_lang_VMClass_getSuperclass           },
	{ (char*) "getInterfaces",           (char*) "(Ljava/lang/Class;)[Ljava/lang/Class;",                         (void*) (uintptr_t) &Java_java_lang_VMClass_getInterfaces           },
	{ (char*) "getComponentType",        (char*) "(Ljava/lang/Class;)Ljava/lang/Class;",                          (void*) (uintptr_t) &Java_java_lang_VMClass_getComponentType        },
	{ (char*) "getModifiers",            (char*) "(Ljava/lang/Class;Z)I",                                         (void*) (uintptr_t) &Java_java_lang_VMClass_getModifiers            },
	{ (char*) "getDeclaringClass",       (char*) "(Ljava/lang/Class;)Ljava/lang/Class;",                          (void*) (uintptr_t) &Java_java_lang_VMClass_getDeclaringClass       },
	{ (char*) "getDeclaredClasses",      (char*) "(Ljava/lang/Class;Z)[Ljava/lang/Class;",                        (void*) (uintptr_t) &Java_java_lang_VMClass_getDeclaredClasses      },
	{ (char*) "getDeclaredFields",       (char*) "(Ljava/lang/Class;Z)[Ljava/lang/reflect/Field;",                (void*) (uintptr_t) &Java_java_lang_VMClass_getDeclaredFields       },
	{ (char*) "getDeclaredMethods",      (char*) "(Ljava/lang/Class;Z)[Ljava/lang/reflect/Method;",               (void*) (uintptr_t) &Java_java_lang_VMClass_getDeclaredMethods      },
	{ (char*) "getDeclaredConstructors", (char*) "(Ljava/lang/Class;Z)[Ljava/lang/reflect/Constructor;",          (void*) (uintptr_t) &Java_java_lang_VMClass_getDeclaredConstructors },
	{ (char*) "getClassLoader",          (char*) "(Ljava/lang/Class;)Ljava/lang/ClassLoader;",                    (void*) (uintptr_t) &Java_java_lang_VMClass_getClassLoader          },
	{ (char*) "forName",                 (char*) "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;", (void*) (uintptr_t) &Java_java_lang_VMClass_forName                 },
	{ (char*) "isArray",                 (char*) "(Ljava/lang/Class;)Z",                                          (void*) (uintptr_t) &Java_java_lang_VMClass_isArray                 },
	{ (char*) "throwException",          (char*) "(Ljava/lang/Throwable;)V",                                      (void*) (uintptr_t) &Java_java_lang_VMClass_throwException          },
#if defined(ENABLE_ANNOTATIONS)
	{ (char*) "getDeclaredAnnotations",  (char*) "(Ljava/lang/Class;)[Ljava/lang/annotation/Annotation;",         (void*) (uintptr_t) &Java_java_lang_VMClass_getDeclaredAnnotations  },
#endif
	{ (char*) "getEnclosingClass",       (char*) "(Ljava/lang/Class;)Ljava/lang/Class;",                          (void*) (uintptr_t) &Java_java_lang_VMClass_getEnclosingClass       },
	{ (char*) "getEnclosingConstructor", (char*) "(Ljava/lang/Class;)Ljava/lang/reflect/Constructor;",            (void*) (uintptr_t) &Java_java_lang_VMClass_getEnclosingConstructor },
	{ (char*) "getEnclosingMethod",      (char*) "(Ljava/lang/Class;)Ljava/lang/reflect/Method;",                 (void*) (uintptr_t) &Java_java_lang_VMClass_getEnclosingMethod      },
	{ (char*) "getClassSignature",       (char*) "(Ljava/lang/Class;)Ljava/lang/String;",                         (void*) (uintptr_t) &Java_java_lang_VMClass_getClassSignature       },
	{ (char*) "isAnonymousClass",        (char*) "(Ljava/lang/Class;)Z",                                          (void*) (uintptr_t) &Java_java_lang_VMClass_isAnonymousClass        },
	{ (char*) "isLocalClass",            (char*) "(Ljava/lang/Class;)Z",                                          (void*) (uintptr_t) &Java_java_lang_VMClass_isLocalClass            },
	{ (char*) "isMemberClass",           (char*) "(Ljava/lang/Class;)Z",                                          (void*) (uintptr_t) &Java_java_lang_VMClass_isMemberClass           },
};


/* _Jv_java_lang_VMClass_init **************************************************

   Register native functions.

*******************************************************************************/

void _Jv_java_lang_VMClass_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/VMClass");

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
