/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2011
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
#include "hash.h"
#include "class.h"
#include "thread.h"
#include "classlib.h"

int initialiseFrame() {
    if(!classlibInitialiseFrame()) {
        jam_fprintf(stderr, "Error initialising VM (initialiseFrame)\n");
        return FALSE;
    }

    return TRUE;
}

Class *getCallerClass(int depth) {
    Frame *last = getExecEnv()->last_frame;

    if(last->prev == NULL)
        return NULL;

    if((last = classlibGetCallerFrame(last, depth)) == NULL)
        return NULL;

    return last->mb->class;
}

Object *getClassContext() {
    Class *class_class = findArrayClass("[Ljava/lang/Class;");
    Frame *last = getExecEnv()->last_frame;
    Frame *bottom = last;
    Object *array;
    int depth = 0;

    if(class_class == NULL)
        return NULL;

    if(last->prev == NULL)
        return allocArray(class_class, 0, sizeof(Class*));

    for(; last != NULL; last = classlibGetCallerFrame(last, 1))
        if(!(last->mb->access_flags & ACC_NATIVE))
            depth++;

    array = allocArray(class_class, depth, sizeof(Class*));

    if(array != NULL) {
        Class **data = ARRAY_DATA(array, Class*);

        for(; bottom != NULL; bottom = classlibGetCallerFrame(bottom, 1))
            if(!(bottom->mb->access_flags & ACC_NATIVE))
                *data++ = bottom->mb->class;
    }

    return array;
}

Object *firstNonNullClassLoader() {
    Frame *last = getExecEnv()->last_frame;

    if(last->prev != NULL)
        do {
            for(; last->mb != NULL; last = last->prev)
                if(!classlibIsSkippedReflectFrame(last)) {
                    Object *loader = CLASS_CB(last->mb->class)->class_loader;
                    if(loader != NULL)
                        return loader;
                }
        } while((last = last->prev)->prev != NULL);

    return NULL;
}

