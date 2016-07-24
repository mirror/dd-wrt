/*
 * Copyright (c) 2014 Sippy Software, Inc., http://www.sippysoft.com
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
#include <netinet/in.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_command_async.h"
#ifdef RTPP_DEBUG
#include "rtpp_math.h"
#endif
#include "rtpp_netio_async.h"
#include "rtpp_proc.h"
#include "rtpp_proc_async.h"
#include "rtpp_queue.h"
#include "rtpp_wi.h"
#include "rtpp_util.h"
#include "rtpp_types.h"
#include "rtpp_stats.h"

struct rtpp_proc_async_cf {
    pthread_t thread_id;
    int clock_tick;
    long long ncycles_ref;
    struct rtpp_anetio_cf *op;
    struct rtpp_queue *time_q;
#if RTPP_DEBUG
    struct recfilter sleep_time;
    struct recfilter poll_time;
    struct recfilter proc_time;
#endif
    struct rtpp_proc_rstats rstats;
};

struct sign_arg {
    int clock_tick;
    long long ncycles_ref;
};

#define FLUSH_STAT(sobj, st)	{ \
    if ((st).cnt > 0) { \
        CALL_METHOD(sobj, updatebyidx, (st).cnt_idx, (st).cnt); \
        (st).cnt = 0; \
    } \
}

static void
flush_rstats(struct rtpp_stats_obj *sobj, struct rtpp_proc_rstats *rsp)
{

    FLUSH_STAT(sobj, rsp->npkts_rcvd);
    FLUSH_STAT(sobj, rsp->npkts_played);
    FLUSH_STAT(sobj, rsp->npkts_relayed);
    FLUSH_STAT(sobj, rsp->npkts_resizer_in);
    FLUSH_STAT(sobj, rsp->npkts_resizer_out);
    FLUSH_STAT(sobj, rsp->npkts_resizer_discard);
    FLUSH_STAT(sobj, rsp->npkts_discard);
}

static void
init_rstats(struct rtpp_stats_obj *sobj, struct rtpp_proc_rstats *rsp)
{

    rsp->npkts_rcvd.cnt_idx = CALL_METHOD(sobj, getidxbyname, "npkts_rcvd");
    rsp->npkts_played.cnt_idx = CALL_METHOD(sobj, getidxbyname, "npkts_played");
    rsp->npkts_relayed.cnt_idx = CALL_METHOD(sobj, getidxbyname, "npkts_relayed");
    rsp->npkts_resizer_in.cnt_idx = CALL_METHOD(sobj, getidxbyname, "npkts_resizer_in");
    rsp->npkts_resizer_out.cnt_idx = CALL_METHOD(sobj, getidxbyname, "npkts_resizer_out");
    rsp->npkts_resizer_discard.cnt_idx = CALL_METHOD(sobj, getidxbyname, "npkts_resizer_discard");
    rsp->npkts_discard.cnt_idx = CALL_METHOD(sobj, getidxbyname, "npkts_discard");
}

static void
rtpp_proc_async_run(void *arg)
{
    struct cfg *cf;
    double last_tick_time;
    int alarm_tick, i, ndrain, rtp_only;
    struct rtpp_proc_async_cf *proc_cf;
    long long ncycles_ref;
#ifdef RTPP_DEBUG
    int ncycles_ref_pre, last_ctick;
#endif
    struct sign_arg *s_a;
    struct rtpp_wi *wi, *wis[10];
    struct sthread_args *sender;
    double tp[4];
    struct rtpp_proc_rstats *rstats;
    struct rtpp_stats_obj *stats_cf;

    cf = (struct cfg *)arg;
    proc_cf = cf->stable->rtpp_proc_cf;
    stats_cf = cf->stable->rtpp_stats;
    rstats = &proc_cf->rstats;

    last_tick_time = 0;
    wi = rtpp_queue_get_item(proc_cf->time_q, 0);
    s_a = (struct sign_arg *)rtpp_wi_sgnl_get_data(wi, NULL);
#ifdef RTPP_DEBUG
    last_ctick = s_a->clock_tick;
    ncycles_ref_pre = s_a->ncycles_ref;
#endif
    rtpp_wi_free(wi);

    tp[0] = getdtime();
    for (;;) {
        i = rtpp_queue_get_items(proc_cf->time_q, wis, 10, 0);
        if (i <= 0) {
            continue;
        }
        i -= 1;
        s_a = (struct sign_arg *)rtpp_wi_sgnl_get_data(wis[i], NULL);
        ndrain = (s_a->ncycles_ref - ncycles_ref) / (cf->stable->target_pfreq / MAX_RTP_RATE);
#ifdef RTPP_DEBUG
        last_ctick = s_a->clock_tick;
        ncycles_ref_pre = ncycles_ref;
#endif
        ncycles_ref = s_a->ncycles_ref;
        for(; i > -1; i--) {
            rtpp_wi_free(wis[i]);
        }

        tp[1] = getdtime();
#if RTPP_DEBUG
        if (last_ctick % (unsigned int)cf->stable->target_pfreq == 0 || last_ctick < 1000) {
            rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "run %lld sptime %f, CSV: %f,%f,%f", \
              last_ctick, tp[1], (double)last_ctick / cf->stable->target_pfreq, \
              ((double)ncycles_ref / cf->stable->target_pfreq) - tp[1], tp[1]);
        }
#endif

        if (ndrain < 1) {
            ndrain = 1;
        }

#if RTPP_DEBUG
        if (ndrain > 1) {
            rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "run %lld " \
              "ncycles_ref %lld, ncycles_ref_pre %lld, ndrain %d CSV: %f,%f,%d", \
              last_ctick, ncycles_ref, ncycles_ref_pre, ndrain, \
              (double)last_ctick / cf->stable->target_pfreq, ndrain);
        }
#endif

        alarm_tick = 0;
        if (last_tick_time == 0 || last_tick_time > tp[1]) {
            last_tick_time = tp[1];
        } else if (last_tick_time + (double)TIMETICK < tp[1]) {
            alarm_tick = 1;
            last_tick_time = tp[1];
        }

        if (alarm_tick || (ncycles_ref % 7) == 0) {
            rtp_only = 0;
        } else {
            rtp_only = 1;
        }

        pthread_mutex_lock(&cf->sessinfo.lock);
        if (cf->sessinfo.nsessions > 0) {
            if (rtp_only == 0) {
                i = poll(cf->sessinfo.pfds_rtcp, cf->sessinfo.nsessions, 0);
            }
            i = poll(cf->sessinfo.pfds_rtp, cf->sessinfo.nsessions, 0);
            pthread_mutex_unlock(&cf->sessinfo.lock);
            if (i < 0 && errno == EINTR) {
                rtpp_command_async_wakeup(cf->stable->rtpp_cmd_cf);
                tp[0] = getdtime();
                continue;
            }
        } else {
            pthread_mutex_unlock(&cf->sessinfo.lock);
        }

        tp[2] = getdtime();

        sender = rtpp_anetio_pick_sender(proc_cf->op);
        if (rtp_only == 0) {
            pthread_mutex_lock(&cf->glock);
            process_rtp(cf, tp[2], alarm_tick, ndrain, sender, rstats);
        } else {
            process_rtp_only(cf, tp[2], ndrain, sender, rstats);
            pthread_mutex_lock(&cf->glock);
        }

        if (cf->rtp_nsessions > 0) {
            process_rtp_servers(cf, tp[2], sender, rstats);
        }
        pthread_mutex_unlock(&cf->glock);
        rtpp_anetio_pump_q(sender);
        rtpp_command_async_wakeup(cf->stable->rtpp_cmd_cf);
        tp[3] = getdtime();
        flush_rstats(stats_cf, rstats);

#if RTPP_DEBUG
        recfilter_apply(&proc_cf->sleep_time, tp[1] - tp[0]);
        recfilter_apply(&proc_cf->poll_time, tp[2] - tp[1]);
        recfilter_apply(&proc_cf->proc_time, tp[3] - tp[2]);
#endif
        tp[0] = tp[3];
#if RTPP_DEBUG
        if (last_ctick % (unsigned int)cf->stable->target_pfreq == 0 || last_ctick < 1000) {
#if 0
            rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "run %lld eptime %f, CSV: %f,%f,%f", \
              last_ctick, tp[3], (double)last_ctick / cf->stable->target_pfreq, tp[3] - tp[1], tp[3]);
#endif
            rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "run %lld eptime %f sleep_time %f poll_time %f proc_time %f CSV: %f,%f,%f,%f", \
              last_ctick, tp[3], proc_cf->sleep_time.lastval, proc_cf->poll_time.lastval, proc_cf->proc_time.lastval, \
              (double)last_ctick / cf->stable->target_pfreq, proc_cf->sleep_time.lastval, proc_cf->poll_time.lastval, proc_cf->proc_time.lastval);
        }
#endif
    }

}

void
rtpp_proc_async_wakeup(struct rtpp_proc_async_cf *proc_cf, int clock, long long ncycles_ref)
{
    struct sign_arg s_a;
    struct rtpp_wi *wi;

    s_a.clock_tick = clock;
    s_a.ncycles_ref = ncycles_ref;
    wi = rtpp_wi_malloc_sgnl(SIGALRM, &s_a, sizeof(s_a));
    if (wi == NULL) {
        /* XXX complain */
        return;
    }
    rtpp_queue_put_item(wi, proc_cf->time_q);
}

