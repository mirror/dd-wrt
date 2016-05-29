/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

#ifndef NO_JNI
#include <string.h>
#include <stdlib.h>
#include "jni.h"
#include "jam.h"
#include "hash.h"
#include "thread.h"
#include "class.h"
#include "lock.h"
#include "symbol.h"
#include "excep.h"
#include "reflect.h"
#include "classlib.h"
#include "jni-internal.h"
#include "alloc.h"

#define JNI_VERSION JNI_VERSION_1_6

/* The extra used in expanding the local refs table.
 * This must be >= size of JNIFrame to be thread safe
 * wrt GC thread suspension */
#define LREF_LIST_INCR 8

/* The extra used in expanding the global refs table.
 * Also the initial list size */
#define GREF_LIST_INCR 32

/* The amount of local reference space "ensured" before
   entering a JNI method.  The method is garaunteed to
   be able to create this amount without failure */
#define JNI_DEFAULT_LREFS 16

/* Forward declarations */
const jchar *Jam_GetStringChars(JNIEnv *env, jstring string, jboolean *isCopy);
void Jam_ReleaseStringChars(JNIEnv *env, jstring string, const jchar *chars);
jobject Jam_NewGlobalRef(JNIEnv *env, jobject obj);
void Jam_DeleteGlobalRef(JNIEnv *env, jobject obj);
JNIFrame *ensureJNILrefCapacity(int cap);
static void initJNIGrefs();

int initialiseJNI() {
    /* Initialise the global reference tables */
    initJNIGrefs();

    if(!classlibInitialiseJNI()) {
        jam_fprintf(stderr, "Error initialising VM (initialiseJNI)\n");
        return FALSE;
    }

    return TRUE;
}

/* ---------- Local reference support functions ---------- */

int initJNILrefs() {
    JNIFrame *frame = ensureJNILrefCapacity(JNI_DEFAULT_LREFS);

    if(frame != NULL) {
        frame->next_ref = frame->lrefs + frame->mb->args_count;
        return TRUE;
    }

    return FALSE;
}

JNIFrame *expandJNILrefs(ExecEnv *ee, JNIFrame *frame, int incr) {
    JNIFrame *new_frame = (JNIFrame*)((Object**)frame + incr);

    if((char*)(new_frame + 1) > ee->stack_end)
        return NULL;

    memcpy(new_frame, frame, sizeof(JNIFrame));
    new_frame->ostack = (uintptr_t*)(new_frame + 1);
    ee->last_frame = (Frame*)new_frame;
    memset(frame, 0, incr * sizeof(Object*));
    return new_frame;
}

JNIFrame *ensureJNILrefCapacity(int cap) {
    ExecEnv *ee = getExecEnv();
    JNIFrame *frame = (JNIFrame*)ee->last_frame;
    int size = (Object**)frame - frame->lrefs - frame->mb->args_count;

    if(size < cap) {
        int incr = cap-size;
        if(incr < sizeof(JNIFrame)/sizeof(Object*))
            incr = sizeof(JNIFrame)/sizeof(Object*);

        if((frame = expandJNILrefs(ee, frame, incr)) == NULL) {
            if(ee->overflow++) {
                /* Overflow when we're already throwing stack
                   overflow.  Stack extension should be enough
                   to throw exception, so something's seriously
                   gone wrong - abort the VM! */
                jam_fprintf(stderr, "Fatal stack overflow!  Aborting VM.\n");
                exitVM(1);
            }
            ee->stack_end += STACK_RED_ZONE_SIZE;
            signalException(java_lang_StackOverflowError,
                            "JNI local references");
        }
    }

    return frame;
}

jobject addJNILref(Object *ref) {
    ExecEnv *ee = getExecEnv();
    JNIFrame *frame = (JNIFrame*)ee->last_frame;

    if(ref == NULL)
        return NULL;

    if(frame->next_ref == (Object**)frame)
        if((frame = expandJNILrefs(ee, frame, LREF_LIST_INCR)) == NULL) {
            jam_fprintf(stderr,
                        "JNI - FatalError: cannot expand local references.\n");
            exitVM(1);
        }

    return *frame->next_ref++ = ref;
}

void delJNILref(Object *ref) {
    ExecEnv *ee = getExecEnv();
    JNIFrame *frame = (JNIFrame*)ee->last_frame;
    Object **opntr = frame->lrefs;

    for(; opntr < frame->next_ref; opntr++)
        if(*opntr == ref) {
            *opntr = NULL;
            return;
        }
}
 
JNIFrame *pushJNILrefFrame(int cap) {
    ExecEnv *ee = getExecEnv();
    JNIFrame *frame = (JNIFrame*)ee->last_frame;
    JNIFrame *new_frame = (JNIFrame*)((Object**)(frame + 1) + cap);

    if((char*)(new_frame + 1) > ee->stack_end) {
        signalException(java_lang_OutOfMemoryError, "JNI local references");
        return NULL;
    }

    new_frame->lrefs = new_frame->next_ref = (Object**)(frame + 1);
    new_frame->ostack = (uintptr_t*)(new_frame + 1);
    new_frame->prev = frame->prev;
    new_frame->mb = frame->mb;

    memset(frame + 1, 0, cap * sizeof(Object*));
    ee->last_frame = (Frame*)new_frame;

    return new_frame;
}

void popJNILrefFrame() {
    ExecEnv *ee = getExecEnv();
    JNIFrame *frame = (JNIFrame*)ee->last_frame;
    JNIFrame *prev = (JNIFrame*)frame->lrefs - 1;

    ee->last_frame = (Frame*)prev;
}

/* ----------------- Global reference support ----------------- */

/* There are 3 global reference tables, one for normal (strong)
   global references, one for weak global references and one for
   weak references that have been cleared */
#define NUM_GLOBAL_TABLES 3

typedef struct global_ref_table {
    Object **table;
    int    size;
    int    next;
    int    has_deleted;
    VMLock lock;
} GlobalRefTable;

static GlobalRefTable global_refs[NUM_GLOBAL_TABLES];

static void initJNIGrefs() {
    int i;

    for(i = 0; i < NUM_GLOBAL_TABLES; i++)
        initVMLock(global_refs[i].lock);
}

void lockJNIGrefs(Thread *self, int type) {
    /* Disabling and enabling suspension is slow,
       so only do it if we have to block */
    if(!tryLockVMLock(global_refs[type].lock, self)) {
        disableSuspend(self);
        lockVMLock(global_refs[type].lock, self);
        enableSuspend(self);
    }
    fastDisableSuspend(self);
}

void unlockJNIGrefs(Thread *self, int type) {
    fastEnableSuspend(self);
    unlockVMLock(global_refs[type].lock, self);
}

