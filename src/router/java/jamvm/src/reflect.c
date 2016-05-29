/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jam.h"
#include "frame.h"
#include "lock.h"
#include "hash.h"
#include "class.h"
#include "symbol.h"
#include "excep.h"
#include "thread.h"
#include "reflect.h"
#include "classlib.h"
#include "properties.h"

static char inited = FALSE;

static Class *class_array_class, *cons_array_class;
static Class *method_array_class, *field_array_class;

static int initReflection() {
    Class *cls_ary_cls, *cons_ary_cls, *mthd_ary_cls, *fld_ary_cls;

    cls_ary_cls  = findArrayClass(SYMBOL(array_java_lang_Class));
    cons_ary_cls = findArrayClass(SYMBOL(array_java_lang_reflect_Constructor));
    mthd_ary_cls = findArrayClass(SYMBOL(array_java_lang_reflect_Method));
    fld_ary_cls  = findArrayClass(SYMBOL(array_java_lang_reflect_Field));

    if(!cls_ary_cls || !cons_ary_cls || !mthd_ary_cls || !fld_ary_cls)
        return FALSE;

    registerStaticClassRefLocked(&class_array_class, cls_ary_cls);
    registerStaticClassRefLocked(&cons_array_class, cons_ary_cls);
    registerStaticClassRefLocked(&method_array_class, mthd_ary_cls);
    registerStaticClassRefLocked(&field_array_class, fld_ary_cls);

    if(!classlibInitReflection())
        return FALSE;

    return inited = TRUE;
}

Class *convertSigElement2Class(char **sig_pntr, Class *declaring_class) {
    char *sig = *sig_pntr;
    Class *class;

    switch(*sig) {
        case '[': {
            char next;
            while(*++sig == '[');
            if(*sig == 'L')
                while(*++sig != ';');
            next = *++sig;
            *sig = '\0';
            class = findArrayClassFromClass(*sig_pntr, declaring_class);
            *sig = next;
            break;
        }

        case 'L':
            while(*++sig != ';');
            *sig++ = '\0';
            class = findClassFromClass((*sig_pntr)+1, declaring_class);
            break;

        default:
            class = findPrimitiveClass(*sig++);
            break;
    }

    *sig_pntr = sig;
    return class;
}

int numElementsInSig(char *sig) {
    int no_params;

    for(no_params = 0; *++sig != ')'; no_params++) {
        if(*sig == '[')
            while(*++sig == '[');
        if(*sig == 'L')
            while(*++sig != ';');
    }

    return no_params;
}

Object *convertSig2ClassArray(char **sig_pntr, Class *declaring_class) {
    int no_params = numElementsInSig(*sig_pntr);
    Class **params;
    Object *array;
    int i = 0;

    if((array = allocArray(class_array_class, no_params, sizeof(Class*))) == NULL)
        return NULL;

    params = ARRAY_DATA(array, Class*);

    *sig_pntr += 1;
    while(**sig_pntr != ')')
        if((params[i++] = convertSigElement2Class(sig_pntr, declaring_class)) == NULL)
            return NULL;

    return array;
}

Class *findClassFromSignature(char *type_name, Class *class) {
    switch(type_name[0]) {
        case 'L': {
            int len = strlen(type_name);
            char name[len - 1];

            memcpy(name, type_name + 1, len - 2);
            name[len - 2] = '\0';

            return findClassFromClass(name, class);
        }

        case '[':
            return findArrayClassFromClass(type_name, class);

        default:
            return findPrimitiveClass(type_name[0]);
    }
}

Class *getFieldType(FieldBlock *fb) {
    return findClassFromSignature(fb->type, fb->class);
}

Object *getMethodParameterTypes(MethodBlock *mb) {
    char *signature, *sig;
    Object *classes;

    signature = sig = sysMalloc(strlen(mb->type) + 1);
    strcpy(sig, mb->type);

    classes = convertSig2ClassArray(&sig, mb->class);
    sysFree(signature);

    return classes;
}

Class *getMethodReturnType(MethodBlock *mb) {
    char *ret = strchr(mb->type, ')') + 1;
    return findClassFromSignature(ret, mb->class);
}

