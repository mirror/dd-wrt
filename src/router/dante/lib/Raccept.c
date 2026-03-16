/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2004, 2005, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2014, 2019, 2024
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
"$Id: Raccept.c,v 1.159.4.3.6.2.4.2 2024/11/20 22:03:26 karls Exp $";

static int
addforwarded(const int local, const int remote,
             const struct sockaddr_storage *remoteaddr,
             const sockshost_t *virtualremoteaddr);
/*
 * Adds a proxy-forwarded remote client to our list over proxied clients.
 * "local" gives the local socket we listen on,
 * "remote" is the socket connected to the remote client,
 * "remoteaddr" is the physical peer of "remote" (the proxy server),
 * and "virtualremoteaddr" is the address the proxy claims to be
 * forwarding.
 *
 * Returns 0 if the remote client was successfully added, or -1 if not.
 */
int
Raccept(s, addr, addrlen)
   int s;
   struct sockaddr *addr;
   socklen_t *addrlen;
{
   const char *function = "Raccept()";
   socklen_t addrlen_mem;
   fd_set *rset;
   socksfd_t socksfd;
   char emsg[256];
   struct sockaddr_storage accepted;
   socks_t packet;
   int fdbits, p, remote;

   clientinit();

   slog(LOG_DEBUG, "%s, fd %d, addrlen %lu",
        function, s, (unsigned long)(addrlen == NULL ? 0 : *addrlen));

   if (addrlen == NULL) {
      addrlen  = &addrlen_mem;
      *addrlen = 0;
   }

   /* can't call Raccept() on unknown descriptors. */
   if (!socks_addrisours(s, &socksfd, 1)
   ||  socksfd.state.command != SOCKS_BIND) {
      struct sockaddr_storage ss;

      p = accept(s, addr, addrlen);

      if (addr != NULL && *addrlen >= sizeof(struct sockaddr_in))
         usrsockaddrcpy(&ss, TOSS(addr), sizeof(ss));

      slog(LOG_DEBUG,
           "%s: fd %d is unregistered, accept(2) returned fd %d (%s): %s",
           function,
           s,
           p,
           p >= 0
           && addr    != NULL
           && *addrlen >= sizeof(struct sockaddr_in) ?
               sockaddr2string(&ss, NULL, 0) : "N/A",
           strerror(errno));

      socks_rmaddr(s, 1);

      return p;
   }

   slog(LOG_DEBUG, "%s: fd %d is setup for proxyprotocol %s",
        function, s, proxyprotocol2string(socksfd.state.version));

   bzero(&packet, sizeof(packet));
   packet.version = (unsigned char)socksfd.state.version;

   SASSERTX(s == socksfd.control);

   switch (packet.version) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V5:
         /*
          * connection to server, for forwarded connections or errors.
          */
         if (socksfd.forus.accepted.atype != SOCKS_ADDR_NOTSET
         &&  !socksfd.state.acceptpending) {
            /*
             * Strange.  Perhaps the socks server closed the connection,
             * and that is what the client detected, but thinking it
             * means a new connection is pending.
             */
            slog(LOG_INFO,
                 "%s: client is trying to accept(2) on a socket we have "
                 "already accept(2)-ed a client on (%s).  The standard "
                 "socks protocol only supports doing accept(2) once however, "
                 "so no more clients can be accepted on this socket (%d)",
                 function,
                 sockshost2string(&socksfd.forus.accepted, NULL, 0),
                 s);

            errno = EWOULDBLOCK;
            return -1;
         }

         break;

      case PROXY_UPNP: {
         struct sockaddr_storage addraccepted;
         socklen_t acceptedlen = sizeof(addraccepted);
         int client;

         slog(LOG_DEBUG,
              "%s: no controldata for UPNP-based bind, can do ordinary "
              "accept(2) on fd %d",
              function, s);

         client = accept(s, TOSA(&addraccepted), &acceptedlen);
         memcpy(addr, &addraccepted, MIN(*addrlen, acceptedlen));

         if (client != -1) {
            if (addforwarded(s,
                             client,
                             &addraccepted,
                             sockaddr2sockshost(&addraccepted, NULL))
            != 0)
               return -1;
         }

         return client;
      }

      default:
         SERRX(packet.version);
   }

   rset = allocate_maxsize_fdset();

   FD_ZERO(rset);
   fdbits = -1;

   FD_SET(s, rset);
   fdbits = MAX(fdbits, s);

   SASSERTX(fdbits >= 0);
   ++fdbits;

   if (fdisblocking(socksfd.control))
      p = selectn(fdbits, rset, NULL, NULL, NULL, NULL, NULL);
   else {
      struct timeval timeout;

      timeout.tv_sec  = 0;
      timeout.tv_usec = 0;

      if ((p = selectn(fdbits, rset, NULL, NULL, NULL, NULL, &timeout)) == 0) {
         errno = EWOULDBLOCK;
         p = -1;
      }
   }

   if (p == -1) {
      free(rset);
      return -1;
   }

   SASSERTX(p > 0);
   SASSERTX(FD_ISSET(s, rset));

   free(rset);

   if (!socksfd.state.acceptpending) {
      /*
       * pending data on control channel, server wants to forward addr info.
       */

      switch (packet.version) {
         case PROXY_SOCKS_V4:
         case PROXY_SOCKS_V5: {
            socksfd_t sfddup, *ptr;
            int dummy;

            packet.res.auth = &socksfd.state.auth;
            if (socks_recvresponse(socksfd.control,
                                   &packet.res,
                                   packet.version,
                                   emsg,
                                   sizeof(emsg)) != 0) {
               slog(LOG_INFO, "%s: socks_recvresponse() failed: %s, errno: %d",
                    function, emsg, errno);

               return -1;
            }

            ptr = socks_getaddr(socksfd.control, &socksfd, 1);
            SASSERTX(ptr != NULL);

            socksfd.forus.accepted = packet.res.host;
            socks_addaddr(socksfd.control, &socksfd, 1);

            /*
             * accept(2) returns a new fd, so try to do the same by
             * dup2(2)'ing the connection to a new fd-index, and then
             * making the old fd-index a "dummy" socket.
             * The latter will hopefully prevent the client from thinking
             * data pending on the socket is a new connection.
             */

            if ((remote = dup(socksfd.control)) == -1) {
               swarn("%s: dup(socksfd.control) failed", function);
               return -1;
            }

            sfddup         = socksfd;
            sfddup.control = remote;


            /*
             * in case previously allocated for some other socket,
             * which now must be closed.
             */
            socks_rmaddr(remote, 1);

            socks_addaddr(remote, &sfddup, 1);
            socks_allocbuffer(remote, SOCK_STREAM);

            /*
             * Now make the old socket/socksfd_t object reference the
             * created dummy socket.  Hopefully the client does not have
             * a dup(2) of the original socket.
             */

            if ((dummy = makedummyfd(AF_INET, SOCK_STREAM)) == -1) {
               swarn("%s: could not create dummy (AF_INET, SOCK_STREAM) socket",
                     function);

               return -1;
            }

            p = dup2(dummy, socksfd.control);
            close(dummy);

            if (p == -1) {
               swarn("%s: dup2(socksfd.control, dummy) failed", function);
               return -1;
            }

            socks_addaddr(socksfd.control, &socksfd, 1);
            break;
         }

         default:
            SERRX(packet.version);
      }

      sockshost2sockaddr(&socksfd.forus.accepted, &accepted);

      slog(LOG_INFO,
           "%s: accepted forwarded connection from %s via proxy on fd %d.  "
           "Will return fd %d to caller",
           function, sockaddr2string(&accepted, NULL, 0), s, remote);
   }
   else {
      /*
       * Pending data must be a new connection to accept(2).
       */
      socklen_t len;

      len = sizeof(accepted);
      if ((remote = accept(s, TOSA(&accepted), &len)) == -1)
         return -1;

      slog(LOG_INFO, "%s: accepted: %s",
           function, sockaddr2string(&accepted, NULL, 0));

      if (socksfd.state.acceptpending) {
         /*
          * connection forwarded by server, or an ordinary (local/direct)
          * connect?
          */

         if (memcmp(GET_SOCKADDRADDR(&accepted),
                    GET_SOCKADDRADDR(&socksfd.reply),
                    salen(accepted.ss_family)) == 0) {
            /*
             * connected address matches servers IP address, so assume
             * it's a forwarded connection.
             */
            int forwarded;

            slog(LOG_INFO,
                 "%s: remote matches servers IP-address.  Assuming connection "
                 "from %s is a forwarded connection",
                 function, sockaddr2string(&accepted, NULL, 0));

            switch (socksfd.state.version) {
               case PROXY_SOCKS_V4:
               case PROXY_SOCKS_V5: {
                  authmethod_t auth = socksfd.state.auth;

                  packet.req.version   = (unsigned char)socksfd.state.version;
                  packet.req.command   = SOCKS_BIND;
                  packet.req.flag      = 0;
                  sockaddr2sockshost(&accepted, &packet.req.host);
                  packet.req.auth      = &auth;

                  if (socks_sendrequest(socksfd.control,
                                        &packet.req,
                                        emsg,
                                        sizeof(emsg)) != 0) {
                     swarnx("%s: socks_sendrequest() failed: %s",
                            function, emsg);

                     close(remote);
                     return -1;
                  }

                  if (socks_recvresponse(socksfd.control,
                                         &packet.res,
                                         packet.req.version,
                                         emsg,
                                         sizeof(emsg)) != 0) {
                     swarnx("%s: socks_recvresponse() failed: %s",
                            function, emsg);

                     close(remote);
                     return -1;
                  }

                  if (packet.res.host.atype != SOCKS_ADDR_IPV4) {
                     swarnx("%s: unexpected atype in bindquery response: %d",
                     function, packet.res.host.atype);
                     close(remote);
                     errno = ECONNABORTED;
                     return -1;
                  }

                  if (packet.res.host.addr.ipv4.s_addr == htonl(0))
                     forwarded = 0;
                  else
                     forwarded = 1;
                  break;
               }

               default:
                  SERRX(socksfd.state.version);
            }

            if (forwarded) {
               if (addforwarded(s, remote, &accepted, &packet.res.host) != 0)
                  return -1;
            }
            /* else; ordinary remote connect, nothing special to do. */
         }
      }
      else
         SWARNX(0);
   }

   if (addr != NULL) {
      *addrlen = MIN(*addrlen, (socklen_t)sizeof(accepted));
      sockaddrcpy(TOSS(addr), &accepted, (size_t)*addrlen);
   }

   return remote;
}

