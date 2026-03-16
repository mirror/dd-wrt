/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2017, 2019,
 *               2020
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
#include "config_parse.h"

static const char rcsid[] =
"$Id: dante_udp.c,v 1.93.4.3.2.3.4.3 2020/11/11 16:11:58 karls Exp $";


udpheader_t *
getudptarget(const char *buf, const size_t buflen, udpheader_t *header,
             size_t *headerlen, char *emsg, const size_t emsglen);
/*
 * Gets the target address for the SOCKS UDP packet stored in "buf".
 * "buflen" gives the length of the received UDP packet.
 *
 * On success "header" is returned, filled in appropriately.
 * "headerlen" gives the length of the SOCKS UDP header.  Payload will
 * start at the first byte following it.
 *
 * On failure NULL is returned.  "emsg" of "emsglen" will then contain an
 * error string describing the reason for failure.
 */


static int
fromaddr_as_expected(struct sockaddr_storage *expected,
                     const struct sockaddr_storage *from,
                     char *emsg, size_t emsglen);

/*
 * Checks that the packet received on socket "s", from "from", matches the
 * expected address as given in "expected".
 * If so, the socket is connected to "from" and "expected" is updated to
 * contain the address of "from".
 *
 * Returns if the from address is as expected, or false otherwise.  If false,
 * "emsg" may be filled with more information as to what went wrong or did
 * not match.
 */

extern int rawsocket;

