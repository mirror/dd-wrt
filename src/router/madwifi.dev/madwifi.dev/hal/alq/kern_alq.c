/*
 * Copyright (c) 2002-2004, Jeffrey Roberson <jeff@freebsd.org>
 * Copyright (c) 2004 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/dnotify.h>
#include <linux/uio.h>
#include <linux/wait.h>

#include <asm/semaphore.h>
#include <asm/uaccess.h>

#include <sys/queue.h>

#include "alq.h"

/* Async. Logging Queue */
struct alq {
	int	aq_entmax;		/* Max entries */
	int	aq_entlen;		/* Entry length */
	char	*aq_entbuf;		/* Buffer for stored entries */
	int	aq_flags;		/* Queue flags */
	spinlock_t	aq_lock;	/* Queue lock */
	wait_queue_head_t aq_waitq;	/* sleep/wakeup */
	struct file	*aq_fp;		/* Open file handle */
	struct ale	*aq_first;	/* First ent */
	struct ale	*aq_entfree;	/* First free ent */
	struct ale	*aq_entvalid;	/* First ent valid for writing */
	LIST_ENTRY(alq)	aq_act;		/* List of active queues */
	LIST_ENTRY(alq)	aq_link;	/* List of all queues */
};

#define	AQ_WANTED	0x0001		/* Wakeup sleeper when io is done */
#define	AQ_ACTIVE	0x0002		/* on the active list */
#define	AQ_FLUSHING	0x0004		/* doing IO */
#define	AQ_SHUTDOWN	0x0008		/* Queue no longer valid */

/*
 * The ald_lock protects the ald_queues list and the ald_active list.
 */
static spinlock_t ald_lock;
static ATH_LIST_HEAD(, alq) ald_queues;
static ATH_LIST_HEAD(, alq) ald_active;
static int ald_shutingdown = 0;
static pid_t ald_proc = -1;

#define	ALD_LOCK()	spin_lock(&ald_lock)
#define	ALD_UNLOCK()	spin_unlock(&ald_lock)

DECLARE_WAIT_QUEUE_HEAD(ald_waitq);
#define	ALD_WAIT(condition)	do {			\
	ALD_UNLOCK();					\
	wait_event_interruptible(ald_waitq, condition);	\
	ALD_LOCK();					\
} while (0)
#define	ALD_WAKEUP()	wake_up_interruptible(&ald_waitq)

/* Daemon functions */
static int ald_add(struct alq *);
static int ald_rem(struct alq *);
static void ald_startup(void *);
static int ald_daemon(void *);
static void ald_shutdown(void *, int);
static void ald_activate(struct alq *);
static void ald_deactivate(struct alq *);

/* Internal queue functions */
static void alq_shutdown(struct alq *);
static int alq_doio(struct alq *);


/*
 * Add a new queue to the global list.  Fail if we're shutting down.
 */
static int
ald_add(struct alq *alq)
{
	int error;

	error = 0;

	ALD_LOCK();
	if (ald_shutingdown) {
		error = -EBUSY;
		goto done;
	}
	LIST_INSERT_HEAD(&ald_queues, alq, aq_link);
done:
	ALD_UNLOCK();
	return (error);
}

/*
 * Remove a queue from the global list unless we're shutting down.  If so,
 * the ald will take care of cleaning up it's resources.
 */
static int
ald_rem(struct alq *alq)
{
	int error;

	error = 0;

	ALD_LOCK();
	if (ald_shutingdown) {
		error = -EBUSY;
		goto done;
	}
	LIST_REMOVE(alq, aq_link);
done:
	ALD_UNLOCK();
	return (error);
}

/*
 * Put a queue on the active list.  This will schedule it for writing.
 */
static void
ald_activate(struct alq *alq)
{
	LIST_INSERT_HEAD(&ald_active, alq, aq_act);
	ALD_WAKEUP();
}

static void
ald_deactivate(struct alq *alq)
{
	LIST_REMOVE(alq, aq_act);
	alq->aq_flags &= ~AQ_ACTIVE;
}

static void
ald_startup(void *arg)
{
	spin_lock_init(&ald_lock);
	LIST_INIT(&ald_queues);
	LIST_INIT(&ald_active);
}

static int
ald_daemon(void *arg)
{
	int needwakeup;
	struct alq *alq;

	daemonize("ALQ Daemon");
	allow_signal(SIGKILL);

	ALD_LOCK();

	for (;;) {
		if ((alq = LIST_FIRST(&ald_active)) == NULL)
			ALD_WAIT((alq = LIST_FIRST(&ald_active)) != NULL);

		if (signal_pending(current))
			break;

		spin_lock_irq(&alq->aq_lock);
		ald_deactivate(alq);
		ALD_UNLOCK();
		needwakeup = alq_doio(alq);
		spin_unlock_irq(&alq->aq_lock);
		if (needwakeup)
			wake_up_interruptible(&alq->aq_waitq);
		ALD_LOCK();
	}

	ALD_UNLOCK();

	return 0;
}

