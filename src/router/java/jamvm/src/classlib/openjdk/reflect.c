/*
 * Copyright (C) 2010, 2011, 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
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
#include "class.h"
#include "excep.h"
#include "symbol.h"
#include "reflect.h"
#include "annotations.h"

/* Accessed from frame.c */
MethodBlock *mthd_invoke_mb;

/* Accessed from mh.c */
Class *cons_reflect_class, *method_reflect_class;
Class *field_reflect_class;

int cons_slot_offset, cons_class_offset;
int mthd_slot_offset, mthd_class_offset;
int fld_slot_offset, fld_class_offset;

static MethodBlock *cons_init_mb, *fld_init_mb, *mthd_init_mb;
static int cons_param_offset, mthd_ret_offset, mthd_param_offset;

#ifdef JSR901
static Class *parameter_array_class;
static MethodBlock *param_init_mb;
#endif

int classlibInitReflection() {
    Class *cons_ref_cls, *mthd_ref_cls, *fld_ref_cls;
    Class *prm_ary_cls;

    FieldBlock *fld_slot_fb, *fld_class_fb;
    FieldBlock *cons_slot_fb, *cons_class_fb, *cons_param_fb;
    FieldBlock *mthd_slot_fb, *mthd_class_fb, *mthd_ret_fb, *mthd_param_fb;

    cons_ref_cls = findSystemClass(SYMBOL(java_lang_reflect_Constructor));
    mthd_ref_cls = findSystemClass(SYMBOL(java_lang_reflect_Method));
    fld_ref_cls = findSystemClass(SYMBOL(java_lang_reflect_Field));

    if(!cons_ref_cls || !mthd_ref_cls || !fld_ref_cls)
        return FALSE;

    cons_slot_fb  = findField(cons_ref_cls, SYMBOL(slot), SYMBOL(I));
    cons_class_fb = findField(cons_ref_cls, SYMBOL(clazz),
                                            SYMBOL(sig_java_lang_Class));
    cons_param_fb = findField(cons_ref_cls, SYMBOL(parameterTypes),
                                            SYMBOL(array_java_lang_Class));

    mthd_slot_fb  = findField(mthd_ref_cls, SYMBOL(slot), SYMBOL(I));
    mthd_class_fb = findField(mthd_ref_cls, SYMBOL(clazz),
                                            SYMBOL(sig_java_lang_Class));
    mthd_ret_fb   = findField(mthd_ref_cls, SYMBOL(returnType),
                                            SYMBOL(sig_java_lang_Class));
    mthd_param_fb = findField(mthd_ref_cls, SYMBOL(parameterTypes),
                                            SYMBOL(array_java_lang_Class));

    fld_slot_fb   = findField(fld_ref_cls,  SYMBOL(slot), SYMBOL(I));
    fld_class_fb  = findField(fld_ref_cls,  SYMBOL(clazz),
                                            SYMBOL(sig_java_lang_Class));

    fld_init_mb = findMethod(fld_ref_cls, SYMBOL(object_init),
                             SYMBOL(java_lang_reflect_field_init_sig));

    cons_init_mb = findMethod(cons_ref_cls, SYMBOL(object_init),
                              SYMBOL(java_lang_reflect_cons_init_sig));

    mthd_init_mb = findMethod(mthd_ref_cls, SYMBOL(object_init),
                              SYMBOL(java_lang_reflect_mthd_init_sig));

    mthd_invoke_mb = findMethod(mthd_ref_cls, SYMBOL(invoke),
                                SYMBOL(java_lang_reflect_mthd_invoke_sig));

    if(!fld_init_mb    || !cons_init_mb  || !mthd_init_mb  ||
       !cons_slot_fb   || !cons_class_fb || !cons_param_fb ||
       !mthd_slot_fb   || !mthd_class_fb || !mthd_ret_fb   ||
       !mthd_param_fb  || !fld_slot_fb   || !fld_class_fb  ||
       !mthd_invoke_mb)  {
        /* Find Field/Method doesn't throw an exception... */
        signalException(java_lang_InternalError,
                        "Expected reflection method/field doesn't exist");
        return FALSE;
    }

    cons_slot_offset = cons_slot_fb->u.offset; 
    cons_class_offset = cons_class_fb->u.offset; 
    cons_param_offset = cons_param_fb->u.offset; 
    mthd_slot_offset = mthd_slot_fb->u.offset; 
    mthd_class_offset = mthd_class_fb->u.offset; 
    mthd_ret_offset = mthd_ret_fb->u.offset; 
    mthd_param_offset = mthd_param_fb->u.offset; 
    fld_slot_offset = fld_slot_fb->u.offset; 
    fld_class_offset = fld_class_fb->u.offset; 

#ifdef JSR901
    prm_ary_cls = findArrayClass(SYMBOL(array_java_lang_reflect_Parameter));

    if(!prm_ary_cls)
        return FALSE;

    param_init_mb = findMethod(CLASS_CB(prm_ary_cls)->element_class,
                               SYMBOL(object_init),
                               SYMBOL(java_lang_reflect_param_init_sig));

    if(!param_init_mb) {
        /* Find Method doesn't throw an exception... */
        signalException(java_lang_InternalError,
                        "Expected init method doesn't exist (Parameter)");
        return FALSE;
    }

    registerStaticClassRefLocked(&parameter_array_class, prm_ary_cls);
#endif

    registerStaticClassRefLocked(&cons_reflect_class, cons_ref_cls);
    registerStaticClassRefLocked(&method_reflect_class, mthd_ref_cls);
    registerStaticClassRefLocked(&field_reflect_class, fld_ref_cls);

    return TRUE;
}

