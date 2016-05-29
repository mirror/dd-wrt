/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011
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

#ifdef NO_JNI
#error to use classpath, Jam must be compiled with JNI!
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "jam.h"
#include "alloc.h"
#include "thread.h"
#include "lock.h"
#include "natives.h"
#include "symbol.h"
#include "excep.h"
#include "reflect.h"
#include "gnuclasspath.h"

static int pd_offset;

int classlibInitialiseNatives() {
    FieldBlock *pd = findField(java_lang_Class, SYMBOL(pd),
                               SYMBOL(sig_java_security_ProtectionDomain));

    if(pd == NULL) {
        jam_fprintf(stderr, "Expected \"pd\" field missing in "
                            "java.lang.Class\n");
        return FALSE;
    }

    pd_offset = pd->u.offset;
    return TRUE;
}

/* java.lang.VMObject */

uintptr_t *getClass(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *ob = (Object*)*ostack;
    *ostack++ = (uintptr_t)ob->class;
    return ostack;
}

uintptr_t *jamClone(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *ob = (Object*)*ostack;
    *ostack++ = (uintptr_t)cloneObject(ob);
    return ostack;
}

/* static method wait(Ljava/lang/Object;JI)V */
uintptr_t *jamWait(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *obj = (Object *)ostack[0];
    long long ms = *((long long *)&ostack[1]);
    int ns = ostack[3];

    objectWait(obj, ms, ns, TRUE);
    return ostack;
}

/* static method notify(Ljava/lang/Object;)V */
uintptr_t *notify(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *obj = (Object *)*ostack;
    objectNotify(obj);
    return ostack;
}

/* static method notifyAll(Ljava/lang/Object;)V */
uintptr_t *notifyAll(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *obj = (Object *)*ostack;
    objectNotifyAll(obj);
    return ostack;
}

/* java.lang.VMSystem */

/* arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V */
uintptr_t *arraycopy(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *src = (Object *)ostack[0];
    int start1 = ostack[1];
    Object *dest = (Object *)ostack[2];
    int start2 = ostack[3];
    int length = ostack[4];

    copyarray(src, start1, dest, start2, length);
    return ostack;
}

uintptr_t *identityHashCode(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *ob = (Object*)*ostack;
    uintptr_t addr = ob == NULL ? 0 : getObjectHashcode(ob);

    *ostack++ = addr & 0xffffffff;
    return ostack;
}

/* java.lang.VMRuntime */

uintptr_t *availableProcessors(Class *class, MethodBlock *mb,
                               uintptr_t *ostack) {

    *ostack++ = nativeAvailableProcessors();
    return ostack;
}

uintptr_t *freeMemory(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *(u8*)ostack = freeHeapMem();
    return ostack + 2;
}

uintptr_t *totalMemory(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *(u8*)ostack = totalHeapMem();
    return ostack + 2;
}

uintptr_t *maxMemory(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *(u8*)ostack = maxHeapMem();
    return ostack + 2;
}

uintptr_t *gc(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    gc1();
    return ostack;
}

uintptr_t *runFinalization(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    runFinalizers();
    return ostack;
}

uintptr_t *exitInternal(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int status = ostack[0];

    shutdownVM();
    jamvm_exit(status);

    /* keep compiler happy */
    return ostack;
}

uintptr_t *nativeLoad(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    char *name = String2Cstr((Object*)ostack[0]);
    Object *class_loader = (Object *)ostack[1];

    ostack[0] = resolveDll(name, class_loader);
    sysFree(name);

    return ostack + 1;
}

uintptr_t *mapLibraryName(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    char *name = String2Cstr((Object*)ostack[0]);
    char *lib = getDllName(name);
    sysFree(name);

    *ostack++ = (uintptr_t)Cstr2String(lib);
    sysFree(lib);

    return ostack;
}

uintptr_t *propertiesPreInit(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *properties = (Object *)*ostack;
    addDefaultProperties(properties);
    return ostack;
}

uintptr_t *propertiesPostInit(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *properties = (Object *)*ostack;
    addCommandLineProperties(properties);
    return ostack;
}

/* java.lang.VMClass */

uintptr_t *isInstance(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *clazz = (Class*)ostack[0];
    Object *ob = (Object*)ostack[1];

    *ostack++ = ob == NULL ? FALSE : (uintptr_t)isInstanceOf(clazz, ob->class);
    return ostack;
}

