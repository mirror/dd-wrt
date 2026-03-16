/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2020
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

static const char rcsid[] =
"$Id: upnp.c,v 1.153.4.4.2.2.4.2.4.1 2024/11/21 16:02:49 karls Exp $";

#include "common.h"

#include "upnp.h"

#if HAVE_LIBMINIUPNP

#if SOCKS_CLIENT
static struct sigaction oldsig;
static void sighandler(int sig);
static void atexit_upnpcleanup(void);
#endif /* SOCKS_CLIENT */

#endif /* HAVE_LIBMINIUPNP */

int
socks_initupnp(gw, emsg, emsglen)
   gateway_t *gw;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_initupnp()";
#if HAVE_LIBMINIUPNP
   struct UPNPDev *dev;
   struct UPNPUrls url;
   struct IGDdatas data;
   char myaddr[INET_ADDRSTRLEN], addrstring[MAXSOCKSHOSTSTRING], vbuf[1024];
   int devtype, rc;

   if (*gw->state.data.upnp.controlurl != NUL) {
      slog(LOG_DEBUG, "%s: already inited with controlurl %s",
           function,
           str2vis(gw->state.data.upnp.controlurl,
                   strlen(gw->state.data.upnp.controlurl),
                   vbuf,
                   sizeof(vbuf)));

      return 0;
   }

   slog(LOG_DEBUG, "%s", function);

   bzero(&url, sizeof(url));
   bzero(&data, sizeof(data));
   errno = 0;

   if (gw->addr.atype == SOCKS_ADDR_URL) {
      slog(LOG_NEGOTIATE, "%s: trying to use UPNP IGD at %s",
           function, str2vis(gw->addr.addr.urlname,
                             strlen(gw->addr.addr.urlname),
                             vbuf,
                             sizeof(vbuf)));

      if (UPNP_GetIGDFromUrl(gw->addr.addr.urlname,
                             &url,
                             &data,
                             myaddr,
                             sizeof(myaddr)) != 1) {
         snprintf(emsg, emsglen, "failed to contact UPNP IGD at url %s: %s",
                  vbuf, strerror(errno));
         swarnx("%s: %s", function, emsg);

         if (errno == 0)
            errno = ENETUNREACH;

         return -1;
      }

      slog(LOG_DEBUG, "%s: UPNP_GetIGDFromUrl() url %s was successful",
           function, vbuf);

      rc = 0;
   }
   else {
      struct UPNPDev *p;
      struct sockaddr_storage addr;
      command_t commands;
      protocol_t protocols;

      slog(LOG_NEGOTIATE, "%s: searching for UPNP IGD using address %s",
           function, sockshost2string(&gw->addr, NULL, 0));

      sockshost2sockaddr(&gw->addr, &addr);
      if (inet_ntop(addr.ss_family,
                    GET_SOCKADDRADDR(&addr),
                    addrstring,
                    sizeof(addrstring)) == NULL) {
         snprintf(emsg, emsglen,
                  "%s: failed to convert %s to an ipaddress: %s",
                  function,
                  sockshost2string(&gw->addr, NULL, 0),
                  strerror(errno));

         swarnx("%s: %s", function, emsg);
         return -1;
      }

      slog(LOG_NEGOTIATE,
           "%s: doing upnp discovery on interface belonging to addr %s, "
           "resolved from %s.  errno = %d",
           function, addrstring, sockshost2string(&gw->addr, NULL, 0), errno);

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC
      socks_mark_io_as_native();
#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

      errno = 0;
      dev   = upnpDiscover(UPNP_DISCOVERYTIME_MS,
                           addrstring,
                           NULL,
                           0
#if HAVE_LIBMINIUPNP17 || HAVE_LIBMINIUPNP228
                          ,0,

#if MINIUPNPC_API_VERSION >= 14
                           UPNP_IP_TTL,
#endif /* MINIUPNPC_API_VERSION >= 14 */

                          &rc
#endif /* HAVE_LIBMINIUPNP17 || HAVE_LIBMINIUPNP228 */
                         );

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC
      socks_mark_io_as_normal();
#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

      if (dev == NULL) {
         snprintf(emsg, emsglen, "upnpDiscover() failed: %s", strerror(errno));
         swarnx("%s: %s", function, emsg);

         if (errno == 0)
            errno = ENETUNREACH;

         return -1;
      }

      slog(LOG_NEGOTIATE,
           "%s: upnp devices found.  Adding direct routes for them", function);

      bzero(&commands, sizeof(commands));
      bzero(&protocols, sizeof(protocols));

      for (p = dev; p != NULL; p = p->pNext) {
         struct sockaddr_storage saddr, smask;
         int gaierr;

         if (urlstring2sockaddr(p->descURL, &saddr, &gaierr, emsg, emsglen)
         == NULL) {
            log_resolvefailed(p->descURL, EXTERNALIF, gaierr);
            continue;
         }

         bzero(&smask, sizeof(smask));
         SET_SOCKADDR(&smask, AF_INET);
         TOIN(&smask)->sin_addr.s_addr = htonl(IPV4_FULLNETMASK);

         commands.connect      = 1;
         commands.udpassociate = 1;

         protocols.tcp         = 1;
         protocols.udp         = 1;

         socks_autoadd_directroute(&commands, &protocols, &saddr, &smask);
      }

#if HAVE_LIBMINIUPNP228
      devtype = UPNP_GetValidIGD(dev, &url, &data, myaddr, sizeof(myaddr),
                                 NULL, 0);
#else /* !HAVE_LIBMINIUPNP228 */
      devtype = UPNP_GetValidIGD(dev, &url, &data, myaddr, sizeof(myaddr));
#endif /* !HAVE_LIBMINIUPNP228 */
      switch (devtype) {
         case UPNP_NO_IGD:
            snprintf(emsg, emsglen, "no UPNP IGD discovered on local network");
            swarnx("%s: %s", function, emsg);
            rc = -1;
            break;

         case UPNP_CONNECTED_IGD:
            slog(LOG_NEGOTIATE, "%s: UPNP IGD discovered at url %s",
                 function,
                 str2vis(url.controlURL,
                         strlen(url.controlURL),
                         vbuf,
                         sizeof(vbuf)));
            rc = 0;
            break;

#if HAVE_LIBMINIUPNP228
         case UPNP_RESERVED_IGD:
            snprintf(emsg, emsglen,
                    "UPNP IGD discovered at url %s, but its IP is reserved",
                    str2vis(url.controlURL,
                           strlen(url.controlURL),
                            vbuf,
                            sizeof(vbuf)));

            swarnx("%s: %s", function, emsg);
            rc = -1;
            break;
#endif /* HAVE_LIBMINIUPNP228 */

         case UPNP_DISCONNECTED_IGD:
            snprintf(emsg, emsglen,
                    "UPNP IGD discovered at url %s, but it is not connected",
                    str2vis(url.controlURL,
                           strlen(url.controlURL),
                            vbuf,
                            sizeof(vbuf)));

            swarnx("%s: %s", function, emsg);
            rc = -1;
            break;

         case UPNP_UNKNOWN_DEVICE:
            snprintf(emsg, emsglen,
                     "%s: unknown upnp device discovered at url %s",
                     function,
                     str2vis(url.controlURL,
                             strlen(url.controlURL),
                             vbuf,
                             sizeof(vbuf)));

            swarnx("%s: %s", emsg, function);

            rc = -1;
            break;

         default:
            snprintf(emsg, emsglen,
                     "%s: unknown return code from UPNP_GetValidIGD(): %d (%s)",
                     function, devtype, strerror(errno));

            swarnx("%s: %s", function, emsg);
            rc = -1;
      }

      freeUPNPDevlist(dev);
   }

   if (rc == 0) {
      char visbuf2[sizeof(vbuf)];

      SASSERTX(url.controlURL != NULL);
      STRCPY_ASSERTLEN(gw->state.data.upnp.controlurl, url.controlURL);

#if HAVE_LIBMINIUPNP13
      STRCPY_ASSERTLEN(gw->state.data.upnp.servicetype, data.servicetype);

#elif HAVE_LIBMINIUPNP14 || HAVE_LIBMINIUPNP17 || HAVE_LIBMINIUPNP228
      STRCPY_ASSERTLEN(gw->state.data.upnp.servicetype, data.CIF.servicetype);

#else
#  error "unexpected miniupnp version"
#endif /* HAVE_LIBMINIUPNP14 || HAVE_LIBMINIUPNP17 || HAVE_LIBMINIUPNP228 */

      slog(LOG_NEGOTIATE, "%s: inited ok.  controlurl: %s, servicetype: %s",
           function,
           str2vis(gw->state.data.upnp.controlurl,
                   strlen(gw->state.data.upnp.controlurl),
                   vbuf,
                   sizeof(vbuf)),
           str2vis(gw->state.data.upnp.servicetype,
                   strlen(gw->state.data.upnp.servicetype),
                   visbuf2,
                   sizeof(visbuf2)));
   }
   else {
      if (errno == 0)
         errno = ENETUNREACH;
   }

   FreeUPNPUrls(&url);

   return rc;

