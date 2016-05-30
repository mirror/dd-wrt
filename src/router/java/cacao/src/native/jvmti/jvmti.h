/* src/native/jvmti.c - implementation of the Java Virtual Machine Tool 
                        Interface functions

   Copyright (C) 1996-2005, 2006, 2008
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#ifndef _JVMTI_H
#define _JVMTI_H

#include "config.h"

#include "native/jni.hpp"


#define JVMTI_VERSION_INTERFACE_JNI   0x00000000
#define JVMTI_VERSION_INTERFACE_JVMTI 0x30000000
#define JVMTI_VERSION_MASK_INTERFACE_TYPE 0x70000000
#define JVMTI_VERSION_1_0 0x30010000
#define JVMTI_VERSION     JVMTI_VERSION_1_0 


typedef jobject jthread;
typedef jobject jthreadGroup;
typedef jlong jlocation;
struct _jrawMonitorID;
typedef struct _jrawMonitorID *jrawMonitorID;
typedef struct jvmtiEnv_struct *jvmtiEnv;

typedef enum {
    JVMTI_ERROR_NONE = 0, /* No error has occurred. This is the error code that is 
                             returned on successful completion of the function. */
    JVMTI_ERROR_NULL_POINTER = 100, /*  Pointer is unexpectedly NULL. */
    JVMTI_ERROR_OUT_OF_MEMORY = 110, /*  The function attempted to allocate memory 
                                         and no more memory was available for 
                                         allocation. */
    JVMTI_ERROR_ACCESS_DENIED = 111, /*  The desired functionality has not been 
                                         enabled in this virtual machine. */
    JVMTI_ERROR_UNATTACHED_THREAD = 115, /* The thread being used to call this 
                                            function is not attached to the virtual 
                                            machine. Calls must be made from attached threads. 
                                            See AttachCurrentThread in the JNI invocation API. */
    JVMTI_ERROR_INVALID_ENVIRONMENT = 116, /*  
                                               The JVM TI environment provided is no longer connected or is not an environment. */
    JVMTI_ERROR_WRONG_PHASE = 112, /*  
                                       The desired functionality is not available in the current phase. Always returned if the virtual machine has completed running. */
    JVMTI_ERROR_INTERNAL = 113, /*  
                                    An unexpected internal error has occurred. */

/* ****** Function Specific Required Errors ********** */
    JVMTI_ERROR_INVALID_PRIORITY = 12, /*  
                                           Invalid priority. */
    JVMTI_ERROR_THREAD_NOT_SUSPENDED = 13, /*  
                                               Thread was not suspended. */
    JVMTI_ERROR_THREAD_SUSPENDED = 14, /*  
                                           Thread already suspended. */
    JVMTI_ERROR_THREAD_NOT_ALIVE = 15, /*  
                                           This operation requires the thread to be alive--that is, it must be started and not yet have died. */
    JVMTI_ERROR_CLASS_NOT_PREPARED = 22, /*  
                                             The class has been loaded but not yet prepared. */
    JVMTI_ERROR_NO_MORE_FRAMES = 31, /*  
                                         There are no Java programming language or JNI stack frames at the specified depth. */
    JVMTI_ERROR_OPAQUE_FRAME = 32, /*  
                                       Information about the frame is not available = e.g. for native frames, */
    JVMTI_ERROR_DUPLICATE = 40, /*  
                                    Item already set. */
    JVMTI_ERROR_NOT_FOUND = 41, /*  
                                    Desired element = e.g. field or breakpoint, not found */
    JVMTI_ERROR_NOT_MONITOR_OWNER = 51, /*  
                                            This thread doesn't own the raw monitor. */
    JVMTI_ERROR_INTERRUPT = 52, /*  
                                    The call has been interrupted before completion. */
    JVMTI_ERROR_UNMODIFIABLE_CLASS = 79, /*  
                                             The class cannot be modified. */
    JVMTI_ERROR_NOT_AVAILABLE = 98, /*  
                                        The functionality is not available in this virtual machine. */
    JVMTI_ERROR_ABSENT_INFORMATION = 101, /*  
                                              The requested information is not available. */
    JVMTI_ERROR_INVALID_EVENT_TYPE = 102, /*  
                                              The specified event type ID is not recognized. */
    JVMTI_ERROR_NATIVE_METHOD = 104, /*  
                                         The requested information is not available for native method. 
                                         Function Specific Agent Errors
                                         The following errors are returned by some JVM TI functions. They are returned in the event of invalid parameters passed by the agent or usage in an invalid context. An implementation is not required to detect these errors. */
    JVMTI_ERROR_INVALID_THREAD = 10, /*  
                                         The passed thread is not a valid thread.*/ 
    JVMTI_ERROR_INVALID_FIELDID = 25, /*  
                                          Invalid field. */
    JVMTI_ERROR_INVALID_METHODID = 23, /*  
                                           Invalid method. */
    JVMTI_ERROR_INVALID_LOCATION = 24, /*  
                                           Invalid location. */
    JVMTI_ERROR_INVALID_OBJECT = 20, /*  
                                         Invalid object. */
    JVMTI_ERROR_INVALID_CLASS = 21, /*  
                                        Invalid class. */
    JVMTI_ERROR_TYPE_MISMATCH = 34, /*  
                                        The variable is not an appropriate type for the function used. */
    JVMTI_ERROR_INVALID_SLOT = 35, /*  
                                       Invalid slot. */
    JVMTI_ERROR_MUST_POSSESS_CAPABILITY = 99, /*  
                                                  The capability being used is false in this environment. */
    JVMTI_ERROR_INVALID_THREAD_GROUP = 11, /*  
                                               Thread group invalid. */
    JVMTI_ERROR_INVALID_MONITOR = 50, /*  
                                          Invalid raw monitor. */
    JVMTI_ERROR_ILLEGAL_ARGUMENT = 103, /*  
                                            Illegal argument. */
    JVMTI_ERROR_INVALID_TYPESTATE = 65, /*  
                                            The state of the thread has been modified, and is now inconsistent. */
    JVMTI_ERROR_UNSUPPORTED_VERSION = 68, /*  
                                              A new class file has a version number not supported by this VM. */
    JVMTI_ERROR_INVALID_CLASS_FORMAT = 60, /*  
                                               A new class file is malformed = the VM would return a ClassFormatError, */
    JVMTI_ERROR_CIRCULAR_CLASS_DEFINITION = 61, /*  
                                                    The new class file definitions would lead to a circular definition = the VM would return a ClassCircularityError, */ 
    JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_ADDED = 63, /*  
                                                                A new class file would require adding a method. */
    JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED = 64, /*  
                                                                  A new class version changes a field. */
    JVMTI_ERROR_FAILS_VERIFICATION = 62, /*  
                                             The class bytes fail verification. */
    JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED = 66, /*  
                                                                     A direct superclass is different for the new class version, or the set of directly implemented interfaces is different. */
    JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED = 67, /*  
                                                                  A new class version does not declare a method declared in the old class version. */
    JVMTI_ERROR_NAMES_DONT_MATCH = 69, /*  
                                           The class name defined in the new class file is different from the name in the old class object. */
    JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_MODIFIERS_CHANGED = 70, /*  
                                                                           A new class version has different modifiers. */
    JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_MODIFIERS_CHANGED = 71 /*  
                                                                           A method in the new class version has different modifiers than its counterpart in the old class version.*/

} jvmtiError;


