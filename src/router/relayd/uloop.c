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
#include <sys/epoll.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

#include "uloop.h"

/**
 * FIXME: uClibc < 0.9.30.3 does not define EPOLLRDHUP for Linux >= 2.6.17
 */
#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

struct uloop_timeout *first_timeout;
static int epoll_fd;
static bool cancel;

int uloop_fd_add(struct uloop_fd *sock, unsigned int flags)
{
	struct epoll_event ev;
	int op = sock->registered ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
	unsigned int fl;
	int ret;

	fl = fcntl(sock->fd, F_GETFL, 0);
	fl |= O_NONBLOCK;
	fcntl(sock->fd, F_SETFL, fl);

	memset(&ev, 0, sizeof(struct epoll_event));

	if (flags & ULOOP_READ)
		ev.events |= EPOLLIN | EPOLLRDHUP;

	if (flags & ULOOP_WRITE)
		ev.events |= EPOLLOUT;

	if (flags & ULOOP_EDGE_TRIGGER)
		ev.events |= EPOLLET;

	ev.data.fd = sock->fd;
	ev.data.ptr = sock;

	ret = epoll_ctl(epoll_fd, op, sock->fd, &ev);
	if (ret < 0)
		goto out;

	sock->registered = true;
	sock->eof = false;

out:
	return ret;
}

int uloop_fd_delete(struct uloop_fd *sock)
{
	sock->registered = false;
	return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock->fd, 0);
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
	struct uloop_timeout **head = &first_timeout;
	struct uloop_timeout *prev = NULL;

	if (timeout->pending)
		return -1;

	while (*head) {
		if (tv_diff(&(*head)->time, &timeout->time) > 0)
			break;

		prev = *head;
		head = &(*head)->next;
	}

	timeout->prev = prev;
	timeout->next = *head;
	if (timeout->next)
		timeout->next->prev = timeout;
	*head = timeout;
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

	if (timeout->prev)
		timeout->prev->next = timeout->next;
	else
		first_timeout = timeout->next;

	if (timeout->next)
		timeout->next->prev = timeout->prev;

	timeout->pending = false;

	return 0;
}

static void uloop_handle_sigint(int signo)
{
	cancel = true;
}

static void uloop_setup_signals(void)
{
	struct sigaction s;
	memset(&s, 0, sizeof(struct sigaction));
	s.sa_handler = uloop_handle_sigint;
	s.sa_flags = 0;
	sigaction(SIGINT, &s, NULL);
}

static int uloop_get_next_timeout(struct timeval *tv)
{
	int diff;

	if (!first_timeout)
		return -1;

	diff = tv_diff(&first_timeout->time, tv);
	if (diff < 0)
		return 0;

	return diff;
}

static void uloop_process_timeouts(struct timeval *tv)
{
	struct uloop_timeout *timeout;

	while (first_timeout) {
		if (tv_diff(&first_timeout->time, tv) > 0)
			break;

		timeout = first_timeout;
		uloop_timeout_cancel(timeout);
		if (timeout->cb)
			timeout->cb(timeout);
	}
}

void uloop_end(void)
{
	cancel = true;
}

int uloop_init(void)
{
	epoll_fd = epoll_create(32);
	if (epoll_fd < 0)
		return -1;

	fcntl(epoll_fd, F_SETFD, fcntl(epoll_fd, F_GETFD) | FD_CLOEXEC);
	return 0;
}

void uloop_run(void)
{
	struct epoll_event events[10];
	struct timeval tv;
	int timeout;
	int nfds, n;

	uloop_setup_signals();
	while(!cancel)
	{
		gettimeofday(&tv, NULL);
		uloop_process_timeouts(&tv);
		timeout = uloop_get_next_timeout(&tv);
		nfds = epoll_wait(epoll_fd, events, ARRAY_SIZE(events), timeout);
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
}

void uloop_done(void)
{
	close(epoll_fd);
}
