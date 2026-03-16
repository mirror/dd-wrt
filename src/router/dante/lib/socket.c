/*
 * Copyright (c) 1997, 1998, 1999, 2001, 2005, 2008, 2009, 2010, 2011, 2012,
 *               2013, 2014, 2016, 2017, 2020
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
"$Id: socket.c,v 1.218.4.7.2.7.4.3 2020/11/11 16:11:54 karls Exp $";

int
socks_connecthost(s,
#if !SOCKS_CLIENT
                  side,
#endif /* !SOCKS_CLIENT */
                  host,
                  laddr,
                  raddr,
                  timeout,
                  emsg,
                  emsglen)
   int s;
#if !SOCKS_CLIENT
   const interfaceside_t side;
#endif /* !SOCKS_CLIENT */
   const sockshost_t *host;
   struct sockaddr_storage *laddr;
   struct sockaddr_storage *raddr;
   const long timeout;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_connecthost()";
   static fd_set *wset;
#if SOCKS_CLIENT
   interfaceside_t side = EXTERNALIF; /* doesn't matter. */
#endif
   dnsinfo_t resmem;
   struct sockaddr_storage laddr_mem, raddr_mem;
   struct addrinfo hints, *res, *next;
   socklen_t len;
   char addrstr[MAXSOCKADDRSTRING], hoststr[MAXSOCKSHOSTSTRING],
        laddrstr[MAXSOCKADDRSTRING];
   int failed, rc;

   /*
    * caller depends on errno to know whether the connect(2) failed
    * permanently, or whether things are now in progress, so make sure
    * errno is correct upon return, and definitely not some old residue.
    */
   errno = 0;

   if (wset == NULL)
      wset = allocate_maxsize_fdset();

   if (laddr == NULL)
      laddr = &laddr_mem;

   if (raddr == NULL)
      raddr = &raddr_mem;

   len = sizeof(*laddr);
   if (getsockname(s, TOSA(laddr), &len) == -1) {
      snprintf(emsg, emsglen, "getsockname(2) failed: %s", strerror(errno));
      return -1;
   }

   sockaddr2string(laddr, laddrstr, sizeof(laddrstr));

   slog(LOG_NEGOTIATE,
        "%s: connect to %s on %s side from %s, fd %d.  Timeout is %ld\n",
        function,
        sockshost2string(host, hoststr, sizeof(hoststr)),

#if !SOCKS_CLIENT

        interfaceside2string(side),

#else /* SOCKS_CLIENT */

        "<N/A>",

#endif /* SOCKS_CLIENT */

        laddrstr,
        s,
        timeout);

   bzero(raddr, sizeof(*raddr));

   switch (host->atype) {
      case SOCKS_ADDR_IPV4:
      case SOCKS_ADDR_IPV6: {
         int connect_errno, flags, changed_to_nonblocking;

         changed_to_nonblocking = 0;
         flags                  = -1;

         if (timeout != -1) {
            if ((flags = fcntl(s, F_GETFL, 0)) == -1) {
               snprintf(emsg, emsglen, "fcntl(F_GETFL) failed: %s",
                        strerror(errno));

               return -1;
            }

            if (!(flags & O_NONBLOCK)) {
               slog(LOG_DEBUG,
                    "%s: temporarily changing fd %d to nonblocking in order "
                    "to facilitate the specified connect timeout (%ld)",
                    function, s, timeout);

               if (fcntl(s, F_SETFL, flags | O_NONBLOCK) == -1) {
                  snprintf(emsg, emsglen,
                           "could not change fd %d to nonblocking: %s",
                           s, strerror(errno));

                  return -1;
               }

               changed_to_nonblocking = 1;
            }
         }

         switch (host->atype) {
            case SOCKS_ADDR_IPV4:
               SET_SOCKADDR(raddr, AF_INET);
               TOIN(raddr)->sin_addr = host->addr.ipv4;
               break;

            case SOCKS_ADDR_IPV6:
               SET_SOCKADDR(raddr, AF_INET6);
               TOIN6(raddr)->sin6_addr     = host->addr.ipv6.ip;
               TOIN6(raddr)->sin6_scope_id = host->addr.ipv6.scopeid;
               break;

            default:
               SERRX(host->atype);
         }

         SET_SOCKADDRPORT(raddr, host->port);

         rc            = connect(s, TOSA(raddr), salen(raddr->ss_family));
         connect_errno = errno;

         if (rc == 0 || errno == EINPROGRESS) {
            /*
             * if local addr was incomplete before, it should be complete now.
             */
            if (!IPADDRISBOUND(laddr)) {
               len = sizeof(*laddr);
               if (getsockname(s, TOSA(laddr), &len) == -1) {
                  snprintf(emsg, emsglen,
                           "getsockname(2) after connect(2) failed: %s",
                           strerror(errno));

                  return -1;
               }

               sockaddr2string(laddr, laddrstr, sizeof(laddrstr));
            }
         }

         snprintf(emsg, emsglen,
                  "connect(2) to %s from %s on fd %d returned %ld (%s)",
                  hoststr, laddrstr, s, (long)rc, strerror(errno));

         slog(LOG_DEBUG, "%s: %s", function, emsg);

         if (changed_to_nonblocking) {
            SASSERTX(flags != -1);

            if (fcntl(s, F_SETFL, flags & ~O_NONBLOCK) == -1)
               swarn("%s: failed reverting fd %d back to blocking",
                     function, s);
         }

         if (rc == 0)
            /*
             * OpenBSD 4.5 sometimes sets errno even though the
             * connect was successful.  Seems to be an artifact of the
             * buggy threads library, where it does a select(2)/poll(2)
             * after making the socket non-blocking, but forgets to
             * reset errno.
             */
            connect_errno = 0;

         errno = connect_errno;

#if SOCKS_CLIENT
         /*
          * if errno is EINTR, it may be due to the client having set up an
          * alarm for this.  We can't know for sure, so better not
          * retry in that case.
          */
          if (rc == -1) {
            if (errno == EINTR) {
               snprintf(emsg, emsglen, "connect(2) to %s from %s failed: %s",
                        sockaddr2string(raddr, NULL, 0),
                        laddrstr,
                        strerror(errno));

               return rc;
            }

            if (!changed_to_nonblocking) {
               /*
                * was passed a non-blocking fd by the client, so client does
                * not want to wait for the connect to complete.  Let the
                * connect child handle this then, if applicable.
                */
               snprintf(emsg, emsglen,
                        "non-blocking connect(2): %s", strerror(errno));

               return rc;
            }
         }
#endif /* SOCKS_CLIENT */

         while (timeout != 0
         &&     rc      == -1
         &&   (  errno == EINPROGRESS
#if !SOCKS_CLIENT
              || errno == EINTR
#endif /* !SOCKS_CLIENT */
         )) {
            struct timeval tval = { timeout, (long)0 };

            FD_ZERO(wset);
            FD_SET(s, wset);

            rc = selectn(s + 1,
                         NULL,
                         NULL,
                         NULL,
                         wset,
                         NULL,
                         timeout >= 0 ? &tval : NULL);

            switch (rc) {
               case -1:
                  if (ERRNOISTMP(errno))
                     continue;
                  else {
                     snprintf(emsg, emsglen, "select(2) on fd %d failed: %s",
                              s, strerror(errno));

                     return -1;
                  }

               case 0:
                  errno = ETIMEDOUT;
                  break;

               default:
                  len = sizeof(errno);
                  getsockopt(s, SOL_SOCKET, SO_ERROR, &errno, &len);
            }

            if (errno == 0)
               rc = 0;
            else
               /*
                * connect(2)-attempt finished, but failed.
                */
               rc = -1;
         }

         slog(LOG_NEGOTIATE, "%s: connect to %s from %s on fd %d %s (%s)",
              function,
              sockaddr2string(raddr, addrstr, sizeof(addrstr)),
              laddrstr,
              s,
              rc == 0 ?   "ok"
                        : errno == EINPROGRESS ? "in progress" : "failed",
              strerror(errno));

         if (rc == -1) {
            log_connectfailed(side, addrstr);

            snprintf(emsg, emsglen, "connect(2) to %s from %s %s: %s",
                     sockaddr2string(raddr, NULL, 0),
                     laddrstr,
                     errno == EINPROGRESS ? "is in progress" : "failed",
                     strerror(errno));
         }

         return rc;
      }

      case SOCKS_ADDR_DOMAIN: {
         socklen_t len;
         char visbuf[MAXHOSTNAMELEN * 4];

         bzero(&hints, sizeof(hints));

         /*
          * We'd like to set ai_family to laddr->ss_family, but
          * IPv4-mapped IPv6 addresses screw that up since if laddr
          * is an IPv4 address (and thus 's' is an IPv4 socket),
          * we can connect to the IPv4-mapped IPv6 address, but we need to
          * include IPv6 in the hints.ai_family in order to get those
          * IPv4-mapped IPv6 addresses from getaddrinfo(3).
          * (cgetaddrinfo() converts them to regular IPv4-addresses for us,
          * but in order for it to get them in the first place ...).
          *
          * A defect in the getaddrinfo() api, not having a flag for us
          * to request it includes IPv4-mapped IPv6 addresses, converted to
          * regular IPv4 addresses.
          */
         if (laddr->ss_family == AF_INET)
            hints.ai_family = 0; /* or we will not get the IPv4-mapped IPv6. */
         else {
            SASSERTX(laddr->ss_family == AF_INET6);
            hints.ai_family = laddr->ss_family;
         }

         len = sizeof(hints.ai_socktype);
         if (getsockopt(s, SOL_SOCKET, SO_TYPE, &hints.ai_socktype, &len) != 0){
            snprintf(emsg, emsglen,
                     "could not determine type of socket for fd %d "
                     "- getsockopt(SO_TYPE) failed: %s",
                     s, strerror(errno));

            return -1;
         }

         if ((rc = cgetaddrinfo(host->addr.domain, NULL, &hints, &res, &resmem))
         != 0) {
            snprintf(emsg, emsglen,
                     "could not resolve hostname \"%s\": %s",
                     str2vis(host->addr.domain,
                             strlen(host->addr.domain),
                             visbuf,
                             sizeof(visbuf)),
                     gai_strerror(rc));

            errno = EHOSTUNREACH; /* anything but EINPROGRESS. */
            return -1;
         }

#if DIAGNOSTIC
         SASSERTX(hints.ai_family  == 0 || res->ai_family == hints.ai_family);
         SASSERTX(res->ai_socktype == hints.ai_socktype);
#endif /* DIAGNOSTIC */

         break;
      }

      default:
         SERRX(host->atype);
   }

   SASSERTX(host->atype == SOCKS_ADDR_DOMAIN);
   SASSERTX(res->ai_addr != NULL);

   /*
    * try all ipaddresses hostname resolved to.
    */

   failed = 0;
   next   = res;
   do {
      sockshost_t newhost;

      if (next->ai_family != laddr->ss_family) {
         snprintf(emsg, emsglen,
                  "can not attempt connect to address %s (resolved from %s) "
                  "from our %s-socket on the external side",
                  sockaddr2string(TOSS(next->ai_addr), NULL, 0),
                  hoststr,
                  safamily2string(laddr->ss_family));
         errno = EAFNOSUPPORT;

         next  = next->ai_next;
         continue;
      }

      if (failed) { /* previously failed, need to create a new socket. */
         int new_s;

         if ((new_s = socketoptdup(s, -1)) == -1) {
            snprintf(emsg, emsglen, "socketoptdup() failed: %s",
                     strerror(errno));

            return -1;
         }

         if (dup2(new_s, s) == -1) {
            snprintf(emsg, emsglen, "dup2() failed: %s", strerror(errno));
            close(new_s);

            return -1;
         }
         close(new_s); /* s is now a new socket but keeps the same index. */

         /*
          * try to bind the same address/port on the new socket.
          */

         if (socks_bind(s, laddr, 1) != 0) {
            snprintf(emsg, emsglen, "socks_bind() failed: %s", strerror(errno));
            return -1;
         }
      }

      /*
       * Convert next address to sockshost_t and call ourselves again.
       */

      sockaddrcpy(raddr, TOSS(next->ai_addr), sizeof(*raddr));
      SET_SOCKADDRPORT(raddr, host->port);

      sockaddr2sockshost(raddr, &newhost);

      if (next->ai_next == NULL) {
         /*
          * no more ip addresses to try.  That means we can simply call
          * socks_connecthost() with the timeout as received.
          * If not, we will need to disregard the passed in timeout and
          * connect to one address at a time and await the result. :-/
          *
          * XXX improve this by keeping track of how much time we've used
          * so far, so we can decrement the timeout on each connecthost()
          * call?
          */
         rc = socks_connecthost(s,
#if !SOCKS_CLIENT
                                side,
#endif /* !SOCKS_CLIENT */
                                &newhost,
                                laddr,
                                raddr,
                                timeout,
                                emsg,
                                emsglen);
      }
      else
         rc = socks_connecthost(s,
#if !SOCKS_CLIENT
                                side,
#endif /* !SOCKS_CLIENT */
                                &newhost,
                                laddr,
                                raddr,
                                sockscf.timeout.connect ?
                                    (long)sockscf.timeout.connect : (long)-1,
                                emsg,
                                emsglen);

      if (rc == 0) {
         errno = 0;

         if (emsg != NULL)
            *emsg = NUL;

         return 0;
      }

      /*
       * Only retry/try next address if errno indicates server/network error.
       */
      switch (errno) {
         case ETIMEDOUT:
         case EINVAL:
         case ECONNREFUSED:
         case ENETUNREACH:
         case EHOSTUNREACH:
            break;

         default:
            return -1;
      }

      failed = 1;
      next   = next->ai_next;
   } while (next != NULL);

   /*
    * list exhausted, no successful connect.
    */

   SASSERTX(errno != 0);

   if (emsg != NULL)
      SASSERTX(*emsg != NUL);

   return -1;
}

