/*
 * Copyright (c) 2012, 2013
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
"$Id: sockd_icmp.c,v 1.23 2013/10/27 15:24:42 karls Exp $";

extern int freefds;

void
send_icmperror(s, receivedonaddr, originalpacketsrc, packettarget,
               iplen, udplen, type, code)
   int s;
   const struct sockaddr_storage *receivedonaddr;
   const struct sockaddr_storage *originalpacketsrc;
   const struct sockaddr_storage *packettarget;
   const int iplen;
   const int udplen;
   const int type;
   const int code;
{
   const char *function = "send_icmperror()";
   socklen_t len;
   struct ip *ip;
   struct icmp *icmp;
   struct udphdr *udp;
   ssize_t rc;
   char pstr[MAXSOCKADDRSTRING], lstr[MAXSOCKADDRSTRING],
        tstr[MAXSOCKADDRSTRING];
   union { /* for alignment reasons. */
        char        data[MIN_ICMPUNREACHLEN];
        struct icmp icmp;
        struct ip   ip;
   } packet;

   slog(LOG_DEBUG,
        "%s: should send icmp type/code %d/%d out on fd %d to %s, "
        "concerning previously sent/received packet for %s having "
        "iplen %d and udplen %d, originally received on %s.  ",
        function,
        type,
        code,
        s,
        originalpacketsrc == NULL ?
            "0.0.0.0" : sockaddr2string(originalpacketsrc, pstr, sizeof(pstr)),
        packettarget == NULL ?
            "0.0.0.0" : sockaddr2string(packettarget, tstr, sizeof(tstr)),
        iplen,
        udplen,
        receivedonaddr == NULL ?
            "0.0.0.0" : sockaddr2string(receivedonaddr, lstr, sizeof(lstr)));

   if (s == -1) {
      slog(LOG_DEBUG,
           "%s: can not send icmp error to %s: have no raw socket to send on",
           function,
           originalpacketsrc == NULL ?
            "0.0.0.0" : sockaddr2string(originalpacketsrc, pstr, sizeof(pstr)));

      return;
   }

   if (originalpacketsrc == NULL
   ||  receivedonaddr    == NULL
   ||  packettarget      == NULL)
      return;

   len = salen(originalpacketsrc->ss_family);

   if (originalpacketsrc->ss_family != AF_INET) {
      slog(LOG_DEBUG,
           "%s: can not send icmp error to %s: no support for %s here yet",
           function,
           sockaddr2string(originalpacketsrc, NULL, 0),
           safamily2string(originalpacketsrc->ss_family));

      return;
   }

   icmp             = (struct icmp *)&packet.icmp;
   icmp->icmp_type  = type;
   icmp->icmp_code  = code;
   icmp->icmp_cksum = 0;

   /* four unused bytes before ipheader; must be zero. */
   bzero((char *)icmp + 4, 4);
   ip          = (struct ip *)icmp->icmp_data;
   ip->ip_hl   = MIN_IPHLEN >> 2; /* number of 32bit words. */
   ip->ip_v    = 4;
   ip->ip_tos  = 0;
   ip->ip_len  = htons(iplen == -1 ? MIN_IPHLEN : iplen);
   ip->ip_id   = 0;
   ip->ip_off  = 0;
   ip->ip_ttl  = 1;
   ip->ip_p    = IPPROTO_UDP;
   ip->ip_sum  = 0;
   ip->ip_src  = TOCIN(originalpacketsrc)->sin_addr;
   ip->ip_dst  = TOCIN(receivedonaddr)->sin_addr;
   ip->ip_sum  = in_cksum((uint16_t *)ip, sizeof(*ip));

   udp = (struct udphdr *)((char *)icmp + 8 + (ip->ip_hl << 2));
   *udphdr_uh_sport(udp) = TOCIN(originalpacketsrc)->sin_port;
   *udphdr_uh_dport(udp) = TOCIN(receivedonaddr)->sin_port;
   *udphdr_uh_ulen(udp)  = htons(udplen == -1 ? MIN_UDPLEN : udplen);
   *udphdr_uh_sum(udp)   = 0; /* don't know, but once upon a time zero was ok */

   icmp->icmp_cksum = in_cksum((uint16_t *)icmp, sizeof(packet));

   if ((rc = sendto(s,
                    &packet.data,
                    sizeof(packet.data),
                    0,
                    TOCSA(originalpacketsrc),
                    len)) != sizeof(packet))
      swarn("%s: could not send raw packet of length %lu to %s.  Sent %ld",
            function,
            (unsigned long)sizeof(packet),
            sockaddr2string(originalpacketsrc, NULL, 0),
            (long)rc);
   else
      slog(LOG_DEBUG, "%s: sent raw packet of length %lu to %s",
            function,
            (unsigned long)sizeof(packet),
            sockaddr2string(originalpacketsrc, NULL, 0));
}