Object *getMethodExceptionTypes(MethodBlock *mb) {
    int i;
    Object *array;
    Class **excps;

    if((array = allocArray(class_array_class, mb->throw_table_size,
                           sizeof(Class*))) == NULL)
        return NULL;

    excps = ARRAY_DATA(array, Class*);

    for(i = 0; i < mb->throw_table_size; i++)
        if((excps[i] = resolveClass(mb->class, mb->throw_table[i],
                                    TRUE, FALSE)) == NULL)
            return NULL;

    return array;
}

Object *getClassConstructors(Class *class, int public) {
    ClassBlock *cb = CLASS_CB(class);
    Object *array, **cons;
    int count = 0;
    int i, j;

    if(!inited && !initReflection())
        return NULL;

    for(i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];
        if((mb->name == SYMBOL(object_init)) &&
                      (!public || (mb->access_flags & ACC_PUBLIC)))
            count++;
    }

    if((array = allocArray(cons_array_class, count, sizeof(Object*))) == NULL)
        return NULL;

    cons = ARRAY_DATA(array, Object*);

    for(i = 0, j = 0; j < count; i++) {
        MethodBlock *mb = &cb->methods[i];

        if((mb->name == SYMBOL(object_init)) &&
                     (!public || (mb->access_flags & ACC_PUBLIC)))

            if((cons[j++] = classlibCreateConstructorObject(mb)) == NULL)
                return NULL;
    }

    return array;
}

Object *getClassMethods(Class *class, int public) {
    ClassBlock *cb = CLASS_CB(class);
    Object *array, **methods;
    int count = 0;
    int i, j;

    if(!inited && !initReflection())
        return NULL;

    for(i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];
        if((mb->name[0] != '<') && (!public || (mb->access_flags & ACC_PUBLIC))
                                && ((mb->access_flags & ACC_MIRANDA) == 0))
            count++;
    }

    if((array = allocArray(method_array_class, count, sizeof(Object*))) == NULL)
        return NULL;

    methods = ARRAY_DATA(array, Object*);

    for(i = 0, j = 0; j < count; i++) {
        MethodBlock *mb = &cb->methods[i];

        if((mb->name[0] != '<') && (!public || (mb->access_flags & ACC_PUBLIC))
                                && ((mb->access_flags & ACC_MIRANDA) == 0))

            if((methods[j++] = classlibCreateMethodObject(mb)) == NULL)
                return NULL;
    }

    return array;
}

Object *getClassFields(Class *class, int public) {
    ClassBlock *cb = CLASS_CB(class);
    Object *array, **fields;
    int count = 0;
    int i, j;

    if(!inited && !initReflection())
        return NULL;

    if(!public)
        count = cb->fields_count;
    else
        for(i = 0; i < cb->fields_count; i++)
            if(cb->fields[i].access_flags & ACC_PUBLIC)
                count++;

    if((array = allocArray(field_array_class, count, sizeof(Object*))) == NULL)
        return NULL;

    fields = ARRAY_DATA(array, Object*);

    for(i = 0, j = 0; j < count; i++) {
        FieldBlock *fb = &cb->fields[i];

        if(!public || (fb->access_flags & ACC_PUBLIC))
            if((fields[j++] = classlibCreateFieldObject(fb)) == NULL)
                return NULL;
    }

    return array;
}

Object *getClassInterfaces(Class *class) {
    ClassBlock *cb = CLASS_CB(class);
    Object *array;

    if(!inited && !initReflection())
        return NULL;

    if((array = allocArray(class_array_class, cb->interfaces_count,
                           sizeof(Class*))) == NULL)
        return NULL;

    memcpy(ARRAY_DATA(array, Class*), cb->interfaces,
           cb->interfaces_count * sizeof(Class*));

    return array;
}

Object *getClassClasses(Class *class, int public) {
    ClassBlock *cb = CLASS_CB(class);
    int i, j, count = 0;
    Class **classes;
    Object *array;

    if(!inited && !initReflection())
        return NULL;

    for(i = 0; i < cb->inner_class_count; i++) {
        Class *iclass;

        if((iclass = resolveClass(class, cb->inner_classes[i],
                                  TRUE, FALSE)) == NULL)
            return NULL;

        if(!public || (CLASS_CB(iclass)->inner_access_flags & ACC_PUBLIC))
            count++;
    }

    if((array = allocArray(class_array_class, count, sizeof(Class*))) == NULL)
        return NULL;

    classes = ARRAY_DATA(array, Class*);

    for(i = 0, j = 0; j < count; i++) {
        Class *iclass = resolveClass(class, cb->inner_classes[i], TRUE, FALSE);

        if(!public || (CLASS_CB(iclass)->inner_access_flags & ACC_PUBLIC))
            classes[j++] = iclass;
    }

    return array;
}

