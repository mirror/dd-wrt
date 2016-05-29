/*
 * Copyright (C) 2011, 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
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

#define JTHREAD                 512
#define CLASSLIB_CLASS_SPECIAL  JTHREAD

#if OPENJDK_VERSION == 8
#define CLASSLIB_CLASS_PAD_SIZE 10*sizeof(Object*)+1*sizeof(int)
#elif OPENJDK_VERSION == 7
#define CLASSLIB_CLASS_PAD_SIZE 18*sizeof(Object*)+2*sizeof(int)
#else
#define CLASSLIB_CLASS_PAD_SIZE 17*sizeof(Object*)+2*sizeof(int)
#endif

#define CLASSLIB_CLASS_EXTRA_FIELDS  \
   Object *protection_domain;        \
   Object *host_class;               \
   Object *signers;

#define CLASSLIB_THREAD_EXTRA_FIELDS \
    /* NONE */

#define CLASSLIB_CLASSBLOCK_REFS_DO(action, cb, ...) \
    action(cb, protection_domain, ## __VA_ARGS__);   \
    action(cb, host_class, ## __VA_ARGS__);          \
    action(cb, signers, ## __VA_ARGS__)

#ifdef JSR292
#define ID_invokeGeneric   (MB_PREPARED + 1)
#define ID_invokeBasic     (MB_PREPARED + 2)
#define ID_linkToStatic    (MB_PREPARED + 3)
#define ID_linkToSpecial   (MB_PREPARED + 4)
#define ID_linkToVirtual   (MB_PREPARED + 5)
#define ID_linkToInterface (MB_PREPARED + 6)

#define mbPolymorphicNameID(mb) mb->state

typedef struct cached_poly_offsets {
    int mem_name_vmtarget;
    int lmda_form_vmentry;
    int mthd_hndl_form;
} CachedPolyOffsets;

#define CLASSLIB_METHOD_ANNOTATIONS(mb, type_name) {                       \
    if(type_name == SYMBOL(sig_sun_reflect_CallerSensitive))               \
        mb->flags |= MB_CALLER_SENSITIVE;                                  \
    else if(type_name == SYMBOL(sig_java_lang_invoke_LambdaForm_Hidden))   \
        mb->flags |= MB_LAMBDA_HIDDEN;                                     \
    else if(type_name == SYMBOL(sig_java_lang_invoke_LambdaForm_Compiled)) \
        mb->flags |= MB_LAMBDA_COMPILED;                                   \
}
#endif
