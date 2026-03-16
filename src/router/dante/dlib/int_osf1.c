/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2008, 2009, 2010, 2011
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
"$Id: int_osf1.c,v 1.24 2012/06/01 20:23:05 karls Exp $";

#undef accept
#undef getpeername
#undef getsockname
#undef readv
#undef recvfrom
#undef recvmsg
#undef sendmsg
#undef writev

   /* the system calls. */

int
sys_Eaccept(s, addr, addrlen)
   int s;
   struct sockaddr * addr;
   socklen_t *addrlen;
{
   int rc;
   int (*function)(int s, struct sockaddr *addr, socklen_t *addrlen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_EACCEPT);
   rc = function(s, addr, addrlen);
   socks_syscall_end(s);
   return rc;
}

int
sys_Egetpeername(s, name, namelen)
   int s;
   struct sockaddr * name;
   socklen_t *namelen;
{
   int rc;
   int (*function)(int s, const struct sockaddr *name, socklen_t *namelen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_EGETPEERNAME);
   rc = function(s, name, namelen);
   socks_syscall_end(s);
   return rc;
}

int
sys_Egetsockname(s, name, namelen)
   int s;
   struct sockaddr * name;
   socklen_t *namelen;
{
   int rc;
   int (*function)(int s, const struct sockaddr *name, socklen_t *namelen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_EGETSOCKNAME);
   rc = function(s, name, namelen);
   socks_syscall_end(s);
   return rc;
}

ssize_t
sys_Ereadv(d, iov, iovcnt)
   int d;
   const struct iovec *iov;
   int iovcnt;
{
   ssize_t rc;
   int (*function)(int d, const struct iovec *iov, int iovcnt);

   socks_syscall_start(d);
   function = symbolfunction(SYMBOL_EREADV);
   rc = function(d, iov, iovcnt);
   socks_syscall_end(d);
   return rc;
}

int
sys_Erecvfrom(s, buf, len, flags, from, fromlen)
   int s;
   void *buf;
   size_t len;
   int flags;
   struct sockaddr * from;
   size_t *fromlen;
{
   int rc;
   int (*function)(int s, void *buf, size_t len, int flags,
                   struct sockaddr *from, socklen_t *fromlen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_ERECVFROM);
   rc = function(s, buf, len, flags, from, fromlen);
   socks_syscall_end(s);
   return rc;
}

ssize_t
sys_Erecvmsg(s, msg, flags)
   int s;
   struct msghdr *msg;
   int flags;
{
   ssize_t rc;
   int (*function)(int s, struct msghdr *msg, int flags);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_ERECVMSG);
   rc = function(s, msg, flags);
   socks_syscall_end(s);
   return rc;
}

ssize_t
sys_Esendmsg(s, msg, flags)
   int s;
   const struct msghdr *msg;
   int flags;
{
   ssize_t rc;
   int (*function)(int s, const struct msghdr *msg, int flags);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_ESENDMSG);
   rc = function(s, msg, flags);
   socks_syscall_end(s);
   return rc;
}

ssize_t
sys_Ewritev(d, iov, iovcnt)
   int d;
   const struct iovec *iov;
   int iovcnt;
{
   ssize_t rc;
   int (*function)(int d, const struct iovec *buf, int iovcnt);

   socks_syscall_start(d);
   function = symbolfunction(SYMBOL_EWRITEV);
   rc = function(d, iov, iovcnt);
   socks_syscall_end(d);
   return rc;
}

   /*
    * the interpositioned functions.
    */

int
_Eaccept(s, addr, addrlen)
   int s;
   struct sockaddr * addr;
   socklen_t *addrlen;
{
   if (socks_issyscall(s, SYMBOL__EACCEPT))
      return sys_Eaccept(s, addr, addrlen);
   return Raccept(s, addr, addrlen);
}

int
_Egetpeername(s, name, namelen)
   int s;
   struct sockaddr * name;
   socklen_t *namelen;
{
   if (socks_issyscall(s, SYMBOL__EGETPEERNAME))
      return sys_Egetpeername(s, name, namelen);
   return Rgetpeername(s, name, namelen);
}

int
_Egetsockname(s, name, namelen)
   int s;
   struct sockaddr * name;
   socklen_t *namelen;
{
   if (socks_issyscall(s, SYMBOL__EGETSOCKNAME))
      return sys_Egetsockname(s, name, namelen);
   return Rgetsockname(s, name, namelen);
}

ssize_t
_Ereadv(d, iov, iovcnt)
   int d;
   const struct iovec *iov;
   int iovcnt;
{
   if (socks_issyscall(d, SYMBOL__EREADV))
      return sys_Ereadv(d, iov, iovcnt);
   return Rreadv(d, iov, iovcnt);
}

ssize_t
_Erecvfrom(s, buf, len, flags, from, fromlen)
   int s;
   void *buf;
   size_t len;
   int flags;
   struct sockaddr * from;
   size_t *fromlen;
{
   if (socks_issyscall(s, SYMBOL__ERECVFROM))
      return sys_Erecvfrom(s, buf, len, flags, from, fromlen);
   return Rrecvfrom(s, buf, len, flags, from, fromlen);
}

ssize_t
_Erecvmsg(s, msg, flags)
   int s;
   struct msghdr *msg;
   int flags;
{
   if (socks_issyscall(s, SYMBOL__ERECVMSG))
      return sys_Erecvmsg(s, msg, flags);
   return Rrecvmsg(s, msg, flags);
}

ssize_t
_Ewritev(d, iov, iovcnt)
   int d;
   const struct iovec *iov;
   int iovcnt;
{
   if (socks_issyscall(d, SYMBOL__EWRITEV))
      return sys_Ewritev(d, iov, iovcnt);
   return Rwritev(d, iov, iovcnt);
}

ssize_t
_Esendmsg(s, msg, flags)
   int s;
   const struct msghdr *msg;
   int flags;
{
   if (socks_issyscall(s, SYMBOL__ESENDMSG))
      return sys_Esendmsg(s, msg, flags);
   return Rsendmsg(s, msg, flags);
}

#endif /* SOCKSLIBRARY_DYNAMIC */

#endif /* HAVE_EXTRA_OSF_SYMBOLS */
