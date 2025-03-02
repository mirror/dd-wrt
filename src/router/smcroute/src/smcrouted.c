/* Static multicast routing daemon
 *
 * Copyright (C) 2011-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "config.h"

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <signal.h>
#include <sysexits.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>		/* gettimeofday() */
#include <sys/un.h>

#include "cap.h"
#include "ipc.h"
#include "log.h"
#include "msg.h"
#include "conf.h"
#include "iface.h"
#include "util.h"
#include "timer.h"
#include "notify.h"
#include "script.h"
#include "socket.h"
#include "mrdisc.h"
#include "mroute.h"
#include "mcgroup.h"

int background = 1;
int do_vifs    = 1;
int do_syslog  = 1;
int cache_tmo  = 60;
int interval   = MRDISC_INTERVAL_DEFAULT;
int startup_delay = 0;
int exit_delay = 0;
int table_id   = 0;

char *script    = NULL;
char *ident     = PACKAGE;
char *prognm    = NULL;
char *pid_file  = NULL;
char *conf_file = NULL;
char *sock_file = NULL;
int   conf_vrfy = 0;

static uid_t uid = 0;
static gid_t gid = 0;

volatile sig_atomic_t reloading = 0;
volatile sig_atomic_t running   = 1;

static const char version_info[] = PACKAGE_NAME " v" PACKAGE_VERSION;


/* Cleans up, i.e. releases allocated resources. Called via atexit() */
static void clean(void)
{
	timer_exit();
	mroute_exit();
	mcgroup_exit();
	ipc_exit();
	iface_exit();
	smclog(LOG_NOTICE, "Exiting.");
}

void reload(void)
{
	notify_reload();

	mcgroup_reload_beg();
	mroute_reload_beg();

	iface_update();
	conf_read(conf_file, do_vifs);

	mroute_reload_end(do_vifs);
	mcgroup_reload_end();

	/* Acknowledge client SIGHUP/reload */
	notify_ready(NULL, uid, gid);
}

/*
 * Signal handler.  Take note of the fact that the signal arrived
 * so that the main loop can take care of it.
 */
static void handler(int signo)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		running = 0;
		break;

	case SIGHUP:
		reloading = 1;
		break;
	}
}

static void signal_init(void)
{
	struct sigaction sa;

	sa.sa_handler = handler;
	sa.sa_flags = 0;	/* Interrupt system calls */
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGHUP,  &sa, NULL)  ||
	    sigaction(SIGTERM, &sa, NULL) ||
	    sigaction(SIGINT,  &sa, NULL))
		smclog(LOG_WARNING, "Failed setting up signal handlers: %s", strerror(errno));
}

static void server_exit(void *arg)
{
	(void)arg;
	smclog(LOG_NOTICE, "Exit delay timer expired.");
	exit(0);
}

static int server_loop(void)
{
	script_init(script);
	mrdisc_init(interval);

	while (running) {
		if (reloading) {
			reload();
			reloading = 0;
		}

		socket_poll(NULL);
	}

	return 0;
}

/* Init everything before forking, so we can fail and return an
 * error code in the parent and the initscript will fail */
static int start_server(void)
{
	int api = 2, busy = 0;

	if (geteuid() != 0) {
		smclog(LOG_ERR, "Need root privileges to start %s", prognm);
		return EX_NOPERM;
	}

	if (background) {
		if (daemon(0, 0) < 0) {
			smclog(LOG_ERR, "Failed daemonizing: %s", strerror(errno));
			return EX_OSERR;
		}
	}

	/* Hello world! */
	smclog(LOG_NOTICE, "%s", version_info);

	if (startup_delay > 0) {
		smclog(LOG_INFO, "Startup delay requested, waiting %d sec before continuing.", startup_delay);
		sleep(startup_delay);
	}

	/*
	 * Timer API needs to be initilized before mroute_init()
	 */
	timer_init();

	if (exit_delay > 0) {
		smclog(LOG_INFO, "Exit delay requested, starting background timer, %d sec", exit_delay);
		timer_add(exit_delay, server_exit, NULL);
	}

	/*
	 * Build list of multicast-capable physical interfaces
	 */
	iface_init();

	if (mroute_init(do_vifs, table_id, cache_tmo)) {
		if (errno == EADDRINUSE)
			busy++;
		api--;
	}

	/* At least one API (IPv4 or IPv6) must have initialized successfully
	 * otherwise we abort the server initialization. */
	if (!api) {
		if (busy) {
			smclog(LOG_ERR, "Another multicast routing application is already running.");
			exit(EX_UNAVAILABLE);
		}

		smclog(LOG_ERR, "Kernel does not support multicast routing.");
		exit(EX_PROTOCOL);
	}

	atexit(clean);
	signal_init();
	mcgroup_init();
	ipc_init(sock_file);

	conf_read(conf_file, do_vifs);

	/* Everything setup, notify any clients waiting for us */
	notify_ready(pid_file, uid, gid);

	/* Drop root privileges before entering the server loop */
	cap_drop_root(uid, gid);

	return server_loop();
}

