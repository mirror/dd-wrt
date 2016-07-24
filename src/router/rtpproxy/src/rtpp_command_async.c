/*
 * Copyright (c) 2010 Sippy Software, Inc., http://www.sippysoft.com
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
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>

#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_command.h"
#include "rtpp_command_async.h"
#include "rtpp_command_private.h"
#include "rtpp_command_stream.h"
#include "rtpp_math.h"
#include "rtpp_network.h"
#include "rtpp_netio_async.h"
#include "rtpp_util.h"
#include "rtpp_types.h"
#include "rtpp_stats.h"
#include "rtpp_list.h"
#include "rtpp_controlfd.h"

#define RTPC_MAX_CONNECTIONS 100

struct rtpp_cmd_pollset {
    struct pollfd *pfds;
    int pfds_used;
    struct rtpp_cmd_connection *rccs[RTPC_MAX_CONNECTIONS];
    pthread_mutex_t pfds_mutex;
};

struct rtpp_cmd_accptset {
    struct pollfd *pfds;
    struct rtpp_ctrl_sock **csocks;
    int pfds_used;
};

struct rtpp_cmd_async_cf {
    pthread_t thread_id;
    pthread_t acpt_thread_id;
    pthread_cond_t cmd_cond;
    pthread_mutex_t cmd_mutex;
    int clock_tick;
    double tused;
    int tstate;
#if 0
    struct recfilter average_load;
#endif
    struct rtpp_command_stats cstats;
    struct rtpp_cmd_pollset pset;
    struct rtpp_cmd_accptset aset;
};

#define TSTATE_RUN   0x0
#define TSTATE_CEASE 0x1

static void
init_cstats(struct rtpp_stats_obj *sobj, struct rtpp_command_stats *csp)
{

    csp->ncmds_rcvd.cnt_idx = CALL_METHOD(sobj, getidxbyname, "ncmds_rcvd");
    csp->ncmds_succd.cnt_idx = CALL_METHOD(sobj, getidxbyname, "ncmds_succd");
    csp->ncmds_errs.cnt_idx = CALL_METHOD(sobj, getidxbyname, "ncmds_errs");
    csp->ncmds_repld.cnt_idx = CALL_METHOD(sobj, getidxbyname, "ncmds_repld");

    csp->nsess_complete.cnt_idx = CALL_METHOD(sobj, getidxbyname, "nsess_complete");
    csp->nsess_created.cnt_idx = CALL_METHOD(sobj, getidxbyname, "nsess_created");

    csp->nplrs_created.cnt_idx = CALL_METHOD(sobj, getidxbyname, "nplrs_created");
    csp->nplrs_destroyed.cnt_idx = CALL_METHOD(sobj, getidxbyname, "nplrs_destroyed");
}

#define FLUSH_CSTAT(sobj, st)    { \
    if ((st).cnt > 0) { \
        CALL_METHOD(sobj, updatebyidx, (st).cnt_idx, (st).cnt); \
        (st).cnt = 0; \
    } \
}

static void
flush_cstats(struct rtpp_stats_obj *sobj, struct rtpp_command_stats *csp)
{

    FLUSH_CSTAT(sobj, csp->ncmds_rcvd);
    FLUSH_CSTAT(sobj, csp->ncmds_succd);
    FLUSH_CSTAT(sobj, csp->ncmds_errs);
    FLUSH_CSTAT(sobj, csp->ncmds_repld);

    FLUSH_CSTAT(sobj, csp->nsess_complete);
    FLUSH_CSTAT(sobj, csp->nsess_created);

    FLUSH_CSTAT(sobj, csp->nplrs_created);
    FLUSH_CSTAT(sobj, csp->nplrs_destroyed);
}

static int
accept_connection(struct cfg *cf, int controlfd_in)
{
    int controlfd;
    socklen_t rlen;
    struct sockaddr_un ifsun;

    rlen = sizeof(ifsun);
    controlfd = accept(controlfd_in, sstosa(&ifsun), &rlen);
    if (controlfd == -1) {
        if (errno != EWOULDBLOCK) {
            rtpp_log_ewrite(RTPP_LOG_ERR, cf->stable->glog,
              "can't accept connection on control socket");
        }
        return (-1);
    }
    return (controlfd);
}

static int
process_commands(struct cfg *cf, int controlfd, double dtime,
  struct rtpp_command_stats *csp, int umode, struct rtpp_stats_obj *rsc)
{
    int i, rval;
    struct rtpp_command *cmd;

    i = 0;
    do {
        cmd = get_command(cf, controlfd, &rval, dtime, csp, umode);
        if (cmd == NULL && rval == 0) {
            /*
             * get_command() failed with error other than I/O error
             * or something, there might be some good commands in
             * the queue.
             */
            continue;
        }
        if (cmd != NULL) {
            if (cmd->cca.op == GET_STATS) {
                flush_cstats(rsc, csp);
            }
            if (cmd->no_glock == 0) {
                pthread_mutex_lock(&cf->glock);
            }
            i = handle_command(cf, cmd);
            if (cmd->no_glock == 0) {
                pthread_mutex_unlock(&cf->glock);
            }
            free_command(cmd);
        } else {
            i = -1;
        }
    } while (i == 0 && umode != 0);
    return (i);
}

