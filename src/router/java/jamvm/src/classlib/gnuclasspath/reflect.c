/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013
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

#include "jam.h"
#include "excep.h"
#include "symbol.h"
#include "reflect.h"

static Class *cons_reflect_class, *method_reflect_class;
static Class *field_reflect_class, *vmcons_reflect_class;
static Class *vmmethod_reflect_class, *vmfield_reflect_class;

static int vm_cons_slot_offset, vm_cons_class_offset, vm_cons_param_offset;
static int vm_cons_cons_offset, vm_mthd_slot_offset, vm_mthd_class_offset;
static int vm_mthd_ret_offset, vm_mthd_param_offset, vm_mthd_m_offset;
static int vm_fld_slot_offset, vm_fld_class_offset, vm_fld_type_offset;
static int vm_fld_f_offset, cons_cons_offset, mthd_m_offset;
static int fld_f_offset, acc_flag_offset;

int classlibInitReflection() {
    Class *cons_ref_cls, *mthd_ref_cls, *fld_ref_cls;
    Class *vm_cons_cls, *vm_mthd_cls, *vm_fld_cls;

    FieldBlock *vm_cons_slot_fb, *vm_cons_class_fb, *vm_cons_param_fb;
    FieldBlock *vm_cons_cons_fb, *vm_mthd_slot_fb, *vm_mthd_class_fb;
    FieldBlock *vm_mthd_ret_fb, *vm_mthd_param_fb, *vm_mthd_m_fb;
    FieldBlock *vm_fld_slot_fb, *vm_fld_class_fb, *vm_fld_type_fb;
    FieldBlock *vm_fld_f_fb, *cons_cons_fb, *mthd_m_fb, *fld_f_fb;
    FieldBlock *acc_flag_fb;

    cons_ref_cls = findSystemClass(SYMBOL(java_lang_reflect_Constructor));
    vm_cons_cls = findSystemClass(SYMBOL(java_lang_reflect_VMConstructor));
    mthd_ref_cls = findSystemClass(SYMBOL(java_lang_reflect_Method));
    vm_mthd_cls = findSystemClass(SYMBOL(java_lang_reflect_VMMethod));
    fld_ref_cls = findSystemClass(SYMBOL(java_lang_reflect_Field));
    vm_fld_cls = findSystemClass(SYMBOL(java_lang_reflect_VMField));

    if(!cons_ref_cls || !mthd_ref_cls || !fld_ref_cls
                     || !vm_cons_cls  || !vm_mthd_cls  || !vm_fld_cls)
        return FALSE;

    vm_cons_slot_fb  = findField(vm_cons_cls, SYMBOL(slot), SYMBOL(I));
    vm_cons_class_fb = findField(vm_cons_cls, SYMBOL(clazz),
                                              SYMBOL(sig_java_lang_Class));
    vm_cons_param_fb = findField(vm_cons_cls, SYMBOL(parameterTypes),
                                              SYMBOL(array_java_lang_Class));
    vm_cons_cons_fb  = findField(vm_cons_cls, SYMBOL(cons),
                                              SYMBOL(sig_java_lang_reflect_Constructor));

    vm_mthd_slot_fb  = findField(vm_mthd_cls, SYMBOL(slot), SYMBOL(I));
    vm_mthd_class_fb = findField(vm_mthd_cls, SYMBOL(clazz),
                                              SYMBOL(sig_java_lang_Class));
    vm_mthd_ret_fb   = findField(vm_mthd_cls, SYMBOL(returnType),
                                              SYMBOL(sig_java_lang_Class));
    vm_mthd_param_fb = findField(vm_mthd_cls, SYMBOL(parameterTypes),
                                              SYMBOL(array_java_lang_Class));
    vm_mthd_m_fb     = findField(vm_mthd_cls, SYMBOL(m),
                                              SYMBOL(sig_java_lang_reflect_Method));

    vm_fld_slot_fb   = findField(vm_fld_cls,  SYMBOL(slot), SYMBOL(I));
    vm_fld_class_fb  = findField(vm_fld_cls,  SYMBOL(clazz),
                                              SYMBOL(sig_java_lang_Class));
    vm_fld_type_fb   = findField(vm_fld_cls,  SYMBOL(type),
                                              SYMBOL(sig_java_lang_Class));
    vm_fld_f_fb      = findField(vm_fld_cls,  SYMBOL(f),
                                              SYMBOL(sig_java_lang_reflect_Field));

    cons_cons_fb  = findField(cons_ref_cls, SYMBOL(cons),
                                            SYMBOL(sig_java_lang_reflect_VMConstructor));
    mthd_m_fb     = findField(mthd_ref_cls, SYMBOL(m),
                                            SYMBOL(sig_java_lang_reflect_VMMethod));
    fld_f_fb      = findField(fld_ref_cls,  SYMBOL(f),
                                            SYMBOL(sig_java_lang_reflect_VMField));

    acc_flag_fb = lookupField(cons_ref_cls, SYMBOL(flag), SYMBOL(Z));

    if(!vm_cons_slot_fb   || !vm_cons_class_fb || !vm_cons_param_fb ||
         !vm_cons_cons_fb || !vm_mthd_slot_fb  || !vm_mthd_class_fb ||
         !vm_mthd_ret_fb  || !vm_mthd_m_fb     || !vm_mthd_param_fb ||
         !vm_fld_slot_fb  || !vm_fld_class_fb  || !vm_fld_type_fb   ||
         !vm_fld_f_fb     || !cons_cons_fb     || !mthd_m_fb        ||
         !fld_f_fb        || !acc_flag_fb) {

        /* Find Field/Method doesn't throw an exception... */
        signalException(java_lang_InternalError,
                        "Expected reflection field doesn't exist");
        return FALSE;
    }

    vm_cons_slot_offset = vm_cons_slot_fb->u.offset; 
    vm_cons_class_offset = vm_cons_class_fb->u.offset; 
    vm_cons_param_offset = vm_cons_param_fb->u.offset; 
    vm_cons_cons_offset = vm_cons_cons_fb->u.offset; 
    vm_mthd_slot_offset = vm_mthd_slot_fb->u.offset; 
    vm_mthd_class_offset = vm_mthd_class_fb->u.offset; 
    vm_mthd_ret_offset = vm_mthd_ret_fb->u.offset; 
    vm_mthd_param_offset = vm_mthd_param_fb->u.offset; 
    vm_mthd_m_offset = vm_mthd_m_fb->u.offset; 
    vm_fld_slot_offset = vm_fld_slot_fb->u.offset; 
    vm_fld_class_offset = vm_fld_class_fb->u.offset; 
    vm_fld_type_offset = vm_fld_type_fb->u.offset; 
    vm_fld_f_offset = vm_fld_f_fb->u.offset; 
    cons_cons_offset = cons_cons_fb->u.offset; 
    mthd_m_offset = mthd_m_fb->u.offset; 
    fld_f_offset = fld_f_fb->u.offset; 
    acc_flag_offset = acc_flag_fb->u.offset;

    registerStaticClassRefLocked(&cons_reflect_class, cons_ref_cls);
    registerStaticClassRefLocked(&vmcons_reflect_class, vm_cons_cls);
    registerStaticClassRefLocked(&method_reflect_class, mthd_ref_cls);
    registerStaticClassRefLocked(&vmmethod_reflect_class, vm_mthd_cls);
    registerStaticClassRefLocked(&field_reflect_class, fld_ref_cls);
    registerStaticClassRefLocked(&vmfield_reflect_class, vm_fld_cls);

    return TRUE;
}