Class *getDeclaringClass(Class *class) {
    ClassBlock *cb = CLASS_CB(class);

    return cb->declaring_class ? resolveClass(class, cb->declaring_class,
                                              TRUE, FALSE)
                               : NULL;
}

int getWrapperPrimTypeIndex(Object *arg) {
    if(arg != NULL) {
        ClassBlock *cb = CLASS_CB(arg->class);

        if(cb->name == SYMBOL(java_lang_Boolean))
            return PRIM_IDX_BOOLEAN;

        if(cb->name == SYMBOL(java_lang_Character))
            return PRIM_IDX_CHAR;

        if(cb->name == SYMBOL(java_lang_Byte))
            return PRIM_IDX_BYTE;

        if(cb->name == SYMBOL(java_lang_Short))
            return PRIM_IDX_SHORT;

        if(cb->name == SYMBOL(java_lang_Integer))
            return PRIM_IDX_INT;

        if(cb->name == SYMBOL(java_lang_Float))
            return PRIM_IDX_FLOAT;

        if(cb->name == SYMBOL(java_lang_Long))
            return PRIM_IDX_LONG;

        if(cb->name == SYMBOL(java_lang_Double))
            return PRIM_IDX_DOUBLE;
    }

    return PRIM_IDX_VOID;
}

Object *createWrapperObject(int prim_type_no, void *pntr, int flags) {
    static char *wrapper_names[] = {"java/lang/Boolean",
                                    "java/lang/Byte",
                                    "java/lang/Character",
                                    "java/lang/Short",
                                    "java/lang/Integer",
                                    "java/lang/Float",
                                    "java/lang/Long",
                                    "java/lang/Double"};
    Object *wrapper = NULL;

    if(prim_type_no > PRIM_IDX_VOID) {
        Class *wrapper_class;

        if((wrapper_class = findSystemClass(wrapper_names[prim_type_no - 1]))
                  && (wrapper = allocObject(wrapper_class)) != NULL) {
            if(prim_type_no > PRIM_IDX_FLOAT)
                INST_BASE(wrapper, u8)[0] = *(u8*)pntr;
            else
                if(flags == REF_SRC_FIELD)
                    INST_BASE(wrapper, u4)[0] = *(u4*)pntr;
                else
                    INST_BASE(wrapper, u4)[0] = *(uintptr_t*)pntr;
        }
    }

    return wrapper;
}

Object *getReflectReturnObject(Class *type, void *pntr, int flags) {
    ClassBlock *type_cb = CLASS_CB(type);

    if(IS_PRIMITIVE(type_cb))
        return createWrapperObject(type_cb->state - CLASS_PRIM, pntr, flags);

    return *(Object**)pntr;
}

