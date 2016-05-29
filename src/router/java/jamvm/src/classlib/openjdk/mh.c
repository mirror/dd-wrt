/*
 * Copyright (C) 2012, 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
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

/* Must be included first to get configure options */
#include "jam.h"

#ifdef JSR292
#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "lock.h"
#include "class.h"
#include "excep.h"
#include "frame.h"
#include "symbol.h"
#include "reflect.h"
#include "openjdk.h"
#include "classlib.h"

static int mem_name_clazz_offset, mem_name_name_offset,
           mem_name_type_offset, mem_name_flags_offset,
           mem_name_vmtarget_offset;
       
static int mthd_type_ptypes_offset, mthd_type_rtype_offset;
static int mthd_hndl_form_offset;
static int lmda_form_vmentry_offset;
static int call_site_target_offset;

static MethodBlock *MHN_linkMethod_mb;
static MethodBlock *MHN_linkCallSite_mb;
static MethodBlock *MHN_findMethodType_mb;
static MethodBlock *MHN_linkMethodHandleConstant_mb;

static Class *method_handle_class;

/* Defined in reflect.c */
extern Class *cons_reflect_class, *method_reflect_class;
extern Class *field_reflect_class;

extern int cons_slot_offset, cons_class_offset;
extern int mthd_slot_offset, mthd_class_offset;
extern int fld_slot_offset, fld_class_offset;

#define CACHE_SIZE 1<<7
static HashTable intrinsic_cache;

VMLock resolve_lock;

void initialiseMethodHandles() {
    Class *member_name;
    FieldBlock *clazz_fb, *name_fb, *type_fb, *flags_fb;
    FieldBlock *vmtarget_fb;
    Class *method_type;
    FieldBlock *ptypes_fb, *rtype_fb;
    Class *method_handle;
    FieldBlock *form_fb;
    Class *lambda_form;
    FieldBlock *vmentry_fb;
    Class *mthd_hndl_natives;
    Class *call_site;
    FieldBlock *target_fb;
    
    member_name = findSystemClass0(SYMBOL(java_lang_invoke_MemberName));

    if(member_name == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: can't find "
                            "java.lang.invoke.MemberName\n");
        exitVM(1);
    }

    clazz_fb = findField(member_name, SYMBOL(clazz),
                                      SYMBOL(sig_java_lang_Class));

    name_fb = findField(member_name, SYMBOL(name),
                                     SYMBOL(sig_java_lang_String));

    type_fb = findField(member_name, SYMBOL(type),
                                     SYMBOL(sig_java_lang_Object));

    flags_fb = findField(member_name, SYMBOL(flags), SYMBOL(I));

    vmtarget_fb = findField(member_name, SYMBOL(vmtarget),
                                         sizeof(void*) == 4 ? SYMBOL(I)
                                                            : SYMBOL(J));

    if(clazz_fb == NULL || name_fb == NULL || type_fb == NULL
                        || vmtarget_fb == NULL || flags_fb == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: Expected fields missing"
                            " in java.lang.invoke.MemberName\n");
        exitVM(1);
    }

    mem_name_clazz_offset = clazz_fb->u.offset;
    mem_name_name_offset = name_fb->u.offset;
    mem_name_type_offset = type_fb->u.offset;
    mem_name_flags_offset = flags_fb->u.offset;
    mem_name_vmtarget_offset = vmtarget_fb->u.offset;
    
    method_type = findSystemClass0(SYMBOL(java_lang_invoke_MethodType));

    if(method_type == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: can't find "
                            "java.lang.invoke.MethodType\n");
        exitVM(1);
    }

    ptypes_fb = findField(method_type, SYMBOL(ptypes),
                                       SYMBOL(array_java_lang_Class));

    rtype_fb = findField(method_type, SYMBOL(rtype),
                                      SYMBOL(sig_java_lang_Class));

    if(ptypes_fb == NULL || rtype_fb == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: Expected fields missing"
                            " in java.lang.invoke.MethodType\n");
        exitVM(1);
    }

    mthd_type_ptypes_offset = ptypes_fb->u.offset;
    mthd_type_rtype_offset = rtype_fb->u.offset;

    method_handle = findSystemClass0(SYMBOL(java_lang_invoke_MethodHandle));

    if(method_handle == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: can't find "
                            "java.lang.invoke.MethodHandle\n");
        exitVM(1);
    }

    form_fb = findField(method_handle, SYMBOL(form),
                                       SYMBOL(sig_java_lang_invoke_LambdaForm));

    if(form_fb == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: Expected fields missing"
                            " in java.lang.invoke.MethodHandle\n");
        exitVM(1);
    }

    mthd_hndl_form_offset = form_fb->u.offset;

    registerStaticClassRefLocked(&method_handle_class, method_handle);

    lambda_form = findSystemClass0(SYMBOL(java_lang_invoke_LambdaForm));

    if(lambda_form == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: can't find "
                            "java.lang.invoke.LambdaForm\n");
        exitVM(1);
    }

    vmentry_fb = findField(lambda_form, SYMBOL(vmentry),
                           SYMBOL(sig_java_lang_invoke_MemberName));

    if(vmentry_fb == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: Expected fields missing"
                            " in java.lang.invoke.LambdaForm\n");
        exitVM(1);
    }

    lmda_form_vmentry_offset = vmentry_fb->u.offset;

    mthd_hndl_natives = findSystemClass0(
                            SYMBOL(java_lang_invoke_MethodHandleNatives));

    if(mthd_hndl_natives == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: can't find "
                            "java.lang.invoke.MethodHandleNatives\n");
        exitVM(1);
    }

    MHN_linkMethod_mb =
         findMethod(mthd_hndl_natives, SYMBOL(linkMethod),
                    SYMBOL(java_lang_invoke_MHN_linkMethod_sig));

    MHN_findMethodType_mb =
         findMethod(mthd_hndl_natives, SYMBOL(findMethodHandleType),
                    SYMBOL(java_lang_invoke_MHN_findMethodType_sig));

    MHN_linkCallSite_mb =
         findMethod(mthd_hndl_natives, SYMBOL(linkCallSite),
                    SYMBOL(java_lang_invoke_MHN_linkCallSite_sig));

    MHN_linkMethodHandleConstant_mb =
         findMethod(mthd_hndl_natives, SYMBOL(linkMethodHandleConstant),
                    SYMBOL(java_lang_invoke_MHN_linkMethodHandleConstant_sig));

    if(MHN_linkMethod_mb == NULL || MHN_linkMethodHandleConstant_mb == NULL ||
       MHN_linkCallSite_mb == NULL || MHN_findMethodType_mb == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: Expected method missing"
                            " in java.lang.invoke.MethodHandleNatives\n");
        exitVM(1);
    }

    call_site = findSystemClass0(SYMBOL(java_lang_invoke_CallSite));

    if(call_site == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: can't find "
                            "java.lang.invoke.CallSite\n");
        exitVM(1);
    }

    target_fb = findField(call_site, SYMBOL(target),
                           SYMBOL(sig_java_lang_invoke_MethodHandle));

    if(target_fb == NULL) {
        jam_fprintf(stderr, "initialiseMethodHandles: Expected fields missing"
                            " in java.lang.invoke.CallSite\n");
        exitVM(1);
    }

    call_site_target_offset = target_fb->u.offset;

    /* Init hash table and create lock */
    initHashTable(intrinsic_cache, CACHE_SIZE, TRUE);
}

