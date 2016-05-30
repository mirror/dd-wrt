/* src/native/vm/openjdk/jvm.cpp - HotSpot VM interface functions

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

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

#if defined(HAVE_SYS_IOCTL_H)
#define BSD_COMP /* Get FIONREAD on Solaris2 */
#include <sys/ioctl.h>
#endif

#include <sys/types.h>

// Include our JNI header before the JVM headers, because the JVM
// headers include jni.h and we want to override the typedefs in
// jni.h.
#include "native/jni.hpp"

// We include jvm_md.h before jvm.h as the latter includes the former.
#include INCLUDE_JVM_MD_H
#include INCLUDE_JVM_H

#include "fdlibm/fdlibm.h"

#include "mm/memory.hpp"

#include "native/llni.hpp"
#include "native/native.hpp"

#include "native/vm/reflection.hpp"

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
#include "native/vm/openjdk/hpi.hpp"
#endif
#include "native/vm/openjdk/management.hpp"

#include "threads/lock.hpp"
#include "threads/thread.hpp"
#include "threads/threadlist.hpp"

#include "toolbox/logging.hpp"
#include "toolbox/list.hpp"

#include "vm/array.hpp"

#if defined(ENABLE_ASSERTION)
#include "vm/assertion.hpp"
#endif

#include "vm/jit/builtin.hpp"
#include "vm/classcache.hpp"
#include "vm/exceptions.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/initialize.hpp"
#include "vm/javaobjects.hpp"
#include "vm/options.hpp"
#include "vm/os.hpp"
#include "vm/package.hpp"
#include "vm/primitive.hpp"
#include "vm/properties.hpp"
#include "vm/resolve.hpp"
#include "vm/signallocal.hpp"
#include "vm/string.hpp"
#include "vm/vm.hpp"

#include "vm/jit/stacktrace.hpp"


/* debugging macros ***********************************************************/

#if !defined(NDEBUG)

# define TRACEJVMCALLS(x)										\
    do {														\
        if (opt_TraceJVMCalls || opt_TraceJVMCallsVerbose) {	\
            log_println x;										\
        }														\
    } while (0)

# define TRACEJVMCALLSENTER(x)									\
    do {														\
        if (opt_TraceJVMCalls || opt_TraceJVMCallsVerbose) {	\
			log_start();										\
            log_print x;										\
        }														\
    } while (0)

# define TRACEJVMCALLSEXIT(x)									\
    do {														\
        if (opt_TraceJVMCalls || opt_TraceJVMCallsVerbose) {	\
			log_print x;										\
			log_finish();										\
        }														\
    } while (0)

# define TRACEJVMCALLSVERBOSE(x)				\
    do {										\
        if (opt_TraceJVMCallsVerbose) {			\
            log_println x;						\
        }										\
    } while (0)

# define PRINTJVMWARNINGS(x)					\
    do {										\
        if (opt_PrintWarnings) {				\
            log_println x;						\
        }										\
    } while (0)

#else

# define TRACEJVMCALLS(x)
# define TRACEJVMCALLSENTER(x)
# define TRACEJVMCALLSEXIT(x)
# define TRACEJVMCALLSVERBOSE(x)
# define PRINTJVMWARNINGS(x)

#endif


