/* src/native/vm/gnuclasspath/java_lang_VMRuntime.cpp

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

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "mm/memory.hpp"
#include "mm/gc.hpp"

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/java_lang_VMRuntime.h"
#endif

#include "vm/jit/builtin.hpp"
#include "vm/exceptions.hpp"
#include "vm/os.hpp"
#include "vm/string.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"

#include "toolbox/buffer.hpp"

static bool finalizeOnExit = false;


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/VMRuntime
 * Method:    exit
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMRuntime_exit(JNIEnv *env, jclass clazz, jint status)
{
	if (finalizeOnExit)
		gc_finalize_all();

	vm_shutdown(status);
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    freeMemory
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_java_lang_VMRuntime_freeMemory(JNIEnv *env, jclass clazz)
{
	return gc_get_free_bytes();
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    totalMemory
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_java_lang_VMRuntime_totalMemory(JNIEnv *env, jclass clazz)
{
	return gc_get_heap_size();
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    maxMemory
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_java_lang_VMRuntime_maxMemory(JNIEnv *env, jclass clazz)
{
	return gc_get_max_heap_size();
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    gc
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_VMRuntime_gc(JNIEnv *env, jclass clazz)
{
	gc_call();
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    runFinalization
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_VMRuntime_runFinalization(JNIEnv *env, jclass clazz)
{
	gc_invoke_finalizers();
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    runFinalizersOnExit
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMRuntime_runFinalizersOnExit(JNIEnv *env, jclass clazz, jboolean value)
{
	/* XXX threading */

	finalizeOnExit = value;
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    runFinalizationsForExit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_java_lang_VMRuntime_runFinalizationForExit(JNIEnv *env, jclass clazz)
{
/*  	if (finalizeOnExit) { */
/*  		gc_call(); */
	/* gc_finalize_all(); */
/*  	} */
/*  	log_text("Java_java_lang_VMRuntime_runFinalizationForExit called"); */
	/*gc_finalize_all();*/
	/*gc_invoke_finalizers();*/
	/*gc_call();*/
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    traceInstructions
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMRuntime_traceInstructions(JNIEnv *env, jclass clazz, jboolean par1)
{
	/* not supported */
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    traceMethodCalls
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_java_lang_VMRuntime_traceMethodCalls(JNIEnv *env, jclass clazz, jboolean par1)
{
	/* not supported */
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    availableProcessors
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_java_lang_VMRuntime_availableProcessors(JNIEnv *env, jclass clazz)
{
	return os::processors_online();
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    nativeLoad
 * Signature: (Ljava/lang/String;Ljava/lang/ClassLoader;)I
 */
JNIEXPORT jint JNICALL Java_java_lang_VMRuntime_nativeLoad(JNIEnv *env, jclass clazz, jstring libname, jobject loader)
{
	classloader_t* cl = loader_hashtable_classloader_add((java_handle_t *) loader);

	/* REMOVEME When we use Java-strings internally. */

	if (libname == NULL) {
		exceptions_throw_nullpointerexception();
		return 0;
	}

	Utf8String name = JavaString((java_handle_t*) libname).to_utf8();

	NativeLibrary library(name, cl);
	return library.load(env);
}


/*
 * Class:     java/lang/VMRuntime
 * Method:    mapLibraryName
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_lang_VMRuntime_mapLibraryName(JNIEnv *env, jclass clazz, jstring libname)
{
	Utf8String     u;
	java_handle_t *o;

	if (libname == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	u = JavaString((java_handle_t*) libname).to_utf8();

	/* generate library name */

	Buffer<> buf;

	buf.write(NATIVE_LIBRARY_PREFIX)
	   .write(u)
	   .write(NATIVE_LIBRARY_SUFFIX);

	o = JavaString::from_utf8(buf.c_str());

	return (jstring) o;
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "exit",                   (char*) "(I)V",                                         (void*) (uintptr_t) &Java_java_lang_VMRuntime_exit                   },
	{ (char*) "freeMemory",             (char*) "()J",                                          (void*) (uintptr_t) &Java_java_lang_VMRuntime_freeMemory             },
	{ (char*) "totalMemory",            (char*) "()J",                                          (void*) (uintptr_t) &Java_java_lang_VMRuntime_totalMemory            },
	{ (char*) "maxMemory",              (char*) "()J",                                          (void*) (uintptr_t) &Java_java_lang_VMRuntime_maxMemory              },
	{ (char*) "gc",                     (char*) "()V",                                          (void*) (uintptr_t) &Java_java_lang_VMRuntime_gc                     },
	{ (char*) "runFinalization",        (char*) "()V",                                          (void*) (uintptr_t) &Java_java_lang_VMRuntime_runFinalization        },
	{ (char*) "runFinalizersOnExit",    (char*) "(Z)V",                                         (void*) (uintptr_t) &Java_java_lang_VMRuntime_runFinalizersOnExit    },
	{ (char*) "runFinalizationForExit", (char*) "()V",                                          (void*) (uintptr_t) &Java_java_lang_VMRuntime_runFinalizationForExit },
	{ (char*) "traceInstructions",      (char*) "(Z)V",                                         (void*) (uintptr_t) &Java_java_lang_VMRuntime_traceInstructions      },
	{ (char*) "traceMethodCalls",       (char*) "(Z)V",                                         (void*) (uintptr_t) &Java_java_lang_VMRuntime_traceMethodCalls       },
	{ (char*) "availableProcessors",    (char*) "()I",                                          (void*) (uintptr_t) &Java_java_lang_VMRuntime_availableProcessors    },
	{ (char*) "nativeLoad",             (char*) "(Ljava/lang/String;Ljava/lang/ClassLoader;)I", (void*) (uintptr_t) &Java_java_lang_VMRuntime_nativeLoad             },
	{ (char*) "mapLibraryName",         (char*) "(Ljava/lang/String;)Ljava/lang/String;",       (void*) (uintptr_t) &Java_java_lang_VMRuntime_mapLibraryName         },
};


/* _Jv_java_lang_VMRuntime_init ************************************************

   Register native functions.

*******************************************************************************/

void _Jv_java_lang_VMRuntime_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/VMRuntime");

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