uintptr_t *isAssignableFrom(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *clazz = (Class*)ostack[0];
    Class *clazz2 = (Class*)ostack[1];

    if(clazz2 == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        *ostack++ = (uintptr_t)isInstanceOf(clazz, clazz2);

    return ostack;
}

uintptr_t *isInterface(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((Class*)ostack[0]);
    *ostack++ = IS_INTERFACE(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isPrimitive(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((Class*)ostack[0]);
    *ostack++ = IS_PRIMITIVE(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isArray(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((Class*)ostack[0]);
    *ostack++ = IS_ARRAY(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isMember(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((Class*)ostack[0]);
    *ostack++ = IS_MEMBER(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isLocal(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((Class*)ostack[0]);
    *ostack++ = IS_LOCAL(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *isAnonymous(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((Class*)ostack[0]);
    *ostack++ = IS_ANONYMOUS(cb) ? TRUE : FALSE;
    return ostack;
}

uintptr_t *getEnclosingClass0(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    Class *clazz = (Class*)ostack[0];
    *ostack++ = (uintptr_t) getEnclosingClass(clazz);
    return ostack;
}

uintptr_t *getEnclosingMethod0(Class *class, MethodBlock *mb,
                               uintptr_t *ostack) {

    Class *clazz = (Class*)ostack[0];
    *ostack++ = (uintptr_t) getEnclosingMethodObject(clazz);
    return ostack;
}

uintptr_t *getEnclosingConstructor(Class *class, MethodBlock *mb,
                                   uintptr_t *ostack) {

    Class *clazz = (Class*)ostack[0];
    *ostack++ = (uintptr_t) getEnclosingConstructorObject(clazz);
    return ostack;
}

uintptr_t *getClassSignature(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((Class*)ostack[0]);
    Object *string = cb->signature == NULL ? NULL : createString(cb->signature);

    *ostack++ = (uintptr_t)string;
    return ostack;
}

uintptr_t *getSuperclass(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    ClassBlock *cb = CLASS_CB((Class*)ostack[0]);
    *ostack++ = (uintptr_t) (IS_PRIMITIVE(cb) ||
                             IS_INTERFACE(cb) ? NULL : cb->super);
    return ostack;
}

uintptr_t *getComponentType(Class *clazz, MethodBlock *mb, uintptr_t *ostack) {
    Class *class = (Class*)ostack[0];
    ClassBlock *cb = CLASS_CB(class);
    Class *componentType = NULL;

    if(IS_ARRAY(cb))
        switch(cb->name[1]) {
            case '[':
                componentType = findArrayClassFromClass(&cb->name[1], class);
                break;

            default:
                componentType = cb->element_class;
                break;
        }
 
    *ostack++ = (uintptr_t) componentType;
    return ostack;
}

uintptr_t *getName(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    char *dot_name = slash2DotsDup(CLASS_CB((Class*)ostack[0])->name);
    Object *string = createString(dot_name);
    *ostack++ = (uintptr_t)string;
    sysFree(dot_name);
    return ostack;
}

uintptr_t *getDeclaredClasses(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    Class *clazz = (Class*)ostack[0];
    int public = ostack[1];
    *ostack++ = (uintptr_t) getClassClasses(clazz, public);
    return ostack;
}

uintptr_t *getDeclaringClass0(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    Class *clazz = (Class*)ostack[0];
    *ostack++ = (uintptr_t) getDeclaringClass(clazz);
    return ostack;
}

uintptr_t *getDeclaredConstructors(Class *class, MethodBlock *mb,
                                   uintptr_t *ostack) {

    Class *clazz = (Class*)ostack[0];
    int public = ostack[1];
    *ostack++ = (uintptr_t) getClassConstructors(clazz, public);
    return ostack;
}

uintptr_t *getDeclaredMethods(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    Class *clazz = (Class*)ostack[0];
    int public = ostack[1];
    *ostack++ = (uintptr_t) getClassMethods(clazz, public);
    return ostack;
}

uintptr_t *getDeclaredFields(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *clazz = (Class*)ostack[0];
    int public = ostack[1];
    *ostack++ = (uintptr_t) getClassFields(clazz, public);
    return ostack;
}

uintptr_t *getClassDeclaredAnnotations(Class *class, MethodBlock *mb,
                                       uintptr_t *ostack) {

    Class *clazz = (Class*)ostack[0];
    *ostack++ = (uintptr_t) getClassAnnotations(clazz);
    return ostack;
}

uintptr_t *getInterfaces(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *clazz = (Class*)ostack[0];
    *ostack++ = (uintptr_t) getClassInterfaces(clazz);
    return ostack;
}

uintptr_t *getClassLoader(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *clazz = (Class*)ostack[0];
    *ostack++ = (uintptr_t)CLASS_CB(clazz)->class_loader;
    return ostack;
}

uintptr_t *getClassModifiers(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *clazz = (Class*)ostack[0];
    int ignore_inner_attrs = ostack[1];
    ClassBlock *cb = CLASS_CB(clazz);

    if(!ignore_inner_attrs && cb->declaring_class)
        *ostack++ = (uintptr_t)cb->inner_access_flags;
    else
        *ostack++ = (uintptr_t)cb->access_flags;

    return ostack;
}

uintptr_t *forName0(uintptr_t *ostack, int resolve, Object *loader) {
    Object *string = (Object *)ostack[0];
    Class *class = NULL;
    int len, i = 0;
    char *cstr;
    
    if(string == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return ostack;
    }

    cstr = String2Utf8(string);
    len = strlen(cstr);

    /* Check the classname to see if it's valid.  It can be
       a 'normal' class or an array class, starting with a [ */

    if(cstr[0] == '[') {
        for(; cstr[i] == '['; i++);
        switch(cstr[i]) {
            case 'Z':
            case 'B':
            case 'C':
            case 'S':
            case 'I':
            case 'F':
            case 'J':
            case 'D':
                if(len-i != 1)
                    goto out;
                break;
            case 'L':
                if(cstr[i+1] == '[' || cstr[len-1] != ';')
                    goto out;
                break;
            default:
                goto out;
                break;
        }
    }

    /* Scan the classname and convert it to internal form
       by converting dots to slashes.  Reject classnames
       containing slashes, as this is an invalid character */

    for(; i < len; i++) {
        if(cstr[i] == '/')
            goto out;
        if(cstr[i] == '.')
            cstr[i] = '/';
    }

    class = findClassFromClassLoader(cstr, loader);

out:
    if(class == NULL) {
        Object *excep = exceptionOccurred();

        clearException();
        signalChainedException(java_lang_ClassNotFoundException, cstr, excep);
    } else
        if(resolve)
            initClass(class);

    sysFree(cstr);
    *ostack++ = (uintptr_t)class;
    return ostack;
}

uintptr_t *forName(Class *clazz, MethodBlock *mb, uintptr_t *ostack) {
    int init = ostack[1];
    Object *loader = (Object*)ostack[2];
    return forName0(ostack, init, loader);
}

uintptr_t *throwException(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *excep = (Object *)ostack[0];
    setException(excep);
    return ostack;
}

uintptr_t *hasClassInitializer(Class *class, MethodBlock *mb,
                               uintptr_t *ostack) {

    Class *clazz = (Class*)ostack[0];
    *ostack++ = findMethod(clazz, SYMBOL(class_init),
                                  SYMBOL(___V)) == NULL ? FALSE : TRUE;
    return ostack;
}

/* java.lang.VMThrowable */

uintptr_t *fillInStackTrace(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t) setStackTrace();
    return ostack;
}

uintptr_t *getStackTrace(Class *class, MethodBlock *m, uintptr_t *ostack) {
    Object *this = (Object *)*ostack;
    *ostack++ = (uintptr_t) convertStackTrace(this);
    return ostack;
}

/* gnu.classpath.VMStackWalker */

uintptr_t *getCallingClass(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t) getCallerClass(2);
    return ostack;
}

uintptr_t *getCallingClassLoader(Class *clazz, MethodBlock *mb,
                                 uintptr_t *ostack) {

    Class *class = getCallerClass(2);

    *ostack++ = (uintptr_t) (class ? CLASS_CB(class)->class_loader : NULL);
    return ostack;
}

uintptr_t *getClassContext0(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t) getClassContext();
    return ostack;
}

uintptr_t *firstNonNullClassLoader0(Class *class, MethodBlock *mb,
                                   uintptr_t *ostack) {

    *ostack++ = (uintptr_t) firstNonNullClassLoader();
    return ostack;
}

/* java.lang.VMClassLoader */

/* loadClass(Ljava/lang/String;I)Ljava/lang/Class; */
uintptr_t *loadClass(Class *clazz, MethodBlock *mb, uintptr_t *ostack) {
    int resolve = ostack[1];

    return forName0(ostack, resolve, NULL);
}

/* getPrimitiveClass(C)Ljava/lang/Class; */
uintptr_t *getPrimitiveClass(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    char prim_type = *ostack;

    *ostack++ = (uintptr_t)findPrimitiveClass(prim_type);
    return ostack;
}

uintptr_t *defineClass0(Class *clazz, MethodBlock *mb, uintptr_t *ostack) {
    Object *class_loader = (Object *)ostack[0];
    Object *string = (Object *)ostack[1];
    Object *array = (Object *)ostack[2];
    int offset = ostack[3];
    int data_len = ostack[4];
    uintptr_t pd = ostack[5];
    Class *class = NULL;

    if(array == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        if((offset < 0) || (data_len < 0) ||
                           ((offset + data_len) > ARRAY_LEN(array)))
            signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
        else {
            char *data = ARRAY_DATA(array, char);
            char *name = string ? dots2Slash(String2Utf8(string)) : NULL;

            class = defineClass(name, data, offset, data_len, class_loader);
            sysFree(name);

            if(class != NULL) {
                INST_DATA(class, uintptr_t, pd_offset) = pd;
                linkClass(class);
            }
        }

    *ostack++ = (uintptr_t) class;
    return ostack;
}

uintptr_t *findLoadedClass(Class *clazz, MethodBlock *mb, uintptr_t *ostack) {
    Object *class_loader = (Object *)ostack[0];
    Object *string = (Object *)ostack[1];
    Class *class;
    char *name;

    if(string == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return ostack;
    }

    name = dots2Slash(String2Utf8(string));
    class = findHashedClass(name, class_loader);
    sysFree(name);

    *ostack++ = (uintptr_t) class;
    return ostack;
}

uintptr_t *resolveClass0(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *clazz = (Class *)*ostack;

    if(clazz == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        initClass(clazz);

    return ostack;
}

uintptr_t *getBootClassPathSize(Class *class, MethodBlock *mb,
                                uintptr_t *ostack) {

    *ostack++ = bootClassPathSize();
    return ostack;
}

uintptr_t *getBootClassPathResource(Class *class, MethodBlock *mb,
                                    uintptr_t *ostack) {

    Object *string = (Object *) ostack[0];
    char *filename = String2Cstr(string);
    int index = ostack[1];

    *ostack++ = (uintptr_t) bootClassPathResource(filename, index);

    sysFree(filename);
    return ostack;
}

uintptr_t *getBootClassPackage(Class *class, MethodBlock *mb,
                               uintptr_t *ostack) {

    Object *string = (Object *) ostack[0];
    char *package_name = String2Cstr(string);

    *ostack++ = (uintptr_t) bootPackage(package_name);

    sysFree(package_name);
    return ostack;
}

uintptr_t *getBootClassPackages(Class *class, MethodBlock *mb,
                                uintptr_t *ostack) {

    *ostack++ = (uintptr_t) bootPackages();
    return ostack;
}

/* java.lang.reflect.VMConstructor */

uintptr_t *constructorConstruct0(Class *class, MethodBlock *mb2,
                                uintptr_t *ostack) {

    Object *this       = (Object*)ostack[0];
    Object *args_array = (Object*)ostack[1]; 

    Object *param_types = getVMConsParamTypes(this);
    int no_access_check = getVMConsAccessFlag(this);
    MethodBlock *mb     = getVMConsMethodBlock(this);

    *ostack++ = (uintptr_t) constructorConstruct(mb, args_array, param_types,
                                                 no_access_check, 2);
    return ostack;
}

uintptr_t *constructorModifiers(Class *class, MethodBlock *mb2,
                                uintptr_t *ostack) {

    MethodBlock *mb = getVMConsMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) mb->access_flags;
    return ostack;
}

uintptr_t *constructorParameterTypes(Class *class, MethodBlock *mb2,
                                uintptr_t *ostack) {

    MethodBlock *mb = getVMConsMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) getMethodParameterTypes(mb);
    return ostack;
}

uintptr_t *methodParameterTypes(Class *class, MethodBlock *mb2,
                                uintptr_t *ostack) {

    MethodBlock *mb = getVMMethodMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) getMethodParameterTypes(mb);
    return ostack;
}

uintptr_t *methodReturnType(Class *class, MethodBlock *mb2,
                                uintptr_t *ostack) {

    MethodBlock *mb = getVMMethodMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) getMethodReturnType(mb);
    return ostack;
}

uintptr_t *constructorExceptionTypes(Class *class, MethodBlock *mb2,
                                uintptr_t *ostack) {

    MethodBlock *mb = getVMConsMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) getMethodExceptionTypes(mb);
    return ostack;
}

uintptr_t *methodExceptionTypes(Class *class, MethodBlock *mb2,
                                uintptr_t *ostack) {

    MethodBlock *mb = getVMMethodMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) getMethodExceptionTypes(mb);
    return ostack;
}

uintptr_t *methodModifiers(Class *class, MethodBlock *mb2, uintptr_t *ostack) {
    MethodBlock *mb = getVMMethodMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) mb->access_flags;
    return ostack;
}

uintptr_t *methodName(Class *class, MethodBlock *mb2, uintptr_t *ostack) {
    MethodBlock *mb = getVMMethodMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) createString(mb->name);
    return ostack;
}

uintptr_t *methodSignature(Class *class, MethodBlock *mb2, uintptr_t *ostack) {
    MethodBlock *mb = getVMMethodMethodBlock((Object*)ostack[0]);
    Object *string = mb->signature == NULL ? NULL : createString(mb->signature);

    *ostack++ = (uintptr_t)string;
    return ostack;
}

uintptr_t *constructorSignature(Class *class, MethodBlock *mb2,
                                uintptr_t *ostack) {

    MethodBlock *mb = getVMConsMethodBlock((Object*)ostack[0]);
    Object *string = mb->signature == NULL ? NULL : createString(mb->signature);

    *ostack++ = (uintptr_t)string;
    return ostack;
}

uintptr_t *methodDefaultValue(Class *class, MethodBlock *mb2,
                              uintptr_t *ostack) {

    MethodBlock *mb = getVMMethodMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t)getMethodDefaultValue(mb);
    return ostack;
}

uintptr_t *methodDeclaredAnnotations(Class *class, MethodBlock *mb2,
                                     uintptr_t *ostack) {

    MethodBlock *mb = getVMMethodMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t)getMethodAnnotations(mb);
    return ostack;
}

