/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007
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

#ifndef USE_FFI
#include <stdio.h>

/* The PowerPC calling convention passes the first 8 integer
   and fp args in registers.  After this, the arguments spill
   onto the stack.  Because of long/double alignment, and arg
   ordering, we must scan the signature to calculate the extra
   stack requirements.  This is done the first time the native
   method is called, and stored in the opaque "extra argument".
*/

int nativeExtraArg(MethodBlock *mb) {
    char *sig = mb->type;
    int iargs = 2; /* take into account the 2 extra arguments
                      passed (JNIEnv and class/this). */
    int fargs = 0;
    int margs = 0;

    while(*++sig != ')')
        switch(*sig) {
            case 'J':
                iargs = (iargs + 3) & ~1;
                if(iargs > 8)
                    margs = (margs + 3) & ~1;
                break;

            case 'D':
                if(++fargs > 8)
                    margs = (margs + 3) & ~1;
                break;

            case 'F':
                if(++fargs > 8)
                    margs++;
                break;

            default:
                if(++iargs > 8)
                    margs++;

                if(*sig == '[')
                    while(*++sig == '[');
                if(*sig == 'L')
                    while(*++sig != ';');
                break;
        }

#ifdef DEBUG_DLL
    jam_printf("<NativeExtraArg: %s.%s%s --> int args: %d fp args: %d stack: %d>\n",
               CLASS_CB(mb->class)->name, mb->name, mb->type, iargs, fargs, margs);
#endif

    return margs;
}
#endif
