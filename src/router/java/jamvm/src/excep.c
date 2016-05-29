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

#include <stdio.h>
#include <stdlib.h>
#include "jam.h"
#include "lock.h"
#include "symbol.h"
#include "excep.h"
#include "hash.h"
#include "class.h"
#include "thread.h"
#include "classlib.h"

static Class *ste_array_class, *ste_class, *throw_class;
static MethodBlock *ste_init_mb;
static int inited = FALSE;

static Class *exceptions[MAX_EXCEPTION_ENUM];

static int exception_symbols[] = {
    CLASSLIB_EXCEPTIONS_DO(SYMBOL_NAME_ENUM)
    EXCEPTIONS_DO(SYMBOL_NAME_ENUM)
};

int initialiseException() {
    int i;

    ste_array_class = findArrayClass(SYMBOL(array_java_lang_StackTraceElement));
    ste_class = findSystemClass0(SYMBOL(java_lang_StackTraceElement));
    throw_class = findSystemClass0(SYMBOL(java_lang_Throwable));

    if(ste_array_class != NULL && ste_class != NULL && throw_class != NULL)
        ste_init_mb = findMethod(ste_class, SYMBOL(object_init),
           SYMBOL(_java_lang_String_java_lang_String_java_lang_String_I__V));

    if(ste_init_mb == NULL)
        goto error;

    registerStaticClassRef(&ste_array_class);
    registerStaticClassRef(&throw_class);
    registerStaticClassRef(&ste_class);

    /* Load and register the exceptions used within the VM.
       These are preloaded to speed up access.  The VM will
       abort if any can't be loaded */

    for(i = 0; i < MAX_EXCEPTION_ENUM; i++) {
        exceptions[i] = findSystemClass0(symbol_values[exception_symbols[i]]);
        registerStaticClassRef(&exceptions[i]);
    }

    if((inited = classlibInitialiseException(throw_class)))
        return TRUE;

error:
    jam_fprintf(stderr, "Error initialising VM (initialiseException)\n");
    return FALSE;
}

Object *exceptionOccurred() {
   return getExecEnv()->exception; 
}

void setException(Object *exp) {
    getExecEnv()->exception = exp;
}

void clearException() {
    ExecEnv *ee = getExecEnv();

    if(ee->overflow) {
        ee->overflow = FALSE;
        ee->stack_end -= STACK_RED_ZONE_SIZE;
    }
    ee->exception = NULL;
}

void signalChainedExceptionClass(Class *exception, char *message,
                                 Object *cause) {

    Object *exp = allocObject(exception);
    Object *str = message == NULL ? NULL : Cstr2String(message);
    MethodBlock *init = lookupMethod(exception, SYMBOL(object_init),
                                                SYMBOL(_java_lang_String__V));

    if(exp && init) {
        executeMethod(exp, init, str);

        if(cause && !exceptionOccurred()) {
            MethodBlock *mb = lookupMethod(exception, SYMBOL(initCause),
                             SYMBOL(_java_lang_Throwable__java_lang_Throwable));
            if(mb)
                executeMethod(exp, mb, cause);
        }
        setException(exp);
    }
}

void signalChainedExceptionName(char *excep_name, char *message,
                                Object *cause) {
    if(!inited) {
        jam_fprintf(stderr, "Exception occurred while VM initialising.\n");
        if(message)
            jam_fprintf(stderr, "%s: %s\n", excep_name, message);
        else
            jam_fprintf(stderr, "%s\n", excep_name);
        exit(1);
    } else {
        Class *exception = findSystemClass(excep_name);

        if(!exceptionOccurred())
            signalChainedExceptionClass(exception, message, cause);
    }
}

void signalChainedExceptionEnum(int excep_enum, char *message, Object *cause) {
    if(!inited) {
        char *excep_name = symbol_values[exception_symbols[excep_enum]];

        jam_fprintf(stderr, "Exception occurred while VM initialising.\n");
        if(message)
            jam_fprintf(stderr, "%s: %s\n", excep_name, message);
        else
            jam_fprintf(stderr, "%s\n", excep_name);
        exit(1);
    }

    signalChainedExceptionClass(exceptions[excep_enum], message, cause);
}

Object *exceptionEnumToException(int excep_enum) {
    return exceptions[excep_enum];
}

void printException() {
    ExecEnv *ee = getExecEnv();
    Object *excep = ee->exception;

    if(excep != NULL) {
        MethodBlock *mb = lookupMethod(excep->class, SYMBOL(printStackTrace),
                                                     SYMBOL(___V));
        clearException();
        executeMethod(excep, mb);

        /* If we're really low on memory we might have been able to throw
         * OutOfMemory, but then been unable to print any part of it!  In
         * this case the VM just seems to stop... */
        if(ee->exception) {
            jam_fprintf(stderr, "Exception occurred while printing exception"
                        " (%s)...\n", CLASS_CB(ee->exception->class)->name);
            jam_fprintf(stderr, "Original exception was %s\n",
                        CLASS_CB(excep->class)->name);
        }
    }
}

