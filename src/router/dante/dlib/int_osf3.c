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

#include "autoconf.h"

#if HAVE_EXTRA_OSF_SYMBOLS

#if SOCKSLIBRARY_DYNAMIC

#undef _XOPEN_SOURCE_EXTENDED
#undef _SOCKADDR_LEN
#define HAVE_OSF_OLDSTYLE 1

/*
 * use of bzero in socks_syscall_start produces vast amounts of warnings
 * when compiling int_osf3.c (on OSF)
 */
#define bzero(a, b) (memset(a, 0, b))

#define SOCKS_DLIB_OSF

/* XXX prevents DEC cc from redefining these to _Ereadv/_Ewritev */
#define readv XXX
#define writev XXX

#include "common.h"

static const char rcsid[] =
"$Id: int_osf3.c,v 1.39 2012/06/01 20:23:05 karls Exp $";

#undef accept
#undef bind
#undef getpeername
#undef getsockname
#undef recvfrom
#undef recvmsg
#undef sendmsg
#undef connect
#undef readv
#undef writev
#undef sendto

/* XXX Attempt to support old library calls, which use old version of
   sockaddr and msghdr struct. Other parts of libdsocks are compiled
   using the new versions. */

/* sockaddr struct, with sa_len */
struct n_sockaddr {
   unsigned char    sa_len;      /* total length */
   sa_family_t sa_family;      /* address family */
   char       sa_data[14];   /* actually longer; address value */
};

struct n_msghdr {
   void     *msg_name;      /* optional address */
   size_t   msg_namelen;   /* size of address */
   struct   iovec *msg_iov;/* scatter/gather array */
   int      msg_iovlen;      /* # elements in msg_iov */
   void    *msg_control;   /* ancillary data, see below */
   size_t   msg_controllen;/* ancillary data buffer len */
   int      msg_flags;      /* flags on received message */
};

ssize_t Rsendto(int, const void *, size_t, int,
                const struct n_sockaddr *, socklen_t);
int Rconnect(int, const struct n_sockaddr *, socklen_t);
ssize_t Rsendmsg(int, const struct n_msghdr *, int);
ssize_t Rrecvfrom(int, void *, int, int, struct n_sockaddr *, socklen_t *);
int Rgetsockname(int, struct n_sockaddr *, socklen_t *);
int Rgetpeername(int, struct n_sockaddr *, socklen_t *);
int Raccept(int, struct n_sockaddr *, socklen_t *);
ssize_t Rrecvmsg(int s, struct n_msghdr *msg, int flags);
int Rbind(int, const struct n_sockaddr *, socklen_t);


   /* 'old' versions (with sockaddr len) of the system calls. */

int
sys_accept(s, addr, addrlen)
   int s;
   struct sockaddr *addr;
   int *addrlen;
{
   int rc;
   int (*function)(int s, struct sockaddr * addr, socklen_t *addrlen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_ACCEPT);
   rc = function(s, addr, (socklen_t *) addrlen);
   socks_syscall_end(s);
   return rc;
}

int
sys_getpeername(s, name, namelen)
   int s;
   struct sockaddr *name;
   int *namelen;
{
   int rc;
   int (*function)(int s, const struct sockaddr *name, socklen_t *namelen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_GETPEERNAME);
   rc = function(s, name, (socklen_t *) namelen);
   socks_syscall_end(s);
   return rc;
}

int
sys_getsockname(s, name, namelen)
   int s;
   struct sockaddr *name;
   int *namelen;
{
   int rc;
   int (*function)(int s, const struct sockaddr *name, socklen_t *namelen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_GETSOCKNAME);
   rc = function(s, name, (socklen_t *) namelen);
   socks_syscall_end(s);
   return rc;
}

