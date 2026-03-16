/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2017, 2024
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
"$Id: sockd_tcp.c,v 1.66.4.4.2.4.8.4 2024/11/22 21:05:24 michaels Exp $";

static ssize_t
io_tcp_rw(sockd_io_direction_t *in, sockd_io_direction_t *out, int *badfd,
          iostatus_t *iostatus,
#if COVENANT
          const requestflags_t *reqflags, size_t *bufused,
#endif /* COVENANT */
          char *buf, size_t bufsize, int flags);
/*
 * Transfers TCP data from "in" to "out" using "buf" as a temporary buffer
 * to store the data, and sets flag "flags" on the send/recv system call
 * used to do the i/o.
 * The data transferred uses "buf" as a buffer, which is of size "bufsize".
 *
 * Covenant has these additional arguments:
 * - "reqflags" is flags for the client side of the request.
 * - "bufused", indicates how much of "buf" has previously been used, but
 *    not written to "out".  Upon return, "bufused" is updated with the
 *    new value.
 *
 * Returns:
 *      On success: number of bytes written to "out" (encoding included).
 *      On failure: -1.  "badfd" is set to the value of the descriptor that
 *                  "failure" was first detected on, and "iostatus" to
 *                  the corresponding failurecode.
 *
 *                  If the error does not correspond to any given
 *                  descriptor (internal error), "badfd" is not changed.
 */

