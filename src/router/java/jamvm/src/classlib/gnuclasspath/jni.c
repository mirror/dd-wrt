/*
 * Copyright (C) 2010 Robert Lougher <rob@jamvm.org.uk>.
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

#include "jam.h"
#include "jni.h"
#include "symbol.h"

/* Cached values initialised on startup for JNI 1.4 NIO support */
static int buffCap_offset, buffAddr_offset, rawdata_offset;
static Class *buffImpl_class, *rawdata_class;
static MethodBlock *buffImpl_init_mb;

int classlibInitialiseJNI() {
    FieldBlock *buffCap_fb, *buffAddr_fb, *rawdata_fb;
    Class *buffer_class;

    /* Cache class and method/fields for JNI 1.4 NIO support */

    buffer_class = findSystemClass0(SYMBOL(java_nio_Buffer));
    buffImpl_class = findSystemClass0(
                         SYMBOL(java_nio_DirectByteBufferImpl_ReadWrite));
    rawdata_class = findSystemClass0(sizeof(uintptr_t) == 4
                                            ? SYMBOL(gnu_classpath_Pointer32)
                                            : SYMBOL(gnu_classpath_Pointer64));

    if(buffer_class == NULL || buffImpl_class == NULL || rawdata_class == NULL)
        return FALSE;

    buffImpl_init_mb = findMethod(buffImpl_class, SYMBOL(object_init),
                      SYMBOL(_java_lang_Object_gnu_classpath_Pointer_III__V));

    buffCap_fb = findField(buffer_class, SYMBOL(cap), SYMBOL(I));
    rawdata_fb = findField(rawdata_class, SYMBOL(data),
                           sizeof(uintptr_t) == 4 ? SYMBOL(I) : SYMBOL(J));
    buffAddr_fb = findField(buffer_class, SYMBOL(address),
                            SYMBOL(sig_gnu_classpath_Pointer));

    if(buffImpl_init_mb == NULL || buffCap_fb == NULL || rawdata_fb == NULL
                                || buffAddr_fb == NULL)
        return FALSE;

    registerStaticClassRef(&buffImpl_class);
    registerStaticClassRef(&rawdata_class);

    buffCap_offset = buffCap_fb->u.offset;
    buffAddr_offset = buffAddr_fb->u.offset;
    rawdata_offset = rawdata_fb->u.offset;

    return TRUE;
}

Object *classlibNewDirectByteBuffer(void *addr, long long capacity) {
    Object *buff, *rawdata;

    if((buff = allocObject(buffImpl_class)) != NULL &&
            (rawdata = allocObject(rawdata_class)) != NULL) {

        INST_DATA(rawdata, void*, rawdata_offset) = addr;
        executeMethod(buff, buffImpl_init_mb, NULL, rawdata, (int)capacity,
                      (int)capacity, 0);
    }

    return buff;
}

void *classlibGetDirectBufferAddress(Object *buff) {
    if(buff != NULL) {
        Object *rawdata = INST_DATA(buff, Object*, buffAddr_offset);

        if(rawdata != NULL)
            return INST_DATA(rawdata, void*, rawdata_offset);
    }

    return NULL;
}

long long classlibGetDirectBufferCapacity(Object *buff) {
    if(buff != NULL) {
        Object *rawdata = INST_DATA(buff, Object*, buffAddr_offset);

        if(rawdata != NULL)
            return INST_DATA(buff, jlong, buffCap_offset);
    }

    return -1;
}

Object *classlibCheckIfOnLoad(Frame *last) {
    if(CLASS_CB(last->mb->class)->name == SYMBOL(java_lang_VMRuntime))
        return (Object*)last->lvars[1];

    return NULL;
}

