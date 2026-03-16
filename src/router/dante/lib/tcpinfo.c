/*
 * Copyright (c) 2013, 2014
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

#if HAVE_NETINET_TCP_FSM_H
#include <netinet/tcp_fsm.h>
#endif /* HAVE_NETINET_TCP_FSM_H */

static const char rcsid[] =
"$Id: tcpinfo.c,v 1.15.4.1 2014/08/15 18:16:41 karls Exp $";

#if HAVE_TCP_INFO

const char *tcpi_ca_state2string(const unsigned long val);
const char *tcpi_state2string(const unsigned long val);
const char *tcpi_options2string(const unsigned long val);

#endif /* HAVE_TCP_INFO */

char *
get_tcpinfo(fdc, fdv, buf, buflen)
   const size_t fdc;
   int fdv[];
   char *buf;
   size_t buflen;
{
   const char *function = "get_tcpinfo()";
#if HAVE_TCP_INFO
   struct tcp_info info_0, info_1;
   socklen_t len;
   size_t bufused;

   if (fdc == 0)
      return NULL;

   if (buf == NULL || buflen == 0) {
      static char bufmem[   (31 + 10) /* number of keywords + 10 extra keyw. */
                         *  (20 /* length of keyword */ + 10 /* key value */)
                         *  2   /* max fdc. */];


      CTASSERT(sizeof(bufmem) > MAXTCPINFOLEN);

      buf    = bufmem;
      buflen = sizeof(bufmem);
   }

   if (fdc > 0) {
      len = sizeof(info_0);
      if (getsockopt(fdv[0], IPPROTO_TCP, TCP_INFO, &info_0, &len) != 0) {
         slog(LOG_DEBUG,
              "%s: getsockopt(TCP_INFO) on fd %d (fdv[0]) failed: %s",
              function, fdv[0], strerror(errno));

         fdv[0] = -1;
      }
   }

   if (fdc > 1) {
      len = sizeof(info_1);
      if (getsockopt(fdv[1], IPPROTO_TCP, TCP_INFO, &info_1, &len) != 0) {
         slog(LOG_DEBUG,
              "%s: getsockopt(TCP_INFO) on fd %d (fdv[1]) failed: %s",
              function, fdv[1], strerror(errno));

         fdv[1] = -1;
      }
   }

   switch (fdc) {
      case 1:
         if (fdv[0] == -1)
            return NULL;
         break;

      case 2:
         if (fdv[0] == -1 && fdv[1] == -1)
            return NULL;
         break;

      default:
         SERRX(fdc);
   }

   bufused = 0;

#define ADDATTR(fdc, fdv, attr, _func, tcpinfo_0, tcpinfo_1)                   \
do {                                                                           \
   const char * (*func)(unsigned long value) = (_func);                        \
                                                                               \
   if (fdc == 1) {                                                             \
      bufused += snprintf(&buf[bufused], buflen - bufused,                     \
                         "%-20s : %lu%s%s%s\n",                                \
                         #attr,                                                \
                         (unsigned long)((tcpinfo_0)->attr),                   \
                         func == NULL ? "" : " (",                             \
                         func == NULL ? "" : func((tcpinfo_0)->attr),          \
                         func == NULL ? "" : ")");                             \
   }                                                                           \
   else if (fdc == 2) {                                                        \
      if (fdv[0] >= 0 && fdv[1] >= 0)                                          \
         bufused += snprintf(&buf[bufused], buflen - bufused,                  \
                             "%-20s : %lu%s%s%s <-> %lu%s%s%s\n",              \
                             #attr,                                            \
                             (unsigned long)((tcpinfo_0)->attr),               \
                             func == NULL ? "" : " (",                         \
                             func == NULL ? "" : func((tcpinfo_0)->attr),      \
                             func == NULL ? "" : ")",                          \
                             (unsigned long)((tcpinfo_1)->attr),               \
                             func == NULL ? "" : " (",                         \
                             func == NULL ? "" : func((tcpinfo_1)->attr),      \
                             func == NULL ? "" : ")");                         \
      else {                                                                   \
         if (fdv[0] >= 0) {                                                    \
            SASSERTX(fdv[1] == -1);                                            \
                                                                               \
            bufused += snprintf(&buf[bufused], buflen - bufused,               \
                                "%-20s : %lu%s%s%s <-> N/A\n",                 \
                                #attr,                                         \
                                (unsigned long)((tcpinfo_0)->attr),            \
                                func == NULL ? "" : " (",                      \
                                func == NULL ? "" : func((tcpinfo_0)->attr),   \
                                func == NULL ? "" : ")");                      \
         }                                                                     \
         else if (fdv[1] >= 0) {                                               \
            SASSERTX(fdv[0] == -1);                                            \
                                                                               \
            bufused += snprintf(&buf[bufused], buflen - bufused,               \
                                "%-20s : N/A <-> %lu%s%s%s\n",                 \
                                #attr,                                         \
                                (unsigned long)((tcpinfo_1)->attr),            \
                                func == NULL ? "" : " (",                      \
                                func == NULL ? "" : func((tcpinfo_1)->attr),   \
                                func == NULL ? "" : ")");                      \
         }                                                                     \
         else                                                                  \
            SERRX(0);                                                          \
      }                                                                        \
   }                                                                           \
   else                                                                        \
      SERRX(fdc);                                                              \
} while (/* CONSTCOND */ 0)

#define ADDNL()                                                                \
do {                                                                           \
   bufused += snprintf(&buf[bufused], buflen - bufused, "\n");                 \
} while (/* CONSTCOND */ 0)

   /*
    * XXX FreeBSD has some further extensions.  Add support
    * for them when we have finished the analysis, as Linux
    * is what customer is using.
    *
    * XXX most comments regarding meaning are as of currently
    * unverified and just guesses.  Add a "verified" comment
    * once verified.
    */

   ADDATTR(fdc, fdv, tcpi_state, tcpi_state2string, &info_0, &info_1);

#if HAVE_TCP_INFO_TCPI_CA_STATE
   /*
    * enum tcp_ca_state.
    */
   ADDATTR(fdc, fdv, tcpi_ca_state, tcpi_ca_state2string, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_CA_STATE */

#if HAVE_TCP_INFO_TCPI_RETRANSMITS
   /*
    * Number of times we have retransmitted currently outstanding
    * data, based on ACK timeouts?  Reset every time successfully ACK'ed
    * and things are moving on? (and possibly in a few other cases too?)
    * [Mostly verified]
    */
   ADDATTR(fdc, fdv, tcpi_retransmits, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_CA_STATE */

#if HAVE_TCP_INFO_TCPI_PROBES
   /*
    * Number of tcp zero window probes sent.
    * Sent if peer advertizes a zero receive window size,
    * preventing us from sending it any more data.
    *
    * Think this is sent if the peer receive window has been zero
    * for a while (without updates to refresh it as zero), and
    * this counts for how long (how many probes) it has currently
    * been zero.
    * Not sure if the max value, before we trigger a send error
    * is the sysctl tcp_retries2, or if the source code comment
    * is still correct in that there is no max value.
    */
   ADDATTR(fdc, fdv, tcpi_probes, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_PROBES */

#if HAVE_TCP_INFO_TCPI_BACKOFF
   /*
    * Current backoff value.  Used to calculate how long to wait
    * before retransmitting un-acked data.
    * The wait is calculated as a factor of this value, so would
    * think this should have the same value as tcpi_retransmits,
    * but looks like this value is reset in some cases where
    * tcpi_retransmits is not.
    *
    * Looks like it is also limited by sysctl tcp_retries2.
    */
   ADDATTR(fdc, fdv, tcpi_backoff, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_BACKOFF */

   /*
    * Options set on socket.
    */
   ADDATTR(fdc, fdv, tcpi_options, tcpi_options2string, &info_0, &info_1);

   /*
    * how much peer told us to scale his receive windows size by?
    */
   ADDATTR(fdc, fdv, tcpi_snd_wscale, NULL, &info_0, &info_1);

   /*
    * how much we told peer to scale our windows receive size by?
    */
   ADDATTR(fdc, fdv, tcpi_rcv_wscale, NULL, &info_0, &info_1);

   /*
    * Retransmission timeout.  Microseconds.
    */
   ADDATTR(fdc, fdv, tcpi_rto, NULL, &info_0, &info_1);

#if HAVE_TCP_INFO_TCPI_ATO
   /*
    * Ack timeout.  Microseconds.
    */
   ADDATTR(fdc, fdv, tcpi_ato, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_ATO */

   /*
    * Maximum segment size for outgoing TCP packets.  Bytes.
    */
   ADDATTR(fdc, fdv, tcpi_snd_mss, NULL, &info_0, &info_1);

   /*
    * Maximum segment size for outgoing TCP packets used by peer.  Bytes.
    * A guess made by our side.
    */
   ADDATTR(fdc, fdv, tcpi_rcv_mss, NULL, &info_0, &info_1);

#if HAVE_TCP_INFO_TCPI_UNACKED
   /*
    * Number of packets sent by us, but not yet ack'ed by peer.
    */
   ADDATTR(fdc, fdv, tcpi_unacked, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_UNACKED */

#if HAVE_TCP_INFO_TCPI_SACKED
   /*
    * Packets that arrived at peer out of order.
    * If TCPI_OPT_SACK is not enabled, this value is still set,
    * but based on an estimate (duplicate acks)?
    * Reset ... when?
    */
   ADDATTR(fdc, fdv, tcpi_sacked, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_SACKED */

#if HAVE_TCP_INFO_TCPI_LOST
   /*
    * Packets lost by network.
    * Reset when the lost data has been retransmitted?
    */
   ADDATTR(fdc, fdv, tcpi_lost, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_LOST */

#if HAVE_TCP_INFO_TCPI_RETRANS
   /*
    * Number of packets retransmitted due to non-timeout reasons (e.g.,
    * the TCP Fast Retransmit feature).  Reset ... when?
    */
   ADDATTR(fdc, fdv, tcpi_retrans, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_RETRANS */

#if HAVE_TCP_INFO_TCPI_FACKETS
   /*
    * Forward ack.  Packets.
    */
   ADDATTR(fdc, fdv, tcpi_fackets, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_FACKETS */

   /*
    * Times since last sent/received.  All in milliseconds.
    * Note that some (all?) of these seems to contain some random value
    * initially.  At least for tcpi_last_data_recv that appears to be
    * the case.
    */

   ADDNL();

#if HAVE_TCP_INFO_TCPI_LAST_DATA_SENT
   /*
    * Time since we last sent any data.
    */
   ADDATTR(fdc, fdv, tcpi_last_data_sent, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_LAST_DATA_SENT */

#if HAVE_TCP_INFO_TCPI_LAST_ACK_SENT
   /*
    * Time since we last sent an ack.
    * NOTE: Linux does not appears to keep track of this value; always zero.
    */
   ADDATTR(fdc, fdv, tcpi_last_ack_sent, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_LAST_ACK_SENT */

#if HAVE_TCP_INFO_TCPI_LAST_DATA_RECV
   /*
    * Time since we last received any data?
    */
   ADDATTR(fdc, fdv, tcpi_last_data_recv, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_LAST_DATA_RECV */

#if HAVE_TCP_INFO_TCPI_LAST_ACK_RECV
   /*
    * Time since we last received an ack. [Verified]
    */
   ADDATTR(fdc, fdv, tcpi_last_ack_recv, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_LAST_ACK_RECV */

   /*
    * Metrics.
    */

   ADDNL();

#if HAVE_TCP_INFO_TCPI_PMTU
   /*
    * Path MTU.  Bytes.
    * What if not known?  Guessed?
    */
   ADDATTR(fdc, fdv, tcpi_pmtu, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_PMTU */

#if HAVE_TCP_INFO_TCPI_RCV_SSTHRESH
   /*
    * receive slow start threshold?
    * Something related to the receive window we advertise,
    * based on how well we have performed (how fast we've
    * read data out of our socket buffer?) in the past and
    * how much memory is available?
    */
   ADDATTR(fdc, fdv, tcpi_rcv_ssthresh, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_RCV_SSTHRESH */

   /*
    * Estimated round-trip-time?
    * Possibly max of the moving average and last rtt calculation
    * done (which is not necessarily the rtt of last packet).
    * Microseconds.  If no value calculated, set to 1000.
    */
   ADDATTR(fdc, fdv, tcpi_rtt, NULL, &info_0, &info_1);

   /*
    * median deviation for tcpi_rtt?  Microseconds.
    */
   ADDATTR(fdc, fdv, tcpi_rttvar, NULL, &info_0, &info_1);

   /*
    * send slow start threshold?
    */
   ADDATTR(fdc, fdv, tcpi_snd_ssthresh, NULL, &info_0, &info_1);

   /*
    * Our current congestion window towards peer.  Packets.
    */
   ADDATTR(fdc, fdv, tcpi_snd_cwnd, NULL, &info_0, &info_1);

#if HAVE_TCP_INFO_TCPI_ADVMSS
   /*
    * MSS we advertise to peer.  If advertised, advertised as part
    * of TCP options.  Otherwise peer has to assume minimum, 536?
    * Number of bytes.
    */
   ADDATTR(fdc, fdv, tcpi_advmss, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_ADVMSS */

#if HAVE_TCP_INFO_TCPI_REORDERING
   /*
    * Same as sysctl tcp_reordering?  I.e. not dynamically updated?
    * If so, why here?
    */
   ADDATTR(fdc, fdv, tcpi_reordering, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_REORDERING */

#if HAVE_TCP_INFO_TCPI_RCV_RTT
   /*
    * "receivers (ours) estimated rtt".  Same as tcpi_rtt, but
    * estimated another way? (without timestamps?)  Microseconds.
    */
   ADDATTR(fdc, fdv, tcpi_rcv_rtt, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_RCV_RTT */

#if HAVE_TCP_INFO_TCPI_RCV_SPACE
   /*
    * Size of our receive buffer?
    * Tuned/modified while session is running?
    * Number of bytes.
    */
   ADDATTR(fdc, fdv, tcpi_rcv_space, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_RCV_SPACE */

   /* end of metrics. */
   ADDNL();

#if HAVE_TCP_INFO_TCPI_TOTAL_RETRANS
   /*
    * Total number of packets retransmitted on this connection,
    * regardless of reason.
    * Note that this appears to be calculated based on the data, so if e.g.
    * a large packet is retransmitted as two smaller packets, that counts as
    * two retransmits, not one.
    * [Verified]
    */
   ADDATTR(fdc, fdv, tcpi_total_retrans, NULL, &info_0, &info_1);
#endif /* HAVE_TCP_INFO_TCPI_TOTAL_RETRANS */

   return buf;

#else

   return NULL;

#endif /* HAVE_TCP_INFO */
}

#if HAVE_TCP_INFO

const char *tcpi_ca_state2string(val)
   const unsigned long val;
{

#if defined __linux__
   switch (val) {
      case TCP_CA_Open:
         return "TCP_CA_Open";

      case TCP_CA_Disorder:
         return "TCP_CA_Disorder";

      case TCP_CA_CWR:
         return "TCP_CA_CWR";

      case TCP_CA_Recovery:
         return "TCP_CA_Recovery";

      case TCP_CA_Loss:
         return "TCP_CA_Loss";
   }
#endif /* __linux__ */

   return "<undecoded>";
}

const char *tcpi_state2string(val)
   const unsigned long val;
{

#if defined(__FreeBSD__)
   switch (val) {
      case TCPS_CLOSED:
         return "CLOSED";

      case TCPS_LISTEN:
         return "LISTEN";

      case TCPS_SYN_SENT:
         return "SYN_SENT";

      case TCPS_SYN_RECEIVED:
         return "SYN_RECEIVED";

      case TCPS_ESTABLISHED:
         return "ESTABLISHED";

      case TCPS_CLOSE_WAIT:
         return "CLOSE_WAIT";

      case TCPS_FIN_WAIT_1:
         return "FIN_WAIT_1";

      case TCPS_CLOSING:
         return "CLOSING";

      case TCPS_LAST_ACK:
         return "LAST_ACK";

      case TCPS_FIN_WAIT_2:
         return "FIN_WAIT_2";

      case TCPS_TIME_WAIT:
         return "TIME_WAIT";
   }

#elif defined(__linux__)

   switch (val) {
      case TCP_ESTABLISHED:
         return "ESTABLISHED";

      case TCP_SYN_SENT:
         return "SYN_SENT";

      case TCP_SYN_RECV:
         return "SYN_RECV";

      case TCP_FIN_WAIT1:
         return "FIN_WAIT1";

      case TCP_FIN_WAIT2:
         return "FIN_WAIT2";

      case TCP_TIME_WAIT:
         return "TIME_WAIT";

      case TCP_CLOSE:
         return "CLOSE";

      case TCP_CLOSE_WAIT:
         return "CLOSE_WAIT";

      case TCP_LAST_ACK:
         return "LAST_ACK";

      case TCP_LISTEN:
         return "LISTEN";

      case TCP_CLOSING:
         return "CLOSING";
   }
#endif /* __linux__ */

   return "<undecoded>";
}

const char *tcpi_options2string(val)
   const unsigned long val;
{
   static char buf[256];
   size_t bufused;

   *buf    = NUL;
   bufused = 0;

   if (val & TCPI_OPT_TIMESTAMPS)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                          "%sTS",
                          *buf == NUL ? "" : ", ");

   if (val & TCPI_OPT_SACK)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                          "%sSACK",
                          *buf == NUL ? "" : ", ");

   if (val & TCPI_OPT_WSCALE)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                          "%sWscale",
                          *buf == NUL ? "" : ", ");

   if (val & TCPI_OPT_ECN)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                          "%sECN",
                          *buf == NUL ? "" : ", ");

   return buf;
}

#endif /* HAVE_TCP_INFO */