#undef socket
int
socks_socket(domain, type, protocol)
   const int domain;
   const int type;
   const int protocol;
{
   const char *function = "socks_socket()";
   int s;

   if ((s = socket(domain, type, protocol)) == -1)
      return -1;

#if !SOCKS_CLIENT
   if (domain == AF_INET6) {
      socklen_t len;
      int value;

      value = 1;
      len   = sizeof(value);

      if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &value, len) != 0)
         swarn("%s: setsockopt(IPV6_V6ONLY) on fd %d failed", function, s);
   }
#endif /* !SOCKS_CLIENT */

   return s;
}


int
acceptn(s, addr, addrlen)
   int s;
   struct sockaddr_storage *addr;
   socklen_t *addrlen;
{
   const char *function = "acceptn()";
   struct sockaddr_storage fulladdr;
   socklen_t fulladdrlen = sizeof(fulladdr);
   int rc;

#if DEBUG /* for occasional internal debugging/analysis. */

#ifdef SO_LISTENQLEN
   if (sockscf.option.debug) {
      socklen_t len;
      int listenqlen, listeninqclen, failed;

      failed = 0;

      len = sizeof(listenqlen);
      if (getsockopt(s, SOL_SOCKET, SO_LISTENQLEN, &listenqlen, &len) == -1) {
         swarn("%s: getsockopt(SO_LISTENQLEN) failed", function);
         failed = 1;
      }

      len  = sizeof(listeninqclen);
      if (getsockopt(s, SOL_SOCKET, SO_LISTENINCQLEN, &listeninqclen, &len)
      == -1) {
         swarn("%s: getsockopt(SO_LISTENINCQLEN) failed", function);
         failed = 1;
      }

      if (!failed)
         slog(LOG_DEBUG, "%s: fd %d.  listenqlen: %d, listeninqclen: %d",
              function, s, listenqlen, listeninqclen);
   }
#endif /* SO_LISTENQLEN */

#endif /* DEBUG */

   while ((rc = accept(s, TOSA(&fulladdr), &fulladdrlen)) == -1
   &&     errno == EINTR)
#if !SOCKS_CLIENT
      /*
       * XXX only here because request children block on accept(2).
       * Remove it if we some day improve the request children so they no
       * longer do that.
       */
      (void)sockd_handledsignals();
#else /* SOCKS_CLIENT */
      ;
#endif /* SOCKS_CLIENT */

#if !SOCKS_CLIENT && HAVE_LINUX_BUGS
   if (rc != -1) {
      /*
       * On Linux fd-flags are not inherited by accept(2) for some reason.
       */
      if (fcntl(rc, F_SETFL, fcntl(s, F_GETFL, 0)) != 0) {
         swarn("%s: attempt to work around Linux bug via fcntl(2) failed",
               function);

         close(rc);
         return -1;
      }
   }
#endif /* !SOCKS_CLIENT && HAVE_LINUX_BUGS */

   if (rc != -1)
      /*
       * Avoids Valgrind complaining about us sendmsg(2)-ing uninitialized
       * parts of the sockaddr_storage struct accept(2)-ed above.
       */
      sockaddrcpy(addr, &fulladdr, (size_t)*addrlen);

   *addrlen = MIN(*addrlen, (socklen_t)fulladdrlen);

   return rc;
}

