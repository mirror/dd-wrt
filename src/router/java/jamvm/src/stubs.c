/*
 * Copyright (C) 2011, 2012 Robert Lougher <rob@jamvm.org.uk>.
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

#ifndef GEN_STUBS_INC
#include "jam.h"
#endif

#if defined(GEN_STUBS_INC) || !defined(USE_MD_STUBS)
#include <string.h>

static char mapSigElement2Simple(char element) {
    switch(element) {
        case 'J':
        case 'D':
        case 'F':
            return element;

        default:
            return 'I';
    }
}

static char mapRet2Simple(char element) {
    switch(element) {
        case 'Z':
            return 'B';

        case '[':
            return 'L';

        default:
            return element;
    }
}

char *convertSig2Simple(char *sig) {
    char *simple_sig = sysMalloc(strlen(sig) + 1);
    char *simple_pntr = simple_sig;
    char *sig_pntr = sig;

    *simple_pntr++ = '(';
    while(*++sig_pntr != ')') {
        *simple_pntr++ = mapSigElement2Simple(*sig_pntr);

        if(*sig_pntr == '[')
            while(*++sig_pntr == '[');
        if(*sig_pntr == 'L')
            while(*++sig_pntr != ';');
    }

    *simple_pntr++ = ')';
    *simple_pntr++ = mapRet2Simple(*++sig_pntr);
    *simple_pntr++ = '\0';

    return sysRealloc(simple_sig, simple_pntr - simple_sig);
}
#endif

