/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011,
 * 2012, 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
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
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stddef.h>

#include "jam.h"
#include "sig.h"
#include "thread.h"
#include "lock.h"
#include "hash.h"
#include "zip.h"
#include "class.h"
#include "interp.h"
#include "symbol.h"
#include "excep.h"
#include "classlib.h"

#define PREPARE(ptr) ptr
#define SCAVENGE(ptr) FALSE
#define FOUND(ptr1, ptr2) ptr2

static int verbose;
static char *bootpath;
static char *classpath;
static int max_cp_element_len;

/* Structures holding the boot loader classpath */
typedef struct bcp_entry {
    char *path;
    ZipFile *zip;
} BCPEntry;

static BCPEntry *bootclasspath;
static int bcp_entries;

/* Cached offsets of fields in java.lang.ref.Reference objects */
int ref_referent_offset = -1;
int ref_queue_offset;

/* hash table containing packages loaded by the boot loader */
#define PCKG_INITSZE 1<<6
static HashTable boot_packages;

/* Instance of java.lang.Class for java.lang.Class */
Class *java_lang_Class = NULL;

/* Method table index of ClassLoader.loadClass - used when
   requesting a Java-level class loader to load a class.
   Cached on first use. */
static int loadClass_mtbl_idx = -1;

/* Method table index of finalizer method and ClassLoader.enqueue.
   Used by finalizer and reference handler threads */
int finalize_mtbl_idx;
int enqueue_mtbl_idx;

/* hash table containing classes loaded by the boot loader and
   internally created arrays */
#define CLASS_INITSZE 1<<8
static HashTable boot_classes;

/* Array large enough to hold all primitive classes -
 * access protected by boot_classes hash table lock */
#define MAX_PRIM_CLASSES 9
static Class *prim_classes[MAX_PRIM_CLASSES];

/* Bytecode for stub abstract method.  If it is invoked
   we'll get an abstract method error. */
static char abstract_method[] = {OPC_ABSTRACT_METHOD_ERROR};

/* Bytecode for a Miranda method.  If it is invoked it executes
   the unimplemented interface method that it represents. */
static char miranda_bridge[] = {OPC_MIRANDA_BRIDGE};

static Class *addClassToHash(Class *class, Object *class_loader) {
    HashTable *table;
    Class *entry;

#define HASH(ptr) utf8Hash(CLASS_CB((Class *)ptr)->name)
#define COMPARE(ptr1, ptr2, hash1, hash2) (hash1 == hash2) && \
            CLASS_CB((Class *)ptr1)->name == CLASS_CB((Class *)ptr2)->name

    if(class_loader == NULL)
        table = &boot_classes;
    else {
        table = classlibLoaderTable(class_loader);

        if(table == NULL) {
            table = classlibCreateLoaderTable(class_loader);

            if(table == NULL)
                return NULL;
        }
    }

    /* Add if absent, no scavenge, locked */
    findHashEntry((*table), class, entry, TRUE, FALSE, TRUE);

    return entry;
}

static void prepareClass(Class *class) {
    ClassBlock *cb = CLASS_CB(class);

    if(cb->name == SYMBOL(java_lang_Class)) {
       java_lang_Class = class->class = class;
       cb->flags |= CLASS_CLASS;
    } else {
       if(java_lang_Class == NULL)
          findSystemClass0(SYMBOL(java_lang_Class));
       class->class = java_lang_Class;
    }
}

#ifdef CLASSLIB_METHOD_ANNOTATIONS
/* Forward declaration */
u1 *skipAnnotation(u1 *data_ptr, int *data_len);

u1 *skipElementValue(u1 *data_ptr, int *data_len) {
    char tag;

    READ_U1(tag, data_ptr, *data_len);

    switch(tag) {
        case 'e':
            SKIP_U2(idx, data_ptr, *data_len);
            /* Fall through */

        case 'Z': case 'B': case 'C': case 'S': case 'I':
        case 'F': case 'J': case 'D': case 's': case 'c':
            SKIP_U2(idx, data_ptr, *data_len);
            break;

        case '@':
            data_ptr = skipAnnotation(data_ptr, data_len);
            break;

        case '[': {
            int i, num_values;

            READ_U2(num_values, data_ptr, *data_len);

            for(i = 0; i < num_values; i++)
                data_ptr = skipElementValue(data_ptr, data_len);

            break;
        }
    }

    return data_ptr;
}

u1 *skipAnnotation(u1 *data_ptr, int *data_len) {
    int no_value_pairs, i;

    SKIP_U2(type_idx, data_ptr, *data_len);
    READ_U2(no_value_pairs, data_ptr, *data_len);

    for(i = 0; i < no_value_pairs; i++) {
        SKIP_U2(element_name_idx, data_ptr, *data_len);
        data_ptr = skipElementValue(data_ptr, data_len);
    }

    return data_ptr;
}

void parseMethodAnnotations(ConstantPool *cp, MethodBlock *mb,
                            u1 *data_ptr, int data_len) {
    int no_annos, i;

    READ_U2(no_annos, data_ptr, data_len);

    for(i = 0; i < no_annos; i++) {
        u1 *ptr = data_ptr;
        char *type_name;
        int type_idx;

        data_ptr = skipAnnotation(data_ptr, &data_len);

        READ_TYPE_INDEX(type_idx, cp, CONSTANT_Utf8, ptr, 2);
        type_name = findUtf8(CP_UTF8(cp, type_idx));

        if(type_name != NULL)
            CLASSLIB_METHOD_ANNOTATIONS(mb, type_name);
    }
}
#else
void parseMethodAnnotations(ConstantPool *cp, MethodBlock *mb,
                            u1 *data_ptr, int data_len) {
}
#endif

static void setIndexedAttributeData(AttributeData ***attributes,
                                    int index, u1 *data, int len,
                                    int size) {
    if(*attributes == NULL) {
        *attributes = sysMalloc(size * sizeof(AttributeData*));
        memset(*attributes, 0, size * sizeof(AttributeData*));
    }

    (*attributes)[index] = sysMalloc(sizeof(AttributeData));
    (*attributes)[index]->len = len;
    (*attributes)[index]->data = sysMalloc(len);
    memcpy((*attributes)[index]->data, data, len);
}

#define setIndexedAttribute(attributes, index, data, len, size) \
    setIndexedAttributeData(&attributes, index, data, len, size)

#define setSingleAttribute(attributes, pntr, length) \
    attributes = sysMalloc(sizeof(AttributeData));   \
    attributes->len = length;                        \
    attributes->data = sysMalloc(length);            \
    memcpy(attributes->data, pntr, length);

