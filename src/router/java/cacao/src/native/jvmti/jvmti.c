/* src/native/jvmti/jvmti.c - implementation of the Java Virtual Machine 
                              Tool Interface functions

   Copyright (C) 1996-2013
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


#include "config.h"

#include <assert.h>
#include <string.h>
#include <linux/unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ltdl.h>
#include <unistd.h>
#include <sched.h>

#include "native/jni.hpp"
#include "native/native.hpp"
#include "native/jvmti/cacaodbg.h"
#include "native/jvmti/jvmti.h"
#include "vm/jit/stacktrace.hpp"
#include "vm/global.hpp"
#include "vm/loader.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/class.hpp"
#include "vm/classcache.hpp"
#include "mm/gc.hpp"
#include "toolbox/logging.hpp"
#include "vm/options.hpp"
#include "vm/string.hpp"
#include "mm/memory.hpp"
#include "threads/mutex.h"
#include "threads/thread.hpp"
#include "threads/lock.hpp"
#include "vm/exceptions.hpp"
#include "native/include/java_io_PrintStream.h"
#include "native/include/java_io_InputStream.h"
#include "native/include/java_lang_Cloneable.h"
#include "native/include/java_lang_ThreadGroup.h"
#include "native/include/java_lang_VMObject.h"
#include "native/include/java_lang_VMSystem.h"
#include "native/include/java_lang_VMClass.h"
#include "vm/suck.hpp"
#include "boehm-gc/include/gc.h"

#if defined(ENABLE_THREADS)
#include <sched.h>
#include <pthread.h>
#endif 

#include "dbg.h"


typedef struct _environment environment;
static environment *envs=NULL;
mutex_t dbgcomlock;

extern const struct JNIInvokeInterface _Jv_JNIInvokeInterface;

static jvmtiPhase phase; 
typedef struct _jvmtiEventModeLL jvmtiEventModeLL;
struct _jvmtiEventModeLL {
	jvmtiEventMode mode;
	jthread event_thread;
	jvmtiEventModeLL *next;
};

typedef struct _jvmtiThreadLocalStorage jvmtiThreadLocalStorage;
struct _jvmtiThreadLocalStorage{
	jthread thread;
	void *data;
	jvmtiThreadLocalStorage *next;
};

struct _environment {
    jvmtiEnv env;
	environment *next;
    jvmtiEventCallbacks callbacks;
    /* table for enabled/disabled jvmtiEvents - first element contains global 
	   behavior */
    jvmtiEventModeLL events[JVMTI_EVENT_END_ENUM - JVMTI_EVENT_START_ENUM]; 
    jvmtiCapabilities capabilities;
    void *EnvironmentLocalStorage;
	jvmtiThreadLocalStorage *tls;
};

static struct jvmtiEnv_struct JVMTI_EnvTable;
static jvmtiCapabilities JVMTI_Capabilities;
static lt_ptr unload;

#define CHECK_PHASE_START  if (!(false 
#define CHECK_PHASE(chkphase) || (phase == chkphase)
#define CHECK_PHASE_END  )) return JVMTI_ERROR_WRONG_PHASE
#define CHECK_CAPABILITY(env,CAP) if(((environment*)                            \
										 env)->capabilities.CAP == 0)           \
                                     return JVMTI_ERROR_MUST_POSSESS_CAPABILITY;
#define CHECK_THREAD_IS_ALIVE(t) if(check_thread_is_alive(t) ==                 \
                                  JVMTI_ERROR_THREAD_NOT_ALIVE)                 \
                                  	return JVMTI_ERROR_THREAD_NOT_ALIVE;




/* check_thread_is_alive *******************************************************

   checks if the given thread is alive

*******************************************************************************/
static jvmtiError check_thread_is_alive(jthread t) {
	if(t == NULL) return JVMTI_ERROR_THREAD_NOT_ALIVE;
	if(((java_lang_Thread*) t)->vmThread == NULL) 
		return JVMTI_ERROR_THREAD_NOT_ALIVE;
	return JVMTI_ERROR_NONE;
}

/* execute_callback ************************************************************

   executes the registerd callbacks for the given jvmti event with parameter
   in the data structure.

*******************************************************************************/
static void execute_callback(jvmtiEvent e, functionptr ec, 
							 genericEventData* data) {
	JNIEnv* jni_env = (JNIEnv*)_Jv_env;

	fprintf(stderr,"execcallback called (event: %d)\n",e);

	switch (e) {
	case JVMTI_EVENT_VM_INIT:
		if (phase != JVMTI_PHASE_LIVE) return;
    case JVMTI_EVENT_THREAD_START:
    case JVMTI_EVENT_THREAD_END: 
		if ((phase == JVMTI_PHASE_START) || (phase == JVMTI_PHASE_LIVE))
			((jvmtiEventThreadStart)ec)(data->jvmti_env,jni_env,data->thread);
		break;

    case JVMTI_EVENT_CLASS_FILE_LOAD_HOOK:
		if ((phase == JVMTI_PHASE_START) || 
			(phase == JVMTI_PHASE_LIVE)  ||
			(phase == JVMTI_PHASE_PRIMORDIAL))
		((jvmtiEventClassFileLoadHook)ec) (data->jvmti_env, 
										   jni_env, 
										   data->klass,
										   data->object,
										   data->name,
										   data->protection_domain,
										   data->jint1,
										   data->class_data,
										   data->new_class_data_len,
										   data->new_class_data);

		/* if class data has been modified use it as class data for other agents 
		   waiting for the same event */
		if (data->new_class_data != NULL) {
			data->jint1 = *(data->new_class_data_len);
			data->class_data = *(data->new_class_data); 
		}
		break;


    case JVMTI_EVENT_CLASS_PREPARE: 
    case JVMTI_EVENT_CLASS_LOAD:
		if ((phase == JVMTI_PHASE_START) || (phase == JVMTI_PHASE_LIVE))
			((jvmtiEventClassLoad)ec) (data->jvmti_env, jni_env, 
									   data->thread, data->klass);
		break;

    case JVMTI_EVENT_VM_DEATH:
		if (phase != JVMTI_PHASE_LIVE) return;
    case JVMTI_EVENT_VM_START: 
		if ((phase == JVMTI_PHASE_START) || (phase == JVMTI_PHASE_LIVE))
		((jvmtiEventVMStart)ec) (data->jvmti_env, jni_env);
		break;

    case JVMTI_EVENT_NATIVE_METHOD_BIND:
		if ((phase == JVMTI_PHASE_START) || 
			(phase == JVMTI_PHASE_LIVE)  ||
			(phase == JVMTI_PHASE_PRIMORDIAL))
			((jvmtiEventNativeMethodBind)ec) (data->jvmti_env, jni_env, 
											  data->thread, 
											  data->method,
											  data->address,
											  data->new_address_ptr);
		break;
	

    case JVMTI_EVENT_DYNAMIC_CODE_GENERATED:
		if ((phase == JVMTI_PHASE_START) || 
			(phase == JVMTI_PHASE_LIVE)  ||
			(phase == JVMTI_PHASE_PRIMORDIAL))
			((jvmtiEventDynamicCodeGenerated)ec) (data->jvmti_env,
												  data->name,
												  data->address,
												  data->jint1);
		break;



	default:
		if (phase != JVMTI_PHASE_LIVE) return;
		switch (e) {
		case JVMTI_EVENT_EXCEPTION:			
			((jvmtiEventException)ec) (data->jvmti_env, jni_env, 
									   data->thread, 
									   data->method, 
									   data->location,
									   data->object,
									   data->catch_method,
									   data->catch_location);
			break;
			
		case JVMTI_EVENT_EXCEPTION_CATCH:
			((jvmtiEventExceptionCatch)ec) (data->jvmti_env, jni_env, 
											data->thread, 
											data->method, 
											data->location,
											data->object);
			break;

		case JVMTI_EVENT_BREAKPOINT:
		case JVMTI_EVENT_SINGLE_STEP:
			((jvmtiEventSingleStep)ec) (data->jvmti_env, jni_env, 
										data->thread, 
										data->method, 
										data->location);
			break;

		case JVMTI_EVENT_FRAME_POP:
			((jvmtiEventFramePop)ec) (data->jvmti_env, jni_env, 
									  data->thread, 
									  data->method, 
									  data->b);
			break;


		case JVMTI_EVENT_FIELD_ACCESS: 
			((jvmtiEventFieldAccess)ec) (data->jvmti_env, jni_env, 
										 data->thread, 
										 data->method, 
										 data->location,
										 data->klass,
										 data->object,
										 data->field);
			break;

		case JVMTI_EVENT_FIELD_MODIFICATION:

			((jvmtiEventFieldModification)ec) (data->jvmti_env, jni_env, 
											   data->thread, 
											   data->method, 
											   data->location,
											   data->klass,
											   data->object,
											   data->field,
											   data->signature_type,
											   data->value);
			break;

		case JVMTI_EVENT_METHOD_ENTRY:
			((jvmtiEventMethodEntry)ec) (data->jvmti_env, jni_env, 
										 data->thread, 
										 data->method);
			break;

		case JVMTI_EVENT_METHOD_EXIT: 
			((jvmtiEventMethodExit)ec) (data->jvmti_env, jni_env, 
										data->thread, 
										data->method,
										data->b,
										data->value);
			break;

		case JVMTI_EVENT_COMPILED_METHOD_LOAD:
			((jvmtiEventCompiledMethodLoad)ec) (data->jvmti_env, 
												data->method,
												data->jint1,
												data->address,
												data->jint2,
												data->map,
												data->compile_info);
			break;
		
		case JVMTI_EVENT_COMPILED_METHOD_UNLOAD:
			((jvmtiEventCompiledMethodUnload)ec) (data->jvmti_env,
												  data->method,
												  data->address);
			break;

		case JVMTI_EVENT_GARBAGE_COLLECTION_START:
		case JVMTI_EVENT_GARBAGE_COLLECTION_FINISH:
		case JVMTI_EVENT_DATA_DUMP_REQUEST: 
			((jvmtiEventDataDumpRequest)ec) (data->jvmti_env);
			break;

		case JVMTI_EVENT_MONITOR_WAIT:
			((jvmtiEventMonitorWait)ec) (data->jvmti_env, jni_env, 
										 data->thread, 
										 data->object,
										 data->jlong);
			break;

		case JVMTI_EVENT_MONITOR_WAITED:
			((jvmtiEventMonitorWaited)ec) (data->jvmti_env, jni_env, 
										   data->thread, 
										   data->object,
										   data->b);
			break;


		case JVMTI_EVENT_MONITOR_CONTENDED_ENTERED:
		case JVMTI_EVENT_MONITOR_CONTENDED_ENTER:
			((jvmtiEventMonitorContendedEnter)ec) (data->jvmti_env, jni_env,
												   data->thread, 
												   data->object);
			break;

		case JVMTI_EVENT_OBJECT_FREE: 
			((jvmtiEventObjectFree)ec) (data->jvmti_env, data->jlong);
			break;

		case JVMTI_EVENT_VM_OBJECT_ALLOC:
			((jvmtiEventVMObjectAlloc)ec) (data->jvmti_env, jni_env, 
										   data->thread, 
										   data->object,
										   data->klass,
										   data->jlong);
			break;
		default:
			log_text ("unknown event");
		}
		break;
	}
}


/* dofireEvent ******************************************************************

   sends event if it is enabled either globally or for some threads

*******************************************************************************/
static void dofireEvent(jvmtiEvent e, genericEventData* data) {
	environment* env;
	jvmtiEventModeLL *evm;
	functionptr ec;

	env = envs;
	while (env != NULL) {
		if (env->events[e-JVMTI_EVENT_START_ENUM].mode == JVMTI_DISABLE) {
			evm = env->events[e-JVMTI_EVENT_START_ENUM].next;
            /* test if the event is enable for some threads */
			while (evm != NULL) { 
				if (evm->mode == JVMTI_ENABLE) {
					ec = ((functionptr*)
						  (&env->callbacks))[e-JVMTI_EVENT_START_ENUM];
					if (ec != NULL) {
						data->jvmti_env=&env->env;
						execute_callback(e, ec, data);
					}
				}
				evm=evm->next;
			}
		} else { /* event enabled globally */
 			data->jvmti_env=&env->env;
			ec = ((functionptr*)(&env->callbacks))[e-JVMTI_EVENT_START_ENUM];
			if (ec != NULL) execute_callback(e, ec, data);
		}
		
		env=env->next;
	}
}


/* fireEvent ******************************************************************

   fire event callback with data arguments. This function mainly fills the
   missing EventData.

*******************************************************************************/
void jvmti_fireEvent(genericEventData* d) {
	jthread thread;
    /* XXX todo : respect event order JVMTI-Spec:Multiple Co-located Events */

	if (d->ev == JVMTI_EVENT_VM_START)
		thread = NULL;
	else
		thread = jvmti_get_current_thread();


	d->thread = thread;
	dofireEvent(d->ev,d);
}


/* SetEventNotificationMode ****************************************************

   Control the generation of events

*******************************************************************************/

