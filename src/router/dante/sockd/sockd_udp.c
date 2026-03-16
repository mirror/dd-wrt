/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2014, 2024
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
"$Id: sockd_udp.c,v 1.58.4.4.14.2 2024/11/20 22:05:42 karls Exp $";

extern int       rawsocket;
extern iostate_t iostate;

#if HAVE_SO_TIMESTAMP
struct iostats {
   iostat_t ro;    /* read-only stats.    */
   iostat_t rw;    /* read/write stats.   */
} iostats;


static iostat_t *
io_updatestat(iostat_t *iostat);
/*
 * Updates (recalculates) the info in "iostat".
 * Returns a pointer to the updated iostats, or NULL if information
 * is not yet available.
 */


static int
io_timercmp(const void *a, const void *b);
/*
 * Comparison function compatible with qsort.
 * "a" and "b" are pointers to struct timeval.
 */

#endif /* HAVE_SO_TIMESTAMP */

iostatus_t
doio_udp(io, rset, badfd)
   sockd_io_t *io;
   fd_set *rset;
   int *badfd;
{
   /*
    * In the socks case the client-side of udp i/o is always fixed.
    * In barefoot the client-side can/will vary for each packet,
    * as one socket can receive packets from multiple clients.
    *
    * Also note that we are less strict about bandwidth in the udp
    * case since we can't truncate packets.  Therefore we don't limit
    * the amount of i/o we do in one go for the udp-case; it has to be
    * whole packets.  Trying to buffer udp packets would probably be
    * suboptimal latency-wise, even if we could attempt to do it by
    * expanding the iobuf-mechanism to handle udp also.
    *
    * In both Barefoot and Dante we need to do both a rulespermit()
    * per packet, but we also need to save the original rule that
    * allowed the udp session (in Barefoot's case, we fake the session).
    *
    * In Dante, the "original" rule is the socks-rule that matched the
    * clients's udpassociate request, or the first udp packet if the address
    * in the request was not exact.  In Barefoot it is the client-rule that
    * was used to generate the corresponding socks-rule.
    *
    * E.g., the following will work correctly since only one of the
    * rules below can be assigned to any udp client in Barefoot and the
    * assignment is fixed (until SIGHUP at least):
    *
    * client pass { from: 10.1/16 to: 10.1.1.1/32 port = echo
    *               bounce to: 10.10.10.1 port = echo
    *               maxsessions: 10 }
    *
    * client pass { from: 10.2/16 to: 10.1.1.1/32 port = echo
    *               bounce to: 10.10.10.2 port = echo
    *               maxsessions: 1 }
    *
    * Correspondingly, in Dante, the below will not work as the
    * socks udp client can change it's destination at any time.
    *
    * pass { from: 10/8 to: 10.1.1.1/32 port = echo
    *        maxsessions: 10 }
    *
    * pass { from: 10/8 to: 10.1.1.2/32 port = echo
    *        maxsessions: 1 }
    *
    * E.g., assume that the client first sends packets matching rule
    * #1, having a maxsessions value of 10.  It then sends a packet
    * matching rule #2, and then a packet matching rule #1 again.
    * Do we then allocate two sessions to this client, one from rule #1
    * and one from rule #2?  Obviously not.
    * Do we move the client from the session belonging to rule #1 when it
    * sends a packet matching rule #2, and then move it back again from
    * rule #2 to rule #1?  Also obviously not.
    * Instead we do the simple thing and and lock the resources when the
    * udp session is established, which will be in the request child.
    */
   const char *function   = "doio_udp()";
   iostatus_t iostatus;
   size_t sideschecked;
   iologaddr_t src, dst;

#if BAREFOOTD

   rule_t packetrule;

#else /* SOCKS_SERVER */

   rule_t *packetrule;

#endif /* SOCKS_SERVER */


   SASSERTX(io->allocated);
   SASSERTX(io->state.protocol == SOCKS_UDP);

#if DIAGNOSTIC /* no buffering of udp should be done. */
   SASSERTX(socks_bufferhasbytes(io->src.s, WRITE_BUF) == 0);
   SASSERTX(socks_bufferhasbytes(io->src.s, READ_BUF)  == 0);
   SASSERTX(socks_bufferhasbytes(io->dst.s, WRITE_BUF) == 0);
   SASSERTX(socks_bufferhasbytes(io->dst.s, READ_BUF)  == 0);
#endif /* DIAGNOSTIC */

   init_iologaddr(&src,
                  object_sockaddr,
                  &io->src.laddr,
                  object_sockaddr,
                  &io->src.raddr,
                  &io->src.auth,
                  NULL);

   init_iologaddr(&dst,
                  object_sockaddr,
                  io->dst.s == -1 ? NULL : &io->dst.laddr,
                  object_sockaddr,
                  io->dst.s == -1 ? NULL : &io->dst.raddr,
                  io->dst.s == -1 ? NULL : &io->dst.auth,
                  NULL);

   slog(LOG_DEBUG, "%s: control-fd %d, src-fd %d, dst-fd %d",
        function, io->control.s, io->src.s, io->dst.s);

   if (!timerisset(&io->state.time.firstio))
      gettimeofday_monotonic(&io->state.time.firstio);

   errno  = 0; /* reset before each call. */
   *badfd = -1;

   iostatus     = IO_EAGAIN;
   sideschecked = 0;

   /*
    * UDP to relay from client to destination?
    */

   if (FD_ISSET(io->src.s, rset)) {
      const int originaldst = io->dst.s;
      iocount_t src_read    = io->src.read,
                src_written = io->src.written,
                dst_read    = io->dst.read,
                dst_written = io->dst.written;
      iologaddr_t lsrc = src, ldst = dst;
      size_t bwused = 0;

      ++sideschecked;

      /*
       * Don't yet know what the target address/socket used will be.
       * If any data is actually to be forwarded to a target, this will
       * be updated based on the target.
       */
      io->dst.s = -1;

#if BAREFOOTD
      /*
       * initalize to crule, and if we get far enough, rulespermit() will
       * update it, possibly changing it to another crule based on the source
       * address of the client who sent the packet we will read.
       *
       * Since we want to be sure to not change the original crule,
       * don't use a pointer for packetrule.
       */
      packetrule = io->crule;

      slog(LOG_DEBUG, "%s: client2target i/o on fd %d -> fd %d",
           function, io->src.s, io->dst.s);

      iostatus = io_udp_client2target(&io->src,
                                      &io->dst,
                                      &io->cauth,
                                      &io->state,
                                      &lsrc,
                                      &ldst,
                                      badfd,
                                      &packetrule,
                                      &bwused);

#else /* SOCKS_SERVER */

      /*
       * Default to packetrule being the rule matched for this udp session,
       * unless we've progressed far enough to have a previously saved rule
       * to use.
       */
      if (!io->src.state.use_saved_srule)
         *io->cmd.udp.sfwdrule = io->srule; /* reset to initial match. */

      packetrule = io->cmd.udp.sfwdrule;

      slog(LOG_DEBUG, "%s: client2target i/o on fd %d -> {fd %d, fd %d}",
           function, io->src.s, io->dst.dstv[0].s, io->dst.dstv[1].s);

      iostatus = io_udp_client2target(&io->control,
                                      &io->src,
                                      &io->dst,
                                      &io->cauth,
                                      &io->state,
                                      &lsrc,
                                      &ldst,
                                      badfd,
                                      packetrule,
                                      &bwused);
#endif /* SOCKS_SERVER */

      if (IOSTATUS_FATALERROR(iostatus))
         /* can not be sure the error did not affect dst, so return now. */
         return iostatus;

      if (iostatus == IO_NOERROR) {
         DO_IOCOUNT(&src_read,
                    &src_written,
                    &dst_read,
                    &dst_written,
                    io);

         io_update(&io->lastio,
                   bwused,
                   &src_read,
                   NULL,
                   NULL,
                   &dst_written,

#if BAREFOOTD
                   &packetrule,
                   &packetrule,

#else /* SOCKS_SERVER */

                   &io->srule,
                   packetrule,

#endif /* SOCKS_SERVER */

                   sockscf.shmemfd);

          /*
           * client2target will have changed dst to the object associated with
           * the client it read the packet from, possibly a brand new dst.
           */
         io->dst.s = originaldst;
      }
      else
         slog(LOG_DEBUG, "%s: some non-fatal error, iostatus = %d",
              function, (int)iostatus);
   }

   /*
    * Datagram reply from target present?
    */

   if (io->dst.s != -1 && FD_ISSET(io->dst.s, rset)) {
      /*
       * - io->src is dst of packet (our client).
       * - io->dst is (presumably) one of client's target and can vary for
       *   each packet.
       */
      iocount_t src_read    = io->src.read,
                src_written = io->src.written,
                dst_read    = io->dst.read,
                dst_written = io->dst.written;
      iologaddr_t lsrc = src, ldst = dst;
      connectionstate_t replystate;
      size_t bwused = 0;

#if BAREFOOTD
      packetrule = io->crule;

#else /* SOCKS_SERVER */
      SASSERTX(io->cmd.udp.sfwdrule   != NULL);
      SASSERTX(io->cmd.udp.sreplyrule != NULL);

      if (!io->dst.state.use_saved_srule)
         *io->cmd.udp.sreplyrule = io->srule; /* reset to initial match. */

      packetrule = io->cmd.udp.sreplyrule;
#endif /* SOCKS_SERVER */

      replystate         = io->state;
      replystate.command = SOCKS_UDPREPLY;

      ++sideschecked;

      slog(LOG_DEBUG,
           "%s: target2client i/o on fd %d -> fd %d",
           function, io->dst.s, io->src.s);

#if BAREFOOTD
      iostatus = io_udp_target2client(&io->src,
                                      &io->dst,
                                      &io->cauth,
                                      &replystate,
                                      &lsrc,
                                      &ldst,
                                      badfd,
                                      &packetrule,
                                      &bwused);
#else /* SOCKS_SERVER */
      iostatus = io_udp_target2client(&io->control,
                                      &io->src,
                                      &io->dst,
                                      &replystate,
                                      &lsrc,
                                      &ldst,
                                      badfd,
                                      packetrule,
                                      &bwused);
#endif /* SOCKS_SERVER */


      if (IOSTATUS_FATALERROR(iostatus))
         return iostatus;

      if (iostatus == IO_NOERROR) {
         DO_IOCOUNT(&src_read,
                    &src_written,
                    &dst_read,
                    &dst_written,
                    io);

         io_update(&io->lastio,
                   bwused,
                   NULL,
                   &src_written,
                   &dst_read,
                   NULL,

#if BAREFOOTD
                   &packetrule,
                   &packetrule,

#else /* SOCKS_SERVER */

                   &io->srule,
                   packetrule,

#endif /* SOCKS_SERVER */

                   sockscf.shmemfd);
      }
   }

   SASSERTX(sideschecked > 0);

   gettimeofday_monotonic(&io->lastio);

   return iostatus;
}

