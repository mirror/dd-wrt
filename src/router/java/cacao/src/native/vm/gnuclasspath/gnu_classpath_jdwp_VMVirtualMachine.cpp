/* src/native/vm/gnu/gnu_classpath_jdwp_VMVirtualMachine.c - jdwp->jvmti interface

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

#include <stdint.h>
#include <string.h>

#include "toolbox/logging.hpp"
#include "native/jni.hpp"
#include "native/include/java_lang_Thread.h"
#include "native/include/java_nio_ByteBuffer.h"
#include "native/include/java_lang_Class.h"
#include "native/include/java_lang_ClassLoader.h"
#include "native/include/java_lang_reflect_Method.h"
#include "native/include/gnu_classpath_jdwp_event_EventRequest.h"
#include "native/include/gnu_classpath_jdwp_VMVirtualMachine.h"
#include "native/jvmti/jvmti.h"
#include "native/jvmti/VMjdwp.h"


/*
 * Class:     gnu_classpath_jdwp_VMVirtualMachine
 * Method:    suspendThread
 * Signature: (Ljava/lang/Thread;)V
 */
JNIEXPORT void JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_suspendThread(JNIEnv *env, jclass clazz, struct java_lang_Thread* par1)
{
	jvmtiError err;

    err = (*jvmtienv)->SuspendThread(jvmtienv, (jthread) par1);
	printjvmtierror("VMVirtualMachine.suspendThread SuspendThread", err);
}

/*
 * Class:     gnu_classpath_jdwp_VMVirtualMachine
 * Method:    resumeThread
 * Signature: (Ljava/lang/Thread;)V
 */
JNIEXPORT void JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_resumeThread(JNIEnv *env, jclass clazz, struct java_lang_Thread* par1)
{
	jvmtiError err;

    err = (*jvmtienv)->ResumeThread(jvmtienv, (jthread) par1);
	printjvmtierror("VMVirtualMachine.resumethread ResumeThread", err);
}


/*
 * Class:     gnu_classpath_jdwp_VMVirtualMachine
 * Method:    getSuspendCount
 * Signature: (Ljava/lang/Thread;)I
 */
JNIEXPORT int32_t JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getSuspendCount(JNIEnv *env, jclass clazz, struct java_lang_Thread* par1) {
    log_text ("VMVirtualMachine_getSuspendCount: not supported");
	return 1;
}

/*
 * Class:     gnu_classpath_jdwp_VMVirtualMachine
 * Method:    getAllLoadedClassesCount
 * Signature: ()I
 */
JNIEXPORT int32_t JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getAllLoadedClassesCount(JNIEnv *env, jclass clazz) {
    jint count;
    jclass* classes;
	jvmtiError err;

	if (JVMTI_ERROR_NONE != (err= (*jvmtienv)->
		GetLoadedClasses(jvmtienv, &count, &classes))) {
		printjvmtierror("VMVirtualMachine_getAllLoadedClassCount GetLoadedClasses",err);
		return 0;
	}
	(*jvmtienv)->Deallocate(jvmtienv,(unsigned char*)classes);
    return count;
}

/*
 * Class:     gnu_classpath_jdwp_VMVirtualMachine
 * Method:    getAllLoadedClasses
 * Signature: ()Ljava/util/Iterator
 */
