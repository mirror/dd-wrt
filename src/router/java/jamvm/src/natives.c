/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010
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

#include <string.h>

#include "jam.h"
#include "hash.h"
#include "excep.h"
#include "class.h"
#include "thread.h"
#include "reflect.h"
#include "classlib.h"

int initialiseNatives() {
    if(!classlibInitialiseNatives()) {
        jam_fprintf(stderr, "Error initialising VM (initialiseNatives)\n");
        return FALSE;
    }

    return TRUE;
}

void copyarray(Object *src, int start1, Object *dest, int start2, int length) {
    ClassBlock *scb, *dcb;
    char *sdata, *ddata;
    Object **sob, **dob;
    int i;

    if((src == NULL) || (dest == NULL)) {
        signalException(java_lang_NullPointerException, NULL);
        return;
    }

    scb = CLASS_CB(src->class);
    dcb = CLASS_CB(dest->class);

    if((scb->name[0] != '[') || (dcb->name[0] != '['))
        goto storeExcep; 

    if((start1 < 0) || (start2 < 0) || (length < 0)
                    || ((start1 + length) > ARRAY_LEN(src))
                    || ((start2 + length) > ARRAY_LEN(dest))) {
        signalException(java_lang_ArrayIndexOutOfBoundsException, NULL);
        return;
    }

    sdata = ARRAY_DATA(src, char);            
    ddata = ARRAY_DATA(dest, char);            

    if(isInstanceOf(dest->class, src->class)) {
        int size = sigElement2Size(scb->name[1]);
        memmove(ddata + start2*size, sdata + start1*size, length*size);
        return;
    }

    if(!(((scb->name[1] == 'L') || (scb->name[1] == '[')) &&
         ((dcb->name[1] == 'L') || (dcb->name[1] == '['))))
        goto storeExcep; 

    /* Not compatible array types, but elements may be compatible...
       e.g. src = [Ljava/lang/Object, dest = [Ljava/lang/String, but
       all src = Strings - check one by one...
     */
            
    if(scb->dim > dcb->dim)
        goto storeExcep;

    sob = &((Object**)sdata)[start1];
    dob = &((Object**)ddata)[start2];

    for(i = 0; i < length; i++) {
        if((*sob != NULL) && !arrayStoreCheck(dest->class, (*sob)->class))
            goto storeExcep;
        *dob++ = *sob++;
    }

    return;

storeExcep:
    signalException(java_lang_ArrayStoreException, NULL);
}

/* sun.misc.Unsafe */

static volatile uintptr_t spinlock = 0;

void lockSpinLock() {
    while(!LOCKWORD_COMPARE_AND_SWAP(&spinlock, 0, 1));
}

void unlockSpinLock() {
    LOCKWORD_WRITE(&spinlock, 0);
}

uintptr_t *objectFieldOffset(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    FieldBlock *fb = fbFromReflectObject((Object*)ostack[1]);

    *(long long*)ostack = (long long)(uintptr_t)
                          &(INST_DATA((Object*)NULL, int, fb->u.offset));
    return ostack + 2;
}

uintptr_t *compareAndSwapInt(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    unsigned int *addr = (unsigned int*)((char *)ostack[1] + offset);
    unsigned int expect = ostack[4];
    unsigned int update = ostack[5];
    int result;

#ifdef COMPARE_AND_SWAP_32
    result = COMPARE_AND_SWAP_32(addr, expect, update);
#else
    lockSpinLock();
    if((result = (*addr == expect)))
        *addr = update;
    unlockSpinLock();
#endif

    *ostack++ = result;
    return ostack;
}

uintptr_t *compareAndSwapLong(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    long long offset = *((long long *)&ostack[2]);
    long long *addr = (long long*)((char*)ostack[1] + offset);
    long long expect = *((long long *)&ostack[4]);
    long long update = *((long long *)&ostack[6]);
    int result;

#ifdef COMPARE_AND_SWAP_64
    result = COMPARE_AND_SWAP_64(addr, expect, update);
#else
    lockSpinLock();
    if((result = (*addr == expect)))
        *addr = update;
    unlockSpinLock();
#endif

    *ostack++ = result;
    return ostack;
}

