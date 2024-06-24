/*
 * Copyright (C) 2011, 2013, 2014, 2015
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

#define JTHREAD                 512
#define CLASSLIB_CLASS_SPECIAL  JTHREAD

/* In OpenJDK 9 the class loader and array component type fields have been
   moved to the Java-level class object.  To support this in JamVM, the
   class_loader and component_class fields have been removed from ClassBlock
   and made part of the classlib.  For versions before OpenJDK 9, they are
   defined as extra fields.  Being references they also require marking and
   threading by GC.  For OpenJDK 9, however, they are defined as part of the
   ClassBlock padding (thus shadowing the Java class object).  They also do
   not need special handling by GC, as being part of the Java object they
   are handled as any other Java-level reference. */

#if OPENJDK_VERSION == 9
#define CLASSLIB_CLASS_PAD                     \
    char pad1[7*sizeof(Object*)];              \
    union {                                    \
        Class *component_class;                \
        Object *host_class;                    \
    };                                         \
    Object *class_loader;                      \
    char pad2[3*sizeof(Object*)+1*sizeof(int)];
#elif OPENJDK_VERSION == 8
#define CLASSLIB_CLASS_PAD char pad[11*sizeof(Object*)+1*sizeof(int)];
#elif OPENJDK_VERSION == 7
#define CLASSLIB_CLASS_PAD char pad[18*sizeof(Object*)+2*sizeof(int)];
#else
#define CLASSLIB_CLASS_PAD char pad[17*sizeof(Object*)+2*sizeof(int)];
#endif

#if OPENJDK_VERSION == 9
#define CLASSLIB_CLASS_EXTRA_FIELDS  \
   Object *protection_domain;        \
   Object *signers;

#define CLASSLIB_CLASSBLOCK_REFS_DO(action, cb, ...) \
    action(cb, protection_domain, ## __VA_ARGS__);   \
    action(cb, signers, ## __VA_ARGS__)

#define CLASSLIB_ARRAY_CLASS_EXTRA_FIELDS
#define CLASSLIB_CLASSBLOCK_ARRAY_REFS_DO(action, cb, ...)
#else
#define CLASSLIB_CLASS_EXTRA_FIELDS  \
   Object *protection_domain;        \
   Object *class_loader;             \
   Object *host_class;               \
   Object *signers;

#define CLASSLIB_CLASSBLOCK_REFS_DO(action, cb, ...) \
    action(cb, protection_domain, ## __VA_ARGS__);   \
    action(cb, class_loader, ## __VA_ARGS__);        \
    action(cb, host_class, ## __VA_ARGS__);          \
    action(cb, signers, ## __VA_ARGS__)

#define CLASSLIB_ARRAY_CLASS_EXTRA_FIELDS Class *component_class;
#define CLASSLIB_CLASSBLOCK_ARRAY_REFS_DO(action, cb, ...) \
    action(cb, component_class, ## __VA_ARGS__);
#endif

#define CLASSLIB_THREAD_EXTRA_FIELDS \
    /* NONE */

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
