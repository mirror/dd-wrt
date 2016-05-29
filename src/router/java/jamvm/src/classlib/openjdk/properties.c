/*
 * Copyright (C) 2010 Robert Lougher <rob@jamvm.org.uk>.
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

#include <string.h>
#include "jam.h"

char *classlibDefaultJavaHome() {
    char *path = nativeJVMPath();
    char *pntr = path + strlen(path);
    int count = 0;
    char *home;

    while(pntr > path && count < 4)
        count += *--pntr == '/';

    if(count != 4) {
        printf("Error: JVM path malformed.   Aborting VM.\n");
        exitVM(1);
    }

    home = sysMalloc(pntr - path + 1);
    memcpy(home, path, pntr - path);
    home[pntr - path] = '\0';
    sysFree(path);

    return home;
}