iostatus_t
io_packet_received(recvflags, bytesreceived, from, receivedon)
   const recvfrom_info_t *recvflags;
   const size_t bytesreceived;
   const struct sockaddr_storage *from;
   const struct sockaddr_storage *receivedon;
{
   const char *function = "io_packet_received()";
#if HAVE_SO_TIMESTAMP
   struct timeval tnow, latency;
#endif /* HAVE_SO_TIMESTAMP */

   if (recvflags->flags & MSG_TRUNC) {
      log_truncatedudp(function, from, bytesreceived);
      return IO_TMPERROR;
   }

#if HAVE_SO_TIMESTAMP
   gettimeofday(&tnow, NULL);

   io_addts(io_calculatelatency(&recvflags->ts, &tnow, &latency), from, NULL);
#endif /* HAVE_SO_TIMESTAMP */

   return IO_NOERROR;
}

iostatus_t
io_packet_sent(bytestosend, bytessent, tsreceived, from, to, emsg, emsglen)
   const size_t bytestosend;
   const size_t bytessent;
   const struct timeval *tsreceived;
   const struct sockaddr_storage *from;
   const struct sockaddr_storage *to;
   char *emsg;
   size_t emsglen;
{
   const char *function = "io_packet_sent()";
#if HAVE_SO_TIMESTAMP
   struct timeval tnow, latency;

   gettimeofday(&tnow, NULL);

   io_addts(io_calculatelatency(tsreceived, &tnow, &latency), from, to);
#endif /* HAVE_SO_TIMESTAMP */

   if (bytestosend != bytessent) {
      snprintf(emsg, emsglen, "%s", strerror(errno));

      slog(LOG_DEBUG, "%s: sendto() failed: %s", function, emsg);

      if (ERRNOISTMP(errno))
         return IO_TMPERROR;
      else
         return IO_IOERROR;
   }

   return IO_NOERROR;
}

