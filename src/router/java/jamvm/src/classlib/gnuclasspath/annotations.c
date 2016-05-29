/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012, 2013
 * 2014 Robert Lougher <rob@jamvm.org.uk>.
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

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

static char anno_inited = FALSE;

static Class *enum_class, *map_class, *anno_inv_class, *obj_array_class;
static Class *anno_array_class, *dbl_anno_array_class;
static MethodBlock *map_init_mb, *map_put_mb, *anno_create_mb, *enum_valueof_mb;

static int initAnnotation() {
    Class *enum_cls, *map_cls, *anno_inv_cls, *obj_ary_cls;
    Class *anno_ary_cls, *dbl_anno_ary_cls;

    enum_cls = findSystemClass("java/lang/Enum");
    map_cls = findSystemClass("java/util/HashMap");
    anno_inv_cls = findSystemClass("sun/reflect/annotation/Annotation"
                                   "InvocationHandler");

    obj_ary_cls = findArrayClass("[Ljava/lang/Object;");
    anno_ary_cls = findArrayClass("[Ljava/lang/annotation/Annotation;");
    dbl_anno_ary_cls = findArrayClass("[[Ljava/lang/annotation/Annotation;");

    if(!enum_cls || !map_cls || !anno_inv_cls || !obj_ary_cls 
                 || !anno_ary_cls || !dbl_anno_ary_cls)
        return FALSE;

    map_init_mb = findMethod(map_cls, SYMBOL(object_init), SYMBOL(___V));
    map_put_mb = findMethod(map_cls, SYMBOL(put),
                            newUtf8("(Ljava/lang/Object;Ljava/lang/Object;)"
                                    "Ljava/lang/Object;"));

    anno_create_mb = findMethod(anno_inv_cls, newUtf8("create"),
                                newUtf8("(Ljava/lang/Class;Ljava/util/Map;)"
                                        "Ljava/lang/annotation/Annotation;"));

    enum_valueof_mb = findMethod(enum_cls, newUtf8("valueOf"),
                                 newUtf8("(Ljava/lang/Class;Ljava/lang/String;)"
                                         "Ljava/lang/Enum;"));

    if(!map_init_mb || !map_put_mb || !anno_create_mb || !enum_valueof_mb) {

        /* FindMethod doesn't throw an exception... */
        signalException(java_lang_InternalError,
                        "Expected field/method doesn't exist");
        return FALSE;
    }

    registerStaticClassRefLocked(&enum_class, enum_cls);
    registerStaticClassRefLocked(&map_class, map_cls);
    registerStaticClassRefLocked(&anno_inv_class, anno_inv_cls);
    registerStaticClassRefLocked(&obj_array_class, obj_ary_cls);
    registerStaticClassRefLocked(&anno_array_class, anno_ary_cls);
    registerStaticClassRefLocked(&dbl_anno_array_class, dbl_anno_ary_cls);

    return anno_inited = TRUE;
}

/* Forward declarations */
Object *parseAnnotation(Class *class, u1 **data_ptr, int *data_len);

Object *parseElementValue(Class *class, u1 **data_ptr, int *data_len) {
    ClassBlock *cb = CLASS_CB(class);
    ConstantPool *cp = &cb->constant_pool;
    char tag;

    READ_U1(tag, *data_ptr, *data_len);

    switch(tag) {
        default: {
            int cp_tag = CONSTANT_Integer;
            int prim_type_no = 0;
            int const_val_idx;

            switch(tag) {
                case 'Z':
                    prim_type_no = PRIM_IDX_BOOLEAN;
                    break;
                case 'B':
                    prim_type_no = PRIM_IDX_BYTE;
                    break;
                case 'C':
                    prim_type_no = PRIM_IDX_CHAR;
                    break;
                case 'S':
                    prim_type_no = PRIM_IDX_SHORT;
                    break;
                case 'I':
                    prim_type_no = PRIM_IDX_INT;
                    break;
                case 'F':
                    cp_tag = CONSTANT_Float;
                    prim_type_no = PRIM_IDX_FLOAT;
                    break;
                case 'J':
                    cp_tag = CONSTANT_Long;
                    prim_type_no = PRIM_IDX_LONG;
                    break;
                case 'D':
                    cp_tag = CONSTANT_Double;
                    prim_type_no = PRIM_IDX_DOUBLE;
                    break;
            }

            READ_TYPE_INDEX(const_val_idx, cp, cp_tag, *data_ptr, *data_len);

            return createWrapperObject(prim_type_no,
                                       &CP_INFO(cp, const_val_idx),
                                       REF_SRC_OSTACK);
        }

        case 's': {
            int const_str_idx;

            READ_TYPE_INDEX(const_str_idx, cp, CONSTANT_Utf8, *data_ptr,
                            *data_len);

            return createString(CP_UTF8(cp, const_str_idx));
        }

        case 'e': {
            int type_name_idx, const_name_idx;
            Object *const_name, *enum_obj;
            Class *type_class;

            READ_TYPE_INDEX(type_name_idx, cp, CONSTANT_Utf8, *data_ptr,
                            *data_len);
            READ_TYPE_INDEX(const_name_idx, cp, CONSTANT_Utf8, *data_ptr,
                            *data_len);
            type_class = findClassFromSignature(CP_UTF8(cp, type_name_idx),
                                                class);
            const_name = createString(CP_UTF8(cp, const_name_idx));

            if(type_class == NULL || const_name == NULL)
                return NULL;

            enum_obj = *(Object**)executeStaticMethod(enum_class,
                                                      enum_valueof_mb,
                                                      type_class, const_name);
            if(exceptionOccurred())
                return NULL;

            return enum_obj;
        }

        case 'c': {
            int class_info_idx;
            READ_TYPE_INDEX(class_info_idx, cp, CONSTANT_Utf8, *data_ptr,
                            *data_len);
            return findClassFromSignature(CP_UTF8(cp, class_info_idx), class);
        }

        case '@':
            return parseAnnotation(class, data_ptr, data_len);

        case '[': {
            Object *array;
            Object **array_data;
            int i, num_values;

            READ_U2(num_values, *data_ptr, *data_len);
            if((array = allocArray(obj_array_class, num_values,
                                   sizeof(Object*))) == NULL)
                return NULL;

            array_data = ARRAY_DATA(array, Object*);

            for(i = 0; i < num_values; i++)
                if((array_data[i] = parseElementValue(class, data_ptr,
                                                      data_len)) == NULL)
                    return NULL;

            return array;
        }
    }
}

