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

#include <signal.h>
#include <semaphore.h>

#include "jam.h"
#include "symbol.h"
#include "thread.h"

extern int name_offset;
extern int daemon_offset;
extern int priority_offset;

static int eetop_offset;
static int thread_status_offset = -1;

static MethodBlock *init_mb_no_name;
static MethodBlock *init_mb_with_name;

int classlibInitJavaThread(Thread *thread, Object *jlthread, Object *name,
                           Object *group, char is_daemon, int priority) {

    INST_DATA(jlthread, Thread*, eetop_offset) = thread;
    INST_DATA(jlthread, int, daemon_offset) = is_daemon;
    INST_DATA(jlthread, int, priority_offset) = priority;

    if(name == NULL)
        executeMethod(jlthread, init_mb_no_name, group, NULL);
    else
        executeMethod(jlthread, init_mb_with_name, group, name);

    return !exceptionOccurred();
}

int classlibCreateJavaThread(Thread *thread, Object *jThread) {
    INST_DATA(jThread, Thread*, eetop_offset) = thread;
    return TRUE;
}

Object *classlibThreadPreInit(Class *thread_class, Class *thrdGrp_class) {
    MethodBlock *system_grp_init_mb, *main_grp_init_mb;
    Object *system_grp, *main_grp, *main_grp_name;
    FieldBlock *thread_status_fb, *eetop_fb;

    init_mb_with_name = findMethod(thread_class, SYMBOL(object_init),
                           SYMBOL(_java_lang_ThreadGroup_java_lang_String__V));

    init_mb_no_name = findMethod(thread_class, SYMBOL(object_init),
                         SYMBOL(_java_lang_ThreadGroup_java_lang_Runnable__V));

    thread_status_fb = findField(thread_class, SYMBOL(threadStatus),
                                               SYMBOL(I));

    eetop_fb = findField(thread_class, SYMBOL(eetop), SYMBOL(J));

    system_grp_init_mb = findMethod(thrdGrp_class, SYMBOL(object_init),
                                                   SYMBOL(___V));

    main_grp_init_mb = findMethod(thrdGrp_class, SYMBOL(object_init),
                          SYMBOL(_java_lang_ThreadGroup_java_lang_String__V));

    if(init_mb_with_name  == NULL || init_mb_no_name  == NULL ||
       system_grp_init_mb == NULL || main_grp_init_mb == NULL ||
       thread_status_fb   == NULL || eetop_fb         == NULL)
        return NULL;

    CLASS_CB(thread_class)->flags |= JTHREAD;

    thread_status_offset = thread_status_fb->u.offset;
    eetop_offset = eetop_fb->u.offset;

    if((system_grp = allocObject(thrdGrp_class)) == NULL)
        return NULL;

    executeMethod(system_grp, system_grp_init_mb);
    if(exceptionOccurred())
        return NULL;

    if((main_grp = allocObject(thrdGrp_class)) == NULL ||
       (main_grp_name = Cstr2String("main")) == NULL)
        return NULL;

    executeMethod(main_grp, main_grp_init_mb, system_grp, main_grp_name);
    if(exceptionOccurred())
        return NULL;

    return main_grp;
}

extern VMLock resolve_lock;

int classlibThreadPostInit() {
    Class *system;

#ifdef JSR292
    /* Initialise lock used in Method Handle resolution - this
       must be done before any invokedynamic instruction is executed */
    initVMLock(resolve_lock);
#endif

    /* Initialise System class */
    system = findSystemClass(SYMBOL(java_lang_System));
    if(system != NULL) {
        MethodBlock *init = findMethod(system, SYMBOL(initializeSystemClass),
                                               SYMBOL(___V));
        if(init != NULL) {
            executeStaticMethod(system, init);
            return !exceptionOccurred();
        }
    }

    return FALSE;
}

Thread *classlibJThread2Thread(Object *jThread) {
    return INST_DATA(jThread, Thread*, eetop_offset);
}

int jThreadIsAlive(Object *jThread) {
    int state = INST_DATA(jThread, int, thread_status_offset);
    return state != CREATING && state != TERMINATED;
}

Object *classlibMarkThreadTerminated(Object *jThread) {
    INST_DATA(jThread, int, thread_status_offset) = TERMINATED;
    return jThread;
}

int classlibGetThreadState(Thread *thread) {
    if(thread_status_offset == -1 || thread->ee == NULL
                                  || thread->ee->thread == NULL)
        return 0;

    return INST_DATA(thread->ee->thread, int, thread_status_offset);
}

void classlibSetThreadState(Thread *thread, int state) {
    if(thread_status_offset != -1 && thread->ee != NULL
                                  && thread->ee->thread != NULL)
        INST_DATA(thread->ee->thread, int, thread_status_offset) = state;
}

void classlibThreadName2Buff(Object *jThread, char *buffer, int buff_len) {
    Object *name = INST_DATA(jThread, Object*, name_offset);
    unsigned short *unicode = ARRAY_DATA(name, unsigned short);
    int i, len = ARRAY_LEN(name) < buff_len ? ARRAY_LEN(name)
                                            : buff_len - 1;

    for(i = 0; i < len; i++)
        buffer[i] = unicode[i];

    buffer[len] = '\0';
}

static sem_t signal_sem;
static MethodBlock *signal_dispatch_mb;
static sig_atomic_t pending_signals[NSIG];

void signalHandler(int sig) {
    pending_signals[sig] = TRUE;
    sem_post(&signal_sem);
}

void classlibSignalThread(Thread *self) {
    int sig;

    disableSuspend0(self, &self);
    for(;;) {
        do {
            sem_wait(&signal_sem);

            for(sig = 0; sig < NSIG && !pending_signals[sig]; sig++);
        } while(sig == NSIG);

        pending_signals[sig] = FALSE;

        if(sig == SIGQUIT)
            printThreadsDump(self);
        else {
            enableSuspend(self);

            executeStaticMethod(signal_dispatch_mb->class,
                                signal_dispatch_mb, sig);

            disableSuspend0(self, &self);
        }
    }
}

int classlibInitialiseSignals() {
    struct sigaction act;
    Class *signal_class;

    act.sa_handler = signalHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    sigaction(SIGQUIT, &act, NULL);

    sem_init(&signal_sem, 0, 0);

    signal_class = findSystemClass(SYMBOL(sun_misc_Signal));
    if(signal_class == NULL)
        return FALSE;

    signal_dispatch_mb = findMethod(signal_class, SYMBOL(dispatch),
                                                  SYMBOL(_I__V));

    return signal_dispatch_mb != NULL;
}
