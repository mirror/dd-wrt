#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif /* HAVE_CONFIG_H */

#include "osdep.h"

/*
 * $Id: pselect.c,v 1.12 2013/01/31 23:01:47 karls Exp $
 *
 * Copyright (c) 2011, 2012
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

static const char rcsid[] =
"$Id: pselect.c,v 1.12 2013/01/31 23:01:47 karls Exp $";

int sockd_handledsignals(void); /* no sockd.h here. */


/* inspired by Stevens 'incorrect' implementation; best effort if no pselect */

inline int
pselect(int nfds, fd_set *rset, fd_set *wset, fd_set *xset,
    const struct timespec *ts, const sigset_t *sigmask)
{
    struct timeval tv;
    sigset_t sm;
    int n;

    if (sigprocmask(SIG_SETMASK, sigmask, &sm) == -1)
       return -1;

    if (ts != NULL) {
        tv.tv_sec  = ts->tv_sec;
        tv.tv_usec = ts->tv_nsec / 1000;
    }

#if SOCKS_SERVER || BAREFOOTD || COVENANT
    if (sockd_handledsignals() != 0) {
      if (sigmask != NULL)
         (void)sigprocmask(SIG_SETMASK, &sm, NULL);

      errno = EINTR;
      return -1;
   }
#endif /* SOCKS_SERVER || BAREFOOTD || COVENANT */


    n = select(nfds, rset, wset, xset, (ts == NULL) ? NULL : &tv);

    if (sigmask != NULL && sigprocmask(SIG_SETMASK, &sm, NULL) == -1)
       return -1;

    return n;
}