Class *parseClass(char *classname, char *data, int offset, int len,
                   Object *class_loader) {

    u2 major_version, minor_version, this_idx, super_idx, attr_count;
    int cp_count, intf_count, injected_fields_count, i, j;
    ExtraAttributes extra_attributes;
    u1 *ptr = (u1 *)data + offset;
    ConstantPool *constant_pool;
    Class **interfaces, *class;
    ClassBlock *classblock;
    u4 magic;

    READ_U4(magic, ptr, len);

    if(magic != 0xcafebabe) {
       signalException(java_lang_ClassFormatError, "bad magic");
       return NULL;
    }

    READ_U2(minor_version, ptr, len);
    READ_U2(major_version, ptr, len);

    if((class = allocClass()) == NULL)
        return NULL;

    classblock = CLASS_CB(class);
    READ_U2(cp_count, ptr, len);

    constant_pool = &classblock->constant_pool;
    constant_pool->type = sysMalloc(cp_count);
    constant_pool->info = sysMalloc(cp_count * sizeof(ConstantPoolEntry));

    for(i = 1; i < cp_count; i++) {
        u1 tag;

        READ_U1(tag, ptr, len);
        CP_TYPE(constant_pool, i) = tag;

        switch(tag) {
           case CONSTANT_Class:
           case CONSTANT_String:
           case CONSTANT_MethodType:
               READ_INDEX(CP_INFO(constant_pool, i), ptr, len);
               break;

           case CONSTANT_Fieldref:
           case CONSTANT_Methodref:
           case CONSTANT_NameAndType:
           case CONSTANT_InvokeDynamic:
           case CONSTANT_InterfaceMethodref:
           {
               u2 idx1, idx2;

               READ_INDEX(idx1, ptr, len);
               READ_INDEX(idx2, ptr, len);
               CP_INFO(constant_pool, i) = (idx2<<16) + idx1;
               break;
           }

           case CONSTANT_MethodHandle:
           {
               u1 kind;
               u2 idx;

               READ_U1(kind, ptr, len);
               READ_INDEX(idx, ptr, len);
               CP_INFO(constant_pool, i) = (idx<<16) + kind;
               break;
           }

           case CONSTANT_Float:
           case CONSTANT_Integer:
               READ_U4(CP_INFO(constant_pool, i), ptr, len);
               break;

           case CONSTANT_Long:
               READ_U8(*(u8 *)&(CP_INFO(constant_pool, i)), ptr, len);
               CP_TYPE(constant_pool, ++i) = 0;
               break;
               
           case CONSTANT_Double:
               READ_DBL(*(u8 *)&(CP_INFO(constant_pool, i)), ptr, len);
               CP_TYPE(constant_pool, ++i) = 0;
               break;

           case CONSTANT_Utf8:
           {
               int length;
               char *buff, *utf8;

               READ_U2(length, ptr, len);
               buff = sysMalloc(length + 1);

               memcpy(buff, ptr, length);
               buff[length] = '\0';
               ptr += length;

               CP_INFO(constant_pool, i) = (uintptr_t) (utf8 = newUtf8(buff));

               if(utf8 != buff)
                   sysFree(buff);

               break;
           }

           default:
               signalException(java_lang_ClassFormatError,
                               "bad constant pool tag");
               return NULL;
        }
    }

    /* Set count after constant pool has been initialised -- it is now
       safe to be scanned by GC */
    classblock->constant_pool_count = cp_count;

    READ_U2(classblock->access_flags, ptr, len);

    READ_TYPE_INDEX(this_idx, constant_pool, CONSTANT_Class, ptr, len);
    classblock->name = CP_UTF8(constant_pool,
                               CP_CLASS(constant_pool, this_idx));

    if(classname && strcmp(classblock->name, classname) != 0) {
        signalException(java_lang_NoClassDefFoundError,
                        "class file has wrong name");
        return NULL;
    }

    prepareClass(class);

    if(classblock->name == SYMBOL(java_lang_Object)) {
        READ_U2(super_idx, ptr, len);
        if(super_idx) {
           signalException(java_lang_ClassFormatError, "Object has super");
           return NULL;
        }
    } else {
        READ_TYPE_INDEX(super_idx, constant_pool, CONSTANT_Class, ptr, len);
    }

    classblock->class_loader = class_loader;

    READ_U2(intf_count = classblock->interfaces_count, ptr, len);
    interfaces = classblock->interfaces =
                         sysMalloc(intf_count * sizeof(Class *));

    memset(interfaces, 0, intf_count * sizeof(Class *));
    for(i = 0; i < intf_count; i++) {
       u2 index;
       READ_TYPE_INDEX(index, constant_pool, CONSTANT_Class, ptr, len);
       interfaces[i] = resolveClass(class, index, FALSE, FALSE);
       if(exceptionOccurred())
           return NULL; 
    }

    memset(&extra_attributes, 0, sizeof(ExtraAttributes));

    READ_U2(classblock->fields_count, ptr, len);
    injected_fields_count = classlibInjectedFieldsCount(classblock->name);
    classblock->fields_count += injected_fields_count;
    classblock->fields = sysMalloc(classblock->fields_count *
                                   sizeof(FieldBlock));

    if(injected_fields_count != 0)
        classlibFillInInjectedFields(classblock->name, classblock->fields);

    for(i = injected_fields_count; i < classblock->fields_count; i++) {
        FieldBlock *field = &classblock->fields[i];
        u2 name_idx, type_idx;

        READ_U2(field->access_flags, ptr, len);
        READ_TYPE_INDEX(name_idx, constant_pool, CONSTANT_Utf8, ptr, len);
        READ_TYPE_INDEX(type_idx, constant_pool, CONSTANT_Utf8, ptr, len);
        field->name = CP_UTF8(constant_pool, name_idx);
        field->type = CP_UTF8(constant_pool, type_idx);
        field->signature = NULL;
        field->constant = 0;

        for(j = 0; j < injected_fields_count; j++)
            if(field->name == classblock->fields[j].name) {
                jam_fprintf(stderr, "Classlib mismatch: injected field "
                                    "\"%s\" already present in %s\n",
                                    field->name, classblock->name);
                exitVM(1);
            }

        READ_U2(attr_count, ptr, len);
        for(; attr_count != 0; attr_count--) {
            u2 attr_name_idx;
            char *attr_name;
            u4 attr_length;

            READ_TYPE_INDEX(attr_name_idx, constant_pool, CONSTANT_Utf8,
                            ptr, len);
            attr_name = CP_UTF8(constant_pool, attr_name_idx);
            READ_U4(attr_length, ptr, len);

            if(attr_name == SYMBOL(ConstantValue)) {
                READ_INDEX(classblock->fields[i].constant, ptr, len);

            } else if(attr_name == SYMBOL(Signature)) {
                u2 signature_idx;
                READ_TYPE_INDEX(signature_idx, constant_pool, CONSTANT_Utf8,
                                ptr, len);
                field->signature = CP_UTF8(constant_pool, signature_idx);

            } else if(attr_name == SYMBOL(RuntimeVisibleAnnotations)) {
                setIndexedAttribute(extra_attributes.field_annos,
                                    field - classblock->fields, ptr,
                                    attr_length, classblock->fields_count); 
                ptr += attr_length;
#ifdef JSR308
            } else if(attr_name == SYMBOL(RuntimeVisibleTypeAnnotations)) {
                setIndexedAttribute(extra_attributes.field_type_annos,
                                    field - classblock->fields, ptr,
                                    attr_length, classblock->fields_count); 
                ptr += attr_length;
#endif
            } else
                ptr += attr_length;
        }
    }

    READ_U2(classblock->methods_count, ptr, len);
    classblock->methods = sysMalloc(classblock->methods_count *
                                    sizeof(MethodBlock));
    memset(classblock->methods, 0, classblock->methods_count *
                                   sizeof(MethodBlock));

    for(i = 0; i < classblock->methods_count; i++) {
        MethodBlock *method = &classblock->methods[i];
        u2 name_idx, type_idx;

        READ_U2(method->access_flags, ptr, len);
        READ_TYPE_INDEX(name_idx, constant_pool, CONSTANT_Utf8, ptr, len);
        READ_TYPE_INDEX(type_idx, constant_pool, CONSTANT_Utf8, ptr, len);

        method->name = CP_UTF8(constant_pool, name_idx);
        method->type = CP_UTF8(constant_pool, type_idx);

        READ_U2(attr_count, ptr, len);
        for(; attr_count != 0; attr_count--) {
            u2 attr_name_idx;
            char *attr_name;
            u4 attr_length;

            READ_TYPE_INDEX(attr_name_idx, constant_pool, CONSTANT_Utf8,
                            ptr, len);
            READ_U4(attr_length, ptr, len);
            attr_name = CP_UTF8(constant_pool, attr_name_idx);

            if(attr_name == SYMBOL(Code)) {
                u4 code_length;
                u2 code_attr_cnt;
                int j;

                READ_U2(method->max_stack, ptr, len);
                READ_U2(method->max_locals, ptr, len);

                READ_U4(code_length, ptr, len);
                method->code = sysMalloc(code_length);
                memcpy(method->code, ptr, code_length);
                ptr += code_length;

                method->code_size = code_length;

                READ_U2(method->exception_table_size, ptr, len);
                method->exception_table =
                        sysMalloc(method->exception_table_size *
                                  sizeof(ExceptionTableEntry));

                for(j = 0; j < method->exception_table_size; j++) {
                    ExceptionTableEntry *entry = &method->exception_table[j];              
                    READ_U2(entry->start_pc, ptr, len);
                    READ_U2(entry->end_pc, ptr, len);
                    READ_U2(entry->handler_pc, ptr, len);
                    READ_U2(entry->catch_type, ptr, len);
                }

                READ_U2(code_attr_cnt, ptr, len);
                for(; code_attr_cnt != 0; code_attr_cnt--) {
                    u2 attr_name_idx;
                    u4 attr_length;

                    READ_TYPE_INDEX(attr_name_idx, constant_pool,
                                    CONSTANT_Utf8, ptr, len);
                    attr_name = CP_UTF8(constant_pool, attr_name_idx);
                    READ_U4(attr_length, ptr, len);

                    if(attr_name == SYMBOL(LineNumberTable)) {
                        READ_U2(method->line_no_table_size, ptr, len);
                        method->line_no_table =
                                sysMalloc(method->line_no_table_size *
                                          sizeof(LineNoTableEntry));

                        for(j = 0; j < method->line_no_table_size; j++) {
                            LineNoTableEntry *entry = &method->line_no_table[j];
                         
                            READ_U2(entry->start_pc, ptr, len);
                            READ_U2(entry->line_no, ptr, len);
                        }
                    } else
                        ptr += attr_length;
                }
            } else if(attr_name == SYMBOL(Exceptions)) {
                int j;

                READ_U2(method->throw_table_size, ptr, len);
                method->throw_table = sysMalloc(method->throw_table_size *
                                                sizeof(u2));
                for(j = 0; j < method->throw_table_size; j++) {
                    READ_U2(method->throw_table[j], ptr, len);
                }

            } else if(attr_name == SYMBOL(Signature)) {
                u2 signature_idx;
                READ_TYPE_INDEX(signature_idx, constant_pool, CONSTANT_Utf8,
                                ptr, len);
                method->signature = CP_UTF8(constant_pool, signature_idx);

            } else if(attr_name == SYMBOL(RuntimeVisibleAnnotations)) {
                setIndexedAttribute(extra_attributes.method_annos,
                                    method - classblock->methods, ptr,
                                    attr_length, classblock->methods_count); 

                parseMethodAnnotations(constant_pool, method, ptr,
                                       attr_length);
                ptr += attr_length;

            } else if(attr_name == SYMBOL(RuntimeVisibleParameterAnnotations)) {
                setIndexedAttribute(extra_attributes.method_parameter_annos,
                                    method - classblock->methods, ptr,
                                    attr_length, classblock->methods_count); 
                ptr += attr_length;

            } else if(attr_name == SYMBOL(AnnotationDefault)) {
                setIndexedAttribute(extra_attributes.method_anno_default_val,
                                    method - classblock->methods, ptr,
                                    attr_length, classblock->methods_count); 
                ptr += attr_length;
#ifdef JSR308
            } else if(attr_name == SYMBOL(RuntimeVisibleTypeAnnotations)) {
                setIndexedAttribute(extra_attributes.method_type_annos,
                                    method - classblock->methods, ptr,
                                    attr_length, classblock->methods_count); 
                ptr += attr_length;
#endif
#ifdef JSR901
            } else if(attr_name == SYMBOL(MethodParameters)) {
                setIndexedAttribute(extra_attributes.method_parameters,
                                    method - classblock->methods, ptr,
                                    attr_length, classblock->methods_count); 
                ptr += attr_length;
#endif
            } else
                ptr += attr_length;
        }
    }

    READ_U2(attr_count, ptr, len);
    for(; attr_count != 0; attr_count--) {
        u2 attr_name_idx;
        char *attr_name;
        u4 attr_length;

        READ_TYPE_INDEX(attr_name_idx, constant_pool, CONSTANT_Utf8, ptr, len);
        attr_name = CP_UTF8(constant_pool, attr_name_idx);
        READ_U4(attr_length, ptr, len);

        if(attr_name == SYMBOL(SourceFile)) {
            u2 file_name_idx;
            READ_TYPE_INDEX(file_name_idx, constant_pool, CONSTANT_Utf8,
                            ptr, len);
            classblock->source_file_name =
                                  CP_UTF8(constant_pool, file_name_idx);

        } else if(attr_name == SYMBOL(InnerClasses)) {
            int j, size;
            READ_U2(size, ptr, len);
            {
                u2 inner_classes[size];
                for(j = 0; j < size; j++) {
                    int inner, outer;
                    READ_TYPE_INDEX(inner, constant_pool, CONSTANT_Class,
                                    ptr, len);
                    READ_TYPE_INDEX(outer, constant_pool, CONSTANT_Class,
                                    ptr, len);

                    if(inner == this_idx) {
                        int inner_name_idx;

                        classblock->declaring_class = outer;

                        READ_TYPE_INDEX(inner_name_idx, constant_pool,
                                        CONSTANT_Utf8, ptr, len);
                        if(inner_name_idx == 0)
                            classblock->flags |= ANONYMOUS;

                        READ_U2(classblock->inner_access_flags, ptr, len);
                    } else {
                        ptr += 4;
                        if(outer == this_idx)
                            inner_classes[classblock->inner_class_count++]
                                     = inner;
                    }
                }

                if(classblock->inner_class_count) {
                    classblock->inner_classes =
                                sysMalloc(classblock->inner_class_count *
                                          sizeof(u2));
                    memcpy(classblock->inner_classes, &inner_classes[0],
                           classblock->inner_class_count * sizeof(u2));
                }
            }
        } else if(attr_name == SYMBOL(EnclosingMethod)) {
            READ_TYPE_INDEX(classblock->enclosing_class, constant_pool,
                            CONSTANT_Class, ptr, len);
            READ_TYPE_INDEX(classblock->enclosing_method, constant_pool,
                            CONSTANT_NameAndType, ptr, len);

        } else if(attr_name == SYMBOL(Signature)) {
            u2 signature_idx;
            READ_TYPE_INDEX(signature_idx, constant_pool, CONSTANT_Utf8,
                            ptr, len);
            classblock->signature = CP_UTF8(constant_pool, signature_idx);

        } else if(attr_name == SYMBOL(Synthetic))
            classblock->access_flags |= ACC_SYNTHETIC;

        else if(attr_name == SYMBOL(RuntimeVisibleAnnotations)) {
            setSingleAttribute(extra_attributes.class_annos,
                               ptr, attr_length);
            ptr += attr_length;
#ifdef JSR308
        } else if(attr_name == SYMBOL(RuntimeVisibleTypeAnnotations)) {
            setSingleAttribute(extra_attributes.class_type_annos,
                               ptr, attr_length);
            ptr += attr_length;
#endif
#ifdef JSR292
        } else if(attr_name == SYMBOL(BootstrapMethods)) {
            int num_methods, *offsets;
            u2 *indexes;
            char *data;

            READ_U2(num_methods, ptr, len);

            data = sysMalloc(attr_length + num_methods*2 + 2);
            indexes = (u2*)(data + num_methods*4 + 4);
            offsets = (int*)data;

            for(; num_methods != 0; num_methods--) {
                int method_ref, num_args;
                READ_U2(method_ref, ptr, len);
                READ_U2(num_args, ptr, len);

                *offsets++ = (char*)indexes - data;
                *indexes++ = method_ref;

                for(; num_args != 0; num_args--) {
                    int arg_idx;
                    READ_U2(arg_idx, ptr, len);
                    *indexes++ = arg_idx;
                }
            }

            *offsets++ = (char*)indexes - data;
            classblock->bootstrap_methods = data;
#endif
        } else
            ptr += attr_length;
    }

    for(i = 0; i < sizeof(ExtraAttributes)/sizeof(void*)
                      && extra_attributes.data[i] == NULL; i++);

    if(i < sizeof(ExtraAttributes)/sizeof(void*)) {
        classblock->extra_attributes = sysMalloc(sizeof(ExtraAttributes));
        memcpy(classblock->extra_attributes, &extra_attributes,
                                             sizeof(ExtraAttributes));
    }

    if(super_idx) {
        classblock->super = resolveClass(class, super_idx, FALSE, FALSE);
        if(exceptionOccurred())
           return NULL;
    }
    
    classblock->state = CLASS_LOADED;
    return class;
}

