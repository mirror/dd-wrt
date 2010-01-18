/* can_sysdep.h - hides differences between individual Linux kernel 
 *                versions and RT extensions 
 * Linux CAN-bus device driver.
 * Written by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifndef _CAN_SYSDEP_H
#define _CAN_SYSDEP_H

#ifdef CAN_WITH_RTL
#include <rtl.h>
#include <rtl_sync.h>
#include <rtl_core.h>
#include <rtl_mutex.h>
#include <rtl_sched.h>
#include <time.h>
#endif /*CAN_WITH_RTL*/

/*#define __NO_VERSION__*/
/*#include <linux/module.h>*/

#include <linux/version.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <asm/errno.h>

#include <asm/io.h>
#include <asm/atomic.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include "lincan_config.h"

/*optional features*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0))
#define CAN_ENABLE_KERN_FASYNC
#ifdef CONFIG_PCI
#define CAN_ENABLE_PCI_SUPPORT
#endif
#ifdef CONFIG_OC_LINCANVME
#define CAN_ENABLE_VME_SUPPORT
#endif
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))
#include <linux/malloc.h>
#else
#include <linux/slab.h>
#endif

#ifdef CAN_ENABLE_PCI_SUPPORT
#include "linux/pci.h"
#endif /*CAN_ENABLE_PCI_SUPPORT*/

/* Next is not sctrictly correct, because of 2.3.0, 2.3.1, 2.3.2
   kernels need next definitions  too */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,2,19)) /* may need correction */
  #define wait_queue_head_t struct wait_queue *
  #define wait_queue_t      struct wait_queue
  #define init_waitqueue_head(queue_head) (*queue_head=NULL)
  #define init_waitqueue_entry(qentry,qtask) \
                        (qentry->next=NULL,qentry->task=qtask)
  #define DECLARE_WAIT_QUEUE_HEAD(name) \
        struct wait_queue * name=NULL
  #define DECLARE_WAITQUEUE(wait, current) \
        struct wait_queue wait = { current, NULL }
  #define init_MUTEX(sem) (*sem=MUTEX)
  #define DECLARE_MUTEX(name) struct semaphore name=MUTEX
#endif /* 2.2.19 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)) && !defined(DECLARE_TASKLET)
  #define tasklet_struct tq_struct
  #define DECLARE_TASKLET(_name, _func, _data) \
                struct tq_struct _name = { sync: 0, routine: _func, data: (void*)_data }

  /* void tasklet_init(struct tasklet_struct *t, void (*func)(unsigned long), unsigned long data); */
  #define tasklet_init(_tasklet, _func, _data) \
    do{ \
       /* (_tasklet)->next=NULL; */ \
       /* Above not needed for 2.2.x and buggy for 2.4.x */ \
       (_tasklet)->sync=0; \
       (_tasklet)->routine=_func; \
       (_tasklet)->data=(void*)_data; \
    }while(0)

  /* void tasklet_schedule(struct tasklet_struct *t) */
  #define tasklet_schedule(_tasklet) \
    do{ \
       queue_task(_tasklet,&tq_immediate); \
       mark_bh(IMMEDIATE_BH); \
    }while(0)

  /* void tasklet_kill(struct tasklet_struct *t); */
  #define tasklet_kill(_tasklet) \
                synchronize_irq()

#endif /* 2.4.0 */


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,7)) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))

#define MINOR_NR \
	(MINOR(file->f_dentry->d_inode->i_rdev))

#else /* Linux kernel < 2.5.7 or >= 2.6.0 */

#define MINOR_NR \
	(minor(file->f_dentry->d_inode->i_rdev))

#endif /* Linux kernel < 2.5.7 or >= 2.6.0 */

#ifndef CAN_WITH_RTL
#if ((LINUX_VERSION_CODE < KERNEL_VERSION(2,5,68)) && !defined(IRQ_RETVAL))
  typedef void can_irqreturn_t;
  #define CAN_IRQ_NONE
  #define CAN_IRQ_HANDLED
  #define CAN_IRQ_RETVAL(x)
#else /* <=2.5.67 */
  typedef irqreturn_t can_irqreturn_t;
  #define CAN_IRQ_NONE    IRQ_NONE
  #define CAN_IRQ_HANDLED IRQ_HANDLED
  #define CAN_IRQ_RETVAL  IRQ_RETVAL
#endif /* <=2.5.67 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
  #define CAN_IRQ_HANDLER_ARGS(irq_number, dev_id) \
		int irq_number, void *dev_id, struct pt_regs *regs
#else /* < 2.6.19 */
  #define CAN_IRQ_HANDLER_ARGS(irq_number, dev_id) \
		int irq_number, void *dev_id
#endif /* < 2.6.19 */
#else /*CAN_WITH_RTL*/
  typedef int can_irqreturn_t;
  #define CAN_IRQ_NONE        0
  #define CAN_IRQ_HANDLED     1
  #define CAN_IRQ_RETVAL(x)   ((x) != 0)
  #define CAN_IRQ_HANDLER_ARGS(irq_number, dev_id) \
		int irq_number, void *dev_id, struct pt_regs *regs
#endif /*CAN_WITH_RTL*/

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,33))
   #define can_synchronize_irq(irqnum) synchronize_irq()
