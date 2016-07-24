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
#include <ctype.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtpp_types.h"
#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_hash_table.h"
#include "rtpp_command.h"
#include "rtpp_command_copy.h"
#include "rtpp_command_private.h"
#include "rtpp_command_ul.h"
#include "rtpp_session.h"
#include "rtp.h"
#include "rtp_resizer.h"
#include "rtpp_network.h"
#include "rtpp_notify.h"
#include "rtpp_util.h"

#define FREE_IF_NULL(p)	{if ((p) != NULL) {free(p); (p) = NULL;}}

struct ul_reply {
    struct sockaddr *ia;
    const char *ia_ov;
    int port;
};

struct ul_opts {
    int asymmetric;
    int weak;
    int requested_nsamples;
    char *codecs;
    char *addr;
    char *port;
    struct sockaddr *ia[2];
    struct sockaddr *lia[2];

    struct ul_reply reply;
    
    int lidx;
    struct sockaddr *local_addr;
    char *socket_name_u;
    char *notify_tag;
    int pf;
};

void
ul_reply_port(struct cfg *cf, struct rtpp_command *cmd, struct ul_reply *ulr)
{
    int len, rport;

    if (ulr == NULL || ulr->ia == NULL || ishostnull(ulr->ia)) {
        rport = (ulr == NULL) ? 0 : ulr->port;
        len = snprintf(cmd->buf_t, sizeof(cmd->buf_t), "%d\n", rport);
    } else {
        if (ulr->ia_ov == NULL) {
            len = snprintf(cmd->buf_t, sizeof(cmd->buf_t), "%d %s%s\n", ulr->port,
              addr2char(ulr->ia), (ulr->ia->sa_family == AF_INET) ? "" : " 6");
        } else {
            len = snprintf(cmd->buf_t, sizeof(cmd->buf_t), "%d %s%s\n", ulr->port,
              ulr->ia_ov, (ulr->ia->sa_family == AF_INET) ? "" : " 6");
        }
    }

    rtpc_doreply(cf, cmd->buf_t, len, cmd, (ulr != NULL) ? 0 : 1);
}

static void
ul_opts_init(struct cfg *cf, struct ul_opts *ulop)
{

    /* In bridge mode all clients are assumed to be asymmetric */
    ulop->asymmetric = (cf->stable->bmode != 0) ? 1 : 0;
    ulop->requested_nsamples = -1;
    ulop->lia[0] = ulop->lia[1] = ulop->reply.ia = cf->stable->bindaddr[0];
    ulop->lidx = 1;
    ulop->pf = AF_INET;
}

void
rtpp_command_ul_opts_free(struct ul_opts *ulop)
{

    FREE_IF_NULL(ulop->codecs);
    FREE_IF_NULL(ulop->ia[0]);
    FREE_IF_NULL(ulop->ia[1]);
    free(ulop);
}