static int
addforwarded(local, remote, remoteaddr, virtualremoteaddr)
   const int local;
   const int remote;
   const struct sockaddr_storage *remoteaddr;
   const sockshost_t *virtualremoteaddr;
{
   const char *function = "addforwarded()";
   socklen_t len;
   socksfd_t socksfd, rfd, *p;
   char raddr[MAXSOCKADDRSTRING], vaddr[MAXSOCKSHOSTSTRING];

   slog(LOG_DEBUG,
         "%s: registering fd %d as accepted from fd %d, address %s, "
         "virtualaddress %s",
         function,
         remote,
         local,
         sockaddr2string(remoteaddr, raddr, sizeof(raddr)),
         sockshost2string(virtualremoteaddr, vaddr, sizeof(vaddr)));


   p = socks_getaddr(local, &socksfd, 1);
   SASSERTX(p != NULL);

   if (socks_addrdup(p, &rfd) == NULL) {
      swarn("%s: socks_addrdup()", function);

      if (errno == EBADF)
         socks_rmaddr(local, 1);

      return -1;
   }

   /*
    * a separate socket with it's own remote address and possibly different
    * local address too, so need to add it to the socksfd table.
    */

   rfd.state.acceptpending = 0;
   sockaddrcpy(&rfd.remote, remoteaddr, salen(rfd.remote.ss_family));
   rfd.forus.accepted      = *virtualremoteaddr;

   /* has a local address now if unbound before. */
   if (!ADDRISBOUND(&rfd.local)) {
      len = sizeof(rfd.local);
      if (getsockname(remote, TOSA(&rfd.local), &len) != 0)
         swarn("%s: getsockname(remote)", function);
   }

   socks_addaddr(remote, &rfd, 1);

   return 0;
}