static jvmtiError
SetEventNotificationMode (jvmtiEnv * env, jvmtiEventMode mode,
			  jvmtiEvent event_type, jthread event_thread, ...)
{
	environment* cacao_env;
	jvmtiEventModeLL *ll;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if(event_thread != NULL) {
		if (!builtin_instanceof(event_thread,class_java_lang_Thread))
			return JVMTI_ERROR_INVALID_THREAD;
		CHECK_THREAD_IS_ALIVE(event_thread);
	}
	
	cacao_env = (environment*) env;    
	if ((mode != JVMTI_ENABLE) && (mode != JVMTI_DISABLE))
		return JVMTI_ERROR_ILLEGAL_ARGUMENT;

	switch (event_type) { /* check capability and set system breakpoint */
    case JVMTI_EVENT_EXCEPTION:
	case JVMTI_EVENT_EXCEPTION_CATCH:
		CHECK_CAPABILITY(env,can_generate_exception_events)
		break;
    case JVMTI_EVENT_SINGLE_STEP:
		CHECK_CAPABILITY(env,can_generate_single_step_events)
		break;
    case JVMTI_EVENT_FRAME_POP:
		CHECK_CAPABILITY(env,can_generate_frame_pop_events)
		break;
    case JVMTI_EVENT_BREAKPOINT:
		CHECK_CAPABILITY(env,can_generate_breakpoint_events)
		break;
    case JVMTI_EVENT_FIELD_ACCESS:
		CHECK_CAPABILITY(env,can_generate_field_access_events)
		break;
    case JVMTI_EVENT_FIELD_MODIFICATION:
		CHECK_CAPABILITY(env,can_generate_field_modification_events)
		break;
    case JVMTI_EVENT_METHOD_ENTRY:
		CHECK_CAPABILITY(env,can_generate_method_entry_events)
		break;
    case JVMTI_EVENT_METHOD_EXIT:
		CHECK_CAPABILITY(env, can_generate_method_exit_events)
		break;
    case JVMTI_EVENT_NATIVE_METHOD_BIND:
		CHECK_CAPABILITY(env, can_generate_native_method_bind_events)
		break;
    case JVMTI_EVENT_COMPILED_METHOD_LOAD:
    case JVMTI_EVENT_COMPILED_METHOD_UNLOAD:
		CHECK_CAPABILITY(env,can_generate_compiled_method_load_events)
		break;
    case JVMTI_EVENT_MONITOR_WAIT:
    case JVMTI_EVENT_MONITOR_WAITED:
    case JVMTI_EVENT_MONITOR_CONTENDED_ENTER:
    case JVMTI_EVENT_MONITOR_CONTENDED_ENTERED:
		CHECK_CAPABILITY(env,can_generate_monitor_events)
		break;
    case JVMTI_EVENT_GARBAGE_COLLECTION_START:
	case JVMTI_EVENT_GARBAGE_COLLECTION_FINISH:
		CHECK_CAPABILITY(env,can_generate_garbage_collection_events)
		break;
    case JVMTI_EVENT_OBJECT_FREE:
		CHECK_CAPABILITY(env,can_generate_object_free_events)
		break;
    case JVMTI_EVENT_VM_OBJECT_ALLOC:
		CHECK_CAPABILITY(env,can_generate_vm_object_alloc_events)
		break;
	default:
		/* all other events are required */
		if ((event_type < JVMTI_EVENT_START_ENUM) ||
			(event_type > JVMTI_EVENT_END_ENUM))
			return JVMTI_ERROR_INVALID_EVENT_TYPE;		
		break;
	}


	if (event_thread != NULL) {
		/* thread level control */
		if ((JVMTI_EVENT_VM_INIT == mode) ||
			(JVMTI_EVENT_VM_DEATH == mode) ||
			(JVMTI_EVENT_VM_START == mode) ||
			(JVMTI_EVENT_THREAD_START == mode) ||
			(JVMTI_EVENT_COMPILED_METHOD_LOAD == mode) ||
			(JVMTI_EVENT_COMPILED_METHOD_UNLOAD == mode) ||
			(JVMTI_EVENT_DYNAMIC_CODE_GENERATED == mode) ||
			(JVMTI_EVENT_DATA_DUMP_REQUEST == mode))
			return JVMTI_ERROR_ILLEGAL_ARGUMENT;
		ll = &(cacao_env->events[event_type-JVMTI_EVENT_START_ENUM]);
		while (ll->next != NULL) {
			ll = ll->next;
			if (ll->event_thread == event_thread) {
				ll->mode=mode;
				return JVMTI_ERROR_NONE;
			}
		}
		ll->next = heap_allocate(sizeof(jvmtiEventModeLL),true,NULL);
		ll->next->mode=mode;		
	} else {
		/* global control */
		cacao_env->events[event_type-JVMTI_EVENT_START_ENUM].mode=mode;
	}

	
    return JVMTI_ERROR_NONE;
}

/* GetAllThreads ***************************************************************

   Get all live threads

*******************************************************************************/

static jvmtiError
GetAllThreads (jvmtiEnv * env, jint * threads_count_ptr,
	       jthread ** threads_ptr)
{
	threadobject** threads;
	int i;
	jvmtiError retval;
	
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    if ((threads_count_ptr == NULL) || (threads_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

	retval=jvmti_get_all_threads(threads_count_ptr, &threads);
	if (retval != JVMTI_ERROR_NONE) return retval;

	*threads_ptr = 
		heap_allocate(sizeof(jthread*)* (*threads_count_ptr),true,NULL);

	for (i=0; i<*threads_count_ptr; i++)
		(*threads_ptr)[i] = threads[i]->o.thread;
 
    return JVMTI_ERROR_NONE;
}


/* SuspendThread ***************************************************************

   Suspend specified thread

*******************************************************************************/

static jvmtiError
SuspendThread (jvmtiEnv * env, jthread thread)
{
	CHECK_PHASE_START
	CHECK_PHASE(JVMTI_PHASE_LIVE)
	CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_suspend);
    
	if(thread == NULL) return JVMTI_ERROR_INVALID_THREAD;
	if (!builtin_instanceof(thread,class_java_lang_Thread))
		return JVMTI_ERROR_INVALID_THREAD;
	CHECK_THREAD_IS_ALIVE(thread);

    /* threads_suspend_thread will implement suspend
	   threads_suspend_thread (
	   (threadobject*)((java_lang_Thread*) thread)->vmThread))*/
	
    return JVMTI_ERROR_NONE;
}

/* ResumeThread ***************************************************************

   Resume a suspended thread

*******************************************************************************/

static jvmtiError
ResumeThread (jvmtiEnv * env, jthread thread)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_suspend);

	if(thread == NULL) return JVMTI_ERROR_INVALID_THREAD;
	if (!builtin_instanceof(thread,class_java_lang_Thread))
		return JVMTI_ERROR_INVALID_THREAD;
	CHECK_THREAD_IS_ALIVE(thread);

    /* threads_resume_thread will implement resume
	   threads_resume_thread (
	   (threadobject*)((java_lang_Thread*) thread)->vmThread))*/

    return JVMTI_ERROR_NONE;
}

/* StopThread *****************************************************************

   Send asynchronous exception to the specified thread. Similar to 
   java.lang.Thread.stop(). Used to kill thread.

*******************************************************************************/

static jvmtiError
StopThread (jvmtiEnv * env, jthread thread, jobject exception)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_signal_thread);
        
	log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
	return JVMTI_ERROR_NOT_AVAILABLE;

    return JVMTI_ERROR_NONE;
}

/* InterruptThread ************************************************************

   Interrupt specified thread. Similar to java.lang.Thread.interrupt()

*******************************************************************************/

static jvmtiError
InterruptThread (jvmtiEnv * env, jthread thread)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_signal_thread)

#if defined(ENABLE_THREADS)
	if(!builtin_instanceof(thread,class_java_lang_Thread))
		return JVMTI_ERROR_INVALID_THREAD;

	CHECK_THREAD_IS_ALIVE(thread);

	threads_thread_interrupt(((java_lang_Thread *) thread)->vmThread);


    return JVMTI_ERROR_NONE;
#else
	return JVMTI_ERROR_NOT_AVAILABLE;
#endif
}

/* GetThreadInfo ***************************************************************

   Get thread information. Details of the specified thread are stored in the 
   jvmtiThreadInfo structure.

*******************************************************************************/

static jvmtiError
GetThreadInfo (jvmtiEnv * env, jthread t, jvmtiThreadInfo * info_ptr)
{
	utf *name;
	java_lang_Thread* th = (java_lang_Thread*)t;


    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	info_ptr->priority=(jint)th->priority;
	info_ptr->is_daemon=(jboolean)th->daemon;
	info_ptr->thread_group=(jthreadGroup)th->group;
	info_ptr->context_class_loader=(jobject)th->contextClassLoader;

	name = JavaString(th->name).to_utf8();
	info_ptr->name=(char*)heap_allocate(sizeof(char)*(UTF_SIZE(name)+1),true,NULL);
	utf_sprint_convert_to_latin1(info_ptr->name, name);

    return JVMTI_ERROR_NONE;
}

/* GetOwnedMonitorInfo *********************************************************

   Gets all  monitors owned by the specified thread

*******************************************************************************/

static jvmtiError
GetOwnedMonitorInfo (jvmtiEnv * env, jthread thread,
		     jint * owned_monitor_count_ptr,
		     jobject ** owned_monitors_ptr)
{
	int i,j,size=20;
	java_objectheader **om;
	lock_record_pool_t* lrp;
	threadobject* t;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_get_owned_monitor_info);

	if ((owned_monitors_ptr == NULL)||(owned_monitor_count_ptr == NULL)) 
		return JVMTI_ERROR_NULL_POINTER;

	if (thread == NULL) {
		t = jvmti_get_current_thread();
	} else {
		if(!builtin_instanceof(thread,class_java_lang_Thread))
			return JVMTI_ERROR_INVALID_THREAD;
		
		CHECK_THREAD_IS_ALIVE(thread);
		t = (threadobject*) thread;
	}

#if defined(ENABLE_THREADS)

	om=MNEW(java_objectheader*,size);

	mutex_lock(&lock_global_pool_lock);
	lrp=lock_global_pool;

	/* iterate over all lock record pools */
	while (lrp != NULL) {
		/* iterate over every lock record in a pool */
		for (j=0; j<lrp->header.size; j++) {
			/* if the lock record is owned by the given thread add it to 
			   the result array */
			if(lrp->lr[j].owner == t) {
				if (i >= size) {
					MREALLOC(om, java_objectheader*, size, size * 2);
					size = size * 2;
				}
				om[i] = lrp->lr[j].obj;
				i++;
				}
		}
		lrp=lrp->header.next;
	}

	mutex_unlock(&lock_global_pool_lock);

	*owned_monitors_ptr	= 
		heap_allocate(sizeof(java_objectheader*) * i, true, NULL);
	memcpy(*owned_monitors_ptr, om, i * sizeof(java_objectheader*));
	MFREE(om, java_objectheader*, size);

	*owned_monitor_count_ptr = i;

#endif

    return JVMTI_ERROR_NONE;
}

/* GetCurrentContendedMonitor *************************************************

   Get the object the specified thread waits for.

*******************************************************************************/

static jvmtiError
GetCurrentContendedMonitor (jvmtiEnv * env, jthread thread,
			    jobject * monitor_ptr)
{
	int j;
	lock_record_pool_t* lrp;
	threadobject* t;
	lock_waiter_t* waiter;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env, can_get_current_contended_monitor)
        
	if (monitor_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;
	*monitor_ptr=NULL;

	if (thread == NULL) {
		t = jvmti_get_current_thread();
	} else {
		if(!builtin_instanceof(thread,class_java_lang_Thread))
			return JVMTI_ERROR_INVALID_THREAD;
		
		CHECK_THREAD_IS_ALIVE(thread);
		t = (threadobject*) thread;
	}

#if defined(ENABLE_THREADS)

	mutex_lock(&lock_global_pool_lock);

	lrp=lock_global_pool;

	/* iterate over all lock record pools */
	while ((lrp != NULL) && (*monitor_ptr == NULL)) {
		/* iterate over every lock record in a pool */
		for (j=0; j<lrp->header.size; j++) {
			/* iterate over every thread that is wait on this lock record */
			waiter = lrp->lr[j].waiters;
			while (waiter != NULL) 
				/* if the waiting thread equals to the given thread we are 
				   done. Stop iterateting. */
				if(waiter->waiter == t) {
					*monitor_ptr=lrp->lr[j].obj;
					break;
				}
		}
		lrp=lrp->header.next;
	}

	mutex_unlock(&lock_global_pool_lock);


#endif
    return JVMTI_ERROR_NONE;
}

typedef struct {
	jvmtiStartFunction sf;
	jvmtiEnv* jvmti_env;
	void* arg;
} runagentparam;


static void *threadstartup(void *t) {
	runagentparam *rap = (runagentparam*)t;
	rap->sf(rap->jvmti_env,(JNIEnv*)&_Jv_JNINativeInterface,rap->arg);
	return NULL;
}

/* RunAgentThread *************************************************************

   Starts the execution of an agent thread of the specified native function 
   within the specified thread

*******************************************************************************/

static jvmtiError
RunAgentThread (jvmtiEnv * env, jthread thread, jvmtiStartFunction proc,
		const void *arg, jint priority)
{
	pthread_attr_t threadattr;
	struct sched_param sp;
	runagentparam rap;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if((thread != NULL)&&(!builtin_instanceof(thread,class_java_lang_Thread))) 
		return JVMTI_ERROR_INVALID_THREAD;
	if (proc == NULL) return JVMTI_ERROR_NULL_POINTER;
	if ((priority < JVMTI_THREAD_MIN_PRIORITY) || 
		(priority > JVMTI_THREAD_MAX_PRIORITY)) 
		return JVMTI_ERROR_INVALID_PRIORITY;

	/* XXX:  Threads started with this function should not be visible to 
	   Java programming language queries but are included in JVM TI queries */

	rap.sf = proc;
	rap.arg = (void*)arg;
	rap.jvmti_env = env;

#if defined(ENABLE_THREADS)
	pthread_attr_init(&threadattr);
	pthread_attr_setdetachstate(&threadattr, PTHREAD_CREATE_DETACHED);
	if (priority == JVMTI_THREAD_MIN_PRIORITY) {
		sp.__sched_priority = sched_get_priority_min(SCHED_FIFO);
	}
	if (priority == JVMTI_THREAD_MAX_PRIORITY) {
		sp.__sched_priority = sched_get_priority_min(SCHED_FIFO);
	}
	pthread_attr_setschedparam(&threadattr,&sp);
	if (pthread_create(&((threadobject*)
						 thread)->impl.tid, &threadattr, &threadstartup, &rap)) {
		log_text("pthread_create failed");
		assert(0);
	}
#endif

    return JVMTI_ERROR_NONE;
}


/* GetTopThreadGroups *********************************************************

   Get all top-level thread groups in the VM.

*******************************************************************************/