struct ul_opts *
rtpp_command_ul_opts_parse(struct cfg *cf, struct rtpp_command *cmd)
{
    int len, tpf, n, i;
    char c;
    char *cp, *t;
    const char *errmsg;
    struct sockaddr_storage tia;
    struct ul_opts *ulop;

    ulop = malloc(sizeof(struct ul_opts));
    if (ulop == NULL) {
        reply_error(cf, cmd, ECODE_NOMEM_1);
        goto err_undo_0;
    }
    memset(ulop, '\0', sizeof(struct ul_opts));
    ul_opts_init(cf, ulop);
    if (cmd->cca.op == UPDATE && cmd->argc > 6) {
        if (cmd->argc == 8) {
            ulop->socket_name_u = cmd->argv[6];
            ulop->notify_tag = cmd->argv[7];
        } else {
            ulop->socket_name_u = cmd->argv[5];
            ulop->notify_tag = cmd->argv[6];
            cmd->cca.to_tag = NULL;
        }
        if (strncmp("unix:", ulop->socket_name_u, 5) == 0)
            ulop->socket_name_u += 5;
        len = url_unquote((uint8_t *)ulop->notify_tag, strlen(ulop->notify_tag));
        if (len == -1) {
            rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog,
              "command syntax error - invalid URL encoding");
            reply_error(cf, cmd, ECODE_PARSE_10);
            goto err_undo_1;
        }
        ulop->notify_tag[len] = '\0';
    }
    ulop->addr = cmd->argv[2];
    ulop->port = cmd->argv[3];
    /* Process additional command modifiers */
    for (cp = cmd->argv[0] + 1; *cp != '\0'; cp++) {
        switch (*cp) {
        case 'a':
        case 'A':
            ulop->asymmetric = 1;
            break;

        case 'e':
        case 'E':
            if (ulop->lidx < 0) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
                reply_error(cf, cmd, ECODE_PARSE_11);
                goto err_undo_1;
            }
            ulop->lia[ulop->lidx] = cf->stable->bindaddr[1];
            ulop->lidx--;
            break;

        case 'i':
        case 'I':
            if (ulop->lidx < 0) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
                reply_error(cf, cmd, ECODE_PARSE_12);
                goto err_undo_1;
            }
            ulop->lia[ulop->lidx] = cf->stable->bindaddr[0];
            ulop->lidx--;
            break;

        case '6':
            ulop->pf = AF_INET6;
            break;

        case 's':
        case 'S':
            ulop->asymmetric = 0;
            break;

        case 'w':
        case 'W':
            ulop->weak = 1;
            break;

        case 'z':
        case 'Z':
            ulop->requested_nsamples = (strtol(cp + 1, &cp, 10) / 10) * 80;
            if (ulop->requested_nsamples <= 0) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
                reply_error(cf, cmd, ECODE_PARSE_13);
                goto err_undo_1;
            }
            cp--;
            break;

        case 'c':
        case 'C':
            cp += 1;
            for (t = cp; *cp != '\0'; cp++) {
                if (!isdigit(*cp) && *cp != ',')
                    break;
            }
            if (t == cp) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
                reply_error(cf, cmd, ECODE_PARSE_14);
                goto err_undo_1;
            }
            ulop->codecs = malloc(cp - t + 1);
            if (ulop->codecs == NULL) {
                reply_error(cf, cmd, ECODE_NOMEM_2);
                goto err_undo_1;
            }
            memcpy(ulop->codecs, t, cp - t);
            ulop->codecs[cp - t] = '\0';
            cp--;
            break;

        case 'l':
        case 'L':
            len = extractaddr(cp + 1, &t, &cp, &tpf);
            if (len == -1) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
                reply_error(cf, cmd, ECODE_PARSE_15);
                goto err_undo_1;
            }
            c = t[len];
            t[len] = '\0';
            pthread_mutex_unlock(&cf->glock);
            ulop->local_addr = host2bindaddr(cf, t, tpf, &errmsg);
            pthread_mutex_lock(&cf->glock);
            if (ulop->local_addr == NULL) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog,
                  "invalid local address: %s: %s", t, errmsg);
                reply_error(cf, cmd, ECODE_INVLARG_1);
                goto err_undo_1;
            }
            t[len] = c;
            cp--;
            break;

        case 'r':
        case 'R':
            len = extractaddr(cp + 1, &t, &cp, &tpf);
            if (len == -1) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
                reply_error(cf, cmd, ECODE_PARSE_16);
                goto err_undo_1;
            }
            c = t[len];
            t[len] = '\0';
            ulop->local_addr = alloca(sizeof(struct sockaddr_storage));
            pthread_mutex_unlock(&cf->glock);
            n = resolve(ulop->local_addr, tpf, t, SERVICE, AI_PASSIVE);
            pthread_mutex_lock(&cf->glock);
            if (n != 0) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog,
                  "invalid remote address: %s: %s", t, gai_strerror(n));
                reply_error(cf, cmd, ECODE_INVLARG_2);
                goto err_undo_1;
            }
            if (local4remote(ulop->local_addr, satoss(ulop->local_addr)) == -1) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog,
                  "can't find local address for remote address: %s", t);
                reply_error(cf, cmd, ECODE_INVLARG_3);
                goto err_undo_1;
            }
            ulop->local_addr = addr2bindaddr(cf, ulop->local_addr, &errmsg);
            if (ulop->local_addr == NULL) {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog,
                  "invalid local address: %s", errmsg);
                reply_error(cf, cmd, ECODE_INVLARG_4);
                goto err_undo_1;
            }
            t[len] = c;
            cp--;
            break;

        default:
            rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "unknown command modifier `%c'",
              *cp);
            break;
        }
    }
    if (ulop->addr != NULL && ulop->port != NULL && strlen(ulop->addr) >= 7) {
        pthread_mutex_unlock(&cf->glock);
        n = resolve(sstosa(&tia), ulop->pf, ulop->addr, ulop->port, AI_NUMERICHOST);
        pthread_mutex_lock(&cf->glock);
        if (n == 0) {
            if (!ishostnull(sstosa(&tia))) {
                for (i = 0; i < 2; i++) {
                    ulop->ia[i] = malloc(SS_LEN(&tia));
                    if (ulop->ia[i] == NULL) {
                        reply_error(cf, cmd, ECODE_NOMEM_3);
                        goto err_undo_1;
                    }
                    memcpy(ulop->ia[i], &tia, SS_LEN(&tia));
                }
                /* Set port for RTCP, will work both for IPv4 and IPv6 */
                n = ntohs(satosin(ulop->ia[1])->sin_port);
                satosin(ulop->ia[1])->sin_port = htons(n + 1);
            }
        } else {
            rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "getaddrinfo(pf=%d, addr=%s, port=%s): %s",
              ulop->pf, ulop->addr, ulop->port, gai_strerror(n));
        }
    }
    return (ulop);

