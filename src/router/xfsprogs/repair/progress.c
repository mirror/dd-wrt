
#include <libxfs.h>
#include "globals.h"
#include "progress.h"
#include "err_protos.h"
#include <signal.h>

#define ONEMINUTE  60
#define ONEHOUR   (60*ONEMINUTE)
#define ONEDAY    (24*ONEHOUR)
#define ONEWEEK   (7*ONEDAY)

static
char *rpt_types[] = {
#define TYPE_INODE 	0
	N_("inodes"),
#define TYPE_BLOCK 	1
	N_("blocks"),
#define TYPE_DIR   	2
	N_("directories"),
#define TYPE_AG		3
	N_("allocation groups"),
#define TYPE_AGI_BUCKET	4
	N_("AGI unlinked buckets"),
#define TYPE_EXTENTS	5
	N_("extents"),
#define TYPE_RTEXTENTS	6
	N_("realtime extents"),
#define TYPE_UNLINKED_LIST 7
	N_("unlinked lists")
};


static
char *rpt_fmts[] = {
#define FMT1 0
N_("        - %02d:%02d:%02d: %s - %llu of %llu %s done\n"),
#define FMT2 1
N_("        - %02d:%02d:%02d: %s - %llu %s done\n"),
};

typedef struct progress_rpt_s {
	short		format;
	char		*msg;
	char		**fmt;
	char		**type;
} progress_rpt_t;

static
progress_rpt_t progress_rpt_reports[] = {
{FMT1, N_("scanning filesystem freespace"),			/*  0 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_AG]},
{FMT1, N_("scanning agi unlinked lists"),			/*  1 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_AG]},
{FMT2, N_("check uncertain AG inodes"),				/*  2 */
	&rpt_fmts[FMT2], &rpt_types[TYPE_AGI_BUCKET]},
{FMT1, N_("process known inodes and inode discovery"),		/*  3 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_INODE]},
{FMT1, N_("process newly discovered inodes"),			/*  4 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_AG]},
{FMT1, N_("setting up duplicate extent list"),			/*  5 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_AG]},
{FMT1, N_("initialize realtime bitmap"),			/*  6 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_BLOCK]},
{FMT1, N_("reset realtime bitmaps"),				/*  7 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_AG]},
{FMT1, N_("check for inodes claiming duplicate blocks"),	/*  8 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_INODE]},
{FMT1, N_("rebuild AG headers and trees"),	 		/*  9 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_AG]},
{FMT1, N_("traversing filesystem"),				/* 10 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_AG]},
{FMT2, N_("traversing all unattached subtrees"),		/* 11 */
	&rpt_fmts[FMT2], &rpt_types[TYPE_DIR]},
{FMT2, N_("moving disconnected inodes to lost+found"),		/* 12 */
	&rpt_fmts[FMT2], &rpt_types[TYPE_INODE]},
{FMT1, N_("verify and correct link counts"),			/* 13 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_INODE]},
{FMT1, N_("verify link counts"),				/* 14 */
	&rpt_fmts[FMT1], &rpt_types[TYPE_INODE]}
};

pthread_t	report_thread;

typedef struct msg_block_s {
	pthread_mutex_t	mutex;
	progress_rpt_t	*format;
	__uint64_t	*done;
	__uint64_t	*total;
	int		count;
	int		interval;
} msg_block_t;
static msg_block_t 	global_msgs;

typedef struct phase_times_s {
	time_t		start;
	time_t		end;
	time_t		duration;
	__uint64_t	item_counts[4];
} phase_times_t;
static phase_times_t phase_times[8];

static void *progress_rpt_thread(void *);
static int current_phase;
static int running;
static __uint64_t prog_rpt_total;