int
rtpp_proc_async_init(struct cfg *cf)
{
    struct rtpp_proc_async_cf *proc_cf;

    proc_cf = malloc(sizeof(*proc_cf));
    if (proc_cf == NULL)
        return (-1);

    memset(proc_cf, '\0', sizeof(*proc_cf));

    init_rstats(cf->stable->rtpp_stats, &proc_cf->rstats);

#if RTPP_DEBUG
    recfilter_init(&proc_cf->sleep_time, 0.999, 0.0, 0);
    recfilter_init(&proc_cf->poll_time, 0.999, 0.0, 0);
    recfilter_init(&proc_cf->proc_time, 0.999, 0.0, 0);
#endif

    proc_cf->time_q = rtpp_queue_init(1, "RTP_PROC(time)");
    if (proc_cf->time_q == NULL) {
        free(proc_cf);
        return (-1);
    }

    proc_cf->op = rtpp_netio_async_init(cf, 10);
    if (proc_cf->op == NULL) {
        rtpp_queue_destroy(proc_cf->time_q);
        free(proc_cf);
        return (-1);
    }

    cf->stable->rtpp_proc_cf = proc_cf;
    if (pthread_create(&proc_cf->thread_id, NULL, (void *(*)(void *))&rtpp_proc_async_run, cf) != 0) {
        rtpp_queue_destroy(proc_cf->time_q);
        rtpp_netio_async_destroy(proc_cf->op);
        free(proc_cf);
        cf->stable->rtpp_proc_cf = NULL;
        return (-1);
    }

    return (0);
}