void addJNIGrefUnlocked(Object *ref, int type) {
    if(global_refs[type].next == global_refs[type].size) {
        /* To save the cost of shuffling entries every time a ref is deleted,
           refs are simply nulled-out, and a flag is set.  We then compact
           the table when it becomes full */
        if(global_refs[type].has_deleted) {
            int i, j;

            for(i = j = 0; i < global_refs[type].size; i++)
                if(global_refs[type].table[i])
                    global_refs[type].table[j++] = global_refs[type].table[i];

            global_refs[type].has_deleted = FALSE;
            global_refs[type].next = j;
        }

        if(global_refs[type].next + GREF_LIST_INCR > global_refs[type].size) {
            global_refs[type].size = global_refs[type].next + GREF_LIST_INCR;
            global_refs[type].table = sysRealloc(global_refs[type].table,
                                   global_refs[type].size * sizeof(Object*));
        }
    }

    global_refs[type].table[global_refs[type].next++] = ref;
}

static jobject addJNIGref(Object *ref, int type) {
    if(ref == NULL)
        return NULL;
    else {
        Thread *self = threadSelf();

        lockJNIGrefs(self, type);
        addJNIGrefUnlocked(ref, type);
        unlockJNIGrefs(self, type);

        return OBJ_TO_REF(ref, type);
    }
}

static int delJNIGref(Object *ref, int type) {
    Thread *self = threadSelf();
    int i;

    lockJNIGrefs(self, type);

    /* As NewRef and DeleteRef are paired, we search
       backwards to optimise the case where the ref
       has recently been added */
    for(i = global_refs[type].next - 1; i >= 0; i--)
        if(global_refs[type].table[i] == ref) {

            /* If it's the last ref we can just update
               the next free index */
            if(i == global_refs[type].next - 1)
                global_refs[type].next = i;
            else {
                global_refs[type].table[i] = NULL;
                global_refs[type].has_deleted = TRUE;
            }
            break;
        }

    unlockJNIGrefs(self, type);
    return i >= 0;
}

/* Called during mark phase of GC.  No need to
   grab lock as no thread can be suspended
   while the list is being modified */

#define MARK_JNI_GLOBAL_REFS(type, ref_type)                    \
void markJNI##type##Refs() {                                    \
    int i;                                                      \
                                                                \
    for(i = 0; i < global_refs[ref_type].next; i++)             \
        if(global_refs[ref_type].table[i])                      \
            markJNI##type##Ref(global_refs[ref_type].table[i]); \
}

MARK_JNI_GLOBAL_REFS(Global, GLOBAL_REF)
MARK_JNI_GLOBAL_REFS(ClearedWeak, CLEARED_WEAK_REF)

void scanJNIWeakGlobalRefs() {
    int i;

    for(i = 0; i < global_refs[WEAK_GLOBAL_REF].next; i++) {
        Object *ref = global_refs[WEAK_GLOBAL_REF].table[i];

        if(ref != NULL && !isMarkedJNIWeakGlobalRef(ref)) {
            addJNIGrefUnlocked(ref, CLEARED_WEAK_REF);
            global_refs[WEAK_GLOBAL_REF].table[i] = NULL;
            global_refs[WEAK_GLOBAL_REF].has_deleted = TRUE;
        }
    }
}

/* --------------- JNI interface functions --------------- */

/* Extensions added to JNI in JDK 1.6 */

jobjectRefType Jam_GetObjectRefType(JNIEnv *env, jobject obj) {
    if(obj != NULL) {
        switch(REF_TYPE(obj)) {
            case GLOBAL_REF:
                return JNIGlobalRefType;

            case WEAK_GLOBAL_REF:
                return JNIWeakGlobalRefType;

            case LOCAL_REF:
                return JNILocalRefType;

            default:
                break;
        }
    }

    return JNIInvalidRefType;
}

/* Extensions added to JNI in JDK 1.4 */

jobject Jam_NewDirectByteBuffer(JNIEnv *env, void *addr, jlong capacity) {
    Object *buff = classlibNewDirectByteBuffer(addr, capacity);
    return addJNILref(buff);
}

static void *Jam_GetDirectBufferAddress(JNIEnv *env, jobject buffer) {
    Object *buff = REF_TO_OBJ(buffer);
    return classlibGetDirectBufferAddress(buff);
}

jlong Jam_GetDirectBufferCapacity(JNIEnv *env, jobject buffer) {
    Object *buff = REF_TO_OBJ(buffer);
    return classlibGetDirectBufferCapacity(buff);
}

/* Extensions added to JNI in JDK 1.2 */

jmethodID Jam_FromReflectedMethod(JNIEnv *env, jobject method) {
    return mbFromReflectObject(REF_TO_OBJ(method));
}

jfieldID Jam_FromReflectedField(JNIEnv *env, jobject field) {
    return fbFromReflectObject(REF_TO_OBJ(field));
}

jobject Jam_ToReflectedMethod(JNIEnv *env, jclass cls, jmethodID methodID,
                              jboolean isStatic) {

    MethodBlock *mb = methodID;
    Object *method;

    if(mb->name == SYMBOL(object_init))
        method = createReflectConstructorObject(mb);
    else
        method = createReflectMethodObject(mb);

    return addJNILref(method);
}

jobject Jam_ToReflectedField(JNIEnv *env, jclass cls, jfieldID fieldID,
                             jboolean isStatic) {

    Object *field = createReflectFieldObject(fieldID);
    return addJNILref(field);
}

jint Jam_PushLocalFrame(JNIEnv *env, jint capacity) {
    return pushJNILrefFrame(capacity) == NULL ? JNI_ERR : JNI_OK;
}

jobject Jam_PopLocalFrame(JNIEnv *env, jobject result) {
    popJNILrefFrame();
    return addJNILref(REF_TO_OBJ(result));
}

jobject Jam_NewLocalRef(JNIEnv *env, jobject obj) {
    return addJNILref(REF_TO_OBJ_WEAK_NULL_CHECK(obj));
}

jint Jam_EnsureLocalCapacity(JNIEnv *env, jint capacity) {
    return ensureJNILrefCapacity(capacity) == NULL ? JNI_ERR : JNI_OK;
}

void Jam_GetStringRegion(JNIEnv *env, jstring string_ref, jsize start,
                         jsize len, jchar *buf) {

    Object *string = REF_TO_OBJ(string_ref);

    if((start + len) > getStringLen(string))
        signalException(java_lang_StringIndexOutOfBoundsException, NULL);
    else
        memcpy(buf, getStringChars(string) + start, len * sizeof(short));
}

void Jam_GetStringUTFRegion(JNIEnv *env, jstring string_ref, jsize start,
                            jsize len, char *buf) {

    Object *string = REF_TO_OBJ(string_ref);

    if((start + len) > getStringLen(string))
        signalException(java_lang_StringIndexOutOfBoundsException, NULL);
    else
        StringRegion2Utf8(string, start, len, buf);
}

void *Jam_GetPrimitiveArrayCritical(JNIEnv *env, jarray array_ref,
                                    jboolean *isCopy) {

    Object *array = REF_TO_OBJ(array_ref);

    if(isCopy != NULL)
        *isCopy = JNI_FALSE;

    /* Pin the array */
    addJNIGref(array, GLOBAL_REF);

    return ARRAY_DATA(array, void);
}

void Jam_ReleasePrimitiveArrayCritical(JNIEnv *env, jarray array,
                                       void *carray, jint mode) {

    delJNIGref(REF_TO_OBJ(array), GLOBAL_REF);
}

