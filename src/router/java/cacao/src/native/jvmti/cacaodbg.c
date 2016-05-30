/* src/native/jvmti/cacaodbg.c - contains entry points for debugging support 
                                 in cacao.

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

*/

#include "native/jvmti/jvmti.h"
#include "native/jvmti/cacaodbg.h"
#include "native/jvmti/dbg.h"
#include "vm/vm.hpp"
#include "vm/loader.hpp"
#include "vm/exceptions.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/string.hpp"
#include "toolbox/logging.hpp"
#include "threads/mutex.h"
#include "threads/thread.hpp"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/wait.h>


/* jvmti_get_all_threads ******************************************************

   Gets an array of threadobjects of all threads

*******************************************************************************/
jvmtiError jvmti_get_all_threads (jint * threads_count_ptr, 
								  threadobject*** threads_ptr) {
    int i = 0, cnt = 8; 
    threadobject *thread, **tthreads;
	
#if defined(ENABLE_THREADS)
	tthreads = MNEW(threadobject*, (sizeof(threadobject*) * cnt));

	thread = mainthreadobj;
    do {
        if(thread->o.thread != NULL) {
			fflush(stderr);	

		   /* count and copy only live threads */
		   if (i>=cnt) {
			   MREALLOC(tthreads,threadobject*,cnt,cnt+8);
			   cnt += 8;
		   }
		   tthreads[i] = thread;
		   i++;
		}
		thread = thread->prev;

		/* repeat until we got the pointer to the mainthread twice */
	} while (mainthreadobj != thread);

	fflush(stderr);	

    *threads_count_ptr = i;
	*threads_ptr = tthreads;

    return JVMTI_ERROR_NONE;
#else
	return JVMTI_ERROR_NOT_AVAILABLE;
#endif
}


/* jvmti_get_current_thread ***************************************************

   Get jthread structure of current thread. 

*******************************************************************************/
jthread jvmti_get_current_thread() {
	return (jthread)(thread_get_current)->o.thread;
}



/*  breakpointtable_creator ***************************************************

   helper function to enlarge the breakpoint table if needed

*******************************************************************************/

static void breakpointtable_creator() {
	struct _brkpt* tmp;
	struct brkpts *jvmtibrkpt;

	jvmtibrkpt = &dbgcom->jvmtibrkpt;;
	if (jvmtibrkpt->size == 0) {
		jvmtibrkpt->brk = MNEW(struct _brkpt, 16);
		memset(jvmtibrkpt->brk, 0, sizeof(struct _brkpt)*16);
		jvmtibrkpt->size = 16;
		jvmtibrkpt->num = BEGINUSERBRK;
	} else {
		jvmtibrkpt->size += 16;
		tmp = jvmtibrkpt->brk;
		jvmtibrkpt->brk = MNEW(struct _brkpt, jvmtibrkpt->size);
		memset(jvmtibrkpt->brk, 0, sizeof(struct _brkpt)*(jvmtibrkpt->size));
		memcpy((void*)jvmtibrkpt->brk,(void*)tmp,jvmtibrkpt->size);
		MFREE(tmp,struct _brkpt,jvmtibrkpt->size-16);
	}	
}


/* jvmti_set_system_breakpoint ************************************************

   sets a system breakpoint in breakpoint table and calls set breakpoint

*******************************************************************************/

void jvmti_set_system_breakpoint(int sysbrk, bool mode) {	
	struct brkpts *jvmtibrkpt;

	mutex_lock(&dbgcomlock);
	jvmtibrkpt = &dbgcom->jvmtibrkpt;

	assert (sysbrk < BEGINUSERBRK);	
	if (jvmtibrkpt->size == jvmtibrkpt->num)
		breakpointtable_creator();

	if (mode) {
		/* add breakpoint*/
		if (jvmtibrkpt->brk[sysbrk].count > 0) {
			jvmtibrkpt->brk[sysbrk].count++;
			mutex_unlock(&dbgcomlock);
			return;
		}
		dbgcom->addbrkpt = true;
		dbgcom->brkaddr = jvmtibrkpt->brk[sysbrk].addr;
	} else {
		/* remove breakpoint*/		
		if ((jvmtibrkpt->brk[sysbrk].count == 1) ) {
			jvmtibrkpt->brk[sysbrk].count--;
			/* remove breakpoint */
			dbgcom->addbrkpt = false;
			dbgcom->brkaddr = jvmtibrkpt->brk[sysbrk].addr;
		} else {
			/* avoid negative counter values */
			if (jvmtibrkpt->brk[sysbrk].count > 0) jvmtibrkpt->brk[sysbrk].count--;
			mutex_unlock(&dbgcomlock);
			return;
		}
	}
	mutex_unlock(&dbgcomlock);
	/* call cacaodbgserver */
	__asm__ ("setsysbrkpt:");
	TRAP; 
}


