/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011,
 * 2012, 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <errno.h>

#include "jam.h"
#include "thread.h"
#include "lock.h"
#include "hash.h"
#include "symbol.h"
#include "excep.h"
#include "class.h"
#include "classlib.h"

#ifdef TRACETHREAD
#define TRACE(fmt, ...) jam_printf(fmt, ## __VA_ARGS__)
#else
#define TRACE(fmt, ...)
#endif

#define HASHTABSZE 1<<4
HashTable thread_id_map;

/* Size of Java stack to use if no size is given */
static int dflt_stack_size;

/* Thread create/destroy lock and condvar */
static pthread_mutex_t lock;
static pthread_cond_t cv;

/* lock and condvar used by main thread to wait for
 * all non-daemon threads to die */
static pthread_mutex_t exit_lock;
static pthread_cond_t exit_cv;

/* Monitor for sleeping threads to do a timed-wait against */
static Monitor sleep_mon;

/* If supported, use thread local storage to store the
   thread's Thread pntr.  If not, use a pthread thread
   specific key to hold it */
#ifdef HAVE_TLS
static __thread Thread *self = NULL;
#else
static pthread_key_t self;
#endif

/* Attributes for spawned threads */
static pthread_attr_t attributes;

/* The main thread info - head of the thread list */
static Thread main_thread;

/* Main thread ExecEnv */
static ExecEnv main_ee;

/* Various field offsets into java.lang.Thread -
   cached at startup and used in thread creation */
int name_offset;
int group_offset;
int daemon_offset;
int priority_offset;
static int threadId_offset;

/* Method table indexes of Thread.run method and
 * ThreadGroup.removeThread - cached at startup */
static int run_mtbl_idx;
static int rmveThrd_mtbl_idx;

MethodBlock *addThread_mb;

/* Cached java.lang.Thread class */
static Class *thread_class;

/* Count of non-daemon threads still running in VM */
static int non_daemon_thrds = 0;

/* Counts used in the ThreadMXBean management API.  The
   counts start from one to include the main thread */
static int threads_count = 1;
static int peak_threads_count = 1;
static long long total_started_threads_count = 1;

/* Guards against threads starting while the "world is stopped" */
static int all_threads_suspended = FALSE;
static int threads_waiting_to_start = 0;

static int main_exited = FALSE;

/* Bitmap - used for generating unique thread ID's */
#define MAP_INC 32
static unsigned int *tidBitmap = NULL;
static int tidBitmapSize = 0;

/* Mark a threadID value as no longer used */
#define freeThreadID(n) tidBitmap[(n-1)>>5] &= ~(1<<((n-1)&0x1f))

/* Generate a new thread ID - assumes the thread queue
 * lock is held */

static int genThreadID() {
    int i = 0;

retry:
    for(; i < tidBitmapSize; i++) {
        if(tidBitmap[i] != 0xffffffff) {
            int n = ffs(~tidBitmap[i]);
            tidBitmap[i] |= 1 << (n-1);
            return (i<<5) + n;
        }
    }

    tidBitmap = sysRealloc(tidBitmap,
                       (tidBitmapSize + MAP_INC) * sizeof(unsigned int));
    memset(tidBitmap + tidBitmapSize, 0, MAP_INC * sizeof(unsigned int));
    tidBitmapSize += MAP_INC;
    goto retry;
}

int threadIsAlive(Thread *thread) {
    int state = classlibGetThreadState(thread);
    return state != CREATING && state != TERMINATED;
}

void threadSleep(Thread *self, long long ms, int ns) {

   /* A sleep of zero should just yield, but a wait
      of zero is "forever".  Therefore wait for a tiny
      amount -- this should yield and we also get the
      interrupted checks. */
    if(ms == 0 &&  ns == 0)
        ns = 1;

    monitorLock(&sleep_mon, self);
    monitorWait(&sleep_mon, self, ms, ns, FALSE, TRUE);
    monitorUnlock(&sleep_mon, self);
}

void threadYield(Thread *thread) {
    sched_yield();
}

int threadInterrupted(Thread *thread) {
    int interrupted = thread->interrupted;
    thread->interrupted = FALSE;
    return interrupted;
}

int threadIsInterrupted(Thread *thread) {
    return thread->interrupted;
}

void threadInterrupt(Thread *thread) {
    Thread *self = threadSelf();
    Monitor *mon;

    /* MonitorWait sets wait_mon _before_ checking interrupted status.
       Therefore, if wait_mon is null, interrupted status will be noticed.
       This guards against a race-condition leading to an interrupt being
       missed.  The memory barrier ensures correct ordering on SMP systems. */
    thread->interrupted = TRUE;
    MBARRIER();

    if((mon = thread->wait_mon) != NULL && thread->wait_next != NULL) {
        int locked;
        thread->interrupting = TRUE;

        /* The thread is waiting on a monitor, but it may not have
           entered the wait (in which case the signal will be lost).
           Loop until we can get ownership (i.e. the thread has released
           it on waiting) */
        while(!(locked = !pthread_mutex_trylock(&mon->lock)) &&
                    mon->owner == NULL)
            sched_yield();

        pthread_cond_signal(&thread->wait_cv);

        if(locked)
            pthread_mutex_unlock(&mon->lock);
    }

    /* Thread may still be parked */
    threadUnpark(thread);

    /* Handle the case where the thread is blocked in a system call.
       This will knock it out with an EINTR.  The suspend signal
       handler will just return (as in the user doing a kill), and do
       nothing otherwise. */

    /* Note, under Linuxthreads pthread_kill obtains a lock on the thread
       being signalled.  If another thread is suspending all threads,
       and the interrupting thread is earlier in the thread list than the
       thread being interrupted, it can be suspended holding the lock.
       When the suspending thread tries to signal the interrupted thread
       it will deadlock.  To prevent this, disable suspension. */

    fastDisableSuspend(self);
    pthread_kill(thread->tid, SIGUSR1);
    fastEnableSuspend(self);
}