Class *defineClass(char *classname, char *data, int offset, int len,
                   Object *class_loader) {

    Class *class = parseClass(classname, data, offset, len, class_loader);

    if(class != NULL) {
        Class *found = addClassToHash(class, class_loader);

        if(found != class) {
            CLASS_CB(class)->flags = CLASS_CLASH;
            if(class_loader != NULL) {
                signalException(java_lang_LinkageError,
                                "duplicate class definition");
                return NULL;
            }
            return found;
        }
    }

    return class;
}

Class *createArrayClass(char *classname, Object *class_loader) {
    Class *comp_class, *elem_class, *class, *found = NULL;
    ClassBlock *elem_cb, *classblock;
    int dim;

    class = allocClass();
    if(class == NULL)
        return NULL;

    classblock = CLASS_CB(class);

    classblock->name = copyUtf8(classname);
    classblock->super = findSystemClass0(SYMBOL(java_lang_Object));
    classblock->method_table = CLASS_CB(classblock->super)->method_table;

    classblock->interfaces_count = 2;
    classblock->interfaces = sysMalloc(sizeof(Class*) * 2);
    classblock->interfaces[0] = findSystemClass0(SYMBOL(java_lang_Cloneable));
    classblock->interfaces[1] = findSystemClass0(SYMBOL(java_io_Serializable));

    classblock->state = CLASS_ARRAY;

    /* Find the array element class and the dimension --
       this is used to speed up type checking (instanceof) */

    if(classname[1] == '[') {
        comp_class = findArrayClassFromClassLoader(classname + 1,
                                                   class_loader);
        if(comp_class == NULL)
            goto error;

        elem_class = CLASS_CB(comp_class)->element_class;
        dim = CLASS_CB(comp_class)->dim + 1;
    } else { 
        if(classname[1] == 'L') {
            int len = strlen(classname);
            char element_name[len - 2];

            memcpy(element_name, classname + 2, len - 3);
            element_name[len - 3] = '\0';

            elem_class = findClassFromClassLoader(element_name, class_loader);
        } else
            elem_class = findPrimitiveClass(classname[1]);

        if(elem_class == NULL)
            goto error;

        comp_class = elem_class;
        dim = 1;
    }

    classblock->component_class = comp_class;
    classblock->element_class = elem_class;
    classblock->dim = dim;

    elem_cb = CLASS_CB(elem_class);

    /* The array's classloader is the loader of the element class */
    classblock->class_loader = elem_cb->class_loader;

    /* The array's visibility (i.e. public, etc.) is that of the element */
    classblock->access_flags = (elem_cb->access_flags & ~ACC_INTERFACE) |
                               ACC_FINAL | ACC_ABSTRACT;

    prepareClass(class);

    found = addClassToHash(class, classblock->class_loader);
    if(found == class) {
        if(verbose)
            jam_printf("[Created array class %s]\n", classname);
        return class;
    }

error:
    classblock->flags = CLASS_CLASH;
    return found;
}

Class *createPrimClass(char *classname, int index) {
    Class *class;
    ClassBlock *classblock;
 
    if((class = allocClass()) == NULL)
        return NULL;

    classblock = CLASS_CB(class);
    classblock->name = classname;
    classblock->state = CLASS_PRIM + index;
    classblock->access_flags = ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT;

    prepareClass(class);

    lockHashTable(boot_classes);
    if(prim_classes[index] == NULL)
        prim_classes[index] = class;
    unlockHashTable(boot_classes);

    if(verbose)
        jam_printf("[Created primitive class %s]\n", classname);

    return prim_classes[index];
}

/* Layout the instance data.

   The object layout places 64-bit fields on a double-word boundary
   as on some architectures this leads to better performance (and
   misaligned loads/store may cause traps).

   Reference fields are also placed together as these must be scanned
   during GC, and placing them together reduces the number of entries
   required in the reference offsets list.

   Double/long fields are layed out first, then references and finally
   int-sized fields.  When padding is needed for 64-bit fields we try
   to place an int-sized field, and only leave a hole when no int-sized
   fields are available */