#else /* !HAVE_LIBMINIUPNP */
   return -1;

#endif /* !HAVE_LIBMINIUPNP */
}

int
upnp_negotiate(s, packet, gw, emsg, emsglen)
   const int s;
   socks_t *packet;
   gateway_t *gw;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "upnp_negotiate()";
#if HAVE_LIBMINIUPNP
   struct sockaddr_storage addr;
   socklen_t addrlen;
   char straddr[INET6_ADDRSTRLEN], strport[sizeof("65535")], vbuf[1024];
   int rc;
#endif /* HAVE_LIBMINIUPNP */

   slog(LOG_DEBUG, "%s: command %s",
        function, command2string(packet->req.command));

#if !HAVE_LIBMINIUPNP
   SERRX(0);

#else
   packet->res.version = PROXY_UPNP;

   switch (packet->req.command) {
      case SOCKS_CONNECT: {
         /*
          * Can only find out what the external ip address of the device is.
          *
          * We could fetch the address here, but if the client never intends
          * to find out what it's local address is, that's a waste of time.
          * Therefor postpone it to the Rgetsockname() call, if it ever
          * comes, and just connect(2) to the target for now, without
          * attempting to retrieve any information from the IGD.
          *
          * For the socks server case (server chained) we need to fetch
          * it here though, since it is part of the response returned
          * to the socks client.
          */

         if (socks_connecthost(s,
#if !SOCKS_CLIENT
                               EXTERNALIF,
#endif /* !SOCKS_CLIENT */
                               &packet->req.host,
                               NULL,
                               NULL,
                               sockscf.timeout.connect
                                 ? (long)sockscf.timeout.connect : (long)-1,
                               emsg,
                               emsglen) != 0)
            if (errno != EINPROGRESS) {
               slog(LOG_NEGOTIATE, "%s: socks_connecthost(%s) failed: %s",
                    function,
                    sockshost2string(&packet->req.host, NULL, 0),
                    emsg);

               socks_set_responsevalue(&packet->res,
                                       errno2reply(errno, PROXY_UPNP));
               return -1;
            }
      }

      /* FALLTHROUGH */

      case SOCKS_UDPASSOCIATE: {
         /*
          * if it was a bind, it would be handled the same as the
          * tcp bind, so this means the client starts by wanting
          * to send a udp packet, or we just did a connect(2).
          *
          * Similarly to a connect, the only information we can provide
          * here is the external ip address of the control device.
          * Postponed for the same reason as for connect.
          */
         const int errno_s = errno;

         packet->res.host.atype = SOCKS_ADDR_IPV4;

#if SOCKS_CLIENT
         /*
          * will be filled with real value if user ever does getsockname(2).
          * Don't waste time on getting the address until it's needed.
          */
         slog(LOG_NEGOTIATE,
              "%s: no need to contact UPNP IGD %s yet, "
              "so just pretending all is ok so far",
              function, sockshost2string(&gw->addr, NULL, 0));

         packet->res.host.addr.ipv4.s_addr = htonl(INADDR_ANY);

#else /* SOCKS_SERVER.  Server needs to know now so it can tell client. */
         if (socks_initupnp(gw, emsg, emsglen) != 0) {
            swarnx("%s: socks_initupnp() could not init upnp state: %s",
                   function, emsg);

            socks_set_responsevalue(&packet->res, UPNP_FAILURE);
            return -1;
         }

         if ((rc = UPNP_GetExternalIPAddress(gw->state.data.upnp.controlurl,
                                             gw->state.data.upnp.servicetype,
                                             straddr)) != UPNPCOMMAND_SUCCESS) {
            snprintf(emsg, emsglen,
                     "failed to get external ip address of UPNP device at %s, "
                     "upnp-error: %d",
                     str2vis(gw->state.data.upnp.controlurl,
                             strlen(gw->state.data.upnp.controlurl),
                             vbuf,
                             sizeof(vbuf)),
                     rc);

            swarnx("%s: %s", function, emsg);

            socks_set_responsevalue(&packet->res, UPNP_FAILURE);
            return -1;
         }
         else
            slog(LOG_NEGOTIATE, "%s: server will use address %s on our behalf",
                 function,
                 str2vis(straddr, strlen(straddr), vbuf, sizeof(vbuf)));

         if (socks_inet_pton(AF_INET,
                             straddr,
                             &packet->res.host.addr.ipv4,
                             NULL) == 1)
            packet->res.host.atype = SOCKS_ADDR_IPV4;
         else if (socks_inet_pton(AF_INET6,
                                  straddr,
                                  &packet->res.host.addr.ipv6,
                                  NULL) == 1)
            packet->res.host.atype = SOCKS_ADDR_IPV6;
         else
            slog(LOG_NOTICE,
                 "%s: could not convert string \"%s\" to address: %s",
                 function,
                 str2vis(straddr,
                         strlen(straddr),
                         vbuf,
                         sizeof(vbuf)),
                 strerror(errno));
#endif /* SOCKS_SERVER */

         addrlen = sizeof(addr);
         if (getsockname(s, TOSA(&addr), &addrlen) != 0) {
            snprintf(emsg, emsglen,
                     "getsockname(s) on socket failed: %s", strerror(errno));

            break;
         }

         slog(LOG_NEGOTIATE,
              "%s: will never know for sure, but hoping IGD will use same "
              "port as we: %u",
              function, ntohs(packet->res.host.port));

         packet->res.host.port = GET_SOCKADDRPORT(&addr);
         errno = errno_s;

         break;
      }

      case SOCKS_BIND: {
         /*
          * Need tell the device to create a port mapping, mapping an
          * address on it's side to the address we have bound.
          * Then we need to get the ip address the device is using
          * on the external side.
          */
#if SOCKS_CLIENT
         static int atexit_registered;
#endif /* SOCKS_CLIENT */
         const int errnoval = EADDRNOTAVAIL;
         struct sockaddr_storage extaddr;
         socklen_t len;
         char buf[256], protocol[16];
         int val;

         addrlen = sizeof(addr);
         if (getsockname(s, TOSA(&addr), &addrlen) != 0) {
            snprintf(emsg, emsglen,
                     "getsockname(s) on socket failed: %s", strerror(errno));

            socks_set_responsevalue(&packet->res, UPNP_FAILURE);
            return -1;
         }

         if (!PORTISBOUND(&addr)) {
            slog(LOG_NEGOTIATE,
                 "%s: port not yet bound locally, so need to bind it before "
                 "we tell the UPNP-router to portforward to our bound port",
                 function);

            if (socks_bind(s, &addr, 0) != 0) {
               snprintf(emsg, emsglen,
                        "could not bind local address %s on fd %d before "
                        "portforward request to UPNP-router: %s",
                         sockaddr2string(&addr, NULL, 0), s, strerror(errno));

               swarnx("%s: %s", function, emsg);

               socks_set_responsevalue(&packet->res, UPNP_FAILURE);
               return -1;
            }
         }

         if (socks_initupnp(gw, emsg, emsglen) != 0) {
            swarnx("%s: socks_initupnp() could not init upnp state: %s",
                   function, emsg);

            socks_set_responsevalue(&packet->res, UPNP_FAILURE);

            errno = errnoval;
            return -1;
         }

         if ((rc = UPNP_GetExternalIPAddress(gw->state.data.upnp.controlurl,
                                             gw->state.data.upnp.servicetype,
                                             straddr)) != UPNPCOMMAND_SUCCESS) {
            snprintf(emsg, emsglen,
                     "failed to get external ip address of upnp device, "
                     "error: %d",
                     rc);

            socks_set_responsevalue(&packet->res, UPNP_FAILURE);

            errno = errnoval;
            return -1;
         }

         extaddr = addr; /* init. */

         if ((rc = socks_inet_pton(extaddr.ss_family,
                                   straddr,
                                   (void *)GET_SOCKADDRADDR(&extaddr),
                                   NULL)) != 1) {
            snprintf(emsg, emsglen,
                     "strange.  UPNP-device said it's external IP-address is "
                     "the %s \"%s\", but can't parse that with inet_pton(3).  "
                     "Errorcode %d (%s)",
                     safamily2string(extaddr.ss_family),
                     str2vis(straddr, strlen(straddr), vbuf, sizeof(vbuf)),
                     rc,
                     strerror(errno));

            socks_set_responsevalue(&packet->res, UPNP_FAILURE);

            errno = errnoval;
            return -1;
         }

         sockaddr2sockshost(&extaddr, &packet->res.host);

         slog(LOG_NEGOTIATE,
              "%s: upnp control point's external ip address is %s",
              function,
              str2vis(straddr, strlen(straddr), vbuf, sizeof(vbuf)));

         if (!ADDRISBOUND(&addr)) {
            /*
             * Address not bound.  Not bound is good enough for us if
             * it it's good enough for caller, but we do need to tell the
             * igd what address it should forward the connection to.
             */
             struct sockaddr_storage tmpaddr;

            switch (packet->gw.addr.atype) {
               case SOCKS_ADDR_IFNAME: {
                  struct sockaddr_storage mask;

                  if (ifname2sockaddr(packet->gw.addr.addr.ifname,
                                      0,
                                      &tmpaddr,
                                      &mask) == NULL) {
                     snprintf(emsg, emsglen,
                              "ifname2sockaddr() could not resolve nic %s to "
                              "an ipaddress",
                              packet->gw.addr.addr.ifname);
                     swarnx("%s: %s", function, emsg);

                     socks_set_responsevalue(&packet->res, UPNP_FAILURE);

                     errno = errnoval;
                     return -1;
                  }

                  /* just want the ipaddr.  Port number, etc., remains same. */
                  SET_SOCKADDRADDR((&addr), GET_SOCKADDRADDR(&tmpaddr));
                  break;
               }

               case SOCKS_ADDR_URL: {
                  socklen_t tmpaddrlen;
                  char lemsg[256] /* local emsg */;
                  int ss, gaierr;

                  if (urlstring2sockaddr(packet->gw.addr.addr.urlname,
                                         &tmpaddr,
                                         &gaierr,
                                         lemsg,
                                         sizeof(lemsg)) == NULL) {

                     log_resolvefailed(packet->gw.addr.addr.urlname,
                                       EXTERNALIF,
                                       gaierr);

                     snprintf(emsg, emsglen,
                              "could not resolve url %s to an ipaddress: %s",
                              str2vis(packet->gw.addr.addr.urlname,
                                      strlen(packet->gw.addr.addr.urlname),
                                      vbuf,
                                      sizeof(vbuf)),

                              lemsg);

                     socks_set_responsevalue(&packet->res, UPNP_FAILURE);

                     errno = errnoval;
                     return -1;
                  }

                  /*
                   * What we need to find out now is, what is the address
                   * this host uses to communicate with the control point?
                   * That is the address we need to tell it to forward
                   * the connection to.  Could use getifa(), but we
                   * are not guaranteed that always works as desired,
                   * so do a regular connect(2) to know for sure.
                   */

                  if ((ss = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                     snprintf(emsg, emsglen, "socket() failed: %s",
                              strerror(errno));
                     swarnx("%s: %s", function, emsg);

                     socks_set_responsevalue(&packet->res, UPNP_FAILURE);
                     return -1;
                  }

                  if (connect(ss, TOSA(&tmpaddr), salen(tmpaddr.ss_family))
                  != 0) {
                     snprintf(emsg, emsglen,
                              "connect to proxy server %s failed: %s",
                              sockaddr2string(&tmpaddr, NULL, 0),
                              strerror(errno));
                     swarnx("%s: %s", function, emsg);

                     socks_set_responsevalue(&packet->res, UPNP_FAILURE);
                     close(ss);


                     errno = errnoval;
                     return -1;
                  }

                  tmpaddrlen = sizeof(tmpaddr);
                  if (getsockname(ss, TOSA(&tmpaddr), &tmpaddrlen) != 0) {
                     snprintf(emsg, emsglen,
                              "getsockname(ss) failed: %s", strerror(errno));

                     socks_set_responsevalue(&packet->res, UPNP_FAILURE);
                     close(ss);


                     errno = errnoval;
                     return -1;
                  }

                  close(ss);
                  SET_SOCKADDRADDR((&addr), GET_SOCKADDRADDR(&tmpaddr));

                  break;
               }

               default:
                  SERRX(packet->gw.addr.atype);
            }
         }

         if (inet_ntop(addr.ss_family,
                       GET_SOCKADDRADDR(&addr),
                       straddr,
                       sizeof(straddr)) == NULL) {
            snprintf(emsg, emsglen,
                     "inet_ntop(3) on address %s failed: %s",
                     sockaddr2string(&addr, NULL, 0), strerror(errno));

            socks_set_responsevalue(&packet->res, UPNP_FAILURE);

            errno = errnoval;
            return -1;
         }

         len = sizeof(val);
         if (getsockopt(s, SOL_SOCKET, SO_TYPE, &val, &len) != 0) {
            snprintf(emsg, emsglen,
                     "getsockopt(s) failed: %s", strerror(errno));
            socks_set_responsevalue(&packet->res, UPNP_FAILURE);

            return -1;
         }
         switch (val) {
            case SOCK_DGRAM:
               snprintf(protocol, sizeof(protocol), PROTOCOL_UDPs);
               break;

            case SOCK_STREAM:
               snprintf(protocol, sizeof(protocol), PROTOCOL_TCPs);
               break;

            default:
               snprintf(emsg, emsglen, "unknown protocol type %d", val);
               swarnx("%s: fd %d (socket s): %s", function, s, emsg);

               socks_set_responsevalue(&packet->res, UPNP_FAILURE);

               errno = errnoval;
               return -1;
         }

         snprintf(strport, sizeof(strport),
                  "%d", ntohs(GET_SOCKADDRPORT(&addr)));

         snprintf(buf, sizeof(buf), "%s (%s/client v%s via libminiupnpc)",
                 __progname, PRODUCT, VERSION);

         slog(LOG_NEGOTIATE,
              "%s: trying to add %s port mapping for fd %d on upnp device at "
              "%s: %s -> %s.%s",
              function,
              protocol,
              s,
              str2vis(gw->state.data.upnp.controlurl,
                      strlen(gw->state.data.upnp.controlurl),
                      vbuf,
                      sizeof(vbuf)),
              strport,
              str2vis(straddr, strlen(straddr), vbuf, sizeof(vbuf)),
              strport);

         str2upper(protocol); /* miniupnp-lib seems to fail if not. :-/ */
         if ((rc = UPNP_AddPortMapping(gw->state.data.upnp.controlurl,
                                       gw->state.data.upnp.servicetype,
                                       strport,
                                       strport,
                                       straddr,
                                       buf,
                                       protocol,
                                       NULL
#if HAVE_LIBMINIUPNP17 || HAVE_LIBMINIUPNP228
                                       ,0
#endif /* HAVE_LIBMINIUPNP17 || HAVE_LIBMINIUPNP228 */
                                       )) != UPNPCOMMAND_SUCCESS) {
               snprintf(emsg, emsglen,
                       "UPNP_AddPortMapping() failed: %s", strupnperror(rc));

               slog(LOG_NEGOTIATE, "%s: %s", function, emsg);

               socks_set_responsevalue(&packet->res, UPNP_FAILURE);

               errno = errnoval;
               return -1;
         }
         else
            slog(LOG_NEGOTIATE,
                 "%s: addition of port mapping succeeded", function);

#if SOCKS_CLIENT
         if (!atexit_registered) {
            struct sigaction oursig;
            size_t i;
            int signalv[] = { SIGINT };

            slog(LOG_DEBUG, "%s: registering cleanup function with atexit(3)",
                 function);

            if (atexit(atexit_upnpcleanup) != 0) {
               snprintf(emsg, emsglen,
                        "atexit() failed to register upnp cleanup function: %s",
                        strerror(errno));
               swarnx("%s: %s", function, emsg);
               break;
            }

            atexit_registered = 1;

            for (i = 0; i < ELEMENTS(signalv); ++i) {
               if (sigaction(signalv[i], NULL, &oldsig) != 0) {
                  swarn("%s: sigaction(%d)", function, signalv[i]);
                  continue;
               }

               oursig = oldsig;
               oursig.sa_handler = sighandler;
               oursig.sa_flags  |= SA_SIGINFO;

               if (sigaction(signalv[i], &oursig, NULL) != 0) {
                  swarn("%s: sigaction(%d)", function, signalv[i]);
                  break;
               }
            }
         }
#endif /* SOCKS_CLIENT */
         break;
      }

      default:
         SERRX(packet->req.command);
   }

   socks_set_responsevalue(&packet->res, UPNP_SUCCESS);

#endif /* HAVE_LIBMINIUPNP */

   return 0;
}

