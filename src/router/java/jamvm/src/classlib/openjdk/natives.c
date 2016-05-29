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

#ifdef NO_JNI
#error to use classpath, Jam must be compiled with JNI!
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "jam.h"
#include "excep.h"
#include "symbol.h"
#include "reflect.h"
#include "natives.h"
#include "openjdk.h"
#include "trace.h"

int classlibInitialiseNatives() {
    Class *field_accessor;
    FieldBlock *base_fb = NULL;
    char *dll_path = getBootDllPath();
    char *dll_name = getDllName("java");
    char path[strlen(dll_path) + strlen(dll_name) + 2];

    strcat(strcat(strcpy(path, dll_path), "/"), dll_name);
    sysFree(dll_name);

    if(!resolveDll(path, NULL)) {
        jam_fprintf(stderr, "Error initialising natives: couldn't open "
                            "libjava.so: use -verbose:jni for more "
                            "information\n");
        return FALSE;
    }

    field_accessor = findSystemClass0(SYMBOL(
                         sun_reflect_UnsafeStaticFieldAccessorImpl));

    if(field_accessor != NULL)
        base_fb = findField(field_accessor, SYMBOL(base),
                                            SYMBOL(sig_java_lang_Object));

    if(base_fb == NULL) {
        jam_fprintf(stderr, "Error initialising natives: %s "
                            "missing or malformed\n",
                            SYMBOL(sun_reflect_UnsafeStaticFieldAccessorImpl));
        return FALSE;
    }

    hideFieldFromGC(base_fb);

    return initialiseJVMInterface();
}

uintptr_t *unsafeRegisterNatives(Class *class, MethodBlock *mb,
                                 uintptr_t *ostack) {
    return ostack;
}

uintptr_t *staticFieldOffset(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *(int64_t *)ostack = 0;
    return ostack + 2;
}

uintptr_t *staticFieldBase(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    FieldBlock *fb = fbFromReflectObject((Object*)ostack[1]);

    *ostack++ = (uintptr_t)&fb->u.static_value;
    return ostack;
}

uintptr_t *getAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    uintptr_t *pntr = (uintptr_t *)(uintptr_t)address;

    *(int64_t *)ostack = (int64_t)*pntr;
    return ostack + 2;
}

uintptr_t *putAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    int64_t value = *(int64_t *)&ostack[3];
    uintptr_t *pntr = (uintptr_t *)(uintptr_t)address;

    *pntr = (uintptr_t)value;
    return ostack;
}

uintptr_t *putLongAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    int64_t value   = *(int64_t *)&ostack[3];
    int64_t *pntr   =  (int64_t *)(uintptr_t)address;

    *pntr = value;
    return ostack;
}

uintptr_t *putIntAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    int32_t *pntr   =  (int32_t *)(uintptr_t)address;

    *pntr = ostack[3];
    return ostack;
}

uintptr_t *putShortAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    int16_t *pntr   =  (int16_t *)(uintptr_t)address;

    *pntr = ostack[3];
    return ostack;
}

uintptr_t *putByteAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    int8_t  *pntr   =  (int8_t  *)(uintptr_t)address;

    *pntr = ostack[3];
    return ostack;
}

uintptr_t *getLongAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    int64_t *pntr   =  (int64_t *)(uintptr_t)address;

    *(int64_t *)ostack = *pntr;
    return ostack + 2;
}

uintptr_t *getIntAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    int32_t *pntr   =  (int32_t *)(uintptr_t)address;

    *ostack++ = *pntr;
    return ostack;
}

uintptr_t *getShortAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    int16_t *pntr   =  (int16_t *)(uintptr_t)address;

    *ostack++ = *pntr;
    return ostack;
}

uintptr_t *getCharAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t  address = *(int64_t  *)&ostack[1];
    uint16_t *pntr   =  (uint16_t *)(uintptr_t)address;

    *ostack++ = *pntr;
    return ostack;
}