Object *classlibCreateConstructorObject(MethodBlock *mb) {
    Object *reflect_ob, *vm_reflect_ob;

    if((reflect_ob = allocObject(cons_reflect_class)) == NULL)
        return NULL;

    if((vm_reflect_ob = allocObject(vmcons_reflect_class)) == NULL)
        return NULL;

    INST_DATA(vm_reflect_ob, Class*, vm_cons_class_offset) = mb->class;
    INST_DATA(vm_reflect_ob, int, vm_cons_slot_offset) =
                                     mb - CLASS_CB(mb->class)->methods;

    /* Link the Java-level and VM-level objects together */
    INST_DATA(vm_reflect_ob, Object*, vm_cons_cons_offset) = reflect_ob;
    INST_DATA(reflect_ob, Object*, cons_cons_offset) = vm_reflect_ob;

    return reflect_ob;
}

Object *classlibCreateMethodObject(MethodBlock *mb) {
    Object *reflect_ob, *vm_reflect_ob;

    if((reflect_ob = allocObject(method_reflect_class)) == NULL)
        return NULL;

    if((vm_reflect_ob = allocObject(vmmethod_reflect_class)) == NULL)
        return NULL;

    INST_DATA(vm_reflect_ob, Class*, vm_mthd_class_offset) = mb->class;
    INST_DATA(vm_reflect_ob, int, vm_mthd_slot_offset) =
                                     mb - CLASS_CB(mb->class)->methods;

    /* Link the Java-level and VM-level objects together */
    INST_DATA(vm_reflect_ob, Object*, vm_mthd_m_offset) = reflect_ob;
    INST_DATA(reflect_ob, Object*, mthd_m_offset) = vm_reflect_ob;

    return reflect_ob;
}