int
setnonblocking(fd, ctx)
   const int fd;
   const char *ctx;
{
   const char *function = "setnonblocking()";
   int flags;

   SASSERTX(ctx != NULL);

   if ((flags = fcntl(fd, F_GETFL, 0))                 != -1
   &&           fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1) {
      slog(LOG_DEBUG, "%s: fd %d: %s", function, fd, ctx);
      return flags;
   }
   else {
      swarn("failed to make fd %d, used for %s, non-blocking", fd, ctx);
      return -1;
   }
}

int
setblocking(fd, ctx)
   const int fd;
   const char *ctx;
{
   const char *function = "setblocking()";
   int flags;

   SASSERTX(ctx != NULL);

   if ((flags = fcntl(fd, F_GETFL, 0))                  != -1
   &&           fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) != -1) {
      slog(LOG_DEBUG, "%s: fd %d: %s", function, fd, ctx);
      return flags;
   }
   else {
      swarn("failed to make fd %d, used for %s, blocking", fd, ctx);
      return -1;
   }
}

int
socks_socketisforlan(s)
   const int s;
{
   const char *function = "socks_socketisforlan()";
   struct in_addr addr;
   socklen_t len;
   unsigned char ttl;
   const int errno_s = errno;

   /*
    * make an educated guess as to whether the socket is intended for
    * lan-only use or not.
    */

   len = sizeof(addr);
   if (getsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &addr, &len) != 0) {
      slog(LOG_DEBUG, "%s: getsockopt(IP_MULTICAST_IF) failed: %s",
           function, strerror(errno));

      errno = errno_s;
      return 0;
   }

   if (addr.s_addr == htonl(INADDR_ANY))
      return 0;

   len = sizeof(ttl);
   if (getsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, &len) != 0) {
      swarn("%s: getsockopt(IP_MULTICAST_TTL)", function);

      errno = errno_s;
      return 0;
   }

   return ttl == 1;
}

