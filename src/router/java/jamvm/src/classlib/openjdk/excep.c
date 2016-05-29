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

static int backtrace_offset;

int classlibInitialiseException(Class *throw_class) {

    FieldBlock *backtrace = findField(throw_class, SYMBOL(backtrace),
                                      SYMBOL(sig_java_lang_Object));

    if(backtrace == NULL) {
        jam_fprintf(stderr, "Expected \"backtrace\" field missing in "
                            "java.lang.Throwable\n");
        return FALSE;
    }

    backtrace_offset = backtrace->u.offset;
    return TRUE;
}

void fillInStackTrace(Object *thrwble) {
    Object *array = stackTrace(getExecEnv(), INT_MAX);
    INST_DATA(thrwble, Object*, backtrace_offset) = array;
}

int stackTraceDepth(Object *thrwble) {
    Object *array;

    if((array = INST_DATA(thrwble, Object*, backtrace_offset)) == NULL)
        return 0;

    return ARRAY_LEN(array)/2;
}

Object *stackTraceElementAtIndex(Object *thrwble, int index) {
    CodePntr pc;
    Object *array;
    MethodBlock *mb;

    if((array = INST_DATA(thrwble, Object*, backtrace_offset)) == NULL)
        return NULL;

    mb = ARRAY_DATA(array, MethodBlock*)[index * 2];
    pc = ARRAY_DATA(array, CodePntr)[index * 2 + 1];

    return stackTraceElement(mb, pc);
}