void threadPark(Thread *self, int absolute, long long time) {
    /* If we have a permit use it and return immediately.
       No locking as we're the only one that can change the
       state at this point */
    if(self->park_state == PARK_PERMIT) {
        self->park_state = PARK_RUNNING;
        MBARRIER();
        return;
    } 

    /* Spin until we can get the park lock.  This avoids having
       to disable suspension around pthread_mutex_lock */
    while(pthread_mutex_trylock(&self->park_lock))
        sched_yield();

    /* A thread may have given us a permit while we were waiting
       for the lock or we may be running.  Reduce the state by
       one (PERMIT -> RUNNING, RUNNING -> BLOCKED) and wait if
       we're now blocked */

    if(--self->park_state == PARK_BLOCKED) {
        /* Really must disable suspension now as we're
           going to sleep */
        disableSuspend(self);

        if(time) {
            struct timespec ts;

            if(absolute)
                getTimeoutAbsolute(&ts, time, 0);
            else
                getTimeoutRelative(&ts, 0, time);

            classlibSetThreadState(self, TIMED_PARKED);
            pthread_cond_timedwait(&self->park_cv, &self->park_lock, &ts);

            /* On Linux/i386 systems using LinuxThreads, pthread_cond_timedwait
               is implemented using sigjmp/longjmp.  This resets the fpu
               control word back to 64-bit precision.  The macro is empty for
               sane platforms. */ 

            FPU_HACK;
        } else {
            classlibSetThreadState(self, PARKED);
            pthread_cond_wait(&self->park_cv, &self->park_lock);
        }
        
        /* If we were unparked park_state will have been updated,
           but not if the wait timed out.  Only update if it's
           blocked, to avoid losing a possible permit */
        if(self->park_state == PARK_BLOCKED)
            self->park_state = PARK_RUNNING;

        classlibSetThreadState(self, RUNNING);

        enableSuspend(self);
    }

    pthread_mutex_unlock(&self->park_lock);
}

void threadUnpark(Thread *thread) {
    if(thread->park_state != PARK_PERMIT) {

        /* Spin until we can get the park lock.  This avoids having
           to disable suspension around pthread_mutex_lock */
        while(pthread_mutex_trylock(&thread->park_lock))
            sched_yield();

        /* If another thread has given a permit while we were
           waiting for the lock do nothing.  Else increase the
           state by one (BLOCKED -> RUNNING, RUNNING -> PERMIT)
           and if the thread was blocked signal it */

        if(thread->park_state != PARK_PERMIT &&
                  thread->park_state++ == PARK_BLOCKED)
            pthread_cond_signal(&thread->park_cv);

        pthread_mutex_unlock(&thread->park_lock);
    }
}

Thread *findHashedThread(Thread *thread, long long id) {

#define DELETED ((void*)-1)
#define FOUND(ptr1, ptr2) ptr2
#define PREPARE(thread_id) thread
#define SCAVENGE(ptr) ptr == DELETED
#define HASH(thread_id) (int)thread_id
#define COMPARE(thread_id, ptr, hash1, hash2) ptr != DELETED && \
                             (hash1 == hash2 && thread_id == javaThreadId(ptr))

    Thread *ptr;

    /* Add if absent, scavenge, locked */
    findHashEntry(thread_id_map, id, ptr, (thread != NULL), TRUE, TRUE);

    return ptr;
}

void addThreadToHash(Thread *thread) {
    findHashedThread(thread, javaThreadId(thread));
}

Thread *findThreadById(long long id) {
    return findHashedThread(NULL, id);
}

void deleteThreadFromHash(Thread *thread) {

#undef HASH
#undef COMPARE
#define HASH(ptr) (int)javaThreadId(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) ptr1 == ptr2

    deleteHashEntry(thread_id_map, thread, TRUE);
}

void *getStackTop(Thread *thread) {
    return thread->stack_top;
}

void *getStackBase(Thread *thread) {
    return thread->stack_base;
}

long long jThread2ThreadId(Object *jthread) {
    return INST_DATA(jthread, long long, threadId_offset);
}

long long javaThreadId(Thread *thread) {
    return jThread2ThreadId(thread->ee->thread);
}

Thread *jThread2Thread(Object *jThread) {
    return classlibJThread2Thread(jThread);
}

Thread *threadSelf() {
#ifdef HAVE_TLS
    return self;
#else
    return (Thread*)pthread_getspecific(self);
#endif
}

void setThreadSelf(Thread *thread) {
#ifdef HAVE_TLS
    self = thread;
#else
    pthread_setspecific(self, thread);
#endif
}