err_undo_1:
    rtpp_command_ul_opts_free(ulop);
err_undo_0:
    return (NULL);
}

static void
handle_nomem(struct cfg *cf, struct rtpp_command *cmd,
  int ecode, struct ul_opts *ulop, int *fds,
  struct rtpp_session *spa, struct rtpp_session *spb)
{
    int i;

    rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "can't allocate memory");
    rtpp_command_ul_opts_free(ulop);
    if (spa != NULL) {
        if (spa->call_id != NULL)
            free(spa->call_id);
        if (spa->tag != NULL)
            free(spa->tag);
        if (spa->tag_nomedianum != NULL)
            free(spa->tag_nomedianum);
        free(spa);
    }
    if (spb != NULL)
        free(spb);
    for (i = 0; i < 2; i++)
        if (fds[i] != -1)
            close(fds[i]);
    reply_error(cf, cmd, ecode);
}

int
rtpp_command_ul_handle(struct cfg *cf, struct rtpp_command *cmd,
  struct ul_opts *ulop, struct rtpp_session *sp, int sidx)
{
    int pidx, lport, i;
    int fds[2];
    char *cp;
    struct rtpp_session *spa, *spb;

    pidx = 1;
    lport = 0;
    spa = spb = NULL;
    fds[0] = fds[1] = -1;
    if (sidx != -1) {
        assert(cmd->cca.op == UPDATE || cmd->cca.op == LOOKUP);
        spa = sp;
        if (spa->fds[sidx] == -1) {
            if (ulop->local_addr != NULL) {
                spa->laddr[sidx] = ulop->local_addr;
            }
            if (rtpp_create_listener(cf, spa->laddr[sidx], &lport, fds) == -1) {
                rtpp_log_write(RTPP_LOG_ERR, spa->log, "can't create listener");
                reply_error(cf, cmd, ECODE_LSTFAIL_1);
                goto err_undo_0;
            }
            assert(spa->fds[sidx] == -1);
            spa->fds[sidx] = fds[0];
            assert(spa->rtcp->fds[sidx] == -1);
            spa->rtcp->fds[sidx] = fds[1];
            spa->ports[sidx] = lport;
            spa->rtcp->ports[sidx] = lport + 1;
            if (spa->complete == 0) {
                cmd->csp->nsess_complete.cnt++;
            }
            spa->complete = spa->rtcp->complete = 1;
            append_session(cf, spa, sidx);
        }
        if (ulop->weak)
            spa->weak[sidx] = 1;
        else if (cmd->cca.op == UPDATE)
            spa->strong = 1;
        lport = spa->ports[sidx];
        ulop->lia[0] = spa->laddr[sidx];
        pidx = (sidx == 0) ? 1 : 0;
        spa->ttl_mode = cf->stable->ttl_mode;
        if (cmd->cca.op == UPDATE) {
            spa->ttl[0] = cf->stable->max_setup_ttl;
            spa->ttl[1] = cf->stable->max_setup_ttl;
            rtpp_log_write(RTPP_LOG_INFO, spa->log,
              "adding %s flag to existing session, new=%d/%d/%d",
              ulop->weak ? ( sidx ? "weak[1]" : "weak[0]" ) : "strong",
              spa->strong, spa->weak[0], spa->weak[1]);
        } else {
            spa->ttl[0] = cf->stable->max_ttl;
            spa->ttl[1] = cf->stable->max_ttl;
        }
        rtpp_log_write(RTPP_LOG_INFO, spa->log,
          "lookup on ports %d/%d, session timer restarted", spa->ports[0],
          spa->ports[1]);
    } else {
        assert(cmd->cca.op == UPDATE);
        rtpp_log_write(RTPP_LOG_INFO, cf->stable->glog,
          "new session %s, tag %s requested, type %s",
          cmd->cca.call_id, cmd->cca.from_tag, ulop->weak ? "weak" : "strong");
        if (cf->stable->slowshutdown != 0) {
            rtpp_log_write(RTPP_LOG_INFO, cf->stable->glog,
              "proxy is in the deorbiting-burn mode, new session rejected");
            reply_error(cf, cmd, ECODE_SLOWSHTDN);
            goto err_undo_0;
        }
        if (ulop->local_addr != NULL) {
            ulop->lia[0] = ulop->lia[1] = ulop->local_addr;
        }
        if (rtpp_create_listener(cf, ulop->lia[0], &lport, fds) == -1) {
            rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "can't create listener");
            reply_error(cf, cmd, ECODE_LSTFAIL_2);
            goto err_undo_0;
        }