void
init_progress_rpt (void)
{

	/*
	 *  allocate the done vector
	 */

	if ((prog_rpt_done = (__uint64_t *)
		malloc(sizeof(__uint64_t)*glob_agcount)) == NULL ) {
		do_error(_("cannot malloc pointer to done vector\n"));
	}
	bzero(prog_rpt_done, sizeof(__uint64_t)*glob_agcount);

	/*
	 *  Setup comm block, start the thread
	 */

	pthread_mutex_init(&global_msgs.mutex, NULL);
	global_msgs.count = glob_agcount;
	global_msgs.interval = report_interval;
	global_msgs.done   = prog_rpt_done;
	global_msgs.total  = &prog_rpt_total;

	if (pthread_create (&report_thread, NULL,
		progress_rpt_thread, (void *)&global_msgs))
		do_error(_("unable to create progress report thread\n"));

	return;
}

void
stop_progress_rpt(void)
{

	/*
	 *  Tell msg thread to shutdown,
	 *  wait for all threads to finished
	 */

	running = 0;
	pthread_kill (report_thread, SIGHUP);
	pthread_join (report_thread, NULL);
	free(prog_rpt_done);
	return;
}

static void *
progress_rpt_thread (void *p)
{

	int i;
	int caught;
	sigset_t sigs_to_catch;
	struct tm *tmp;
	time_t now, elapsed;
	timer_t timerid;
	struct itimerspec timespec;
	char *msgbuf;
	__uint64_t *donep;
	__uint64_t sum;
	msg_block_t *msgp = (msg_block_t *)p;
	__uint64_t percent;

	if ((msgbuf = (char *)malloc(DURATION_BUF_SIZE)) == NULL)
		do_error (_("progress_rpt: cannot malloc progress msg buffer\n"));

	running = 1;

	/*
	 * Specify a repeating timer that fires each MSG_INTERVAL seconds.
	 */

	timespec.it_value.tv_sec = msgp->interval;
	timespec.it_value.tv_nsec = 0;
	timespec.it_interval.tv_sec = msgp->interval;
	timespec.it_interval.tv_nsec = 0;

	if (timer_create (CLOCK_REALTIME, NULL, &timerid))
		do_error(_("progress_rpt: cannot create timer\n"));

	if (timer_settime (timerid, 0, &timespec, NULL))
		do_error(_("progress_rpt: cannot set timer\n"));

	/*
	 * Main loop - output messages based on periodic signal arrival
	 * set this thread's signal mask to block out all other signals
	 */

	sigemptyset (&sigs_to_catch);
	sigaddset (&sigs_to_catch, SIGALRM);
	sigaddset (&sigs_to_catch, SIGHUP);
	sigwait (&sigs_to_catch, &caught);

	while (caught != SIGHUP) {
		/*
		 *  Allow the mainline to hold off messages by holding
		 *  the lock. We don't want to just skip a period in case the
		 *  reporting interval is very long... people get nervous. But,
		 *  if the interval is very short, we can't let the timer go
		 *  off again without sigwait'ing for it. So disarm the timer
		 *  while we try to get the lock and giveup the cpu... the
		 *  mainline shouldn't take that long.
		 */

		if (pthread_mutex_lock(&msgp->mutex)) {
			do_error(_("progress_rpt: cannot lock progress mutex\n"));
		}

		if (!running)
			break;

		now = time (NULL);
		tmp = localtime ((const time_t *) &now);

		/*
		 *  Sum the work
		 */

		sum = 0;
		donep = msgp->done;
		for (i = 0; i < msgp->count; i++) {
			sum += *donep++;
		}

		percent = 0;
		switch(msgp->format->format) {
		case FMT1:
			if (*msgp->total)
				percent = (sum * 100) / ( *msgp->total );
			sprintf (msgbuf, *msgp->format->fmt,
				tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
				msgp->format->msg, sum,
				*msgp->total, *msgp->format->type);
			break;
		case FMT2:
			sprintf (msgbuf, *msgp->format->fmt,
				tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
				msgp->format->msg, sum,
				*msgp->format->type);
			break;
		}

		do_log(_("%s"), msgbuf);
		elapsed = now - phase_times[current_phase].start;
		if ((msgp->format->format == FMT1) && sum && elapsed &&
			((current_phase == 3) ||
			 (current_phase == 4) ||
			 (current_phase == 7))) {
			/* for inode phase report % complete */
			do_log(
				_("\t- %02d:%02d:%02d: Phase %d: elapsed time %s - processed %d %s per minute\n"),
				tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
				current_phase, duration(elapsed, msgbuf),
				(int) (60*sum/(elapsed)), *msgp->format->type);
			do_log(
	_("\t- %02d:%02d:%02d: Phase %d: %" PRIu64 "%% done - estimated remaining time %s\n"),
				tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
				current_phase, percent,
				duration((int) ((*msgp->total - sum) * (elapsed)/sum), msgbuf));
		}

		if (pthread_mutex_unlock(&msgp->mutex) != 0) {
			do_error(
			_("progress_rpt: error unlock msg mutex\n"));
		}
		sigwait (&sigs_to_catch, &caught);
	}

	if (timer_delete (timerid))
		do_warn(_("cannot delete timer\n"));

	free (msgbuf);
	return (NULL);
}