ExecEnv *getExecEnv() {
    return threadSelf()->ee;
}

char *getThreadStateString(Thread *thread) {
    switch(classlibGetThreadState(thread)) {
        case CREATING:
            return "NEW";
        case RUNNING:
            return "RUNNABLE";
        case PARKED:
        case WAITING:
        case OBJECT_WAIT:
            return "WAITING";
        case SLEEPING:
        case TIMED_PARKED:
        case TIMED_WAITING:
        case OBJECT_TIMED_WAIT:
            return "TIMED_WAITING";
        case BLOCKED:
            return "BLOCKED";
        case TERMINATED:
            return "TERMINATED";
    }
    return "INVALID";
}

int getThreadsCount() {
    return threads_count;
}

int getPeakThreadsCount() {
    return peak_threads_count;
}

long long getTotalStartedThreadsCount() {
    return total_started_threads_count;
}

void resetPeakThreadsCount() {
    Thread *self = threadSelf();

    /* Grab the thread lock to protect against
       concurrent update by threads starting/dying */
    disableSuspend(self);
    pthread_mutex_lock(&lock);
    peak_threads_count = threads_count;
    pthread_mutex_unlock(&lock);
    enableSuspend(self);
}

void initialiseJavaStack(ExecEnv *ee) {
   int stack_size = ee->stack_size
          ? (ee->stack_size > MIN_STACK ? ee->stack_size : MIN_STACK)
          : dflt_stack_size;
   char *stack = sysMalloc(stack_size);
   MethodBlock *mb = (MethodBlock *)stack;
   Frame *top = (Frame *)(mb + 1);

   top->ostack = (uintptr_t*)(top + 1);
   top->lvars = (uintptr_t*)top;
   mb->args_count = 0;
   mb->max_stack = 0;
   top->prev = NULL;
   top->mb = mb;

   ee->stack = stack;
   ee->last_frame = top;
   ee->stack_size = stack_size;
   ee->stack_end = stack + stack_size-STACK_RED_ZONE_SIZE;
}

Object *initJavaThread(Thread *thread, char is_daemon, char *name,
                       Object *group) {

    Object *jlthread, *thread_name = NULL;

    /* Create the java.lang.Thread object */
    if((jlthread = allocObject(thread_class)) == NULL)
        return NULL;

    thread->ee->thread = jlthread;

    /* Create the string for the thread name.  If null is specified
       the initialiser method will generate a name of the form Thread-X */
    if(name != NULL && (thread_name = Cstr2String(name)) == NULL)
        return NULL;

    if(!classlibInitJavaThread(thread, jlthread, thread_name, group,
                               is_daemon, NORM_PRIORITY))
        return NULL;

    /* Add thread to thread ID map hash table. */
    addThreadToHash(thread);

    return jlthread;
}

void initThread(Thread *thread, char is_daemon, void *stack_base) {

    /* Create the thread stack and store the thread structure in
       thread-specific memory */
    initialiseJavaStack(thread->ee);
    setThreadSelf(thread);

    /* Initialise wait condvar (the condvar is per-thread,
       not per-monitor) */
    pthread_cond_init(&thread->wait_cv, NULL);

    /* Initialise per-thread lock/condvar used for parking
       and set initial park state */
    thread->park_state = PARK_RUNNING;
    pthread_cond_init(&thread->park_cv, NULL);
    pthread_mutex_init(&thread->park_lock, NULL);

    /* Record the thread's stack base */
    thread->stack_base = stack_base;

    /* Grab thread list lock.  This also stops suspension */
    pthread_mutex_lock(&lock);

    /* If all threads are suspended (i.e. GC in progress) we cannot start
       until the threads are resumed.  Record we're waiting and wait until
       the world is restarted... */

    threads_waiting_to_start++;

    while(all_threads_suspended)
        pthread_cond_wait(&cv, &lock);

    threads_waiting_to_start--;

    /* Add to thread list... After this point (once we release the lock)
       we are suspendable */
    if((thread->next = main_thread.next))
        main_thread.next->prev = thread;
    thread->prev = &main_thread;
    main_thread.next = thread;

    /* Keep track of threads counts */
    if(++threads_count > peak_threads_count)
        peak_threads_count = threads_count;
    total_started_threads_count++;

    /* If we're not a deamon thread the main thread must
       wait until we exit */
    if(!is_daemon)
        non_daemon_thrds++;

    /* Get a thread ID (used in thin locking -- done here
       as the thread lock must be held) */
    thread->id = genThreadID();

    pthread_mutex_unlock(&lock);
}

void signalThreadRunning(Thread *thread) {
    disableSuspend(thread);
    pthread_mutex_lock(&lock);

    classlibSetThreadState(thread, RUNNING);
    pthread_cond_broadcast(&cv);

    pthread_mutex_unlock(&lock);
    enableSuspend(thread);
}