const jchar *Jam_GetStringCritical(JNIEnv *env, jstring string,
                                   jboolean *isCopy) {

    return Jam_GetStringChars(env, string, isCopy);
}

void Jam_ReleaseStringCritical(JNIEnv *env, jstring string,
                               const jchar *chars) {

    Jam_ReleaseStringChars(env, string, chars);
}

jweak Jam_NewWeakGlobalRef(JNIEnv *env, jobject obj) {
    return addJNIGref(REF_TO_OBJ_WEAK_NULL_CHECK(obj), WEAK_GLOBAL_REF);
}

void Jam_DeleteWeakGlobalRef(JNIEnv *env, jweak obj) {
    if(REF_TYPE(obj) == WEAK_GLOBAL_REF) {
        Object *ob = REF_TO_OBJ(obj);

        if(!delJNIGref(ob, WEAK_GLOBAL_REF))
            delJNIGref(ob, CLEARED_WEAK_REF);
    }
}

jboolean Jam_ExceptionCheck(JNIEnv *env) {
    return exceptionOccurred() ? JNI_TRUE : JNI_FALSE;
}

/* JNI 1.1 interface */

jint Jam_GetVersion(JNIEnv *env) {
    return JNI_VERSION;
}

jclass Jam_DefineClass(JNIEnv *env, const char *name, jobject loader,
                       const jbyte *buf, jsize bufLen) {

    Class *class = defineClass((char*)name, (char *)buf, 0,
                               (int)bufLen, REF_TO_OBJ(loader));

    if(class != NULL)
        linkClass(class);

    return addJNILref(class);
}

jclass Jam_FindClass(JNIEnv *env, const char *name) {
    /* We use the class loader associated with the calling native method.
       However, if this has been called from an attached thread there may
       be no native Java frame.  In this case use the system class loader */
    Frame *last = getExecEnv()->last_frame;
    Object *loader;
    Class *class;

    if(last->prev != NULL) {
        loader = CLASS_CB(last->mb->class)->class_loader;

        /* Ensure correct context if called from JNI_OnLoad */
        if(loader == NULL)
            loader = classlibCheckIfOnLoad(last);
    } else
        loader = getSystemClassLoader();

    class = findClassFromClassLoader((char*) name, loader);

    return addJNILref(class);
}

jclass Jam_GetSuperClass(JNIEnv *env, jclass clazz) {
    ClassBlock *cb = CLASS_CB(REF_TO_OBJ(clazz));
    return IS_INTERFACE(cb) ? NULL : addJNILref(cb->super);
}

jboolean Jam_IsAssignableFrom(JNIEnv *env, jclass clazz1, jclass clazz2) {
    return isInstanceOf(REF_TO_OBJ(clazz2), REF_TO_OBJ(clazz1));
}

jint Jam_Throw(JNIEnv *env, jthrowable obj) {
    setException(REF_TO_OBJ(obj));
    return JNI_TRUE;
}

jint Jam_ThrowNew(JNIEnv *env, jclass clazz, const char *message) {
    signalExceptionClass(REF_TO_OBJ(clazz), (char*)message);
    return JNI_TRUE;
}

jthrowable Jam_ExceptionOccurred(JNIEnv *env) {
    return addJNILref(exceptionOccurred());
}

void Jam_ExceptionDescribe(JNIEnv *env) {
    printException();
}

void Jam_ExceptionClear(JNIEnv *env) {
    clearException();
}

void Jam_FatalError(JNIEnv *env, const char *message) {
    jam_fprintf(stderr, "JNI - FatalError: %s\n", message);
    exitVM(1);
}

jobject Jam_NewGlobalRef(JNIEnv *env, jobject obj) {
    return addJNIGref(REF_TO_OBJ_WEAK_NULL_CHECK(obj), GLOBAL_REF);
}

void Jam_DeleteGlobalRef(JNIEnv *env, jobject obj) {
    if(REF_TYPE(obj) == GLOBAL_REF)
        delJNIGref(REF_TO_OBJ(obj), GLOBAL_REF);
}

void Jam_DeleteLocalRef(JNIEnv *env, jobject obj) {
    delJNILref(obj);
}

jboolean Jam_IsSameObject(JNIEnv *env, jobject obj1, jobject obj2) {
    return REF_TO_OBJ_WEAK_NULL_CHECK(obj1)
        == REF_TO_OBJ_WEAK_NULL_CHECK(obj2);
}

/* JNI helper function.  The class may be invalid
   or it may not have been initialised yet */
Object *allocObjectClassCheck(Class *class) {
    ClassBlock *cb = CLASS_CB(class);

    /* Check the class can be instantiated */
    if(cb->access_flags & (ACC_INTERFACE | ACC_ABSTRACT)) {
        signalException(java_lang_InstantiationException, cb->name);
        return NULL;
    }

    /* Creating an instance of a class is an active use;
       make sure it is initialised */
    if(initClass(class) == NULL)
        return NULL;
        
    return allocObject(class);
}

jobject Jam_AllocObject(JNIEnv *env, jclass clazz) {
    Object *obj = allocObjectClassCheck(REF_TO_OBJ(clazz));
    return addJNILref(obj);
}

jclass Jam_GetObjectClass(JNIEnv *env, jobject obj) {
    return addJNILref(REF_TO_OBJ(obj)->class);
}

jboolean Jam_IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz) {
    return (obj == NULL) || isInstanceOf(REF_TO_OBJ(clazz),
                                         REF_TO_OBJ(obj)->class);
}

jmethodID getMethodID(JNIEnv *env, jclass clazz, const char *name,
                      const char *sig, char is_static) {

    Class *class = initClass(REF_TO_OBJ(clazz));
    MethodBlock *mb = NULL;

    if(class != NULL) {
        if(!IS_PRIMITIVE(CLASS_CB(class))) {
            char *method_name = findUtf8((char*)name);
            char *method_sig = findUtf8((char*)sig);

            if(method_name != NULL && method_sig != NULL) {
                if(method_name == SYMBOL(object_init) ||
                   method_name == SYMBOL(class_init))
                    mb = findMethod(class, method_name, method_sig);
                else
                    mb = lookupMethod(class, method_name, method_sig);
            }
        }

        if(mb == NULL || ((mb->access_flags & ACC_STATIC) != 0) != is_static)
            signalException(java_lang_NoSuchMethodError, (char*)name);
    }

    return mb;
}

jmethodID Jam_GetMethodID(JNIEnv *env, jclass clazz, const char *name,
                          const char *sig) {

    return getMethodID(env, clazz, name, sig, FALSE);
}

jfieldID Jam_GetFieldID(JNIEnv *env, jclass clazz, const char *name,
                        const char *sig) {

    char *field_name = findUtf8((char*)name);
    char *field_sig = findUtf8((char*)sig);

    Class *class = initClass(REF_TO_OBJ(clazz));
    FieldBlock *fb = NULL;

    if(class != NULL) {
        if(field_name != NULL && field_sig != NULL)
            fb = lookupField(class, field_name, field_sig);

        if(fb == NULL)
            signalException(java_lang_NoSuchFieldError, field_name);
    }

    return fb;
}

