/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2014 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "rtpp_types.h"
#include "rtp.h"
#include "rtp_resizer.h"
#include "rtp_server.h"
#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_network.h"
#include "rtpp_notify.h"
#include "rtpp_netio_async.h"
#include "rtpp_proc.h"
#include "rtpp_record.h"
#include "rtpp_session.h"
#include "rtpp_stats.h"
#include "rtpp_util.h"

struct rtpp_proc_ready_lst {
    struct rtpp_session *sp;
    int ridx;
};

static void send_packet(struct cfg *, struct rtpp_session *, int, \
  struct rtp_packet *, struct sthread_args *, struct rtpp_proc_rstats *);

void
process_rtp_servers(struct cfg *cf, double dtime, struct sthread_args *sender,
  struct rtpp_proc_rstats *rsp)
{
    int j, sidx, len, skipfd;
    struct rtpp_session *sp;
    struct rtp_packet *pkt;

    skipfd = 0;
    for (j = 0; j < cf->rtp_nsessions; j++) {
	sp = cf->rtp_servers[j];
	if (sp == NULL) {
	    skipfd++;
	    continue;
	}
	if (skipfd > 0) {
	    cf->rtp_servers[j - skipfd] = cf->rtp_servers[j];
	    sp->sridx = j - skipfd;
	}
	for (sidx = 0; sidx < 2; sidx++) {
	    if (sp->rtps[sidx] == NULL || sp->addr[sidx] == NULL)
		continue;
            for (;;) {
                pkt = rtp_server_get(sp->rtps[sidx], dtime, &len);
                if (pkt == NULL) {
                    if (len == RTPS_EOF) {
                        rtp_server_free(sp->rtps[sidx]);
                        CALL_METHOD(cf->stable->rtpp_stats, updatebyname, "nplrs_destroyed", 1);
                        sp->rtps[sidx] = NULL;
                        if (sp->rtps[0] == NULL && sp->rtps[1] == NULL) {
                            assert(cf->rtp_servers[sp->sridx] == sp);
                            cf->rtp_servers[sp->sridx] = NULL;
                            sp->sridx = -1;
                        }
                    } else if (len != RTPS_LATER) {
                        /* XXX some error, brag to logs */
                    }
                    break;
		}
                rtpp_anetio_send_pkt(sender, sp->fds[sidx], sp->addr[sidx], \
                  SA_LEN(sp->addr[sidx]), pkt);
                rsp->npkts_played.cnt++;
	    }
	}
    }
    cf->rtp_nsessions -= skipfd;
}

static void
rxmit_packets(struct cfg *cf, struct rtpp_proc_ready_lst *rready, int rlen,
  double dtime, int drain_repeat, struct sthread_args *sender,
  struct rtpp_proc_rstats *rsp)
{
    int ndrain, i, port, rn, ridx;
    struct rtp_packet *packet = NULL;
    struct rtpp_session *sp;

