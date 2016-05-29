/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2014
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#define __USE_GNU
#include <dlfcn.h>
#include <pthread.h>

#include "../../jam.h"

long long nativePhysicalMemory() {
    /* Long longs are used here because with PAE, a 32-bit
       machine can have more than 4GB of physical memory */

    long long num_pages = sysconf(_SC_PHYS_PAGES);
    long long page_size = sysconf(_SC_PAGESIZE);

    return num_pages * page_size;
}

void *nativeStackBase() {
#ifdef __UCLIBC__
    return NULL;
#else
    pthread_attr_t attr;
    void *addr;
    size_t size;

    pthread_getattr_np(pthread_self(), &attr);
    pthread_attr_getstack(&attr, &addr, &size);

    return addr+size;
#endif
}

int nativeAvailableProcessors() {
#ifdef __UCLIBC__
    return 1;
#else
    return get_nprocs();
#endif
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

char *nativeJVMPath() {
    Dl_info info;
    char *path;

    if(dladdr(nativeJVMPath, &info) == 0) {
        printf("Error: dladdr failed.  Aborting VM\n");
        exitVM(1);
    }

    path = sysMalloc(strlen(info.dli_fname) + 1);
    strcpy(path, info.dli_fname);

    return path;
}