int widenPrimitiveValue(int src_idx, int dest_idx, void *src, void *dest,
                        int flags) {

#define err 0
#define U4  1
#define U8  2
#define I2F 3
#define I2D 4
#define I2J 5
#define J2F 6
#define J2D 7
#define F2D 8

    static char conv_table[9][8] = {
        /*  bool byte char shrt int  flt long  dbl             */
           {err, err, err, err, err, err, err, err},  /* !prim */
           {U4,  err, err, err, err, err, err, err},  /* bool  */
           {err, U4,  err, U4,  U4,  I2F, I2J, I2D},  /* byte  */
           {err, err, U4,  err, U4,  I2F, I2J, I2D},  /* char  */
           {err, err, err, U4,  U4,  I2F, I2J, I2D},  /* short */
           {err, err, err, err, U4,  I2F, I2J, I2D},  /* int   */
           {err, err, err, err, err, U4,  err, F2D},  /* float */
           {err, err, err, err, err, J2F, U8,  J2D},  /* long  */
           {err, err, err, err, err, err, err, U8 }   /* dbl   */
    };

    static void *handlers[3][9] = {
         /* field -> field */
         {&&illegal_arg, &&u4_f2f, &&u8, &&i2f_f2f, &&i2d_sf,
                         &&i2j_sf, &&j2f_df, &&j2d, &&f2d_sf},
         /* ostack -> field */
         {&&illegal_arg, &&u4_o2f, &&u8, &&i2f_o2f, &&i2d_so,
                         &&i2j_so, &&j2f_df, &&j2d, &&f2d_so},
         /* field -> ostack */
         {&&illegal_arg, &&u4_f2o, &&u8, &&i2f_f2o, &&i2d_sf,
                         &&i2j_sf, &&j2f_do, &&j2d, &&f2d_sf}
    };

    int handler = conv_table[src_idx][dest_idx - 1];
    goto *handlers[flags][handler];

u4_o2f: /* ostack -> field */
    *(u4*)dest = *(uintptr_t*)src;
    return 1;
u4_f2o: /* field -> ostack */
    *(uintptr_t*)dest = *(u4*)src;
    return 1;
u4_f2f: /* field -> field */
    *(u4*)dest = *(u4*)src;
    return 1;
u8:
    *(u8*)dest = *(u8*)src;
    return 2;
i2f_o2f: /* ostack -> field */
    *(float*)dest = (float)(int)*(uintptr_t*)src;
    return 1;
i2f_f2o: /* field -> ostack */
    *((float*)dest + IS_BE64) = (float)*(int*)src;
    return 1;
i2f_f2f: /* field -> field */
    *(float*)dest = (float)*(int*)src;
    return 1;
i2d_so: /* src ostack */
    *(double*)dest = (double)(int)*(uintptr_t*)src;
    return 2;
i2d_sf: /* src field */
    *(double*)dest = (double)*(int*)src;
    return 2;
i2j_so: /* src ostack */
    *(long long*)dest = (long long)(int)*(uintptr_t*)src;
    return 2;
i2j_sf: /* src field */
    *(long long*)dest = (long long)*(int*)src;
    return 2;
j2f_do: /* dst ostack */
    *((float*)dest + IS_BE64) = (float)*(long long*)src;
    return 1;
j2f_df: /* dst field */
    *(float*)dest = (float)*(long long*)src;
    return 1;
j2d:
    *(double*)dest = (double)*(long long*)src;
    return 2;
f2d_so: /* src ostack */
    *(double*)dest = (double)*((float*)src + IS_BE64);
    return 2;
f2d_sf: /* src field */
    *(double*)dest = (double)*(float*)src;
    return 2;

illegal_arg:
    return 0;
}

int unwrapAndWidenObject(Class *type, Object *arg, void *pntr, int flags) {
    ClassBlock *type_cb = CLASS_CB(type);

    if(IS_PRIMITIVE(type_cb)) {
        int formal_idx = getPrimTypeIndex(type_cb);
        int actual_idx = getWrapperPrimTypeIndex(arg);
        void *data = INST_BASE(arg, void);

        return widenPrimitiveValue(actual_idx, formal_idx, data, pntr,
                                   flags | REF_SRC_FIELD);
    }

    if((arg == NULL) || isInstanceOf(type, arg->class)) {
        *(uintptr_t*)pntr = (uintptr_t)arg;
        return 1;
    }

    return 0;
}

Object *invoke(Object *ob, MethodBlock *mb, Object *arg_array,
                Object *param_types) {

    Object **args = ARRAY_DATA(arg_array, Object*);
    Class **types = ARRAY_DATA(param_types, Class*);

    int types_len = ARRAY_LEN(param_types);
    int args_len = arg_array ? ARRAY_LEN(arg_array) : 0;

    ExecEnv *ee = getExecEnv();
    uintptr_t *sp;
    Object *excep;
    void *ret;
    int i;

    if(args_len != types_len) {
        signalException(java_lang_IllegalArgumentException,
                        "wrong number of args");
        return NULL;
    }

    CREATE_TOP_FRAME(ee, mb->class, mb, sp, ret);

    if(ob) *sp++ = (uintptr_t)ob;

    for(i = 0; i < args_len; i++) {
        int size = unwrapAndWidenObject(*types++, *args++, sp, REF_DST_OSTACK);

        if(size == 0) {
            POP_TOP_FRAME(ee);
            signalException(java_lang_IllegalArgumentException,
                            "arg type mismatch");
            return NULL;
        }

        sp += size;
    }

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectLock(ob ? ob : mb->class);

    if(mb->access_flags & ACC_NATIVE)
        (*mb->native_invoker)(mb->class, mb, ret);
    else
        executeJava();

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectUnlock(ob ? ob : mb->class);

    POP_TOP_FRAME(ee);

    if((excep = exceptionOccurred())) {
        Object *ite_excep;
        MethodBlock *init;
        Class *ite_class;

        clearException();        
        ite_class = findSystemClass("java/lang/reflect/InvocationTargetException");

        if(!exceptionOccurred() && (ite_excep = allocObject(ite_class)) &&
                      (init = lookupMethod(ite_class, SYMBOL(object_init),
                                           SYMBOL(_java_lang_Throwable__V)))) {
            executeMethod(ite_excep, init, excep);
            setException(ite_excep);
        }
        return NULL;
    }

    return ret;
}