    /* Repeat since we may have several packets queued on the same socket */
    ndrain = -1;
    for (rn = 0; rn < rlen; rn += (ndrain > 0) ? 0 : 1) {
        if (ndrain < 0) {
            ndrain = drain_repeat - 1;
        } else {
            ndrain -= 1;
        }
	if (packet != NULL)
	    rtp_packet_free(packet);

        sp = rready[rn].sp;
        ridx = rready[rn].ridx;

	packet = rtp_recv(sp->fds[ridx]);
	if (packet == NULL) {
            /* Move on to the next session */
            ndrain = -1;
	    continue;
        }
	packet->laddr = sp->laddr[ridx];
	packet->rport = sp->ports[ridx];
	packet->rtime = dtime;
        rsp->npkts_rcvd.cnt++;

	i = 0;

	port = ntohs(satosin(&packet->raddr)->sin_port);

	if (sp->addr[ridx] != NULL) {
	    /* Check that the packet is authentic, drop if it isn't */
	    if (sp->asymmetric[ridx] == 0) {
		if (memcmp(sp->addr[ridx], &packet->raddr, packet->rlen) != 0) {
		    if (sp->canupdate[ridx] == 0) {
			/*
			 * Continue, since there could be good packets in
			 * queue.
			 */
                        ndrain += 1;
                        rsp->npkts_discard.cnt++;
			continue;
		    }
		    /* Signal that an address has to be updated */
		    i = 1;
		} else if (sp->canupdate[ridx] != 0) {
		    if (sp->last_update[ridx] == 0 ||
		      dtime - sp->last_update[ridx] > UPDATE_WINDOW) {
			rtpp_log_write(RTPP_LOG_INFO, sp->log,
			  "%s's address latched in: %s:%d (%s)",
			  (ridx == 0) ? "callee" : "caller",
			  addr2char(sstosa(&packet->raddr)), port,
			  (sp->rtp == NULL) ? "RTP" : "RTCP");
			sp->canupdate[ridx] = 0;
		    }
		}
	    } else {
		/*
		 * For asymmetric clients don't check
		 * source port since it may be different.
		 */
		if (!ishostseq(sp->addr[ridx], sstosa(&packet->raddr))) {
		    /*
		     * Continue, since there could be good packets in
		     * queue.
		     */
                    ndrain += 1;
                    rsp->npkts_discard.cnt++;
		    continue;
                }
	    }
	    sp->pcount[ridx]++;
	} else {
	    sp->pcount[ridx]++;
	    sp->addr[ridx] = malloc(packet->rlen);
	    if (sp->addr[ridx] == NULL) {
		sp->pcount[3]++;
		rtpp_log_write(RTPP_LOG_ERR, sp->log,
		  "can't allocate memory for remote address - "
		  "removing session");
		remove_session(cf, GET_RTP(sp));
		/* Move on to the next session, sp is invalid now */
                ndrain = -1;
                rsp->npkts_discard.cnt++;
		continue;
	    }
	    /* Signal that an address have to be updated. */
	    i = 1;
	}

	/*
	 * Update recorded address if it's necessary. Set "untrusted address"
	 * flag in the session state, so that possible future address updates
	 * from that client won't get address changed immediately to some
	 * bogus one.
	 */
	if (i != 0) {
	    sp->untrusted_addr[ridx] = 1;
	    memcpy(sp->addr[ridx], &packet->raddr, packet->rlen);
	    if (sp->prev_addr[ridx] == NULL || memcmp(sp->prev_addr[ridx],
	      &packet->raddr, packet->rlen) != 0) {
	        sp->canupdate[ridx] = 0;
	    }

	    rtpp_log_write(RTPP_LOG_INFO, sp->log,
	      "%s's address filled in: %s:%d (%s)",
	      (ridx == 0) ? "callee" : "caller",
	      addr2char(sstosa(&packet->raddr)), port,
	      (sp->rtp == NULL) ? "RTP" : "RTCP");

	    /*
	     * Check if we have updated RTP while RTCP is still
	     * empty or contains address that differs from one we
	     * used when updating RTP. Try to guess RTCP if so,
	     * should be handy for non-NAT'ed clients, and some
	     * NATed as well.
	     */
	    if (sp->rtcp != NULL && (sp->rtcp->addr[ridx] == NULL ||
	      !ishostseq(sp->rtcp->addr[ridx], sstosa(&packet->raddr)))) {
		if (sp->rtcp->addr[ridx] == NULL) {
		    sp->rtcp->addr[ridx] = malloc(packet->rlen);
		    if (sp->rtcp->addr[ridx] == NULL) {
			sp->pcount[3]++;
			rtpp_log_write(RTPP_LOG_ERR, sp->log,
			  "can't allocate memory for remote address - "
			  "removing session");
			remove_session(cf, sp);
			/* Move on to the next session, sp is invalid now */
                        ndrain = -1;
                        rsp->npkts_discard.cnt++;
			continue;
		    }
		}
		memcpy(sp->rtcp->addr[ridx], &packet->raddr, packet->rlen);
		satosin(sp->rtcp->addr[ridx])->sin_port = htons(port + 1);
		/* Use guessed value as the only true one for asymmetric clients */
		sp->rtcp->canupdate[ridx] = NOT(sp->rtcp->asymmetric[ridx]);
		rtpp_log_write(RTPP_LOG_INFO, sp->log, "guessing RTCP port "
		  "for %s to be %d",
		  (ridx == 0) ? "callee" : "caller", port + 1);
	    }
	}

	if (sp->resizers[ridx] != NULL) {
	    rtp_resizer_enqueue(sp->resizers[ridx], &packet, rsp);
            if (packet == NULL) {
                rsp->npkts_resizer_in.cnt++;
            }
        }
	if (packet != NULL) {
	    send_packet(cf, sp, ridx, packet, sender, rsp);
            packet = NULL;
        }
    }
    if (packet != NULL)
        rtp_packet_free(packet);
}

