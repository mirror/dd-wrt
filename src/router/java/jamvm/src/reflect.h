/*
 * Copyright (C) 2009, 2010, 2011, 2012, 2014
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

#define REF_SRC_FIELD  0
#define REF_DST_FIELD  0
#define REF_SRC_OSTACK 1
#define REF_DST_OSTACK 2

#define getPrimTypeIndex(cb) (cb->state - CLASS_PRIM)

extern Object *getClassConstructors(Class *class, int public);
extern Object *getClassMethods(Class *class, int public);
extern Object *getClassFields(Class *class, int public);
extern Object *getClassInterfaces(Class *class);
extern Object *getClassClasses(Class *class, int public);
extern Class *getDeclaringClass(Class *class);
extern Class *getEnclosingClass(Class *class);
extern Object *getEnclosingMethodObject(Class *class);
extern Object *getEnclosingConstructorObject(Class *class);
extern Object *getMethodExceptionTypes(MethodBlock *mb);
extern Object *getMethodParameterTypes(MethodBlock *mb);
extern Class *getMethodReturnType(MethodBlock *mb);
extern Class *getFieldType(FieldBlock *fb);

extern Class *findClassFromSignature(char *type_name, Class *class);

extern Object *getReflectReturnObject(Class *type, void *pntr, int flags);
extern int widenPrimitiveValue(int src_idx, int dest_idx, void *src,
                               void *dest, int flags);
extern int unwrapAndWidenObject(Class *type, Object *arg, void *pntr,
                                int flags);

extern Object *constructorConstruct(MethodBlock *mb, Object *args_array,
                                    Object *param_types, int no_access_check,
                                    int depth);

extern Object *methodInvoke(Object *ob, MethodBlock *mb, Object *args_array,
                            Class *ret_type, Object *param_types,
                            int no_access_check, int depth);

extern MethodBlock *mbFromReflectObject(Object *reflect_ob);
extern FieldBlock *fbFromReflectObject(Object *reflect_ob);

extern Object *createReflectConstructorObject(MethodBlock *mb);
extern Object *createReflectMethodObject(MethodBlock *mb);
extern Object *createReflectFieldObject(FieldBlock *fb);
extern int checkObject(Object *ob, Class *type);
extern int getWrapperPrimTypeIndex(Object *arg);
extern int numElementsInSig(char *sig);

extern Class *convertSigElement2Class(char **sig_pntr, Class *declaring_class);
extern Object *convertSig2ClassArray(char **sig_pntr, Class *declaring_class);

extern Object *createWrapperObject(int prim_type_no, void *pntr, int flags);
