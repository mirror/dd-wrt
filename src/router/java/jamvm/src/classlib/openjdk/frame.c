/*
 * Copyright (C) 2011, 2012, 2013 Robert Lougher <rob@jamvm.org.uk>.
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

static Class *cons_accessor;
static Class *method_accessor;

/* Defined in reflect.c */
extern MethodBlock *mthd_invoke_mb;

int classlibInitialiseFrame() {
    method_accessor = findSystemClass0(SYMBOL(sun_reflect_MethodAccessorImpl));
    cons_accessor = findSystemClass0(SYMBOL(sun_reflect_ConstructorAccessorImpl));

    if(method_accessor == NULL || cons_accessor == NULL)
        return FALSE;

    registerStaticClassRef(&cons_accessor);
    registerStaticClassRef(&method_accessor);

    return TRUE;
}
            
int classlibIsSkippedReflectFrame(Frame *frame) {

    return isSubClassOf(cons_accessor, frame->mb->class) ||
           isSubClassOf(method_accessor, frame->mb->class);
}

/* The function classlibGetCallerFrame() is used in code that does
   security related stack-walking.  It guards against invocation
   via reflection.  These frames must be skipped, else it will
   appear that the caller was loaded by the boot loader. */

Frame *classlibGetCallerFrame(Frame *last, int depth) {
    do {
        for(; last->mb != NULL; last = last->prev)
            if(!isSubClassOf(method_accessor, last->mb->class)
                   && !(last->mb->flags & MB_LAMBDA_COMPILED)
                   && last->mb != mthd_invoke_mb
                   && depth-- <= 0)
                return last;
    } while((last = last->prev)->prev != NULL);

    return NULL;
}