static int
process_commands_stream(struct cfg *cf, struct rtpp_cmd_connection *rcc,
  double dtime, struct rtpp_command_stats *csp, struct rtpp_stats_obj *rsc)
{
    int rval;
    struct rtpp_command *cmd;

    rval = rtpp_command_stream_doio(cf, rcc);
    if (rval <= 0) {
        return (-1);
    }
    do {
        cmd = rtpp_command_stream_get(cf, rcc, &rval, dtime, csp);
        if (cmd == NULL) {
            if (rval != 0) {
                break;
            }
            continue;
        }
        if (cmd->cca.op == GET_STATS) {
            flush_cstats(rsc, csp);
        }
        if (cmd->no_glock == 0) {
            pthread_mutex_lock(&cf->glock);
        }
        rval = handle_command(cf, cmd);
        if (cmd->no_glock == 0) {
            pthread_mutex_unlock(&cf->glock);
        }
        free_command(cmd);
    } while (rval == 0);
    return (rval);
}

static struct rtpp_cmd_connection *
rtpp_cmd_connection_ctor(int controlfd_in, int controlfd_out, struct rtpp_ctrl_sock *csock)
{
    struct rtpp_cmd_connection *rcc;

    rcc = malloc(sizeof(struct rtpp_cmd_connection));
    if (rcc == NULL) {
        return (NULL);
    }
    memset(rcc, '\0', sizeof(struct rtpp_cmd_connection));
    rcc->controlfd_in = controlfd_in;
    rcc->controlfd_out = controlfd_out;
    rcc->csock = csock;
    return (rcc);
}

void
rtpp_cmd_connection_dtor(struct rtpp_cmd_connection *rcc)
{

    if (rcc->controlfd_in != rcc->csock->controlfd_in) {
        close(rcc->controlfd_in);
        if (rcc->controlfd_out != rcc->controlfd_in) {
            close(rcc->controlfd_out);
        }
    }
    free(rcc);
}

static void
rtpp_cmd_acceptor_run(void *arg)
{
    struct cfg *cf;
    struct rtpp_cmd_async_cf *cmd_cf;
    struct pollfd *tp;
    struct rtpp_cmd_pollset *psp;
    struct rtpp_cmd_accptset *asp;
    struct rtpp_cmd_connection *rcc;
    int nready, controlfd, i;

    cf = (struct cfg *)arg;
    cmd_cf = cf->stable->rtpp_cmd_cf;
    psp = &cmd_cf->pset;
    asp = &cmd_cf->aset;

    for (;;) {
        nready = poll(asp->pfds, asp->pfds_used, INFTIM);
        if (nready <= 0)
            continue;
        for (i = 0; i < asp->pfds_used; i++) {
            if ((asp->pfds[i].revents & POLLIN) == 0) {
                continue;
            }
            pthread_mutex_lock(&psp->pfds_mutex);
            if (psp->pfds_used >= RTPC_MAX_CONNECTIONS) {
                pthread_mutex_unlock(&psp->pfds_mutex);
                continue;
            }
            controlfd = accept_connection(cf, asp->pfds[i].fd);
            if (controlfd < 0) {
                pthread_mutex_unlock(&psp->pfds_mutex);
                continue;
            }
            tp = realloc(psp->pfds, sizeof(struct pollfd) * (psp->pfds_used + 1));
            if (tp == NULL) {
                pthread_mutex_unlock(&psp->pfds_mutex);
                close(controlfd); /* Yeah, sorry, please try later */
                continue;
            }
            rcc = rtpp_cmd_connection_ctor(controlfd, controlfd, asp->csocks[i]);
            if (rcc == NULL) {
                pthread_mutex_unlock(&psp->pfds_mutex);
                close(controlfd); /* Yeah, sorry, please try later */
                continue;
            }
            psp->pfds = tp;
            psp->pfds[psp->pfds_used].fd = controlfd;
            psp->pfds[psp->pfds_used].events = POLLIN | POLLERR | POLLHUP;
            psp->pfds[psp->pfds_used].revents = 0;
            psp->rccs[psp->pfds_used] = rcc;
            psp->pfds_used++;
            pthread_mutex_unlock(&psp->pfds_mutex);
            rtpp_command_async_wakeup(cmd_cf);
        }
    }
}