Object *getAnnotationsAsArray(AttributeData *annotations) {
    Object *array;

    if(annotations == NULL)
        return NULL;

    if((array = allocTypeArray(T_BYTE, annotations->len)) == NULL)
        return NULL;

    memcpy(ARRAY_DATA(array, void*), annotations->data, annotations->len);

    return array;
}

Object *classlibCreateConstructorObject(MethodBlock *mb) {
    AttributeData *annotations = getMethodAnnotationData(mb);
    AttributeData *parameters = getMethodParameterAnnotationData(mb);
    Object *reflect_ob;

    if((reflect_ob = allocObject(cons_reflect_class)) == NULL)
        return NULL;

    executeMethod(reflect_ob, cons_init_mb,
        mb->class,
        getMethodParameterTypes(mb),
        getMethodExceptionTypes(mb),
        mb->access_flags,
        mb - CLASS_CB(mb->class)->methods,
        mb->signature == NULL ? NULL
                      : findInternedString(createString(mb->signature)),
        getAnnotationsAsArray(annotations),
        getAnnotationsAsArray(parameters));

    return reflect_ob;
}

Object *classlibCreateMethodObject(MethodBlock *mb) {
    AttributeData *annotations = getMethodAnnotationData(mb);
    AttributeData *dft_val = getMethodDefaultValueAnnotationData(mb);
    AttributeData *parameters = getMethodParameterAnnotationData(mb);
    Object *reflect_ob;

    if((reflect_ob = allocObject(method_reflect_class)) == NULL)
        return NULL;

    executeMethod(reflect_ob, mthd_init_mb,
        mb->class,
        findInternedString(createString(mb->name)),
        getMethodParameterTypes(mb),
        getMethodReturnType(mb),
        getMethodExceptionTypes(mb),
        mb->access_flags,
        mb - CLASS_CB(mb->class)->methods,
        mb->signature == NULL ? NULL
                      : findInternedString(createString(mb->signature)),
        getAnnotationsAsArray(annotations),
        getAnnotationsAsArray(parameters),
        getAnnotationsAsArray(dft_val));

    return reflect_ob;
}

Object *classlibCreateFieldObject(FieldBlock *fb) {
    AttributeData *annotations = getFieldAnnotationData(fb);
    Object *reflect_ob;

    if((reflect_ob = allocObject(field_reflect_class)) == NULL)
        return NULL;

    executeMethod(reflect_ob, fld_init_mb,
        fb->class,
        findInternedString(createString(fb->name)),
        getFieldType(fb),
        fb->access_flags,
        fb - CLASS_CB(fb->class)->fields,
        fb->signature == NULL ? NULL
                      : findInternedString(createString(fb->signature)),
        getAnnotationsAsArray(annotations));

    return reflect_ob;
}

Object *enclosingMethodInfo(Class *class) {
   ClassBlock *cb = CLASS_CB(class);
   Object *info = NULL;

   if(cb->enclosing_class) {
       Class *enc_class = resolveClass(class, cb->enclosing_class,
                                       TRUE, FALSE);

       if(enc_class != NULL) {
           Class *ary_class = findArrayClass(SYMBOL(array_java_lang_Object));

           if(ary_class != NULL) {
               info = allocArray(ary_class, 3, sizeof(Object*));

               if(info != NULL) {
                   ARRAY_DATA(info, Object*)[0] = enc_class;

                   if(cb->enclosing_method) {
                       ConstantPool *cp = &cb->constant_pool;
                       char *methodname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp,
                                                      cb->enclosing_method));
                       char *methodtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp,
                                                      cb->enclosing_method));
                       Object *name = createString(methodname);
                       Object *type = createString(methodtype);

                       if(name == NULL || type == NULL)
                           return NULL;

                       ARRAY_DATA(info, Object*)[1] = name;
                       ARRAY_DATA(info, Object*)[2] = type;
                   }
               }
           }
       }
   }

   return info;
}

Object *consNewInstance(Object *reflect_ob, Object *args_array) {
    Object *params = INST_DATA(reflect_ob, Object*, cons_param_offset);
    Class *decl_class = INST_DATA(reflect_ob, Class*, cons_class_offset);
    int slot = INST_DATA(reflect_ob, int, cons_slot_offset);
    MethodBlock *mb = &(CLASS_CB(decl_class)->methods[slot]);

    return constructorConstruct(mb, args_array, params, TRUE, 0);
}