jmethodID Jam_GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name,
                                const char *sig) {

    return getMethodID(env, clazz, name, sig, TRUE);
}

jfieldID Jam_GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name,
                              const char *sig) {

    char *field_name = findUtf8((char*)name);
    char *field_sig = findUtf8((char*)sig);

    Class *class = initClass(REF_TO_OBJ(clazz));
    FieldBlock *fb = NULL;

    if(class != NULL) {
        if(field_name != NULL && field_sig != NULL)
            fb = findField(class, field_name, field_sig);

        if(fb == NULL)
            signalException(java_lang_NoSuchFieldError, field_name);
    }

    return fb;
}

jstring Jam_NewString(JNIEnv *env, const jchar *unicodeChars, jsize len) {
    Object *str = createStringFromUnicode((unsigned short*)unicodeChars, len);
    return addJNILref(str);
}

jsize Jam_GetStringLength(JNIEnv *env, jstring string) {
    return getStringLen(REF_TO_OBJ(string));
}

const jchar *Jam_GetStringChars(JNIEnv *env, jstring string_ref,
                                jboolean *isCopy) {

    Object *string = REF_TO_OBJ(string_ref);

    if(isCopy != NULL)
        *isCopy = JNI_FALSE;

    /* Pin the reference */
    addJNIGref(getStringCharsArray(string), GLOBAL_REF);

    return (const jchar *)getStringChars(string);
}

void Jam_ReleaseStringChars(JNIEnv *env, jstring string, const jchar *chars) {
    /* Unpin the reference */
    delJNIGref(getStringCharsArray(REF_TO_OBJ(string)), GLOBAL_REF);
}

jstring Jam_NewStringUTF(JNIEnv *env, const char *bytes) {
    if(bytes == NULL)
        return NULL;
    return addJNILref(createString((char*)bytes));
}

jsize Jam_GetStringUTFLength(JNIEnv *env, jstring string) {
    if(string == NULL)
        return 0;
    return getStringUtf8Len(REF_TO_OBJ(string));
}

const char *Jam_GetStringUTFChars(JNIEnv *env, jstring string,
                                  jboolean *isCopy) {
    if(isCopy != NULL)
        *isCopy = JNI_TRUE;

    if(string == NULL)
        return NULL;
    return (const char*)String2Utf8(REF_TO_OBJ(string));
}

void Jam_ReleaseStringUTFChars(JNIEnv *env, jstring string, const char *chars) {
    sysFree((void*)chars);
}

jsize Jam_GetArrayLength(JNIEnv *env, jarray array) {
    return ARRAY_LEN(REF_TO_OBJ(array));
}

jobject Jam_NewObject(JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    Object *ob = allocObjectClassCheck(REF_TO_OBJ(clazz));

    if(ob != NULL) {
        va_list jargs;
        va_start(jargs, methodID);
        executeMethodVaList(ob, ob->class, methodID, jargs);
        va_end(jargs);
    }

    return addJNILref(ob);
}

jobject Jam_NewObjectA(JNIEnv *env, jclass clazz, jmethodID methodID,
                       jvalue *args) {

    Object *ob = allocObjectClassCheck(REF_TO_OBJ(clazz));

    if(ob != NULL)
        executeMethodList(ob, ob->class, methodID, (u8*)args);

    return addJNILref(ob);
}

jobject Jam_NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID,
                       va_list args) {

    Object *ob = allocObjectClassCheck(REF_TO_OBJ(clazz));

    if(ob != NULL)
        executeMethodVaList(ob, ob->class, methodID, args);

    return addJNILref(ob);
}

jarray Jam_NewObjectArray(JNIEnv *env, jsize length, jclass elementClass_ref,
                          jobject initialElement_ref) {

    Object *initial_element = REF_TO_OBJ(initialElement_ref);
    Class *element_class = REF_TO_OBJ(elementClass_ref);
    Object *array;

    if(length < 0) {
        signalException(java_lang_NegativeArraySizeException, NULL);
        return NULL;
    }

    array = allocObjectArray(element_class, length);

    if(array != NULL && initial_element != NULL) {
        Object **data = ARRAY_DATA(array, Object*);

        while(length--)
           *data++ = initial_element;
    }

    return addJNILref(array);
}

jarray Jam_GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index) {
    return addJNILref(ARRAY_DATA(REF_TO_OBJ(array), Object*)[index]);
}

void Jam_SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index,
                               jobject value) {

    ARRAY_DATA(REF_TO_OBJ(array), Object*)[index] = REF_TO_OBJ(value);
}

jint Jam_RegisterNatives(JNIEnv *env, jclass clazz,
                         const JNINativeMethod *methods, jint nMethods) {

    int i;
    Class *class = REF_TO_OBJ(clazz);

    for(i = 0; i < nMethods; i++) {
        MethodBlock *mb = NULL;
        char *method_name = findUtf8(methods[i].name);
        char *method_sig = findUtf8(methods[i].signature);

        if(method_name != NULL && method_sig != NULL)
            mb = findMethod(class, method_name, method_sig);

        if(mb == NULL || !(mb->access_flags & ACC_NATIVE)) {
            signalException(java_lang_NoSuchMethodError, methods[i].name);
            return JNI_ERR;
        }

        setJNIMethod(mb, methods[i].fnPtr);
    }

    return JNI_OK;
}

jint Jam_UnregisterNatives(JNIEnv *env, jclass clazz) {
    return JNI_OK;
}

jint Jam_MonitorEnter(JNIEnv *env, jobject obj) {
    objectLock(REF_TO_OBJ(obj));
    return JNI_OK;
}

jint Jam_MonitorExit(JNIEnv *env, jobject obj) {
    objectUnlock(REF_TO_OBJ(obj));
    return JNI_OK;
}

struct _JNIInvokeInterface Jam_JNIInvokeInterface;
JavaVM jni_invoke_intf = &Jam_JNIInvokeInterface; 

jint Jam_GetJavaVM(JNIEnv *env, JavaVM **vm) {
    *vm = &jni_invoke_intf;
    return JNI_OK;
}

#define GET_FIELD(type, native_type)                                         \
native_type Jam_Get##type##Field(JNIEnv *env, jobject obj,                   \
                                 jfieldID fieldID) {                         \
    FieldBlock *fb = fieldID;                                                \
    Object *ob = REF_TO_OBJ(obj);                                            \
    return INST_DATA(ob, native_type, fb->u.offset);                         \
}

#define INT_GET_FIELD(type, native_type)                                     \
native_type Jam_Get##type##Field(JNIEnv *env, jobject obj,                   \
                                 jfieldID fieldID) {                         \
    FieldBlock *fb = fieldID;                                                \
    Object *ob = REF_TO_OBJ(obj);                                            \
    return (native_type)INST_DATA(ob, int, fb->u.offset);                    \
}