typedef enum {
    JVMTI_PHASE_ONLOAD = 1,
    JVMTI_PHASE_PRIMORDIAL = 2,
    JVMTI_PHASE_START = 6,
    JVMTI_PHASE_LIVE = 4,
    JVMTI_PHASE_DEAD = 8
} jvmtiPhase;


typedef enum {
    JVMTI_JLOCATION_JVMBCI = 1,
    JVMTI_JLOCATION_MACHINEPC = 2,
    JVMTI_JLOCATION_OTHER = 0
} jvmtiJlocationFormat;

typedef struct {
    jlocation start_location;
    jint line_number;
} jvmtiLineNumberEntry;

typedef struct {
    char* name;
    jint priority;
    jboolean is_daemon;
    jthreadGroup thread_group;
    jobject context_class_loader;
} jvmtiThreadInfo;

typedef enum {
    JVMTI_VERBOSE_OTHER = 0,
    JVMTI_VERBOSE_GC = 1,
    JVMTI_VERBOSE_CLASS = 2,
    JVMTI_VERBOSE_JNI = 4
} jvmtiVerboseFlag;

typedef struct {
    jclass klass;
    jint class_byte_count;
    const unsigned char* class_bytes;
} jvmtiClassDefinition;

typedef struct JNINativeInterface jniNativeInterface;

typedef struct {
    const void* start_address;
    jlocation location;
} jvmtiAddrLocationMap;


typedef void (JNICALL *jvmtiEventSingleStep) 
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method,
     jlocation location);

typedef void (JNICALL *jvmtiEventBreakpoint)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method,
     jlocation location);

typedef void (JNICALL *jvmtiEventFieldAccess)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method,
     jlocation location,
     jclass field_klass,
     jobject object,
     jfieldID field);

typedef void (JNICALL *jvmtiEventFieldModification)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method,
     jlocation location,
     jclass field_klass,
     jobject object,
     jfieldID field,
     char signature_type,
     jvalue new_value);

typedef void (JNICALL *jvmtiEventFramePop)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method,
     jboolean was_popped_by_exception);

typedef void (JNICALL *jvmtiEventMethodEntry)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method);

typedef void (JNICALL *jvmtiEventMethodExit)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method,
     jboolean was_popped_by_exception,
     jvalue return_value);

typedef void (JNICALL *jvmtiEventNativeMethodBind)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method,
     void* address,
     void** new_address_ptr);

typedef void (JNICALL *jvmtiEventException)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method,
     jlocation location,
     jobject exception,
     jmethodID catch_method,
     jlocation catch_location);

typedef void (JNICALL *jvmtiEventExceptionCatch)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jmethodID method,
     jlocation location,
     jobject exception);

typedef void (JNICALL *jvmtiEventThreadStart)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread);

typedef void (JNICALL *jvmtiEventThreadEnd)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread);

typedef void (JNICALL *jvmtiEventClassLoad)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jclass klass);

typedef void (JNICALL *jvmtiEventClassPrepare)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jclass klass);

typedef void (JNICALL *jvmtiEventClassFileLoadHook)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jclass class_being_redefined,
     jobject loader,
     const char* name,
     jobject protection_domain,
     jint class_data_len,
     const unsigned char* class_data,
     jint* new_class_data_len,
     unsigned char** new_class_data);

typedef void (JNICALL *jvmtiEventVMStart)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env);

typedef void (JNICALL *jvmtiEventVMInit)
    (jvmtiEnv *jvmti_env, 
     JNIEnv* jni_env,
     jthread thread);

typedef void (JNICALL *jvmtiEventVMDeath)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env);


typedef void (JNICALL *jvmtiEventCompiledMethodLoad)
    (jvmtiEnv *jvmti_env,
     jmethodID method,
     jint code_size,
     const void* code_addr,
     jint map_length,
     const jvmtiAddrLocationMap* map,
     const void* compile_info);

typedef void (JNICALL *jvmtiEventCompiledMethodUnload)
    (jvmtiEnv *jvmti_env,
     jmethodID method,
     const void* code_addr);

typedef void (JNICALL *jvmtiEventDynamicCodeGenerated)
    (jvmtiEnv *jvmti_env,
     const char* name,
     const void* address,
     jint length);

typedef void (JNICALL *jvmtiEventDataDumpRequest)
    (jvmtiEnv *jvmti_env);

typedef void (JNICALL *jvmtiEventMonitorContendedEnter)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jobject object);

typedef void (JNICALL *jvmtiEventMonitorContendedEntered)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jobject object);

typedef void (JNICALL *jvmtiEventMonitorWait)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jobject object,
     jlong timeout);

typedef void (JNICALL *jvmtiEventMonitorWaited)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jobject object,
     jboolean timed_out);

typedef void (JNICALL *jvmtiEventVMObjectAlloc)
    (jvmtiEnv *jvmti_env,
     JNIEnv* jni_env,
     jthread thread,
     jobject object,
     jclass object_klass,
     jlong size);

