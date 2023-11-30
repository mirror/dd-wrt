/*
 * uloop - event loop implementation
 *
 * Copyright (C) 2010-2016 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * FIXME: uClibc < 0.9.30.3 does not define EPOLLRDHUP for Linux >= 2.6.17
 */
#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif

static int uloop_init_pollfd(void)
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

	ev.data.ptr = fd;

	return epoll_ctl(poll_fd, op, fd->fd, &ev);
}

static struct epoll_event events[ULOOP_MAX_EVENTS];

static int __uloop_fd_delete(struct uloop_fd *sock)
{
	sock->flags = 0;
	return epoll_ctl(poll_fd, EPOLL_CTL_DEL, sock->fd, 0);
}

static int uloop_fetch_events(int timeout)
{
	int n, nfds;

	nfds = epoll_wait(poll_fd, events, ARRAY_SIZE(events), timeout);
	for (n = 0; n < nfds; ++n) {
		struct uloop_fd_event *cur = &cur_fds[n];
		struct uloop_fd *u = events[n].data.ptr;
		unsigned int ev = 0;

		cur->fd = u;
		if (!u)
			continue;

		if (events[n].events & (EPOLLERR|EPOLLHUP)) {
			u->error = true;
			if (!(u->flags & ULOOP_ERROR_CB))
				uloop_fd_delete(u);
		}

		if(!(events[n].events & (EPOLLRDHUP|EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP))) {
			cur->fd = NULL;
			continue;
		}

		if(events[n].events & EPOLLRDHUP)
			u->eof = true;

		if(events[n].events & EPOLLIN)
			ev |= ULOOP_READ;

		if(events[n].events & EPOLLOUT)
			ev |= ULOOP_WRITE;

		cur->events = ev;
	}

	return nfds;
}

static void dispatch_timer(struct uloop_fd *u, unsigned int events)
{
	if (!(events & ULOOP_READ))
		return;

	uint64_t fired;

	if (read(u->fd, &fired, sizeof(fired)) != sizeof(fired))
		return;

	struct uloop_interval *tm = container_of(u, struct uloop_interval, priv.ufd);

	tm->expirations += fired;
	tm->cb(tm);
}

static int timer_register(struct uloop_interval *tm, unsigned int msecs)
{
	if (!tm->priv.ufd.registered) {
		int fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK);

		if (fd == -1)
			return -1;

		tm->priv.ufd.fd = fd;
		tm->priv.ufd.cb = dispatch_timer;
	}

	struct itimerspec spec = {
		.it_value = {
			.tv_sec = msecs / 1000,
			.tv_nsec = (msecs % 1000) * 1000000
		},
		.it_interval = {
			.tv_sec = msecs / 1000,
			.tv_nsec = (msecs % 1000) * 1000000
		}
	};

	if (timerfd_settime(tm->priv.ufd.fd, 0, &spec, NULL) == -1)
		goto err;

	if (uloop_fd_add(&tm->priv.ufd, ULOOP_READ) == -1)
		goto err;

	return 0;

err:
	uloop_fd_delete(&tm->priv.ufd);
	close(tm->priv.ufd.fd);
	memset(&tm->priv.ufd, 0, sizeof(tm->priv.ufd));

	return -1;
}

static int timer_remove(struct uloop_interval *tm)
{
	int ret = __uloop_fd_delete(&tm->priv.ufd);

	if (ret == 0) {
		close(tm->priv.ufd.fd);
		memset(&tm->priv.ufd, 0, sizeof(tm->priv.ufd));
	}

	return ret;
}

static int64_t timer_next(struct uloop_interval *tm)
{
	struct itimerspec spec;

	if (!tm->priv.ufd.registered)
		return -1;

	if (timerfd_gettime(tm->priv.ufd.fd, &spec) == -1)
		return -1;

	return spec.it_value.tv_sec * 1000 + spec.it_value.tv_nsec / 1000000;
}