int
sys_recvfrom(s, buf, len, flags, from, fromlen)
   int s;
   void *buf;
   int len;
   int flags;
   struct sockaddr *from;
   int *fromlen;
{
   int rc;
   int (*function)(int s, void *buf, size_t len, int flags,
                   struct sockaddr *from, socklen_t *fromlen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_RECVFROM);
   rc = function(s, buf, len, flags, from, (socklen_t *) fromlen);
   socks_syscall_end(s);
   return rc;
}

ssize_t
sys_recvmsg(s, msg, flags)
   int s;
   struct msghdr *msg;
   int flags;
{
   ssize_t rc;
   int (*function)(int s, struct msghdr *msg, int flags);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_RECVMSG);
   rc = function(s, msg, flags);
   socks_syscall_end(s);
   return rc;
}

ssize_t
sys_sendmsg(s, msg, flags)
   int s;
   struct msghdr *msg;
   int flags;
{
   ssize_t rc;
   int (*function)(int s, const struct msghdr *msg, int flags);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_SENDMSG);
   rc = function(s, msg, flags);
   socks_syscall_end(s);
   return rc;
}

ssize_t
sys_readv(d, iov, iovcnt)
   int d;
   const struct iovec *iov;
   int iovcnt;
{
   ssize_t rc;
   int (*function)(int d, const struct iovec *iov, int iovcnt);

   socks_syscall_start(d);
   function = symbolfunction(SYMBOL_READV);
   rc = function(d, iov, iovcnt);
   socks_syscall_end(d);
   return rc;
}

ssize_t
sys_writev(d, iov, iovcnt)
   int d;
   const struct iovec *iov;
   int iovcnt;
{
   ssize_t rc;
   int (*function)(int d, const struct iovec *buf, int iovcnt);

   socks_syscall_start(d);
   function = symbolfunction(SYMBOL_WRITEV);
   rc = function(d, iov, iovcnt);
   socks_syscall_end(d);
   return rc;
}

int
sys_connect(s, name, namelen)
   int s;
   const struct sockaddr *name;
   int namelen;
{
   int rc;
   int (*function)(int s, const struct sockaddr *name, socklen_t namelen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_CONNECT);
   rc = function(s, name, namelen);
   socks_syscall_end(s);
   return rc;
}

int
sys_bind(s, name, namelen)
   int s;
   const struct sockaddr *name;
   int namelen;
{
   int rc;
   int (*function)(int s, const struct sockaddr *name, socklen_t namelen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_BIND);
   rc = function(s, name, namelen);
   socks_syscall_end(s);
   return rc;
}

int
sys_sendto(s, msg, len, flags, to, tolen)
   int s;
   const void *msg;
   int len;
   int flags;
   const struct sockaddr *to;
   socklen_t tolen;
{
   ssize_t rc;
   int (*function)(int s, const void *msg, size_t len, int flags,
                   const struct sockaddr *to, socklen_t tolen);

   socks_syscall_start(s);
   function = symbolfunction(SYMBOL_SENDTO);
   rc = function(s, msg, len, flags, to, tolen);
   socks_syscall_end(s);
   return rc;
}

   /*
    * the interpositioned functions.
    */

/* sockaddr *, new len */
#define ADDRLEN_SET(a,b) (((a) == NULL) ? 0 : (b))

/* source sockaddr, len, dest sockaddr, len */
#define SOCKADDR_COPYRES(a,b,c,d) \
 do { if (((c) != NULL) && (*(d) > 0) && (*(b) > 0)) { \
      struct n_sockaddr n = *(a); \
      int tmplen = *(d); \
\
      if (tmplen >= sizeof(n.sa_family)) \
         (c)->sa_family = (unsigned short)n.sa_family; \
      tmplen -= sizeof((c)->sa_family); \
      tmplen = MAX(0, tmplen); \
      memcpy((c)->sa_data, n.sa_data, (size_t)tmplen); \
      *(d) = MIN(*d, sizeof(struct sockaddr)); \
   } \
  } while (/*CONSTCOND*/0)