iostat_t *
io_get_io_stats(void)
{

#if HAVE_SO_TIMESTAMP
   return io_updatestat(&iostats.rw);
#else /* !HAVE_SO_TIMESTAMP */
   return NULL;
#endif /* !HAVE_SO_TIMESTAMP */
}

iostat_t *
io_get_ro_stats(void)
{

#if HAVE_SO_TIMESTAMP
   return io_updatestat(&iostats.ro);
#else /* !HAVE_SO_TIMESTAMP */
   return NULL;
#endif /* !HAVE_SO_TIMESTAMP */
}

struct timeval *
io_calculatelatency(ts_recv, tnow, latency)
   const struct timeval *ts_recv;
   const struct timeval *tnow;
   struct timeval *latency;
{
   const char *function = "io_calculatelatency()";

   timersub(tnow, ts_recv, latency);

   return latency;
}

#if HAVE_SO_TIMESTAMP
void
io_addts(ts, from, to)
   const struct timeval *ts;
   const struct sockaddr_storage *from;
   const struct sockaddr_storage *to;
{
   const char *function = "io_addts()";
   iostat_t *iostat;
   char fstr[MAXSOCKADDRSTRING], tstr[MAXSOCKADDRSTRING];

   slog(ts->tv_sec < 0 ? LOG_WARNING : LOG_DEBUG,
        "%s: packetlatency for packet from %s to %s: %ld.%06lds%s",
        function,
        sockaddr2string(from, fstr, sizeof(fstr)),
        to == NULL ? "<read by us>" : sockaddr2string(to, tstr, sizeof(tstr)),
        (long)ts->tv_sec,
        (long)ts->tv_usec,
        ts->tv_sec < 0 ?
            ".  Sub-zero latency.  Impossible.  Clock changed?" : "");

   if (ts->tv_sec < 0)
      return;

   if (to == NULL)
      iostat = &iostats.ro;
   else
      iostat = &iostats.rw;

   if (iostat->lastlatencyi + 1 >= ELEMENTS(iostat->latencyv))
      iostat->lastlatencyi = 0; /* treat as a circular buffer. */
   else {
      if (iostat->latencyc > 0)
         ++iostat->lastlatencyi;

      if (iostat->latencyc < ELEMENTS(iostat->latencyv))
         ++iostat->latencyc;
      /* else; keep at max - that's how many ts's we have. */
   }

   iostat->latencyv[iostat->lastlatencyi] = *ts;

   SASSERTX(iostat->latencyc     <= ELEMENTS(iostat->latencyv));
   SASSERTX(iostat->lastlatencyi <= ELEMENTS(iostat->latencyv));
}