typedef void (JNICALL *jvmtiEventObjectFree)
    (jvmtiEnv *jvmti_env,
     jlong tag);

typedef void (JNICALL *jvmtiEventGarbageCollectionStart)
    (jvmtiEnv *jvmti_env);

typedef void (JNICALL *jvmtiEventGarbageCollectionFinish)
    (jvmtiEnv *jvmti_env);

typedef void (JNICALL *jvmtiStartFunction)
    (jvmtiEnv* jvmti_env, 
     JNIEnv* jni_env, 
     void* arg);

typedef struct {
    jthreadGroup parent;
    char* name;
    jint max_priority;
    jboolean is_daemon;
} jvmtiThreadGroupInfo;

typedef struct {
    jthread owner;
    jint entry_count;
    jint waiter_count;
    jthread* waiters;
    jint notify_waiter_count;
    jthread* notify_waiters;
} jvmtiMonitorUsage;

typedef struct {
    jlocation start_location;
    jint length;
    char* name;
    char* signature;
    char* generic_signature;
    jint slot;
} jvmtiLocalVariableEntry;

typedef struct {
    jmethodID method;
    jlocation location;
} jvmtiFrameInfo;

typedef struct {
    jthread thread;
    jint state;
    jvmtiFrameInfo* frame_buffer;
    jint frame_count;
} jvmtiStackInfo;

typedef enum {
    JVMTI_HEAP_OBJECT_TAGGED = 1,
    JVMTI_HEAP_OBJECT_UNTAGGED = 2,
    JVMTI_HEAP_OBJECT_EITHER = 3
} jvmtiHeapObjectFilter;

typedef enum {
    JVMTI_REFERENCE_CLASS = 1,
    JVMTI_REFERENCE_FIELD = 2,
    JVMTI_REFERENCE_ARRAY_ELEMENT = 3,
    JVMTI_REFERENCE_CLASS_LOADER = 4,
    JVMTI_REFERENCE_SIGNERS = 5,
    JVMTI_REFERENCE_PROTECTION_DOMAIN = 6,
    JVMTI_REFERENCE_INTERFACE = 7,
    JVMTI_REFERENCE_STATIC_FIELD = 8,
    JVMTI_REFERENCE_CONSTANT_POOL = 9
} jvmtiObjectReferenceKind;

typedef enum {
    JVMTI_ITERATION_CONTINUE = 1,
    JVMTI_ITERATION_IGNORE = 2,
    JVMTI_ITERATION_ABORT = 0
} jvmtiIterationControl;

typedef enum {
    JVMTI_HEAP_ROOT_JNI_GLOBAL = 1,
    JVMTI_HEAP_ROOT_SYSTEM_CLASS = 2,
    JVMTI_HEAP_ROOT_MONITOR = 3,
    JVMTI_HEAP_ROOT_STACK_LOCAL = 4,
    JVMTI_HEAP_ROOT_JNI_LOCAL = 5,
    JVMTI_HEAP_ROOT_THREAD = 6,
    JVMTI_HEAP_ROOT_OTHER = 7
} jvmtiHeapRootKind;

typedef jvmtiIterationControl (JNICALL *jvmtiObjectReferenceCallback)
    (jvmtiObjectReferenceKind reference_kind, 
     jlong class_tag, 
     jlong size, 
     jlong* tag_ptr, 
     jlong referrer_tag, 
     jint referrer_index, 
     void* user_data);

typedef jvmtiIterationControl (JNICALL *jvmtiHeapRootCallback)
    (jvmtiHeapRootKind root_kind, 
     jlong class_tag, 
     jlong size, 
     jlong* tag_ptr, 
     void* user_data);

typedef jvmtiIterationControl (JNICALL *jvmtiStackReferenceCallback)
    (jvmtiHeapRootKind root_kind, 
     jlong class_tag, 
     jlong size, 
     jlong* tag_ptr, 
     jlong thread_tag, 
     jint depth, 
     jmethodID method, 
     jint slot, 
     void* user_data);

typedef jvmtiIterationControl (JNICALL *jvmtiHeapObjectCallback)
    (jlong class_tag, 
     jlong size, 
     jlong* tag_ptr, 
     void* user_data);

typedef jvmtiError (JNICALL *jvmtiExtensionFunction)
    (jvmtiEnv* jvmti_env, 
      ...);

typedef enum {
    JVMTI_KIND_IN = 91,
    JVMTI_KIND_IN_PTR = 92,
    JVMTI_KIND_IN_BUF = 93,
    JVMTI_KIND_ALLOC_BUF = 94,
    JVMTI_KIND_ALLOC_ALLOC_BUF = 95,
    JVMTI_KIND_OUT = 96,
    JVMTI_KIND_OUT_BUF = 97
} jvmtiParamKind;

typedef enum {
    JVMTI_TYPE_JBYTE = 101,
    JVMTI_TYPE_JCHAR = 102,
    JVMTI_TYPE_JSHORT = 103,
    JVMTI_TYPE_JINT = 104,
    JVMTI_TYPE_JLONG = 105,
    JVMTI_TYPE_JFLOAT = 106,
    JVMTI_TYPE_JDOUBLE = 107,
    JVMTI_TYPE_JBOOLEAN = 108,
    JVMTI_TYPE_JOBJECT = 109,
    JVMTI_TYPE_JTHREAD = 110,
    JVMTI_TYPE_JCLASS = 111,
    JVMTI_TYPE_JVALUE = 112,
    JVMTI_TYPE_JFIELDID = 113,
    JVMTI_TYPE_JMETHODID = 114,
    JVMTI_TYPE_CCHAR = 115,   
    JVMTI_TYPE_CVOID = 116,
    JVMTI_TYPE_JNIENV = 117
} jvmtiParamTypes;

typedef struct {
    char* name;
    jvmtiParamKind kind;
    jvmtiParamTypes base_type;
    jboolean null_ok;
} jvmtiParamInfo;

typedef struct {
    jint extension_event_index;
    char* id;
    char* short_description;
    jint param_count;
    jvmtiParamInfo* params;
} jvmtiExtensionEventInfo;

typedef void (JNICALL *jvmtiExtensionEvent)
    (jvmtiEnv* jvmti_env, 
      ...);

