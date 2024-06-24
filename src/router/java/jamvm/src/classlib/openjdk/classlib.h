/*
 * Copyright (C) 2010, 2011, 2012, 2013, 2014
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

/* Thread */

extern int classlibInitJavaThread(Thread *thread, Object *jlthread,
                                  Object *name, Object *group,
                                  char is_daemon, int priority);

extern Object *classlibThreadPreInit(Class *thread_class,
                                     Class *thrdGrp_class);

extern int classlibThreadPostInit();
extern int classlibCreateJavaThread(Thread *thread, Object *jThread);
extern Thread *classlibJThread2Thread(Object *jThread);
extern Object *classlibMarkThreadTerminated(Object *jThread);

#define classlibThreadIdName() SYMBOL(tid)
#define classlibAddThreadName() SYMBOL(add)
#define classlibThreadNameType() SYMBOL(sig_java_lang_String)
#define classlibRemoveThreadName() SYMBOL(removeThreadName)
#define classlibExceptionHandlerName() SYMBOL(uncaughtExceptionHandler)

extern int classlibGetThreadState(Thread *thread);
extern void classlibSetThreadState(Thread *thread, int state);
extern void classlibThreadName2Buff(Object *jThread, char *buffer,
                                    int buff_len);

extern int classlibInitialiseSignals();
extern void classlibSignalThread(Thread *self);

/* Class */

#define classlibInitialiseClass() \
    /* NOTHING TO DO */ TRUE

extern void classlibCacheClassLoaderFields(Class *loader_class);
extern HashTable *classlibLoaderTable(Object *class_loader);
extern HashTable *classlibCreateLoaderTable(Object *class_loader);
extern Object *classlibBootPackage(PackageEntry *entry);
extern Object *classlibBootPackages(PackageEntry *entry);
extern Class *classlibBootPackagesArrayClass();
extern char *classlibDefaultBootClassPath();
extern char *classlibDefaultEndorsedDirs();
extern char *classlibDefaultExtDirs();

#define classlibBootClassPathOpt(args) \
    classlibDefaultBootClassPath()

extern void classlibNewLibraryUnloader(Object *class_loader, void *entry);
extern Object *classlibSkipReflectionLoader(Object *loader);
extern char *classlibExternalClassName(Class *class);

#ifdef JSR292
#define classlibInjectedFieldsCount(classname) \
    (classname == SYMBOL(java_lang_invoke_MemberName) ? 1 : 0)

#define classlibFillInInjectedFields(classname, field) {      \
    field->name = SYMBOL(vmtarget);                           \
    field->type = sizeof(void*) == 4 ? SYMBOL(I) : SYMBOL(J); \
    field->access_flags = ACC_PRIVATE;                        \
    field->signature = NULL;                                  \
}
#else
#define classlibInjectedFieldsCount(classname) 0
#define classlibFillInInjectedFields(classname, field) {}
#endif

/* Reflection */

extern int classlibInitReflection();
extern Object *classlibCreateConstructorObject(MethodBlock *mb);
extern Object *classlibCreateMethodObject(MethodBlock *mb);
extern Object *classlibCreateFieldObject(FieldBlock *fb);
extern MethodBlock *classlibMbFromReflectObject(Object *reflect_ob);
extern FieldBlock *classlibFbFromReflectObject(Object *reflect_ob);

/* DLL */

extern int classlibInitialiseDll();
extern char *classlibDefaultBootDllPath();
extern void *classlibLookupLoadedDlls(char *name, Object *loader);

/* JNI */

extern int classlibInitialiseJNI();
extern Object *classlibNewDirectByteBuffer(void *addr, long long capacity);
extern void *classlibGetDirectBufferAddress(Object *buff);
extern long long classlibGetDirectBufferCapacity(Object *buff);
extern Object *classlibCheckIfOnLoad(Frame *last);

/* Properties */

#define classlibAddDefaultProperties(properties) \
    /* NOTHING TO DO */

