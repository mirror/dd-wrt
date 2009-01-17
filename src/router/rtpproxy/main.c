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
 * $Id: main.c,v 1.87 2008/12/24 10:46:03 sobomax Exp $
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
#include <ctype.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <netdb.h>
#include <poll.h>
#include <pwd.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "rtp.h"
#include "rtp_resizer.h"
#include "rtp_server.h"
#include "rtpp_defines.h"
#include "rtpp_command.h"
#include "rtpp_log.h"
#include "rtpp_record.h"
#include "rtpp_session.h"
#include "rtpp_util.h"

static const char *cmd_sock = CMD_SOCK;
static const char *pid_file = PID_FILE;
static rtpp_log_t glog;

static void setbindhost(struct sockaddr *, int, const char *, const char *);
static void usage(void);
static void send_packet(struct cfg *, struct rtpp_session *, int,
  struct rtp_packet *);

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
usage(void)
{

    fprintf(stderr, "usage: rtpproxy [-2fvFiPa] [-l addr1[/addr2]] "
      "[-6 addr1[/addr2]] [-s path]\n\t[-t tos] [-r rdir [-S sdir]] [-T ttl] "
      "[-L nfiles] [-m port_min]\n\t[-M port_max] [-u uname[:gname]] "
      "[-n timeout_socket] [-d log_level]\n");
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

static void 
init_config(struct cfg *cf, int argc, char **argv)
{
    int ch, i;
    char *bh[2], *bh6[2], *cp;
    struct passwd *pp;
    struct group *gp;

    bh[0] = bh[1] = bh6[0] = bh6[1] = NULL;

    cf->port_min = PORT_MIN;
    cf->port_max = PORT_MAX;

    cf->max_ttl = SESSION_TIMEOUT;
    cf->tos = TOS;
    cf->rrtcp = 1;
    cf->ttl_mode = TTL_UNIFIED;
    cf->log_level = LOG_LEVEL;

    cf->timeout_handler.socket_name = NULL;
    cf->timeout_handler.fd = -1;
    cf->timeout_handler.connected = 0;

    if (getrlimit(RLIMIT_NOFILE, &(cf->nofile_limit)) != 0)
	err(1, "getrlimit");

    while ((ch = getopt(argc, argv, "vf2Rl:6:s:S:t:r:p:T:L:m:M:u:Fin:Pad:")) != -1)
	switch (ch) {
	case 'f':
	    cf->nodaemon = 1;
	    break;

	case 'l':
	    bh[0] = optarg;
	    bh[1] = strchr(bh[0], '/');
	    if (bh[1] != NULL) {
		*bh[1] = '\0';
		bh[1]++;
		cf->bmode = 1;
	    }
	    break;

	case '6':
	    bh6[0] = optarg;
	    bh6[1] = strchr(bh6[0], '/');
	    if (bh6[1] != NULL) {
		*bh6[1] = '\0';
		bh6[1]++;
		cf->bmode = 1;
	    }
	    break;

	case 's':
	    if (strncmp("udp:", optarg, 4) == 0) {
		cf->umode = 1;
		optarg += 4;
	    } else if (strncmp("udp6:", optarg, 5) == 0) {
		cf->umode = 6;
		optarg += 5;
	    } else if (strncmp("unix:", optarg, 5) == 0) {
		cf->umode = 0;
		optarg += 5;
	    }
	    cmd_sock = optarg;
	    break;

	case 't':
	    cf->tos = atoi(optarg);
	    break;

	case '2':
	    cf->dmode = 1;
	    break;

	case 'v':
	    printf("Basic version: %d\n", CPROTOVER);
	    for (i = 1; proto_caps[i].pc_id != NULL; ++i) {
		printf("Extension %s: %s\n", proto_caps[i].pc_id,
		    proto_caps[i].pc_description);
	    }
	    exit(0);
	    break;

	case 'r':
	    cf->rdir = optarg;
	    break;

	case 'S':
	    cf->sdir = optarg;
	    break;

	case 'R':
	    cf->rrtcp = 0;
	    break;

	case 'p':
	    pid_file = optarg;
	    break;

	case 'T':
	    cf->max_ttl = atoi(optarg);
	    break;

	case 'L':
	    cf->nofile_limit.rlim_cur = cf->nofile_limit.rlim_max = atoi(optarg);
	    if (setrlimit(RLIMIT_NOFILE, &(cf->nofile_limit)) != 0)
		err(1, "setrlimit");
	    if (getrlimit(RLIMIT_NOFILE, &(cf->nofile_limit)) != 0)
		err(1, "getrlimit");
	    if (cf->nofile_limit.rlim_max < atoi(optarg))
		warnx("limit allocated by setrlimit (%d) is less than "
		  "requested (%d)", (int) cf->nofile_limit.rlim_max,
		  atoi(optarg));
	    break;

	case 'm':
	    cf->port_min = atoi(optarg);
	    break;

	case 'M':
	    cf->port_max = atoi(optarg);
	    break;

	case 'u':
	    cf->run_uname = optarg;
	    cp = strchr(optarg, ':');
	    if (cp != NULL) {
		if (cp == optarg)
		    cf->run_uname = NULL;
		cp[0] = '\0';
		cp++;
	    }
	    cf->run_gname = cp;
	    cf->run_uid = -1;
	    cf->run_gid = -1;
	    if (cf->run_uname != NULL) {
		pp = getpwnam(cf->run_uname);
		if (pp == NULL)
		    err(1, "can't find ID for the user: %s", cf->run_uname);
		cf->run_uid = pp->pw_uid;
		if (cf->run_gname == NULL)
		    cf->run_gid = pp->pw_gid;
	    }
	    if (cf->run_gname != NULL) {
		gp = getgrnam(cf->run_gname);
		if (gp == NULL)
		    err(1, "can't find ID for the group: %s", cf->run_gname);
		cf->run_gid = gp->gr_gid;
	    }
	    break;

	case 'F':
	    cf->no_check = 1;
	    break;

	case 'i':
	    cf->ttl_mode = TTL_INDEPENDENT;
	    break;

	case 'n':
	    if(strncmp("unix:", optarg, 5) == 0)
		optarg += 5;
	    if(strlen(optarg) == 0)
		errx(1, "timeout notification socket name too short");
	    cf->timeout_handler.socket_name = (char *)malloc(strlen(optarg) + 1);
	    if(cf->timeout_handler.socket_name == NULL)
		err(1, "can't allocate memory");
	    strcpy(cf->timeout_handler.socket_name, optarg);
	    break;

	case 'P':
	    cf->record_pcap = 1;
	    break;

	case 'a':
	    cf->record_all = 1;
	    break;

	case 'd':
	    cf->log_level = rtpp_log_str2lvl(optarg);
	    if (cf->log_level == -1)
		errx(1, "%s: invalid log level", optarg);
	    break;

	case '?':
	default:
	    usage();
	}
    if (cf->rdir == NULL && cf->sdir != NULL)
	errx(1, "-S switch requires -r switch");

    if (cf->no_check == 0 && getuid() == 0 && cf->run_uname == NULL) {
	if (cf->umode != 0) {
	    errx(1, "running this program as superuser in a remote control "
	      "mode is strongly not recommended, as it poses serious security "
	      "threat to your system. Use -u option to run as an unprivileged "
	      "user or -F is you want to run as a superuser anyway.");
	} else {
	    warnx("WARNING!!! Running this program as superuser is strongly "
	      "not recommended, as it may pose serious security threat to "
	      "your system. Use -u option to run as an unprivileged user "
	      "or -F to surpress this warning.");
	}
    }

    /* make sure that port_min and port_max are even */
    if ((cf->port_min % 2) != 0)
	cf->port_min++;
    if ((cf->port_max % 2) != 0) {
	cf->port_max--;
    } else {
	/*
	 * If port_max is already even then there is no
	 * "room" for the RTCP port, go back by two ports.
	 */
	cf->port_max -= 2;
    }

    if (cf->port_min <= 0 || cf->port_min > 65535)
	errx(1, "invalid value of the port_min argument, "
	  "not in the range 1-65535");
    if (cf->port_max <= 0 || cf->port_max > 65535)
	errx(1, "invalid value of the port_max argument, "
	  "not in the range 1-65535");
    if (cf->port_min > cf->port_max)
	errx(1, "port_min should be less than port_max");

    cf->sessions = malloc((sizeof cf->sessions[0]) *
      (((cf->port_max - cf->port_min + 1) * 2) + 1));
    cf->rtp_servers =  malloc((sizeof cf->rtp_servers[0]) *
      (((cf->port_max - cf->port_min + 1) * 2) + 1));
    cf->pfds = malloc((sizeof cf->pfds[0]) *
      (((cf->port_max - cf->port_min + 1) * 2) + 1));

    if (bh[0] == NULL && bh[1] == NULL && bh6[0] == NULL && bh6[1] == NULL) {
	if (cf->umode != 0)
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
    if (cf->bmode != 0) {
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
	cf->bindaddr[i] = NULL;
	if (bh[i] != NULL) {
	    cf->bindaddr[i] = malloc(sizeof(struct sockaddr_storage));
	    setbindhost(cf->bindaddr[i], AF_INET, bh[i], SERVICE);
	    continue;
	}
	if (bh6[i] != NULL) {
	    cf->bindaddr[i] = malloc(sizeof(struct sockaddr_storage));
	    setbindhost(cf->bindaddr[i], AF_INET6, bh6[i], SERVICE);
	    continue;
	}
    }
    if (cf->bindaddr[0] == NULL) {
	cf->bindaddr[0] = cf->bindaddr[1];
	cf->bindaddr[1] = NULL;
    }
}

static int
init_controlfd(struct cfg *cf)
{
    struct sockaddr_un ifsun;
    struct sockaddr_storage ifsin;
    char *cp;
    int i, controlfd, flags;

    if (cf->umode == 0) {
	unlink(cmd_sock);
	memset(&ifsun, '\0', sizeof ifsun);
#if defined(HAVE_SOCKADDR_SUN_LEN)
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
	if ((cf->run_uname != NULL || cf->run_gname != NULL) &&
	  chown(cmd_sock, cf->run_uid, cf->run_gid) == -1)
	    err(1, "can't set owner of the socket");
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
	i = (cf->umode == 6) ? AF_INET6 : AF_INET;
	setbindhost(sstosa(&ifsin), i, cmd_sock, cp);
	controlfd = socket(i, SOCK_DGRAM, 0);
	if (controlfd == -1)
	    err(1, "can't create socket");
	if (bind(controlfd, sstosa(&ifsin), SS_LEN(&ifsin)) < 0)
	    err(1, "can't bind to a socket");
    }
    flags = fcntl(controlfd, F_GETFL);
    fcntl(controlfd, F_SETFL, flags | O_NONBLOCK);

    return controlfd;
}

static void
process_rtp_servers(struct cfg *cf, double dtime)
{
    int j, k, sidx, len, skipfd;
    struct rtpp_session *sp;

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
	    while ((len = rtp_server_get(sp->rtps[sidx], dtime)) != RTPS_LATER) {
		if (len == RTPS_EOF) {
		    rtp_server_free(sp->rtps[sidx]);
		    sp->rtps[sidx] = NULL;
		    if (sp->rtps[0] == NULL && sp->rtps[1] == NULL) {
			assert(cf->rtp_servers[sp->sridx] == sp);
			cf->rtp_servers[sp->sridx] = NULL;
			sp->sridx = -1;
		    }
		    break;
		}
		for (k = (cf->dmode && len < LBR_THRS) ? 2 : 1; k > 0; k--) {
		    sendto(sp->fds[sidx], sp->rtps[sidx]->buf, len, 0,
		      sp->addr[sidx], SA_LEN(sp->addr[sidx]));
		}
	    }
	}
    }
    cf->rtp_nsessions -= skipfd;
}

