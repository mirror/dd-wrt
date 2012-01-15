/*
 *   Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 *   Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2010 Steven Barth <steven@midlink.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <sys/time.h>
#include <sys/types.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include "uloop.h"

#ifdef USE_KQUEUE
#include <sys/event.h>
#endif
#ifdef USE_EPOLL
#include <sys/epoll.h>
#endif
#include <sys/wait.h>


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif
#define ULOOP_MAX_EVENTS 10

static struct list_head timeouts = LIST_HEAD_INIT(timeouts);
static struct list_head processes = LIST_HEAD_INIT(processes);

static int poll_fd = -1;
bool uloop_cancelled = false;
bool uloop_handle_sigchld = true;
static bool do_sigchld = false;

#ifdef USE_KQUEUE

int uloop_init(void)
{
	if (poll_fd >= 0)
		return 0;

	poll_fd = kqueue();
	if (poll_fd < 0)
		return -1;

	return 0;
}


static uint16_t get_flags(unsigned int flags, unsigned int mask)
{
	uint16_t kflags = 0;

	if (!(flags & mask))
		return EV_DELETE;

	kflags = EV_ADD;
	if (flags & ULOOP_EDGE_TRIGGER)
		kflags |= EV_CLEAR;

	return kflags;
}

static int register_poll(struct uloop_fd *fd, unsigned int flags)
{
	struct timespec timeout = { 0, 0 };
	struct kevent ev[2];
	unsigned int changed;
	int nev = 0;

	changed = fd->kqflags ^ flags;
	if (changed & ULOOP_EDGE_TRIGGER)
		changed |= flags;

	if (changed & ULOOP_READ) {
		uint16_t kflags = get_flags(flags, ULOOP_READ);
		EV_SET(&ev[nev++], fd->fd, EVFILT_READ, kflags, 0, 0, fd);
	}

	if (changed & ULOOP_WRITE) {
		uint16_t kflags = get_flags(flags, ULOOP_WRITE);
		EV_SET(&ev[nev++], fd->fd, EVFILT_WRITE, kflags, 0, 0, fd);
	}

	if (nev && (kevent(poll_fd, ev, nev, NULL, 0, &timeout) == -1))
		return -1;

	fd->kqflags = flags;
	return 0;
}

int uloop_fd_delete(struct uloop_fd *sock)
{
	sock->registered = false;
	return register_poll(sock, 0);
}

static void uloop_run_events(int timeout)
{
	struct kevent events[ULOOP_MAX_EVENTS];
	struct timespec ts;
	int nfds, n;

	if (timeout > 0) {
		ts.tv_sec = timeout / 1000;
		ts.tv_nsec = (timeout % 1000) * 1000000;
	}

	nfds = kevent(poll_fd, NULL, 0, events, ARRAY_SIZE(events), timeout > 0 ? &ts : NULL);
	for(n = 0; n < nfds; ++n)
	{
		struct uloop_fd *u = events[n].udata;
		unsigned int ev = 0;

		if(events[n].flags & EV_ERROR) {
			u->error = true;
			uloop_fd_delete(u);
		}

		if(events[n].filter == EVFILT_READ)
			ev |= ULOOP_READ;
		else if (events[n].filter == EVFILT_WRITE)
			ev |= ULOOP_WRITE;

		if(events[n].flags & EV_EOF)
			u->eof = true;
		else if (!ev)
			continue;

		if(u->cb)
			u->cb(u, ev);
	}
}

#endif

#ifdef USE_EPOLL

/**
 * FIXME: uClibc < 0.9.30.3 does not define EPOLLRDHUP for Linux >= 2.6.17
 */
#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif

int uloop_init(void)
{
	if (poll_fd >= 0)
		return 0;

	poll_fd = epoll_create(32);
	if (poll_fd < 0)
		return -1;

	fcntl(poll_fd, F_SETFD, fcntl(poll_fd, F_GETFD) | FD_CLOEXEC);
	return 0;
}

static int register_poll(struct uloop_fd *fd, unsigned int flags)
{
	struct epoll_event ev;
	int op = fd->registered ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

	memset(&ev, 0, sizeof(struct epoll_event));

	if (flags & ULOOP_READ)
		ev.events |= EPOLLIN | EPOLLRDHUP;

	if (flags & ULOOP_WRITE)
		ev.events |= EPOLLOUT;

	if (flags & ULOOP_EDGE_TRIGGER)
		ev.events |= EPOLLET;

	ev.data.fd = fd->fd;
	ev.data.ptr = fd;

	return epoll_ctl(poll_fd, op, fd->fd, &ev);
}

int uloop_fd_delete(struct uloop_fd *sock)
{
	sock->registered = false;
	return epoll_ctl(poll_fd, EPOLL_CTL_DEL, sock->fd, 0);
}

static void uloop_run_events(int timeout)
{
	struct epoll_event events[ULOOP_MAX_EVENTS];
	int nfds, n;

	nfds = epoll_wait(poll_fd, events, ARRAY_SIZE(events), timeout);
	for(n = 0; n < nfds; ++n)
	{
		struct uloop_fd *u = events[n].data.ptr;
		unsigned int ev = 0;

		if(events[n].events & EPOLLERR) {
			u->error = true;
			uloop_fd_delete(u);
		}

		if(!(events[n].events & (EPOLLRDHUP|EPOLLIN|EPOLLOUT|EPOLLERR)))
			continue;

		if(events[n].events & EPOLLRDHUP)
			u->eof = true;

		if(events[n].events & EPOLLIN)
			ev |= ULOOP_READ;

		if(events[n].events & EPOLLOUT)
			ev |= ULOOP_WRITE;

		if(u->cb)
			u->cb(u, ev);
	}
}