struct sockaddr_storage *
socketisconnected(s, addr, addrlen)
   const int s;
   struct sockaddr_storage *addr;
   socklen_t addrlen;
{
   const char *function = "socketisconnected()";
   socklen_t len;
   int err;

   if (addr == NULL || addrlen == 0) {
      static struct sockaddr_storage addrmem;

      addr     = &addrmem;
   }

   len = sizeof(err);
   (void)getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &len);

   if (err == 0) {
      if (getpeername(s, TOSA(addr), &len) == -1) { /* strange. */
         /*
          * presumably a timing issue; connection failed between
          * our getsockopt(2) call and now.
          */
         return NULL;
      }
   }
   else
      return NULL;

   return addr;
}

int
socks_rebind(s, protocol, from, to, emsg, emsglen)
   int s;
   int protocol;
   struct sockaddr_storage *from;
   const struct ruleaddr_t *to;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_rebind()";
   struct sockaddr_storage tobind;

   slog(LOG_NEGOTIATE, "%s: fd %d, protocol %s, from %s, to %s",
        function,
        s,
        protocol2string(protocol),
        sockaddr2string(from, NULL, 0),
        ruleaddr2string(to, ADDRINFO_PORT, NULL, 0));

   ruleaddr2sockaddr(to, &tobind, protocol);
   if (!IPADDRISBOUND(&tobind)) {
      snprintf(emsg, emsglen, "could not convert %s to an IP-address",
               ruleaddr2string(to, 0, NULL, 0));
      swarnx("%s: %s", function, emsg);

      errno = EADDRNOTAVAIL;
      return -1;
   }

   if (IPADDRISBOUND(from) || PORTISBOUND(from)) {
#if SOCKS_CLIENT

      rlim_t maxofiles;
      int i;

#endif /* SOCKS_CLIENT */

      int new_s, rc;

      /*
       * bound locally already.  Does it by coincidence match what we
       * want, or do we need to create a new socket and bind that instead?
       */
      if (addrmatch(to, sockaddr2sockshost(from, NULL), NULL, protocol, 0))
         return 0; /* matches already. */

      /*
       * Nope, need to create a new socket so we can bind wanted address.
       */

      if ((new_s = socketoptdup(s, -1)) == -1) {
         snprintf(emsg, emsglen,
                  "could not dup(2) fd %d with socketopdup(): %s",
                  s, strerror(errno));

         swarnx("%s: %s", function, emsg);
         return -1;
      }

#if SOCKS_CLIENT

      /*
       * The problem now is that caller may also have created dup(2)'s
       * of this socket, so us creating a new socket and assigning it the
       * index of the old socket won't do much good if the caller keeps using
       * his own dup(2)-ed version of the old socket.
       * We try to handle this by searching through the fd index space
       * for dups of 's', and dup2(2)'ing them too.
       */
      maxofiles = getmaxofiles(softlimit);
      for (i = 0; i < (int)maxofiles; ++i) {
         if (i == s)
            continue;

         if (fdisdup(i, s)) {
            slog(LOG_INFO,
                 "%s: found socket duped by client, fd %d is dup of fd %d",
                 function, i, s);

            if (dup2(new_s, i) == -1) {
               snprintf(emsg, emsglen,
                        "could not dup2(2) fd %d to %d (for clients dup): %s",
                        new_s, i, strerror(errno));

               swarnx("%s: %s", function, emsg);
               close(new_s);

               /*
                * If we previously found sockets to dup2(), we are basically
                * SOL and things will be screwed up for the client.  Sorry. :-/
                */
               return -1;
            }
         }
      }

#endif /* SOCKS_CLIENT */

      rc = dup2(new_s, s);
      close(new_s);

      if (rc == -1) {
         snprintf(emsg, emsglen, "could not dup2(2) fd %d to %d: %s",
                  new_s, s, strerror(errno));

         swarnx("%s: %s", function, emsg);
         return -1;
      }

      /*
       * Fist try to close the old socket and rebind the new with the same
       * portnumber as the old used, if the old portnumber matches what
       * caller wants.
       */

      SET_SOCKADDRPORT(&tobind, GET_SOCKADDRPORT(from));
      if (addrmatch(to, sockaddr2sockshost(&tobind, NULL), NULL, protocol, 0)) {
         if (socks_bind(s, &tobind, 0) == 0)
            return 0;
      }
      SET_SOCKADDRPORT(&tobind, htons(0));
   }
   /* else: socket is not bound, so can just bind it with the right address. */

   if (socks_bindinrange(s,
                         &tobind,
                         protocol == SOCKS_TCP ? to->port.tcp : to->port.udp,
                         to->portend,
                         to->operator) != 0) {
      snprintf(emsg, emsglen, "could not bind(2) fd %d in range %s: %s",
               s, ruleaddr2string(to, ADDRINFO_PORT, NULL, 0), strerror(errno));

      swarnx("%s: %s", function, emsg);

      errno = EADDRNOTAVAIL;
      return -1;
   }

   slog(LOG_NEGOTIATE, "%s: successfully rebound %s-fd %d.  New address is %s",
        function,
        protocol2string(protocol),
        s,
        sockaddr2string(&tobind, NULL, 0));

   return 0;
}

