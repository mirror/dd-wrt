/*
 *   $Id: timer.c,v 1.6 2001/11/14 19:58:11 lutchann Exp $
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996-2000 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <lutchann@litech.org>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>

static struct timer_lst timers_head = {
	{LONG_MAX, LONG_MAX},
	NULL, NULL,
	&timers_head, &timers_head
};

static void alarm_handler(int sig);

static void
schedule_timer(void)
{
	struct timer_lst *tm = timers_head.next;
	struct timeval tv;

	gettimeofday(&tv, NULL);

	if (tm != &timers_head)
	{
		struct itimerval next;
	       
	        memset(&next, 0, sizeof(next));
	       
	        timersub(&tm->expires, &tv, &next.it_value);

		signal(SIGALRM, alarm_handler);

		if ((next.it_value.tv_sec > 0) || 
				((next.it_value.tv_sec == 0) && (next.it_value.tv_usec > 0)))
		{
			dlog(LOG_DEBUG, 4, "calling alarm: %ld secs, %ld usecs", 
					next.it_value.tv_sec, next.it_value.tv_usec);
			setitimer(ITIMER_REAL, &next,  NULL);
		}
		else
		{
			dlog(LOG_DEBUG, 4, "next timer has already expired, queueing signal");	
			kill(getpid(), SIGALRM);
		}
	}
}

void
set_timer(struct timer_lst *tm, double secs)
{
	struct timeval tv;
	struct timer_lst *lst;
	sigset_t bmask, oldmask;
	struct timeval firein;

	dlog(LOG_DEBUG, 3, "setting timer: %.2f secs", secs);

	firein.tv_sec = (long)secs;
	firein.tv_usec = (long)((secs - (double)firein.tv_sec) * 1000000);

	dlog(LOG_DEBUG, 5, "setting timer: %ld secs %ld usecs", firein.tv_sec, firein.tv_usec);

	gettimeofday(&tv, NULL);
	timeradd(&tv, &firein, &tm->expires);

	sigemptyset(&bmask);
	sigaddset(&bmask, SIGALRM);
	sigprocmask(SIG_BLOCK, &bmask, &oldmask);

	lst = &timers_head;

	do {
		lst = lst->next;
	} while ((tm->expires.tv_sec > lst->expires.tv_sec) ||
		 ((tm->expires.tv_sec == lst->expires.tv_sec) && 
		  (tm->expires.tv_usec > lst->expires.tv_usec)));

	tm->next = lst;
	tm->prev = lst->prev;
	lst->prev = tm;
	tm->prev->next = tm;

	dlog(LOG_DEBUG, 5, "calling schedule_timer from set_timer context");
	schedule_timer();

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void
clear_timer(struct timer_lst *tm)
{
	sigset_t bmask, oldmask;

	sigemptyset(&bmask);
	sigaddset(&bmask, SIGALRM);
	sigprocmask(SIG_BLOCK, &bmask, &oldmask);
	
	tm->prev->next = tm->next;
	tm->next->prev = tm->prev;
	
	tm->prev = tm->next = NULL;
	
	dlog(LOG_DEBUG, 5, "calling schedule_timer from clear_timer context");
	schedule_timer();

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

static void
alarm_handler(int sig)
{
	struct timer_lst *tm, *back;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	tm = timers_head.next;

	while ((tm->expires.tv_sec < tv.tv_sec)
			|| ((tm->expires.tv_sec == tv.tv_sec) 
			    && (tm->expires.tv_usec <= tv.tv_usec)))
	{		
		tm->prev->next = tm->next;
		tm->next->prev = tm->prev;

		back = tm;
		tm = tm->next;
		back->prev = back->next = NULL;

		(*back->handler)(back->data);
	}

	dlog(LOG_DEBUG, 5, "calling schedule_timer from alarm_handler context");
	schedule_timer();
}


void
init_timer(struct timer_lst *tm, void (*handler)(void *), void *data)
{
	memset(tm, 0, sizeof(struct timer_lst));
	tm->handler = handler;
	tm->data = data;
}
