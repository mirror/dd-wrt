/*
 * A system independant way of adding entropy to the kernels pool
 * this way the drivers can focus on the real work and we can take
 * care of pushing it to the appropriate place in the kernel.
 *
 * This should be fast and callable from timers/interrupts
 *
 * Written by David McCullough <david_mccullough@au.securecomputing.com>
 * Copyright (C) 2004-2005 Intel Corporation.
 *
 * LICENSE TERMS
 *
 * The free distribution and use of this software in both source and binary
 * form is allowed (with or without changes) provided that:
 *
 *   1. distributions of this source code include the above copyright
 *      notice, this list of conditions and the following disclaimer;
 *
 *   2. distributions in binary form include the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other associated materials;
 *
 *   3. the copyright holder's name is not used to endorse products
 *      built using this software without specific written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this product
 * may be distributed under the terms of the GNU General Public License (GPL),
 * in which case the provisions of the GPL apply INSTEAD OF those given above.
 *
 * DISCLAIMER
 *
 * This software is provided 'as is' with no explicit or implied warranties
 * in respect of its properties, including, but not limited to, correctness
 * and/or fitness for purpose.
 */

#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/unistd.h>
#include <linux/poll.h>
#include <linux/random.h>
#include <cryptodev.h>

#ifdef FIPS_TEST_RNG
#include "rndtest.h"
#endif

/*
 * a hack to access the debug levels from the crypto driver
 */
extern int *crypto_debug;
#define debug (*crypto_debug)

/*
 * a list of all registered random providers
 */
static LIST_HEAD(random_ops);
static int started = 0;
static int initted = 0;

struct random_op {
	struct list_head random_list;
	u_int32_t driverid;
	int (*read_random)(void *arg, int *buf, int len);
	void *arg;
};

static int random_proc(void *arg);

static pid_t		randomproc = (pid_t) -1;
static spinlock_t	random_lock;

static int errno;
static inline _syscall3(int,open,const char *,file,int, flags, int, mode);
static inline _syscall3(int,ioctl,int,fd,unsigned int,cmd,unsigned long,arg);
static inline _syscall3(int,poll,struct pollfd *,pollfds,unsigned int,nfds,long,timeout);

/*
 * just init the spin locks
 */
static int
crypto_random_init(void)
{
	spin_lock_init(&random_lock);
	initted = 1;
	return(0);
}

/*
 * Add the given random reader to our list (if not present)
 * and start the thread (if not already started)
 *
 * we have to assume that driver id is ok for now
 */
int
crypto_rregister(
	u_int32_t driverid,
	int (*read_random)(void *arg, int *buf, int len),
	void *arg)
{
	unsigned long flags;
	int ret = 0;
	struct random_op	*rops, *tmp;

	dprintk("%s,%d: %s(0x%x, %p, %p)\n", __FILE__, __LINE__,
			__FUNCTION__, driverid, read_random, arg);

	/* FIXME: currently random support is broken for 64bit OS's */
	if (sizeof(int) != sizeof(long))
		return 0;

	if (!initted)
		crypto_random_init();

#if 0
	struct cryptocap	*cap;

	cap = crypto_checkdriver(driverid);
	if (!cap)
		return EINVAL;
#endif

	list_for_each_entry_safe(rops, tmp, &random_ops, random_list) {
		if (rops->driverid == driverid && rops->read_random == read_random)
			return EEXIST;
	}

	rops = (struct random_op *) kmalloc(sizeof(*rops), GFP_KERNEL);
	if (!rops)
		return ENOMEM;

	rops->driverid    = driverid;
	rops->read_random = read_random;
	rops->arg = arg;

	spin_lock_irqsave(&random_lock, flags);
	list_add_tail(&rops->random_list, &random_ops);
	if (!started) {
		randomproc = kernel_thread(random_proc, NULL, CLONE_FS|CLONE_FILES);
		if (randomproc < 0) {
			ret = randomproc;
			printk("crypto: crypto_rregister cannot start random thread; "
					"error %d", ret);
		} else
			started = 1;
	}
	spin_unlock_irqrestore(&random_lock, flags);

	return ret;
}
EXPORT_SYMBOL(crypto_rregister);

