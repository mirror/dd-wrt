/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 * 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
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
#include "jam.h"
#include "symbol.h"
#include "excep.h"
#include "thread.h"
#include "hash.h"
#include "class.h"
#include "classlib.h"

MethodBlock *findMethod(Class *class, char *methodname, char *type) {
   ClassBlock *cb = CLASS_CB(class);
   MethodBlock *mb = cb->methods;
   int i;

   for(i = 0; i < cb->methods_count; i++,mb++)
       if(mb->name == methodname && mb->type == type)
          return mb;

   return NULL;
}

/* As a Java program can't have two fields with the same name but different
   types, we used to give up if we found a field with the right name but wrong
   type.  However, obfuscators rename fields, breaking this optimisation.
*/
FieldBlock *findField(Class *class, char *fieldname, char *type) {
    ClassBlock *cb = CLASS_CB(class);
    FieldBlock *fb = cb->fields;
    int i;

    for(i = 0; i < cb->fields_count; i++,fb++)
        if(fb->name == fieldname && fb->type == type)
            return fb;

    return NULL;
}

MethodBlock *lookupMethod(Class *class, char *methodname, char *type) {
    MethodBlock *mb;

#ifdef JSR292
    if(CLASS_CB(class)->name == SYMBOL(java_lang_invoke_MethodHandle)) {
        if(methodname == SYMBOL(invoke) || methodname == SYMBOL(invokeExact))
            return NULL;
    }
#endif

    if((mb = findMethod(class, methodname, type)))
       return mb;

    if(CLASS_CB(class)->super)
        return lookupMethod(CLASS_CB(class)->super, methodname, type);

    return NULL;
}

MethodBlock *lookupInterfaceMethod(Class *class, char *methodname,
                                   char *type) {

    MethodBlock *mb = lookupMethod(class, methodname, type);

    if(mb == NULL) {
        ClassBlock *cb = CLASS_CB(class);
        int i;

        for(i = 0; mb == NULL && (i < cb->imethod_table_size); i++) {
            Class *intf = cb->imethod_table[i].interface;
            mb = findMethod(intf, methodname, type);
        }
    }

    return mb;
}

FieldBlock *lookupField(Class *class, char *fieldname, char *fieldtype) {
    ClassBlock *cb;
    FieldBlock *fb;
    int i;

    if((fb = findField(class, fieldname, fieldtype)) != NULL)
        return fb;

    cb = CLASS_CB(class);
    i = cb->super ? CLASS_CB(cb->super)->imethod_table_size : 0;

    for(; i < cb->imethod_table_size; i++) {
        Class *intf = cb->imethod_table[i].interface;
        if((fb = findField(intf, fieldname, fieldtype)) != NULL)
            return fb;
    }

    if(cb->super)
        return lookupField(cb->super, fieldname, fieldtype);

    return NULL;
}

Class *resolveClass(Class *class, int cp_index, int check_access, int init) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    Class *resolved_class = NULL;

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_ResolvedClass:
            resolved_class = (Class *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_Class: {
            char *classname;
            int name_idx = CP_CLASS(cp, cp_index);

            MBARRIER();
            if(CP_TYPE(cp, cp_index) != CONSTANT_Class)
                goto retry;

            classname = CP_UTF8(cp, name_idx);
            resolved_class = findClassFromClass(classname, class);

            /* If we can't find the class an exception will already have
               been thrown */

            if(resolved_class == NULL)
                return NULL;

            /* Ensure class is linked (a class is linked immediately
               after it is recorded in the class table, but a race
               condition exists where another thread may find it in
               the table before the first thread has linked it) */

            if(CLASS_CB(resolved_class)->state < CLASS_LINKED)
                linkClass(resolved_class);

            if(check_access && !checkClassAccess(resolved_class, class)) {
                signalException(java_lang_IllegalAccessError,
                                "class is not accessible");
                return NULL;
            }

            CP_TYPE(cp, cp_index) = CONSTANT_Locked;
            MBARRIER();
            CP_INFO(cp, cp_index) = (uintptr_t)resolved_class;
            MBARRIER();
            CP_TYPE(cp, cp_index) = CONSTANT_ResolvedClass;

            break;
        }
    }

    if(init)
        initClass(resolved_class);

    return resolved_class;
}

