/* Timer helper functions
 *
 * Copyright (C) 2017-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "queue.h"

#include <errno.h>
#include <signal.h>
#include <string.h>		/* memset() */
#include <stdlib.h>		/* malloc() */
#include <sysexits.h>
#include <unistd.h>		/* read()/write() */
#include <time.h>

#include "log.h"
#include "socket.h"
#include "timer.h"

/*
 * TODO
 * - Timers should ideally be sorted in priority order, and/or
 * - Investigate using the pipe to notify which timer expired
 */
struct timer {
	LIST_ENTRY(timer) link;
	int             active;	/* Set to 0 to delete */

	int             period;	/* period time in seconds */
	struct timespec timeout;

	void (*cb)(void *arg);
	void *arg;
};

static timer_t timer;
static int timerfd[2];
static LIST_HEAD(tlist, timer) timer_list = LIST_HEAD_INITIALIZER();


static void set(struct timer *t, struct timespec *now)
{
	t->timeout.tv_sec  = now->tv_sec + t->period;
	t->timeout.tv_nsec = now->tv_nsec;
}

static int expired(struct timer *t, struct timespec *now)
{
	long round_nsec;

	round_nsec = now->tv_nsec + 250000000;
	round_nsec = round_nsec > 999999999 ? 999999999 : round_nsec;

	if (t->timeout.tv_sec < now->tv_sec)
		return 1;

	if (t->timeout.tv_sec == now->tv_sec && t->timeout.tv_nsec <= round_nsec)
		return 1;

	return 0;
}

static struct timer *compare(struct timer *a, struct timer *b)
{
	if (a->timeout.tv_sec <= b->timeout.tv_sec) {
		if (a->timeout.tv_sec == b->timeout.tv_sec) {
			if (a->timeout.tv_nsec <= b->timeout.tv_nsec)
				return a;

			return b;
		}

		return a;
	}

	return b;
}

static struct timer *find(void (*cb), void *arg)
{
	struct timer *entry;

	LIST_FOREACH(entry, &timer_list, link) {
		if (entry->cb != cb || entry->arg != arg)
			continue;

		return entry;
	}

	return NULL;
}


static int start(struct timespec *now)
{
	struct itimerspec it = { 0 };
	struct timer *next, *entry;

	if (LIST_EMPTY(&timer_list))
		return -1;

	next = LIST_FIRST(&timer_list);
	LIST_FOREACH(entry, &timer_list, link)
		next = compare(next, entry);

	it.it_value.tv_sec  = next->timeout.tv_sec - now->tv_sec;
	it.it_value.tv_nsec = next->timeout.tv_nsec - now->tv_nsec;
	if (it.it_value.tv_nsec < 0) {
		it.it_value.tv_sec -= 1;
		it.it_value.tv_nsec = 1000000000 + it.it_value.tv_nsec;
	}
	if (it.it_value.tv_sec < 0)
		it.it_value.tv_sec = 0;

	if (timer_settime(timer, 0, &it, NULL))
		smclog(LOG_ERR, "Failed starting %.1f sec period timer, errno %d: %s",
		       difftime(next->timeout.tv_sec, now->tv_sec), errno, strerror(errno));

	return 0;
}

/* callback for activity on pipe */
static void run(int sd, void *arg)
{
	struct timer *entry, *tmp;
	struct timespec now;
	char dummy;

	(void)arg;
	if (read(sd, &dummy, 1) < 0)
		smclog(LOG_DEBUG, "Failed read(pipe): %s", strerror(errno));

	clock_gettime(CLOCK_MONOTONIC, &now);
	LIST_FOREACH_SAFE(entry, &timer_list, link, tmp) {
		if (expired(entry, &now)) {
			if (entry->cb)
				entry->cb(entry->arg);
			set(entry, &now);
		}

		if (!entry->active) {
			LIST_REMOVE(entry, link);
			free(entry);
		}
	}

	start(&now);
}

/* write to pipe to create an event for select() on SIGALRM */
static void handler(int signo)
{
	(void)signo;
	if (write(timerfd[1], "!", 1) < 0)
		smclog(LOG_DEBUG, "Failed write(pipe): %s", strerror(errno));
}

/*
 * register signal pipe and callbacks
 */
int timer_init(void)
{
	struct sigaction sa;

	if (pipe(timerfd)) {
		smclog(LOG_ERR, "Failed creating timer pipe(): %s", strerror(errno));
		exit(EX_OSERR);
	}

	socket_register(timerfd[0], run, NULL);
	socket_register(timerfd[1], NULL, NULL);

	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	if (timer_create(CLOCK_MONOTONIC, NULL, &timer)) {
		socket_close(timerfd[0]);
		socket_close(timerfd[1]);
		return -1;
	}

	return 0;
}

/*
 * cleanup and free for program exit
 */
void timer_exit(void)
{
	struct timer *entry, *tmp;

	timer_delete(timer);

	socket_close(timerfd[0]);
	socket_close(timerfd[1]);

	LIST_FOREACH_SAFE(entry, &timer_list, link, tmp) {
		LIST_REMOVE(entry, link);
		free(entry);
	}
}

/*
 * create periodic timer (seconds)
 */
int timer_add(int period, void (*cb)(void *), void *arg)
{
	struct timespec now;
	struct timer *t;

	t = find(cb, arg);
	if (t && t->active) {
		errno = EEXIST;
		return -1;
	}

	if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
		return -1;

	t = malloc(sizeof(*t));
	if (!t) {
		smclog(LOG_ERR, "Out of memory in %s()", __func__);
		exit(EX_OSERR);
	}

	t->active = 1;
	t->period = period;
	t->cb     = cb;
	t->arg    = arg;

	set(t, &now);

	LIST_INSERT_HEAD(&timer_list, t, link);

	return start(&now);
}

/*
 * delete a timer
 */
int timer_del(void (*cb)(void *), void *arg)
{
	struct timer *entry;

	entry = find(cb, arg);
	if (!entry)
		return 1;

	/* Mark for deletion and issue a new run */
	entry->active = 0;
	handler(0);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