static void
rxmit_packets(struct cfg *cf, struct rtpp_session *sp, int ridx,
  double dtime)
{
    int ndrain, i, port;
    struct rtp_packet *packet = NULL;

    /* Repeat since we may have several packets queued on the same socket */
    for (ndrain = 0; ndrain < 5; ndrain++) {
	if (packet != NULL)
	    rtp_packet_free(packet);

	packet = rtp_recv(sp->fds[ridx]);
	if (packet == NULL)
	    break;
	packet->laddr = sp->laddr[ridx];
	packet->rport = sp->ports[ridx];
	packet->rtime = dtime;

	i = 0;
	if (sp->addr[ridx] != NULL) {
	    /* Check that the packet is authentic, drop if it isn't */
	    if (sp->asymmetric[ridx] == 0) {
		if (memcmp(sp->addr[ridx], &packet->raddr, packet->rlen) != 0) {
		    if (sp->canupdate[ridx] == 0) {
			/*
			 * Continue, since there could be good packets in
			 * queue.
			 */
			continue;
		    }
		    /* Signal that an address has to be updated */
		    i = 1;
		} else if (sp->canupdate[ridx] != 0 &&
		  sp->last_update[ridx] != 0 &&
		  dtime - sp->last_update[ridx] > UPDATE_WINDOW) {
		    sp->canupdate[ridx] = 0;
		}
	    } else {
		/*
		 * For asymmetric clients don't check
		 * source port since it may be different.
		 */
		if (!ishostseq(sp->addr[ridx], sstosa(&packet->raddr)))
		    /*
		     * Continue, since there could be good packets in
		     * queue.
		     */
		    continue;
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
		/* Break, sp is invalid now */
		break;
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

	    port = ntohs(satosin(&packet->raddr)->sin_port);

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
			/* Break, sp is invalid now */
			break;
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

	if (sp->resizers[ridx].output_nsamples > 0)
	    rtp_resizer_enqueue(&sp->resizers[ridx], &packet);
	if (packet != NULL)
	    send_packet(cf, sp, ridx, packet);
    }

    if (packet != NULL)
	rtp_packet_free(packet);
}

static void
send_packet(struct cfg *cf, struct rtpp_session *sp, int ridx,
  struct rtp_packet *packet)
{
    int i, sidx;

    GET_RTP(sp)->ttl[ridx] = cf->max_ttl;

    /* Select socket for sending packet out. */
    sidx = (ridx == 0) ? 1 : 0;

    /*
     * Check that we have some address to which packet is to be
     * sent out, drop otherwise.
     */
    if (sp->addr[sidx] == NULL || GET_RTP(sp)->rtps[sidx] != NULL) {
	sp->pcount[3]++;
    } else {
	sp->pcount[2]++;
	for (i = (cf->dmode && packet->size < LBR_THRS) ? 2 : 1; i > 0; i--) {
	    sendto(sp->fds[sidx], packet->data.buf, packet->size, 0, sp->addr[sidx],
	      SA_LEN(sp->addr[sidx]));
	}
    }

    if (sp->rrcs[ridx] != NULL && GET_RTP(sp)->rtps[ridx] == NULL)
	rwrite(sp, sp->rrcs[ridx], packet);
}

static void
process_rtp(struct cfg *cf, double dtime, int alarm_tick)
{
    int readyfd, skipfd, ridx;
    struct rtpp_session *sp;
    struct rtp_packet *packet;

    /* Relay RTP/RTCP */
    skipfd = 0;
    for (readyfd = 1; readyfd < cf->nsessions; readyfd++) {
	sp = cf->sessions[readyfd];

	if (alarm_tick != 0 && sp != NULL && sp->rtcp != NULL &&
	  sp->sidx[0] == readyfd) {
	    if (get_ttl(sp) == 0) {
		rtpp_log_write(RTPP_LOG_INFO, sp->log, "session timeout");
		do_timeout_notification(sp, 1);
		remove_session(cf, sp);
	    } else {
		if (sp->ttl[0] != 0)
		    sp->ttl[0]--;
		if (sp->ttl[1] != 0)
		    sp->ttl[1]--;
	    }
	}

	if (cf->pfds[readyfd].fd == -1) {
	    /* Deleted session, count and move one */
	    skipfd++;
	    continue;
	}

	/* Find index of the call leg within a session */
	for (ridx = 0; ridx < 2; ridx++)
	    if (cf->pfds[readyfd].fd == sp->fds[ridx])
		break;
	/*
	 * Can't happen.
	 */
	assert(ridx != 2);

	/* Compact pfds[] and sessions[] by eliminating removed sessions */
	if (skipfd > 0) {
	    cf->pfds[readyfd - skipfd] = cf->pfds[readyfd];
	    cf->sessions[readyfd - skipfd] = cf->sessions[readyfd];
	    sp->sidx[ridx] = readyfd - skipfd;;
	}

	if (sp->complete != 0) {
	    if ((cf->pfds[readyfd].revents & POLLIN) != 0)
		rxmit_packets(cf, sp, ridx, dtime);
	    if (sp->resizers[ridx].output_nsamples > 0) {
		while ((packet = rtp_resizer_get(&sp->resizers[ridx], dtime)) != NULL) {
		    send_packet(cf, sp, ridx, packet);
		    rtp_packet_free(packet);
		}
	    }
	}
    }
    /* Trim any deleted sessions at the end */
    cf->nsessions -= skipfd;
}

static void
process_commands(struct cfg *cf, double dtime)
{
    int controlfd, i;
    socklen_t rlen;
    struct sockaddr_un ifsun;

    if ((cf->pfds[0].revents & POLLIN) == 0)
	return;

    do {
	if (cf->umode == 0) {
	    rlen = sizeof(ifsun);
	    controlfd = accept(cf->pfds[0].fd, sstosa(&ifsun), &rlen);
	    if (controlfd == -1) {
		if (errno != EWOULDBLOCK)
		    rtpp_log_ewrite(RTPP_LOG_ERR, cf->glog,
		      "can't accept connection on control socket");
		break;
	    }
	} else {
	    controlfd = cf->pfds[0].fd;
	}
	i = handle_command(cf, controlfd, dtime);
	if (cf->umode == 0) {
	    close(controlfd);
	}
    } while (i == 0);
}

int
main(int argc, char **argv)
{
    int i, len, timeout, controlfd, alarm_tick;
    double sptime, eptime, last_tick_time;
    unsigned long delay;
    struct cfg cf;
    char buf[256];

    memset(&cf, 0, sizeof(cf));

    init_config(&cf, argc, argv);

    seedrandom();

    init_hash_table(&cf);
    init_port_table(&cf);

    controlfd = init_controlfd(&cf);

    if (cf.nodaemon == 0) {
	if (rtpp_daemon(0, 0) == -1)
	    err(1, "can't switch into daemon mode");
	    /* NOTREACHED */
    }

    glog = cf.glog = rtpp_log_open(&cf, "rtpproxy", NULL, LF_REOPEN);
    atexit(ehandler);
    rtpp_log_write(RTPP_LOG_INFO, cf.glog, "rtpproxy started, pid %d", getpid());

    i = open(pid_file, O_WRONLY | O_CREAT | O_TRUNC, DEFFILEMODE);
    if (i >= 0) {
	len = sprintf(buf, "%u\n", getpid());
	write(i, buf, len);
	close(i);
    } else {
	rtpp_log_ewrite(RTPP_LOG_ERR, cf.glog, "can't open pidfile for writing");
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

    if (cf.run_uname != NULL || cf.run_gname != NULL) {
	if (drop_privileges(&cf) != 0) {
	    rtpp_log_ewrite(RTPP_LOG_ERR, cf.glog,
	      "can't switch to requested user/group");
	    exit(1);
	}
    }

    cf.pfds[0].fd = controlfd;
    cf.pfds[0].events = POLLIN;
    cf.pfds[0].revents = 0;

    cf.sessions[0] = NULL;
    cf.nsessions = 1;
    cf.rtp_nsessions = 0;

    sptime = 0;
    last_tick_time = 0;
    for (;;) {
	if (cf.rtp_nsessions > 0 || cf.nsessions > 1)
	    timeout = RTPS_TICKS_MIN;
	else
	    timeout = TIMETICK * 1000;
	eptime = getdtime();
	delay = (eptime - sptime) * 1000000.0;
	if (delay < (1000000 / POLL_LIMIT)) {
	    usleep((1000000 / POLL_LIMIT) - delay);
	    sptime = getdtime();
	} else {
	    sptime = eptime;
	}
	i = poll(cf.pfds, cf.nsessions, timeout);
	if (i < 0 && errno == EINTR)
	    continue;
	eptime = getdtime();
	if (cf.rtp_nsessions > 0) {
	    process_rtp_servers(&cf, eptime);
	}
	if (eptime > last_tick_time + TIMETICK) {
	    alarm_tick = 1;
	    last_tick_time = eptime;
	} else {
	    alarm_tick = 0;
	}
	process_rtp(&cf, eptime, alarm_tick);
	if (i > 0) {
	    process_commands(&cf, eptime);
	}
    }

    exit(0);
}
