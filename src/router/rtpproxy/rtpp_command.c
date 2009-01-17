/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2007 Sippy Software, Inc., http://www.sippysoft.com
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
 * $Id: rtpp_command.c,v 1.24 2009/01/12 10:56:59 sobomax Exp $
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtpp_command.h"
#include "rtpp_log.h"
#include "rtpp_record.h"
#include "rtpp_session.h"
#include "rtpp_util.h"

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
    { NULL, NULL }
};

static int create_twinlistener(struct cfg *, struct sockaddr *, int, int *);
static int create_listener(struct cfg *, struct sockaddr *, int *, int *);
static int handle_delete(struct cfg *, char *, char *, char *, int);
static void handle_noplay(struct cfg *, struct rtpp_session *, int);
static int handle_play(struct cfg *, struct rtpp_session *, int, char *, char *, int);
static void handle_copy(struct cfg *, struct rtpp_session *, int, char *);
static int handle_record(struct cfg *, char *, char *, char *);
static void handle_query(struct cfg *, int, struct sockaddr_storage *,
  socklen_t, char *, struct rtpp_session *, int);

static int
create_twinlistener(struct cfg *cf, struct sockaddr *ia, int port, int *fds)
{
    struct sockaddr_storage iac;
    int rval, i, flags;

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
	if ((ia->sa_family == AF_INET) &&
	  (setsockopt(fds[i], IPPROTO_IP, IP_TOS, &cf->tos, sizeof(cf->tos)) == -1))
	    rtpp_log_ewrite(RTPP_LOG_ERR, cf->glog, "unable to set TOS to %d", cf->tos);
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

static int
create_listener(struct cfg *cf, struct sockaddr *ia, int *port, int *fds)
{
    int i, idx, rval;

    for (i = 0; i < 2; i++)
	fds[i] = -1;

    for (i = 1; i < cf->port_table_len; i++) {
	idx = (cf->port_table_idx + i) % cf->port_table_len;
	*port = cf->port_table[idx];
	rval = create_twinlistener(cf, ia, *port, fds);
	if (rval == 0) {
	    cf->port_table_idx = idx;
	    return 0;
	}
	if (rval == -1)
	    break;
    }
    return -1;
}

static void
doreply(struct cfg *cf, int fd, char *buf, int len,
  struct sockaddr_storage *raddr, socklen_t rlen)
{

    buf[len] = '\0';
    rtpp_log_write(RTPP_LOG_DBUG, cf->glog, "sending reply \"%s\"", buf);
    if (cf->umode == 0) {
	write(fd, buf, len);
    } else {
	while (sendto(fd, buf, len, 0, sstosa(raddr),
	  rlen) == -1 && errno == ENOBUFS);
    }
}

static void
reply_number(struct cfg *cf, int fd, struct sockaddr_storage *raddr,
  socklen_t rlen, char *cookie, int number)
{
    int len;
    char buf[1024 * 8];

    if (cookie != NULL)
	len = sprintf(buf, "%s %d\n", cookie, number);
    else {
	len = sprintf(buf, "%d\n", number);
    }
    doreply(cf, fd, buf, len, raddr, rlen);
}

static void
reply_ok(struct cfg *cf, int fd, struct sockaddr_storage *raddr,
  socklen_t rlen, char *cookie)
{

    reply_number(cf, fd, raddr, rlen, cookie, 0);
}

static void
reply_port(struct cfg *cf, int fd, struct sockaddr_storage *raddr,
  socklen_t rlen, char *cookie, int lport, struct sockaddr **lia)
{
    int len;
    char buf[1024 * 8], *cp;

    cp = buf;
    len = 0;
    if (cookie != NULL) {
	len = sprintf(cp, "%s ", cookie);
	cp += len;
    }
    if (lia[0] == NULL || ishostnull(lia[0]))
	len += sprintf(cp, "%d\n", lport);
    else
	len += sprintf(cp, "%d %s%s\n", lport, addr2char(lia[0]),
	  (lia[0]->sa_family == AF_INET) ? "" : " 6");
    doreply(cf, fd, buf, len, raddr, rlen);
}

static void
reply_error(struct cfg *cf, int fd, struct sockaddr_storage *raddr,
  socklen_t rlen, char *cookie, int ecode)
{
    int len;
    char buf[1024 * 8];

    if (cookie != NULL)
	len = sprintf(buf, "%s E%d\n", cookie, ecode);
    else
	len = sprintf(buf, "E%d\n", ecode);
    doreply(cf, fd, buf, len, raddr, rlen);
}

static void
handle_nomem(struct cfg *cf, int fd, struct sockaddr_storage *raddr,
  socklen_t rlen, char *cookie, int ecode, struct sockaddr **ia, int *fds,
  struct rtpp_session *spa, struct rtpp_session *spb)
{
    int i;

    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "can't allocate memory");
    for (i = 0; i < 2; i++)
	if (ia[i] != NULL)
	    free(ia[i]);
    if (spa != NULL) {
	if (spa->call_id != NULL)
	    free(spa->call_id);
	free(spa);
    }
    if (spb != NULL)
	free(spb);
    for (i = 0; i < 2; i++)
	if (fds[i] != -1)
	    close(fds[i]);
    reply_error(cf, fd, raddr, rlen, cookie, ecode);
}