void cachePolyOffsets(CachedPolyOffsets *cpo) {
    cpo->mthd_hndl_form = mthd_hndl_form_offset;
    cpo->mem_name_vmtarget = mem_name_vmtarget_offset;
    cpo->lmda_form_vmentry = lmda_form_vmentry_offset;
}

/* JVM_CONSTANT_MethodHandle subtypes */
#define REF_getField                1
#define REF_getStatic               2
#define REF_putField                3
#define REF_putStatic               4
#define REF_invokeVirtual           5
#define REF_invokeStatic            6
#define REF_invokeSpecial           7
#define REF_newInvokeSpecial        8
#define REF_invokeInterface         9

#define IS_METHOD        0x010000
#define IS_CONSTRUCTOR   0x020000
#define IS_FIELD         0x040000
#define IS_TYPE          0x080000
#define CALLER_SENSITIVE 0x100000

#define SEARCH_SUPERCLASSES 0x100000
#define SEARCH_INTERFACES   0x200000

#define ALL_KINDS (IS_METHOD | IS_CONSTRUCTOR | IS_FIELD | IS_TYPE)
                
#define REFERENCE_KIND_SHIFT 24
#define REFERENCE_KIND_MASK  (0xf000000 >> REFERENCE_KIND_SHIFT)

static int stackOverflowCheck(ExecEnv *ee, char *sp) {
    if(sp > ee->stack_end) {
        if(ee->overflow++) {
            /* Overflow when we're already throwing stack
               overflow.  Stack extension should be enough
               to throw exception, so something's seriously
               gone wrong - abort the VM! */
            jam_fprintf(stderr, "Fatal stack overflow!  Aborting VM.\n");
            exitVM(1);
        }
        ee->stack_end += STACK_RED_ZONE_SIZE;
        signalException(java_lang_StackOverflowError, NULL);
        return TRUE;
    }

    return FALSE;
}

static void executePolyMethod(Object *ob, MethodBlock *mb, uintptr_t *lvars) {
    if(mb->access_flags & ACC_NATIVE)
        (*mb->native_invoker)(mb->class, mb, lvars);
    else {
        ExecEnv *ee = getExecEnv();
        Frame *last = ee->last_frame->prev;
        Frame *dummy = (Frame*)(lvars + mb->max_locals);
        Frame *new_frame = dummy + 1;
        uintptr_t *new_ostack = ALIGN_OSTACK(new_frame + 1);

        if(stackOverflowCheck(ee, (char *)(new_ostack + mb->max_stack)))
            return;

        dummy->prev = last;
        dummy->mb = NULL;
        dummy->ostack = (uintptr_t *)new_frame;

        new_frame->mb = mb;
        new_frame->lvars = lvars;
        new_frame->ostack = new_ostack;
        new_frame->prev = dummy;

        ee->last_frame = new_frame;

        if(mb->access_flags & ACC_SYNCHRONIZED)
            objectLock(ob ? ob : mb->class);

        executeJava();

        if(mb->access_flags & ACC_SYNCHRONIZED)
            objectUnlock(ob ? ob : mb->class);
    }
}

static int sigRetSlotSize(char *sig) {
    int len = strlen(sig);

    if(sig[len-2] != ')')
        return 1;

    switch(sig[len-1]) {
        case 'V':
            return 0;
        case 'J':
        case 'D':
            return 2;
        default:
            return 1;
    }
}

