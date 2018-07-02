/*
 * Expose some of the kernel scheduler routines
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_linux_sched.c 514727 2014-11-12 03:02:48Z $
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <typedefs.h>
#include <linuxver.h>

int setScheduler(struct task_struct *p, int policy, struct sched_param *param)
{
	int rc = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	rc = sched_setscheduler(p, policy, param);
#endif /* LinuxVer */
	return rc;
}

int get_scheduler_policy(struct task_struct *p)
{
	int rc = SCHED_NORMAL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	rc = p->policy;
#endif /* LinuxVer */
	return rc;
}
