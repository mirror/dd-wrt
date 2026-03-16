/*
 * Copyright (c) 2013
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
#include "monitor.h"

static const char rcsid[] =
"$Id: monitor_util.c,v 1.5.4.8 2014/08/24 11:41:34 karls Exp $";

#if 0
#warning "XXX change to LOG_DEBUG"
#define LOG_DEBUG LOG_NOTICE
#endif
int
recv_monitor(s, monitor)
   const int s;
   monitor_test_t *monitor;
{
   const char *function = "recv_monitor()";
   struct iovec iovecv[1];
   struct msghdr msg;
   ssize_t received;
   int iovecc, fdexpect, fdreceived;
   CMSG_AALLOC(cmsg, sizeof(int) * FDPASS_MAX);

   bzero(iovecv, sizeof(iovecv));
   iovecc = 0;

   iovecv[iovecc].iov_base = monitor;
   iovecv[iovecc].iov_len  = sizeof(*monitor);
   ++iovecc;

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iovecv;
   msg.msg_iovlen  = iovecc;
   msg.msg_name    = NULL;
   msg.msg_namelen = 0;

   /* LINTED pointer casts may be troublesome */
   CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));

   if ((received = recvmsgn(s, &msg, 0)) < (ssize_t)sizeof(*monitor)) {
      if (received == -1 && errno == EAGAIN)
         ;
      else
         slog(LOG_DEBUG,
              "%s: recvmsg(): unexpected short read on socket %d (%ld < %lu): "
              "%s",
              function,
              s,
              (long)received,
              (unsigned long)(sizeof(*monitor)),
              strerror(errno));

      return -1;
   }

   if (socks_msghaserrors(function, &msg))
      return -1;

   fdexpect = 2;
   if (!CMSG_RCPTLEN_ISOK(msg, sizeof(int) * fdexpect)) {
      swarnx("%s: received control message has the invalid len of %d",
              function, (int)CMSG_TOTLEN(msg));

      return -1;
   }


   SASSERTX(cmsg->cmsg_level == SOL_SOCKET);
   SASSERTX(cmsg->cmsg_type  == SCM_RIGHTS);

   /*
    * Get the descriptors sent us.
    */

   fdreceived = 0;

   CMSG_GETOBJECT(monitor->internal.s,
                  cmsg,
                  sizeof(monitor->internal.s) * fdreceived++);

   CMSG_GETOBJECT(monitor->external.s,
                  cmsg,
                  sizeof(monitor->external.s) * fdreceived++);

   gettimeofday_monotonic(&monitor->ts_received);

   sockd_check_ipclatency("monitor received",
                          &monitor->ts_sent,
                          &monitor->ts_received,
                          &monitor->ts_received);


   if (sockscf.option.debug || 1) {
      char src[MAXSOCKADDRSTRING * 3], dst[sizeof(src)];

      slog(LOG_DEBUG,
           "%s: %d fd(s) received.  "
           "Internal-fd %d: %s, external-fd %d: %s",
           function,
           fdreceived,
           monitor->internal.s,
           socket2string(monitor->internal.s, src, sizeof(src)),
           monitor->external.s,
           socket2string(monitor->external.s, dst, sizeof(dst)));
   }

   return 0;
}

int
send_monitor(s, monitor)
   const int s;
   const monitor_test_t *monitor;
{
   const char *function = "send_monitor()";
   struct iovec iov[1];
   struct msghdr msg;
   struct timeval tnow;
   ssize_t rc, length;
   int ioc, fdtosend;
   CMSG_AALLOC(cmsg, sizeof(int) * FDPASS_MAX);

   bzero(iov, sizeof(iov));
   length = 0;
   ioc    = 0;

   iov[ioc].iov_base  = monitor;
   iov[ioc].iov_len   = sizeof(*monitor);
   length            += iov[ioc].iov_len;
   ++ioc;

   fdtosend = 0;

   CMSG_ADDOBJECT(monitor->internal.s,
                  cmsg,
                  sizeof(monitor->internal.s) * fdtosend++);

   CMSG_ADDOBJECT(monitor->external.s,
                  cmsg,
                  sizeof(monitor->external.s) * fdtosend++);

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iov;
   msg.msg_iovlen  = ioc;
   msg.msg_name    = NULL;

   CMSG_SETHDR_SEND(msg, cmsg, sizeof(int) * fdtosend);

   if (sockscf.option.debug || 1) {
      char ibuf[MAXSOCKADDRSTRING * 3], ebuf[sizeof(ibuf)];

      slog(LOG_DEBUG,
           "%s: sending %d descriptors with monitor to process on fd %d.  "
           "Internal-fd: %d (%s), external-fd: %d (%s)",
           function,
           fdtosend,
           s,
           monitor->internal.s,
           socket2string(monitor->internal.s, ibuf, sizeof(ibuf)),
           monitor->external.s,
           socket2string(monitor->external.s, ebuf, sizeof(ebuf)));
   }

   if ((rc = sendmsgn(s, &msg, 0, 0)) != length) {
      slog(LOG_DEBUG,
           "%s: send of monitor object to monitor process failed: %ld/%ld: %s",
           function, (long)rc, (long)length, strerror(errno));

      return -1;
   }

   gettimeofday_monotonic(&tnow);
   sockscf.state.monitor_sent = tnow.tv_sec;

   return 0;
}

const monitor_t *
shmid2monitor(shmid)
   const unsigned long shmid;
{
   const char *function = "shmid2monitor()";
   monitor_t *m;

   for (m = sockscf.monitor; m != NULL; m = m->next)
      if (m->mstats_shmid == shmid) {
         slog(LOG_DEBUG, "%s: monitor #%lu has shmid %lu",
             function, (unsigned long)m->number, (unsigned long)shmid);

         return m;
      }
   return NULL;
}