static jvmtiError
GetTopThreadGroups (jvmtiEnv * env, jint * group_count_ptr,
		    jthreadGroup ** groups_ptr)
{
	jint threads_count_ptr;
	threadobject *threads_ptr;
	int i,j,x,size=20;
	jthreadGroup **tg,*ttgp;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    if ((groups_ptr == NULL) || (group_count_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

#if defined(ENABLE_THREADS)
	tg = MNEW(jthreadGroup*,size);
	x = 0;
	if (JVMTI_ERROR_NONE!=GetAllThreads(env,&threads_count_ptr,(jthread**)&threads_ptr))
		return JVMTI_ERROR_INTERNAL;

	for (i=0;i<threads_count_ptr;i++){
		if (threads_ptr[i].o.thread->group == NULL) {
			log_text("threadgroup not set");
			return JVMTI_ERROR_INTERNAL;
		}
		ttgp = (jthreadGroup*)((java_lang_ThreadGroup*)threads_ptr[i].o.thread->group)->parent;
		if (ttgp == NULL) {
			j=0;
			while((j<x)&&(tg[j]!=ttgp)) { /* unique ? */
				j++;
			}
			if (j == x) {
				if (x >= size){
					MREALLOC(tg,jthreadGroup*,size,size*2);
					size=size*2;
				}
				tg[x]=ttgp;
				x++;
			}
		}
	}

	*groups_ptr	= heap_allocate(sizeof(jthreadGroup*)*x,true,NULL);
	memcpy(*groups_ptr,tg,x*sizeof(jthreadGroup*));
	MFREE(tg,jthreadGroup*,size);

	*group_count_ptr = x;

#else
	return JVMTI_ERROR_NOT_AVAILABLE;
#endif
    return JVMTI_ERROR_NONE;
}


/* GetThreadGroupInfo *********************************************************

   Get information about the specified thread group.

*******************************************************************************/

static jvmtiError
GetThreadGroupInfo (jvmtiEnv * env, jthreadGroup group,
		    jvmtiThreadGroupInfo * info_ptr)
{
	int size;
	char* name;
	java_lang_ThreadGroup* grp;
	
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
	if (info_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;
	if (!builtin_instanceof(group,class_java_lang_ThreadGroup))
		return JVMTI_ERROR_INVALID_THREAD_GROUP;

	grp = (java_lang_ThreadGroup*)group;
	
	info_ptr->parent = (jthreadGroup) 
		Java_java_lang_VMObject_clone(NULL, 
									  (jclass)grp->header.vftbl->class,
									  (java_lang_Cloneable*) &grp->parent);

	name = JavaString((java_objectheader*)grp->name).to_chars();
	size = strlen(name);
	info_ptr->name=heap_allocate(size*sizeof(char),true,NULL);
	strncpy(info_ptr->name,name,size);
	info_ptr->max_priority= (jint)grp->maxpri;
	info_ptr->is_daemon= (jboolean)grp->daemon_flag;
	
    return JVMTI_ERROR_NONE;
}


/* GetThreadGroupChildren *****************************************************

   Get the live threads and active subgroups in this thread group. 

*******************************************************************************/

static jvmtiError
GetThreadGroupChildren (jvmtiEnv * env, jthreadGroup group,
			jint * thread_count_ptr, jthread ** threads_ptr,
			jint * group_count_ptr, jthreadGroup ** groups_ptr)
{
	java_lang_ThreadGroup* tgp;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
	if ((thread_count_ptr == NULL) || (threads_ptr == NULL) ||
		(group_count_ptr == NULL) || (groups_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

	if (!builtin_instanceof(group,class_java_lang_ThreadGroup))
		return JVMTI_ERROR_INVALID_THREAD_GROUP;

	tgp = (java_lang_ThreadGroup*)group;

	*thread_count_ptr = (jint)tgp->threads->elementCount;

	*threads_ptr = 
		heap_allocate(sizeof(jthread)*(*thread_count_ptr),true,NULL);

	memcpy(*threads_ptr, &tgp->threads->elementData, 
		   (*thread_count_ptr)*sizeof(java_objectarray*));

	*group_count_ptr = (jint) tgp->groups->elementCount;

	*groups_ptr	= 
		heap_allocate(sizeof(jthreadGroup)*(*group_count_ptr),true,NULL);	

	memcpy(*groups_ptr, &tgp->threads->elementData,
		   (*group_count_ptr)*sizeof(jthreadGroup*));

    return JVMTI_ERROR_NONE;
}


/* getcacaostacktrace *********************************************************

   Helper function that retrives stack trace for specified thread. 
   
*******************************************************************************/

static jvmtiError getcacaostacktrace(stacktracebuffer** trace, jthread thread) {
	threadobject *t;
	bool resume;
	
	if (thread == NULL) {
		t = jvmti_get_current_thread();
		*trace = stacktrace_create(t);
	} else {
		t = (threadobject*)((java_lang_Thread*)thread)->vmThread;
/*		if (t != jvmti_get_current_thread())
			resume = threads_suspend_thread_if_running(thread);
    
		*trace = stacktrace_create(thread );
		
		if (resume)
		threads_resume_thread ( thread );*/
	}

    return JVMTI_ERROR_NONE;
}


/* GetFrameCount **************************************************************


   Get the number of frames in the specified thread's stack. Calling function
   has to take care of suspending/resuming thread.

*******************************************************************************/

static jvmtiError
GetFrameCount (jvmtiEnv * env, jthread thread, jint * count_ptr)
{
	stacktracebuffer* trace;
	jvmtiError er;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    
	if (thread != NULL){
		if(!builtin_instanceof(thread,class_java_lang_Thread))
			return JVMTI_ERROR_INVALID_THREAD;

		CHECK_THREAD_IS_ALIVE(thread);
	}
	
	if(count_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

	er = getcacaostacktrace(&trace, thread);
	if (er == JVMTI_ERROR_NONE) {
		heap_free(trace);
		return er;
	}

	*count_ptr = trace->used;

	heap_free(trace);
    return JVMTI_ERROR_NONE;
}


/* GetThreadState **************************************************************

   Get the state of a thread.

*******************************************************************************/

static jvmtiError
GetThreadState (jvmtiEnv * env, jthread thread, jint * thread_state_ptr)
{
	java_lang_Thread* th = (java_lang_Thread*)thread;
	threadobject* t = (threadobject*)th->vmThread;

	CHECK_PHASE_START
	CHECK_PHASE(JVMTI_PHASE_LIVE)
	CHECK_PHASE_END;

	if(!builtin_instanceof(thread,class_java_lang_Thread))
		return JVMTI_ERROR_INVALID_THREAD;

	if (thread_state_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

	*thread_state_ptr = 0;
#if defined(ENABLE_THREADS)
	if((th->vmThread == NULL)&&(th->group == NULL)) { /* alive ? */
		/* not alive */
		if (((threadobject*)th->vmThread)->tid == 0)
			*thread_state_ptr = JVMTI_THREAD_STATE_TERMINATED;
	} else {
		/* alive */
		*thread_state_ptr = JVMTI_THREAD_STATE_ALIVE;
		if (t->interrupted) *thread_state_ptr |= JVMTI_THREAD_STATE_INTERRUPTED;
		/* XXX todo -  info not available */
		if (false) *thread_state_ptr |= JVMTI_THREAD_STATE_SUSPENDED;
		if (false) *thread_state_ptr |= JVMTI_THREAD_STATE_IN_NATIVE;
		if (false) *thread_state_ptr |= JVMTI_THREAD_STATE_RUNNABLE;
		if (false) *thread_state_ptr |= JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER;
		if (false /* t->ee.waiting ? */) *thread_state_ptr |= JVMTI_THREAD_STATE_WAITING;
		if (false) *thread_state_ptr |= JVMTI_THREAD_STATE_WAITING_INDEFINITELY;
		if (false) *thread_state_ptr |= JVMTI_THREAD_STATE_WAITING_WITH_TIMEOUT;
		if (false) *thread_state_ptr |= JVMTI_THREAD_STATE_IN_OBJECT_WAIT;
		if (false) *thread_state_ptr |= JVMTI_THREAD_STATE_PARKED;
		if (false) *thread_state_ptr |= JVMTI_THREAD_STATE_SLEEPING;
	}
#else
	return JVMTI_ERROR_INTERNAL;
#endif

    return JVMTI_ERROR_NONE;
}


/* GetFrameLocation ************************************************************

   Get the location of the instruction currently executing

*******************************************************************************/

static jvmtiError
GetFrameLocation (jvmtiEnv * env, jthread thread, jint depth,
		  jmethodID * method_ptr, jlocation * location_ptr)
{
	stackframeinfo_t *sfi;
	int i;
	threadobject* th;
		
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
	if (thread == NULL) {
		th = jvmti_get_current_thread();
	} else {
		if(!builtin_instanceof(thread,class_java_lang_Thread))
			return JVMTI_ERROR_INVALID_THREAD;
		
		CHECK_THREAD_IS_ALIVE(thread);
		th = (threadobject*) ((java_lang_Thread*)thread)->vmThread;
	}

	if (depth < 0) return JVMTI_ERROR_ILLEGAL_ARGUMENT;

	if ((method_ptr == NULL)&&(location_ptr == NULL)) 
		return JVMTI_ERROR_NULL_POINTER;
	
	sfi = th->_stackframeinfo;
	
	i = 0;
	while ((sfi != NULL) && (i<depth)) {
		sfi = sfi->prev;
		i++;
	}
	
	if (i>depth) return JVMTI_ERROR_NO_MORE_FRAMES;

	*method_ptr=(jmethodID)sfi->code->m;
	*location_ptr = 0; /* todo: location MachinePC not avilable - Linenumber not expected */
	
    return JVMTI_ERROR_NONE;
}


/* NotifyFramePop *************************************************************

   

*******************************************************************************/

static jvmtiError
NotifyFramePop (jvmtiEnv * env, jthread thread, jint depth)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_generate_frame_pop_events)
        
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}

/* GetLocalObject *************************************************************

   

*******************************************************************************/

static jvmtiError
GetLocalObject (jvmtiEnv * env,
		jthread thread, jint depth, jint slot, jobject * value_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)

  log_text ("JVMTI-Call: TBD-OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}

/* GetLocalInt ****************************************************************

   

*******************************************************************************/

static jvmtiError
GetLocalInt (jvmtiEnv * env,
	     jthread thread, jint depth, jint slot, jint * value_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)        
  log_text ("JVMTI-Call: TBD OPTIONAL  IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}

/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
GetLocalLong (jvmtiEnv * env, jthread thread, jint depth, jint slot,
	      jlong * value_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)        
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
GetLocalFloat (jvmtiEnv * env, jthread thread, jint depth, jint slot,
	       jfloat * value_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)        
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
GetLocalDouble (jvmtiEnv * env, jthread thread, jint depth, jint slot,
		jdouble * value_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)        
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
SetLocalObject (jvmtiEnv * env, jthread thread, jint depth, jint slot,
		jobject value)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)
    
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
SetLocalInt (jvmtiEnv * env, jthread thread, jint depth, jint slot,
	     jint value)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
SetLocalLong (jvmtiEnv * env, jthread thread, jint depth, jint slot,
	      jlong value)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
SetLocalFloat (jvmtiEnv * env, jthread thread, jint depth, jint slot,
	       jfloat value)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
SetLocalDouble (jvmtiEnv * env, jthread thread, jint depth, jint slot,
		jdouble value)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_access_local_variables)
    
	 log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* CreateRawMonitor ***********************************************************

   This function creates a new raw monitor.

*******************************************************************************/

static jvmtiError
CreateRawMonitor (jvmtiEnv * env, const char *name,
		  jrawMonitorID * monitor_ptr)
{
	struct _jrawMonitorID *monitor = (struct _jrawMonitorID*) monitor_ptr;
		
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
	if ((name == NULL) || (monitor_ptr == NULL)) 
		return JVMTI_ERROR_NULL_POINTER;

#if defined(ENABLE_THREADS)
	monitor->name = JavaString::from_utf8(name);
#else
	log_text ("CreateRawMonitor not supported");
#endif

    return JVMTI_ERROR_NONE;
}


/* DestroyRawMonitor **********************************************************

   This function destroys a raw monitor.   

*******************************************************************************/

static jvmtiError
DestroyRawMonitor (jvmtiEnv * env, jrawMonitorID monitor)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if (!builtin_instanceof((java_objectheader*)monitor->name,class_java_lang_String))
		return JVMTI_ERROR_INVALID_MONITOR;

#if defined(ENABLE_THREADS)
	if (!lock_is_held_by_current_thread((java_objectheader*)monitor->name))
		return JVMTI_ERROR_NOT_MONITOR_OWNER;
	
	lock_monitor_exit((java_objectheader *) monitor->name);

	heap_free(monitor);
#else
	log_text ("DestroyRawMonitor not supported");
#endif

	return JVMTI_ERROR_NONE;
}


/* RawMonitorEnter ************************************************************

  Gain exclusive ownership of a raw monitor

*******************************************************************************/

static jvmtiError
RawMonitorEnter (jvmtiEnv * env, jrawMonitorID monitor)
{
	if (!builtin_instanceof((java_objectheader*)monitor->name,class_java_lang_String))
		return JVMTI_ERROR_INVALID_MONITOR;

#if defined(ENABLE_THREADS)
	lock_monitor_enter((java_objectheader *) monitor->name);
#else
	log_text ("RawMonitorEnter not supported");
#endif

    return JVMTI_ERROR_NONE;
}


/* RawMonitorExit *************************************************************

   Release raw monitor

*******************************************************************************/

static jvmtiError
RawMonitorExit (jvmtiEnv * env, jrawMonitorID monitor)
{
	if (!builtin_instanceof((java_objectheader*)monitor->name,class_java_lang_String))
		return JVMTI_ERROR_INVALID_MONITOR;

#if defined(ENABLE_THREADS)
	/* assure current thread owns this monitor */
	if (!lock_is_held_by_current_thread((java_objectheader*)monitor->name))
		return JVMTI_ERROR_NOT_MONITOR_OWNER;

	lock_monitor_exit((java_objectheader *) monitor->name);
#else
	log_text ("RawMonitorExit not supported");
#endif

    return JVMTI_ERROR_NONE;
}


/* RawMonitorWait *************************************************************

   Wait for notification of the raw monitor.

*******************************************************************************/

static jvmtiError
RawMonitorWait (jvmtiEnv * env, jrawMonitorID monitor, jlong millis)
{
	if (!builtin_instanceof((java_objectheader*)monitor->name,class_java_lang_String))
		return JVMTI_ERROR_INVALID_MONITOR;

#if defined(ENABLE_THREADS)
	/* assure current thread owns this monitor */
	if (!lock_is_held_by_current_thread((java_objectheader*)monitor->name))
		return JVMTI_ERROR_NOT_MONITOR_OWNER;

	lock_wait_for_object(&monitor->name->header, millis,0);
	if (builtin_instanceof((java_objectheader*)exceptionptr, load_class_bootstrap(Utf8String::from_utf8("java/lang/InterruptedException"))))
		return JVMTI_ERROR_INTERRUPT;

#else
	log_text ("RawMonitorWait not supported");
#endif

    return JVMTI_ERROR_NONE;
}


/* RawMonitorNotify ***********************************************************

 Notify one thread waiting on the given monitor.

*******************************************************************************/

static jvmtiError
RawMonitorNotify (jvmtiEnv * env, jrawMonitorID monitor)
{
	if (!builtin_instanceof((java_objectheader*)monitor->name,class_java_lang_String))
		return JVMTI_ERROR_INVALID_MONITOR;

#if defined(ENABLE_THREADS)
	/* assure current thread owns this monitor */
	if (!lock_is_held_by_current_thread((java_objectheader*)monitor->name))
		return JVMTI_ERROR_NOT_MONITOR_OWNER;

	lock_notify_object((java_objectheader*)&monitor->name);
#else
	log_text ("RawMonitorNotify not supported");
#endif

    return JVMTI_ERROR_NONE;
}


/* RawMonitorNotifyAll *********************************************************

 Notify all threads waiting on the given monitor.   

*******************************************************************************/

static jvmtiError
RawMonitorNotifyAll (jvmtiEnv * env, jrawMonitorID monitor)
{
	if (!builtin_instanceof((java_objectheader*)monitor->name,class_java_lang_String))
		return JVMTI_ERROR_INVALID_MONITOR;

#if defined(ENABLE_THREADS)
	/* assure current thread owns this monitor */
	if (!lock_is_held_by_current_thread((java_objectheader*)monitor->name))
		return JVMTI_ERROR_NOT_MONITOR_OWNER;

	lock_notify_all_object((java_objectheader*)&monitor->name);
#else
	log_text ("RawMonitorNotifyAll not supported");
#endif

    return JVMTI_ERROR_NONE;
}


/* SetBreakpoint **************************************************************

   

*******************************************************************************/

static jvmtiError
SetBreakpoint (jvmtiEnv * env, jmethodID method, jlocation location)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_generate_breakpoint_events)

		/* addbrkpt */
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
ClearBreakpoint (jvmtiEnv * env, jmethodID method, jlocation location)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_generate_breakpoint_events)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* SetFieldAccessWatch ********************************************************

   

*******************************************************************************/

static jvmtiError
SetFieldAccessWatch (jvmtiEnv * env, jclass klass, jfieldID field)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_generate_field_access_events)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
ClearFieldAccessWatch (jvmtiEnv * env, jclass klass, jfieldID field)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_generate_field_access_events)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
SetFieldModificationWatch (jvmtiEnv * env, jclass klass, jfieldID field)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_generate_field_modification_events)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
ClearFieldModificationWatch (jvmtiEnv * env, jclass klass, jfieldID field)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_generate_field_modification_events)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* Allocate ********************************************************************

   Allocate an area of memory through the JVM TI allocator. The allocated 
   memory should be freed with Deallocate