JNIEXPORT struct java_util_Iterator* JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getAllLoadedClasses(JNIEnv *env, jclass clazz) {
	jclass *classes, *cl;
	jint classcount;
	jobjectArray joa;
/*	jthrowable e;*/
	jmethodID m;
	jobject *ol,*oi;
	int i;
	jvmtiError err;

	if (JVMTI_ERROR_NONE != (err= (*jvmtienv)->
		GetLoadedClasses(jvmtienv, &classcount, &classes))) {
		printjvmtierror("VMVirtualMachine_getAllLoadedClasses GetLoadedClasses",err);

		/* we should throw JDWP Exception INTERNAL = 113;*/
/*		env->ThrowNew(env,ec,"jvmti error occoured");  */
		return NULL;
	}

	cl = (*env)->FindClass(env,"java.lang.Class");
	if (!cl) return NULL;

	/* Arrays.asList(Object[] classes)->List.Iterator()->Iterator */
	joa = (*env)->NewObjectArray(env, (jsize)classcount, cl , NULL);
	if (!joa) return NULL;

	for (i = 0; i < classcount; i++)
		(*env)->SetObjectArrayElement(env,joa,(jsize)i, (jobject)classes[i]);
	(*jvmtienv)->Deallocate(jvmtienv, (unsigned char*)classes);

	cl = (*env)->FindClass(env,"java.util.Arrays");
	if (!cl) return NULL;

	m = (*env)->GetStaticMethodID(env, cl, "asList", "([Ljava/lang/Object;)Ljava/util/List;");
	if (!m) return NULL;

	ol = (*env)->CallStaticObjectMethod(env,(jclass)cl,m,joa);
	if (!ol) return NULL;

	cl = (*env)->FindClass(env,"java.util.List");
	if (!cl) return NULL;
	m = (*env)->GetMethodID(env,cl,"iterator","()Ljava/util/Iterator;");
	if (!m) return NULL;
	oi = (*env)->CallObjectMethod(env,ol,m);

	return (struct java_util_Iterator*)oi;
}

/* Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    getClassStatus
 * Signature: (Ljava/lang/Class;)I
 */
JNIEXPORT int32_t JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getClassStatus(JNIEnv *env, jclass clazz, struct java_lang_Class* par1) {
	jint status;
	jvmtiError err;

	err = (*jvmtienv)->GetClassStatus(jvmtienv, (jclass) par1, &status);
	printjvmtierror("VMVirtualMachine_getClassStatus GetClassStatus", err);

	return status;
}