#if BAREFOOTD
rawsocketstatus_t
rawsocket_recv(s, ioc, iov)
   const int s;
   const size_t ioc;
   sockd_io_t iov[];
{
   const char *function = "rawsocket_recv()";
   const size_t maxiterations = 10; /* icmp errors are not the priority. */
   rawsocketstatus_t rc;
   union { /* for alignment-reasons. */
      char         data[MAX_IPHLEN + MAX_ICMPUNREACHLEN];
      struct ip    ip;
      struct icmp  icmp;
   } packet;
   udptarget_t *client;
   struct icmp *icmp;
   struct ip *ip;
   struct udphdr *udp;
   struct sockaddr_storage from, srcaddr, dstaddr;
   socklen_t addrlen;
   ssize_t r;
   size_t ioi, iterations;
   char  fromstr[MAXSOCKADDRSTRING],
         srcstr[MAXSOCKADDRSTRING], dststr[MAXSOCKADDRSTRING];
   int send_icmp_to_client = 0, send_icmp_to_target = 0;

   rc = RAWSOCKET_NOP;
   for (iterations = 0; iterations < maxiterations; ++iterations) {
      addrlen = sizeof(from);
      if ((r = recvfrom(s,
                        &packet.data,
                        sizeof(packet.data),
                        0,
                        TOSA(&from),
                        &addrlen)) == -1) {
         if (!ERRNOISTMP(errno))
            swarn("%s: recvfrom() on raw socket (fd %d) failed", function, s);

         break;
      }

      if (r < MIN_IPHLEN + MIN_ICMPUNREACHLEN) {
         slog(LOG_DEBUG,
              "%s: packet received from %s on raw socket is too short to be of "
              "interest (%ld/%lu)",
              function,
              sockaddr2string(&from, NULL, 0),
              (long)r,
              (unsigned long)(MIN_IPHLEN + MIN_ICMPUNREACHLEN));

         continue;
      }

      ip  = &packet.ip;
      if (r < (ip->ip_hl << 2)) {
         swarn("%s: strange ... kernel says ip header length in the packet "
               "from %s is %u bytes long, but read packet size is only %ld.  "
               "Are we reading too little?  (Tried to read up to %lu bytes)",
               function,
               sockaddr2string(&from, NULL, 0),
               (unsigned)(ip->ip_hl << 2),
               (long)r,
               (unsigned long)sizeof(packet));

         continue;
      }

      icmp = (struct icmp *)(packet.data + (ip->ip_hl << 2));
      if (r - (ip->ip_hl << 2) < MIN_ICMPUNREACHLEN) {
         slog(LOG_DEBUG,
               "%s: icmp data in packet received from %s on raw socket is too "
               "short to be of interest (%ld/%lu)",
               function,
               sockaddr2string(&from, NULL, 0),
               (long)(r - (ip->ip_hl << 2)),
               (unsigned long)MIN_ICMPUNREACHLEN);

         continue;
      }

      /* ip-packet the icmp error is in reply to. */
      ip = (struct ip *)(icmp->icmp_data);

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG,
              "%s: received raw packet from %s, type/code %d/%d, ip proto %u, "
              "total length %ld, ip_hl %lu, icmp len %lu",
              function,
              inet_ntop(from.ss_family,
                        GET_SOCKADDRADDR(&from),
                        fromstr,
                        sizeof(fromstr)),
              icmp->icmp_type,
              icmp->icmp_code,
              (unsigned)ip->ip_p,
              (long)r,
              (unsigned long)(ip->ip_hl << 2),
              (unsigned long)(r - (ip->ip_hl << 2)));

      if (icmp->icmp_type != ICMP_UNREACH)
         continue;

      if (ip->ip_p != IPPROTO_UDP)
         continue;

      bzero(&srcaddr, sizeof(srcaddr));
      SET_SOCKADDR(&srcaddr, AF_INET);
      dstaddr = srcaddr;

      udp                    = (struct udphdr *)((char *)ip + (ip->ip_hl << 2));
      TOIN(&srcaddr)->sin_addr = ip->ip_src;
      TOIN(&srcaddr)->sin_port = *udphdr_uh_sport(udp);
      TOIN(&dstaddr)->sin_addr = ip->ip_dst;
      TOIN(&dstaddr)->sin_port = *udphdr_uh_dport(udp);

      slog(LOG_DEBUG, "%s: icmp packet is in relation to packet from %s to %s",
           function,
           sockaddr2string(&srcaddr, srcstr, sizeof(srcstr)),
           sockaddr2string(&dstaddr, dststr, sizeof(dststr)));

      /*
       * Figure out is this icmp error is related to a packet we sent.
       * Two possibilities if so:
       *    1: Response to an udpreply forwarded by us from target to client:
       *       - Dstaddr in error packet should match a client addr.
       *       - Srcaddr should match a address used by the client at the given
       *         dstaddr.
       *
       *    2: Response to an udp packet forwarded by us from client to target:
       *       - Dstaddr in error packet should match a target address.
       *       - Srcaddr should match a laddr we use for forwarding packets
       *         to the given target address.
       */

      client = NULL;
      for (ioi = 0; ioi < ioc; ++ioi) {
         size_t dsti;

         if (iov[ioi].state.protocol != SOCKS_UDP)
            continue;

         /*
          * Check possibility 1 first.
          */
         if ((client = clientofclientaddr(&dstaddr,
                                          iov[ioi].dst.dstc,
                                          iov[ioi].dst.dstv)) != NULL) {
            const int matches = sockaddrareeq(&srcaddr, &iov[ioi].src.laddr, 0);

            slog(LOG_DEBUG,
                 "%s: dstaddr %s matches a client address, srcaddress %s %s an "
                 "address we listen on",
                 function,
                 sockaddr2string(&dstaddr, dststr, sizeof(dststr)),
                 sockaddr2string(&srcaddr, srcstr, sizeof(srcstr)),
                 matches ? "also matches" : "however does not");

            if (matches) {
               send_icmp_to_target = 1;
               break;
            }
            else
               client = NULL;
         }

         SASSERTX(client == NULL);

         /*
          * Nope.  How about possibility 2)?
          */
         for (dsti = 0; dsti < iov[ioi].dst.dstc; ++dsti) {
            SASSERTX(iov[ioi].dst.dstv[dsti].s != -1);

            if (!sockaddrareeq(&iov[ioi].dst.dstv[dsti].raddr, &dstaddr, 0))
               continue;

            if (!sockaddrareeq(&srcaddr, &iov[ioi].dst.dstv[dsti].laddr, 0))
               continue;

            client = &iov[ioi].dst.dstv[dsti];
            break;
         }

         if (client != NULL) { /* found our match. */
            SASSERTX(dsti < iov[ioi].dst.dstc);
            send_icmp_to_client = 1;
            break;
         }
      }

      if (client != NULL) {
         struct sockaddr_storage *receivedonaddr, *icmptargetaddr;

         SASSERTX(ioi < ioc);

         SASSERTX(send_icmp_to_client || send_icmp_to_target);
         SASSERTX(!(send_icmp_to_client && send_icmp_to_target));

         io_syncudp(&iov[ioi], client);

         if (send_icmp_to_target) {
            /*
             * error is related to case 1 (packet from target to client).
             */
            SASSERTX(sockaddrareeq(&srcaddr, &iov[ioi].src.laddr, 0));
            SASSERTX(sockaddrareeq(&dstaddr, &client->client, 0));
            receivedonaddr = &iov[ioi].dst.laddr;
            icmptargetaddr = &iov[ioi].dst.raddr;
         }
         else {
            /*
             * error is related to case 2 (packet from client to target).
             */
            SASSERTX(send_icmp_to_client);

            SASSERTX(sockaddrareeq(&srcaddr, &iov[ioi].dst.laddr, 0));
            SASSERTX(sockaddrareeq(&dstaddr, &iov[ioi].dst.raddr, 0));
            receivedonaddr = &iov[ioi].src.laddr;
            icmptargetaddr = &iov[ioi].src.raddr;
         }

         slog(LOG_DEBUG, "%s: packet received from %s is related to client %s",
              function,
              sockaddr2string(&from, fromstr, sizeof(fromstr)),
              sockaddr2string(&iov[ioi].src.raddr, NULL, 0));

         send_icmperror(s,
                        receivedonaddr,
                        icmptargetaddr,
                        &dstaddr,
                        ntohs(ip->ip_len),
                        ntohs(*udphdr_uh_ulen(udp)),
                        icmp->icmp_type,
                        icmp->icmp_code);

         slog(LOG_DEBUG, "%s: removing client %s from iov #%lu",
              function,
              sockaddr2string(&iov[ioi].src.raddr, NULL, 0),
              (unsigned long)ioi);

         io_delete(-1 /* nothing to ack */, &iov[ioi], client->s, IO_CLOSE);
         rc = RAWSOCKET_IO_DELETED;

         continue;
      }

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG,
              "%s: icmp error received from %s refers to packet from %s to "
              "%s.  Not a session known to us",
              function,
              sockaddr2string(&from, fromstr, sizeof(fromstr)),
              sockaddr2string(&srcaddr, srcstr, sizeof(srcstr)),
              sockaddr2string(&dstaddr, dststr, sizeof(dststr)));

   }

   return rc;
}
#endif /* BAREFOOTD */
