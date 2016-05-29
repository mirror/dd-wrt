/*
 * Copyright (C) 2012 Robert Lougher <rob@jamvm.org.uk>.
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
#include "natives.h"
#include "classlib.h"

uintptr_t *registerNatives(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    return ostack;
}

uintptr_t *highResFrequency(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *(long long*)ostack = 0;
    return ostack + 2;
}

uintptr_t *highResCounter(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *(long long*)ostack = 0;
    return ostack + 2;
}

uintptr_t *createByteArray(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t)NULL;
    return ostack;
}

uintptr_t *createLong(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t)classlibNewDirectByteBuffer(sysMalloc(8), 8);
    return ostack;
}

uintptr_t *attach(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    *ostack++ = (uintptr_t)NULL;
    return ostack;
}

uintptr_t *detach(Class *class, MethodBlock *mb, uintptr_t *ostack) {
    return ostack;
}

VMMethod sun_misc_perf[] = {
    {"registerNatives",   "()V", registerNatives},
    {"highResFrequency",  "()J", highResFrequency},
    {"highResCounter",    "()J", highResCounter},
    {"createByteArray",   "(Ljava/lang/String;II[BI)Ljava/nio/ByteBuffer;",
                          createByteArray},
    {"createLong",        "(Ljava/lang/String;IIJ)Ljava/nio/ByteBuffer;",
                          createLong},
    {"attach",            "(Ljava/lang/String;II)Ljava/nio/ByteBuffer;",
                          attach},
    {"detach",            "(Ljava/nio/ByteBuffer;)V", detach},
    {NULL,                NULL, NULL}
};

