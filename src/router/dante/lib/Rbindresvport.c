/*
 * Copyright (c) 1997, 1998, 1999, 2001, 2008, 2009, 2012, 2013
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
"$Id: Rbindresvport.c,v 1.47 2013/10/27 15:24:42 karls Exp $";

/*
 * Note that for this function to work correctly the remote socks server
 * would have to be using the bind extension.
 */

int
Rbindresvport(s, _sin)
   int s;
   struct sockaddr_in *_sin;
{
   const char *function = "Rbindresvport()";
   struct sockaddr_storage sin;
   socklen_t sinlen;
   int rc;

   clientinit();

   slog(LOG_DEBUG, "%s, fd %d", function, s);

   /*
    * Nothing can be called before Rbindresvport(), delete any old cruft.
    */
   socks_rmaddr(s, 1);

   if (_sin == NULL) {
      slog(LOG_DEBUG, "%s: fd %d, _sin = %p", function, s, _sin);
      return bindresvport(s, _sin);
   }

   usrsockaddrcpy(&sin, TOSS(_sin), sizeof(*_sin));
   if (bindresvport(s, TOIN(&sin)) != 0) {
      slog(LOG_DEBUG, "%s: bindresvport(%d, %s) failed: %s",
           function,
           s,
           sockaddr2string(&sin, NULL, 0),
           strerror(errno));

      return -1;
   }


   sinlen = salen(sin.ss_family);
   if (getsockname(s, TOSA(&sin), &sinlen) != 0)
      return -1;

   /*
    * Rbind() will accept failure at binding socket that is already bound
    * (assuming it has been bound already in some way) and will continue to
    * try a remote server binding too if appropriate.
    */
   if ((rc = Rbind(s, TOSA(&sin), sinlen)) == -1)
      return -1;

   sockaddrcpy(TOSS(_sin), &sin, salen(sin.ss_family));

   return rc;
}