Object *parseAnnotation(Class *class, u1 **data_ptr, int *data_len) {
    ClassBlock *cb = CLASS_CB(class);
    ConstantPool *cp = &cb->constant_pool;
    Object *map, *anno;
    int no_value_pairs;
    Class *type_class;
    int type_idx;
    int i;

    if((map = allocObject(map_class)) == NULL)
        return NULL;

    executeMethod(map, map_init_mb);
    if(exceptionOccurred())
        return NULL;

    READ_TYPE_INDEX(type_idx, cp, CONSTANT_Utf8, *data_ptr, *data_len);
    if((type_class = findClassFromSignature(CP_UTF8(cp, type_idx),
                                            class)) == NULL)
        return NULL;

    READ_U2(no_value_pairs, *data_ptr, *data_len);

    for(i = 0; i < no_value_pairs; i++) {
        Object *element_name, *element_value;
        int element_name_idx;

        READ_TYPE_INDEX(element_name_idx, cp, CONSTANT_Utf8, *data_ptr,
                        *data_len);

        element_name = createString(CP_UTF8(cp, element_name_idx));
        element_value = parseElementValue(class, data_ptr, data_len);
        if(element_name == NULL || element_value == NULL)
            return NULL;

        executeMethod(map, map_put_mb, element_name, element_value);
        if(exceptionOccurred())
            return NULL;
    }

    anno = *(Object**)executeStaticMethod(anno_inv_class, anno_create_mb,
                                          type_class, map);
    if(exceptionOccurred())
        return NULL;

    return anno;
}

Object *parseAnnotations(Class *class, AttributeData *annotations) {
    if(!anno_inited && !initAnnotation())
        return NULL;

    if(annotations == NULL)
        return allocArray(anno_array_class, 0, sizeof(Object*));
    else {
        u1 *data_ptr = annotations->data;
        int data_len = annotations->len;
        Object **array_data;
        Object *array;
        int no_annos;
        int i;

        READ_U2(no_annos, data_ptr, data_len);
        if((array = allocArray(anno_array_class, no_annos,
                               sizeof(Object*))) == NULL)
            return NULL;

        array_data = ARRAY_DATA(array, Object*);

        for(i = 0; i < no_annos; i++)
            if((array_data[i] = parseAnnotation(class, &data_ptr,
                                                &data_len)) == NULL)
                return NULL;

        return array;
    }
}

Object *getClassAnnotations(Class *class) {
    return parseAnnotations(class, getClassAnnotationData(class));
}

Object *getFieldAnnotations(FieldBlock *fb) {
    return parseAnnotations(fb->class, getFieldAnnotationData(fb));
}

Object *getMethodAnnotations(MethodBlock *mb) {
    return parseAnnotations(mb->class, getMethodAnnotationData(mb));
}

Object *getMethodParameterAnnotations(MethodBlock *mb) {
    AttributeData *annotations;
    Object **outer_array_data;
    Object *outer_array;
    int no_params, i;
    u1 *data_ptr;
    int data_len;

    if(!anno_inited && !initAnnotation())
        return NULL;

    annotations = getMethodParameterAnnotationData(mb);
    if(annotations == NULL) {
        no_params = numElementsInSig(mb->type);
        data_len = no_params * 2 + 1;
        data_ptr = alloca(data_len);
        memset(data_ptr, 0, data_len);
        data_ptr[0] = no_params;
    } else {
        data_ptr = annotations->data;
        data_len = annotations->len;
    }

    READ_U1(no_params, data_ptr, data_len);
    if((outer_array = allocArray(dbl_anno_array_class, no_params,
                                 sizeof(Object*))) == NULL)
        return NULL;

    outer_array_data = ARRAY_DATA(outer_array, Object*);

    for(i = 0; i < no_params; i++) {
        Object **inner_array_data;
        Object *inner_array;
        int no_annos, j;

        READ_U2(no_annos, data_ptr, data_len);
        if((inner_array = allocArray(anno_array_class, no_annos,
                                     sizeof(Object*))) == NULL)
            return NULL;

        inner_array_data = ARRAY_DATA(inner_array, Object*);

        for(j = 0; j < no_annos; j++)
            if((inner_array_data[j] = parseAnnotation(mb->class, &data_ptr,
                                                      &data_len)) == NULL)
                return NULL;

        outer_array_data[i] = inner_array;
    }
    return outer_array;
}

Object *getMethodDefaultValue(MethodBlock *mb) {
    AttributeData *annotations;

    if(!anno_inited && !initAnnotation())
        return NULL;

    annotations = getMethodDefaultValueAnnotationData(mb);
    if(annotations == NULL)
        return NULL;
    else {
        u1 *data = annotations->data;
        int len = annotations->len;

        return parseElementValue(mb->class, &data, &len);
    }
}