*******************************************************************************/

static jvmtiError
Allocate (jvmtiEnv * env, jlong size, unsigned char **mem_ptr)
{
    
    if (mem_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;
    if (size < 0) return JVMTI_ERROR_ILLEGAL_ARGUMENT;

    *mem_ptr = heap_allocate(sizeof(size),true,NULL);
    if (*mem_ptr == NULL) 
        return JVMTI_ERROR_OUT_OF_MEMORY;
    else
        return JVMTI_ERROR_NONE;
    
}


/* Deallocate ******************************************************************

   Deallocate mem using the JVM TI allocator.

*******************************************************************************/

static jvmtiError
Deallocate (jvmtiEnv * env, unsigned char *mem)
{
    /* let Boehm GC do the job */
	heap_free(mem);
    return JVMTI_ERROR_NONE;
}


/* GetClassSignature ************************************************************

   For the class indicated by klass, return the JNI type signature and the 
   generic signature of the class.

*******************************************************************************/

static jvmtiError
GetClassSignature (jvmtiEnv * env, jclass klass, char **signature_ptr,
		   char **generic_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    
	if (klass == NULL) return JVMTI_ERROR_INVALID_CLASS;
	if (!builtin_instanceof(klass,class_java_lang_Class))
		return JVMTI_ERROR_INVALID_CLASS;

	if (signature_ptr != NULL) {
		*signature_ptr = (char*)
			heap_allocate(sizeof(char) * 
						  UTF_SIZE(((classinfo*)klass)->name)+1,true,NULL);
		
		utf_sprint_convert_to_latin1(*signature_ptr,((classinfo*)klass)->name);
	}

	if (generic_ptr!= NULL)
		*generic_ptr = NULL;

    return JVMTI_ERROR_NONE;
}

/* GetClassStatus *************************************************************

   Get status of the class.

*******************************************************************************/

static jvmtiError
GetClassStatus (jvmtiEnv * env, jclass klass, jint * status_ptr)
{
	classinfo *c;
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if (!builtin_instanceof((java_objectheader*)klass,class_java_lang_Class))
		return JVMTI_ERROR_INVALID_CLASS; 

    if (status_ptr == NULL) 
        return JVMTI_ERROR_NULL_POINTER;

	c = (classinfo*)klass;
	*status_ptr = 0;	

/* 	if (c) *status_ptr = *status_ptr | JVMTI_CLASS_STATUS_VERIFIED; ? */
	if (c->state & CLASS_LINKED) 
		*status_ptr = *status_ptr | JVMTI_CLASS_STATUS_PREPARED;

	if (c->state & CLASS_INITIALIZED) 
		*status_ptr = *status_ptr | JVMTI_CLASS_STATUS_INITIALIZED;

	if (c->state & CLASS_ERROR) 
		*status_ptr = *status_ptr | JVMTI_CLASS_STATUS_ERROR;

	if (c->vftbl->arraydesc != NULL) 
		*status_ptr = *status_ptr | JVMTI_CLASS_STATUS_ARRAY;

	if (Java_java_lang_VMClass_isPrimitive(NULL,NULL,(struct java_lang_Class*)c)) 
		*status_ptr = *status_ptr | JVMTI_CLASS_STATUS_PRIMITIVE; 

    return JVMTI_ERROR_NONE;
}


/* GetSourceFileName **********************************************************

   For the class indicated by klass, return the source file name.

*******************************************************************************/

static jvmtiError
GetSourceFileName (jvmtiEnv * env, jclass klass, char **source_name_ptr)
{
    int size; 

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_get_source_file_name)
        
    if ((klass == NULL)||(source_name_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;
    
    size = UTF_SIZE(((classinfo*)klass)->sourcefile)+1;

    *source_name_ptr = (char*) heap_allocate(sizeof(char)* size,true,NULL);
    
    memcpy(*source_name_ptr,(UTF_TEXT((classinfo*)klass)->sourcefile), size);
	(*source_name_ptr)[size]='\0';

    return JVMTI_ERROR_NONE;
}


/* GetClassModifiers **********************************************************

   For class klass return the access flags

*******************************************************************************/

static jvmtiError
GetClassModifiers (jvmtiEnv * env, jclass klass, jint * modifiers_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
	if (modifiers_ptr == NULL)
		return JVMTI_ERROR_NULL_POINTER;	

	if (!builtin_instanceof((java_objectheader*)klass,class_java_lang_Class))
		return JVMTI_ERROR_INVALID_CLASS;

	*modifiers_ptr = (jint) ((classinfo*)klass)->flags;
	
    return JVMTI_ERROR_NONE;
}


/* GetClassMethods *************************************************************

   For class klass return a count of methods and a list of method IDs

*******************************************************************************/

static jvmtiError
GetClassMethods (jvmtiEnv * env, jclass klass, jint * method_count_ptr,
		 jmethodID ** methods_ptr)
{
	int i;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    	
    if ((klass == NULL)||(methods_ptr == NULL)||(method_count_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

	if (!builtin_instanceof((java_objectheader*)klass,class_java_lang_Class))
		return JVMTI_ERROR_INVALID_CLASS;

    *method_count_ptr = (jint)((classinfo*)klass)->methodscount;
    *methods_ptr = (jmethodID*) 
        heap_allocate(sizeof(jmethodID) * (*method_count_ptr),true,NULL);

    for (i=0; i<*method_count_ptr;i++)
		(*methods_ptr)[i]=(jmethodID) &(((classinfo*)klass)->methods[i]);

    return JVMTI_ERROR_NONE;
}


/* GetClassFields *************************************************************

   For the class indicated by klass, return a count of fields and a list of 
   field IDs.

*******************************************************************************/

static jvmtiError
GetClassFields (jvmtiEnv * env, jclass klass, jint * field_count_ptr,
		jfieldID ** fields_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if ((klass == NULL)||(fields_ptr == NULL)||(field_count_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

    *field_count_ptr = (jint)((classinfo*)klass)->fieldscount;
    *fields_ptr = (jfieldID*) 
        heap_allocate(sizeof(jfieldID) * (*field_count_ptr),true,NULL);
    
    memcpy (*fields_ptr, ((classinfo*)klass)->fields, 
            sizeof(jfieldID) * (*field_count_ptr));
    
    return JVMTI_ERROR_NONE;
}


/* GetImplementedInterfaces ***************************************************

   Return the direct super-interfaces of this class.

*******************************************************************************/

static jvmtiError
GetImplementedInterfaces (jvmtiEnv * env, jclass klass,
			  jint * interface_count_ptr,
			  jclass ** interfaces_ptr)
{
	int i;
	classref_or_classinfo *interfaces;
	classinfo *tmp;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if ((interfaces_ptr == NULL) || (interface_count_ptr == NULL))
		return JVMTI_ERROR_NULL_POINTER;

	if (!builtin_instanceof((java_objectheader*)klass,class_java_lang_Class))
		return JVMTI_ERROR_INVALID_CLASS;


    *interface_count_ptr = (jint)((classinfo*)klass)->interfacescount;
    *interfaces_ptr      = heap_allocate(sizeof(jclass*) * (*interface_count_ptr),true,NULL);

	interfaces = ((classinfo*)klass)->interfaces;
	for (i=0; i<*interface_count_ptr; i++) {
		if (interfaces[i].is_classref())
			tmp = load_class_bootstrap(interfaces[i].ref->name);
		else
			tmp = interfaces[i].cls;

		*interfaces_ptr[i]=tmp;
	}

    return JVMTI_ERROR_NONE;
}


/* IsInterface ****************************************************************

   Determines whether a class object reference represents an interface.

*******************************************************************************/

static jvmtiError
IsInterface (jvmtiEnv * env, jclass klass, jboolean * is_interface_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if ((klass == NULL)||(is_interface_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;
    
    *is_interface_ptr = (((classinfo*)klass)->flags & ACC_INTERFACE);

    return JVMTI_ERROR_NONE;
}

/* IsArrayClass ***************************************************************

   Determines whether a class object reference represents an array.

*******************************************************************************/

static jvmtiError
IsArrayClass (jvmtiEnv * env, jclass klass, jboolean * is_array_class_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if (is_array_class_ptr == NULL) 
        return JVMTI_ERROR_NULL_POINTER;

    *is_array_class_ptr = ((classinfo*)klass)->vftbl->arraydesc != NULL;

    return JVMTI_ERROR_NONE;
}


/* GetClassLoader *************************************************************

   For the class indicated by klass, return via classloader_ptr a reference to 
   the class loader for the class.

*******************************************************************************/

static jvmtiError
GetClassLoader (jvmtiEnv * env, jclass klass, jobject * classloader_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if ((klass == NULL)||(classloader_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

    *classloader_ptr = (jobject)((classinfo*)klass)->classloader;
 
    return JVMTI_ERROR_NONE;
}


/* GetObjectHashCode **********************************************************

   Return hash code for object object

*******************************************************************************/

static jvmtiError
GetObjectHashCode (jvmtiEnv * env, jobject object, jint * hash_code_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
   
	if (hash_code_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;
	if (!builtin_instanceof(object,class_java_lang_Object))
		return JVMTI_ERROR_INVALID_OBJECT;
     
	*hash_code_ptr = Java_java_lang_VMSystem_identityHashCode(NULL,NULL,(struct java_lang_Object*)object);

    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
GetObjectMonitorUsage (jvmtiEnv * env, jobject object,
		       jvmtiMonitorUsage * info_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_get_monitor_info)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* GetFieldName ***************************************************************

   For the field indicated by klass and field, return the field name and 
   signature.

*******************************************************************************/

static jvmtiError
GetFieldName (jvmtiEnv * env, jclass klass, jfieldID field,
	      char **name_ptr, char **signature_ptr, char **generic_ptr)
{
    int size; 

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if (klass == NULL) 
		return JVMTI_ERROR_INVALID_CLASS;
	else 
		if (!builtin_instanceof(klass,class_java_lang_Class))
			return JVMTI_ERROR_INVALID_CLASS;
    if (field == NULL) return JVMTI_ERROR_INVALID_FIELDID;
    
    if (name_ptr != NULL) {
		size = UTF_SIZE(((fieldinfo*)field)->name)+1;
		*name_ptr = (char*) heap_allocate(sizeof(char)* size,true,NULL);
		utf_sprint_convert_to_latin1(*name_ptr, ((fieldinfo*)field)->name);
	}

	if (signature_ptr != NULL) {
		size = UTF_SIZE(((fieldinfo*)field)->descriptor)+1;
		*signature_ptr = (char*) heap_allocate(sizeof(char)* size,true,NULL); 
		utf_sprint_convert_to_latin1(*signature_ptr, 
									 ((fieldinfo*)field)->descriptor);
	}

	if (generic_ptr != NULL) 
		*generic_ptr = NULL;

    return JVMTI_ERROR_NONE;
}


/* GetFieldDeclaringClass *****************************************************

   For the field indicated by klass and field return the class that defined it 
   The declaring class will either be klass, a superclass, or an implemented 
   interface.

*******************************************************************************/

static jvmtiError
GetFieldDeclaringClass (jvmtiEnv * env, jclass klass, jfieldID field,
			jclass * declaring_class_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if (klass == NULL) 
		return JVMTI_ERROR_INVALID_CLASS;
	else 
		if (!builtin_instanceof(klass,class_java_lang_Class))
			return JVMTI_ERROR_INVALID_CLASS;

    if (field == NULL) return JVMTI_ERROR_INVALID_FIELDID;	

    if (declaring_class_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

	*declaring_class_ptr = (jclass) ((fieldinfo*)field)->class;
 
    return JVMTI_ERROR_NONE;
}


/* GetFieldModifiers **********************************************************

   Return access flags of field field 

*******************************************************************************/

static jvmtiError
GetFieldModifiers (jvmtiEnv * env, jclass klass, jfieldID field,
		   jint * modifiers_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if (klass == NULL) 
		return JVMTI_ERROR_INVALID_CLASS;
	else 
		if (!builtin_instanceof(klass,class_java_lang_Class))
			return JVMTI_ERROR_INVALID_CLASS;

    if (field == NULL) return JVMTI_ERROR_INVALID_FIELDID;

	if (modifiers_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;
 
	*modifiers_ptr = ((fieldinfo*)field)->flags;
	
    return JVMTI_ERROR_NONE;
}


/* IsFieldSynthetic ***********************************************************

   

*******************************************************************************/

static jvmtiError
IsFieldSynthetic (jvmtiEnv * env, jclass klass, jfieldID field,
		  jboolean * is_synthetic_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_get_synthetic_attribute)
        
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* GetMethodName ***************************************************************

   For the method indicated by method, return the method name via name_ptr and 
   method signature via signature_ptr.

*******************************************************************************/

static jvmtiError
GetMethodName (jvmtiEnv * env, jmethodID method, char **name_ptr,
	       char **signature_ptr, char **generic_ptr)
{
    methodinfo* m = (methodinfo*)method;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;


	if (method == NULL) return JVMTI_ERROR_INVALID_METHODID;

	if (name_ptr != NULL) {
		*name_ptr = (char*)
			heap_allocate(sizeof(char) * (UTF_SIZE(m->name)+1),true,NULL);
		utf_sprint_convert_to_latin1(*name_ptr, m->name);
	}
	
	if (signature_ptr != NULL) {
		*signature_ptr = (char*)
			heap_allocate(sizeof(char)*(UTF_SIZE(m->descriptor)+1),true,NULL);
		utf_sprint_convert_to_latin1(*signature_ptr, m->descriptor);
	}

	if (generic_ptr != NULL) {
        /* there is no generic signature attribute */
		*generic_ptr = NULL;
	}

    return JVMTI_ERROR_NONE;
}


/* GetMethodDeclaringClass *****************************************************

  For the method indicated by method, return the class that defined it.

*******************************************************************************/

static jvmtiError
GetMethodDeclaringClass (jvmtiEnv * env, jmethodID method,
			 jclass * declaring_class_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
     
    if ((method == NULL) || (declaring_class_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;
    
    *declaring_class_ptr = (jclass)((methodinfo*)method)->class;
    
    return JVMTI_ERROR_NONE;
}


/* GetMethodModifiers **********************************************************

   For the method indicated by method, return the access flags.

*******************************************************************************/

static jvmtiError
GetMethodModifiers (jvmtiEnv * env, jmethodID method, jint * modifiers_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if ((method == NULL) || (modifiers_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

    *modifiers_ptr = (jint) (((methodinfo*)method)->flags);

    return JVMTI_ERROR_NONE;
}


/* GetMaxLocals ****************************************************************

   For the method indicated by method, return the number of local variable slots 
   used by the method, including the local variables used to pass parameters to 
   the method on its invocation.

*******************************************************************************/

static jvmtiError
GetMaxLocals (jvmtiEnv * env, jmethodID method, jint * max_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if ((method == NULL)||(max_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;    
    
    if (((methodinfo*)method)->flags & ACC_NATIVE)  
        return JVMTI_ERROR_NATIVE_METHOD;
 
    *max_ptr = (jint) ((methodinfo*)method)->maxlocals;

    return JVMTI_ERROR_NONE;
}



/* GetArgumentsSize ************************************************************

   Return the number of local variable slots used by the method's arguments.

*******************************************************************************/

static jvmtiError
GetArgumentsSize (jvmtiEnv * env, jmethodID method, jint * size_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    if ((method == NULL)||(size_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;    
    
    if (((methodinfo*)method)->flags & ACC_NATIVE)  
        return JVMTI_ERROR_NATIVE_METHOD;

    *size_ptr = (jint)((methodinfo*)method)->parseddesc->paramslots;
    return JVMTI_ERROR_NONE;
}



/* GetLineNumberTable **********************************************************

   Return table of source line number entries for a given method

*******************************************************************************/

static jvmtiError
GetLineNumberTable (jvmtiEnv * env, jmethodID method,
		    jint * entry_count_ptr, jvmtiLineNumberEntry ** table_ptr)
{
    int i;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_get_line_numbers)
   
    if ((method == NULL) || (entry_count_ptr == NULL) || (table_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;    

    if (((methodinfo*)method)->flags & ACC_NATIVE)  
        return JVMTI_ERROR_NATIVE_METHOD;

    if (((methodinfo*)method)->linenumbers == NULL) 
        return JVMTI_ERROR_ABSENT_INFORMATION;

    *entry_count_ptr= (jint)((methodinfo*)method)->linenumbercount;
    *table_ptr = (jvmtiLineNumberEntry*) heap_allocate(
        sizeof(jvmtiLineNumberEntry) * (*entry_count_ptr),true,NULL);


    for (i=0; i < *entry_count_ptr; i++) {
        (*table_ptr)[i].start_location = 
            (jlocation) ((methodinfo*)method)->linenumbers[i].start_pc;
        (*table_ptr)[i].line_number = 
            (jint) ((methodinfo*)method)->linenumbers[i].line_number;
    }
    
    return JVMTI_ERROR_NONE;
}


/* GetMethodLocation ***********************************************************

   For the method indicated by method, return the beginning and ending addresses 
   through start_location_ptr and end_location_ptr. In cacao this points to 
   entry point in machine code and length of machine code

*******************************************************************************/

static jvmtiError
GetMethodLocation (jvmtiEnv * env, jmethodID method,
		   jlocation * start_location_ptr,
		   jlocation * end_location_ptr)
{
    methodinfo* m = (methodinfo*)method;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if (method == NULL)	return JVMTI_ERROR_INVALID_METHODID;

	if (((methodinfo*)method)->flags & ACC_NATIVE) 
		return JVMTI_ERROR_NATIVE_METHOD;

    if ((start_location_ptr == NULL) || (end_location_ptr == NULL)) 
    	return JVMTI_ERROR_NULL_POINTER;
    
	/* XXX we return the location of the most recent code. Don't know
	 * if there is a way to teach jvmti that a method can have more
	 * than one location. -Edwin */

	/* XXX Don't know if that's the right way to deal with not-yet-
	 * compiled methods. -Edwin */

    fprintf(stderr,"GetMethodLocation *** XXX todo \n");


	/* -1 states location information is not available */
    *start_location_ptr = (jlocation)-1;
    *end_location_ptr = (jlocation)-1;

/*	
    *start_location_ptr = (jlocation)m->code->mcode;
    *end_location_ptr = (jlocation)(m->code->mcode)+m->code->mcodelength;*/
    return JVMTI_ERROR_NONE;
}


/* GetLocalVariableTable *******************************************************

   Return local variable information.

*******************************************************************************/

static jvmtiError
GetLocalVariableTable (jvmtiEnv * env, jmethodID method,
		       jint * entry_count_ptr,
		       jvmtiLocalVariableEntry ** table_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_access_local_variables)
        
  log_text ("JVMTI-Call: TBD OPTIONAL IMPLEMENT ME!!!");

    return JVMTI_ERROR_NONE;
}


/* GetBytecode *****************************************************************

   For the method indicated by method, return the byte codes that implement the 
   method.

*******************************************************************************/

static jvmtiError
GetBytecodes (jvmtiEnv * env, jmethodID method,
	      jint * bytecode_count_ptr, unsigned char **bytecodes_ptr)
{
    methodinfo* m = (methodinfo*)method;;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_get_bytecodes)
        
    if ((method == NULL) || (bytecode_count_ptr == NULL) || 
        (bytecodes_ptr == NULL)) return JVMTI_ERROR_NULL_POINTER;

    *bytecode_count_ptr = m->jcodelength;
    *bytecodes_ptr = (unsigned char*)heap_allocate(m->jcodelength,true,NULL);
    memcpy(*bytecodes_ptr, m->jcode, m->jcodelength);

    return JVMTI_ERROR_NONE;
}


/* IsMethodNative **************************************************************

   For the method indicated by method, return a value indicating whether the 
   method is a native function

*******************************************************************************/

static jvmtiError
IsMethodNative (jvmtiEnv * env, jmethodID method, jboolean * is_native_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    
    if ((method == NULL)||(is_native_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;    

    if (((methodinfo*)method)->flags & ACC_NATIVE) 
        *is_native_ptr = JNI_TRUE;
    else
        *is_native_ptr = JNI_FALSE;

    return JVMTI_ERROR_NONE;
}


/* IsMethodSynthetic ***********************************************************

   return a value indicating whether the method is synthetic. Synthetic methods 
   are generated by the compiler but not present in the original source code.

*******************************************************************************/

static jvmtiError
IsMethodSynthetic (jvmtiEnv * env, jmethodID method,
		   jboolean * is_synthetic_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_get_synthetic_attribute)

  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* GetLoadedClasses ************************************************************

   Return an array of all classes loaded in the virtual machine.

*******************************************************************************/

static jvmtiError
GetLoadedClasses (jvmtiEnv * env, jint * class_count_ptr, jclass ** classes_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    if (class_count_ptr == NULL)
		return JVMTI_ERROR_NULL_POINTER;

    if (classes_ptr == NULL)
		return JVMTI_ERROR_NULL_POINTER;

	classcache_jvmti_GetLoadedClasses(class_count_ptr, classes_ptr);

    return JVMTI_ERROR_NONE;
}


/* GetClassLoaderClasses *******************************************************

   Returns an array of those classes for which this class loader has been 
   recorded as an initiating loader.

*******************************************************************************/

static jvmtiError
GetClassLoaderClasses (jvmtiEnv * env, jobject initiating_loader,
		       jint * class_count_ptr, jclass ** classes_ptr)
{
	log_text("GetClassLoaderClasses called");

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    if ((class_count_ptr == NULL) || (classes_ptr == NULL)) 
		return JVMTI_ERROR_NULL_POINTER;
        
	/* behave like jdk 1.1 and make no distinction between initiating and 
	   defining class loaders */
	
    return GetLoadedClasses(env, class_count_ptr, classes_ptr);
}


/* PopFrame *******************************************************************

   

*******************************************************************************/

static jvmtiError
PopFrame (jvmtiEnv * env, jthread thread)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_pop_frame)
        
		log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* RedefineClasses ************************************************************

   

*******************************************************************************/

static jvmtiError
RedefineClasses (jvmtiEnv * env, jint class_count,
		 const jvmtiClassDefinition * class_definitions)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_redefine_classes)    
	CHECK_CAPABILITY(env,can_redefine_any_class)

  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* GetVersionNumber ***********************************************************

   Return the JVM TI version identifier.   

*******************************************************************************/

static jvmtiError
GetVersionNumber (jvmtiEnv * env, jint * version_ptr)
{
    if (version_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

    *version_ptr = JVMTI_VERSION;
    
    return JVMTI_ERROR_NONE;
}


/* GetCapabilities ************************************************************

   Returns the optional JVM TI features which this environment currently 
   possesses.

*******************************************************************************/

static jvmtiError
GetCapabilities (jvmtiEnv * env, jvmtiCapabilities * capabilities_ptr)
{
    if (capabilities_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

    memcpy(capabilities_ptr, &(((environment*) env)->capabilities), sizeof(JVMTI_Capabilities));

    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
GetSourceDebugExtension (jvmtiEnv * env, jclass klass,
			 char **source_debug_extension_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_get_source_debug_extension)
        
    log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* IsMethodObsolete ************************************************************

   Determine if a method ID refers to an obsolete method version. 

*******************************************************************************/

static jvmtiError
IsMethodObsolete (jvmtiEnv * env, jmethodID method,
		  jboolean * is_obsolete_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_redefine_classes)        

    log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* SuspendThreadList **********************************************************
   
   Suspend all threads in the request list.

*******************************************************************************/

static jvmtiError
SuspendThreadList (jvmtiEnv * env, jint request_count,
		   const jthread * request_list, jvmtiError * results)
{
	int i;
	int suspendme = -1;
	jthread me;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_suspend);
    
	if (request_count<0) return JVMTI_ERROR_ILLEGAL_ARGUMENT;
	if ((request_list == NULL) || (results == NULL)) 
		return JVMTI_ERROR_NULL_POINTER;

	me = jvmti_get_current_thread();

	for (i=0;i<request_count;i++) {
		if (request_list[i] == me) 
			suspendme = i;
		else 
			results[i]=SuspendThread(env, request_list[i]);
	}

	if (suspendme != -1) 
		results[suspendme]=SuspendThread(env, request_list[suspendme]);

    return JVMTI_ERROR_NONE;
}


/* ResumeThreadList ***********************************************************

   Resumes all threads in the request list.   

*******************************************************************************/

static jvmtiError
ResumeThreadList (jvmtiEnv * env, jint request_count,
		  const jthread * request_list, jvmtiError * results)
{
	int i;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    CHECK_CAPABILITY(env,can_suspend);
    
	if (request_count<0) return JVMTI_ERROR_ILLEGAL_ARGUMENT;
	if ((request_list == NULL) || (results == NULL)) 
		return JVMTI_ERROR_NULL_POINTER;

	for (i=0;i<request_count;i++) 
			results[i]=ResumeThread(env, request_list[i]);

    return JVMTI_ERROR_NONE;
}


/* GetStackTrace **************************************************************

   Get information about the stack of a thread

*******************************************************************************/

static jvmtiError
GetStackTrace (jvmtiEnv * env, jthread thread, jint start_depth,
	       jint max_frame_count, jvmtiFrameInfo * frame_buffer,
	       jint * count_ptr)
{
	stacktracebuffer* trace;
	jvmtiError er;
	int i,j;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    
	if (thread != NULL){
		if(!builtin_instanceof(thread,class_java_lang_Thread))
			return JVMTI_ERROR_INVALID_THREAD;

		CHECK_THREAD_IS_ALIVE(thread);
	}

	if((count_ptr == NULL)||(frame_buffer == NULL)) 
		return JVMTI_ERROR_NULL_POINTER;

	if (max_frame_count <0) return JVMTI_ERROR_ILLEGAL_ARGUMENT;

	er = getcacaostacktrace(&trace, thread);
	if (er == JVMTI_ERROR_NONE) {
		heap_free(trace);
		return er;
	}

	if ((trace->used >= start_depth) || ((trace->used * -1) > start_depth)) 
		return JVMTI_ERROR_ILLEGAL_ARGUMENT; 
	
	for (i=start_depth, j=0;i<trace->used;i++,j++) {
		frame_buffer[j].method = (jmethodID)trace->entries[i].method;
        /* XXX todo: location BCI/MachinePC not avilable - Linenumber not expected */
		frame_buffer[j].location = 0;
		}

	heap_free(trace);
	
    return JVMTI_ERROR_NONE;
}


/* GetThreadListStackTraces ***************************************************

   Get information about the stacks of the supplied threads.

*******************************************************************************/

static jvmtiError
GetThreadListStackTraces (jvmtiEnv * env, jint thread_count,
			  const jthread * thread_list,
			  jint max_frame_count,
			  jvmtiStackInfo ** stack_info_ptr)
{
	int i;
	jvmtiError er;
	
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
	if ((stack_info_ptr == NULL)||(thread_list == NULL)) 
		return JVMTI_ERROR_NULL_POINTER;

	if (thread_count < 0) return JVMTI_ERROR_ILLEGAL_ARGUMENT;

	if (max_frame_count < 0) return JVMTI_ERROR_ILLEGAL_ARGUMENT;

	*stack_info_ptr = (jvmtiStackInfo*) 
		heap_allocate(sizeof(jvmtiStackInfo) * thread_count, true, NULL);

	for(i=0; i<thread_count; i++) { /* fill in stack info sturcture array */
		(*stack_info_ptr)[i].thread=thread_list[i];
		GetThreadState(env,thread_list[i],&((*stack_info_ptr)[i].state));
		(*stack_info_ptr)[i].frame_buffer = 
			heap_allocate(sizeof(jvmtiFrameInfo) * max_frame_count,true,NULL);
		er = GetStackTrace(env,thread_list[i],0,max_frame_count,
						   (*stack_info_ptr)[i].frame_buffer,
						   &((*stack_info_ptr)[i].frame_count));

		if (er != JVMTI_ERROR_NONE) return er;
	}

    return JVMTI_ERROR_NONE;
}


/* GetAllStackTraces **********************************************************

   Get stack traces of all live threads

*******************************************************************************/

static jvmtiError
GetAllStackTraces (jvmtiEnv * env, jint max_frame_count,
		   jvmtiStackInfo ** stack_info_ptr, jint * thread_count_ptr)
{
	jthread *threads_ptr;
	jvmtiError er;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    
	if (thread_count_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;
    
	if (JVMTI_ERROR_NONE!=GetAllThreads(env,thread_count_ptr,&threads_ptr))
		return JVMTI_ERROR_INTERNAL;

	GetThreadListStackTraces(env, *thread_count_ptr, threads_ptr,
							 max_frame_count, stack_info_ptr);

	if (er != JVMTI_ERROR_NONE) return er;

    return JVMTI_ERROR_NONE;
}


/* GetThreadLocalStorage ******************************************************

   Get the value of the JVM TI thread-local storage.

*******************************************************************************/

static jvmtiError
GetThreadLocalStorage (jvmtiEnv * env, jthread thread, void **data_ptr)
{
	jvmtiThreadLocalStorage *tls;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if(thread == NULL)
		thread = (jthread) THREADOBJECT;
	else {
		if (!builtin_instanceof(thread,class_java_lang_Thread)) 
			return JVMTI_ERROR_INVALID_THREAD;
		CHECK_THREAD_IS_ALIVE(thread);
	}

	if(data_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

	tls = ((environment*)env)->tls;
	while ((tls->thread != thread) && (tls != NULL)) {
		tls = tls->next;
	}
	
	if (tls == NULL) return JVMTI_ERROR_INTERNAL; /* env/thread pair not found */
	
	*data_ptr = tls->data;
	
    return JVMTI_ERROR_NONE;
}


/* SetThreadLocalStorage *******************************************************

   Stores a pointer value associated with each environment-thread pair. The 
   value is NULL unless set with this function. Agents can allocate memory in 
   which they store thread specific information

*******************************************************************************/

jvmtiError
SetThreadLocalStorage (jvmtiEnv * jenv, jthread thread, const void *data)
{
	jvmtiThreadLocalStorage *tls, *pre;
	environment* env = (environment*)jenv;

	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
	if(thread == NULL)
		thread = (jthread) THREADOBJECT;
	else {
		if (!builtin_instanceof(thread,class_java_lang_Thread)) 
			return JVMTI_ERROR_INVALID_THREAD;
		CHECK_THREAD_IS_ALIVE(thread);
	}
	
	if (env->tls == NULL) {
		tls = env->tls = heap_allocate(sizeof(jvmtiThreadLocalStorage),true,NULL);
	} else {
		tls = env->tls;
		while ((tls->thread != thread) && (tls->next != NULL)) {
			tls = tls->next;
		}
		if (tls->thread != thread) {
			tls->next = heap_allocate(sizeof(jvmtiThreadLocalStorage),true,NULL);
			tls = tls->next;
		}
	}
	
	if (data != NULL) {
		tls->data = (void*)data;
	} else { 
		/* remove current tls */
		pre = env->tls;
		while (pre->next == tls) pre = pre->next;
		pre->next = tls->next;
	}
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
GetTag (jvmtiEnv * env, jobject object, jlong * tag_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_tag_objects)
    
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}

/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
SetTag (jvmtiEnv * env, jobject object, jlong tag)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_tag_objects)
        
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* ForceGarbageCollection *****************************************************

   Force boehm-gc to perform a garbage collection

*******************************************************************************/

static jvmtiError
ForceGarbageCollection (jvmtiEnv * env)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	gc_call();        

    return JVMTI_ERROR_NONE;
}


/* IterateOverObjectsReachableFromObject **************************************

   

*******************************************************************************/

static jvmtiError
IterateOverObjectsReachableFromObject (jvmtiEnv * env, jobject object,
				       jvmtiObjectReferenceCallback
				       object_reference_callback,
				       void *user_data)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_tag_objects)
        
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* IterateOverReachableObjects ************************************************

   

*******************************************************************************/

static jvmtiError
IterateOverReachableObjects (jvmtiEnv * env, jvmtiHeapRootCallback
			     heap_root_callback,
			     jvmtiStackReferenceCallback
			     stack_ref_callback,
			     jvmtiObjectReferenceCallback
			     object_ref_callback, void *user_data)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_tag_objects)
    
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* IterateOverHeap ************************************************************

   

*******************************************************************************/

static jvmtiError
IterateOverHeap (jvmtiEnv * env, jvmtiHeapObjectFilter object_filter,
		 jvmtiHeapObjectCallback heap_object_callback,
		 void *user_data)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_tag_objects)
    
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* IterateOverInstancesOfClass ************************************************

   

*******************************************************************************/

static jvmtiError
IterateOverInstancesOfClass (jvmtiEnv * env, jclass klass,
			     jvmtiHeapObjectFilter object_filter,
			     jvmtiHeapObjectCallback
			     heap_object_callback, void *user_data)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_tag_objects)
   
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   

*******************************************************************************/

static jvmtiError
GetObjectsWithTags (jvmtiEnv * env, jint tag_count, const jlong * tags,
		    jint * count_ptr, jobject ** object_result_ptr,
		    jlong ** tag_result_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_tag_objects)
        
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}


/* SetJNIFunctionTable **********************************************************

   Set the JNI function table in all current and future JNI environments

*******************************************************************************/

static jvmtiError
SetJNIFunctionTable (jvmtiEnv * env,
		     const jniNativeInterface * function_table)
{ 
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;;
    
    if (function_table == NULL) return JVMTI_ERROR_NULL_POINTER;
    _Jv_env->env = (void*)heap_allocate(sizeof(jniNativeInterface),true,NULL);
    memcpy((void*)_Jv_env->env, function_table, sizeof(jniNativeInterface));
    return JVMTI_ERROR_NONE;
}


/* GetJNIFunctionTable *********************************************************

   Get the JNI function table. The JNI function table is copied into allocated 
   memory.

*******************************************************************************/

static jvmtiError
GetJNIFunctionTable (jvmtiEnv * env, jniNativeInterface ** function_table)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    if (function_table == NULL) return JVMTI_ERROR_NULL_POINTER;
    *function_table = (jniNativeInterface*)
        heap_allocate(sizeof(jniNativeInterface),true,NULL);
    memcpy(*function_table, _Jv_env->env, sizeof(jniNativeInterface));
    return JVMTI_ERROR_NONE;
}


/* SetEventCallbacks **********************************************************

   Set the functions to be called for each event. The callbacks are specified 
   by supplying a replacement function table.

*******************************************************************************/

static jvmtiError
SetEventCallbacks (jvmtiEnv * env,
		   const jvmtiEventCallbacks * callbacks,
		   jint size_of_callbacks)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    if (size_of_callbacks < 0) return JVMTI_ERROR_ILLEGAL_ARGUMENT;


	if (callbacks == NULL) { /* remove the existing callbacks */
        memset(&(((environment* )env)->callbacks), 0, 
			   sizeof(jvmtiEventCallbacks));
    }

    memcpy (&(((environment* )env)->callbacks),callbacks,size_of_callbacks);

    return JVMTI_ERROR_NONE;
}


/* GenerateEvents *************************************************************

   Generate events (CompiledMethodLoad and DynamicCodeGenerated) to represent 
   the current state of the VM.

*******************************************************************************/

static jvmtiError
GenerateEvents (jvmtiEnv * env, jvmtiEvent event_type)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
 	CHECK_CAPABILITY(env,can_generate_compiled_method_load_events);

    return JVMTI_ERROR_NONE;
}


/* GetExtensionFunctions ******************************************************

   Returns the set of extension functions.

*******************************************************************************/

static jvmtiError
GetExtensionFunctions (jvmtiEnv * env, jint * extension_count_ptr,
		       jvmtiExtensionFunctionInfo ** extensions)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if ((extension_count_ptr == NULL)||(extensions == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

    /* cacao has no extended functions yet */
    *extension_count_ptr = 0;

    return JVMTI_ERROR_NONE;
}


/* GetExtensionEvents *********************************************************

   Returns the set of extension events.

*******************************************************************************/

static jvmtiError
GetExtensionEvents (jvmtiEnv * env, jint * extension_count_ptr,
		    jvmtiExtensionEventInfo ** extensions)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if ((extension_count_ptr == NULL)||(extensions == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

    /* cacao has no extended events yet */
    *extension_count_ptr = 0;

    return JVMTI_ERROR_NONE;
}


/* SetExtensionEventCallback **************************************************

   Sets the callback function for an extension event and enables the event.

*******************************************************************************/

static jvmtiError
SetExtensionEventCallback (jvmtiEnv * env, jint extension_event_index,
			   jvmtiExtensionEvent callback)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    /* cacao has no extended events yet */
    return JVMTI_ERROR_ILLEGAL_ARGUMENT;
}


/* DisposeEnvironment **********************************************************

   Shutdown a JVM TI connection created with JNI GetEnv.

*******************************************************************************/

static jvmtiError
DisposeEnvironment (jvmtiEnv * env)
{
	environment* cacao_env = (environment*)env;
	environment* tenvs = envs;
	jvmtiThreadLocalStorage *jtls, *tjtls;

	if (tenvs != cacao_env) {
		while (tenvs->next != cacao_env) {
			tenvs = tenvs->next;
		}
		tenvs->next = cacao_env->next;
	} else
		envs = NULL;

	cacao_env->env=NULL;
    memset(&(cacao_env->callbacks),0,sizeof(jvmtiEventCallbacks)*
		   (JVMTI_EVENT_END_ENUM-JVMTI_EVENT_START_ENUM));
    memset(cacao_env->events,0,sizeof(jvmtiEventModeLL)*
		   (JVMTI_EVENT_END_ENUM-JVMTI_EVENT_START_ENUM));
    cacao_env->EnvironmentLocalStorage = NULL;

	jtls = cacao_env->tls;
	while (jtls != NULL) {
		tjtls = jtls;
		jtls = jtls->next;
		tjtls->next = NULL;
	}
	cacao_env->tls = NULL;


	jvmti_cacaodbgserver_quit();

    /* let the GC do the rest */
    return JVMTI_ERROR_NONE;
}


/* GetErrorName ***************************************************************

   Return the symbolic name for an error code.

*******************************************************************************/

#define COPY_RESPONSE(name_ptr,str) *name_ptr = (char*) heap_allocate(sizeof(str),true,NULL); \
                                    memcpy(*name_ptr, &str, sizeof(str)); \
                                    break

static jvmtiError
GetErrorName (jvmtiEnv * env, jvmtiError error, char **name_ptr)
{
    if (name_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

    switch (error) {
    case JVMTI_ERROR_NONE : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_NONE");
    case JVMTI_ERROR_NULL_POINTER : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_NULL_POINTER"); 
    case JVMTI_ERROR_OUT_OF_MEMORY : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_OUT_OF_MEMORY");
    case JVMTI_ERROR_ACCESS_DENIED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_ACCESS_DENIED");
    case JVMTI_ERROR_UNATTACHED_THREAD : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_UNATTACHED_THREAD");
    case JVMTI_ERROR_INVALID_ENVIRONMENT : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_ENVIRONMENT"); 
    case JVMTI_ERROR_WRONG_PHASE : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_WRONG_PHASE");
    case JVMTI_ERROR_INTERNAL : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INTERNAL");
    case JVMTI_ERROR_INVALID_PRIORITY : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_PRIORITY");
    case JVMTI_ERROR_THREAD_NOT_SUSPENDED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_THREAD_NOT_SUSPENDED");
    case JVMTI_ERROR_THREAD_SUSPENDED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_THREAD_SUSPENDED");
    case JVMTI_ERROR_THREAD_NOT_ALIVE : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_THREAD_NOT_ALIVE");
    case JVMTI_ERROR_CLASS_NOT_PREPARED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_CLASS_NOT_PREPARED");
    case JVMTI_ERROR_NO_MORE_FRAMES : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_NO_MORE_FRAMES");
    case JVMTI_ERROR_OPAQUE_FRAME : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_OPAQUE_FRAME");
    case JVMTI_ERROR_DUPLICATE : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_DUPLICATE");
    case JVMTI_ERROR_NOT_FOUND : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_NOT_FOUND");
    case JVMTI_ERROR_NOT_MONITOR_OWNER : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_NOT_MONITOR_OWNER");
    case JVMTI_ERROR_INTERRUPT : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INTERRUPT");
    case JVMTI_ERROR_UNMODIFIABLE_CLASS : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_UNMODIFIABLE_CLASS");
    case JVMTI_ERROR_NOT_AVAILABLE : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_NOT_AVAILABLE");
    case JVMTI_ERROR_ABSENT_INFORMATION : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_ABSENT_INFORMATION");
    case JVMTI_ERROR_INVALID_EVENT_TYPE : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_EVENT_TYPE");
    case JVMTI_ERROR_NATIVE_METHOD : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_NATIVE_METHOD");
    case JVMTI_ERROR_INVALID_THREAD : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_THREAD");
    case JVMTI_ERROR_INVALID_FIELDID : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_FIELDID");
    case JVMTI_ERROR_INVALID_METHODID : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_METHODID");
    case JVMTI_ERROR_INVALID_LOCATION : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_LOCATION");
    case JVMTI_ERROR_INVALID_OBJECT : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_OBJECT");
    case JVMTI_ERROR_INVALID_CLASS : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_CLASS");
    case JVMTI_ERROR_TYPE_MISMATCH : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_TYPE_MISMATCH");
    case JVMTI_ERROR_INVALID_SLOT : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_SLOT");
    case JVMTI_ERROR_MUST_POSSESS_CAPABILITY : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_MUST_POSSESS_CAPABILITY");
    case JVMTI_ERROR_INVALID_THREAD_GROUP : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_THREAD_GROUP");
    case JVMTI_ERROR_INVALID_MONITOR : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_MONITOR");
    case JVMTI_ERROR_ILLEGAL_ARGUMENT : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_ILLEGAL_ARGUMENT");
    case JVMTI_ERROR_INVALID_TYPESTATE : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_TYPESTATE");
    case JVMTI_ERROR_UNSUPPORTED_VERSION : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_UNSUPPORTED_VERSION");
    case JVMTI_ERROR_INVALID_CLASS_FORMAT : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_INVALID_CLASS_FORMAT");
    case JVMTI_ERROR_CIRCULAR_CLASS_DEFINITION : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_CIRCULAR_CLASS_DEFINITION");
    case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_ADDED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_ADDED");
    case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED");
    case JVMTI_ERROR_FAILS_VERIFICATION : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_FAILS_VERIFICATION");
    case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED");
    case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED");
    case JVMTI_ERROR_NAMES_DONT_MATCH : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_NAMES_DONT_MATCH");
    case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_MODIFIERS_CHANGED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_MODIFIERS_CHANGED");
    case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_MODIFIERS_CHANGED : 
        COPY_RESPONSE (name_ptr, "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_MODIFIERS_CHANGED");
    default:
        return JVMTI_ERROR_ILLEGAL_ARGUMENT;
    }
    return JVMTI_ERROR_NONE;
}

/* GetJLocationFormat **********************************************************

   This function describes the representation of jlocation used in this VM.

*******************************************************************************/

static jvmtiError
GetJLocationFormat (jvmtiEnv * env, jvmtiJlocationFormat * format_ptr)
{
    *format_ptr = JVMTI_JLOCATION_OTHER;
    return JVMTI_ERROR_NONE;
}


/* GetSystemProperties ********************************************************

   The list of VM system property keys which may be used with GetSystemProperty 
   is returned.

*******************************************************************************/

static jvmtiError
GetSystemProperties (jvmtiEnv * env, jint * count_ptr, char ***property_ptr)
{
	jmethodID mid, moremid;
    classinfo *sysclass, *propclass, *enumclass;
    java_objectheader *sysprop, *keys, *obj;
    char* ch;
    int i;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    if ((count_ptr == NULL) || (property_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

    sysclass = load_class_from_sysloader(Utf8String::from_utf8("java/lang/System"));

    if (!sysclass) throw_main_exception_exit();

    mid = (jmethodID)class_resolvemethod(sysclass,
                                         Utf8String::from_utf8("getProperties"),
                                         Utf8String::from_utf8("()Ljava/util/Properties;"));
    if (!mid) throw_main_exception_exit();


    sysprop = _Jv_JNINativeInterface.CallStaticObjectMethod(NULL, sysclass, mid);
    if (!sysprop) throw_main_exception_exit();

    propclass = sysprop->vftbl->class;

    mid = (jmethodID)class_resolvemethod(propclass, 
                                         Utf8String::from_utf8("size"),
                                         Utf8String::from_utf8("()I"));
    if (!mid) throw_main_exception_exit();

    *count_ptr = 
        _Jv_JNINativeInterface.CallIntMethod(NULL, sysprop, mid);
    *property_ptr = heap_allocate(sizeof(char*) * (*count_ptr) ,true,NULL);

    mid = (jmethodID)class_resolvemethod(propclass, 
                                         Utf8String::from_utf8("keys"),
                                         Utf8String::from_utf8("()Ljava/util/Enumeration;"));
    if (!mid) throw_main_exception_exit();

    keys = _Jv_JNINativeInterface.CallObjectMethod(NULL, sysprop, mid);
    enumclass = keys->vftbl->class;
        
    moremid = (jmethodID)class_resolvemethod(enumclass, 
                                             Utf8String::from_utf8("hasMoreElements"),
                                             Utf8String::from_utf8("()Z"));
    if (!moremid) throw_main_exception_exit();

    mid = (jmethodID)class_resolvemethod(propclass,
                                         Utf8String::from_utf8("nextElement"),
                                         Utf8String::from_utf8("()Ljava/lang/Object;"));
    if (!mid) throw_main_exception_exit();

    i = 0;
    while (_Jv_JNINativeInterface.CallBooleanMethod(NULL,keys,(jmethodID)moremid)) {
        obj = _Jv_JNINativeInterface.CallObjectMethod(NULL, keys, mid);
        ch = JavaString(obj).to_chars();
        *property_ptr[i] = heap_allocate(sizeof(char*) * strlen (ch),true,NULL);
        memcpy(*property_ptr[i], ch, strlen (ch));
        MFREE(ch,char,strlen(ch)+1);
        i++;
    }

    return JVMTI_ERROR_NONE;
}


/* GetSystemProperty **********************************************************

   Return a VM system property value given the property key.

*******************************************************************************/

static jvmtiError
GetSystemProperty (jvmtiEnv * env, const char *property, char **value_ptr)
{
    jmethodID mid;
    classinfo *sysclass, *propclass;
    java_objectheader *sysprop, *obj;
    char* ch;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

    if ((value_ptr == NULL) || (property == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

    sysclass = load_class_from_sysloader(Utf8String::from_utf8("java/lang/System"));
    if (!sysclass) throw_main_exception_exit();

    mid = (jmethodID)class_resolvemethod(sysclass, 
                                         Utf8String::from_utf8("getProperties"),
                                         Utf8String::from_utf8("()Ljava/util/Properties;"));
    if (!mid) throw_main_exception_exit();

    sysprop = _Jv_JNINativeInterface.CallStaticObjectMethod(NULL, (jclass)sysclass, mid);

    propclass = sysprop->vftbl->class;

    mid = (jmethodID)class_resolvemethod(propclass, 
                                         Utf8String::from_utf8("getProperty"),
                                         Utf8String::from_utf8("(Ljava/lang/String;)Ljava/lang/String;"));
    if (!mid) throw_main_exception_exit();

    obj = (java_objectheader*)_Jv_JNINativeInterface.CallObjectMethod(
        NULL, sysprop, mid, JavaString::from_utf8(property));
    if (!obj) return JVMTI_ERROR_NOT_AVAILABLE;

    ch = JavaString(obj).to_chars();
    *value_ptr = heap_allocate(sizeof(char*) * strlen (ch),true,NULL);
    memcpy(*value_ptr, ch, strlen (ch));
    MFREE(ch,char,strlen(ch)+1);       

    return JVMTI_ERROR_NONE;
}


/* SetSystemProperty **********************************************************

   Set a VM system property value.

*******************************************************************************/

static jvmtiError
SetSystemProperty (jvmtiEnv * env, const char *property, const char *value)
{
    jmethodID mid;
    classinfo *sysclass, *propclass;
    java_objectheader *sysprop;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE_END;
        
    if (property == NULL) return JVMTI_ERROR_NULL_POINTER;
    if (value == NULL) return JVMTI_ERROR_NOT_AVAILABLE;

    sysclass = load_class_from_sysloader(Utf8String::from_utf8("java/lang/System"));
    if (!sysclass) throw_main_exception_exit();

    mid = (jmethodID)class_resolvemethod(sysclass, 
                                         Utf8String::from_utf8("getProperties"),
                                         Utf8String::from_utf8("()Ljava/util/Properties;"));
    if (!mid) throw_main_exception_exit();

    sysprop = _Jv_JNINativeInterface.CallStaticObjectMethod(NULL, (jclass)sysclass, mid);

    propclass = sysprop->vftbl->class;

    mid = (jmethodID)class_resolvemethod(propclass, 
                                         Utf8String::from_utf8("setProperty"),
                                         Utf8String::from_utf8("(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;"));
    if (!mid) throw_main_exception_exit();

    _Jv_JNINativeInterface.CallObjectMethod(NULL, 
                                            sysprop, 
                                            mid, 
                                            JavaString::from_utf8(property), 
                                            JavaString::from_utf8(value));
    
    return JVMTI_ERROR_NONE;
}

/* GetPhase ********************************************************************

   Return the current phase of VM execution

*******************************************************************************/

static jvmtiError
GetPhase (jvmtiEnv * env, jvmtiPhase * phase_ptr)
{
    if (phase_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;
    
    *phase_ptr = phase;

    return JVMTI_ERROR_NONE;
}

/* GetCurrentThreadCpuTimerInfo ************************************************

   

*******************************************************************************/

static jvmtiError
GetCurrentThreadCpuTimerInfo (jvmtiEnv * env, jvmtiTimerInfo * info_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_get_current_thread_cpu_time)     

  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");

    return JVMTI_ERROR_NONE;
}

/* GetCurrentThreadCpuTime ****************************************************

   

*******************************************************************************/

static jvmtiError
GetCurrentThreadCpuTime (jvmtiEnv * env, jlong * nanos_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_get_current_thread_cpu_time)     
        
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}

/* GetThreadCpuTimerInfo ******************************************************

   

*******************************************************************************/

static jvmtiError
GetThreadCpuTimerInfo (jvmtiEnv * env, jvmtiTimerInfo * info_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_get_thread_cpu_time)
    
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}

/* GetThreadCpuTime ***********************************************************

   

*******************************************************************************/

static jvmtiError
GetThreadCpuTime (jvmtiEnv * env, jthread thread, jlong * nanos_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
	CHECK_CAPABILITY(env,can_get_thread_cpu_time)        
  log_text ("JVMTI-Call: OPTIONAL IMPLEMENT ME!!!");
    return JVMTI_ERROR_NONE;
}

/* GetTimerInfo ***************************************************************

   Get information about the GetTime timer.    

*******************************************************************************/

static jvmtiError
GetTimerInfo (jvmtiEnv * env, jvmtiTimerInfo * info_ptr)
{
    if (info_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

    info_ptr->max_value = !0x0;
	info_ptr->may_skip_forward = true;
	info_ptr->may_skip_backward = true;
	info_ptr->kind = JVMTI_TIMER_TOTAL_CPU;
   
    return JVMTI_ERROR_NONE;
}

/* GetTime ********************************************************************

   Return the current value of the system timer, in nanoseconds

*******************************************************************************/

static jvmtiError
GetTime (jvmtiEnv * env, jlong * nanos_ptr)
{
    /* Note: this implementation copied directly from Japhar's, by Chris Toshok. */
    struct timeval tp;
    
    if (nanos_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

    if (gettimeofday (&tp, NULL) == -1)
        _Jv_JNINativeInterface.FatalError (NULL, "gettimeofday call failed.");
    
    *nanos_ptr = (jlong) tp.tv_sec;
    *nanos_ptr *= 1000;
    *nanos_ptr += (tp.tv_usec / 1000);

    return JVMTI_ERROR_NONE;
}

/* GetPotentialCapabilities ***************************************************

   Returns the JVM TI features that can potentially be possessed by this 
   environment at this time.

*******************************************************************************/

static jvmtiError
GetPotentialCapabilities (jvmtiEnv * env, jvmtiCapabilities * capabilities_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if (capabilities_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

    memcpy(capabilities_ptr, &JVMTI_Capabilities, sizeof(JVMTI_Capabilities));

    return JVMTI_ERROR_NONE;
}


#define CHECK_ADD_CAPABILITY(env,CAN)          \
        if (capabilities_ptr->CAN == 1) {      \
           if (JVMTI_Capabilities.CAN == 0)    \
             return JVMTI_ERROR_NOT_AVAILABLE; \
           else                                \
             env->capabilities.CAN = 1;        \
        }                                     

/* AddCapabilities ************************************************************

   Set new capabilities by adding the capabilities pointed to by 
   capabilities_ptr. All previous capabilities are retained.

*******************************************************************************/

static jvmtiError
AddCapabilities (jvmtiEnv * env, const jvmtiCapabilities * capabilities_ptr)
{
    environment* cacao_env;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if ((env == NULL) || (capabilities_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;
    
    cacao_env = (environment*)env;
    
    CHECK_ADD_CAPABILITY(cacao_env,can_tag_objects)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_field_modification_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_field_access_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_bytecodes)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_synthetic_attribute)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_owned_monitor_info)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_current_contended_monitor)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_monitor_info)
    CHECK_ADD_CAPABILITY(cacao_env,can_pop_frame)
    CHECK_ADD_CAPABILITY(cacao_env,can_redefine_classes)
    CHECK_ADD_CAPABILITY(cacao_env,can_signal_thread)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_source_file_name)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_line_numbers)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_source_debug_extension)
    CHECK_ADD_CAPABILITY(cacao_env,can_access_local_variables)
    CHECK_ADD_CAPABILITY(cacao_env,can_maintain_original_method_order)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_single_step_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_exception_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_frame_pop_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_breakpoint_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_suspend)
    CHECK_ADD_CAPABILITY(cacao_env,can_redefine_any_class)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_current_thread_cpu_time)
    CHECK_ADD_CAPABILITY(cacao_env,can_get_thread_cpu_time)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_method_entry_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_method_exit_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_all_class_hook_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_compiled_method_load_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_monitor_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_vm_object_alloc_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_native_method_bind_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_garbage_collection_events)
    CHECK_ADD_CAPABILITY(cacao_env,can_generate_object_free_events)


    return JVMTI_ERROR_NONE;    
}


#define CHECK_DEL_CAPABILITY(env,CAN)      \
        if (capabilities_ptr->CAN == 1) \
           env->capabilities.CAN = 0;

/* RelinquishCapabilities *****************************************************

   Relinquish the capabilities pointed to by capabilities_ptr.

*******************************************************************************/

static jvmtiError
RelinquishCapabilities (jvmtiEnv * env,
			const jvmtiCapabilities * capabilities_ptr)
{
    environment* cacao_env;
    
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
        
    if ((env == NULL) || (capabilities_ptr == NULL)) 
        return JVMTI_ERROR_NULL_POINTER;

    cacao_env = (environment*)env;

    CHECK_DEL_CAPABILITY(cacao_env,can_tag_objects)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_field_modification_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_field_access_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_bytecodes)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_synthetic_attribute)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_owned_monitor_info)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_current_contended_monitor)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_monitor_info)
    CHECK_DEL_CAPABILITY(cacao_env,can_pop_frame)
    CHECK_DEL_CAPABILITY(cacao_env,can_redefine_classes)
    CHECK_DEL_CAPABILITY(cacao_env,can_signal_thread)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_source_file_name)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_line_numbers)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_source_debug_extension)
    CHECK_DEL_CAPABILITY(cacao_env,can_access_local_variables)
    CHECK_DEL_CAPABILITY(cacao_env,can_maintain_original_method_order)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_single_step_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_exception_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_frame_pop_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_breakpoint_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_suspend)
    CHECK_DEL_CAPABILITY(cacao_env,can_redefine_any_class)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_current_thread_cpu_time)
    CHECK_DEL_CAPABILITY(cacao_env,can_get_thread_cpu_time)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_method_entry_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_method_exit_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_all_class_hook_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_compiled_method_load_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_monitor_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_vm_object_alloc_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_native_method_bind_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_garbage_collection_events)
    CHECK_DEL_CAPABILITY(cacao_env,can_generate_object_free_events)

    return JVMTI_ERROR_NONE;
}

/* GetAvailableProcessors *****************************************************

   Get number of processors available to the virtual machine.

*******************************************************************************/

static jvmtiError
GetAvailableProcessors (jvmtiEnv * env, jint * processor_count_ptr)
{
	CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;
    
	if (processor_count_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;

	log_text ("GetAvailableProcessors IMPLEMENT ME!!!");
	
	*processor_count_ptr = 1; /* where do I get this ?*/
	
    return JVMTI_ERROR_NONE;
}

/* GetEnvironmentLocalStorage **************************************************

   Called by the agent to get the value of the JVM TI environment-local storage.

*******************************************************************************/

static jvmtiError
GetEnvironmentLocalStorage (jvmtiEnv * env, void **data_ptr)
{
    if ((env == NULL) || (data_ptr == NULL)) return JVMTI_ERROR_NULL_POINTER;

    *data_ptr = ((environment*)env)->EnvironmentLocalStorage;

    return JVMTI_ERROR_NONE;
}

/* SetEnvironmentLocalStorage **************************************************

   The VM stores a pointer value associated with each environment. Agents can 
   allocate memory in which they store environment specific information.

*******************************************************************************/

static jvmtiError
SetEnvironmentLocalStorage (jvmtiEnv * env, const void *data)
{
    if (env == NULL) return JVMTI_ERROR_NULL_POINTER;

    ((environment*)env)->EnvironmentLocalStorage = (void*) data;

    return JVMTI_ERROR_NONE;
}

/* AddToBootstrapClassLoaderSearch ********************************************

   After the bootstrap class loader unsuccessfully searches for a class, the 
   specified platform-dependent search path segment will be searched as well.

*******************************************************************************/

static jvmtiError
AddToBootstrapClassLoaderSearch (jvmtiEnv * env, const char *segment)
{
    char* tmp_bcp;
    int ln;

    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_ONLOAD)
    CHECK_PHASE_END;

    if (segment == NULL) return JVMTI_ERROR_NULL_POINTER;

    ln = strlen(bootclasspath) + strlen(":") + strlen(segment);
    tmp_bcp = MNEW(char, ln);
    strcat(tmp_bcp, bootclasspath);
    strcat(tmp_bcp, ":");
    strcat(tmp_bcp, segment);
    MFREE(bootclasspath,char,ln);
    bootclasspath = tmp_bcp;

    return JVMTI_ERROR_NONE;
}

/* SetVerboseFlag *************************************************************

   Control verbose output. This is the output which typically is sent to stderr

*******************************************************************************/

static jvmtiError
SetVerboseFlag (jvmtiEnv * env, jvmtiVerboseFlag flag, jboolean value)
{
    switch (flag) {
    case JVMTI_VERBOSE_OTHER: 
		/* where is this defined ? 
		   runverbose = value;
		*/
        break;
    case JVMTI_VERBOSE_GC: 
        opt_verbosegc = value;
        break;
    case JVMTI_VERBOSE_CLASS: 
        loadverbose = value;
        break;
    case JVMTI_VERBOSE_JNI: 
        break;
    default:
        return JVMTI_ERROR_ILLEGAL_ARGUMENT;            
    }
    return JVMTI_ERROR_NONE;
}

/* GetObjectSize **************************************************************

   For the object object return the size.

*******************************************************************************/

static jvmtiError
GetObjectSize (jvmtiEnv * env, jobject object, jlong * size_ptr)
{
    CHECK_PHASE_START
    CHECK_PHASE(JVMTI_PHASE_START)
    CHECK_PHASE(JVMTI_PHASE_LIVE)
    CHECK_PHASE_END;

	if (size_ptr == NULL) return JVMTI_ERROR_NULL_POINTER;
	if (!builtin_instanceof(object,class_java_lang_Object))
		return JVMTI_ERROR_INVALID_OBJECT;

	*size_ptr = ((java_objectheader*)object)->vftbl->class->instancesize;

    return JVMTI_ERROR_NONE;
}


/* *****************************************************************************

   Environment variables

*******************************************************************************/

static jvmtiCapabilities JVMTI_Capabilities = {
  0,				/* can_tag_objects */
  0,				/* can_generate_field_modification_events */
  0,				/* can_generate_field_access_events */
  1,				/* can_get_bytecodes */
  0,				/* can_get_synthetic_attribute */

#if defined(ENABLE_THREADS)
  1,				/* can_get_owned_monitor_info */
  1,				/* can_get_current_contended_monitor */
#else
  0,				/* can_get_owned_monitor_info */
  0,				/* can_get_current_contended_monitor */
#endif

  0,				/* can_get_monitor_info */
  0,				/* can_pop_frame */
  0,				/* can_redefine_classes */
  0,				/* can_signal_thread */
  1,				/* can_get_source_file_name */
  1,				/* can_get_line_numbers */
  0,				/* can_get_source_debug_extension */
  0,				/* can_access_local_variables */
  0,				/* can_maintain_original_method_order */
  0,				/* can_generate_single_step_events */
  1,				/* can_generate_exception_events */
  0,				/* can_generate_frame_pop_events */
  1,				/* can_generate_breakpoint_events */
  1,				/* can_suspend */
  0,				/* can_redefine_any_class */
  0,				/* can_get_current_thread_cpu_time */
  0,				/* can_get_thread_cpu_time */
  1,				/* can_generate_method_entry_events */
  0,				/* can_generate_method_exit_events */
  0,				/* can_generate_all_class_hook_events */
  0,				/* can_generate_compiled_method_load_events */
  1,				/* can_generate_monitor_events */
  0,				/* can_generate_vm_object_alloc_events */
  0,				/* can_generate_native_method_bind_events */
  0,				/* can_generate_garbage_collection_events */
  0,				/* can_generate_object_free_events */
};

static struct jvmtiEnv_struct JVMTI_EnvTable = {
    NULL,
    &SetEventNotificationMode,
    NULL,
    &GetAllThreads,
    &SuspendThread,
    &ResumeThread,
    &StopThread,
    &InterruptThread,
    &GetThreadInfo,
    &GetOwnedMonitorInfo,
    &GetCurrentContendedMonitor,
    &RunAgentThread,
    &GetTopThreadGroups,
    &GetThreadGroupInfo,
    &GetThreadGroupChildren,
    &GetFrameCount,
    &GetThreadState,
    NULL,
    &GetFrameLocation,
    &NotifyFramePop,
    &GetLocalObject,
    &GetLocalInt,
    &GetLocalLong,
    &GetLocalFloat,
    &GetLocalDouble,
    &SetLocalObject,
    &SetLocalInt,
    &SetLocalLong,
    &SetLocalFloat,
    &SetLocalDouble,
    &CreateRawMonitor,
    &DestroyRawMonitor,
    &RawMonitorEnter,
    &RawMonitorExit,
    &RawMonitorWait,
    &RawMonitorNotify,
    &RawMonitorNotifyAll,
    &SetBreakpoint,
    &ClearBreakpoint,
    NULL,
    &SetFieldAccessWatch,
    &ClearFieldAccessWatch,
    &SetFieldModificationWatch,
    &ClearFieldModificationWatch,
    NULL,
    &Allocate,
    &Deallocate,
    &GetClassSignature,
    &GetClassStatus,
    &GetSourceFileName,
    &GetClassModifiers,
    &GetClassMethods,
    &GetClassFields,
    &GetImplementedInterfaces,
    &IsInterface,
    &IsArrayClass,
    &GetClassLoader, 
    &GetObjectHashCode, 
    &GetObjectMonitorUsage, 
    &GetFieldName, 
    &GetFieldDeclaringClass, 
    &GetFieldModifiers, 
    &IsFieldSynthetic, 
    &GetMethodName, 
    &GetMethodDeclaringClass, 
    &GetMethodModifiers, 
    NULL,
    &GetMaxLocals, 
    &GetArgumentsSize, 
    &GetLineNumberTable, 
    &GetMethodLocation, 
    &GetLocalVariableTable, 
    NULL,
    NULL,
    &GetBytecodes, 
    &IsMethodNative, 
    &IsMethodSynthetic, 
    &GetLoadedClasses, 
    &GetClassLoaderClasses, 
    &PopFrame, 
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &RedefineClasses, 
    &GetVersionNumber, 
    &GetCapabilities, 
    &GetSourceDebugExtension, 
    &IsMethodObsolete, 
    &SuspendThreadList, 
    &ResumeThreadList, 
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &GetAllStackTraces, 
    &GetThreadListStackTraces, 
    &GetThreadLocalStorage, 
    &SetThreadLocalStorage, 
    &GetStackTrace, 
    NULL,
    &GetTag, 
    &SetTag, 
    &ForceGarbageCollection,
    &IterateOverObjectsReachableFromObject, 
    &IterateOverReachableObjects, 
    &IterateOverHeap, 
    &IterateOverInstancesOfClass, 
    NULL,
    &GetObjectsWithTags, 
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &SetJNIFunctionTable, 
    &GetJNIFunctionTable, 
    &SetEventCallbacks, 
    &GenerateEvents, 
    &GetExtensionFunctions, 
    &GetExtensionEvents, 
    &SetExtensionEventCallback, 
    &DisposeEnvironment,
    &GetErrorName, 
    &GetJLocationFormat, 
    &GetSystemProperties, 
    &GetSystemProperty, 
    &SetSystemProperty, 
    &GetPhase, 
    &GetCurrentThreadCpuTimerInfo, 
    &GetCurrentThreadCpuTime, 
    &GetThreadCpuTimerInfo, 
    &GetThreadCpuTime, 
    &GetTimerInfo, 
    &GetTime, 
    &GetPotentialCapabilities, 
    NULL,
    &AddCapabilities,
    &RelinquishCapabilities,
    &GetAvailableProcessors,
    NULL,
    NULL,
    &GetEnvironmentLocalStorage,
    &SetEnvironmentLocalStorage,
    &AddToBootstrapClassLoaderSearch,
    &SetVerboseFlag,
    NULL,
    NULL,
    NULL,
    &GetObjectSize
};

/* jvmti_set_phase ************************************************************

  sets a new jvmti phase a fires an apropriate event.

*******************************************************************************/

void jvmti_set_phase(jvmtiPhase p) {
	genericEventData d;

	fprintf (stderr,"set JVMTI phase %d\n",p);
	fflush(stderr);

    switch (p) {
    case JVMTI_PHASE_ONLOAD:
		phase = p;
        return;
    case JVMTI_PHASE_PRIMORDIAL:
		phase = p;
        return;
    case JVMTI_PHASE_START: 
		phase = p;
		d.ev = JVMTI_EVENT_VM_START;
        break;
    case JVMTI_PHASE_LIVE: 
		phase = p; 
		d.ev = JVMTI_EVENT_VM_INIT;
		jvmti_fireEvent(&d);
		/* thread start event for main thread */
		d.ev = JVMTI_EVENT_THREAD_START;
		break;
    case JVMTI_PHASE_DEAD:
		phase = p;
		d.ev = JVMTI_EVENT_VM_DEATH;
        break;
	default:
		log_text("wrong jvmti phase to be set");
		exit(1);
    }

	jvmti_fireEvent(&d);
}


/* jvmti_new_environment ******************************************************

  creates a new JVMTI environment

*******************************************************************************/

jvmtiEnv* jvmti_new_environment() {
    environment* env;

	if (envs == NULL) {
		envs = heap_allocate(sizeof(environment),true,NULL);
		env = envs;
	} else {
		env = envs;
		while (env->next != NULL) env = env->next;
		env->next = heap_allocate(sizeof(environment),true,NULL);
		env = env->next;
	}

	env->env = heap_allocate(sizeof(struct jvmtiEnv_struct),true,NULL);
    memcpy(env->env,&JVMTI_EnvTable,sizeof(struct jvmtiEnv_struct));
	memset(&(env->events),JVMTI_DISABLE,(JVMTI_EVENT_END_ENUM - JVMTI_EVENT_START_ENUM)*
		   sizeof(jvmtiEventModeLL));
    /* To possess a capability, the agent must add the capability.*/
    memset(&(env->capabilities), 0, sizeof(jvmtiCapabilities));
    RelinquishCapabilities(&(env->env),&(env->capabilities));
    env->EnvironmentLocalStorage = NULL;
	env->tls = NULL;
	
	/* initialize cacao debugging facilities */
	jvmti_cacao_debug_init();

	return (jvmtiEnv*)env;
}

/* jvmti_agentload ************************************************************

  loads the indicated shared library containing the jvmti agent and calls the
  Agent_OnLoad function.

*******************************************************************************/

void jvmti_agentload(char* opt_arg, bool agentbypath, lt_dlhandle  *handle, char **libname) {
	lt_ptr onload;
	char *arg;
	int i=0,len;
	jint retval;

	len = strlen(opt_arg);
	
	/* separate arguments */

	while ((opt_arg[i] != '=') && (i < len))
		i++;

	opt_arg[i] = '\0';

	if (i < len)
		arg = &opt_arg[i + 1];
	else
		arg = "";

	if (agentbypath) {
		/* -agentpath */

		*libname = GCMNEW(char, i);

		strcpy(*libname, opt_arg);
	}
	else {
		/* -agentlib */

		len = strlen("lib") + i + strlen(".so") + strlen("0");

		*libname = GCMNEW(char, len);

		strcpy(*libname, "lib");
		strcat(*libname, opt_arg);
		strcat(*libname, ".so");
	}

	/* try to open the library */
	lt_dlinit();
	if (!(*handle = lt_dlopen(*libname))) {
		fprintf(stderr,"Could not find agent library: %s (%s)\n",*libname,lt_dlerror());
		vm_shutdown(1);
	}
		
	/* resolve Agent_OnLoad function */
	if (!(onload = lt_dlsym(*handle, "Agent_OnLoad"))) {
		fprintf(stderr,"unable to load Agent_OnLoad function in %s (%s)\n",*libname,lt_dlerror());
		vm_shutdown(1);
	}

	/* resolve Agent_UnLoad function */
	unload = lt_dlsym(*handle, "Agent_Unload");

	retval = 
		((JNIEXPORT jint JNICALL (*) (JavaVM *vm, char *options, void *reserved))
		 onload) ((JavaVM *) _Jv_jvm, arg, NULL);

	if (retval != 0) exit (retval);
}

/* jvmti_agentunload **********************************************************

  calls the Agent_UnLoad function in the jvmti agent if present.

*******************************************************************************/

void jvmti_agentunload() {
	if (unload != NULL) {
		((JNIEXPORT void JNICALL (*) (JavaVM *vm)) unload) 
			((JavaVM*) &_Jv_JNIInvokeInterface);
	}
}


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
 * vim:noexpandtab:sw=4:ts=4:
 */