uintptr_t *constructorDeclaredAnnotations(Class *class, MethodBlock *mb2,
                                          uintptr_t *ostack) {

    MethodBlock *mb = getVMConsMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t)getMethodAnnotations(mb);
    return ostack;
}

uintptr_t *methodParameterAnnotations(Class *class, MethodBlock *mb2,
                                      uintptr_t *ostack) {

    MethodBlock *mb = getVMMethodMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t)getMethodParameterAnnotations(mb);
    return ostack;
}

uintptr_t *constructorParameterAnnotations(Class *class, MethodBlock *mb2,
                                           uintptr_t *ostack) {

    MethodBlock *mb = getVMConsMethodBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t)getMethodParameterAnnotations(mb);
    return ostack;
}

uintptr_t *fieldModifiers(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    FieldBlock *fb = getVMFieldFieldBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) fb->access_flags;
    return ostack;
}

uintptr_t *fieldName(Class *class, MethodBlock *mb2, uintptr_t *ostack) {
    FieldBlock *fb = getVMFieldFieldBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) createString(fb->name);
    return ostack;
}

uintptr_t *fieldType(Class *class, MethodBlock *mb2, uintptr_t *ostack) {
    FieldBlock *fb = getVMFieldFieldBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t) getFieldType(fb);
    return ostack;
}