iostatus_t
doio_tcp(io, rset, wset, flags, badfd)
   sockd_io_t *io;
   fd_set *rset, *wset;
   const int flags;
   int *badfd;
{
   const char *function = "doio_tcp()";
   const int isreversed = (io->state.command == SOCKS_BINDREPLY ? 1 : 0);
   iocount_t src_read    = io->src.read,
             src_written = io->src.written,
             dst_read    = io->dst.read,
             dst_written = io->dst.written;
   iostatus_t iostatus;
   sendto_info_t sendtoflags;
   iologaddr_t src, dst, proxy;
   struct { objecttype_t type; void *object; } dstraddr;
   ssize_t r, w;
   size_t bwused;
   int bothways;

#if COVENANT

   char *buf      = io->clientdata;
   size_t buflen  = sizeof(io->clientdata) - io->clientdatalen;
   size_t bufused = io->clientdatalen;

#else

   char buf[SOCKD_BUFSIZE];
   size_t buflen  = sizeof(buf);

#endif /* !COVENANT */


   SASSERTX(io->allocated);
   SASSERTX(io->state.protocol == SOCKS_TCP);
   SASSERTX(!(io->src.state.fin_received && io->dst.state.fin_received));

   if (io->state.command == SOCKS_CONNECT)
      SASSERTX(io->dst.state.isconnected);

   if ((FD_ISSET(io->src.s, rset) && FD_ISSET(io->dst.s, wset))
   ||  (FD_ISSET(io->dst.s, rset) && FD_ISSET(io->src.s, wset)))
      ;
   else {
      swarnx("%s: FD_ISSET(io->src.s, rset) = %d, "
             "    FD_ISSET(io->src.s, wset) = %d "
             "    FD_ISSET(io->dst.s, rset) = %d, "
             "    FD_ISSET(io->dst.s, wset) = %d, ",
             function,
             FD_ISSET(io->src.s, rset),
             FD_ISSET(io->src.s, wset),
             FD_ISSET(io->dst.s, rset),
             FD_ISSET(io->dst.s, wset));

      SWARNX(0);

      *badfd = io->dst.s;
      return IO_ERROR;
   }

   slog(LOG_DEBUG, "%s: control-fd %d, src-fd %d, dst-fd %d",
        function, io->control.s, io->src.s, io->dst.s);

   if (!timerisset(&io->state.time.firstio))
      gettimeofday_monotonic(&io->state.time.firstio);

   if (FD_ISSET(io->src.s, rset) && FD_ISSET(io->dst.s, wset)
   &&  FD_ISSET(io->dst.s, rset) && FD_ISSET(io->src.s, wset))
      bothways = 1;
   else
      bothways = 0;

   if (SHMEMRULE(io)->bw_shmid != 0) {
      /*
       * If most clients are active, this should distribute the bw
       * reasonably fair.  If not, this is suboptimal as we may do
       * more i/o operations than otherwise necessary because our
       * buflen is smaller than it needs to be.
       */

      buflen = MIN(SHMEMRULE(io)->bw->object.bw.maxbps, buflen);

      SASSERTX(SHMEMRULE(io)->bw->mstate.clients > 0);
      buflen = MAX(1,
                   ((buflen / SHMEMRULE(io)->bw->mstate.clients)
                    / (bothways ? 2 : 1)));

      slog(LOG_DEBUG,
           "%s: # of clients is %lu, bothways? %s.  Buflen set to %lu",
           function,
           (unsigned long)SHMEMRULE(io)->bw->mstate.clients,
           bothways ? "yes" : "no",
           (unsigned long)buflen);
   }

   init_iologaddr(&src,
                  object_sockaddr,
                  &io->src.laddr,
                  object_sockaddr,
                  &io->src.raddr,
                  &io->src.auth,
                  &io->state.hostid);

   if (io->state.proxychain.proxyprotocol == PROXY_DIRECT) {
      dstraddr.object = &io->dst.raddr;
      dstraddr.type   = object_sockaddr;
   }
   else {
      dstraddr.object = &io->dst.host;
      dstraddr.type   = object_sockshost;
   }

   init_iologaddr(&dst,
                  object_sockaddr,
                  &io->dst.laddr,
                  dstraddr.type,
                  dstraddr.object,
                  &io->dst.auth,
                  NULL);

   init_iologaddr(&proxy,
                  object_sockaddr,
                  io->state.proxychain.proxyprotocol == PROXY_DIRECT ?
                     NULL : &io->dst.raddr,
                  object_sockshost,
                  io->state.proxychain.proxyprotocol == PROXY_DIRECT ?
                     NULL : &io->state.proxychain.extaddr,
                  NULL,
                  NULL);

#define CHECK_ALARM(iostatus)                                                  \
do {                                                                           \
   if (((iostatus) == IO_IOERROR || (iostatus) == IO_CLOSE)                    \
   &&  io->srule.mstats_shmid != 0                                             \
   &&  (io->srule.alarmsconfigured & ALARM_DISCONNECT)) {                      \
      sockd_io_direction_t *disconnectside;                                    \
      clientinfo_t cinfo;                                                      \
      int alarmside, weclosedfirst;                                            \
                                                                               \
      SASSERTX(io->srule.mstats != NULL);                                      \
                                                                               \
      if (*badfd == io->src.s) {                                               \
         if (io->src.state.alarmdisconnectdone)                                \
            break;                                                             \
                                                                               \
         if (io->dst.state.fin_received)                                       \
            weclosedfirst = 1;                                                 \
         else                                                                  \
            weclosedfirst = 0;                                                 \
                                                                               \
         if (isreversed)                                                       \
            alarmside = ALARM_EXTERNAL;                                        \
         else                                                                  \
            alarmside = ALARM_INTERNAL;                                        \
                                                                               \
         disconnectside = &io->src;                                            \
      }                                                                        \
      else {                                                                   \
         SASSERTX(*badfd == io->dst.s);                                        \
                                                                               \
         if (io->dst.state.alarmdisconnectdone)                                \
            break;                                                             \
                                                                               \
         if (io->src.state.fin_received)                                       \
            weclosedfirst = 1;                                                 \
         else                                                                  \
            weclosedfirst = 0;                                                 \
                                                                               \
         if (isreversed)                                                       \
            alarmside = ALARM_INTERNAL;                                        \
         else                                                                  \
            alarmside = ALARM_EXTERNAL;                                        \
                                                                               \
         disconnectside = &io->dst;                                            \
      }                                                                        \
                                                                               \
      cinfo.from   = CONTROLIO(io)->raddr;                                     \
      cinfo.hostid = io->state.hostid;                                         \
                                                                               \
      alarm_add_disconnect(weclosedfirst,                                      \
                           &io->srule,                                         \
                           alarmside,                                          \
                           &cinfo,                                             \
                           (iostatus) == IO_CLOSE ? "EOF" : strerror(errno),   \
                           sockscf.shmemfd);                                   \
                                                                               \
      disconnectside->state.alarmdisconnectdone = 1;                           \
   }                                                                           \
} while (/* CONSTCOND */ 0)

#define CHECK_IOSTATUS(iostatus)                                               \
do {                                                                           \
   if (*(iostatus) == IO_CLOSE) {                                              \
      if (io->src.state.fin_received && io->dst.state.fin_received) {          \
         /* let badfd be the side that closed first. */                        \
         *badfd = (*badfd == io->src.s ? io->dst.s : io->src.s);               \
                                                                               \
         slog(LOG_DEBUG,                                                       \
              "%s: both sides (fd %d and fd %d) have been shut down.  "        \
              "%s-side (fd %d) closed first.  Should close session now",       \
              function, io->src.s, io->dst.s,                                  \
              *badfd == io->src.s ? "Src" : "Dst", *badfd);                    \
      }                                                                        \
      else                                                                     \
         *(iostatus) = IO_NOERROR; /* close when both ends are shut down. */   \
   }                                                                           \
} while (/* CONSTCOND */ 0)

   bzero(&sendtoflags, sizeof(sendtoflags));
   bwused = 0;

   /* from in to out ... */
   if (FD_ISSET(io->src.s, rset) && FD_ISSET(io->dst.s, wset)) {
      int allflushed;

      errno  = 0; /* reset before each call. */
      *badfd = -1;


      /*
       * If we have previously tried to write, but could not write it
       * all, we will have data buffered for the socket. In that case
       * we need to flush the buffer before writing anything else.
       * Since that data has already been logged as written (even if
       * only to buffer), don't log it again.
       */

      sendtoflags.side       = isreversed ? INTERNALIF : EXTERNALIF;
      w                      = socks_flushbuffer(io->dst.s, -1, &sendtoflags);
      io->dst.written.bytes += sendtoflags.tosocket;

      if (w == -1) {
         *badfd = io->dst.s;

         if (!ERRNOISTMP(errno)) {
            CHECK_ALARM(IO_IOERROR);
            return IO_IOERROR;
         }

         allflushed = 0;
      }
      else
         allflushed = 1;

      if (allflushed) {
         r = io_tcp_rw(&io->src,
                       &io->dst,
                       badfd,
                       &iostatus,
#if COVENANT
                       &io->reqflags,
                       &bufused,
#endif /* COVENANT */
                       buf,
                       buflen,
                       flags);

         CHECK_ALARM(iostatus);
         CHECK_IOSTATUS(&iostatus);

         if (IOSTATUS_FATALERROR(iostatus))
            return iostatus;

         switch (r) {
            case -1:
               r = 0; /* not fatal error, so must be temporary error. */
               break;

            case 0: /* log EOF too. */
            default: {
               int fdv[] = { CLIENTIO(io)->s, EXTERNALIO(io)->s };

               if (io->srule.log.tcpinfo
               && (io->srule.log.iooperation || io->srule.log.data)) {
                  SASSERTX(io->state.tcpinfo == NULL);
                  io->state.tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
               }

               iolog(&io->srule,
                     &io->state,
                     OPERATION_IO,
                     &src,
                     &dst,
                     NULL,
                     &proxy,
                     buf,
                     (size_t)r);

               io->state.tcpinfo = NULL;
            }
         }

         bwused += r;
      }
   }

   /* ... and out to in. */
   if (FD_ISSET(io->dst.s, rset) && FD_ISSET(io->src.s, wset)) {
      int allflushed;

      errno  = 0; /* reset before each call. */
      *badfd = -1;

      /*
       * If we have previously tried to write, but could not write it
       * all, we will have data buffered for the socket. In that case
       * we need to flush the buffer before writing anything else.
       * Since that data has already been logged as written (even if
       * only to buffer), don't log it again.
       */
      sendtoflags.side       = isreversed ? EXTERNALIF : INTERNALIF;
      w                      = socks_flushbuffer(io->src.s, -1, &sendtoflags);
      io->src.written.bytes += sendtoflags.tosocket;

      if (w == -1) {
         *badfd = io->src.s;

         if (!ERRNOISTMP(errno)) {
            CHECK_ALARM(IO_IOERROR);
            return IO_ERROR;
         }

         allflushed = 0;
      }
      else
         allflushed = 1;

      if (allflushed) {
         r = io_tcp_rw(&io->dst,
                       &io->src,
                       badfd,
                       &iostatus,
#if COVENANT
                       &io->reqflags,
                       &bufused,
#endif /* COVENANT */
                       buf,
                       buflen,
                       flags);

         CHECK_ALARM(iostatus);
         CHECK_IOSTATUS(&iostatus);

         if (IOSTATUS_FATALERROR(iostatus))
            return iostatus;

         switch (r) {
            case -1:
               r = 0; /* not fatal error, so must be temporary error. */
               break;

            case 0: /* log EOF too. */
            default: {
               int fdv[] = { EXTERNALIO(io)->s, CLIENTIO(io)->s };

               if (io->srule.log.tcpinfo
               && (io->srule.log.iooperation || io->srule.log.data)) {
                  SASSERTX(io->state.tcpinfo == NULL);
                  io->state.tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
               }

               iolog(&io->srule,
                     &io->state,
                     OPERATION_IO,
                     &dst,
                     &src,
                     &proxy,
                     NULL,
                     buf,
                     (size_t)r);

               io->state.tcpinfo = NULL;
            }
         }

         bwused += r;
      }
   }

   gettimeofday_monotonic(&io->lastio);
   DO_IOCOUNT(&src_read, &src_written, &dst_read, &dst_written, io);

   io_update(&io->lastio,
             bwused,
             isreversed ? &dst_read    : &src_read,
             isreversed ? &dst_written : &src_written,
             isreversed ? &src_read    : &dst_read,
             isreversed ? &src_written : &dst_written,
             SHMEMRULE(io),
             SHMEMRULE(io),
             sockscf.shmemfd);

   slog(LOG_DEBUG, "%s: bwused = %ld", function, (unsigned long)bwused);

   if (bwused)
      return IO_NOERROR;
   else
      return IO_EAGAIN; /* nothing available at the moment. */
}

