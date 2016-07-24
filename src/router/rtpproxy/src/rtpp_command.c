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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtp.h"
#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_command.h"
#include "rtpp_command_async.h"
#include "rtpp_command_copy.h"
#include "rtpp_command_parse.h"
#include "rtpp_command_private.h"
#include "rtpp_command_stats.h"
#include "rtpp_command_ul.h"
#include "rtpp_netio_async.h"
#include "rtpp_network.h"
#include "rtpp_notify.h"
#include "rtpp_session.h"
#include "rtp_server.h"
#include "rtpp_util.h"
#include "rtpp_types.h"
#include "rtpp_stats.h"

struct proto_cap proto_caps[] = {
    /*
     * The first entry must be basic protocol version and isn't shown
     * as extension on -v.
     */
    { "20040107", "Basic RTP proxy functionality" },
    { "20050322", "Support for multiple RTP streams and MOH" },
    { "20060704", "Support for extra parameter in the V command" },
    { "20071116", "Support for RTP re-packetization" },
    { "20071218", "Support for forking (copying) RTP stream" },
    { "20080403", "Support for RTP statistics querying" },
    { "20081102", "Support for setting codecs in the update/lookup command" },
    { "20081224", "Support for session timeout notifications" },
    { "20090810", "Support for automatic bridging" },
    { "20140323", "Support for tracking/reporting load" },
    { "20140617", "Support for anchoring session connect time" },
    { "20141004", "Support for extendable performance counters" },
    { NULL, NULL }
};

struct d_opts;

static int create_twinlistener(struct rtpp_cfg_stable *, struct sockaddr *, int, int *);
static int handle_delete(struct cfg *, struct common_cmd_args *, int);
static void handle_noplay(struct cfg *, struct rtpp_session *, int, struct rtpp_command *);
static int handle_play(struct cfg *, struct rtpp_session *, int, char *, char *, int,
  struct rtpp_command *);
static int handle_record(struct cfg *, struct common_cmd_args *, int);
static void handle_query(struct cfg *, struct rtpp_command *,
  struct rtpp_session *, int);
static void handle_info(struct cfg *, struct rtpp_command *,
  const char *);
static void handle_ver_feature(struct cfg *cf, struct rtpp_command *cmd);

static int
create_twinlistener(struct rtpp_cfg_stable *cf, struct sockaddr *ia, int port, int *fds)
{
    struct sockaddr_storage iac;
    int rval, i, flags, so_rcvbuf;

    fds[0] = fds[1] = -1;

    rval = -1;
    for (i = 0; i < 2; i++) {
	fds[i] = socket(ia->sa_family, SOCK_DGRAM, 0);
	if (fds[i] == -1) {
	    rtpp_log_ewrite(RTPP_LOG_ERR, cf->glog, "can't create %s socket",
	      (ia->sa_family == AF_INET) ? "IPv4" : "IPv6");
	    goto failure;
	}
	memcpy(&iac, ia, SA_LEN(ia));
	satosin(&iac)->sin_port = htons(port);
	if (bind(fds[i], sstosa(&iac), SA_LEN(ia)) != 0) {
	    if (errno != EADDRINUSE && errno != EACCES) {
		rtpp_log_ewrite(RTPP_LOG_ERR, cf->glog, "can't bind to the %s port %d",
		  (ia->sa_family == AF_INET) ? "IPv4" : "IPv6", port);
	    } else {
		rval = -2;
	    }
	    goto failure;
	}
	port++;
	if ((ia->sa_family == AF_INET) && (cf->tos >= 0) &&
	  (setsockopt(fds[i], IPPROTO_IP, IP_TOS, &cf->tos, sizeof(cf->tos)) == -1))
	    rtpp_log_ewrite(RTPP_LOG_ERR, cf->glog, "unable to set TOS to %d", cf->tos);
	so_rcvbuf = 256 * 1024;
	if (setsockopt(fds[i], SOL_SOCKET, SO_RCVBUF, &so_rcvbuf, sizeof(so_rcvbuf)) == -1)
	    rtpp_log_ewrite(RTPP_LOG_ERR, cf->glog, "unable to set 256K receive buffer size");
	flags = fcntl(fds[i], F_GETFL);
	fcntl(fds[i], F_SETFL, flags | O_NONBLOCK);
    }
    return 0;

failure:
    for (i = 0; i < 2; i++)
	if (fds[i] != -1) {
	    close(fds[i]);
	    fds[i] = -1;
	}
    return rval;
}