Object *classlibCreateFieldObject(FieldBlock *fb) {
    Object *reflect_ob, *vm_reflect_ob;

    if((reflect_ob = allocObject(field_reflect_class)) == NULL)
        return NULL;

    if((vm_reflect_ob = allocObject(vmfield_reflect_class)) == NULL)
        return NULL;

    INST_DATA(vm_reflect_ob, Class*, vm_fld_class_offset) = fb->class;
    INST_DATA(vm_reflect_ob, int, vm_fld_slot_offset) =
                                     fb - CLASS_CB(fb->class)->fields;

    /* Link the Java-level and VM-level objects together */
    INST_DATA(vm_reflect_ob, Object*, vm_fld_f_offset) = reflect_ob;
    INST_DATA(reflect_ob, Object*, fld_f_offset) = vm_reflect_ob;

    return reflect_ob;
}

/* Functions to get values from the VM-level reflection objects */

MethodBlock *getVMConsMethodBlock(Object *vm_cons_obj) {
    Class *decl_class = INST_DATA(vm_cons_obj, Class*, vm_cons_class_offset);
    int slot = INST_DATA(vm_cons_obj, int, vm_cons_slot_offset);

    return &CLASS_CB(decl_class)->methods[slot];
}

int getVMConsAccessFlag(Object *vm_cons_obj) {
    Object *cons_obj = INST_DATA(vm_cons_obj, Object*, vm_cons_cons_offset);
    return INST_DATA(cons_obj, int, acc_flag_offset);
}

int getVMMethodAccessFlag(Object *vm_mthd_obj) {
    Object *mthd_obj = INST_DATA(vm_mthd_obj, Object*, vm_mthd_m_offset);
    return INST_DATA(mthd_obj, int, acc_flag_offset);
}

MethodBlock *getVMMethodMethodBlock(Object *vm_mthd_obj) {
    Class *decl_class = INST_DATA(vm_mthd_obj, Class*, vm_mthd_class_offset);
    int slot = INST_DATA(vm_mthd_obj, int, vm_mthd_slot_offset);

    return &CLASS_CB(decl_class)->methods[slot];
}

FieldBlock *getVMFieldFieldBlock(Object *vm_fld_obj) {
    Class *decl_class = INST_DATA(vm_fld_obj, Class*, vm_fld_class_offset);
    int slot = INST_DATA(vm_fld_obj, int, vm_fld_slot_offset);

    return &(CLASS_CB(decl_class)->fields[slot]);
}

int getVMFieldAccessFlag(Object *vm_fld_obj) {
    Object *fld_obj = INST_DATA(vm_fld_obj, Object*, vm_fld_f_offset);
    return INST_DATA(fld_obj, int, acc_flag_offset);
}

Object *getVMConsParamTypes(Object *vm_cons_obj) {
    Object *params = INST_DATA(vm_cons_obj, Object*, vm_cons_param_offset);

    if(params == NULL) {
        MethodBlock *mb = getVMConsMethodBlock(vm_cons_obj);

        params = getMethodParameterTypes(mb);
        INST_DATA(vm_cons_obj, Object*, vm_cons_param_offset) = params;
    }

    return params;
}