CodePntr findCatchBlockInMethod(MethodBlock *mb, Class *exception,
                                CodePntr pc_pntr) {

    ExceptionTableEntry *table = mb->exception_table;
    int size = mb->exception_table_size;
    int pc = pc_pntr - ((CodePntr)mb->code);
    int i;
 
    for(i = 0; i < size; i++)
        if((pc >= table[i].start_pc) && (pc < table[i].end_pc)) {

            /* If the catch_type is 0 it's a finally block, which matches
               any exception.  Otherwise, the thrown exception class must
               be an instance of the caught exception class to catch it */

            if(table[i].catch_type != 0) {
                Class *caught_class = resolveClass(mb->class,
                                                   table[i].catch_type,
                                                   TRUE, FALSE);
                if(caught_class == NULL) {
                    clearException();
                    continue;
                }
                if(!isInstanceOf(caught_class, exception))
                    continue;
            }
            return ((CodePntr)mb->code) + table[i].handler_pc;
        }

    return NULL;
}
    
CodePntr findCatchBlock(Class *exception) {
    Frame *frame = getExecEnv()->last_frame;
    CodePntr handler_pc = NULL;

    while(((handler_pc = findCatchBlockInMethod(frame->mb, exception,
                                                frame->last_pc)) == NULL)
                    && (frame->prev->mb != NULL)) {

        if(frame->mb->access_flags & ACC_SYNCHRONIZED) {
            Object *sync_ob = frame->mb->access_flags & ACC_STATIC ?
                    (Object*)frame->mb->class : (Object*)frame->lvars[0];
            objectUnlock(sync_ob);
        }
        frame = frame->prev;
    }

    getExecEnv()->last_frame = frame;

    return handler_pc;
}

int mapPC2LineNo(MethodBlock *mb, CodePntr pc_pntr) {
    int pc = pc_pntr - (CodePntr) mb->code;
    int i;

    if(mb->line_no_table_size > 0) {
        for(i = mb->line_no_table_size-1; i &&
                    pc < mb->line_no_table[i].start_pc; i--);

        return mb->line_no_table[i].line_no;
    }

    return -1;
}

Frame *skipExceptionFrames(Frame *last) {
    for(; last->mb != NULL && last->mb->name == SYMBOL(fillInStackTrace);
          last = last->prev);

    for(; last->mb != NULL && last->mb->name == SYMBOL(object_init)
                           && isInstanceOf(throw_class, last->mb->class);
          last = last->prev);

    return last;
}

int countStackFrames(Frame *last, int max_depth) {
    int depth = 0;

    do {
        for(; last->mb != NULL; last = last->prev, depth++)
            if(depth == max_depth)
                goto out;
    } while((last = last->prev)->prev != NULL);
    
out:
    return depth;
}

void stackTrace2Buffer(Frame *last, void **data, int max_depth) {
    int limit = max_depth * 2, depth = 0;

    do {
        for(; last->mb != NULL; last = last->prev) {
            if(depth == limit)
                return;

            data[depth++] = last->mb;
            data[depth++] = last->last_pc;
        }
    } while((last = last->prev)->prev != NULL);
}

Object *stackTrace(ExecEnv *ee, int max_depth) {
    Frame *last = ee->last_frame;
    Object *array;
    void **data;
    int depth;

    if(last->prev == NULL)
        return allocTypeArray(sizeof(uintptr_t) == 4 ? T_INT : T_LONG, 0);
    
    last = skipExceptionFrames(last);
    depth = countStackFrames(last, max_depth);

    array = allocTypeArray(sizeof(uintptr_t) == 4 ? T_INT : T_LONG, depth*2);
    if(array == NULL)
        return NULL;

    data = ARRAY_DATA(array, void *);
    stackTrace2Buffer(last, data, depth);

    return array;
}

Object *stackTraceElement(MethodBlock *mb, CodePntr pc) {
    ClassBlock *cb = CLASS_CB(mb->class);
    int is_native = mb->access_flags & ACC_NATIVE;
    char *dot_name = classlibExternalClassName(mb->class);

    Object *methodname = createString(mb->name);
    Object *classname = createString(dot_name);
    Object *ste = allocObject(ste_class);
    Object *filename = NULL;

    sysFree(dot_name);
    if(methodname == NULL || classname == NULL || ste == NULL)
        return NULL;

    if(!is_native && cb->source_file_name != NULL) {
        filename = createString(cb->source_file_name);
        if(filename == NULL)
            return NULL;
    }

    executeMethod(ste, ste_init_mb,
                  findInternedString(classname), 
                  findInternedString(methodname), 
                  findInternedString(filename),
                  is_native ? -2 : mapPC2LineNo(mb, pc));

    if(exceptionOccurred())
        return NULL;

    return ste;
}

Object *convertTrace2Elements(void **trace, int len) {
    Object *ste_array;
    Object **dest;
    int i, j;

    ste_array = allocArray(ste_array_class, len/2, sizeof(Object*));
    if(ste_array == NULL)
        return NULL;

    dest = ARRAY_DATA(ste_array, Object*);

    for(i = 0, j = 0; i < len; j++) {
        MethodBlock *mb = trace[i++];
        CodePntr pc = trace[i++];
        Object *ste;

        if((ste = stackTraceElement(mb, pc)) == NULL)
            return NULL;

        dest[j] = ste;
    }

    return ste_array;
}

Object *stackTraceElements(Object *trace) {
    void **data = ARRAY_DATA(trace, void *);
    int len = ARRAY_LEN(trace);

    return convertTrace2Elements(data, len);
}