// Interface functions are exported as C functions.
extern "C" {

int jio_vsnprintf(char *str, size_t count, const char *fmt, va_list args)
{
	if ((intptr_t) count <= 0)
		return -1;

	return vsnprintf(str, count, fmt, args);
}


int jio_snprintf(char *str, size_t count, const char *fmt, ...)
{
	va_list ap;
	int     len;

	va_start(ap, fmt);
	len = jio_vsnprintf(str, count, fmt, ap);
	va_end(ap);

	return len;
}


int jio_fprintf(FILE* f, const char *fmt, ...)
{
	log_println("jio_fprintf: IMPLEMENT ME!");

	return 0;
}


int jio_vfprintf(FILE* f, const char *fmt, va_list args)
{
	log_println("jio_vfprintf: IMPLEMENT ME!");

	return 0;
}


int jio_printf(const char *fmt, ...)
{
	log_println("jio_printf: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetInterfaceVersion */

jint JVM_GetInterfaceVersion()
{
	/* This is defined in hotspot/src/share/vm/prims/jvm.h */

#define JVM_INTERFACE_VERSION 4

	return JVM_INTERFACE_VERSION;
}


/* JVM_CurrentTimeMillis */

jlong JVM_CurrentTimeMillis(JNIEnv *env, jclass ignored)
{
	TRACEJVMCALLS(("JVM_CurrentTimeMillis(env=%p, ignored=%p)", env, ignored));

	return (jlong) builtin_currenttimemillis();
}


/* JVM_NanoTime */

jlong JVM_NanoTime(JNIEnv *env, jclass ignored)
{
	TRACEJVMCALLS(("JVM_NanoTime(env=%p, ignored=%p)", env, ignored));

	return (jlong) builtin_nanotime();
}


/* JVM_ArrayCopy */

void JVM_ArrayCopy(JNIEnv *env, jclass ignored, jobject src, jint src_pos, jobject dst, jint dst_pos, jint length)
{
	java_handle_t *s;
	java_handle_t *d;

	s = (java_handle_t *) src;
	d = (java_handle_t *) dst;

	TRACEJVMCALLSVERBOSE(("JVM_ArrayCopy(env=%p, ignored=%p, src=%p, src_pos=%d, dst=%p, dst_pos=%d, length=%d)", env, ignored, src, src_pos, dst, dst_pos, length));

	builtin_arraycopy(s, src_pos, d, dst_pos, length);
}


/* JVM_InitProperties */

jobject JVM_InitProperties(JNIEnv *env, jobject properties)
{
	java_handle_t *h;
	char           buf[256];

	TRACEJVMCALLS(("JVM_InitProperties(env=%p, properties=%p)", env, properties));

	h = (java_handle_t *) properties;

	/* Convert the -XX:MaxDirectMemorySize= command line flag to the
	   sun.nio.MaxDirectMemorySize property.  Do this after setting
	   user properties to prevent people from setting the value with a
	   -D option, as requested. */

	jio_snprintf(buf, sizeof(buf), "%" PRId64, opt_MaxDirectMemorySize);
	VM::get_current()->get_properties().put("sun.nio.MaxDirectMemorySize", buf);

	// Fill the java.util.Properties object.
	VM::get_current()->get_properties().fill(h);

	return properties;
}


/* JVM_Exit */

void JVM_Exit(jint code)
{
	log_println("JVM_Exit: IMPLEMENT ME!");
}


/* JVM_Halt */

void JVM_Halt(jint code)
{
	TRACEJVMCALLS(("JVM_Halt(code=%d)", code));

/* 	vm_exit(code); */
	vm_shutdown(code);
}


/* JVM_OnExit(void (*func)) */

void JVM_OnExit(void (*func)(void))
{
	log_println("JVM_OnExit(void (*func): IMPLEMENT ME!");
}


/* JVM_GC */

void JVM_GC(void)
{
	TRACEJVMCALLS(("JVM_GC()"));

	gc_call();
}


/* JVM_MaxObjectInspectionAge */

jlong JVM_MaxObjectInspectionAge(void)
{
	log_println("JVM_MaxObjectInspectionAge: IMPLEMENT ME!");

	return 0;
}


/* JVM_TraceInstructions */

void JVM_TraceInstructions(jboolean on)
{
	log_println("JVM_TraceInstructions: IMPLEMENT ME!");
}


/* JVM_TraceMethodCalls */

void JVM_TraceMethodCalls(jboolean on)
{
	log_println("JVM_TraceMethodCalls: IMPLEMENT ME!");
}


/* JVM_TotalMemory */

jlong JVM_TotalMemory(void)
{
	TRACEJVMCALLS(("JVM_TotalMemory()"));

	return gc_get_heap_size();
}


/* JVM_FreeMemory */

jlong JVM_FreeMemory(void)
{
	TRACEJVMCALLS(("JVM_FreeMemory()"));

	return gc_get_free_bytes();
}


/* JVM_MaxMemory */

jlong JVM_MaxMemory(void)
{
	TRACEJVMCALLS(("JVM_MaxMemory()"));

	return gc_get_max_heap_size();
}


/* JVM_ActiveProcessorCount */

jint JVM_ActiveProcessorCount(void)
{
	TRACEJVMCALLS(("JVM_ActiveProcessorCount()"));

	return os::processors_online();
}


/* JVM_FillInStackTrace */

void JVM_FillInStackTrace(JNIEnv *env, jobject receiver)
{
	TRACEJVMCALLS(("JVM_FillInStackTrace(env=%p, receiver=%p)", env, receiver));

	java_handle_bytearray_t* ba = stacktrace_get_current();

	if (ba == NULL)
		return;

	java_lang_Throwable jlt(receiver, ba);
}


/* JVM_PrintStackTrace */

void JVM_PrintStackTrace(JNIEnv *env, jobject receiver, jobject printable)
{
	log_println("JVM_PrintStackTrace: IMPLEMENT ME!");
}


/* JVM_GetStackTraceDepth */

jint JVM_GetStackTraceDepth(JNIEnv *env, jobject throwable)
{
	TRACEJVMCALLS(("JVM_GetStackTraceDepth(env=%p, throwable=%p)", env, throwable));

	java_lang_Throwable jlt(throwable);

	if (jlt.is_null()) {
		exceptions_throw_nullpointerexception();
		return 0;
	}

	ByteArray ba(jlt.get_backtrace());

	if (ba.is_null())
		return 0;

	// We need a critical section here as the stacktrace structure is
	// mapped onto a Java byte-array.

	LLNI_CRITICAL_START;

	stacktrace_t* st = (stacktrace_t *) ba.get_raw_data_ptr();

	int32_t depth = st->length;

	LLNI_CRITICAL_END;

	return depth;
}


/* JVM_GetStackTraceElement */

jobject JVM_GetStackTraceElement(JNIEnv *env, jobject throwable, jint index)
{
	TRACEJVMCALLS(("JVM_GetStackTraceElement(env=%p, throwable=%p, index=%d)", env, throwable, index));

	java_lang_Throwable jlt(throwable);
	ByteArray ba(jlt.get_backtrace());

	// XXX We need a critical section here as the stacktrace structure is
	// mapped onto a Java byte-array.
	stacktrace_t* st = (stacktrace_t *) ba.get_raw_data_ptr();

	return stacktrace_get_StackTraceElement(st, index);
}


/* JVM_IHashCode */

jint JVM_IHashCode(JNIEnv* env, jobject handle)
{
	TRACEJVMCALLS(("JVM_IHashCode(env=%p, jobject=%p)", env, handle));

	java_lang_Object o(handle);

	return o.get_hashcode();
}


/* JVM_MonitorWait */

void JVM_MonitorWait(JNIEnv* env, jobject handle, jlong ms)
{
	TRACEJVMCALLS(("JVM_MonitorWait(env=%p, handle=%p, ms=%ld)", env, handle, ms));
    if (ms < 0) {
/* 		exceptions_throw_illegalargumentexception("argument out of range"); */
		exceptions_throw_illegalargumentexception();
		return;
	}

	lock_wait_for_object((java_handle_t*) handle, ms, 0);
}


/* JVM_MonitorNotify */

void JVM_MonitorNotify(JNIEnv* env, jobject handle)
{
	TRACEJVMCALLS(("JVM_MonitorNotify(env=%p, handle=%p)", env, handle));

	lock_notify_object((java_handle_t*) handle);
}


/* JVM_MonitorNotifyAll */

void JVM_MonitorNotifyAll(JNIEnv* env, jobject handle)
{
	TRACEJVMCALLS(("JVM_MonitorNotifyAll(env=%p, handle=%p)", env, handle));

	lock_notify_all_object((java_handle_t*) handle);
}


/* JVM_Clone */

jobject JVM_Clone(JNIEnv* env, jobject handle)
{
	TRACEJVMCALLS(("JVM_Clone(env=%p, handle=%p)", env, handle));

	return (jobject) builtin_clone(env, (java_handle_t *) handle);
}


/* JVM_InitializeCompiler  */

void JVM_InitializeCompiler (JNIEnv *env, jclass compCls)
{
	log_println("JVM_InitializeCompiler : IMPLEMENT ME!");
}


/* JVM_IsSilentCompiler */

jboolean JVM_IsSilentCompiler(JNIEnv *env, jclass compCls)
{
	log_println("JVM_IsSilentCompiler: IMPLEMENT ME!");

	return 0;
}


/* JVM_CompileClass */

jboolean JVM_CompileClass(JNIEnv *env, jclass compCls, jclass cls)
{
	log_println("JVM_CompileClass: IMPLEMENT ME!");

	return 0;
}


/* JVM_CompileClasses */

jboolean JVM_CompileClasses(JNIEnv *env, jclass cls, jstring jname)
{
	log_println("JVM_CompileClasses: IMPLEMENT ME!");

	return 0;
}


/* JVM_CompilerCommand */

jobject JVM_CompilerCommand(JNIEnv *env, jclass compCls, jobject arg)
{
	log_println("JVM_CompilerCommand: IMPLEMENT ME!");

	return 0;
}


/* JVM_EnableCompiler */

void JVM_EnableCompiler(JNIEnv *env, jclass compCls)
{
	TRACEJVMCALLS(("JVM_EnableCompiler(env=%p, compCls=%p)", env, compCls));
	PRINTJVMWARNINGS(("JVM_EnableCompiler not supported"));
}


/* JVM_DisableCompiler */

void JVM_DisableCompiler(JNIEnv *env, jclass compCls)
{
	TRACEJVMCALLS(("JVM_DisableCompiler(env=%p, compCls=%p)", env, compCls));
	PRINTJVMWARNINGS(("JVM_DisableCompiler not supported"));
}


/* JVM_GetLastErrorString */

jint JVM_GetLastErrorString(char* buf, int len)
{
	TRACEJVMCALLS(("JVM_GetLastErrorString(buf=%p, len=%d", buf, len));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return hpi.get_system().GetLastErrorString(buf, len);
#else
	//XXX IMPLEMENT ME!
	return 0;
#endif
}


/* JVM_NativePath */

char *JVM_NativePath(char* path)
{
	TRACEJVMCALLS(("JVM_NativePath(path=%s)", path));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return hpi.get_file().NativePath(path);
#else
	return path;
#endif
}


/* JVM_GetCallerClass */

jclass JVM_GetCallerClass(JNIEnv* env, int depth)
{
	classinfo *c;

	TRACEJVMCALLS(("JVM_GetCallerClass(env=%p, depth=%d)", env, depth));

	c = stacktrace_get_caller_class(depth);

	return (jclass) c;
}


/* JVM_FindPrimitiveClass */

jclass JVM_FindPrimitiveClass(JNIEnv* env, const char* s)
{
	TRACEJVMCALLS(("JVM_FindPrimitiveClass(env=%p, s=%s)", env, s));

	Utf8String u = Utf8String::from_utf8(s);
	classinfo *c = Primitive::get_class_by_name(u);

	return (jclass) LLNI_classinfo_wrap(c);
}


/* JVM_ResolveClass */

void JVM_ResolveClass(JNIEnv* env, jclass cls)
{
	TRACEJVMCALLS(("JVM_ResolveClass(env=%p, cls=%p)", env, cls));
	PRINTJVMWARNINGS(("JVM_ResolveClass not implemented"));
}


/* JVM_FindClassFromBootLoader */

jclass JVM_FindClassFromBootLoader(JNIEnv* env, const char* name)
{
	TRACEJVMCALLS(("JVM_FindClassFromBootLoader(name=%s)", name));

	Utf8String u = Utf8String::from_utf8(name);
	classinfo *c = load_class_from_classloader(u, NULL);

	if (c == NULL)
		return NULL;

	return (jclass) LLNI_classinfo_wrap(c);
}


/* JVM_FindClassFromClassLoader */

jclass JVM_FindClassFromClassLoader(JNIEnv* env, const char* name, jboolean init, jobject loader, jboolean throwError)
{
	TRACEJVMCALLS(("JVM_FindClassFromClassLoader(name=%s, init=%d, loader=%p, throwError=%d)", name, init, loader, throwError));

	/* As of now, OpenJDK does not call this function with throwError
	   is true. */

	assert(throwError == false);

	Utf8String     u  = Utf8String::from_utf8(name);
	classloader_t *cl = loader_hashtable_classloader_add((java_handle_t *) loader);
	classinfo     *c  = load_class_from_classloader(u, cl);

	if (c == NULL)
		return NULL;

	if (init)
		if (!(c->state & CLASS_INITIALIZED))
			if (!initialize_class(c))
				return NULL;

	return (jclass) LLNI_classinfo_wrap(c);
}


/* JVM_FindClassFromClass */

jclass JVM_FindClassFromClass(JNIEnv *env, const char *name, jboolean init, jclass from)
{
	log_println("JVM_FindClassFromClass: IMPLEMENT ME!");

	return NULL;
}


/* JVM_DefineClass */

jclass JVM_DefineClass(JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize len, jobject pd)
{
	log_println("JVM_DefineClass: IMPLEMENT ME!");

	return NULL;
}


/* JVM_DefineClassWithSource */

jclass JVM_DefineClassWithSource(JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize len, jobject pd, const char *source)
{
	TRACEJVMCALLS(("JVM_DefineClassWithSource(env=%p, name=%s, loader=%p, buf=%p, len=%d, pd=%p, source=%s)", env, name, loader, buf, len, pd, source));

	Utf8String     u  = (name != NULL) ? Utf8String::from_utf8(name) : NULL;
	classloader_t *cl = loader_hashtable_classloader_add((java_handle_t *) loader);

	/* XXX do something with source */

	classinfo *c = class_define(u, cl, len, (uint8_t *) buf, (java_handle_t *) pd);

	return (jclass) LLNI_classinfo_wrap(c);
}


/* JVM_FindLoadedClass */

jclass JVM_FindLoadedClass(JNIEnv *env, jobject loader, jstring name)
{
	TRACEJVMCALLS(("JVM_FindLoadedClass(env=%p, loader=%p, name=%p)", env, loader, name));

	classloader_t *cl = loader_hashtable_classloader_add((java_handle_t *) loader);
	Utf8String     u  = JavaString((java_handle_t*) name).to_utf8_dot_to_slash();
	classinfo     *c  = classcache_lookup(cl, u);

	return (jclass) LLNI_classinfo_wrap(c);
}


/* JVM_GetClassName */

jstring JVM_GetClassName(JNIEnv *env, jclass cls)
{
	classinfo* c;

	TRACEJVMCALLS(("JVM_GetClassName(env=%p, cls=%p)", env, cls));

	c = LLNI_classinfo_unwrap(cls);

	return (jstring) class_get_classname(c);
}


/* JVM_GetClassInterfaces */

jobjectArray JVM_GetClassInterfaces(JNIEnv *env, jclass cls)
{
	classinfo                 *c;
	java_handle_objectarray_t *oa;

	TRACEJVMCALLS(("JVM_GetClassInterfaces(env=%p, cls=%p)", env, cls));

	c = LLNI_classinfo_unwrap(cls);

	oa = class_get_interfaces(c);

	return oa;
}


/* JVM_GetClassLoader */

jobject JVM_GetClassLoader(JNIEnv *env, jclass cls)
{
	classinfo     *c;
	classloader_t *cl;

	TRACEJVMCALLSENTER(("JVM_GetClassLoader(env=%p, cls=%p)", env, cls));

	c  = LLNI_classinfo_unwrap(cls);
	cl = class_get_classloader(c);

	TRACEJVMCALLSEXIT(("->%p", cl));

	return (jobject) cl;
}


/* JVM_IsInterface */

jboolean JVM_IsInterface(JNIEnv *env, jclass cls)
{
	classinfo *c;

	TRACEJVMCALLS(("JVM_IsInterface(env=%p, cls=%p)", env, cls));

	c = LLNI_classinfo_unwrap(cls);

	return class_is_interface(c);
}


/* JVM_GetClassSigners */

jobjectArray JVM_GetClassSigners(JNIEnv *env, jclass cls)
{
	log_println("JVM_GetClassSigners: IMPLEMENT ME!");

	return NULL;
}


/* JVM_SetClassSigners */

void JVM_SetClassSigners(JNIEnv *env, jclass cls, jobjectArray signers)
{
	TRACEJVMCALLS(("JVM_SetClassSigners(env=%p, cls=%p, signers=%p)", env, cls, signers));

	classinfo* c = LLNI_classinfo_unwrap(cls);

	ObjectArray oa(signers);

    /* This call is ignored for primitive types and arrays.  Signers
	   are only set once, ClassLoader.java, and thus shouldn't be
	   called with an array.  Only the bootstrap loader creates
	   arrays. */

	if (class_is_primitive(c) || class_is_array(c))
		return;

	// XXX: Fix this!
	LLNI_classinfo_field_set(c, signers, (java_objectarray_t*) oa.get_handle());
}


/* JVM_GetProtectionDomain */

jobject JVM_GetProtectionDomain(JNIEnv *env, jclass cls)
{
	classinfo *c;

	TRACEJVMCALLS(("JVM_GetProtectionDomain(env=%p, cls=%p)", env, cls));

	c = LLNI_classinfo_unwrap(cls);

	if (c == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

    /* Primitive types do not have a protection domain. */

	if (class_is_primitive(c))
		return NULL;

	return (jobject) c->protectiondomain;
}


/* JVM_SetProtectionDomain */

void JVM_SetProtectionDomain(JNIEnv *env, jclass cls, jobject protection_domain)
{
	log_println("JVM_SetProtectionDomain: IMPLEMENT ME!");
}


/* JVM_DoPrivileged */

jobject JVM_DoPrivileged(JNIEnv *env, jclass cls, jobject action, jobject context, jboolean wrapException)
{
	java_handle_t *h;
	classinfo     *c;
	methodinfo    *m;
	java_handle_t *result;
	java_handle_t *e;

	TRACEJVMCALLS(("JVM_DoPrivileged(env=%p, cls=%p, action=%p, context=%p, wrapException=%d)", env, cls, action, context, wrapException));

	h = (java_handle_t *) action;
	LLNI_class_get(h, c);

	if (action == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	/* lookup run() method (throw no exceptions) */

	m = class_resolveclassmethod(c, utf8::run, utf8::void__java_lang_Object, c,
								 false);

	if ((m == NULL) || !(m->flags & ACC_PUBLIC) || (m->flags & ACC_STATIC)) {
		exceptions_throw_internalerror("No run method");
		return NULL;
	}

	/* XXX It seems something with a privileged stack needs to be done
	   here. */

	result = vm_call_method(m, h);

	e = exceptions_get_exception();

	if (e != NULL) {
		if ( builtin_instanceof(e, class_java_lang_Exception) &&
			!builtin_instanceof(e, class_java_lang_RuntimeException)) {
			exceptions_clear_exception();
			exceptions_throw_privilegedactionexception(e);
		}

		return NULL;
	}

	return (jobject) result;
}


/* JVM_GetInheritedAccessControlContext */

jobject JVM_GetInheritedAccessControlContext(JNIEnv *env, jclass cls)
{
	log_println("JVM_GetInheritedAccessControlContext: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetStackAccessControlContext */

jobject JVM_GetStackAccessControlContext(JNIEnv *env, jclass cls)
{
	TRACEJVMCALLS(("JVM_GetStackAccessControlContext(env=%p, cls=%p): IMPLEMENT ME!", env, cls));

	/* XXX All stuff I tested so far works without that function.  At
	   some point we have to implement it, but I disable the output
	   for now to make IcedTea happy. */

	return NULL;
}


/* JVM_IsArrayClass */

jboolean JVM_IsArrayClass(JNIEnv *env, jclass cls)
{
	classinfo *c;

	TRACEJVMCALLS(("JVM_IsArrayClass(env=%p, cls=%p)", env, cls));

	c = LLNI_classinfo_unwrap(cls);

	return class_is_array(c);
}


/* JVM_IsPrimitiveClass */

jboolean JVM_IsPrimitiveClass(JNIEnv *env, jclass cls)
{
	classinfo *c;

	TRACEJVMCALLS(("JVM_IsPrimitiveClass(env=%p, cls=%p)", env, cls));

	c = LLNI_classinfo_unwrap(cls);

	return class_is_primitive(c);
}


/* JVM_GetComponentType */

jclass JVM_GetComponentType(JNIEnv *env, jclass cls)
{
	classinfo *component;
	classinfo *c;
	
	TRACEJVMCALLS(("JVM_GetComponentType(env=%p, cls=%p)", env, cls));

	c = LLNI_classinfo_unwrap(cls);
	
	component = class_get_componenttype(c);

	return (jclass) LLNI_classinfo_wrap(component);
}


/* JVM_GetClassModifiers */

jint JVM_GetClassModifiers(JNIEnv *env, jclass cls)
{
	classinfo *c;
	int32_t    flags;

	TRACEJVMCALLS(("JVM_GetClassModifiers(env=%p, cls=%p)", env, cls));

	c = LLNI_classinfo_unwrap(cls);

	flags = class_get_modifiers(c, false);

	return flags;
}


/* JVM_GetDeclaredClasses */

jobjectArray JVM_GetDeclaredClasses(JNIEnv *env, jclass ofClass)
{
	classinfo                 *c;
	java_handle_objectarray_t *oa;

	TRACEJVMCALLS(("JVM_GetDeclaredClasses(env=%p, ofClass=%p)", env, ofClass));

	c = LLNI_classinfo_unwrap(ofClass);

	oa = class_get_declaredclasses(c, false);

	return oa;
}


/* JVM_GetDeclaringClass */

jclass JVM_GetDeclaringClass(JNIEnv *env, jclass ofClass)
{
	classinfo *c;
	classinfo *dc;

	TRACEJVMCALLS(("JVM_GetDeclaringClass(env=%p, ofClass=%p)", env, ofClass));

	c = LLNI_classinfo_unwrap(ofClass);

	dc = class_get_declaringclass(c);

	return (jclass) LLNI_classinfo_wrap(dc);
}


/* JVM_GetClassSignature */

jstring JVM_GetClassSignature(JNIEnv *env, jclass cls)
{
	TRACEJVMCALLS(("JVM_GetClassSignature(env=%p, cls=%p)", env, cls));

	classinfo *c = LLNI_classinfo_unwrap(cls);

	/* Get the signature of the class. */

	Utf8String u = class_get_signature(c);

	if (u == NULL)
		return NULL;

	/* Convert UTF-string to a Java-string. */

	return (jstring) JavaString::from_utf8(u);
}


/* JVM_GetClassAnnotations */

jbyteArray JVM_GetClassAnnotations(JNIEnv *env, jclass cls)
{
	TRACEJVMCALLS(("JVM_GetClassAnnotations(env=%p, cls=%p)", env, cls));

	if (cls == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}
	
	classinfo* c = LLNI_classinfo_unwrap(cls);

	/* get annotations: */
	java_handle_bytearray_t* annotations = class_get_annotations(c);

	return (jbyteArray) annotations;
}


/* JVM_GetFieldAnnotations */

jbyteArray JVM_GetFieldAnnotations(JNIEnv *env, jobject field)
{
	TRACEJVMCALLS(("JVM_GetFieldAnnotations(env=%p, field=%p)", env, field));

	java_lang_reflect_Field jlrf(field);

	if (jlrf.is_null()) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	return (jbyteArray) jlrf.get_annotations();
}


/* JVM_GetMethodAnnotations */

jbyteArray JVM_GetMethodAnnotations(JNIEnv *env, jobject method)
{
	TRACEJVMCALLS(("JVM_GetMethodAnnotations(env=%p, method=%p)", env, method));

	java_lang_reflect_Method jlrm(method);

	if (jlrm.is_null()) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	return (jbyteArray) jlrm.get_annotations();
}


/* JVM_GetMethodDefaultAnnotationValue */

jbyteArray JVM_GetMethodDefaultAnnotationValue(JNIEnv *env, jobject method)
{
	TRACEJVMCALLS(("JVM_GetMethodDefaultAnnotationValue(env=%p, method=%p)", env, method));

	java_lang_reflect_Method jlrm(method);

	if (jlrm.is_null()) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	return (jbyteArray) jlrm.get_annotationDefault();
}


/* JVM_GetMethodParameterAnnotations */

jbyteArray JVM_GetMethodParameterAnnotations(JNIEnv *env, jobject method)
{
	TRACEJVMCALLS(("JVM_GetMethodParameterAnnotations(env=%p, method=%p)", env, method));

	java_lang_reflect_Method jlrm(method);

	if (jlrm.is_null()) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	return (jbyteArray) jlrm.get_parameterAnnotations();
}


/* JVM_GetClassDeclaredFields */

jobjectArray JVM_GetClassDeclaredFields(JNIEnv *env, jclass ofClass, jboolean publicOnly)
{
	classinfo                 *c;
	java_handle_objectarray_t *oa;

	TRACEJVMCALLS(("JVM_GetClassDeclaredFields(env=%p, ofClass=%p, publicOnly=%d)", env, ofClass, publicOnly));

	c = LLNI_classinfo_unwrap(ofClass);

	oa = class_get_declaredfields(c, publicOnly);

	return oa;
}


/* JVM_GetClassDeclaredMethods */

jobjectArray JVM_GetClassDeclaredMethods(JNIEnv *env, jclass ofClass, jboolean publicOnly)
{
	TRACEJVMCALLS(("JVM_GetClassDeclaredMethods(env=%p, ofClass=%p, publicOnly=%d)", env, ofClass, publicOnly));

	classinfo* c = LLNI_classinfo_unwrap(ofClass);

	java_handle_objectarray_t* oa = class_get_declaredmethods(c, publicOnly);

	return oa;
}


/* JVM_GetClassDeclaredConstructors */

jobjectArray JVM_GetClassDeclaredConstructors(JNIEnv *env, jclass ofClass, jboolean publicOnly)
{
	classinfo                 *c;
	java_handle_objectarray_t *oa;

	TRACEJVMCALLS(("JVM_GetClassDeclaredConstructors(env=%p, ofClass=%p, publicOnly=%d)", env, ofClass, publicOnly));

	c = LLNI_classinfo_unwrap(ofClass);

	oa = class_get_declaredconstructors(c, publicOnly);

	return oa;
}


/* JVM_GetClassAccessFlags */

jint JVM_GetClassAccessFlags(JNIEnv *env, jclass cls)
{
	classinfo *c;

	TRACEJVMCALLS(("JVM_GetClassAccessFlags(env=%p, cls=%p)", env, cls));

	c = LLNI_classinfo_unwrap(cls);

	/* Primitive type classes have the correct access flags. */

	return c->flags & ACC_CLASS_REFLECT_MASK;
}


/* JVM_GetClassConstantPool */

jobject JVM_GetClassConstantPool(JNIEnv *env, jclass cls)
{
#if defined(ENABLE_ANNOTATIONS)
	TRACEJVMCALLS(("JVM_GetClassConstantPool(env=%p, cls=%p)", env, cls));

	java_handle_t* h = native_new_and_init(class_sun_reflect_ConstantPool);
	sun_reflect_ConstantPool cp(h, cls);
	
	if (cp.is_null()) {
		return NULL;
	}
	
	return (jobject) cp.get_handle();
#else
	log_println("JVM_GetClassConstantPool(env=%p, cls=%p): not implemented in this configuration!", env, cls);
	return NULL;
#endif
}


/* JVM_ConstantPoolGetSize */

jint JVM_ConstantPoolGetSize(JNIEnv *env, jobject unused, jobject jcpool)
{
	classinfo *c; /* classinfo of the class for which 'this' is the constant pool */

	TRACEJVMCALLS(("JVM_ConstantPoolGetSize(env=%p, unused=%p, jcpool=%p)", env, unused, jcpool));

	c = LLNI_classinfo_unwrap(jcpool);

	return c->cpcount;
}


/* JVM_ConstantPoolGetClassAt */

jclass JVM_ConstantPoolGetClassAt(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	constant_classref *ref;    /* classref to the class at constant pool index 'index' */
	classinfo         *c;      /* classinfo of the class for which 'this' is the constant pool */
	classinfo         *result; /* classinfo of the class at constant pool index 'index' */

	TRACEJVMCALLS(("JVM_ConstantPoolGetClassAt(env=%p, jcpool=%p, index=%d)", env, jcpool, index));

	c = LLNI_classinfo_unwrap(jcpool);

	ref = (constant_classref *) class_getconstant(c, index, CONSTANT_Class);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	result = resolve_classref_eager(ref);

	return (jclass) LLNI_classinfo_wrap(result);
}


/* JVM_ConstantPoolGetClassAtIfLoaded */

jclass JVM_ConstantPoolGetClassAtIfLoaded(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	constant_classref *ref;    /* classref to the class at constant pool index 'index' */
	classinfo         *c;      /* classinfo of the class for which 'this' is the constant pool */
	classinfo         *result; /* classinfo of the class at constant pool index 'index' */

	TRACEJVMCALLS(("JVM_ConstantPoolGetClassAtIfLoaded(env=%p, unused=%p, jcpool=%p, index=%d)", env, unused, jcpool, index));

	c = LLNI_classinfo_unwrap(jcpool);

	ref = (constant_classref *) class_getconstant(c, index, CONSTANT_Class);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}
	
	if (!resolve_classref(NULL, ref, resolveLazy, true, true, &result)) {
		return NULL;
	}

	if ((result == NULL) || !(result->state & CLASS_LOADED)) {
		return NULL;
	}
	
	return (jclass) LLNI_classinfo_wrap(result);
}


/* JVM_ConstantPoolGetMethodAt */

jobject JVM_ConstantPoolGetMethodAt(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	constant_FMIref *ref; /* reference to the method in constant pool at index 'index' */
	classinfo *cls;       /* classinfo of the class for which 'this' is the constant pool */
	
	TRACEJVMCALLS(("JVM_ConstantPoolGetMethodAt: jcpool=%p, index=%d", jcpool, index));
	
	cls = LLNI_classinfo_unwrap(jcpool);
	ref = (constant_FMIref*)class_getconstant(cls, index, CONSTANT_Methodref);
	
	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	// Create a new java.lang.reflect.Method Java object.
	/* XXX: is that right? or do I have to use resolve_method_*? */
	java_lang_reflect_Method jlrm(ref->p.method);

	return (jobject) jlrm.get_handle();
}


/* JVM_ConstantPoolGetMethodAtIfLoaded */

jobject JVM_ConstantPoolGetMethodAtIfLoaded(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	constant_FMIref *ref; /* reference to the method in constant pool at index 'index' */
	classinfo *c = NULL;  /* resolved declaring class of the method */
	classinfo *cls;       /* classinfo of the class for which 'this' is the constant pool */

	TRACEJVMCALLS(("JVM_ConstantPoolGetMethodAtIfLoaded: jcpool=%p, index=%d", jcpool, index));

	cls = LLNI_classinfo_unwrap(jcpool);
	ref = (constant_FMIref*)class_getconstant(cls, index, CONSTANT_Methodref);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	if (!resolve_classref(NULL, ref->p.classref, resolveLazy, true, true, &c)) {
		return NULL;
	}

	if (c == NULL || !(c->state & CLASS_LOADED)) {
		return NULL;
	}

	// Create a new java.lang.reflect.Method Java object.
	java_lang_reflect_Method jlrm(ref->p.method);

	return (jobject) jlrm.get_handle();
}


/* JVM_ConstantPoolGetFieldAt */

jobject JVM_ConstantPoolGetFieldAt(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	constant_FMIref *ref; /* reference to the field in constant pool at index 'index' */
	classinfo *cls;       /* classinfo of the class for which 'this' is the constant pool */
	
	TRACEJVMCALLS(("JVM_ConstantPoolGetFieldAt: jcpool=%p, index=%d", jcpool, index));

	cls = LLNI_classinfo_unwrap(jcpool);
	ref = (constant_FMIref*)class_getconstant(cls, index, CONSTANT_Fieldref);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	// Create a new java.lang.reflect.Field Java object.
	java_lang_reflect_Field jlrf(ref->p.field);

	return (jobject) jlrf.get_handle();
}


/* JVM_ConstantPoolGetFieldAtIfLoaded */

jobject JVM_ConstantPoolGetFieldAtIfLoaded(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	constant_FMIref *ref; /* reference to the field in constant pool at index 'index' */
	classinfo *c;         /* resolved declaring class for the field */
	classinfo *cls;       /* classinfo of the class for which 'this' is the constant pool */

	TRACEJVMCALLS(("JVM_ConstantPoolGetFieldAtIfLoaded: jcpool=%p, index=%d", jcpool, index));

	cls = LLNI_classinfo_unwrap(jcpool);
	ref = (constant_FMIref*)class_getconstant(cls, index, CONSTANT_Fieldref);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	if (!resolve_classref(NULL, ref->p.classref, resolveLazy, true, true, &c)) {
		return NULL;
	}

	if (c == NULL || !(c->state & CLASS_LOADED)) {
		return NULL;
	}

	// Create a new java.lang.reflect.Field Java object.
	java_lang_reflect_Field jlrf(ref->p.field);

	return (jobject) jlrf.get_handle();
}


/* JVM_ConstantPoolGetMemberRefInfoAt */

jobjectArray JVM_ConstantPoolGetMemberRefInfoAt(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	log_println("JVM_ConstantPoolGetMemberRefInfoAt: jcpool=%p, index=%d, IMPLEMENT ME!", jcpool, index);

	/* TODO: implement. (this is yet unused be OpenJDK but, so very low priority) */

	return NULL;
}


/* JVM_ConstantPoolGetIntAt */

jint JVM_ConstantPoolGetIntAt(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	TRACEJVMCALLS(("JVM_ConstantPoolGetIntAt: jcpool=%p, index=%d", jcpool, index));

	classinfo *cls = LLNI_classinfo_unwrap(jcpool);
	int32_t   *ref = (int32_t*) class_getconstant(cls, index, CONSTANT_Integer);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return 0;
	}

	return *ref;
}


/* JVM_ConstantPoolGetLongAt */

jlong JVM_ConstantPoolGetLongAt(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	TRACEJVMCALLS(("JVM_ConstantPoolGetLongAt: jcpool=%p, index=%d", jcpool, index));

	classinfo *cls = LLNI_classinfo_unwrap(jcpool);
	int64_t   *ref = (int64_t*) class_getconstant(cls, index, CONSTANT_Long);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return 0;
	}

	return *ref;
}


/* JVM_ConstantPoolGetFloatAt */

jfloat JVM_ConstantPoolGetFloatAt(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	TRACEJVMCALLS(("JVM_ConstantPoolGetFloatAt: jcpool=%p, index=%d", jcpool, index));

	classinfo *cls = LLNI_classinfo_unwrap(jcpool);
	float     *ref = (float*) class_getconstant(cls, index, CONSTANT_Float);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return 0;
	}

	return *ref;
}


/* JVM_ConstantPoolGetDoubleAt */

jdouble JVM_ConstantPoolGetDoubleAt(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	TRACEJVMCALLS(("JVM_ConstantPoolGetDoubleAt: jcpool=%p, index=%d", jcpool, index));

	classinfo *cls = LLNI_classinfo_unwrap(jcpool);
	double    *ref = (double*) class_getconstant(cls, index, CONSTANT_Double);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return 0;
	}

	return *ref;
}


/* JVM_ConstantPoolGetStringAt */

jstring JVM_ConstantPoolGetStringAt(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	Utf8String ref; /* utf object for the string in constant pool at index 'index' */
	classinfo *cls; /* classinfo of the class for which 'this' is the constant pool */

	TRACEJVMCALLS(("JVM_ConstantPoolGetStringAt: jcpool=%p, index=%d", jcpool, index));
	
	cls = LLNI_classinfo_unwrap(jcpool);
	ref = (utf*)class_getconstant(cls, index, CONSTANT_String);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	/* XXX: I hope JavaString::literal is the right Function. */
	return (jstring)JavaString::literal(ref);
}


/* JVM_ConstantPoolGetUTF8At */

jstring JVM_ConstantPoolGetUTF8At(JNIEnv *env, jobject unused, jobject jcpool, jint index)
{
	Utf8String ref; /* utf object for the utf8 data in constant pool at index 'index' */
	classinfo *cls; /* classinfo of the class for which 'this' is the constant pool */

	TRACEJVMCALLS(("JVM_ConstantPoolGetUTF8At: jcpool=%p, index=%d", jcpool, index));

	cls = LLNI_classinfo_unwrap(jcpool);
	ref = (utf*)class_getconstant(cls, index, CONSTANT_Utf8);

	if (ref == NULL) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	/* XXX: I hope JavaString::literal is the right Function. */
	return (jstring)JavaString::literal(ref);
}


/* JVM_DesiredAssertionStatus */

jboolean JVM_DesiredAssertionStatus(JNIEnv *env, jclass unused, jclass cls)
{
#if defined(ENABLE_ASSERTION)
	classinfo         *c;
	jboolean           status;

	TRACEJVMCALLS(("JVM_DesiredAssertionStatus(env=%p, unused=%p, cls=%p)", env, unused, cls));

	c = LLNI_classinfo_unwrap(cls);

	if (c->classloader == NULL) {
		status = (jboolean)assertion_system_enabled;
	}
	else {
		status = (jboolean)assertion_user_enabled;
	}

	if (list_assertion_names != NULL) {
		for (List<assertion_name_t*>::iterator it = list_assertion_names->begin();
			 it != list_assertion_names->end(); it++) {
			assertion_name_t* item = *it;

			Utf8String name = Utf8String::from_utf8(item->name);
			if (name == c->packagename) {
				status = (jboolean)item->enabled;
			}
			else if (name == c->name) {
				status = (jboolean)item->enabled;
			}
		}
	}

	return status;
#else
	return (jboolean)false;
#endif
}


/* JVM_AssertionStatusDirectives */

jobject JVM_AssertionStatusDirectives(JNIEnv *env, jclass unused)
{
#if defined(ENABLE_ASSERTION)
	java_handle_t* js;
	s4             i, j;
#endif

	TRACEJVMCALLS(("JVM_AssertionStatusDirectives(env=%p, unused=%p)", env, unused));

#if defined(ENABLE_ASSERTION)
	ObjectArray classes(assertion_class_count, class_java_lang_Object);
#else
	ObjectArray classes(0, class_java_lang_Object);
#endif
	if (classes.is_null())
		return NULL;

#if defined(ENABLE_ASSERTION)
	ObjectArray packages(assertion_package_count, class_java_lang_Object);
#else
	ObjectArray packages(0, class_java_lang_Object);
#endif
	if (packages.is_null())
		return NULL;
	
#if defined(ENABLE_ASSERTION)
	BooleanArray classEnabled(assertion_class_count);
#else
	BooleanArray classEnabled(0);
#endif
	if (classEnabled.is_null())
		return NULL;

#if defined(ENABLE_ASSERTION)
	BooleanArray packageEnabled(assertion_package_count);
#else
	BooleanArray packageEnabled(0);
#endif
	if (packageEnabled.is_null())
		return NULL;

#if defined(ENABLE_ASSERTION)
	/* initialize arrays */

	if (list_assertion_names != NULL) {
		i = 0;
		j = 0;
		
		for (List<assertion_name_t*>::iterator it = list_assertion_names->begin(); it != list_assertion_names->end(); it++) {
			assertion_name_t* item = *it;

			js = JavaString::from_utf8(item->name);
			if (js == NULL) {
				return NULL;
			}

			if (item->package == false) {
				classes.set_element(i, js);
				classEnabled.set_element(i, (jboolean) item->enabled);
				i += 1;
			}
			else {
				packages.set_element(j, js);
				packageEnabled.set_element(j, (jboolean) item->enabled);
				j += 1;
			}
		}
	}
#endif

	/* set instance fields */

	java_lang_AssertionStatusDirectives jlasd(
			classes.get_handle(),
			classEnabled.get_handle(),
			packages.get_handle(),
			packageEnabled.get_handle());

	return (jobject) jlasd.get_handle();
}


/* JVM_GetClassNameUTF */

const char *JVM_GetClassNameUTF(JNIEnv *env, jclass cls)
{
	log_println("JVM_GetClassNameUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetClassCPTypes */

void JVM_GetClassCPTypes(JNIEnv *env, jclass cls, unsigned char *types)
{
	log_println("JVM_GetClassCPTypes: IMPLEMENT ME!");
}


/* JVM_GetClassCPEntriesCount */

jint JVM_GetClassCPEntriesCount(JNIEnv *env, jclass cls)
{
	log_println("JVM_GetClassCPEntriesCount: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetClassFieldsCount */

jint JVM_GetClassFieldsCount(JNIEnv *env, jclass cls)
{
	log_println("JVM_GetClassFieldsCount: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetClassMethodsCount */

jint JVM_GetClassMethodsCount(JNIEnv *env, jclass cls)
{
	log_println("JVM_GetClassMethodsCount: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetMethodIxExceptionIndexes */

void JVM_GetMethodIxExceptionIndexes(JNIEnv *env, jclass cls, jint method_index, unsigned short *exceptions)
{
	log_println("JVM_GetMethodIxExceptionIndexes: IMPLEMENT ME!");
}


/* JVM_GetMethodIxExceptionsCount */

jint JVM_GetMethodIxExceptionsCount(JNIEnv *env, jclass cls, jint method_index)
{
	log_println("JVM_GetMethodIxExceptionsCount: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetMethodIxByteCode */

void JVM_GetMethodIxByteCode(JNIEnv *env, jclass cls, jint method_index, unsigned char *code)
{
	log_println("JVM_GetMethodIxByteCode: IMPLEMENT ME!");
}


/* JVM_GetMethodIxByteCodeLength */

jint JVM_GetMethodIxByteCodeLength(JNIEnv *env, jclass cls, jint method_index)
{
	log_println("JVM_GetMethodIxByteCodeLength: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetMethodIxExceptionTableEntry */

void JVM_GetMethodIxExceptionTableEntry(JNIEnv *env, jclass cls, jint method_index, jint entry_index, JVM_ExceptionTableEntryType *entry)
{
	log_println("JVM_GetMethodIxExceptionTableEntry: IMPLEMENT ME!");
}


/* JVM_GetMethodIxExceptionTableLength */

jint JVM_GetMethodIxExceptionTableLength(JNIEnv *env, jclass cls, int method_index)
{
	log_println("JVM_GetMethodIxExceptionTableLength: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetMethodIxModifiers */

jint JVM_GetMethodIxModifiers(JNIEnv *env, jclass cls, int method_index)
{
	log_println("JVM_GetMethodIxModifiers: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetFieldIxModifiers */

jint JVM_GetFieldIxModifiers(JNIEnv *env, jclass cls, int field_index)
{
	log_println("JVM_GetFieldIxModifiers: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetMethodIxLocalsCount */

jint JVM_GetMethodIxLocalsCount(JNIEnv *env, jclass cls, int method_index)
{
	log_println("JVM_GetMethodIxLocalsCount: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetMethodIxArgsSize */

jint JVM_GetMethodIxArgsSize(JNIEnv *env, jclass cls, int method_index)
{
	log_println("JVM_GetMethodIxArgsSize: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetMethodIxMaxStack */

jint JVM_GetMethodIxMaxStack(JNIEnv *env, jclass cls, int method_index)
{
	log_println("JVM_GetMethodIxMaxStack: IMPLEMENT ME!");

	return 0;
}


/* JVM_IsConstructorIx */

jboolean JVM_IsConstructorIx(JNIEnv *env, jclass cls, int method_index)
{
	log_println("JVM_IsConstructorIx: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetMethodIxNameUTF */

const char *JVM_GetMethodIxNameUTF(JNIEnv *env, jclass cls, jint method_index)
{
	log_println("JVM_GetMethodIxNameUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetMethodIxSignatureUTF */

const char *JVM_GetMethodIxSignatureUTF(JNIEnv *env, jclass cls, jint method_index)
{
	log_println("JVM_GetMethodIxSignatureUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetCPFieldNameUTF */

const char *JVM_GetCPFieldNameUTF(JNIEnv *env, jclass cls, jint cp_index)
{
	log_println("JVM_GetCPFieldNameUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetCPMethodNameUTF */

const char *JVM_GetCPMethodNameUTF(JNIEnv *env, jclass cls, jint cp_index)
{
	log_println("JVM_GetCPMethodNameUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetCPMethodSignatureUTF */

const char *JVM_GetCPMethodSignatureUTF(JNIEnv *env, jclass cls, jint cp_index)
{
	log_println("JVM_GetCPMethodSignatureUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetCPFieldSignatureUTF */

const char *JVM_GetCPFieldSignatureUTF(JNIEnv *env, jclass cls, jint cp_index)
{
	log_println("JVM_GetCPFieldSignatureUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetCPClassNameUTF */

const char *JVM_GetCPClassNameUTF(JNIEnv *env, jclass cls, jint cp_index)
{
	log_println("JVM_GetCPClassNameUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetCPFieldClassNameUTF */

const char *JVM_GetCPFieldClassNameUTF(JNIEnv *env, jclass cls, jint cp_index)
{
	log_println("JVM_GetCPFieldClassNameUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetCPMethodClassNameUTF */

const char *JVM_GetCPMethodClassNameUTF(JNIEnv *env, jclass cls, jint cp_index)
{
	log_println("JVM_GetCPMethodClassNameUTF: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetCPFieldModifiers */

jint JVM_GetCPFieldModifiers(JNIEnv *env, jclass cls, int cp_index, jclass called_cls)
{
	log_println("JVM_GetCPFieldModifiers: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetCPMethodModifiers */

jint JVM_GetCPMethodModifiers(JNIEnv *env, jclass cls, int cp_index, jclass called_cls)
{
	log_println("JVM_GetCPMethodModifiers: IMPLEMENT ME!");

	return 0;
}


/* JVM_ReleaseUTF */

void JVM_ReleaseUTF(const char *utf)
{
	log_println("JVM_ReleaseUTF: IMPLEMENT ME!");
}


/* JVM_IsSameClassPackage */

jboolean JVM_IsSameClassPackage(JNIEnv *env, jclass class1, jclass class2)
{
	log_println("JVM_IsSameClassPackage: IMPLEMENT ME!");

	return 0;
}


/* JVM_Open */
#ifdef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
/* Taken from openjdk/jdk/src/solaris/hpi/src/system_md.c of openjdk6 */

int sysFfileMode(int fd, int *mode)
{
    struct stat64 buf64;
    int ret = fstat64(fd, &buf64);
    (*mode) = buf64.st_mode;
    return ret;
}

int open64_w(const char *path, int oflag, int mode)
{
    int fd = open64(path, oflag, mode);
    if (fd == -1) return -1;

    /* If the open succeeded, the file might still be a directory */
    {
        int st_mode;
        if (sysFfileMode(fd, &st_mode) != -1) {
            if ((st_mode & S_IFMT) == S_IFDIR) {
                errno = EISDIR;
                close(fd);
                return -1;
            }
        } else {
            close(fd);
            return -1;
        }
    }

    /*
     * All file descriptors that are opened in the JVM and not
     * specifically destined for a subprocess should have the
     * close-on-exec flag set.  If we don't set it, then careless 3rd
     * party native code might fork and exec without closing all
     * appropriate file descriptors (e.g. as we do in closeDescriptors in
     * UNIXProcess.c), and this in turn might:
     *
     * - cause end-of-file to fail to be detected on some file
     *   descriptors, resulting in mysterious hangs, or
     *
     * - might cause an fopen in the subprocess to fail on a system
     *   suffering from bug 1085341.
     *
     * (Yes, the default setting of the close-on-exec flag is a Unix
     * design flaw)
     *
     * See:
     * 1085341: 32-bit stdio routines should support file descriptors >255
     * 4843136: (process) pipe file descriptor from Runtime.exec not being closed
     * 6339493: (process) Runtime.exec does not close all file descriptors on Solaris 9
     */
#ifdef FD_CLOEXEC
    {
        int flags = fcntl(fd, F_GETFD);
        if (flags != -1)
            fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
    }
#endif
    return fd;
}

static jint _JVM_SocketAvailable(jint fd, jint *pbytes);

#ifndef O_DELETE
#define O_DELETE 0x10000
#endif
/*
 * Open a file. Unlink the file immediately after open returns
 * if the specified oflag has the O_DELETE flag set.
 */
int sysOpen(const char *path, int oflag, int mode)
{
    int fd;
    int del = (oflag & O_DELETE);
    oflag = oflag & ~O_DELETE;
    fd = open64_w(path, oflag, mode);
    if (del != 0) {
        unlink(path);
    }
    return fd;
}
#endif

/* Taken from: hotspot/src/share/vm/prims/jvm.h */

/*
 * JVM I/O error codes
 */
#define JVM_EEXIST       -100

jint JVM_Open(const char* fname, jint flags, jint mode)
{
	int result;

	TRACEJVMCALLS(("JVM_Open(fname=%s, flags=%d, mode=%d)", fname, flags, mode));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	result = hpi.get_file().Open(fname, flags, mode);
#else
	result = sysOpen(fname, flags, mode);
#endif

	if (result >= 0) {
		return result;
	}
	else {
		switch (errno) {
		case EEXIST:
			return JVM_EEXIST;
		default:
			return -1;
		}
	}
}


/* JVM_Close */

jint JVM_Close(jint fd)
{
	TRACEJVMCALLS(("JVM_Close(fd=%d)", fd));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return hpi.get_file().Close(fd);
#else
	return close(fd);
#endif
}


/* JVM_Read */

jint JVM_Read(jint fd, char* buf, jint nbytes)
{
	TRACEJVMCALLS(("JVM_Read(fd=%d, buf=%p, nbytes=%d)", fd, buf, nbytes));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return (jint) hpi.get_file().Read(fd, buf, nbytes);
#else
	return read(fd, buf, nbytes);
#endif
}


/* JVM_Write */

jint JVM_Write(jint fd, char* buf, jint nbytes)
{
	TRACEJVMCALLS(("JVM_Write(fd=%d, buf=%s, nbytes=%d)", fd, buf, nbytes));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return (jint) hpi.get_file().Write(fd, buf, nbytes);
#else
	return write(fd,buf,nbytes);
#endif
}


/* JVM_Available */

jint JVM_Available(jint fd, jlong* pbytes)
{
	TRACEJVMCALLS(("JVM_Available(fd=%d, pbytes=%p)", fd, pbytes));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return hpi.get_file().Available(fd, pbytes);
#else
	off64_t cur, end;
	int mode;

	if (sysFfileMode(fd, &mode) >= 0)
		if (S_ISCHR(mode) || S_ISFIFO(mode) || S_ISSOCK(mode)) {
			jint bytes;
			if (_JVM_SocketAvailable(fd, &bytes)) {
				*pbytes = bytes;
				return 1;
			}
		}
	if ((cur = lseek64(fd, 0L, SEEK_CUR)) == -1)
		return 0;
	else if ((end = lseek64(fd, 0L, SEEK_END)) == -1)
		return 0;
	else if (lseek64(fd, cur, SEEK_SET) == -1)
		return 0;
	*pbytes = end - cur;
	return 1;
#endif
}


/* JVM_Lseek */

jlong JVM_Lseek(jint fd, jlong offset, jint whence)
{
	TRACEJVMCALLS(("JVM_Lseek(fd=%d, offset=%ld, whence=%d)", fd, offset, whence));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return hpi.get_file().Seek(fd, (off_t) offset, whence);
#else
	return lseek64(fd, (off_t) offset, whence);
#endif
}


/* JVM_SetLength */

jint JVM_SetLength(jint fd, jlong length)
{
	TRACEJVMCALLS(("JVM_SetLength(fd=%d, length=%ld)", length));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return hpi.get_file().SetLength(fd, length);
#else
	return ftruncate64(fd, length);
#endif
}


/* JVM_Sync */

jint JVM_Sync(jint fd)
{
	TRACEJVMCALLS(("JVM_Sync(fd=%d)", fd));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return hpi.get_file().Sync(fd);
#else
	return fsync(fd);
#endif
}


/* JVM_StartThread */

void JVM_StartThread(JNIEnv* env, jobject jthread)
{
	TRACEJVMCALLS(("JVM_StartThread(env=%p, jthread=%p)", env, jthread));

	threads_thread_start((java_handle_t *) jthread);
}


/* JVM_StopThread */

void JVM_StopThread(JNIEnv* env, jobject jthread, jobject throwable)
{
	log_println("JVM_StopThread: Deprecated.  Not implemented.");
}


/* JVM_IsThreadAlive */

jboolean JVM_IsThreadAlive(JNIEnv* env, jobject jthread)
{
	java_handle_t *h;
	threadobject  *t;
	bool           result;

	TRACEJVMCALLS(("JVM_IsThreadAlive(env=%p, jthread=%p)", env, jthread));

	h = (java_handle_t *) jthread;
	t = thread_get_thread(h);

	/* The threadobject is null when a thread is created in Java. */

	if (t == NULL)
		return 0;

	result = threads_thread_is_alive(t);

	return result;
}


/* JVM_SuspendThread */

void JVM_SuspendThread(JNIEnv* env, jobject jthread)
{
	java_handle_t *h;
	threadobject  *t;

	TRACEJVMCALLS(("JVM_SuspendThread(env=%p, jthread=%p)", env, jthread));

	if (opt_PrintWarnings)
		log_println("JVM_SuspendThread: Deprecated, do not use!");

	h = (java_handle_t *) jthread;
	t = thread_get_thread(h);

	/* The threadobject is null when a thread is created in Java. */

	if (t == NULL)
		return;

	threads_suspend_thread(t, SUSPEND_REASON_JAVA);
}


/* JVM_ResumeThread */

void JVM_ResumeThread(JNIEnv* env, jobject jthread)
{
	java_handle_t *h;
	threadobject  *t;

	TRACEJVMCALLS(("JVM_ResumeThread(env=%p, jthread=%p)", env, jthread));

	if (opt_PrintWarnings)
		log_println("JVM_ResumeThread: Deprecated, do not use!");

	h = (java_handle_t *) jthread;
	t = thread_get_thread(h);

	/* The threadobject is null when a thread is created in Java. */

	if (t == NULL)
		return;

	threads_resume_thread(t, SUSPEND_REASON_JAVA);
}


/* JVM_SetThreadPriority */

void JVM_SetThreadPriority(JNIEnv* env, jobject jthread, jint prio)
{
	java_handle_t *h;
	threadobject  *t;

	TRACEJVMCALLS(("JVM_SetThreadPriority(env=%p, jthread=%p, prio=%d)", env, jthread, prio));

	h = (java_handle_t *) jthread;
	t = thread_get_thread(h);

	/* The threadobject is null when a thread is created in Java. The
	   priority is set later during startup. */

	if (t == NULL)
		return;

	threads_set_thread_priority(t, prio);
}


/* JVM_SetNativeThreadName */

void JVM_SetNativeThreadName(JNIEnv* env, jclass cls, jobject name)
{
	TRACEJVMCALLS(("JVM_SetNativeThreadName(env=%p, cls=%p, name=%p)", env, cls, name));
}


/* JVM_Yield */

void JVM_Yield(JNIEnv *env, jclass threadClass)
{
	TRACEJVMCALLS(("JVM_Yield(env=%p, threadClass=%p)", env, threadClass));

	threads_yield();
}


/* JVM_Sleep */

void JVM_Sleep(JNIEnv* env, jclass threadClass, jlong millis)
{
	TRACEJVMCALLS(("JVM_Sleep(env=%p, threadClass=%p, millis=%ld)", env, threadClass, millis));

	threads_sleep(millis, 0);
}


/* JVM_CurrentThread */

jobject JVM_CurrentThread(JNIEnv* env, jclass threadClass)
{
	java_object_t *o;

	TRACEJVMCALLSVERBOSE(("JVM_CurrentThread(env=%p, threadClass=%p)", env, threadClass));

	o = thread_get_current_object();

	return (jobject) o;
}


/* JVM_CountStackFrames */

jint JVM_CountStackFrames(JNIEnv* env, jobject jthread)
{
	log_println("JVM_CountStackFrames: Deprecated.  Not implemented.");

	return 0;
}


/* JVM_Interrupt */

void JVM_Interrupt(JNIEnv* env, jobject jthread)
{
	java_handle_t *h;
	threadobject  *t;

	TRACEJVMCALLS(("JVM_Interrupt(env=%p, jthread=%p)", env, jthread));

	h = (java_handle_t *) jthread;
	t = thread_get_thread(h);

	/* The threadobject is null when a thread is created in Java. */

	if (t == NULL)
		return;

	threads_thread_interrupt(t);
}


/* JVM_IsInterrupted */

jboolean JVM_IsInterrupted(JNIEnv* env, jobject jthread, jboolean clear_interrupted)
{
	java_handle_t *h;
	threadobject  *t;
	jboolean       interrupted;

	TRACEJVMCALLS(("JVM_IsInterrupted(env=%p, jthread=%p, clear_interrupted=%d)", env, jthread, clear_interrupted));

	h = (java_handle_t *) jthread;
	t = thread_get_thread(h);

	/* The threadobject is null when a thread is created in Java. */

	if (t == NULL)
		return JNI_FALSE;

	interrupted = thread_is_interrupted(t);

	if (interrupted && clear_interrupted)
		thread_set_interrupted(t, false);

	return interrupted;
}


/* JVM_HoldsLock */

jboolean JVM_HoldsLock(JNIEnv* env, jclass threadClass, jobject obj)
{
	java_handle_t *h;
	bool           result;

	TRACEJVMCALLS(("JVM_HoldsLock(env=%p, threadClass=%p, obj=%p)", env, threadClass, obj));

	h = (java_handle_t *) obj;

	if (h == NULL) {
		exceptions_throw_nullpointerexception();
		return JNI_FALSE;
	}

	result = lock_is_held_by_current_thread(h);

	return result;
}


/* JVM_DumpAllStacks */

void JVM_DumpAllStacks(JNIEnv* env, jclass unused)
{
	log_println("JVM_DumpAllStacks: IMPLEMENT ME!");
}


/* JVM_CurrentLoadedClass */

jclass JVM_CurrentLoadedClass(JNIEnv *env)
{
	log_println("JVM_CurrentLoadedClass: IMPLEMENT ME!");

	return NULL;
}


/* JVM_CurrentClassLoader */

jobject JVM_CurrentClassLoader(JNIEnv *env)
{
	TRACEJVMCALLS(("JVM_CurrentClassLoader(env=%p)", env));
	PRINTJVMWARNINGS(("JVM_CurrentClassLoader is deprecated, do not use it."));

	return stacktrace_first_nonsystem_classloader();
}


/* JVM_GetClassContext */

jobjectArray JVM_GetClassContext(JNIEnv *env)
{
	TRACEJVMCALLS(("JVM_GetClassContext(env=%p)", env));

	return stacktrace_getClassContext();
}


/* JVM_ClassDepth */

jint JVM_ClassDepth(JNIEnv *env, jstring name)
{
	log_println("JVM_ClassDepth: IMPLEMENT ME!");

	return 0;
}


/* JVM_ClassLoaderDepth */

jint JVM_ClassLoaderDepth(JNIEnv *env)
{
	log_println("JVM_ClassLoaderDepth: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetSystemPackage */

jstring JVM_GetSystemPackage(JNIEnv *env, jstring name)
{
	TRACEJVMCALLS(("JVM_GetSystemPackage(env=%p, name=%p)", env, name));

	Utf8String u_ = JavaString((java_handle_t*) name).to_utf8();
	// strip off trailing slash
	Utf8String u = Utf8String::from_utf8_slash_to_dot(u_.begin(), u_.size() - 1);
	Utf8String result = Package::find(u);

	return (jstring) (result ? JavaString::from_utf8(result) : NULL);
}


/* JVM_GetSystemPackages */

jobjectArray JVM_GetSystemPackages(JNIEnv *env)
{
	TRACEJVMCALLS(("JVM_GetSystemPackages(env=%p)", env));

	AnyClassLocker<Package> lock;
	const std::set<Utf8String> &packages(Package::packages());
	int num = packages.size();

	ObjectArray result(num, class_java_lang_String);
	std::set<Utf8String>::const_iterator it = packages.begin();
	for (int i=0; it != packages.end(); ++it, i++) {
		// Make up the format expected by OpenJDK -- slash-separated
		// with extra trailing slash
		std::vector<char> temp;
		temp.reserve(it->size() + 1);
		const char *end = it->end();
		for (const char *c = it->begin(); c != end; ++c)
			temp.push_back(*c == '.' ? '/' : *c);
		temp.push_back('/');

		JavaString package_name = JavaString::from_utf8(&temp[0], temp.size());
		java_handle_t *hs = package_name;
		if (!hs)
			return NULL;
		result.set_element(i, hs);
	}
	return result.get_handle();
}


/* JVM_AllocateNewObject */

jobject JVM_AllocateNewObject(JNIEnv *env, jobject receiver, jclass currClass, jclass initClass)
{
	log_println("JVM_AllocateNewObject: IMPLEMENT ME!");

	return NULL;
}


/* JVM_AllocateNewArray */

jobject JVM_AllocateNewArray(JNIEnv *env, jobject obj, jclass currClass, jint length)
{
	log_println("JVM_AllocateNewArray: IMPLEMENT ME!");

	return NULL;
}


/* JVM_LatestUserDefinedLoader */

jobject JVM_LatestUserDefinedLoader(JNIEnv *env)
{
	TRACEJVMCALLS(("JVM_LatestUserDefinedLoader(env=%p)", env));

	return stacktrace_first_nonnull_classloader();
}


/* JVM_LoadClass0 */

jclass JVM_LoadClass0(JNIEnv *env, jobject receiver, jclass currClass, jstring currClassName)
{
	log_println("JVM_LoadClass0: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetArrayLength */

jint JVM_GetArrayLength(JNIEnv *env, jobject arr)
{
	TRACEJVMCALLS(("JVM_GetArrayLength(arr=%p)", arr));

	if (arr == NULL) {
		exceptions_throw_nullpointerexception();
		return -1;
	}

	Array a(arr);

	// Check for exception in constructor.
	if (a.is_null()) {
		return -1;
	}

	return a.get_length();
}


/* JVM_GetArrayElement */

jobject JVM_GetArrayElement(JNIEnv *env, jobject arr, jint index)
{
	TRACEJVMCALLS(("JVM_GetArrayElement(env=%p, arr=%p, index=%d)", env, arr, index));

	Array a(arr);

/* 	if (!class_is_array(a->objheader.vftbl->class)) { */
/* 		exceptions_throw_illegalargumentexception(); */
/* 		return NULL; */
/* 	} */

	return a.get_boxed_element(index);
}


/* JVM_GetPrimitiveArrayElement */

jvalue JVM_GetPrimitiveArrayElement(JNIEnv *env, jobject arr, jint index, jint wCode)
{
	jvalue jv;

	log_println("JVM_GetPrimitiveArrayElement: IMPLEMENT ME!");

	jv.l = NULL;

	return jv;
}


/* JVM_SetArrayElement */

void JVM_SetArrayElement(JNIEnv *env, jobject arr, jint index, jobject val)
{
	TRACEJVMCALLS(("JVM_SetArrayElement(env=%p, arr=%p, index=%d, val=%p)", env, arr, index, val));

	Array a(arr);

	a.set_boxed_element(index, val);
}


/* JVM_SetPrimitiveArrayElement */

void JVM_SetPrimitiveArrayElement(JNIEnv *env, jobject arr, jint index, jvalue v, unsigned char vCode)
{
	log_println("JVM_SetPrimitiveArrayElement: IMPLEMENT ME!");
}


/* JVM_NewArray */

jobject JVM_NewArray(JNIEnv *env, jclass eltClass, jint length)
{
	TRACEJVMCALLS(("JVM_NewArray(env=%p, eltClass=%p, length=%d)", env, eltClass, length));

	if (eltClass == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	/* NegativeArraySizeException is checked by array constructor. */

	classinfo* c = LLNI_classinfo_unwrap(eltClass);

	/* Create primitive or object array. */

	if (class_is_primitive(c)) {
		classinfo* pc = Primitive::get_arrayclass_by_name(c->name);

		/* void arrays are not allowed. */

		if (pc == NULL) {
			exceptions_throw_illegalargumentexception();
			return NULL;
		}

		Array a(length, pc);

		return (jobject) a.get_handle();
	}
	else {
		ObjectArray oa(length, c);

		return (jobject) oa.get_handle();
	}
}


/* JVM_NewMultiArray */

jobject JVM_NewMultiArray(JNIEnv *env, jclass eltClass, jintArray dim)
{
	classinfo                 *c;
	int32_t                    length;
	long                      *dims;
	int32_t                    value;
	int32_t                    i;
	classinfo                 *ac;
	java_handle_objectarray_t *a;

	TRACEJVMCALLS(("JVM_NewMultiArray(env=%p, eltClass=%p, dim=%p)", env, eltClass, dim));

	if (eltClass == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	/* NegativeArraySizeException is checked in builtin_newarray. */

	c = LLNI_classinfo_unwrap(eltClass);

	IntArray ia(dim);

	/* We check here for exceptions thrown in array_length_get,
	   otherwise these exceptions get overwritten by the following
	   IllegalArgumentException. */

	if (ia.is_null())
		return NULL;

	length = ia.get_length();

	if ((length <= 0) || (length > /* MAX_DIM */ 255)) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	/* XXX This is just a quick hack to get it working. */

	dims = MNEW(long, length);

	for (i = 0; i < length; i++) {
		value = ia.get_element(i);
		dims[i] = (long) value;
	}

	/* Create an array-class if necessary. */

	if (class_is_primitive(c)) {
		ac = Primitive::get_arrayclass_by_name(c->name);

		// Arrays of void are not allowed.
		if (ac == NULL) {
			exceptions_throw_illegalargumentexception();
			return NULL;
		}

		if (length > 1)
			ac = class_multiarray_of((length - 1), ac, true);
	}
	else
		ac = class_multiarray_of(length, c, true);

	if (ac == NULL)
		return NULL;

	/* Allocate a new array on the heap. */

	a = builtin_multianewarray(length, (java_handle_t *) ac, dims);

	return (jobject) a;
}


/* JVM_InitializeSocketLibrary */

jint JVM_InitializeSocketLibrary()
{
	TRACEJVMCALLS(("JVM_InitializeSocketLibrary()"));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	return hpi.initialize_socket_library();
#else
	return 0;
#endif
}


/* JVM_Socket */

jint JVM_Socket(jint domain, jint type, jint protocol)
{
	TRACEJVMCALLS(("JVM_Socket(domain=%d, type=%d, protocol=%d)", domain, type, protocol));

	return os::socket(domain, type, protocol);
}


/* JVM_SocketClose */

jint JVM_SocketClose(jint fd)
{
	TRACEJVMCALLS(("JVM_SocketClose(fd=%d)", fd));

	return os::close(fd);
}


/* JVM_SocketShutdown */

jint JVM_SocketShutdown(jint fd, jint howto)
{
	TRACEJVMCALLS(("JVM_SocketShutdown(fd=%d, howto=%d)", fd, howto));

	return os::shutdown(fd, howto);
}


/* JVM_Recv */

jint JVM_Recv(jint fd, char *buf, jint nBytes, jint flags)
{
	log_println("JVM_Recv: IMPLEMENT ME!");

	return 0;
}


/* JVM_Send */

jint JVM_Send(jint fd, char *buf, jint nBytes, jint flags)
{
	TRACEJVMCALLSENTER(("JVM_Send(fd=%d, buf=%p, nBytes=%d, flags=%d", fd, buf, nBytes, flags));

	int result = os::send(fd, buf, nBytes, flags);

	TRACEJVMCALLSEXIT(("->%d", result));

	return result;
}


/* JVM_Timeout */

jint JVM_Timeout(int fd, long timeout)
{
	log_println("JVM_Timeout: IMPLEMENT ME!");

	return 0;
}


/* JVM_Listen */

jint JVM_Listen(jint fd, jint count)
{
	TRACEJVMCALLS(("JVM_Listen(fd=%d, count=%d)", fd, count));

	return os::listen(fd, count);
}


/* JVM_Connect */

jint JVM_Connect(jint fd, struct sockaddr *him, jint len)
{
	TRACEJVMCALLS(("JVM_Connect(fd=%d, him=%p, len=%d)", fd, him, len));

	return os::connect(fd, him, len);
}


/* JVM_Bind */

jint JVM_Bind(jint fd, struct sockaddr *him, jint len)
{
	log_println("JVM_Bind: IMPLEMENT ME!");

	return 0;
}


/* JVM_Accept */

jint JVM_Accept(jint fd, struct sockaddr *him, jint *len)
{
	TRACEJVMCALLS(("JVM_Accept(fd=%d, him=%p, len=%p)", fd, him, len));

	return os::accept(fd, him, (socklen_t *) len);
}


/* JVM_RecvFrom */

jint JVM_RecvFrom(jint fd, char *buf, int nBytes, int flags, struct sockaddr *from, int *fromlen)
{
	log_println("JVM_RecvFrom: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetSockName */

jint JVM_GetSockName(jint fd, struct sockaddr *him, int *len)
{
	TRACEJVMCALLS(("JVM_GetSockName(fd=%d, him=%p, len=%p)", fd, him, len));

	return os::getsockname(fd, him, (socklen_t *) len);
}


/* JVM_SendTo */

jint JVM_SendTo(jint fd, char *buf, int len, int flags, struct sockaddr *to, int tolen)
{
	log_println("JVM_SendTo: IMPLEMENT ME!");

	return 0;
}


/* JVM_SocketAvailable */

jint _JVM_SocketAvailable(jint fd, jint *pbytes)
{
#if defined(FIONREAD)
	int bytes;
	int result;

	*pbytes = 0;

	result = ioctl(fd, FIONREAD, &bytes);

	if (result < 0)
		return 0;

	*pbytes = bytes;

	return 1;
#else
# error FIONREAD not defined
#endif
}

jint JVM_SocketAvailable(jint fd, jint *pbytes)
{
	TRACEJVMCALLS(("JVM_SocketAvailable(fd=%d, pbytes=%p)", fd, pbytes));
	return _JVM_SocketAvailable(fd, pbytes);
}


/* JVM_GetSockOpt */

jint JVM_GetSockOpt(jint fd, int level, int optname, char *optval, int *optlen)
{
	TRACEJVMCALLS(("JVM_GetSockOpt(fd=%d, level=%d, optname=%d, optval=%s, optlen=%p)", fd, level, optname, optval, optlen));

	return os::getsockopt(fd, level, optname, optval, (socklen_t *) optlen);
}


/* JVM_SetSockOpt */

jint JVM_SetSockOpt(jint fd, int level, int optname, const char *optval, int optlen)
{
	TRACEJVMCALLS(("JVM_SetSockOpt(fd=%d, level=%d, optname=%d, optval=%s, optlen=%d)", fd, level, optname, optval, optlen));

	return os::setsockopt(fd, level, optname, optval, optlen);
}


/* JVM_GetHostName */

int JVM_GetHostName(char *name, int namelen)
{
	int result;

	TRACEJVMCALLSENTER(("JVM_GetHostName(name=%s, namelen=%d)", name, namelen));

	result = os::gethostname(name, namelen);

	TRACEJVMCALLSEXIT(("->%d (name=%s)", result, name));

	return result;
}


/* JVM_GetHostByAddr */

struct hostent *JVM_GetHostByAddr(const char* name, int len, int type)
{
	log_println("JVM_GetHostByAddr: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetHostByName */

struct hostent *JVM_GetHostByName(char* name)
{
	log_println("JVM_GetHostByName: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetProtoByName */

struct protoent *JVM_GetProtoByName(char* name)
{
	log_println("JVM_GetProtoByName: IMPLEMENT ME!");

	return NULL;
}


/* JVM_LoadLibrary */

void* JVM_LoadLibrary(const char* name)
{
	TRACEJVMCALLSENTER(("JVM_LoadLibrary(name=%s)", name));

	Utf8String u = Utf8String::from_utf8(name);

	NativeLibrary nl(u);
	void* handle = nl.open();
	
	TRACEJVMCALLSEXIT(("->%p", handle));

	return handle;
}


/* JVM_UnloadLibrary */

void JVM_UnloadLibrary(void* handle)
{
	TRACEJVMCALLS(("JVM_UnloadLibrary(handle=%p)", handle));

	NativeLibrary nl(handle);
	nl.close();
}


/* JVM_FindLibraryEntry */

void *JVM_FindLibraryEntry(void* handle, const char* name)
{
	void* symbol;

	TRACEJVMCALLSENTER(("JVM_FindLibraryEntry(handle=%p, name=%s)", handle, name));

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI& hpi = VM::get_current()->get_hpi();
	symbol = hpi.get_library().FindLibraryEntry(handle, name);
#else
	//because we use glibc we need a mutex here
	VM::get_current()->get_jniMutex().lock();
	symbol = dlsym(handle, name);
	VM::get_current()->get_jniMutex().unlock();
#endif

	TRACEJVMCALLSEXIT(("->%p", symbol));

	return symbol;
}


/* JVM_IsNaN */

jboolean JVM_IsNaN(jdouble d)
{
	bool result;

	TRACEJVMCALLSENTER(("JVM_IsNaN(d=%f)", d));

	result = isnan(d);

	TRACEJVMCALLSEXIT(("->%d", result));

	return result;
}


/* JVM_IsSupportedJNIVersion */

jboolean JVM_IsSupportedJNIVersion(jint version)
{
	TRACEJVMCALLS(("JVM_IsSupportedJNIVersion(version=%d)", version));

	return jni_version_check(version);
}


/* JVM_InternString */

jstring JVM_InternString(JNIEnv *env, jstring str)
{
	TRACEJVMCALLS(("JVM_InternString(env=%p, str=%p)", env, str));

	return (jstring) JavaString((java_handle_t*) str).intern();
}


/* JVM_RawMonitorCreate */

JNIEXPORT void* JNICALL JVM_RawMonitorCreate(void)
{
	TRACEJVMCALLS(("JVM_RawMonitorCreate()"));

	Mutex* m = new Mutex();

	return m;
}


/* JVM_RawMonitorDestroy */

JNIEXPORT void JNICALL JVM_RawMonitorDestroy(void* mon)
{
	TRACEJVMCALLS(("JVM_RawMonitorDestroy(mon=%p)", mon));

	delete ((Mutex*) mon);
}


/* JVM_RawMonitorEnter */

JNIEXPORT jint JNICALL JVM_RawMonitorEnter(void* mon)
{
	TRACEJVMCALLS(("JVM_RawMonitorEnter(mon=%p)", mon));

	((Mutex*) mon)->lock();

	return 0;
}


/* JVM_RawMonitorExit */

JNIEXPORT void JNICALL JVM_RawMonitorExit(void* mon)
{
	TRACEJVMCALLS(("JVM_RawMonitorExit(mon=%p)", mon));

	((Mutex*) mon)->unlock();
}


/* JVM_SetPrimitiveFieldValues */

void JVM_SetPrimitiveFieldValues(JNIEnv *env, jclass cb, jobject obj, jlongArray fieldIDs, jcharArray typecodes, jbyteArray data)
{
	log_println("JVM_SetPrimitiveFieldValues: IMPLEMENT ME!");
}


/* JVM_GetPrimitiveFieldValues */

void JVM_GetPrimitiveFieldValues(JNIEnv *env, jclass cb, jobject obj, jlongArray fieldIDs, jcharArray typecodes, jbyteArray data)
{
	log_println("JVM_GetPrimitiveFieldValues: IMPLEMENT ME!");
}


/* JVM_AccessVMBooleanFlag */

jboolean JVM_AccessVMBooleanFlag(const char* name, jboolean* value, jboolean is_get)
{
	log_println("JVM_AccessVMBooleanFlag: IMPLEMENT ME!");

	return 0;
}


/* JVM_AccessVMIntFlag */

jboolean JVM_AccessVMIntFlag(const char* name, jint* value, jboolean is_get)
{
	log_println("JVM_AccessVMIntFlag: IMPLEMENT ME!");

	return 0;
}


/* JVM_VMBreakPoint */

void JVM_VMBreakPoint(JNIEnv *env, jobject obj)
{
	log_println("JVM_VMBreakPoint: IMPLEMENT ME!");
}


/* JVM_GetClassFields */

jobjectArray JVM_GetClassFields(JNIEnv *env, jclass cls, jint which)
{
	log_println("JVM_GetClassFields: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetClassMethods */

jobjectArray JVM_GetClassMethods(JNIEnv *env, jclass cls, jint which)
{
	log_println("JVM_GetClassMethods: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetClassConstructors */

jobjectArray JVM_GetClassConstructors(JNIEnv *env, jclass cls, jint which)
{
	log_println("JVM_GetClassConstructors: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetClassField */

jobject JVM_GetClassField(JNIEnv *env, jclass cls, jstring name, jint which)
{
	log_println("JVM_GetClassField: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetClassMethod */

jobject JVM_GetClassMethod(JNIEnv *env, jclass cls, jstring name, jobjectArray types, jint which)
{
	log_println("JVM_GetClassMethod: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetClassConstructor */

jobject JVM_GetClassConstructor(JNIEnv *env, jclass cls, jobjectArray types, jint which)
{
	log_println("JVM_GetClassConstructor: IMPLEMENT ME!");

	return NULL;
}


/* JVM_NewInstance */

jobject JVM_NewInstance(JNIEnv *env, jclass cls)
{
	log_println("JVM_NewInstance: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetField */

jobject JVM_GetField(JNIEnv *env, jobject field, jobject obj)
{
	log_println("JVM_GetField: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetPrimitiveField */

jvalue JVM_GetPrimitiveField(JNIEnv *env, jobject field, jobject obj, unsigned char wCode)
{
	jvalue jv;

	log_println("JVM_GetPrimitiveField: IMPLEMENT ME!");

	jv.l = NULL;

	return jv;
}


/* JVM_SetField */

void JVM_SetField(JNIEnv *env, jobject field, jobject obj, jobject val)
{
	log_println("JVM_SetField: IMPLEMENT ME!");
}


/* JVM_SetPrimitiveField */

void JVM_SetPrimitiveField(JNIEnv *env, jobject field, jobject obj, jvalue v, unsigned char vCode)
{
	log_println("JVM_SetPrimitiveField: IMPLEMENT ME!");
}


/* JVM_InvokeMethod */

jobject JVM_InvokeMethod(JNIEnv *env, jobject method, jobject obj, jobjectArray args0)
{
	TRACEJVMCALLS(("JVM_InvokeMethod(env=%p, method=%p, obj=%p, args0=%p)", env, method, obj, args0));

	java_lang_reflect_Method jlrm(method);
	
	java_handle_t* result = jlrm.invoke((java_handle_t*) obj, (java_handle_objectarray_t*) args0);

	return (jobject) result;
}


/* JVM_NewInstanceFromConstructor */

jobject JVM_NewInstanceFromConstructor(JNIEnv *env, jobject con, jobjectArray args0)
{
	TRACEJVMCALLS(("JVM_NewInstanceFromConstructor(env=%p, c=%p, args0=%p)", env, con, args0));

	java_lang_reflect_Constructor jlrc(con);
	java_handle_t* o = jlrc.new_instance((java_handle_objectarray_t*) args0);

	return (jobject) o;
}


/* JVM_SupportsCX8 */

jboolean JVM_SupportsCX8()
{
	TRACEJVMCALLS(("JVM_SupportsCX8()"));

	/* IMPLEMENT ME */

	return 0;
}


/* JVM_CX8Field */

jboolean JVM_CX8Field(JNIEnv *env, jobject obj, jfieldID fid, jlong oldVal, jlong newVal)
{
	log_println("JVM_CX8Field: IMPLEMENT ME!");

	return 0;
}


/* JVM_GetAllThreads */

jobjectArray JVM_GetAllThreads(JNIEnv *env, jclass dummy)
{
	// Get a list of all active threads.
	List<threadobject*> active_threads;
	ThreadList::get()->get_active_java_threads(active_threads);

	// Allocate array to hold the java.lang.Thread objects.
	int32_t length = active_threads.size();
	ObjectArray oa(length, class_java_lang_Thread);

	if (oa.is_null())
		return NULL;

	// Iterate over all threads (which were active just a second ago).
	int32_t index = 0;
	for (List<threadobject*>::iterator it = active_threads.begin(); it != active_threads.end(); it++) {
		threadobject* t = *it;

		java_handle_t* h = LLNI_WRAP(t->object);
		assert(h != NULL);

		oa.set_element(index, h);

		index++;
	}

	return oa.get_handle();
}


/* JVM_DumpThreads */

jobjectArray JVM_DumpThreads(JNIEnv *env, jclass threadClass, jobjectArray threads)
{
	int32_t i;

	TRACEJVMCALLS(("JVM_DumpThreads((env=%p, threadClass=%p, threads=%p)", env, threadClass, threads));

	if (threads == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	ObjectArray oa(threads);

	// Get length of the threads array.
	int32_t length = oa.get_length();

	if (length <= 0) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	// Allocate array to hold stacktraces.
	classinfo* arrayclass = class_array_of(class_java_lang_StackTraceElement, true);
	ObjectArray oaresult(length, arrayclass);

	if (oaresult.is_null()) {
		return NULL;
	}

	// Iterate over all passed thread objects.
	for (i = 0; i < length; i++) {
		java_handle_t* thread = oa.get_element(i);

		// Get thread for the given thread object.
		threadobject* t = thread_get_thread(thread);

		// The threadobject is null when a thread is created in Java.
		if (t == NULL)
			continue;

		// Get stacktrace for given thread.
		stacktrace_t* st = stacktrace_get_of_thread(t);

		// Convert stacktrace into array of StackTraceElements.
		java_handle_objectarray_t* oaste = stacktrace_get_StackTraceElements(st);

		if (oaste == NULL)
			return NULL;

		oaresult.set_element(i, oaste);
	}

	return oaresult.get_handle();
}


/* JVM_GetManagement */

void *JVM_GetManagement(jint version)
{
	TRACEJVMCALLS(("JVM_GetManagement(version=%d)", version));

	return Management::get_jmm_interface(version);
}


/* JVM_InitAgentProperties */

jobject JVM_InitAgentProperties(JNIEnv *env, jobject properties)
{
	log_println("JVM_InitAgentProperties: IMPLEMENT ME!");

	return NULL;
}


/* JVM_GetEnclosingMethodInfo */

jobjectArray JVM_GetEnclosingMethodInfo(JNIEnv *env, jclass ofClass)
{
	TRACEJVMCALLS(("JVM_GetEnclosingMethodInfo(env=%p, ofClass=%p)", env, ofClass));

	classinfo* c = LLNI_classinfo_unwrap(ofClass);

	if ((c == NULL) || class_is_primitive(c))
		return NULL;

	methodinfo* m = class_get_enclosingmethod_raw(c);

	if (m == NULL)
		return NULL;

	ObjectArray oa(3, class_java_lang_Object);

	if (oa.is_null())
		return NULL;

	oa.set_element(0, (java_handle_t *) LLNI_classinfo_wrap(m->clazz));
	oa.set_element(1, JavaString::from_utf8(m->name));
	oa.set_element(2, JavaString::from_utf8(m->descriptor));

	return oa.get_handle();
}


/* JVM_GetThreadStateValues */

jintArray JVM_GetThreadStateValues(JNIEnv* env, jint javaThreadState)
{
	TRACEJVMCALLS(("JVM_GetThreadStateValues(env=%p, javaThreadState=%d)",
				  env, javaThreadState));

	/* If new thread states are added in future JDK and VM versions,
	   this should check if the JDK version is compatible with thread
	   states supported by the VM.  Return NULL if not compatible.
	
	   This function must map the VM java_lang_Thread::ThreadStatus
	   to the Java thread state that the JDK supports. */

	switch (javaThreadState) {
    case THREAD_STATE_NEW:
		{
			IntArray ia(1);

			if (ia.is_null())
				return NULL;

			ia.set_element(0, THREAD_STATE_NEW);
			return ia.get_handle();
		}

    case THREAD_STATE_RUNNABLE:
		{
			IntArray ia(1);

			if (ia.is_null())
				return NULL;

			ia.set_element(0, THREAD_STATE_RUNNABLE);
			return ia.get_handle();
		}

    case THREAD_STATE_BLOCKED:
		{
			IntArray ia(1);

			if (ia.is_null())
				return NULL;

			ia.set_element(0, THREAD_STATE_BLOCKED);
			return ia.get_handle();
		}

    case THREAD_STATE_WAITING:
		{
			IntArray ia(2);

			if (ia.is_null())
				return NULL;

			ia.set_element(0, THREAD_STATE_WAITING);
			ia.set_element(1, THREAD_STATE_PARKED);
			return ia.get_handle();
		}

    case THREAD_STATE_TIMED_WAITING:
		{
			IntArray ia(2);

			if (ia.is_null())
				return NULL;

			/* XXX Not sure about that one. */
/* 			ia.set_element(0, SLEEPING); */
			ia.set_element(0, THREAD_STATE_TIMED_WAITING);
			ia.set_element(1, THREAD_STATE_TIMED_PARKED);
			return ia.get_handle();
		}

    case THREAD_STATE_TERMINATED:
		{
			IntArray ia(1);

			if (ia.is_null())
				return NULL;

			ia.set_element(0, THREAD_STATE_TERMINATED);
			return ia.get_handle();
		}

    default:
		/* Unknown state - probably incompatible JDK version */
		return NULL;
	}
}


/* JVM_GetThreadStateNames */

jobjectArray JVM_GetThreadStateNames(JNIEnv* env, jint javaThreadState, jintArray values)
{
	java_object_t* s;

	TRACEJVMCALLS(("JVM_GetThreadStateNames(env=%p, javaThreadState=%d, values=%p)",
				  env, javaThreadState, values));

	IntArray ia(values);

	/* If new thread states are added in future JDK and VM versions,
	   this should check if the JDK version is compatible with thread
	   states supported by the VM.  Return NULL if not compatible.
	
	   This function must map the VM java_lang_Thread::ThreadStatus
	   to the Java thread state that the JDK supports. */

	if (values == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	switch (javaThreadState) {
    case THREAD_STATE_NEW:
		{
			assert(ia.get_length() == 1 && ia.get_element(0) == THREAD_STATE_NEW);

			ObjectArray oa(1, class_java_lang_String);

			if (oa.is_null())
				return NULL;

			s = JavaString::from_utf8("NEW");

			if (s == NULL)
				return NULL;

			oa.set_element(0, s);
			return oa.get_handle();
		}

    case THREAD_STATE_RUNNABLE:
		{
			ObjectArray oa(1, class_java_lang_String);

			if (oa.is_null())
				return NULL;

			s = JavaString::from_utf8("RUNNABLE");

			if (s == NULL)
				return NULL;

			oa.set_element(0, s);
			return oa.get_handle();
		}

    case THREAD_STATE_BLOCKED:
		{
			ObjectArray oa(1, class_java_lang_String);

			if (oa.is_null())
				return NULL;

			s = JavaString::from_utf8("BLOCKED");

			if (s == NULL)
				return NULL;

			oa.set_element(0, s);
			return oa.get_handle();
		}

    case THREAD_STATE_WAITING:
		{
			ObjectArray oa(2, class_java_lang_String);

			if (oa.is_null())
				return NULL;

			s = JavaString::from_utf8("WAITING.OBJECT_WAIT");

			if (s == NULL)
				return NULL;

			oa.set_element(0, s);

			s = JavaString::from_utf8("WAITING.PARKED");

			if (s == NULL)
				return NULL;

			oa.set_element(1, s);
			return oa.get_handle();
		}

    case THREAD_STATE_TIMED_WAITING:
		{
			ObjectArray oa(2, class_java_lang_String);

			if (oa.is_null())
				return NULL;

/* 			s = JavaString::from_utf8("TIMED_WAITING.SLEEPING"); */
			s = JavaString::from_utf8("TIMED_WAITING.OBJECT_WAIT");

			if (s == NULL)
				return NULL;

			oa.set_element(0, s);

			s = JavaString::from_utf8("TIMED_WAITING.PARKED");

			if (s == NULL)
				return NULL;

			oa.set_element(1, s);
			return oa.get_handle();
		}

    case THREAD_STATE_TERMINATED:
		{
			ObjectArray oa(1, class_java_lang_String);

			if (oa.is_null())
				return NULL;

			s = JavaString::from_utf8("TERMINATED");

			if (s == NULL)
				return NULL;

			oa.set_element(0, s);
			return oa.get_handle();
		}

	default:
		/* Unknown state - probably incompatible JDK version */
		return NULL;
	}
}


/* JVM_GetVersionInfo */

void JVM_GetVersionInfo(JNIEnv* env, jvm_version_info* info, size_t info_size)
{
	TRACEJVMCALLS(("JVM_GetVersionInfo(env=%p, info=%p, info_size=%zd)", env, info, info_size));

	memset(info, 0, info_size);

	info->jvm_version = ((VERSION_MAJOR & 0xff) << 24) | ((VERSION_MINOR & 0xff) << 16) | (VERSION_MICRO & 0xff);
	info->update_version = 0;
	info->special_update_version = 0;
	info->is_attach_supported = 0;
	info->is_kernel_jvm = 0;
}


/* OS: JVM_RegisterSignal */

void *JVM_RegisterSignal(jint sig, void *handler)
{
	functionptr newHandler;

	TRACEJVMCALLS(("JVM_RegisterSignal(sig=%d, handler=%p)", sig, handler));

	if (handler == (void *) 2)
		newHandler = (functionptr) signal_thread_handler;
	else
		newHandler = (functionptr) (uintptr_t) handler;

	switch (sig) {
    case SIGILL:
    case SIGFPE:
    case SIGUSR1:
    case SIGSEGV:
		/* These signals are already used by the VM. */
		return (void *) -1;

    case SIGQUIT:
		/* This signal is used by the VM to dump thread stacks unless
		   ReduceSignalUsage is set, in which case the user is allowed
		   to set his own _native_ handler for this signal; thus, in
		   either case, we do not allow JVM_RegisterSignal to change
		   the handler. */
		return (void *) -1;

	case SIGHUP:
	case SIGINT:
	case SIGTERM:
		break;
	}

	signal_register_signal(sig, newHandler, SA_RESTART | SA_SIGINFO);

	/* XXX Should return old handler. */

	return (void *) 2;
}


/* OS: JVM_RaiseSignal */

jboolean JVM_RaiseSignal(jint sig)
{
	log_println("JVM_RaiseSignal: IMPLEMENT ME! sig=%s", sig);

	return false;
}


/* OS: JVM_FindSignal */

jint JVM_FindSignal(const char *name)
{
	TRACEJVMCALLS(("JVM_FindSignal(name=%s)", name));

#if defined(__LINUX__)
	if (strcmp(name, "HUP") == 0)
		return SIGHUP;

	if (strcmp(name, "INT") == 0)
		return SIGINT;

	if (strcmp(name, "TERM") == 0)
		return SIGTERM;
#elif defined(__SOLARIS__)
	int signum;

	if (os::str2sig(name, &signum) == -1)
		return -1;

	return signum;
#else
# error Not implemented for this OS.
#endif

	return -1;
}

} // extern "C"


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
