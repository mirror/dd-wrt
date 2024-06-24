/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009
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

#include "../../../jam.h"

#include <string.h>
#include "../../../sig.h"

#define RET_VOID    0
#define RET_DOUBLE  1
#define RET_LONG    2
#define RET_FLOAT   3
#define RET_BYTE    4
#define RET_CHAR    5
#define RET_SHORT   6
#define RET_DFLT    7

int nativeExtraArg(MethodBlock *mb) {
    int len = strlen(mb->type);
    if(mb->type[len-2] == ')')
        switch(mb->type[len-1]) {
            case 'V':
                return RET_VOID;
            case 'D':
                return RET_DOUBLE;
            case 'J':
                return RET_LONG;
            case 'F':
                return RET_FLOAT;
            case 'B':
            case 'Z':
                return RET_BYTE;
            case 'C':
                return RET_CHAR;
            case 'S':
                return RET_SHORT;
        }

    return RET_DFLT;
}

u4 *callJNIMethod(void *env, Class *class, char *sig, int ret_type,
                  u4 *ostack, unsigned char *f, int args) {

    u4 *opntr = ostack + args;
    int i;

    for(i = 0; i < args; i++)
        asm volatile ("pushl %0" :: "m" (*--opntr) : "sp");

    if(class) {
        asm volatile ("pushl %0" :: "m" (class) : "sp");
	args++;
    }

    asm volatile ("pushl %0" :: "m" (env) : "sp");

    switch(ret_type) {
        case RET_VOID:
            (*(void (*)())f)();
            break;

        case RET_DOUBLE:
            *(double*)ostack = (*(double (*)())f)();
            ostack += 2;
            break;

        case RET_LONG:
            *(long long*)ostack = (*(long long (*)())f)();
            ostack += 2;
            break;

        case RET_FLOAT:
            *(float*)ostack = (*(float (*)())f)();
            ostack++;
            break;

        case RET_BYTE:
            *ostack++ = (*(signed char (*)())f)();
            break;

        case RET_CHAR:
            *ostack++ = (*(unsigned short (*)())f)();
            break;

        case RET_SHORT:
            *ostack++ = (*(signed short (*)())f)();
            break;

        default:
            *ostack++ = (*(u4 (*)())f)();
            break;
    }

    asm volatile ("addl %0,%%esp" :: "r" ((args + 1) * sizeof(u4)) 
                  : "cc", "sp");
    return ostack;
}
