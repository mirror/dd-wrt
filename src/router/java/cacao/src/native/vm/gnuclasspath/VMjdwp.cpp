/* src/native/vm/VMjdwp.c - jvmti->jdwp interface

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

   Contact: cacao@cacaojvm.org

   Author: Martin Platter

   Changes:

*/

#include "native/jvmti/jvmti.h"
#include "native/jvmti/VMjdwp.hpp"

#include <stdlib.h>
#include <string.h>

void printjvmtierror(char *desc, jvmtiError err) {
    char* errdesc;

	if (err == JVMTI_ERROR_NONE) return;
	(*jvmtienv)->GetErrorName(jvmtienv,err, &errdesc);
	fprintf(stderr,"%s: jvmti error %s\n",desc, errdesc);
	fflush(stderr);
	(*jvmtienv)->Deallocate(jvmtienv,(unsigned char*)errdesc);
}


/* class and method IDs */
static jclass Jdwpclass, threadstartclass,threadendclass, classprepareclass, 	vmmethodclass, locationclass, breakpointclass;
static jmethodID notifymid, threadstartmid,threadendmid, classpreparemid,
	vmmethodmid, locationmid, breakpointmid;

static void notify (JNIEnv* jni_env, jobject event){
	fprintf(stderr,"VMjdwp notfiy called\n");

	(*jni_env)->CallStaticVoidMethod(jni_env,Jdwpclass,notifymid,event);
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"Exception occourred in notify mehtod\n");
		(*jni_env)->ExceptionDescribe(jni_env);
	}

}

static void ThreadStart (jvmtiEnv *jvmti_env, JNIEnv* jni_env,
                         jthread thread){
	jobject obj;

	obj = (*jni_env)->
		NewObject(jni_env, threadstartclass, threadstartmid, thread);
	if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"error calling ThreadStartEvent constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		return;
	}

	fprintf(stderr,"VMjdwp:ThreadStart: thread %p\n",thread);
	fflush(stderr);

	notify (jni_env,obj);
}


static void ThreadEnd (jvmtiEnv *jvmti_env, JNIEnv* jni_env,
                         jthread thread){
	jobject obj;


	obj = (*jni_env)->NewObject(jni_env, threadendclass, threadendmid, thread);
	if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"error calling ThreadEndEvent constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		return;
	}

	fprintf(stderr,"VMjdwp:ThreadEnd: thread %p\n",thread);
	fflush(stderr);

	notify (jni_env,obj);
}


static void ClassPrepare (jvmtiEnv *jvmti_env, JNIEnv* jni_env,
						  jthread thread, jclass klass) {
	jobject obj;
	int classstatus;
	jvmtiError e;

	if (JVMTI_ERROR_NONE !=
		(e = (*jvmtienv)->GetClassStatus(jvmtienv, klass, &classstatus))) {
		printjvmtierror("unable to get class status", e);
		return;
	}

	obj = (*jni_env)->NewObject(jni_env, classprepareclass, classpreparemid, thread, klass, classstatus);
	if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"error calling ClassPrepareEvent constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		return;
	}

	fprintf(stderr,"VMjdwp:ClassPrepareEvent: thread %p\n",thread);
	fflush(stderr);

	notify (jni_env,obj);
}

static void Exception (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread,
					   jmethodID method, jlocation location, jobject exception,
					   jmethodID catch_method, jlocation catch_location) {
	/* gnu classpath jdwp has no ExceptionEvent yet */
	fprintf(stderr,"VMjdwp:Exception: thread %p\n",thread);
	fflush(stderr);

}

static void Breakpoint (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread,
						jmethodID method, jlocation location) {
	jobject vmmethod, loc, ev;
	jclass mcl;
	jvmtiError e;

	if (JVMTI_ERROR_NONE !=
		(e = (*jvmtienv)->GetMethodDeclaringClass(jvmtienv,
									   method,
									   &mcl))){
		printjvmtierror("unable to get declaring class", e);
		return;
	}

	vmmethod = (*jni_env)->NewObject(jni_env, vmmethodclass, vmmethodmid,
									 mcl, method);
	if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"error calling VMMethod constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		return;
	}

	loc = (*jni_env)->NewObject(jni_env, locationclass, locationmid,
									 vmmethod, location);
	if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"error calling location constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		return;
	}

	/* XXX todo: get object instance - needs jvmti local variable support */
	ev = (*jni_env)->NewObject(jni_env, breakpointclass, breakpointmid,
									 thread, loc,NULL);
	if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"error calling breakpoint constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		return;
	}

	fprintf(stderr,"VMjdwp:Breakpoint: thread %p\n",thread);
	fflush(stderr);

	notify (jni_env,ev);
}


static void MethodEntry (jvmtiEnv *jvmti_env, JNIEnv* jni_env,
						 jthread thread, jmethodID method) {
	/* do not report gnu/classpath/jdwp method entries */
}


static void VMDeath (jvmtiEnv *jvmti_env,
                     JNIEnv* jni_env) {
  fprintf(stderr,"JVMTI-Event: IMPLEMENT ME!!!");
}