static ssize_t
io_tcp_rw(in, out, badfd, iostatus,
#if COVENANT
          reqflags, bufused,
#endif /* COVENANT */
          buf, bufsize, flags)
          sockd_io_direction_t *in;
          sockd_io_direction_t *out;
          int *badfd;
          iostatus_t *iostatus;
#if COVENANT
          const requestflags_t *reqflags;
          size_t *bufused;
#endif /* COVENANT */
          char *buf;
          size_t bufsize;
          int flags;
{
   const char *function = "io_tcp_rw()";
   sendto_info_t sendtoflags;
   recvfrom_info_t recvfromflags;
   ssize_t r, w, p;
#if 0 /* for aid in debuging bufferproblems. */
   static size_t j;
   size_t lenv[] = { 60000, 60001, 60002, 60003, 60004, 60005, 60006, 60007,
                     60008, 60009, 60010, 60011, 60012, 60013, 60014, 60015 };
#endif

#if !COVENANT
   size_t bufusedmem = 0, *bufused = &bufusedmem;
#endif /* COVENANT */

   SASSERTX(!in->state.fin_received);

   *iostatus = IO_NOERROR;

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG,
           "%s: fd %d -> fd %d, bufsize = %lu, bufused = %lu, flags = %d",
           function,
           in->s,
           out->s,
           (unsigned long)bufsize,
           (unsigned long)*bufused,
           flags);

   if (in->state.err != 0) {
      errno = in->state.err;
      *badfd  = in->s;
   }
   else if (out->state.err != 0) {
      errno = out->state.err;
      *badfd  = out->s;
   }
   else
      *badfd = -1; /* no error so far. */

   if (*badfd != -1) {
      *iostatus = IO_IOERROR;

      slog(LOG_DEBUG,
           "%s: failure already detected on fd %d (%s)",
           function, *badfd, strerror(errno));

      return -1;
   }

   /*
    * read data from in ...
    */

   /*
    * We receive OOB in-line.  If flags has MSG_OOB set, it means we
    * are at the OOB marker.  The next byte we read (and it will be
    * only one) is the OOB byte, and since we receive it in-line,
    * we should turn off the MSG_OOB flags.
    *
    * When we write the data we will keep the MSG_OOB flags, and
    * hopefully that will work even if the write is combined with
    * data read from the buffer (so we send more than one byte).
    * The last byte, which we will now read from the socket, should then be
    * tagged as the "oob" data.  Possible this will not work 100% correctly
    * on some non-BSD implementations, but go with it for now.
    */

   if (in->flags & MSG_OOB)
      /*
       * The problem with oob is that select(2) signals we have oob until;
       * we've read past it (i.e., read at least one normal byte).
       *
       * To handle receiving two oob-bytes in a row, we need to check whether
       * the last byte we received on this socket was also an oob-byte, as
       * if it was, we can't use select(2) to check for it (select(2) would
       * keep returning, since the oob flags is set until we've read _past_ the
       * oob byte), and we must instead check here for oob regardless of what
       * flags says.
       */
      flags |= MSG_OOB;

   if (flags & MSG_OOB) {
      if (sockatmark(in->s) != 1)
         flags &= ~MSG_OOB;
      else
         slog(LOG_DEBUG, "%s: have OOB data on fd %d", function, in->s);
   }

   /*
    * never read more from in than we can write to out, iobuf included.
    * Since we don't know how much we can write to the socket, except
    * it should normally always be at least one byte, only count the
    * space left in the iobuf.
    * Also make sure we can always NUL-terminate buf if necessary, to
    * make things easier for covenant.
    */
   p = MIN(bufsize - *bufused, socks_freeinbuffer(out->s, WRITE_BUF));
   if (p <= 0) {
      swarnx("%s: no more room in iobuf for fd %d.  "
             "This should only happen if the kernel for some reason has told "
             "us before that the socket is writable, yet we were not able "
             "to write even one byte.  Closing session now, rather than "
             "risk busy-looping due to what looks like a kernel bug",
             function, out->s);

      *badfd      = out->s;
      errno       = ENOBUFS; /* something vaguely related. */
      *iostatus   = IO_ERROR;

      return -1;
   }

   if (in->isclientside)
      sendtoflags.side = EXTERNALIF;
   else
      sendtoflags.side = INTERNALIF;