int checkInvokeAccess(MethodBlock *mb, int depth) {
    Class *caller = getCallerClass(depth);

    if(!checkClassAccess(mb->class, caller) ||
                            !checkMethodAccess(mb, caller)) {

        signalException(java_lang_IllegalAccessException,
                        "method is not accessible");
            return FALSE;
    }

    return TRUE;
}

Object *constructorConstruct(MethodBlock *mb, Object *args_array,
                             Object *param_types, int no_access_check,
                             int depth) {

    ClassBlock *cb = CLASS_CB(mb->class); 
    Object *ob;

    /* First check that the constructor can be accessed (this
       error takes priority in the reference implementation,
       and Mauve expects this to be thrown before instantiation
       error) */

    if(!no_access_check && !checkInvokeAccess(mb, depth))
        return NULL;

    if(cb->access_flags & ACC_ABSTRACT) {
        signalException(java_lang_InstantiationException, cb->name);
        return NULL;
    }

    /* Creating an instance of the class is an
       active use; make sure it is initialised */
    if(initClass(mb->class) == NULL)
        return NULL;

    if((ob = allocObject(mb->class)) != NULL)
        invoke(ob, mb, args_array, param_types);

    return ob;
}

int checkObject(Object *ob, Class *type) {
    if(ob == NULL) {
        signalException(java_lang_NullPointerException, NULL);
        return FALSE;
    }

    if(!isInstanceOf(type, ob->class)) {
        signalException(java_lang_IllegalArgumentException,
                        "object is not an instance of declaring class");
        return FALSE;
    }

    return TRUE;
}

Object *methodInvoke(Object *ob, MethodBlock *mb, Object *args_array,
                     Class *ret_type, Object *param_types,
                     int no_access_check, int depth) {

    uintptr_t *ret;

    /* First check that the method can be accessed (this
       error takes priority in the reference implementation) */

    if(!no_access_check && !checkInvokeAccess(mb, depth))
        return NULL;

    /* If it's a static method, class may not be initialised;
       interfaces are also not normally initialised. */

    if((mb->access_flags & ACC_STATIC) || IS_INTERFACE(CLASS_CB(mb->class)))
        if(initClass(mb->class) == NULL)
            return NULL;

    if(mb->access_flags & ACC_STATIC)
        ob = NULL;
    else
        if(!checkObject(ob, mb->class) ||
                ((mb = lookupVirtualMethod(ob, mb)) == NULL))
            return NULL;
 
    if((ret = (uintptr_t*) invoke(ob, mb, args_array, param_types)) == NULL)
        return NULL;

    return getReflectReturnObject(ret_type, ret, REF_SRC_OSTACK);
}

/* Reflection access from JNI */

Object *createReflectConstructorObject(MethodBlock *mb) {
    if(!inited && !initReflection())
        return NULL;

    return classlibCreateConstructorObject(mb);
}

Object *createReflectMethodObject(MethodBlock *mb) {
    if(!inited && !initReflection())
        return NULL;

    return classlibCreateMethodObject(mb);
}

Object *createReflectFieldObject(FieldBlock *fb) {
    if(!inited && !initReflection())
        return NULL;

    return classlibCreateFieldObject(fb);
}

MethodBlock *mbFromReflectObject(Object *reflect_ob) {
    return classlibMbFromReflectObject(reflect_ob);
}

FieldBlock *fbFromReflectObject(Object *reflect_ob) {
    return classlibFbFromReflectObject(reflect_ob);
}