/* setup_jdwp_thread **********************************************************

   Helper function to start JDWP listening thread

*******************************************************************************/

static void setup_jdwp_thread(JNIEnv* jni_env) {
	jobject o;
	jmethodID m;
	jstring  s;

	/* new gnu.classpath.jdwp.Jdwp() */
	m = (*jni_env)->GetMethodID(jni_env,Jdwpclass,"<init>","()V");
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"could not get Jdwp constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}

	o = (*jni_env)->NewObject(jni_env, Jdwpclass, m);
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"error calling Jdwp constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}

	jdwpthread = (jthread)o;


	/* configure(jdwpoptions) */
	m = (*jni_env)->GetMethodID(jni_env,Jdwpclass,"configure",
								"(Ljava/lang/String;)V");
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"could not get Jdwp configure method\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}


	s = (*jni_env)->NewStringUTF(jni_env,jdwpoptions);
    if (s == NULL) {
        fprintf(stderr,"could not get new java string from jdwp options\n");
		exit(1);
	}

	free(jdwpoptions);

	(*jni_env)->CallVoidMethod(jni_env,o,m,s);
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"Exception occourred in Jdwp configure\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}

	m = (*jni_env)->GetMethodID(jni_env,Jdwpclass,"_doInitialization","()V");
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"could not get Jdwp _doInitialization method\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}


	(*jni_env)->CallVoidMethod(jni_env,o,m);
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"Exception occourred in Jdwp _doInitialization\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}
}

#define FINDCLASSWITHEXCEPTION(CLASS,SIGNATURE) \
	CLASS = (*jni_env)->FindClass(jni_env, SIGNATURE);     \
	if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {  \
		fprintf(stderr,"could not find %s\n", SIGNATURE);  \
		(*jni_env)->ExceptionDescribe(jni_env);            \
		exit(1);                                           \
	}
#define GETMIDWITHEXCEPTION(CLASS, CLASSNAME, MID, METHODNAME, METHODSIG) \
	FINDCLASSWITHEXCEPTION(CLASS, CLASSNAME);                             \
	MID = (*jni_env)->GetMethodID(jni_env, CLASS, METHODNAME, METHODSIG); \
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {                 \
        fprintf(stderr,"could not get %s %s\n",CLASSNAME, METHODNAME);    \
		(*jni_env)->ExceptionDescribe(jni_env);                           \
		exit(1);                                                          \
	}


