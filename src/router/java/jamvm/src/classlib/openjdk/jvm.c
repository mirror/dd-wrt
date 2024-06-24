/*
 * Copyright (C) 2010, 2011, 2012, 2013, 2014
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "config.h"

#define BSD_COMP
#define _FILE_OFFSET_BITS 64
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <math.h>

#include "jam.h"
#include "jni.h"
#include "lock.h"
#include "hash.h"
#include "class.h"
#include "excep.h"
#include "symbol.h"
#include "reflect.h"
#include "openjdk.h"
#include "classlib.h"
#include "properties.h"
#include "annotations.h"
#include "trace.h"

#define JVM_INTERFACE_VERSION 4

/* We use the monotonic clock if it is available.  As the clock_id may be
   present but not actually supported, we check it on startup */
#if defined(HAVE_LIBRT) && defined(CLOCK_MONOTONIC)
static int have_monotonic_clock;
#endif

static Class *cloneable_class, *constant_pool_class;
static Class *exception_class, *runtime_excp_class;

static MethodBlock *priv_act_excp_init_mb;
static int constant_pool_oop_offset;

int initialiseJVMInterface() {
    Class *pae_class;

#if defined(HAVE_LIBRT) && defined(CLOCK_MONOTONIC)
    struct timespec ts;
    have_monotonic_clock = (clock_gettime(CLOCK_MONOTONIC, &ts) != -1);
#endif

    cloneable_class = findSystemClass0(SYMBOL(java_lang_Cloneable));
    constant_pool_class = findSystemClass0(SYMBOL(sun_reflect_ConstantPool));
    exception_class = findSystemClass0(SYMBOL(java_lang_Exception));
    runtime_excp_class = findSystemClass0(SYMBOL(java_lang_RuntimeException));
    pae_class = findSystemClass0(SYMBOL(java_security_PrivilegedActionException));

    registerStaticClassRef(&cloneable_class);
    registerStaticClassRef(&constant_pool_class);
    registerStaticClassRef(&exception_class);
    registerStaticClassRef(&runtime_excp_class);

    if(cloneable_class != NULL && constant_pool_class != NULL &&
           exception_class != NULL && runtime_excp_class != NULL &&
           pae_class != NULL) {
        FieldBlock *fb = findField(constant_pool_class,
                                   SYMBOL(constantPoolOop),
                                   SYMBOL(sig_java_lang_Object));

        priv_act_excp_init_mb = findMethod(pae_class, SYMBOL(object_init),
                                           SYMBOL(_java_lang_Exception__V));

        if(fb != NULL && priv_act_excp_init_mb != NULL) {
            constant_pool_oop_offset = fb->u.offset;
            return TRUE;
        }
    }

    return FALSE;
}

int jio_vsnprintf(char *str, size_t count, const char *fmt, va_list args) {
    if ((intptr_t) count <= 0)
        return -1;

    return vsnprintf(str, count, fmt, args);
}


int jio_snprintf(char *str, size_t count, const char *fmt, ...) {
    va_list ap;
    int     len;

    va_start(ap, fmt);
    len = jio_vsnprintf(str, count, fmt, ap);
    va_end(ap);

    return len;
}


int jio_fprintf(FILE* f, const char *fmt, ...) {
    UNIMPLEMENTED("jio_fprintf");

    return 0;
}


int jio_vfprintf(FILE* f, const char *fmt, va_list args) {
    UNIMPLEMENTED("jio_vfprintf");

    return 0;
}


int jio_printf(const char *fmt, ...) {
    UNIMPLEMENTED("jio_printf");

    return 0;
}


/* JVM_GetInterfaceVersion */

jint JVM_GetInterfaceVersion() {
    return JVM_INTERFACE_VERSION;
}

void JVM_BeforeHalt() {

}

/* JVM_CurrentTimeMillis */