static uintptr_t *invokeBasic(Class *class, MethodBlock *mb,
                              uintptr_t *ostack) {

    Object *method_handle = (Object*)ostack[0];
    Object *form = INST_DATA(method_handle, Object*, mthd_hndl_form_offset);
    Object *vmentry = INST_DATA(form, Object*, lmda_form_vmentry_offset);
    MethodBlock *vmtarget = INST_DATA(vmentry, MethodBlock*, 
    	                              mem_name_vmtarget_offset);

    executePolyMethod(NULL, vmtarget, ostack);

    ostack += mb->ret_slot_size;
    return ostack;
}

static uintptr_t *linkToSpecial(Class *class, MethodBlock *mb,
                                uintptr_t *ostack) {

    Object *mem_name = (Object*)ostack[mb->args_count-1];
    MethodBlock *vmtarget = INST_DATA(mem_name, MethodBlock*,
    	                              mem_name_vmtarget_offset);

    executePolyMethod(NULL, vmtarget, ostack);

    ostack += mb->ret_slot_size;
    return ostack;
}

static uintptr_t *linkToVirtual(Class *class, MethodBlock *mb,
                                uintptr_t *ostack) {

    Object *this = (Object*)ostack[0];
    Object *mem_name = (Object*)ostack[mb->args_count-1];
    MethodBlock *vmtarget = INST_DATA(mem_name, MethodBlock*,
    	                              mem_name_vmtarget_offset);

    vmtarget = lookupVirtualMethod(this, vmtarget);
    if(vmtarget != NULL)
        executePolyMethod(this, vmtarget, ostack);

    ostack += mb->ret_slot_size;
    return ostack;
}

static int mbFlags(MethodBlock *mb) {
    int flags = mb->access_flags;

    if(mb->flags & MB_CALLER_SENSITIVE)
        flags |= CALLER_SENSITIVE;

    return flags;
}

void initMemberName(Object *mname, Object *target) {

    if(target->class == method_reflect_class) {
        int slot = INST_DATA(target, int, mthd_slot_offset);
        Class *decl_class = INST_DATA(target, Class*, mthd_class_offset);

        ClassBlock *cb = CLASS_CB(decl_class);
        MethodBlock *mb = &(cb->methods[slot]);
        int flags = mbFlags(mb) | IS_METHOD;
        int ref_kind;
        
        if(mb->access_flags & ACC_STATIC)
            ref_kind = REF_invokeStatic;
        else if(IS_INTERFACE(cb))
            ref_kind = REF_invokeInterface;
        else
            ref_kind = REF_invokeVirtual;

        flags |= ref_kind << REFERENCE_KIND_SHIFT;

        INST_DATA(mname, Class*, mem_name_clazz_offset) = decl_class;
        INST_DATA(mname, int, mem_name_flags_offset) = flags;
        INST_DATA(mname, MethodBlock*, mem_name_vmtarget_offset) = mb;

   } else if(target->class == cons_reflect_class) {
        int slot = INST_DATA(target, int, cons_slot_offset);
        Class *decl_class = INST_DATA(target, Class*, cons_class_offset);
        MethodBlock *mb = &(CLASS_CB(decl_class)->methods[slot]);
        int flags = mbFlags(mb) | IS_CONSTRUCTOR |
                    (REF_invokeSpecial << REFERENCE_KIND_SHIFT);

        INST_DATA(mname, Class*, mem_name_clazz_offset) = decl_class;
        INST_DATA(mname, int, mem_name_flags_offset) = flags;
        INST_DATA(mname, MethodBlock*, mem_name_vmtarget_offset) = mb;

   } else if(target->class == field_reflect_class) {
        Class *decl_class = INST_DATA(target, Class*, fld_class_offset);
        int slot = INST_DATA(target, int, fld_slot_offset);
        FieldBlock *fb = &(CLASS_CB(decl_class)->fields[slot]);
        int flags = fb->access_flags | IS_FIELD;

        flags |= (fb->access_flags & ACC_STATIC ? REF_getStatic
                                                : REF_getField)
                  << REFERENCE_KIND_SHIFT;

        INST_DATA(mname, Class*, mem_name_clazz_offset) = decl_class;
        INST_DATA(mname, int, mem_name_flags_offset) = flags;
        INST_DATA(mname, FieldBlock*, mem_name_vmtarget_offset) = fb;
   } else
        signalException(java_lang_InternalError,
                        "initMemberName: unimplemented target");
}

void expandMemberName(Object *mname) {
    void *vmtarget = INST_DATA(mname, void*, mem_name_vmtarget_offset);

    if(vmtarget == NULL)
        signalException(java_lang_IllegalArgumentException, "vmtarget");
    else {
        Object *name = INST_DATA(mname, Object*, mem_name_name_offset);
        Object *type = INST_DATA(mname, Object*, mem_name_type_offset);
        int flags = INST_DATA(mname, int, mem_name_flags_offset);

        switch(flags & ALL_KINDS) {
            case IS_METHOD:
            case IS_CONSTRUCTOR: {
                MethodBlock *mb = vmtarget;

                if(name == NULL)
                    INST_DATA(mname, Object*, mem_name_name_offset) =
                                     findInternedString(createString(mb->name));
                if(type == NULL)
                    INST_DATA(mname, Object*, mem_name_type_offset) =
                                     createString(mb->type);
                break;
            }

            case IS_FIELD: {
                FieldBlock *fb = vmtarget;

                if(name == NULL)
                    INST_DATA(mname, Object*, mem_name_name_offset) =
                                     findInternedString(createString(fb->name));
                if(type == NULL)
                    INST_DATA(mname, Object*, mem_name_type_offset) =
                                     getFieldType(fb);
                break;
            }

            default:
                signalException(java_lang_InternalError, "flags kind");
        }
    }
}