extern char *classlibDefaultJavaHome();

/* Access */
    
extern int classlibInitialiseAccess();
extern int classlibAccessCheck(Class *class1, Class *class2);

/* Natives */

extern int classlibInitialiseNatives();

/* Excep */

extern int classlibInitialiseException(Class *throw_class);

/* Frame */

extern int classlibInitialiseFrame();
extern Frame *classlibGetCallerFrame(Frame *last, int depth);
extern int classlibIsSkippedReflectFrame(Frame *frame);

/* Shutdown */

extern void classlibVMShutdown();

/* Alloc */

#define classlibMarkSpecial(ob, mark) \
    /* NOTHING TO DO */

extern void classlibHandleUnmarkedSpecial(Object *ob);

#ifdef JSR292
#define classlibPostCompact() \
    updateIntrinsicCache()
#else
#define classlibPostCompact() \
    /* NOTHING TO DO */
#endif

/* Method Handles */

#ifdef JSR292
extern int isPolymorphicRef(Class *class, int cp_index);
extern Object *resolveMethodType(Class *class, int cp_index);
extern Object *resolveMethodHandle(Class *class, int cp_index);
extern PolyMethodBlock *resolvePolyMethod(Class *class, int cp_index);

extern MethodBlock *findInvokeDynamicInvoker(Class *class,
                                             ResolvedInvDynCPEntry *entry,
                                             Object **appendix);
extern void resolveLock(Thread *self);
extern void resolveUnlock(Thread *self);
extern ResolvedInvDynCPEntry *resolveInvokeDynamic(Class *class, int cp_index);
extern InvDynMethodBlock *resolveCallSite(ResolvedInvDynCPEntry *entry,
                                          MethodBlock *invoker,
                                          Object *appendix_box);
extern InvDynMethodBlock *resolvedCallSite(ResolvedInvDynCPEntry *entry,
                                           int id);

extern MethodBlock *lookupPolymorphicMethod(Class *class,
                                            Class *accessing_class,
                                            char *methodname, char *type);
extern void cachePolyOffsets(CachedPolyOffsets *cpo);
extern void freeResolvedPolyData(Class *class);
extern void updateIntrinsicCache();

#define CACHED_POLY_OFFSETS                      \
    static CachedPolyOffsets cpo = {-1, -1, -1};

#define CACHE_POLY_OFFSETS                                           \
    if(cpo.mem_name_vmtarget == -1 || cpo.mthd_hndl_form == -1       \
                                   || cpo.lmda_form_vmentry == -1) { \
        cachePolyOffsets(&cpo);                                      \
        MBARRIER();                                                  \
}

#define getInvokeBasicTarget(method_handle) ({                            \
    Object *form = INST_DATA(method_handle, Object*, cpo.mthd_hndl_form); \
    Object *vmentry = INST_DATA(form, Object*, cpo.lmda_form_vmentry);    \
    INST_DATA(vmentry, MethodBlock*, cpo.mem_name_vmtarget);              \
})

#define getLinkToSpecialTarget(mem_name)                      \
    INST_DATA(mem_name, MethodBlock*, cpo.mem_name_vmtarget)

#define getLinkToVirtualTarget(this, mem_name) ({             \
    MethodBlock *vmtarget = INST_DATA(mem_name, MethodBlock*, \
    	                              cpo.mem_name_vmtarget); \
    if(!(vmtarget->access_flags & ACC_PRIVATE)) {             \
        ClassBlock *cb = CLASS_CB((this)->class);             \
        int mtbl_idx = vmtarget->method_table_index;          \
        vmtarget = cb->method_table[mtbl_idx];                \
    }                                                         \
    vmtarget;                                                 \
})

#define getLinkToInterfaceTarget(this, mem_name) ({           \
    MethodBlock *vmtarget = INST_DATA(mem_name, MethodBlock*, \
    	                              cpo.mem_name_vmtarget); \
    lookupVirtualMethod(this, vmtarget);                      \
})
#endif
