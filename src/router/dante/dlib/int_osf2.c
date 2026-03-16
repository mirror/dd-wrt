/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2008, 2009, 2010
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

#if HAVE_EXTRA_OSF_SYMBOLS

#if SOCKSLIBRARY_DYNAMIC

static const char rcsid[] =
"$Id: int_osf2.c,v 1.21 2011/05/18 13:48:45 karls Exp $";

#undef accept
#undef getpeername
#undef getsockname
#undef recvfrom
#undef recvmsg
#undef sendmsg

   /* nfoo versions (with sockaddr len) of the system calls. */

int
sys_naccept(s, addr, addrlen)
   int s;
   struct sockaddr *addr;
   socklen_t *addrlen;
{
   int rc;
   int (*function)(int s, struct sockaddr *addr, socklen_t *addrlen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_NACCEPT);
   rc = function(s, addr, addrlen);
   socks_syscall_end(s);
   return rc;
}

int
sys_ngetpeername(s, name, namelen)
   int s;
   struct sockaddr *name;
   socklen_t *namelen;
{
   int rc;
   int (*function)(int s, const struct sockaddr *name, socklen_t *namelen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_NGETPEERNAME);
   rc = function(s, name, namelen);
   socks_syscall_end(s);
   return rc;
}

int
sys_ngetsockname(s, name, namelen)
   int s;
   struct sockaddr *name;
   socklen_t *namelen;
{
   int rc;
   int (*function)(int s, const struct sockaddr *name, socklen_t *namelen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_NGETSOCKNAME);
   rc = function(s, name, namelen);
   socks_syscall_end(s);
   return rc;
}

int
sys_nrecvfrom(s, buf, len, flags, from, fromlen)
   int s;
   void *buf;
   size_t len;
   int flags;
   struct sockaddr *from;
   size_t *fromlen;
{
   int rc;
   int (*function)(int s, void *buf, size_t len, int flags,
                   struct sockaddr *from, socklen_t *fromlen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_NRECVFROM);
   rc = function(s, buf, len, flags, from, fromlen);
   socks_syscall_end(s);
   return rc;
}

ssize_t
sys_nrecvmsg(s, msg, flags)
   int s;
   struct msghdr *msg;
   int flags;
{
   ssize_t rc;
   int (*function)(int s, struct msghdr *msg, int flags);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_NRECVMSG);
   rc = function(s, msg, flags);
   socks_syscall_end(s);
   return rc;
}

ssize_t
sys_nsendmsg(s, msg, flags)
   int s;
   const struct msghdr *msg;
   int flags;
{
   ssize_t rc;
   int (*function)(int s, const struct msghdr *msg, int flags);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_NSENDMSG);
   rc = function(s, msg, flags);
   socks_syscall_end(s);
   return rc;
}

   /*
    * the interpositioned functions.
    */

int
naccept(s, addr, addrlen)
   int s;
   struct sockaddr *addr;
   socklen_t *addrlen;
{
   if (socks_issyscall(s, SYMBOL_NACCEPT))
      return sys_naccept(s, addr, addrlen);
   return Raccept(s, addr, addrlen);
}
int
ngetpeername(s, name, namelen)
   int s;
   struct sockaddr *name;
   socklen_t *namelen;
{
   if (socks_issyscall(s, SYMBOL_NGETPEERNAME))
      return sys_ngetpeername(s, name, namelen);
   return Rgetpeername(s, name, namelen);
}

int
ngetsockname(s, name, namelen)
   int s;
   struct sockaddr *name;
   socklen_t *namelen;
{
   if (socks_issyscall(s, SYMBOL_NGETSOCKNAME))
      return sys_ngetpeername(s, name, namelen);
   return Rgetsockname(s, name, namelen);
}

ssize_t
nrecvfrom(s, buf, len, flags, from, fromlen)
   int s;
   void *buf;
   size_t len;
   int flags;
   struct sockaddr *from;
   size_t *fromlen;
{
   if (socks_issyscall(s, SYMBOL_NRECVFROM))
      return sys_nrecvfrom(s, buf, len, flags, from, fromlen);
   return Rrecvfrom(s, buf, len, flags, from, fromlen);
}

ssize_t
nrecvmsg(s, msg, flags)
   int s;
   struct msghdr *msg;
   int flags;
{
   if (socks_issyscall(s, SYMBOL_NRECVMSG))
      return sys_nrecvmsg(s, msg, flags);
   return Rrecvmsg(s, msg, flags);
}

ssize_t
nsendmsg(s, msg, flags)
   int s;
   const struct msghdr *msg;
   int flags;
{
   if (socks_issyscall(s, SYMBOL_NSENDMSG))
      return sys_nsendmsg(s, msg, flags);
   return Rsendmsg(s, msg, flags);
}

#endif /* SOCKSLIBRARY_DYNAMIC */

#endif /* HAVE_EXTRA_OSF_SYMBOLS */