iostatus_t
io_udp_client2target(control, client, twotargets, cauth, state,
                     clog, dlog, badfd, packetrule, bwused)
   sockd_io_direction_t *control;
   sockd_io_direction_t *client;
   sockd_io_direction_t *twotargets;
   const authmethod_t *cauth;
   connectionstate_t *state;
   int *badfd;
   iologaddr_t *clog;
   iologaddr_t *dlog;
   rule_t *packetrule;
   size_t *bwused;
{
   /*
    * Dante has only one client per i/o session, but each UDP-based i/o
    * session can have up to two targets (ipv4 and ipv6).  When transferring
    * data from the client we thus want to bill it to the correct target
    * target address.
    */
   const char *function = "io_udp_client2target()";
   iostatus_t iostatus = IO_NOERROR;
   udptarget_t *target = NULL;
   recvfrom_info_t recvflags;
   sendto_info_t sendtoflags;
   udpheader_t header;
   struct sockaddr_storage from, targetaddr;
   socklen_t len;
   ssize_t w, r;
   size_t headerlen, payloadlen, emsglen;
   char hosta[MAXSOCKSHOSTSTRING], hostb[MAXSOCKSHOSTSTRING], emsg[2048],
        buf[SOCKD_BUFSIZE + sizeof(udpheader_t)], *payload;
   int sametarget, gaierr,
       doconnect           = 0,
       permit              = 0,
       icmp_type           = ICMP_TYPE_UNREACHABLE,
       icmp_code           = ICMP_CODE_UNREACHABLE_PORT;

   recvflags.side = INTERNALIF;
   recvflags.peer = client->raddr;
   recvflags.type = SOCK_DGRAM;

   len = sizeof(from);
   if ((r = socks_recvfrom(client->s,
                           buf,
                           sizeof(buf),
                           0,
                           &from,
                           &len,
                           &recvflags,
                           &client->auth)) == -1) {
      if (ERRNOISPREVIOUSPACKET(errno)) {
         /*
          * error is from a previous packet sent by us out on this socket,
          * i.e., /from target/ to client.
          * Note that Linux apparently can return this error even if the
          * socket is not connected.
          *
          * Don't treat this as fatal; more packets could come
          * and they may be accepted by the client.  As long
          * as we still have the control connection, assume
          * the client is alive.  When we no longer have the
          * control connection, we delete this io.
          */
         struct sockaddr_storage *packetsrc;
         struct sockaddr_storage *receivedonaddr;
         const int oldcommand = state->command;
         state->command       = SOCKS_UDPREPLY;

         if (twotargets->dstc <= 1) {
            *badfd = twotargets->dstv[0].s;
            SASSERTX(*badfd != -1);

            receivedonaddr = &twotargets->dstv[0].laddr;
            packetsrc      = &twotargets->dstv[0].raddr;
         }
         else {
            /*
             * don't know which target socket was used.
             */
            *badfd         = -1;
            receivedonaddr = NULL;
            packetsrc      = NULL;
         }

         emsglen = snprintf(emsg, sizeof(emsg),
                            "%s (delayed error)", strerror(errno));

         iolog(packetrule,
               state,
               OPERATION_TMPERROR,
               dlog,
               clog,
               NULL,
               NULL,
               emsg,
               emsglen);

         state->command = oldcommand;

         send_icmperror(rawsocket,
                        receivedonaddr,
                        packetsrc,
                        &client->raddr,
                        -1,
                        -1,
                        icmp_type,
                        icmp_code);


         return IO_TMPERROR;
      }

      *badfd = client->s;

      if (ERRNOISTMP(errno))
         return IO_EAGAIN;
      else {
         log_unexpected_udprecv_error(function, client->s, errno, INTERNALIF);
         return IO_IOERROR;
      }
   }

   iostatus = io_packet_received(&recvflags, r, &from, &client->laddr);

   if (iostatus != IO_NOERROR) {
      *badfd = client->s;
      return iostatus;
   }

   sockaddr2sockshost(&from, &clog->peer);
   clog->peer_isset = 1;

   slog(LOG_DEBUG, "%s: received udp packet from %s, length: %ld",
        function, sockaddr2string(&from, NULL, 0), (long)r);

   /*
    * Read udp packet.  Now figure out if it should be forwarded, where it
    * should be forwarded, and from what address we should forward it.
    */

   *emsg = NUL;

   if (!client->state.isconnected) {
      /*
       * Have not yet connected to the client, so need to check whether
       * the packet is really from our expected client.  After the connect,
       * the kernel takes care of this for us.
       *
       * XXX or, what happens if somebody else also sends packets that ends
       * up in client the socket buffer before we connect to the real client?
       * Do we get those packet too from recvfrom(), even after we connect to
       * a different address?  Should check this out.
       */
      const int blocked
      = !fromaddr_as_expected(&client->raddr, &from, emsg, sizeof(emsg));

      if (!blocked) {
         sockaddr2sockshost(&client->raddr, &client->host);

         /* More efficient to be connected.  */
         if (socks_connecthost(client->s,
                               INTERNALIF,
                               &client->host,
                               NULL,
                               NULL,
                               (long)-1,
                               emsg,
                               sizeof(emsg)) != 0 ) {
            swarnx("%s: strange ... could not connect(2) back to new UDP "
                   "client that sent us its first packet from %s: %s",
                   function, sockaddr2string(&client->raddr, NULL, 0), emsg);

            *badfd = client->s;
            iostatus = IO_ERROR;
         }
         else
            client->state.isconnected = 1;
      }

      SASSERTX(!((iostatus != IO_NOERROR) && blocked));

      if (iostatus != IO_NOERROR || blocked) {
         iolog(packetrule,
               state,
               IOOP(blocked, iostatus),
               clog,
               dlog,
               NULL,
               NULL,
               emsg,
               strlen(emsg));

         return blocked ? IO_TMPERROR : iostatus;
      }

      SASSERTX(client->state.isconnected);
   }
   else
      /*
       * already connected to client.  Kernel should make sure address is
       * correct.
       */

   SASSERTX(iostatus == IO_NOERROR);

   if (getudptarget(buf, (size_t)r, &header, &headerlen, emsg, sizeof(emsg))
   == NULL) {
      slog(LOG_DEBUG,
           "%s: getudptarget() failed for packet of length %lu from client %s, "
           "received on local address %s for target %s: %s",
           function,
           (unsigned long)r,
           sockaddr2string(&client->raddr, hosta, sizeof(hosta)),
           sockaddr2string(&client->laddr, hostb, sizeof(hostb)),
           header.host.atype == SOCKS_ADDR_NOTSET ?
               "N/A" : sockshost2string(&header.host, NULL, 0),
           emsg);

      icmp_code        = ICMP_CODE_UNREACHABLE_DESTHOSTUNKNOWN;
      iostatus         = IO_TMPERROR;
   }

   if (header.host.atype != SOCKS_ADDR_NOTSET) {
      /*
       * Whether getudptarget() failed or not, we did get a hostname at
       * least, so update for logging.
       */
      dlog->peer        = header.host;
      dlog->peer_isset  = 1;
   }
   else
      dlog->peer_isset  = 0;

   if (iostatus == IO_NOERROR)  {
      SASSERTX(header.host.atype != SOCKS_ADDR_NOTSET);

      SASSERTX(headerlen > 0);
      payload    = buf + headerlen;
      payloadlen = r   - headerlen;

      sockshost2sockaddr2(&header.host,
                          &targetaddr,
                          &gaierr,
                          emsg,
                          sizeof(emsg));

      if (IPADDRISBOUND(&targetaddr)) {
         /*
          * we want to log ipaddress used, not (a possible) hostname sent
          * by the client.
          */
         sockaddr2sockshost(&targetaddr, &dlog->peer);
         SASSERTX(dlog->peer_isset);
      }
      else {
         if (gaierr != 0) {  /* dns-error. */
            SASSERTX(header.host.atype == SOCKS_ADDR_DOMAIN);
            log_resolvefailed(header.host.addr.domain, EXTERNALIF, gaierr);

            snprintf(emsg, sizeof(emsg),
                     "could not resolve target address %s: %s ",
                     sockshost2string(&header.host, NULL, 0),
                     gai_strerror(gaierr));
         }
         else { /* some other, non-dns error. */
            snprintf(emsg, sizeof(emsg),
                     "could not resolve target address %s.  Non-DNS error: %s ",
                     sockshost2string(&header.host, NULL, 0),
                     strerror(errno));
         }

         iostatus  = IO_TMPERROR;
         icmp_code = ICMP_CODE_UNREACHABLE_DESTHOSTUNKNOWN;
      }
   }
   else {
      bzero(&targetaddr, sizeof(targetaddr));
      SET_SOCKADDR(&targetaddr, AF_INET);
   }

   if (iostatus != IO_NOERROR) {
      iolog(packetrule,
            state,
            IOOP(packetrule->verdict == VERDICT_BLOCK, iostatus),
            clog,
            dlog,
            NULL,
            NULL,
            emsg,
            strlen(emsg));

      /*
       * This may be considered a violation of rfc 1928, which says UDP
       * packets should be forwarded or dropped "silently", but what does
       * silently mean?  Just not closing the control connection (obviously),
       * or not providing any ICMP errors either?
       * Allowing the latter seems useful, so keep it until we have reason to
       * think it breaks things for someone.
       */
      send_icmperror(rawsocket,
                     &client->laddr,
                     &client->raddr,
                     IPADDRISBOUND(&targetaddr) ? &targetaddr : NULL,
                     -1,
                     -1,
                     icmp_type,
                     iostatus == IO_NOERROR ?
                        ICMP_CODE_UNREACHABLE_HOSTPROHIBITED : icmp_code);

      return IO_TMPERROR;
   }

   /*
    * Ok, received a packet and things look ok so far.  Now check the rules
    * for whether the packet is to be allowed through or not.
    */

   slog(LOG_DEBUG,
        "%s: packet of length %lu (payload length %lu) from client %s "
        "received on local address, %s for target %s",
        function,
        (unsigned long)r,
        (unsigned long)payloadlen,
        sockaddr2string(&client->raddr, hosta, sizeof(hosta)),
        sockaddr2string(&client->laddr, hostb, sizeof(hostb)),
        sockshost2string(&header.host, NULL, 0));

   /*
    * Do we have a saved rule from before?  If so, see if we can reuse it
    * and avoid having to do a new rule-lookup.
    */
   if (client->state.use_saved_srule) {
      int addtarget;

      SASSERTX(twotargets->dstc > 0);

      /*
       * Can reuse, but only if target is the same as last time.
       * First figure out what target object is the correct one to use,
       * so we know what the target was the last time.
       */

      switch (twotargets->dstc) {
         case 1:
            /*
             * Have one target socket created already.  Is it of the
             * same addressfamily as the packet we now received?
             */
            if (twotargets->dstv[0].raddr.ss_family == targetaddr.ss_family) {
              target    = &twotargets->dstv[0]; /* yes. */
              addtarget = 0;
            }
            else /* nope, must create a new socket for this target. */
              addtarget = 1;

            break;

         case 2:
            /*
             * Have two target sockets created already.  One of them
             * must be of the correct addressfamily.
             */
            if (twotargets->dstv[0].raddr.ss_family == targetaddr.ss_family)
              target = &twotargets->dstv[0]; /* this is the one. */
            else {
               SASSERTX(twotargets->dstv[1].raddr.ss_family
               == targetaddr.ss_family);

              target = &twotargets->dstv[1]; /* this is the one. */
            }

            addtarget = 0;
            break;

         default:
            SERRX(twotargets->dstc);
      }

      if (addtarget) {
         SASSERTX(twotargets->dstc == 1);
         SASSERTX(twotargets->dstv[0].raddr.ss_family != targetaddr.ss_family);

         slog(LOG_DEBUG,
              "%s: new to add a new targetsocket due to change of af.  "
              "Have %s, need %s",
              function,
              safamily2string(twotargets->dstv[0].raddr.ss_family),
              safamily2string(targetaddr.ss_family));

         SASSERTX(twotargets->dstv[twotargets->dstc].s == 0);

         if (initclient(control->s,
                        &client->laddr,
                        &client->raddr,
                        &header.host,
                        &targetaddr,
                        packetrule,
                        emsg,
                        sizeof(emsg),
                        &twotargets->dstv[twotargets->dstc]) != NULL) {
            target = addclient(&client->laddr,
                               &twotargets->dstv[twotargets->dstc],
                               &twotargets->dstc,
                               &twotargets->dstcmax,
                               &twotargets->dstv,
                               state,
                               packetrule);

            SASSERTX(target != NULL);
            SASSERTX(twotargets->dstc == 2);

            doconnect = sockscf.udpconnectdst;
         }
         else {
            /*
             * still have the other (first) target socket, so don't consider
             * this a fatal error.
             */
            iostatus = IO_TMPERROR;

            iolog(packetrule,
                  state,
                  IOOP(packetrule->verdict == VERDICT_BLOCK, iostatus),
                  clog,
                  dlog,
                  NULL,
                  NULL,
                  emsg,
                  strlen(emsg));

            send_icmperror(rawsocket,
                           &client->laddr,
                           &client->raddr,
                           &targetaddr,
                           -1,
                           -1,
                           icmp_type,
                           icmp_code);

            return iostatus;
         }

         sametarget = 0;
      }
      else
         sametarget = addrmatch(sockshost2ruleaddr(&target->raddrhost, NULL),
                                &header.host,
                                NULL,
                                SOCKS_UDP,
                                0);

      twotargets->s = target->s; /* update as soon as known. */

      sockaddr2sockshost(&target->laddr, &dlog->local);
      dlog->local_isset = 1;

      slog(LOG_DEBUG,
           "%s: use_saved_srule set.  UDP packet #%"PRIu64" from %s.  "
           "Destination %s.  Already set up with socks-rule #%lu (%s) "
           "for previous destination %s (%ssame as now)",
           function,
           client->read.packets + 1,
           sockaddr2string(&client->raddr, NULL, 0),
           sockshost2string(&header.host, NULL, 0),
           (unsigned long)packetrule->number,
           verdict2string(packetrule->verdict),
           sockshost2string(&target->raddrhost, hosta, sizeof(hosta)),
           sametarget ? "" : "not ");

      if (sametarget)
         permit = (packetrule->verdict == VERDICT_PASS);
      else {
         sockshost_t targethost;
         char clientstr[MAXSOCKADDRSTRING], dstbefore[MAXSOCKSHOSTSTRING],
              dstnow[MAXSOCKSHOSTSTRING];

         SASSERTX(iostatus == IO_NOERROR);

         permit = rulespermit(control->s,
                              &control->raddr,
                              &control->laddr,
                              cauth,
                              &control->auth,
                              packetrule,
                              state,
                              &client->host,
                              &header.host,
                              &targethost,
                              emsg,
                              sizeof(emsg));

         if (permit)
            header.host = targethost;

         if (!addtarget)
            slog(LOG_DEBUG,
                 "%s: destination host for packet from client %s changed "
                 "from %s to %s%s",
                 function,
                 sockaddr2string(&control->raddr, clientstr, sizeof(clientstr)),
                 sockshost2string(&target->raddrhost,
                                  dstbefore,
                                  sizeof(dstbefore)),
                 sockshost2string(&header.host,
                                  dstnow,
                                  sizeof(dstnow)),
                 permit ?
                   "" : ".  Packets to this address are not permitted however");

         /*
          * Target destination changed.
          * Unconnect the socket if it's connected and keep it unconnected,
          * so we can receive replies from the previous destination too.
          *
          * We do this regardless of whether the new destination is
          * permitted or not, as we have no reason to assume the previous
          * (permitted) target is any more likely to be the next target
          * than this (possibly, not permitted) target.  I.e., we want to
          * cache the negative rulespermit() verdict also.
          */

         if (target->isconnected) {
            sockd_unconnect(target->s, &target->raddr);
            target->isconnected = 0;
         }

         target->raddrhost = header.host;
         target->raddr     = targetaddr;

         if (permit) {
            /* possibly the socket options differ in this rule. */
            setconfsockoptions(target->s,
                               client->s,
                               SOCKS_UDP,
                               0,
                               packetrule->socketoptionc,
                               packetrule->socketoptionv,
                               SOCKETOPT_PRE | SOCKETOPT_ANYTIME,
                               0);
         }
      }

      if (permit) {
         if (redirect(target->s,
                      &target->laddr,
                      &target->raddrhost,
                      state->command,
                      &packetrule->rdr_from,
                      &packetrule->rdr_to) == 0)
            sockaddr2sockshost(&target->laddr, &dlog->local);
         else {
            snprintf(emsg, sizeof(emsg),
                     "redirect failed: %s", strerror(errno));

            if (errno != 0)
               swarnx("%s: %s", function, emsg);

            iostatus = IO_TMPERROR;
         }
      }
   }
   else {
      /*
       * No saved rule we can reuse.  Must do a new rule-lookup.
       */
      sockshost_t targethost;
      int addtarget;

      permit = rulespermit(control->s,
                           &control->raddr,
                           &control->laddr,
                           cauth,
                           &control->auth,
                           packetrule,
                           state,
                           &client->host,
                           &header.host,
                           &targethost,
                           emsg,
                           sizeof(emsg));

      if (permit) {
         /*
          * Ok, packet is permitted.  Now figure out what target struct to
          * use for forwarding this packet.
          */

         header.host = targethost;

         switch (twotargets->dstc) {
            case 0:
               /*
                * No target sockets created yet, so create the first now.
                */
               addtarget = 1;
               break;

            case 1:
               /*
                * Have one target socket created already.  Is it of the
                * same addressfamily as the target?
                */
               if (twotargets->dstv[0].raddr.ss_family
               == targetaddr.ss_family) {
                 target    = &twotargets->dstv[0]; /* yes. */
                 addtarget = 0;
               }
               else
                 addtarget = 1;

               break;

            case 2:
               /*
                * Have two target sockets created already.  One of them must
                * be of the correct addressfamily.
                */
               if (twotargets->dstv[0].raddr.ss_family == targetaddr.ss_family)
                 target = &twotargets->dstv[0]; /* this is the one. */
               else {
                  SASSERTX(twotargets->dstv[1].raddr.ss_family
                  == targetaddr.ss_family);

                 target = &twotargets->dstv[1]; /* this is the one. */
               }

               addtarget = 0;
               break;

            default:
               SERRX(twotargets->dstc);
         }

         if (addtarget) {
            SASSERTX(twotargets->dstc < 2);

            if (initclient(control->s,
                           &client->laddr,
                           &client->raddr,
                           &header.host,
                           &targetaddr,
                           packetrule,
                           emsg,
                           sizeof(emsg),
                           &twotargets->dstv[twotargets->dstc]) != NULL) {
               target = addclient(&client->laddr,
                                  &twotargets->dstv[twotargets->dstc],
                                  &twotargets->dstc,
                                  &twotargets->dstcmax,
                                  &twotargets->dstv,
                                  state,
                                  packetrule);

               SASSERTX(target != NULL);
               SASSERTX(twotargets->dstc > 0);

               doconnect = sockscf.udpconnectdst;
            }
            else
               iostatus = IO_TMPERROR;
         }

         if (target != NULL) {
            sockaddr2sockshost(&target->laddr, &dlog->local);
            dlog->local_isset = 1;
            twotargets->s     = target->s; /* update as soon as known. */
         }
      }
      else {
         /*
          * This packet was not permitted.
          */

         if (twotargets->dstc == 0) {
            /*
             * No targets sockets added yet and this packet from the client
             * was not permitted.  Is it possible other packets from this
             * client will be permitted?  Try.  If not, we can close this
             * session as there's no point in having the client keep
             * sending us packets if all of them will be blocked.
             * A bit unfortunate since we already told the client that
             * the udpassociate was ok, but that was only because it did
             * not tell us what address it would be sending us packets from.
             */
            rule_t tryrule;
            const int try_permits = rulespermit(control->s,
                                                &control->raddr,
                                                &control->laddr,
                                                cauth,
                                                &control->auth,
                                                &tryrule,
                                                state,
                                                &client->host,
                                                NULL, /* try any dst. */
                                                NULL,
                                                NULL,
                                                0);

            if (!try_permits) {
               slog(LOG_DEBUG,
                   "%s: only now we have the complete UDP client address (%s) "
                   "for the client that connected to us from %s.  "
                   "Packets from that client address are not permitted "
                   "according to current rules however, regardless of their "
                   "target destination.  We thus close this session now, "
                   "though we are unfortunately unable to tell the client why",
                   function,
                   sockaddr2string(&client->raddr, hosta, sizeof(hosta)),
                   sockaddr2string(&control->raddr, hostb, sizeof(hostb)));

               iostatus = IO_BLOCK;
            }
         }
      }

      if (iostatus != IO_NOERROR || !permit) {
         iolog(packetrule,
               state,
               IOOP(!permit, iostatus),
               clog,
               dlog,
               NULL,
               NULL,
               emsg,
               strlen(emsg));

         send_icmperror(rawsocket,
                        &client->laddr,
                        &client->raddr,
                        &targetaddr,
                        -1,
                        -1,
                        icmp_type,
                        iostatus == IO_NOERROR ?
                           ICMP_CODE_UNREACHABLE_HOSTPROHIBITED : icmp_code);

         if (iostatus != IO_NOERROR)
            return iostatus;
         else
            return IO_TMPBLOCK;
      }

      if (addtarget) {
         sametarget = 0;

         slog(LOG_DEBUG, "%s: new target: %s",
              function, sockshost2string(&header.host, hostb, sizeof(hostb)));
      }
      else {
         sametarget = addrmatch(sockshost2ruleaddr(&target->raddrhost, NULL),
                                &header.host,
                                NULL,
                                SOCKS_UDP,
                                0);

         slog(LOG_DEBUG,
              "%s: previous target: %s, current: %s (%s before)",
              function,
              sockshost2string(&target->raddrhost, hosta, sizeof(hosta)),
              sockshost2string(&header.host, hostb, sizeof(hostb)),
              sametarget ? "same as" : "different from");
      }

      if (target->isconnected && !sametarget) {
         /*
          * need to unconnect the socket so we can continue to receive
          * packets from the old destination.
          */
          sockd_unconnect(target->s, &target->raddr);
          target->isconnected = 0;
      }

      target->raddr = targetaddr;
   }

   SASSERTX(target != NULL);

   target->client_read.bytes   += recvflags.fromsocket;
   target->client_read.packets += 1;

   client->state.use_saved_srule = 0; /* XXX temporarily disabled.  Bug. */

   if (permit && iostatus == IO_NOERROR) {
      /*
       * Should we connect to the destination?
       * If the client will only be sending udp packets to one address, it
       * is more efficient to connect the socket to that address.
       * If we do that we must however be sure to unconnect the socket before
       * sending target on it again if the client wants to send to a new
       * address, and from that point on, leave the socket unconnected,
       * so that possible future packets from the address we first
       * sent/connected to will also be received.
       */

      if (doconnect) {
         slog(LOG_DEBUG, "%s: connecting fd %d to %s, for client %s",
              function,
              target->s,
              sockshost2string(&header.host, hosta, sizeof(hosta)),
              sockshost2string(&client->host, hostb, sizeof(hostb)));

         if (socks_connecthost(target->s,
                               EXTERNALIF,
                               &header.host,
                               NULL,
                               NULL,
                               (long)-1,
                               emsg,
                               sizeof(emsg)) == -1) {
            iolog(packetrule,
                  state,
                  OPERATION_TMPERROR,
                  clog,
                  dlog,
                  NULL,
                  NULL,
                  emsg,
                  strlen(emsg));

            iostatus = IO_TMPERROR;
            return iostatus;
         }

         target->isconnected = 1;
      }

      sendtoflags.side = EXTERNALIF;
      w = socks_sendto(target->s,
                       payload,
                       payloadlen,
                       0,
                       target->isconnected ?  NULL : &target->raddr,
                       target->isconnected ?
                       (socklen_t)0 : (socklen_t)sizeof(target->raddr),
                       &sendtoflags,
                       NULL);

      if (w >= 0) {
         iostatus = io_packet_sent(payloadlen,
                                   w,
                                   &recvflags.ts,
                                   &client->raddr,
                                   &target->raddr,
                                   emsg,
                                   sizeof(emsg));

         *bwused = w;

         target->target_written.bytes   += sendtoflags.tosocket;
         target->target_written.packets += 1;

         gettimeofday_monotonic(&target->lastio);

         /*
          * Also update twotargets so caller can know how much i/o was done
          * on this call, as otherwise he would have no way of knowing this
          * because he could not beforehand know what our target socket would
          * be (could even be a new target).
          */
         twotargets->written.bytes   += sendtoflags.tosocket;
         twotargets->written.packets += 1;
      }
      else {
         snprintf(emsg, sizeof(emsg), "sendto of %lu bytes failed: %s",
                  (unsigned long)payloadlen, strerror(errno));

         iostatus = IOSTATUS_UDP_SEND_FAILED(errno);
         *badfd   = target->s;
      }
   }
   else
      w = -1;

   if (iostatus != IO_NOERROR || !permit) {
      iolog(packetrule,
            state,
            IOOP(!permit, iostatus),
            clog,
            dlog,
            NULL,
            NULL,
            emsg,
            strlen(emsg));

      send_icmperror(rawsocket,
                     &client->laddr,
                     &client->raddr,
                     &targetaddr,
                     -1,
                     -1,
                     icmp_type,
                     iostatus == IO_NOERROR ?
                        ICMP_CODE_UNREACHABLE_HOSTPROHIBITED : icmp_code);

      if (iostatus != IO_NOERROR)
         return iostatus;
      else
         return IO_TMPBLOCK;
   }

   SASSERTX(w == (ssize_t)payloadlen);

   iolog(packetrule,
         state,
         OPERATION_IO,
         clog,
         dlog,
         NULL,
         NULL,
         payload,
         payloadlen);

   return IO_NOERROR;
}

