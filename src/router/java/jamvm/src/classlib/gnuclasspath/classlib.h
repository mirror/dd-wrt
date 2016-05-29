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


/* Thread */

extern int classlibInitJavaThread(Thread *thread, Object *jlthread,
                                  Object *name, Object *group,
                                  char is_daemon, int priority);

extern Object *classlibThreadPreInit(Class *thread_class,
                                     Class *thrdGrp_class);

extern int classlibCreateJavaThread(Thread *thread, Object *jThread);
extern Object *classlibMarkThreadTerminated(Object *jThread);

extern Thread *classlibJThread2Thread(Object *jThread);

#define classlibThreadPostInit() \
    /* NOTHING TO DO */ TRUE

#define classlibThreadIdName() SYMBOL(threadId)
#define classlibAddThreadName() SYMBOL(addThread)
#define classlibRemoveThreadName() SYMBOL(removeThread)
#define classlibThreadNameType() SYMBOL(sig_java_lang_String)
#define classlibExceptionHandlerName() SYMBOL(exceptionHandler)

#define classlibGetThreadState(thread) \
    (thread)->state

#define classlibSetThreadState(thread, new_state) \
    (thread)->state = new_state

extern void classlibThreadName2Buff(Object *jThread, char *buffer,
                                    int buff_len);

#define classlibInitialiseSignals() \
    /* NOTHING TO DO */ TRUE

extern void classlibSignalThread(Thread *self);


/* Class */

extern int classlibInitialiseClass();
extern void classlibCacheClassLoaderFields(Class *loader_class);
extern HashTable *classlibLoaderTable(Object *class_loader);
extern HashTable *classlibCreateLoaderTable(Object *class_loader);
extern Object *classlibBootPackage(PackageEntry *entry);
extern Object *classlibBootPackages(PackageEntry *entry);
extern Class *classlibBootPackagesArrayClass();
extern char *classlibBootClassPathOpt(InitArgs *args);
extern char *classlibDefaultBootClassPath();
extern char *classlibDefaultEndorsedDirs();
extern char *classlibDefaultExtDirs();

extern void classlibNewLibraryUnloader(Object *class_loader, void *entry);

#define classlibSkipReflectionLoader(loader) \
    loader

#define classlibInjectedFieldsCount(classname) 0
#define classlibFillInInjectedFields(classname, field) {}

#define classlibExternalClassName(class) \
    slash2DotsDup(CLASS_CB(class)->name);


/* Reflection */

extern int classlibInitReflection();
extern Object *classlibCreateConstructorObject(MethodBlock *mb);
extern Object *classlibCreateMethodObject(MethodBlock *mb);
extern Object *classlibCreateFieldObject(FieldBlock *fb);
extern MethodBlock *classlibMbFromReflectObject(Object *reflect_ob);
extern FieldBlock *classlibFbFromReflectObject(Object *reflect_ob);


/* DLL */

#define classlibInitialiseDll() \
    /* NOTHING TO DO */ TRUE

extern char *classlibDefaultBootDllPath();
extern void *classlibLookupLoadedDlls(char *name, Object *loader);


/* JNI */

extern int classlibInitialiseJNI();
extern Object *classlibNewDirectByteBuffer(void *addr, long long capacity);
extern void *classlibGetDirectBufferAddress(Object *buff);
extern long long classlibGetDirectBufferCapacity(Object *buff);
extern Object *classlibCheckIfOnLoad(Frame *last);


/* Properties */

extern char *classlibDefaultJavaHome();
extern void classlibAddDefaultProperties(Object *properties);


/* Access */

#define classlibInitialiseAccess() \
    /* NOTHING TO DO */ TRUE

#define classlibAccessCheck(class1, class2) FALSE

/* Natives */

extern int classlibInitialiseNatives();

/* Excep */

extern int classlibInitialiseException(Class *throw_class);

/* Frame */

#define classlibInitialiseFrame() \
    /* NOTHING TO DO */ TRUE

#define classlibIsSkippedReflectFrame(frame) FALSE

extern Frame *classlibGetCallerFrame(Frame *last, int depth);

/* Shutdown */

#define classlibVMShutdown() exitVM(0)

/* Alloc */

extern void classlibMarkSpecial(Object *ob, int mark);
extern void classlibHandleUnmarkedSpecial(Object *ob);

#define classlibPostCompact() \
    /* NOTHING TO DO */