MethodBlock *resolveMethod(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    MethodBlock *mb = NULL;
    int tag;

retry:
    tag = CP_TYPE(cp, cp_index);
    switch(tag) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_ResolvedMethod:
            mb = (MethodBlock *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref: {
            Class *resolved_class;
            ClassBlock *resolved_cb;
            char *methodname, *methodtype;
            int cl_idx = CP_METHOD_CLASS(cp, cp_index);
            int name_type_idx = CP_METHOD_NAME_TYPE(cp, cp_index);

            MBARRIER();
            if(CP_TYPE(cp, cp_index) != tag)
                goto retry;

            methodname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp, name_type_idx));

            methodtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp, name_type_idx));
            resolved_class = resolveClass(class, cl_idx, TRUE, FALSE);
            resolved_cb = CLASS_CB(resolved_class);

            if(exceptionOccurred())
                return NULL;

            if(resolved_cb->access_flags & ACC_INTERFACE) {
#ifdef JSR335
                mb = lookupInterfaceMethod(resolved_class, methodname,
                	                   methodtype);
#else
                signalException(java_lang_IncompatibleClassChangeError, NULL);
                return NULL;
#endif
            } else
                mb = lookupMethod(resolved_class, methodname, methodtype);

#ifdef JSR292
            if(mb == NULL)
                mb = lookupPolymorphicMethod(resolved_class, class,
                                             methodname, methodtype);
#endif

            if(mb != NULL) {
                if((mb->access_flags & ACC_ABSTRACT) &&
                       !(resolved_cb->access_flags & ACC_ABSTRACT)) {
                    signalException(java_lang_AbstractMethodError, methodname);
                    return NULL;
                }

                if(!checkMethodAccess(mb, class)) {
                    signalException(java_lang_IllegalAccessError,
                                    "method is not accessible");
                    return NULL;
                }

                CP_TYPE(cp, cp_index) = CONSTANT_Locked;
                MBARRIER();
                CP_INFO(cp, cp_index) = (uintptr_t)mb;
                MBARRIER();
                CP_TYPE(cp, cp_index) = CONSTANT_ResolvedMethod;
            } else
                signalException(java_lang_NoSuchMethodError, methodname);

            break;
        }
    }

    return mb;
}

MethodBlock *resolveInterfaceMethod(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    MethodBlock *mb = NULL;

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_Resolved:
            mb = (MethodBlock *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_InterfaceMethodref: {
            Class *resolved_class;
            char *methodname, *methodtype;
            int cl_idx = CP_METHOD_CLASS(cp, cp_index);
            int name_type_idx = CP_METHOD_NAME_TYPE(cp, cp_index);

            MBARRIER();
            if(CP_TYPE(cp, cp_index) != CONSTANT_InterfaceMethodref)
                goto retry;

            methodname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp, name_type_idx));
            methodtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp, name_type_idx));
            resolved_class = resolveClass(class, cl_idx, TRUE, FALSE);

            if(exceptionOccurred())
                return NULL;

            if(!(CLASS_CB(resolved_class)->access_flags & ACC_INTERFACE)) {
                signalException(java_lang_IncompatibleClassChangeError, NULL);
                return NULL;
            }
            
            mb = lookupInterfaceMethod(resolved_class, methodname, methodtype);

            if(mb != NULL) {
                CP_TYPE(cp, cp_index) = CONSTANT_Locked;
                MBARRIER();
                CP_INFO(cp, cp_index) = (uintptr_t)mb;
                MBARRIER();
                CP_TYPE(cp, cp_index) = CONSTANT_Resolved;
            } else
                signalException(java_lang_NoSuchMethodError, methodname);

            break;
        }
    }

    return mb;
}