int
set_progress_msg (int report, __uint64_t total)
{

	if (!ag_stride)
		return (0);

	if (pthread_mutex_lock(&global_msgs.mutex))
		do_error(_("set_progress_msg: cannot lock progress mutex\n"));

	prog_rpt_total = total;
	global_msgs.format = &progress_rpt_reports[report];

	/* reset all the accumulative totals */
	if (prog_rpt_done)
		bzero(prog_rpt_done, sizeof(__uint64_t)*glob_agcount);

	if (pthread_mutex_unlock(&global_msgs.mutex))
		do_error(_("set_progress_msg: cannot unlock progress mutex\n"));

	return (0);
}

__uint64_t
print_final_rpt(void)
{
	int i;
	struct tm *tmp;
	time_t now;
	__uint64_t *donep;
	__uint64_t sum;
	msg_block_t 	*msgp = &global_msgs;
	char		msgbuf[DURATION_BUF_SIZE];

	if (!ag_stride)
		return 0;

	if (pthread_mutex_lock(&global_msgs.mutex))
		do_error(_("print_final_rpt: cannot lock progress mutex\n"));

	bzero(&msgbuf, sizeof(msgbuf));

	now = time (NULL);
	tmp = localtime ((const time_t *) &now);

	/*
	*  Sum the work
	*/

	sum = 0;
	donep = msgp->done;
	for (i = 0; i < msgp->count; i++) {
		sum += *donep++;
	}

	if (report_interval) {
		switch(msgp->format->format) {
		case FMT1:
			sprintf (msgbuf, _(*msgp->format->fmt),
				tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
				_(msgp->format->msg), sum,
				*msgp->total, _(*msgp->format->type));
			break;
		case FMT2:
			sprintf (msgbuf, _(*msgp->format->fmt),
				tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
				_(msgp->format->msg), sum,
				_(*msgp->format->type));
			break;
		}
		do_log(_("%s"), msgbuf);
	}

	if (pthread_mutex_unlock(&global_msgs.mutex))
		do_error(_("print_final_rpt: cannot unlock progress mutex\n"));

	return(sum);
}

void
timediff(int phase)
{
	phase_times[phase].duration =
		phase_times[phase].end - phase_times[phase].start;

}