void prepareFields(Class *class) {
    ClassBlock *cb = CLASS_CB(class);
    Class *super = (cb->access_flags & ACC_INTERFACE) ? NULL
                                                      : cb->super;
    RefsOffsetsEntry *spr_rfs_offsts_tbl = NULL;
    int spr_rfs_offsts_sze = 0;

    FieldBlock *ref_head = NULL;
    FieldBlock *int_head = NULL;
    FieldBlock *dbl_head = NULL;

    int field_offset = sizeof(Object);
    int refs_start_offset = 0;
    int refs_end_offset = 0;
    int i;

    if(super != NULL) {
        field_offset = CLASS_CB(super)->object_size;
        spr_rfs_offsts_sze = CLASS_CB(super)->refs_offsets_size;
        spr_rfs_offsts_tbl = CLASS_CB(super)->refs_offsets_table;
    }

    /* Initialise static fields to default value, and separate
       instance fields into three linked lists, holding
       int-sized fields, double-sized fields and reference
       fields */

    for(i = 0; i < cb->fields_count; i++) {
        FieldBlock *fb = &cb->fields[i];

        if(fb->access_flags & ACC_STATIC)
            fb->u.static_value.l = 0;
        else {
            FieldBlock **list;

            if(fb->type[0] == 'L' || fb->type[0] == '[')
                list = &ref_head;
            else
                if(fb->type[0] == 'J' || fb->type[0] == 'D')
                    list = &dbl_head;
                else
                    list = &int_head;

            fb->u.static_value.p = *list;
            *list = fb;
        }

        fb->class = class;
    }

    /* Layout the double-sized fields.  If padding is required,
       use the first int-sized field (int_list head), or leave
       a hole if no int-fields */

    if(dbl_head != NULL) {
        if(field_offset & 0x7) {
            if(int_head != NULL) {
                FieldBlock *fb = int_head;
                int_head = int_head->u.static_value.p;
                fb->u.offset = field_offset;
            }
            field_offset += 4;
        }

        do {
            FieldBlock *fb = dbl_head;
            dbl_head = dbl_head->u.static_value.p;
            fb->u.offset = field_offset;
            field_offset += 8;
        } while(dbl_head != NULL);
    }

    /* Layout the reference fields.  If padding is required,
       use an int-sized field (int_list head), or leave
       a hole if no int-fields remaining */

    if(ref_head != NULL) {
        if(sizeof(Object*) == 8 && field_offset & 0x7) {
            if(int_head != NULL) {
                FieldBlock *fb = int_head;
                int_head = int_head->u.static_value.p;
                fb->u.offset = field_offset;
            }
            field_offset += 4;
        }

        refs_start_offset = field_offset;

        do {
            FieldBlock *fb = ref_head;
            ref_head = ref_head->u.static_value.p;
            fb->u.offset = field_offset;
            field_offset += sizeof(Object*);
        } while(ref_head != NULL);

        refs_end_offset = field_offset;
    }

    /* Layout the remaining int-sized fields */

    while(int_head != NULL) {
        FieldBlock *fb = int_head;
        int_head = int_head->u.static_value.p;
        fb->u.offset = field_offset;
        field_offset += 4;
    }

   cb->object_size = field_offset;

   /* Construct the reference offsets list.  This is used to speed up
      scanning of an objects references during the mark phase of GC.
      If possible, merge the entry with the previous entry */

   if(refs_start_offset) {
       if(spr_rfs_offsts_sze > 0 && spr_rfs_offsts_tbl[spr_rfs_offsts_sze-1].end
                                           == refs_start_offset) {

           cb->refs_offsets_size = spr_rfs_offsts_sze;
           refs_start_offset = spr_rfs_offsts_tbl[spr_rfs_offsts_sze-1].start;
       } else
           cb->refs_offsets_size = spr_rfs_offsts_sze + 1;

      cb->refs_offsets_table = sysMalloc(cb->refs_offsets_size *
                                         sizeof(RefsOffsetsEntry));

      memcpy(cb->refs_offsets_table, spr_rfs_offsts_tbl,
             spr_rfs_offsts_sze * sizeof(RefsOffsetsEntry));

      cb->refs_offsets_table[cb->refs_offsets_size-1].start = refs_start_offset;
      cb->refs_offsets_table[cb->refs_offsets_size-1].end = refs_end_offset;
   } else {
       cb->refs_offsets_size = spr_rfs_offsts_sze;
       cb->refs_offsets_table = spr_rfs_offsts_tbl;
   }
}

int hideFieldFromGC(FieldBlock *hidden) {
    ClassBlock *cb = CLASS_CB(hidden->class);
    FieldBlock *fb;
    int i;

    for(fb = cb->fields, i = 0; i < cb->fields_count; i++,fb++)
        if(fb->u.offset > hidden->u.offset)
            fb->u.offset -= sizeof(Object*);

    cb->refs_offsets_table[cb->refs_offsets_size-1].end -= sizeof(Object*);

    return hidden->u.offset = cb->object_size - sizeof(Object*);
}

#define fillinMTable(method_table, methods, methods_count)              \
{                                                                       \
    int i;                                                              \
    for(i = 0; i < methods_count; i++, methods++) {                     \
        if((methods->access_flags & (ACC_STATIC | ACC_PRIVATE)) ||      \
               (methods->name[0] == '<'))                               \
            continue;                                                   \
        method_table[methods->method_table_index] = methods;            \
    }                                                                   \
}

#define MRNDA_CACHE_INCR 16

/* Structure of each Miranda cache entry.  The Miranda
   cache holds details of new Miranda methods found during
   the construction of the interface method table. */

typedef struct miranda {
    MethodBlock *mb;
    int mtbl_idx;
    int default_conflict;
} Miranda;