static int
wait_next_clock(struct rtpp_cmd_async_cf *cmd_cf)
{
    static int last_ctick = -1;
    int tstate;

    pthread_mutex_lock(&cmd_cf->cmd_mutex);
    if (last_ctick == -1) {
        last_ctick = cmd_cf->clock_tick;
    }
    tstate = cmd_cf->tstate;
    while (cmd_cf->clock_tick == last_ctick && tstate == TSTATE_RUN) {
        pthread_cond_wait(&cmd_cf->cmd_cond, &cmd_cf->cmd_mutex);
    }
    last_ctick = cmd_cf->clock_tick;
    pthread_mutex_unlock(&cmd_cf->cmd_mutex);
    return (tstate);
}

static void
rtpp_cmd_queue_run(void *arg)
{
    struct cfg *cf;
    struct rtpp_cmd_async_cf *cmd_cf;
    struct rtpp_cmd_pollset *psp;
    int i, nready, rval, umode;
    double sptime;
#if 0
    double eptime, tused;
#endif
    struct rtpp_command_stats *csp;
    struct rtpp_stats_obj *rtpp_stats_cf;

    cf = (struct cfg *)arg;
    cmd_cf = cf->stable->rtpp_cmd_cf;
    rtpp_stats_cf = cf->stable->rtpp_stats;
    csp = &cmd_cf->cstats;

    psp = &cmd_cf->pset;

    for (;;) {
        sptime = getdtime();

        pthread_mutex_lock(&psp->pfds_mutex);
        if (psp->pfds_used == 0) {
            pthread_mutex_unlock(&psp->pfds_mutex);
            if (wait_next_clock(cmd_cf) == TSTATE_CEASE) {
                break;
            }
            continue;
        }
        nready = poll(psp->pfds, psp->pfds_used, 2);
        if (nready == 0) {
            pthread_mutex_unlock(&psp->pfds_mutex);
            if (wait_next_clock(cmd_cf) == TSTATE_CEASE) {
                break;
            }
            continue;
        }
        if (nready < 0 && errno == EINTR) {
            pthread_mutex_unlock(&psp->pfds_mutex);
            continue;
        }
        if (nready > 0) {
            for (i = 0; i < psp->pfds_used; i++) {
                if ((psp->pfds[i].revents & (POLLERR | POLLHUP)) != 0) {
                    if (RTPP_CTRL_ISUNIX(psp->rccs[i]->csock)) {
                        goto closefd;
                    }
                    if (psp->rccs[i]->csock->type == RTPC_STDIO && (psp->pfds[i].revents & POLLIN) == 0) {
                        goto closefd;
                    }
                }
                if ((psp->pfds[i].revents & POLLIN) == 0) {
                    continue;
                }
                if (RTPP_CTRL_ISSTREAM(psp->rccs[i]->csock)) {
                    rval = process_commands_stream(cf, psp->rccs[i], sptime, csp, rtpp_stats_cf);
                } else {
                    umode = RTPP_CTRL_ISDG(psp->rccs[i]->csock);
                    rval = process_commands(cf, psp->pfds[i].fd, sptime, csp, umode, rtpp_stats_cf);
                }
                /*
                 * Shut down non-datagram sockets that got I/O error
                 * and also all non-continuous UNIX sockets are recycled
                 * after each use.
                 */
                if (!RTPP_CTRL_ISDG(psp->rccs[i]->csock) && (rval == -1 || RTPP_CTRL_ISUNIX(psp->rccs[i]->csock))) {
closefd:
                    if (psp->rccs[i]->csock->type == RTPC_STDIO && psp->rccs[i]->csock->exit_on_close != 0) {
                        cf->stable->slowshutdown = 1;
                    }
                    rtpp_cmd_connection_dtor(psp->rccs[i]);
                    psp->pfds_used--;
                    if (psp->pfds_used > 0 && i < psp->pfds_used) {
                        memcpy(&psp->pfds[i], &psp->pfds[i + 1],
                          (psp->pfds_used - i) * sizeof(struct pollfd));
                        memcpy(&psp->rccs[i], &psp->rccs[i + 1],
                          (psp->pfds_used - i) * sizeof(struct rtpp_ctrl_connection *));
                    }
                }
            }
        }
        pthread_mutex_unlock(&psp->pfds_mutex);
        if (nready > 0) {
            rtpp_anetio_pump(cf->stable->rtpp_netio_cf);
        }
#if 0
        eptime = getdtime();
        pthread_mutex_lock(&cmd_cf->cmd_mutex);
        recfilter_apply(&cmd_cf->average_load, (eptime - sptime + tused) * cf->stable->target_pfreq);
        pthread_mutex_unlock(&cmd_cf->cmd_mutex);
#endif
        flush_cstats(rtpp_stats_cf, csp);
#if 0
#if RTPP_DEBUG
        if (last_ctick % (unsigned int)cf->stable->target_pfreq == 0 || last_ctick < 1000) {
            rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "rtpp_cmd_queue_run %lld sptime %f eptime %f, CSV: %f,%f,%f,%f,%f", \
              last_ctick, sptime, eptime, (double)last_ctick / cf->stable->target_pfreq, \
              eptime - sptime + tused, eptime, sptime, tused);
            rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "run %lld average load %f, CSV: %f,%f", last_ctick, \
              cmd_cf->average_load.lastval * 100.0, (double)last_ctick / cf->stable->target_pfreq, cmd_cf->average_load.lastval);
        }
