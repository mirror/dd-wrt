/*
 *   Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
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

#ifndef _ULOOP_H__
#define _ULOOP_H__

#include <sys/time.h>
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

typedef void (*uloop_fd_handler)(struct uloop_fd *u, unsigned int events);
typedef void (*uloop_timeout_handler)(struct uloop_timeout *t);
typedef void (*uloop_process_handler)(struct uloop_process *c, int ret);

#define ULOOP_READ		(1 << 0)
#define ULOOP_WRITE		(1 << 1)
#define ULOOP_EDGE_TRIGGER	(1 << 2)
#define ULOOP_BLOCKING		(1 << 3)

struct uloop_fd
{
	uloop_fd_handler cb;
	int fd;
	bool eof;
	bool error;
	bool registered;
#ifdef USE_KQUEUE
	uint8_t kqflags;
#endif
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

extern bool uloop_cancelled;
extern bool uloop_handle_sigchld;

int uloop_fd_add(struct uloop_fd *sock, unsigned int flags);
int uloop_fd_delete(struct uloop_fd *sock);

int uloop_timeout_add(struct uloop_timeout *timeout);
int uloop_timeout_set(struct uloop_timeout *timeout, int msecs);
int uloop_timeout_cancel(struct uloop_timeout *timeout);

int uloop_process_add(struct uloop_process *p);
int uloop_process_delete(struct uloop_process *p);

static inline void uloop_end(void)
{
	uloop_cancelled = true;
}

int uloop_init(void);
void uloop_run(void);
void uloop_done(void);

#endif