typedef enum {
    JVMTI_TIMER_USER_CPU = 30,
    JVMTI_TIMER_TOTAL_CPU = 31,
    JVMTI_TIMER_ELAPSED = 32
} jvmtiTimerKind;

typedef struct {
    jlong max_value;
    jboolean may_skip_forward;
    jboolean may_skip_backward;
    jvmtiTimerKind kind;
    jlong reserved1;
    jlong reserved2;
} jvmtiTimerInfo;

typedef struct {
    jvmtiExtensionFunction func;
    char* id;
    char* short_description;
    jint param_count;
    jvmtiParamInfo* params;
    jint error_count;
    jvmtiError* errors;
} jvmtiExtensionFunctionInfo;

typedef void* jvmtiEventReserved;

typedef struct {
    jvmtiEventVMInit VMInit;
    jvmtiEventVMDeath VMDeath;
    jvmtiEventThreadStart ThreadStart;
    jvmtiEventThreadEnd ThreadEnd;
    jvmtiEventClassFileLoadHook ClassFileLoadHook;
    jvmtiEventClassLoad ClassLoad;
    jvmtiEventClassPrepare ClassPrepare;
    jvmtiEventVMStart VMStart;
    jvmtiEventException Exception;
    jvmtiEventExceptionCatch ExceptionCatch;
    jvmtiEventSingleStep SingleStep;
    jvmtiEventFramePop FramePop;
    jvmtiEventBreakpoint Breakpoint;
    jvmtiEventFieldAccess FieldAccess;
    jvmtiEventFieldModification FieldModification;
    jvmtiEventMethodEntry MethodEntry;
    jvmtiEventMethodExit MethodExit;
    jvmtiEventNativeMethodBind NativeMethodBind;
    jvmtiEventCompiledMethodLoad CompiledMethodLoad;
    jvmtiEventCompiledMethodUnload CompiledMethodUnload;
    jvmtiEventDynamicCodeGenerated DynamicCodeGenerated;
    jvmtiEventDataDumpRequest DataDumpRequest;
    jvmtiEventReserved reserved72;
    jvmtiEventMonitorWait MonitorWait;
    jvmtiEventMonitorWaited MonitorWaited;
    jvmtiEventMonitorContendedEnter MonitorContendedEnter;
    jvmtiEventMonitorContendedEntered MonitorContendedEntered;
    jvmtiEventReserved reserved77;
    jvmtiEventReserved reserved78;
    jvmtiEventReserved reserved79;
    jvmtiEventReserved reserved80;
    jvmtiEventGarbageCollectionStart GarbageCollectionStart;
    jvmtiEventGarbageCollectionFinish GarbageCollectionFinish;
    jvmtiEventObjectFree ObjectFree;
    jvmtiEventVMObjectAlloc VMObjectAlloc;
} jvmtiEventCallbacks;

typedef enum {
    JVMTI_ENABLE = 1,
    JVMTI_DISABLE = 0
} jvmtiEventMode;

typedef enum {
    JVMTI_EVENT_START_ENUM = 50,
    JVMTI_EVENT_VM_INIT = 50,
    JVMTI_EVENT_VM_DEATH = 51,
    JVMTI_EVENT_THREAD_START = 52,
    JVMTI_EVENT_THREAD_END = 53,
    JVMTI_EVENT_CLASS_FILE_LOAD_HOOK = 54,
    JVMTI_EVENT_CLASS_LOAD = 55,
    JVMTI_EVENT_CLASS_PREPARE = 56,
    JVMTI_EVENT_VM_START = 57,
    JVMTI_EVENT_EXCEPTION = 58,
    JVMTI_EVENT_EXCEPTION_CATCH = 59,
    JVMTI_EVENT_SINGLE_STEP = 60,
    JVMTI_EVENT_FRAME_POP = 61,
    JVMTI_EVENT_BREAKPOINT = 62,
    JVMTI_EVENT_FIELD_ACCESS = 63,
    JVMTI_EVENT_FIELD_MODIFICATION = 64,
    JVMTI_EVENT_METHOD_ENTRY = 65,
    JVMTI_EVENT_METHOD_EXIT = 66,
    JVMTI_EVENT_NATIVE_METHOD_BIND = 67,
    JVMTI_EVENT_COMPILED_METHOD_LOAD = 68,
    JVMTI_EVENT_COMPILED_METHOD_UNLOAD = 69,
    JVMTI_EVENT_DYNAMIC_CODE_GENERATED = 70,
    JVMTI_EVENT_DATA_DUMP_REQUEST = 71,
    JVMTI_EVENT_MONITOR_WAIT = 73,
    JVMTI_EVENT_MONITOR_WAITED = 74,
    JVMTI_EVENT_MONITOR_CONTENDED_ENTER = 75,
    JVMTI_EVENT_MONITOR_CONTENDED_ENTERED = 76,
    JVMTI_EVENT_GARBAGE_COLLECTION_START = 81,
    JVMTI_EVENT_GARBAGE_COLLECTION_FINISH = 82,
    JVMTI_EVENT_OBJECT_FREE = 83,
    JVMTI_EVENT_VM_OBJECT_ALLOC = 84,
    JVMTI_EVENT_END_ENUM = 84
} jvmtiEvent;


typedef struct {
    unsigned int can_tag_objects : 1;
    unsigned int can_generate_field_modification_events : 1;
    unsigned int can_generate_field_access_events : 1;
    unsigned int can_get_bytecodes : 1;
    unsigned int can_get_synthetic_attribute : 1;
    unsigned int can_get_owned_monitor_info : 1;
    unsigned int can_get_current_contended_monitor : 1;
    unsigned int can_get_monitor_info : 1;
    unsigned int can_pop_frame : 1;
    unsigned int can_redefine_classes : 1;
    unsigned int can_signal_thread : 1;
    unsigned int can_get_source_file_name : 1;
    unsigned int can_get_line_numbers : 1;
    unsigned int can_get_source_debug_extension : 1;
    unsigned int can_access_local_variables : 1;
    unsigned int can_maintain_original_method_order : 1;
    unsigned int can_generate_single_step_events : 1;
    unsigned int can_generate_exception_events : 1;
    unsigned int can_generate_frame_pop_events : 1;
    unsigned int can_generate_breakpoint_events : 1;
    unsigned int can_suspend : 1;
    unsigned int can_redefine_any_class : 1;
    unsigned int can_get_current_thread_cpu_time : 1;
    unsigned int can_get_thread_cpu_time : 1;
    unsigned int can_generate_method_entry_events : 1;
    unsigned int can_generate_method_exit_events : 1;
    unsigned int can_generate_all_class_hook_events : 1;
    unsigned int can_generate_compiled_method_load_events : 1;
    unsigned int can_generate_monitor_events : 1;
    unsigned int can_generate_vm_object_alloc_events : 1;
    unsigned int can_generate_native_method_bind_events : 1;
    unsigned int can_generate_garbage_collection_events : 1;
    unsigned int can_generate_object_free_events : 1;
    unsigned int : 15;
    unsigned int : 16;
    unsigned int : 16;
    unsigned int : 16;
    unsigned int : 16;
    unsigned int : 16;
} jvmtiCapabilities;