Object *invokeMethod(Object *reflect_ob, Object *ob, Object *args_array) {
    Object *ret = INST_DATA(reflect_ob, Object*, mthd_ret_offset);
    Object *params = INST_DATA(reflect_ob, Object*, mthd_param_offset);
    Class *decl_class = INST_DATA(reflect_ob, Class*, mthd_class_offset);
    int slot = INST_DATA(reflect_ob, int, mthd_slot_offset);
    MethodBlock *mb = &(CLASS_CB(decl_class)->methods[slot]);

    return methodInvoke(ob, mb, args_array, ret, params, TRUE, 0);
}

int typeNo2PrimTypeIndex(int type_no) {
    static char type_map[] = {PRIM_IDX_BOOLEAN, PRIM_IDX_CHAR,
                              PRIM_IDX_FLOAT, PRIM_IDX_DOUBLE,
                              PRIM_IDX_BYTE, PRIM_IDX_SHORT,
                              PRIM_IDX_INT, PRIM_IDX_LONG};

    return type_map[type_no - T_BOOLEAN];
}

char primClass2TypeChar(Class *prim) {
    static char type_char[] = {'V', 'Z', 'B', 'C', 'S',
                               'I', 'F', 'J', 'D'};
    return type_char[getPrimTypeIndex(CLASS_CB(prim))];
}

int primTypeIndex2Size(int prim_idx) {
    return prim_idx < PRIM_IDX_INT ? prim_idx < PRIM_IDX_CHAR ? 1 : 2
                                   : prim_idx < PRIM_IDX_LONG ? 4 : 8;
}

int widenPrimitiveElement(int src_idx, int dst_idx, void *src_addr,
                          void *dst_addr) {
    u4 widened;

    if(src_idx < PRIM_IDX_INT) {
        if(dst_idx < PRIM_IDX_INT) {
            if(src_idx != dst_idx) {
                if(src_idx != PRIM_IDX_BYTE || dst_idx != PRIM_IDX_SHORT)
                    goto error;
                *(signed short*)dst_addr = *(signed char*)src_addr;
                return TRUE;
            }
            
            if(src_idx < PRIM_IDX_CHAR)
                *(char*)dst_addr = *(char*)src_addr;
            else
                *(short*)dst_addr = *(short*)src_addr;
            return TRUE;
        }

        widened = src_idx < PRIM_IDX_CHAR ? *(signed char*)src_addr : 
                   src_idx == PRIM_IDX_SHORT ? *(signed short*)src_addr
                                             : *(unsigned short*)src_addr;
        src_addr = &widened;
    }

    if(widenPrimitiveValue(src_idx, dst_idx, src_addr, dst_addr,
                           REF_SRC_FIELD | REF_DST_FIELD))
        return TRUE;

error:
    signalException(java_lang_IllegalArgumentException, "can't widen");
    return FALSE;
}

/* Reflection access from JNI */

MethodBlock *classlibMbFromReflectObject(Object *reflect_ob) {
    int is_cons = reflect_ob->class == cons_reflect_class;
    int slot_offset = is_cons ? cons_slot_offset : mthd_slot_offset;
    int class_offset = is_cons ? cons_class_offset : mthd_class_offset;

    Class *decl_class = INST_DATA(reflect_ob, Class*, class_offset);
    int slot = INST_DATA(reflect_ob, int, slot_offset);

    return &(CLASS_CB(decl_class)->methods[slot]);
}

FieldBlock *classlibFbFromReflectObject(Object *reflect_ob) {
    Class *decl_class = INST_DATA(reflect_ob, Class*, fld_class_offset);
    int slot = INST_DATA(reflect_ob, int, fld_slot_offset);

    return &(CLASS_CB(decl_class)->fields[slot]);
}

#ifdef JSR901
Object *getMethodParameters(Object *method) {
    Object *params = NULL;
    MethodBlock *mb = classlibMbFromReflectObject(method);
    AttributeData *attr = METHOD_EXTRA_ATTRIBUTES(mb, method_parameters);

    if(attr != NULL) {
        u1 *data = attr->data;
        int len = attr->len;
        int no_params;

        READ_U1(no_params, data, len);
        params = allocArray(parameter_array_class, no_params, sizeof(Object*));

        if(params != NULL) {
            ConstantPool *cp = &CLASS_CB(mb->class)->constant_pool;
            Object **params_data = ARRAY_DATA(params, Object*);
            int i;

            for(i = 0; i < no_params; i++) {
                Object *param = allocObject(param_init_mb->class);
                int name_idx, access_flags;
                Object *name_str = NULL;

                if(param == NULL)
                    return NULL;

                READ_U2(name_idx, data, len);
                READ_U2(access_flags, data, len);

                if(name_idx != 0) {
                    name_str = createString(CP_UTF8(cp, name_idx));
                    if(name_str == NULL)
                        return NULL;
                }
                    
                executeMethod(param, param_init_mb, name_str,
                              access_flags, method, i);

                params_data[i] = param;
            }
        }
    }

    return params;
}
#endif