void linkClass(Class *class) {
   static MethodBlock *obj_fnlzr_mthd = NULL;

   ClassBlock *cb = CLASS_CB(class);
   Class *super = (cb->access_flags & ACC_INTERFACE) ? NULL : cb->super;

   ITableEntry *spr_imthd_tbl = NULL;
   MethodBlock **method_table = NULL;
   MethodBlock **spr_mthd_tbl = NULL;
   MethodBlock *mb;

   int new_methods_count = 0;
   int spr_imthd_tbl_sze = 0;
   int itbl_offset_count = 0;
   int spr_mthd_tbl_sze = 0;
   int method_table_size;
   int new_itable_count;
   int itbl_idx, i, j;
   int spr_flags = 0;
   Thread *self;

   if(cb->state >= CLASS_LINKED)
       return;

   objectLock(class);

   if(cb->state >= CLASS_LINKED)
       goto unlock;

   if(verbose)
       jam_printf("[Linking class %s]\n", cb->name);

   if(super) {
      ClassBlock *super_cb = CLASS_CB(super);
      if(super_cb->state < CLASS_LINKED)
          linkClass(super);

      spr_flags = super_cb->flags;
      spr_mthd_tbl = super_cb->method_table;
      spr_imthd_tbl = super_cb->imethod_table;
      spr_mthd_tbl_sze = super_cb->method_table_size;
      spr_imthd_tbl_sze = super_cb->imethod_table_size;
   }

   /* Calculate object layout */
   prepareFields(class);

   /* Prepare methods */

   for(mb = cb->methods, i = 0; i < cb->methods_count; i++,mb++) {
       int count = 0;
       char *sig = mb->type;

       /* calculate argument count from signature */
       SCAN_SIG(sig, count+=2, count++);

       if(mb->access_flags & ACC_STATIC)
           mb->args_count = count;
       else
           mb->args_count = count + 1;

       mb->class = class;

       /* Set abstract method to stub */
       if(mb->access_flags & ACC_ABSTRACT) {
           mb->code_size = sizeof(abstract_method);
           mb->code = abstract_method;
       }

       if(mb->access_flags & ACC_NATIVE) {

           /* set up native invoker to wrapper to resolve function 
              on first invocation */

           mb->native_invoker = &resolveNativeWrapper;

           /* native methods have no code attribute so these aren't filled
              in at load time - as these values are used when creating frame
              set to appropriate values */

           mb->max_locals = mb->args_count;
           mb->max_stack = 0;
       }

       /* Static, private or init methods aren't dynamically invoked, so
         don't stick them in the table to save space */

       if((mb->access_flags & (ACC_STATIC | ACC_PRIVATE)) ||
                              (mb->name[0] == '<'))
           continue;

       /* if it's overriding an inherited method, replace in method table */

       for(j = 0; j < spr_mthd_tbl_sze; j++)
           if(mb->name == spr_mthd_tbl[j]->name &&
                        mb->type == spr_mthd_tbl[j]->type &&
                        checkMethodAccess(spr_mthd_tbl[j], class)) {
               mb->method_table_index = spr_mthd_tbl[j]->method_table_index;
               break;
           }

       if(j == spr_mthd_tbl_sze)
           mb->method_table_index = spr_mthd_tbl_sze + new_methods_count++;
   }

   /* construct method table */

   method_table_size = spr_mthd_tbl_sze + new_methods_count;

   if(!(cb->access_flags & ACC_INTERFACE)) {
       method_table = sysMalloc(method_table_size * sizeof(MethodBlock*));

       /* Copy parents method table to the start */
       memcpy(method_table, spr_mthd_tbl, spr_mthd_tbl_sze *
                                          sizeof(MethodBlock*));

       /* fill in the additional methods -- we use a
          temporary because fillinMtable alters mb */
       mb = cb->methods;
       fillinMTable(method_table, mb, cb->methods_count);
   }

   /* setup interface method table */

   /* number of interfaces implemented by this class is those implemented by
    * parent, plus number of interfaces directly implemented by this class,
    * and the total number of their superinterfaces */

   new_itable_count = cb->interfaces_count;
   for(i = 0; i < cb->interfaces_count; i++)
       new_itable_count += CLASS_CB(cb->interfaces[i])->imethod_table_size;

   cb->imethod_table = sysMalloc((spr_imthd_tbl_sze + new_itable_count) *
                                 sizeof(ITableEntry));

   /* the interface references in the imethod table are updated by the
      GC during heap compaction - disable suspension while it is being
      setup */
   self = threadSelf();
   fastDisableSuspend(self);

   cb->imethod_table_size = spr_imthd_tbl_sze + new_itable_count;

   /* copy parent's interface table - the offsets into the method
      table won't change */

   memcpy(cb->imethod_table, spr_imthd_tbl, spr_imthd_tbl_sze *
                                            sizeof(ITableEntry));

   /* now run through the extra interfaces implemented by this class,
    * fill in the interface part, and calculate the number of offsets
    * needed (i.e. the number of methods defined in the interfaces) */

   itbl_idx = spr_imthd_tbl_sze;
   for(i = 0; i < cb->interfaces_count; i++) {
       Class *intf = cb->interfaces[i];
       ClassBlock *intf_cb = CLASS_CB(intf);

       cb->imethod_table[itbl_idx++].interface = intf;
       itbl_offset_count += intf_cb->method_table_size;

       for(j = 0; j < intf_cb->imethod_table_size; j++) {
           Class *spr_intf = intf_cb->imethod_table[j].interface;

           cb->imethod_table[itbl_idx++].interface = spr_intf;
           itbl_offset_count += CLASS_CB(spr_intf)->method_table_size;
       }
   }

   fastEnableSuspend(self);

   /* if we're an interface all finished - offsets aren't used */

   if(!(cb->access_flags & ACC_INTERFACE)) {
       int *offsets_pntr = sysMalloc(itbl_offset_count * sizeof(int));
       Miranda *mirandas = NULL;
       int new_mtbl_count = 0;
       int miranda_count = 0;

       /* run through table again, this time filling in the offsets array -
        * for each new interface, run through it's methods and locate
        * each method in this classes method table */

       for(i = spr_imthd_tbl_sze; i < cb->imethod_table_size; i++) {
           Class *interface = cb->imethod_table[i].interface;
           ClassBlock *intf_cb = CLASS_CB(interface);

           cb->imethod_table[i].offsets = offsets_pntr;

           for(j = 0; j < intf_cb->methods_count; j++) {
               MethodBlock *intf_mb = &intf_cb->methods[j];
               int mtbl_idx, mrnda_idx;

               if((intf_mb->access_flags & (ACC_STATIC | ACC_PRIVATE)) ||
                      (intf_mb->name[0] == '<'))
                   continue;

               /* We scan backwards so that we find methods defined in
                  sub-classes before super-classes.  This ensures we find
                  non-overridden methods before the inherited non-accessible
                  method */
               for(mtbl_idx = method_table_size - 1; mtbl_idx >= 0; mtbl_idx--)
                   if(intf_mb->name == method_table[mtbl_idx]->name &&
                           intf_mb->type == method_table[mtbl_idx]->type)
                       break;

               /* If we found the method in the method table and it's
                  a real method (i.e. implemented) or we already have
                  conflicting defaults we're done */
               if(mtbl_idx >= 0) {
                   MethodBlock *mb = method_table[mtbl_idx];
                   if(!(mb->access_flags & ACC_MIRANDA) ||
                                      mb->flags & MB_DEFAULT_CONFLICT) {
                       *offsets_pntr++ = mtbl_idx;
                       continue;
                   }
               }

               /* We didn't find it, or we already have a matching
                  inherited Miranda method.  Search the Miranda cache
                  to see if we have cached a new Miranda method (either
                  a new Miranda or an override - see below) */
               for(mrnda_idx = 0; mrnda_idx < miranda_count; mrnda_idx++)
                   if(intf_mb->name == mirandas[mrnda_idx].mb->name &&
                               intf_mb->type == mirandas[mrnda_idx].mb->type)
                       break;
                           
               if(mrnda_idx == miranda_count) {
                   int new_mtbl_idx, default_conflict = FALSE;

                   /* No cached Miranda method.  If we found a Miranda
                      method in the method table we need to check it
                      against the interface method to see if either one
                      extends the other.  If the method table Miranda
                      extends the new method it takes precedence and we
                      don't need a new Miranda.  If, however, the new method
                      extends the method table Miranda, or they are
                      unrelated, we need to add a new Miranda to override
                      the method table Miranda (with the new Miranda, or a
                      default conflict). */
                   if(mtbl_idx >= 0) {
                       MethodBlock *mtbl_mb = method_table[mtbl_idx];

                       if(((mtbl_mb->access_flags & ACC_ABSTRACT) 
                                   && (intf_mb->access_flags & ACC_ABSTRACT))
                             || mtbl_mb->miranda_mb->class == interface 
                             || implements(interface, mtbl_mb->miranda_mb->class)) {
                           *offsets_pntr++ = mtbl_idx;
                           continue;
                       }

                       if(!implements(mtbl_mb->miranda_mb->class, interface))
                           default_conflict = TRUE;

                       /* The new Miranda is an override, so it replaces
                          the method in the method table */
                       new_mtbl_idx = mtbl_idx;
                   } else {
                       /* No cached Miranda, and none in the method table -
                          simply add a new Miranda - it has a new method
                          table index */
                       new_mtbl_idx = method_table_size + new_mtbl_count++;
                   }

                   /* Extend the Miranda cache if it's full */
                   if((miranda_count % MRNDA_CACHE_INCR) == 0)
                       mirandas = sysRealloc(mirandas, (miranda_count +
                                     MRNDA_CACHE_INCR) * sizeof(Miranda));

                   /* Add the new Miranda to the cache */
                   mirandas[miranda_count].mb = intf_mb;
                   mirandas[miranda_count].mtbl_idx = new_mtbl_idx;
                   mirandas[miranda_count].default_conflict = default_conflict;
                   miranda_count++;
               } else {
                   /* Found a matching Miranda method in the cache (either
                      an override or a new Miranda).  Check the two methods
                      against each other as above.  The difference is if
                      we need to override, we simply modify the cached
                      Miranda - we don't need to add a new Miranda for the
                      override */
                   MethodBlock *mrnda_mb = mirandas[mrnda_idx].mb;

                   if(!((mrnda_mb->access_flags & ACC_ABSTRACT) 
                                && (intf_mb->access_flags & ACC_ABSTRACT))
                         && !mirandas[mrnda_idx].default_conflict
                         && mrnda_mb->class != interface
                         && !implements(interface, mrnda_mb->class)) {

                       if(implements(mrnda_mb->class, interface))
                           mirandas[mrnda_idx].mb = intf_mb;
                       else
                           mirandas[mrnda_idx].default_conflict = TRUE;
                   }
               }

               *offsets_pntr++ = mirandas[mrnda_idx].mtbl_idx;
           }
       }

       if(miranda_count > 0) {
           /* We've created some new Miranda methods.  Add them to
              the method area.  The method table may also need expanding
              if they are not all overrides */
   
           mb = sysRealloc(cb->methods, (cb->methods_count + miranda_count)
                                        * sizeof(MethodBlock));

           /* If the realloc of the method area gave us a new pointer, the
              pointers to them in the method table are now wrong. */
           if(mb != cb->methods) {
               /*  mb will be left pointing to the end of the methods */
               cb->methods = mb;
               fillinMTable(method_table, mb, cb->methods_count);
           } else
               mb += cb->methods_count;

           memset(mb, 0, miranda_count * sizeof(MethodBlock));

           if(new_mtbl_count > 0) {
               method_table_size += new_mtbl_count;
               method_table = sysRealloc(method_table, method_table_size *
                                                       sizeof(MethodBlock*));
           }

           /* Now we've expanded the methods, run through the Miranda
              cache and fill them in */
           for(i = 0; i < miranda_count; i++,mb++) {
               MethodBlock *intf_mb = mirandas[i].mb;

               mb->class = class;
               mb->name = intf_mb->name;
               mb->type = intf_mb->type;
               mb->max_stack = intf_mb->max_stack;
               mb->max_locals = intf_mb->max_locals;
               mb->args_count = intf_mb->args_count;
               mb->method_table_index = mirandas[i].mtbl_idx;
               mb->access_flags = intf_mb->access_flags | ACC_MIRANDA;

               if(mirandas[i].default_conflict) {
                   mb->flags = MB_DEFAULT_CONFLICT;
                   mb->code_size = sizeof(abstract_method);
                   mb->code = abstract_method;
               } else {
                   mb->miranda_mb = intf_mb;
                   mb->code_size = sizeof(miranda_bridge);
                   mb->code = miranda_bridge;
               }

               method_table[mirandas[i].mtbl_idx] = mb;
           }

           sysFree(mirandas);
           cb->methods_count += miranda_count;
       }
   }

   cb->method_table = method_table;
   cb->method_table_size = method_table_size;

   /* Handle finalizer */

   /* If this is Object find the finalize method.  All subclasses will
      have it in the same place in the method table.  Note, Object
      should always have a valid finalizer -- but check just in case */

   if(cb->super == NULL) {
       MethodBlock *finalizer = findMethod(class, SYMBOL(finalize),
                                                  SYMBOL(___V));

       if(finalizer != NULL && !(finalizer->access_flags &
                                 (ACC_STATIC | ACC_PRIVATE))) {
           finalize_mtbl_idx = finalizer->method_table_index;
           obj_fnlzr_mthd = finalizer;
       }
   }

   cb->flags |= spr_flags;

   /* Mark as finalized only if it's overridden Object's finalizer.
      We don't want to finalize every object, and Object's imp is empty */

   if(super != NULL && obj_fnlzr_mthd != NULL &&
          method_table[finalize_mtbl_idx] != obj_fnlzr_mthd)
       cb->flags |= FINALIZED;

   /* Handle reference classes */

   if(ref_referent_offset == -1 &&
               cb->name == SYMBOL(java_lang_ref_Reference)) {

       FieldBlock *ref_fb = findField(class, SYMBOL(referent),
                                             SYMBOL(sig_java_lang_Object));
       FieldBlock *queue_fb = findField(class, SYMBOL(queue),
                                     SYMBOL(sig_java_lang_ref_ReferenceQueue));
       MethodBlock *enqueue_mb = findMethod(class, SYMBOL(enqueue),
                                                   SYMBOL(___Z));

       if(ref_fb == NULL || queue_fb == NULL || enqueue_mb == NULL) {
           jam_fprintf(stderr, "Expected fields/methods missing in"
                               " java.lang.ref.Reference\n");
           exitVM(1);
       }

       ref_referent_offset = hideFieldFromGC(ref_fb);
       enqueue_mtbl_idx = enqueue_mb->method_table_index;
       ref_queue_offset = queue_fb->u.offset;

       cb->flags |= REFERENCE;
   }

   if(spr_flags & REFERENCE) {
       if(cb->name == SYMBOL(java_lang_ref_SoftReference))
           cb->flags |= SOFT_REFERENCE;
       else
           if(cb->name == SYMBOL(java_lang_ref_WeakReference))
               cb->flags |= WEAK_REFERENCE;
           else
               if(cb->name == SYMBOL(java_lang_ref_PhantomReference))
                   cb->flags |= PHANTOM_REFERENCE;
   }

   /* Handle class loader classes */

   if(cb->name == SYMBOL(java_lang_ClassLoader)) {
       classlibCacheClassLoaderFields(class);
       cb->flags |= CLASS_LOADER;
   }

   cb->state = CLASS_LINKED;

unlock:
   objectUnlock(class);
}

