/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2004, 2005, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2016, 2019, 2020, 2021
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
"$Id: udp.c,v 1.289.6.3.4.6 2021/02/02 19:34:11 karls Exp $";

/* ARGSUSED */
ssize_t
Rsendto(s, msg, len, flags, _to, tolen)
   int s;
   const void *msg;
   size_t len;
   int flags;
   const struct sockaddr *_to;
   socklen_t tolen;
{
   const char *function = "Rsendto()";
   socksfd_t socksfd;
   sockshost_t tohost;
   struct sockaddr_storage tomem, *to;
   size_t nlen;
   socklen_t typelen;
   ssize_t n;
   char srcstr[MAXSOCKADDRSTRING], dststr[sizeof(srcstr)], nmsg[SOCKD_BUFSIZE];
   int type;

   clientinit();

   if (_to == NULL)
      to = NULL;
   else {
      to = &tomem;
      usrsockaddrcpy(to, TOCSS(_to), salen(_to->sa_family));
   }

   slog(LOG_DEBUG, "%s: fd %d, len %lu (%s ...), address %s",
        function,
        s,
        (long unsigned)len,
        str2vis(msg, len, nmsg, MIN(len, MIN(32, sizeof(nmsg)))),
        to == NULL ? "NULL" : sockaddr2string(to, NULL, 0));

   if (to != NULL && to->ss_family != AF_INET) {
      slog(LOG_DEBUG, "%s: unsupported address family '%d', system fallback",
           function, to->ss_family);

      return sendto(s, msg, len, flags, TOCSA(to), tolen);
   }

   typelen = sizeof(type);
   if (getsockopt(s, SOL_SOCKET, SO_TYPE, &type, &typelen) != 0) {
      swarn("%s: getsockopt(SO_TYPE)", function);
      return -1;
   }

   if (type != SOCK_DGRAM && type != SOCK_STREAM) {
      n = sendto(s, msg, len, flags, TOCSA(to), tolen);

      slog(LOG_DEBUG,
           "%s: fd %d is neither SOCK_STREAM nor SOCK_DGRAM.  "
           "Direct systemcall returned %ld",
           function, s, (long)n);

      return n;
   }

   if (type == SOCK_DGRAM) {
      char emsg[256];

      socksfd.route = udpsetup(s, to, SOCKS_SEND, 0, emsg, sizeof(emsg));
      if (socksfd.route == NULL) {
         if (to == NULL) {
            /*
             * Since the caller has managed to connect the socket by himself,
             * assume it's a socket we should not proxy.
             */
            n = sendto(s, msg, len, flags, TOCSA(to), tolen);

            slog(LOG_DEBUG,
                 "%s: no route returned by udpsetup() for fd %d, and to is "
                 "NULL.  Direct fallback to sendto(2) returned %ld (%s)",
                 function, s, (long)n, strerror(errno));

            return n;

         }
         else {
            slog(LOG_DEBUG,
                  "%s: no route by udpsetup() for fd %d to %s (%s).  "
                  "Returning -1",
                  function, s, sockaddr2string(to, NULL, 0), emsg);

            errno = ENETUNREACH;
            return -1;
         }
      }
      else {
         slog(LOG_DEBUG,
              "%s: route returned by udpsetup() for fd %d is a %s route",
              function,
              s,
              proxyprotocols2string(&socksfd.route->gw.state.proxyprotocol,
                                    NULL,
                                    0));

         if (socksfd.route->gw.state.proxyprotocol.direct)
            return sendto(s, msg, len, flags, TOSA(to), tolen);

         if (!socks_addrisours(s, &socksfd, 1))
            SERRX(s);
      }
   }

   if (!socks_addrisours(s, &socksfd, 1)) {
      slog(LOG_DEBUG, "%s: unknown fd %d, going direct", function, s);

      return sendto(s, msg, len, flags, TOSA(to), tolen);
   }

   if (socksfd.state.err != 0) {
      slog(LOG_DEBUG, "%s: session on fd %d already failed with errno %d",
           function, s, socksfd.state.err);

      errno = socksfd.state.err;
      return -1;
   }

   if (socksfd.state.issyscall
   ||  socksfd.state.version == PROXY_DIRECT
   ||  socksfd.state.version == PROXY_UPNP) {
      n = sendto(s, msg, len, flags, TOSA(to), tolen);

      slog(LOG_DEBUG, "%s: sendto(2) to %s on fd %d returned %ld (%s)",
           function,
           to == NULL ?
               "NULL" : sockaddr2string(to, NULL, 0),
           s,
           (long)n,
           strerror(errno));

      return n;
   }

   if (type == SOCK_STREAM) {
      if (socksfd.state.inprogress) {
         SASSERTX(socksfd.state.command == SOCKS_CONNECT);

         slog(LOG_INFO,
              "%s: write attempted on connect still in progress: fd %d",
              function, s);

         /*
          * Either the user is 1) using this system call to figure out
          * whether the connection completed, before continuing with other
          * things if not, or 2) our attempt to hide our usage of the
          * user's fd to set up the socks session (without the user getting
          * any indication that his fd is being written to/read from)
          * via select(2)/poll(2)/etc. failed.
          *
          * In case of 1), the correct thing would be to return ENOTCONN,
          * but in case 2), we could be called due to the the user having
          * multiple fd's pointing to the same filedescription index,
          * meaning that even though we have hidden our usage of "s", the
          * user is using another fd (s').  Normally we would of course be
          * called with s' then, but if the user is using e.g. epoll(2),
          * our dup(2)ing s to temporary dummy-fd does apparently not
          * change what the fd used by epoll(2) points to.  Not verified,
          * but one possible explanation for a problem seen would be that
          * adding a fd to epoll(2), and then dup2(2)'ing that fd to
          * something else (but with the same fd-index/number) does not
          * change what the fd used by epoll points to; epoll(2) continues
          * to use what the fd pointed to before, at least if what it
          * pointed to before is open.  Is there a way to avoid this
          * problem?
          *
          * So what do we do?  We don't know whether it's 1) or 2)
          * happening.  If it's 2), returning ENOTCONN can be taken as
          * an indication that the connect(2) failed, which it has
          * not (yet, at least) done.  If we return EAGAIN, the
          * user will hopefully retry again, whenever the systemcall
          * he used to detect that the fd was readable say it's readable.
          * If the connect is still in progress, we again assume the
          * readability was only related to i/o done by our connect-child
          * over the fd, and was not intended for the user, and again
          * return EAGAIN.
          *
          * If the i/o length attempted is 0, it seems relatively safe
          * to assume the user just wants to test whether the connect
          * completed though.
          */

         if (tolen == 0)
            errno = ENOTCONN;
         else
            errno = EAGAIN;

         return -1;
      }

      n = socks_sendto(s,
                       msg,
                       len,
                       flags,
                       to,
                       tolen,
                       NULL,
                       &socksfd.state.auth);

      slog(LOG_DEBUG, "%s: %s: %s: %s -> %s (%ld)",
           function,
           proxyprotocol2string(socksfd.state.version),
           protocol2string(SOCKS_TCP),
           sockaddr2string(&socksfd.local,
                           dststr,
                           sizeof(dststr)),
           sockaddr2string(&socksfd.server,
                           srcstr,
                           sizeof(srcstr)),
           (long)n);

      /* in case something changed, e.g. gssoverhead. */
      (void)socks_addaddr(s, &socksfd, 1);

      return n;
   }

   SASSERTX(type == SOCK_DGRAM);

   if (to == NULL) {
      if (socksfd.state.udpconnect)
         tohost = socksfd.forus.connected;
      else {
         swarnx("%s: called with destination address NULL, but fd %d is not "
                "connected via us, so we don't know what the intended "
                "destination is",
                function, s);

         errno = EDESTADDRREQ;
         return -1;
      }
   }
   else
      fakesockaddr2sockshost(to, &tohost);

   /*
    * need to prefix a socks udp header to the message.  Copy the original
    * payload into nmsg, which should have room to prefix the socks
    * udpheader, and then send it.
    */
   memcpy(nmsg, msg, len);
   nlen = len;
   if (udpheader_add(&tohost, nmsg, &nlen, sizeof(nmsg)) == NULL)
      return -1;

   n = socks_sendto(s,
                    nmsg,
                    nlen,
                    flags,
                    socksfd.state.udpconnect ? NULL : &socksfd.reply,
                    socksfd.state.udpconnect ?
                        (socklen_t)0 : salen(socksfd.reply.ss_family),
                    NULL,
                    &socksfd.state.auth);

   n -= (ssize_t)(nlen - len);

   slog(LOG_DEBUG,
        "%s: %s: %s: %s -> %s -> %s (%ld)",
        function,
        proxyprotocol2string(socksfd.state.version),
        protocol2string(SOCKS_UDP),
        sockaddr2string(&socksfd.local, dststr, sizeof(dststr)),
        sockaddr2string(&socksfd.reply, srcstr, sizeof(srcstr)),
        sockshost2string(&tohost, NULL, 0),
        (long)n);

   /* in case something changed, e.g. gssoverhead. */
   (void)socks_addaddr(s, &socksfd, 1);

   return MAX(-1, n);
}