uintptr_t *putOrderedInt(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile unsigned int *addr = (unsigned int*)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    *addr = value;
    return ostack;
}

uintptr_t *putOrderedLong(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    long long value = *((long long *)&ostack[4]);
    volatile long long *addr = (long long*)((char*)ostack[1] + offset);

    if(sizeof(uintptr_t) == 8)
        *addr = value;
    else {
        lockSpinLock();
        *addr = value;
        unlockSpinLock();
    }

    return ostack;
}

uintptr_t *putIntVolatile(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile unsigned int *addr = (unsigned int *)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    MBARRIER();
    *addr = value;

    return ostack;
}

uintptr_t *getIntVolatile(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile unsigned int *addr = (unsigned int*)((char *)ostack[1] + offset);

    *ostack++ = *addr;
    MBARRIER();

    return ostack;
}

uintptr_t *putLong(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    long long value = *((long long *)&ostack[4]);
    long long *addr = (long long*)((char*)ostack[1] + offset);

    if(sizeof(uintptr_t) == 8)
        *addr = value;
    else {
        lockSpinLock();
        *addr = value;
        unlockSpinLock();
    }

    return ostack;
}

uintptr_t *getLongVolatile(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile long long *addr = (long long*)((char*)ostack[1] + offset);

    if(sizeof(uintptr_t) == 8)
        *(long long*)ostack = *addr;
    else {
        lockSpinLock();
        *(long long*)ostack = *addr;
        unlockSpinLock();
    }

    return ostack + 2;
}

uintptr_t *getLong(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    long long *addr = (long long*)((char*)ostack[1] + offset);

    if(sizeof(uintptr_t) == 8)
        *(long long*)ostack = *addr;
    else {
        lockSpinLock();
        *(long long*)ostack = *addr;
        unlockSpinLock();
    }

    return ostack + 2;
}

uintptr_t *compareAndSwapObject(Class *class, MethodBlock *mb,
                                uintptr_t *ostack) {

    long long offset = *((long long *)&ostack[2]);
    uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);
    uintptr_t expect = ostack[4];
    uintptr_t update = ostack[5];
    int result;

#ifdef COMPARE_AND_SWAP
    result = COMPARE_AND_SWAP(addr, expect, update);
#else
    lockSpinLock();
    if((result = (*addr == expect)))
        *addr = update;
    unlockSpinLock();
#endif

    *ostack++ = result;
    return ostack;
}

uintptr_t *putOrderedObject(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    *addr = value;
    return ostack;
}

uintptr_t *putObjectVolatile(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    MBARRIER();
    *addr = value;

    return ostack;
}

uintptr_t *getObjectVolatile(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    volatile uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);

    *ostack++ = *addr;
    MBARRIER();

    return ostack;
}

uintptr_t *putObject(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    long long offset = *((long long *)&ostack[2]);
    uintptr_t *addr = (uintptr_t*)((char *)ostack[1] + offset);
    uintptr_t value = ostack[4];

    *addr = value;
    return ostack;
}

uintptr_t *arrayBaseOffset(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t)ARRAY_DATA((Object*)NULL, void);
    return ostack;
}

uintptr_t *arrayIndexScale(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Class *array_class = (Class*)ostack[1];
    ClassBlock *cb = CLASS_CB(array_class);
    int scale = 0;

    /* Sub-int fields within objects are widened to int, whereas
       sub-int arrays are packed.  This means sub-int arrays can
       not be accessed, and must return zero. */

    if(cb->name[0] == '[')
        switch(cb->name[1]) {
            case 'I':
            case 'F':
                scale = 4;
                break;

            case 'J':
            case 'D':
                scale = 8;
                break;

            case '[':
            case 'L':
                scale = sizeof(Object*);
                break;
        }

    *ostack++ = scale;
    return ostack;
}

uintptr_t *unpark(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    Object *jThread = (Object *)ostack[1];

    if(jThread != NULL) {
        Thread *thread = jThread2Thread(jThread);

        if(thread != NULL)
            threadUnpark(thread);
    }
    return ostack;
}

uintptr_t *park(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    int absolute = ostack[1];
    long long time = *((long long *)&ostack[2]);
    Thread *thread = threadSelf();

    threadPark(thread, absolute, time);
    return ostack;
}

