/*
 * Copyright (c) 1998, 2008, 2009, 2010, 2011, 2012, 2013
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

#include "common.h"

static const char rcsid[] =
"$Id: Rlisten.c,v 1.36 2013/10/27 15:24:42 karls Exp $";

int
Rlisten(s, backlog)
   int s;
   int backlog;
{
   const char *function = "Rlisten()";
   socksfd_t socksfd;
   int rc;

   clientinit();

   slog(LOG_DEBUG, "%s, fd %d, backlog %d", function, s, backlog);

   if (!socks_addrisours(s, &socksfd, 1))
      rc = listen(s, backlog);
   else if (socksfd.state.command != SOCKS_BIND) {
      swarnx("%s: doing listen on socket, but command state is %d",
             function, socksfd.state.command);

      socks_rmaddr(s, 1);

      rc = listen(s, backlog);
   }
   else if (socksfd.state.acceptpending)
      rc = listen(s, backlog);
   else {
      /*
       * If it's using the bind extension, we do a standard listen(2), if
       * not, we need to drop the listen(2), as doing listen(2) on a socket
       * we have previously done connect(2) on (for connect(2) to the socks
       * server) does not necessarily work so well on all (any?) systems.
       */

      slog(LOG_DEBUG, "%s: no system listen(2) to do on fd %d", function, s);
      return 0;
   }

   slog(LOG_DEBUG, "%s: listen(2) on fd %d returned %d", function, s, rc);
   return rc;
}