long long memNameFieldOffset(Object *mname) {
    FieldBlock *fb = INST_DATA(mname, FieldBlock*, mem_name_vmtarget_offset);
    return fb->u.offset;
}

long long memNameStaticFieldOffset(Object *mname) {
    FieldBlock *fb = INST_DATA(mname, FieldBlock*, mem_name_vmtarget_offset);
    return (uintptr_t)&fb->u.static_value;
}

void setCallSiteTargetNormal(Object *call_site, Object *target) {
    INST_DATA(call_site, Object*, call_site_target_offset) = target;
}

void setCallSiteTargetVolatile(Object *call_site, Object *target) {
    INST_DATA(call_site, Object*, call_site_target_offset) = target;
}

int getMembers(Class *clazz, Object *match_name, Object *match_sig,
               int match_flags, Class *caller, int skip, Object *results) {

    int search_super = (match_flags & SEARCH_SUPERCLASSES) != 0;
    int search_intf = (match_flags & SEARCH_INTERFACES) != 0;
    int local = !(search_super || search_intf);
    char *name_sym = NULL, *sig_sym = NULL;

    int rlen = ARRAY_LEN(results);
    Object **rpntr = ARRAY_DATA(results, Object*);

    ClassBlock *cb = CLASS_CB(clazz);

    if(match_name != NULL) {
        char *str = String2Utf8(match_name);
        name_sym = findUtf8(str);
        sysFree(str);
        if(name_sym == NULL)
            goto no_match;
    }

    if(match_sig != NULL) {
        char *str = String2Utf8(match_sig);
        sig_sym = findUtf8(str);
        sysFree(str);
        if(sig_sym == NULL)
            goto no_match;
    }

    if(match_flags & IS_FIELD)
        goto unimplemented;

    if(!local)
        goto unimplemented;

    if(match_flags & (IS_METHOD | IS_CONSTRUCTOR)) {
        int i, j = 0;

        for(i = cb->methods_count-1; i >= 0; i--) {
            MethodBlock *mb = &cb->methods[i];

            if(mb->name == SYMBOL(class_init))
                continue;
            if(mb->name == SYMBOL(object_init))
                continue;
            if(skip-- > 0)
                continue;

            if(j++ < rlen) {
                Object *mname = *rpntr++;
                int flags = mbFlags(mb) | IS_METHOD;

                flags |= (mb->access_flags & ACC_STATIC ? REF_invokeStatic
                                                        : REF_invokeVirtual)
                          << REFERENCE_KIND_SHIFT;

                INST_DATA(mname, int, mem_name_flags_offset) = flags;
                INST_DATA(mname, Class*, mem_name_clazz_offset) = mb->class;
                INST_DATA(mname, Object*, mem_name_name_offset) =
                                 findInternedString(createString(mb->name));
                INST_DATA(mname, Object*, mem_name_type_offset) =
                                 createString(mb->type);
                INST_DATA(mname, MethodBlock*, mem_name_vmtarget_offset) = mb;
            }
        }

        return j;
    }

unimplemented:
    signalException(java_lang_InternalError, "getMembers: unimplemented");

no_match:
    return 0;
}

static int class2Signature(Class *class, char *buff[], int pos,
                           int *buff_len)  {

    ClassBlock *cb = CLASS_CB(class);
    int rem, len, name_len;

    if(IS_PRIMITIVE(cb))
        len = 2;
    else {
        name_len = strlen(cb->name);
        len = name_len + (IS_ARRAY(cb) ? 1 : 3);
    }
            
    rem = *buff_len - pos - len;
    if(rem < 0)
        *buff = sysRealloc(*buff, *buff_len += -rem + 128);

    if(IS_PRIMITIVE(cb))
        (*buff)[pos++] = primClass2TypeChar(class);
     else {
        if(!IS_ARRAY(cb))
            (*buff)[pos++] = 'L';

        memcpy(*buff + pos, cb->name, name_len);
        pos += name_len;

        if(!IS_ARRAY(cb))
            (*buff)[pos++] = ';';
    }

    (*buff)[pos] = '\0';
    return pos;
}

static char *type2Signature(Object *type, int add_if_absent) {
    char *sig, *found;

    if(IS_CLASS(type)) {
        int buff_len = 0;
        sig = NULL;
        class2Signature(type, &sig, 0, &buff_len);
    } else {
        char *type_classname = CLASS_CB(type->class)->name;
         
        if(type_classname == SYMBOL(java_lang_String))
            sig = String2Utf8(type);
        else
            if(type_classname == SYMBOL(java_lang_invoke_MethodType)) {
                Object *ptypes_array = INST_DATA(type, Object *,
                                                 mthd_type_ptypes_offset);
                Class *rtype = INST_DATA(type, Class *,
                                         mthd_type_rtype_offset);
                Object **ptypes = ARRAY_DATA(ptypes_array, Object*);
                int num_ptypes = ARRAY_LEN(ptypes_array);
                int i, pos, buff_len = 128;

                sig = sysMalloc(128);
                sig[0] = '(';

                for(i = 0, pos = 1; i < num_ptypes; i++)
                    pos = class2Signature(ptypes[i], &sig, pos, &buff_len);

                sig[pos++] = ')';
                class2Signature(rtype, &sig, pos, &buff_len);
            } else {
                signalException(java_lang_InternalError,
                                "unrecognised type");
                return NULL;
            }
    }

    sig = sysRealloc(sig, strlen(sig) + 1);
    if((found = findHashedUtf8(sig, add_if_absent)) != sig)
        sysFree(sig);

    return found;
}