#define SET_FIELD(type, native_type)                                         \
void Jam_Set##type##Field(JNIEnv *env, jobject obj, jfieldID fieldID,        \
                          native_type value) {                               \
    FieldBlock *fb = fieldID;                                                \
    Object *ob = REF_TO_OBJ(obj);                                            \
    INST_DATA(ob, native_type, fb->u.offset) = value;                        \
}

#define INT_SET_FIELD(type, native_type)                                     \
void Jam_Set##type##Field(JNIEnv *env, jobject obj, jfieldID fieldID,        \
                          native_type value) {                               \
    FieldBlock *fb = fieldID;                                                \
    Object *ob = REF_TO_OBJ(obj);                                            \
    INST_DATA(ob, int, fb->u.offset) = (int)value;                           \
}

#define GET_STATIC_FIELD(type, native_type)                                  \
native_type Jam_GetStatic##type##Field(JNIEnv *env, jclass clazz,            \
                                       jfieldID fieldID) {                   \
    FieldBlock *fb = fieldID;                                                \
    return *(native_type *)fb->u.static_value.data;                          \
}

#define INT_GET_STATIC_FIELD(type, native_type)                              \
native_type Jam_GetStatic##type##Field(JNIEnv *env, jclass clazz,            \
                                       jfieldID fieldID) {                   \
    FieldBlock *fb = fieldID;                                                \
    return (native_type)fb->u.static_value.i;                                \
}

#define SET_STATIC_FIELD(type, native_type)                                  \
void Jam_SetStatic##type##Field(JNIEnv *env, jclass clazz, jfieldID fieldID, \
                                native_type value) {                         \
    FieldBlock *fb = fieldID;                                                \
    *(native_type *)fb->u.static_value.data = value;                         \
}

#define INT_SET_STATIC_FIELD(type, native_type)                              \
void Jam_SetStatic##type##Field(JNIEnv *env, jclass clazz, jfieldID fieldID, \
                native_type value) {                                         \
    FieldBlock *fb = fieldID;                                                \
    fb->u.static_value.i = (int)value;                                       \
}

#define FIELD_ACCESS(type, native_type)          \
        GET_FIELD(type, native_type);            \
        SET_FIELD(type, native_type);            \
        GET_STATIC_FIELD(type, native_type);     \
        SET_STATIC_FIELD(type, native_type);

#define INT_FIELD_ACCESS(type, native_type)      \
        INT_GET_FIELD(type, native_type);        \
        INT_SET_FIELD(type, native_type);        \
        INT_GET_STATIC_FIELD(type, native_type); \
        INT_SET_STATIC_FIELD(type, native_type);

INT_FIELD_ACCESS(Boolean, jboolean);
INT_FIELD_ACCESS(Byte, jbyte);
INT_FIELD_ACCESS(Char, jchar);
INT_FIELD_ACCESS(Short, jshort);
INT_FIELD_ACCESS(Int, jint);
FIELD_ACCESS(Long, jlong);
FIELD_ACCESS(Float, jfloat);
FIELD_ACCESS(Double, jdouble);

jobject Jam_GetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    Object *ob = REF_TO_OBJ(obj);
    FieldBlock *fb = fieldID;

    return addJNILref(INST_DATA(ob, Object*, fb->u.offset));
}

void Jam_SetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID,
                        jobject value) {
    Object *ob = REF_TO_OBJ(obj);
    FieldBlock *fb = fieldID;

    INST_DATA(ob, jobject, fb->u.offset) = REF_TO_OBJ(value);
}

jobject Jam_GetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID) {
    FieldBlock *fb = fieldID;
    return addJNILref(fb->u.static_value.p);
}

void Jam_SetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID,
                              jobject value) {

    FieldBlock *fb = fieldID;
    fb->u.static_value.p = value;
}

#define VIRTUAL_METHOD(type, native_type)                                    \
native_type Jam_Call##type##Method(JNIEnv *env, jobject obj,                 \
                                   jmethodID mID, ...) {                     \
    Object *ob = REF_TO_OBJ(obj);                                            \
    native_type *ret;                                                        \
    va_list jargs;                                                           \
                                                                             \
    MethodBlock *mb = lookupVirtualMethod(ob, mID);                          \
    if(mb == NULL)                                                           \
        return (native_type) 0;                                              \
                                                                             \
    va_start(jargs, mID);                                                    \
    ret = (native_type*) executeMethodVaList(ob, ob->class, mb, jargs);      \
    va_end(jargs);                                                           \
                                                                             \
    return *ret;                                                             \
}                                                                            \
                                                                             \
native_type Jam_Call##type##MethodV(JNIEnv *env, jobject obj, jmethodID mID, \
                                    va_list jargs) {                         \
    Object *ob = REF_TO_OBJ(obj);                                            \
    MethodBlock *mb = lookupVirtualMethod(ob, mID);                          \
    if(mb == NULL)                                                           \
        return (native_type) 0;                                              \
    return *(native_type*)executeMethodVaList(ob, ob->class, mb, jargs);     \
}                                                                            \
                                                                             \
native_type Jam_Call##type##MethodA(JNIEnv *env, jobject obj, jmethodID mID, \
                                    jvalue *jargs) {                         \
    Object *ob = REF_TO_OBJ(obj);                                            \
    MethodBlock *mb = lookupVirtualMethod(ob, mID);                          \
    if(mb == NULL)                                                           \
        return (native_type) 0;                                              \
    return *(native_type*)executeMethodList(ob, ob->class, mb, (u8*)jargs);  \
}

#define NONVIRTUAL_METHOD(type, native_type)                                 \
native_type Jam_CallNonvirtual##type##Method(JNIEnv *env, jobject obj,       \
                                             jclass clazz, jmethodID mID,    \
                                             ...) {                          \
    native_type *ret;                                                        \
    va_list jargs;                                                           \
                                                                             \
    va_start(jargs, mID);                                                    \
    ret = (native_type*) executeMethodVaList(REF_TO_OBJ(obj),                \
                                             REF_TO_OBJ(clazz), mID, jargs); \
    va_end(jargs);                                                           \
                                                                             \
    return *ret;                                                             \
}                                                                            \
                                                                             \
native_type Jam_CallNonvirtual##type##MethodV(JNIEnv *env, jobject obj,      \
                                              jclass clazz, jmethodID mID,   \
                                              va_list jargs) {               \
    return *(native_type*)                                                   \
                  executeMethodVaList(REF_TO_OBJ(obj), REF_TO_OBJ(clazz),    \
                                      mID, jargs);                           \
}                                                                            \
                                                                             \
native_type Jam_CallNonvirtual##type##MethodA(JNIEnv *env, jobject obj,      \
                                              jclass clazz, jmethodID mID,   \
                                              jvalue *jargs) {               \
    return *(native_type*)                                                   \
                  executeMethodList(REF_TO_OBJ(obj), REF_TO_OBJ(clazz),      \
                                    mID, (u8*)jargs);                        \
}