/*
**  Get the time and save in the phase time
**  array.
*/
char *
timestamp(int end, int phase, char *buf)
{

	time_t    now;
	struct tm *tmp;

	if (verbose > 1)
		cache_report(stderr, "libxfs_bcache", libxfs_bcache);

	now = time(NULL);

	if (end) {
		phase_times[phase].end = now;
		timediff(phase);

		/* total time in slot zero */
		phase_times[0].end = now;
		timediff(0);

		if (phase < 7) {
			phase_times[phase+1].start = now;
			current_phase = phase + 1;
		}
	}
	else {
		phase_times[phase].start = now;
		current_phase = phase;
	}

	if (buf) {
		tmp = localtime((const time_t *)&now);
		sprintf(buf, _("%02d:%02d:%02d"), tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	}
	return(buf);
}

char *
duration(int length, char *buf)
{
	int sum;
	int weeks;
	int days;
	int hours;
	int minutes;
	int seconds;
	char temp[128];

	*buf = '\0';
	weeks = days = hours = minutes = seconds = sum = 0;
	if (length >= ONEWEEK) {
		weeks = length / ONEWEEK;
		sum = (weeks * ONEWEEK);
		if (weeks) {
			sprintf(buf, _("%d week"), weeks);
			if (weeks > 1) strcat(buf, _("s"));
			if ((length-sum) == 0)
				return(buf);
		}
	}
	if (length >= ONEDAY)  {
		days = (length - sum) / ONEDAY;
		sum += (days * ONEDAY);
		if (days) {
			sprintf(temp, _("%d day"), days);
			if (days > 1) strcat(temp, _("s"));
			if (((length-sum) == 0) && (!weeks)) {
				strcpy(buf, temp);
				return(buf);
			}
			else if (weeks) {
				strcat(buf, _(", "));
			}
			strcat(buf, temp);
		}
	}
	if (length >= ONEHOUR) {
		hours = (length - sum) / ONEHOUR;
		sum += (hours * ONEHOUR);
		if (hours) {
			sprintf(temp, _("%d hour"), hours);
			if (hours > 1) strcat(temp, _("s"));
			if (((length-sum) == 0) &&
				(!weeks) && (!days)) {
				strcpy(buf, temp);
				return(buf);
			}
			else if ((weeks) || (days)) {
				strcat(buf, _(", "));
			}
			strcat(buf, temp);
		}

	}
	if (length >= ONEMINUTE) {
		minutes = (length - sum) / ONEMINUTE;
		sum += (minutes * ONEMINUTE);
		if (minutes) {
			sprintf(temp, _("%d minute"), minutes);
			if (minutes > 1) strcat(temp, _("s"));
			if (((length-sum) == 0) &&
				(!weeks) && (!days) && (!hours)) {
				strcpy(buf, temp);
				return(buf);
			}
			else if ((weeks)||(days)||(hours)) {
				strcat(buf, _(", "));
			}
			strcat(buf, temp);
		}
	}
	seconds = length - sum;
	if (seconds) {
		sprintf(temp, _("%d second"), seconds);
		if (seconds > 1) strcat(temp, _("s"));
		if ((weeks)||(days)||(hours)||(minutes))
			strcat(buf, _(", "));
		strcat(buf, temp);
	}

	return(buf);
}

void
summary_report(void)
{
	int i;
	time_t now;
	struct tm end;
	struct tm start;
	char	msgbuf[DURATION_BUF_SIZE];

	now = time(NULL);

	do_log(_("\n        XFS_REPAIR Summary    %s\n"),
		ctime((const time_t *)&now));
	do_log(_("Phase\t\tStart\t\tEnd\t\tDuration\n"));
	for (i = 1; i < 8; i++) {
		localtime_r((const time_t *)&phase_times[i].start, &start);
		localtime_r((const time_t *)&phase_times[i].end, &end);
		if ((no_modify) && (i == 5)) {
			do_log(_("Phase %d:\tSkipped\n"), i);
		}
		else if ((bad_ino_btree) && ((i == 6) || (i == 7))) {
			do_log(_("Phase %d:\tSkipped\n"), i);
		}
		else {
			do_log(
	_("Phase %d:\t%02d/%02d %02d:%02d:%02d\t%02d/%02d %02d:%02d:%02d\t%s\n"), i,
			start.tm_mon+1, start.tm_mday, start.tm_hour, start.tm_min, start.tm_sec,
			end.tm_mon+1, end.tm_mday, end.tm_hour, end.tm_min, end.tm_sec,
			duration(phase_times[i].duration, msgbuf));
		}
	}
	do_log(_("\nTotal run time: %s\n"), duration(phase_times[0].duration, msgbuf));
}