iostatus_t
io_udp_target2client(control, client, twotargets, state,
                     clog, dlog, badfd, packetrule, bwused)
   sockd_io_direction_t *control;
   sockd_io_direction_t *client;
   sockd_io_direction_t *twotargets;
   connectionstate_t *state;
   iologaddr_t *clog;
   iologaddr_t *dlog;
   int *badfd;
   rule_t *packetrule;
   size_t *bwused;
{
   /*
    * Received a reply from one of the up to two twotargets sockets (one for
    * ipv4 and one for ipv6).
    * When called we already know what the twotargets socket is and things
    * should be set up correctly.
    */
   const char *function = "io_udp_target2client()";
   iostatus_t iostatus = IO_NOERROR;
   udptarget_t *target = NULL;
   sendto_info_t sendtoflags;
   recvfrom_info_t recvflags;
   struct sockaddr_storage from, hostaddr;
   socklen_t len;
   ssize_t w, r;
   size_t headerlen, payloadlen, emsglen, originallen;
   char hosta[MAXSOCKSHOSTSTRING], hostb[MAXSOCKSHOSTSTRING], emsg[1024],
        buf[SOCKD_BUFSIZE + sizeof(udpheader_t)], *payload;
   int samesrc, permit = 0,
       icmp_type = ICMP_TYPE_UNREACHABLE,
       icmp_code = ICMP_CODE_UNREACHABLE_PORT;

   target = clientofsocket(twotargets->s, twotargets->dstc, twotargets->dstv);
   SASSERTX(target != NULL);

   *emsg = NUL;

   recvflags.side = EXTERNALIF;
   recvflags.peer = twotargets->raddr;
   recvflags.type = SOCK_DGRAM;

   len = sizeof(from);
   if ((r = socks_recvfrom(twotargets->s,
                           buf,
                           sizeof(buf),
                           0,
                           &from,
                           &len,
                           &recvflags,
                           &twotargets->auth)) == -1) {
      if (ERRNOISPREVIOUSPACKET(errno)) {
         /*
          * error is from the target of an earlier packet from client,
          * sent by us /out/ on this socket, i.e. from client to target.
          * Note that Linux apparently can return this error even if
          * the socket is not connected.
          *
          * Don't treat it as fatal, more packets could come, and they
          * may be accepted by the twotargets.
          */
         const int oldcommand = state->command;
         state->command       = SOCKS_UDPASSOCIATE;

         *badfd = client->s;

         emsglen = snprintf(emsg, sizeof(emsg),
                            "%s (delayed error)", strerror(errno));

         iolog(packetrule,
               state,
               OPERATION_TMPERROR,
               clog,
               dlog,
               NULL,
               NULL,
               emsg,
               emsglen);

         state->command = oldcommand;

         send_icmperror(rawsocket,
                        &client->laddr,
                        &client->raddr,
                        &twotargets->raddr,
                        -1,
                        -1,
                        icmp_type,
                        icmp_code);

         /* tmp error; using control connection do detect fatal errors. */
         return IO_TMPERROR;
      }

      *badfd = twotargets->s;

      if (ERRNOISTMP(errno))
         return IO_EAGAIN;
      else {
         log_unexpected_udprecv_error(function,
                                      twotargets->s,
                                      errno,
                                      EXTERNALIF);
         return IO_IOERROR;
      }
   }

   iostatus = io_packet_received(&recvflags, r, &from, &twotargets->laddr);

   gettimeofday_monotonic(&target->lastio);

   if (iostatus != IO_NOERROR) {
      *badfd = twotargets->s;
      return iostatus;
   }

   /*
    * Read a packet.  Now check whether it should be forwarded.
    */

   payloadlen                   = r;

   target->target_read.bytes   += recvflags.fromsocket;
   target->target_read.packets += 1;

   slog(LOG_DEBUG,
        "%s: udp packet #%"PRIu64" from twotargets %s received for client %s "
        "on fd %d, packet length is %ld, use_saved_srule is %d",
        function,
        twotargets->read.packets,
        sockaddr2string(&from, hosta, sizeof(hosta)),
        sockaddr2string(&client->raddr, hostb, sizeof(hostb)),
        twotargets->s,
        (long)r,
        twotargets->state.use_saved_srule);

   if (twotargets->written.packets == 0) {
      slog(LOG_DEBUG,
           "%s: unusual ... the client at %s has not sent any packets "
           "yet, but is receiving a %ld byte reply on address %s from %s",
           function,
           sockaddr2string(&client->raddr, NULL, 0),
           (long)r,
           sockaddr2string(&twotargets->laddr, hosta, sizeof(hosta)),
           sockaddr2string(&from, hostb, sizeof(hostb)));
   }

   /*
    * Can we reuse the saved rule, if any, from the last time we received a
    * packet on this socket, or must we do a new rule-lookup now?
    */
   if (twotargets->state.use_saved_srule) {
      /*
       * Yes, previous rule lookup is valid, but if the packet is received
       * from the same address as previously.
       */
      if (twotargets->state.isconnected) {
         slog(LOG_DEBUG, "%s: socket is connected.  Kernel should take care "
                         "of making sure the source address %s is the same "
                         "as last, so that we can reuse previous rule-lookup "
                         "(rule #%lu, verdict: %s)",
                         function,
                         sockaddr2string(&from, NULL, 0),
                         (unsigned long)packetrule->number,
                         verdict2string(packetrule->verdict));
         samesrc = 1;
      }
      else {
         if (IPADDRISBOUND(&twotargets->raddr)) {
            if (! (samesrc = sockaddrareeq(&twotargets->raddr, &from, 0))) {
               /*
                * ack, not reply from previous twotargets.
                */
               slog(LOG_DEBUG,
                    "%s: use_saved_srule set.  UDP packet #%"PRIu64" from %s.  "
                    "Destination %s.  Previously set up with "
                    "%s #%lu (%s) for packets from twotargets %s.  "
                    "Not the same as now, so have to do a new rule-lookup",
                    function,
                    twotargets->read.packets,
                    sockaddr2string(&from, hosta, sizeof(hosta)),
                    sockshost2string(&client->host, hostb, sizeof(hostb)),
                    objecttype2string(packetrule->type),
                    (unsigned long)packetrule->number,
                    verdict2string(packetrule->verdict),
                    sockaddr2string(&twotargets->raddr, NULL, 0));
            }
         }
         else
            samesrc = 0; /* have nothing we should compare with. */
      }
   }
   else
      samesrc = 0; /* have nothing we should compare with. */

   if (samesrc)
      permit = (packetrule->verdict == VERDICT_PASS);
   else {
      /*
       * Nope, can not reuse the last rule lookup.  Have to do a new one.
       */

      twotargets->raddr = from;

      sockaddr2sockshost(&twotargets->raddr, &twotargets->host);
      dlog->peer        = twotargets->host;

      slog(LOG_DEBUG,
           "%s: received packet from %s to %s on fd %d, "
           "use_saved_srule is %d, samesrc is %d",
           function,
           sockaddr2string(&twotargets->raddr, hosta, sizeof(hosta)),
           sockaddr2string(&client->raddr, hostb, sizeof(hostb)),
           twotargets->s,
           twotargets->state.use_saved_srule,
           samesrc);

      if (!twotargets->state.use_saved_srule) {
         if (client->written.packets > 0)
         slog(LOG_DEBUG, "%s: not first packet and use_saved_srule not set "
                         "... should only happen after SIGHUP",
                         function);
      }

      SASSERTX(iostatus == IO_NOERROR);

      permit = rulespermit(control->s,
                           &control->raddr,
                           &control->laddr,
                           NULL,
                           &twotargets->auth,
                           packetrule,
                           state,
                           &twotargets->host,
                           &client->host,
                           NULL,
                           NULL,
                           0);

      if (permit) {
         /* use redirected addresses, if applicable. */
         /*
          * XXX make sure this does not change the address of s, as
          * we need to continue receiving packets on that socket.
          */
         struct sockaddr_storage p;

         if (redirect(client->s,
                      &client->laddr,
                      &client->host,
                      state->command,
                      &packetrule->rdr_from,
                      &packetrule->rdr_to) == 0) {

            /*
             * Use ipaddresses when logging.
             */

            sockaddr2sockshost(&client->laddr, &clog->local);
            sockaddr2sockshost(sockshost2sockaddr(&client->host, &p),
                               &clog->peer);
         }
         else {
            snprintf(emsg, sizeof(emsg),
                     "redirect failed: %s", strerror(errno));

            iostatus = IO_TMPERROR;
         }
      }
   }

   if (iostatus != IO_NOERROR || !permit) {
      iolog(packetrule,
            state,
            IOOP(!permit, iostatus),
            dlog,
            clog,
            NULL,
            NULL,
            emsg,
            strlen(emsg));

      send_icmperror(rawsocket,
                     &twotargets->laddr,
                     &twotargets->raddr,
                     sockshost2sockaddr(&client->host, &hostaddr),
                     -1,
                     -1,
                     icmp_type,
                     iostatus == IO_NOERROR ?
                        ICMP_CODE_UNREACHABLE_HOSTPROHIBITED : icmp_code);

      if (iostatus != IO_NOERROR)
         return iostatus;
      else
         return IO_TMPBLOCK;
   }

   twotargets->state.use_saved_srule = 0; /* XXX temporarily disabled.  Bug. */

   sendtoflags.side = INTERNALIF;

   originallen = payloadlen;
   payload = udpheader_add(sockaddr2sockshost(&twotargets->raddr, NULL),
                           buf,
                           &payloadlen,
                           sizeof(buf));

   SASSERTX(payload == buf);
   SASSERTX(payloadlen > (size_t)r);
   SASSERTX(payloadlen - originallen >= MINSOCKSUDPHLEN);

   headerlen = payloadlen - originallen;

   w = socks_sendto(client->s,
                    payload,
                    payloadlen,
                    0,
                    client->state.isconnected ?  NULL : &client->raddr,
                    client->state.isconnected ?
                        (socklen_t)0 : (socklen_t)sizeof(client->raddr),
                    &sendtoflags,
                    &client->auth);

   if (w >= 0) {
      iostatus = io_packet_sent(payloadlen,
                                w,
                                &recvflags.ts,
                                &twotargets->raddr,
                                &client->raddr,
                                emsg,
                                sizeof(emsg));

      *bwused = w;

      target->client_written.bytes   += sendtoflags.tosocket;
      target->client_written.packets += 1;
   }
   else {
      snprintf(emsg, sizeof(emsg), "sendto of %lu bytes failed: %s",
               (unsigned long)payloadlen, strerror(errno));

      iostatus = IOSTATUS_UDP_SEND_FAILED(errno);
      *badfd   = client->s;
   }

   if (iostatus != IO_NOERROR) {
      iolog(packetrule,
            state,
            IOOP(!permit, iostatus),
            dlog,
            clog,
            NULL,
            NULL,
            emsg,
            strlen(emsg));

      send_icmperror(rawsocket,
                     &twotargets->laddr,
                     &twotargets->raddr,
                     sockshost2sockaddr(&client->host, &hostaddr),
                     -1,
                     -1,
                     icmp_type,
                     icmp_code);

      return iostatus;
   }

   SASSERTX(w == (ssize_t)payloadlen);

   iolog(packetrule,
         state,
         OPERATION_IO,
         dlog,
         clog,
         NULL,
         NULL,
         payload    + headerlen,
         payloadlen - headerlen);

   return IO_NOERROR;
}