/* source sockaddr, len, dest sockaddr, len */
#define SOCKADDR_COPYPARAM(a,b,c,d) \
 do { if (((a) != NULL) && (*(b) > 0)) { \
      struct n_sockaddr n; \
      int tmplen = *(b); \
\
      bzero((char *)&n, sizeof(n));  \
      /* struct sockaddr -> sa_family == unsigned short */ \
      if (tmplen >= sizeof(unsigned short))  \
         n.sa_family = (sa_family_t)(a)->sa_family; \
      tmplen -= sizeof(n.sa_family); \
      tmplen = MAX(0, tmplen); \
      memcpy(n.sa_data, (a)->sa_data, (size_t)tmplen); \
      *(d) = sizeof(n); \
      *c = n; \
   } else { \
      *(d) = *(b); \
   } \
  } while (/*CONSTCOND*/0)

int
accept(s, addr, addrlen)
   int s;
   struct sockaddr *addr;
   int *addrlen;
{
   struct n_sockaddr n_addr;
   socklen_t n_addrlen;
   int rc;

   if (socks_issyscall(s, SYMBOL_ACCEPT))
      return sys_accept(s, addr, addrlen);

   n_addrlen = ADDRLEN_SET(addr, sizeof(n_addr));

   rc = Raccept(s, &n_addr, &n_addrlen);

   if (rc != -1)
      SOCKADDR_COPYRES(&n_addr, &n_addrlen, addr, addrlen);

   return rc;
}

int
getpeername(s, name, namelen)
   int s;
   struct sockaddr *name;
   int *namelen;
{
   struct n_sockaddr n_name;
   socklen_t n_namelen;
   int rc;

   if (socks_issyscall(s, SYMBOL_GETPEERNAME))
      return sys_getpeername(s, name, namelen);

   n_namelen = ADDRLEN_SET(name, sizeof(n_name));

   rc = Rgetpeername(s, &n_name, &n_namelen);

   if (rc != -1)
      SOCKADDR_COPYRES(&n_name, &n_namelen, name, namelen);

   return rc;
}

int
getsockname(s, name, namelen)
   int s;
   struct sockaddr *name;
   int *namelen;
{
   struct n_sockaddr n_name;
   socklen_t n_namelen;
   int rc;

   if (socks_issyscall(s, SYMBOL_GETSOCKNAME))
      return sys_getsockname(s, name, namelen);

   n_namelen = ADDRLEN_SET(name, sizeof(n_name));

   rc = Rgetsockname(s, &n_name, &n_namelen);

   if (rc != -1)
      SOCKADDR_COPYRES(&n_name, &n_namelen, name, namelen);

   return rc;
}

int
recvfrom(s, buf, len, flags, from, fromlen)
   int s;
   void *buf;
   int len;
   int flags;
   struct sockaddr *from;
   int *fromlen;
{
   struct n_sockaddr n_from;
   socklen_t n_fromlen;
   ssize_t rc;

   if (socks_issyscall(s, SYMBOL_RECVFROM))
      return sys_recvfrom(s, buf, len, flags, from, fromlen);

   n_fromlen = ADDRLEN_SET(from, sizeof(n_from));

   rc = Rrecvfrom(s, buf, len, flags, (struct n_sockaddr *)&n_from, &n_fromlen);

   if (rc != -1)
      SOCKADDR_COPYRES(&n_from, &n_fromlen, from, fromlen);

   return rc;
}