/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    getAllClassMethods
 * Signature: (Ljava/lang/Class;)[Lgnu/classpath/jdwp/VMMethod;
 */
JNIEXPORT java_objectarray* JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getAllClassMethods(JNIEnv *env, jclass clazz, struct java_lang_Class* par1) {
    jint count;
    jmethodID* methodID, m;
   	jvmtiError err;

	jclass *cl;
	jobject *ol;
	jobjectArray joa;
	int i;

    if (JVMTI_ERROR_NONE != (err= (*jvmtienv)->
							 GetClassMethods(jvmtienv, (jclass) par1,
											 &count, &methodID))) {
		printjvmtierror("VMVirtualMachine_getAllClassMethods GetClassMethods", err);
		return NULL;
	}

	m = (*env)->
		GetStaticMethodID(env, clazz, "getClassMethod",
						  "(Ljava/lang/Class;J)Lgnu/classpath/jdwp/VMMethod;");
	if (!m) return NULL;

    cl = (*env)->FindClass(env,"gnu.classpath.jdwp.VMMethod");
	if (!cl) return NULL;

	joa = (*env)->NewObjectArray(env, (jsize)count, cl , NULL);
	if (!joa) return NULL;

    for (i = 0; i < count; i++) {
    	ol = (*env)->
			CallStaticObjectMethod(env,clazz,m,(jobject)par1, methodID[i]);
		if (!ol) return NULL;
    	(*env)->SetObjectArrayElement(env,joa,(jsize)i, ol);
    }
	return joa;
}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    getClassMethod
 * Signature: (Ljava/lang/Class;J)Lgnu/classpath/jdwp/VMMethod;
 */
JNIEXPORT struct gnu_classpath_jdwp_VMMethod* JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getClassMethod(JNIEnv *env, jclass clazz, struct java_lang_Class* par1, s8 par2) {
	jclass *cl;
    jmethodID m;
    jobject *ol;

    cl = (*env)->FindClass(env,"gnu.classpath.jdwp.VMMethod");
	if (!cl) return NULL;

	m = (*env)->GetMethodID(env, cl, "<init>", "(Ljava/lang/Class;J)V");
	if (!m) return NULL;

    ol = (*env)->NewObject(env, cl, m, par1, par2);

	return (struct gnu_classpath_jdwp_VMMethod*)ol;
}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    getFrames
 * Signature: (Ljava/lang/Thread;II)Ljava/util/ArrayList;
 */
JNIEXPORT struct java_util_ArrayList* JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getFrames(JNIEnv *env, jclass clazz, struct java_lang_Thread* par1, int32_t par2, int32_t par3) {
    log_text ("VMVirtualMachine_getFrames - IMPLEMENT ME!!!");
/*	jclass ec = (*env)->FindClass(env,"gnu/classpath/jdwp/JdwpInternalErrorException");
	if (JVMTI_ERROR_NONE != (*jvmtienv)->GetClassStatus(jvmtienv, par1, &status))
	env->ThrowNew(env,ec,"jvmti error occoured");*/
	return 0;
}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    getFrame
 * Signature: (Ljava/lang/Thread;Ljava/nio/ByteBuffer;)Lgnu/classpath/jdwp/VMFrame;
 */
JNIEXPORT struct gnu_classpath_jdwp_VMFrame* JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getFrame(JNIEnv *env, jclass clazz, struct java_lang_Thread* par1, struct java_nio_ByteBuffer* par2) {
    log_text ("VMVirtualMachine_getFrame - IMPLEMENT ME!!!");
	return 0;
}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    getFrameCount
 * Signature: (Ljava/lang/Thread;)I
 */
JNIEXPORT int32_t JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getFrameCount(JNIEnv *env, jclass clazz, struct java_lang_Thread* par1) {
	jint count;
	jvmtiError err;
	err = (*jvmtienv)->GetFrameCount(jvmtienv, (jthread)par1, &count);
	printjvmtierror("VMVirtualMachine_getFrameCount GetFrameCount", err);
	return count;
}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    getThreadStatus
 * Signature: (Ljava/lang/Thread;)I
 */
JNIEXPORT int32_t JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getThreadStatus(JNIEnv *env, jclass clazz, struct java_lang_Thread* par1) {
	jint status;
	jvmtiError err;
	if (JVMTI_ERROR_NONE != (err = (*jvmtienv)->GetThreadState(jvmtienv, (jthread)par1, &status))) {
		printjvmtierror("VMVirtualMachine_getThreadStatus GetThreadState", err);
		return 0;
	}
	if (status && JVMTI_THREAD_STATE_ALIVE) {
		if (status && JVMTI_THREAD_STATE_WAITING) {
			return 4; /* WAIT - see JdwpConstants */
		}
		if (status && JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER) {
			return 3; /* MONITOR - see JdwpConstants */
		}
		if (status && JVMTI_THREAD_STATE_SLEEPING) {
			return 2; /* SLEEPING - see JdwpConstants */
		}
		return 1; /* RUNNING - see JdwpConstants */
	} else
		return 0; /* ZOMBIE - see JdwpConstants */
	return -1; /* some error */
}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    getLoadRequests
 * Signature: (Ljava/lang/ClassLoader;)Ljava/util/ArrayList;
 */
JNIEXPORT struct java_util_ArrayList* JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getLoadRequests(JNIEnv *env, jclass clazz, struct java_lang_ClassLoader* par1) {
    log_text ("VMVirtualMachine_getLoadRequests(");
	return 0;
}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    executeMethod
 * Signature: (Ljava/lang/Object;Ljava/lang/Thread;Ljava/lang/Class;Ljava/lang/reflect/Method;[Ljava/lang/Object;Z)Lgnu/classpath/jdwp/util/MethodResult;
 */
JNIEXPORT struct gnu_classpath_jdwp_util_MethodResult* JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_executeMethod(JNIEnv *env, jclass clazz, struct java_lang_Object* par1, struct java_lang_Thread* par2, struct java_lang_Class* par3, struct java_lang_reflect_Method* par4, java_objectarray* par5, int32_t par6) {
    log_text ("VMVirtualMachine_executeMethod");
	return 0;
}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    getSourceFile
 * Signature: (Ljava/lang/Class;)Ljava/lang/String;
 */
JNIEXPORT struct java_lang_String* JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_getSourceFile(JNIEnv *env, jclass clazz, struct java_lang_Class* par1) {
	char* srcname;
	jstring str;
	jvmtiError err;

    if (JVMTI_ERROR_NONE !=(err=(*jvmtienv)->
		GetSourceFileName(jvmtienv, (jclass)par1, &srcname))) {
		printjvmtierror("VMVirtualMachine.getSourceFile GetSourceFileName", err);
		return NULL;
	}

	str = (*env)->NewString(env,(jchar*)srcname,(jsize)strlen(srcname));

	return (struct java_lang_String*)str;
}

/* match JdwpConstants.EventKind to jvmtiEvent constants */
static jvmtiEvent EventKind2jvmtiEvent(jbyte kind){
	switch (kind) {
	case /* SINGLE_STEP */ 1: return JVMTI_EVENT_SINGLE_STEP;
	case /* BREAKPOINT */ 2: return JVMTI_EVENT_BREAKPOINT;
    case /*  FRAME_POP */ 3: return JVMTI_EVENT_FRAME_POP;
    case /*  EXCEPTION */ 4: return JVMTI_EVENT_EXCEPTION;
    case /*  USER_DEFINED */ 5: return -1; /* can this be matched ? */
    case /*  THREAD_START */ 6: return JVMTI_EVENT_THREAD_START;
    case /*  THREAD_END */ 7: return JVMTI_EVENT_THREAD_END;
    case /*  CLASS_PREPARE */ 8: return JVMTI_EVENT_CLASS_PREPARE;
    case /*  CLASS_UNLOAD */ 9: return -1; /* can this be matched ? */
    case /*  CLASS_LOAD */ 10: return JVMTI_EVENT_CLASS_LOAD;
    case /*  FIELD_ACCESS */ 20: return JVMTI_EVENT_FIELD_ACCESS;
    case /*  FIELD_MODIFICATION */ 21: return JVMTI_EVENT_FIELD_MODIFICATION;
    case /*  EXCEPTION_CATCH */ 30: return JVMTI_EVENT_EXCEPTION_CATCH;
    case /*  METHOD_ENTRY */ 40: return JVMTI_EVENT_METHOD_ENTRY;
    case /*  METHOD_EXIT */ 41: return JVMTI_EVENT_METHOD_EXIT;
    case /*  VM_INIT */ 90: return JVMTI_EVENT_VM_INIT;
    case /*  VM_DEATH */ 99: return JVMTI_EVENT_VM_DEATH;
    case /*  VM_DISCONNECTED */ 100: return -1; /* can this be matched ? */
	default: return -1;
	}
}

/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    registerEvent
 * Signature: (Lgnu/classpath/jdwp/event/EventRequest;)V
 */
JNIEXPORT void JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_registerEvent(JNIEnv *env, jclass clazz, struct gnu_classpath_jdwp_event_EventRequest* par1) {
	jbyte kind;
	jfieldID kindid;
	jclass erc;
	jvmtiError err;

	erc = (*env)->FindClass(env,"gnu.classpath.jdwp.event.EventRequest");

	kindid = (*env)->GetFieldID(env, erc, "_kind", "B");
	kind = (*env)->GetByteField(env, (jobject)par1, kindid);

	if (JVMTI_ERROR_NONE != (err= (*jvmtienv)->
		SetEventNotificationMode(jvmtienv, JVMTI_ENABLE,
								 EventKind2jvmtiEvent(kind), NULL)))
		printjvmtierror("VMVirtualMachine_registerEvent SetEventNotificationMode",err);

}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    unregisterEvent
 * Signature: (Lgnu/classpath/jdwp/event/EventRequest;)V
 */
JNIEXPORT void JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_unregisterEvent(JNIEnv *env, jclass clazz, struct gnu_classpath_jdwp_event_EventRequest* par1) {
	jbyte kind;
	jfieldID kindid;
	jclass erc;
	jvmtiError err;

	erc = (*env)->FindClass(env,"gnu.classpath.jdwp.event.EventRequest");

	kindid = (*env)->GetFieldID(env, erc, "_kind", "B");
	kind = (*env)->GetByteField(env, (jobject)par1, kindid);

	if (JVMTI_ERROR_NONE != (err= (*jvmtienv)->
		SetEventNotificationMode(jvmtienv, JVMTI_DISABLE,
								 EventKind2jvmtiEvent(kind), NULL)))
		printjvmtierror("VMVirtualMachine_registerEvent SetEventNotificationMode",err);

}


/*
 * Class:     gnu/classpath/jdwp/VMVirtualMachine
 * Method:    clearEvents
 * Signature: (B)V
 */
JNIEXPORT void JNICALL Java_gnu_classpath_jdwp_VMVirtualMachine_clearEvents(JNIEnv *env, jclass clazz, int32_t par1) {
	/* jvmti events are not saved - there is nothing to clear */
}


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
