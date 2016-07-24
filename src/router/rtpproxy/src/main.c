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
 */

#ifdef LINUX_XXX
/* Apparently needed for drand48(3) */
#define _SVID_SOURCE	1
#endif

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "config_pp.h"

#if !defined(NO_ERR_H)
#include <err.h>
#else
#include "rtpp_util.h"
#endif

#ifdef HAVE_SYSTEMD_DAEMON
#include <systemd/sd-daemon.h>
#endif

#include "rtpp_types.h"
#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_controlfd.h"
#include "rtpp_hash_table.h"
#include "rtpp_command.h"
#include "rtpp_command_async.h"
#include "rtpp_proc_async.h"
#include "rtpp_network.h"
#include "rtpp_notify.h"
#include "rtpp_util.h"
#include "rtpp_math.h"
#include "rtpp_stats.h"
#include "rtpp_list.h"
#ifdef RTPP_CHECK_LEAKS
#include "rtpp_memdeb_internal.h"
#endif

#ifndef RTPP_DEBUG
# define RTPP_DEBUG	0
#else
# define RTPP_DEBUG	1
#endif

static void usage(void);

static void
usage(void)
{

    fprintf(stderr, "usage:\trtpproxy [-2fvFiPaRb] [-l addr1[/addr2]] "
      "[-6 addr1[/addr2]] [-s path]\n\t  [-t tos] [-r rdir [-S sdir]] [-T ttl] "
      "[-L nfiles] [-m port_min]\n\t  [-M port_max] [-u uname[:gname]] [-w sock_mode] "
      "[-n timeout_socket]\n\t  [-d log_level[:log_facility]] [-p pid_file]\n"
      "\t  [-c fifo|rr] [-A addr1[/addr2] [-N random/sched_offset] [-W setup_ttl]\n"
      "\trtpproxy -V\n");
    exit(1);
}

static struct cfg *_sig_cf;

static void
rtpp_exit(void)
{
    int ecode;

    ecode = 0;
#ifdef RTPP_CHECK_LEAKS
    ecode = rtpp_memdeb_dumpstats(_sig_cf) == 0 ? 0 : 1;
#ifdef RTPP_MEMDEB_STDOUT
    fclose(stdout);
#endif
#endif
    exit(ecode);
}

static void
fatsignal(int sig)
{

    rtpp_log_write(RTPP_LOG_INFO, _sig_cf->stable->glog, "got signal %d", sig);
    rtpp_exit();
}

static void
sighup(int sig)
{

    if (_sig_cf->stable->slowshutdown == 0) {
        rtpp_log_write(RTPP_LOG_INFO, _sig_cf->stable->glog,
          "got SIGHUP, initiating deorbiting-burn sequence");
    }
    _sig_cf->stable->slowshutdown = 1;
}

static void
ehandler(void)
{

#ifdef MP_MPATROL_H
    __mp_leaktable(0, MP_LT_UNFREED, 0);
#endif

    rtpp_controlfd_cleanup(_sig_cf);
    unlink(_sig_cf->stable->pid_file);
    rtpp_log_write(RTPP_LOG_INFO, _sig_cf->stable->glog, "rtpproxy ended");
    rtpp_log_close(_sig_cf->stable->glog);
}

long long
rtpp_rlim_max(struct cfg *cf)
{

    return (long long)(cf->stable->nofile_limit->rlim_max);
}

