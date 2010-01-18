#ifndef _KTHREAD_H
#define _KTHREAD_H
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
#include <linux/config.h>
#endif

#include <linux/kernel.h>
#include <linux/sched.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,40))
  #include <linux/tqueue.h>
  #define tasklet_struct tq_struct
#else
  #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
    #include <linux/devfs_fs_kernel.h>
  #endif
#endif

#include <linux/wait.h>
#include <linux/interrupt.h>

#include <asm/unistd.h>
#include <asm/semaphore.h>

/* a structure to store all information we need
   for our thread */
typedef struct kthread_struct
{
        /* private data */

        /* Linux task structure of thread */
        struct task_struct *thread;
        /* Task queue/Tasklet need to launch thread */
        struct tasklet_struct tq;
        /* function to be started as thread */
        void (*function) (struct kthread_struct *kthread);
        /* semaphore needed on start and creation of thread. */
        struct semaphore startstop_sem;

        /* public data */

        /* queue thread is waiting on. Gets initialized by
           init_kthread, can be used by thread itself.
        */
        wait_queue_head_t queue;
        /* flag to tell thread whether to die or not.
           When the thread receives a signal, it must check
           the value of terminate and call exit_kthread and terminate
           if set.
        */
        int terminate;
        /* additional data to pass to kernel thread */
        void *arg;
} kthread_t;

/* prototypes */

/* start new kthread (called by creator) */
void start_kthread(void (*func)(kthread_t *), kthread_t *kthread);

/* stop a running thread (called by "killer") */
void stop_kthread(kthread_t *kthread);

/* setup thread environment (called by new thread) */
void init_kthread(kthread_t *kthread, char *name);

/* cleanup thread environment (called by thread upon receiving termination signal) */
void exit_kthread(kthread_t *kthread);

#endif