Thread *attachThread(char *name, char is_daemon, void *stack_base,
                     Thread *thread, Object *group) {

    Object *java_thread;

    /* Create the ExecEnv for the thread */
    ExecEnv *ee = sysMalloc(sizeof(ExecEnv));
    memset(ee, 0, sizeof(ExecEnv));

    thread->tid = pthread_self();
    thread->ee = ee;

    /* Complete initialisation of the thread structure, create the thread
       stack and add the thread to the thread list */
    initThread(thread, is_daemon, stack_base);

    /* Initialise the Java-level thread objects representing this thread */
    if((java_thread = initJavaThread(thread, is_daemon, name, group)) == NULL)
        return NULL;

    /* Set state to running and notify any waiting thread */
    signalThreadRunning(thread);

    /* We're now attached to the VM...*/
    TRACE("Thread %p id: %d attached\n", thread, thread->id);
    return thread;
}

/* Call uncaughtException on the thread's exception handler, or the
   thread's group if this is unset */

void uncaughtException() {
    Thread *thread = threadSelf();
    ExecEnv *ee = thread->ee;
    Object *jThread = ee->thread;
    Object *excep = exceptionOccurred();
    Object *group = INST_DATA(jThread, Object*, group_offset);
    FieldBlock *fb = findField(thread_class,
                        classlibExceptionHandlerName(),
                        SYMBOL(sig_java_lang_Thread_UncaughtExceptionHandler));
    Object *thread_handler = fb == NULL ? NULL :
                                  INST_DATA(jThread, Object*, fb->u.offset);
    Object *handler = thread_handler == NULL ? group : thread_handler;
    MethodBlock *uncaught_mb = lookupMethod(handler->class,
                              SYMBOL(uncaughtException),
                              SYMBOL(_java_lang_Thread_java_lang_Throwable__V));

    if(uncaught_mb != NULL) {
        clearException();
        executeMethod(handler, uncaught_mb, jThread, excep);
 
        /* If an exception occurred while trying to handle
           the exception reinstate the original exception. */
        if(exceptionOccurred())
            setException(excep);
    }

    /* If no method is found, or an error occurred, try to print
       the exception (if it was handled, no exception will be
       pending and the call will do nothing) */
    printException();
}

void *detachThread(Thread *thread) {
    Object *keep_alive;
    ExecEnv *ee = thread->ee;
    Object *java_thread = ee->thread;
    Object *group = INST_DATA(java_thread, Object*, group_offset);

    /* If there's an exception pending, it is uncaught */
    if(exceptionOccurred0(ee))
        uncaughtException();

    /* Don't do anything if this is the main thread */
    if(thread->prev == NULL)
        return NULL;

    /* remove thread from thread group */
    executeMethod(group, (CLASS_CB(group->class))->
                              method_table[rmveThrd_mtbl_idx], java_thread);

    /* Remove thread from the ID map hash table */
    deleteThreadFromHash(thread);

    objectLock(java_thread);

    /* Mark the thread as terminated.  This state is used in determining
       if the thread is alive and so must be done before notifying joining
       threads.  The VM thread structure is tied to a Java-level object 
       (see comment below).  The keep_alive is an object which must be
       kept alive to prevent the structure from being freed while we are
       still accessing it */
    keep_alive = classlibMarkThreadTerminated(java_thread);

    /* Notify any threads waiting on the thread object -
        these are joining this thread */
    objectNotifyAll(java_thread);

    objectUnlock(java_thread);

    /* Thread's about to die, so no need to enable suspend
       afterwards. */
    disableSuspend(thread);

    /* Grab global lock, and update thread structures protected by
       it (thread list, thread ID and number of daemon threads) */
    pthread_mutex_lock(&lock);

    /* remove from thread list... */
    if((thread->prev->next = thread->next))
        thread->next->prev = thread->prev;

    /* One less live thread */
    threads_count--;

    /* Recycle the thread's thread ID */
    freeThreadID(thread->id);

    /* Handle daemon thread status */
    if(!INST_DATA(java_thread, int, daemon_offset))
        non_daemon_thrds--;

    pthread_mutex_unlock(&lock);

    /* It is safe to free the thread's ExecEnv and stack now as these are
       only used within the thread.  It is _not_ safe to free the native
       thread structure as another thread may be concurrently accessing it.
       However, they must have a reference to the java level thread --
       therefore, it is safe to free during GC when the thread is determined
       to be no longer reachable. */
    sysFree(ee->stack);
    sysFree(ee);

    /* If no more daemon threads notify the main thread (which
       may be waiting to exit VM).  Note, this is not protected
       by lock, but main thread checks again */

    if(non_daemon_thrds == 0) {
        /* No need to bother with disabling suspension
           around lock, as we're no longer on thread list */
        pthread_mutex_lock(&exit_lock);
        pthread_cond_signal(&exit_cv);
        pthread_mutex_unlock(&exit_lock);
    }

    /* Finally, clear the thread local data */
    setThreadSelf(NULL);

    TRACE("Thread %p id: %d detached from VM\n", thread, thread->id);
    return keep_alive;
}

void *threadStart(void *arg) {
    Thread *thread = (Thread *)arg;
    Object *jThread = thread->ee->thread;

    /* Parent thread created thread with suspension disabled.
       This is inherited so we need to enable */
    enableSuspend(thread);

    /* Complete initialisation of the thread structure, create the thread
       stack and add the thread to the thread list */
    initThread(thread, INST_DATA(jThread, int, daemon_offset), &thread);

    /* Add thread to thread ID map hash table. */
    addThreadToHash(thread);

    /* Set state to running and notify creating thread */
    signalThreadRunning(thread);

    /* Execute the thread's run method */
    executeMethod(jThread, CLASS_CB(jThread->class)->method_table[run_mtbl_idx]);

    /* Run has completed.  Detach the thread from the VM and exit */
    detachThread(thread);

    TRACE("Thread %p id: %d exited\n", thread, thread->id);
    return NULL;
}