#endif
#endif
    }
}

double
rtpp_command_async_get_aload(struct rtpp_cmd_async_cf *cmd_cf)
{
#if 0
    double aload;

    pthread_mutex_lock(&cmd_cf->cmd_mutex);
    aload = cmd_cf->average_load.lastval;
    pthread_mutex_unlock(&cmd_cf->cmd_mutex);

    return (aload);
#else
    return (0);
#endif
}

int
rtpp_command_async_wakeup(struct rtpp_cmd_async_cf *cmd_cf)
{
    int old_clock;

    pthread_mutex_lock(&cmd_cf->cmd_mutex);

    old_clock = cmd_cf->clock_tick;
    cmd_cf->clock_tick++;

    /* notify worker thread */
    pthread_cond_signal(&cmd_cf->cmd_cond);

    pthread_mutex_unlock(&cmd_cf->cmd_mutex);

    return (old_clock);
}

static int
init_pollset(struct cfg *cf, struct rtpp_cmd_pollset *psp)
{
    struct rtpp_ctrl_sock *ctrl_sock;
    int pfds_used, msize, i;

    pfds_used = 0;
    ctrl_sock = RTPP_LIST_HEAD(cf->stable->ctrl_socks);
    for (pfds_used = 0; ctrl_sock != NULL; ctrl_sock = RTPP_ITER_NEXT(ctrl_sock)) {
        if (RTPP_CTRL_ISUNIX(ctrl_sock))
            continue;
        pfds_used++;
    }
    msize = pfds_used > 0 ? pfds_used : 1;
    psp->pfds = malloc(sizeof(struct pollfd) * msize);
    if (psp->pfds == NULL) {
        return (-1);
    }
    if (pthread_mutex_init(&psp->pfds_mutex, NULL) != 0) {
        free(psp->pfds);
        return (-1);
    }
    psp->pfds_used = pfds_used;
    if (psp->pfds_used == 0) {
        return (0);
    }
    ctrl_sock = RTPP_LIST_HEAD(cf->stable->ctrl_socks);
    for (i = 0; ctrl_sock != NULL; ctrl_sock = RTPP_ITER_NEXT(ctrl_sock)) {
        if (RTPP_CTRL_ISUNIX(ctrl_sock))
            continue;
        psp->pfds[i].fd = ctrl_sock->controlfd_in;
        psp->pfds[i].events = POLLIN;
        psp->pfds[i].revents = 0;
        psp->rccs[i] = rtpp_cmd_connection_ctor(ctrl_sock->controlfd_in, 
          ctrl_sock->controlfd_out, ctrl_sock);
        i++;
    }
    if (i == 1 && RTPP_CTRL_ISSTREAM(psp->rccs[0]->csock)) {
        psp->rccs[0]->csock->exit_on_close = 1;
    }
    return (0);
}

static void
free_pollset(struct rtpp_cmd_pollset *psp)
{
    int i;

    for (i = 0; i < psp->pfds_used; i ++) {
        rtpp_cmd_connection_dtor(psp->rccs[i]);
    }
    free(psp->pfds);
}