int
rtpp_create_listener(struct cfg *cf, struct sockaddr *ia, int *port, int *fds)
{
    int i, idx, rval;

    for (i = 0; i < 2; i++)
	fds[i] = -1;

    for (i = 1; i < cf->stable->port_table_len; i++) {
	idx = (cf->port_table_idx + i) % cf->stable->port_table_len;
	*port = cf->stable->port_table[idx];
	if (*port == cf->stable->port_ctl || *port == (cf->stable->port_ctl - 1))
	    continue;
	rval = create_twinlistener(cf->stable, ia, *port, fds);
	if (rval == 0) {
	    cf->port_table_idx = idx;
	    return 0;
	}
	if (rval == -1)
	    break;
    }
    return -1;
}

void
rtpc_doreply(struct cfg *cf, char *buf, int len, struct rtpp_command *cmd, int errd)
{

    buf[len] = '\0';
    rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "sending reply \"%s\"", buf);
    if (cmd->umode == 0) {
	write(cmd->controlfd, buf, len);
    } else {
        if (cmd->cookie != NULL) {
            len = snprintf(cmd->buf_r, sizeof(cmd->buf_r), "%s %s", cmd->cookie,
              buf);
            buf = cmd->buf_r;
        }
        rtpp_anetio_sendto(cf->stable->rtpp_netio_cf, cmd->controlfd, buf, len, 0,
          sstosa(&cmd->raddr), cmd->rlen);
    }
    cmd->csp->ncmds_repld.cnt++;
    if (errd == 0) {
        cmd->csp->ncmds_succd.cnt++;
    } else {
        cmd->csp->ncmds_errs.cnt++;
    }
}

static void
reply_number(struct cfg *cf, struct rtpp_command *cmd,
  int number)
{
    int len;

    len = snprintf(cmd->buf_t, sizeof(cmd->buf_t), "%d\n", number);
    rtpc_doreply(cf, cmd->buf_t, len, cmd, 0);
}

static void
reply_ok(struct cfg *cf, struct rtpp_command *cmd)
{

    reply_number(cf, cmd, 0);
}

void
reply_error(struct cfg *cf, struct rtpp_command *cmd,
  int ecode)
{
    int len;

    len = snprintf(cmd->buf_t, sizeof(cmd->buf_t), "E%d\n", ecode);
    rtpc_doreply(cf, cmd->buf_t, len, cmd, 1);
}

void
free_command(struct rtpp_command *cmd)
{

    free(cmd);
}

struct rtpp_command *
get_command(struct cfg *cf, int controlfd, int *rval, double dtime,
  struct rtpp_command_stats *csp, int umode)
{
    char **ap;
    char *cp;
    int len, i;
    struct rtpp_command *cmd;

    cmd = malloc(sizeof(struct rtpp_command));
    if (cmd == NULL) {
        *rval = ENOMEM;
        return (NULL);
    }
    memset(cmd, 0, sizeof(struct rtpp_command));
    cmd->controlfd = controlfd;
    cmd->dtime = dtime;
    cmd->csp = csp;
    cmd->umode = umode;
    if (umode == 0) {
        for (;;) {
            len = read(controlfd, cmd->buf, sizeof(cmd->buf) - 1);
            if (len != -1 || (errno != EAGAIN && errno != EINTR))
                break;
        }
    } else {
        cmd->rlen = sizeof(cmd->raddr);
        len = recvfrom(controlfd, cmd->buf, sizeof(cmd->buf) - 1, 0,
          sstosa(&cmd->raddr), &cmd->rlen);
    }
    if (len == -1) {
        if (errno != EAGAIN && errno != EINTR)
            rtpp_log_ewrite(RTPP_LOG_ERR, cf->stable->glog, "can't read from control socket");
        free(cmd);
        *rval = -1;
        return (NULL);
    }
    cmd->buf[len] = '\0';

    rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "received command \"%s\"", cmd->buf);
    csp->ncmds_rcvd.cnt++;

    cp = cmd->buf;
    for (ap = cmd->argv; (*ap = rtpp_strsep(&cp, "\r\n\t ")) != NULL;) {
        if (**ap != '\0') {
            cmd->argc++;
            if (++ap >= &cmd->argv[RTPC_MAX_ARGC])
                break;
        }
    }
    if (cmd->argc < 1 || (umode != 0 && cmd->argc < 2)) {
        rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
        reply_error(cf, cmd, ECODE_PARSE_1);
        *rval = 0;
        free(cmd);
        return (NULL);
    }

    /* Stream communication mode doesn't use cookie */
    if (umode != 0) {
        cmd->cookie = cmd->argv[0];
        for (i = 1; i < cmd->argc; i++)
            cmd->argv[i - 1] = cmd->argv[i];
        cmd->argc--;
        cmd->argv[cmd->argc] = NULL;
    }

    /* Step I: parse parameters that are common to all ops */
    if (rtpp_command_pre_parse(cf, cmd) != 0) {
        /* Error reply is handled by the rtpp_command_pre_parse() */
        *rval = 0;
        free(cmd);
        return (NULL);
    }

    return (cmd);
}

struct d_opts {
    int weak;
};

int
handle_command(struct cfg *cf, struct rtpp_command *cmd)
{
    int i, verbose;
    int playcount;
    char *cp, *tcp;
    char *pname, *codecs, *recording_name;
    struct rtpp_session *spa;
    int record_single_file;
    struct ul_opts *ulop;
    struct d_opts dopt;

    spa = NULL;
    recording_name = NULL;
    codecs = NULL;

    /* Step II: parse parameters that are specific to a particular op and run simple ops */
    switch (cmd->cca.op) {
    case VER_FEATURE:
        handle_ver_feature(cf, cmd);
        return 0;

    case GET_VER:
        /* This returns base version. */
        reply_number(cf, cmd, CPROTOVER);
        return 0;

    case DELETE_ALL:
        /* Delete all active sessions */
        rtpp_log_write(RTPP_LOG_INFO, cf->stable->glog, "deleting all active sessions");
        pthread_mutex_lock(&cf->sessinfo.lock);
        for (i = 0; i < cf->sessinfo.nsessions; i++) {
            spa = cf->sessinfo.sessions[i];
            if (spa == NULL || spa->sidx[0] != i)
                continue;
            remove_session(cf, spa);
        }
        pthread_mutex_unlock(&cf->sessinfo.lock);
        reply_ok(cf, cmd);
        return 0;

    case INFO:
        handle_info(cf, cmd, &cmd->argv[0][1]);
        return 0;

    case PLAY:
        /*
         * P callid pname codecs from_tag to_tag
         *
         *   <codecs> could be either comma-separated list of supported
         *   payload types or word "session" (without quotes), in which
         *   case list saved on last session update will be used instead.
         */
        playcount = 1;
        pname = cmd->argv[2];
        codecs = cmd->argv[3];
        tcp = &(cmd->argv[0][1]);
	if (*tcp != '\0') {
	    playcount = strtol(tcp, &cp, 10);
            if (cp == tcp || *cp != '\0') {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
                reply_error(cf, cmd, ECODE_PARSE_6);
                return 0;
            }
        }
        break;

    case COPY:
        recording_name = cmd->argv[2];
        /* Fallthrough */
    case RECORD:
        if (cmd->argv[0][1] == 'S' || cmd->argv[0][1] == 's') {
            if (cmd->argv[0][2] != '\0') {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
                reply_error(cf, cmd, ECODE_PARSE_2);
                return 0;
            }
            record_single_file = (cf->stable->record_pcap == 0) ? 0 : 1;
        } else {
            if (cmd->argv[0][1] != '\0') {
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
                reply_error(cf, cmd, ECODE_PARSE_3);
                return 0;
            }
            record_single_file = 0;
        }
        break;

    case DELETE:
        /* D[w] call_id from_tag [to_tag] */
        dopt.weak = 0;
        for (cp = cmd->argv[0] + 1; *cp != '\0'; cp++) {
            switch (*cp) {
            case 'w':
            case 'W':
                dopt.weak = 1;
                break;

            default:
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog,
                  "DELETE: unknown command modifier `%c'", *cp);
                reply_error(cf, cmd, ECODE_PARSE_4);
                return 0;
            }
        }
        break;

    case UPDATE:
    case LOOKUP:
        ulop = rtpp_command_ul_opts_parse(cf, cmd);
        if (ulop == NULL) {
            return 0;
        }
	break;

    case GET_STATS:
        verbose = 0;
        for (cp = cmd->argv[0] + 1; *cp != '\0'; cp++) {
            switch (*cp) {
            case 'v':
            case 'V':
                verbose = 1;
                break;

            default:
                rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog,
                  "STATS: unknown command modifier `%c'", *cp);
                reply_error(cf, cmd, ECODE_PARSE_5);
                return 0;
            }
        }
        i = handle_get_stats(cf, cmd, verbose);
        if (i != 0) {
            reply_error(cf, cmd, i);
        }
        return 0;

    default:
        break;
    }

    /*
     * Record and delete need special handling since they apply to all
     * streams in the session.
     */
    switch (cmd->cca.op) {
    case DELETE:
	i = handle_delete(cf, &cmd->cca, dopt.weak);
	break;

    case RECORD:
	i = handle_record(cf, &cmd->cca, record_single_file);
	break;

    default:
	i = find_stream(cf, cmd->cca.call_id, cmd->cca.from_tag, cmd->cca.to_tag, &spa);
	if (i != -1 && cmd->cca.op != UPDATE)
	    i = NOT(i);
	break;
    }

    if (i == -1 && cmd->cca.op != UPDATE) {
	rtpp_log_write(RTPP_LOG_INFO, cf->stable->glog,
	  "%s request failed: session %s, tags %s/%s not found", cmd->cca.rname,
	  cmd->cca.call_id, cmd->cca.from_tag, cmd->cca.to_tag != NULL ? cmd->cca.to_tag : "NONE");
	if (cmd->cca.op == LOOKUP) {
            rtpp_command_ul_opts_free(ulop);
	    ul_reply_port(cf, cmd, NULL);
	    return 0;
	}
	reply_error(cf, cmd, ECODE_SESUNKN);
	return 0;
    }

    switch (cmd->cca.op) {
    case DELETE:
    case RECORD:
	reply_ok(cf, cmd);
	break;

    case NOPLAY:
	handle_noplay(cf, spa, i, cmd);
	reply_ok(cf, cmd);
	break;

    case PLAY:
	handle_noplay(cf, spa, i, cmd);
	if (strcmp(codecs, "session") == 0) {
	    if (spa->codecs[i] == NULL) {
		reply_error(cf, cmd, ECODE_INVLARG_5);
		return 0;
	    }
	    codecs = spa->codecs[i];
	}
	if (playcount != 0 && handle_play(cf, spa, i, codecs, pname, playcount, cmd) != 0) {
	    reply_error(cf, cmd, ECODE_PLRFAIL);
	    return 0;
	}
	reply_ok(cf, cmd);
	break;

    case COPY:
	if (handle_copy(cf, spa, i, recording_name, record_single_file) != 0) {
            reply_error(cf, cmd, ECODE_CPYFAIL);
            return 0;
        }
	reply_ok(cf, cmd);
	break;

    case QUERY:
	handle_query(cf, cmd, spa, i);
	break;

    case LOOKUP:
    case UPDATE:
        rtpp_command_ul_handle(cf, cmd, ulop, spa, i);
	break;

    default:
	/* Programmatic error, should not happen */
	abort();
    }

    return 0;
}

