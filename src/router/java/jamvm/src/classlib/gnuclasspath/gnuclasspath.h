/*
 * Copyright (C) 2011 Robert Lougher <rob@jamvm.org.uk>.
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

#define IS_VMTHREAD(cb)    (cb->flags & VMTHREAD)
#define IS_VMTHROWABLE(cb) (cb->flags & VMTHROWABLE)

extern Object *getVMConsParamTypes(Object *vm_cons_obj);
extern Object *getVMMethodParamTypes(Object *vm_method_obj);
extern Class *getVMMethodReturnType(Object *vm_method_obj);
extern Class *getVMFieldType(Object *vm_field_obj);

extern MethodBlock *getVMConsMethodBlock(Object *cons_ref_obj);
extern int getVMConsAccessFlag(Object *cons_ref_obj);
extern MethodBlock *getVMMethodMethodBlock(Object *mthd_ref_obj);
extern int getVMMethodAccessFlag(Object *mthd_ref_obj);
extern FieldBlock *getVMFieldFieldBlock(Object *fld_ref_obj);
extern int getVMFieldAccessFlag(Object *fld_ref_obj);

extern Object *getClassAnnotations(Class *class);
extern Object *getFieldAnnotations(FieldBlock *fb);
extern Object *getMethodAnnotations(MethodBlock *mb);
extern Object *getMethodParameterAnnotations(MethodBlock *mb);
extern Object *getMethodDefaultValue(MethodBlock *mb);

extern Class *getReflectMethodClass();

extern Thread *vmThread2Thread(Object *vmThread);
extern void markVMThrowable(Object *vmthrwble, int mark);