int
recvmsg(s, msg, flags)
   int s;
   struct msghdr *msg;
   int flags;
{
   struct n_msghdr n_msg;
   ssize_t rc;

   if (socks_issyscall(s, SYMBOL_RECVMSG))
      return sys_recvmsg(s, msg, flags);

   bzero((char *)&n_msg, sizeof(struct n_msghdr));
   n_msg.msg_name = msg->msg_name;
   n_msg.msg_namelen = msg->msg_namelen;
   n_msg.msg_iov = msg->msg_iov;
   n_msg.msg_iovlen = msg->msg_iovlen;
   /* XXX msg_control/msg_accrights */

   rc = Rrecvmsg(s, &n_msg, flags);

   if (rc != -1) {
      msg->msg_name = n_msg.msg_name;
      msg->msg_namelen = n_msg.msg_namelen;
      msg->msg_iov = n_msg.msg_iov;
      msg->msg_iovlen = n_msg.msg_iovlen;
      /* XXX msg_control / msg_accrights */
      if (n_msg.msg_controllen != 0)
         swarn("msg_accrights/controllen conversion not supported");
   }

   return rc;
}

int
sendmsg(s, msg, flags)
   int s;
   struct msghdr *msg;
   int flags;
{
   struct n_msghdr n_msg;
   ssize_t rc;

   if (socks_issyscall(s, SYMBOL_SENDMSG))
      return sys_sendmsg(s, msg, flags);

   bzero((char *)&n_msg, sizeof(struct n_msghdr));
   n_msg.msg_name = msg->msg_name;
   n_msg.msg_namelen = msg->msg_namelen;
   n_msg.msg_iov = msg->msg_iov;
   n_msg.msg_iovlen = msg->msg_iovlen;
   /* XXX msg_control/msg_accrights */
   if  (msg->msg_accrightslen != 0)
      swarn("warning: msg_accrights/controllen conversion not supported");


   rc = Rsendmsg(s, (struct n_msghdr *)&n_msg, flags);

   if (rc != -1) {
      msg->msg_name = n_msg.msg_name;
      msg->msg_namelen = n_msg.msg_namelen;
      msg->msg_iov = n_msg.msg_iov;
      msg->msg_iovlen = n_msg.msg_iovlen;
      /* XXX msg_control / msg_accrights */
   }

   return rc;
}

int
connect(s, name, namelen)
   int s;
   const struct sockaddr *name;
   int namelen;
{
   struct n_sockaddr n_name;
   int n_namelen;

   if (socks_issyscall(s, SYMBOL_CONNECT))
      return sys_connect(s, name, namelen);

   SOCKADDR_COPYPARAM(name, &namelen, &n_name, &n_namelen);

   return Rconnect(s, &n_name, n_namelen);
}

int
bind(s, name, namelen)
   int s;
   const struct sockaddr *name;
   int namelen;
{
   struct n_sockaddr n_name;
   int n_namelen;

   if (socks_issyscall(s, SYMBOL_BIND))
      return sys_bind(s, name, namelen);

   SOCKADDR_COPYPARAM(name, &namelen, &n_name, &n_namelen);

   return Rbind(s, &n_name, n_namelen);
}

int
sendto(s, msg, len, flags, to, tolen)
   int s;
   const void *msg;
   int len;
   int flags;
   const struct sockaddr *to;
   int tolen;
{
   struct n_sockaddr n_to;
   int n_tolen;

   if (socks_issyscall(s, SYMBOL_SENDTO))
      return sys_sendto(s, msg, len, flags, to, tolen);

   SOCKADDR_COPYPARAM(to, &tolen, &n_to, &n_tolen);

   return Rsendto(s, msg, len, flags, &n_to, n_tolen);
}

ssize_t
writev(d, iov, iovcnt)
   int d;
   const struct iovec *iov;
   int iovcnt;
{
   if (socks_issyscall(d, SYMBOL_WRITEV))
      return sys_writev(d, iov, iovcnt);

   return Rwritev(d, iov, iovcnt);
}

ssize_t
readv(d, iov, iovcnt)
   int d;
   const struct iovec *iov;
   int iovcnt;
{
   if (socks_issyscall(d, SYMBOL_READV))
      return sys_readv(d, iov, iovcnt);

   return Rreadv(d, iov, iovcnt);
}

#endif /* SOCKSLIBRARY_DYNAMIC */

#endif /* HAVE_EXTRA_OSF_SYMBOLS */
