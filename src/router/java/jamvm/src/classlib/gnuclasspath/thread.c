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

/* Required on OpenSolaris to get standard conforming sigwait. */
#define _POSIX_PTHREAD_SEMANTICS
#include <signal.h>

#include "jam.h"
#include "symbol.h"
#include "thread.h"

static int vmData_offset;
static int thread_offset;
static int vmthread_offset;

static MethodBlock *init_mb;
static Class *vmthread_class;

extern int name_offset;
extern int group_offset;
extern MethodBlock *addThread_mb;

int classlibInitJavaThread(Thread *thread, Object *jlthread, Object *name,
                           Object *group, char is_daemon, int priority) {

    Object *vmthread;

    /* Create the VMThread object */
    if((vmthread = allocObject(vmthread_class)) == NULL)
        return FALSE;

    INST_DATA(vmthread, Thread*, vmData_offset) = thread;
    INST_DATA(vmthread, Object*, thread_offset) = jlthread;

    /* Call the initialiser method -- this is for use by threads
       created or attached by the VM "outside" of Java */
    executeMethod(jlthread, init_mb, vmthread, name, priority, is_daemon);

    if(exceptionOccurred())
        return FALSE;

    /* Handle the thread group */
    INST_DATA(jlthread, Object*, group_offset) = group;
    executeMethod(group, addThread_mb, jlthread);

    return TRUE;
}

int classlibCreateJavaThread(Thread *thread, Object *jThread) {
    Object *vmthread = allocObject(vmthread_class);

    if(vmthread == NULL)
        return FALSE;

    INST_DATA(vmthread, Thread*, vmData_offset) = thread;
    INST_DATA(vmthread, Object*, thread_offset) = jThread;
    INST_DATA(jThread, Object*, vmthread_offset) = vmthread;

    return TRUE;
}

Object *classlibMarkThreadTerminated(Object *jThread) {
    Object *vmthread = INST_DATA(jThread, Object*, vmthread_offset);

    /* set VMThread ref in Thread object to null - operations after this
       point will result in an IllegalThreadStateException */
    INST_DATA(jThread, Object*, vmthread_offset) = NULL;

    return vmthread;
}

Object *classlibThreadPreInit(Class *thread_class, Class *thrdGrp_class) {
    FieldBlock *vmData;
    FieldBlock *vmThread;
    FieldBlock *root, *thread;

    if((vmthread_class = findSystemClass0(SYMBOL(java_lang_VMThread))) == NULL)
        return NULL;

    CLASS_CB(vmthread_class)->flags |= VMTHREAD;

    /* Register class reference for compaction threading */
    registerStaticClassRef(&vmthread_class);

    vmThread = findField(thread_class, SYMBOL(vmThread),
                                       SYMBOL(sig_java_lang_VMThread));

    init_mb = findMethod(thread_class, SYMBOL(object_init),
                         SYMBOL(_java_lang_VMThread_java_lang_String_I_Z__V));

    thread = findField(vmthread_class, SYMBOL(thread),
                                       SYMBOL(sig_java_lang_Thread));
    vmData = findField(vmthread_class, SYMBOL(vmData), SYMBOL(J));

    root = findField(thrdGrp_class, SYMBOL(root),
                                    SYMBOL(sig_java_lang_ThreadGroup));

    /* findField and findMethod do not throw an exception... */
    if(init_mb == NULL || vmData == NULL || vmThread == NULL
                       || thread == NULL || root == NULL)
        return NULL;

    vmthread_offset = vmThread->u.offset;
    thread_offset = thread->u.offset;
    vmData_offset = vmData->u.offset;

    return root->u.static_value.p;
}

Thread *vmThread2Thread(Object *vmThread) {
    return INST_DATA(vmThread, Thread*, vmData_offset);
}

Thread *classlibJThread2Thread(Object *jThread) {
    Object *vmthread = INST_DATA(jThread, Object*, vmthread_offset);

    return vmthread == NULL ? NULL : vmThread2Thread(vmthread);
}

void classlibThreadName2Buff(Object *jThread, char *buffer, int buff_len) {
    Object *name = INST_DATA(jThread, Object*, name_offset);
    String2Buff(name, buffer, buff_len);
}

void classlibSignalThread(Thread *self) {
    sigset_t mask;
    int sig;

    sigemptyset(&mask);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGINT);

    disableSuspend0(self, &self);
    for(;;) {
        sigwait(&mask, &sig);

        /* If it was an interrupt (e.g. Ctrl-C) terminate the VM */
        if(sig == SIGINT)
            exitVM(0);

        /* It must be a SIGQUIT.  Do a thread dump */
        printThreadsDump(self);
    }
}