int
socks_bindinrange(s, addr, first, last, op)
   int s;
   struct sockaddr_storage *addr;
   in_port_t first, last;
   const enum operator_t op;
{
   const char *function = "socks_bindinrange()";
   in_port_t port;
   int exhausted;

   slog(LOG_DEBUG, "%s: %s %u %s %u",
        function,
        sockaddr2string(addr, NULL, 0),
        ntohs(first),
        operator2string(op),
        ntohs(last));


   /*
    * use ports in host order to make it easier.  Only convert before bind.
    */

   /* try this one first, if possible. */
   port        = 0;
   first       = ntohs(first);
   last        = ntohs(last);
   exhausted   = 0;

   do {
      if (port + 1 == 0) /* wrapped. */
         exhausted = 1;

      /* find next port to try. */
      switch (op) {
         case none:
            port = 0; /* any port is good. */
            break;

         case eq:
            port = first;
            break;

         case neq:
            if (++port == first)
               ++port;
            break;

         case ge:
            if (port < first)
               port = first;
            else
               ++port;
            break;

         case gt:
            if (port <= first)
               port = first + 1;
            else
               ++port;
            break;

         case le:
            if (++port > first)
               exhausted = 1;
            break;

         case lt:
            if (++port >= first)
               exhausted = 1;
            break;

         case range:
            if (port < first)
               port = first;
            else
               ++port;

            if (port > last)
               exhausted = 1;
            break;

         default:
            SERRX(op);
      }

      if (exhausted) {
         slog(LOG_NEGOTIATE,
              "%s: exhausted search for port to bind in range %u %s %u",
              function, first, operator2string(op), last);

         return -1;
      }

      SET_SOCKADDRPORT(addr,  htons(port));
      if (socks_bind(s, addr, 0) == 0)
         return 0;

      if (errno == EACCES) {
         if  (op == gt || op == ge || op == range)
            port = 1023; /* short-circuit to first possibility - 1. */
         else if (op == lt || op == le)
            exhausted = 1; /* going down, will get same error for all. */
      }

      if (op == eq || op == none)
         break; /* nothing to retry for these. */
   } while (!exhausted);

   return -1;
}

