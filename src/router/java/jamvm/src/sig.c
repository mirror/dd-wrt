/*
 * Copyright (C) 2009, 2012 Robert Lougher <rob@jamvm.org.uk>.
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
#include "sig.h"

int sigElement2Size(char element) {
    switch(element) {
        case 'B':
        case 'Z':
            return 1;

        case 'C':
        case 'S':
            return 2;

        case 'I':
        case 'F':
           return 4;

        case 'L':
        case '[':
            return sizeof(Object*);

        default:
            return 8;
    }
}

int sigArgsCount(char *sig) {
    int count = 0;

    SCAN_SIG(sig, count+=2, count++);

    return count;
}
