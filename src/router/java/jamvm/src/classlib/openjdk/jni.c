/*
 * Copyright (C) 2010, 2011 Robert Lougher <rob@jamvm.org.uk>.
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
#include "symbol.h"

static int buffCap_offset, buffAddr_offset;
static MethodBlock *buffImpl_init_mb;
static Class *buffImpl_class;

int classlibInitialiseJNI() {
    FieldBlock *buffCap_fb, *buffAddr_fb;
    Class *buffer_class;

    /* Cache class and method/fields for JNI 1.4 NIO support */

    buffer_class = findSystemClass0(SYMBOL(java_nio_Buffer));
    buffImpl_class = findSystemClass0(SYMBOL(java_nio_DirectByteBuffer));

    if(buffer_class == NULL || buffImpl_class == NULL)
        return FALSE;

    buffImpl_init_mb = findMethod(buffImpl_class, SYMBOL(object_init),
                                                  SYMBOL(_JI__V));

    buffCap_fb = findField(buffer_class, SYMBOL(capacity), SYMBOL(I));
    buffAddr_fb = findField(buffer_class, SYMBOL(address), SYMBOL(J));

    if(buffImpl_init_mb == NULL || buffCap_fb == NULL || buffAddr_fb == NULL)
        return FALSE;

    registerStaticClassRef(&buffImpl_class);

    buffCap_offset = buffCap_fb->u.offset;
    buffAddr_offset = buffAddr_fb->u.offset;

    return TRUE;
}

Object *classlibNewDirectByteBuffer(void *addr, long long capacity) {
    Object *buff;

    if((buff = allocObject(buffImpl_class)) != NULL)
        executeMethod(buff, buffImpl_init_mb, (long long)(uintptr_t)addr,
                                              (int)capacity);

    return buff;
}

long long classlibGetDirectBufferCapacity(Object *buff) {
    return INST_DATA(buff, int, buffCap_offset);
}

void *classlibGetDirectBufferAddress(Object *buff) {
    return (void*)(uintptr_t)INST_DATA(buff, long long, buffAddr_offset);
}

Object *classlibCheckIfOnLoad(Frame *last) {
    Class *class = last->mb->class;

    if(CLASS_CB(class)->name == SYMBOL(java_lang_ClassLoader_NativeLibrary)) {
        MethodBlock *mb = findMethod(class, SYMBOL(getFromClass),
                                            SYMBOL(___java_lang_Class));

        if(mb != NULL) {
            Class *result = *(Class**)executeStaticMethod(class, mb);           

            if(!exceptionOccurred())
                return CLASS_CB(result)->class_loader;
        }
    }

    return NULL;
}
