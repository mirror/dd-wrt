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

static Class *vmthrow_class;
static int backtrace_offset;

int classlibInitialiseException(Class *throw_class) {
    FieldBlock *backtrace = NULL;

    vmthrow_class = findSystemClass0(SYMBOL(java_lang_VMThrowable));
    if(vmthrow_class != NULL)
        backtrace = findField(vmthrow_class, SYMBOL(backtrace),
                                             SYMBOL(sig_java_lang_Object));

    if(backtrace == NULL) {
        jam_fprintf(stderr, "Expected \"backtrace\" field missing in "
                            "java.lang.VMThrowable\n");
        return FALSE;
    }

    CLASS_CB(vmthrow_class)->flags |= VMTHROWABLE;
    backtrace_offset = backtrace->u.offset;

    registerStaticClassRef(&vmthrow_class);
    return TRUE;
}

Object *setStackTrace0(ExecEnv *ee, int max_depth) {
    Object *array = stackTrace(ee, max_depth);
    Object *vmthrwble = allocObject(vmthrow_class);

    if(vmthrwble != NULL)
        INST_DATA(vmthrwble, Object*, backtrace_offset) = array;

    return vmthrwble;
}

Object *convertStackTrace(Object *vmthrwble) {
    Object *array = INST_DATA(vmthrwble, Object*, backtrace_offset);

    if(array == NULL)
        return NULL;

    return stackTraceElements(array);
}

/* GC support for marking classes referenced by a VMThrowable.
   In rare circumstances a stack backtrace may hold the only
   reference to a class */

void markVMThrowable(Object *vmthrwble, int mark) {
    Object *array;

    if((array = INST_DATA(vmthrwble, Object*, backtrace_offset)) != NULL) {
        uintptr_t *src = ARRAY_DATA(array, uintptr_t);
        int i, depth = ARRAY_LEN(array);

        for(i = 0; i < depth; i += 2) {
            MethodBlock *mb = (MethodBlock*)src[i];
            markObject(mb->class, mark);
        }
    }
}