#define STATIC_METHOD(type, native_type)                                     \
native_type Jam_CallStatic##type##Method(JNIEnv *env, jclass clazz,          \
                                         jmethodID methodID, ...) {          \
    native_type *ret;                                                        \
    va_list jargs;                                                           \
                                                                             \
    va_start(jargs, methodID);                                               \
    ret = (native_type*)                                                     \
             executeMethodVaList(NULL, REF_TO_OBJ(clazz), methodID, jargs);  \
    va_end(jargs);                                                           \
                                                                             \
    return *ret;                                                             \
}                                                                            \
                                                                             \
native_type Jam_CallStatic##type##MethodV(JNIEnv *env, jclass clazz,         \
                                          jmethodID mID, va_list jargs) {    \
    return *(native_type*)                                                   \
               executeMethodVaList(NULL, REF_TO_OBJ(clazz), mID, jargs);     \
}                                                                            \
                                                                             \
native_type Jam_CallStatic##type##MethodA(JNIEnv *env, jclass clazz,         \
                                          jmethodID mID, jvalue *jargs) {    \
    return *(native_type*)                                                   \
               executeMethodList(NULL, REF_TO_OBJ(clazz), mID, (u8*)jargs);  \
}

#define CALL_METHOD(access)         \
access##_METHOD(Boolean, jboolean); \
access##_METHOD(Byte, jbyte);       \
access##_METHOD(Char, jchar);       \
access##_METHOD(Short, jshort);     \
access##_METHOD(Int, jint);         \
access##_METHOD(Long, jlong);       \
access##_METHOD(Float, jfloat);     \
access##_METHOD(Double, jdouble);

CALL_METHOD(VIRTUAL);
CALL_METHOD(NONVIRTUAL);
CALL_METHOD(STATIC);

jobject Jam_CallObjectMethod(JNIEnv *env, jobject obj,
                             jmethodID methodID, ...) {
    va_list jargs;
    Object **ret, *ob = REF_TO_OBJ(obj);
    MethodBlock *mb = lookupVirtualMethod(ob, methodID);

    if(mb == NULL)
        return NULL;

    va_start(jargs, methodID);
    ret = executeMethodVaList(ob, ob->class, mb, jargs);
    va_end(jargs);

    return addJNILref(*ret);
}

jobject Jam_CallObjectMethodV(JNIEnv *env, jobject obj, jmethodID methodID,
                              va_list jargs) {

    Object **ret, *ob = REF_TO_OBJ(obj);
    MethodBlock *mb = lookupVirtualMethod(ob, methodID);

    if(mb == NULL)
        return NULL;

    ret = executeMethodVaList(ob, ob->class, mb, jargs);
    return addJNILref(*ret);
}

jobject Jam_CallObjectMethodA(JNIEnv *env, jobject obj, jmethodID methodID,
                              jvalue *jargs) {

    Object **ret, *ob = REF_TO_OBJ(obj);
    MethodBlock *mb = lookupVirtualMethod(ob, methodID);

    if(mb == NULL)
        return NULL;

    ret = executeMethodList(ob, ob->class, mb, (u8*)jargs);
    return addJNILref(*ret);
}

jobject Jam_CallNonvirtualObjectMethod(JNIEnv *env, jobject obj, jclass clazz,
                                       jmethodID methodID, ...) {
    Object **ret;
    va_list jargs;

    va_start(jargs, methodID);
    ret = executeMethodVaList(REF_TO_OBJ(obj), REF_TO_OBJ(clazz),
                              methodID, jargs);
    va_end(jargs);

    return addJNILref(*ret);
}

jobject Jam_CallNonvirtualObjectMethodV(JNIEnv *env, jobject obj, jclass clazz,
                                        jmethodID methodID, va_list jargs) {

    Object **ret = executeMethodVaList(REF_TO_OBJ(obj), REF_TO_OBJ(clazz),
                                       methodID, jargs);
    return addJNILref(*ret);
}

jobject Jam_CallNonvirtualObjectMethodA(JNIEnv *env, jobject obj, jclass clazz,
                                        jmethodID methodID, jvalue *jargs) {

    Object **ret = executeMethodList(REF_TO_OBJ(obj), REF_TO_OBJ(clazz),
                                     methodID, (u8*)jargs);
    return addJNILref(*ret);
}

jobject Jam_CallStaticObjectMethod(JNIEnv *env, jclass clazz,
                                   jmethodID methodID, ...) {
    Object **ret;
    va_list jargs;

    va_start(jargs, methodID);
    ret = executeMethodVaList(NULL, REF_TO_OBJ(clazz), methodID, jargs);
    va_end(jargs);

    return addJNILref(*ret);
}

jobject Jam_CallStaticObjectMethodV(JNIEnv *env, jclass clazz,
                jmethodID methodID, va_list jargs) {

    Object **ret = executeMethodVaList(NULL, REF_TO_OBJ(clazz),
                                       methodID, jargs);
    return addJNILref(*ret);
}

jobject Jam_CallStaticObjectMethodA(JNIEnv *env, jclass clazz,
                                    jmethodID methodID, jvalue *jargs) {

    Object **ret = executeMethodList(NULL, REF_TO_OBJ(clazz),
                                     methodID, (u8*)jargs);
    return addJNILref(*ret);
}

void Jam_CallVoidMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    va_list jargs;
    MethodBlock *mb;
    Object *ob = REF_TO_OBJ(obj);
 
    va_start(jargs, methodID);
    if((mb = lookupVirtualMethod(ob, methodID)) != NULL)
        executeMethodVaList(ob, ob->class, mb, jargs);
    va_end(jargs);
}

void Jam_CallVoidMethodV(JNIEnv *env, jobject obj, jmethodID methodID,
                         va_list jargs) {
    MethodBlock *mb;
    Object *ob = REF_TO_OBJ(obj);

    if((mb = lookupVirtualMethod(ob, methodID)) != NULL)
        executeMethodVaList(ob, ob->class, mb, jargs);
}

void Jam_CallVoidMethodA(JNIEnv *env, jobject obj, jmethodID methodID,
                         jvalue *jargs) {
    MethodBlock *mb;
    Object *ob = REF_TO_OBJ(obj);

    if((mb = lookupVirtualMethod(ob, methodID)) != NULL)
        executeMethodList(ob, ob->class, mb, (u8*)jargs);
}

void Jam_CallNonvirtualVoidMethod(JNIEnv *env, jobject obj, jclass clazz,
                                  jmethodID methodID, ...) {
    va_list jargs;

    va_start(jargs, methodID);
    executeMethodVaList(REF_TO_OBJ(obj), REF_TO_OBJ(clazz), methodID, jargs);
    va_end(jargs);
}

void Jam_CallNonvirtualVoidMethodV(JNIEnv *env, jobject obj, jclass clazz,
                jmethodID methodID, va_list jargs) {

      executeMethodVaList(REF_TO_OBJ(obj), REF_TO_OBJ(clazz), methodID, jargs);
}

void Jam_CallNonvirtualVoidMethodA(JNIEnv *env, jobject obj, jclass clazz,
                jmethodID methodID, jvalue *jargs) {

    executeMethodList(REF_TO_OBJ(obj), REF_TO_OBJ(clazz), methodID, (u8*)jargs);
}

