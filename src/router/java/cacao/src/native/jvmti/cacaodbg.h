/* src/native/jvmti/cacaodbg.h - contains cacao specifics for debugging support

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

*/

#ifndef _CACAODBG_H
#define _CACAODBG_H

#include "threads/mutex.h"
#include "threads/thread.hpp"
#include "native/jvmti/jvmti.h"
#include "native/include/java_lang_String.h"
#include <ltdl.h>



typedef struct {
	jvmtiEvent ev;
	jvmtiEnv *jvmti_env;
	jthread thread;
	jmethodID method;
	jlocation location;
	jclass klass;
	jobject object;
	jfieldID field;
	char signature_type;
	jvalue value;
	jboolean b;
	void* address;
	void** new_address_ptr;
	jmethodID catch_method;
	jlocation catch_location;
	char* name;
	jobject protection_domain;
	jint jint1;
	jint jint2;
	unsigned char* class_data;
	jint* new_class_data_len;
	unsigned char** new_class_data;
	jvmtiAddrLocationMap* map;
	void* compile_info;
	jlong jlong;
} genericEventData;



#if defined(ENABLE_THREADS)
struct _jrawMonitorID {
    java_lang_String *name;
};


#endif


/* constants where system breakpoints are stored in the breakpoint table       */
#define SETSYSBRKPT             0 /* used for setsysbrkpt calls                */
#define CACAODBGSERVERQUIT      1 
#define NOTHARDCODEDBRK         2 /* here is where the first not hard coded 
   								     breakpoint is stored                      */
#define THREADSTARTBRK          2 
#define THREADENDBRK            3 
#define CLASSLOADBRK            4
#define CLASSPREPARERK          5
#define CLASSFILELOADHOOKBRK    6
#define COMPILEDMETHODLOADBRK   7
#define COMPILEDMETHODUNLOADBRK 8
#define BEGINUSERBRK            9 /* here is where the first user breakpoint  
								     is stored                                 */

struct _brkpt {
    jmethodID method;
    jlocation location;
    void* addr; /* memory address          */
	int count;
};


struct brkpts {
	struct _brkpt* brk;
	int num;
	int size;
};


typedef struct {
	int running;
	bool addbrkpt;
	void* brkaddr;
	struct brkpts jvmtibrkpt;
} cacaodbgcommunication;

cacaodbgcommunication *dbgcom;

bool jvmti;                 /* jvmti agent  */

extern mutex_t dbgcomlock;

jvmtiEnv* jvmti_new_environment();
void jvmti_set_phase(jvmtiPhase p);
void jvmti_fireEvent(genericEventData* data);
void jvmti_agentload(char* opt_arg, bool agentbypath, 
					 lt_dlhandle  *handle, char **libname);
void jvmti_agentunload();
void jvmti_add_breakpoint(void* addr, jmethodID method, jlocation location);
void jvmti_set_system_breakpoint(int sysbrk, bool mode);
jvmtiError jvmti_get_all_threads (jint * threads_count_ptr, 
								  threadobject *** threads_ptr);
jthread jvmti_get_current_thread();
void jvmti_cacao_debug_init();
void jvmti_cacaodbgserver_quit();

void jvmti_ClassLoadPrepare(bool prepared, classinfo *c);
void jvmti_ClassFileLoadHook(utf* name, int class_data_len, 
							 unsigned char* class_data, 
							 java_objectheader* loader, 
							 java_objectheader* protection_domain, 
							 jint* new_class_data_len, 
							 unsigned char** new_class_data);
void jvmti_MonitorContendedEntering(bool entered, jobject obj);
void jvmti_MonitorWaiting(bool wait, jobject obj, jlong timeout);
void jvmti_ThreadStartEnd(jvmtiEvent ev);
void jvmti_NativeMethodBind(jmethodID method, void* address, 
							void** new_address_ptr);
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