static iostat_t *
io_updatestat(iostat)
   iostat_t *iostat;
{
   const char *function = "io_updatestat()";

   if (iostat->latencyc == 0)
      return NULL;

   SASSERTX(iostat->latencyc <= ELEMENTS(iostat->latencyv));

   /* save last ts before we start sorting the info. */
   SASSERTX(iostat->lastlatencyi < ELEMENTS(iostat->latencyv));
   iostat->last_us = tv2us(&iostat->latencyv[iostat->lastlatencyi]);

   /*
    * the rest of the calculations require a sorted array.
    */
   qsort(iostat->latencyv,
         iostat->latencyc,
         sizeof(*iostat->latencyv),
         io_timercmp);

   iostat->min_us     = tv2us(&iostat->latencyv[0]);
   iostat->max_us     = tv2us(&iostat->latencyv[iostat->latencyc - 1]);
   iostat->median_us  = medtv(iostat->latencyv, iostat->latencyc);
   iostat->average_us = avgtv(iostat->latencyv, iostat->latencyc);
   iostat->stddev_us  = stddevtv(iostat->latencyv,
                                 iostat->latencyc,
                                 iostat->average_us);

#if 0
   for (i = 0; i < iostat->latencyc; ++i) {
      slog(LOG_DEBUG, "%s: index #%lu, latency: %ld.%06ld",
           function,
           (unsigned long)i,
           (long)iostat->latencyv[i].tv_sec,
           (long)iostat->latencyv[i].tv_usec);
   }
#endif

   return iostat;
}

static int
io_timercmp(a, b)
   const void *a;
   const void *b;
{

   if (timercmp((const struct timeval *)a, (const struct timeval *)b, <))
      return -1;

   if (timercmp((const struct timeval *)a, (const struct timeval *)b, ==))
      return 0;

   return 1;
}
#endif /* HAVE_SO_TIMESTAMP */