static void
ald_shutdown(void *arg, int howto)
{
	struct alq *alq;

	ALD_LOCK();

	ald_shutingdown = 1;

	while ((alq = LIST_FIRST(&ald_queues)) != NULL) {
		LIST_REMOVE(alq, aq_link);
		ALD_UNLOCK();
		alq_shutdown(alq);
		ALD_LOCK();
	}
	ALD_UNLOCK();
}

static void
alq_shutdown(struct alq *alq)
{
	unsigned long flags;

	spin_lock_irqsave(&alq->aq_lock, flags);

	/* Stop any new writers. */
	alq->aq_flags |= AQ_SHUTDOWN;

	/* Drain IO */
	if (alq->aq_flags & (AQ_FLUSHING|AQ_ACTIVE)) {
		alq->aq_flags |= AQ_WANTED;
		spin_unlock_irqrestore(&alq->aq_lock, flags);
		wait_event_interruptible(alq->aq_waitq,
			(alq->aq_flags & (AQ_FLUSHING|AQ_ACTIVE)) == 0);
	} else
		spin_unlock_irqrestore(&alq->aq_lock, flags);

	filp_close(alq->aq_fp, /*XXX*/0);
}

/*
 * Flush all pending data to disk.  This operation will block.
 */
static int
alq_doio(struct alq *alq)
{
	mm_segment_t oldfs;
	struct iovec aiov[2];
	struct file *fp;
	struct ale *ale;
	struct ale *alstart;
	int totlen;
	int iov;
	int err;

	fp = alq->aq_fp;
	totlen = 0;
	iov = 0;

	alstart = ale = alq->aq_entvalid;
	alq->aq_entvalid = NULL;

	memset(&aiov, 0, sizeof(aiov));

	do {
		if (aiov[iov].iov_base == NULL)
			aiov[iov].iov_base = ale->ae_data;
		aiov[iov].iov_len += alq->aq_entlen;
		totlen += alq->aq_entlen;
		/* Check to see if we're wrapping the buffer */
		if (ale->ae_data + alq->aq_entlen != ale->ae_next->ae_data)
			iov++;
		ale->ae_flags &= ~AE_VALID;
		ale = ale->ae_next;
	} while (ale->ae_flags & AE_VALID);

	alq->aq_flags |= AQ_FLUSHING;
	spin_unlock_irq(&alq->aq_lock);

	if (iov == 2 || aiov[iov].iov_base == NULL)
		iov--;

	oldfs = get_fs(); set_fs(KERNEL_DS);
	/* XXX ignore errors */
	err = fp->f_op->writev(fp, aiov, iov + 1, &fp->f_pos);
	set_fs(oldfs);
	if (err >= 0)
		dnotify_parent(fp->f_dentry, DN_MODIFY);

	spin_lock_irq(&alq->aq_lock);
	alq->aq_flags &= ~AQ_FLUSHING;

	if (alq->aq_entfree == NULL)
		alq->aq_entfree = alstart;

	if (alq->aq_flags & AQ_WANTED) {
		alq->aq_flags &= ~AQ_WANTED;
		return (1);
	}

	return(0);
}


/* User visible queue functions */

/*
 * Create the queue data structure, allocate the buffer, and open the file.
 */
int
alq_open(struct alq **alqp, const char *file, int size, int count)
{
	struct file *f;
	struct ale *ale;
	struct ale *alp;
	struct alq *alq;
	char *bufp;
	int error;
	int i;

	*alqp = NULL;

	/* NB: no O_TRUNC */
	f = filp_open(file, O_WRONLY | O_LARGEFILE | O_CREAT, 0644);
	if (IS_ERR(f))
		return PTR_ERR(f);

	alq = kmalloc(sizeof(*alq), GFP_KERNEL);
	if (alq == NULL) {
		printk("%s: no memory for alq structure\n", __func__);
		error = -ENOMEM;
		goto bad1;
	}
	memset(alq, 0, sizeof(*alq));
	alq->aq_entbuf = kmalloc(count * size, GFP_KERNEL);
	if (alq->aq_entbuf == NULL) {
		printk("%s: no memory for alq entbuf\n", __func__);
		error = -ENOMEM;
		goto bad2;
	}
	memset(alq->aq_entbuf, 0, count * size);
	alq->aq_first = kmalloc(sizeof(*ale) * count, GFP_KERNEL);
	if (alq->aq_first == NULL) {
		printk("%s: no memory for alq entries\n", __func__);
		error = -ENOMEM;
		goto bad3;
	}
	memset(alq->aq_first, 0, sizeof(*ale) * count);
	alq->aq_fp = f;
	alq->aq_entmax = count;
	alq->aq_entlen = size;
	alq->aq_entfree = alq->aq_first;
	spin_lock_init(&alq->aq_lock);
	init_waitqueue_head(&alq->aq_waitq);

	bufp = alq->aq_entbuf;
	ale = alq->aq_first;
	alp = NULL;

	/* Match up entries with buffers */
	for (i = 0; i < count; i++) {
		if (alp)
			alp->ae_next = ale;
		ale->ae_data = bufp;
		alp = ale;
		ale++;
		bufp += size;
	}

	alp->ae_next = alq->aq_first;

	if ((error = ald_add(alq)) != 0) {
		printk("%s: unable to add alq, probably shutting down\n",
			__func__);
		goto bad4;
	}
	*alqp = alq;

	/* XXX bump module refcnt */

	return (0);
bad4:
	kfree(alq->aq_first);
bad3:
	kfree(alq->aq_entbuf);
bad2:
	kfree(alq);
bad1:
	filp_close(f, /*XXX*/0);
	return error;
}
EXPORT_SYMBOL(alq_open);