static int
handle_delete(struct cfg *cf, struct common_cmd_args *ccap, int weak)
{
    int ndeleted;
    unsigned int medianum;
    struct rtpp_session *spa, *spb;
    int cmpr, cmpr1, idx;

    ndeleted = 0;
    for (spa = session_findfirst(cf, ccap->call_id); spa != NULL;) {
	medianum = 0;
	if ((cmpr1 = compare_session_tags(spa->tag, ccap->from_tag, &medianum)) != 0) {
	    idx = 1;
	    cmpr = cmpr1;
	} else if (ccap->to_tag != NULL &&
	  (cmpr1 = compare_session_tags(spa->tag, ccap->to_tag, &medianum)) != 0) {
	    idx = 0;
	    cmpr = cmpr1;
	} else {
	    spa = session_findnext(cf, spa);
	    continue;
	}

	if (weak)
	    spa->weak[idx] = 0;
	else
	    spa->strong = 0;

	/*
	 * This seems to be stable from reiterations, the only side
	 * effect is less efficient work.
	 */
	if (spa->strong || spa->weak[0] || spa->weak[1]) {
	    rtpp_log_write(RTPP_LOG_INFO, spa->log,
	      "delete: medianum=%u: removing %s flag, seeing flags to"
	      " continue session (strong=%d, weak=%d/%d)",
	      medianum,
	      weak ? ( idx ? "weak[1]" : "weak[0]" ) : "strong",
	      spa->strong, spa->weak[0], spa->weak[1]);
	    /* Skipping to next possible stream for this call */
	    ++ndeleted;
	    spa = session_findnext(cf, spa);
	    continue;
	}
	rtpp_log_write(RTPP_LOG_INFO, spa->log,
	  "forcefully deleting session %u on ports %d/%d",
	   medianum, spa->ports[0], spa->ports[1]);
	/* Search forward before we do removal */
	spb = spa;
	spa = session_findnext(cf, spa);
        pthread_mutex_lock(&cf->sessinfo.lock);
	remove_session(cf, spb);
        pthread_mutex_unlock(&cf->sessinfo.lock);
	++ndeleted;
	if (cmpr != 2) {
	    break;
	}
    }
    if (ndeleted == 0) {
	return -1;
    }
    return 0;
}