static void
send_packet(struct cfg *cf, struct rtpp_session *sp, int ridx,
  struct rtp_packet *packet, struct sthread_args *sender, 
  struct rtpp_proc_rstats *rsp)
{
    int sidx;


    GET_RTP(sp)->ttl[ridx] = cf->stable->max_ttl;

    /* Select socket for sending packet out. */
    sidx = (ridx == 0) ? 1 : 0;

    if (sp->rrcs[ridx] != NULL && GET_RTP(sp)->rtps[sidx] == NULL) {
        rwrite(sp, sp->rrcs[ridx], packet, sp->addr[sidx], sp->laddr[sidx],
          sp->ports[sidx], sidx);
    }

    /*
     * Check that we have some address to which packet is to be
     * sent out, drop otherwise.
     */
    if (sp->addr[sidx] == NULL || GET_RTP(sp)->rtps[sidx] != NULL) {
        rtp_packet_free(packet);
	sp->pcount[3]++;
        rsp->npkts_discard.cnt++;
    } else {
        rtpp_anetio_send_pkt(sender, sp->fds[sidx], sp->addr[sidx], \
          SA_LEN(sp->addr[sidx]), packet);
        sp->pcount[2]++;
        rsp->npkts_relayed.cnt++;
    }
}

static void
drain_socket(int rfd, struct rtpp_proc_rstats *rsp)
{
    struct rtp_packet *packet;

    for (;;) {
        packet = rtp_recv(rfd);
        if (packet == NULL)
            break;
        rsp->npkts_discard.cnt++;
        rtp_packet_free(packet);
    }
}

#define	RR_ADD_PUSH(__rready, __rready_len, __sp, __ridx) { \
  __rready[__rready_len].sp = __sp; \
  __rready[rready_len].ridx = __ridx; \
  __rready_len += 1; \
  if (__rready_len == 10) { \
    rxmit_packets(cf, __rready, __rready_len, dtime, drain_repeat, sender, rsp); \
    __rready_len = 0; \
  } }\

static int
find_ridx(struct cfg *cf, int readyfd, struct rtpp_session *sp)
{
    int ridx;

    for (ridx = 0; ridx < 2; ridx++)
        if (cf->sessinfo.pfds_rtp[readyfd].fd == sp->fds[ridx])
            break;
    /*
     * Can't happen.
     */
    assert(ridx != 2);

    return (ridx);
}

void
process_rtp_only(struct cfg *cf, double dtime, int drain_repeat, \
  struct sthread_args *sender, struct rtpp_proc_rstats *rsp)
{
    int readyfd, ridx, rready_len;
    struct rtpp_session *sp;
    struct rtp_packet *packet;
    struct rtpp_proc_ready_lst rready[10];

    rready_len = 0;
    pthread_mutex_lock(&cf->sessinfo.lock);
    for (readyfd = 0; readyfd < cf->sessinfo.nsessions; readyfd++) {
        sp = cf->sessinfo.sessions[readyfd];

        if (cf->sessinfo.pfds_rtp[readyfd].fd == -1) {
            /* Deleted session, move one */
            continue;
        }
        if (sp->complete != 0) {
            ridx = find_ridx(cf, readyfd, sp);
            if ((cf->sessinfo.pfds_rtp[readyfd].revents & POLLIN) != 0) {
                RR_ADD_PUSH(rready, rready_len, sp, ridx);
            }
            if (sp->resizers[ridx] != NULL) {
                while ((packet = rtp_resizer_get(sp->resizers[ridx], dtime)) != NULL) {
                    send_packet(cf, sp, ridx, packet, sender, rsp);
                    rsp->npkts_resizer_out.cnt++;
                    packet = NULL;
                }
            }
        } else if ((cf->sessinfo.pfds_rtp[readyfd].revents & POLLIN) != 0) {
#if RTPP_DEBUG
            rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "Draining RTP socket %d", cf->sessinfo.pfds_rtp[readyfd].fd);
#endif
            drain_socket(cf->sessinfo.pfds_rtp[readyfd].fd, rsp);
        }
    }
    if (rready_len > 0) {
        rxmit_packets(cf, rready, rready_len, dtime, drain_repeat, sender, rsp);
        rready_len = 0;
    }
    pthread_mutex_unlock(&cf->sessinfo.lock);
}

