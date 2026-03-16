/*
 * Copyright (c) 2001, 2008, 2009, 2010, 2011, 2012
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
"$Id: redirect.c,v 1.18 2013/03/06 07:50:37 michaels Exp $";

const char module_redirect_version[] =
"$Id: redirect.c,v 1.18 2013/03/06 07:50:37 michaels Exp $";


void
redirectsetup(void)
{

}

int
redirect(s, addr,
#if !BAREFOOTD
         host,
#endif /* !BAREFOOTD */
         command, from
#if !BAREFOOTD
         , to
#endif /* !BAREFOOTD */
        )
   int s;
   struct sockaddr_storage *addr;
#if !BAREFOOTD
   struct sockshost_t *host;
#endif /* !BAREFOOTD */
   int command;
   const struct ruleaddr_t *from;
#if !BAREFOOTD
   const struct ruleaddr_t *to;
#endif /* !BAREFOOTD */
{

   (void)s;
   (void)addr;
#if !BAREFOOTD
   (void)host;
#endif /* !BAREFOOTD */
   (void)command;
   (void)from;
#if !BAREFOOTD
   (void)to;
#endif /* !BAREFOOTD */

   return 0;
}