FieldBlock *resolveField(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    FieldBlock *fb = NULL;

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_Resolved:
            fb = (FieldBlock *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_Fieldref: {
            Class *resolved_class;
            char *fieldname, *fieldtype;
            int cl_idx = CP_FIELD_CLASS(cp, cp_index);
            int name_type_idx = CP_FIELD_NAME_TYPE(cp, cp_index);

            MBARRIER();
            if(CP_TYPE(cp, cp_index) != CONSTANT_Fieldref)
                goto retry;

            fieldname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp, name_type_idx));
            fieldtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp, name_type_idx));
            resolved_class = resolveClass(class, cl_idx, TRUE, FALSE);

            if(exceptionOccurred())
                return NULL;

            fb = lookupField(resolved_class, fieldname, fieldtype);

            if(fb != NULL) {
                if(!checkFieldAccess(fb, class)) {
                    signalException(java_lang_IllegalAccessError,
                                    "field is not accessible");
                    return NULL;
                }

                CP_TYPE(cp, cp_index) = CONSTANT_Locked;
                MBARRIER();
                CP_INFO(cp, cp_index) = (uintptr_t)fb;
                MBARRIER();
                CP_TYPE(cp, cp_index) = CONSTANT_Resolved;
            } else
                signalException(java_lang_NoSuchFieldError, fieldname);

            break;
        }
    }
 
    return fb;
}

uintptr_t resolveSingleConstant(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_Class:
            resolveClass(class, cp_index, TRUE, FALSE);
            break;

#ifdef JSR292
        case CONSTANT_MethodType:
            resolveMethodType(class, cp_index);
            break;

        case CONSTANT_MethodHandle:
            resolveMethodHandle(class, cp_index);
            break;
#endif

        case CONSTANT_String: {
            Object *string;
            int idx = CP_STRING(cp, cp_index);

            MBARRIER();
            if(CP_TYPE(cp, cp_index) != CONSTANT_String)
                goto retry;

            string = createString(CP_UTF8(cp, idx));

            if(string) {
                CP_TYPE(cp, cp_index) = CONSTANT_Locked;
                MBARRIER();
                CP_INFO(cp, cp_index) = (uintptr_t)findInternedString(string);
                MBARRIER();
                CP_TYPE(cp, cp_index) = CONSTANT_ResolvedString;
            }

            break;
        }

        default:
            break;
    }
    
    return CP_INFO(cp, cp_index);
}

MethodBlock *lookupVirtualMethod(Object *ob, MethodBlock *mb) {
    ClassBlock *cb = CLASS_CB(ob->class);
    int mtbl_idx = mb->method_table_index;

    if(mb->access_flags & ACC_PRIVATE)
        return mb;

    if(CLASS_CB(mb->class)->access_flags & ACC_INTERFACE) {
        int i;

        for(i = 0; (i < cb->imethod_table_size) &&
                   (mb->class != cb->imethod_table[i].interface); i++);

        if(i == cb->imethod_table_size) {
            signalException(java_lang_IncompatibleClassChangeError,
                            "unimplemented interface");
            return NULL;
        }
        mtbl_idx = cb->imethod_table[i].offsets[mtbl_idx];
    }

    mb = cb->method_table[mtbl_idx];

    if(mb->access_flags & ACC_ABSTRACT) {
        signalException(java_lang_AbstractMethodError, mb->name);
        return NULL;
    }

    return mb;
}

/* This function is used when rewriting a field access bytecode
   in the direct-threaded interpreter.  We need to know how many
   slots are used on the stack, but the field reference may not
   be resolved, and resolving at preparation will break Java's
   lazy resolution semantics. */

#ifdef DIRECT
int peekIsFieldLong(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    char *type = NULL;

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_Resolved:
            type = ((FieldBlock *)CP_INFO(cp, cp_index))->type;
            break;

        case CONSTANT_Fieldref: {
            int name_type_idx = CP_FIELD_NAME_TYPE(cp, cp_index);

            if(CP_TYPE(cp, cp_index) != CONSTANT_Fieldref)
                goto retry;

            type = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp, name_type_idx));
            break;
        }
    }
 
    return *type == 'J' || *type == 'D';
}
#endif