#define isStaticPolymorphicSig(id) (id >= ID_linkToStatic)

static int polymorphicNameID(Class *clazz, char *name) {
    if(CLASS_CB(clazz)->name == SYMBOL(java_lang_invoke_MethodHandle)) {
        if(name == SYMBOL(invoke) || name == SYMBOL(invokeExact))
            return ID_invokeGeneric;
        else if(name == SYMBOL(invokeBasic))
            return ID_invokeBasic;
        else if(name == SYMBOL(linkToVirtual))
            return ID_linkToVirtual;
        else if(name == SYMBOL(linkToStatic))
            return ID_linkToStatic;
        else if(name == SYMBOL(linkToSpecial))
            return ID_linkToSpecial;
        else if(name == SYMBOL(linkToInterface))
            return ID_linkToInterface;
    }
    return -1;
}

static NativeMethod polymorphicID2Invoker(int id) {
    switch(id) {
        case ID_invokeBasic:
            return &invokeBasic;
        case ID_linkToSpecial:
        case ID_linkToStatic:
            return &linkToSpecial;
        case ID_linkToVirtual:
        case ID_linkToInterface:
            return &linkToVirtual;
    }
    return NULL;
}

static Object *findMethodHandleType(char *type, Class *accessing_class) {
    Object *method_type, *ptypes;
    char *signature, *sig;
    Class *rtype;

    signature = sig = sysMalloc(strlen(type) + 1);
    strcpy(sig, type);

    ptypes = convertSig2ClassArray(&sig, accessing_class);

    sig += 1;
    rtype = convertSigElement2Class(&sig, accessing_class);

    sysFree(signature);
    if(ptypes == NULL || rtype == NULL)
        return NULL;

    /* An invokedynamic bytecode can be executed without MethodHandleNatives
       being initialised, so initialiseMethodHandles will not have been
       called (via registerNatives) */
    if(MHN_findMethodType_mb == NULL)
        findSystemClass(SYMBOL(java_lang_invoke_MethodHandleNatives));

    method_type = *(Object**)executeStaticMethod(MHN_findMethodType_mb->class,
                                                 MHN_findMethodType_mb, rtype,
                                                 ptypes);

    if(exceptionOccurred())
        return NULL;

    return method_type;
}

Object *resolveMethodType(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    Object *mt = NULL;

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_ResolvedMethodType:
            mt = (Object *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_MethodType: {
            char *methodtype;
            int type_idx = CP_METHOD_TYPE(cp, cp_index);

            MBARRIER();
            if(CP_TYPE(cp, cp_index) != CONSTANT_MethodType)
                goto retry;

            methodtype = CP_UTF8(cp, type_idx);
            mt = findMethodHandleType(methodtype, class);
            if(mt == NULL)
                return NULL;

            CP_TYPE(cp, cp_index) = CONSTANT_Locked;
            MBARRIER();
            CP_INFO(cp, cp_index) = (uintptr_t)mt;
            MBARRIER();
            CP_TYPE(cp, cp_index) = CONSTANT_ResolvedMethodType;

            break;
        }
    }

    return mt;
}

static MethodBlock *findMethodHandleInvoker(Class *class,
                                            Class *accessing_class,
                                            char *methodname,
                                            char *type,
                                            Object **appendix) {

    Object *name_str = findInternedString(createString(methodname));
    Class *obj_array_class = findArrayClass("[Ljava/lang/Object;");
    Object *appendix_box;
    Object *member_name;
    Object *method_type;

    if(name_str == NULL || obj_array_class == NULL)
        return NULL;

    appendix_box = allocArray(obj_array_class, 1, sizeof(Object*));
    if(appendix_box == NULL)
        return NULL;

    method_type = findMethodHandleType(type, accessing_class);
    if(method_type == NULL)
        return NULL;

    member_name = *(Object**)executeStaticMethod(MHN_linkMethod_mb->class,
                                                 MHN_linkMethod_mb,
                                                 accessing_class,
                                                 REF_invokeVirtual,
                                                 class,
                                                 name_str,
                                                 method_type,
                                                 appendix_box);

    if(exceptionOccurred())
        return NULL;

    *appendix = ARRAY_DATA(appendix_box, Object*)[0];
    return INST_DATA(member_name, MethodBlock*, mem_name_vmtarget_offset);
}

void resolveLock(Thread *self) {
    disableSuspend(self);
    lockVMLock(resolve_lock, self);
}

void resolveUnlock(Thread *self) {
    unlockVMLock(resolve_lock, self);
    enableSuspend(self);
}

PolyMethodBlock *resolvePolyMethod(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    PolyMethodBlock *pmb = NULL;

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_ResolvedPolyMethod:
            pmb = (PolyMethodBlock *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_Methodref: {
            Thread *self;
            Object *appendix;
            MethodBlock *invoker;
            char *methodname, *methodtype;
            int cl_idx = CP_METHOD_CLASS(cp, cp_index);
            int name_type_idx = CP_METHOD_NAME_TYPE(cp, cp_index);

            MBARRIER();
            if(CP_TYPE(cp, cp_index) != CONSTANT_Methodref)
                goto retry;

            methodname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp, name_type_idx));
            methodtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp, name_type_idx));

            invoker = findMethodHandleInvoker((Class*)CP_INFO(cp, cl_idx),
                                              class, methodname,
                                              methodtype, &appendix);

            if(invoker == NULL)
                return NULL;

            resolveLock(self = threadSelf());
            if(CP_TYPE(cp, cp_index) != CONSTANT_Methodref) {
                resolveUnlock(self);
                goto retry;
            }

            pmb = sysMalloc(sizeof(PolyMethodBlock));

            pmb->name = methodname;
            pmb->type = methodtype;
            pmb->invoker = invoker;
            pmb->appendix = appendix;

            CP_TYPE(cp, cp_index) = CONSTANT_Locked;
            MBARRIER();
            CP_INFO(cp, cp_index) = (uintptr_t)pmb;
            MBARRIER();
            CP_TYPE(cp, cp_index) = CONSTANT_ResolvedPolyMethod;

            resolveUnlock(self);
            break;
        }
    }

    return pmb;
}