ssize_t
Rrecvfrom(s, buf, len, flags, from, fromlen)
   int s;
   void *buf;
   size_t len;
   int flags;
   struct sockaddr *from;
   socklen_t *fromlen;
{
   const char *function = "Rrecvfrom()";
   socksfd_t socksfd;
   udpheader_t header;
   struct sockaddr_storage newfrom;
   socklen_t typelen, newfromlen;
   char srcstr[MAXSOCKSHOSTSTRING], dststr[sizeof(srcstr)], *newbuf;
   size_t payloadoffset, newlen;
   ssize_t n;
   int type, isfromproxy;

again:

   slog(LOG_DEBUG, "%s: fd %d, len %lu", function, s, (long unsigned)len);

   slog(LOG_DEBUG, "%s: fd %d, len %lu (%s ...)",
        function,
        s,
        (long unsigned)len,
        str2vis(buf, len, srcstr, MIN(len, MIN(32, sizeof(srcstr)))));

   typelen = sizeof(type);
   if (getsockopt(s, SOL_SOCKET, SO_TYPE, &type, &typelen) != 0) {
      swarn("%s: getsockopt(SO_TYPE)", function);
      return -1;
   }

   if (type != SOCK_DGRAM && type != SOCK_STREAM) {
      n = recvfrom(s, buf, len, flags, from, fromlen);

      slog(LOG_DEBUG,
           "%s: fd %d is neither SOCK_STREAM nor SOCK_DGRAM.  "
           "Direct systemcall returned %ld",
           function, s, (long)n);

      return n;
   }

   if (socks_addrisours(s, &socksfd, 1)) {
      if (socksfd.state.err != 0) {
         slog(LOG_DEBUG, "%s: session on fd %d already failed with errno %d",
              function, s, socksfd.state.err);

         errno = socksfd.state.err;
         return -1;
      }

      if (socksfd.state.issyscall
      ||  socksfd.state.version == PROXY_DIRECT
      ||  socksfd.state.version == PROXY_UPNP) {
         n = recvfrom(s, buf, len, flags, from, fromlen);

         slog(LOG_DEBUG, "%s: recvfrom(2) on fd %d returned %ld (%s)",
              function, s, (long)n, strerror(errno));

         return n;
      }
   }
   else {
      socks_rmaddr(s, 1);

      if (type != SOCK_DGRAM) /* nothing we can do with this one. */
         return recvfrom(s, buf, len, flags, from, fromlen);

      bzero(&socksfd, sizeof(socksfd));
   }

   if (type == SOCK_DGRAM) {
      char emsg[256];

      socksfd.route
      = udpsetup(s, TOSS(from), SOCKS_RECV, 0, emsg, sizeof(emsg));

      if (socksfd.route == NULL) {
         slog(LOG_DEBUG,
              "%s: no route found by udpsetup() for fd %d: %s.  Doing direct "
              "fallback",
              function, s, emsg);

         return recvfrom(s, buf, len, flags, from, fromlen);
      }
      else {
         slog(LOG_DEBUG,
              "%s: route returned by udpsetup() for fd %d is a %s route",
              function,
              s,
              proxyprotocols2string(&socksfd.route->gw.state.proxyprotocol,
                                    NULL,
                                    0));

         if (socksfd.route->gw.state.proxyprotocol.direct)
            return recvfrom(s, buf, len, flags, from, fromlen);

         if (!socks_addrisours(s, &socksfd, 1))
            SERRX(s);
      }

      SASSERTX(socks_addrisours(s, &socksfd, 1));
   }


   /* XXX split this up into socks_tcp_recvfrom() and socks_udp_recvfrom(). */

   if (socksfd.state.protocol.tcp) {
      const sockshost_t *forus;

      SASSERTX(type == SOCK_STREAM);

      if (socksfd.state.inprogress) {
         SASSERTX(socksfd.state.command == SOCKS_CONNECT);

         slog(LOG_INFO,
              "%s: read attempted on connect still in progress: fd %d",
              function, s);

         /*
          * See comment for same case in Rsendto().
          */
         if (fromlen == 0)
            errno = ENOTCONN;
         else
            errno = EAGAIN;

         return -1;
      }

      n = socks_recvfromn(s,
                          buf,
                          len,
                          0,
                          flags,
                          TOSS(from),
                          fromlen,
                          NULL,
                          &socksfd.state.auth);

      switch (socksfd.state.command) {
         case SOCKS_CONNECT:
            forus = &socksfd.forus.connected;
            break;

         case SOCKS_BIND:
            forus = &socksfd.forus.accepted;

            if (forus->atype == SOCKS_ADDR_NOTSET) {
               slog(LOG_DEBUG, "%s: trying to read from fd %d, which is "
                               "for bind, but no bind-reply handled yet ...",
                               function, s);

               forus = NULL;
               n     = -1;
               errno = ENOTCONN;
            }
            break;

         default:
            SERRX(socksfd.state.command);
      }

      slog(LOG_DEBUG, "%s: %s: %s: %s -> %s (%ld)",
           function,
           proxyprotocol2string(socksfd.state.version),
           protocol2string(SOCKS_TCP),
           forus == NULL ?
               "<NULL>" : sockshost2string(forus, srcstr, sizeof(srcstr)),
           sockaddr2string(&socksfd.local, dststr, sizeof(dststr)),
           (long)n);

      /* in case something changed, e.g. gssoverhead. */
      (void)socks_addaddr(s, &socksfd, 1);

      return n;
   }

   SASSERTX(socksfd.state.protocol.udp);

   /*
    * udp.  If packet is from socks server it will be prefixed with a header,
    * so make sure we have room for it.
    */
   newlen = len + sizeof(header);
   if ((newbuf = malloc(sizeof(*newbuf) * newlen)) == NULL) {
      errno = ENOBUFS;
      return -1;
   }

   newfromlen = sizeof(newfrom);
   if ((n = socks_recvfrom(s,
                           newbuf,
                           newlen,
                           flags,
                           &newfrom,
                           &newfromlen,
                           NULL,
                           &socksfd.state.auth)) == -1) {
      free(newbuf);
      return n;
   }

   SASSERTX(newfromlen > 0);

   if (sockaddrareeq(&newfrom, &socksfd.reply, 0)) {
      isfromproxy = 1;

      if (string2udpheader(newbuf, (size_t)n, &header) == NULL) {
         swarnx("%s: unrecognized socks udp packet from %s",
                function, sockaddr2string(&newfrom, NULL, 0));

         free(newbuf);

         errno = EAGAIN;
         return -1;
      }

      slog(LOG_DEBUG, "%s: proxy server at %s says udp packet is from %s",
           function,
           sockaddr2string(&newfrom, NULL, 0),
           sockshost2string(&header.host, NULL, 0));

      /* replace "newfrom" with the address socks server says packet is from. */
      fakesockshost2sockaddr(&header.host, &newfrom);

      /* callee doesn't want socks header. */
      n -= (ssize_t)HEADERSIZE_UDP(&header);
      payloadoffset = HEADERSIZE_UDP(&header);
   }
   else {
      isfromproxy = 0;

      slog(LOG_DEBUG, "%s: packet is from %s, not from the proxy server (%s)",
           function,
           sockaddr2string(&newfrom, srcstr, sizeof(srcstr)),
           sockaddr2string(&socksfd.reply, dststr, sizeof(dststr)));

      payloadoffset = 0;
   }

   SASSERTX(n >= 0);

   if (socksfd.state.udpconnect) {
      /*
       * Need to filter out packets that are not from the address the
       * client connected to.
       */
      int dropit = 0;

      if (isfromproxy) {
         /*
          * XXX
          * only supported for ip-addresses at the moment.  Need to decide
          * how the server should treat hostnames with regards to replies
          * before hostnames can be supported.
          */
         if (socksfd.forus.connected.atype == SOCKS_ADDR_IPV4
         && !sockshostareeq(&header.host, &socksfd.forus.connected)) {
            slog(LOG_INFO,
                 "%s: client connected to address %s via proxy, but proxy "
                 "says this packet is from %s.  Dropping it",
                 function,
                 sockshost2string(&socksfd.forus.connected,
                                  dststr,
                                  sizeof(dststr)),
                 sockshost2string(&header.host, srcstr, sizeof(srcstr)));

            dropit = 1;
         }
      }
      else {
         slog(LOG_INFO,
              "%s: client connected to address %s via proxy server at %s, but "
              "this packet not from the proxy, but from from %s.  Dropping it",
              function,
              sockshost2string(&socksfd.forus.connected, NULL, 0),
              sockaddr2string(&socksfd.reply,
                              srcstr,
                              sizeof(srcstr)),
              sockaddr2string(&newfrom,
                              dststr,
                              sizeof(dststr)));
         dropit = 1;
      }

      if (dropit) {
         free(newbuf);

         if (fdisblocking(s)) {
            slog(LOG_DEBUG,
                 "%s: fd %d is blocking but we have no packet to return our "
                 "client.  Going round again",
                 function, s);

            goto again;
         }

         errno = EAGAIN;
         return -1;
      }
   }

   memcpy(buf, &newbuf[payloadoffset], MIN(len, (size_t)n));
   free(newbuf);

   slog(LOG_DEBUG, "%s: %s: %s: %s -> %s%s%s (%ld)",
        function,
        proxyprotocol2string(socksfd.state.version),
        protocol2string(SOCKS_UDP),
        sockaddr2string(&newfrom, srcstr, sizeof(srcstr)),
        isfromproxy ?
            sockaddr2string(&socksfd.reply, NULL, 0) : "",
        isfromproxy ? " -> " : "",
        sockaddr2string(&socksfd.local, dststr, sizeof(dststr)),
        (long)(n));

   if (from != NULL) {
      *fromlen = MIN(*fromlen, newfromlen);
      sockaddrcpy(TOSS(from), &newfrom, (size_t)*fromlen);
   }

   /* in case something changed, e.g. gssoverhead. */
   (void)socks_addaddr(s, &socksfd, 1);

   return MIN(len, (size_t)n);
}