int
handle_command(struct cfg *cf, int controlfd, double dtime)
{
    int len, argc, i, j, pidx, asymmetric;
    int external, pf, lidx, playcount, weak;
    int fds[2], lport, n;
    socklen_t rlen;
    char buf[1024 * 8];
    char *cp, *call_id, *from_tag, *to_tag, *addr, *port, *cookie;
    char *pname, *codecs, *recording_name, *t;
    struct rtpp_session *spa, *spb;
    char **ap, *argv[10];
    const char *rname;
    struct sockaddr *ia[2], *lia[2];
    struct sockaddr_storage raddr;
    int requested_nsamples;
    enum {DELETE, RECORD, PLAY, NOPLAY, COPY, UPDATE, LOOKUP, QUERY} op;
    int max_argc;
    char *socket_name_u, *notify_tag;

    requested_nsamples = -1;
    ia[0] = ia[1] = NULL;
    spa = spb = NULL;
    lia[0] = lia[1] = cf->bindaddr[0];
    lidx = 1;
    fds[0] = fds[1] = -1;
    recording_name = NULL;
    socket_name_u = notify_tag = NULL;

    if (cf->umode == 0) {
	for (;;) {
	    len = read(controlfd, buf, sizeof(buf) - 1);
	    if (len != -1 || (errno != EAGAIN && errno != EINTR))
		break;
	    sched_yield();
	}
    } else {
	rlen = sizeof(raddr);
	len = recvfrom(controlfd, buf, sizeof(buf) - 1, 0,
	  sstosa(&raddr), &rlen);
    }
    if (len == -1) {
	if (errno != EAGAIN && errno != EINTR)
	    rtpp_log_ewrite(RTPP_LOG_ERR, cf->glog, "can't read from control socket");
	return -1;
    }
    buf[len] = '\0';

    rtpp_log_write(RTPP_LOG_DBUG, cf->glog, "received command \"%s\"", buf);

    cp = buf;
    argc = 0;
    memset(argv, 0, sizeof(argv));
    for (ap = argv; (*ap = rtpp_strsep(&cp, "\r\n\t ")) != NULL;)
	if (**ap != '\0') {
	    argc++;
	    if (++ap >= &argv[10])
		break;
	}
    cookie = NULL;
    if (argc < 1 || (cf->umode != 0 && argc < 2)) {
	rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
	reply_error(cf, controlfd, &raddr, rlen, cookie, 0);
	return 0;
    }

    /* Stream communication mode doesn't use cookie */
    if (cf->umode != 0) {
	cookie = argv[0];
	for (i = 1; i < argc; i++)
	    argv[i - 1] = argv[i];
	argc--;
	argv[argc] = NULL;
    } else {
	cookie = NULL;
    }

    addr = port = NULL;
    switch (argv[0][0]) {
    case 'u':
    case 'U':
	/* U[opts] callid remote_ip remote_port from_tag [to_tag] */
	op = UPDATE;
	rname = "update/create";
	break;

    case 'l':
    case 'L':
	op = LOOKUP;
	rname = "lookup";
	break;

    case 'd':
    case 'D':
	op = DELETE;
	rname = "delete";
	break;

    case 'p':
    case 'P':
	/*
	 * P callid pname codecs from_tag to_tag
	 *
	 *   <codecs> could be either comma-separated list of supported
	 *   payload types or word "session" (without quotes), in which
	 *   case list saved on last session update will be used instead.
	 */
	op = PLAY;
	rname = "play";
	playcount = 1;
	pname = argv[2];
	codecs = argv[3];
	break;

    case 'r':
    case 'R':
	op = RECORD;
	rname = "record";
	break;

    case 'c':
    case 'C':
	op = COPY;
	rname = "copy";
	break;

    case 's':
    case 'S':
	op = NOPLAY;
	rname = "noplay";
	break;

    case 'v':
    case 'V':
	if (argv[0][1] == 'F' || argv[0][1] == 'f') {
	    int i, known;
	    /*
	     * Wait for protocol version datestamp and check whether we
	     * know it.
	     */
	    if (argc != 2 && argc != 3) {
		rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
		reply_error(cf, controlfd, &raddr, rlen, cookie, 2);
		return 0;
	    }
	    for (known = i = 0; proto_caps[i].pc_id != NULL; ++i) {
		if (!strcmp(argv[1], proto_caps[i].pc_id)) {
		    known = 1;
		    break;
		}
	    }
	    reply_number(cf, controlfd, &raddr, rlen, cookie, known);
	    return 0;
	}
	if (argc != 1 && argc != 2) {
	    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
	    reply_error(cf, controlfd, &raddr, rlen, cookie, 2);
	    return 0;
	}
	/* This returns base version. */
	reply_number(cf, controlfd, &raddr, rlen, cookie, CPROTOVER);
	return 0;

    case 'i':
    case 'I':
	if (cookie == NULL)
	    len = sprintf(buf, "sessions created: %llu\nactive sessions: %d\n"
	      "active streams: %d\n", cf->sessions_created,
	      cf->sessions_active, cf->nsessions / 2);
	else
	    len = sprintf(buf, "%s sessions created: %llu\nactive sessions: %d\n"
	      "active streams: %d\n", cookie, cf->sessions_created,
	      cf->sessions_active, cf->nsessions / 2);
	for (i = 1; i < cf->nsessions; i++) {
	    char addrs[4][256];

	    spa = cf->sessions[i];
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

	    len += sprintf(buf + len,
	      "%s/%s: caller = %s:%d/%s, callee = %s:%d/%s, "
	      "stats = %lu/%lu/%lu/%lu, ttl = %d/%d\n",
	      spb->call_id, spb->tag, addrs[0], spb->ports[1], addrs[1],
	      addrs[2], spb->ports[0], addrs[3], spa->pcount[0], spa->pcount[1],
	      spa->pcount[2], spa->pcount[3], spb->ttl[0], spb->ttl[1]);
	    if (len + 512 > sizeof(buf)) {
		doreply(cf, controlfd, buf, len, &raddr, rlen);
		len = 0;
	    }
	}
	if (len > 0)
	    doreply(cf, controlfd, buf, len, &raddr, rlen);;
	return 0;
	break;

    case 'q':
    case 'Q':
	op = QUERY;
	rname = "query";
	break;

    case 'x':
    case 'X':
        /* Delete all active sessions */
        rtpp_log_write(RTPP_LOG_INFO, cf->glog, "deleting all active sessions");
        for (i = 1; i < cf->nsessions; i++) {
	    spa = cf->sessions[i];
	    if (spa == NULL || spa->sidx[0] != i)
		continue;
	    /* Skip RTCP twin session */
	    if (spa->rtcp != NULL) {
		remove_session(cf, spa);
	    }
        }
        reply_ok(cf, controlfd, &raddr, rlen, cookie);
        return 0;
        break;

    default:
	rtpp_log_write(RTPP_LOG_ERR, cf->glog, "unknown command");
	reply_error(cf, controlfd, &raddr, rlen, cookie, 3);
	return 0;
    }
    call_id = argv[1];
    if (op == UPDATE || op == LOOKUP || op == PLAY) {
	max_argc = (op == UPDATE ? 8 : 6);
	if (argc < 5 || argc > max_argc) {
	    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
	    reply_error(cf, controlfd, &raddr, rlen, cookie, 4);
	    return 0;
	}
	from_tag = argv[4];
	to_tag = argv[5];
	if (op == PLAY && argv[0][1] != '\0')
	    playcount = atoi(argv[0] + 1);
	if (op == UPDATE && argc > 6) {
	    socket_name_u = argv[6];
	    if (strncmp("unix:", socket_name_u, 5) == 0)
		socket_name_u += 5;
	    if (argc == 8) {
		notify_tag = argv[7];
		len = url_unquote((uint8_t *)notify_tag, strlen(notify_tag));
		if (len == -1) {
		    rtpp_log_write(RTPP_LOG_ERR, cf->glog,
		      "command syntax error - invalid URL encoding");
		    reply_error(cf, controlfd, &raddr, rlen, cookie, 4);
		    return 0;
		}
		notify_tag[len] = '\0';
	    }
	}
    }
    if (op == COPY) {
	if (argc < 4 || argc > 5) {
	    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
	    reply_error(cf, controlfd, &raddr, rlen, cookie, 1);
	    return 0;
	}
	recording_name = argv[2];
	from_tag = argv[3];
	to_tag = argv[4];
    }
    if (op == DELETE || op == RECORD || op == NOPLAY || op == QUERY) {
	if (argc < 3 || argc > 4) {
	    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
	    reply_error(cf, controlfd, &raddr, rlen, cookie, 1);
	    return 0;
	}
	from_tag = argv[2];
	to_tag = argv[3];
    }
    if (op == DELETE || op == RECORD || op == COPY || op == NOPLAY) {
	/* D, R and S commands don't take any modifiers */
	if (argv[0][1] != '\0') {
	    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
	    reply_error(cf, controlfd, &raddr, rlen, cookie, 1);
	    return 0;
	}
    }
    if (op == UPDATE || op == LOOKUP || op == DELETE) {
	addr = argv[2];
	port = argv[3];
	/* Process additional command modifiers */
	external = 1;
	/* In bridge mode all clients are assumed to be asymmetric */
	asymmetric = (cf->bmode != 0) ? 1 : 0;
	pf = AF_INET;
	weak = 0;
	for (cp = argv[0] + 1; *cp != '\0'; cp++) {
	    switch (*cp) {
	    case 'a':
	    case 'A':
		asymmetric = 1;
		break;

	    case 'e':
	    case 'E':
		if (lidx < 0) {
		    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
		    reply_error(cf, controlfd, &raddr, rlen, cookie, 1);
		    return 0;
		}
		lia[lidx] = cf->bindaddr[1];
		lidx--;
		break;

	    case 'i':
	    case 'I':
		if (lidx < 0) {
		    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
		    reply_error(cf, controlfd, &raddr, rlen, cookie, 1);
		    return 0;
		}
		lia[lidx] = cf->bindaddr[0];
		lidx--;
		break;

	    case '6':
		pf = AF_INET6;
		break;

	    case 's':
	    case 'S':
		asymmetric = 0;
		break;

	    case 'w':
	    case 'W':
		weak = 1;
		break;

	    case 'z':
	    case 'Z':
		requested_nsamples = (strtol(cp + 1, &cp, 10) / 10) * 80;
		if (requested_nsamples <= 0) {
		    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
		    reply_error(cf, controlfd, &raddr, rlen, cookie, 1);
		    return 0;
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
		    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "command syntax error");
		    reply_error(cf, controlfd, &raddr, rlen, cookie, 1);
		    return 0;
		}
		codecs = alloca(cp - t + 1);
		memcpy(codecs, t, cp - t);
		codecs[cp - t] = '\0';
		cp--;
		break;

	    default:
		rtpp_log_write(RTPP_LOG_ERR, cf->glog, "unknown command modifier `%c'",
		  *cp);
		break;
	    }
	}
	if (op != DELETE && addr != NULL && port != NULL && strlen(addr) >= 7) {
	    struct sockaddr_storage tia;

	    if ((n = resolve(sstosa(&tia), pf, addr, port,
	      AI_NUMERICHOST)) == 0) {
		if (!ishostnull(sstosa(&tia))) {
		    for (i = 0; i < 2; i++) {
			ia[i] = malloc(SS_LEN(&tia));
			if (ia[i] == NULL) {
			    handle_nomem(cf, controlfd, &raddr, rlen, cookie,
			      5, ia, fds, spa, spb);
			    return 0;
			}
			memcpy(ia[i], &tia, SS_LEN(&tia));
		    }
		    /* Set port for RTCP, will work both for IPv4 and IPv6 */
		    n = ntohs(satosin(ia[1])->sin_port);
		    satosin(ia[1])->sin_port = htons(n + 1);
		}
	    } else {
		rtpp_log_write(RTPP_LOG_ERR, cf->glog, "getaddrinfo: %s",
		  gai_strerror(n));
	    }
	}
    }

    /*
     * Record and delete need special handling since they apply to all
     * streams in the session.
     */
    switch (op) {
    case DELETE:
	i = handle_delete(cf, call_id, from_tag, to_tag, weak);
	break;

    case RECORD:
	i = handle_record(cf, call_id, from_tag, to_tag);
	break;

    default:
	i = find_stream(cf, call_id, from_tag, to_tag, &spa);
	if (i != -1 && op != UPDATE)
	    i = NOT(i);
	break;
    }

    if (i == -1 && op != UPDATE) {
	rtpp_log_write(RTPP_LOG_INFO, cf->glog,
	  "%s request failed: session %s, tags %s/%s not found", rname,
	  call_id, from_tag, to_tag != NULL ? to_tag : "NONE");
	if (op == LOOKUP) {
	    for (i = 0; i < 2; i++)
		if (ia[i] != NULL)
		    free(ia[i]);
	    reply_port(cf, controlfd, &raddr, rlen, cookie, 0, lia);
	    return 0;
	}
	reply_error(cf, controlfd, &raddr, rlen, cookie, 8);
	return 0;
    }

    switch (op) {
    case DELETE:
    case RECORD:
	reply_ok(cf, controlfd, &raddr, rlen, cookie);
	return 0;

    case NOPLAY:
	handle_noplay(cf, spa, i);
	reply_ok(cf, controlfd, &raddr, rlen, cookie);
	return 0;

    case PLAY:
	handle_noplay(cf, spa, i);
	if (strcmp(codecs, "session") == 0) {
	    if (spa->codecs[i] == NULL) {
		reply_error(cf, controlfd, &raddr, rlen, cookie, 6);
		return 0;
	    }
	    codecs = spa->codecs[i];
	}
	if (playcount != 0 && handle_play(cf, spa, i, codecs, pname, playcount) != 0) {
	    reply_error(cf, controlfd, &raddr, rlen, cookie, 6);
	    return 0;
	}
	reply_ok(cf, controlfd, &raddr, rlen, cookie);
	return 0;

    case COPY:
	handle_copy(cf, spa, i, recording_name);
	reply_ok(cf, controlfd, &raddr, rlen, cookie);
	return 0;

    case QUERY:
	handle_query(cf, controlfd, &raddr, rlen, cookie, spa, i);
	return 0;

    case LOOKUP:
    case UPDATE:
	/* those are handled below */
	break;

    default:
	/* Programmatic error, should not happen */
	abort();
    }

    pidx = 1;
    lport = 0;
    if (i != -1) {
	assert(op == UPDATE || op == LOOKUP);
	if (spa->fds[i] == -1) {
	    j = ishostseq(cf->bindaddr[0], spa->laddr[i]) ? 0 : 1;
	    if (create_listener(cf, spa->laddr[i], &lport, fds) == -1) {
		rtpp_log_write(RTPP_LOG_ERR, spa->log, "can't create listener");
		reply_error(cf, controlfd, &raddr, rlen, cookie, 7);
		return 0;
	    }
	    assert(spa->fds[i] == -1);
	    spa->fds[i] = fds[0];
	    assert(spa->rtcp->fds[i] == -1);
	    spa->rtcp->fds[i] = fds[1];
	    spa->ports[i] = lport;
	    spa->rtcp->ports[i] = lport + 1;
	    spa->complete = spa->rtcp->complete = 1;
	    append_session(cf, spa, i);
	    append_session(cf, spa->rtcp, i);
	}
	if (weak)
	    spa->weak[i] = 1;
	else if (op == UPDATE)
	    spa->strong = 1;
	lport = spa->ports[i];
	lia[0] = spa->laddr[i];
	pidx = (i == 0) ? 1 : 0;
	spa->ttl_mode = cf->ttl_mode;
	spa->ttl[0] = cf->max_ttl;
	spa->ttl[1] = cf->max_ttl;
	if (op == UPDATE) {
	    rtpp_log_write(RTPP_LOG_INFO, spa->log,
	      "adding %s flag to existing session, new=%d/%d/%d",
	      weak ? ( i ? "weak[1]" : "weak[0]" ) : "strong",
	      spa->strong, spa->weak[0], spa->weak[1]);
	}
	rtpp_log_write(RTPP_LOG_INFO, spa->log,
	  "lookup on ports %d/%d, session timer restarted", spa->ports[0],
	  spa->ports[1]);
    } else {
	assert(op == UPDATE);
	rtpp_log_write(RTPP_LOG_INFO, cf->glog,
	  "new session %s, tag %s requested, type %s",
	  call_id, from_tag, weak ? "weak" : "strong");

	j = ishostseq(cf->bindaddr[0], lia[0]) ? 0 : 1;
	if (create_listener(cf, cf->bindaddr[j], &lport, fds) == -1) {
	    rtpp_log_write(RTPP_LOG_ERR, cf->glog, "can't create listener");
	    reply_error(cf, controlfd, &raddr, rlen, cookie, 10);
	    return 0;
	}

	/*
	 * Session creation. If creation is requested with weak flag,
	 * set weak[0].
	 */
	spa = malloc(sizeof(*spa));
	if (spa == NULL) {
	    handle_nomem(cf, controlfd, &raddr, rlen, cookie, 11, ia,
	      fds, spa, spb);
	    return 0;
	}
	/* spb is RTCP twin session for this one. */
	spb = malloc(sizeof(*spb));
	if (spb == NULL) {
	    handle_nomem(cf, controlfd, &raddr, rlen, cookie, 12, ia,
	      fds, spa, spb);
	    return 0;
	}
	memset(spa, 0, sizeof(*spa));
	memset(spb, 0, sizeof(*spb));
	for (i = 0; i < 2; i++) {
	    spa->fds[i] = spb->fds[i] = -1;
	    spa->last_update[i] = 0;
	    spb->last_update[i] = 0;
	}
	spa->call_id = strdup(call_id);
	if (spa->call_id == NULL) {
	    handle_nomem(cf, controlfd, &raddr, rlen, cookie, 13, ia,
	      fds, spa, spb);
	    return 0;
	}
	spb->call_id = spa->call_id;
	spa->tag = strdup(from_tag);
	if (spa->tag == NULL) {
	    handle_nomem(cf, controlfd, &raddr, rlen, cookie, 14, ia,
	      fds, spa, spb);
	    return 0;
	}
	spb->tag = spa->tag;
	for (i = 0; i < 2; i++) {
	    spa->rrcs[i] = NULL;
	    spb->rrcs[i] = NULL;
	    spa->laddr[i] = lia[i];
	    spb->laddr[i] = lia[i];
	}
	spa->strong = spa->weak[0] = spa->weak[1] = 0;
	if (weak)
	    spa->weak[0] = 1;
	else
	    spa->strong = 1;
	assert(spa->fds[0] == -1);
	spa->fds[0] = fds[0];
	assert(spb->fds[0] == -1);
	spb->fds[0] = fds[1];
	spa->ports[0] = lport;
	spb->ports[0] = lport + 1;
	spa->ttl[0] = cf->max_ttl;
	spa->ttl[1] = cf->max_ttl;
	spb->ttl[0] = -1;
	spb->ttl[1] = -1;
	spa->log = rtpp_log_open(cf, "rtpproxy", spa->call_id, 0);
	spb->log = spa->log;
	spa->rtcp = spb;
	spb->rtcp = NULL;
	spa->rtp = NULL;
	spb->rtp = spa;
	spa->sridx = spb->sridx = -1;

	append_session(cf, spa, 0);
	append_session(cf, spa, 1);
	append_session(cf, spb, 0);
	append_session(cf, spb, 1);

	hash_table_append(cf, spa);

	cf->sessions_created++;
	cf->sessions_active++;
	/*
	 * Each session can consume up to 5 open file descriptors (2 RTP,
	 * 2 RTCP and 1 logging) so that warn user when he is likely to
	 * exceed 80% mark on hard limit.
	 */
	if (cf->sessions_active > (cf->nofile_limit.rlim_max * 80 / (100 * 5)) &&
	  cf->nofile_limit_warned == 0) {
	    cf->nofile_limit_warned = 1;
	    rtpp_log_write(RTPP_LOG_WARN, cf->glog, "passed 80%% "
	      "threshold on the open file descriptors limit (%d), "
	      "consider increasing the limit using -L command line "
	      "option", (int)cf->nofile_limit.rlim_max);
	}

	rtpp_log_write(RTPP_LOG_INFO, spa->log, "new session on a port %d created, "
	  "tag %s", lport, from_tag);
	if (cf->record_all != 0) {
	    handle_copy(cf, spa, 0, NULL);
	    handle_copy(cf, spa, 1, NULL);
	}
    }

    if (op == UPDATE) {
	if (cf->timeout_handler.socket_name == NULL && socket_name_u != NULL)
	    rtpp_log_write(RTPP_LOG_ERR, spa->log, "must permit notification socket with -n");
	if (spa->timeout_data.notify_tag != NULL) {
	    free(spa->timeout_data.notify_tag);
	    spa->timeout_data.notify_tag = NULL;
	}
	if (cf->timeout_handler.socket_name != NULL && socket_name_u != NULL) {
	    if (strcmp(cf->timeout_handler.socket_name, socket_name_u) != 0) {
		rtpp_log_write(RTPP_LOG_ERR, spa->log, "invalid socket name %s", socket_name_u);
		socket_name_u = NULL;
	    } else {
		rtpp_log_write(RTPP_LOG_INFO, spa->log, "setting timeout handler");
		spa->timeout_data.handler = &cf->timeout_handler;
		spa->timeout_data.notify_tag = strdup(notify_tag);
	    }
	} else if (socket_name_u == NULL && spa->timeout_data.handler != NULL) {
	    spa->timeout_data.handler = NULL;
	    rtpp_log_write(RTPP_LOG_INFO, spa->log, "disabling timeout handler");
	}
    }

    if (ia[0] != NULL && ia[1] != NULL) {
        if (spa->addr[pidx] != NULL)
            spa->last_update[pidx] = dtime;
        if (spa->rtcp->addr[pidx] != NULL)
            spa->rtcp->last_update[pidx] = dtime;
	/*
	 * Unless the address provided by client historically
	 * cannot be trusted and address is different from one
	 * that we recorded update it.
	 */
	if (spa->untrusted_addr[pidx] == 0 && !(spa->addr[pidx] != NULL &&
	  SA_LEN(ia[0]) == SA_LEN(spa->addr[pidx]) &&
	  memcmp(ia[0], spa->addr[pidx], SA_LEN(ia[0])) == 0)) {
	    rtpp_log_write(RTPP_LOG_INFO, spa->log, "pre-filling %s's address "
	      "with %s:%s", (pidx == 0) ? "callee" : "caller", addr, port);
	    if (spa->addr[pidx] != NULL) {
	        if (spa->canupdate[pidx] == 0) {
	            if (spa->prev_addr[pidx] != NULL)
	                 free(spa->prev_addr[pidx]);
	            spa->prev_addr[pidx] = spa->addr[pidx];
	        } else {
		    free(spa->addr[pidx]);
		}
	    }
	    spa->addr[pidx] = ia[0];
	    ia[0] = NULL;
	}
	if (spa->rtcp->untrusted_addr[pidx] == 0 && !(spa->rtcp->addr[pidx] != NULL &&
	  SA_LEN(ia[1]) == SA_LEN(spa->rtcp->addr[pidx]) &&
	  memcmp(ia[1], spa->rtcp->addr[pidx], SA_LEN(ia[1])) == 0)) {
	    if (spa->rtcp->addr[pidx] != NULL) {
	        if (spa->rtcp->canupdate[pidx] == 0) {
	            if (spa->rtcp->prev_addr[pidx] != NULL)
	                free(spa->rtcp->prev_addr[pidx]);
	            spa->rtcp->prev_addr[pidx] = spa->rtcp->addr[pidx];
	        } else {
		    free(spa->rtcp->addr[pidx]);
		}
	    }
	    spa->rtcp->addr[pidx] = ia[1];
	    ia[1] = NULL;
	}
    }
    spa->asymmetric[pidx] = spa->rtcp->asymmetric[pidx] = asymmetric;
    spa->canupdate[pidx] = spa->rtcp->canupdate[pidx] = NOT(asymmetric);
    if (spa->codecs[pidx] != NULL) {
	free(spa->codecs[pidx]);
	spa->codecs[pidx] = NULL;
    }
    if (codecs != NULL)
	spa->codecs[pidx] = strdup(codecs);
    if (requested_nsamples > 0) {
	rtpp_log_write(RTPP_LOG_INFO, spa->log, "RTP packets from %s "
	  "will be resized to %d milliseconds",
	  (pidx == 0) ? "callee" : "caller", requested_nsamples / 8);
    } else if (spa->resizers[pidx].output_nsamples > 0) {
	  rtpp_log_write(RTPP_LOG_INFO, spa->log, "Resizing of RTP "
	  "packets from %s has been disabled",
	  (pidx == 0) ? "callee" : "caller");
    }
    spa->resizers[pidx].output_nsamples = requested_nsamples;

    for (i = 0; i < 2; i++)
	if (ia[i] != NULL)
	    free(ia[i]);

    assert(lport != 0);
    reply_port(cf, controlfd, &raddr, rlen, cookie, lport, lia);
    return 0;
}