static void
init_config(struct cfg *cf, int argc, char **argv)
{
    int ch, i, umode, stdio_mode;
    char *bh[2], *bh6[2], *cp, *tp[2];
    const char *errmsg;
    struct passwd *pp;
    struct group *gp;
    double x, y;
    struct rtpp_ctrl_sock *ctrl_sock;

    bh[0] = bh[1] = bh6[0] = bh6[1] = NULL;

    umode = stdio_mode = 0;

    cf->stable->pid_file = PID_FILE;

    cf->stable->port_min = PORT_MIN;
    cf->stable->port_max = PORT_MAX;
    cf->stable->port_ctl = 0;

    cf->stable->advaddr[0] = NULL;
    cf->stable->advaddr[1] = NULL;

    cf->stable->max_ttl = SESSION_TIMEOUT;
    cf->stable->tos = TOS;
    cf->stable->rrtcp = 1;
    cf->stable->sock_mode = 0;
    cf->stable->ttl_mode = TTL_UNIFIED;
    cf->stable->log_level = -1;
    cf->stable->log_facility = -1;
    cf->stable->sched_offset = 0.0;
    cf->stable->sched_hz = rtpp_get_sched_hz();
    cf->stable->sched_policy = SCHED_OTHER;
    cf->stable->target_pfreq = MIN(POLL_RATE, cf->stable->sched_hz);
#if RTPP_DEBUG
    fprintf(stderr, "target_pfreq = %f\n", cf->stable->target_pfreq);
#endif
    cf->stable->slowshutdown = 0;

    cf->timeout_handler = rtpp_th_init();
    if (cf->timeout_handler == NULL)
        err(1, "rtpp_th_init");

    pthread_mutex_init(&cf->glock, NULL);
    pthread_mutex_init(&cf->sessinfo.lock, NULL);
    pthread_mutex_init(&cf->bindaddr_lock, NULL);

    cf->stable->nofile_limit = malloc(sizeof(*cf->stable->nofile_limit));
    if (cf->stable->nofile_limit == NULL)
        err(1, "malloc");
    if (getrlimit(RLIMIT_NOFILE, cf->stable->nofile_limit) != 0)
	err(1, "getrlimit");

    while ((ch = getopt(argc, argv, "vf2Rl:6:s:S:t:r:p:T:L:m:M:u:Fin:Pad:VN:c:A:w:bW:")) != -1) {
	switch (ch) {
        case 'c':
            if (strcmp(optarg, "fifo") == 0) {
                 cf->stable->sched_policy = SCHED_FIFO;
                 break;
            }
            if (strcmp(optarg, "rr") == 0) {
                 cf->stable->sched_policy = SCHED_RR;
                 break;
            }
            errx(1, "%s: unknown scheduling policy", optarg);
            break;

        case 'N':
	    if (strcmp(optarg, "random") == 0) {
                x = getdtime() * 1000000.0;
                srand48((long)x);
                cf->stable->sched_offset = drand48();
            } else {
                tp[0] = optarg;
                tp[1] = strchr(tp[0], '/');
       	        if (tp[1] == NULL) {
                    errx(1, "%s: -N should be in the format X/Y", optarg);
                }
                *tp[1] = '\0';
                tp[1]++;
                x = (double)strtol(tp[0], &tp[0], 10);
                y = (double)strtol(tp[1], &tp[1], 10);
                cf->stable->sched_offset = x / y;
            }
            x = (double)cf->stable->sched_hz / cf->stable->target_pfreq;
            cf->stable->sched_offset = my_trunc(x * cf->stable->sched_offset) / x;
            cf->stable->sched_offset /= cf->stable->target_pfreq;
            warnx("sched_offset = %f",  cf->stable->sched_offset);
            break;

	case 'f':
	    cf->stable->nodaemon = 1;
	    break;

	case 'l':
	    bh[0] = optarg;
	    bh[1] = strchr(bh[0], '/');
	    if (bh[1] != NULL) {
		*bh[1] = '\0';
		bh[1]++;
		cf->stable->bmode = 1;
	    }
	    break;

	case '6':
	    bh6[0] = optarg;
	    bh6[1] = strchr(bh6[0], '/');
	    if (bh6[1] != NULL) {
		*bh6[1] = '\0';
		bh6[1]++;
		cf->stable->bmode = 1;
	    }
	    break;

    case 'A':
        if (*optarg == '\0') {
            errx(1, "first advertised address is invalid");
        }
        cf->stable->advaddr[0] = optarg;
        cp = strchr(optarg, '/');
        if (cp != NULL) {
            *cp = '\0';
            cp++;
            if (*cp == '\0') {
                errx(1, "second advertised address is invalid");
            }
        }
        cf->stable->advaddr[1] = cp;
        break;

	case 's':
            ctrl_sock = rtpp_ctrl_sock_parse(optarg);
            if (ctrl_sock == NULL) {
                errx(1, "can't parse control socket argument");
            }
            rtpp_list_append(cf->stable->ctrl_socks, ctrl_sock);
            if (RTPP_CTRL_ISDG(ctrl_sock)) {
                umode = 1;
            } else if (ctrl_sock->type == RTPC_STDIO) {
                stdio_mode = 1;
            }
	    break;

	case 't':
	    cf->stable->tos = atoi(optarg);
	    if (cf->stable->tos > 255)
		errx(1, "%d: TOS is too large", cf->stable->tos);
	    break;

	case '2':
	    cf->stable->dmode = 1;
	    break;

	case 'v':
	    printf("Basic version: %d\n", CPROTOVER);
	    for (i = 1; proto_caps[i].pc_id != NULL; ++i) {
		printf("Extension %s: %s\n", proto_caps[i].pc_id,
		    proto_caps[i].pc_description);
	    }
	    rtpp_exit();
	    break;

	case 'r':
	    cf->stable->rdir = optarg;
	    break;

	case 'S':
	    cf->stable->sdir = optarg;
	    break;

	case 'R':
	    cf->stable->rrtcp = 0;
	    break;

	case 'p':
	    cf->stable->pid_file = optarg;
	    break;

	case 'T':
	    cf->stable->max_ttl = atoi(optarg);
	    break;

	case 'L':
	    cf->stable->nofile_limit->rlim_cur = cf->stable->nofile_limit->rlim_max = atoi(optarg);
	    if (setrlimit(RLIMIT_NOFILE, cf->stable->nofile_limit) != 0)
		err(1, "setrlimit");
	    if (getrlimit(RLIMIT_NOFILE, cf->stable->nofile_limit) != 0)
		err(1, "getrlimit");
	    if (cf->stable->nofile_limit->rlim_max < atoi(optarg))
		warnx("limit allocated by setrlimit (%d) is less than "
		  "requested (%d)", (int) cf->stable->nofile_limit->rlim_max,
		  atoi(optarg));
	    break;

	case 'm':
	    cf->stable->port_min = atoi(optarg);
	    break;

	case 'M':
	    cf->stable->port_max = atoi(optarg);
	    break;

	case 'u':
	    cf->stable->run_uname = optarg;
	    cp = strchr(optarg, ':');
	    if (cp != NULL) {
		if (cp == optarg)
		    cf->stable->run_uname = NULL;
		cp[0] = '\0';
		cp++;
	    }
	    cf->stable->run_gname = cp;
	    cf->stable->run_uid = -1;
	    cf->stable->run_gid = -1;
	    if (cf->stable->run_uname != NULL) {
		pp = getpwnam(cf->stable->run_uname);
		if (pp == NULL)
		    err(1, "can't find ID for the user: %s", cf->stable->run_uname);
		cf->stable->run_uid = pp->pw_uid;
		if (cf->stable->run_gname == NULL)
		    cf->stable->run_gid = pp->pw_gid;
	    }
	    if (cf->stable->run_gname != NULL) {
		gp = getgrnam(cf->stable->run_gname);
		if (gp == NULL)
		    err(1, "can't find ID for the group: %s", cf->stable->run_gname);
		cf->stable->run_gid = gp->gr_gid;
                if (cf->stable->sock_mode == 0) {
                    cf->stable->sock_mode = 0755;
                }
	    }
	    break;

	case 'w':
	    cf->stable->sock_mode = atoi(optarg);
	    break;

	case 'F':
	    cf->stable->no_check = 1;
	    break;

	case 'i':
	    cf->stable->ttl_mode = TTL_INDEPENDENT;
	    break;

	case 'n':
	    if(strncmp("unix:", optarg, 5) == 0)
		optarg += 5;
	    if(strlen(optarg) == 0)
		errx(1, "timeout notification socket name too short");
            if (rtpp_th_set_sn(cf->timeout_handler, optarg, NULL) == NULL) {
		err(1, "can't allocate memory");
            }
	    break;

	case 'P':
	    cf->stable->record_pcap = 1;
	    break;

	case 'a':
	    cf->stable->record_all = 1;
	    break;

	case 'd':
	    cp = strchr(optarg, ':');
	    if (cp != NULL) {
		cf->stable->log_facility = rtpp_log_str2fac(cp + 1);
		if (cf->stable->log_facility == -1)
		    errx(1, "%s: invalid log facility", cp + 1);
		*cp = '\0';
	    }
	    cf->stable->log_level = rtpp_log_str2lvl(optarg);
	    if (cf->stable->log_level == -1)
		errx(1, "%s: invalid log level", optarg);
	    break;

	case 'V':
	    printf("%s\n", RTPP_SW_VERSION);
	    rtpp_exit();
	    break;

        case 'W':
            cf->stable->max_setup_ttl = atoi(optarg);
            break;

        case 'b':
            cf->stable->seq_ports = 1;
            break;

	case '?':
	default:
	    usage();
	}
    }

    if (cf->stable->max_setup_ttl == 0) {
        cf->stable->max_setup_ttl = cf->stable->max_ttl;
    }

    /* No control socket has been specified, add a default one */
    if (RTPP_LIST_IS_EMPTY(cf->stable->ctrl_socks)) {
        ctrl_sock = rtpp_ctrl_sock_parse(CMD_SOCK);
        if (ctrl_sock == NULL) {
            errx(1, "can't parse control socket: \"%s\"", CMD_SOCK);
        }
        rtpp_list_append(cf->stable->ctrl_socks, ctrl_sock);
    }

    if (cf->stable->rdir == NULL && cf->stable->sdir != NULL)
	errx(1, "-S switch requires -r switch");

    if (cf->stable->nodaemon == 0 && stdio_mode != 0)
        errx(1, "stdio command mode requires -f switch");

    if (cf->stable->no_check == 0 && getuid() == 0 && cf->stable->run_uname == NULL) {
	if (umode != 0) {
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
    if ((cf->stable->port_min % 2) != 0)
	cf->stable->port_min++;
    if ((cf->stable->port_max % 2) != 0) {
	cf->stable->port_max--;
    } else {
	/*
	 * If port_max is already even then there is no
	 * "room" for the RTCP port, go back by two ports.
	 */
	cf->stable->port_max -= 2;
    }

    if (!IS_VALID_PORT(cf->stable->port_min))
	errx(1, "invalid value of the port_min argument, "
	  "not in the range 1-65535");
    if (!IS_VALID_PORT(cf->stable->port_max))
	errx(1, "invalid value of the port_max argument, "
	  "not in the range 1-65535");
    if (cf->stable->port_min > cf->stable->port_max)
	errx(1, "port_min should be less than port_max");

    cf->sessinfo.sessions = malloc((sizeof cf->sessinfo.sessions[0]) *
      (((cf->stable->port_max - cf->stable->port_min + 1)) + 1));
    cf->rtp_servers =  malloc((sizeof cf->rtp_servers[0]) *
      (((cf->stable->port_max - cf->stable->port_min + 1) * 2) + 1));
    cf->sessinfo.pfds_rtp = malloc((sizeof cf->sessinfo.pfds_rtp[0]) *
      (((cf->stable->port_max - cf->stable->port_min + 1)) + 1));
    cf->sessinfo.pfds_rtcp = malloc((sizeof cf->sessinfo.pfds_rtcp[0]) *
      (((cf->stable->port_max - cf->stable->port_min + 1)) + 1));

    if (bh[0] == NULL && bh[1] == NULL && bh6[0] == NULL && bh6[1] == NULL) {
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
    if (cf->stable->bmode != 0) {
	if (bh[0] != NULL && bh6[0] != NULL)
	    errx(1, "either IPv4 or IPv6 should be configured for external "
	      "interface in bridging mode, not both");
	if (bh[1] != NULL && bh6[1] != NULL)
	    errx(1, "either IPv4 or IPv6 should be configured for internal "
	      "interface in bridging mode, not both");
    if (cf->stable->advaddr[0] != NULL && cf->stable->advaddr[1] == NULL)
        errx(1, "two advertised addresses are required for internal "
          "and external interfaces in bridging mode");
	if (i != 2)
	    errx(1, "incomplete configuration of the bridging mode - exactly "
	      "2 listen addresses required, %d provided", i);
    } else if (i != 1) {
	errx(1, "exactly 1 listen addresses required, %d provided", i);
    }

    for (i = 0; i < 2; i++) {
	cf->stable->bindaddr[i] = NULL;
	if (bh[i] != NULL) {
	    cf->stable->bindaddr[i] = host2bindaddr(cf, bh[i], AF_INET, &errmsg);
	    if (cf->stable->bindaddr[i] == NULL)
		errx(1, "host2bindaddr: %s", errmsg);
	    continue;
	}
	if (bh6[i] != NULL) {
	    cf->stable->bindaddr[i] = host2bindaddr(cf, bh6[i], AF_INET6, &errmsg);
	    if (cf->stable->bindaddr[i] == NULL)
		errx(1, "host2bindaddr: %s", errmsg);
	    continue;
	}
    }
    if (cf->stable->bindaddr[0] == NULL) {
	cf->stable->bindaddr[0] = cf->stable->bindaddr[1];
	cf->stable->bindaddr[1] = NULL;
    }
}

int
main(int argc, char **argv)
{
    int i, len;
    double eval, clk;
    long long ncycles_ref, counter;
    double eptime;
    double add_delay;
    struct cfg cf;
    char buf[256];
    struct recfilter loop_error;
    struct PFD phase_detector;
    long usleep_time;
    struct sched_param sparam;
#if RTPP_DEBUG
    double sleep_time, filter_lastval;
#endif

#ifdef RTPP_CHECK_LEAKS
    if (rtpp_memdeb_selftest() != 0) {
        errx(1, "MEMDEB self-test has failed");
        /* NOTREACHED */
    }
#endif

    memset(&cf, 0, sizeof(cf));

    cf.stable = malloc(sizeof(struct rtpp_cfg_stable));
    if (cf.stable == NULL) {
         err(1, "can't allocate memory for the struct rtpp_cfg_stable");
         /* NOTREACHED */
    }
    memset(cf.stable, '\0', sizeof(struct rtpp_cfg_stable));
    cf.stable->ctrl_socks = malloc(sizeof(struct rtpp_list));
    if (cf.stable->ctrl_socks == NULL) {
         err(1, "can't allocate memory for the struct rtpp_cfg_stable");
         /* NOTREACHED */
    }
    memset(cf.stable->ctrl_socks, '\0', sizeof(struct rtpp_list));
    RTPP_LIST_RESET(cf.stable->ctrl_socks);    

    init_config(&cf, argc, argv);

    seedrandom();

    cf.stable->sessions_ht = rtpp_hash_table_ctor();
    if (cf.stable->sessions_ht == NULL) {
        err(1, "can't allocate memory for the hash table");
         /* NOTREACHED */
    }
    cf.stable->rtpp_stats = rtpp_stats_ctor();
    if (cf.stable->rtpp_stats == NULL) {
        err(1, "can't allocate memory for the stats data");
         /* NOTREACHED */
    }
    init_port_table(&cf);

    if (rtpp_controlfd_init(&cf) != 0) {
        err(1, "can't inilialize control socket%s",
          cf.stable->ctrl_socks->len > 1 ? "s" : "");
    }

    if (cf.stable->nodaemon == 0) {
	if (rtpp_daemon(0, 0) == -1)
	    err(1, "can't switch into daemon mode");
	    /* NOTREACHED */
    }

    if (rtpp_notify_init() != 0)
        errx(1, "can't start notification thread");

    cf.stable->glog = rtpp_log_open(cf.stable, "rtpproxy", NULL, LF_REOPEN);
    rtpp_log_setlevel(cf.stable->glog, cf.stable->log_level);
    _sig_cf = &cf;
    atexit(ehandler);
    rtpp_log_write(RTPP_LOG_INFO, cf.stable->glog, "rtpproxy started, pid %d", getpid());

    i = open(cf.stable->pid_file, O_WRONLY | O_CREAT | O_TRUNC, DEFFILEMODE);
    if (i >= 0) {
	len = sprintf(buf, "%u\n", (unsigned int)getpid());
	write(i, buf, len);
	close(i);
    } else {
	rtpp_log_ewrite(RTPP_LOG_ERR, cf.stable->glog, "can't open pidfile for writing");
    }

    signal(SIGHUP, sighup);
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

    if (cf.stable->sched_policy != SCHED_OTHER) {
        sparam.sched_priority = sched_get_priority_max(cf.stable->sched_policy);
        if (sched_setscheduler(0, cf.stable->sched_policy, &sparam) == -1) {
            rtpp_log_ewrite(RTPP_LOG_ERR, cf.stable->glog, "sched_setscheduler(SCHED_%s, %d)",
              (cf.stable->sched_policy == SCHED_FIFO) ? "FIFO" : "RR", sparam.sched_priority);
        }
    }

    if (cf.stable->run_uname != NULL || cf.stable->run_gname != NULL) {
	if (drop_privileges(&cf) != 0) {
	    rtpp_log_ewrite(RTPP_LOG_ERR, cf.stable->glog,
	      "can't switch to requested user/group");
	    exit(1);
	}
    }
    set_rlimits(&cf);

    cf.sessinfo.sessions[0] = NULL;
    cf.sessinfo.nsessions = 0;
    cf.rtp_nsessions = 0;

    rtpp_proc_async_init(&cf);
    rtpp_command_async_init(&cf);

    counter = 0;
    recfilter_init(&loop_error, 0.96, 0.0, 0);
    PFD_init(&phase_detector, 0.0);
#ifdef HAVE_SYSTEMD_DAEMON
    sd_notify(0, "READY=1");
#endif
#ifdef RTPP_CHECK_LEAKS
    rtpp_memdeb_setbaseln();
#endif
    for (;;) {
	eptime = getdtime();

        clk = (eptime + cf.stable->sched_offset) * cf.stable->target_pfreq;

        ncycles_ref = clk;

        eval = PFD_get_error(&phase_detector, clk);

#if RTPP_DEBUG
        filter_lastval = loop_error.lastval;
#endif

        if (eval != 0.0) {
            recfilter_apply(&loop_error, sigmoid(eval));
        }

#if RTPP_DEBUG
        if (counter % (unsigned int)cf.stable->target_pfreq == 0 || counter < 1000) {
          rtpp_log_write(RTPP_LOG_DBUG, cf.stable->glog, "run %lld ncycles %f raw error1 %f, filter lastval %f, filter nextval %f",
            counter, clk, eval, filter_lastval, loop_error.lastval);
        }
#endif
        add_delay = freqoff_to_period(cf.stable->target_pfreq, 1.0, loop_error.lastval);
        usleep_time = add_delay * 1000000.0;
#if RTPP_DEBUG
        if (counter % (unsigned int)cf.stable->target_pfreq == 0 || counter < 1000) {
            rtpp_log_write(RTPP_LOG_DBUG, cf.stable->glog, "run %lld filter lastval %f, filter nextval %f, error %f",
              counter, filter_lastval, loop_error.lastval, sigmoid(eval));
            rtpp_log_write(RTPP_LOG_DBUG, cf.stable->glog, "run %lld extra sleeping time %llu", counter, usleep_time);
        }
        sleep_time = getdtime();
#endif
        rtpp_proc_async_wakeup(cf.stable->rtpp_proc_cf, counter, ncycles_ref);
        usleep(usleep_time);
#if RTPP_DEBUG
        sleep_time = getdtime() - sleep_time;
        if (counter % (unsigned int)cf.stable->target_pfreq == 0 || counter < 1000 || sleep_time > add_delay * 2.0) {
            rtpp_log_write(RTPP_LOG_DBUG, cf.stable->glog, "run %lld sleeping time required %llu sleeping time actual %f, CSV: %f,%f,%f", \
              counter, usleep_time, sleep_time, (double)counter / cf.stable->target_pfreq, ((double)usleep_time) / 1000.0, sleep_time * 1000.0);
        }
#endif
        counter += 1;
        if (cf.stable->slowshutdown != 0) {
            pthread_mutex_lock(&cf.sessinfo.lock);
            if (cf.sessinfo.nsessions == 0) {
                /* The below unlock is not necessary, but does not hurt either */
                pthread_mutex_unlock(&cf.sessinfo.lock);
                rtpp_log_write(RTPP_LOG_INFO, cf.stable->glog,
                  "deorbiting-burn sequence completed, exiting");
                break;
            }
            pthread_mutex_unlock(&cf.sessinfo.lock);
        }
    }

#ifdef HAVE_SYSTEMD_DAEMON
    sd_notify(0, "STATUS=Exited");
#endif

    rtpp_exit();
}
