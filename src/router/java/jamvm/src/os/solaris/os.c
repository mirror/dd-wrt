/*
 * Copyright (C) 2008, 2009, 2014 Robert Lougher <rob@jamvm.org.uk>.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include </usr/include/thread.h>
#include <signal.h>
#include <dlfcn.h>

#include "../../jam.h"

void *nativeStackBase() {
    stack_t ss;

    thr_stksegment(&ss);
    return ss.ss_sp;
}

int nativeAvailableProcessors() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

char *nativeLibError() {
    return dlerror();
}

char *nativeLibPath() {
    return getenv("LD_LIBRARY_PATH");
}

void *nativeLibOpen(char *path) {
    return dlopen(path, RTLD_LAZY);
}

void nativeLibClose(void *handle) {
    dlclose(handle);
}

void *nativeLibSym(void *handle, char *symbol) {
    return dlsym(handle, symbol);
}

char *nativeLibMapName(char *name) {
   char *buff = sysMalloc(strlen(name) + sizeof("lib.so") + 1);

   sprintf(buff, "lib%s.so", name);
   return buff;
}

long long nativePhysicalMemory() {
    return 0; /* TBD */
}
