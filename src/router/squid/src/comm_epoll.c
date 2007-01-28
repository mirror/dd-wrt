
/*
 * $Id: comm_epoll.c,v 1.27 2006/10/23 11:22:21 hno Exp $
 *
 * DEBUG: section 5     Socket Functions
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#include "squid.h"

#include <sys/epoll.h>

#define MAX_EVENTS	256	/* max events to process in one go */

/* epoll structs */
static int kdpfd;
static struct epoll_event events[MAX_EVENTS];
static int epoll_fds = 0;
static unsigned *epoll_state;	/* keep track of the epoll state */

#include "comm_generic.c"

static const char *
epolltype_atoi(int x)
{
    switch (x) {

    case EPOLL_CTL_ADD:
	return "EPOLL_CTL_ADD";

    case EPOLL_CTL_DEL:
	return "EPOLL_CTL_DEL";

    case EPOLL_CTL_MOD:
	return "EPOLL_CTL_MOD";

    default:
	return "UNKNOWN_EPOLLCTL_OP";
    }
}

static void
do_select_init()
{
    kdpfd = epoll_create(Squid_MaxFD);
    if (kdpfd < 0)
	fatalf("comm_select_init: epoll_create(): %s\n", xstrerror());
    fd_open(kdpfd, FD_UNKNOWN, "epoll ctl");
    commSetCloseOnExec(kdpfd);

    epoll_state = xcalloc(Squid_MaxFD, sizeof(*epoll_state));
}

void
comm_select_postinit()
{
    debug(5, 1) ("Using epoll for the IO loop\n");
}

static void
do_select_shutdown()
{
    fd_close(kdpfd);
    close(kdpfd);
    kdpfd = -1;
    safe_free(epoll_state);
}

void
comm_select_status(StoreEntry * sentry)
{
    storeAppendPrintf(sentry, "\tIO loop method:                     epoll\n");
}

void
commSetEvents(int fd, int need_read, int need_write)
{
    int epoll_ctl_type = 0;
    struct epoll_event ev;

    assert(fd >= 0);
    debug(5, 8) ("commSetEvents(fd=%d)\n", fd);

    if (RUNNING_ON_VALGRIND) {
	/* Keep valgrind happy.. complains about uninitialized bytes otherwise */
	memset(&ev, 0, sizeof(ev));
    }
    ev.events = 0;
    ev.data.fd = fd;

    if (need_read)
	ev.events |= EPOLLIN;

    if (need_write)
	ev.events |= EPOLLOUT;

    if (ev.events)
	ev.events |= EPOLLHUP | EPOLLERR;

    if (ev.events != epoll_state[fd]) {
	/* If the struct is already in epoll MOD or DEL, else ADD */
	if (!ev.events) {
	    epoll_ctl_type = EPOLL_CTL_DEL;
	} else if (epoll_state[fd]) {
	    epoll_ctl_type = EPOLL_CTL_MOD;
	} else {
	    epoll_ctl_type = EPOLL_CTL_ADD;
	}

	/* Update the state */
	epoll_state[fd] = ev.events;

	if (epoll_ctl(kdpfd, epoll_ctl_type, fd, &ev) < 0) {
	    debug(5, 1) ("commSetEvents: epoll_ctl(%s): failed on fd=%d: %s\n",
		epolltype_atoi(epoll_ctl_type), fd, xstrerror());
	}
	switch (epoll_ctl_type) {
	case EPOLL_CTL_ADD:
	    epoll_fds++;
	    break;
	case EPOLL_CTL_DEL:
	    epoll_fds--;
	    break;
	default:
	    break;
	}
    }
}

static int
do_comm_select(int msec)
{
    int i;
    int num;
    int fd;
    struct epoll_event *cevents;

    if (epoll_fds == 0) {
	assert(shutting_down);
	return COMM_SHUTDOWN;
    }
    statCounter.syscalls.polls++;
    num = epoll_wait(kdpfd, events, MAX_EVENTS, msec);
    if (num < 0) {
	getCurrentTime();
	if (ignoreErrno(errno))
	    return COMM_OK;

	debug(5, 1) ("comm_select: epoll failure: %s\n", xstrerror());
	return COMM_ERROR;
    }
    statHistCount(&statCounter.select_fds_hist, num);

    if (num == 0)
	return COMM_TIMEOUT;

    for (i = 0, cevents = events; i < num; i++, cevents++) {
	fd = cevents->data.fd;
	comm_call_handlers(fd, cevents->events & ~EPOLLOUT, cevents->events & ~EPOLLIN);
    }

    return COMM_OK;
}