udptarget_t *
initclient(control, client_l, client_r, tohost, toaddr, rule,
            emsg, emsglen, udpdst)
   const int control;
   const struct sockaddr_storage *client_l;
   const struct sockaddr_storage *client_r;
   const sockshost_t *tohost;
   const struct sockaddr_storage *toaddr;
   const rule_t *rule;
   char *emsg;
   const size_t emsglen;
   udptarget_t *udpdst;
{
   const char *function = "initclient()";
   char fromstr[MAXSOCKADDRSTRING], tohoststr[MAXSOCKSHOSTSTRING],
        toaddrstr[MAXSOCKADDRSTRING];
   int s, rc;

   slog(LOG_DEBUG, "%s: from %s to %s (%s)",
        function,
        sockaddr2string(client_r, fromstr,   sizeof(fromstr)),
        sockshost2string(tohost,  tohoststr, sizeof(tohoststr)),
        sockaddr2string(toaddr,   toaddrstr, sizeof(toaddrstr)));

   bzero(udpdst, sizeof(*udpdst));

   udpdst->raddrhost = *tohost;
   udpdst->raddr     = *toaddr;

   /*
    * Create a new socket and use that for sending out packets
    * from this client only.  When reading replies on this socket,
    * we will thus know who it's destined for (from).
    * Since we place no bound on the number of udp clients we
    * handle, we need to make sure we leave room for at least
    * SOCKD_IOMAX tcp clients, so we don't fail on recvmsg(2)
    * when mother sends us a new tcp client.
    */

   errno = 0;
   s     = -1;

   if (iostate.freefds  <= ((SOCKD_IOMAX - 1) * FDPASS_MAX)
   || (s = socket(udpdst->raddr.ss_family, SOCK_DGRAM, 0)) == -1) {
      snprintf(emsg, emsglen, "could not create %s udp socket: %s",
               safamily2string(udpdst->raddr.ss_family),
               errno == 0 ?
                  "already running short of sockets" : strerror(errno));

      swarnx("%s: %s", function, emsg);

      if (s != -1)
         close(s);

      return NULL;
   }

   SASSERTX(s != -1);

   if (getoutaddr(&udpdst->laddr,
                  client_l,
                  client_r,
                  SOCKS_UDPASSOCIATE,
                  &udpdst->raddrhost,
                  emsg,
                  emsglen) == NULL) {
      slog(LOG_DEBUG,
           "%s: could not establish address to use for sending UDP to %s: %s",
           function, sockshost2string(&udpdst->raddrhost, NULL, 0), emsg);

      close(s);
      return NULL;
   }

   setsockoptions(s, udpdst->raddr.ss_family, SOCK_DGRAM, 0);

   setconfsockoptions(s,
                      control,
                      SOCKS_UDP,
                      0,
                      rule->socketoptionc,
                      rule->socketoptionv,
                      SOCKETOPT_PRE | SOCKETOPT_ANYTIME,
                      SOCKETOPT_PRE | SOCKETOPT_ANYTIME);

   if ((rc = socks_bind(s, &udpdst->laddr, 0)) != 0) {
      if (GET_SOCKADDRPORT(&udpdst->laddr) != htons(0))
         SET_SOCKADDRPORT(&udpdst->laddr, htons(0));

      rc = socks_bind(s, &udpdst->laddr, 0);
   }

   if (rc != 0) {
      log_bind_failed(function, SOCKS_UDP, &udpdst->laddr);

      snprintf(emsg, emsglen, "could not bind udp address %s: %s",
               sockaddr2string(&udpdst->laddr, NULL, 0),
               strerror(errno));

      close(s);
      return NULL;
   }

   log_boundexternaladdress(function, &udpdst->laddr);

#if SOCKS_SERVER
  /*
   * Dante uses iobufs for some udp stuff too (just to save a getsockopt(2)
   * call?), though we don't buffer udp packets.  Barefootd is not dimensioned
   * for allocating iobufs for udp however.
   */
  socks_allocbuffer(s, SOCK_DGRAM);
#endif /* SOCKS_SERVER */

   /*
    * All ok, save contents into udpdst and return success.
    */


#if BAREFOOTD
   sockaddrcpy(&udpdst->client, client_r, salen(client_r->ss_family));
   sockaddr2sockshost(&udpdst->client, &udpdst->clienthost);
#endif /* BAREFOOTD */

   gettimeofday_monotonic(&udpdst->lastio);
   udpdst->firstio = udpdst->lastio;
   udpdst->s       = s;

   --iostate.freefds;

   slog(LOG_DEBUG,
        "%s: allocated socket fd %d for packets from %s to, initially, %s",
        function, udpdst->s, fromstr, tohoststr);

   return udpdst;
}

