
/*
 * $Id: comm_select.c,v 1.80 2006/12/28 22:11:26 hno Exp $
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

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifndef        howmany
#define howmany(x, y)   (((x)+((y)-1))/(y))
#endif
#ifndef        NBBY
#define        NBBY    8
#endif
#define FD_MASK_BYTES sizeof(fd_mask)
#define FD_MASK_BITS (FD_MASK_BYTES*NBBY)

static fd_set *global_readfds;
static fd_set *global_writefds;
static fd_set *current_readfds;
static fd_set *current_writefds;
static fd_set *current_errfds;
static int nreadfds;
static int nwritefds;

static void
do_select_init()
{
    global_readfds = xcalloc(FD_MASK_BYTES, howmany(Squid_MaxFD, FD_MASK_BITS));
    global_writefds = xcalloc(FD_MASK_BYTES, howmany(Squid_MaxFD, FD_MASK_BITS));
    current_readfds = xcalloc(FD_MASK_BYTES, howmany(Squid_MaxFD, FD_MASK_BITS));
    current_writefds = xcalloc(FD_MASK_BYTES, howmany(Squid_MaxFD, FD_MASK_BITS));
    current_errfds = xcalloc(FD_MASK_BYTES, howmany(Squid_MaxFD, FD_MASK_BITS));
    nreadfds = nwritefds = 0;
}

void
comm_select_postinit()
{
    debug(5, 1) ("Using select for the IO loop\n");
}

static void
do_select_shutdown()
{
    safe_free(global_readfds);
    safe_free(global_writefds);
    safe_free(current_readfds);
    safe_free(current_writefds);
    safe_free(current_errfds);
}

void
comm_select_status(StoreEntry * sentry)
{
    storeAppendPrintf(sentry, "\tIO loop method:                     select\n");
}

void
commSetEvents(int fd, int need_read, int need_write)
{
    if (need_read && !FD_ISSET(fd, global_readfds)) {
	FD_SET(fd, global_readfds);
	nreadfds++;
    } else if (!need_read && FD_ISSET(fd, global_readfds)) {
	FD_CLR(fd, global_readfds);
	nreadfds--;
    }
    if (need_write && !FD_ISSET(fd, global_writefds)) {
	FD_SET(fd, global_writefds);
	nwritefds++;
    } else if (!need_write && FD_ISSET(fd, global_writefds)) {
	FD_CLR(fd, global_writefds);
	nwritefds--;
    }
}

static int
do_comm_select(int msec)
{
    int num;
    struct timeval tv;
    fd_mask *rfdsp = (fd_mask *) current_readfds;
    fd_mask *wfdsp = (fd_mask *) current_writefds;
    fd_mask *efdsp = (fd_mask *) current_errfds;
    int maxindex = howmany(Biggest_FD + 1, FD_MASK_BITS);
    int fd_set_size = maxindex * FD_MASK_BYTES;
    int j;

    if (nreadfds + nwritefds == 0) {
	assert(shutting_down);
	return COMM_SHUTDOWN;
    }
    memcpy(current_readfds, global_readfds, fd_set_size);
    memcpy(current_writefds, global_writefds, fd_set_size);
    memcpy(current_errfds, global_writefds, fd_set_size);
    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec % 1000) * 1000;
    statCounter.syscalls.selects++;
    num = select(Biggest_FD + 1, current_readfds, current_writefds, current_errfds, &tv);

    if (num < 0) {
	getCurrentTime();
	if (ignoreErrno(errno))
	    return COMM_OK;

	debug(5, 1) ("comm_select: select failure: %s\n", xstrerror());
	return COMM_ERROR;
    }
    statHistCount(&statCounter.select_fds_hist, num);

    if (num == 0)
	return COMM_TIMEOUT;

    for (j = 0; j < maxindex; j++) {
	int k;
	fd_mask tmask = rfdsp[j] | wfdsp[j] | efdsp[j];
	for (k = 0; tmask && k < FD_MASK_BITS; k++) {
	    int fd;
	    int read_event, write_event;
	    if (!EBIT_TEST(tmask, k))
		continue;
	    /* Found a set bit */
	    fd = (j * FD_MASK_BITS) + k;
	    read_event = FD_ISSET(fd, current_readfds);
	    write_event = FD_ISSET(fd, current_writefds) | FD_ISSET(fd, current_errfds);
	    EBIT_CLR(tmask, k);	/* this will be done */
	    comm_call_handlers(fd, read_event, write_event);
	}
    }
    return COMM_OK;
}
