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
#include "hash.h"
#include "class.h"
#include "symbol.h"

/* Cached offset of vmdata field in java.lang.ClassLoader objects */
int ldr_vmdata_offset;

static MethodBlock *ldr_new_unloader = NULL;
static int ldr_data_tbl_offset;

/* Helper method to create a Package Object representing a
   package loaded by the boot loader */
static MethodBlock *vm_loader_create_package = NULL;
static Class *package_array_class;

#define CLASS_INITSZE 1<<8

void classlibCacheClassLoaderFields(Class *loader_class) {
    FieldBlock *ldr_fb = findField(loader_class, SYMBOL(vmdata),
                                                 SYMBOL(sig_java_lang_Object));

    if(ldr_fb == NULL) {
        jam_fprintf(stderr, "Expected \"vmdata\" field missing in "
                            "java.lang.ClassLoader\n");
        exitVM(1);
    }

    ldr_vmdata_offset = ldr_fb->u.offset;
}

HashTable *classlibLoaderTable(Object *class_loader) {
    Object *vmdata = INST_DATA(class_loader, Object*, ldr_vmdata_offset);

    if(vmdata == NULL)
        return NULL;

    return INST_DATA(vmdata, HashTable*, ldr_data_tbl_offset);
}

HashTable *classlibCreateLoaderTable(Object *class_loader) {
    Object *vmdata = allocObject(ldr_new_unloader->class);
    HashTable *table;

    if(vmdata == NULL)
        return NULL;

    table = sysMalloc(sizeof(HashTable));
    initHashTable((*table), CLASS_INITSZE, TRUE);

    INST_DATA(vmdata, HashTable*, ldr_data_tbl_offset) = table;
    INST_DATA(class_loader, Object*, ldr_vmdata_offset) = vmdata;

    return table;
}

Object *createBootPackage(PackageEntry *package_entry) {
    Object *name = createString(package_entry->name);

    if(name != NULL) {
        Object *package = *(Object**)executeStaticMethod(
                                            vm_loader_create_package->class,
                                            vm_loader_create_package, name,
                                            package_entry->index);

        if(!exceptionOccurred()) 
            return package;
    }

    return NULL;
}

Object *classlibBootPackage(PackageEntry *package_entry) {
    return createBootPackage(package_entry);
}

Object *classlibBootPackages(PackageEntry *package_entry) {
    return createBootPackage(package_entry);
}

Class *classlibBootPackagesArrayClass() {
    return package_array_class;
}

/* The default value of the boot classpath is based on the JamVM
   and Classpath install directories.  If zip support is enabled
   the classes will be contained in ZIP files, else they will be
   separate class files in a directory structure */

#ifdef USE_ZIP
#define JAMVM_CLASSES INSTALL_DIR"/share/jamvm/classes.zip"
#define CLASSPATH_CLASSES CLASSPATH_INSTALL_DIR"/share/classpath/glibj.zip"
#else
#define JAMVM_CLASSES INSTALL_DIR"/share/jamvm/classes"
#define CLASSPATH_CLASSES CLASSPATH_INSTALL_DIR"/share/classpath"
#endif

#define DFLT_BCP JAMVM_CLASSES":"CLASSPATH_CLASSES

char *classlibBootClassPathOpt(InitArgs *args) {
    char *vm_path = args->bootpath_v != NULL ? args->bootpath_v
                                             : JAMVM_CLASSES;
    char *cp_path = args->bootpath_c != NULL ? args->bootpath_c
                                             : CLASSPATH_CLASSES;
        
    char *bootpath = sysMalloc(strlen(vm_path) + strlen(cp_path) + 2);

    return strcat(strcat(strcpy(bootpath, vm_path), ":"), cp_path);
}

char *classlibDefaultBootClassPath() {
    return DFLT_BCP;
}

char *classlibDefaultExtDirs() {
    return INSTALL_DIR"/share/jamvm/ext";
}

char *classlibDefaultEndorsedDirs() {
    return INSTALL_DIR"/share/jamvm/endorsed";
}

/* Add a library unloader object to the class loader for the
   library contained within entry.  The library has an unload
   function, which will be called from the unloader finalizer
   when the class loader is garbage collected */
void classlibNewLibraryUnloader(Object *class_loader, void *entry) {
    Object *vmdata = INST_DATA(class_loader, Object*, ldr_vmdata_offset);
    
    if(vmdata != NULL)
        executeMethod(vmdata, ldr_new_unloader, (long long)(uintptr_t)entry);
}

int classlibInitialiseClass() {
    FieldBlock *hashtable = NULL;
    Class *loader_data_class;
    Class *vm_loader_class;

    loader_data_class = findSystemClass0(SYMBOL(jamvm_java_lang_VMClassLoaderData));
    if(loader_data_class != NULL) {
        ldr_new_unloader = findMethod(loader_data_class,
                                      SYMBOL(newLibraryUnloader),
                                      SYMBOL(_J__V));
        hashtable = findField(loader_data_class, SYMBOL(hashtable), SYMBOL(J));
    }

    if(hashtable == NULL || ldr_new_unloader == NULL) {
        jam_fprintf(stderr, "Fatal error: Bad VMClassLoaderData (missing or corrupt)\n");
        return FALSE;
    }
    ldr_data_tbl_offset = hashtable->u.offset;

    vm_loader_class = findSystemClass0(SYMBOL(java_lang_VMClassLoader));
    if(vm_loader_class != NULL)
       vm_loader_create_package =
                  findMethod(vm_loader_class, SYMBOL(createBootPackage),
                             SYMBOL(_java_lang_String_I__java_lang_Package));

    if(vm_loader_create_package == NULL) {
        jam_fprintf(stderr, "Fatal error: Bad java.lang.VMClassLoader (missing or corrupt)\n");
        return FALSE;
    }

    package_array_class = findArrayClass(SYMBOL(array_java_lang_Package));
    registerStaticClassRef(&package_array_class);

    if(package_array_class == NULL) {
        jam_fprintf(stderr, "Fatal error: missing java.lang.Package\n");
        return FALSE;
    }

    return TRUE;
}