#if HAVE_GSSAPI
   /*
    * If the data we are writing needs to be gssapi-encapsulated,
    * also make sure we don't read more than we can encapsulate in
    * a gssapi token; we don't want to deal with segmented gssapi tokens.
    */
   if (out->auth.method == AUTHMETHOD_GSSAPI) {
      SASSERTX(out->auth.mdata.gssapi.state.maxgssdata > 0);

      p -= GSSAPI_OVERHEAD(&out->auth.mdata.gssapi.state) + GSSAPI_HLEN;
      p  = MIN(p, (ssize_t)out->auth.mdata.gssapi.state.maxgssdata);

      if (p <= 0) {
         /*
          * We are not expecting this to happen since we should not get
          * here as long as we have unflushed data left in the buffer.
          */
         swarnx("%s: write buffer for fd %d on %s-side is almost full.  "
                "Only %lu byte%s free, with a gssapi overhead of %lu",
                function,
                out->s,
                interfaceside2string(sendtoflags.side),
                (unsigned long)socks_freeinbuffer(out->s, WRITE_BUF),
                (unsigned long)socks_freeinbuffer(out->s, WRITE_BUF) == 1 ?
                  "" : "s",
                (unsigned long)GSSAPI_OVERHEAD(&out->auth.mdata.gssapi.state));

         p = 1; /* try to make some progress. */
      }
   }