static void
handle_noplay(struct cfg *cf, struct rtpp_session *spa, int idx, struct rtpp_command *cmd)
{

    if (spa->rtps[idx] != NULL) {
	rtp_server_free(spa->rtps[idx]);
	cmd->csp->nplrs_destroyed.cnt++;
	spa->rtps[idx] = NULL;
	rtpp_log_write(RTPP_LOG_INFO, spa->log,
	  "stopping player at port %d", spa->ports[idx]);
	if (spa->rtps[0] == NULL && spa->rtps[1] == NULL) {
	    assert(cf->rtp_servers[spa->sridx] == spa);
	    cf->rtp_servers[spa->sridx] = NULL;
	    spa->sridx = -1;
	}
   }
}

static int
handle_play(struct cfg *cf, struct rtpp_session *spa, int idx, char *codecs,
  char *pname, int playcount, struct rtpp_command *cmd)
{
    int n;
    char *cp;

    while (*codecs != '\0') {
	n = strtol(codecs, &cp, 10);
	if (cp == codecs)
	    break;
	codecs = cp;
	if (*codecs != '\0')
	    codecs++;
	spa->rtps[idx] = rtp_server_new(pname, n, playcount);
	if (spa->rtps[idx] == NULL)
	    continue;
	cmd->csp->nplrs_created.cnt++;
	rtpp_log_write(RTPP_LOG_INFO, spa->log,
	  "%d times playing prompt %s codec %d", playcount, pname, n);
	if (spa->sridx == -1)
	    append_server(cf, spa);
	return 0;
    }
    rtpp_log_write(RTPP_LOG_ERR, spa->log, "can't create player");
    return -1;
}

static int
handle_record(struct cfg *cf, struct common_cmd_args *ccap,
  int record_single_file)
{
    int nrecorded, idx;
    struct rtpp_session *spa;

    nrecorded = 0;
    for (spa = session_findfirst(cf, ccap->call_id); spa != NULL;
      spa = session_findnext(cf, spa)) {
	if (compare_session_tags(spa->tag, ccap->from_tag, NULL) != 0) {
	    idx = 1;
	} else if (ccap->to_tag != NULL &&
	  (compare_session_tags(spa->tag, ccap->to_tag, NULL)) != 0) {
	    idx = 0;
	} else {
	    continue;
	}
	if (handle_copy(cf, spa, idx, NULL, record_single_file) == 0) {
            nrecorded++;
        }
    }
    return (nrecorded == 0 ? -1 : 0);
}

static void
handle_query(struct cfg *cf, struct rtpp_command *cmd,
  struct rtpp_session *spa, int idx)
{
    int len;

    len = snprintf(cmd->buf_t, sizeof(cmd->buf_t), "%d %lu %lu %lu %lu\n", get_ttl(spa),
      spa->pcount[idx], spa->pcount[NOT(idx)], spa->pcount[2],
      spa->pcount[3]);
    rtpc_doreply(cf, cmd->buf_t, len, cmd, 0);
}

