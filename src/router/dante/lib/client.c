/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2005, 2008, 2009, 2010,
 *               2011, 2012
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
"$Id: client.c,v 1.108 2013/01/02 13:22:39 karls Exp $";


#if HAVE_DARWIN || !HAVE_PROGNAME
   char *__progname = "danteclient";
#endif /* HAVE_DARWIN || !HAVE_PROGNAME */

int
SOCKSinit(progname)
   char *progname;
{

   __progname = progname;
   return 0;
}

void
clientinit(void)
{
#ifdef HAVE_VOLATILE_SIG_ATOMIC_T
   static sig_atomic_t initing; /* XXX should be our threadid. */
#else
   static volatile sig_atomic_t initing;
#endif /* HAVE_VOLATILE_SIG_ATOMIC_T */
/*   const char *function = "clientinit()"; */

   if (sockscf.state.inited
      /*
       * XXX should really be sched_yield() or something if initing, unless
       * the thread initing is ours.  If the thread initing is ours,
       * we can just return, to handle recursive problems during init.
       */
   ||  initing)
      return;

   initing = 1;

   sockscf.loglock = -1;

#if HAVE_DARWIN
   __progname = getprogname();
#endif /* HAVE_DARWIN */

/*   sleep(20);    */

   /* needs to be as early as possible, before any i/o calls if possible. */
   socks_addrinit();

   if ((sockscf.option.configfile = socks_getenv(ENV_SOCKS_CONF, dontcare))
   == NULL)
      sockscf.option.configfile = SOCKS_CONFIGFILE;

   genericinit();
   newprocinit();

   /* no writing to stderr in client, so need to delay until here. */
   runenvcheck();

   showconfig(&sockscf);

   slog(LOG_INFO, "%s/client v%s running", PRODUCT, VERSION);
/*   sleep(20);                           */

   sockscf.state.inited = 1;
   initing = 0;
}
