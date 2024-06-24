/*
 * Copyright (C) 2008, 2010 Robert Lougher <rob@jamvm.org.uk>.
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

#include <stdio.h>

int nativeExtraArg(MethodBlock *mb) {
    char *sig = mb->type;
    int stack_space;
    int iargs = 0;
    int fargs = 0;

    while(*++sig != ')')
        switch(*sig) {
            case 'D':
            case 'F':
                fargs++;
                break;

            default:
                iargs++;

                if(*sig == '[')
                    while(*++sig == '[');
                if(*sig == 'L')
                    while(*++sig != ';');
                break;
        }

    stack_space = ((iargs > 4 ? iargs - 4 : 0) +
                   (fargs > 8 ? fargs - 8 : 0)) << 3;

    /* Ensure the stack remains 16 byte aligned.  As
       callJNIMethod pushes an even number of registers
       the extra space must also be even. */
    return (stack_space + 15) & ~15;
}