#endif /* HAVE_GSSAPI */

#if COVENANT
   if (in->isclientside && !reqflags->httpconnect)
      flags |= MSG_PEEK;
#endif /* COVENANT  */

   recvfromflags.type = SOCK_STREAM;

   if (in->isclientside)
      recvfromflags.side = INTERNALIF;
   else
      recvfromflags.side = EXTERNALIF;

#if 0
   p = MIN(lenv[j % ELEMENTS(lenv)], p);
   ++j;
#endif

   SASSERTX(p >= 0);
   r = socks_recvfrom(in->s,
                      &buf[*bufused],
                      (size_t)p,
                      flags & ~MSG_OOB,
                      NULL,
                      NULL,
                      &recvfromflags,
                      &in->auth);

   if (r <= 0) {
      *badfd = in->s;

      if (r == 0) {
         /*
          * FIN from "in".  It won't send us any more data, so we shutdown
          * "out" for writing (send a FIN to it) to let it know.
          *
          * When "out" has nothing more to send, it will send us a FIN too,
          * and we will shutdown "in" for writing.  At that point, both "in"
          * and "out" have sent a FIN, meaning none of them will send us any
          * more data.  Only then can we close the socket.
          */

         *iostatus = IO_CLOSE;

         slog(LOG_DEBUG,
               "%s: got EOF on in->s (fd %d) when trying to read %ld bytes.  "
               "Status of out (fd %d): fin: %d",
               function, in->s, (long)p, out->s, out->state.fin_received);

         in->state.fin_received = 1;

#if HAVE_DARWIN
         if (in->read.bytes == 0) {
            socklen_t optlen;
            int opt;

            optlen = sizeof(opt);
            if (getsockopt(in->s, SOL_SOCKET, SO_RCVBUF, &opt, &optlen) == -1)
               swarn("%s: getsockopt(SO_RCVBUF)", function);

            if (opt == 0)
               swarnx("%s: There appears to be a bug in the OS X Kernel, "
                      "v10.8.0 and earlier at least, that for some reason "
                      "makes a socket's SO_RCVBUF become zero sometimes "
                      "during the processes of passing the file descriptor "
                      "around between processes.  "
                      "Subsequent reads from it return 0 even if the other "
                      "side has not closed the connection.  "
                      "This makes TCP's EOF indication not work correctly, "
                      "and %s ends up closing the session prematurely.",
                      function, PRODUCT);
         }
#endif /* HAVE_DARWIN */

         SASSERTX(socks_bytesinbuffer(out->s, WRITE_BUF, 0) == 0);

         if (out->state.fin_received)
            errno = 0; /* no error, just proper eof from both sides. */

         /*
          * use shutdown() to forward FIN, but continue reading.
          */
         slog(LOG_DEBUG,
              "%s: shutting down out->s (fd %d) for writing", function, out->s);

         if (shutdown(out->s, SHUT_WR) != 0) {
            slog(LOG_DEBUG,
                 "%s: shutdown(2) on fd %d after receiving FIN from fd %d "
                 "failed (%s).  FIN previously received from fd %d ? %s",
                 function,
                 out->s,
                 in->s,
                 strerror(errno),
                 out->s,
                 out->state.fin_received ? "Yes" : "No");

            if (out->state.fin_received)
               /*
                * don't consider this an error - normal operation in most
                * cases.
                */
               errno = 0;
            else {
               *badfd    = out->s;
               *iostatus = IO_IOERROR;
            }
         }
      }
      else {
         if (ERRNOISTMP(errno))
            *iostatus = IO_TMPERROR;
         else
            *iostatus = IO_IOERROR;
      }

      return r;
   }

   in->read.bytes += recvfromflags.fromsocket;

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: read %ld bytes (%lu from socket)",
           function, (long)r, (unsigned long)recvfromflags.fromsocket);