static Object *findMethodHandleConstant(Class *class, int ref_kind,
                                        Class *defining_class,
                                        char *name, Object *type) {

    Object *mh;
    Object *name_str = findInternedString(createString(name));

    if(name_str == NULL)
        return NULL;

    mh = *(Object**)executeStaticMethod(MHN_linkMethodHandleConstant_mb->class,
                                        MHN_linkMethodHandleConstant_mb,
                                        class, ref_kind, defining_class,
                                        name_str, type);

    if(exceptionOccurred())
        return NULL;

   return mh;
}

Object *resolveMethodHandle(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    Object *mh = NULL;

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_ResolvedMethodHandle:
            mh = (Object *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_MethodHandle: {
            char *name;
            Object *type_obj;
            Class *resolved_class;
            int ref_idx = CP_METHOD_HANDLE_REF(cp, cp_index);
            int ref_kind = CP_METHOD_HANDLE_KIND(cp, cp_index);

            MBARRIER();
            if(CP_TYPE(cp, cp_index) != CONSTANT_MethodHandle)
                goto retry;

            if(ref_kind >= REF_invokeVirtual) {
                MethodBlock *mb;
                char *type;

                if(ref_kind == REF_invokeInterface)
                    mb = resolveInterfaceMethod(class, ref_idx);
                else
                    mb = resolveMethod(class, ref_idx);

                if(mb == NULL) {
                    if(ref_kind == REF_invokeVirtual &&
                             isPolymorphicRef(class, ref_idx)) {
                        PolyMethodBlock *pmb;

                        clearException();
                        pmb = resolvePolyMethod(class, ref_idx);

                        if(pmb == NULL)
                            return NULL;

                        name = pmb->name;
                        type = pmb->type;
                        resolved_class = method_handle_class;
                    } else
                        return NULL;
                } else {
                    name = mb->name;
                    type = mb->type;
                    resolved_class = mb->class;
                }

                type_obj = findMethodHandleType(type, resolved_class);
            } else {
                FieldBlock *fb = resolveField(class, ref_idx);

                if(fb == NULL)
                    return NULL;

                name = fb->name;
                resolved_class = fb->class;
                type_obj = findClassFromSignature(fb->type, resolved_class);
            }

            if(type_obj == NULL)
                return NULL;

            mh = findMethodHandleConstant(class, ref_kind, resolved_class,
                                          name, type_obj);

            if(mh == NULL)
                return NULL;

            CP_TYPE(cp, cp_index) = CONSTANT_Locked;
            MBARRIER();
            CP_INFO(cp, cp_index) = (uintptr_t)mh;
            MBARRIER();
            CP_TYPE(cp, cp_index) = CONSTANT_ResolvedMethodHandle;

            break;
        }
    }

    return mh;
}

static int cpType2PrimIdx(int type) {
    switch(type) {
        case CONSTANT_Integer:
            return PRIM_IDX_INT;
        case CONSTANT_Float:
            return PRIM_IDX_FLOAT;
        case CONSTANT_Long:
            return PRIM_IDX_LONG;
        case CONSTANT_Double:
            return PRIM_IDX_DOUBLE;
        default:
            return -1;
    }
}

MethodBlock *findInvokeDynamicInvoker(Class *class,
                                      ResolvedInvDynCPEntry *entry,
                                      Object **appendix) {
    Object *exception;
    Object *boot_mthd;
    Object *method_type;
    Object *member_name;
    Object *appendix_box;
    Object *args_array = NULL;
    ClassBlock *cb = CLASS_CB(class);
    ConstantPool *cp = &cb->constant_pool;
    int boot_mthd_idx = entry->boot_method_cp_idx;
    Class *obj_array_class = findArrayClass("[Ljava/lang/Object;");
    Object *name_str = findInternedString(createString(entry->name));
    int mthd_idx = BOOTSTRAP_METHOD_REF(cb->bootstrap_methods, boot_mthd_idx);
    int args = BOOTSTRAP_METHOD_ARG_COUNT(cb->bootstrap_methods, boot_mthd_idx);

    if(args != 0) {
        Object **args_data;
        int i;

        args_array = allocArray(obj_array_class, args, sizeof(Object*));
        if(args_array == NULL)
            return NULL;

        args_data = ARRAY_DATA(args_array, Object*);

        for(i = 0; i < args; i++) {
            int idx = BOOTSTRAP_METHOD_ARG(cb->bootstrap_methods,
                                           boot_mthd_idx, i);
            int prim_idx = cpType2PrimIdx(CP_TYPE(cp, idx));
            Object *arg;

            if(prim_idx != -1) {
                arg = createWrapperObject(prim_idx, &CP_INFO(cp, idx),
                                          REF_SRC_FIELD);
                if(arg == NULL)
                    return NULL;
            } else {
                arg = (Object*)resolveSingleConstant(class, idx);

                if(exceptionOccurred())
                    return NULL;
            }
            args_data[i] = arg;
        }
    }

    appendix_box = allocArray(obj_array_class, 1, sizeof(Object*));
    if(appendix_box == NULL)
        return NULL;

    method_type = findMethodHandleType(entry->type, class);
    if(method_type == NULL)
        return NULL;

    boot_mthd = resolveMethodHandle(class, mthd_idx);
    if(boot_mthd == NULL)
        return NULL;

    member_name = *(Object**)executeStaticMethod(MHN_linkCallSite_mb->class,
                                                 MHN_linkCallSite_mb,
                                                 class, boot_mthd, name_str,
                                                 method_type, args_array,
                                                 appendix_box);

    /* Intercept LinkageErrors */
    if((exception = exceptionOccurred())) {
        if(!isSubClassOf(EXCEPTION(java_lang_BootstrapMethodError),
                         exception->class) &&
           isSubClassOf(EXCEPTION(java_lang_LinkageError), exception->class)) {
            clearException();
            signalChainedException(java_lang_BootstrapMethodError,
                                   NULL, exception);
        }
        return NULL;
    }

    *appendix = ARRAY_DATA(appendix_box, Object*)[0];
    return INST_DATA(member_name, MethodBlock*, mem_name_vmtarget_offset);
}