uintptr_t *fieldSignature(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    FieldBlock *fb = getVMFieldFieldBlock((Object*)ostack[0]);
    Object *string = fb->signature == NULL ? NULL : createString(fb->signature);

    *ostack++ = (uintptr_t)string;
    return ostack;
}

uintptr_t *fieldDeclaredAnnotations(Class *class, MethodBlock *mb,
                                    uintptr_t *ostack) {

    FieldBlock *fb = getVMFieldFieldBlock((Object*)ostack[0]);
    *ostack++ = (uintptr_t)getFieldAnnotations(fb);
    return ostack;
}

void *getPntr2Field(uintptr_t *ostack) {
    Object *this        = (Object*)ostack[0];
    Object *ob          = (Object*)ostack[1];
    FieldBlock *fb      = getVMFieldFieldBlock(this);
    int no_access_check = getVMFieldAccessFlag(this);

    if(!no_access_check) {
        Class *caller = getCallerClass(2);

        if(!checkClassAccess(fb->class, caller) ||
           !checkFieldAccess(fb, caller)) {

            signalException(java_lang_IllegalAccessException,
                            "field is not accessible");
            return NULL;
        }
    }

    if(fb->access_flags & ACC_STATIC) {

        /* Setting/getting a static field of a class is an
           active use.  Make sure it is initialised */
        if(initClass(fb->class) == NULL)
            return NULL;

        return fb->u.static_value.data;
    }

    if(!checkObject(ob, fb->class))
        return NULL;

    return &INST_DATA(ob, int, fb->u.offset);
}

uintptr_t *fieldGet(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *field_type = getVMFieldType((Object*)ostack[0]);

    /* If field is static, getPntr2Field also initialises the
       field's declaring class */
    void *field = getPntr2Field(ostack);

    if(field != NULL)
        *ostack++ = (uintptr_t) getReflectReturnObject(field_type, field,
                                                       REF_SRC_FIELD);

    return ostack;
}

uintptr_t *fieldGetPrimitive(int type_no, uintptr_t *ostack) {

    Class *field_type = getVMFieldType((Object*)ostack[0]);
    ClassBlock *type_cb = CLASS_CB(field_type);

    /* If field is static, getPntr2Field also initialises the
       field's declaring class */
    void *field = getPntr2Field(ostack);

    if(field != NULL) {
        if(IS_PRIMITIVE(type_cb)) {
            int size = widenPrimitiveValue(getPrimTypeIndex(type_cb),
                                           type_no, field, ostack,
                                           REF_SRC_FIELD | REF_DST_OSTACK);

            if(size > 0)
                return ostack + size;
        }

        /* Field is not primitive, or the source type cannot
           be widened to the destination type */
        signalException(java_lang_IllegalArgumentException,
                        "field type mismatch");
    }

    return ostack;
}