#if COVENANT
   if (in->isclientside && !reqflags->httpconnect) {
      /*
       * As long as the target of the clients request does not change, we
       * can forward it as normal.  If it changes, we need to restart
       * negotiation however.
       * Since we have no other way to know when the target changes, we have
       * to parse all data from the the http client before we can forward it,
       * as if the request is to a different server, it should not be
       * forwarded to the current target.
       */
      const char *http_eof = "\r\n\r\n";
      sockd_client_t client;
      char *p, emsg[512];

      buf[*bufused + r] = NUL;
      p = strstr(buf, http_eof);

      slog(LOG_DEBUG, "%s: read %ld bytes now, %lu bytes in total.  "
                      "%s HTTP request eof",
                      function, (long)r, (unsigned long)*bufused + r,
                      p == NULL ? "Not yet at" : "Now have");

      if (p == NULL)
         ;  /* no request-eof yet, save all read so far and continue later. */
      else { /* got the end of the request.  How far out in the buffer is it? */
          r        = (p + strlen(http_eof)) - buf;
          *bufused = 0;
      }

      flags &= ~MSG_PEEK;

      /* re-read the data we previously just peeked at. */
      w = socks_recvfrom(in->s,
                         &buf[*bufused],
                         r,
                         flags & ~MSG_OOB,
                         NULL,
                         NULL,
                         NULL,
                         &in->auth);
      SASSERTX(r == w);

      if (p == NULL) {
         errno     = EAGAIN;
         *iostatus = IO_TMPERROR;
         *badfd    = in->s;

         return -1; /* no end of request yet.  Return. */
      }

      /*
       * got the request.  Parse it and see if the target is still
       * the same.
       */
       client.request.auth = &client.auth;
       if (parse_httprequest(&client.request, buf, emsg, sizeof(emsg)) != 0) {
         char visbuf[2048];

         slog(LOG_INFO, "%s: failed to parse http request \"%s\" from %s: %s",
              function,
              socket2string(in->s, NULL, 0),
              str2vis(buf, r, visbuf, sizeof(visbuf)),
              emsg);
      }

      if (!sockshostareeq(&out->host, &client.request.host)) {
         char old[MAXSOCKSHOSTSTRING], new[MAXSOCKSHOSTSTRING];

         slog(LOG_DEBUG,
              "%s: client at %s changed target from %s to %s.  "
              "Need to renegotiate before continuing",
              function,
              socket2string(in->s, NULL, 0),
              sockshost2string(&out->host, old, sizeof(old)),
              sockshost2string(&client.request.host, new, sizeof(new)));

         memcpy(client.clientdata, buf, *bufused + r);
         client.clientdatalen = *bufused + r;
         client.s             = in->s;
         gettimeofday_monotonic(&client.accepted);

#warning "need to fix handling of race-condition between sending client/ack."

         if (send_client(sockscf.state.mother.s, &client, buf, *bufused) != 0)
            slog(LOG_DEBUG,
                 "%s: could not send client %s back to mother.  Dropping it",
                 function,
                 sockaddr2string(&client.from, NULL, 0));

         *iostatus = IO_ERROR;
         return -1;
      }
      else
         slog(LOG_DEBUG, "%s: no problem, target in the new request is the "
                         "same as before (%s)",
                         function,
                         sockshost2string(&client.request.host, NULL, 0));
   }