int
crypto_runregister_all(u_int32_t driverid)
{
	struct random_op *rops, *tmp;
	unsigned long flags;

	dprintk("%s,%d: %s(0x%x)\n", __FILE__, __LINE__, __FUNCTION__, driverid);

	list_for_each_entry_safe(rops, tmp, &random_ops, random_list) {
		if (rops->driverid == driverid) {
			list_del(&rops->random_list);
			kfree(rops);
		}
	}

	spin_lock_irqsave(&random_lock, flags);
	if (list_empty(&random_ops) && started)
		kill_proc(randomproc, SIGKILL, 1);
	spin_unlock_irqrestore(&random_lock, flags);
	return(0);
}
EXPORT_SYMBOL(crypto_runregister_all);

/*
 * while we can write to /dev/random continue to read random data from
 * the drivers
 */
static int
random_proc(void *arg)
{
	int rfd, n;
	struct pollfd pfd;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	daemonize();
	spin_lock_irq(&current->sigmask_lock);
	sigemptyset(&current->blocked);
	recalc_sigpending(current);
	spin_unlock_irq(&current->sigmask_lock);
	sprintf(current->comm, "random");
#else
	daemonize("random");
	allow_signal(SIGKILL);
#endif

	(void) get_fs();
	set_fs(get_ds());

	rfd = open("/dev/random", O_RDWR, 0);
	if (rfd == -1)
		printk("crypto: open failed /dev/random: %d\n", errno);

	while (rfd != -1) {
#ifdef FIPS_TEST_RNG
#define NUM_INT (RNDTEST_NBYTES/sizeof(int))
#else
#define NUM_INT 32
#endif
		static int			buf[2 + NUM_INT];
		static int			bufcnt  = 0;
		struct random_op	*rops, *tmp;

		if (signal_pending(current)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
			spin_lock_irq(&current->sigmask_lock);
#endif
			flush_signals(current);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
			spin_unlock_irq(&current->sigmask_lock);
#endif
			spin_lock_irq(&random_lock);
			if (list_empty(&random_ops))
				goto exit_locked;
			spin_unlock_irq(&random_lock);
		}

		/*
		 * if we have a certified buffer,  we can send some data
		 * to /dev/random and move along
		 */
#ifdef FIPS_TEST_RNG
		if (bufcnt == NUM_INT) {
#else
		if (bufcnt) {
#endif
			pfd.fd = rfd;
			pfd.events = POLLOUT;
			pfd.revents = 0;

			if (poll(&pfd, 1, -1) == -1) {
				if (errno == EINTR)
					continue;

				printk("crypto: poll failed: %d\n", errno);
				break;
			}

			buf[0] = NUM_INT * sizeof(int) * 8;
			buf[1] = NUM_INT * sizeof(int);
			(void) ioctl(rfd, RNDADDENTROPY, (unsigned long) buf);
			bufcnt = 0;
		}

		while (bufcnt < NUM_INT && !signal_pending(current)) {
			list_for_each_entry_safe(rops, tmp, &random_ops, random_list) {
				n = (*rops->read_random)(rops->arg, &buf[2+bufcnt],
						NUM_INT - bufcnt);
				/* on failure remove the random number generator */
				if (n == -1) {
					list_del(&rops->random_list);
					printk("crypto: RNG (driverid=0x%x) failed, disabling\n",
							rops->driverid);
					kfree(rops);
				} else if (n > 0)
					bufcnt += n;
			}
			schedule();
		}

#ifdef FIPS_TEST_RNG
		if (rndtest_buf((unsigned char *) &buf[2])) {
			dprintk("crypto: buffer had fips errors, discarding\n");
			bufcnt = 0;
		}
#endif
	}

	spin_lock_irq(&random_lock);
exit_locked:
	randomproc = (pid_t) -1;
	started = 0;
	spin_unlock_irq(&random_lock);

	return 0;
}