int
socks_bind(s, addr, retries)
   int s;
   struct sockaddr_storage *addr;
   size_t retries;
{
   const char *function = "socks_bind()";
   int p;

   slog(LOG_DEBUG, "%s: trying to bind address %s on fd %d.  Retries is %lu",
        function, sockaddr2string(addr, NULL, 0), s, (unsigned long)retries);

   errno = 0;
   while (1) {
#if !SOCKS_CLIENT
      if (PORTISRESERVED(GET_SOCKADDRPORT(addr)))
         sockd_priv(SOCKD_PRIV_NET_ADDR, PRIV_ON);
#endif /* !SOCKS_CLIENT */

      p = bind(s, TOSA(addr), salen(addr->ss_family));

#if !SOCKS_CLIENT
      if (PORTISRESERVED(GET_SOCKADDRPORT(addr)))
         sockd_priv(SOCKD_PRIV_NET_ADDR, PRIV_OFF);
#endif /* !SOCKS_CLIENT */

      if (p == 0) {
         socklen_t addrlen = sizeof(*addr);
         p                 = getsockname(s, TOSA(addr), &addrlen);
         break;
      }

      slog(LOG_DEBUG, "%s: failed to bind %s (%s)",
           function, sockaddr2string(addr, NULL, 0), strerror(errno));

      /*
       * else;  non-fatal error and retry?
       */
      switch (errno) {
         case EINTR:
            continue; /* don't count this attempt. */

         case EADDRINUSE:
            if (retries--) {
               sleep(1);
               continue;
            }
            break;

         /* default: fatal error. */
      }

      break;
   }

   if (p == 0)
      slog(LOG_DEBUG, "%s: bound address %s on fd %d",
           function, sockaddr2string(addr, NULL, 0), s);

   return p;
}