static int
init_accptset(struct cfg *cf, struct rtpp_cmd_accptset *asp)
{
    int i, pfds_used;
    struct rtpp_ctrl_sock *ctrl_sock;

    pfds_used = 0;
    ctrl_sock = RTPP_LIST_HEAD(cf->stable->ctrl_socks);
    for (pfds_used = 0; ctrl_sock != NULL; ctrl_sock = RTPP_ITER_NEXT(ctrl_sock)) {
        if (RTPP_CTRL_ISUNIX(ctrl_sock) == 0)
            continue;
        pfds_used++;
    }
    if (pfds_used == 0) {
        return (0);
    }

    asp->pfds = malloc(sizeof(struct pollfd) * pfds_used);
    if (asp->pfds == NULL) {
        return (-1);
    }
    asp->pfds_used = pfds_used;
    asp->csocks = malloc(sizeof(struct rtpp_ctrl_sock) * pfds_used);
    if (asp->csocks == NULL) {
        free(asp->pfds);
        return (-1);
    }
    ctrl_sock = RTPP_LIST_HEAD(cf->stable->ctrl_socks);
    for (i = 0; i < asp->pfds_used; ctrl_sock = RTPP_ITER_NEXT(ctrl_sock)) {
        if (RTPP_CTRL_ISUNIX(ctrl_sock) == 0)
            continue;
        asp->pfds[i].fd = ctrl_sock->controlfd_in;
        asp->pfds[i].events = POLLIN;
        asp->pfds[i].revents = 0;
        asp->csocks[i] = ctrl_sock;
        i++;
    }
    return (pfds_used);
}

static void
free_accptset(struct rtpp_cmd_accptset *asp)
{
    if (asp->pfds_used > 0) {
        free(asp->csocks);
        free(asp->pfds);
    }
}

int
rtpp_command_async_init(struct cfg *cf)
{
    struct rtpp_cmd_async_cf *cmd_cf;
    int need_acptr;

    cmd_cf = malloc(sizeof(*cmd_cf));
    if (cmd_cf == NULL)
        return (-1);

    memset(cmd_cf, '\0', sizeof(*cmd_cf));

    if (init_pollset(cf, &cmd_cf->pset) == -1) {
        free(cmd_cf);
        return (-1);
    }
    need_acptr = init_accptset(cf, &cmd_cf->aset);
    if (need_acptr == -1) {
        free_pollset(&cmd_cf->pset);
        free(cmd_cf);
        return (-1);
    }

    init_cstats(cf->stable->rtpp_stats, &cmd_cf->cstats);

    pthread_cond_init(&cmd_cf->cmd_cond, NULL);
    pthread_mutex_init(&cmd_cf->cmd_mutex, NULL);

#if 0
    recfilter_init(&cmd_cf->average_load, 0.999, 0.0, 1);
#endif

    cf->stable->rtpp_cmd_cf = cmd_cf;
    if (need_acptr != 0) {
        pthread_create(&cmd_cf->acpt_thread_id, NULL,
          (void *(*)(void *))&rtpp_cmd_acceptor_run, cf);
    }
    if (pthread_create(&cmd_cf->thread_id, NULL,
      (void *(*)(void *))&rtpp_cmd_queue_run, cf) != 0) {
        pthread_cond_destroy(&cmd_cf->cmd_cond);
        pthread_mutex_destroy(&cmd_cf->cmd_mutex);
        free_pollset(&cmd_cf->pset);
        free_accptset(&cmd_cf->aset);
        free(cmd_cf);
        cf->stable->rtpp_cmd_cf = NULL;
        return (-1);
    }

    return (0);
}

#if 0
/* Incomplete, needs more work, utility unknown at the moment */
void
rtpp_command_async_dtor(struct cfg *cf)
{
    struct rtpp_cmd_async_cf *cmd_cf;

    cmd_cf = cf->stable->rtpp_cmd_cf;

    pthread_mutex_lock(&cmd_cf->cmd_mutex);
    cmd_cf->tstate = TSTATE_CEASE;
    /* notify worker thread */
    pthread_cond_signal(&cmd_cf->cmd_cond);
    pthread_mutex_unlock(&cmd_cf->cmd_mutex);
    pthread_join(cmd_cf->thread_id, NULL);

    pthread_cond_destroy(&cmd_cf->cmd_cond);
    pthread_mutex_destroy(&cmd_cf->cmd_mutex);
    free_pollset(&cmd_cf->pset);
    free_accptset(&cmd_cf->aset);
    free(cmd_cf);
    cf->stable->rtpp_cmd_cf = NULL;
}
#endif