ResolvedInvDynCPEntry *resolveInvokeDynamic(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);
    ResolvedInvDynCPEntry *entry = NULL;

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_ResolvedInvokeDynamic:
            entry = (ResolvedInvDynCPEntry *)CP_INFO(cp, cp_index);
            break;

        case CONSTANT_InvokeDynamic: {
            Thread *self = threadSelf();
            int boot_mthd_idx, name_type_idx;

            resolveLock(self);
            if(CP_TYPE(cp, cp_index) != CONSTANT_InvokeDynamic) {
                resolveUnlock(self);
                goto retry;
            }

            CP_TYPE(cp, cp_index) = CONSTANT_Locked;
            resolveUnlock(self);

            boot_mthd_idx = CP_INVDYN_BOOT_MTHD(cp, cp_index);
            name_type_idx = CP_INVDYN_NAME_TYPE(cp, cp_index);

            entry = sysMalloc(sizeof(ResolvedInvDynCPEntry));

            entry->idmb_list = NULL;
            entry->boot_method_cp_idx = boot_mthd_idx;
            entry->name = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp, name_type_idx));
            entry->type = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp, name_type_idx));

            CP_INFO(cp, cp_index) = (uintptr_t)entry;
            MBARRIER();
            CP_TYPE(cp, cp_index) = CONSTANT_ResolvedInvokeDynamic;

            break;
        }
    }

    return entry;
}

InvDynMethodBlock *resolveCallSite(ResolvedInvDynCPEntry *entry,
                                   MethodBlock *invoker,
                                   Object *appendix) {

    InvDynMethodBlock *idmb = sysMalloc(sizeof(InvDynMethodBlock));

    idmb->invoker = invoker;
    idmb->appendix = appendix;

#ifndef DIRECT
    idmb->id = entry->idmb_list == NULL ? 0 : entry->idmb_list->id + 1;
    entry->cache = idmb;
#endif

    idmb->next = entry->idmb_list;
    entry->idmb_list = idmb;

    return idmb;
}

#ifndef DIRECT
InvDynMethodBlock *resolvedCallSite(ResolvedInvDynCPEntry *entry, int id) {
    InvDynMethodBlock *idmb = entry->idmb_list;
    int index = idmb->id - id;

    while(index--)
        idmb = idmb->next;

    return entry->cache = idmb;
}
#endif

/* Intrinsic cache hashtable definitions */

#define HASH(ptr) ((ptr->state * 31 + ptr->args_count) * 31 + \
                  ptr->ret_slot_size)

#define COMPARE(ptr1, ptr2, hash1, hash2)                     \
                  (hash1 == hash2 &&                          \
                  ptr1->state == ptr2->state &&               \
                  ptr1->args_count == ptr2->args_count &&     \
                  ptr1->ret_slot_size == ptr2->ret_slot_size)

#define PREPARE(ptr) ptr

#define SCAVENGE(ptr) ({                                      \
    int result = ((MethodBlock*)ptr)->ref_count == 0;         \
    if(result) sysFree(ptr);                                  \
    result;                                                   \
})

#define FOUND(ptr1, ptr2) ({                                  \
    ptr2->ref_count++;                                        \
    ptr2;                                                     \
})

#define ITERATE(ptr) ((MethodBlock*)ptr)->class = method_handle_class

/* Called after heap compaction to update the intrinsic method class
   references.  The method handle class may have moved and as the
   intrinsic methods are not part of the class methods they won't get
   updated normally */

void updateIntrinsicCache() {
    hashIterate(intrinsic_cache);
}

MethodBlock *lookupPolymorphicMethod(Class *class, Class *accessing_class,
                                     char *methodname, char *type) {

    int id = polymorphicNameID(class, methodname);
    MethodBlock *found, *mb;

    if(id <= ID_invokeGeneric)
        return NULL;

    mb = sysMalloc(sizeof(MethodBlock));
    memset(mb, 0, sizeof(MethodBlock));

    mb->type = type;
    mb->class = class;
    mb->name = methodname;
    mb->args_count = sigArgsCount(type);
    mb->access_flags = ACC_PUBLIC | ACC_PRIVATE | ACC_NATIVE;

    if(isStaticPolymorphicSig(id))
        mb->access_flags |= ACC_STATIC;
    else
        mb->args_count++;

    mb->state = id;
    mb->ref_count = 1;
    mb->max_locals = mb->args_count;
    mb->ret_slot_size = sigRetSlotSize(type);
    mb->native_invoker = polymorphicID2Invoker(id);

    /* Add if absent, scavenge, locked */
    findHashEntry(intrinsic_cache, mb, found, TRUE, TRUE, TRUE);

    if(mb != found)
        sysFree(mb);

    return found;
}

