/*
 * Copyright (c) 2003 - 2005 Maxim Sobolev
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
 * $Id: main.c,v 1.42 2006/07/04 23:12:15 sobomax Exp $
 *
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <assert.h>
#if !defined(__solaris__)
#include <err.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <poll.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "rtp_server.h"
#include "rtpp_defines.h"
#include "rtpp_log.h"
#include "rtpp_record.h"
#include "rtpp_session.h"
#include "rtpp_util.h"

#define	GET_RTP(sp)	(((sp)->rtp != NULL) ? (sp)->rtp : (sp))
#define	NOT(x)		(((x) == 0) ? 1 : 0)

static struct rtpp_session *sessions[MAX_FDS + 1];
static struct rtpp_session *rtp_servers[MAX_FDS + 1];
static struct pollfd fds[MAX_FDS + 1];
static int nsessions;
static unsigned long long sessions_created = 0;
static int rtp_nsessions;
static int bmode = 0;			/* Bridge mode */
static int umode = 0;			/* UDP control mode */
static int max_ttl = SESSION_TIMEOUT;
static const char *cmd_sock = CMD_SOCK;
static const char *pid_file = PID_FILE;

struct proto_cap {
    const char	*pc_id;
    const char	*pc_description;
};

static struct proto_cap proto_caps[] = {
    /*
     * The first entry must be basic protocol version and isn't shown
     * as extension on -v.
     */
    { "20040107", "Basic RTP proxy functionality" },
    { "20050322", "Support for multiple RTP streams and MOH" },
    { "20060704", "Support for extra parameter in the V command" },
    { NULL, NULL }
};

/*
 * The first address is for external interface, the second one - for
 * internal one. Second can be NULL, in this case there is no bridge
 * mode enabled.
 */
static struct sockaddr *bindaddr[2];	/* RTP socket(s) addresses */

static rtpp_log_t glog;
static int tos = TOS;
static int lastport[2] = {PORT_MIN - 1, PORT_MIN - 1};
static const char *rdir = NULL;
static const char *sdir = NULL;
static int rrtcp = 1;

static void setbindhost(struct sockaddr *, int, const char *, const char *);
static void remove_session(struct rtpp_session *);
static void alarmhandler(int);
static int create_twinlistener(struct sockaddr *, int, int *);
static int create_listener(struct sockaddr *, int, int, int, int *, int *);
static int handle_command(int);
static void usage(void);

static void
setbindhost(struct sockaddr *ia, int pf, const char *bindhost,
  const char *servname)
{
    int n;

    /*
     * If user specified * then change it to NULL,
     * that will make getaddrinfo to return addr_any socket
     */
    if (bindhost && (strcmp(bindhost, "*") == 0))
	bindhost = NULL;

    if ((n = resolve(ia, pf, bindhost, servname, AI_PASSIVE)) != 0)
	errx(1, "setbindhost: %s", gai_strerror(n));
}

static void
append_session(struct rtpp_session *sp, int index)
{

    if (sp->fds[index] != -1) {
	sessions[nsessions] = sp;
	fds[nsessions].fd = sp->fds[index];
	fds[nsessions].events = POLLIN;
	fds[nsessions].revents = 0;
	sp->sidx[index] = nsessions;
	nsessions++;
    } else {
	sp->sidx[index] = -1;
    }
}

static void
append_server(struct rtpp_session *sp)
{

    if (sp->rtps[0] != NULL || sp->rtps[1] != NULL) {
        if (sp->sridx == -1) {
	    rtp_servers[rtp_nsessions] = sp;
	    sp->sridx = rtp_nsessions;
	    rtp_nsessions++;
	}
    } else {
        sp->sridx = -1;
    }
}

static void
alarmhandler(int sig __attribute__ ((unused)))
{
    struct rtpp_session *sp;
    int i;

    for(i = 1; i < nsessions; i++) {
	sp = sessions[i];
	if (sp == NULL || sp->rtcp == NULL || sp->sidx[0] != i)
	    continue;
	if (sp->ttl == 0) {
	    rtpp_log_write(RTPP_LOG_INFO, sp->log, "session timeout");
	    remove_session(sp);
	    continue;
	}
	sp->ttl--;
    }
}

static void
remove_session(struct rtpp_session *sp)
{
    int i;

    rtpp_log_write(RTPP_LOG_INFO, sp->log, "RTP stats: %lu in from callee, %lu "
      "in from caller, %lu relayed, %lu dropped", sp->pcount[0], sp->pcount[1],
      sp->pcount[2], sp->pcount[3]);
    rtpp_log_write(RTPP_LOG_INFO, sp->log, "RTCP stats: %lu in from callee, %lu "
      "in from caller, %lu relayed, %lu dropped", sp->rtcp->pcount[0],
      sp->rtcp->pcount[1], sp->rtcp->pcount[2], sp->rtcp->pcount[3]);
    rtpp_log_write(RTPP_LOG_INFO, sp->log, "session on ports %d/%d is cleaned up",
      sp->ports[0], sp->ports[1]);
    for (i = 0; i < 2; i++) {
	if (sp->addr[i] != NULL)
	    free(sp->addr[i]);
	if (sp->rtcp->addr[i] != NULL)
	    free(sp->rtcp->addr[i]);
	if (sp->fds[i] != -1) {
	    close(sp->fds[i]);
	    assert(sessions[sp->sidx[i]] == sp);
	    sessions[sp->sidx[i]] = NULL;
	    assert(fds[sp->sidx[i]].fd == sp->fds[i]);
	    fds[sp->sidx[i]].fd = -1;
	    fds[sp->sidx[i]].events = 0;
	}
	if (sp->rtcp->fds[i] != -1) {
	    close(sp->rtcp->fds[i]);
	    assert(sessions[sp->rtcp->sidx[i]] == sp->rtcp);
	    sessions[sp->rtcp->sidx[i]] = NULL;
	    assert(fds[sp->rtcp->sidx[i]].fd == sp->rtcp->fds[i]);
	    fds[sp->rtcp->sidx[i]].fd = -1;
	    fds[sp->rtcp->sidx[i]].events = 0;
	}
	if (sp->rrcs[i] != NULL)
	    rclose(sp, sp->rrcs[i]);
	if (sp->rtcp->rrcs[i] != NULL)
	    rclose(sp, sp->rtcp->rrcs[i]);
	if (sp->rtps[i] != NULL) {
	    rtp_servers[sp->sridx] = NULL;
	    rtp_server_free(sp->rtps[i]);
	}
    }
    if (sp->call_id != NULL)
	free(sp->call_id);
    if (sp->tag != NULL)
	free(sp->tag);
    rtpp_log_close(sp->log);
    free(sp->rtcp);
    free(sp);
}