int
fdisdup(fd1, fd2)
   const int fd1;
   const int fd2;
{
   const char *function = "fdisdup()";
#if HAVE_UNIQUE_SOCKET_INODES
   struct stat sb1, sb2;
#endif /* HAVE_UNIQUE_SOCKET_INODES */
   socklen_t len1, len2;
   int isdup, rc1, rc2, errno1, errno2, flags1, flags2,  newflags1, newflags2,
       testflag = SO_REUSEADDR, setflag;

   slog(LOG_DEBUG, "%s: fd %d, fd %d", function, fd1, fd2);

   if (fd1 == fd2)
      return 1;

#if HAVE_UNIQUE_SOCKET_INODES
   rc1    = fstat(fd1, &sb1);
   errno1 = errno;

   rc2    = fstat(fd2, &sb2);
   errno2 = errno;

   if (rc1 != rc2 || errno1 != errno2) {
      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: failed due to fstat() on line %d",
               function, __LINE__);

      return 0;
   }

   if (rc1 == -1) {
      SASSERTX(rc2 == -1 && errno1 == errno2);

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: failed due to rc1 on line %d",
              function, __LINE__);

      return 1; /* assume any failed socket is as good as any other. */
   }

   if (sb1.st_ino == 0)
      slog(LOG_DEBUG, "%s: socket inode is 0.  Assuming kernel does "
                      "not support the inode field for (this) socket, so "
                      "continuing with other tests",
                      function);
   else if (sb1.st_dev != sb2.st_dev
   ||       sb1.st_ino != sb2.st_ino) {
      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: failed due to inode-compare on line %d "
                         "(sb1.st_dev = %d, sb2.st_dev = %d, "
                         "sb1.st_ino = %d, sb2.st_ino = %d)",
                         function, __LINE__,
                         (int)sb1.st_dev, (int)sb2.st_dev,
                         (int)sb1.st_ino, (int)sb2.st_ino);

      return 0;
   }
#endif /* HAVE_UNIQUE_SOCKET_INODES */

   len1   = sizeof(flags1);
   rc1    = getsockopt(fd1, SOL_SOCKET, testflag, &flags1, &len1);
   errno1 = errno;

   len2   = sizeof(flags2);
   rc2    = getsockopt(fd2, SOL_SOCKET, testflag, &flags2, &len2);
   errno2 = errno;

   if (rc1 != rc2 || errno1 != errno2 || flags1 != flags2) {
      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: failed due to flags/errno/len-compare on line %d",
              function, __LINE__);

      return 0;
   }

   /*
    * Test is to set a flag on fd1, and see if the same flag then gets set on
    * fd2.  Note that this flag must be a flag we can set on a socket that
    * failed during connect(2), or where the peer has closed it's side
    * of the session, and which will be shared between descriptors that are
    * dup(2)'s of each other.
    *
    * File status flags are shared, but descriptor flags (e.g., FD_CLOEXEC),
    * are of course not.  Also note that not all platforms let all F_SETFL
    * commands change the same flags, and not all platforms let us set
    * the flag on a "failed" socket (a socket where the connect(2) failed).
    * We however assume that if the socket failed, and we are getting the
    * same errno from the socket we are checking against, it is either the
    * socket, or any failed socket is as good as any other failed socket.
    *
    * XXX this does not work on OpenBSD if one of the descriptors were passed
    * us by another process (e.g., passed us by the connect child).
    * Need to sendbug this.
    *    On OpenBSD 4.5, if we have a process A, and that process sends
    *    a file descriptor to process B, and process B then send that
    *    same descriptor back to process A, the file status flags, at
    *    least O_NONBLOCK, is not shared.
    *    Thus if process A sends descriptor k to process B, and
    *    process B later sends that same descriptor back to process A,
    *    the descriptor B sends to A is a dup of k, and gets allocated
    *    a new index, e.g. k2.  We then expect that if we change the
    *    O_NONBLOCK flag on k2, it will be reflected on k, but the bug
    *    is that it is not.
    *
    * The reason we do not do this test first is that if there are multiple
    * processes/threads using the same fd, we want to minimize the chance
    * of us changing the descriptor under their feet while they are using it.
    */

   if (rc1 == -1 && rc2 == -1) {
      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG,
              "%s: succeeded due to getsockopt(2) failing (%s) on line %d",
              function, strerror(errno1), __LINE__);

      return 1; /* assume any failed socket is as good as any other failed. */
   }

   if (rc1 == -1 && errno1 == ENOTSOCK) {
      SWARNX(fd1); /* should not happen as we are only interested in sockets. */

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: failed due to errno = ENOTSOCK on line %d",
              function, __LINE__);

      return 0;
   }

   slog(LOG_DEBUG, "%s: all looks equal so far, doing final test, flags = %d",
        function, flags1);

   SASSERTX(flags1 == flags2);

   if (flags1)
      /*
       * remove testflag from fd1 and see if it gets removed from fd2 too.
       */
      setflag = 0;
   else
      /*
       * add testflag to fd1 and see if it gets added to fd2 too.
       */
      setflag = 1;

   if (setsockopt(fd1, SOL_SOCKET, testflag, &setflag, sizeof(setflag)) != 0) {
      if (setsockopt(fd2, SOL_SOCKET, testflag, &setflag, sizeof(setflag)) != 0)
      {
         slog(LOG_DEBUG, "%s: succeeded due to setsockopt() failing on line %d",
              function, __LINE__);

         return 1;
      }
      else {
         if (setsockopt(fd2, SOL_SOCKET, testflag, &flags2, sizeof(flags2))
         != 0)
            slog(LOG_DEBUG, "%s: could not restore original flags on fd %d: %s",
                 function, fd2, strerror(errno));

         slog(LOG_DEBUG, "%s: failed due to setsockopt() failing on line %d",
              function, __LINE__);

         return 0;
      }
   }

   len1   = sizeof(newflags1);
   rc1    = getsockopt(fd1, SOL_SOCKET, testflag, &newflags1, &len1);
   errno1 = errno;

   len2   = sizeof(newflags2);
   rc2    = getsockopt(fd2, SOL_SOCKET, testflag, &newflags2, &len2);
   errno2 = errno;

   if (newflags1 == newflags2) {
      slog(LOG_DEBUG, "%s: newflags1 = newflags2 -> %d is a dup of %d",
           function, fd1, fd2);

      isdup = 1;
   }
   else if (rc1 == -1 && rc2 == rc1
   &&       errno1 == errno2) {
      slog(LOG_DEBUG, "%s: flagcheck failed, but rc (%d) and errno (%d) is "
                      "the same, so assuming %d is a dup of %d, or that "
                      "any failed socket is as good as any other failed "
                      "socket.  Not many other choices",
                      function, rc1, errno1, fd1, fd2);
      isdup = 1;
   }
   else
      isdup = 0;

   /* restore flags back to original. */
   SASSERTX(flags1 == flags2);
   (void)setsockopt(fd1, SOL_SOCKET, testflag, &flags1, sizeof(flags1));
   (void)setsockopt(fd2, SOL_SOCKET, testflag, &flags2, sizeof(flags2));

   slog(LOG_DEBUG, "%s: final test indicates fd %d %s of fd %d",
        function, fd1, isdup ? "is a dup" : "is not a dup", fd2);

   return isdup;
}

