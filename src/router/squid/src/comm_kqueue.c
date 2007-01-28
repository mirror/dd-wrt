
/*
 * $Id: comm_kqueue.c,v 1.10 2006/10/23 11:22:21 hno Exp $
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
#include "comm_generic.c"

#if HAVE_SYS_EVENT_H
#include <sys/event.h>
#endif

#define KE_LENGTH       128
#define STATE_READ      1
#define STATE_WRITE     2

static int kq;
static struct timespec zero_timespec;
static struct kevent *kqlst;	/* kevent buffer */
static int kqmax;		/* max structs to buffer */
static int kqoff;		/* offset into the buffer */
static unsigned *kqueue_state;	/* keep track of the kqueue state */

static void
do_select_init()
{
    kq = kqueue();
    if (kq < 0)
	fatalf("comm_select_init: kqueue(): %s\n", xstrerror());
    fd_open(kq, FD_UNKNOWN, "kqueue ctl");
    commSetCloseOnExec(kq);
    kqmax = getdtablesize();
    kqlst = xmalloc(sizeof(*kqlst) * kqmax);
    kqueue_state = xcalloc(Squid_MaxFD, sizeof(*kqueue_state));
    zero_timespec.tv_sec = 0;
    zero_timespec.tv_nsec = 0;
}

void
comm_select_postinit()
{
    debug(5, 1) ("Using kqueue for the IO loop\n");
}

static void
do_select_shutdown()
{
    fd_close(kq);
    close(kq);
    kq = -1;
    safe_free(kqueue_state);
}

void
comm_select_status(StoreEntry * sentry)
{
    storeAppendPrintf(sentry, "\tIO loop method:                     kqueue\n");
}

void
commSetEvents(int fd, int need_read, int need_write)
{
    struct kevent *kep;
    int st_read = (kqueue_state[fd] & STATE_READ) != 0;
    int st_write = (kqueue_state[fd] & STATE_WRITE) != 0;

    assert(fd >= 0);
    debug(5, 8) ("commSetEvents(fd=%d)\n", fd);

    kep = kqlst + kqoff;

    if (need_read != st_read) {
	EV_SET(kep, (uintptr_t) fd, EVFILT_READ,
	    need_read ? EV_ADD : EV_DELETE, 0, 0, 0);
	kqoff++;
	kep++;
    }
    if (need_write != st_write) {
	EV_SET(kep, (uintptr_t) fd, EVFILT_WRITE,
	    need_write ? EV_ADD : EV_DELETE, 0, 0, 0);
	kqoff++;
	kep++;
    }
    kqueue_state[fd] = (need_read ? STATE_READ : 0) |
	(need_write ? STATE_WRITE : 0);

    if (kqoff >= kqmax - 2) {
	if (kevent(kq, kqlst, kqoff, NULL, 0, &zero_timespec) == -1) {
	    debug(5, 1) ("commSetEvents(): kevent() failed on fd=%d: %s\n",
		fd, xstrerror());
	    return;
	}
	kqoff = 0;
    }
}

static int
do_comm_select(int msec)
{
    int i;
    int num;
    static struct kevent ke[KE_LENGTH];
    struct timespec timeout;

    timeout.tv_sec = msec / 1000;
    timeout.tv_nsec = (msec % 1000) * 1000000;

    statCounter.syscalls.polls++;
    num = kevent(kq, kqlst, kqoff, ke, KE_LENGTH, &timeout);
    kqoff = 0;
    if (num < 0) {
	getCurrentTime();
	if (ignoreErrno(errno))
	    return COMM_OK;

	debug(5, 1) ("comm_select: kevent failure: %s\n", xstrerror());
	return COMM_ERROR;
    }
    statHistCount(&statCounter.select_fds_hist, num);
    if (num == 0)
	return COMM_TIMEOUT;

    for (i = 0; i < num; i++) {
	int fd = (int) ke[i].ident;
	if (ke[i].flags & EV_ERROR) {
	    errno = ke[i].data;
	    debug(5, 3) ("comm_select: kqueue event error: %s\n",
		xstrerror());
	    continue;		/* XXX! */
	}
	switch (ke[i].filter) {
	case EVFILT_READ:
	    comm_call_handlers(fd, 1, 0);
	    break;
	case EVFILT_WRITE:
	    comm_call_handlers(fd, 0, 1);
	    break;
	default:
	    debug(5, 1) ("comm_select: unexpected event: %d\n",
		ke[i].filter);
	    break;
	}
    }

    return COMM_OK;
}
