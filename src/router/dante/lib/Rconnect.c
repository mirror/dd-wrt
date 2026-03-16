/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2005, 2008, 2009, 2010, 2011,
 *               2012, 2013, 2016, 2017
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

#include "upnp.h"

static const char rcsid[] =
"$Id: Rconnect.c,v 1.242.6.3.8.1 2024/11/21 10:22:42 michaels Exp $";

int
Rconnect(s, _name, namelen)
   int s;
   const struct sockaddr *_name;
   socklen_t namelen;
{
   const char *function = "Rconnect()";
   const int force_blockingconnect
   = socks_getenv(ENV_SOCKS_FORCE_BLOCKING_CONNECT, istrue) == NULL ? 0 : 1;
   struct sockaddr_storage name;
   socksfd_t socksfd;
   sockshost_t src, dst;
   authmethod_t auth;
   socks_t packet;
   socklen_t len;
   char namestr[MAXSOCKADDRSTRING], emsg[256];
   int type, rc, fd_is_nonblocking, savederrno;

   clientinit();

   if (_name == NULL) {
      rc = connect(s, _name, namelen);

      slog(LOG_DEBUG,
           "%s: dst is NULL, fallback to system connect(2) returned %d",
           function, rc);

      return rc;
   }

   if (_name->sa_family != AF_INET) {
      rc = connect(s, _name, namelen);

      slog(LOG_DEBUG,
           "%s: unsupported address family '%d' for dst %s, "
           "fallback to system connect(2) returned %d",
           function,
           _name->sa_family,
           sockaddr2string(TOCSS(_name), NULL, 0),
           rc);

      return rc;
   }

   usrsockaddrcpy(&name, TOCSS(_name), MIN(sizeof(name), namelen));

   if (socks_socketisforlan(s)) {
      slog(LOG_DEBUG, "%s: fd %d wanting to connect to %s is a lan-only "
                      "socket.  Falling back to system connect",
                      function,
                      s,
                      sockaddr2string(&name, namestr, sizeof(namestr)));

      return connect(s, _name, namelen);
   }


   slog(LOG_INFO, "%s: fd %d, address %s",
        function,
        s,
        sockaddr2string(&name, namestr, sizeof(namestr)));

   if (socks_addrisours(s, &socksfd, 1)) {
      slog(LOG_DEBUG, "%s: socket is a %s socket, err = %d, inprogress = %d",
                      function, proxyprotocol2string(socksfd.state.version),
                      socksfd.state.err, socksfd.state.inprogress);

      switch (socksfd.state.command) {
         case SOCKS_BIND:
            if (socksfd.state.protocol.tcp) {
               /*
                * Our guess; the client has succeeded to bind a specific
                * address and is now trying to connect out from it.
                * That also indicates the socks server is listening on a port
                * for this client.
                * Can't accept() on a connected socket so lets close the
                * connection to the server so it can stop listening on our
                * behalf, and we continue as if this was an ordinary connect().
                * Can only hope the server will use same port as we for
                * connecting out.
                *
                * Client might get problems if it has done a getsockname(2)
                * already, and thus thinks it knows it's local address,
                * as this Rconnect() will have to change it.
                */
               int tmp_s;

               slog(LOG_DEBUG,
                    "%s: continuing with Rconnect() after Rbind() on fd %d",
                    function, s);

               if (socksfd.state.version == PROXY_UPNP)
                  upnpcleanup(s);
               else {
                  /*
                   * socket must have connected to proxy before for Rbind().
                   * Need a new one.
                   */

                  if ((tmp_s = socketoptdup(s, -1)) == -1)
                     break;

                  if (dup2(tmp_s, s) == -1) {
                     close(tmp_s);
                     break;
                  }

                  close(tmp_s);
                  socks_rmaddr(s, 1);
               }
            }
            else if (socksfd.state.protocol.udp) {
               /*
                * Previously bound the udp socket, and now want to
                * connect out on the same socket.  In this case
                * we want to keep the port bound on the server, and
                * just add a connect to the peer, so let udpsetup() do
                * it's thing.
                */
            }
            else
               SERRX(0);

            break;

         case SOCKS_CONNECT:
            if (socksfd.state.version == PROXY_UPNP) {
               rc = connect(s, TOSA(&name), namelen);

               slog(LOG_DEBUG, "%s: connect(2) called again on upnp socket "
                               "returned %d, errno = %d (%s)",
                               function, rc, errno, strerror(errno));

               return rc;
            }

            if (socksfd.state.err != 0)
               errno = socksfd.state.err;
            else {
               if (socksfd.state.inprogress)
                  errno = EALREADY;
               else
                  errno = EISCONN;
            }

            return -1;

         case SOCKS_UDPASSOCIATE:
            /*
             * Trying to connect a udp socket (to a new address)?
             * Just continue as usual, udpsetup() will reuse existing
             * setup and we just assign the new ("connected") address.
             */
            break;

         default:
            SERRX(socksfd.state.command);
      }
   }
   else {
      slog(LOG_DEBUG,
           "%s: unknown fd %d.  Doing socks_rmaddr() before continuing ...",
           function, s);

      socks_rmaddr(s, 1);
   }

   len = sizeof(type);
   if (getsockopt(s, SOL_SOCKET, SO_TYPE, &type, &len) != 0) {
      swarn("%s: getsockopt(SO_TYPE)", function);
      return -1;
   }

   bzero(&packet, sizeof(packet));

   switch (type) {
      case SOCK_DGRAM:
         socksfd.route = udpsetup(s, &name, SOCKS_SEND, 1, emsg, sizeof(emsg));
         if (socksfd.route == NULL) {
            slog(LOG_INFO,
                 "%s: udpsetup() returned no route to %s for fd %d: %s",
                 function,
                 sockaddr2string(&name, NULL, 0),
                 s,
                 emsg);

            errno = ENETUNREACH;
            return -1;
         }

         slog(LOG_INFO, "%s: route set up for fd %d to %s is a %s route",
              function,
              s,
              sockaddr2string(&name, NULL, 0),
              proxyprotocols2string(&socksfd.route->gw.state.proxyprotocol,
                                    NULL,
                                    0));

          if (socksfd.route->gw.state.proxyprotocol.direct) {
             rc = connect(s, TOSA(&name), namelen);

             slog(LOG_DEBUG,
                  "%s: direct connect on fd %d to %s returned %ld: (%s)",
                  function,
                  s,
                  sockaddr2string(&name, NULL, 0),
                  (long)rc,
                  strerror(errno));

             return (int)rc;
          }
          else
             return 0;

      case SOCK_STREAM:
         packet.req.protocol = SOCKS_TCP;
         break;

      default:
         swarnx("%s: unknown protocol type %d, falling back to system connect",
                function, type);
         return connect(s, TOSA(&name), namelen);
   }

   bzero(&socksfd, sizeof(socksfd));
   len = sizeof(socksfd.local);
   if (getsockname(s, TOSA(&socksfd.local), &len) != 0)
      return -1;

   slog(LOG_DEBUG, "%s: local address of fd %d is %s",
        function, s, sockaddr2string(&socksfd.local, NULL, 0));

   bzero(&src, sizeof(src)); /* silence valgrind warning */
   src.atype     = SOCKS_ADDR_IPV4;
   src.addr.ipv4 = TOIN(&socksfd.local)->sin_addr;
   src.port      = TOIN(&socksfd.local)->sin_port;

   bzero(&dst, sizeof(dst)); /* silence valgrind warning */
   fakesockaddr2sockshost(&name, &dst);

   bzero(&auth, sizeof(auth));
   auth.method        = AUTHMETHOD_NOTSET;

   packet.req.version = PROXY_DIRECT;
   packet.req.command = SOCKS_CONNECT;
   packet.req.host    = dst;
   packet.req.auth    = &auth;

   if ((socksfd.route = socks_requestpolish(&packet.req, &src, &dst)) == NULL)
      return -1;

   if (socksfd.route->gw.state.proxyprotocol.direct)
      return connect(s, _name, namelen);

   if (socks_routesetup(s, s, socksfd.route, emsg, sizeof(emsg)) != 0) {
      swarnx("%s: socks_routesetup() failed: %s", function, emsg);
      return -1;
   }

   /* again in case routesetup() changed local address. */
   len = sizeof(socksfd.local);
   if (getsockname(s, TOSA(&socksfd.local), &len) != 0)
      return -1;

   slog(LOG_DEBUG, "%s: local address of fd %d is %s",
        function, s, sockaddr2string(&socksfd.local, NULL, 0));

   src.addr.ipv4 = TOIN(&socksfd.local)->sin_addr;
   src.port      = TOIN(&socksfd.local)->sin_port;

   slog(LOG_DEBUG, "%s: route prepared for fd %d is a %s route",
        function, s, proxyprotocol2string(packet.req.version));

   if (packet.req.version == PROXY_DIRECT) {
      rc = connect(s, TOSA(&name), namelen);

      slog(LOG_DEBUG, "%s: direct connect on fd %d to %s returned %d: (%s)",
           function,
           s,
           sockaddr2string(&name, NULL, 0),
           rc,
           strerror(errno));

      return rc;
   }

   switch (packet.version = packet.req.version) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V5:
      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
      case PROXY_UPNP:
         socksfd.control = s;
         break;

      default:
         SERRX(packet.req.version);
   }

   if (packet.version == PROXY_UPNP)
      /*
       * no negotiation to do before the connect, so we don't need
       * to care here whether the socket is blocking or not.
       */
      fd_is_nonblocking = 0;
   else
      /*
       * Check if the socket is non-blocking.  If so, fork a child
       * to negotiate with the proxy server and establish the connection.
       * In the case of UPNP, no negotiation is done, so don't waste
       * time on that.
       */
      fd_is_nonblocking = !fdisblocking(s);

   errno = 0;
   if (fd_is_nonblocking && !force_blockingconnect) {
      socksfd.route = socks_nbconnectroute(s,
                                           socksfd.control,
                                           &packet,
                                           &src,
                                           &dst,
                                           emsg,
                                           sizeof(emsg));

      if (socksfd.route == NULL)
         SASSERTX(errno != 0);
   }
   else
      socksfd.route = socks_connectroute(socksfd.control,
                                         &packet,
                                         &src,
                                         &dst,
                                         emsg,
                                         sizeof(emsg));

   slog(LOG_INFO, "%s: %s a %s route for fd %d, errno = %d",
        function,
        socksfd.route == NULL ?  "could not establish" : "established",
        proxyprotocol2string(packet.req.version),
        s,
        errno);

   if (socksfd.route == NULL) {
      if (s != socksfd.control)
         close(socksfd.control);

      switch (errno) {
         case EADDRINUSE: {
            /*
             * This problem can arise when we are socksifying
             * a server application that does several outbound
             * connections from the same address (e.g. ftpd) to the
             * same socks server.
             * It has by now successfully bound the address (it thinks)
             * and is not expecting this error.
             * Not sure what is best to do, just failing here prevents
             * ftpd from working for clients using the PORT command.
             *
             * For now, lets retry with a new socket.
             * This means the server no longer has bound the address
             * it (may) think it has of course, so not sure how smart this
             * really is.
             */
            int tmp_s;

            swarn("%s: server socksified?  trying to work around problem...",
                  function);

            if ((tmp_s = socketoptdup(s, -1)) == -1)
               break;
            if (dup2(tmp_s, s) == -1)
               break;
            close(tmp_s);

            /*
             * if s was bound to a privileged port, try to bind the new
             * s too to a privileged port.
             */
            /* LINTED pointer casts may be troublesome */
            if (PORTISRESERVED(TOIN(&socksfd.local)->sin_port)) {
               /* LINTED pointer casts may be troublesome */
               TOIN(&socksfd.local)->sin_port = htons(0);

               /* LINTED pointer casts may be troublesome */
               bindresvport(s, TOIN(&socksfd.local));
            }

            return Rconnect(s, TOSA(&name), namelen);
         }
      }

      swarnx("%s: could not connect route: %s", function, emsg);
      return -1;
   }

   if (fd_is_nonblocking && !force_blockingconnect) {
      if (!socks_addrisours(s, &socksfd, 1)) {
         swarn("%s: something went wrong when setting up non-blocking connect",
                function);

         return -1;
      }

      if (socksfd.state.inprogress) {
         slog(LOG_DEBUG, "%s: got route, non-blocking connect in progress",
                          function);

         errno = EINPROGRESS;
         return -1;
      }
      else {
         if (socksfd.state.err != 0) {
            slog(LOG_INFO,
                 "%s: non-blocking connect to %s failed with errno %d (%s)",
                 function,
                 sockaddr2string(&name, NULL, 0),
                 socksfd.state.err,
                 strerror(socksfd.state.err));

            errno = socksfd.state.err;
            return -1;
         }
         else {
            slog(LOG_INFO,
                 "%s: got route, non-blocking connect to %s finished ok",
                 function, sockaddr2string(&name, NULL, 0));

            return 0;
         }
      }
   }

   if (fd_is_nonblocking && force_blockingconnect)
      setblocking(s, "Rconnect(): forcing connect(2) to block");

   rc = socks_negotiate(s,
                        socksfd.control,
                        &packet,
                        socksfd.route,
                        emsg,
                        sizeof(emsg));

   if (fd_is_nonblocking && force_blockingconnect)
      setnonblocking(s, "Rconnect()");

   if (rc != 0) {
      swarnx("%s: socks_negotiate() failed: %s", function, emsg);

      return -1;
   }

   savederrno = errno;

   update_after_negotiate(&packet, &socksfd);

   slog(LOG_DEBUG,
        "%s: errno after successful socks_negotiate() is %d, version is %d",
        function, savederrno, socksfd.state.version);

   SASSERTX(type == SOCK_STREAM);
   socksfd.state.protocol.tcp    = 1;

   sockshost2sockaddr(&packet.res.host, &socksfd.remote);
   sockaddr2sockshost(&name, &socksfd.forus.connected);

   /* LINTED pointer casts may be troublesome */
   if (TOIN(&socksfd.local)->sin_port != htons(0)
   &&  TOIN(&socksfd.local)->sin_port != TOIN(&socksfd.remote)->sin_port) {
      /*
       * unfortunate; the client is trying to connect from a specific
       * port, a port it has successfully bound, but the port is currently
       * in use on the server side or the server doesn't care.
       */

      /* LINTED pointer casts may be troublesome */
      slog(LOG_DEBUG,
           "%s: wanted port %u, but got %u.  Continuing anyway",
           function,
           ntohs(TOIN(&socksfd.local)->sin_port),
           ntohs(TOIN(&socksfd.remote)->sin_port));
   }

   len = sizeof(socksfd.server);
   if (getpeername(s, TOSA(&socksfd.server), &len) == 0)
      slog(LOG_DEBUG, "%s: getpeername(fd %d) = %s",
           function, s, sockaddr2string(&socksfd.server, NULL, 0));
   else
      slog(LOG_DEBUG, "%s: getpeername(fd %d): %s",
           function, s, strerror(errno));

   len = sizeof(socksfd.local);
   if (getsockname(s, TOSA(&socksfd.local), &len) != 0)
      slog(LOG_DEBUG, "%s: getsockname(s): %s", function, strerror(errno));

   socks_addaddr(s, &socksfd, 1);

   sockaddr2sockshost(&name, &sockscf.state.lastconnect); /* for socks bind. */

   slog(LOG_DEBUG, "%s: returning.  Current errno is %d (%s), saved is %d",
        function, errno, strerror(errno), savederrno);

   errno = savederrno;

   if (errno != 0)
      return -1;

   return 0;
}