route_t *
udpsetup(s, to, type, shouldconnect, emsg, emsglen)
   int s;
   const struct sockaddr_storage *to;
   int type;
   int shouldconnect;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "udpsetup()";
   static route_t directroute;
   socksfd_t socksfd;
   authmethod_t auth;
   socks_t packet;
   sockshost_t src, dst;
   struct sockaddr_storage addr;
   socklen_t len;

   slog(LOG_DEBUG, "%s: fd %d, type = %s, to = %s, shouldconnect = %d",
         function,
         s,
         type == SOCKS_RECV ? "receive" : "send",
         (to == NULL || type == SOCKS_RECV) ?
            "N/A" : sockaddr2string(to, NULL, 0),
         shouldconnect);

   errno = 0;

   /*
    * don't bother setting it fully up, not expecting anybody to access
    * any other fields if direct is set.
    */
   directroute.gw.state.proxyprotocol.direct = 1;

   bzero(&socksfd, sizeof(socksfd));
   len = sizeof(addr);
   if (getsockname(s, TOSA(&addr), &len) != 0) {
      snprintf(emsg, emsglen, "getsockname(s) failed: %s", strerror(errno));
      return NULL;
   }
   else
      slog(LOG_DEBUG, "%s: local address of fd %d is %s",
           function, s, sockaddr2string(&addr, NULL, 0));

   switch (TOSA(&addr)->sa_family) {
      case AF_INET:
         break;

      default:
         snprintf(emsg, emsglen, "unsupported af %d", TOSA(&addr)->sa_family);
         return NULL;
   }

   if (socks_addrisours(s, &socksfd, 1)) {
      if (socksfd.state.command == SOCKS_UDPASSOCIATE) {
         slog(LOG_DEBUG, "%s: things already set up for fd %d", function, s);
         return socksfd.route;
      }
      else
         slog(LOG_DEBUG, "%s: socket was previously used for command %s",
              function, command2string(socksfd.state.command));
   }

   socks_rmaddr(s, 1);

   if (socks_socketisforlan(s)) {
      slog(LOG_INFO, "%s: fd %d is for lan only", function, s);
      return &directroute;
   }

   bzero(&socksfd, sizeof(socksfd));
   socksfd.control = -1;
   socksfd.local   = addr;

   switch (type) {
      case SOCKS_RECV:
         /*
          * Either a socket for which the route has previously been determined
          * to be direct, in which case we are not bothering to keep track
          * off the fd, or something more problematic; trying to receive on
          * socket not sent on.
          *
          * Only UPnP supports the latter, and in that case, the socket
          * should already have been bound, so socks_addrisours()
          * should have been true.  Nothing we can do in other cases.
          */
         snprintf(emsg, emsglen,
                  "%s: attempted receive on unregistered fd %d", function, s);

         return NULL;

      case SOCKS_SEND:
         if (to == NULL) {
            /*
             * no address and unknown socket.  Has a connect(2) been done
             * on the socket, but for some reason not been caught by us?
             */
            socklen_t addrlen = sizeof(addr);
            if (getpeername(s, TOSA(&addr), &addrlen) == 0) {
               int val;

               len = sizeof(val);
               if (getsockopt(s, SOL_SOCKET, SO_TYPE, &val, &len) != 0) {
                  snprintf(emsg, emsglen, "getsockopt(SO_TYPE) failed: %s",
                           strerror(errno));

                  return NULL;
               }

               switch (val) {
                  case SOCK_DGRAM:
                     slog(LOG_INFO,
                          "%s: fd %d is unregistered, but has a datagram peer: "
                          "%s.  Trying to accommodate ... ",
                          function,
                          s,
                          sockaddr2string(&addr, NULL, 0));

                     break;

                  case SOCK_STREAM:
                     snprintf(emsg, emsglen,
                              "fd %d is unregistered, but has a stream peer "
                              "(%s) already; nothing to do",
                              s,
                              sockaddr2string(&addr, NULL, 0));
                     return NULL;

                  default:
                     return &directroute;
               }

               to            = &addr;
               shouldconnect = 1;
            }
            else {
               snprintf(emsg, emsglen, "unknown fd %d and no to-addr", s);
               return NULL;
            }
         }
         break;

      default:
         SERRX(type);
   }

   sockaddr2sockshost(&socksfd.local, &src);
   fakesockaddr2sockshost(to, &dst);

   bzero(&auth, sizeof(auth));
   auth.method          = AUTHMETHOD_NOTSET;

   bzero(&packet, sizeof(packet));
   packet.version       = PROXY_DIRECT;;
   packet.req.version   = packet.version;
   packet.req.command   = SOCKS_UDPASSOCIATE;