/* Function Interface */
struct jvmtiEnv_struct {    
    void *reserved1;
    jvmtiError (JNICALL *SetEventNotificationMode) (jvmtiEnv* env, 
                                                    jvmtiEventMode mode, 
                                                    jvmtiEvent event_type, 
                                                    jthread event_thread, 
                                                    ...);
    void *reserved3;
    jvmtiError (JNICALL *GetAllThreads) (jvmtiEnv* env, 
                                         jint* threads_count_ptr, 
                                         jthread** threads_ptr);
    jvmtiError (JNICALL *SuspendThread) (jvmtiEnv* env, 
                                         jthread thread);
    jvmtiError (JNICALL *ResumeThread) (jvmtiEnv* env, 
                                        jthread thread);
    jvmtiError (JNICALL *StopThread) (jvmtiEnv* env, 
                                      jthread thread, 
                                      jobject exception);
    jvmtiError (JNICALL *InterruptThread) (jvmtiEnv* env, 
                                           jthread thread);
    jvmtiError (JNICALL *GetThreadInfo) (jvmtiEnv* env, 
                                         jthread thread, 
                                         jvmtiThreadInfo* info_ptr);
    jvmtiError (JNICALL *GetOwnedMonitorInfo) (jvmtiEnv* env, 
                                               jthread thread, 
                                               jint* owned_monitor_count_ptr, 
                                               jobject** owned_monitors_ptr);
    jvmtiError (JNICALL *GetCurrentContendedMonitor) (jvmtiEnv* env, 
                                                      jthread thread, 
                                                      jobject* monitor_ptr);
    jvmtiError (JNICALL *RunAgentThread) (jvmtiEnv* env, 
                                          jthread thread, 
                                          jvmtiStartFunction proc, 
                                          const void* arg, 
                                          jint priority);
    jvmtiError (JNICALL *GetTopThreadGroups) (jvmtiEnv* env, 
                                              jint* group_count_ptr, 
                                              jthreadGroup** groups_ptr);
    jvmtiError (JNICALL *GetThreadGroupInfo) (jvmtiEnv* env, 
                                              jthreadGroup group, 
                                              jvmtiThreadGroupInfo* info_ptr);
    jvmtiError (JNICALL *GetThreadGroupChildren) (jvmtiEnv* env, 
                                                  jthreadGroup group, 
                                                  jint* thread_count_ptr, 
                                                  jthread** threads_ptr, 
                                                  jint* group_count_ptr, 
                                                  jthreadGroup** groups_ptr);
    jvmtiError (JNICALL *GetFrameCount) (jvmtiEnv* env, 
                                         jthread thread, 
                                         jint* count_ptr);
    jvmtiError (JNICALL *GetThreadState) (jvmtiEnv* env, 
                                          jthread thread, 
                                          jint* thread_state_ptr);
    void *reserved18;
    jvmtiError (JNICALL *GetFrameLocation) (jvmtiEnv* env, 
                                            jthread thread, 
                                            jint depth, 
                                            jmethodID* method_ptr, 
                                            jlocation* location_ptr);
    jvmtiError (JNICALL *NotifyFramePop) (jvmtiEnv* env, 
                                          jthread thread, 
                                          jint depth);
    jvmtiError (JNICALL *GetLocalObject) (jvmtiEnv* env, 
                                          jthread thread, 
                                          jint depth, 
                                          jint slot, 
                                          jobject* value_ptr);
    jvmtiError (JNICALL *GetLocalInt) (jvmtiEnv* env, 
                                       jthread thread, 
                                       jint depth, 
                                       jint slot, 
                                       jint* value_ptr);
    jvmtiError (JNICALL *GetLocalLong) (jvmtiEnv* env, 
                                        jthread thread, 
                                        jint depth, 
                                        jint slot, 
                                        jlong* value_ptr);
    jvmtiError (JNICALL *GetLocalFloat) (jvmtiEnv* env, 
                                         jthread thread, 
                                         jint depth, 
                                         jint slot, 
                                         jfloat* value_ptr);
    jvmtiError (JNICALL *GetLocalDouble) (jvmtiEnv* env, 
                                          jthread thread, 
                                          jint depth, 
                                          jint slot, 
                                          jdouble* value_ptr);
    jvmtiError (JNICALL *SetLocalObject) (jvmtiEnv* env, 
                                          jthread thread, 
                                          jint depth, 
                                          jint slot, 
                                          jobject value);
    jvmtiError (JNICALL *SetLocalInt) (jvmtiEnv* env, 
                                       jthread thread, 
                                       jint depth, 
                                       jint slot, 
                                       jint value);
    jvmtiError (JNICALL *SetLocalLong) (jvmtiEnv* env, 
                                        jthread thread, 
                                        jint depth, 
                                        jint slot, 
                                        jlong value);
    jvmtiError (JNICALL *SetLocalFloat) (jvmtiEnv* env, 
                                         jthread thread, 
                                         jint depth, 
                                         jint slot, 
                                         jfloat value);
    jvmtiError (JNICALL *SetLocalDouble) (jvmtiEnv* env, 
                                          jthread thread, 
                                          jint depth, 
                                          jint slot, 
                                          jdouble value);
    jvmtiError (JNICALL *CreateRawMonitor) (jvmtiEnv* env, 
                                            const char* name, 
                                            jrawMonitorID* monitor_ptr);
    jvmtiError (JNICALL *DestroyRawMonitor) (jvmtiEnv* env, 
                                             jrawMonitorID monitor);
    jvmtiError (JNICALL *RawMonitorEnter) (jvmtiEnv* env, 
                                           jrawMonitorID monitor);
    jvmtiError (JNICALL *RawMonitorExit) (jvmtiEnv* env, 
                                          jrawMonitorID monitor);
    jvmtiError (JNICALL *RawMonitorWait) (jvmtiEnv* env, 
                                          jrawMonitorID monitor, 
                                          jlong millis);
    jvmtiError (JNICALL *RawMonitorNotify) (jvmtiEnv* env, 
                                            jrawMonitorID monitor);
    jvmtiError (JNICALL *RawMonitorNotifyAll) (jvmtiEnv* env, 
                                               jrawMonitorID monitor);
    jvmtiError (JNICALL *SetBreakpoint) (jvmtiEnv* env, 
                                         jmethodID method, 
                                         jlocation location);
    jvmtiError (JNICALL *ClearBreakpoint) (jvmtiEnv* env, 
                                           jmethodID method, 
                                           jlocation location);
    void *reserved40;
    jvmtiError (JNICALL *SetFieldAccessWatch) (jvmtiEnv* env, 
                                               jclass klass, 
                                               jfieldID field);
    jvmtiError (JNICALL *ClearFieldAccessWatch) (jvmtiEnv* env, 
                                                 jclass klass, 
                                                 jfieldID field);
    jvmtiError (JNICALL *SetFieldModificationWatch) (jvmtiEnv* env, 
                                                     jclass klass, 
                                                     jfieldID field);
    jvmtiError (JNICALL *ClearFieldModificationWatch) (jvmtiEnv* env, 
                                                       jclass klass, 
                                                       jfieldID field);
    void *reserved45;
    jvmtiError (JNICALL *Allocate) (jvmtiEnv* env, 
                                    jlong size, 
                                    unsigned char** mem_ptr);
    jvmtiError (JNICALL *Deallocate) (jvmtiEnv* env, 
                                      unsigned char* mem);
    jvmtiError (JNICALL *GetClassSignature) (jvmtiEnv* env, 
                                             jclass klass, 
                                             char** signature_ptr, 
                                             char** generic_ptr);
    jvmtiError (JNICALL *GetClassStatus) (jvmtiEnv* env, 
                                          jclass klass, 
                                          jint* status_ptr);
    jvmtiError (JNICALL *GetSourceFileName) (jvmtiEnv* env, 
                                             jclass klass, 
                                             char** source_name_ptr);
    jvmtiError (JNICALL *GetClassModifiers) (jvmtiEnv* env, 
                                             jclass klass, 
                                             jint* modifiers_ptr);
    jvmtiError (JNICALL *GetClassMethods) (jvmtiEnv* env, 
                                           jclass klass, 
                                           jint* method_count_ptr, 
                                           jmethodID** methods_ptr);
    jvmtiError (JNICALL *GetClassFields) (jvmtiEnv* env, 
                                          jclass klass, 
                                          jint* field_count_ptr, 
                                          jfieldID** fields_ptr);
    jvmtiError (JNICALL *GetImplementedInterfaces) (jvmtiEnv* env, 
                                                    jclass klass, 
                                                    jint* interface_count_ptr, 
                                                    jclass** interfaces_ptr);
    jvmtiError (JNICALL *IsInterface) (jvmtiEnv* env, 
                                       jclass klass, 
                                       jboolean* is_interface_ptr);
    jvmtiError (JNICALL *IsArrayClass) (jvmtiEnv* env, 
                                        jclass klass, 
                                        jboolean* is_array_class_ptr);
    jvmtiError (JNICALL *GetClassLoader) (jvmtiEnv* env, 
                                          jclass klass, 
                                          jobject* classloader_ptr);
    jvmtiError (JNICALL *GetObjectHashCode) (jvmtiEnv* env, 
                                             jobject object, 
                                             jint* hash_code_ptr);
    jvmtiError (JNICALL *GetObjectMonitorUsage) (jvmtiEnv* env, 
                                                 jobject object, 
                                                 jvmtiMonitorUsage* info_ptr);
    jvmtiError (JNICALL *GetFieldName) (jvmtiEnv* env, 
                                        jclass klass, 
                                        jfieldID field, 
                                        char** name_ptr, 
                                        char** signature_ptr, 
                                        char** generic_ptr);
    jvmtiError (JNICALL *GetFieldDeclaringClass) (jvmtiEnv* env, 
                                                  jclass klass, 
                                                  jfieldID field, 
                                                  jclass* declaring_class_ptr);
    jvmtiError (JNICALL *GetFieldModifiers) (jvmtiEnv* env, 
                                             jclass klass, 
                                             jfieldID field, 
                                             jint* modifiers_ptr);
    jvmtiError (JNICALL *IsFieldSynthetic) (jvmtiEnv* env, 
                                            jclass klass, 
                                            jfieldID field, 
                                            jboolean* is_synthetic_ptr);
    jvmtiError (JNICALL *GetMethodName) (jvmtiEnv* env, 
                                         jmethodID method, 
                                         char** name_ptr, 
                                         char** signature_ptr, 
                                         char** generic_ptr);
    jvmtiError (JNICALL *GetMethodDeclaringClass) (jvmtiEnv* env, 
                                                   jmethodID method, 
                                                   jclass* declaring_class_ptr);
    jvmtiError (JNICALL *GetMethodModifiers) (jvmtiEnv* env, 
                                              jmethodID method, 
                                              jint* modifiers_ptr);
    void *reserved67;
    jvmtiError (JNICALL *GetMaxLocals) (jvmtiEnv* env, 
                                        jmethodID method, 
                                        jint* max_ptr);
    jvmtiError (JNICALL *GetArgumentsSize) (jvmtiEnv* env, 
                                            jmethodID method, 
                                            jint* size_ptr);
    jvmtiError (JNICALL *GetLineNumberTable) (jvmtiEnv* env, 
                                              jmethodID method, 
                                              jint* entry_count_ptr, 
                                              jvmtiLineNumberEntry** table_ptr);
    jvmtiError (JNICALL *GetMethodLocation) (jvmtiEnv* env, 
                                             jmethodID method, 
                                             jlocation* start_location_ptr, 
                                             jlocation* end_location_ptr);
    jvmtiError (JNICALL *GetLocalVariableTable) (jvmtiEnv* env, 
                                                 jmethodID method, 
                                                 jint* entry_count_ptr, 
                                                 jvmtiLocalVariableEntry** table_ptr);
    void *reserved73;
    void *reserved74;
    jvmtiError (JNICALL *GetBytecodes) (jvmtiEnv* env, 
                                        jmethodID method, 
                                        jint* bytecode_count_ptr, 
                                        unsigned char** bytecodes_ptr);
    jvmtiError (JNICALL *IsMethodNative) (jvmtiEnv* env, 
                                          jmethodID method, 
                                          jboolean* is_native_ptr);
    jvmtiError (JNICALL *IsMethodSynthetic) (jvmtiEnv* env, 
                                             jmethodID method, 
                                             jboolean* is_synthetic_ptr);
    jvmtiError (JNICALL *GetLoadedClasses) (jvmtiEnv* env, 
                                            jint* class_count_ptr, 
                                            jclass** classes_ptr);
    jvmtiError (JNICALL *GetClassLoaderClasses) (jvmtiEnv* env, 
                                                 jobject initiating_loader, 
                                                 jint* class_count_ptr, 
                                                 jclass** classes_ptr);
    jvmtiError (JNICALL *PopFrame) (jvmtiEnv* env, 
                                    jthread thread);
    void *reserved81;
    void *reserved82;
    void *reserved83;
    void *reserved84;
    void *reserved85;
    void *reserved86;
    jvmtiError (JNICALL *RedefineClasses) (jvmtiEnv* env, 
                                           jint class_count, 
                                           const jvmtiClassDefinition* class_definitions);
    jvmtiError (JNICALL *GetVersionNumber) (jvmtiEnv* env, 
                                            jint* version_ptr);
    jvmtiError (JNICALL *GetCapabilities) (jvmtiEnv* env, 
                                           jvmtiCapabilities* capabilities_ptr);
    jvmtiError (JNICALL *GetSourceDebugExtension) (jvmtiEnv* env, 
                                                   jclass klass, 
                                                   char** source_debug_extension_ptr);
    jvmtiError (JNICALL *IsMethodObsolete) (jvmtiEnv* env, 
                                            jmethodID method, 
                                            jboolean* is_obsolete_ptr);
    jvmtiError (JNICALL *SuspendThreadList) (jvmtiEnv* env, 
                                             jint request_count, 
                                             const jthread* request_list, 
                                             jvmtiError* results);
    jvmtiError (JNICALL *ResumeThreadList) (jvmtiEnv* env, 
                                            jint request_count, 
                                            const jthread* request_list, 
                                            jvmtiError* results);
    void *reserved94;
    void *reserved95;
    void *reserved96;
    void *reserved97;
    void *reserved98;
    void *reserved99;
    jvmtiError (JNICALL *GetAllStackTraces) (jvmtiEnv* env, 
                                             jint max_frame_count, 
                                             jvmtiStackInfo** stack_info_ptr, 
                                             jint* thread_count_ptr);
    jvmtiError (JNICALL *GetThreadListStackTraces) (jvmtiEnv* env, 
                                                    jint thread_count, 
                                                    const jthread* thread_list, 
                                                    jint max_frame_count, 
                                                    jvmtiStackInfo** stack_info_ptr);
    jvmtiError (JNICALL *GetThreadLocalStorage) (jvmtiEnv* env, 
                                                 jthread thread, 
                                                 void** data_ptr);
    jvmtiError (JNICALL *SetThreadLocalStorage) (jvmtiEnv* env, 
                                                 jthread thread, 
                                                 const void* data);
    jvmtiError (JNICALL *GetStackTrace) (jvmtiEnv* env, 
                                         jthread thread, 
                                         jint start_depth, 
                                         jint max_frame_count, 
                                         jvmtiFrameInfo* frame_buffer, 
                                         jint* count_ptr);
    void *reserved105;
    jvmtiError (JNICALL *GetTag) (jvmtiEnv* env, 
                                  jobject object, 
                                  jlong* tag_ptr);
    jvmtiError (JNICALL *SetTag) (jvmtiEnv* env, 
                                  jobject object, 
                                  jlong tag);
    jvmtiError (JNICALL *ForceGarbageCollection) (jvmtiEnv* env);
    jvmtiError (JNICALL *IterateOverObjectsReachableFromObject) (jvmtiEnv* env, 
                                                                 jobject object, 
                                                                 jvmtiObjectReferenceCallback object_reference_callback, 
                                                                 void* user_data);
    jvmtiError (JNICALL *IterateOverReachableObjects) (jvmtiEnv* env, 
                                                       jvmtiHeapRootCallback heap_root_callback, 
                                                       jvmtiStackReferenceCallback stack_ref_callback, 
                                                       jvmtiObjectReferenceCallback object_ref_callback, 
                                                       void* user_data);
    jvmtiError (JNICALL *IterateOverHeap) (jvmtiEnv* env, 
                                           jvmtiHeapObjectFilter object_filter, 
                                           jvmtiHeapObjectCallback heap_object_callback, 
                                           void* user_data);
    jvmtiError (JNICALL *IterateOverInstancesOfClass) (jvmtiEnv* env, 
                                                       jclass klass, 
                                                       jvmtiHeapObjectFilter object_filter, 
                                                       jvmtiHeapObjectCallback heap_object_callback, 
                                                       void* user_data);
    void *reserved113;
    jvmtiError (JNICALL *GetObjectsWithTags) (jvmtiEnv* env, 
                                              jint tag_count, 
                                              const jlong* tags, 
                                              jint* count_ptr, 
                                              jobject** object_result_ptr, 
                                              jlong** tag_result_ptr);
    void *reserved115;
    void *reserved116;
    void *reserved117;
    void *reserved118;
    void *reserved119;
    jvmtiError (JNICALL *SetJNIFunctionTable) (jvmtiEnv* env, 
                                               const jniNativeInterface* function_table);
    jvmtiError (JNICALL *GetJNIFunctionTable) (jvmtiEnv* env, 
                                               jniNativeInterface** function_table);
    jvmtiError (JNICALL *SetEventCallbacks) (jvmtiEnv* env, 
                                             const jvmtiEventCallbacks* callbacks, 
                                             jint size_of_callbacks);
    jvmtiError (JNICALL *GenerateEvents) (jvmtiEnv* env, 
                                          jvmtiEvent event_type);
    jvmtiError (JNICALL *GetExtensionFunctions) (jvmtiEnv* env, 
                                                 jint* extension_count_ptr, 
                                                 jvmtiExtensionFunctionInfo** extensions);
    jvmtiError (JNICALL *GetExtensionEvents) (jvmtiEnv* env, 
                                              jint* extension_count_ptr, 
                                              jvmtiExtensionEventInfo** extensions);
    jvmtiError (JNICALL *SetExtensionEventCallback) (jvmtiEnv* env, 
                                                     jint extension_event_index, 
                                                     jvmtiExtensionEvent callback);
    jvmtiError (JNICALL *DisposeEnvironment) (jvmtiEnv* env);
    jvmtiError (JNICALL *GetErrorName) (jvmtiEnv* env, 
                                        jvmtiError error, 
                                        char** name_ptr);
    jvmtiError (JNICALL *GetJLocationFormat) (jvmtiEnv* env, 
                                              jvmtiJlocationFormat* format_ptr);
    jvmtiError (JNICALL *GetSystemProperties) (jvmtiEnv* env, 
                                               jint* count_ptr, 
                                               char*** property_ptr);
    jvmtiError (JNICALL *GetSystemProperty) (jvmtiEnv* env, 
                                             const char* property, 
                                             char** value_ptr);
    jvmtiError (JNICALL *SetSystemProperty) (jvmtiEnv* env, 
                                             const char* property, 
                                             const char* value);
    jvmtiError (JNICALL *GetPhase) (jvmtiEnv* env, 
                                    jvmtiPhase* phase_ptr);
    jvmtiError (JNICALL *GetCurrentThreadCpuTimerInfo) (jvmtiEnv* env, 
                                                        jvmtiTimerInfo* info_ptr);
    jvmtiError (JNICALL *GetCurrentThreadCpuTime) (jvmtiEnv* env, 
                                                   jlong* nanos_ptr);
    jvmtiError (JNICALL *GetThreadCpuTimerInfo) (jvmtiEnv* env, 
                                                 jvmtiTimerInfo* info_ptr);
    jvmtiError (JNICALL *GetThreadCpuTime) (jvmtiEnv* env, 
                                            jthread thread, 
                                            jlong* nanos_ptr);
    jvmtiError (JNICALL *GetTimerInfo) (jvmtiEnv* env, 
                                        jvmtiTimerInfo* info_ptr);
    jvmtiError (JNICALL *GetTime) (jvmtiEnv* env, 
                                   jlong* nanos_ptr);
    jvmtiError (JNICALL *GetPotentialCapabilities) (jvmtiEnv* env, 
                                                    jvmtiCapabilities* capabilities_ptr);
    void *reserved141;
    jvmtiError (JNICALL *AddCapabilities) (jvmtiEnv* env, 
                                           const jvmtiCapabilities* capabilities_ptr);
    jvmtiError (JNICALL *RelinquishCapabilities) (jvmtiEnv* env, 
                                                  const jvmtiCapabilities* capabilities_ptr);
    jvmtiError (JNICALL *GetAvailableProcessors) (jvmtiEnv* env, 
                                                  jint* processor_count_ptr);
    void *reserved145;
    void *reserved146;
    jvmtiError (JNICALL *GetEnvironmentLocalStorage) (jvmtiEnv* env, 
                                                      void** data_ptr);
    jvmtiError (JNICALL *SetEnvironmentLocalStorage) (jvmtiEnv* env, 
                                                      const void* data);
    jvmtiError (JNICALL *AddToBootstrapClassLoaderSearch) (jvmtiEnv* env, 
                                                           const char* segment);
    jvmtiError (JNICALL *SetVerboseFlag) (jvmtiEnv* env, 
                                          jvmtiVerboseFlag flag, 
                                          jboolean value);
    void *reserved151;
    void *reserved152;
    void *reserved153;
    jvmtiError (JNICALL *GetObjectSize) (jvmtiEnv* env, 
                                         jobject object, 
                                         jlong* size_ptr);
}; 