/* jvmti_add_breakpoint *******************************************************

   adds a breakpoint to breakpoint table and calls set breakpoint

*******************************************************************************/

void jvmti_add_breakpoint(void* addr, jmethodID method, jlocation location) {
	struct brkpts *jvmtibrkpt;

	mutex_lock(&dbgcomlock);
	jvmtibrkpt = &dbgcom->jvmtibrkpt;;

	if (jvmtibrkpt->size == jvmtibrkpt->num)
		breakpointtable_creator();

	assert (jvmtibrkpt->size > jvmtibrkpt->num);
	fprintf (stderr,"add brk add: %p\n",addr);
	jvmtibrkpt->brk[jvmtibrkpt->num].addr = addr;
	jvmtibrkpt->brk[jvmtibrkpt->num].method = method;
	jvmtibrkpt->brk[jvmtibrkpt->num].location = location;

	/* todo: set breakpoint */
/*	jvmtibrkpt.brk[jvmtibrkpt.num].orig = */
	jvmtibrkpt->num++;
	mutex_unlock(&dbgcomlock);

	fprintf (stderr,"add brk done\n");
}




/* jvmti_cacaodbgserver_quit **************************************************

   quits cacaodbgserver if the last jvmti environment gets disposed

*******************************************************************************/
void jvmti_cacaodbgserver_quit(){
	mutex_lock(&dbgcomlock);
	dbgcom->running--;
	if (dbgcom->running  == 0) {
		__asm__ ("cacaodbgserver_quit:");
		TRAP;
		/* get cacaodbserver exit */
		wait(NULL);
		dbgcom = NULL;
	}
	mutex_unlock(&dbgcomlock);
}



/* jvmti_cacao_generic_breakpointhandler **************************************

   convert cacao breakpoints in jvmti events and fire event

*******************************************************************************/

static void jvmti_cacao_generic_breakpointhandler(int kindofbrk){
	genericEventData data; 

	switch (kindofbrk) {
	case THREADSTARTBRK:
		data.ev=JVMTI_EVENT_THREAD_START; 
		break;
	case THREADENDBRK:
		data.ev=JVMTI_EVENT_THREAD_END; 
		break;
	case CLASSLOADBRK:
		data.ev=JVMTI_EVENT_CLASS_LOAD; 
		break;
	case CLASSPREPARERK:
		data.ev=JVMTI_EVENT_CLASS_PREPARE; 
		break;
	case CLASSFILELOADHOOKBRK:
		data.ev=JVMTI_EVENT_CLASS_FILE_LOAD_HOOK; 
		break;
	case COMPILEDMETHODLOADBRK:
		data.ev=JVMTI_EVENT_COMPILED_METHOD_LOAD; 
		break;
	case COMPILEDMETHODUNLOADBRK:
		data.ev=JVMTI_EVENT_COMPILED_METHOD_UNLOAD; 
		break;
	default:
		fprintf(stderr,"unhandled kind of cacao break %d\n",kindofbrk);
		return;
	}
	jvmti_fireEvent(&data);
}



/* jvmti_cacao_debug_init ***************************************************************

   starts up a new cacaodbgserver process if needed

*******************************************************************************/

