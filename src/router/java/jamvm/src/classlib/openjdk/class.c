/*
 * Copyright (C) 2010, 2011, 2014 Robert Lougher <rob@jamvm.org.uk>.
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
#include "hash.h"
#include "class.h"
#include "excep.h"
#include "symbol.h"

/* Cached offset of classes field in java.lang.ClassLoader objects */
int ldr_classes_offset;
int ldr_parent_offset;

Class *delegating_ldr_class;

#define CLASS_INITSZE 1<<8

void classlibCacheClassLoaderFields(Class *loader_class) {
    FieldBlock *classes_fb = findField(loader_class, SYMBOL(classes),
                                       SYMBOL(sig_java_util_vector));

    FieldBlock *parent_fb = findField(loader_class, SYMBOL(parent),
                                      SYMBOL(sig_java_lang_ClassLoader));

    if(classes_fb == NULL || parent_fb == NULL) {
        jam_fprintf(stderr, "Expected \"classes\" and/or \"parent\" field "
                            "missing in java.lang.ClassLoader\n");
        exitVM(1);
    }

    hideFieldFromGC(classes_fb);

    ldr_classes_offset = classes_fb->u.offset;
    ldr_parent_offset = parent_fb->u.offset;
}

HashTable *classlibLoaderTable(Object *class_loader) {
    void *pntr = INST_DATA(class_loader, void*, ldr_classes_offset);

    if(isObject(pntr))
        return NULL;

    return pntr;
}

HashTable *classlibCreateLoaderTable(Object *class_loader) {
    HashTable *table = sysMalloc(sizeof(HashTable));

    initHashTable((*table), CLASS_INITSZE, TRUE);
    INST_DATA(class_loader, HashTable*, ldr_classes_offset) = table;

    return table;
}

Object *classlibBootPackage(PackageEntry *package_entry) {
    char *entry = getBootClassPathEntry(package_entry->index);
    return createString(entry);
}

Object *classlibBootPackages(PackageEntry *package_entry) {
    char *name = package_entry->name;
    char padded[strlen(name) + 2];
    Object *string;
    
    strcat(strcpy(padded, name), " ");
    string = createString(padded);

    return string;
}

Class *classlibBootPackagesArrayClass() {
    return findArrayClass(SYMBOL(array_java_lang_String));
}

/* Add a library unloader object to the class loader for the
   library contained within entry.  The library has an unload
   function, which will be called from the unloader finalizer
   when the class loader is garbage collected */
void classlibNewLibraryUnloader(Object *class_loader, void *entry) {
}

Object *classlibSkipReflectionLoader(Object *loader) {
    if(loader != NULL) {
        if(delegating_ldr_class == NULL) {
            Class *class = findSystemClass0(SYMBOL(
                                   sun_reflect_DelegatingClassLoader));

            if(class == NULL)
                return loader;

            registerStaticClassRefLocked(&delegating_ldr_class, class);
        }

        if(isSubClassOf(delegating_ldr_class, loader->class))
            return INST_DATA(loader, Object*, ldr_parent_offset);
    }

    return loader;
}

char *classlibDefaultBootClassPath() {
    static char *entries[] = {"lib/resources.jar",
                              "lib/rt.jar",
                              "lib/sunrsasign.jar",
                              "lib/jsse.jar",
                              "lib/jce.jar",
                              "lib/charsets.jar",
                              "classes",
                              NULL};
    char *java_home = getJavaHome();
    char *path, *pntr;
    int i, j, len = 0;

    for(i = 0; entries[i] != NULL; i++)
        len += strlen(entries[i]);

    if(i == 0)
        return "";

    pntr = path = sysMalloc(len + i * (strlen(java_home) + 2));

    for(j = 0; j < i - 1; j++)
        pntr += sprintf(pntr, "%s/%s:", java_home, entries[j]);

    sprintf(pntr, "%s/%s", java_home, entries[j]);
    
    return path;
}

char *classlibDefaultExtDirs() {
    char *java_home = getJavaHome();
    char *ext_dirs = sysMalloc(strlen(java_home) + 
                               sizeof("/lib/ext:/usr/java/packages/lib/ext"));

    return strcat(strcpy(ext_dirs, java_home),
                  "/lib/ext:/usr/java/packages/lib/ext");
}

char *classlibDefaultEndorsedDirs() {
    char *java_home = getJavaHome();
    char *endorsed_dirs = sysMalloc(strlen(java_home) +
                                    sizeof("/lib/endorsed"));

    return strcat(strcpy(endorsed_dirs, java_home), "/lib/endorsed");
}

char *classlibExternalClassName(Class *class) {
    ClassBlock *cb = CLASS_CB(class);
    char *dot_name = slash2DotsDup(cb->name);

    if(cb->host_class != NULL) {
        char buff[21];
        int len = strlen(dot_name);
        uint64_t hash = getObjectHashcode(class);
        int hash_len = sprintf(buff, "%llu", hash);

        dot_name = sysRealloc(dot_name, len + hash_len + 2);
        memcpy(dot_name + len + 1, buff, hash_len + 1);
        dot_name[len] = '/';
    }

    return dot_name;
}

Class *findClassFromLoader(char *name, int init, Object *loader,
                           int throw_error) {

    Class *class = findClassFromClassLoader(name, loader);

    if(class == NULL) {
        if(!throw_error) {
            Object *excep = exceptionOccurred();
            char *dot_name = slash2DotsDup(name);

            clearException();
            signalChainedException(java_lang_ClassNotFoundException,
                                   dot_name, excep);
            sysFree(dot_name);
        }
    } else if(init)
        initClass(class);

    return class;
}