/*
 * Copy a new entry into the queue.  If the operation would block either
 * wait or return an error depending on the value of waitok.
 */
int
alq_write(struct alq *alq, void *data, int waitok)
{
	unsigned long flags;
	struct ale *ale;

	local_irq_save(flags);
	if ((ale = alq_get(alq, waitok)) == NULL) {
		local_irq_restore(flags);
		return EWOULDBLOCK;
	}
	memcpy(ale->ae_data, data, alq->aq_entlen);
	alq_post(alq, ale);
	local_irq_restore(flags);

	return 0;
}
EXPORT_SYMBOL(alq_write);

struct ale *
alq_get(struct alq *alq, int waitok)
{
	struct ale *ale;
	struct ale *aln;

	ale = NULL;

	/* XXX caller must block irq */
	spin_lock(&alq->aq_lock);

	/* Loop until we get an entry or we're shutting down */
	if ((alq->aq_flags & AQ_SHUTDOWN) == 0 && 
	    (ale = alq->aq_entfree) == NULL &&
	    (waitok & ALQ_WAITOK)) {
		alq->aq_flags |= AQ_WANTED;
		spin_unlock(&alq->aq_lock);
		wait_event_interruptible(alq->aq_waitq,
		    (alq->aq_flags & AQ_SHUTDOWN) || 
		    (ale = alq->aq_entfree) != NULL ||
		    (waitok & ALQ_WAITOK) == 0);
		spin_lock(&alq->aq_lock);
	}

	if (ale != NULL) {
		aln = ale->ae_next;
		if ((aln->ae_flags & AE_VALID) == 0)
			alq->aq_entfree = aln;
		else
			alq->aq_entfree = NULL;
	} else
		spin_unlock(&alq->aq_lock);

	return (ale);
}
EXPORT_SYMBOL(alq_get);

void
alq_post(struct alq *alq, struct ale *ale)
{
	int activate;

	ale->ae_flags |= AE_VALID;

	if (alq->aq_entvalid == NULL)
		alq->aq_entvalid = ale;

	if ((alq->aq_flags & AQ_ACTIVE) == 0) {
		alq->aq_flags |= AQ_ACTIVE;
		activate = 1;
	} else
		activate = 0;

	spin_unlock(&alq->aq_lock);
	if (activate) {
		ALD_LOCK();
		ald_activate(alq);
		ALD_UNLOCK();
	}
}
EXPORT_SYMBOL(alq_post);

void
alq_flush(struct alq *alq)
{
	int needwakeup = 0;

	ALD_LOCK();
	spin_lock_irq(&alq->aq_lock);
	if (alq->aq_flags & AQ_ACTIVE) {
		ald_deactivate(alq);
		ALD_UNLOCK();
		needwakeup = alq_doio(alq);
	} else
		ALD_UNLOCK();
	spin_unlock_irq(&alq->aq_lock);

	if (needwakeup)
		wake_up_interruptible(&alq->aq_waitq);
}
EXPORT_SYMBOL(alq_flush);

/*
 * Flush remaining data, close the file and free all resources.
 */
void
alq_close(struct alq *alq)
{
	/*
	 * If we're already shuting down someone else will flush and close
	 * the vnode.
	 */
	if (ald_rem(alq) != 0)
		return;

	/*
	 * Drain all pending IO.
	 */
	alq_shutdown(alq);

	/* XXX drop module refcnt */

	kfree(alq->aq_first);
	kfree(alq->aq_entbuf);
	kfree(alq);
}
EXPORT_SYMBOL(alq_close);

/*
 * Module glue.
 */
static	char *version = "0.1";
static	char *mod_info = "alq";

MODULE_AUTHOR("Errno Consulting, Sam Leffler");
MODULE_DESCRIPTION("Low-overhead event collection package");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

static int __init
init_alq(void)
{
	printk(KERN_INFO "%s: version %s\n", mod_info, version);
	ald_startup(NULL);
	ald_proc = kernel_thread(ald_daemon, NULL, CLONE_KERNEL);
	return 0;
}
module_init(init_alq);

static void __exit
exit_alq(void)
{
	if (ald_proc != -1)
		kill_proc(ald_proc, SIGKILL, -1);
	ald_shutdown(NULL, 0);
	printk(KERN_INFO "%s: driver unloaded\n", mod_info);
}
module_exit(exit_alq);