void createJavaThread(Object *jThread, long long stack_size) {
    Thread *self = threadSelf();
    ExecEnv *ee = sysMalloc(sizeof(ExecEnv));
    Thread *thread = sysMalloc(sizeof(Thread));

    memset(ee, 0, sizeof(ExecEnv));
    memset(thread, 0, sizeof(Thread));

    thread->ee = ee;
    ee->thread = jThread;
    ee->stack_size = stack_size;

    if(!classlibCreateJavaThread(thread, jThread)) {
        sysFree(thread);
        sysFree(ee);
        return;
    }

    disableSuspend(self);

    if(pthread_create(&thread->tid, &attributes, threadStart, thread)) {
        classlibMarkThreadTerminated(jThread);
        sysFree(ee);
        enableSuspend(self);
        signalException(java_lang_OutOfMemoryError, "can't create thread");
        return;
    }

    pthread_mutex_lock(&lock);

    /* Wait for thread to start */
    while(classlibGetThreadState(thread) == CREATING)
        pthread_cond_wait(&cv, &lock);

    pthread_mutex_unlock(&lock);
    enableSuspend(self);
}

static void initialiseSignalMask();

Thread *attachJNIThread(char *name, char is_daemon, Object *group) {
    Thread *thread = sysMalloc(sizeof(Thread));
    void *stack_base = nativeStackBase();

    /* If no group is given add it to the main group */
    if(group == NULL)
        group = INST_DATA(main_ee.thread, Object*, group_offset);

    /* Initialise internal thread structure */
    memset(thread, 0, sizeof(Thread));

    /* Externally created threads will not inherit signal state */
    initialiseSignalMask();

    /* Initialise the thread and add it to the VM thread list */
    return attachThread(name, is_daemon, stack_base, thread, group);
}

void detachJNIThread(Thread *thread) {
    /* The JNI spec says we should release all locks held by this thread.
       We don't do this (yet), and only remove the thread from the VM. */
    detachThread(thread);
}

static void *shell(void *args) {
    void *start = ((void**)args)[1];
    Thread *self = ((Thread**)args)[2];

    if(main_exited)
        return NULL;

    /* VM helper threads should be added to the system group, but this doesn't
       exist.  As the root group is main, we add it to that for now... */
    attachThread(((char**)args)[0], TRUE, &self, self,
                 INST_DATA(main_ee.thread, Object*, group_offset));

    sysFree(args);
    (*(void(*)(Thread*))start)(self);
    return NULL;
}

void createVMThread(char *name, void (*start)(Thread*)) {
    Thread *thread = sysMalloc(sizeof(Thread));
    void **args = sysMalloc(3 * sizeof(void*));
    pthread_t tid;

    args[0] = name;
    args[1] = start;
    args[2] = thread;

    memset(thread, 0, sizeof(Thread));
    pthread_create(&tid, &attributes, shell, args);

    /* Wait for thread to start */

    pthread_mutex_lock(&lock);
    while(classlibGetThreadState(thread) == CREATING)
        pthread_cond_wait(&cv, &lock);
    pthread_mutex_unlock(&lock);
}

Object *runningThreadStackTrace(Thread *thread, int max_depth,
                                                int *in_native) {
    int depth = 0;
    void **trace = NULL;
    Thread *self = threadSelf();
    int is_self = thread == self;

    if(!is_self) {
        disableSuspend(self);
        pthread_mutex_lock(&lock);
    }

    if(threadIsAlive(thread)) {
        Frame *last;

        if(!is_self)
            suspendThread(thread);

        last = thread->ee->last_frame;

        if(last->prev != NULL) {
            depth = countStackFrames(last, max_depth);
            trace = alloca(depth * 2 * sizeof(void*));

            stackTrace2Buffer(last, trace, depth);
        }

        if(in_native != NULL)
            *in_native = last->prev == NULL ||
                              last->mb->access_flags & ACC_NATIVE;

        if(!is_self)
            resumeThread(thread);
    }

    if(!is_self) {
        pthread_mutex_unlock(&lock);
        enableSuspend(self);
    }

    return convertTrace2Elements(trace, depth * 2);
}

void suspendThread(Thread *thread) {
    thread->suspend = TRUE;
    MBARRIER();

    if(thread->suspend_state == SUSP_NONE) {
        TRACE("Sending suspend signal to thread %p id: %d\n",
              thread, thread->id);
        pthread_kill(thread->tid, SIGUSR1);
    }

    while(thread->suspend_state != SUSP_BLOCKING &&
          thread->suspend_state != SUSP_SUSPENDED) {
        TRACE("Waiting for thread %p id: %d to suspend\n",
              thread, thread->id);
        sched_yield();
    }
}

void resumeThread(Thread *thread) {
    thread->suspend = FALSE;
    MBARRIER();

    if(thread->suspend_state == SUSP_SUSPENDED) {
        TRACE("Sending resume signal to thread %p id: %d\n",
              thread, thread->id);
        pthread_kill(thread->tid, SIGUSR1);
    }

    while(thread->suspend_state == SUSP_SUSPENDED) {
        TRACE("Waiting for thread %p id: %d to resume\n", thread,
              thread->id);
        sched_yield();
    }
}