#endif

int uloop_fd_add(struct uloop_fd *sock, unsigned int flags)
{
	unsigned int fl;
	int ret;

	if (!sock->registered && !(flags & ULOOP_BLOCKING)) {
		fl = fcntl(sock->fd, F_GETFL, 0);
		fl |= O_NONBLOCK;
		fcntl(sock->fd, F_SETFL, fl);
	}

	ret = register_poll(sock, flags);
	if (ret < 0)
		goto out;

	sock->registered = true;
	sock->eof = false;

out:
	return ret;
}

static int tv_diff(struct timeval *t1, struct timeval *t2)
{
	if (t1->tv_sec != t2->tv_sec)
		return (t1->tv_sec - t2->tv_sec) * 1000;
	else
		return (t1->tv_usec - t2->tv_usec) / 1000;
}

int uloop_timeout_add(struct uloop_timeout *timeout)
{
	struct uloop_timeout *tmp;
	struct list_head *h = &timeouts;

	if (timeout->pending)
		return -1;

	list_for_each_entry(tmp, &timeouts, list) {
		if (tv_diff(&tmp->time, &timeout->time) > 0) {
			h = &tmp->list;
			break;
		}
	}

	list_add_tail(&timeout->list, h);
	timeout->pending = true;

	return 0;
}

int uloop_timeout_set(struct uloop_timeout *timeout, int msecs)
{
	struct timeval *time = &timeout->time;

	if (timeout->pending)
		uloop_timeout_cancel(timeout);

	gettimeofday(&timeout->time, NULL);

	time->tv_sec += msecs / 1000;
	time->tv_usec += msecs % 1000;

	if (time->tv_usec > 1000000) {
		time->tv_sec++;
		time->tv_usec %= 100000;
	}

	return uloop_timeout_add(timeout);
}

int uloop_timeout_cancel(struct uloop_timeout *timeout)
{
	if (!timeout->pending)
		return -1;

	list_del(&timeout->list);
	timeout->pending = false;

	return 0;
}

int uloop_process_add(struct uloop_process *p)
{
	struct uloop_process *tmp;
	struct list_head *h = &processes;

	if (p->pending)
		return -1;

	list_for_each_entry(tmp, &processes, list) {
		if (tmp->pid > p->pid) {
			h = &tmp->list;
			break;
		}
	}

	list_add_tail(&p->list, h);
	p->pending = true;

	return 0;
}

int uloop_process_delete(struct uloop_process *p)
{
	if (!p->pending)
		return -1;

	list_del(&p->list);
	p->pending = false;

	return 0;
}

static void uloop_handle_processes(void)
{
	struct uloop_process *p, *tmp;
	pid_t pid;
	int ret;

	do_sigchld = false;

	while (1) {
		pid = waitpid(-1, &ret, WNOHANG);
		if (pid <= 0)
			return;

		list_for_each_entry_safe(p, tmp, &processes, list) {
			if (p->pid < pid)
				continue;

			if (p->pid > pid)
				break;

			uloop_process_delete(p);
			p->cb(p, ret);
		}
	}

}

static void uloop_handle_sigint(int signo)
{
	uloop_cancelled = true;
}

static void uloop_sigchld(int signo)
{
	do_sigchld = true;
}

static void uloop_setup_signals(void)
{
	struct sigaction s;

	memset(&s, 0, sizeof(struct sigaction));
	s.sa_handler = uloop_handle_sigint;
	s.sa_flags = 0;
	sigaction(SIGINT, &s, NULL);

	if (uloop_handle_sigchld) {
		s.sa_handler = uloop_sigchld;
		sigaction(SIGCHLD, &s, NULL);
	}
}

static int uloop_get_next_timeout(struct timeval *tv)
{
	struct uloop_timeout *timeout;
	int diff;

	if (list_empty(&timeouts))
		return -1;

	timeout = list_first_entry(&timeouts, struct uloop_timeout, list);
	diff = tv_diff(&timeout->time, tv);
	if (diff < 0)
		return 0;

	return diff;
}

static void uloop_process_timeouts(struct timeval *tv)
{
	struct uloop_timeout *t, *tmp;

	list_for_each_entry_safe(t, tmp, &timeouts, list) {
		if (tv_diff(&t->time, tv) > 0)
			break;

		uloop_timeout_cancel(t);
		if (t->cb)
			t->cb(t);
	}
}

void uloop_run(void)
{
	struct timeval tv;

	uloop_setup_signals();
	while(!uloop_cancelled)
	{
		gettimeofday(&tv, NULL);
		uloop_process_timeouts(&tv);
		if (uloop_cancelled)
			break;

		if (do_sigchld)
			uloop_handle_processes();
		uloop_run_events(uloop_get_next_timeout(&tv));
	}
}

void uloop_done(void)
{
	if (poll_fd < 0)
		return;

	close(poll_fd);
	poll_fd = -1;
}
