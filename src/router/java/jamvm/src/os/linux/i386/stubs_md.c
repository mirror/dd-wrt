/*
 * Copyright (C) 2010, 2012 Robert Lougher <rob@jamvm.org.uk>.
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

#if defined(GEN_STUBS_INC) || defined(USE_MD_STUBS)
#include <string.h>
#include "sig.h"

char *convertSig2Simple(char *sig) {
    char *simple_sig, *simple_pntr;
    int count = 0;
    int i;

    SCAN_SIG(sig, count+=2, count++);
    simple_sig = simple_pntr = sysMalloc((count + 1)/2 + 4);

    *simple_pntr++ = '(';
    for(i = 0; i < count/2; i++)
        *simple_pntr++ = 'J';

    if(count&0x1)
        *simple_pntr++ = 'I';
    *simple_pntr++ = ')';

    switch(*sig) {
        case 'Z':
            *simple_pntr++ = 'B';
            break;

        case '[':
            *simple_pntr++ = 'L';
            break;

        default:
            *simple_pntr++ = *sig;
            break;
    }

    *simple_pntr = '\0';
    return simple_sig;
}
#endif