static int
create_twinlistener(struct sockaddr *ia, int port, int *fds)
{
    struct sockaddr_storage iac;
    int rval, i, flags;

    fds[0] = fds[1] = -1;

    rval = -1;
    for (i = 0; i < 2; i++) {
	fds[i] = socket(ia->sa_family, SOCK_DGRAM, 0);
	if (fds[i] == -1) {
	    rtpp_log_ewrite(RTPP_LOG_ERR, glog, "can't create %s socket",
	      (ia->sa_family == AF_INET) ? "IPv4" : "IPv6");
	    goto failure;
	}
	memcpy(&iac, ia, SA_LEN(ia));
	satosin(&iac)->sin_port = htons(port);
	if (bind(fds[i], sstosa(&iac), SA_LEN(ia)) != 0) {
	    if (errno != EADDRINUSE && errno != EACCES) {
		rtpp_log_ewrite(RTPP_LOG_ERR, glog, "can't bind to the %s port %d",
		  (ia->sa_family == AF_INET) ? "IPv4" : "IPv6", port);
	    } else {
		rval = -2;
	    }
	    goto failure;
	}
	port++;
	if ((ia->sa_family == AF_INET) &&
	  (setsockopt(fds[i], IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) == -1))
	    rtpp_log_ewrite(RTPP_LOG_ERR, glog, "unable to set TOS to %d", tos);
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
create_listener(struct sockaddr *ia, int minport, int maxport,
  int startport, int *port, int *fds)
{
    int i, init, rval;

    /* make sure that {min,max,start}port is even */
    if ((minport & 0x1) != 0)
	minport++;
    if ((maxport & 0x1) != 0)
	maxport--;
    if ((startport & 0x1) != 0)
	startport++;

    for (i = 0; i < 2; i++)
	fds[i] = -1;

    init = 0;
    if (startport < minport || startport > maxport)
	startport = minport;
    for (*port = startport; *port != startport || init == 0; (*port) += 2) {
	init = 1;
	rval = create_twinlistener(ia, *port, fds);
	if (rval != 0) {
	    if (rval == -1)
		break;
	    if (*port >= maxport)
		*port = minport - 2;
	    continue;
	}
	return 0;
    }
    return -1;
}

static int
compare_session_tags(char *tag1, char *tag0, unsigned *medianum_p)
{
    size_t len0 = strlen(tag0);
    if (!strncmp(tag1, tag0, len0)) {
	if (tag1[len0] == ';') {
		if (medianum_p != 0)
			*medianum_p = strtoul(tag1+len0+1, NULL, 10);
		return 2;
	}
	if (tag1[len0] == 0) return 1;
	return 0;
    }
    return 0;
}

static int
handle_command(int controlfd)
{
    int len, delete, argc, i, j, pidx, request, response, asymmetric;
    int external, pf, ecode, lidx, play, record, noplay, weak;
    int ndeleted, cmpr, cmpr1;
    int fds[2], lport, n;
    socklen_t rlen;
    unsigned medianum;
    char buf[1024 * 8];
    char *cp, *call_id, *from_tag, *to_tag, *addr, *port, *cookie;
    char *pname, *codecs;
    struct rtpp_session *spa, *spb;
    char **ap, *argv[10];
    const char *rname;
    struct sockaddr *ia[2], *lia[2];
    struct sockaddr_storage raddr;

#define	doreply() \
    { \
	buf[len] = '\0'; \
	rtpp_log_write(RTPP_LOG_DBUG, glog, "sending reply \"%s\"", buf); \
	if (umode == 0) { \
	    write(controlfd, buf, len); \
	} else { \
	    while (sendto(controlfd, buf, len, 0, sstosa(&raddr), \
	      rlen) == -1 && errno == ENOBUFS); \
	} \
    }

    ia[0] = ia[1] = NULL;
    spa = spb = NULL;
    lia[0] = lia[1] = bindaddr[0];
    lidx = 1;
    fds[0] = fds[1] = -1;

    if (umode == 0) {
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
	    rtpp_log_ewrite(RTPP_LOG_ERR, glog, "can't read from control socket");
	return -1;
    }
    buf[len] = '\0';

    rtpp_log_write(RTPP_LOG_DBUG, glog, "received command \"%s\"", buf);

    cp = buf;
    argc = 0;
    memset(argv, 0, sizeof(argv));
    for (ap = argv; (*ap = strsep(&cp, "\r\n\t ")) != NULL;)
	if (**ap != '\0') {
	    argc++;
	    if (++ap >= &argv[10])
		break;
	}
    cookie = NULL;
    if (argc < 1 || (umode != 0 && argc < 2)) {
	rtpp_log_write(RTPP_LOG_ERR, glog, "command syntax error");
	ecode = 0;
	goto goterror;
    }

    /* Stream communication mode doesn't use cookie */
    if (umode != 0) {
	cookie = argv[0];
	for (i = 1; i < argc; i++)
	    argv[i - 1] = argv[i];
	argc--;
	argv[argc] = NULL;
    } else {
	cookie = NULL;
    }

    request = response = delete = play = record = noplay = 0;
    addr = port = NULL;
    switch (argv[0][0]) {
    case 'u':
    case 'U':
	request = 1;
	break;

    case 'l':
    case 'L':
	response = 1;
	break;

    case 'd':
    case 'D':
	delete = 1;
	break;

    case 'p':
    case 'P':
	/* P callid pname codecs from_tag to_tag */
	play = 1;
	pname = argv[2];
	codecs = argv[3];
	break;

    case 'r':
    case 'R':
        record = 1;
        break;

    case 's':
    case 'S':
        noplay = 1;
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
		rtpp_log_write(RTPP_LOG_ERR, glog, "command syntax error");
		ecode = 2;
		goto goterror;
	    }
	    for (known = i = 0; proto_caps[i].pc_id != NULL; ++i) {
		if (!strcmp(argv[1], proto_caps[i].pc_id)) {
		    known = 1;
		    break;
		}
	    }
	    if (cookie == NULL)
		len = sprintf(buf, "%d\n", known);
	    else
		len = sprintf(buf, "%s %d\n", cookie, known);
	    goto doreply;
	}
	if (argc != 1 && argc != 2) {
	    rtpp_log_write(RTPP_LOG_ERR, glog, "command syntax error");
	    ecode = 2;
	    goto goterror;
	}
	/* This returns base version. */
	if (cookie == NULL)
	    len = sprintf(buf, "%d\n", CPROTOVER);
	else
	    len = sprintf(buf, "%s %d\n", cookie, CPROTOVER);
	goto doreply;
	break;

    case 'i':
    case 'I':
	len = sprintf(buf, "sessions created: %llu\nactive sessions:\n", sessions_created);
	for (i = 1; i < nsessions; i++) {
	    char addrs[4][256];

            spa = sessions[i];
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
	      "stats = %lu/%lu/%lu/%lu, ttl = %d\n",
	      spb->call_id, spb->tag, addrs[0], spb->ports[1], addrs[1],
	      addrs[2], spb->ports[0], addrs[3], spa->pcount[0], spa->pcount[1],
	      spa->pcount[2], spa->pcount[3], spb->ttl);
	    if (len + 512 > sizeof(buf)) {
		doreply();
		len = 0;
	    }
	}
	if (len > 0)
	    doreply();
	return 0;
	break;

    default:
	rtpp_log_write(RTPP_LOG_ERR, glog, "unknown command");
	ecode = 3;
	goto goterror;
    }
    call_id = argv[1];
    if (request != 0 || response != 0 || play != 0) {
	if (argc < 5 || argc > 6) {
	    rtpp_log_write(RTPP_LOG_ERR, glog, "command syntax error");
	    ecode = 4;
	    goto goterror;
	}
	from_tag = argv[4];
	to_tag = argv[5];
	if (play != 0 && argv[0][1] != '\0')
	    play = atoi(argv[0] + 1);
    }
    if (delete != 0 || record != 0 || noplay != 0) {
	if (argc < 3 || argc > 4) {
	    rtpp_log_write(RTPP_LOG_ERR, glog, "command syntax error");
	    ecode = 1;
	    goto goterror;
	}
	from_tag = argv[2];
	to_tag = argv[3];
    }
    if (request != 0 || response != 0 || delete != 0) {
	addr = argv[2];
	port = argv[3];
	/* Process additional options */
	external = 1;
	/* In bridge mode all clients are assumed to be asymmetric */
	asymmetric = (bmode != 0) ? 1 : 0;
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
		    rtpp_log_write(RTPP_LOG_ERR, glog, "command syntax error");
		    ecode = 1;
		    goto goterror;
		}
		lia[lidx] = bindaddr[1];
		lidx--;
		break;

	    case 'i':
	    case 'I':
		if (lidx < 0) {
		    rtpp_log_write(RTPP_LOG_ERR, glog, "command syntax error");
		    ecode = 1;
		    goto goterror;
		}
		lia[lidx] = bindaddr[0];
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

	    default:
		rtpp_log_write(RTPP_LOG_ERR, glog, "unknown command modifier `%c'",
		  *cp);
		break;
	    }
	}
	if (delete == 0 && addr != NULL && port != NULL && strlen(addr) >= 7) {
	    struct sockaddr_storage tia;

	    if ((n = resolve(sstosa(&tia), pf, addr, port,
	      AI_NUMERICHOST)) == 0) {
		if (!ishostnull(sstosa(&tia))) {
		    for (i = 0; i < 2; i++) {
			ia[i] = malloc(SS_LEN(&tia));
			if (ia[i] == NULL) {
			    ecode = 5;
			    goto nomem;
			}
			memcpy(ia[i], &tia, SS_LEN(&tia));
		    }
		    /* Set port for RTCP, will work both for IPv4 and IPv6 */
		    n = ntohs(satosin(ia[1])->sin_port);
		    satosin(ia[1])->sin_port = htons(n + 1);
		}
	    } else {
		rtpp_log_write(RTPP_LOG_ERR, glog, "getaddrinfo: %s",
		  gai_strerror(n));
	    }
	}
    }

    lport = 0;
    pidx = 1;
    ndeleted = 0;
    for (i = 1; i < nsessions; i++) {
        spa = sessions[i];
	if (spa == NULL || spa->sidx[0] != i || spa->rtcp == NULL ||
	  spa->call_id == NULL || strcmp(spa->call_id, call_id) != 0)
	    continue;
	medianum = 0;
	if ((cmpr1 = compare_session_tags(spa->tag, from_tag, &medianum)) != 0)
	{
	    i = (request == 0) ? 1 : 0;
	    cmpr = cmpr1;
	} else if (to_tag != NULL &&
	  (cmpr1 = compare_session_tags(spa->tag, to_tag, &medianum)) != 0)
	{
	    i = (request == 0) ? 0 : 1;
	    cmpr = cmpr1;
	} else
	    continue;

	if (delete != 0) {
	    if (weak)
		spa->weak[i] = 0;
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
		    weak ? ( i ? "weak[1]" : "weak[0]" ) : "strong",
		    spa->strong, spa->weak[0], spa->weak[1]);
		/* Skipping to next possible stream for this call */
		++ndeleted;
		continue;
	    }
	    rtpp_log_write(RTPP_LOG_INFO, spa->log,
	      "forcefully deleting session %d on ports %d/%d",
	      medianum, spa->ports[0], spa->ports[1]);
	    remove_session(spa);
	    if (cmpr == 2) {
		++ndeleted;
		continue;
	    }
	    goto do_ok;
	}

	if (play != 0 || noplay != 0) {
	    if (spa->rtps[i] != NULL) {
		rtp_server_free(spa->rtps[i]);
		spa->rtps[i] = NULL;
		rtpp_log_write(RTPP_LOG_INFO, spa->log,
	          "stopping player at port %d", spa->ports[i]);
	        if (spa->rtps[0] == NULL && spa->rtps[1] == NULL) {
	            assert(rtp_servers[spa->sridx] == spa);
	            rtp_servers[spa->sridx] = NULL;
	            spa->sridx = -1;
	        }
	    }
	    if (play == 0)
		goto do_ok;
	}

	if (play != 0) {
	    while (*codecs != '\0') {
		n = strtol(codecs, &cp, 10);
		if (cp == codecs)
		    break;
		codecs = cp;
		if (*codecs != '\0')
		    codecs++;
		spa->rtps[i] = rtp_server_new(pname, n, play);
		if (spa->rtps[i] == NULL)
		    continue;
		rtpp_log_write(RTPP_LOG_INFO, spa->log,
		  "%d times playing prompt %s codec %d", play, pname, n);
		if (spa->sridx == -1)
		    append_server(spa);
		goto do_ok;
	    }
	    rtpp_log_write(RTPP_LOG_ERR, spa->log, "can't create player");
	    ecode = 6;
	    goto goterror;
	}

	if (record != 0) {
	    if (rdir != NULL) {
	        if (spa->rrcs[i] == NULL) {
		    spa->rrcs[i] = ropen(spa, rdir, sdir, i);
	            rtpp_log_write(RTPP_LOG_INFO, spa->log,
	              "starting recording RTP session on port %d", spa->ports[i]);
	        }
	        if (spa->rtcp->rrcs[i] == NULL && rrtcp != 0) {
		    spa->rtcp->rrcs[i] = ropen(spa->rtcp, rdir, sdir, i);
	            rtpp_log_write(RTPP_LOG_INFO, spa->log,
	              "starting recording RTCP session on port %d", spa->rtcp->ports[i]);
	        }
	    }
	    goto do_ok;
	}

	if (spa->fds[i] == -1) {
	    j = ishostseq(bindaddr[0], spa->laddr[i]) ? 0 : 1;
	    if (create_listener(spa->laddr[i], PORT_MIN, PORT_MAX,
	      lastport[j], &lport, fds) == -1) {
		rtpp_log_write(RTPP_LOG_ERR, spa->log, "can't create listener");
		ecode = 7;
		goto goterror;
	    }
	    lastport[j] = lport + 1;
	    assert(spa->fds[i] == -1);
	    spa->fds[i] = fds[0];
	    assert(spa->rtcp->fds[i] == -1);
	    spa->rtcp->fds[i] = fds[1];
	    spa->ports[i] = lport;
	    spa->rtcp->ports[i] = lport + 1;
	    spa->complete = spa->rtcp->complete = 1;
	    append_session(spa, i);
	    append_session(spa->rtcp, i);
	}
	if (weak)
	    spa->weak[i] = 1;
	else if (response == 0)
	    spa->strong = 1;
	lport = spa->ports[i];
	lia[0] = spa->laddr[i];
	pidx = (i == 0) ? 1 : 0;
	spa->ttl = max_ttl;
	if (response == 0) {
		rtpp_log_write(RTPP_LOG_INFO, spa->log,
		  "adding %s flag to existing session, new=%d/%d/%d",
		  weak ? ( i ? "weak[1]" : "weak[0]" ) : "strong",
		  spa->strong, spa->weak[0], spa->weak[1]);
	}
	rtpp_log_write(RTPP_LOG_INFO, spa->log,
	  "lookup on ports %d/%d, session timer restarted", spa->ports[0],
	  spa->ports[1]);
	goto writeport;
    }
    if (delete != 0 && ndeleted != 0) {
	/*
	 * Multiple stream deleting stops here because we had to
	 * iterate full list.
	 */
	goto do_ok;
    }
    rname = NULL;
    if (delete != 0)
        rname = "delete";
    if (play != 0)
        rname = "play";
    if (noplay != 0)
	rname = "noplay";
    if (record != 0)
        rname = "record";
    if (response != 0)
        rname = "lookup";
    if (rname != NULL) {
	rtpp_log_write(RTPP_LOG_INFO, glog,
	  "%s request failed: session %s, tags %s/%s not found", rname,
	  call_id, from_tag, to_tag != NULL ? to_tag : "NONE");
	if (response != 0) {
	    pidx = -1;
	    goto writeport;
	}
	ecode = 8;
	goto goterror;
    }

    rtpp_log_write(RTPP_LOG_INFO, glog,
	"new session %s, tag %s requested, type %s",
	call_id, from_tag, weak ? "weak" : "strong");

    j = ishostseq(bindaddr[0], lia[0]) ? 0 : 1;
    if (create_listener(bindaddr[j], PORT_MIN, PORT_MAX,
      lastport[j], &lport, fds) == -1) {
	rtpp_log_write(RTPP_LOG_ERR, glog, "can't create listener");
	ecode = 10;
	goto goterror;
    }
    lastport[j] = lport + 1;

    /* Session creation. If creation is requested with weak flag,
     * set weak[0].
     */
    spa = malloc(sizeof(*spa));
    if (spa == NULL) {
    	ecode = 11;
	goto nomem;
    }
    /* spb is RTCP twin session for this one. */
    spb = malloc(sizeof(*spb));
    if (spb == NULL) {
	ecode = 12;
	goto nomem;
    }
    memset(spa, 0, sizeof(*spa));
    memset(spb, 0, sizeof(*spb));
    for (i = 0; i < 2; i++)
	spa->fds[i] = spb->fds[i] = -1;
    spa->call_id = strdup(call_id);
    if (spa->call_id == NULL) {
	ecode = 13;
	goto nomem;
    }
    spb->call_id = spa->call_id;
    spa->tag = strdup(from_tag);
    if (spa->tag == NULL) {
	ecode = 14;
	goto nomem;
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
    spa->ttl = max_ttl;
    spb->ttl = -1;
    spa->log = rtpp_log_open("rtpproxy", spa->call_id, 0);
    spb->log = spa->log;
    spa->rtcp = spb;
    spb->rtcp = NULL;
    spa->rtp = NULL;
    spb->rtp = spa;
    spa->sridx = spb->sridx = -1;

    append_session(spa, 0);
    append_session(spa, 1);
    append_session(spb, 0);
    append_session(spb, 1);

    sessions_created++;

    rtpp_log_write(RTPP_LOG_INFO, spa->log, "new session on a port %d created, "
      "tag %s", lport, from_tag);

writeport:
    if (pidx >= 0) {
	if (ia[0] != NULL && ia[1] != NULL) {
	    /* If address is different from one that recorded update it */
	    if (!(spa->addr[pidx] != NULL &&
	      SA_LEN(ia[0]) == SA_LEN(spa->addr[pidx]) &&
	      memcmp(ia[0], spa->addr[pidx], SA_LEN(ia[0])) == 0)) {
		rtpp_log_write(RTPP_LOG_INFO, spa->log, "pre-filling %s's address "
		  "with %s:%s", (pidx == 0) ? "callee" : "caller", addr, port);
		if (spa->addr[pidx] != NULL)
		    free(spa->addr[pidx]);
		spa->addr[pidx] = ia[0];
		ia[0] = NULL;
	    }
	    if (!(spa->rtcp->addr[pidx] != NULL &&
	      SA_LEN(ia[1]) == SA_LEN(spa->rtcp->addr[pidx]) &&
	      memcmp(ia[1], spa->rtcp->addr[pidx], SA_LEN(ia[1])) == 0)) {
		if (spa->rtcp->addr[pidx] != NULL)
		    free(spa->rtcp->addr[pidx]);
		spa->rtcp->addr[pidx] = ia[1];
		ia[1] = NULL;
	    }
	}
	spa->asymmetric[pidx] = spa->rtcp->asymmetric[pidx] = asymmetric;
	spa->canupdate[pidx] = spa->rtcp->canupdate[pidx] = NOT(asymmetric);
    }
    for (i = 0; i < 2; i++)
	if (ia[i] != NULL)
	    free(ia[i]);
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
doreply:
    doreply();
    return 0;

nomem:
    rtpp_log_write(RTPP_LOG_ERR, glog, "can't allocate memory");
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
goterror:
    if (cookie != NULL)
	len = sprintf(buf, "%s E%d\n", cookie, ecode);
    else
	len = sprintf(buf, "E%d\n", ecode);
    goto doreply;
do_ok:
    if (cookie != NULL)
	len = sprintf(buf, "%s 0\n", cookie);
    else {
	strcpy(buf, "0\n");
	len = 2;
    }
    goto doreply;
    return 0;
}

