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

#include <stdlib.h>

#include "jam.h"
#include "symbol.h"

#define SYMBOL_VALUE(name, value) value
char *symbol_values[] = {
    CLASSLIB_SYMBOLS_DO(SYMBOL_VALUE),
    SYMBOLS_DO(SYMBOL_VALUE)
};

int initialiseSymbol() {
    int i;

    for(i = 0; i < MAX_SYMBOL_ENUM; i++)
        if(symbol_values[i] != newUtf8(symbol_values[i])) {
            jam_fprintf(stderr, "Error when initialising VM symbols."
                                "  Aborting VM.\n");
            return FALSE;
        }

    return TRUE;
}
