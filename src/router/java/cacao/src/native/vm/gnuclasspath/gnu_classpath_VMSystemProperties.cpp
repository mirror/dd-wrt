/* src/native/vm/gnuclasspath/gnu_classpath_VMSystemProperties.cpp

   Copyright (C) 1996-2014
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
#include <string.h>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/gnu_classpath_VMSystemProperties.h"
#endif

#include "toolbox/buffer.hpp"

#include "vm/exceptions.hpp"
#include "vm/properties.hpp"
#include "vm/vm.hpp"


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     gnu/classpath/VMSystemProperties
 * Method:    preInit
 * Signature: (Ljava/util/Properties;)V
 */
JNIEXPORT void JNICALL Java_gnu_classpath_VMSystemProperties_preInit(JNIEnv *env, jclass clazz, jobject properties)
{
	java_handle_t *p;

	p = (java_handle_t *) properties;

	if (p == NULL) {
		exceptions_throw_nullpointerexception();
		return;
	}

	// Fill the java.util.Properties object.
	VM::get_current()->get_properties().fill(p);
}


/*
 * Class:     gnu/classpath/VMSystemProperties
 * Method:    postInit
 * Signature: (Ljava/util/Properties;)V
 */
JNIEXPORT void JNICALL Java_gnu_classpath_VMSystemProperties_postInit(JNIEnv *env, jclass clazz, jobject properties)
{
	java_handle_t *p;
#if defined(ENABLE_JRE_LAYOUT)
	const char *java_home;
#endif

	p = (java_handle_t *) properties;

	if (p == NULL) {
		exceptions_throw_nullpointerexception();
		return;
	}

	/* post-set some properties */

#if defined(ENABLE_JRE_LAYOUT)
	/* XXX when we do it that way, we can't set these properties on
	   commandline */

	java_home = VM::get_current()->get_properties().get("java.home");

	Properties::put(p, "gnu.classpath.home", java_home);

	// Use sequence builder to assemble value.
	Buffer<> buf;
	
	buf.write("file://")
	   .write(java_home)
	   .write("/lib");

	Properties::put(p, "gnu.classpath.home.url", buf.c_str());
#endif
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "preInit",  (char*) "(Ljava/util/Properties;)V", (void*) (uintptr_t) &Java_gnu_classpath_VMSystemProperties_preInit  },
	{ (char*) "postInit", (char*) "(Ljava/util/Properties;)V", (void*) (uintptr_t) &Java_gnu_classpath_VMSystemProperties_postInit },
};


/* _Jv_gnu_classpat_VMSystemProperties_init ************************************

   Register native functions.

*******************************************************************************/

void _Jv_gnu_classpath_VMSystemProperties_init(void)
{
	Utf8String u = Utf8String::from_utf8("gnu/classpath/VMSystemProperties");

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