uintptr_t *getByteAddress(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];
    int8_t  *pntr   =  (int8_t  *)(uintptr_t)address;

    *ostack++ = *pntr;
    return ostack;
}

uintptr_t *putInt(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t   offset = *(int64_t  *)&ostack[2];
    uint32_t  *pntr  =  (uint32_t *)((char *)ostack[1] + offset);
    uintptr_t value  = ostack[4];

    *pntr = value;
    return ostack;
}

uintptr_t *getObject(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t   offset = *(int64_t   *)&ostack[2];
    uintptr_t *pntr  =  (uintptr_t *)((char *)ostack[1] + offset);

    *ostack++ = *pntr;
    return ostack;
}

uintptr_t *getInt(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t  offset = *(int64_t  *)&ostack[2];
    uint32_t *pntr  =  (uint32_t *)((char *)ostack[1] + offset);

    *ostack++ = *pntr;
    return ostack;
}

uintptr_t *allocateMemory(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t bytes = *(int64_t *)&ostack[1];
    size_t size = bytes;
    void *pntr;

    if(bytes < 0 || bytes != size) {
        signalException(java_lang_IllegalArgumentException, NULL);
        return ostack;
    }

    if((pntr = malloc(size)) == NULL) {
        signalException(java_lang_OutOfMemoryError, NULL);
        return ostack;
    }
   
    *(int64_t *)ostack = (int64_t)(uintptr_t)pntr;
    return ostack + 2;
}

uintptr_t *freeMemory(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int64_t address = *(int64_t *)&ostack[1];

    free((void *)(uintptr_t)address);
    return ostack;
}

uintptr_t *setMemoryOpenJDK6(Class *class, MethodBlock *mb,
                             uintptr_t *ostack) {

    int64_t address = *(int64_t *)&ostack[1];
    int64_t bytes   = *(int64_t *)&ostack[3];
    int32_t value   = ostack[5];
    void    *pntr   = (void *)(uintptr_t)address;
    size_t  size    = bytes;

    if(bytes < 0 || bytes != size) {
        signalException(java_lang_IllegalArgumentException, NULL);
        return ostack;
    }

    memset(pntr, value, size);
    return ostack;
}

uintptr_t *copyMemoryOpenJDK6(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    int64_t src_addr  = *(int64_t *)&ostack[1];
    int64_t dst_addr  = *(int64_t *)&ostack[3];
    int64_t bytes     = *(int64_t *)&ostack[5];
    void    *src_pntr = (void *)(uintptr_t)src_addr;
    void    *dst_pntr = (void *)(uintptr_t)dst_addr;
    size_t  size      = bytes;

    if(bytes < 0 || bytes != size) {
        signalException(java_lang_IllegalArgumentException, NULL);
        return ostack;
    }

    memcpy(dst_pntr, src_pntr, size);
    return ostack;
}

uintptr_t *setMemoryOpenJDK7(Class *class, MethodBlock *mb,
                             uintptr_t *ostack) {

    Object *base    = (Object *)ostack[1];
    int64_t offset  = *(int64_t *)&ostack[2];
    int64_t bytes   = *(int64_t *)&ostack[4];
    int32_t value   = ostack[6];
    void   *pntr    = (char *)base + offset;
    size_t  size    = bytes;

    if(bytes < 0 || bytes != size) {
        signalException(java_lang_IllegalArgumentException, NULL);
        return ostack;
    }

    memset(pntr, value, size);
    return ostack;
}

uintptr_t *copyMemoryOpenJDK7(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    Object *src_base  = (Object *)ostack[1];
    int64_t src_ofst  = *(int64_t *)&ostack[2];
    Object *dst_base  = (Object *)ostack[4];
    int64_t dst_ofst  = *(int64_t *)&ostack[5];
    int64_t bytes     = *(int64_t *)&ostack[7];
    void   *src_pntr  = (char *)src_base + src_ofst;
    void   *dst_pntr  = (char *)dst_base + dst_ofst;
    size_t  size      = bytes;

    if(bytes < 0 || bytes != size) {
        signalException(java_lang_IllegalArgumentException, NULL);
        return ostack;
    }

    memcpy(dst_pntr, src_pntr, size);
    return ostack;
}

