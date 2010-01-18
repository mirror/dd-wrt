#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
#include <linux/config.h>
#endif

#if defined(MODVERSIONS)
#include <linux/modversions.h>
#endif
#include <linux/kernel.h>
#include <linux/sched.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,40))
  #include <linux/tqueue.h>
#else
  #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
    #include <linux/devfs_fs_kernel.h>
  #endif
#endif

#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/interrupt.h>


#include <asm/semaphore.h>
#include <linux/smp_lock.h>

#include "../include/kthread.h"

/* private functions */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,40))
static void kthread_launcher(void *data)
#else
static void kthread_launcher(unsigned long data)
#endif
{
  kthread_t *kthread = (kthread_t *)data;

  kernel_thread((int (*)(void *))kthread->function, (void *)kthread, 0);
}

/* public functions */

/* create a new kernel thread. Called by the creator. */
void start_kthread(void (*func)(kthread_t *), kthread_t *kthread)
{
    /* initialize the semaphore:
       we start with the semaphore locked. The new kernel
       thread will setup its stuff and unlock it. This
       control flow (the one that creates the thread) blocks
       in the down operation below until the thread has reached
       the up() operation.
    */
    sema_init(&kthread->startstop_sem, 0);

    /* store the function to be executed in the data passed to
       the launcher */
    kthread->function=func;

    /* create the new thread my running a task through keventd */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,40))

    /* initialize the task queue structure */
    kthread->tq.sync = 0;
    INIT_LIST_HEAD(&kthread->tq.list);
    kthread->tq.routine = kthread_launcher;
    kthread->tq.data = kthread;

    /* and schedule it for execution */
    schedule_task(&kthread->tq);
#else
    /* initialize tasklet */
    tasklet_init(&kthread->tq, kthread_launcher, (unsigned long)kthread);
    /* and schedule it for execution */
    tasklet_schedule(&kthread->tq);
#endif

    /* wait till it has reached the setup_thread routine */
    down(&kthread->startstop_sem);
}

/* stop a kernel thread. Called by the removing instance */
void stop_kthread(kthread_t *kthread)
{
    if (kthread->thread == NULL)
    {
        printk("stop_kthread: killing non existing thread!\n");
        return;
    }

    /* this function needs to be protected with the big
    kernel lock (lock_kernel()). The lock must be
       grabbed before changing the terminate
    flag and released after the down() call. */
    lock_kernel();

    /* initialize the semaphore. We lock it here, the
       leave_thread call of the thread to be terminated
       will unlock it. As soon as we see the semaphore
       unlocked, we know that the thread has exited.
    */
    sema_init(&kthread->startstop_sem,0);

    /* We need to do a memory barrier here to be sure that
       the flags are visible on all CPUs. 
    */
    mb();

    /* set flag to request thread termination */
    kthread->terminate = 1;

    /* We need to do a memory barrier here to be sure that
       the flags are visible on all CPUs. 
    */
    mb();
    kill_proc(kthread->thread->pid, SIGKILL, 1);

    /* block till thread terminated */
    down(&kthread->startstop_sem);

    /* release the big kernel lock */
    unlock_kernel();

    /* now we are sure the thread is in zombie state. We
       notify keventd to clean the process up.
    */
    kill_proc(2, SIGCHLD, 1);

}

/* initialize new created thread. Called by the new thread. */
void init_kthread(kthread_t *kthread, char *name)
{
    /* fill in thread structure */
    kthread->thread = current;

    /* initialise termination flag */
    kthread->terminate = 0;

    /* initialise wait queue */
    init_waitqueue_head(&kthread->queue);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,40))
    /* lock the kernel. A new kernel thread starts without
      the big kernel lock, regardless of the lock state
      of the creator (the lock level is *not* inheritated)
    */
    lock_kernel();
/*    daemonize(); */

    /* set signal mask to what we want to respond */
    siginitsetinv(&current->blocked, sigmask(SIGKILL)|sigmask(SIGINT)|sigmask(SIGTERM));

    /* set name of this process (max 15 chars + 0 !) */
    sprintf(current->comm, name);

    /* let others run */
    unlock_kernel();
#else

    daemonize(name);
#endif

    /* tell the creator that we are ready and let him continue */
    up(&kthread->startstop_sem);
}

/* cleanup of thread. Called by the exiting thread. */
void exit_kthread(kthread_t *kthread)
{
    /* we are terminating */

    /* lock the kernel, the exit will unlock it */
    lock_kernel();
    kthread->thread = NULL;
    mb();

    /* notify the stop_kthread() routine that we are terminating. */
    up(&kthread->startstop_sem);
    /* the kernel_thread that called clone() does a do_exit here. */

    /* there is no race here between execution of the "killer" and real termination
       of the thread (race window between up and do_exit), since both the
       thread and the "killer" function are running with the kernel lock held.
       The kernel lock will be freed after the thread exited, so the code
       is really not executed anymore as soon as the unload functions gets
       the kernel lock back.
       The init process may not have made the cleanup of the process here,
       but the cleanup can be done safely with the module unloaded.
    */

}