#endif /* COVENANT */

   if (flags & MSG_OOB)
      in->flags |= MSG_OOB;   /* read oob data.            */
   else
      in->flags &= ~MSG_OOB;  /* did not read oob data.    */

   /*
    * ... and send the data read to out.
    */

   if ((w = socks_sendto(out->s,
                         buf,
                         r,
                         flags,
                         NULL,
                         0,
                         &sendtoflags,
                         &out->auth)) == -1) {
      slog(LOG_DEBUG, "%s: write to fd %d failed.  Wrote %ld/%ld: %s",
           function, out->s, (long)w, (long)r, strerror(errno));

      if (ERRNOISTMP(errno))
        /*
         * Should never read more than we can write/buffer, so this should
         * not happen, but try once more before bailing out.
         */
         w = socks_sendto(out->s,
                          buf,
                          r,
                          flags,
                          NULL,
                          0,
                          &sendtoflags,
                          &out->auth);
   }

   out->written.bytes += sendtoflags.tosocket;

   if (w != r) {
      *badfd    = out->s;
      *iostatus = IO_ERROR;

      if (w > 0) {
         swarnx("%s: wrote only %ld/%ld (%s) to fd %d, but we should never "
                "read more than we can write, so this should not happen",
                function, (long)w, (long)r, strerror(errno), out->s);

#if PRERELEASE

         SERRX(out->s);

#else /* !PRERELEASE */

         SWARNX(out->s);

#endif /* PRERELEASE */
      }
   }
   else if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: wrote %ld bytes (%lu to socket)",
           function, (long)w, (unsigned long)sendtoflags.tosocket);

   /*
    * we want to select(2) for read again on the socket we sent data out on,
    * regardless of whether we have received a FIN from it, to detect write
    * errors as a response to sending data out on the socket.
    *
    * Unfortunately there's no way to make prevent select() from constantly
    * returning ready-for-read once the client has sent the FIN, and we can
    * not busy-loop around that of course.
    *
    * What we would have wanted is to only select(2) for errors only on the
    * socket from the point we receive the FIN, but that is not supported
    * by select(2).  It appears to be supported by poll(2) though ...
    * Note that selecting for exceptions does not help, as socket errors
    * are not considered exceptions, nor does doing shutdown(2) for reading;
    * select(2) still continuously reports that the socket is readable.
    *
    * Best we can do is to let io_fillset() skip sockets that have
    * fin_received set, as all we will receive from them is the same EOF,
    * even if a later write(2) to the socket resulted in error (at least, that
    * is the case on Linux).
    * This means some sessions can occupy space for a long time, until
    * tcp keep-alive check kicks in.
    * XXX Need to think more about this.
    */

   /*
    * would have preferred to return encoded data read + encoded data written,
    * but we don't know how much encoded data was read, only decoded data
    * read.
    */
   return w;
}
