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
#include "openjdk.h"

#ifdef TRACEGC
#define TRACE(fmt, ...) jam_printf(fmt, ## __VA_ARGS__)
#else
#define TRACE(fmt, ...)
#endif

void classlibHandleUnmarkedSpecial(Object *ob) {
    if(IS_JTHREAD(CLASS_CB(ob->class))) {
        /* Free the native thread structure (see comment
           in detachThread (thread.c) */
        TRACE("FREE: Freeing native thread for java thread object %p\n", ob);
        gcPendingFree(jThread2Thread(ob));
    }
}