void Jam_CallStaticVoidMethod(JNIEnv *env, jclass clazz, jmethodID methodID,
                              ...) {
    va_list jargs;

    va_start(jargs, methodID);
    executeMethodVaList(NULL, REF_TO_OBJ(clazz), methodID, jargs);
    va_end(jargs);
}

void Jam_CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID,
                               va_list jargs) {

    executeMethodVaList(NULL, REF_TO_OBJ(clazz), methodID, jargs);
}

void Jam_CallStaticVoidMethodA(JNIEnv *env, jclass clazz, jmethodID methodID,
                               jvalue *jargs) {

    executeMethodList(NULL, REF_TO_OBJ(clazz), methodID, (u8*)jargs);
}

#define NEW_PRIM_ARRAY(type, native_type, array_type)                        \
native_type##Array Jam_New##type##Array(JNIEnv *env, jsize length) {         \
    return (native_type##Array)                                              \
               addJNILref(allocTypeArray(array_type, length));               \
}

#define GET_PRIM_ARRAY_ELEMENTS(type, native_type)                           \
native_type *Jam_Get##type##ArrayElements(JNIEnv *env,                       \
                                          native_type##Array array_ref,      \
                                          jboolean *isCopy) {                \
    Object *array = REF_TO_OBJ(array_ref);                                   \
    if(isCopy != NULL)                                                       \
        *isCopy = JNI_FALSE;                                                 \
    addJNIGref(array, GLOBAL_REF);                                           \
    return ARRAY_DATA(array, native_type);                                   \
}

#define RELEASE_PRIM_ARRAY_ELEMENTS(type, native_type)                       \
void Jam_Release##type##ArrayElements(JNIEnv *env, native_type##Array array, \
                                      native_type *elems, jint mode) {       \
    delJNIGref(REF_TO_OBJ(array), GLOBAL_REF);                               \
}

#define GET_PRIM_ARRAY_REGION(type, native_type)                             \
void Jam_Get##type##ArrayRegion(JNIEnv *env, native_type##Array array,       \
                                jsize start, jsize len, native_type *buf) {  \
    memcpy(buf, ARRAY_DATA(REF_TO_OBJ(array), native_type) + start,          \
           len * sizeof(native_type));                                       \
}

#define SET_PRIM_ARRAY_REGION(type, native_type)                             \
void Jam_Set##type##ArrayRegion(JNIEnv *env, native_type##Array array,       \
                                jsize start, jsize len, native_type *buf) {  \
    memcpy(ARRAY_DATA(REF_TO_OBJ(array), native_type) + start, buf,          \
           len * sizeof(native_type));                                       \
}

#define PRIM_ARRAY_OP(type, native_type, array_type) \
    NEW_PRIM_ARRAY(type, native_type, array_type);   \
    GET_PRIM_ARRAY_ELEMENTS(type, native_type);      \
    RELEASE_PRIM_ARRAY_ELEMENTS(type, native_type);  \
    GET_PRIM_ARRAY_REGION(type, native_type);        \
    SET_PRIM_ARRAY_REGION(type, native_type);

PRIM_ARRAY_OP(Boolean, jboolean, T_BOOLEAN);
PRIM_ARRAY_OP(Byte, jbyte, T_BYTE);
PRIM_ARRAY_OP(Char, jchar, T_CHAR);
PRIM_ARRAY_OP(Short, jshort, T_SHORT);
PRIM_ARRAY_OP(Int, jint, T_INT);
PRIM_ARRAY_OP(Long, jlong, T_LONG);
PRIM_ARRAY_OP(Float, jfloat, T_FLOAT);
PRIM_ARRAY_OP(Double, jdouble, T_DOUBLE);

#define METHOD(type, ret_type)                \
    Jam_Call##type##ret_type##Method,         \
    Jam_Call##type##ret_type##MethodV,        \
    Jam_Call##type##ret_type##MethodA

#define METHODS(type)                         \
    METHOD(type, Object),                     \
    METHOD(type, Boolean),                    \
    METHOD(type, Byte),                       \
    METHOD(type, Char),                       \
    METHOD(type, Short),                      \
    METHOD(type, Int),                        \
    METHOD(type, Long),                       \
    METHOD(type, Float),                      \
    METHOD(type, Double),                     \
    METHOD(type, Void)

#define FIELD(direction, type, field_type)    \
    Jam_##direction##type##field_type##Field

#define FIELDS2(direction, type)              \
        FIELD(direction, type,  Object),      \
        FIELD(direction, type, Boolean),      \
        FIELD(direction, type,  Byte),        \
        FIELD(direction, type, Char),         \
        FIELD(direction, type, Short),        \
        FIELD(direction, type, Int),          \
        FIELD(direction, type, Long),         \
        FIELD(direction, type, Float),        \
        FIELD(direction, type, Double)

#define FIELDS(type)                          \
        FIELDS2(Get, type),                   \
        FIELDS2(Set, type)

#define ARRAY(op, el_type, type)              \
        Jam_##op##el_type##Array##type

#define ARRAY_OPS(op, type)                   \
        ARRAY(op, Boolean, type),             \
        ARRAY(op, Byte, type),                \
        ARRAY(op, Char, type),                \
        ARRAY(op, Short, type),               \
        ARRAY(op, Int, type),                 \
        ARRAY(op, Long, type),                \
        ARRAY(op, Float, type),               \
        ARRAY(op, Double, type)

struct _JNINativeInterface Jam_JNINativeInterface = {
    NULL,
    NULL,
    NULL,
    NULL,
    Jam_GetVersion,
    Jam_DefineClass,
    Jam_FindClass,
    Jam_FromReflectedMethod,
    Jam_FromReflectedField,
    Jam_ToReflectedMethod,
    Jam_GetSuperClass,
    Jam_IsAssignableFrom,
    Jam_ToReflectedField,
    Jam_Throw,
    Jam_ThrowNew,
    Jam_ExceptionOccurred,
    Jam_ExceptionDescribe,
    Jam_ExceptionClear,
    Jam_FatalError,
    Jam_PushLocalFrame,
    Jam_PopLocalFrame,
    Jam_NewGlobalRef,
    Jam_DeleteGlobalRef,
    Jam_DeleteLocalRef,
    Jam_IsSameObject,
    Jam_NewLocalRef,
    Jam_EnsureLocalCapacity,
    Jam_AllocObject,
    Jam_NewObject,
    Jam_NewObjectV,
    Jam_NewObjectA,
    Jam_GetObjectClass,
    Jam_IsInstanceOf,
    Jam_GetMethodID,
    METHODS(/*virtual*/),
    METHODS(Nonvirtual),
    Jam_GetFieldID,
    FIELDS(/*instance*/),
    Jam_GetStaticMethodID,
    METHODS(Static),
    Jam_GetStaticFieldID,
    FIELDS(Static),
    Jam_NewString,
    Jam_GetStringLength,
    Jam_GetStringChars,
    Jam_ReleaseStringChars,
    Jam_NewStringUTF,
    Jam_GetStringUTFLength,
    Jam_GetStringUTFChars,
    Jam_ReleaseStringUTFChars,
    Jam_GetArrayLength,
    ARRAY(New, Object,),
    ARRAY(Get, Object, Element),
    ARRAY(Set, Object, Element),
    ARRAY_OPS(New,),
    ARRAY_OPS(Get, Elements),
    ARRAY_OPS(Release, Elements),
    ARRAY_OPS(Get, Region),
    ARRAY_OPS(Set, Region),
    Jam_RegisterNatives,
    Jam_UnregisterNatives,
    Jam_MonitorEnter,
    Jam_MonitorExit,
    Jam_GetJavaVM,
    Jam_GetStringRegion,
    Jam_GetStringUTFRegion,
    Jam_GetPrimitiveArrayCritical,
    Jam_ReleasePrimitiveArrayCritical,
    Jam_GetStringCritical,
    Jam_ReleaseStringCritical,
    Jam_NewWeakGlobalRef,
    Jam_DeleteWeakGlobalRef,
    Jam_ExceptionCheck,
    Jam_NewDirectByteBuffer,
    Jam_GetDirectBufferAddress,
    Jam_GetDirectBufferCapacity,
    Jam_GetObjectRefType
};