        /*
         * Session creation. If creation is requested with weak flag,
         * set weak[0].
         */
        spa = malloc(sizeof(*spa));
        if (spa == NULL) {
            handle_nomem(cf, cmd, ECODE_NOMEM_4, ulop,
              fds, spa, spb);
            return (-1);
        }
        /* spb is RTCP twin session for this one. */
        spb = malloc(sizeof(*spb));
        if (spb == NULL) {
            handle_nomem(cf, cmd, ECODE_NOMEM_5, ulop,
              fds, spa, spb);
            return (-1);
        }
        memset(spa, 0, sizeof(*spa));
        memset(spb, 0, sizeof(*spb));
        spa->init_ts = cmd->dtime;
        spb->init_ts = cmd->dtime;
        for (i = 0; i < 2; i++) {
            spa->fds[i] = spb->fds[i] = -1;
            spa->last_update[i] = 0;
            spb->last_update[i] = 0;
        }
        spa->call_id = strdup(cmd->cca.call_id);
        if (spa->call_id == NULL) {
            handle_nomem(cf, cmd, ECODE_NOMEM_6, ulop,
              fds, spa, spb);
            return (-1);
        }
        spb->call_id = spa->call_id;
        spa->tag = strdup(cmd->cca.from_tag);
        spa->tag_nomedianum = strdup(cmd->cca.from_tag);
        if (spa->tag == NULL) {
            handle_nomem(cf, cmd, ECODE_NOMEM_7, ulop,
              fds, spa, spb);
            return (-1);
        }
        cp = strrchr(spa->tag_nomedianum, ';');
        if (cp != NULL)
            *cp = '\0';
        spb->tag = spa->tag;
        spb->tag_nomedianum = spa->tag_nomedianum;
        for (i = 0; i < 2; i++) {
            spa->rrcs[i] = NULL;
            spb->rrcs[i] = NULL;
            spa->laddr[i] = ulop->lia[i];
            spb->laddr[i] = ulop->lia[i];
        }
        spa->strong = spa->weak[0] = spa->weak[1] = 0;
        if (ulop->weak)
            spa->weak[0] = 1;
        else
            spa->strong = 1;
        assert(spa->fds[0] == -1);
        spa->fds[0] = fds[0];
        assert(spb->fds[0] == -1);
        spb->fds[0] = fds[1];
        spa->ports[0] = lport;
        spb->ports[0] = lport + 1;
        spa->ttl[0] = cf->stable->max_setup_ttl;
        spa->ttl[1] = cf->stable->max_setup_ttl;
        spb->ttl[0] = -1;
        spb->ttl[1] = -1;
        spa->log = rtpp_log_open(cf->stable, "rtpproxy", spa->call_id, 0);
        rtpp_log_setlevel(spa->log, cf->stable->log_level);
        spb->log = spa->log;
        spa->rtcp = spb;
        spb->rtcp = NULL;
        spa->rtp = NULL;
        spb->rtp = spa;
        spa->sridx = spb->sridx = -1;

