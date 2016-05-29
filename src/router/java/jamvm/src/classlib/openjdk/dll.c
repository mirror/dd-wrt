/*
 * Copyright (C) 2010, 2011 Robert Lougher <rob@jamvm.org.uk>.
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
#include "symbol.h"

static MethodBlock *findNative_mb;

int classlibInitialiseDll() {
    Class *ldr_class = findSystemClass0(SYMBOL(java_lang_ClassLoader));

    if(ldr_class != NULL)
        findNative_mb = findMethod(ldr_class, SYMBOL(findNative),
                   SYMBOL(_java_lang_ClassLoader_java_lang_String__J));

    if(findNative_mb == NULL)  {
        jam_fprintf(stderr, "Expected \"findNative\" method missing "
                            "in java.lang.ClassLoader\n");
        return FALSE;
    }

    return TRUE;
}

char *classlibDefaultBootDllPath() {
    char *java_home = getJavaHome();
    char *dll_path = sysMalloc(strlen(java_home) + sizeof("/lib/"OS_ARCH));

    return strcat(strcpy(dll_path, java_home), "/lib/"OS_ARCH);
}

void *classlibLookupLoadedDlls(char *name, Object *loader) {
    Object *name_string;

    if(loader == NULL) {
        void *address = lookupLoadedDlls0(name, NULL);

        if(address != NULL)
            return address;
    }

    if((name_string = createString(name)) != NULL) {
        int64_t ret = *(int64_t*)executeStaticMethod(findNative_mb->class,
                                                     findNative_mb,
                                                     loader,
                                                     name_string);

        return (void *)(uintptr_t)ret;
    }

    return NULL;
}

