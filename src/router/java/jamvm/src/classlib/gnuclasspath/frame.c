/*
 * Copyright (C) 2011 Robert Lougher <rob@jamvm.org.uk>.
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
#include "thread.h"
#include "reflect.h"
#include "gnuclasspath.h"

/* The function classlibGetCallerFrame() is used in code that does
   security related stack-walking.  It guards against invocation
   via reflection.  These frames must be skipped, else it will
   appear that the caller was loaded by the boot loader. */

Frame *classlibGetCallerFrame(Frame *last, int depth) {
    for(;;) {
        /* Loop until the required number of frames have been skipped
           or we hit a dummy frame (top of this invocation) */
        for(; last->mb != NULL; last = last->prev)
            if(depth-- <= 0)
                return last;

        /* Skip the dummy frame, and check if we're
           at the top of the stack */
        if((last = last->prev)->prev == NULL)
            return NULL;

        /* Check if we were invoked via reflection */
        if(last->mb->class == getReflectMethodClass()) {
            /* There will be two frames for invoke */
            last = last->prev->prev;
        }
    }
}
