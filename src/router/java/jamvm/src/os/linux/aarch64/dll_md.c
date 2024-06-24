/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011
 * Robert Lougher <rob@jamvm.org.uk>.
 * Copyright (C) 2020 Simon South <simon@simonsouth.net>.
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

int nativeExtraArg(MethodBlock *mb) {
    char *sig = mb->type;
    int stack_args = 0;
    int int_args = 6;
    int fp_args = 8;

    while(*++sig != ')')
        switch(*sig) {
        case 'F':
        case 'D':
            if(fp_args == 0)
                stack_args += 8;
            else
                fp_args--;

        default:
            if(int_args == 0)
                stack_args += 8;
            else
                int_args--;

            if(*sig == '[')
                while(*++sig == '[');
            if(*sig == 'L')
                while(*++sig != ';');
            break;
        }

    /* Ensure the stack remains 16 byte aligned. */
    return (stack_args + 15) & ~15;
}
