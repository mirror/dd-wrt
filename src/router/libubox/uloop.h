/*
 * uloop - event loop implementation
 *
 * Copyright (C) 2010-2013 Felix Fietkau <nbd@openwrt.org>
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
#ifndef _ULOOP_H__
#define _ULOOP_H__

#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

#if defined(__APPLE__) || defined(__FreeBSD__)
#define USE_KQUEUE
#else
#define USE_EPOLL
#endif

#include "list.h"

struct uloop_fd;
struct uloop_timeout;
struct uloop_process;
struct uloop_interval;
struct uloop_signal;

typedef void (*uloop_fd_handler)(struct uloop_fd *u, unsigned int events);
typedef void (*uloop_timeout_handler)(struct uloop_timeout *t);
typedef void (*uloop_process_handler)(struct uloop_process *c, int ret);
typedef void (*uloop_interval_handler)(struct uloop_interval *t);
typedef void (*uloop_signal_handler)(struct uloop_signal *s);

#define ULOOP_READ		(1 << 0)
#define ULOOP_WRITE		(1 << 1)
#define ULOOP_EDGE_TRIGGER	(1 << 2)
#define ULOOP_BLOCKING		(1 << 3)

#define ULOOP_EVENT_MASK	(ULOOP_READ | ULOOP_WRITE)

/* internal flags */
#define ULOOP_EVENT_BUFFERED	(1 << 4)
#ifdef USE_KQUEUE
#define ULOOP_EDGE_DEFER	(1 << 5)
#endif

#define ULOOP_ERROR_CB		(1 << 6)

struct uloop_fd
{
	uloop_fd_handler cb;
	int fd;
	bool eof;
	bool error;
	bool registered;
	uint8_t flags;
};

struct uloop_timeout
{
	struct list_head list;
	bool pending;

	uloop_timeout_handler cb;
	struct timeval time;
};

struct uloop_process
{
	struct list_head list;
	bool pending;

	uloop_process_handler cb;
	pid_t pid;
};

struct uloop_interval
{
	uloop_interval_handler cb;
	uint64_t expirations;

	union {
		struct uloop_fd ufd;
		struct {
			int64_t fired;
			unsigned int msecs;
		} time;
	} priv;
};

struct uloop_signal
{
	struct list_head list;
	struct sigaction orig;
	bool pending;

	uloop_signal_handler cb;
	int signo;
};

extern bool uloop_cancelled;
extern bool uloop_handle_sigchld;
extern uloop_fd_handler uloop_fd_set_cb;

int uloop_fd_add(struct uloop_fd *sock, unsigned int flags);
int uloop_fd_delete(struct uloop_fd *sock);

int uloop_get_next_timeout(void);
int uloop_timeout_add(struct uloop_timeout *timeout);
int uloop_timeout_set(struct uloop_timeout *timeout, int msecs);
int uloop_timeout_cancel(struct uloop_timeout *timeout);
int uloop_timeout_remaining(struct uloop_timeout *timeout) __attribute__((deprecated("use uloop_timeout_remaining64")));
int64_t uloop_timeout_remaining64(struct uloop_timeout *timeout);

int uloop_process_add(struct uloop_process *p);
int uloop_process_delete(struct uloop_process *p);

int uloop_interval_set(struct uloop_interval *timer, unsigned int msecs);
int uloop_interval_cancel(struct uloop_interval *timer);
int64_t uloop_interval_remaining(struct uloop_interval *timer);

int uloop_signal_add(struct uloop_signal *s);
int uloop_signal_delete(struct uloop_signal *s);

bool uloop_cancelling(void);

static inline void uloop_end(void)
{
	uloop_cancelled = true;
}

int uloop_init(void);
int uloop_run_timeout(int timeout);
static inline int uloop_run(void)
{
	return uloop_run_timeout(-1);
}
void uloop_done(void);

#endif