Class *initClass(Class *class) {
   ClassBlock *cb = CLASS_CB(class);
   ConstantPool *cp = &cb->constant_pool;
   FieldBlock *fb = cb->fields;
   MethodBlock *mb;
   Object *excep;
   int state, i;

   if(cb->state >= CLASS_INITED)
      return class;

   linkClass(class);
   objectLock(class);

   while(cb->state == CLASS_INITING)
      if(cb->initing_tid == threadSelf()->id) {
         objectUnlock(class);
         return class;
      } else {
          /* FALSE means this wait is non-interruptible.
             An interrupt will appear as if the initialiser
             failed (below), and clearing will lose the
             interrupt status */
          objectWait(class, 0, 0, FALSE);
      }

   if(cb->state >= CLASS_INITED) {
      objectUnlock(class);
      return class;
   }

   if(cb->state == CLASS_BAD) {
       objectUnlock(class);
       signalException(java_lang_NoClassDefFoundError, cb->name);
       return NULL;
   }

   cb->state = CLASS_INITING;
   cb->initing_tid = threadSelf()->id;

   objectUnlock(class);

   if(!(cb->access_flags & ACC_INTERFACE) && cb->super
              && (CLASS_CB(cb->super)->state != CLASS_INITED)) {
      initClass(cb->super);
      if(exceptionOccurred()) {
          state = CLASS_BAD;
          goto set_state_and_notify;
      }
   }

   /* Never used to bother with this as only static finals use it and
      the constant value's copied at compile time.  However, separate
      compilation can result in a getstatic to a (now) constant field,
      and the VM didn't initialise it... */

   for(i = 0; i < cb->fields_count; i++,fb++)
      if((fb->access_flags & ACC_STATIC) && fb->constant) {
         if((*fb->type == 'J') || (*fb->type == 'D'))
            fb->u.static_value.l = *(u8*)&(CP_INFO(cp, fb->constant));
         else
            fb->u.static_value.u = resolveSingleConstant(class, fb->constant);
      }

   if((mb = findMethod(class, SYMBOL(class_init), SYMBOL(___V))) != NULL)
      executeStaticMethod(class, mb);

   if((excep = exceptionOccurred())) {
       Class *error;

       clearException(); 

       /* Don't wrap exceptions of type java.lang.Error... */
       error = findSystemClass0(SYMBOL(java_lang_Error));
       if(error != NULL && !isInstanceOf(error, excep->class)) {
           Class *init_error = findSystemClass(
                         SYMBOL(java_lang_ExceptionInInitializerError));
           if(init_error != NULL) {
               mb = findMethod(init_error, SYMBOL(object_init),
                                           SYMBOL(_java_lang_Throwable__V));
               if(mb != NULL) {
                   Object *ob = allocObject(init_error);

                   if(ob != NULL) {
                       executeMethod(ob, mb, excep);
                       excep = ob;
                   }
               }
           }
       }

       setException(excep);

       state = CLASS_BAD;
   } else
       state = CLASS_INITED;
   
set_state_and_notify:
   objectLock(class);
   cb->state = state;

   objectNotifyAll(class);
   objectUnlock(class);

   return state == CLASS_BAD ? NULL : class;
}

char *findFileEntry(char *path, int *file_len) {
    int read_len;
    char *data;
    FILE *fd;

    if((fd = fopen(path, "r")) == NULL)
        return NULL;

    fseek(fd, 0L, SEEK_END);
    *file_len = ftell(fd);
    fseek(fd, 0L, SEEK_SET);

    data = sysMalloc(*file_len);
    read_len = fread(data, sizeof(char), *file_len, fd);
    fclose(fd);

    if(read_len == *file_len)
        return data;

    sysFree(data);
    return NULL;
}

void defineBootPackage(char *classname, int index) {
    char *last_slash = strrchr(classname, '/');

    if(last_slash != NULL && last_slash != classname) {
        int len = last_slash - classname + 1;
        PackageEntry *package = sysMalloc(sizeof(PackageEntry) + len);
        PackageEntry *hashed;
        
        package->index = index;
        slash2DotsBuff(classname, package->name, len);

#undef HASH
#undef COMPARE
#define HASH(ptr) utf8Hash(((PackageEntry*)ptr)->name)
#define COMPARE(ptr1, ptr2, hash1, hash2) (hash1 == hash2 && \
            utf8Comp(((PackageEntry*)ptr1)->name, ((PackageEntry*)ptr2)->name))

        /* Add if absent, no scavenge, locked */
        findHashEntry(boot_packages, package, hashed, TRUE, FALSE, TRUE);

        if(package != hashed)
            sysFree(package);
    }
}

Class *loadSystemClass(char *classname) {
    int file_len, fname_len = strlen(classname) + 8;
    char buff[max_cp_element_len + fname_len];
    char filename[fname_len];
    Class *class = NULL;
    char *data = NULL;
    int i;

    filename[0] = '/';
    strcat(strcpy(&filename[1], classname), ".class");

    for(i = 0; i < bcp_entries && data == NULL; i++)
        if(bootclasspath[i].zip)
            data = findArchiveEntry(filename + 1, bootclasspath[i].zip,
                                    &file_len);
        else
            data = findFileEntry(strcat(strcpy(buff, bootclasspath[i].path),
                                 filename), &file_len);

    if(data == NULL) {
        signalException(java_lang_NoClassDefFoundError, classname);
        return NULL;
    }

    /* If this class belongs to a package not yet seen add it
       to the list of bootloader packages */
    defineBootPackage(classname, i - 1);

    class = defineClass(classname, data, 0, file_len, NULL);
    sysFree(data);

    if(verbose && class)
        jam_printf("[Loaded %s from %s]\n", classname, bootclasspath[i-1].path);

    return class;
}

void addInitiatingLoaderToClass(Object *class_loader, Class *class) {
    ClassBlock *cb = CLASS_CB(class);

    /* The defining class loader is automatically an initiating
       loader so don't add again */
    if(cb->class_loader != class_loader)
        addClassToHash(class, class_loader);
}

Class *findHashedClass(char *classname, Object *class_loader) {
    HashTable *table;
    Class *class;
    char *name;

    /* If the class name is not in the utf8 table it can't
       have been loaded */
    if((name = findUtf8(classname)) == NULL)
        return NULL;

    if(class_loader == NULL)
        table = &boot_classes;
    else
        if((table = classlibLoaderTable(class_loader)) == NULL)
            return NULL;

#undef HASH
#undef COMPARE
#define HASH(ptr) utf8Hash(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) (hash1 == hash2) && \
            (ptr1 == CLASS_CB((Class *)ptr2)->name)

    /* Do not add if absent, no scavenge, locked */
   findHashEntry((*table), name, class, FALSE, FALSE, TRUE);

   return class;
}

Class *findSystemClass0(char *classname) {
   Class *class = findHashedClass(classname, NULL);

   if(class == NULL)
       class = loadSystemClass(classname);

   if(!exceptionOccurred())
       linkClass(class);

   return class;
}

Class *findSystemClass(char *classname) {
   Class *class = findSystemClass0(classname);

   if(!exceptionOccurred())
       initClass(class);

   return class;
}

