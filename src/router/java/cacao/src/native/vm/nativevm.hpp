/* src/native/vm/nativevm.hpp - Register native VM interface functions.

   Copyright (C) 2007, 2008
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


#ifndef _NATIVEVM_HPP
#define _NATIVEVM_HPP

#include "config.h"

/* function prototypes ********************************************************/

void nativevm_preinit(void);
bool nativevm_init(void);

#if defined(ENABLE_JAVASE)
# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

void _Jv_gnu_classpath_VMStackWalker_init();
void _Jv_gnu_classpath_VMSystemProperties_init();
void _Jv_gnu_java_lang_VMCPStringBuilder_init();
void _Jv_gnu_java_lang_management_VMClassLoadingMXBeanImpl_init();
void _Jv_gnu_java_lang_management_VMMemoryMXBeanImpl_init();
void _Jv_gnu_java_lang_management_VMRuntimeMXBeanImpl_init();
void _Jv_gnu_java_lang_management_VMThreadMXBeanImpl_init();
void _Jv_java_lang_VMClass_init();
void _Jv_java_lang_VMClassLoader_init();
void _Jv_java_lang_VMObject_init();
void _Jv_java_lang_VMRuntime_init();
void _Jv_java_lang_VMString_init();
void _Jv_java_lang_VMSystem_init();
void _Jv_java_lang_VMThread_init();
void _Jv_java_lang_VMThrowable_init();
void _Jv_java_lang_management_VMManagementFactory_init();
void _Jv_java_lang_reflect_VMConstructor_init();
void _Jv_java_lang_reflect_VMField_init();
void _Jv_java_lang_reflect_VMMethod_init();
void _Jv_java_lang_reflect_VMProxy_init();
void _Jv_java_security_VMAccessController_init();
void _Jv_java_util_concurrent_atomic_AtomicLong_init();
void _Jv_sun_misc_Unsafe_init();

#if defined(ENABLE_ANNOTATIONS)
void _Jv_sun_reflect_ConstantPool_init();
#endif

# elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

void _Jv_sun_misc_Perf_init();
void _Jv_sun_misc_Unsafe_init();

# else
#  error unknown classpath configuration
# endif

#elif defined(ENABLE_JAVAME_CLDC1_1)

void _Jv_com_sun_cldc_io_ResourceInputStream_init();
void _Jv_com_sun_cldc_io_j2me_socket_Protocol_init();
void _Jv_com_sun_cldchi_io_ConsoleOutputStream_init();
void _Jv_com_sun_cldchi_jvm_JVM_init();
void _Jv_java_lang_Class_init();
void _Jv_java_lang_Double_init();
void _Jv_java_lang_Float_init();
void _Jv_java_lang_Math_init();
void _Jv_java_lang_Object_init();
void _Jv_java_lang_Runtime_init();
void _Jv_java_lang_String_init();
void _Jv_java_lang_System_init();
void _Jv_java_lang_Thread_init();
void _Jv_java_lang_Throwable_init();

#else
# error unknown Java configuration
#endif

#endif // _NATIVEVM_HPP


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
