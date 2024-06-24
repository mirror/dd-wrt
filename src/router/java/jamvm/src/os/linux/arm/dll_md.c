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

#include "../../../jam.h"

#include <stdio.h>

#ifdef __ARM_EABI__

/* EABI requires 8 byte alignment of long longs and doubles.
 * To do this, the signature must be scanned, first to work
 * out stack requirements and then to push arguments.  To
 * save the first scan at call time, the signature is pre-
 * scanned and stack requirement stored in the extra argument. */

#ifdef __ARM_PCS_VFP
int nativeExtraArg(MethodBlock *mb) {
    char *sig = mb->type;
    int fp_backfill = 0;
    int stack_args = 0;
    int int_args = 2;
    int fp_args = 16;

    while(*++sig != ')')
        switch(*sig) {
            case 'J':
                if(int_args < 2)
                    stack_args = (stack_args + 15) & ~7;
                int_args = 0;
                break;

            case 'D':
                fp_backfill |= fp_args & 1;
                fp_args &= ~1;

                if(fp_args == 0) {
                    stack_args = (stack_args + 15) & ~7;
                    fp_backfill = 0;
                } else
                    fp_args -= 2;
                break;

            case 'F':
                if(fp_backfill)
                    fp_backfill = 0;
                else {
                    if(fp_args == 0) 
                        stack_args += 4;
                    else
                        fp_args--;
                }
                break;

            default:
                if(int_args == 0)
                    stack_args += 4;
                else
                    int_args--;

                if(*sig == '[')
                    while(*++sig == '[');
                if(*sig == 'L')
                    while(*++sig != ';');
                break;
        }

    return (stack_args + 7) & ~7;
}
#else
int nativeExtraArg(MethodBlock *mb) {
    char *sig = mb->type;
    int args = 0;

    while(*++sig != ')')
        switch(*sig) {
            case 'J':
            case 'D':
                args = (args + 15) & ~7;
                break;

            default:
                args += 4;

                if(*sig == '[')
                    while(*++sig == '[');
                if(*sig == 'L')
                    while(*++sig != ';');
                break;
        }

    /* For efficiency, callNativeEABI.inc pushes all arguments
       onto stack, and pops into r2/r3 before calling the
       native method, so minimum stack requirement is 8 bytes. */
    return args < 8 ? 8 : args;
}
#endif
#else

/* Under OABI, arguments can be copied onto the stack "as is"
 * from the operand stack.  As the signature isn't scanned
 * the return type must be pre-computed.  We take the
 * opportunity to turn it into a jump-table index number.
 */
int nativeExtraArg(MethodBlock *mb) {
    int len = strlen(mb->type);
    if(mb->type[len-2] == ')')
        switch(mb->type[len-1]) {
            case 'V':
                return 0;
#if defined(__VFP_FP__) || defined(__SOFTFP__)
            case 'D':
            case 'J':
                return 1;
        }

    return 2;
#else
            case 'D':
                return 1;
            case 'F':
                return 2;
            case 'J':
                return 3;
        }

    return 4;
#endif
}
#endif