void suspendAllThreads(Thread *self) {
    Thread *thread;

    TRACE("Thread %p id: %d is suspending all threads\n", self, self->id);
    pthread_mutex_lock(&lock);

    for(thread = &main_thread; thread != NULL; thread = thread->next) {
        if(thread == self)
            continue;

        thread->suspend = TRUE;
        MBARRIER();

        if(thread->suspend_state == SUSP_NONE) {
            TRACE("Sending suspend signal to thread %p id: %d\n",
                  thread, thread->id);
            if(pthread_kill(thread->tid, SIGUSR1) == ESRCH) {
                /* ESRCH indicates that the thread has died.  This can only
                   occur when an external thread has been attached to the VM
                   via JNI and it has exited without detaching.  Although it
                   is a user error, it will deadlock the suspension code as it
                   will hang waiting for the thread to suspend.  Set the state
                   to BLOCKING, to ignore the thread. Note, no attempt is made
                   to clean-up the error; the thread will still appear to be
                   "live" (as with Hotspot).  We simply stop the thread from
                   hanging the VM. */
                TRACE("Setting thread %p id: %d state to BLOCKING "
                      "as it has died\n", thread, thread->id);
                thread->suspend_state = SUSP_BLOCKING;
            }
        }
    }

    for(thread = &main_thread; thread != NULL; thread = thread->next) {
        if(thread == self)
            continue;

        while(thread->suspend_state != SUSP_BLOCKING &&
              thread->suspend_state != SUSP_SUSPENDED) {
            TRACE("Waiting for thread %p id: %d to suspend\n",
                  thread, thread->id);
            sched_yield();
        }
    }

    all_threads_suspended = TRUE;

    TRACE("All threads suspended...\n");
    pthread_mutex_unlock(&lock);
}

void resumeAllThreads(Thread *self) {
    Thread *thread;

    TRACE("Thread %p id: %d is resuming all threads\n", self, self->id);
    pthread_mutex_lock(&lock);

    for(thread = &main_thread; thread != NULL; thread = thread->next) {
        if(thread == self)
            continue;

        thread->suspend = FALSE;
        MBARRIER();

        if(thread->suspend_state == SUSP_SUSPENDED) {
            TRACE("Sending resume signal to thread %p id: %d\n",
                  thread, thread->id);
            pthread_kill(thread->tid, SIGUSR1);
        }
    }

    for(thread = &main_thread; thread != NULL; thread = thread->next) {
        while(thread->suspend_state == SUSP_SUSPENDED) {
            TRACE("Waiting for thread %p id: %d to resume\n",
                  thread, thread->id);
            sched_yield();
        }
    }

    all_threads_suspended = FALSE;
    if(threads_waiting_to_start) {
        TRACE("%d threads waiting to start...\n", threads_waiting_to_start);
        pthread_cond_broadcast(&cv);
    }

    TRACE("All threads resumed...\n");
    pthread_mutex_unlock(&lock);
}

static void suspendLoop(Thread *thread) {
    char old_state = thread->suspend_state;
    sigjmp_buf env;
    sigset_t mask;

    sigsetjmp(env, FALSE);

    thread->stack_top = &env;
    thread->suspend_state = SUSP_SUSPENDED;
    MBARRIER();

    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGTERM);

    while(thread->suspend && old_state == SUSP_NONE)
        sigsuspend(&mask);

    thread->suspend_state = old_state;
    MBARRIER();
}

static void suspendHandler(int sig) {
    Thread *thread = threadSelf();
    suspendLoop(thread);
}

/* The "slow" disable.  Normally used when a thread enters
   a blocking code section, such as waiting on a lock.  */