int isPolymorphicRef(Class *class, int cp_index) {
    ConstantPool *cp = &(CLASS_CB(class)->constant_pool);

retry:
    switch(CP_TYPE(cp, cp_index)) {
        case CONSTANT_Locked:
            goto retry;

        case CONSTANT_ResolvedPolyMethod:
            return TRUE;

        case CONSTANT_Methodref: {
            char *methodname;
            Class *resolved_class;
            int cl_idx = CP_METHOD_CLASS(cp, cp_index);
            int name_type_idx = CP_METHOD_NAME_TYPE(cp, cp_index);

            MBARRIER();
            if(CP_TYPE(cp, cp_index) != CONSTANT_Methodref)
                goto retry;

            if(CP_TYPE(cp, cl_idx) != CONSTANT_ResolvedClass)
                return FALSE;

            methodname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp, name_type_idx));
            resolved_class = (Class*)CP_INFO(cp, cl_idx);

            return polymorphicNameID(resolved_class, methodname) ==
                       ID_invokeGeneric;
        }
    }

    return FALSE;
}

Object *resolveMemberName(Class *mh_class, Object *mname) {

    Object *name_str = INST_DATA(mname, Object*, mem_name_name_offset);
    Class *clazz = INST_DATA(mname, Class*, mem_name_clazz_offset);
    Object *type = INST_DATA(mname, Object*, mem_name_type_offset);
    int flags = INST_DATA(mname, int, mem_name_flags_offset);
    char *name_utf, *name_sym, *type_sym;
    int name_id;

    if(clazz == NULL || name_str == NULL || type == NULL) {
        signalException(java_lang_IllegalArgumentException, NULL);
        return NULL;
    }

    name_utf = String2Utf8(name_str);
    name_sym = findUtf8(name_utf);
    sysFree(name_utf);

    if(name_sym == NULL || name_sym == SYMBOL(class_init))
        goto throw_excep;

    name_id = polymorphicNameID(clazz, name_sym);
    type_sym = type2Signature(type, name_id != -1);
    if(type_sym == NULL)
        goto throw_excep;

    switch(flags & ALL_KINDS) {
        case IS_METHOD: {
            MethodBlock *mb;

            if(IS_INTERFACE(CLASS_CB(clazz)))
                mb = lookupInterfaceMethod(clazz, name_sym, type_sym);
            else {
                mb = lookupMethod(clazz, name_sym, type_sym);
                if(mb == NULL)
                    mb = lookupPolymorphicMethod(clazz, mh_class, name_sym,
                                                 type_sym);
            }

            if(mb == NULL)
                goto throw_excep;

            flags |= mbFlags(mb);
            INST_DATA(mname, int, mem_name_flags_offset) = flags;
            INST_DATA(mname, MethodBlock*, mem_name_vmtarget_offset) = mb;
            break;
        }
        case IS_CONSTRUCTOR: {
            MethodBlock *mb;

            mb = findMethod(clazz, name_sym, type_sym);
            if(mb == NULL)
                goto throw_excep;

            flags |= mbFlags(mb);
            INST_DATA(mname, int, mem_name_flags_offset) = flags;
            INST_DATA(mname, MethodBlock*, mem_name_vmtarget_offset) = mb;
            break;
        }
        case IS_FIELD: {
            FieldBlock *fb;

            fb = lookupField(clazz, name_sym, type_sym);
            if(fb == NULL)
                goto throw_excep;

            flags |= fb->access_flags;
            INST_DATA(mname, int, mem_name_flags_offset) = flags;
            INST_DATA(mname, FieldBlock*, mem_name_vmtarget_offset) = fb;
            break;
        }

        default:
            goto throw_excep;
    }

    return mname;

throw_excep:
    switch(flags & ALL_KINDS) {
        case IS_METHOD:
        case IS_CONSTRUCTOR:
            signalException(java_lang_NoSuchMethodError, "resolve member name");
            break;

        case IS_FIELD:
            signalException(java_lang_NoSuchFieldError, "resolve member name");
            break;

        default:
            signalException(java_lang_LinkageError, "resolve member name");
            break;
    }
    return NULL;
}

void freeResolvedPolyData(Class *class) {
    ClassBlock *cb = CLASS_CB(class);
    ConstantPool *cp = &cb->constant_pool;
    int i;

    for(i = 1; i < cb->constant_pool_count; i++)
        switch(CP_TYPE(cp, i)) {
            default:
                break;

            case CONSTANT_ResolvedPolyMethod:
                gcPendingFree((void*)CP_INFO(cp, i));
                break;

            case CONSTANT_ResolvedMethod: {
                MethodBlock *mb = (MethodBlock*)CP_INFO(cp, i);

                if(mbPolymorphicNameID(mb) >= ID_invokeGeneric)
                    mb->ref_count--;
                break;
            }

            case CONSTANT_ResolvedInvokeDynamic: {
                ResolvedInvDynCPEntry *entry = (ResolvedInvDynCPEntry*)
                                               CP_INFO(cp, i);
                InvDynMethodBlock *idmb;

                for(idmb = entry->idmb_list; idmb != NULL; idmb = idmb->next)
                    gcPendingFree(idmb);

                gcPendingFree(entry);
                break;
            }
        }
}
#endif