#if HAVE_LIBMINIUPNP

#if SOCKS_CLIENT
static void
sighandler(sig)
   int sig;
{
   const char *function = "sighandler()";

   slog(LOG_DEBUG, "%s", function);

   upnpcleanup(-1);

   /* reinstall original signal handler. */
   if (sigaction(SIGINT, &oldsig, NULL) != 0)
      serr("%s: restoring old signal handler failed", function);

   raise(SIGINT);
}
#endif /* SOCKS_CLIENT */

#if SOCKS_CLIENT
void
upnpcleanup(s)
   const int s;
{
   const char *function = "upnpcleanup()";
   socksfd_t socksfd;
   int rc, current, last;

   slog(LOG_DEBUG, "%s: fd %d", function, s);

   if (s == -1) {
      current = 0;
      last    = getmaxofiles(softlimit) - 1;
   }
   else {
      current  = s;
      last     = s;
   }

   for (; current <= last; ++current) {
      static int deleting;
      char port[sizeof("65535")], protocol[sizeof("TCP")];

      if (deleting == current)
         continue;

      if (socks_getaddr(current, &socksfd, 0 /* XXX */) == NULL)
         continue;

      if (socksfd.state.version != PROXY_UPNP)
         continue;

      slog(LOG_NEGOTIATE,
           "%s: fd %d has upnp session set up for command %s, "
           "accept pending: %s",
           function,
           current,
           command2string(socksfd.state.command),
           socksfd.state.acceptpending ? "yes" : "no");

      if (socksfd.state.command != SOCKS_BIND)
         continue;

      /*
       * Is this the socket we listened on?  Or just a client we accept(2)'ed?
       * The port mapping is just created for the first case.
       */
      if (!socksfd.state.acceptpending)
         continue; /* just a client we accepted. */

      snprintf(port, sizeof(port),
               "%d", ntohs(GET_SOCKADDRPORT(&socksfd.remote)));

      if (socksfd.state.protocol.tcp)
         snprintf(protocol, sizeof(protocol), PROTOCOL_TCPs);
      else if (socksfd.state.protocol.udp)
         snprintf(protocol, sizeof(protocol), PROTOCOL_UDPs);
      else {
         SWARNX(0);
         continue;
      }

      slog(LOG_NEGOTIATE, "%s: deleting port mapping for external %s port %s",
           function, protocol, port);

      str2upper(protocol);

      /*
       * needed to avoid recursion, as the below delete-call might
       * very well end up calling us again, which makes us try
       * to delete the port mapping twice.
       */
      deleting = current;

      rc = UPNP_DeletePortMapping(socksfd.route->gw.state.data.upnp.controlurl,
                                  socksfd.route->gw.state.data.upnp.servicetype,
                                  port,
                                  protocol,
                                  NULL);

      if (rc != UPNPCOMMAND_SUCCESS)
         swarnx("%s: UPNP_DeletePortMapping(%s, %s) failed: %s",
                function, port, protocol, strupnperror(rc));
      else
         slog(LOG_NEGOTIATE, "%s: deleted port mapping for external %s port %s",
              function, protocol, port);

      deleting = -1;
   }
}

static void
atexit_upnpcleanup(void)
{
   const char *function = "atexit_upnpcleanup()";

   slog(LOG_DEBUG, "%s", function);

   upnpcleanup(-1);
}
#endif /* SOCKS_CLIENT */
#else /* !HAVE_LIBMINIUPNP */
void
upnpcleanup(s)
   const int s;
{
        return;
}
#endif /* !HAVE_LIBMINIUPNP */
