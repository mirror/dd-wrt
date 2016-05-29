/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009
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

#include "jam.h"
#include "sig.h"
#include "frame.h"
#include "lock.h"
#include "symbol.h"
#include "excep.h"
#include "properties.h"
#include "jni-internal.h"

#define VA_DOUBLE(args, sp)                                 \
    if(*sig == 'D')                                         \
        *(double*)sp = va_arg(args, double);                \
    else                                                    \
        *(u8*)sp = va_arg(args, u8);                        \
    sp+=2
         
#define VA_SINGLE(args, sp)                                 \
    if(*sig == 'L' || *sig == '[')                          \
        *sp = va_arg(args, uintptr_t) & ~REF_MASK;          \
    else                                                    \
        if(*sig == 'F')                                     \
            *((float*)sp + IS_BE64) = va_arg(args, double); \
        else                                                \
            *sp = va_arg(args, u4);                         \
    sp++

#define JA_DOUBLE(args, sp)  *(u8*)sp = *args++; sp+=2
#define JA_SINGLE(args, sp)                                 \
    switch(*sig) {                                          \
        case 'L': case '[':                                 \
            *sp = *(uintptr_t*)args & ~REF_MASK;            \
            break;                                          \
        case 'F':                                           \
            *((float*)sp + IS_BE64) = *(float*)args;        \
            break;                                          \
        case 'B': case 'Z':                                 \
            *sp = *(signed char*)args;                      \
            break;                                          \
        case 'C':                                           \
            *sp = *(unsigned short*)args;                   \
            break;                                          \
        case 'S':                                           \
            *sp = *(signed short*)args;                     \
            break;                                          \
        case 'I':                                           \
            *sp = *(signed int*)args;                       \
            break;                                          \
    }                                                       \
    sp++; args++

#define ADJUST_RET_ADDR(ret_addr, ret_type) ({              \
    char *adjusted = ret_addr;                              \
    if(IS_BIG_ENDIAN) {                                     \
        int size;                                           \
        switch(ret_type) {                                  \
            case 'B': case 'Z':                             \
                size = sizeof(uintptr_t) - 1;               \
                break;                                      \
            case 'C': case 'S':                             \
                size = sizeof(uintptr_t) - 2;               \
                break;                                      \
            case 'I': case 'F':                             \
                size = sizeof(uintptr_t) - 4;               \
                break;                                      \
            default:                                        \
                size = 0;                                   \
                break;                                      \
        }                                                   \
        adjusted += size;                                   \
    }                                                       \
    adjusted;                                               \
})

void *executeMethodArgs(Object *ob, Class *class, MethodBlock *mb, ...) {
    va_list jargs;
    void *ret;

    va_start(jargs, mb);
    ret = executeMethodVaList(ob, class, mb, jargs);
    va_end(jargs);

    return ret;
}

void *executeMethodVaList(Object *ob, Class *class, MethodBlock *mb,
                          va_list jargs) {

    ExecEnv *ee = getExecEnv();
    char *sig = mb->type;
    uintptr_t *sp;
    void *ret;

    CREATE_TOP_FRAME(ee, class, mb, sp, ret);

    /* copy args onto stack */

    if(ob)
        *sp++ = (uintptr_t) ob; /* push receiver first */

    SCAN_SIG(sig, VA_DOUBLE(jargs, sp), VA_SINGLE(jargs, sp))

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectLock(ob ? ob : mb->class);

    if(mb->access_flags & ACC_NATIVE)
        (*mb->native_invoker)(class, mb, ret);
    else
        executeJava();

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectUnlock(ob ? ob : mb->class);

    POP_TOP_FRAME(ee);

    return ADJUST_RET_ADDR(ret, *sig);
}

void *executeMethodList(Object *ob, Class *class, MethodBlock *mb, u8 *jargs) {
    char *sig = mb->type;

    ExecEnv *ee = getExecEnv();
    uintptr_t *sp;
    void *ret;

    CREATE_TOP_FRAME(ee, class, mb, sp, ret);

    /* copy args onto stack */

    if(ob)
        *sp++ = (uintptr_t) ob; /* push receiver first */

    SCAN_SIG(sig, JA_DOUBLE(jargs, sp), JA_SINGLE(jargs, sp))

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectLock(ob ? ob : mb->class);

    if(mb->access_flags & ACC_NATIVE)
        (*mb->native_invoker)(class, mb, ret);
    else
        executeJava();

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectUnlock(ob ? ob : mb->class);

    POP_TOP_FRAME(ee);

    return ADJUST_RET_ADDR(ret, *sig);
}