#define JVMTI_THREAD_STATE_ALIVE 0x0001
#define JVMTI_THREAD_STATE_TERMINATED  0x0002   
#define JVMTI_THREAD_STATE_RUNNABLE  0x0004   
#define JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER  0x0400
#define JVMTI_THREAD_STATE_WAITING  0x0080
#define JVMTI_THREAD_STATE_WAITING_INDEFINITELY  0x0010
#define JVMTI_THREAD_STATE_WAITING_WITH_TIMEOUT  0x0020
#define JVMTI_THREAD_STATE_SLEEPING  0x0040
#define JVMTI_THREAD_STATE_IN_OBJECT_WAIT  0x0100
#define JVMTI_THREAD_STATE_PARKED  0x0200
#define JVMTI_THREAD_STATE_SUSPENDED  0x100000
#define JVMTI_THREAD_STATE_INTERRUPTED  0x200000
#define JVMTI_THREAD_STATE_IN_NATIVE  0x400000
#define JVMTI_THREAD_STATE_VENDOR_1  0x10000000
#define JVMTI_THREAD_STATE_VENDOR_2  0x20000000
#define JVMTI_THREAD_STATE_VENDOR_3  0x40000000

#define JVMTI_THREAD_MIN_PRIORITY  1   
#define JVMTI_THREAD_NORM_PRIORITY  5
#define JVMTI_THREAD_MAX_PRIORITY  10

#define JVMTI_CLASS_STATUS_VERIFIED  1   
#define JVMTI_CLASS_STATUS_PREPARED  2   
#define JVMTI_CLASS_STATUS_INITIALIZED  4
#define JVMTI_CLASS_STATUS_ERROR  8   
#define JVMTI_CLASS_STATUS_ARRAY  16  
#define JVMTI_CLASS_STATUS_PRIMITIVE  32 

#endif




/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