#define FIELD_GET_PRIMITIVE(name, type)                                       \
uintptr_t *fieldGet##name(Class *class, MethodBlock *mb, uintptr_t *ostack) { \
    return fieldGetPrimitive(PRIM_IDX_##type, ostack);                        \
}

FIELD_GET_PRIMITIVE(Boolean, BOOLEAN)
FIELD_GET_PRIMITIVE(Byte, BYTE)
FIELD_GET_PRIMITIVE(Char, CHAR)
FIELD_GET_PRIMITIVE(Short, SHORT)
FIELD_GET_PRIMITIVE(Int, INT)
FIELD_GET_PRIMITIVE(Float, FLOAT)
FIELD_GET_PRIMITIVE(Long, LONG)
FIELD_GET_PRIMITIVE(Double, DOUBLE)

uintptr_t *fieldSet(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *field_type = getVMFieldType((Object*)ostack[0]);
    Object *value = (Object*)ostack[2];

    /* If field is static, getPntr2Field also initialises the
       field's declaring class */
    void *field = getPntr2Field(ostack);

    if(field != NULL) {
        int size = unwrapAndWidenObject(field_type, value, field,
                                        REF_DST_FIELD);

        if(size == 0)
            signalException(java_lang_IllegalArgumentException,
                            "field type mismatch");
    }

    return ostack;
}

uintptr_t *fieldSetPrimitive(int type_no, uintptr_t *ostack) {

    Class *field_type = getVMFieldType((Object*)ostack[0]);
    ClassBlock *type_cb = CLASS_CB(field_type);

    /* If field is static, getPntr2Field also initialises the
       field's declaring class */
    void *field = getPntr2Field(ostack);

    if(field != NULL) {
        if(IS_PRIMITIVE(type_cb)) {

            int size = widenPrimitiveValue(type_no, getPrimTypeIndex(type_cb),
                                           &ostack[2], field,
                                           REF_SRC_OSTACK | REF_DST_FIELD);

            if(size > 0)
                return ostack;
        }

        /* Field is not primitive, or the source type cannot
           be widened to the destination type */
        signalException(java_lang_IllegalArgumentException,
                        "field type mismatch");
    }

    return ostack;
}

#define FIELD_SET_PRIMITIVE(name, type)                                       \
uintptr_t *fieldSet##name(Class *class, MethodBlock *mb, uintptr_t *ostack) { \
    return fieldSetPrimitive(PRIM_IDX_##type, ostack);                        \
}

FIELD_SET_PRIMITIVE(Boolean, BOOLEAN)
FIELD_SET_PRIMITIVE(Byte, BYTE)
FIELD_SET_PRIMITIVE(Char, CHAR)
FIELD_SET_PRIMITIVE(Short, SHORT)
FIELD_SET_PRIMITIVE(Int, INT)
FIELD_SET_PRIMITIVE(Float, FLOAT)
FIELD_SET_PRIMITIVE(Long, LONG)
FIELD_SET_PRIMITIVE(Double, DOUBLE)

/* java.lang.reflect.Method */

uintptr_t *methodInvoke0(Class *class, MethodBlock *mb2, uintptr_t *ostack) {
    Object *this       = (Object*)ostack[0];
    Object *ob         = (Object*)ostack[1];
    Object *args_array = (Object*)ostack[2]; 

    Class *ret_type     = getVMMethodReturnType(this);
    Object *param_types = getVMMethodParamTypes(this);
    int no_access_check = getVMMethodAccessFlag(this);
    MethodBlock *mb     = getVMMethodMethodBlock(this);

    *ostack++ = (uintptr_t) methodInvoke(ob, mb, args_array, ret_type,
                                         param_types, no_access_check, 2);
    return ostack;
}

/* java.lang.VMString */

/* static method - intern(Ljava/lang/String;)Ljava/lang/String; */
uintptr_t *intern(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *string = (Object*)ostack[0];
    ostack[0] = (uintptr_t)findInternedString(string);
    return ostack + 1;
}

/* java.lang.VMThread */

/* static method currentThread()Ljava/lang/Thread; */
uintptr_t *currentThread(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t)getExecEnv()->thread;
    return ostack;
}

/* static method create(Ljava/lang/Thread;J)V */
uintptr_t *create(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *this = (Object *)ostack[0];
    long long stack_size = *((long long*)&ostack[1]);
    createJavaThread(this, stack_size);
    return ostack;
}

/* static method sleep(JI)V */
uintptr_t *jamSleep(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long ms = *((long long *)&ostack[0]);
    int ns = ostack[2];
    Thread *thread = threadSelf();

    threadSleep(thread, ms, ns);
    return ostack;
}

/* instance method interrupt()V */
uintptr_t *interrupt(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *vmThread = (Object *)*ostack;
    Thread *thread = vmThread2Thread(vmThread);
    if(thread)
        threadInterrupt(thread);
    return ostack;
}

/* instance method isAlive()Z */
uintptr_t *isAlive(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *vmThread = (Object *)*ostack;
    Thread *thread = vmThread2Thread(vmThread);
    *ostack++ = thread ? threadIsAlive(thread) : FALSE;
    return ostack;
}

/* static method yield()V */
uintptr_t *yield(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Thread *thread = threadSelf();
    threadYield(thread);
    return ostack;
}

/* instance method isInterrupted()Z */
uintptr_t *isInterrupted(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *vmThread = (Object *)*ostack;
    Thread *thread = vmThread2Thread(vmThread);
    *ostack++ = thread ? threadIsInterrupted(thread) : FALSE;
    return ostack;
}

/* static method interrupted()Z */
uintptr_t *interrupted(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Thread *thread = threadSelf();
    *ostack++ = threadInterrupted(thread);
    return ostack;
}

/* instance method nativeSetPriority(I)V */
uintptr_t *nativeSetPriority(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    return ostack + 1;
}

