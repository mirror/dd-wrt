/* src/native/vm/openjdk/sun_misc_Perf.cpp - sun/misc/Perf

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2009 Theobroma Systems Ltd.

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

#include "native/jni.hpp"
#include "native/native.hpp"

#include "toolbox/logging.hpp"

#include "vm/utf8.hpp"
#include "vm/vm.hpp"
#include "vm/jit/builtin.hpp"

static jlong initial_timer;

// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     sun/misc/Perf
 * Method:    registerNatives
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_misc_Perf_registerNatives(JNIEnv *env, jclass clazz)
{
	/* The native methods of this function are already registered in
	   _Jv_sun_misc_Perf_init() which is called during VM
	   startup. */
}


/*
 * Class:     sun/misc/Perf
 * Method:    attach
 * Signature: (Ljava/lang/String;II)Ljava/nio/ByteBuffer;
 */
JNIEXPORT jobject JNICALL Java_sun_misc_Perf_attach(JNIEnv *env, jobject _this, jstring user, jint lvmid, jint mode)
{
	log_println("Java_sun_misc_Perf_attach: Not supported!");
	return NULL;
}


/*
 * Class:     sun/misc/Perf
 * Method:    createByteArray
 * Signature: (Ljava/lang/String;II[BI)Ljava/nio/ByteBuffer;
 */
JNIEXPORT jobject JNICALL Java_sun_misc_Perf_createByteArray(JNIEnv *env, jobject _this, jstring name, jint variability, jint units, jbyteArray value, jint max_length)
{
	log_println("Java_sun_misc_Perf_createByteArray: Not supported!");
	return NULL;
}

static uint64_t dummy_perf;

/*
 * Class:     sun/misc/Perf
 * Method:    createLong
 * Signature: (Ljava/lang/String;IIJ)Ljava/nio/ByteBuffer;
 */
JNIEXPORT jobject JNICALL Java_sun_misc_Perf_createLong(JNIEnv *env, jobject _this, jstring name, jint variability, jint units, jlong value)
{
	return env->functions->NewDirectByteBuffer(env, &dummy_perf, sizeof(dummy_perf));
}


/*
 * Class:     sun/misc/Perf
 * Method:    detach
 * Signature: (Ljava/nio/ByteBuffer;)V
 */
JNIEXPORT void JNICALL Java_sun_misc_Perf_detach(JNIEnv *env, jobject _this, jobject bb)
{
	log_println("Java_sun_misc_Perf_detach: Not supported!");
}


/*
 * Class:     sun/misc/Perf
 * Method:    highResCounter
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_sun_misc_Perf_highResCounter(JNIEnv *env, jobject _this)
{
	return builtin_nanotime()/1000 - initial_timer;
}


/*
 * Class:     sun/misc/Perf
 * Method:    highResFrequency
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_sun_misc_Perf_highResFrequency(JNIEnv *env, jobject _this)
{
	return 1000000;
}

} // extern "C"


/* native methods implemented by this file ************************************/

static JNINativeMethod methods[] = {
	{ (char*) "registerNatives",  (char*) "()V",                                            (void*) (uintptr_t) &Java_sun_misc_Perf_registerNatives  },
	{ (char*) "attach",           (char*) "(Ljava/lang/String;II)Ljava/nio/ByteBuffer;",    (void*) (uintptr_t) &Java_sun_misc_Perf_attach           },
	{ (char*) "createByteArray",  (char*) "(Ljava/lang/String;II[BI)Ljava/nio/ByteBuffer;", (void*) (uintptr_t) &Java_sun_misc_Perf_createByteArray  },
	{ (char*) "createLong",       (char*) "(Ljava/lang/String;IIJ)Ljava/nio/ByteBuffer;",   (void*) (uintptr_t) &Java_sun_misc_Perf_createLong       },
	{ (char*) "detach",           (char*) "(Ljava/nio/ByteBuffer;)V",                       (void*) (uintptr_t) &Java_sun_misc_Perf_detach           },
	{ (char*) "highResCounter",   (char*) "()J",                                            (void*) (uintptr_t) &Java_sun_misc_Perf_highResCounter   },
	{ (char*) "highResFrequency", (char*) "()J",                                            (void*) (uintptr_t) &Java_sun_misc_Perf_highResFrequency },
};


/* _Jv_sun_misc_Perf_init ******************************************************

   Register native functions.

*******************************************************************************/

void _Jv_sun_misc_Perf_init(void)
{
	initial_timer = builtin_nanotime()/1000;

	Utf8String u = Utf8String::from_utf8("sun/misc/Perf");

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