static int
handle_delete(struct cfg *cf, char *call_id, char *from_tag, char *to_tag, int weak)
{
    int ndeleted;
    unsigned int medianum;
    struct rtpp_session *spa, *spb;
    int cmpr, cmpr1, idx;

    ndeleted = 0;
    for (spa = session_findfirst(cf, call_id); spa != NULL;) {
	medianum = 0;
	if ((cmpr1 = compare_session_tags(spa->tag, from_tag, &medianum)) != 0) {
	    idx = 1;
	    cmpr = cmpr1;
	} else if (to_tag != NULL &&
	  (cmpr1 = compare_session_tags(spa->tag, to_tag, &medianum)) != 0) {
	    idx = 0;
	    cmpr = cmpr1;
	} else {
	    spa = session_findnext(spa);
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
	    spa = session_findnext(spa);
	    continue;
	}
	rtpp_log_write(RTPP_LOG_INFO, spa->log,
	  "forcefully deleting session %u on ports %d/%d",
	   medianum, spa->ports[0], spa->ports[1]);
	/* Search forward before we do removal */
	spb = spa;
	spa = session_findnext(spa);
	remove_session(cf, spb);
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
handle_noplay(struct cfg *cf, struct rtpp_session *spa, int idx)
{

    if (spa->rtps[idx] != NULL) {
	rtp_server_free(spa->rtps[idx]);
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
  char *pname, int playcount)
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
	rtpp_log_write(RTPP_LOG_INFO, spa->log,
	  "%d times playing prompt %s codec %d", playcount, pname, n);
	if (spa->sridx == -1)
	    append_server(cf, spa);
	return 0;
    }
    rtpp_log_write(RTPP_LOG_ERR, spa->log, "can't create player");
    return -1;
}

static void
handle_copy(struct cfg *cf, struct rtpp_session *spa, int idx, char *rname)
{

    if (spa->rrcs[idx] == NULL) {
	spa->rrcs[idx] = ropen(cf, spa, rname, idx);
	rtpp_log_write(RTPP_LOG_INFO, spa->log,
	  "starting recording RTP session on port %d", spa->ports[idx]);
    }
    if (spa->rtcp->rrcs[idx] == NULL && cf->rrtcp != 0) {
	spa->rtcp->rrcs[idx] = ropen(cf, spa->rtcp, rname, idx);
	rtpp_log_write(RTPP_LOG_INFO, spa->log,
	  "starting recording RTCP session on port %d", spa->rtcp->ports[idx]);
    }
}

static int
handle_record(struct cfg *cf, char *call_id, char *from_tag, char *to_tag)
{
    int nrecorded, idx;
    struct rtpp_session *spa;

    nrecorded = 0;
    for (spa = session_findfirst(cf, call_id); spa != NULL;
      spa = session_findnext(spa)) {
	if (compare_session_tags(spa->tag, from_tag, NULL) != 0) {
	    idx = 1;
	} else if (to_tag != NULL &&
	  (compare_session_tags(spa->tag, to_tag, NULL)) != 0) {
	    idx = 0;
	} else {
	    continue;
	}
	nrecorded++;
	handle_copy(cf, spa, idx, NULL);
    }
    return (nrecorded == 0 ? -1 : 0);
}

static void
handle_query(struct cfg *cf, int fd, struct sockaddr_storage *raddr,
  socklen_t rlen, char *cookie, struct rtpp_session *spa, int idx)
{
    char buf[1024 * 8];
    int len;

    if (cookie != NULL) {
	len = sprintf(buf, "%s %d %lu %lu %lu %lu\n", cookie, get_ttl(spa),
	  spa->pcount[idx], spa->pcount[NOT(idx)], spa->pcount[2],
	  spa->pcount[3]);
    } else {
	len = sprintf(buf, "%d %lu %lu %lu %lu\n", get_ttl(spa),
	  spa->pcount[idx], spa->pcount[NOT(idx)], spa->pcount[2],
	  spa->pcount[3]);
    }
    doreply(cf, fd, buf, len, raddr, rlen);
}