void disableSuspend0(Thread *thread, void *stack_top) {
    sigset_t mask;

    thread->stack_top = stack_top;
    thread->suspend_state = SUSP_BLOCKING;
    MBARRIER();

    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

void enableSuspend(Thread *thread) {
    sigset_t mask;

    thread->suspend_state = SUSP_NONE;
    MBARRIER();

    if(thread->suspend) {
        TRACE("Thread %p id: %d is self suspending\n", thread, thread->id);
        suspendLoop(thread);
        TRACE("Thread %p id: %d resumed\n", thread, thread->id);
    }

    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
}

/* Fast disable/enable suspend doesn't modify the signal mask
   or save the thread context.  It's used in critical code
   sections where the thread cannot be suspended (such as
   modifying the loaded classes hash table).  Thread supension
   blocks until the thread self-suspends.  */

void fastEnableSuspend(Thread *thread) {

    thread->suspend_state = SUSP_NONE;
    MBARRIER();

    if(thread->suspend) {
        sigset_t mask;

        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        pthread_sigmask(SIG_BLOCK, &mask, NULL);

        TRACE("Thread %p id: %d is fast self suspending\n",
              thread, thread->id);

        suspendLoop(thread);

        TRACE("Thread %p id: %d resumed\n", thread, thread->id);

        pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
    }
}

void printThreadsDump(Thread *self) {
    char buffer[256];
    Thread *thread;

    suspendAllThreads(self);
    jam_printf("\n------ JamVM version %s Full Thread Dump -------\n",
               VERSION);

    for(thread = &main_thread; thread != NULL; thread = thread->next) {
        Object *jThread = thread->ee->thread;
        int priority = INST_DATA(jThread, int, priority_offset);
        int daemon = INST_DATA(jThread, int, daemon_offset);
        Frame *last = thread->ee->last_frame;

        /* Get thread name; we don't use String2Cstr(), as this mallocs
           memory and may deadlock with a thread suspended in
           malloc/realloc/free */
        classlibThreadName2Buff(jThread, buffer, sizeof(buffer));

        jam_printf("\n\"%s\"%s %p priority: %d tid: %p id: %d state: "
                   "%s (0x%x)\n", buffer, daemon ? " (daemon)" : "",
                   thread, priority, thread->tid, thread->id,
                   getThreadStateString(thread),
                   classlibGetThreadState(thread));

        while(last->prev != NULL) {
            for(; last->mb != NULL; last = last->prev) {
                MethodBlock *mb = last->mb;
                ClassBlock *cb = CLASS_CB(mb->class);

                /* Convert slashes in class name to dots.  Similar to
                   above, we don't use slash2DotsDup(), as this mallocs
                   memory */
                slash2DotsBuff(cb->name, buffer, sizeof(buffer)); 
                jam_printf("\tat %s.%s(", buffer, mb->name);

                if(mb->access_flags & ACC_NATIVE)
                    jam_printf("Native method");
                else
                    if(cb->source_file_name == NULL)
                        jam_printf("Unknown source");
                    else {
                        int line = mapPC2LineNo(mb, last->last_pc);
                        jam_printf("%s", cb->source_file_name);
                        if(line != -1)
                            jam_printf(":%d", line);
                    }
                jam_printf(")\n");
            }
            last = last->prev;
        }
    }
    resumeAllThreads(self);
}

static void initialiseSignalMask() {
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGPIPE);
    sigprocmask(SIG_BLOCK, &mask, NULL);
}

static int initialiseSignals() {
    struct sigaction act;

    /* Initialise signal mask.  Signal masks are per-thread,
       but as this is the main thread it will be inherited
       by all threads created wtihin Java */
    initialiseSignalMask();

    /* Setup signal handler for thread suspension.  Signal
       handlers are process-wide */

    act.sa_handler = suspendHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &act, NULL);

    /* Do classlib specific initialisation */
    return classlibInitialiseSignals();
}

/* garbage collection support */

extern void scanThread(Thread *thread);

void scanThreads() {
    Thread *thread;

    pthread_mutex_lock(&lock);
    for(thread = &main_thread; thread != NULL; thread = thread->next)
        scanThread(thread);
    pthread_mutex_unlock(&lock);
}

int systemIdle(Thread *self) {
    Thread *thread;

    for(thread = &main_thread; thread != NULL; thread = thread->next)
        if(thread != self && classlibGetThreadState(thread) == RUNNING)
            return FALSE;

    return TRUE;
}

Thread *findRunningThreadByTid(int tid) {
    Thread *thread, *self = threadSelf();

    disableSuspend(self);
    pthread_mutex_lock(&lock);

    for(thread = &main_thread; thread != NULL && thread->id != tid;
        thread = thread->next);

    pthread_mutex_unlock(&lock);
    enableSuspend(self);

    return thread;
}

Object *runningThreadObjects() {
    Class *array_class = findArrayClass("[Ljava/lang/Thread;");
    Thread *thread, *self = threadSelf();
    Object **threads, *array;
    int count, i = 0;

    if(array_class == NULL)
        return NULL;

    disableSuspend(self);
    pthread_mutex_lock(&lock);

    count = threads_count;
    threads = alloca(count * sizeof(Object*));

    for(thread = &main_thread; thread != NULL; thread = thread->next)
        threads[i++] = thread->ee->thread;

    pthread_mutex_unlock(&lock);
    enableSuspend(self);

    if((array = allocArray(array_class, count, sizeof(Object*))) == NULL)
        return NULL;

    for(i = 0; i < count; i++)
        ARRAY_DATA(array, Object*)[i] = threads[i];

    return array;
}

void exitVM(int status) {
    main_exited = TRUE;

    /* Execute System.exit() to run any registered shutdown hooks.
       In the unlikely event that System.exit() can't be found, or
       it returns, fall through and exit. */

    if(!VMInitialising()) {
        Class *system = findSystemClass(SYMBOL(java_lang_System));
        if(system) {
            MethodBlock *exit = findMethod(system, SYMBOL(exit), SYMBOL(_I__V));
            if(exit)
                executeStaticMethod(system, exit, status);
        }
    }

    jamvm_exit(status);
}

void mainThreadWaitToExitVM() {
    Thread *self = threadSelf();
    TRACE("Waiting for %d non-daemon threads to exit\n", non_daemon_thrds);

    disableSuspend(self);
    pthread_mutex_lock(&exit_lock);

    classlibSetThreadState(self, WAITING);
    while(non_daemon_thrds)
        pthread_cond_wait(&exit_cv, &exit_lock);

    pthread_mutex_unlock(&exit_lock);
    enableSuspend(self);
}