void
process_rtp(struct cfg *cf, double dtime, int alarm_tick, int drain_repeat, \
  struct sthread_args *sender, struct rtpp_proc_rstats *rsp)
{
    int readyfd, skipfd, ridx, rready_len;
    struct rtpp_session *sp;
    struct rtp_packet *packet;
    struct rtpp_proc_ready_lst rready[10];

    /* Relay RTP/RTCP */
    skipfd = 0;
    rready_len = 0;
    pthread_mutex_lock(&cf->sessinfo.lock);
    for (readyfd = 0; readyfd < cf->sessinfo.nsessions; readyfd++) {
	sp = cf->sessinfo.sessions[readyfd];

	if (alarm_tick != 0 && sp != NULL && sp->sidx[0] == readyfd) {
	    if (get_ttl(sp) == 0) {
		rtpp_log_write(RTPP_LOG_INFO, sp->log, "session timeout");
		rtpp_notify_schedule(cf, sp);
		remove_session(cf, sp);
		CALL_METHOD(cf->stable->rtpp_stats, updatebyname, "nsess_timeout", 1);
	    } else {
		if (sp->ttl[0] != 0)
		    sp->ttl[0]--;
		if (sp->ttl[1] != 0)
		    sp->ttl[1]--;
	    }
	}

	if (cf->sessinfo.pfds_rtp[readyfd].fd == -1) {
	    /* Deleted session, count and move one */
	    skipfd++;
	    continue;
	}

	/* Find index of the call leg within a session */
        ridx = find_ridx(cf, readyfd, sp);

	/* Compact pfds[] and sessions[] by eliminating removed sessions */
	if (skipfd > 0) {
	    cf->sessinfo.pfds_rtp[readyfd - skipfd] = cf->sessinfo.pfds_rtp[readyfd];
	    cf->sessinfo.pfds_rtcp[readyfd - skipfd] = cf->sessinfo.pfds_rtcp[readyfd];
	    cf->sessinfo.sessions[readyfd - skipfd] = cf->sessinfo.sessions[readyfd];
	    sp->sidx[ridx] = readyfd - skipfd;
	    sp->rtcp->sidx[ridx] = readyfd - skipfd;
	}

	if (sp->complete != 0) {
	    if ((cf->sessinfo.pfds_rtp[readyfd].revents & POLLIN) != 0) {
                RR_ADD_PUSH(rready, rready_len, sp, ridx);
            }
            if ((cf->sessinfo.pfds_rtcp[readyfd].revents & POLLIN) != 0) {
                RR_ADD_PUSH(rready, rready_len, sp->rtcp, ridx);
            }
	    if (sp->resizers[ridx] != NULL) {
		while ((packet = rtp_resizer_get(sp->resizers[ridx], dtime)) != NULL) {
		    send_packet(cf, sp, ridx, packet, sender, rsp);
                    rsp->npkts_resizer_out.cnt++;
		    packet = NULL;
		}
	    }
	} else if ((cf->sessinfo.pfds_rtp[readyfd].revents & POLLIN) != 0) {
#if RTPP_DEBUG
            rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "Draining RTP socket %d", cf->sessinfo.pfds_rtp[readyfd].fd);
#endif
            drain_socket(cf->sessinfo.pfds_rtp[readyfd].fd, rsp);
        } else if ((cf->sessinfo.pfds_rtcp[readyfd].revents & POLLIN) != 0) {
#if RTPP_DEBUG
            rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "Draining RTCP socket %d", cf->sessinfo.pfds_rtcp[readyfd].fd);
#endif
            drain_socket(cf->sessinfo.pfds_rtcp[readyfd].fd, rsp);
        }
    }
    if (rready_len > 0) {
        rxmit_packets(cf, rready, rready_len, dtime, drain_repeat, sender, rsp);
        rready_len = 0;
    }
    /* Trim any deleted sessions at the end */
    cf->sessinfo.nsessions -= skipfd;
    pthread_mutex_unlock(&cf->sessinfo.lock);
}
