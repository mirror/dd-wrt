/*
 * Copyright (C) 2003-2005 Maxina GmbH - Jordan Hrycaj
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Jordan Hrycaj <jordan@mjh.teddy.net.com>
 *
 * $Id: do.c,v 1.8 2005/04/19 19:44:10 jordan Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>

#ifdef _DEBUG
#define __KERNEL__
#include <linux/errno.h>
#endif

#include <errno.h>

#include "util.h"

#ifdef _TESTING
#include "../exit23.h"
#endif

#define __SHAT_DO_PRIVATE
#include "do.h"

/* ----------------------------------------------------------------------- *
 * Library
 * ----------------------------------------------------------------------- */

#ifdef _DEBUG
static jmp_buf go_here;           /* jump buffer for long jumps */
#endif

static
int __select_eval (int n) {
    if (n < 0) {
        logger (LOG_ERR, "%s: select() error(%d): %s",
                progname, errno, strerror (errno));
#       ifdef _DEBUG
        /* eg. profiling timer */
        if (errno == EINTR) longjmp (go_here, 77) ;
#       endif
        exit (2);
    }
    return n ;
}

/* ----------------------------------------------------------------------- *
 * Public
 * ----------------------------------------------------------------------- */

void do_destroy (DO *dp) {
    free (dp);
}

DO *do_init (void(*cb)(void*),
             void       *data) {
#   ifdef _TESTING
    X23SNOOP();
#   endif

    DO *dp = (DO*)xmalloc (sizeof (union _do)) ;
    dp [0].def.length =    1 ;
    dp [0].def.idle   =   cb ;
    dp [0].def.data   = data ;
    return dp ;
}



DO *do_action (DO           *dp,
               int           fd,
               void(*cb)(void*),
               void       *data) {
    
    if (fd >= 0 && cb != 0) {
        int top = (dp == 0 ? 1 : dp [0].def.length) ;

        dp = (DO*)xrealloc (dp, ++ top * sizeof (union _do)) ;
        dp   [0].def.length = top -- ;
        dp [top].set.rfd    = fd ;
        dp [top].set.cb     = cb ;
        dp [top].set.data   = data ;
    
        if (fd > dp [0].def.max_fd)
            dp [0].def.max_fd = fd ;
    }

    return dp ;
}


void do_resched (DO     *dp,
                 int tmo_ms) {

    if (tmo_ms == 0)
        tmo_ms = dp->def.default_timeout ;

    if (tmo_ms > 0) {
        dp->def.tv.tv_sec  =  tmo_ms / 1000;
        dp->def.tv.tv_usec = (tmo_ms % 1000) * 1000;
    }

    dp->def.timeout = tmo_ms ;
    dp->def.due     = time (0) + dp->def.tv.tv_sec ;
}


int do_loop (DO           *dp,
             int (*eval)(int),
             int       tmo_ms) {
    
    fd_set rfds, fds;
    struct timeval rtv, *_tv;
    int n ;
    
#   ifdef _TESTING
    X23CHECK();
#   endif

    /* initialize action loop */
    FD_ZERO (&fds);
    for (n = 1; n < dp->def.length; n ++) {
        FD_SET (dp [n].set.rfd, &fds);
    }

    dp->def.default_timeout = 
        dp->def.timeout = tmo_ms ;
 
    if (tmo_ms >= 0) {
        dp->def.tv.tv_sec  =  tmo_ms / 1000;
        dp->def.tv.tv_usec = (tmo_ms % 1000) * 1000;
        dp->def.due = time (0) + dp->def.tv.tv_sec ;
    }
    else
        dp->def.due = 0 ;

    if (eval == 0)
        eval = __select_eval ;

#   ifdef _DEBUG
    if (setjmp (go_here))
        logger (LOG_ERR, "%s: recovering ...", progname);
#   endif

    /* action loop */
    for (;;) {
        /* reset parameters, note that we may have been rescheduled */
        rfds = fds ;
        if (dp->def.timeout >= 0) {
            int delta = time (0) + dp->def.tv.tv_sec - dp->def.due ;
            if (delta > 0) {
                /* need a smaller timeout */
                rtv.tv_sec  = delta ;
                rtv.tv_usec = 0 ;
            }
            else
                rtv = dp->def.tv ;
            _tv = &rtv ;
        }
        else
            _tv = 0 ;

        /* wait for an event */
        if ((n = eval (select (dp->def.max_fd + 1, &rfds, 0, 0, _tv))) < 0)
            return n;

        if (n > 0) {
            /* check action list (at least one should apply) */
            for (n = 1; n < dp->def.length; n ++)
                if (FD_ISSET (dp [n].set.rfd, &rfds))
                    (*dp [n].set.cb) (dp [n].set.data) ;

            /* do not call the idle time when it is too early */
            if (time (0) < dp->def.due) continue ;
        }
        else
            /* return immediately if we are polling or on no timeout */
            if (dp->def.timeout <= 0) return 0;

        /* call idle loop call back function */
        if (dp [0].def.idle != 0)
            (*dp [0].def.idle) (dp [0].def.data) ;
        
        /* next idle time call in dp->def.tv seconds */
        dp->def.due = time (0) + dp->def.tv.tv_sec ;
        continue ;
    }
    /*NOTREACHED*/
}


/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