static int
fromaddr_as_expected(expected, from, emsg, emsglen)
   struct sockaddr_storage *expected;
   const struct sockaddr_storage *from;
   char *emsg;
   size_t emsglen;
{
   const char *function = "fromaddr_as_expected()";
   char expectedstr[MAXSOCKADDRSTRING], fromstr[MAXSOCKADDRSTRING],
        buf[sizeof(expectedstr) + sizeof(fromstr) + 256];
   int matches;

   snprintf(buf, sizeof(buf),
            "expected udp packet from clientaddress %s, got it from %s",
            sockaddr2string(expected, expectedstr, sizeof(expectedstr)),
            sockaddr2string(from, fromstr, sizeof(fromstr)));

   if (!ADDRISBOUND(expected)) {
      /*
       * Client hasn't sent us it's complete address yet, but if
       * the parts of the address it has sent (if any) matches
       * the source of this packet, we have to assume this packet
       * is from it.  We can then update the expected address with
       * complete info, based on this packet.
       *
       * We also connect the socket to the client, for better performance,
       * for receiving errors from sendto(2), for getpeername(2) by libwrap
       * in rulespermit(), for ...  well, that's reasons enough already.
       */
       struct sockaddr_storage test;

       sockaddrcpy(&test, expected, sizeof(test));

      if (!IPADDRISBOUND(expected))
         SET_SOCKADDRADDR(&test,   GET_SOCKADDRADDR(from));

      if (!PORTISBOUND(expected))
         SET_SOCKADDRPORT((&test), GET_SOCKADDRPORT(from));

      matches = sockaddrareeq(&test, from, 0);

      if (matches) {
         /*
          * from-address matches the parts the client told us, so
          * assume this packet is from the client and update expected
          * address to contain the complete address, both ip and port.
          */
         sockaddrcpy(expected, &test, sizeof(test));
      }
      /* else; no match, presumably not from client. */
   }
   else /* clients complete address is known to us.  Matches this packet? */
      matches = sockaddrareeq(expected, from, 0);

   if (!matches && emsglen > 0) {
      strncpy(emsg, buf, emsglen - 1);
      emsg[emsglen - 1] = NUL;
   }

   slog(LOG_DEBUG, "%s: %s: %s",
        function, buf, matches ? "matches" : "no match");

   return matches;
}