udptarget_t *
addclient(clientladdr, client, clientc, maxclientc, clientv, state, rule)
   const struct sockaddr_storage *clientladdr;
   const udptarget_t *client;
   size_t *clientc;
   size_t *maxclientc;
   udptarget_t **clientv;
   const connectionstate_t *state;
   const struct rule_t *rule;
{
   const char *function = "addclient()";
#if BAREFOOTD
   iologaddr_t src, dst;

   char client_str[MAXSOCKADDRSTRING],
        laddr_str[MAXSOCKADDRSTRING],
        raddr_str[MAXSOCKSHOSTSTRING];

   slog(LOG_DEBUG,
        "%s: adding client on fd %d: client %s, bound %s, dst %s (%s).  "
        "New clientc will become %lu, new free fds will become %lu",
        function,
        client->s,
        sockaddr2string(&client->client, client_str, sizeof(client_str)),
        sockaddr2string(&client->laddr, laddr_str, sizeof(laddr_str)),
        sockaddr2string(&client->raddr, raddr_str, sizeof(raddr_str)),
        sockshost2string(&client->raddrhost, NULL, 0),
        (unsigned long)(*clientc + 1),
        (unsigned long)(iostate.freefds - 1));

   if (*clientc >= *maxclientc) {
      udptarget_t *pv;

      SASSERTX(*clientc == *maxclientc);

      if ((pv = realloc(*clientv, ((*maxclientc) + 1) * sizeof(*pv))) == NULL) {
         swarn("%s: failed to allocate memory for new udp client from %s",
               function, sockaddr2string(&client->client, NULL, 0));

         return NULL;
      }

      if (pv != *clientv)
         *clientv = pv;

      *maxclientc += 1;

      slog(LOG_DEBUG,
           "%s: reallocated memory for udp clients.  Have memory for a total "
           "of %lu clients now, and %lu clients already",
           function, (unsigned long)*maxclientc, (unsigned long)*clientc);
   }

#else  /* !BAREFOOTD */

      /*
       * In Dante there can never be more than two; one for IPv4 and
       * one for IPv6.
       */
      SASSERTX(*clientc < *maxclientc);
#endif /* !BAREFOOTD */

   (*clientv)[*clientc] = *client;

#if BAREFOOTD
   init_iologaddr(&src,
                  object_sockaddr,
                  clientladdr,
                  object_sockshost,
                  &client->clienthost,
                  NULL,
                  NULL);

   init_iologaddr(&dst,
                  object_sockaddr,
                  &client->laddr,
                  object_sockshost,
                  &client->raddrhost,
                  NULL,
                  NULL);

   iolog(rule,
         state,
         OPERATION_CONNECT,
         &src,
         &dst,
         NULL,
         NULL,
         NULL);

#endif /* !BAREFOOTD */

   return &(*clientv)[(*clientc)++];
}

void
io_syncudp(io, udpclient)
   sockd_io_t *io;
   udptarget_t *udpclient;
{
   io->state.time.established     = udpclient->firstio;

#if BAREFOOTD
   io->crule                      = udpclient->crule;
#endif /* BAREFOOTD */

   io_syncudpsrc(&io->src, udpclient);

   io_syncudpdst(&io->dst, udpclient);
}

void
io_syncudpsrc(src, udpclient)
   sockd_io_direction_t *src;
   const udptarget_t *udpclient;
{

   /* src.laddr is the same for all clients and never changes. */

#if BAREFOOTD
   /*
    * In Dante this remain the same as Dante only has one client per
    * udp i/o-session.  Each client can have up to two targets though,
    * with independent byte counts for each.
    */

   src->raddr                 = udpclient->client;
   src->host                  = udpclient->clienthost;
#endif /* BAREFOOTD */

   src->read                  = udpclient->client_read;
   src->written               = udpclient->client_written;
}


void
io_syncudpdst(dst, udptarget)
   sockd_io_direction_t *dst;
   const udptarget_t *udptarget;
{
   dst->s                     = udptarget->s;
   dst->laddr                 = udptarget->laddr;
   dst->raddr                 = udptarget->raddr;
   dst->host                  = udptarget->raddrhost;

   dst->read                  = udptarget->target_read;
   dst->written               = udptarget->target_written;

   dst->state.isconnected     = udptarget->isconnected;
}



udptarget_t *
clientofsocket(s, udpclientc, udpclientv)
   const int s;
   const size_t udpclientc;
   udptarget_t *udpclientv;
{
   static size_t i;

   if (i < udpclientc && udpclientv[i].s == s)
      return &udpclientv[i];

   for (i = 0; i < udpclientc; ++i)
      if (udpclientv[i].s == s)
         return &udpclientv[i];

   return NULL;
}