static void cleanup(void)
{
	if (conf_file)
		free(conf_file);
	conf_file = NULL;
	if (sock_file)
		free(sock_file);
	sock_file = NULL;
}

static int compose_paths(void)
{
	/* Default .conf file path: "/etc" + '/' + "smcroute" + ".conf" */
	if (!conf_file) {
		size_t len = strlen(SYSCONFDIR) + strlen(ident) + 7;

		conf_file = malloc(len);
		if (!conf_file) {
			smclog(LOG_ERR, "Failed allocating memory, exiting: %s", strerror(errno));
			exit(EX_OSERR);
		}

		snprintf(conf_file, len, "%s/%s.conf", SYSCONFDIR, ident);
	}

	if (!sock_file) {
		size_t len = strlen(RUNSTATEDIR) + strlen(ident) + 7;

		sock_file = malloc(len);
		if (!sock_file) {
			smclog(LOG_ERR, "Failed allocating memory, exiting: %s", strerror(errno));
			exit(EX_OSERR);
		}

		snprintf(sock_file, len, "%s/%s.sock", RUNSTATEDIR, ident);
	}

	/* Default is to let pidfile() API construct PID file from ident */
	if (!pid_file)
		pid_file = ident;

	atexit(cleanup);

	return 0;
}

static int usage(int code)
{
        char *pidfn;
	size_t len;

	compose_paths();
	len = sizeof(RUNSTATEDIR) + strlen(pid_file) + 6;
	pidfn = malloc(len);
	if (!pidfn)
		err(EX_OSERR, "Failed allocating memory for PID file name");

	if (pid_file[0] != '/')
		snprintf(pidfn, len, "%s/%s.pid", RUNSTATEDIR, pid_file);
	else
		snprintf(pidfn, len, "%s", pid_file);

	printf("Usage:\n"
	       "  %s [-hnNsv] [-c SEC] [-d SEC] [-e CMD] [-f FILE] [-i NAME] [-l LVL] "
	       "\n"
	       "                     "
#ifdef ENABLE_MRDISC
	       "[-m SEC] "
#endif
	       "[-P FILE] [-t ID] [-u FILE]\n"
	       "\n"
	       "Options:\n"
	       "  -c SEC          Flush dynamic (*,G) multicast routes every SEC seconds,\n"
	       "                  default 60 sec.  Useful when source/interface changes\n"
	       "  -d SEC          Startup delay, useful for delaying interface probe at boot\n"
	       "  -e CMD          Script or command to call on startup/reload when all routes\n"
	       "                  have been installed, or when a (*,G) is installed\n"
	       "  -f FILE         Configuration file, default use ident NAME: %s\n"
	       "  -F FILE         Check configuration file syntax, use -l to increase verbosity\n"
	       "  -h              This help text\n"
	       "  -i NAME         Identity for .conf/.pid/.sock file, and syslog, default: %s\n"
	       "  -l LVL          Set log level: none, err, notice*, info, debug\n"
#ifdef ENABLE_MRDISC
	       "  -m SEC          Multicast router discovery, 4-180, default: 20 sec\n"
#endif
	       "  -n              Run daemon in foreground, when started by systemd or finit\n"
	       "  -N              No multicast VIFs/MIFs created by default.  Use with\n"
	       "                  smcroute.conf `phyint enable` directive\n"
#ifdef ENABLE_LIBCAP
	       "  -p USER[:GROUP] After initialization set UID and GID to USER and GROUP\n"
#endif
	       "  -P FILE         Set daemon PID file name, with optional path.\n"
	       "                  Default use ident NAME: %s\n"
	       "  -s              Use syslog, default unless running in foreground, -n\n"
	       "  -t ID           Set multicast routing table ID, default: 0\n"
	       "  -u FILE         UNIX domain socket path, for use with smcroutectl.\n"
	       "                  Default use ident NAME: %s\n"
	       "  -v              Show program version and support information\n"
	       "\n", prognm, conf_file, ident, pidfn, sock_file);

	free(pidfn);
	cleanup();

	return code;
}