void mainThreadSetContextClassLoader(Object *loader) {
    FieldBlock *fb = findField(thread_class, SYMBOL(contextClassLoader),
                                             SYMBOL(sig_java_lang_ClassLoader));
    if(fb != NULL)
        INST_DATA(main_ee.thread, Object*, fb->u.offset) = loader;
}

int initialiseThreadStage1(InitArgs *args) {
    size_t size;

    /* Set the default size of the Java stack for each _new_ thread */
    dflt_stack_size = args->java_stack;

    /* Initialise internal locks and pthread state */
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cv, NULL);

    pthread_mutex_init(&exit_lock, NULL);
    pthread_cond_init(&exit_cv, NULL);

#ifndef HAVE_TLS
    pthread_key_create(&self, NULL);
#endif

    pthread_attr_init(&attributes);
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

    /* Ensure the thread stacks are at least 1 megabyte in size */
    pthread_attr_getstacksize(&attributes, &size);
    if(size < 1*MB)
        pthread_attr_setstacksize(&attributes, 1*MB);

    monitorInit(&sleep_mon);
    initHashTable(thread_id_map, HASHTABSZE, TRUE);

    /* We need to cache field and method offsets to create and initialise
       threads.  However, the class loading component requires a valid
       thread and ExecEnv.  To get round this we initialise the main thread
       in two parts.  First part is sufficient to _load_ classes, but it
       is still not fully setup so we don't allow initialisers to run */

    main_thread.stack_base = args->main_stack_base;
    main_thread.tid = pthread_self();
    main_thread.id = genThreadID();
    main_thread.ee = &main_ee;

    initialiseJavaStack(&main_ee);
    setThreadSelf(&main_thread);

    pthread_cond_init(&main_thread.wait_cv, NULL);

    main_thread.park_state = PARK_RUNNING;
    pthread_cond_init(&main_thread.park_cv, NULL);
    pthread_mutex_init(&main_thread.park_lock, NULL);

    return TRUE;
}

int initialiseThreadStage2(InitArgs *args) {
    Class *thrdGrp_class;
    Object *main_group, *java_thread;
    FieldBlock *priority, *threadId;
    FieldBlock *group, *daemon, *name;
    MethodBlock *run, *remove_thread;

    /* Load thread class and register reference for compaction threading */
    if((thread_class = findSystemClass0(SYMBOL(java_lang_Thread))) == NULL)
        goto error;

    registerStaticClassRef(&thread_class);

    name = findField(thread_class, SYMBOL(name), classlibThreadNameType());
    daemon = findField(thread_class, SYMBOL(daemon), SYMBOL(Z));
    group = findField(thread_class, SYMBOL(group),
                                    SYMBOL(sig_java_lang_ThreadGroup));
    priority = findField(thread_class, SYMBOL(priority), SYMBOL(I));
    threadId = findField(thread_class, classlibThreadIdName(), SYMBOL(J));

    run = findMethod(thread_class, SYMBOL(run), SYMBOL(___V));

    /* findField and findMethod do not throw an exception... */
    if((run == NULL) || (daemon == NULL)   || (group == NULL)
                     || (priority == NULL) || (threadId == NULL)
                     || (name == NULL))
        goto error;

    name_offset = name->u.offset;
    daemon_offset = daemon->u.offset;
    group_offset = group->u.offset;
    priority_offset = priority->u.offset;
    threadId_offset = threadId->u.offset;
    run_mtbl_idx = run->method_table_index;

    thrdGrp_class = findSystemClass(SYMBOL(java_lang_ThreadGroup));

    if(exceptionOccurred())
        goto error;

    addThread_mb = findMethod(thrdGrp_class, classlibAddThreadName(),
                                             SYMBOL(_java_lang_Thread__V));

    remove_thread = findMethod(thrdGrp_class, classlibRemoveThreadName(),
                                              SYMBOL(_java_lang_Thread__V));

    /* findField and findMethod do not throw an exception... */
    if((addThread_mb == NULL) || (remove_thread == NULL))
        goto error;

    rmveThrd_mtbl_idx = remove_thread->method_table_index;

    /* Classlib specific initialisation prior to main thread being
       setup */
    main_group = classlibThreadPreInit(thread_class, thrdGrp_class);
    if(main_group == NULL)
        goto error;

    /* Initialise the Java-level thread objects for the main thread */
    java_thread = initJavaThread(&main_thread, FALSE, "main", main_group);
    if(java_thread == NULL)
        goto error;

    classlibSetThreadState(&main_thread, RUNNING);

    /* Setup signal handling.  This will be inherited by all
       threads created within Java */
    if(!initialiseSignals())
        goto error;

    /* Classlib specific initialisation once main thread has been
       setup */
    if(!classlibThreadPostInit())
        goto error;

    /* Create the signal handler thread.  It is responsible for
       catching and handling SIGQUIT (thread dump) and SIGINT
       (user-termination of the VM, e.g. via Ctrl-C).  Note it
       must be a valid Java-level thread as it needs to run the
       shutdown hooks in the event of user-termination */
    createVMThread("Signal Handler", classlibSignalThread);

    return TRUE;

error:
    jam_fprintf(stderr, "Error initialising VM (initialiseMainThread)\nCheck "
                        "the README for compatible class-libraries/versions\n");
    printException();
    return FALSE;
}