int
makedummyfd(_safamily, _socktype)
   const sa_family_t _safamily;
   const int _socktype;
{
   const char *function = "makedummyfd()";
   struct sockaddr_storage addr;
   sa_family_t safamily;
   int socktype, s;

   if (_safamily == 0)
      safamily = AF_INET;
   else
      safamily = _safamily;

   if (_socktype == 0)
      socktype = SOCK_DGRAM;
   else
      socktype = _socktype;

   if ((s = socket(safamily, socktype, 0)) == -1) {
      swarn("%s: failed to create dummysocket of type %s, socktype %s",
            function, safamily2string(safamily), socktype2string(socktype));

      return -1;
   }

   if (socktype == SOCK_DGRAM)
      return s;

   /*
    * Else, need to do some more complex work, creating a genuine socket
    * that will not be marked ad readable or writable by select(2).
    */

   /*
    * The below bind(2) and listen(2) is necessary for Linux not to mark
    * the socket as readable/writable.  Under other UNIX systems, just a
    * socket(2) is enough.  Judging from the Open Unix spec., Linux is the
    * one that is correct though.
    */

   bzero(&addr, sizeof(addr));
   SET_SOCKADDR(&addr, safamily);

   if (safamily == AF_INET)
      TOIN(&addr)->sin_addr.s_addr = htonl(INADDR_ANY);
   else {
      SASSERTX(safamily == AF_INET6);
      memcpy(&TOIN6(&addr)->sin6_addr, &in6addr_any, sizeof(in6addr_any));
   }

   SET_SOCKADDRPORT(&addr, htons(0));

   if (socks_bind(s, TOSS(&addr), 0) != 0) {
      swarn("%s: could not bind address (%s)",
            function, sockaddr2string(&addr, NULL, 0));

      return s;
   }

   if (listen(s, 1) != 0) {
      swarn("%s: could not listen(2) on socket", function);
      return s;
   }

   return s;

}

#if SOCKS_CLIENT

int
fd_is_network_socket(fd)
   const int fd;
{
   struct stat statbuf;
   struct sockaddr_storage addr;
   socklen_t addrlen = sizeof(addr);

   if (fstat(fd, &statbuf) != 0)
      return 0;

   if (S_ISSOCK(statbuf.st_mode) == 0)
      return 0;


#if SOCKSLIBRARY_DYNAMIC
   /*
    * This function is used to decide whether to track a fd or not,
    * so be sure to not call ourselves again when figuring out
    * whether sys_getsockname() should track the fd or not.
    */

   if (sys_getsockname_notracking(fd, TOSA(&addr), &addrlen) != 0)
      return 0;

#else /* !SOCKSLIBRARY_DYNAMIC */

   if (getsockname(fd, TOSA(&addr), &addrlen) != 0)
      return 0;

#endif /* !SOCKSLIBRARY_DYNAMIC */

   switch (addr.ss_family) {
      case AF_INET:
      case AF_INET6:
         return 1;

      default:
         return 0;
   }
}

#endif /* SOCKS_CLIENT */