void jvmti_cacao_debug_init() {
	pid_t dbgserver;	

	/* start new cacaodbgserver if needed*/
	mutex_lock(&dbgcomlock);
	if (dbgcom == NULL) {
		dbgcom = heap_allocate(sizeof(cacaodbgcommunication),true,NULL);		
		dbgcom->running = 1;
		jvmti = true;

		breakpointtable_creator();
		/* set addresses of hard coded TRAPs */
		__asm__ ("movl $setsysbrkpt,%0;" 
			 :"=m"(dbgcom->jvmtibrkpt.brk[SETSYSBRKPT].addr));
		__asm__ ("movl $cacaodbgserver_quit,%0;" 
			 :"=m"(dbgcom->jvmtibrkpt.brk[CACAODBGSERVERQUIT].addr));

		dbgserver = fork();
		if (dbgserver  == (-1)) {
			log_text("cacaodbgserver fork error");
			exit(1);
		} else {
			if (dbgserver == 0) {
				if (execlp("cacaodbgserver","cacaodbgserver",(char *) NULL) == -1) {
					log_text("unable to execute cacaodbgserver");
					exit(1);
				}
			}
		}
		mutex_unlock(&dbgcomlock);
		/* let cacaodbgserver get ready */
		sleep(1);
	} else {
		dbgcom->running++;
		mutex_unlock(&dbgcomlock);
	}
}


/* jvmti_ClassFileLoadHook ****************************************************

  prepares firing a new Class File Load Hook event

*******************************************************************************/

void jvmti_ClassFileLoadHook(utf* name, int class_data_len, 
							 unsigned char* class_data, 
							 java_objectheader* loader, 
							 java_objectheader* protection_domain, 
							 jint* new_class_data_len, 
							 unsigned char** new_class_data) {
	genericEventData d;
	
	d.ev = JVMTI_EVENT_CLASS_FILE_LOAD_HOOK;
	d.klass = NULL; /* class is not redefined */
	d.object = loader;
	d.name = (char*)MNEW(char,(UTF_SIZE(name)+1));
	utf_sprint_convert_to_latin1(d.name, name);
	d.protection_domain = protection_domain;
	d.class_data = class_data;
	d.jint1 = class_data_len;
	d.new_class_data_len = new_class_data_len;
	d.new_class_data = new_class_data;

	jvmti_fireEvent(&d);
	MFREE(d.name,char,UTF_SIZE(name)+1);
}


/* jvmti_ClassFileLoadHook ****************************************************

  prepares firing a new Class Prepare or Load event

*******************************************************************************/

void jvmti_ClassLoadPrepare(bool prepared, classinfo *c) {
	genericEventData d;

	if (prepared) 
		d.ev = JVMTI_EVENT_CLASS_PREPARE;
	else 
		d.ev = JVMTI_EVENT_CLASS_LOAD;

	d.klass = c;
	jvmti_fireEvent(&d);	
}


/* jvmti_MonitorContendedEntering *********************************************

  prepares firing a new Monitor Contended Enter or Entered event

*******************************************************************************/

void jvmti_MonitorContendedEntering(bool entered, jobject obj) {
	genericEventData d;

	if (entered) 
		d.ev = JVMTI_EVENT_MONITOR_CONTENDED_ENTERED;
	else 
		d.ev = JVMTI_EVENT_MONITOR_CONTENDED_ENTER;

	d.object = obj;

	jvmti_fireEvent(&d);	
}

/* jvmti_MonitorWaiting ******************************************************

  prepares firing a new Monitor Wait or Waited event

*******************************************************************************/

void jvmti_MonitorWaiting(bool wait, jobject obj, jlong timeout) {
	genericEventData d;

	if (wait) {
		d.ev = JVMTI_EVENT_MONITOR_WAIT;
		d.jlong = timeout;
	} else {
		d.ev = JVMTI_EVENT_MONITOR_WAITED;
		d.b = timeout != 0;
	}

	d.object = obj;

	jvmti_fireEvent(&d);	
}

/* jvmti_ThreadStartEnd ********************************************************

  prepares firing a new Thread Start or End event

*******************************************************************************/

void jvmti_ThreadStartEnd(jvmtiEvent ev) {
	genericEventData d;

	d.ev = ev;
	jvmti_fireEvent(&d);	
}

/* jvmti_NativeMethodBind *****************************************************

  prepares firing a new Native Method Bind event

*******************************************************************************/

void jvmti_NativeMethodBind(jmethodID method, void* address, 
							void** new_address_ptr) {
	genericEventData d;

	d.ev = JVMTI_EVENT_NATIVE_METHOD_BIND;
	d.method = method;
	d.address = address;
	d.new_address_ptr = new_address_ptr;
	
	jvmti_fireEvent(&d);	
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
