/* src/native/vm/gnuclasspath/java_lang_reflect_VMProxy.c - java/lang/reflect/VMProxy

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

#include "native/jni.hpp"
#include "native/native.hpp"

// FIXME
//#include "native/include/java_lang_reflect_VMProxy.h"


/* native methods implemented by this file ************************************/

#if 0
static JNINativeMethod methods[] = {
	{ "getProxyClass",      "(Ljava/lang/ClassLoader;[Ljava/lang/Class;)Ljava/lang/Class;",                   (void *) (ptrint) &Java_java_lang_reflect_VMProxy_getProxyClass      },
	{ "getProxyData",       "(Ljava/lang/ClassLoader;[Ljava/lang/Class;)Ljava/lang/reflect/Proxy$ProxyData;", (void *) (ptrint) &Java_java_lang_reflect_VMProxy_getProxyData       },
	{ "generateProxyClass", "(Ljava/lang/ClassLoader;Ljava/lang/reflect/Proxy$ProxyData;)Ljava/lang/Class;",  (void *) (ptrint) &Java_java_lang_reflect_VMProxy_generateProxyClass },
};
#endif


/* _Jv_java_lang_reflect_VMProxy_init ******************************************

   Register native functions.

*******************************************************************************/

#if 0
void _Jv_java_lang_reflect_VMProxy_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/reflect/VMProxy");

	native_method_register(u, methods, NATIVE_METHODS_COUNT);
}
#endif


#if 0
/*
 * Class:     java/lang/reflect/VMProxy
 * Method:    getProxyClass
 * Signature: (Ljava/lang/ClassLoader;[Ljava/lang/Class;)Ljava/lang/Class;
 */
JNIEXPORT java_lang_Class* JNICALL Java_java_lang_reflect_VMProxy_getProxyClass(JNIEnv *env, jclass clazz, java_lang_ClassLoader *par1, java_objectarray *par2)
{
	return NULL;
}


/*
 * Class:     java/lang/reflect/VMProxy
 * Method:    getProxyData
 * Signature: (Ljava/lang/ClassLoader;[Ljava/lang/Class;)Ljava/lang/reflect/Proxy$ProxyData;
 */
JNIEXPORT struct java_lang_reflect_Proxy_ProxyData* JNICALL Java_java_lang_reflect_VMProxy_getProxyData(JNIEnv *env, jclass clazz, java_lang_ClassLoader *par1, java_objectarray *par2)
{
	return NULL;
}


/*
 * Class:     java/lang/reflect/VMProxy
 * Method:    generateProxyClass
 * Signature: (Ljava/lang/ClassLoader;Ljava/lang/reflect/Proxy$ProxyData;)Ljava/lang/Class;
 */
JNIEXPORT java_lang_Class* JNICALL Java_java_lang_reflect_VMProxy_generateProxyClass(JNIEnv *env, jclass clazz, java_lang_ClassLoader *par1, struct java_lang_reflect_Proxy_ProxyData *par2)
{
	return NULL;
}
#endif


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