Class *findArrayClassFromClassLoader(char *classname, Object *class_loader) {
   Class *class = findHashedClass(classname, class_loader);

   if(class == NULL)
       if((class = createArrayClass(classname, class_loader)) != NULL)
           addInitiatingLoaderToClass(class_loader, class);

   return class;
}

Class *findPrimitiveClassByName(char *classname) {
   int index;
   Class *prim;
   char *prim_name;

    if((prim_name = findUtf8(classname)) == NULL)
        goto error;

    if(prim_name == SYMBOL(boolean))
       index = PRIM_IDX_BOOLEAN;
    else if(prim_name == SYMBOL(byte))
       index = PRIM_IDX_BYTE;
    else if(prim_name == SYMBOL(char))
       index = PRIM_IDX_CHAR;
    else if(prim_name == SYMBOL(short))
        index = PRIM_IDX_SHORT;
    else if(prim_name == SYMBOL(int))
        index = PRIM_IDX_INT;
    else if(prim_name == SYMBOL(float))
        index = PRIM_IDX_FLOAT;
    else if(prim_name == SYMBOL(long))
        index = PRIM_IDX_LONG;
    else if(prim_name == SYMBOL(double))
        index = PRIM_IDX_DOUBLE;
    else if(prim_name == SYMBOL(void))
        index = PRIM_IDX_VOID;
    else
        goto error;

    prim = prim_classes[index];
    return prim ? prim : createPrimClass(prim_name, index);

error:
    signalException(java_lang_NoClassDefFoundError, NULL);
    return NULL;
}

Class *findPrimitiveClass(char prim_type) {
   int index;
   Class *prim;
   char *classname;

   switch(prim_type) {
      case 'Z':
          classname = SYMBOL(boolean);
          index = PRIM_IDX_BOOLEAN;
          break;
      case 'B':
          classname = SYMBOL(byte);
          index = PRIM_IDX_BYTE;
          break;
      case 'C':
          classname = SYMBOL(char);
          index = PRIM_IDX_CHAR;
          break;
      case 'S':
          classname = SYMBOL(short);
           index = PRIM_IDX_SHORT;
          break;
      case 'I':
          classname = SYMBOL(int);
           index = PRIM_IDX_INT;
          break;
      case 'F':
          classname = SYMBOL(float);
           index = PRIM_IDX_FLOAT;
          break;
      case 'J':
          classname = SYMBOL(long);
           index = PRIM_IDX_LONG;
          break;
      case 'D':
          classname = SYMBOL(double);
           index = PRIM_IDX_DOUBLE;
          break;
      case 'V':
          classname = SYMBOL(void);
           index = PRIM_IDX_VOID;
          break;
      default:
          signalException(java_lang_NoClassDefFoundError, NULL);
          return NULL;
   }

   prim = prim_classes[index];
   return prim ? prim : createPrimClass(classname, index);
}

Class *findNonArrayClassFromClassLoader(char *classname, Object *loader) {
    Class *class = findHashedClass(classname, loader);

    if(class == NULL) {
        char *dot_name = slash2DotsDup(classname);
        Object *string = createString(dot_name);
        MethodBlock *loadClass;
        Object *excep;

        sysFree(dot_name);
        if(string == NULL)
            return NULL;

        if(loadClass_mtbl_idx == -1) {
            MethodBlock *mb = lookupMethod(loader->class, SYMBOL(loadClass),
                            SYMBOL(_java_lang_String__java_lang_Class));
            if(mb == NULL)
                return NULL;

            loadClass_mtbl_idx = mb->method_table_index;
        }

        loadClass = CLASS_CB(loader->class)->method_table[loadClass_mtbl_idx];

        /* The public loadClass is not synchronized.
           Lock the class-loader to be thread-safe */
        objectLock(loader);
        class = *(Class**)executeMethod(loader, loadClass, string);
        objectUnlock(loader);

        if((excep = exceptionOccurred()) || class == NULL) {
            clearException();
            signalChainedException(java_lang_NoClassDefFoundError,
                                   classname, excep);
            return NULL;
        }

        addInitiatingLoaderToClass(loader, class);

        if(verbose && (CLASS_CB(class)->class_loader == loader))
            jam_printf("[Loaded %s]\n", classname);
    }
    return class;
}

Class *findClassFromClassLoader(char *classname, Object *loader) {
    loader = classlibSkipReflectionLoader(loader);

    if(*classname == '[')
        return findArrayClassFromClassLoader(classname, loader);

    if(loader != NULL)
        return findNonArrayClassFromClassLoader(classname, loader);

    return findSystemClass0(classname);
}

Object *getSystemClassLoader() {
    Class *class_loader = findSystemClass(SYMBOL(java_lang_ClassLoader));

    if(!exceptionOccurred()) {
        MethodBlock *mb = findMethod(class_loader,
                                     SYMBOL(getSystemClassLoader),
                                     SYMBOL(___java_lang_ClassLoader));

        if(mb != NULL) {
            Object *loader = *(Object**)executeStaticMethod(class_loader, mb);

            if(!exceptionOccurred()) 
                return loader;
        }
    }
    return NULL;
}

Object *bootPackage(char *package_name) {
    PackageEntry *hashed;

#undef HASH
#undef COMPARE
#define HASH(ptr) utf8Hash(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) (hash1 == hash2 && \
                                 utf8Comp(ptr1, ((PackageEntry*)ptr2)->name))

    /* Do not add if absent, no scavenge, locked */
    findHashEntry(boot_packages, package_name, hashed, FALSE, FALSE, TRUE);

    if(hashed != NULL)
        return classlibBootPackage(hashed);

    return NULL;
}

#define ITERATE(ptr)                                          \
    if((data[--count] = classlibBootPackages(ptr)) == NULL) { \
        array = NULL;                                         \
        goto error;                                           \
    }

Object *bootPackages() {
    Class *array_class = classlibBootPackagesArrayClass();
    Object **data, *array;
    int count;

    lockHashTable(boot_packages);

    count = hashTableCount(boot_packages);
    if((array = allocArray(array_class, count, sizeof(Object*))) == NULL)
        goto error;

    data = ARRAY_DATA(array, Object*);
    hashIterate(boot_packages);

error:
    unlockHashTable(boot_packages);
    return array;
}

/* gc support for marking classes */

#undef ITERATE
#define ITERATE(ptr) markRoot(ptr)

void markBootClasses() {
   int i;

   hashIterate(boot_classes);

   for(i = 0; i < MAX_PRIM_CLASSES; i++)
       if(prim_classes[i] != NULL)
           markRoot((Object*)prim_classes[i]);
}

#undef ITERATE
#define ITERATE(ptr)  threadReference((Object**)ptr)

void threadBootClasses() {
   int i;

   hashIterateP(boot_classes);

   for(i = 0; i < MAX_PRIM_CLASSES; i++)
       if(prim_classes[i] != NULL)
           threadReference((Object**)&prim_classes[i]);
}

#undef ITERATE
#define ITERATE(ptr)                                         \
    if(CLASS_CB((Class *)ptr)->class_loader == class_loader) \
        markObject(ptr, mark)

void markLoaderClasses(Object *class_loader, int mark) {
    HashTable *table = classlibLoaderTable(class_loader);

    if(table != NULL) {
        hashIterate((*table));
    }
}

#undef ITERATE
#define ITERATE(ptr) threadReference((Object**)ptr)

void threadLoaderClasses(Object *class_loader) {
    HashTable *table = classlibLoaderTable(class_loader);

    if(table != NULL) {
        hashIterateP((*table));
    }
}

static void freeIndexedAttributes(AttributeData **attributes, int size) {
    int i;

    if(attributes == NULL)
        return;

    for(i = 0; i < size; i++)
        if(attributes[i] != NULL) {
            gcPendingFree(attributes[i]->data);
            gcPendingFree(attributes[i]);
        }

    gcPendingFree(attributes);
}

#define freeSingleAttributes(attributes) \
    if(attributes != NULL) {             \
        gcPendingFree(attributes->data); \
        gcPendingFree(attributes);       \
    }

