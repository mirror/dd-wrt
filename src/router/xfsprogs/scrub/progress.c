// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <dirent.h>
#include <pthread.h>
#include <sys/statvfs.h>
#include <time.h>
#include "libfrog/paths.h"
#include "disk.h"
#include "read_verify.h"
#include "xfs_scrub.h"
#include "common.h"
#include "counter.h"
#include "progress.h"

/*
 * Progress Tracking
 *
 * For scrub phases that expect to take a long time, this facility uses
 * the threaded counter and some phase/state information to report the
 * progress of a particular phase to stdout.  Each phase that wants
 * progress information needs to set up the tracker with an estimate of
 * the work to be done and periodic updates when work items finish.  In
 * return, the progress tracker will print a pretty progress bar and
 * twiddle to a tty, or a raw numeric output compatible with fsck -C.
 */
struct progress_tracker {
	FILE			*fp;
	const char		*tag;
	struct ptcounter	*ptc;
	uint64_t		max;
	unsigned int		phase;
	int			rshift;
	int			twiddle;
	bool			isatty;
	bool			terminate;
	pthread_t		thread;

	/* static state */
	pthread_mutex_t		lock;
	pthread_cond_t		wakeup;
};

static struct progress_tracker pt = {
	.lock			= PTHREAD_MUTEX_INITIALIZER,
	.wakeup			= PTHREAD_COND_INITIALIZER,
};

/* Add some progress. */
void
progress_add(
	uint64_t		x)
{
	if (pt.fp)
		ptcounter_add(pt.ptc, x);
}

static const char twiddles[] = "|/-\\";

static void
progress_report(
	uint64_t		sum)
{
	char			buf[81];
	int			tag_len;
	int			num_len;
	int			pbar_len;
	int			plen;

	if (!pt.fp)
		return;

	if (sum > pt.max)
		sum = pt.max;

	/* Emulate fsck machine-readable output (phase, cur, max, label) */
	if (!pt.isatty) {
		snprintf(buf, sizeof(buf), _("%u %"PRIu64" %"PRIu64" %s"),
				pt.phase, sum, pt.max, pt.tag);
		fprintf(pt.fp, "%s\n", buf);
		fflush(pt.fp);
		return;
	}

	/* Interactive twiddle progress bar. */
	if (debug) {
		num_len = snprintf(buf, sizeof(buf),
				"%c %"PRIu64"/%"PRIu64" (%.1f%%)",
				twiddles[pt.twiddle],
				sum >> pt.rshift,
				pt.max >> pt.rshift,
				100.0 * sum / pt.max);
	} else {
		num_len = snprintf(buf, sizeof(buf),
				"%c (%.1f%%)",
				twiddles[pt.twiddle],
				100.0 * sum / pt.max);
	}
	memmove(buf + sizeof(buf) - (num_len + 1), buf, num_len + 1);
	tag_len = snprintf(buf, sizeof(buf), _("Phase %u: |"), pt.phase);
	pbar_len = sizeof(buf) - (num_len + 1 + tag_len);
	plen = (int)((double)pbar_len * sum / pt.max);
	memset(buf + tag_len, '=', plen);
	memset(buf + tag_len + plen, ' ', pbar_len - plen);
	pt.twiddle = (pt.twiddle + 1) % 4;
	fprintf(pt.fp, "%c%s\r%c", START_IGNORE, buf, END_IGNORE);
	fflush(pt.fp);
}

#define NSEC_PER_SEC	(1000000000)
static void *
progress_report_thread(void *arg)
{
	struct timespec		abstime;
	int			ret;

	pthread_mutex_lock(&pt.lock);
	while (1) {
		uint64_t	progress_val;

		/* Every half second. */
		ret = clock_gettime(CLOCK_REALTIME, &abstime);
		if (ret)
			break;
		abstime.tv_nsec += NSEC_PER_SEC / 2;
		if (abstime.tv_nsec > NSEC_PER_SEC) {
			abstime.tv_sec++;
			abstime.tv_nsec -= NSEC_PER_SEC;
		}
		ret = pthread_cond_timedwait(&pt.wakeup, &pt.lock, &abstime);
		if (ret && ret != ETIMEDOUT)
			break;
		if (pt.terminate)
			break;
		ret = ptcounter_value(pt.ptc, &progress_val);
		if (!ret)
			progress_report(progress_val);
	}
	pthread_mutex_unlock(&pt.lock);
	return NULL;
}

/* End a phase of progress reporting. */
void
progress_end_phase(void)
{
	if (!pt.fp)
		return;

	pthread_mutex_lock(&pt.lock);
	pt.terminate = true;
	pthread_mutex_unlock(&pt.lock);
	pthread_cond_broadcast(&pt.wakeup);
	pthread_join(pt.thread, NULL);

	progress_report(pt.max);
	ptcounter_free(pt.ptc);
	pt.max = 0;
	pt.ptc = NULL;
	if (pt.fp) {
		fprintf(pt.fp, CLEAR_EOL);
		fflush(pt.fp);
	}
	pt.fp = NULL;
}

/*
 * Set ourselves up to report progress.  If errors are encountered, this
 * function will log them and return nonzero.
 */
int
progress_init_phase(
	struct scrub_ctx	*ctx,
	FILE			*fp,
	unsigned int		phase,
	uint64_t		max,
	int			rshift,
	unsigned int		nr_threads)
{
	int			ret;

	assert(pt.fp == NULL);
	if (fp == NULL || max == 0) {
		pt.fp = NULL;
		return 0;
	}
	pt.fp = fp;
	pt.isatty = isatty(fileno(fp));
	pt.tag = ctx->mntpoint;
	pt.max = max;
	pt.phase = phase;
	pt.rshift = rshift;
	pt.twiddle = 0;
	pt.terminate = false;

	ret = ptcounter_alloc(nr_threads, &pt.ptc);
	if (ret) {
		str_liberror(ctx, ret, _("allocating progress counter"));
		goto out_max;
	}

	ret = pthread_create(&pt.thread, NULL, progress_report_thread, NULL);
	if (ret) {
		str_liberror(ctx, ret, _("creating progress reporting thread"));
		goto out_ptcounter;
	}

	return 0;

out_ptcounter:
	ptcounter_free(pt.ptc);
	pt.ptc = NULL;
out_max:
	pt.max = 0;
	pt.fp = NULL;
	return ret;
}