jlong JVM_CurrentTimeMillis(JNIEnv *env, jclass ignored) {
    struct timeval tv;

    TRACE("JVM_CurrentTimeMillis(env=%p, ignored=%p)", env, ignored);

    gettimeofday(&tv, NULL);
    return (jlong) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


/* JVM_NanoTime */

jlong JVM_NanoTime(JNIEnv *env, jclass ignored) {
    TRACE("JVM_NanoTime(env=%p, ignored=%p)", env, ignored);

#if defined(HAVE_LIBRT) && defined(CLOCK_MONOTONIC)
    if(have_monotonic_clock) {
        struct timespec ts;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (jlong) ts.tv_sec * 1000000000 + ts.tv_nsec;
    }
#endif
    {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        return (jlong) tv.tv_sec * 1000000000 + tv.tv_usec * 1000;
    }
}


/* JVM_ArrayCopy */

void JVM_ArrayCopy(JNIEnv *env, jclass ignored, jobject src, jint src_pos,
                   jobject dst, jint dst_pos, jint length) {

    TRACE("JVM_ArrayCopy(env=%p, ignored=%p, src=%p, src_pos=%d, "
          "dst=%p, dst_pos=%d, length=%d)", env, ignored, src,
          src_pos, dst, dst_pos, length);

    copyarray(src, src_pos, dst, dst_pos, length);
}


/* JVM_InitProperties */

jobject JVM_InitProperties(JNIEnv *env, jobject properties) {
    TRACE("JVM_InitProperties(env=%p, properties=%p)", env, properties);

    addDefaultProperties(properties);
    addCommandLineProperties(properties);
    return properties;
}


/* JVM_Exit */

void JVM_Exit(jint code) {
    UNIMPLEMENTED("JVM_Exit");
}


/* JVM_Halt */

void JVM_Halt(jint code) {
    TRACE("JVM_Halt(code=%d)", code);

    shutdownVM();
    jamvm_exit(code);
}


/* JVM_OnExit(void (*func)) */

void JVM_OnExit(void (*func)(void)) {
    UNIMPLEMENTED("JVM_OnExit(void (*func)");
}


/* JVM_GC */

void JVM_GC(void) {
    TRACE("JVM_GC()");

    gc1();
}


/* JVM_MaxObjectInspectionAge */

jlong JVM_MaxObjectInspectionAge(void) {
    UNIMPLEMENTED("JVM_MaxObjectInspectionAge");

    return 0;
}


/* JVM_TraceInstructions */

void JVM_TraceInstructions(jboolean on) {
    UNIMPLEMENTED("JVM_TraceInstructions");
}


/* JVM_TraceMethodCalls */

void JVM_TraceMethodCalls(jboolean on) {
    UNIMPLEMENTED("JVM_TraceMethodCalls");
}


/* JVM_TotalMemory */

jlong JVM_TotalMemory(void) {
    TRACE("JVM_TotalMemory()");

    return totalHeapMem();
}


/* JVM_FreeMemory */

jlong JVM_FreeMemory(void) {
    TRACE("JVM_FreeMemory()");

    return freeHeapMem();
}


/* JVM_MaxMemory */

jlong JVM_MaxMemory(void) {
    TRACE("JVM_MaxMemory()");

    return maxHeapMem();
}


/* JVM_ActiveProcessorCount */

jint JVM_ActiveProcessorCount(void) {
    TRACE("JVM_ActiveProcessorCount()");

    return nativeAvailableProcessors();
}


/* JVM_FillInStackTrace */

void JVM_FillInStackTrace(JNIEnv *env, jobject receiver) {
    TRACE("JVM_FillInStackTrace(env=%p, receiver=%p)", env, receiver);

    fillInStackTrace(receiver);
}


/* JVM_PrintStackTrace */

void JVM_PrintStackTrace(JNIEnv *env, jobject receiver, jobject printable) {
    UNIMPLEMENTED("JVM_PrintStackTrace");
}


/* JVM_GetStackTraceDepth */

jint JVM_GetStackTraceDepth(JNIEnv *env, jobject throwable) {
    TRACE("JVM_GetStackTraceDepth(env=%p, throwable=%p)", env, throwable);

    return stackTraceDepth(throwable);
}



/* JVM_GetStackTraceElement */

jobject JVM_GetStackTraceElement(JNIEnv *env, jobject throwable, jint index) {
    TRACE("JVM_GetStackTraceElement(env=%p, throwable=%p, index=%d)", env,
          throwable, index);

    return stackTraceElementAtIndex(throwable, index);

}


/* JVM_IHashCode */

jint JVM_IHashCode(JNIEnv* env, jobject obj) {
    TRACE("JVM_IHashCode(env=%p, jobject=%p)", env, obj);

    return obj == NULL ? 0 : (unsigned int)getObjectHashcode(obj);
}


/* JVM_MonitorWait */

void JVM_MonitorWait(JNIEnv* env, jobject handle, jlong ms) {
    TRACE("JVM_MonitorWait(env=%p, handle=%p, ms=%ld)", env, handle, ms);

    if(ms < 0) {
        signalException(java_lang_IllegalArgumentException,
                        "argument out of range");
        return;
    }

    objectWait(handle, ms, 0, TRUE);
}


/* JVM_MonitorNotify */

void JVM_MonitorNotify(JNIEnv* env, jobject handle) {
    TRACE("JVM_MonitorNotify(env=%p, handle=%p)", env, handle);

    objectNotify(handle);
}


/* JVM_MonitorNotifyAll */

void JVM_MonitorNotifyAll(JNIEnv* env, jobject handle) {
    TRACE("JVM_MonitorNotifyAll(env=%p, handle=%p)", env, handle);

    objectNotifyAll(handle);
}


/* JVM_Clone */

jobject JVM_Clone(JNIEnv* env, jobject handle) {
    Class *class = ((Object*)handle)->class;

    TRACE("JVM_Clone(env=%p, handle=%p)", env, handle);

    if(!implements(cloneable_class, class)) {
        signalChainedExceptionName("java/lang/CloneNotSupportedException",
                                   NULL, NULL);
        return NULL;
    }

    return cloneObject(handle);
}


/* JVM_InitializeCompiler  */

void JVM_InitializeCompiler (JNIEnv *env, jclass compCls) {
    UNIMPLEMENTED("JVM_InitializeCompiler");
}


/* JVM_IsSilentCompiler */

jboolean JVM_IsSilentCompiler(JNIEnv *env, jclass compCls) {
    UNIMPLEMENTED("JVM_IsSilentCompiler");

    return 0;
}


/* JVM_CompileClass */

jboolean JVM_CompileClass(JNIEnv *env, jclass compCls, jclass cls) {
    UNIMPLEMENTED("JVM_CompileClass");

    return 0;
}


/* JVM_CompileClasses */

jboolean JVM_CompileClasses(JNIEnv *env, jclass cls, jstring jname) {
    UNIMPLEMENTED("JVM_CompileClasses");

    return 0;
}


/* JVM_CompilerCommand */

jobject JVM_CompilerCommand(JNIEnv *env, jclass compCls, jobject arg) {
    UNIMPLEMENTED("JVM_CompilerCommand");

    return 0;
}


/* JVM_EnableCompiler */

void JVM_EnableCompiler(JNIEnv *env, jclass compCls) {
    IGNORED("JVM_EnableCompiler");
}


/* JVM_DisableCompiler */

void JVM_DisableCompiler(JNIEnv *env, jclass compCls) {
    IGNORED("JVM_DisableCompiler");
}


/* JVM_GetLastErrorString */

jint JVM_GetLastErrorString(char* buf, int len) {
    TRACE("JVM_GetLastErrorString(buf=%p, len=%d", buf, len);

    return strerror_r(errno, buf, len);
}


/* JVM_NativePath */

char *JVM_NativePath(char* path) {
    TRACE("JVM_NativePath(path=%s)", path);

    return path;
}


/* JVM_GetCallerClass */

jclass JVM_GetCallerClass(JNIEnv* env, int depth) {
    TRACE("JVM_GetCallerClass(env=%p, depth=%d)", env, depth);

    return getCallerClass(depth == -1 ? 2 : depth);
}


/* JVM_FindPrimitiveClass */

jclass JVM_FindPrimitiveClass(JNIEnv* env, const char *prim_name) {
    TRACE("JVM_FindPrimitiveClass(env=%p, name=%s)", env, prim_name);

    return findPrimitiveClassByName((char*)prim_name);
}


/* JVM_ResolveClass */

void JVM_ResolveClass(JNIEnv* env, jclass cls) {
    IGNORED("JVM_ResolveClass");
}


/* JVM_FindClassFromBootLoader */

jclass JVM_FindClassFromBootLoader(JNIEnv *env, const char *name) {
    Class *class;
    
    TRACE("JVM_FindClassFromBootLoader(env=%p, name=%s)", env, name);
  
    class = findClassFromClassLoader((char *)name, NULL);

    if(class == NULL)
        clearException();

    return class;
}


/* JVM_FindClassFromClassLoader */

jclass JVM_FindClassFromClassLoader(JNIEnv *env, const char *name,
                                    jboolean init, jobject loader,
                                    jboolean throw_error) {

    TRACE("JVM_FindClassFromClassLoader(env=%p, name=%s, init=%d, loader=%p,"
          " throw_error=%d)", env, name, init, loader, throw_error);

    return findClassFromLoader((char *)name, init, loader, throw_error);
}


/* JVM_FindClassFromCaller */

jclass JVM_FindClassFromCaller(JNIEnv *env, const char *name, jboolean init,
                               jobject loader, jclass caller) {

    TRACE("JVM_FindClassFromCaller(env=%p, name=%s, init=%d, loader=%p,"
          " caller=%p)", env, name, init, loader, caller);

    return findClassFromLoader((char *)name, init, loader, FALSE);
}


/* JVM_FindClassFromClass */

jclass JVM_FindClassFromClass(JNIEnv *env, const char *name, jboolean init,
                              jclass from) {
    UNIMPLEMENTED("JVM_FindClassFromClass");

    return NULL;
}


/* JVM_DefineClass */

jclass JVM_DefineClass(JNIEnv *env, const char *name, jobject loader,
                       const jbyte *buf, jsize len, jobject pd) {
    UNIMPLEMENTED("JVM_DefineClass");

    return NULL;
}


/* JVM_DefineClassWithSource */

jclass JVM_DefineClassWithSource(JNIEnv *env, const char *name, jobject loader,
                                 const jbyte *buf, jsize len, jobject pd,
                                 const char *source) {
    Class *class;

    TRACE("JVM_DefineClassWithSource(env=%p, name=%s, loader=%p, "
          "buf=%p, len=%d, pd=%p, source=%s)", env, name, loader,
          buf, len, pd, source);

    class = defineClass((char *)name, (char *)buf, 0, len, loader);

    if(class != NULL) {
        CLASS_CB(class)->protection_domain = pd;
        linkClass(class);
    }

    return class;
}


/* JVM_FindLoadedClass */

jclass JVM_FindLoadedClass(JNIEnv *env, jobject loader, jstring name) {
    char *classname = dots2Slash(String2Utf8(name));
    Class *class;

    TRACE("JVM_FindLoadedClass(env=%p, loader=%p, name=%p)", env, loader,
          name);

    class = findHashedClass(classname, loader);

    sysFree(classname);
    return class;
}


/* JVM_GetClassName */

jstring JVM_GetClassName(JNIEnv *env, jclass class) {
    Object *string;
    char *dot_name = classlibExternalClassName(class);

    TRACE("JVM_GetClassName(env=%p, cls=%p)", env, cls);

    string = createString(dot_name);
    sysFree(dot_name);

    return string;
}


/* JVM_GetClassInterfaces */

jobjectArray JVM_GetClassInterfaces(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassInterfaces(env=%p, cls=%p)", env, cls);

    return getClassInterfaces(cls);
}


/* JVM_GetClassLoader */

jobject JVM_GetClassLoader(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassLoader(env=%p, cls=%p)", env, cls);

    return CLASS_CB((Class*)cls)->class_loader;
}


/* JVM_IsInterface */

jboolean JVM_IsInterface(JNIEnv *env, jclass cls) {
    TRACE("JVM_IsInterface(env=%p, cls=%p)", env, cls);

    return IS_INTERFACE(CLASS_CB((Class*)cls)) ? TRUE : FALSE;
}


/* JVM_GetClassSigners */

jobjectArray JVM_GetClassSigners(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassSigners(env=%p, cls=%p)", env, cls);

    return CLASS_CB((Class*)cls)->signers;
}


/* JVM_SetClassSigners */

void JVM_SetClassSigners(JNIEnv *env, jclass cls, jobjectArray signers) {
    ClassBlock *cb = CLASS_CB((Class*)cls);

    TRACE("JVM_SetClassSigners(env=%p, cls=%p, signers=%p)",
          env, cls, signers);

    if(IS_PRIMITIVE(cb) || IS_ARRAY(cb))
        return;

    cb->signers = signers;
}


/* JVM_GetResourceLookupCacheURLs
   is part of the
   JDK-8061651 JDK8u API
*/

jobjectArray JVM_GetResourceLookupCacheURLs(JNIEnv *env, jobject loader) {
    return NULL; // tell OpenJDK 8 that the lookup cache API is unavailable
}

/* JVM_GetResourceLookupCache
   is unused however it is part of the
   JDK-8061651 JDK8u API
*/

jintArray JVM_GetResourceLookupCache(JNIEnv *env, jobject loader, const char *resource_name) {
    UNIMPLEMENTED("JVM_GetResourceLookupCache");
    return 0;
}

/* JVM_KnownToNotExist
   is unused however it is part of the
   JDK-8061651 JDK8u API
*/

jboolean JVM_KnownToNotExist(JNIEnv *env, jobject loader, const char *classname) {
    UNIMPLEMENTED("JVM_KnownToNotExist");
    return 0;
}

/* JVM_GetProtectionDomain */

jobject JVM_GetProtectionDomain(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetProtectionDomain(env=%p, cls=%p)", env, cls);

    return CLASS_CB((Class*)cls)->protection_domain;
}


/* JVM_SetProtectionDomain */

void JVM_SetProtectionDomain(JNIEnv *env, jclass cls,
                             jobject protection_domain) {
    UNIMPLEMENTED("JVM_SetProtectionDomain");
}


/* JVM_DoPrivileged */

jobject JVM_DoPrivileged(JNIEnv *env, jclass cls, jobject action,
                         jobject context, jboolean wrapException) {
    Object *result, *excep;
    MethodBlock *mb;

    TRACE("JVM_DoPrivileged(env=%p, cls=%p, action=%p, context=%p,"
          " wrapException=%d)", env, cls, action, context, wrapException);

    if (action == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return NULL;
    }

    /* lookup run() method (throw no exceptions) */
    mb = lookupMethod(((Object*)action)->class, SYMBOL(run),
                                                SYMBOL(___java_lang_Object));

    if(mb == NULL || !(mb->access_flags & ACC_PUBLIC)
                  ||  (mb->access_flags & ACC_STATIC)) {

        signalException(java_lang_InternalError,
                        "JVM_DoPrivileged: no run method");
        return NULL;
    }

    /* XXX It seems something with a privileged stack needs to be done
       here. */

    result = *(Object**)executeMethod(((Object*)action), mb);

    if((excep = exceptionOccurred())) {
        if(isInstanceOf(exception_class, excep->class) &&
                !isInstanceOf(runtime_excp_class, excep->class)) {
            Object *pae;
            clearException(); 

            if((pae = allocObject(priv_act_excp_init_mb->class)) != NULL) {
                executeMethod(pae, priv_act_excp_init_mb, excep);
                setException(pae);
            }
        }
        return NULL;
    }

    return result;
}


/* JVM_GetInheritedAccessControlContext */

jobject JVM_GetInheritedAccessControlContext(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetInheritedAccessControlContext");

    return NULL;
}


/* JVM_GetStackAccessControlContext */

jobject JVM_GetStackAccessControlContext(JNIEnv *env, jclass cls) {
    IGNORED("GetStackAccessControlContext(env=%p, cls=%p)", env, cls);

    /* XXX All stuff I tested so far works without that function.  At
       some point we have to implement it, but I disable the output
       for now to make IcedTea happy. */

    return NULL;
}


/* JVM_IsArrayClass */

jboolean JVM_IsArrayClass(JNIEnv *env, jclass cls) {
    TRACE("JVM_IsArrayClass(env=%p, cls=%p)", env, cls);

    return IS_ARRAY(CLASS_CB((Class*)cls));
}


/* JVM_IsPrimitiveClass */

jboolean JVM_IsPrimitiveClass(JNIEnv *env, jclass cls) {
    TRACE("JVM_IsPrimitiveClass(env=%p, cls=%p)", env, cls);

    return IS_PRIMITIVE(CLASS_CB((Class*)cls));
}


/* JVM_GetComponentType */

jclass JVM_GetComponentType(JNIEnv *env, jclass cls) {
    Class *class = cls;
    Class *component_type = NULL;
    ClassBlock *cb = CLASS_CB(class);

    TRACE("JVM_GetComponentType(env=%p, cls=%p)", env, cls);

    if(IS_ARRAY(cb))
        switch(cb->name[1]) {
            case '[':
                component_type = findArrayClassFromClass(&cb->name[1], class);
                break;

            default:
                component_type = cb->element_class;
                break;
        }
 
    return component_type;
}


/* JVM_GetClassModifiers */

jint JVM_GetClassModifiers(JNIEnv *env, jclass cls) {
    ClassBlock *cb = CLASS_CB((Class*)cls);
    int flags;

    TRACE("JVM_GetClassModifiers(env=%p, cls=%p)", env, cls);

    if(cb->declaring_class)
        flags = cb->inner_access_flags;
    else
        flags = cb->access_flags;

    return flags & ~ACC_SUPER;
}


/* JVM_GetDeclaredClasses */

jobjectArray JVM_GetDeclaredClasses(JNIEnv *env, jclass ofClass) {
    TRACE("JVM_GetDeclaredClasses(env=%p, ofClass=%p)", env, ofClass);

    return getClassClasses(ofClass, FALSE);
}


/* JVM_GetDeclaringClass */

jclass JVM_GetDeclaringClass(JNIEnv *env, jclass ofClass) {
    TRACE("JVM_GetDeclaringClass(env=%p, ofClass=%p)", env, ofClass);

    return getDeclaringClass(ofClass);
}


/* JVM_GetClassSignature */

jstring JVM_GetClassSignature(JNIEnv *env, jclass cls) {
    ClassBlock *cb = CLASS_CB((Class*)cls);

    TRACE("JVM_GetClassSignature(env=%p, cls=%p)", env, cls);

    return cb->signature == NULL ? NULL : createString(cb->signature);
}


/* JVM_GetClassAnnotations */

jbyteArray JVM_GetClassAnnotations(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassAnnotations(env=%p, cls=%p)", env, cls);

    return getAnnotationsAsArray(getClassAnnotationData((Class*)cls));
}


#ifdef JSR308
/* JVM_GetClassTypeAnnotations */

jbyteArray JVM_GetClassTypeAnnotations(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassTypeAnnotations(env=%p, cls=%p)", env, cls);

    return getAnnotationsAsArray(getClassTypeAnnotationData((Class*)cls));
}

/* JVM_GetFieldTypeAnnotations */

jbyteArray JVM_GetFieldTypeAnnotations(JNIEnv *env, jobject field) {
    FieldBlock *fb = fbFromReflectObject(field);

    TRACE("JVM_GetFieldTypeAnnotations(env=%p, field=%p)", env, field);

    return getAnnotationsAsArray(getFieldTypeAnnotationData(fb));
}

/* JVM_GetMethodTypeAnnotations */

jbyteArray JVM_GetMethodTypeAnnotations(JNIEnv *env, jobject method) {
    MethodBlock *mb = mbFromReflectObject(method);

    TRACE("JVM_GetMethodTypeAnnotations(env=%p, method=%p)", env, method);

    return getAnnotationsAsArray(getMethodTypeAnnotationData(mb));
}
#endif


/* JVM_GetFieldAnnotations */

jbyteArray JVM_GetFieldAnnotations(JNIEnv *env, jobject field) {
    FieldBlock *fb = fbFromReflectObject(field);

    TRACE("JVM_GetFieldAnnotations(env=%p, field=%p)", env, field);

    return getAnnotationsAsArray(getFieldAnnotationData(fb));
}


/* JVM_GetMethodAnnotations */

jbyteArray JVM_GetMethodAnnotations(JNIEnv *env, jobject method) {
    MethodBlock *mb = mbFromReflectObject(method);

    TRACE("JVM_GetMethodAnnotations(env=%p, method=%p)", env, method);

    return getAnnotationsAsArray(getMethodAnnotationData(mb));
}


/* JVM_GetMethodDefaultAnnotationValue */

jbyteArray JVM_GetMethodDefaultAnnotationValue(JNIEnv *env, jobject method) {
    MethodBlock *mb = mbFromReflectObject(method);

    TRACE("JVM_GetMethodDefaultAnnotationValue(env=%p, method=%p)", env,
          method);

    return getAnnotationsAsArray(getMethodDefaultValueAnnotationData(mb));
}


/* JVM_GetMethodParameterAnnotations */

jbyteArray JVM_GetMethodParameterAnnotations(JNIEnv *env, jobject method) {
    MethodBlock *mb = mbFromReflectObject(method);

    TRACE("JVM_GetMethodParameterAnnotations(env=%p, method=%p)", env, method);

    return getAnnotationsAsArray(getMethodParameterAnnotationData(mb));
}


#ifdef JSR901
/* JVM_GetMethodParameters */

jbyteArray JVM_GetMethodParameters(JNIEnv *env, jobject method) {
    TRACE("JVM_GetMethodParameters(env=%p, method=%p)", env, method);

    return getMethodParameters(method);
}
#endif


/* JVM_GetClassDeclaredFields */

jobjectArray JVM_GetClassDeclaredFields(JNIEnv *env, jclass ofClass,
                                        jboolean publicOnly) {

    TRACE("JVM_GetClassDeclaredFields(env=%p, ofClass=%p, publicOnly=%d)",
          env, ofClass, publicOnly);

    return getClassFields(ofClass, publicOnly);
}


/* JVM_GetClassDeclaredMethods */

jobjectArray JVM_GetClassDeclaredMethods(JNIEnv *env, jclass ofClass,
                                         jboolean publicOnly) {

    TRACE("JVM_GetClassDeclaredMethods(env=%p, ofClass=%p, publicOnly=%d)",
          env, ofClass, publicOnly);

    return getClassMethods(ofClass, publicOnly);
}


/* JVM_GetClassDeclaredConstructors */

jobjectArray JVM_GetClassDeclaredConstructors(JNIEnv *env, jclass ofClass,
                                              jboolean publicOnly) {

    TRACE("JVM_GetClassDeclaredConstructors(env=%p, ofClass=%p, publicOnly=%d)",
           env, ofClass, publicOnly);

    return getClassConstructors(ofClass, publicOnly);
}


/* JVM_GetClassAccessFlags */

jint JVM_GetClassAccessFlags(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassAccessFlags(env=%p, cls=%p)", env, cls);

    return CLASS_CB((Class*)cls)->access_flags;
}


/* JVM_GetClassConstantPool */

jobject JVM_GetClassConstantPool(JNIEnv *env, jclass cls) {
    Object *constant_pool;

    TRACE("JVM_GetClassConstantPool(env=%p, cls=%p)", env, cls);

    if((constant_pool = allocObject(constant_pool_class)) != NULL)
        INST_DATA(constant_pool, Object*, constant_pool_oop_offset) = cls;

    return constant_pool;
}


/* JVM_ConstantPoolGetSize */

jint JVM_ConstantPoolGetSize(JNIEnv *env, jobject unused, jobject jcpool) {
    Class *class = jcpool;

    TRACE("JVM_ConstantPoolGetSize(env=%p, unused=%p, jcpool=%p)", env,
          unused, jcpool);

    return CLASS_CB(class)->constant_pool_count;
}


ConstantPool *getConstantPool(Object *jcpool, int idx, int type) {
    Class *class = jcpool;
    ClassBlock *cb = CLASS_CB(class);
    ConstantPool *cp = &cb->constant_pool;

    if(idx < 0 || idx >= cb->constant_pool_count) {
        signalException(java_lang_IllegalArgumentException,
                        "constant pool index out of bounds");
        return NULL;
    }

    if(CP_TYPE(cp, idx) != type) {
        signalException(java_lang_IllegalArgumentException,
                        "constant pool index has wrong type");
        return NULL;
    }

    return cp;
}

/* JVM_ConstantPoolGetClassAt */

jclass JVM_ConstantPoolGetClassAt(JNIEnv *env, jobject unused, jobject jcpool,
                                  jint index) {
    TBD("JVM_ConstantPoolGetClassAt(env=%p, jcpool=%p, index=%d)", env,
          jcpool, index);

    return NULL;
}


/* JVM_ConstantPoolGetClassAtIfLoaded */

jclass JVM_ConstantPoolGetClassAtIfLoaded(JNIEnv *env, jobject unused,
                                          jobject jcpool, jint index) {
    TBD("JVM_ConstantPoolGetClassAtIfLoaded(env=%p, unused=%p, jcpool=%p,"
          " index=%d)", env, unused, jcpool, index);

    return NULL;
}


/* JVM_ConstantPoolGetMethodAt */

jobject JVM_ConstantPoolGetMethodAt(JNIEnv *env, jobject unused,
                                    jobject jcpool, jint index) {
    TBD("JVM_ConstantPoolGetMethodAt: jcpool=%p, index=%d", jcpool, index);

    return NULL;
}


/* JVM_ConstantPoolGetMethodAtIfLoaded */

jobject JVM_ConstantPoolGetMethodAtIfLoaded(JNIEnv *env, jobject unused,
                                            jobject jcpool, jint index) {

    TBD("JVM_ConstantPoolGetMethodAtIfLoaded: jcpool=%p, index=%d",
          jcpool, index);

    return NULL;
}


/* JVM_ConstantPoolGetFieldAt */

jobject JVM_ConstantPoolGetFieldAt(JNIEnv *env, jobject unused, jobject jcpool,
                                   jint index) {
    TBD("JVM_ConstantPoolGetFieldAt: jcpool=%p, index=%d", jcpool, index);

    return NULL;
}


/* JVM_ConstantPoolGetFieldAtIfLoaded */

jobject JVM_ConstantPoolGetFieldAtIfLoaded(JNIEnv *env, jobject unused,
                                           jobject jcpool, jint index) {

    TBD("JVM_ConstantPoolGetFieldAtIfLoaded: jcpool=%p, index=%d",
          jcpool, index);

    return NULL;
}


/* JVM_ConstantPoolGetMemberRefInfoAt */

jobjectArray JVM_ConstantPoolGetMemberRefInfoAt(JNIEnv *env, jobject unused,
                                                jobject jcpool, jint index) {
    UNUSED("JVM_ConstantPoolGetMemberRefInfoAt: jcpool=%p, index=%d",
           jcpool, index);

    return NULL;
}


/* JVM_ConstantPoolGetIntAt */

jint JVM_ConstantPoolGetIntAt(JNIEnv *env, jobject unused, jobject jcpool,
                              jint index) {

    ConstantPool *cp = getConstantPool(jcpool, index, CONSTANT_Integer);

    TRACE("JVM_ConstantPoolGetIntAt: jcpool=%p, index=%d", jcpool, index);

    if(cp != NULL)
        return CP_INTEGER(cp, index);

    return 0;
}


/* JVM_ConstantPoolGetLongAt */

jlong JVM_ConstantPoolGetLongAt(JNIEnv *env, jobject unused, jobject jcpool,
                                jint index) {

    ConstantPool *cp = getConstantPool(jcpool, index, CONSTANT_Long);

    TRACE("JVM_ConstantPoolGetLongAt: jcpool=%p, index=%d", jcpool, index);

    if(cp != NULL)
        return CP_LONG(cp, index);

    return 0;
}


/* JVM_ConstantPoolGetFloatAt */

jfloat JVM_ConstantPoolGetFloatAt(JNIEnv *env, jobject unused, jobject jcpool,
                                  jint index) {

    ConstantPool *cp = getConstantPool(jcpool, index, CONSTANT_Float);

    TRACE("JVM_ConstantPoolGetFloatAt: jcpool=%p, index=%d", jcpool, index);

    if(cp != NULL)
        return CP_FLOAT(cp, index);

    return 0;
}


/* JVM_ConstantPoolGetDoubleAt */

jdouble JVM_ConstantPoolGetDoubleAt(JNIEnv *env, jobject unused,
                                    jobject jcpool, jint index) {

    ConstantPool *cp = getConstantPool(jcpool, index, CONSTANT_Double);

    TRACE("JVM_ConstantPoolGetDoubleAt: jcpool=%p, index=%d", jcpool, index);

    if(cp != NULL)
        return CP_DOUBLE(cp, index);

    return 0;
}


/* JVM_ConstantPoolGetStringAt */

jstring JVM_ConstantPoolGetStringAt(JNIEnv *env, jobject unused,
                                    jobject jcpool, jint index) {
    TBD("JVM_ConstantPoolGetStringAt: jcpool=%p, index=%d", jcpool, index);

    return NULL;
}


/* JVM_ConstantPoolGetUTF8At */

jstring JVM_ConstantPoolGetUTF8At(JNIEnv *env, jobject unused, jobject jcpool,
                                  jint index) {

    ConstantPool *cp = getConstantPool(jcpool, index, CONSTANT_Utf8);

    TRACE("JVM_ConstantPoolGetUTF8At: jcpool=%p, index=%d", jcpool, index);

    if(cp != NULL)
        return createString(CP_UTF8(cp, index));

    return NULL;
}


/* JVM_DesiredAssertionStatus */

jboolean JVM_DesiredAssertionStatus(JNIEnv *env, jclass unused, jclass cls) {
    TRACE("JVM_DesiredAssertionStatus(env=%p, unused=%p, cls=%p)", env,
          unused, cls);

    return FALSE;
}


/* JVM_AssertionStatusDirectives */

jobject JVM_AssertionStatusDirectives(JNIEnv *env, jclass unused) {
    Class *asd_class = findSystemClass("java/lang/AssertionStatusDirectives");
    Class *string_class = findArrayClass(SYMBOL(array_java_lang_String));
    FieldBlock *classes_fb, *packages_fb;
    Object *asd, *names;

    TRACE("JVM_AssertionStatusDirectives(env=%p, unused=%p)", env, unused);

    if(asd_class == NULL || string_class == NULL)
        return NULL;

    classes_fb = findField(asd_class, SYMBOL(classes),
                                      SYMBOL(array_java_lang_String));
    packages_fb = findField(asd_class, newUtf8("packages"),
                                       SYMBOL(array_java_lang_String));

    asd = allocObject(asd_class);
    names = allocArray(string_class, 0, sizeof(Object*));

    if(asd == NULL)
        return NULL;

    INST_DATA(asd, Object*, classes_fb->u.offset) = names;
    INST_DATA(asd, Object*, packages_fb->u.offset) = names;

    return asd;
}

/* JVM_GetClassNameUTF */

const char *JVM_GetClassNameUTF(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassNameUTF");

    return NULL;
}


/* JVM_GetClassCPTypes */

void JVM_GetClassCPTypes(JNIEnv *env, jclass cls, unsigned char *types) {
    UNIMPLEMENTED("JVM_GetClassCPTypes");
}


/* JVM_GetClassCPEntriesCount */

jint JVM_GetClassCPEntriesCount(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassCPEntriesCount");

    return 0;
}


/* JVM_GetClassFieldsCount */

jint JVM_GetClassFieldsCount(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassFieldsCount");

    return 0;
}


/* JVM_GetClassMethodsCount */

jint JVM_GetClassMethodsCount(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassMethodsCount");

    return 0;
}


/* JVM_GetMethodIxExceptionIndexes */

void JVM_GetMethodIxExceptionIndexes(JNIEnv *env, jclass cls,
                                     jint method_index,
                                     unsigned short *exceptions) {
    UNIMPLEMENTED("JVM_GetMethodIxExceptionIndexes");
}


/* JVM_GetMethodIxExceptionsCount */

jint JVM_GetMethodIxExceptionsCount(JNIEnv *env, jclass cls,
                                    jint method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxExceptionsCount");

    return 0;
}


/* JVM_GetMethodIxByteCode */

void JVM_GetMethodIxByteCode(JNIEnv *env, jclass cls, jint method_index,
                             unsigned char *code) {
    UNIMPLEMENTED("JVM_GetMethodIxByteCode");
}


/* JVM_GetMethodIxByteCodeLength */

jint JVM_GetMethodIxByteCodeLength(JNIEnv *env, jclass cls,
                                   jint method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxByteCodeLength");

    return 0;
}


/* JVM_GetMethodIxExceptionTableEntry */

typedef struct {
    jint start_pc;
    jint end_pc;
    jint handler_pc;
    jint catchType;
} JVM_ExceptionTableEntryType;

void JVM_GetMethodIxExceptionTableEntry(JNIEnv *env, jclass cls,
                                        jint method_index, jint entry_index,
                                        JVM_ExceptionTableEntryType *entry) {
    UNIMPLEMENTED("JVM_GetMethodIxExceptionTableEntry");
}


/* JVM_GetMethodIxExceptionTableLength */

jint JVM_GetMethodIxExceptionTableLength(JNIEnv *env, jclass cls,
                                         int method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxExceptionTableLength");

    return 0;
}


/* JVM_GetMethodIxModifiers */

jint JVM_GetMethodIxModifiers(JNIEnv *env, jclass cls, int method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxModifiers");

    return 0;
}


/* JVM_GetFieldIxModifiers */

jint JVM_GetFieldIxModifiers(JNIEnv *env, jclass cls, int field_index) {
    UNIMPLEMENTED("JVM_GetFieldIxModifiers");

    return 0;
}


/* JVM_GetMethodIxLocalsCount */

jint JVM_GetMethodIxLocalsCount(JNIEnv *env, jclass cls, int method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxLocalsCount");

    return 0;
}


/* JVM_GetMethodIxArgsSize */

jint JVM_GetMethodIxArgsSize(JNIEnv *env, jclass cls, int method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxArgsSize");

    return 0;
}


/* JVM_GetMethodIxMaxStack */

jint JVM_GetMethodIxMaxStack(JNIEnv *env, jclass cls, int method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxMaxStack");

    return 0;
}


/* JVM_IsConstructorIx */

jboolean JVM_IsConstructorIx(JNIEnv *env, jclass cls, int method_index) {
    UNIMPLEMENTED("JVM_IsConstructorIx");

    return 0;
}


/* JVM_IsVMGeneratedMethodIx */

jboolean JVM_IsVMGeneratedMethodIx(JNIEnv *env, jclass cls, int method_index) {
    UNIMPLEMENTED("JVM_IsVMGeneratedMethodIx");

    return 0;
}

/* JVM_GetMethodIxNameUTF */

const char *JVM_GetMethodIxNameUTF(JNIEnv *env, jclass cls, jint method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxNameUTF");

    return NULL;
}


/* JVM_GetMethodIxSignatureUTF */

const char *JVM_GetMethodIxSignatureUTF(JNIEnv *env, jclass cls,
                                        jint method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxSignatureUTF");

    return NULL;
}


/* JVM_GetCPFieldNameUTF */

const char *JVM_GetCPFieldNameUTF(JNIEnv *env, jclass cls, jint cp_index) {
    UNIMPLEMENTED("JVM_GetCPFieldNameUTF");

    return NULL;
}


/* JVM_GetCPMethodNameUTF */

const char *JVM_GetCPMethodNameUTF(JNIEnv *env, jclass cls, jint cp_index) {
    UNIMPLEMENTED("JVM_GetCPMethodNameUTF");

    return NULL;
}


/* JVM_GetCPMethodSignatureUTF */

const char *JVM_GetCPMethodSignatureUTF(JNIEnv *env, jclass cls,
                                        jint cp_index) {
    UNIMPLEMENTED("JVM_GetCPMethodSignatureUTF");

    return NULL;
}


/* JVM_GetCPFieldSignatureUTF */

const char *JVM_GetCPFieldSignatureUTF(JNIEnv *env, jclass cls, jint cp_index) {
    UNIMPLEMENTED("JVM_GetCPFieldSignatureUTF");

    return NULL;
}


/* JVM_GetCPClassNameUTF */

const char *JVM_GetCPClassNameUTF(JNIEnv *env, jclass cls, jint cp_index) {
    UNIMPLEMENTED("JVM_GetCPClassNameUTF");

    return NULL;
}


/* JVM_GetCPFieldClassNameUTF */

const char *JVM_GetCPFieldClassNameUTF(JNIEnv *env, jclass cls, jint cp_index) {
    UNIMPLEMENTED("JVM_GetCPFieldClassNameUTF");

    return NULL;
}


/* JVM_GetCPMethodClassNameUTF */

const char *JVM_GetCPMethodClassNameUTF(JNIEnv *env, jclass cls,
                                        jint cp_index) {
    UNIMPLEMENTED("JVM_GetCPMethodClassNameUTF");

    return NULL;
}


/* JVM_GetCPFieldModifiers */

jint JVM_GetCPFieldModifiers(JNIEnv *env, jclass cls, int cp_index,
                             jclass called_cls) {
    UNIMPLEMENTED("JVM_GetCPFieldModifiers");

    return 0;
}


/* JVM_GetCPMethodModifiers */

jint JVM_GetCPMethodModifiers(JNIEnv *env, jclass cls, int cp_index,
                              jclass called_cls) {
    UNIMPLEMENTED("JVM_GetCPMethodModifiers");

    return 0;
}


/* JVM_ReleaseUTF */

void JVM_ReleaseUTF(const char *utf) {
    UNIMPLEMENTED("JVM_ReleaseUTF");
}


/* JVM_IsSameClassPackage */

jboolean JVM_IsSameClassPackage(JNIEnv *env, jclass class1, jclass class2) {
    UNIMPLEMENTED("JVM_IsSameClassPackage");

    return 0;
}


/* JVM_Open */

#define JVM_EEXIST       -100
#define O_DELETE      0x10000

jint JVM_Open(const char *fname, jint flags, jint mode) {
    int result;

    TRACE("JVM_Open(fname=%s, flags=%d, mode=%d)", fname, flags, mode);

    /* O_DELETE is a special classlib flag which indicates that
       the file should be deleted after opening.  As it is not a
       standard flag we must remove it (it also conflicts with
       Linux's non-standard O_DIRECTORY, causing open to fail)
   */
    result = open(fname, flags & ~O_DELETE, mode);
    if(flags & O_DELETE)
        unlink(fname);

    if(result >= 0)
        return result;

    switch(errno) {
        case EEXIST:
            return JVM_EEXIST;
        default:
            return -1;
    }
}


/* JVM_Close */

jint JVM_Close(jint fd) {
    TRACE("JVM_Close(fd=%d)", fd);

    return close(fd);
}


/* JVM_Read */

jint JVM_Read(jint fd, char* buf, jint nbytes) {
    TRACE("JVM_Read(fd=%d, buf=%p, nbytes=%d)", fd, buf, nbytes);

    return read(fd, buf, nbytes);
}


/* JVM_Write */

jint JVM_Write(jint fd, char* buf, jint nbytes) {
    TRACE("JVM_Write(fd=%d, buf=%s, nbytes=%d)", fd, buf, nbytes);

    return write(fd, buf, nbytes);
}


/* JVM_Available */

jint JVM_Available(jint fd, jlong *bytes) {
    struct stat sb;

    TRACE("JVM_Available(fd=%d, bytes=%p)", fd, bytes);

    if(fstat(fd, &sb) == -1)
        return 0;

    switch(sb.st_mode & S_IFMT) {
        case S_IFCHR:
        case S_IFIFO:
        case S_IFSOCK: {
            int n;

            if(ioctl(fd, FIONREAD, &n) == -1)
                return 0;

            *bytes = n;
            return 1;
        }

        default: {
            off_t cur, end;

            if((cur = lseek(fd, 0, SEEK_CUR)) == -1)
                return 0;

            if((end = lseek(fd, 0, SEEK_END)) == -1)
                return 0;

            if(lseek(fd, cur, SEEK_SET) == -1)
                return 0;

            *bytes = end - cur;
            return 1;
        }
    }
}


/* JVM_Lseek */

jlong JVM_Lseek(jint fd, jlong offset, jint whence) {
    TRACE("JVM_Lseek(fd=%d, offset=%ld, whence=%d)", fd, offset, whence);

    return lseek(fd, offset, whence);
}


/* JVM_SetLength */

jint JVM_SetLength(jint fd, jlong length) {
    TRACE("JVM_SetLength(fd=%d, length=%ld)", length);

    if(ftruncate(fd, length) == -1)
        return 0;

    return 1;
}


/* JVM_Sync */

jint JVM_Sync(jint fd) {
    TRACE("JVM_Sync(fd=%d)", fd);

    return fsync(fd);
}


/* JVM_StartThread */

void JVM_StartThread(JNIEnv* env, jobject jthread) {
    TRACE("JVM_StartThread(env=%p, jthread=%p)", env, jthread);

    createJavaThread(jthread, 0);
}


/* JVM_StopThread */

void JVM_StopThread(JNIEnv* env, jobject jthread, jobject throwable) {
    DEPRECATED("JVM_StopThread");
}


/* JVM_IsThreadAlive */

jboolean JVM_IsThreadAlive(JNIEnv* env, jobject jthread) {
    TRACE("JVM_IsThreadAlive(env=%p, jthread=%p)", env, jthread);

    return jThreadIsAlive(jthread);
}


/* JVM_SuspendThread */

void JVM_SuspendThread(JNIEnv* env, jobject jthread) {
    UNIMPLEMENTED("JVM_SuspendThread(env=%p, jthread=%p)", env, jthread);
}


/* JVM_ResumeThread */

void JVM_ResumeThread(JNIEnv* env, jobject jthread) {
    UNIMPLEMENTED("JVM_ResumeThread(env=%p, jthread=%p)", env, jthread);
}


/* JVM_SetNativeThreadName */

void JVM_SetNativeThreadName(JNIEnv *env, jobject jthread, jobject name) {
    IGNORED("JVM_SetNativeThreadName");
}


/* JVM_SetThreadPriority */

void JVM_SetThreadPriority(JNIEnv* env, jobject jthread, jint prio) {
    IGNORED("JVM_SetThreadPriority(env=%p, jthread=%p, prio=%d)",
            env, jthread, prio);
}


/* JVM_Yield */

void JVM_Yield(JNIEnv *env, jclass threadClass) {
    TRACE("JVM_Yield(env=%p, threadClass=%p)", env, threadClass);

    threadYield(NULL);
}


/* JVM_Sleep */

void JVM_Sleep(JNIEnv* env, jclass threadClass, jlong millis) {
    Thread *self = threadSelf();

    TRACE("JVM_Sleep(env=%p, threadClass=%p, millis=%ld)", env, threadClass,
          millis);

    threadSleep(self, millis, 0);
}


/* JVM_CurrentThread */

jobject JVM_CurrentThread(JNIEnv* env, jclass threadClass) {
    TRACE("JVM_CurrentThread(env=%p, threadClass=%p)", env, threadClass);

    return getExecEnv()->thread; 
}


/* JVM_CountStackFrames */

jint JVM_CountStackFrames(JNIEnv* env, jobject jthread) {
    DEPRECATED("JVM_CountStackFrames");

    return 0;
}


/* JVM_Interrupt */

void JVM_Interrupt(JNIEnv* env, jobject jthread) {
    Thread *thread;

    TRACE("JVM_Interrupt(env=%p, jthread=%p)", env, jthread);

    if((thread = jThread2Thread(jthread)) == NULL)
        return;

    threadInterrupt(thread);
}


/* JVM_IsInterrupted */

jboolean JVM_IsInterrupted(JNIEnv* env, jobject jthread,
                           jboolean clear_interrupted) {

    Thread *thread;

    TRACE("JVM_IsInterrupted(env=%p, jthread=%p, clear_interrupted=%d)",
          env, jthread, clear_interrupted);

    if((thread = jThread2Thread(jthread)) == NULL)
        return JNI_FALSE;

    return clear_interrupted ? threadInterrupted(thread)
                             : threadIsInterrupted(thread);
}


/* JVM_HoldsLock */

jboolean JVM_HoldsLock(JNIEnv *env, jclass threadClass, jobject obj) {
    TRACE("JVM_HoldsLock(env=%p, threadClass=%p, obj=%p)", env,
          threadClass, obj);

    if(obj == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return FALSE;
    }

    return objectLockedByCurrent(obj);
}


/* JVM_DumpAllStacks */

void JVM_DumpAllStacks(JNIEnv* env, jclass unused) {
    UNIMPLEMENTED("JVM_DumpAllStacks");
}


/* JVM_CurrentLoadedClass */

jclass JVM_CurrentLoadedClass(JNIEnv *env) {
    UNIMPLEMENTED("JVM_CurrentLoadedClass");

    return NULL;
}


/* JVM_CurrentClassLoader */

jobject JVM_CurrentClassLoader(JNIEnv *env) {
    TRACE("JVM_CurrentClassLoader(env=%p)", env);

    return firstNonNullClassLoader();
}


/* JVM_GetClassContext */

jobjectArray JVM_GetClassContext(JNIEnv *env) {
    TRACE("JVM_GetClassContext(env=%p)", env);

    return getClassContext();
}


/* JVM_ClassDepth */

jint JVM_ClassDepth(JNIEnv *env, jstring name) {
    UNIMPLEMENTED("JVM_ClassDepth");

    return 0;
}


/* JVM_ClassLoaderDepth */

jint JVM_ClassLoaderDepth(JNIEnv *env) {
    UNIMPLEMENTED("JVM_ClassLoaderDepth");

    return 0;
}


/* JVM_GetSystemPackage */

jstring JVM_GetSystemPackage(JNIEnv *env, jstring name) {
    char *package_name;
    Object *package;

    TRACE("JVM_GetSystemPackage(env=%p, name=%p)", env, name);

    package_name = slash2Dots(String2Utf8(name));
    package_name[strlen(package_name) - 1] = '\0';
    package = bootPackage(package_name);
    sysFree(package_name);

    return package;
}


/* JVM_GetSystemPackages */

jobjectArray JVM_GetSystemPackages(JNIEnv *env) {
    TRACE("JVM_GetSystemPackages(env=%p)", env);

    return bootPackages();
}


/* JVM_AllocateNewObject */

jobject JVM_AllocateNewObject(JNIEnv *env, jobject receiver,
                              jclass currClass, jclass initClass) {
    UNIMPLEMENTED("JVM_AllocateNewObject");

    return NULL;
}


/* JVM_AllocateNewArray */

jobject JVM_AllocateNewArray(JNIEnv *env, jobject obj, jclass currClass,
                             jint length) {
    UNIMPLEMENTED("JVM_AllocateNewArray");

    return NULL;
}


/* JVM_LatestUserDefinedLoader */

jobject JVM_LatestUserDefinedLoader(JNIEnv *env) {
    TRACE("JVM_LatestUserDefinedLoader(env=%p)", env);

    return firstNonNullClassLoader();
}


/* JVM_LoadClass0 */

jclass JVM_LoadClass0(JNIEnv *env, jobject receiver, jclass currClass,
                      jstring currClassName) {
    UNIMPLEMENTED("JVM_LoadClass0");

    return NULL;
}


/* JVM_GetArrayLength */

jint JVM_GetArrayLength(JNIEnv *env, jobject arr) {
    TRACE("JVM_GetArrayLength(env=%p, arr=%p)", env, arr);

    if(arr == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return 0;
    } else {
        ClassBlock *cb = CLASS_CB(((Object*)arr)->class);

        if(!IS_ARRAY(cb)) {
            signalException(java_lang_IllegalArgumentException, NULL);
            return 0;
        }

        return ARRAY_LEN((Object *)arr);
    }
}


jboolean JVM_IsUseContainerSupport() {
    return FALSE;
}

/* JVM_GetArrayElement */

jobject JVM_GetArrayElement(JNIEnv *env, jobject arr, jint index) {
    TRACE("JVM_GetArrayElement(env=%p, arr=%p, index=%d)", env, arr, index);

    if(arr == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return NULL;
    } else {
        ClassBlock *cb = CLASS_CB(((Object*)arr)->class);

        if(!IS_ARRAY(cb)) {
            signalException(java_lang_IllegalArgumentException, NULL);
            return NULL;
        }

        if(index > ARRAY_LEN((Object *)arr)) {
            signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
            return NULL;
        } else {
            u4 widened;
            int size = sigElement2Size(cb->name[1]);
            void *addr = &ARRAY_DATA((Object*)arr, char)[index * size];
            Class *type = cb->dim > 1 ? cb->super : cb->element_class;

            if(size < sizeof(u4)) {
                widened = size == 1 ? *(signed char*)addr : 
                                  cb->name[1] == 'S' ? *(signed short*)addr
                                                     : *(unsigned short*)addr;
                addr = &widened;
            }

            return getReflectReturnObject(type, addr, REF_SRC_FIELD);
        }
    }
}

/* JVM_GetPrimitiveArrayElement */

jvalue JVM_GetPrimitiveArrayElement(JNIEnv *env, jobject arr, jint index,
                                    jint wCode) {
    jvalue jv;
    
    TRACE("JVM_GetPrimitiveArrayElement(env=%p, arr=%p, index=%d, wCode=%d)",
    	  env, arr, index, wCode);

    if(arr == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else {
        ClassBlock *cb = CLASS_CB(((Object*)arr)->class);

        if(!IS_ARRAY(cb))
            signalException(java_lang_IllegalArgumentException, NULL);
        else {
            if(index > ARRAY_LEN((Object *)arr))
                signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
            else {
                ClassBlock *elem_cb = CLASS_CB(cb->element_class);
                if(!IS_PRIMITIVE(elem_cb) || cb->dim > 1)
                    signalException(java_lang_IllegalArgumentException, NULL);
                else {
                    int src_idx = getPrimTypeIndex(elem_cb);
                    int size = primTypeIndex2Size(src_idx);
                    int dst_idx = typeNo2PrimTypeIndex(wCode);
                    void *addr = &ARRAY_DATA((Object*)arr, char)[index * size];

                    widenPrimitiveElement(src_idx, dst_idx, addr, &jv);
                }
            }
        }
    }
    return jv;
}


/* JVM_SetArrayElement */

void JVM_SetArrayElement(JNIEnv *env, jobject arr, jint index, jobject val) {
    TRACE("JVM_SetArrayElement(env=%p, arr=%p, index=%d, val=%p)", env, arr,
          index, val);

    if(arr == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else {
        ClassBlock *cb = CLASS_CB(((Object*)arr)->class);

        if(!IS_ARRAY(cb))
            goto illegal_arg;

        if(index > ARRAY_LEN((Object *)arr))
            signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
        else {
            ClassBlock *elem_cb = CLASS_CB(cb->element_class);

            if(!IS_PRIMITIVE(elem_cb) || cb->dim > 1) {
                if(val != NULL && !arrayStoreCheck(((Object*)arr)->class,
                                                   ((Object*)val)->class))
                    goto illegal_arg;

                ARRAY_DATA((Object*)arr, Object*)[index] = val;
            } else {
                int src_idx = getWrapperPrimTypeIndex(val);

                if(src_idx == PRIM_IDX_VOID)
                    goto illegal_arg;
                else {
                    int dst_idx = getPrimTypeIndex(elem_cb);
                    void *src_addr = INST_BASE((Object*)val, void);
                    int size = primTypeIndex2Size(dst_idx);
                    void *dst_addr = &ARRAY_DATA((Object*)arr, char)
                                                [index * size];

                    if(dst_idx < PRIM_IDX_INT) {
                        u4 value = *(u4*)src_addr;

                        if(src_idx != dst_idx) {
                            if(src_idx != PRIM_IDX_BYTE ||
                               dst_idx != PRIM_IDX_SHORT)
                                goto illegal_arg;
                            *(signed short*)dst_addr = (signed char)value;
                        } else {
                            if(src_idx < PRIM_IDX_CHAR)
                                *(char*)dst_addr = (char)value;
                            else
                                *(short*)dst_addr = (short)value;
                        }
                    } else
                        if(!widenPrimitiveValue(src_idx, dst_idx, src_addr,
                                                dst_addr, REF_SRC_FIELD |
                                                          REF_DST_FIELD))
                            goto illegal_arg;
                }
            }
        }
    }

    return;

illegal_arg:
    signalException(java_lang_IllegalArgumentException, NULL);
}


/* JVM_SetPrimitiveArrayElement */

void JVM_SetPrimitiveArrayElement(JNIEnv *env, jobject arr, jint index,
                                  jvalue val, unsigned char vCode) {

    TRACE("JVM_SetPrimitiveArrayElement(env=%p, arr=%p, index=%d, val=%lld,"
    	  "vCode=%d)", env, arr, index, val.j, vCode);

    if(arr == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else {
        ClassBlock *cb = CLASS_CB(((Object*)arr)->class);

        if(!IS_ARRAY(cb))
            signalException(java_lang_IllegalArgumentException, NULL);
        else {
            if(index > ARRAY_LEN((Object *)arr))
                signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
            else {
                ClassBlock *elem_cb = CLASS_CB(cb->element_class);
                if(!IS_PRIMITIVE(elem_cb) || cb->dim > 1)
                    signalException(java_lang_IllegalArgumentException, NULL);
                else {
                    int dst_idx = getPrimTypeIndex(elem_cb);
                    int size = primTypeIndex2Size(dst_idx);
                    int src_idx = typeNo2PrimTypeIndex(vCode);
                    void *addr = &ARRAY_DATA((Object*)arr, char)[index * size];

                    widenPrimitiveElement(src_idx, dst_idx, &val, addr);
                }
            }
        }
    }
}


/* JVM_NewArray */

jobject JVM_NewArray(JNIEnv *env, jclass eltClass, jint length) {
    TRACE("JVM_NewArray(env=%p, eltClass=%p, length=%d)",
          env, eltClass, length);

    if(eltClass == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return NULL;
    }

    if(length < 0) {
        signalException(java_lang_NegativeArraySizeException, NULL);
        return NULL;
    } else {
        ClassBlock *cb = CLASS_CB((Class*)eltClass);

        if(IS_PRIMITIVE(cb)) {
            static char type_map[] = {T_BOOLEAN, T_BYTE, T_CHAR, T_SHORT,
                                      T_INT, T_FLOAT, T_LONG, T_DOUBLE};
            int type = getPrimTypeIndex(cb);
        
            if(type == PRIM_IDX_VOID) {
                signalException(java_lang_IllegalArgumentException,
                                "cannot create a void array");
                return NULL;
            }

            return allocTypeArray(type_map[type - 1], length);
        }

        if(cb->dim == 255) {
            signalException(java_lang_IllegalArgumentException, NULL);
            return NULL;
        }

        return allocObjectArray(eltClass, length);
    }
}


/* JVM_NewMultiArray */

jobject JVM_NewMultiArray(JNIEnv *env, jclass eltClass, jintArray dim) {
    TRACE("JVM_NewMultiArray(env=%p, eltClass=%p, dim=%p)",
          env, eltClass, dim);

    if(eltClass == NULL || dim == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return NULL;
    } else {
        int i;
        char *array_name;
        intptr_t *dim_ptr;
        Object *array_class;
        int len = ARRAY_LEN((Class*)dim);
        int *dim_data = ARRAY_DATA((Class*)dim, int);
        ClassBlock *cb = CLASS_CB((Class*)eltClass);

        if(len == 0 || cb->dim + len > 255) {
            signalException(java_lang_IllegalArgumentException, NULL);
            return NULL;
        }

        if(IS_PRIMITIVE(cb)) {
            /* If the element is a primitive class, we
               need to convert from primitive class name
               to primitive char (e.g. "int" -> "I") */

            static char type_name[] = {'Z', 'B', 'C', 'S',
                                       'I', 'F', 'L', 'D'};
            int type = getPrimTypeIndex(cb);
        
            if(type == PRIM_IDX_VOID) {
                signalException(java_lang_IllegalArgumentException,
                                "cannot create a void array");
                return NULL;
            }

            /* Construct primitive array name, e.g. "[[I" */
            array_name = alloca(len + 2);
            array_name[len] = type_name[type - 1];
            array_name[len + 1] = '\0';
        } else {
            /* Construct object array name, e.g. "[[Ljava/lang/String;" */
            int name_len = strlen(cb->name);

            if(IS_ARRAY(cb)) {
                array_name = alloca(len + name_len + 1);
                memcpy(array_name + len, cb->name, name_len);
                array_name[len + name_len] = '\0';
            } else {
                array_name = alloca(len + name_len + 3);
                array_name[len] = 'L';
                memcpy(array_name + len + 1, cb->name, name_len);
                array_name[len + name_len + 1] = ';';
                array_name[len + name_len + 2] = '\0';
            }
        }

        /* Add a [ for each dimension */
        memset(array_name, '[', len);

        /* allocMultiArray expects dimensions to be an array
           of intptr_t.  If int is the same size as a pointer,
           we can use the int array data "as is" otherwise we
           need to convert */

        if(sizeof(int) != sizeof(intptr_t))
            dim_ptr = alloca(sizeof(intptr_t) * len);
        else
            dim_ptr = (intptr_t*)dim_data;

        for(i = 0; i < len; i++) {
            if(dim_data[i] < 0) {
                signalException(java_lang_NegativeArraySizeException, NULL);
                return NULL;
            }
            /* Trivially optimised out if dim_pntr == dim_data */
            dim_ptr[i] = dim_data[i];
        }

        array_class = findArrayClassFromClass(array_name, (Class*)eltClass);
        if(array_class == NULL)
            return NULL;

        return allocMultiArray(array_class, len, dim_ptr);
    }
}


/* JVM_InitializeSocketLibrary */

jint JVM_InitializeSocketLibrary() {
    TRACE("JVM_InitializeSocketLibrary()");

    /* Nothing to be done */
    return JNI_TRUE;
}


/* JVM_Socket */

jint JVM_Socket(jint domain, jint type, jint protocol) {
    TRACE("JVM_Socket(domain=%d, type=%d, protocol=%d)", domain,
          type, protocol);

    return socket(domain, type, protocol);
}


/* JVM_SocketClose */

jint JVM_SocketClose(jint fd) {
    TRACE("JVM_SocketClose(fd=%d)", fd);

    return close(fd);
}


/* JVM_SocketShutdown */

jint JVM_SocketShutdown(jint fd, jint howto) {
    TRACE("JVM_SocketShutdown(fd=%d, howto=%d)", fd, howto);

    /* Nothing to be done */
    return JNI_TRUE;
}


/* JVM_Recv */

jint JVM_Recv(jint fd, char *buf, jint nBytes, jint flags) {
    TRACE("JVM_Recv");

    return recv(fd, buf, nBytes, flags);
}


/* JVM_Send */

jint JVM_Send(jint fd, char *buf, jint nBytes, jint flags) {
    TRACE("JVM_Send(fd=%d, buf=%p, nBytes=%d, flags=%d", fd, buf,
          nBytes, flags);

    return send(fd, buf, nBytes, flags);
}


/* JVM_Timeout */

jint JVM_Timeout(int fd, long timeout) {
    UNIMPLEMENTED("JVM_Timeout");

    return 0;
}


/* JVM_Listen */

jint JVM_Listen(jint fd, jint count) {
    TRACE("JVM_Listen(fd=%d, count=%d)", fd, count);

    return listen(fd, count);
}


/* JVM_Connect */

jint JVM_Connect(jint fd, struct sockaddr *addr, jint addrlen) {
    TRACE("JVM_Connect(fd=%d, him=%p, len=%d)", fd, addr, addrlen);

    return connect(fd, addr, addrlen);
}


/* JVM_Bind */

jint JVM_Bind(jint fd, struct sockaddr *addr, jint addrlen) {
    TRACE("JVM_Bind");

    return bind(fd, addr, addrlen);
}


/* JVM_Accept */

jint JVM_Accept(jint fd, struct sockaddr *addr, jint *addrlen) {
    TRACE("JVM_Accept(fd=%d, him=%p, len=%p)", fd, addr, addrlen);

    return accept(fd, addr, addrlen);
}


/* JVM_RecvFrom */

jint JVM_RecvFrom(jint fd, char *buf, int nBytes, int flags,
                  struct sockaddr *from, int *fromlen) {
    UNIMPLEMENTED("JVM_RecvFrom");

    return 0;
}


/* JVM_GetSockName */

jint JVM_GetSockName(jint fd, struct sockaddr *addr, int *addrlen) {
    TRACE("JVM_GetSockName(fd=%d, him=%p, len=%p)", fd, addr, addrlen);

    return getsockname(fd, addr, addrlen);
}


/* JVM_SendTo */

jint JVM_SendTo(jint fd, char *buf, int len, int flags, struct sockaddr *to,
                int tolen) {
    UNIMPLEMENTED("JVM_SendTo");

    return 0;
}


/* JVM_SocketAvailable */

jint JVM_SocketAvailable(jint fd, jint *pbytes) {
    TRACE("JVM_SocketAvailable(fd=%d, pbytes=%p)", fd, pbytes);

    if(ioctl(fd, FIONREAD, pbytes) == -1)
        return JNI_FALSE;

    return JNI_TRUE;
}


/* JVM_GetSockOpt */

jint JVM_GetSockOpt(jint fd, int level, int optname, char *optval,
                    int *optlen) {

    TRACE("JVM_GetSockOpt(fd=%d, level=%d, optname=%d, optval=%s, optlen=%p)",
          fd, level, optname, optval, optlen);

    return getsockopt(fd, level, optname, optval, optlen);
}


/* JVM_SetSockOpt */

jint JVM_SetSockOpt(jint fd, int level, int optname, const char *optval,
                    int optlen) {

    TRACE("JVM_SetSockOpt(fd=%d, level=%d, optname=%d, optval=%s, optlen=%d)",
          fd, level, optname, optval, optlen);

    return setsockopt(fd, level, optname, optval, optlen);
}


/* JVM_GetHostName */

int JVM_GetHostName(char *name, int namelen) {
    TRACE("JVM_GetHostName(name=%s, namelen=%d)", name, namelen);

    return gethostname(name, namelen);
}


/* JVM_GetHostByAddr */

struct hostent *JVM_GetHostByAddr(const char *name, int len, int type) {
    UNIMPLEMENTED("JVM_GetHostByAddr");

    return NULL;
}


/* JVM_GetHostByName */

struct hostent *JVM_GetHostByName(char *name) {
    TRACE("JVM_GetHostByName");

    return gethostbyname(name);
}


/* JVM_GetProtoByName */

struct protoent *JVM_GetProtoByName(char *name) {
    UNIMPLEMENTED("JVM_GetProtoByName");

    return NULL;
}


/* JVM_LoadLibrary */

void* JVM_LoadLibrary(const char* name) {
    TRACE("JVM_LoadLibrary(name=%s)", name);

    return nativeLibOpen((char *)name);
}


/* JVM_UnloadLibrary */

void JVM_UnloadLibrary(void* handle) {
    TRACE("JVM_UnloadLibrary(handle=%p)", handle);

    return nativeLibClose(handle);
}


/* JVM_FindLibraryEntry */

void *JVM_FindLibraryEntry(void* handle, const char* name) {
    TRACE("JVM_FindLibraryEntry(handle=%p, name=%s)", handle, name);

    return nativeLibSym(handle, (char *)name);
}


/* JVM_IsNaN */

jboolean JVM_IsNaN(jdouble d) {
    TRACE("JVM_IsNaN(d=%f)", d);

    return isnan(d);
}


/* JVM_IsSupportedJNIVersion */

jboolean JVM_IsSupportedJNIVersion(jint version) {
    TRACE("JVM_IsSupportedJNIVersion(version=%d)", version);

    return isSupportedJNIVersion_1_1(version);
}


/* JVM_InternString */

jstring JVM_InternString(JNIEnv *env, jstring str) {
    TRACE("JVM_InternString(env=%p, str=%p)", env, str);

    return findInternedString(str);
}


/* JVM_RawMonitorCreate */

void *JVM_RawMonitorCreate(void) {
    VMLock *mon = sysMalloc(sizeof(VMLock));

    TRACE("JVM_RawMonitorCreate()");

    initVMLock(*mon);
    return mon;
}


/* JVM_RawMonitorDestroy */

void JVM_RawMonitorDestroy(void *mon) {
    TRACE("JVM_RawMonitorDestroy(mon=%p)", mon);

    sysFree(mon);
}


/* JVM_RawMonitorEnter */

jint JVM_RawMonitorEnter(void *mon) {
    Thread *self = threadSelf();

    TRACE("JVM_RawMonitorEnter(mon=%p)", mon);

    lockVMLock(*(VMLock*)mon, self);
    return 0;
}


/* JVM_RawMonitorExit */

void JVM_RawMonitorExit(void *mon) {
    Thread *self = threadSelf();

    TRACE("JVM_RawMonitorExit(mon=%p)", mon);

    unlockVMLock(*(VMLock*)mon, self);
}


/* JVM_SetPrimitiveFieldValues */

void JVM_SetPrimitiveFieldValues(JNIEnv *env, jclass cb, jobject obj,
                                 jlongArray fieldIDs, jcharArray typecodes,
                                 jbyteArray data) {
    UNIMPLEMENTED("JVM_SetPrimitiveFieldValues");
}


/* JVM_GetPrimitiveFieldValues */

void JVM_GetPrimitiveFieldValues(JNIEnv *env, jclass cb, jobject obj,
                                 jlongArray fieldIDs, jcharArray typecodes,
                                 jbyteArray data) {
    UNIMPLEMENTED("JVM_GetPrimitiveFieldValues");
}


/* JVM_AccessVMBooleanFlag */

jboolean JVM_AccessVMBooleanFlag(const char* name, jboolean* value,
                                 jboolean is_get) {
    UNIMPLEMENTED("JVM_AccessVMBooleanFlag");

    return 0;
}


/* JVM_AccessVMIntFlag */

jboolean JVM_AccessVMIntFlag(const char* name, jint* value, jboolean is_get) {
    UNIMPLEMENTED("JVM_AccessVMIntFlag");

    return 0;
}


/* JVM_VMBreakPoint */

void JVM_VMBreakPoint(JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("JVM_VMBreakPoint");
}


/* JVM_GetClassFields */

jobjectArray JVM_GetClassFields(JNIEnv *env, jclass cls, jint which) {
    UNUSED("JVM_GetClassFields");

    return NULL;
}


/* JVM_GetClassMethods */

jobjectArray JVM_GetClassMethods(JNIEnv *env, jclass cls, jint which) {
    UNUSED("JVM_GetClassMethods");

    return NULL;
}


/* JVM_GetClassConstructors */

jobjectArray JVM_GetClassConstructors(JNIEnv *env, jclass cls, jint which) {
    UNUSED("JVM_GetClassConstructors");

    return NULL;
}


/* JVM_GetClassField */

jobject JVM_GetClassField(JNIEnv *env, jclass cls, jstring name, jint which) {
    UNUSED("JVM_GetClassField");

    return NULL;
}


/* JVM_GetClassMethod */

jobject JVM_GetClassMethod(JNIEnv *env, jclass cls, jstring name,
                           jobjectArray types, jint which) {
    UNUSED("JVM_GetClassMethod");

    return NULL;
}


/* JVM_GetClassConstructor */

jobject JVM_GetClassConstructor(JNIEnv *env, jclass cls, jobjectArray types,
                                jint which) {
    UNUSED("JVM_GetClassConstructor");

    return NULL;
}


/* JVM_NewInstance */

jobject JVM_NewInstance(JNIEnv *env, jclass cls) {
    UNUSED("JVM_NewInstance");

    return NULL;
}


/* JVM_GetField */

jobject JVM_GetField(JNIEnv *env, jobject field, jobject obj) {
    UNUSED("JVM_GetField");

    return NULL;
}


/* JVM_GetPrimitiveField */

jvalue JVM_GetPrimitiveField(JNIEnv *env, jobject field, jobject obj,
                             unsigned char wCode) {
    jvalue jv;

    UNUSED("JVM_GetPrimitiveField");

    jv.l = NULL;
    return jv;
}


/* JVM_SetField */

void JVM_SetField(JNIEnv *env, jobject field, jobject obj, jobject val) {
    UNUSED("JVM_SetField");
}


/* JVM_SetPrimitiveField */

void JVM_SetPrimitiveField(JNIEnv *env, jobject field, jobject obj, jvalue v,
                           unsigned char vCode) {
    UNUSED("JVM_SetPrimitiveField");
}


/* JVM_InvokeMethod */

jobject JVM_InvokeMethod(JNIEnv *env, jobject method, jobject obj,
                         jobjectArray args0) {

    TRACE("JVM_InvokeMethod(env=%p, method=%p, obj=%p, args0=%p)", env,
          method, obj, args0);

    return invokeMethod(method, obj, args0);
}


/* JVM_NewInstanceFromConstructor */

jobject JVM_NewInstanceFromConstructor(JNIEnv *env, jobject con,
                                       jobjectArray args0) {

    TRACE("JVM_NewInstanceFromConstructor(env=%p, c=%p, args0=%p)", env,
          con, args0);

    return consNewInstance(con, args0);
}


/* JVM_SupportsCX8 */

jboolean JVM_SupportsCX8() {
    TRACE("JVM_SupportsCX8");

    return FALSE;
}


/* JVM_CX8Field */

jboolean JVM_CX8Field(JNIEnv *env, jobject obj, jfieldID fid, jlong oldVal,
                      jlong newVal) {
    UNIMPLEMENTED("JVM_CX8Field");

    return 0;
}


/* JVM_GetAllThreads */

jobjectArray JVM_GetAllThreads(JNIEnv *env, jclass dummy) {
    TRACE("JVM_GetAllThreads((env=%p)", env);

    return runningThreadObjects();
}


/* JVM_DumpThreads */

jobjectArray JVM_DumpThreads(JNIEnv *env, jclass threadClass,
                             jobjectArray array) {
    int i;
    Object *traces;
    Object *threads = array;
    int len = ARRAY_LEN(threads);
    Class *array_class = findArrayClass("[[Ljava/lang/StackTraceElement;");

    TRACE("JVM_DumpThreads((env=%p, threadClass=%p, threads=%p)",
        env, threadClass, threads);

    if(array_class == NULL)
        return NULL;

    if((traces = allocArray(array_class, len, sizeof(Object*))) == NULL)
        return NULL;

    for(i = 0; i < len; i++) {
        Object *jthread = ARRAY_DATA(threads, Object*)[i];
        Thread *thread = jThread2Thread(jthread);
        Object *trace;

        if(thread != NULL)
            trace = runningThreadStackTrace(thread, INT_MAX, NULL);
        else
            trace = convertTrace2Elements(NULL, 0);

        if(trace == NULL)
            return NULL;

        ARRAY_DATA(traces, Object*)[i] = trace;
    }

    return traces;
}


/* JVM_GetManagement */

void *JVM_GetManagement(jint version) {
    TRACE("JVM_GetManagement(version=%d)", version);

    return getJMMInterface(version);
}


/* JVM_InitAgentProperties */

jobject JVM_InitAgentProperties(JNIEnv *env, jobject properties) {
    UNIMPLEMENTED("JVM_InitAgentProperties");

    return NULL;
}


/* JVM_GetEnclosingMethodInfo */

jobjectArray JVM_GetEnclosingMethodInfo(JNIEnv *env, jclass ofClass) {
    TRACE("JVM_GetEnclosingMethodInfo(env=%p, ofClass=%p)", env, ofClass);

    if(ofClass == NULL)
        return NULL;

    return enclosingMethodInfo(ofClass);
}


/* JVM_GetThreadStateValues */

enum {
    JAVA_THREAD_STATE_NEW           = 0,
    JAVA_THREAD_STATE_RUNNABLE      = 1,
    JAVA_THREAD_STATE_BLOCKED       = 2,
    JAVA_THREAD_STATE_WAITING       = 3,
    JAVA_THREAD_STATE_TIMED_WAITING = 4,
    JAVA_THREAD_STATE_TERMINATED    = 5,
    JAVA_THREAD_STATE_COUNT         = 6
};

jintArray JVM_GetThreadStateValues(JNIEnv* env, jint javaThreadState) {
    TRACE("JVM_GetThreadStateValues(env=%p, javaThreadState=%d)",
                                    env, javaThreadState);

    int i, count = 0, states[4];
    Object *array;

    switch(javaThreadState) {
        case JAVA_THREAD_STATE_NEW:
            states[count++] = CREATING;
            break;

        case JAVA_THREAD_STATE_RUNNABLE:
            states[count++] = RUNNING;
            break;

        case JAVA_THREAD_STATE_BLOCKED:
            states[count++] = BLOCKED;
            break;

        case JAVA_THREAD_STATE_WAITING:
            states[count++] = PARKED;
            states[count++] = WAITING;
            states[count++] = OBJECT_WAIT;
            break;

        case JAVA_THREAD_STATE_TIMED_WAITING:
            states[count++] = SLEEPING;
            states[count++] = TIMED_PARKED;
            states[count++] = TIMED_WAITING;
            states[count++] = OBJECT_TIMED_WAIT;
            break;

        case JAVA_THREAD_STATE_TERMINATED:
            states[count++] = TERMINATED;
            break;
    }

    if(count == 0)
        return NULL;

    if((array = allocTypeArray(T_INT, count)) == NULL)
        return NULL;

    for(i = 0; i < count; i++)
        ARRAY_DATA(array, int)[i] = states[i];

    return array;
}


/* JVM_GetThreadStateNames */

jobjectArray JVM_GetThreadStateNames(JNIEnv* env, jint javaThreadState,
                                     jintArray values) {

    TRACE("JVM_GetThreadStateNames(env=%p, javaThreadState=%d, values=%p)",
          env, javaThreadState, values);

    Class *array_class = findArrayClass(SYMBOL(array_java_lang_String));
    char *state_names[4];
    int i, count = 0;
    Object *array;

    switch(javaThreadState) {
        case JAVA_THREAD_STATE_NEW:
            state_names[count++] = "NEW";
            break;

        case JAVA_THREAD_STATE_RUNNABLE:
            state_names[count++] = "RUNNABLE";
            break;

        case JAVA_THREAD_STATE_BLOCKED:
            state_names[count++] = "BLOCKED";
            break;

        case JAVA_THREAD_STATE_WAITING:
            state_names[count++] = "WAITING.PARKED";
            state_names[count++] = "WAITING.INTERNAL";
            state_names[count++] = "WAITING.OBJECT_WAIT";
            break;

        case JAVA_THREAD_STATE_TIMED_WAITING:
            state_names[count++] = "TIMED_WAITING.SLEEPING";
            state_names[count++] = "TIMED_WAITING.PARKED";
            state_names[count++] = "TIMED_WAITING.INTERNAL";
            state_names[count++] = "TIMED_WAITING.OBJECT_WAIT";
            break;

        case JAVA_THREAD_STATE_TERMINATED:
            state_names[count++] = "TERMINATED";
            break;
    }

    if(array_class == NULL || count == 0)
        return NULL;

    if((array = allocArray(array_class, count, sizeof(Object*))) == NULL)
        return NULL;

    for(i = 0; i < count; i++)
        if((ARRAY_DATA(array, Object*)[i] =
                  createString(state_names[i])) == NULL)
            return NULL;

    return array;
}


/* JVM_GetVersionInfo */

typedef struct {
    unsigned int jvm_version;
    unsigned int update_version : 8;
    unsigned int special_update_version : 8;
    unsigned int reserved1 : 16;
    unsigned int reserved2;
    unsigned int is_attachable : 1;
    unsigned int is_kernel_jvm : 1;
    unsigned int : 30;
    unsigned int : 32;
    unsigned int : 32;
} jvm_version_info;

void JVM_GetVersionInfo(JNIEnv *env, jvm_version_info *info, size_t info_size) {
    TRACE("JVM_GetVersionInfo(env=%p, info=%p, info_size=%zd)",
          env, info, info_size);

    memset(info, 0, info_size);
    info->jvm_version = ((VERSION_MAJOR & 0xff) << 24) |
                        ((VERSION_MINOR & 0xff) << 16) |
                         (VERSION_MICRO & 0xff);
}


/* JVM_GetTemporaryDirectory
 * Return the temporary directory that the VM uses for the attach
 * and perf data files.
 *
 * It is important that this directory is well-known and the
 * same for all VM instances. It cannot be affected by configuration
 * variables such as java.io.tmpdir.
 *
 * JamVM do not support the jvmstat framework thus this is left unimplemented.
 */

jstring JVM_GetTemporaryDirectory(JNIEnv *env) {
    UNIMPLEMENTED("JVM_GetTemporaryDirectory");

    return 0;
}


/* JVM_RegisterSignal */

extern void signalHandler(int sig);

void *JVM_RegisterSignal(jint sig, void *handler) {
    struct sigaction act, oldact;

    TRACE("JVM_RegisterSignal(sig=%d, handler=%p)", sig, handler);

    /* Signals that are used by the VM can't be used */
    if(sig == SIGUSR1 || sig == SIGQUIT)
        return (void*)-1;

    /* Convert special handler values into signal handlers.
       Note, SIG_DFL = 0, and SIG_IGN = 1, so the last two
       IFs can be optimised out.  The checks are included
       for safety in case this ever changes.  It's done as
       IFs to make it obvious to the compiler (at least on
       ARM the compiler misses it if it's done via x?y:z or
       a switch) */
    if(handler == (void*)2)
        handler = signalHandler;
    else if(handler == (void*)1)
        handler = SIG_IGN;
    else if(handler == (void*)0)
        handler = SIG_DFL;

    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    sigaction(sig, &act, &oldact);

    /* Convert the old signal handler value to the special
       handler values.  See note above */
    if(oldact.sa_handler == signalHandler)
        return (void*)2;
    if(oldact.sa_handler == SIG_IGN)
        return (void*)1;
    if(oldact.sa_handler == SIG_DFL)
        return (void*)0;
    return oldact.sa_handler;
}


/* JVM_RaiseSignal */

jboolean JVM_RaiseSignal(jint sig) {
    TRACE("JVM_RaiseSignal(sig=%s)", sig);

    raise(sig);
    return JNI_TRUE;
}


/* JVM_FindSignal */

typedef struct signal {
    char *name;
    int number;
} Signal;

static Signal signals[] = {
    {"HUP",  SIGHUP},
    {"INT",  SIGINT},
    {"TERM", SIGTERM},
    {NULL,   -1}
};

jint JVM_FindSignal(const char *name) {
    int i;

    TRACE("JVM_FindSignal(name=%s)", name);

    for(i = 0; signals[i].name != NULL && strcmp(signals[i].name, name); i++); 

    return signals[i].number;
}