#else /* >=2.5.33 */
   #define can_synchronize_irq synchronize_irq
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))
  #define del_timer_sync del_timer
#endif /* <2.4.0 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9))
   typedef unsigned long can_ioptr_t;
   #define can_ioptr2ulong(ioaddr) ((unsigned long)(ioaddr))
   #define can_ulong2ioptr(addr)   ((unsigned long)(addr))
   #define can_inb(ioaddr) inb(ioaddr)
   #define can_outb(data,ioaddr) outb(data,ioaddr)
   #define can_inw(ioaddr) inb(ioaddr)
   #define can_outw(data,ioaddr) outb(data,ioaddr)
   #define can_inl(ioaddr) inb(ioaddr)
   #define can_outl(data,ioaddr) outb(data,ioaddr)
#else /* >=2.6.9 */
   typedef void __iomem * can_ioptr_t;
   #define can_ioptr2ulong(ioaddr) ((unsigned long __force)(ioaddr))
   #define can_ulong2ioptr(addr)   ((can_ioptr_t)(addr))
   #define can_inb(ioaddr) inb(can_ioptr2ulong(ioaddr))
   #define can_outb(data,ioaddr) outb(data,can_ioptr2ulong(ioaddr))
   #define can_inw(ioaddr) inb(can_ioptr2ulong(ioaddr))
   #define can_outw(data,ioaddr) outb(data,can_ioptr2ulong(ioaddr))
   #define can_inl(ioaddr) inb(can_ioptr2ulong(ioaddr))
   #define can_outl(data,ioaddr) outb(data,can_ioptr2ulong(ioaddr))
#endif

#define can_readb  readb
#define can_writeb writeb
#define can_readw  readw
#define can_writew writew
#define can_readl  readl
#define can_writel writel

#define can_ioport2ioptr can_ulong2ioptr

#ifdef __HAVE_ARCH_CMPXCHG
  #define CAN_HAVE_ARCH_CMPXCHG
#endif

#ifndef CAN_WITH_RTL
/* Standard LINUX kernel */

#define can_spinlock_t             spinlock_t
#define can_spin_irqflags_t        unsigned long
#define can_spin_lock              spin_lock
#define can_spin_unlock            spin_unlock
#define can_spin_lock_irqsave      spin_lock_irqsave
#define can_spin_unlock_irqrestore spin_unlock_irqrestore
#define can_spin_lock_init         spin_lock_init

#ifndef DEFINE_SPINLOCK
#define CAN_DEFINE_SPINLOCK(x)     can_spinlock_t x = SPIN_LOCK_UNLOCKED
#else /*DEFINE_SPINLOCK*/
#define CAN_DEFINE_SPINLOCK        DEFINE_SPINLOCK
#endif /*DEFINE_SPINLOCK*/

#if !defined(CONFIG_PREEMPT_RT) && ( defined(CONFIG_PREEMPT) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) )
#define can_preempt_disable        preempt_disable
#define can_preempt_enable         preempt_enable
#else /*CONFIG_PREEMPT*/
#define can_preempt_disable()      do { } while (0)
#define can_preempt_enable()       do { } while (0)
#endif /*CONFIG_PREEMPT*/

#define can_enable_irq             enable_irq
#define can_disable_irq            disable_irq

#define can_printk                 printk

/* CAN message timestamp source, it is called from interrupt context */
#define can_gettimeofday do_gettimeofday

#else /*CAN_WITH_RTL*/

#define can_spinlock_t             rtl_spinlock_t
#define can_spin_irqflags_t        rtl_irqstate_t
#define can_spin_lock              rtl_spin_lock
#define can_spin_unlock            rtl_spin_unlock
#define can_spin_lock_irqsave      rtl_spin_lock_irqsave
#define can_spin_unlock_irqrestore rtl_spin_unlock_irqrestore
#define can_spin_lock_init         rtl_spin_lock_init

#define CAN_DEFINE_SPINLOCK(x)     can_spinlock_t x = SPIN_LOCK_UNLOCKED

#define can_preempt_disable()      do { } while (0)
#define can_preempt_enable()       do { } while (0)

#define can_enable_irq             rtl_hard_enable_irq
#define can_disable_irq            rtl_hard_disable_irq

#define can_printk                 rtl_printf

/*
 * terrible hack to test rtl_file private_data concept, ugh !!!
 * this would result in crash on architectures,  where 
 * sizeof(int) < sizeof(void *)
 */
#define can_set_rtl_file_private_data(fptr, p) do{ fptr->f_minor=(long)(p); } while(0)
#define can_get_rtl_file_private_data(fptr) ((void*)((fptr)->f_minor))

extern can_spinlock_t can_irq_manipulation_lock;

/* CAN message timestamp source, it is called from interrupt context */
#define can_gettimeofday(ptr) do {\
	  struct timespec temp_timespec;\
	  clock_gettime(CLOCK_REALTIME,&temp_timespec);\
	  ptr->tv_usec=temp_timespec.tv_nsec/1000;\
	  ptr->tv_sec=temp_timespec.tv_sec;\
	} while(0)

#endif /*CAN_WITH_RTL*/

#endif /*_CAN_SYSDEP_H*/