uintptr_t *ensureClassInitialized(Class *class, MethodBlock *mb,
                                  uintptr_t *ostack) {
    initClass((Class*)ostack[1]);
    return ostack;
}

uintptr_t *unsafeDefineClass(Class *clazz, MethodBlock *mb,
                                   uintptr_t *ostack) {
    Object *string   = (Object *)ostack[1];
    Object *array    = (Object *)ostack[2];
    int32_t offset   = ostack[3];
    int32_t data_len = ostack[4];
    Class *class     = NULL;

    TRACE("unsafeDefineClass\n");

    if(array == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        if((offset < 0) || (data_len < 0) ||
                           ((offset + data_len) > ARRAY_LEN(array)))
            signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
        else {
            char *data = ARRAY_DATA(array, char);
            char *name = string ? dots2Slash(String2Utf8(string)) : NULL;

            class = defineClass(name, data, offset, data_len, NULL);
            sysFree(name);

            if(class != NULL)
                linkClass(class);
        }

    *ostack++ = (uintptr_t) class;
    return ostack;
}

uintptr_t *defineClassWithLoaderPD(Class *clazz, MethodBlock *mb,
                                   uintptr_t *ostack) {
    Object *string   = (Object *)ostack[1];
    Object *array    = (Object *)ostack[2];
    int32_t offset   = ostack[3];
    int32_t data_len = ostack[4];
    Object *loader   = (Object *)ostack[5];
    Object *pd       = (Object *)ostack[6];
    Class *class     = NULL;

    if(array == NULL)
        signalException(java_lang_NullPointerException, NULL);
    else
        if((offset < 0) || (data_len < 0) ||
                           ((offset + data_len) > ARRAY_LEN(array)))
            signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
        else {
            char *data = ARRAY_DATA(array, char);
            char *name = string ? dots2Slash(String2Utf8(string)) : NULL;

            class = defineClass(name, data, offset, data_len, loader);
            sysFree(name);

            if(class != NULL) {
                CLASS_CB(class)->protection_domain = pd;
                linkClass(class);
            }
        }

    *ostack++ = (uintptr_t) class;
    return ostack;
}

uintptr_t *throwException(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    setException((Object *)ostack[1]);
    return ostack;
}

uintptr_t *allocateInstance(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *obj = allocObjectClassCheck((Class *)ostack[1]);
    *ostack++ = (uintptr_t)obj;
    return ostack;
}

uintptr_t *addressSize(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = sizeof(void*);
    return ostack;
}

uintptr_t *pageSize(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = getpagesize();
    return ostack;
}

uintptr_t *defineAnonymousClass(Class *class, MethodBlock *mb, 
	                        uintptr_t *ostack) {
    Class *host_class = (Class *)ostack[1];
    Object *data = (Object *)ostack[2];
    Object *cp_patches = (Object *)ostack[3];
    ClassBlock *host_cb = CLASS_CB(host_class);

    TRACE("defineAnonymousClass\n");

    class = parseClass(NULL, ARRAY_DATA(data, char), 0, ARRAY_LEN(data),
                       host_cb->class_loader);

    if(class != NULL) {
        int cp_patches_len = cp_patches == NULL ? 0 : ARRAY_LEN(cp_patches);
        ClassBlock *cb = CLASS_CB(class);
        ConstantPool *cp = &(cb->constant_pool);
        int i;

        for(i = 0; i < cp_patches_len; i++) {
            Object *obj = ARRAY_DATA(cp_patches, Object*)[i];
            if(obj != NULL) {
                int type = CP_TYPE(cp, i);
            
                switch(type) {
                    case CONSTANT_String:
                        CP_INFO(cp, i) = (uintptr_t)obj;
                        CP_TYPE(cp, i) = CONSTANT_ResolvedString;
                        break;

                    default:
                        signalException(java_lang_InternalError,
                                        "defineAnonymousClass: "
                                        "unimplemented patch type");
                        goto out;
                }
            }
        }

        cb->protection_domain = host_cb->protection_domain;
        cb->host_class = host_class;

        linkClass(class);
    }

out:
    *ostack++ = (uintptr_t) class;
    return ostack;
}

uintptr_t *shouldBeInitialized(Class *clazz, MethodBlock *mb,
	                       uintptr_t *ostack) {
    Class *class = (Class *)ostack[1];

    *ostack++ = CLASS_CB(class)->state < CLASS_INITED;
    return ostack;
}

uintptr_t *fullFence(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    MBARRIER();
    return ostack;
}

uintptr_t *loadFence(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    RMBARRIER();
    return ostack;
}

uintptr_t *storeFence(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    WMBARRIER();
    return ostack;
}

VMMethod sun_misc_unsafe[] = {
    {"registerNatives",        "()V", unsafeRegisterNatives},
    {"objectFieldOffset",      "(Ljava/lang/reflect/Field;)J",
                               objectFieldOffset},
    {"staticFieldOffset",      "(Ljava/lang/reflect/Field;)J",
                               staticFieldOffset},
    {"staticFieldBase",        "(Ljava/lang/reflect/Field;)Ljava/lang/Object;",
                               staticFieldBase},
    {"getAddress",             "(J)J", getAddress},
    {"putAddress",             "(JJ)V", putAddress},
    {"putLong",                "(JJ)V", putLongAddress},
    {"putDouble",              "(JD)V", putLongAddress},
    {"putInt",                 "(JI)V", putIntAddress},
    {"putFloat",               "(JF)V", putIntAddress},
    {"putShort",               "(JS)V", putShortAddress},
    {"putChar",                "(JC)V", putShortAddress},
    {"putByte",                "(JB)V", putByteAddress},
    {"getLong",                "(J)J", getLongAddress},
    {"getDouble",              "(J)D", getLongAddress},
    {"getInt",                 "(J)I", getIntAddress},
    {"getFloat",               "(J)F", getIntAddress},
    {"getShort",               "(J)S", getShortAddress},
    {"getChar",                "(J)C", getCharAddress},
    {"getByte",                "(J)B", getByteAddress},
    {"compareAndSwapInt",      "(Ljava/lang/Object;JII)Z", compareAndSwapInt},
    {"compareAndSwapLong",     "(Ljava/lang/Object;JJJ)Z", compareAndSwapLong},
    {"compareAndSwapObject",   "(Ljava/lang/Object;JLjava/lang/Object;"
                               "Ljava/lang/Object;)Z", compareAndSwapObject},
    {"putOrderedInt",          "(Ljava/lang/Object;JI)V", putOrderedInt},
    {"putOrderedLong",         "(Ljava/lang/Object;JJ)V", putOrderedLong},
    {"putOrderedObject",       "(Ljava/lang/Object;JLjava/lang/Object;)V",
                               putOrderedObject},
    {"putIntVolatile",         "(Ljava/lang/Object;JI)V", putIntVolatile},
    {"putByteVolatile",        "(Ljava/lang/Object;JB)V", putIntVolatile},
    {"putCharVolatile",        "(Ljava/lang/Object;JC)V", putIntVolatile},
    {"putShortVolatile",       "(Ljava/lang/Object;JS)V", putIntVolatile},
    {"putBooleanVolatile",     "(Ljava/lang/Object;JZ)V", putIntVolatile},
    {"putLongVolatile",        "(Ljava/lang/Object;JJ)V", putOrderedLong},
    {"putObjectVolatile",      "(Ljava/lang/Object;JLjava/lang/Object;)V",
                               putObjectVolatile},
    {"getIntVolatile",         "(Ljava/lang/Object;J)I", getIntVolatile},
    {"getByteVolatile",        "(Ljava/lang/Object;J)B", getIntVolatile},
    {"getCharVolatile",        "(Ljava/lang/Object;J)C", getIntVolatile},
    {"getShortVolatile",       "(Ljava/lang/Object;J)S", getIntVolatile},
    {"getFloatVolatile",       "(Ljava/lang/Object;J)F", getIntVolatile},
    {"getBooleanVolatile",     "(Ljava/lang/Object;J)Z", getIntVolatile},
    {"getLongVolatile",        "(Ljava/lang/Object;J)J", getLongVolatile},
    {"getDoubleVolatile",      "(Ljava/lang/Object;J)D", getLongVolatile},
    {"getObjectVolatile",      "(Ljava/lang/Object;J)Ljava/lang/Object;",
                               getObjectVolatile},
    {"putInt",                 "(Ljava/lang/Object;JI)V", putInt},
    {"putByte",                "(Ljava/lang/Object;JB)V", putInt},
    {"putChar",                "(Ljava/lang/Object;JC)V", putInt},
    {"putShort",               "(Ljava/lang/Object;JS)V", putInt},
    {"putFloat",               "(Ljava/lang/Object;JF)V", putInt},
    {"putBoolean",             "(Ljava/lang/Object;JZ)V", putInt},
    {"putLong",                "(Ljava/lang/Object;JJ)V", putLong},
    {"putDouble",              "(Ljava/lang/Object;JD)V", putLong},
    {"putObject",              "(Ljava/lang/Object;JLjava/lang/Object;)V",
                               putObject},
    {"getInt",                 "(Ljava/lang/Object;J)I", getInt},
    {"getByte",                "(Ljava/lang/Object;J)B", getInt},
    {"getChar",                "(Ljava/lang/Object;J)C", getInt},
    {"getShort",               "(Ljava/lang/Object;J)S", getInt},
    {"getFloat",               "(Ljava/lang/Object;J)F", getInt},
    {"getBoolean",             "(Ljava/lang/Object;J)Z", getInt},
    {"getLong",                "(Ljava/lang/Object;J)J", getLong},
    {"getDouble",              "(Ljava/lang/Object;J)D", getLong},
    {"getObject",              "(Ljava/lang/Object;J)Ljava/lang/Object;",
                               getObject},
    {"setMemory",              "(JJB)V", setMemoryOpenJDK6},
    {"copyMemory",             "(JJJ)V", copyMemoryOpenJDK6},
    {"setMemory",              "(Ljava/lang/Object;JJB)V", setMemoryOpenJDK7},
    {"copyMemory",             "(Ljava/lang/Object;JLjava/lang/Object;JJ)V",
                               copyMemoryOpenJDK7},
    {"arrayBaseOffset",        NULL, arrayBaseOffset},
    {"arrayIndexScale",        NULL, arrayIndexScale},
    {"unpark",                 NULL, unpark},
    {"park",                   NULL, park},
    {"allocateMemory",         NULL, allocateMemory},
    {"freeMemory",             NULL, freeMemory},
    {"ensureClassInitialized", NULL, ensureClassInitialized},
    {"defineClass",            "(Ljava/lang/String;[BII"
                                "Ljava/lang/ClassLoader;"
                                "Ljava/security/ProtectionDomain;"
                               ")Ljava/lang/Class;",
                               defineClassWithLoaderPD},
    {"defineClass",            "(Ljava/lang/String;[BII"
                               ")Ljava/lang/Class;",
                               unsafeDefineClass},
    {"throwException",         NULL, throwException},
    {"allocateInstance",       NULL, allocateInstance},
    {"addressSize",            NULL, addressSize},
    {"pageSize",               NULL, pageSize},
    {"defineAnonymousClass",   NULL, defineAnonymousClass},
    {"shouldBeInitialized",    NULL, shouldBeInitialized},
    {"fullFence",              "()V", fullFence},
    {"loadFence",              "()V", loadFence},
    {"storeFence",             "()V", storeFence},
    {NULL,                     NULL, NULL}
};

#ifdef JSR292
uintptr_t *MHN_registerNatives(Class *class, MethodBlock *mb,
                               uintptr_t *ostack) {

    TRACE("MNH_registerNatives\n");

    initialiseMethodHandles();
    return ostack;
}

// (I)I
uintptr_t *MHN_getConstant(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    TRACE("MHN_getConstant: type %d\n", (int)*ostack);

    *ostack++ = 0;
    return ostack;
}

// (Ljava/lang/invoke/MemberName;Ljava/lang/Object;)V
uintptr_t *MHN_initMemberName(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    Object *mname = (Object*)ostack[0];
    Object *target = (Object*)ostack[1];

    TRACE("MNH_initMemberName\n");

    initMemberName(mname, target);
    return ostack;
}

// (Ljava/lang/invoke/MemberName;)V
uintptr_t *MHN_expandMemberName(Class *class, MethodBlock *mb,
                                uintptr_t *ostack) {

    Object *mname = (Object*)ostack[0];

    TRACE("MHN_expandMemberName\n");

    expandMemberName(mname);
    return ostack;
}

uintptr_t *MHN_objectFieldOffset(Class *class, MethodBlock *mb,
                                 uintptr_t *ostack) {

    Object *mname = (Object*)ostack[0];

    TRACE("MHN_objectFieldOffset\n");

    *(long long *)ostack = memNameFieldOffset(mname);
    return ostack + 2;
}

uintptr_t *MHN_staticFieldOffset(Class *class, MethodBlock *mb,
                                 uintptr_t *ostack) {

    Object *mname = (Object*)ostack[0];

    TRACE("MHN_staticFieldOffset\n");

    *(long long *)ostack = memNameStaticFieldOffset(mname);
    return ostack + 2;
}

uintptr_t *MHN_staticFieldBase(Class *class, MethodBlock *mb,
	                       uintptr_t *ostack) {

    TRACE("MHN_staticFieldBase\n");

    *ostack++ = 0;
    return ostack;
}

uintptr_t *MHN_getMemberVMInfo(Class *class, MethodBlock *mb,
                               uintptr_t *ostack) {

    TRACE("MHN_getMemberVMInfo\n");

    signalException(java_lang_InternalError,
                    "MHN_getMemberVMInfo: unimplemented");
    return ostack;
}

uintptr_t *MHN_setCallSiteTargetNormal(Class *class, MethodBlock *mb, 
	                               uintptr_t *ostack) {

    Object *call_site = (Object *)ostack[0];
    Object *target = (Object *)ostack[1];

    TRACE("MHN_setCallSiteTargetNormal\n");

    setCallSiteTargetNormal(call_site, target);
    return ostack;
}

uintptr_t *MHN_setCallSiteTargetVolatile(Class *class, MethodBlock *mb,
	                                 uintptr_t *ostack) {

    Object *call_site = (Object *)ostack[0];
    Object *target = (Object *)ostack[1];

    TRACE("MHN_setCallSiteTargetVolatile\n");

    setCallSiteTargetVolatile(call_site, target);
    return ostack;
}

// (Ljava/lang/Class;Ljava/lang/String;Ljava/lang/String;ILjava/lang/Class;I
//  [Ljava/lang/invoke/MemberName;)I
uintptr_t *MHN_getMembers(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *clazz = (Class *)ostack[0];
    Object *match_name = (Object *)ostack[1];
    Object *match_sig = (Object *)ostack[2];
    int match_flags = ostack[3];
    Class *caller = (Class *)ostack[4];
    int skip = ostack[5];
    Object *results = (Object *)ostack[6];

    TRACE("MHN_getMembers\n");

    *ostack++ = getMembers(clazz, match_name, match_sig, match_flags,
                           caller, skip, results);
    return ostack;
}

// (I[Ljava/lang/Object;)I
uintptr_t *MHN_getNamedCon(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    TRACE("MHN_getNamedCon\n");

    signalException(java_lang_InternalError, "MHN_getNamedCon: unimplemented");
    return ostack;
}

// ([Ljava/lang/Object;)Ljava/lang/Object;
uintptr_t *MH_invoke(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    TRACE("MH_invoke\n");

    signalChainedExceptionName("java/lang/UnsupportedOperationException",
                               "invoke cannot be invoked reflectively",
                               NULL);
    return ostack;
}

// ([Ljava/lang/Object;)Ljava/lang/Object;
uintptr_t *MH_invokeExact(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    TRACE("MH_invokeExact\n");

    signalChainedExceptionName("java/lang/UnsupportedOperationException",
                               "invokeExact cannot be invoked reflectively",
                               NULL);
    return ostack;
}

// (Ljava/lang/invoke/MemberName;Ljava/lang/Class;)V
uintptr_t *MHN_resolveMemberName(Class *class, MethodBlock *mb,
                                 uintptr_t *ostack) {

    Object *mname = (Object*)ostack[0];

    TRACE("MHN_resolveMemberName\n");

    *ostack++ = (uintptr_t)resolveMemberName(class, mname);
    return ostack;
}

VMMethod method_handle_natives[] = {
    {"registerNatives",            "()V", MHN_registerNatives},
    {"getConstant",                "(I)I", MHN_getConstant},
    {"resolve",                    "(Ljava/lang/invoke/MemberName;"
                                   "Ljava/lang/Class;)"
                                   "Ljava/lang/invoke/MemberName;",
                                   MHN_resolveMemberName},
    {"init",                       "(Ljava/lang/invoke/MemberName;"
                                   "Ljava/lang/Object;)V", MHN_initMemberName},
    {"expand",                     "(Ljava/lang/invoke/MemberName;)V",
                                   MHN_expandMemberName},
    {"getMembers",                 "(Ljava/lang/Class;Ljava/lang/String;"
                                   "Ljava/lang/String;ILjava/lang/Class;I"
                                   "[Ljava/lang/invoke/MemberName;)I",
                                   MHN_getMembers},
    {"getNamedCon",                "(I[Ljava/lang/Object;)I", MHN_getNamedCon},
    {"objectFieldOffset",          "(Ljava/lang/invoke/MemberName;)J",
                                   MHN_objectFieldOffset},
    {"staticFieldOffset",          "(Ljava/lang/invoke/MemberName;)J",
                                   MHN_staticFieldOffset},
    {"staticFieldBase",            "(Ljava/lang/invoke/MemberName;)"
                                   "Ljava/lang/Object;", MHN_staticFieldBase},
    {"getMemberVMInfo",            "(Ljava/lang/invoke/MemberName;)"
                                   "Ljava/lang/Object;", MHN_getMemberVMInfo},
    {"setCallSiteTargetNormal",    "(Ljava/lang/invoke/CallSite;"
                                   "Ljava/lang/invoke/MethodHandle;)V",
                                   MHN_setCallSiteTargetNormal},
    {"setCallSiteTargetVolatile",  "(Ljava/lang/invoke/CallSite;"
                                   "Ljava/lang/invoke/MethodHandle;)V",
                                   MHN_setCallSiteTargetVolatile},
    {NULL,                         NULL, NULL}
};

VMMethod method_handle[] = {
    {"invoke",                 "([Ljava/lang/Object;)Ljava/lang/Object;",
                               MH_invoke},
    {"invokeExact",            "([Ljava/lang/Object;)Ljava/lang/Object;",
                               MH_invokeExact},
    {NULL,                     NULL, NULL}
};
#endif

extern VMMethod sun_misc_perf[];

VMClass native_methods[] = {
#ifdef JSR292
    {"java/lang/invoke/MethodHandleNatives",      method_handle_natives},
    {"java/lang/invoke/MethodHandle",             method_handle},
#endif
    {"sun/misc/Unsafe",                           sun_misc_unsafe},
    {"sun/misc/Perf",                             sun_misc_perf},
    {NULL,                                        NULL}
};