        append_session(cf, spa, 0);
        append_session(cf, spa, 1);

        spa->hte = CALL_METHOD(cf->stable->sessions_ht, append, spa->call_id, spa);
        if (spa->hte == NULL) {
            remove_session(cf, spa);
            remove_session(cf, spb);
            handle_nomem(cf, cmd, ECODE_NOMEM_8, ulop, fds, spa, spb);
            return (-1);
        }

        cf->sessions_created++;
        cf->sessions_active++;
        cmd->csp->nsess_created.cnt++;

        /*
         * Each session can consume up to 5 open file descriptors (2 RTP,
         * 2 RTCP and 1 logging) so that warn user when he is likely to
         * exceed 80% mark on hard limit.
         */
        if (cf->sessions_active > (rtpp_rlim_max(cf) * 80 / (100 * 5)) &&
          cf->nofile_limit_warned == 0) {
            cf->nofile_limit_warned = 1;
            rtpp_log_write(RTPP_LOG_WARN, cf->stable->glog, "passed 80%% "
              "threshold on the open file descriptors limit (%d), "
              "consider increasing the limit using -L command line "
              "option", (int)rtpp_rlim_max(cf));
        }

        rtpp_log_write(RTPP_LOG_INFO, spa->log, "new session on a port %d created, "
          "tag %s", lport, cmd->cca.from_tag);
        if (cf->stable->record_all != 0) {
            handle_copy(cf, spa, 0, NULL, 0);
            handle_copy(cf, spa, 1, NULL, 0);
        }
    }

    if (cmd->cca.op == UPDATE) {
        if (rtpp_th_get_sn(cf->timeout_handler) == NULL && ulop->socket_name_u != NULL)
            rtpp_log_write(RTPP_LOG_ERR, spa->log, "must permit notification socket with -n");
        if (spa->timeout_data.notify_tag != NULL) {
            free(spa->timeout_data.notify_tag);
            spa->timeout_data.notify_tag = NULL;
        }
        if (rtpp_th_get_sn(cf->timeout_handler) != NULL && ulop->socket_name_u != NULL) {
            if (strcmp(rtpp_th_get_sn(cf->timeout_handler), ulop->socket_name_u) != 0) {
                rtpp_log_write(RTPP_LOG_ERR, spa->log, "invalid socket name %s", ulop->socket_name_u);
                ulop->socket_name_u = NULL;
            } else {
                rtpp_log_write(RTPP_LOG_INFO, spa->log, "setting timeout handler");
                spa->timeout_data.handler = cf->timeout_handler;
                spa->timeout_data.notify_tag = strdup(ulop->notify_tag);
            }
        } else if (ulop->socket_name_u == NULL && spa->timeout_data.handler != NULL) {
            spa->timeout_data.handler = NULL;
            rtpp_log_write(RTPP_LOG_INFO, spa->log, "disabling timeout handler");
        }
    }

    if (ulop->ia[0] != NULL && ulop->ia[1] != NULL) {
        if (spa->addr[pidx] != NULL)
            spa->last_update[pidx] = cmd->dtime;
        if (spa->rtcp->addr[pidx] != NULL)
            spa->rtcp->last_update[pidx] = cmd->dtime;
        /*
         * Unless the address provided by client historically
         * cannot be trusted and address is different from one
         * that we recorded update it.
         */
        if (spa->untrusted_addr[pidx] == 0 && !(spa->addr[pidx] != NULL &&
          SA_LEN(ulop->ia[0]) == SA_LEN(spa->addr[pidx]) &&
          memcmp(ulop->ia[0], spa->addr[pidx], SA_LEN(ulop->ia[0])) == 0)) {
            const char *obr, *cbr;
            if (ulop->pf == AF_INET) {
                obr = "";
                cbr = "";
            } else {
                obr = "[";
                cbr = "]";
            }
            rtpp_log_write(RTPP_LOG_INFO, spa->log, "pre-filling %s's address "
              "with %s%s%s:%s", (pidx == 0) ? "callee" : "caller", obr, ulop->addr,
              cbr, ulop->port);
            if (spa->addr[pidx] != NULL) {
                if (spa->canupdate[pidx] == 0) {
                    if (spa->prev_addr[pidx] != NULL)
                         free(spa->prev_addr[pidx]);
                    spa->prev_addr[pidx] = spa->addr[pidx];
                } else {
                    free(spa->addr[pidx]);
                }
            }
            spa->addr[pidx] = ulop->ia[0];
            ulop->ia[0] = NULL;
        }
        if (spa->rtcp->untrusted_addr[pidx] == 0 && !(spa->rtcp->addr[pidx] != NULL &&
          SA_LEN(ulop->ia[1]) == SA_LEN(spa->rtcp->addr[pidx]) &&
          memcmp(ulop->ia[1], spa->rtcp->addr[pidx], SA_LEN(ulop->ia[1])) == 0)) {
            if (spa->rtcp->addr[pidx] != NULL) {
                if (spa->rtcp->canupdate[pidx] == 0) {
                    if (spa->rtcp->prev_addr[pidx] != NULL)
                        free(spa->rtcp->prev_addr[pidx]);
                    spa->rtcp->prev_addr[pidx] = spa->rtcp->addr[pidx];
                } else {
                    free(spa->rtcp->addr[pidx]);
                }
            }
            spa->rtcp->addr[pidx] = ulop->ia[1];
            ulop->ia[1] = NULL;
        }
    }
    spa->asymmetric[pidx] = spa->rtcp->asymmetric[pidx] = ulop->asymmetric;
    spa->canupdate[pidx] = spa->rtcp->canupdate[pidx] = NOT(ulop->asymmetric);
    if (spa->codecs[pidx] != NULL) {
        free(spa->codecs[pidx]);
        spa->codecs[pidx] = NULL;
    }
    if (ulop->codecs != NULL) {
        spa->codecs[pidx] = ulop->codecs;
        ulop->codecs = NULL;
    }
    if (ulop->requested_nsamples > 0) {
        rtpp_log_write(RTPP_LOG_INFO, spa->log, "RTP packets from %s "
          "will be resized to %d milliseconds",
          (pidx == 0) ? "callee" : "caller", ulop->requested_nsamples / 8);
    } else if (spa->resizers[pidx] != NULL) {
          rtpp_log_write(RTPP_LOG_INFO, spa->log, "Resizing of RTP "
          "packets from %s has been disabled",
          (pidx == 0) ? "callee" : "caller");
    }
    if (ulop->requested_nsamples > 0) {
        if (spa->resizers[pidx] != NULL) {
            rtp_resizer_set_onsamples(spa->resizers[pidx], ulop->requested_nsamples);
        } else {
            spa->resizers[pidx] = rtp_resizer_new(ulop->requested_nsamples);
        }
    } else if (spa->resizers[pidx] != NULL) {
        rtp_resizer_free(cf->stable->rtpp_stats, spa->resizers[pidx]);
        spa->resizers[pidx] = NULL;
    }

    assert(lport != 0);
    ulop->reply.port = lport;
    ulop->reply.ia = ulop->lia[0];
    if (cf->stable->advaddr[0] != NULL) {
        if (cf->stable->bmode != 0 && cf->stable->advaddr[1] != NULL &&
          ulop->lia[0] == cf->stable->bindaddr[1]) {
            ulop->reply.ia_ov = cf->stable->advaddr[1];
        } else {
            ulop->reply.ia_ov = cf->stable->advaddr[0];
        }
    }
    ul_reply_port(cf, cmd, &ulop->reply);
    rtpp_command_ul_opts_free(ulop);
    return (0);

err_undo_0:
    rtpp_command_ul_opts_free(ulop);
    return (-1);
}