static void
usage(void)
{

    fprintf(stderr, "usage: rtpproxy [-2fv] [-l addr1[/addr2]] "
      "[-6 addr1[/addr2]] [-s path] [-t tos] [-r rdir [-S sdir]] [-T ttl] [-L nfiles]\n");
    exit(1);
}

static void
fatsignal(int sig)
{

    rtpp_log_write(RTPP_LOG_INFO, glog, "got signal %d", sig);
    exit(0);
}

static void
ehandler(void)
{

    unlink(cmd_sock);
    unlink(pid_file);
    rtpp_log_write(RTPP_LOG_INFO, glog, "rtpproxy ended");
    rtpp_log_close(glog);
}

int
main(int argc, char **argv)
{
    int controlfd, i, j, k, readyfd, len, nodaemon, dmode, port, ridx, sidx;
    int timeout, flags, skipfd;
    sigset_t set, oset;
    struct rtpp_session *sp;
    struct sockaddr_un ifsun;
    struct sockaddr_storage ifsin, raddr;
    socklen_t rlen;
    struct itimerval tick;
    char buf[1024 * 8];
    char ch, *bh[2], *bh6[2], *cp;
    double sptime, eptime;
    unsigned long delay;
    struct rlimit lim;

    bh[0] = bh[1] = bh6[0] = bh6[1] = NULL;
    nodaemon = 0;

    dmode = 0;

    while ((ch = getopt(argc, argv, "vf2Rl:6:s:S:t:r:p:T:L:")) != -1)
	switch (ch) {
	case 'f':
	    nodaemon = 1;
	    break;

	case 'l':
	    bh[0] = optarg;
	    bh[1] = strchr(bh[0], '/');
	    if (bh[1] != NULL) {
		*bh[1] = '\0';
		bh[1]++;
		bmode = 1;
	    }
	    break;

	case '6':
	    bh6[0] = optarg;
	    bh6[1] = strchr(bh6[0], '/');
	    if (bh6[1] != NULL) {
		*bh6[1] = '\0';
		bh6[1]++;
		bmode = 1;
	    }
	    break;

	case 's':
	    if (strncmp("udp:", optarg, 4) == 0) {
		umode = 1;
		optarg += 4;
	    } else if (strncmp("udp6:", optarg, 5) == 0) {
		umode = 6;
		optarg += 5;
	    } else if (strncmp("unix:", optarg, 5) == 0) {
		umode = 0;
		optarg += 5;
	    }
	    cmd_sock = optarg;
	    break;

	case 't':
	    tos = atoi(optarg);
	    break;

	case '2':
	    dmode = 1;
	    break;

	case 'v':
	    printf("Basic version: %d\n", CPROTOVER);
	    for(i = 1; proto_caps[i].pc_id != NULL; ++i) {
		printf("Extension %s: %s\n", proto_caps[i].pc_id,
		    proto_caps[i].pc_description);
	    }
	    exit(0);
	    break;

	case 'r':
	    rdir = optarg;
	    break;

	case 'S':
	    sdir = optarg;
	    break;

	case 'R':
	    rrtcp = 0;
	    break;

	case 'p':
	    pid_file = optarg;
	    break;

	case 'T':
	    max_ttl = atoi(optarg);
	    break;

	case 'L':
	    lim.rlim_cur = lim.rlim_max = atoi(optarg);
	    if (setrlimit(RLIMIT_NOFILE, &lim) != 0)
	        err(1, "setrlimit");
	    break;

	case '?':
	default:
	    usage();
	}
    argc -= optind;
    argv += optind;

    if (rdir == NULL && sdir != NULL)
        errx(1, "-S switch requires -r switch");

    if (bh[0] == NULL && bh[1] == NULL && bh6[0] == NULL && bh6[1] == NULL) {
	if (umode != 0)
	    errx(1, "explicit binding address has to be specified in UDP "
	      "command mode");
	bh[0] = "*";
    }

    for (i = 0; i < 2; i++) {
	if (bh[i] != NULL && *bh[i] == '\0')
	    bh[i] = NULL;
	if (bh6[i] != NULL && *bh6[i] == '\0')
	    bh6[i] = NULL;
    }

    i = ((bh[0] == NULL) ? 0 : 1) + ((bh[1] == NULL) ? 0 : 1) +
      ((bh6[0] == NULL) ? 0 : 1) + ((bh6[1] == NULL) ? 0 : 1);
    if (bmode != 0) {
	if (bh[0] != NULL && bh6[0] != NULL)
	    errx(1, "either IPv4 or IPv6 should be configured for external "
	      "interface in bridging mode, not both");
	if (bh[1] != NULL && bh6[1] != NULL)
	    errx(1, "either IPv4 or IPv6 should be configured for internal "
	      "interface in bridging mode, not both");
	if (i != 2)
	    errx(1, "incomplete configuration of the bridging mode - exactly "
	      "2 listen addresses required, %d provided", i);
    } else if (i != 1) {
	errx(1, "exactly 1 listen addresses required, %d provided", i);
    }

    for (i = 0; i < 2; i++) {
	bindaddr[i] = NULL;
	if (bh[i] != NULL) {
	    bindaddr[i] = alloca(sizeof(struct sockaddr_storage));
	    setbindhost(bindaddr[i], AF_INET, bh[i], SERVICE);
	    continue;
	}
	if (bh6[i] != NULL) {
	    bindaddr[i] = alloca(sizeof(struct sockaddr_storage));
	    setbindhost(bindaddr[i], AF_INET6, bh6[i], SERVICE);
	    continue;
	}
    }
    if (bindaddr[0] == NULL) {
	bindaddr[0] = bindaddr[1];
	bindaddr[1] = NULL;
    }

    if (umode == 0) {
	unlink(cmd_sock);
	memset(&ifsun, '\0', sizeof ifsun);
#if !defined(__linux__) && !defined(__solaris__)
	ifsun.sun_len = strlen(cmd_sock);
#endif
	ifsun.sun_family = AF_LOCAL;
	strcpy(ifsun.sun_path, cmd_sock);
	controlfd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (controlfd == -1)
	    err(1, "can't create socket");
	setsockopt(controlfd, SOL_SOCKET, SO_REUSEADDR, &controlfd,
	  sizeof controlfd);
	if (bind(controlfd, sstosa(&ifsun), sizeof ifsun) < 0)
	    err(1, "can't bind to a socket");
	if (listen(controlfd, 32) != 0)
	    err(1, "can't listen on a socket");
    } else {
	cp = strrchr(cmd_sock, ':');
	if (cp != NULL) {
	    *cp = '\0';
	    cp++;
	}
	if (cp == NULL || *cp == '\0')
	    cp = CPORT;
	i = (umode == 6) ? AF_INET6 : AF_INET;
	setbindhost(sstosa(&ifsin), i, cmd_sock, cp);
	controlfd = socket(i, SOCK_DGRAM, 0);
	if (controlfd == -1)
	    err(1, "can't create socket");
	if (bind(controlfd, sstosa(&ifsin), SS_LEN(&ifsin)) < 0)
	    err(1, "can't bind to a socket");
    }
    flags = fcntl(controlfd, F_GETFL);
    fcntl(controlfd, F_SETFL, flags | O_NONBLOCK);

#if !defined(__solaris__)
    if (nodaemon == 0) {
	if (daemon(0, 0) == -1)
	    err(1, "can't switch into daemon mode");
	    /* NOTREACHED */
	for (i = 0; i < (int)FD_SETSIZE; i++)
	    if (i != controlfd)
		close(i);
    }
#endif

    atexit(ehandler);
    glog = rtpp_log_open("rtpproxy", NULL, LF_REOPEN);
    rtpp_log_write(RTPP_LOG_INFO, glog, "rtpproxy started, pid %d", getpid());

    i = open(pid_file, O_WRONLY | O_CREAT | O_TRUNC, DEFFILEMODE);
    if (i >= 0) {
	len = sprintf(buf, "%u\n", getpid());
	write(i, buf, len);
	close(i);
    } else {
	rtpp_log_ewrite(RTPP_LOG_ERR, glog, "can't open pidfile for writing");
    }

    signal(SIGHUP, fatsignal);
    signal(SIGINT, fatsignal);
    signal(SIGKILL, fatsignal);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, fatsignal);
    signal(SIGXCPU, fatsignal);
    signal(SIGXFSZ, fatsignal);
    signal(SIGVTALRM, fatsignal);
    signal(SIGPROF, fatsignal);
    signal(SIGUSR1, fatsignal);
    signal(SIGUSR2, fatsignal);

    fds[0].fd = controlfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    sessions[0] = NULL;
    nsessions = 1;
    rtp_nsessions = 0;

    memset(&tick, 0, sizeof(tick));
    tick.it_interval.tv_sec = TIMETICK;
    tick.it_value.tv_sec = TIMETICK;
    signal(SIGALRM, SIG_IGN);
    setitimer(ITIMER_REAL, &tick, NULL);
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);

    signal(SIGALRM, alarmhandler);

    sptime = 0;
    while(1) {
	if (rtp_nsessions > 0)
	    timeout = RTPS_TICKS_MIN;
	else
	    timeout = INFTIM;
	sigprocmask(SIG_UNBLOCK, &set, &oset);
	eptime = getctime();
	delay = (eptime - sptime) * 1000000.0;
	if (delay < (1000000 / POLL_LIMIT)) {
	    usleep((1000000 / POLL_LIMIT) - delay);
	    sptime = getctime();
	} else {
	    sptime = eptime;
	}
	i = poll(fds, nsessions, timeout);
	if (i < 0 && errno == EINTR)
	    continue;
	sigprocmask(SIG_BLOCK, &set, &oset);
	if (rtp_nsessions > 0) {
	    skipfd = 0;
	    for (j = 0; j < rtp_nsessions; j++) {
		sp = rtp_servers[j];
		if (sp == NULL) {
		    skipfd++;
		    continue;
		}
		if (skipfd > 0) {
		    rtp_servers[j - skipfd] = rtp_servers[j];
		    sp->sridx = j - skipfd;
		}
		for (sidx = 0; sidx < 2; sidx++) {
		    if (sp->rtps[sidx] == NULL || sp->addr[sidx] == NULL)
			continue;
		    while ((len = rtp_server_get(sp->rtps[sidx])) != RTPS_LATER) {
			if (len == RTPS_EOF) {
			    rtp_server_free(sp->rtps[sidx]);
			    sp->rtps[sidx] = NULL;
			    if (sp->rtps[0] == NULL && sp->rtps[1] == NULL) {
			        assert(rtp_servers[sp->sridx] == sp);
			        rtp_servers[sp->sridx] = NULL;
			        sp->sridx = -1;
			    }
			    break;
			}
			for (k = (dmode && len < LBR_THRS) ? 2 : 1; k > 0; k--) {
			    sendto(sp->fds[sidx], sp->rtps[sidx]->buf, len, 0,
			      sp->addr[sidx], SA_LEN(sp->addr[sidx]));
			}
		    }
		}
	    }
	    rtp_nsessions -= skipfd;
	    if (i == 0)
		continue;
	}
	skipfd = 0;
	for (readyfd = 0; readyfd < nsessions; readyfd++) {
	    if (readyfd > 0) {
	        if (fds[readyfd].fd == -1) {
	            skipfd++;
	            continue;
	        }
	        sp = sessions[readyfd];
	        for (ridx = 0; ridx < 2; ridx++)
		    if (fds[readyfd].fd == sp->fds[ridx])
		        break;
	        /*
	         * Can't happen.
	         */
	        assert(ridx != 2);

	        if (skipfd > 0) {
	            fds[readyfd - skipfd] = fds[readyfd];
	            sessions[readyfd - skipfd] = sessions[readyfd];
	            sp->sidx[ridx] = readyfd - skipfd;;
	        }
	    }
	    if ((fds[readyfd].revents & POLLIN) == 0)
		continue;
	    if (readyfd == 0) {
	        do {
		    if (umode == 0) {
		        rlen = sizeof(ifsun);
		        controlfd = accept(fds[readyfd].fd,
		          sstosa(&ifsun), &rlen);
		        if (controlfd == -1) {
		            if (errno != EWOULDBLOCK)
			        rtpp_log_ewrite(RTPP_LOG_ERR, glog,
			          "can't accept connection on control socket");
			    break;
		        }
		    } else {
		        controlfd = fds[readyfd].fd;
		    }
		    i = handle_command(controlfd);
		    if (umode == 0) {
		        close(controlfd);
		    }
		} while (i == 0);
		continue;
	    }
drain:
	    rlen = sizeof(raddr);
	    len = recvfrom(sp->fds[ridx], buf, sizeof(buf), 0,
	      sstosa(&raddr), &rlen);
	    if (len <= 0)
		continue;

	    if (sp->complete == 0)
		continue;

	    i = 0;
	    if (sp->addr[ridx] != NULL) {
		/* Check that the packet is authentic, drop if it isn't */
		if (sp->asymmetric[ridx] == 0) {
			if (memcmp(sp->addr[ridx], &raddr, rlen) != 0) {
			    if (sp->canupdate[ridx] == 0)
				continue;
			    /* Signal that an address have to be updated */
			    i = 1;
			}
		} else {
		    /*
		     * For asymmetric clients don't check
		     * source port since it may be different.
		     */
		    if (!ishostseq(sp->addr[ridx], sstosa(&raddr)))
			continue;
		}
		sp->pcount[ridx]++;
	    } else {
		sp->pcount[ridx]++;
		sp->addr[ridx] = malloc(rlen);
		if (sp->addr[ridx] == NULL) {
		    sp->pcount[3]++;
		    rtpp_log_write(RTPP_LOG_ERR, sp->log,
		      "can't allocate memory for remote address - "
		      "removing session");
		    remove_session(GET_RTP(sp));
		    continue;
		}
		/* Signal that an address have to be updated. */
		i = 1;
	    }

	    /* Update recorded address if it's necessary. */
	    if (i != 0) {
		memcpy(sp->addr[ridx], &raddr, rlen);
		sp->canupdate[ridx] = 0;

		port = ntohs(satosin(&raddr)->sin_port);

		rtpp_log_write(RTPP_LOG_INFO, sp->log,
		  "%s's address filled in: %s:%d (%s)",
		  (ridx == 0) ? "callee" : "caller",
		  addr2char(sstosa(&raddr)), port,
		  (sp->rtp == NULL) ? "RTP" : "RTCP");

		/*
		 * Check if we have updated RTP while RTCP is still
		 * empty or contains address that differs from one we
		 * used when updating RTP. Try to guess RTCP if so,
		 * should be handy for non-NAT'ed clients, and some
		 * NATed as well.
		 */
		if (sp->rtcp != NULL && (sp->rtcp->addr[ridx] == NULL ||
		  !ishostseq(sp->rtcp->addr[ridx], sstosa(&raddr)))) {
		    if (sp->rtcp->addr[ridx] == NULL) {
		        sp->rtcp->addr[ridx] = malloc(rlen);
		        if (sp->rtcp->addr[ridx] == NULL) {
			    sp->pcount[3]++;
			    rtpp_log_write(RTPP_LOG_ERR, sp->log,
			      "can't allocate memory for remote address - "
			      "removing session");
			    remove_session(sp);
			    continue;
			}
		    }
		    memcpy(sp->rtcp->addr[ridx], &raddr, rlen);
		    satosin(sp->rtcp->addr[ridx])->sin_port = htons(port + 1);
		    /* Use guessed value as the only true one for asymmetric clients */
		    sp->rtcp->canupdate[ridx] = NOT(sp->rtcp->asymmetric[ridx]);
		    rtpp_log_write(RTPP_LOG_INFO, sp->log, "guessing RTCP port "
		      "for %s to be %d",
		      (ridx == 0) ? "callee" : "caller", port + 1);
		}
	    }

	    /* Select socket for sending packet out. */
	    sidx = (ridx == 0) ? 1 : 0;

	    GET_RTP(sp)->ttl = max_ttl;

	    /*
	     * Check that we have some address to which packet is to be
	     * sent out, drop otherwise.
	     */
	    if (sp->addr[sidx] == NULL || GET_RTP(sp)->rtps[sidx] != NULL) {
		sp->pcount[3]++;
		goto do_record;
	    }

	    sp->pcount[2]++;
	    for (i = (dmode && len < LBR_THRS) ? 2 : 1; i > 0; i--) {
		sendto(sp->fds[sidx], buf, len, 0, sp->addr[sidx],
		  SA_LEN(sp->addr[sidx]));
	    }
do_record:
	    if (sp->rrcs[ridx] != NULL && GET_RTP(sp)->rtps[ridx] == NULL)
		rwrite(sp, sp->rrcs[ridx], sstosa(&raddr), buf, len);
	    /* Repeat since we may have several packets queued */
	    goto drain;
	}
	nsessions -= skipfd;
    }

    exit(0);
}