/* instance method holdsLock(Ljava/lang/Object;)Z */
uintptr_t *holdsLock(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *ob = (Object *)ostack[0];
    if(ob == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        *ostack++ = objectLockedByCurrent(ob);
    return ostack;
}

/* instance method getState()Ljava/lang/String; */
uintptr_t *getState(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *vmThread = (Object *)*ostack;
    Thread *thread = vmThread2Thread(vmThread);
    char *state = thread ? getThreadStateString(thread) : "TERMINATED";

    *ostack++ = (uintptr_t)Cstr2String(state);
    return ostack;
}

/* java.security.VMAccessController */

/* instance method getStack()[[Ljava/lang/Object; */
uintptr_t *getStack(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *object_class = findArrayClass("[[Ljava/lang/Object;");
    Class *class_class = findArrayClass("[Ljava/lang/Class;");
    Class *string_class = findArrayClass("[Ljava/lang/String;");
    Object *stack, *names, *classes;
    Frame *frame;
    int depth;

    if(object_class == NULL || class_class == NULL || string_class == NULL)
      return ostack;

    frame = getExecEnv()->last_frame;
    depth = 0;

    do {
        for(; frame->mb != NULL; frame = frame->prev, depth++);
    } while((frame = frame->prev)->prev != NULL);

    stack = allocArray(object_class, 2, sizeof(Object*));
    classes = allocArray(class_class, depth, sizeof(Object*));
    names = allocArray(string_class, depth, sizeof(Object*));

    if(stack != NULL && names != NULL && classes != NULL) {
        Class **dcl = ARRAY_DATA(classes, Class*);
        Object **dnm = ARRAY_DATA(names, Object*);
        Object **stk = ARRAY_DATA(stack, Object*);

        frame = getExecEnv()->last_frame;

        do {
            for(; frame->mb != NULL; frame = frame->prev) {
                *dcl++ = frame->mb->class;
                *dnm++ = createString(frame->mb->name);
            }
        } while((frame = frame->prev)->prev != NULL);

        stk[0] = classes;
        stk[1] = names;
    }

    *ostack++ = (uintptr_t) stack;
    return ostack;
}

uintptr_t *getThreadCount(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = getThreadsCount();
    return ostack;
}

uintptr_t *getPeakThreadCount(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    *ostack++ = getPeakThreadsCount();
    return ostack;
}

uintptr_t *getTotalStartedThreadCount(Class *class, MethodBlock *mb,
                                      uintptr_t *ostack) {

    *(u8*)ostack = getTotalStartedThreadsCount();
    return ostack + 2;
}

uintptr_t *resetPeakThreadCount(Class *class, MethodBlock *mb,
                                uintptr_t *ostack) {
    resetPeakThreadsCount();
    return ostack;
}

uintptr_t *findMonitorDeadlockedThreads(Class *class, MethodBlock *mb,
                                        uintptr_t *ostack) {
    *ostack++ = (uintptr_t)NULL;
    return ostack;
}

uintptr_t *getThreadInfoForId(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    long long id = *((long long *)&ostack[0]);
    int max_depth = ostack[2];

    Thread *thread = findThreadById(id);
    Object *info = NULL;

    if(thread != NULL) {
        Class *helper_class = findSystemClass("jamvm/ThreadInfoHelper");
        Class *info_class = findSystemClass("java/lang/management/ThreadInfo");

        if(info_class != NULL && helper_class != NULL) {
            MethodBlock *helper = findMethod(helper_class,
                                             newUtf8("createThreadInfo"),
                                             newUtf8("(Ljava/lang/Thread;"
                                                     "Ljava/lang/Object;"
                                                     "Ljava/lang/Thread;)"
                                                     "[Ljava/lang/Object;"));
            MethodBlock *init = findMethod(info_class, SYMBOL(object_init),
                                 newUtf8("(JLjava/lang/String;"
                                         "Ljava/lang/Thread$State;"
                                         "JJLjava/lang/String;"
                                         "JLjava/lang/String;JJZZ"
                                         "[Ljava/lang/StackTraceElement;"
                                         "[Ljava/lang/management/MonitorInfo;"
                                         "[Ljava/lang/management/LockInfo;)V"));


            if(init != NULL && helper != NULL &&
                        (info = allocObject(info_class)) != NULL) {

                Monitor *mon = thread->blocked_mon;
                Object *lock = mon != NULL ? mon->obj : NULL;
                Thread *owner = lock != NULL ?
                                     objectLockedBy(lock) : NULL;
                Object *lock_owner = owner != NULL ?
                                       owner->ee->thread : NULL;
                long long owner_id = owner != NULL ?
                                       javaThreadId(owner) : -1;
                int in_native;
                Object *trace = runningThreadStackTrace(thread, max_depth,
                                                        &in_native); 

                if(trace != NULL) {
                    Object **helper_info;

                    helper_info = executeStaticMethod(helper_class,
                                          helper, thread->ee->thread, lock,
                                          lock_owner);

                    if(!exceptionOccurred()) {
                        helper_info = ARRAY_DATA(*helper_info, Object*);

                        executeMethod(info, init, id,
                                      helper_info[0], helper_info[1],
                                      thread->blocked_count, 0LL,
                                      helper_info[2], owner_id,
                                      helper_info[3], thread->waited_count,
                                      0LL, in_native, FALSE, trace,
                                      NULL, NULL);
                    }
                }
            }
        }
    }

    *ostack++ = (uintptr_t)info;
    return ostack;
}

uintptr_t *getMemoryPoolNames(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {
    Class *array_class = findArrayClass(SYMBOL(array_java_lang_String));

    if(array_class != NULL)
        *ostack++ = (uintptr_t)allocArray(array_class, 0, sizeof(Object*));
        
    return ostack;
}

uintptr_t *vmSupportsCS8(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = FALSE;
    return ostack;
}

/* jamvm.java.lang.VMClassLoaderData */

uintptr_t *nativeUnloadDll(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    unloaderUnloadDll((uintptr_t)*(long long*)&ostack[1]);
    return ostack;
}

VMMethod vm_object[] = {
    {"getClass",                    NULL, getClass},
    {"clone",                       NULL, jamClone},
    {"wait",                        NULL, jamWait},
    {"notify",                      NULL, notify},
    {"notifyAll",                   NULL, notifyAll},
    {NULL,                          NULL, NULL}
};

VMMethod vm_system[] = {
    {"arraycopy",                   NULL, arraycopy},
    {"identityHashCode",            NULL, identityHashCode},
    {NULL,                          NULL, NULL}
};

VMMethod vm_runtime[] = {
    {"availableProcessors",         NULL, availableProcessors},
    {"freeMemory",                  NULL, freeMemory},
    {"totalMemory",                 NULL, totalMemory},
    {"maxMemory",                   NULL, maxMemory},
    {"gc",                          NULL, gc},
    {"runFinalization",             NULL, runFinalization},
    {"exit",                        NULL, exitInternal},
    {"nativeLoad",                  NULL, nativeLoad},
    {"mapLibraryName",              NULL, mapLibraryName},
    {NULL,                          NULL, NULL}
};

VMMethod vm_class[] = {
    {"isInstance",                  NULL, isInstance},
    {"isAssignableFrom",            NULL, isAssignableFrom},
    {"isInterface",                 NULL, isInterface},
    {"isPrimitive",                 NULL, isPrimitive},
    {"isArray",                     NULL, isArray},
    {"isMemberClass",               NULL, isMember},
    {"isLocalClass",                NULL, isLocal},
    {"isAnonymousClass",            NULL, isAnonymous},
    {"getEnclosingClass",           NULL, getEnclosingClass0},
    {"getEnclosingMethod",          NULL, getEnclosingMethod0},
    {"getEnclosingConstructor",     NULL, getEnclosingConstructor},
    {"getClassSignature",           NULL, getClassSignature},
    {"getSuperclass",               NULL, getSuperclass},
    {"getComponentType",            NULL, getComponentType},
    {"getName",                     NULL, getName},
    {"getDeclaredClasses",          NULL, getDeclaredClasses},
    {"getDeclaringClass",           NULL, getDeclaringClass0},
    {"getDeclaredConstructors",     NULL, getDeclaredConstructors},
    {"getDeclaredMethods",          NULL, getDeclaredMethods},
    {"getDeclaredFields",           NULL, getDeclaredFields},
    {"getInterfaces",               NULL, getInterfaces},
    {"getClassLoader",              NULL, getClassLoader},
    {"getModifiers",                NULL, getClassModifiers},
    {"forName",                     NULL, forName},
    {"throwException",              NULL, throwException},
    {"hasClassInitializer",         NULL, hasClassInitializer},
    {"getDeclaredAnnotations",      NULL, getClassDeclaredAnnotations},
    {NULL,                          NULL, NULL}
};

VMMethod vm_string[] = {
    {"intern",                      NULL, intern},
    {NULL,                          NULL, NULL}
};

VMMethod vm_thread[] = {
    {"currentThread",               NULL, currentThread},
    {"create",                      NULL, create},
    {"sleep",                       NULL, jamSleep},
    {"interrupt",                   NULL, interrupt},
    {"isAlive",                     NULL, isAlive},
    {"yield",                       NULL, yield},
    {"isInterrupted",               NULL, isInterrupted},
    {"interrupted",                 NULL, interrupted},
    {"nativeSetPriority",           NULL, nativeSetPriority},
    {"holdsLock",                   NULL, holdsLock},
    {"getState",                    NULL, getState},
    {NULL,                          NULL, NULL}
};

VMMethod vm_throwable[] = {
    {"fillInStackTrace",            NULL, fillInStackTrace},
    {"getStackTrace",               NULL, getStackTrace},
    {NULL,                          NULL, NULL}
};

VMMethod vm_classloader[] = {
    {"loadClass",                   NULL, loadClass},
    {"getPrimitiveClass",           NULL, getPrimitiveClass},
    {"defineClass",                 NULL, defineClass0},
    {"findLoadedClass",             NULL, findLoadedClass},
    {"resolveClass",                NULL, resolveClass0},
    {"getBootClassPathSize",        NULL, getBootClassPathSize},
    {"getBootClassPathResource",    NULL, getBootClassPathResource},
    {"getBootPackages",             NULL, getBootClassPackages},
    {"getBootPackage",              NULL, getBootClassPackage},
    {NULL,                          NULL, NULL}
};

VMMethod vm_reflect_constructor[] = {
    {"construct",                   NULL, constructorConstruct0},
    {"getSignature",                NULL, constructorSignature},
    {"getModifiersInternal",        NULL, constructorModifiers},
    {"getExceptionTypes",           NULL, constructorExceptionTypes},
    {"getDeclaredAnnotations",      NULL, constructorDeclaredAnnotations},
    {"getParameterAnnotations",     NULL, constructorParameterAnnotations},
    {"getParameterTypesNative",     NULL, constructorParameterTypes},
    {NULL,                          NULL, NULL}
};

VMMethod vm_reflect_method[] = {
    {"getName",                     NULL, methodName},
    {"invoke",                      NULL, methodInvoke0},
    {"getSignature",                NULL, methodSignature},
    {"getModifiersInternal",        NULL, methodModifiers},
    {"getDefaultValue",             NULL, methodDefaultValue},
    {"getExceptionTypes",           NULL, methodExceptionTypes},
    {"getDeclaredAnnotations",      NULL, methodDeclaredAnnotations},
    {"getParameterAnnotations",     NULL, methodParameterAnnotations},
    {"getParameterTypesNative",     NULL, methodParameterTypes},
    {"getReturnTypeNative",         NULL, methodReturnType},
    {NULL,                          NULL, NULL}
};

VMMethod vm_reflect_field[] = {
    {"getName",                     NULL, fieldName},
    {"getTypeNative",               NULL, fieldType},
    {"getModifiersInternal",        NULL, fieldModifiers},
    {"getSignature",                NULL, fieldSignature},
    {"getDeclaredAnnotations",      NULL, fieldDeclaredAnnotations},
    {"set",                         NULL, fieldSet},
    {"setBoolean",                  NULL, fieldSetBoolean},
    {"setByte",                     NULL, fieldSetByte},
    {"setChar",                     NULL, fieldSetChar},
    {"setShort",                    NULL, fieldSetShort},
    {"setInt",                      NULL, fieldSetInt},
    {"setLong",                     NULL, fieldSetLong},
    {"setFloat",                    NULL, fieldSetFloat},
    {"setDouble",                   NULL, fieldSetDouble},
    {"get",                         NULL, fieldGet},
    {"getBoolean",                  NULL, fieldGetBoolean},
    {"getByte",                     NULL, fieldGetByte},
    {"getChar",                     NULL, fieldGetChar},
    {"getShort",                    NULL, fieldGetShort},
    {"getInt",                      NULL, fieldGetInt},
    {"getLong",                     NULL, fieldGetLong},
    {"getFloat",                    NULL, fieldGetFloat},
    {"getDouble",                   NULL, fieldGetDouble},
    {NULL,                          NULL, NULL}
};

VMMethod vm_system_properties[] = {
    {"preInit",                     NULL, propertiesPreInit},
    {"postInit",                    NULL, propertiesPostInit},
    {NULL,                          NULL, NULL}
};

VMMethod vm_stack_walker[] = {
    {"getClassContext",             NULL, getClassContext0},
    {"getCallingClass",             NULL, getCallingClass},
    {"getCallingClassLoader",       NULL, getCallingClassLoader},
    {"firstNonNullClassLoader",     NULL, firstNonNullClassLoader0},
    {NULL,                          NULL, NULL}
};

VMMethod sun_misc_unsafe[] = {
    {"objectFieldOffset",           NULL, objectFieldOffset},
    {"compareAndSwapInt",           NULL, compareAndSwapInt},
    {"compareAndSwapLong",          NULL, compareAndSwapLong},
    {"compareAndSwapObject",        NULL, compareAndSwapObject},
    {"putOrderedInt",               NULL, putOrderedInt},
    {"putOrderedLong",              NULL, putOrderedLong},
    {"putOrderedObject",            NULL, putOrderedObject},
    {"putIntVolatile",              NULL, putIntVolatile},
    {"getIntVolatile",              NULL, getIntVolatile},
    {"putLongVolatile",             NULL, putOrderedLong},
    {"putLong",                     NULL, putLong},
    {"getLongVolatile",             NULL, getLongVolatile},
    {"getLong",                     NULL, getLong},
    {"putObjectVolatile",           NULL, putObjectVolatile},
    {"putObject",                   NULL, putObject},
    {"getObjectVolatile",           NULL, getObjectVolatile},
    {"arrayBaseOffset",             NULL, arrayBaseOffset},
    {"arrayIndexScale",             NULL, arrayIndexScale},
    {"unpark",                      NULL, unpark},
    {"park",                        NULL, park},
    {NULL,                          NULL, NULL}
};

VMMethod vm_access_controller[] = {
    {"getStack",                    NULL, getStack},
    {NULL,                          NULL, NULL}
};

VMMethod vm_management_factory[] = {
    {"getMemoryPoolNames",          NULL, getMemoryPoolNames},
    {"getMemoryManagerNames",       NULL, getMemoryPoolNames},
    {"getGarbageCollectorNames",    NULL, getMemoryPoolNames},
    {NULL,                          NULL, NULL}
};

VMMethod vm_threadmx_bean_impl[] = {
    {"getThreadCount",              NULL, getThreadCount},
    {"getPeakThreadCount",          NULL, getPeakThreadCount},
    {"getTotalStartedThreadCount",  NULL, getTotalStartedThreadCount},
    {"resetPeakThreadCount",        NULL, resetPeakThreadCount},
    {"getThreadInfoForId",          NULL, getThreadInfoForId},
    {"findMonitorDeadlockedThreads",NULL, findMonitorDeadlockedThreads},
    {NULL,                          NULL, NULL}
};

VMMethod concurrent_atomic_long[] = {
    {"VMSupportsCS8",               NULL, vmSupportsCS8},
    {NULL,                          NULL, NULL}
};

VMMethod vm_class_loader_data[] = {
    {"nativeUnloadDll",             NULL, nativeUnloadDll},
    {NULL,                          NULL, NULL}
};

VMClass native_methods[] = {
    {"java/lang/VMClass",                           vm_class},
    {"java/lang/VMObject",                          vm_object},
    {"java/lang/VMThread",                          vm_thread},
    {"java/lang/VMSystem",                          vm_system},
    {"java/lang/VMString",                          vm_string},
    {"java/lang/VMRuntime",                         vm_runtime},
    {"java/lang/VMThrowable",                       vm_throwable},
    {"java/lang/VMClassLoader",                     vm_classloader},
    {"java/lang/reflect/VMField",                   vm_reflect_field},
    {"java/lang/reflect/VMMethod",                  vm_reflect_method},
    {"java/lang/reflect/VMConstructor",             vm_reflect_constructor},
    {"java/security/VMAccessController",            vm_access_controller},
    {"gnu/classpath/VMSystemProperties",            vm_system_properties},
    {"gnu/classpath/VMStackWalker",                 vm_stack_walker},
    {"java/lang/management/VMManagementFactory",    vm_management_factory},
    {"gnu/java/lang/management/VMThreadMXBeanImpl", vm_threadmx_bean_impl},
    {"sun/misc/Unsafe",                             sun_misc_unsafe},
    {"jamvm/java/lang/VMClassLoaderData$Unloader",  vm_class_loader_data},
    {"java/util/concurrent/atomic/AtomicLong",      concurrent_atomic_long},
    {NULL,                                          NULL}
};