jint Jam_DestroyJavaVM(JavaVM *vm) {
    mainThreadWaitToExitVM();
    classlibVMShutdown();

    return JNI_OK;
}

/* Common definition of env */
void *jni_env = &Jam_JNINativeInterface;

int isSupportedJNIVersion(int version) {
    return version == JNI_VERSION_1_6 ||
           version == JNI_VERSION_1_4 ||
           version == JNI_VERSION_1_2;
}

int isSupportedJNIVersion_1_1(int version) {
    return version == JNI_VERSION_1_1 ||
           isSupportedJNIVersion(version);
} 

static jint attachCurrentThread(void **penv, void *args, int is_daemon) {
    if(threadSelf() == NULL) {
        char *name = NULL;
        Object *group = NULL;

        if(args != NULL) {
            JavaVMAttachArgs *attach_args = (JavaVMAttachArgs*)args;

            if(!isSupportedJNIVersion(attach_args->version))
                return JNI_EVERSION;

            name = attach_args->name;
            group = attach_args->group;
        }

        if(attachJNIThread(name, is_daemon, group) == NULL)
            return JNI_ERR;

        if(!initJNILrefs())
            return JNI_ERR;
    }

    *penv = &jni_env;
    return JNI_OK;
}

jint Jam_AttachCurrentThread(JavaVM *vm, void **penv, void *args) {
    return attachCurrentThread(penv, args, FALSE);
}

jint Jam_AttachCurrentThreadAsDaemon(JavaVM *vm, void **penv, void *args) {
    return attachCurrentThread(penv, args, TRUE);
}

jint Jam_DetachCurrentThread(JavaVM *vm) {
    Thread *thread = threadSelf();

    if(thread == NULL)
        return JNI_EDETACHED;

    detachJNIThread(thread);
    return JNI_OK;
}

jint Jam_GetEnv(JavaVM *vm, void **penv, jint version) {
    if(!isSupportedJNIVersion_1_1(version)) {
        *penv = NULL;
        return JNI_EVERSION;
    }

    if(threadSelf() == NULL) {
        *penv = NULL;
        return JNI_EDETACHED;
    }

    *penv = &jni_env;
    return JNI_OK;
}

struct _JNIInvokeInterface Jam_JNIInvokeInterface = {
    NULL,
    NULL,
    NULL,
    Jam_DestroyJavaVM,
    Jam_AttachCurrentThread,
    Jam_DetachCurrentThread,
    Jam_GetEnv,
    Jam_AttachCurrentThreadAsDaemon,
};

jint JNI_GetDefaultJavaVMInitArgs(void *args) {
    JavaVMInitArgs *vm_args = (JavaVMInitArgs*) args;

    if(!isSupportedJNIVersion(vm_args->version))
        return JNI_EVERSION;

    return JNI_OK;
}

jint parseInitOptions(JavaVMInitArgs *vm_args, InitArgs *args) {
    Property props[vm_args->nOptions];
    int i;

    args->commandline_props = &props[0];

    for(i = 0; i < vm_args->nOptions; i++) {
        char *string = vm_args->options[i].optionString;

        switch(parseCommonOpts(string, args, TRUE)) {
            case OPT_OK:
                break;

            case OPT_ERROR:
                goto error;

            case OPT_UNREC:
            default:
                if(strcmp(string, "vfprintf") == 0)
                    args->vfprintf = vm_args->options[i].extraInfo;

                else if(strcmp(string, "exit") == 0)
                    args->exit = vm_args->options[i].extraInfo;

                else if(strcmp(string, "abort") == 0)
                    args->abort = vm_args->options[i].extraInfo;

                else if(strcmp(string, "-verbose") == 0)
                    args->verboseclass = TRUE;

                else if(strncmp(string, "-verbose:", 9) == 0) {
                    char *type = &string[8];

                    do {
                        type++;

                        if(strncmp(type, "class", 5) == 0) {
                            args->verboseclass = TRUE;
                            type += 5;
                         }
                        else if(strncmp(type, "gc", 2) == 0) {
                            args->verbosegc = TRUE;
                            type += 2;
                        }
                        else if(strncmp(type, "jni", 3) == 0) {
                            args->verbosedll = TRUE;
                            type += 3;
                        }
                    } while(*type == ',');

                } else if(!vm_args->ignoreUnrecognized) {
                    optError(args, "Unrecognised option: %s\n", string);
                    goto error;
                }
        }
    }

    if(args->min_heap > args->max_heap) {
        optError(args, "Minimum heap size greater than max!\n");
        goto error;
    }

    if(args->props_count) {
        args->commandline_props = sysMalloc(args->props_count *
                                            sizeof(Property));
        memcpy(args->commandline_props, &props[0], args->props_count *
                                                   sizeof(Property));
    }

    return JNI_OK;

error:
    return JNI_ERR;
}

jint JNI_CreateJavaVM(JavaVM **pvm, void **penv, void *args) {
    JavaVMInitArgs *vm_args = (JavaVMInitArgs*) args;
    InitArgs init_args;

    if(!isSupportedJNIVersion(vm_args->version))
        return JNI_EVERSION;

    setDefaultInitArgs(&init_args);

    if(parseInitOptions(vm_args, &init_args) == JNI_ERR)
        return JNI_ERR;

    init_args.main_stack_base = nativeStackBase();

    if(!initVM(&init_args))
        return JNI_ERR;

    if(!initJNILrefs())
        return JNI_ERR;

    *penv = &jni_env;
    *pvm = &jni_invoke_intf;

    return JNI_OK;
}

jint JNI_GetCreatedJavaVMs(JavaVM **buff, jsize buff_len, jsize *num) {
    if(buff_len > 0) {
        *buff = &jni_invoke_intf;
        *num = 1;
        return JNI_OK;
    }
    return JNI_ERR;
}

void *getJNIInterface() {
    return &Jam_JNINativeInterface;
}
#endif