static void
handle_info(struct cfg *cf, struct rtpp_command *cmd,
  const char *opts)
{
#if 0
    struct rtpp_session *spa, *spb;
    char addrs[4][256];
#endif
    int len, i, brief, load;
    char buf[1024 * 8];
    unsigned long long packets_in, packets_out;

    brief = 0;
    load = 0;
    for (i = 0; opts[i] != '\0'; i++) {
        switch (opts[i]) {
        case 'b':
        case 'B':
            brief = 1;
            break;

        case 'l':
        case 'L':
            load = 1;
            break;

        default:
            rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
            reply_error(cf, cmd, ECODE_PARSE_7);
            return;
        }
    }

    packets_in = CALL_METHOD(cf->stable->rtpp_stats, getlvalbyname, "npkts_rcvd");
    packets_out = CALL_METHOD(cf->stable->rtpp_stats, getlvalbyname, "npkts_relayed") +
      CALL_METHOD(cf->stable->rtpp_stats, getlvalbyname, "npkts_played");
    pthread_mutex_lock(&cf->sessinfo.lock);
    len = snprintf(buf, sizeof(buf), "sessions created: %llu\nactive sessions: %d\n"
      "active streams: %d\npackets received: %llu\npackets transmitted: %llu\n",
      cf->sessions_created, cf->sessions_active, cf->sessinfo.nsessions,
      packets_in, packets_out);
    if (load != 0) {
          len += snprintf(buf + len, sizeof(buf) - len, "average load: %f\n",
            rtpp_command_async_get_aload(cf->stable->rtpp_cmd_cf));
    }
#if 0
XXX this needs work to fix it after rtp/rtcp split 
    for (i = 0; i < cf->sessinfo.nsessions && brief == 0; i++) {
        spa = cf->sessinfo.sessions[i];
        if (spa == NULL || spa->sidx[0] != i)
            continue;
        /* RTCP twin session */
        if (spa->rtcp == NULL) {
            spb = spa->rtp;
            buf[len++] = '\t';
        } else {
            spb = spa->rtcp;
            buf[len++] = '\t';
            buf[len++] = 'C';
            buf[len++] = ' ';
        }

        addr2char_r(spb->laddr[1], addrs[0], sizeof(addrs[0]));
        if (spb->addr[1] == NULL) {
            strcpy(addrs[1], "NONE");
        } else {
            sprintf(addrs[1], "%s:%d", addr2char(spb->addr[1]),
              addr2port(spb->addr[1]));
        }
        addr2char_r(spb->laddr[0], addrs[2], sizeof(addrs[2]));
        if (spb->addr[0] == NULL) {
            strcpy(addrs[3], "NONE");
        } else {
            sprintf(addrs[3], "%s:%d", addr2char(spb->addr[0]),
              addr2port(spb->addr[0]));
        }

        len += snprintf(buf + len, sizeof(buf) - len,
          "%s/%s: caller = %s:%d/%s, callee = %s:%d/%s, "
          "stats = %lu/%lu/%lu/%lu, ttl = %d/%d\n",
          spb->call_id, spb->tag, addrs[0], spb->ports[1], addrs[1],
          addrs[2], spb->ports[0], addrs[3], spa->pcount[0], spa->pcount[1],
          spa->pcount[2], spa->pcount[3], spb->ttl[0], spb->ttl[1]);
        if (len + 512 > sizeof(buf)) {
            rtpc_doreply(cf, buf, len, cmd);
            len = 0;
        }
    }
#endif
    pthread_mutex_unlock(&cf->sessinfo.lock);
    if (len > 0) {
        rtpc_doreply(cf, buf, len, cmd, 0);
    }
}

static void
handle_ver_feature(struct cfg *cf, struct rtpp_command *cmd)
{
    int i, known;

    /*
     * Wait for protocol version datestamp and check whether we
     * know it.
     */
    /*
     * Only list 20081224 protocol mod as supported if
     * user actually enabled notification with -n
     */
    if (strcmp(cmd->argv[1], "20081224") == 0 &&
      rtpp_th_get_sn(cf->timeout_handler) == NULL) {
        reply_number(cf, cmd, 0);
        return;
    }
    for (known = i = 0; proto_caps[i].pc_id != NULL; ++i) {
        if (!strcmp(cmd->argv[1], proto_caps[i].pc_id)) {
            known = 1;
            break;
        }
    }
    reply_number(cf, cmd, known);
}