udpheader_t *
getudptarget(buf, buflen, header, headerlen, emsg, emsglen)
   const char *buf;
   const size_t buflen;
   udpheader_t *header;
   size_t *headerlen;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "getudptarget()";

   if (string2udpheader(buf, buflen, header) == NULL) {
      snprintf(emsg, emsglen,
               "SOCKS protocol error in socks udp packet of length %lu.  "
               "Could not extract valid address from SOCKS UDP header",
               (unsigned long)buflen);

      header->host.atype = SOCKS_ADDR_NOTSET;
      return NULL;
   }

   if (header->frag != 0) {
      snprintf(emsg, emsglen, "fragmented socks udp packets are not supported");
      return NULL;
   }

   *headerlen = HEADERSIZE_UDP(header);

   switch (header->host.atype) {
      case SOCKS_ADDR_IPV6:
         if (IN6_IS_ADDR_V4MAPPED(&header->host.addr.ipv6.ip)) {
            /*
             * We never use that; either we have IPv4 available on the
             * external interface, or we do not.  Convert it to an ordinary
             * IPv4 address and proceed as usual.
             */
            sockshost_t convertedhost;

            convertedhost = header->host;
            ipv4_mapped_to_regular(&header->host.addr.ipv6.ip,
                                   &convertedhost.addr.ipv4);

            header->host.atype     = SOCKS_ADDR_IPV4;
            header->host.addr.ipv4 = convertedhost.addr.ipv4;
         }

         /* FALLTHROUGH */

      case SOCKS_ADDR_IPV4:
         if (!external_has_safamily(atype2safamily(header->host.atype))) {
            snprintf(emsg, emsglen,
                     "packet for %s target, but no %s configured for "
                     "our usage on the external interface ",
                     atype2string(header->host.atype),
                     atype2string(header->host.atype));

            return NULL;
         }

         break;

      case SOCKS_ADDR_DOMAIN:
         break;

      default:
         SERRX(header->host.atype);
   }

   return header;
}