#if 0 /*
       * some (nec-based) socks-servers misinterpret this to mean something
       * completely different than what the draft says.
       */
   packet.req.flag     |= SOCKS_USECLIENTPORT;
#endif

   packet.req.host      = src;
   packet.req.protocol  = SOCKS_UDP;
   packet.req.auth      = &auth;

   if ((socksfd.route = socks_requestpolish(&packet.req, &src, &dst)) == NULL) {
      char srcstr[MAXSOCKSHOSTSTRING], dststr[sizeof(srcstr)];

      snprintf(emsg, emsglen, "no route from %s to %s found",
               sockshost2string(&src, srcstr, sizeof(srcstr)),
               sockshost2string(&dst, dststr, sizeof(dststr)));

      return NULL;
   }

   if (socksfd.route->gw.state.proxyprotocol.direct) {
      slog(LOG_DEBUG, "%s: direct system calls for fd %d", function, s);

      directroute = *socksfd.route;
      return &directroute;
   }

   /* only ones we support udp via. */
   switch (packet.version = packet.req.version) {
      case PROXY_SOCKS_V5:
      case PROXY_UPNP:
         if ((socksfd.control = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            snprintf(emsg, emsglen,
                     "failed to create control socket: %s", strerror(errno));
            return NULL;
         }

         slog(LOG_DEBUG, "%s: control fd %d created for data fd %d",
              function, socksfd.control, s);
         break;

      case PROXY_DIRECT:
         break;

      default:
         SERRX(packet.version);
   }

   if (socks_routesetup(socksfd.control, s, socksfd.route, emsg, emsglen) != 0){
      swarnx("%s: socks_routesetup() failed: %s", function, emsg);

      if (socksfd.control != -1)
         close(socksfd.control);

      return NULL;
   }

   /*
    * routesetup may have changed local address due to redirect statement.
    */
   len = sizeof(socksfd.local);
   if (getsockname(s, TOSA(&socksfd.local), &len) != 0) {
      snprintf(emsg, emsglen, "getsockname(s) failed: %s", strerror(errno));
      return NULL;
   }

   slog(LOG_DEBUG, "%s: local address of fd %d (data-fd) is %s",
        function, s, sockaddr2string(&socksfd.local, NULL, 0));

   sockaddr2sockshost(&socksfd.local, &src);

   socksfd.route = socks_connectroute(socksfd.control,
                                      &packet,
                                      &src,
                                      &dst,
                                      emsg,
                                      emsglen);

   if (socksfd.route == NULL) {
      swarnx("could not connect route: %s", emsg);

      close(socksfd.control);
      return NULL;
   }

   if (socksfd.route->gw.state.proxyprotocol.direct)
      return socksfd.route;

   /*
    * we need to send the socks server our address.
    * First check if the socket already has a name, if so
    * use that, otherwise assign the name ourselves before informing the
    * socks server.
    */
   if (PORTISBOUND(&socksfd.local))
      slog(LOG_DEBUG, "%s: fd %d already bound to %s, using that",
           function, s, sockaddr2string(&socksfd.local, NULL, 0));
   else {
      /*
       * local addr not fixed, so set it so we can tell the socks-server.
       */

      /*
       * don't have much of an idea on what IP address to use so
       * use the same address as the tcp connection to socks server uses.
       */
      len = sizeof(socksfd.local);
      if (getsockname(socksfd.control, TOSA(&socksfd.local), &len) != 0) {
         snprintf(emsg, emsglen, "getsockname(socksfd.control) failed: %s",
                  strerror(errno));

         close(socksfd.control);
         return NULL;
      }
      SET_SOCKADDRPORT(&socksfd.local, htons(0));

      if (bind(s, TOSA(&socksfd.local), salen(socksfd.local.ss_family)) != 0) {
         snprintf(emsg, emsglen, "bind() of fd %d (s) to address %s failed: %s",
                  s,
                  sockaddr2string(&socksfd.local, NULL, 0),
                  strerror(errno));

         close(socksfd.control);
         return NULL;
      }

      if (getsockname(s, TOSA(&socksfd.local), &len) != 0) {
         snprintf(emsg, emsglen, "getsockname() on fd %d (s) failed: %s",
                  s, strerror(errno));

         close(socksfd.control);
         return NULL;
      }
   }

   sockaddr2sockshost(&socksfd.local, &packet.req.host);

   if (socks_negotiate(s,
                       socksfd.control,
                       &packet,
                       socksfd.route,
                       emsg,
                       emsglen) != 0) {
      close(socksfd.control);

      swarnx("%s: socks_negotiate() failed: %s", function, emsg);
      return NULL;
   }

   update_after_negotiate(&packet, &socksfd);
   socksfd.state.protocol.udp = 1;

   if (socksfd.state.version == PROXY_UPNP)
      sockshost2sockaddr(&packet.res.host, &socksfd.remote);
   else {
      sockshost2sockaddr(&packet.res.host, &socksfd.reply);

      len = sizeof(socksfd.server);
      if (getpeername(socksfd.control, TOSA(&socksfd.server), &len) != 0) {
         snprintf(emsg, emsglen,
                  "getpeername() on fd %d (socksfd.control) failed: %s",
                  socksfd.control, strerror(errno));

         close(socksfd.control);
         return NULL;
      }
   }

   if (shouldconnect) {
      char lstr[MAXSOCKADDRSTRING], pstr[sizeof(lstr)];
      int rc;

      socksfd.state.udpconnect = 1;

      switch (socksfd.state.version) {
         case PROXY_SOCKS_V5:
            fakesockaddr2sockshost(to, &socksfd.forus.connected);

            rc = connect(s,
                         TOSA(&socksfd.reply),
                         salen(socksfd.reply.ss_family));

            snprintf(emsg, emsglen,
                     "connecting fd %d from %s to %s-server %s %s: %s",
                     s,
                     sockaddr2string(&socksfd.local,
                                     lstr,
                                     sizeof(lstr)),
                     proxyprotocol2string(socksfd.state.version),
                     sockaddr2string(&socksfd.reply,
                                     pstr,
                                     sizeof(pstr)),
                     rc == 0 ? "succeeded" : "failed",
                     strerror(errno));

            slog(rc == 0 ? LOG_INFO : LOG_WARNING, "%s: %s", function, emsg);

            if (rc != 0) {
               close(socksfd.control);
               return NULL;
            }

            break;

         case PROXY_UPNP:
            rc = connect(s, TOCSA(to), salen(to->ss_family));
            snprintf(emsg, emsglen,
                     "connecting fd %d from %s to %s %s: %s",
                     s,
                     sockaddr2string(&socksfd.local,
                                     lstr,
                                     sizeof(lstr)),
                     sockaddr2string(to, pstr, sizeof(pstr)),
                     rc == 0 ? "succeeded" : "failed",
                     strerror(errno));


            slog(rc == 0 ? LOG_INFO : LOG_WARNING, "%s: %s", function, emsg);

            if (rc != 0)
               return NULL;

            break;

         default:
            SERRX(socksfd.state.version);
      }

   }

   if (socksfd.state.version == PROXY_UPNP) {
      /*
       * is a one-time thing, nothing more expected on the control socket
       * and no need to keep it open any longer.
       */
      close(socksfd.control);
      socksfd.control = s;
   }

   if (socks_addaddr(s, &socksfd, 1) == NULL) {
      snprintf(emsg, emsglen, "socks_addaddr() failed: %s", strerror(errno));

      close(socksfd.control);
      return NULL;
   }

   return socksfd.route;
}