static char *progname(const char *arg0)
{
	char *nm;

	nm = strrchr(arg0, '/');
	if (nm)
		nm++;
	else
		nm = (char *)arg0;

	return nm;
}

/**
 * main - Main program
 *
 * Parses command line options and enters either daemon or client mode.
 *
 * In daemon mode, acquires multicast routing sockets, opens IPC socket
 * and goes in receive-execute command loop.
 *
 * In client mode, creates commands from command line and sends them to
 * the daemon.
 */
int main(int argc, char *argv[])
{
	int log_opts = LOG_NDELAY | LOG_PID;
	int c, new_log_level = -1;

	prognm = progname(argv[0]);
	while ((c = getopt(argc, argv, "c:d:D:e:f:F:hI:i:l:m:nNp:P:st:u:v")) != EOF) {
		switch (c) {
		case 'c':	/* cache timeout */
			cache_tmo = atoi(optarg);
			break;

		case 'd':
			startup_delay = atoi(optarg);
			break;

		case 'D':		/* Undocumented, used for testing. */
			exit_delay = atoi(optarg);
			break;

		case 'e':
			script = optarg;
			break;

		case 'F':
			log_level = LOG_INFO; /* Raise log level for verify */
			conf_vrfy = 1;
			/* fallthrough */
		case 'f':
			conf_file = strdup(optarg);
			break;

		case 'h':	/* help */
			return usage(EX_OK);

		case 'I':	/* compat with previous versions */
		case 'i':
			ident = optarg;
			break;

		case 'l':
			new_log_level = loglvl(optarg);
			break;

		case 'm':
#ifndef ENABLE_MRDISC
			warnx("Built without mrdisc support.");
#else
			interval = atoi(optarg);
			if (interval < 4 || interval > 180)
				errx(1, "Invalid mrdisc announcement interval, 4-180.");
#endif
			break;

		case 'n':	/* run daemon in foreground, i.e., do not fork */
			background = 0;
			do_syslog--;
			break;

		case 'N':
			do_vifs = 0;
			break;

		case 'p':
			cap_set_user(optarg, &uid, &gid);
			break;

		case 'P':
			/* Handled separately from the other files, no strdup() */
			pid_file = optarg;
			break;

		case 's':	/* Force syslog even though in foreground */
			do_syslog++;
			break;

		case 't':
#ifndef __linux__
			errx(1, "Different multicast routing tables only available on Linux.");
#else
			table_id = atoi(optarg);
			if (table_id < 0)
				return usage(EX_USAGE);
#endif
			break;

		case 'u':
			sock_file = strdup(optarg);
			break;

		case 'v':	/* version */
			puts(version_info);
			printf("\n"
			       "Bug report address: %s\n", PACKAGE_BUGREPORT);
#ifdef PACKAGE_URL
			printf("Project homepage:   %s\n", PACKAGE_URL);
#endif
			return EX_OK;

		default:	/* unknown option */
			return usage(EX_USAGE);
		}
	}

	if (new_log_level != -1)
		log_level = new_log_level;

	compose_paths();

	if (conf_vrfy) {
		smclog(LOG_INFO, "Verifying configuration file %s ...", conf_file);
		iface_init();
		c = conf_read(conf_file, do_vifs);
		iface_exit();

		return c;
	}

	if (!background && do_syslog < 1)
		log_opts |= LOG_PERROR;

	openlog(ident, log_opts, LOG_DAEMON);
	setlogmask(LOG_UPTO(log_level));

	return start_server();
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