Object *getVMMethodParamTypes(Object *vm_mthd_obj) {
    Object *params = INST_DATA(vm_mthd_obj, Object*, vm_mthd_param_offset);

    if(params == NULL) {
        MethodBlock *mb = getVMMethodMethodBlock(vm_mthd_obj);

        params = getMethodParameterTypes(mb);
        INST_DATA(vm_mthd_obj, Object*, vm_mthd_param_offset) = params;
    }

    return params;
}

Class *getVMMethodReturnType(Object *vm_mthd_obj) {
    Class *ret = INST_DATA(vm_mthd_obj, Class*, vm_mthd_ret_offset);

    if(ret == NULL) {
        MethodBlock *mb = getVMMethodMethodBlock(vm_mthd_obj);

        ret = getMethodReturnType(mb);
        INST_DATA(vm_mthd_obj, Class*, vm_mthd_ret_offset) = ret;
    }

    return ret;
}

Class *getVMFieldType(Object *vm_field_obj) {
    Class *type = INST_DATA(vm_field_obj, Class*, vm_fld_type_offset);

    if(type == NULL) {
        FieldBlock *fb = getVMFieldFieldBlock(vm_field_obj);

        type = getFieldType(fb);
        INST_DATA(vm_field_obj, Class*, vm_fld_type_offset) = type;
    }

    return type;
}

MethodBlock *classlibMbFromReflectObject(Object *reflect_ob) {
    MethodBlock *mb;

    if(reflect_ob->class == cons_reflect_class) {
        Object *vm_cons_obj = INST_DATA(reflect_ob, Object*, cons_cons_offset);
        mb = getVMConsMethodBlock(vm_cons_obj);
    } else {
        Object *vm_mthd_obj = INST_DATA(reflect_ob, Object*, mthd_m_offset);
        mb = getVMMethodMethodBlock(vm_mthd_obj);
    }

    return mb;
}

FieldBlock *classlibFbFromReflectObject(Object *reflect_ob) {
    Object *vm_fld_obj = INST_DATA(reflect_ob, Object*, fld_f_offset);
    return getVMFieldFieldBlock(vm_fld_obj);
}

Class *getEnclosingClass(Class *class) {
    ClassBlock *cb = CLASS_CB(class);
    int encl_class_idx = cb->enclosing_class;

    /* A member class doesn't have an EnclosingMethod attribute,
       so the enclosing class is the declaring class */
    if(!encl_class_idx)
        encl_class_idx = cb->declaring_class;

    if(!encl_class_idx)
        return NULL;

    return resolveClass(class, encl_class_idx, FALSE, TRUE);
}

MethodBlock *getEnclosingMethod(Class *class) {
    ClassBlock *cb = CLASS_CB(class);

    if(cb->enclosing_class && cb->enclosing_method) {
        Class *enclosing_class = resolveClass(class, cb->enclosing_class,
                                              FALSE, TRUE);

        if(enclosing_class != NULL) {
            ConstantPool *cp = &cb->constant_pool;
            char *methodname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp,
                                               cb->enclosing_method));
            char *methodtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp,
                                               cb->enclosing_method));
            MethodBlock *mb = findMethod(enclosing_class, methodname,
                                         methodtype);

            if(mb != NULL)
                return mb;

            /* The "reference implementation" throws an InternalError if a
               method with the name and type cannot be found in the enclosing
               class */
            signalException(java_lang_InternalError,
                            "Enclosing method doesn't exist");
        }
    }

    return NULL;
}

Object *getEnclosingMethodObject(Class *class) {
    MethodBlock *mb = getEnclosingMethod(class);

    if(mb != NULL && mb->name != SYMBOL(object_init))
        return classlibCreateMethodObject(mb);

    return NULL;
}

Object *getEnclosingConstructorObject(Class *class) {
    MethodBlock *mb = getEnclosingMethod(class);

    if(mb != NULL && mb->name == SYMBOL(object_init))
        return classlibCreateConstructorObject(mb);

    return NULL;
}

/* Needed for stack walking.  Walking backwards, the
   first class we will see is VMMethod */

Class *getReflectMethodClass() {
    return vmmethod_reflect_class;
}