static void fillidcache(JNIEnv* jni_env) {
	FINDCLASSWITHEXCEPTION(Jdwpclass, "gnu/classpath/jdwp/Jdwp");

	notifymid = (*jni_env)->
	 GetStaticMethodID(jni_env,Jdwpclass,
					   "notify","(Lgnu/classpath/jdwp/event/Event;)V");
	if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
		fprintf(stderr,"could not get notify method\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}

	GETMIDWITHEXCEPTION(threadstartclass,
						"gnu/classpath/jdwp/event/ThreadStartEvent",
						threadstartmid, "<init>", "(Ljava/lang/Thread;)V");


	GETMIDWITHEXCEPTION(threadendclass,
						"gnu/classpath/jdwp/event/ThreadEndEvent",
						threadendmid, "<init>", "(Ljava/lang/Thread;)V");


	GETMIDWITHEXCEPTION(classprepareclass,
						"gnu/classpath/jdwp/event/ClassPrepareEvent",
						classpreparemid, "<init>",
						"(Ljava/lang/Thread;Ljava/lang/Class;I)V");


	GETMIDWITHEXCEPTION(vmmethodclass, "gnu/classpath/jdwp/VMMethod",
						vmmethodmid, "<init>", "(Ljava/lang/Class;J)V");

	GETMIDWITHEXCEPTION(locationclass, "gnu/classpath/jdwp/util/Location",
						locationmid, "<init>",
						"(Lgnu/classpath/jdwp/VMMethod;J)V");


	GETMIDWITHEXCEPTION(
		breakpointclass,
		"gnu/classpath/jdwp/event/BreakpointEvent",
		breakpointmid, "<init>",
		"(Ljava/lang/Thread;Lgnu/classpath/jdwp/util/Location;Ljava/lang/Object;)V");

}

static void VMInit (jvmtiEnv *jvmti_env,
                    JNIEnv* jni_env,
                    jthread thread) {
	jclass cl;
	jmethodID m;
	jobject eventobj;
	jvmtiError err;

	fprintf(stderr,"JDWP VMInit\n");

	/* get needed jmethodIDs and jclasses for callbacks */
	fillidcache(jni_env);

	/* startup gnu classpath jdwp thread */
	setup_jdwp_thread(jni_env);

	fprintf(stderr,"JDWP listening thread started\n");

	/* send VmInitEvent */
    cl = (*jni_env)->FindClass(jni_env,
									  "gnu/classpath/jdwp/event/VmInitEvent");
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"could not find class VMInitEvent\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}

	m = (*jni_env)->GetMethodID(jni_env,cl,"<init>",
								"(Ljava/lang/Thread;)V");
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"could not get VmInitEvent constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}

	eventobj = (*jni_env)->NewObject(jni_env, cl, m, thread);
    if ((*jni_env)->ExceptionOccurred(jni_env) != NULL) {
        fprintf(stderr,"error calling VmInitEvent constructor\n");
		(*jni_env)->ExceptionDescribe(jni_env);
		exit(1);
	}


	notify (jni_env,eventobj);

	if (suspend) {
		fprintf(stderr,"suspend initial thread\n");
		err = (*jvmti_env)->SuspendThread(jvmti_env,thread);
		printjvmtierror("error suspending initial thread",err);
	}
}

static void usage() {
	puts("usage jdwp:[help]|(<option>=<value>),*");
	puts("   transport=[dt_socket|...]");
	puts("   address=<hostname:port>");
	puts("   server=[y|n]");
	puts("   suspend=[y|n]");
}

static bool processoptions(char *options) {
	int i,len;

	if (strncmp(options,"help",4) == 0) {
		usage();
		return false;
	}

	suspend = true; 	/* default value */


	/* copy options for later use in java jdwp listen thread configure */
	jdwpoptions = malloc(sizeof(char)*strlen(options));
	strncpy(jdwpoptions, options, sizeof(char)*strlen(options));

	len = strlen(options);

	i=0;
	while (i<len) {
		if (strncmp("suspend=",&options[i],8)==0) {
			if (8>=strlen(&options[i])) {
				if ((options[i+8]== 'y') || (options[i+8]== 'n')) {
					suspend = options[i+8]== 'y';
				} else {
					printf("jdwp error argument: %s\n",options);
					usage();
					return -1;
				}
			}
		} else {
			/* these options will be handled by jdwp java configure */
			if ((strncmp("transport=",options,10)==0) ||
				(strncmp("server=",options,7)==0)) {
			} else {
				printf("jdwp unkown argument: %s\n",options);
				usage();
				return false;
			}
		}
		while ((options[i]!=',')&&(i<len)) i++;
		i++;
	}
	return true;
}


JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
	jint rc;
	jvmtiCapabilities cap;
	jvmtiError e;


	fprintf(stderr,"jdwp Agent_OnLoad options: %s\n",options);
	if (!processoptions(options)) return -1;

	rc = (*vm)->GetEnv(vm, (void**)&jvmtienv, JVMTI_VERSION_1_0);
	if (rc != JNI_OK) {
		fprintf(stderr, "jdwp: Unable to get jvmtiEnv error=%d\n", rc);
		return -1;
	}

	/* set eventcallbacks */
	if (JVMTI_ERROR_NONE !=
		(e = (*jvmtienv)->SetEventCallbacks(jvmtienv,
									   &jvmti_jdwp_EventCallbacks,
									   sizeof(jvmtiEventCallbacks)))){
		printjvmtierror("jdwp: unable to setup event callbacks", e);
		return -1;
	}

	e = (*jvmtienv)->GetPotentialCapabilities(jvmtienv, &cap);
	printjvmtierror("jdwp: unable to get potential capabilities", e);
	if (e == JVMTI_ERROR_NONE)
		e = (*jvmtienv)->AddCapabilities(jvmtienv, &cap);
	if (e != JVMTI_ERROR_NONE) {
		printjvmtierror("jdwp: error adding jvmti capabilities", e);
		return -1;
	}

	/* only enable needed events. VMVirtualMachine.registerEvent will
	   be used to enable other events by need */
	if (JVMTI_ERROR_NONE != (e = (*jvmtienv)->
							 SetEventNotificationMode(jvmtienv, JVMTI_ENABLE,
													  JVMTI_EVENT_VM_INIT,
													  NULL))) {
		printjvmtierror("jdwp unable to enable vm init callback",e);
		return -1;
	}

	return 0;
}


jvmtiEventCallbacks jvmti_jdwp_EventCallbacks = {
    &VMInit,
    &VMDeath,
    &ThreadStart,
    &ThreadEnd,
    NULL, /* &ClassFileLoadHook, */
    NULL, /* &ClassLoad, */
    &ClassPrepare,
    NULL, /* &VMStart */
    &Exception,
    NULL, /* &ExceptionCatch, */
    NULL, /* &SingleStep, */
    NULL, /* &FramePop, */
    &Breakpoint,
    NULL, /* &FieldAccess, */
    NULL, /* &FieldModification, */
    &MethodEntry,
    NULL, /* &MethodExit, */
    NULL, /* &NativeMethodBind, */
    NULL, /* &CompiledMethodLoad, */
    NULL, /* &CompiledMethodUnload, */
    NULL, /* &DynamicCodeGenerated, */
    NULL, /* &DataDumpRequest, */
    NULL,
    NULL, /* &MonitorWait, */
    NULL, /* &MonitorWaited, */
    NULL, /* &MonitorContendedEnter, */
    NULL, /* &MonitorContendedEntered, */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, /* &GarbageCollectionStart, */
    NULL, /* &GarbageCollectionFinish, */
    NULL, /* &ObjectFree, */
    NULL, /* &VMObjectAlloc, */
};


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