void freeClassData(Class *class) {
    ClassBlock *cb = CLASS_CB(class);
    int i;

    if(IS_ARRAY(cb)) {
        gcPendingFree(cb->interfaces);
        return;
    }

#ifdef JSR292
    freeResolvedPolyData(class);
#endif

    gcPendingFree((void*)cb->constant_pool.type);
    gcPendingFree(cb->constant_pool.info);
    gcPendingFree(cb->interfaces);
    gcPendingFree(cb->fields);

    for(i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];

#ifdef DIRECT
        if(mb->state == MB_PREPARED) {
#ifdef INLINING
            freeMethodInlinedInfo(mb);
#endif
            gcPendingFree(mb->code);
        } else
#endif
        if(!(mb->access_flags & (ACC_NATIVE | ACC_ABSTRACT | ACC_MIRANDA)))
            gcPendingFree(mb->code);

        /* Miranda methods have no exception, line number
           or throw tables */
        if(mb->access_flags & ACC_MIRANDA)
            continue;

        if(!(mb->access_flags & ACC_NATIVE)) {
            gcPendingFree(mb->exception_table);
            gcPendingFree(mb->line_no_table);
        }
        gcPendingFree(mb->throw_table);
    } 

    if(cb->extra_attributes != NULL) {
        int methods_count;

        /* The method extra attributes does not include the Miranda
           methods so we need to adjust the methods count.  The
           Mirandas are all at the end of the methods and are small
           in number - this quick loop is preferable to storing an
           extra count in the method block.  Likewise, we could put
           something in the loop above to track the last non-Miranda,
           but extra attributes are also rare. */
        for(i = cb->methods_count - 1; i >= 0 &&
            cb->methods[i].access_flags & ACC_MIRANDA; i--);
        methods_count = i + 1;

        freeSingleAttributes(cb->extra_attributes->class_annos);

        freeIndexedAttributes(cb->extra_attributes->field_annos,
                              cb->fields_count);
        freeIndexedAttributes(cb->extra_attributes->method_annos,
                              methods_count);
        freeIndexedAttributes(cb->extra_attributes->method_parameter_annos,
                              methods_count);
        freeIndexedAttributes(cb->extra_attributes->method_anno_default_val,
                              methods_count);

#ifdef JSR308
        freeSingleAttributes(cb->extra_attributes->class_type_annos);
        freeIndexedAttributes(cb->extra_attributes->field_type_annos,
                              cb->fields_count);
        freeIndexedAttributes(cb->extra_attributes->method_type_annos,
                              methods_count);
#endif
#ifdef JSR901
        freeIndexedAttributes(cb->extra_attributes->method_parameters,
                              methods_count);
#endif

        gcPendingFree(cb->extra_attributes);
    }

    gcPendingFree(cb->methods);
    gcPendingFree(cb->inner_classes);

    if(cb->state >= CLASS_LINKED) {
        ClassBlock *super_cb = CLASS_CB(cb->super);

        /* interfaces do not have a method table, or 
            imethod table offsets */
        if(!IS_INTERFACE(cb)) {
            int spr_imthd_sze = super_cb->imethod_table_size;

            gcPendingFree(cb->method_table);
            if(cb->imethod_table_size > spr_imthd_sze)
                gcPendingFree(cb->imethod_table[spr_imthd_sze].offsets);
        }

        gcPendingFree(cb->imethod_table);

        if(cb->refs_offsets_table != super_cb->refs_offsets_table)
            gcPendingFree(cb->refs_offsets_table);
    }
}

void freeClassLoaderData(Object *class_loader) {
    HashTable *table = classlibLoaderTable(class_loader);

    if(table != NULL) {
        gcFreeHashTable((*table));
        gcPendingFree(table);
    }
}

void parseBootClassPath() {
    char *cp, *pntr, *start;
    int i, j, len, max = 0;
    struct stat info;

    cp = sysMalloc(strlen(bootpath)+1);
    strcpy(cp, bootpath);

    for(i = 0, start = pntr = cp; *pntr; pntr++) {
        if(*pntr == ':') {
            if(start != pntr) {
                *pntr = '\0';
                i++;
            }
            start = pntr+1;
        }
    }
    if(start != pntr)
        i++;

    bootclasspath = sysMalloc(sizeof(BCPEntry)*i);

    for(j = 0, pntr = cp; i > 0; i--) {
        while(*pntr == ':')
            pntr++;

        start = pntr;
        pntr += (len = strlen(pntr))+1;

        if(stat(start, &info) == 0) {
            if(S_ISDIR(info.st_mode)) {
                bootclasspath[j].zip = NULL;
                if(len > max)
                    max = len;
            } else
                if((bootclasspath[j].zip = processArchive(start)) == NULL)
                    continue;
            bootclasspath[j++].path = start;
        }
    }

    max_cp_element_len = max;
    bcp_entries = j;
}

void setClassPath(InitArgs *args) {
    char *env;
    classpath = args->classpath ? args->classpath : 
                 ((env = getenv("CLASSPATH")) ? env : ".");
}

char *getClassPath() {
    return classpath;
}

#ifdef __linux__
int filter(const struct dirent *entry) {
#else
int filter(struct dirent *entry) {
#endif
    int len = strlen(entry->d_name);
    char *ext = (char*)&entry->d_name[len-4];

    return len >= 4 && (strcasecmp(ext, ".zip") == 0 ||
                        strcasecmp(ext, ".jar") == 0);
}

void scanDirForJars(char *dir) {
    int bootpathlen = strlen(bootpath) + 1;
    int dirlen = strlen(dir);
    struct dirent **namelist;
    int n;

    n = scandir(dir, &namelist, &filter, &alphasort);

    if(n >= 0) {
        while(--n >= 0) {
            char *buff;
            bootpathlen += strlen(namelist[n]->d_name) + dirlen + 2;
            buff = sysMalloc(bootpathlen);

            strcat(strcat(strcat(strcat(strcpy(buff, dir), "/"),
                                 namelist[n]->d_name), ":"), bootpath);

            sysFree(bootpath);
            bootpath = buff;
            free(namelist[n]);
        }
        free(namelist);
    }
}

void scanDirsForJars(char *directories) {
    int dirslen = strlen(directories);
    char *pntr, *end, *dirs = sysMalloc(dirslen + 1);

    strcpy(dirs, directories);
    for(end = pntr = &dirs[dirslen]; pntr != dirs; pntr--) {
        if(*pntr == ':') {
            char *start = pntr + 1;
            if(start != end)
                scanDirForJars(start);

            *(end = pntr) = '\0';
        }
    }

    if(end != dirs)
        scanDirForJars(dirs);

    sysFree(dirs);
}

char *getEndorsedDirs() {
    char *endorsed_dirs = getCommandLineProperty("java.endorsed.dirs");
    if(endorsed_dirs == NULL)
        endorsed_dirs = classlibDefaultEndorsedDirs();

    return endorsed_dirs;
}

void setBootClassPath(InitArgs *args) {
    char *path = args->bootpath;

    if(path == NULL)
        path = getCommandLineProperty("sun.boot.class.path");
    if(path == NULL)
        path = getCommandLineProperty("java.boot.class.path");
    if(path == NULL)
        path = getenv("BOOTCLASSPATH");
    if(path == NULL)
        path = classlibBootClassPathOpt(args);

    if(args->bootpath_a != NULL) {
        bootpath = sysMalloc(strlen(path) + strlen(args->bootpath_a) + 2);
        strcat(strcat(strcpy(bootpath, path), ":"), args->bootpath_a);
    } else
        bootpath = strcpy(sysMalloc(strlen(path) + 1), path);

    scanDirsForJars(getEndorsedDirs());

    if(args->bootpath_p != NULL) {
        path = sysMalloc(strlen(bootpath) + strlen(args->bootpath_p) + 2);
        strcat(strcat(strcpy(path, args->bootpath_p), ":"), bootpath);
        sysFree(bootpath);
        bootpath = path;
    }
}

char *getBootClassPath() {
    return bootpath;
}

char *getBootClassPathEntry(int index) {
    return bootclasspath[index].path;
}

int bootClassPathSize() {
    return bcp_entries;
}

Object *bootClassPathResource(char *filename, int index) {
    Object *res = NULL;

    if(index < bcp_entries) {
        char *buff, *path = bootclasspath[index].path;
        int path_len = strlen(path);

        if(path[0] != '/') {
            char *cwd = getCwd();
            path_len += strlen(cwd) + 1;
            path = strcat(strcat(strcpy(sysMalloc(path_len + 1), cwd),
                                 "/"), path);
            sysFree(cwd);
        }

        /* Alloc enough space for Jar file URL --
           jar:file://<path>!/<filename> */
        buff = sysMalloc(strlen(filename) + path_len + 14);

        if(bootclasspath[index].zip != NULL) {
            while(*filename == '/')
                filename++;

            if(findArchiveDirEntry(filename, bootclasspath[index].zip) == NULL)
                goto out;

            sprintf(buff, "jar:file://%s!/%s", path, filename);
        } else {
            struct stat info;

            sprintf(buff, "file://%s/%s", path, filename);
            if(stat(&buff[7], &info) != 0 || S_ISDIR(info.st_mode))
                goto out;
        }

        res = createString(buff);

out:
        if(path != bootclasspath[index].path)
            sysFree(path);
        sysFree(buff);
    }

    return res;
}

int initialiseClassStage1(InitArgs *args) {
    verbose = args->verboseclass;

    setClassPath(args);
    setBootClassPath(args);

    parseBootClassPath();

    /* Init hash table, and create lock for the bootclassloader classes */
    initHashTable(boot_classes, CLASS_INITSZE, TRUE);

    /* Init hash table, and create lock for the bootclassloader packages */
    initHashTable(boot_packages, PCKG_INITSZE, TRUE);

    /* Register the address of where the java.lang.Class ref _will_ be */
    registerStaticClassRef(&java_lang_Class);

    /* Do classlib specific class initialisation */
    if(!classlibInitialiseClass()) {
        jam_fprintf(stderr, "Error initialising VM (initialiseClassStage1)\n");
        return FALSE;
    }

    return TRUE;
}

int initialiseClassStage2() {
    int padding = offsetof(ClassBlock, state);
    int fields = CLASS_CB(java_lang_Class)->object_size - sizeof(Object);

    if(padding < fields) {
        jam_fprintf(stderr, "Error initialising VM (initialiseClassStage2)\n"
                            "ClassBlock padding is less than java.lang.Class "
                            "fields!\n");
        return FALSE;
    }

    /* Ensure that java.lang.Class is initialised.  We can't do it in
       stage1 (above) as it is too early in the initialisation process
       to run Java code */
    if(initClass(java_lang_Class) == NULL) {
        jam_fprintf(stderr, "Error initialising VM (initialiseClassStage2)\n"
                            "java.lang.Class could not be initialised!\n");
        return FALSE;
    }

    return TRUE;
}
