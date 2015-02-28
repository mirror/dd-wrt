/*
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996-2000 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */

#include "config.h"
#include "includes.h"
#include "radvd.h"
#include "pathnames.h"

#ifdef HAVE_NETLINK
#include "netlink.h"
#endif

#include <poll.h>
#include <sys/file.h>
#include <libgen.h>

#ifdef HAVE_GETOPT_LONG

/* *INDENT-OFF* */
static char usage_str[] = {
"\n"
"  -C, --config=PATH       Set the config file.  Default is /etc/radvd.d.\n"
"  -c, --configtest        Parse the config file and exit.\n"
"  -d, --debug=NUM         Set the debug level.  Values can be 1, 2, 3, 4 or 5.\n"
"  -f, --facility=NUM      Set the logging facility.\n"
"  -h, --help              Show this help screen.\n"
"  -l, --logfile=PATH      Set the log file.\n"
"  -m, --logmethod=X       Set method to: syslog, stderr, stderr_syslog, logfile, or none.\n"
"  -n, --nodaemon          Prevent the daemonizing.\n"
"  -p, --pidfile=PATH      Set the pid file.\n"
"  -t, --chrootdir=PATH    Chroot to the specified path.\n"
"  -u, --username=USER     Switch to the specified user.\n"
"  -v, --version           Print the version and quit.\n"
};

static struct option prog_opt[] = {
	{"chrootdir", 1, 0, 't'},
	{"config", 1, 0, 'C'},
	{"configtest", 0, 0, 'c'},
	{"debug", 1, 0, 'd'},
	{"facility", 1, 0, 'f'},
	{"help", 0, 0, 'h'},
	{"logfile", 1, 0, 'l'},
	{"logmethod", 1, 0, 'm'},
	{"nodaemon", 0, 0, 'n'},
	{"pidfile", 1, 0, 'p'},
	{"username", 1, 0, 'u'},
	{"version", 0, 0, 'v'},
	{NULL, 0, 0, 0}
};

#else

static char usage_str[] = {
"[-hsvcn] [-d level] [-C config_path] [-m log_method] [-l log_file]\n"
"\t[-f facility] [-p pid_file] [-u username] [-t chrootdir]"

};
/* *INDENT-ON* */

#endif

static volatile int sighup_received = 0;
static volatile int sigint_received = 0;
static volatile int sigterm_received = 0;
static volatile int sigusr1_received = 0;

static int check_conffile_perm(const char *, const char *);
static int drop_root_privileges(const char *);
static int open_and_lock_pid_file(char const * daemon_pid_file_ident);
static int write_pid_file(char const * daemon_pid_file_ident, pid_t pid);
static pid_t daemonp(int nochdir, int noclose, char const * daemon_pid_file_ident);
static pid_t do_daemonize(int log_method, char const * daemon_pid_file_ident);
static struct Interface * main_loop(int sock, struct Interface *ifaces, char const *conf_path);
static struct Interface *reload_config(int sock, struct Interface *ifaces, char const *conf_path);
static void check_pid_file(char const * daemon_pid_file_ident);
static void config_interface(struct Interface *iface);
static void kickoff_adverts(int sock, struct Interface *iface);
static void reset_prefix_lifetimes(struct Interface *ifaces);
static void reset_prefix_lifetimes_foo(struct Interface *iface, void *data);
static void setup_iface_foo(struct Interface *iface, void *data);
static void setup_ifaces(int sock, struct Interface *ifaces);
static void sighup_handler(int sig);
static void sigint_handler(int sig);
static void sigterm_handler(int sig);
static void sigusr1_handler(int sig);
static void stop_advert_foo(struct Interface *iface, void *data);
static void stop_adverts(int sock, struct Interface *ifaces);
static void timer_handler(int sock, struct Interface *iface);
static void usage(char const *pname);
static void version(void);

/* daemonize and write pid file.  The pid of the daemon child process
 * will be written to the pid file from the *parent* process.  This
 * insures there is no race condition as described in redhat bug 664783. */
static pid_t daemonp(int nochdir, int noclose, char const * daemon_pid_file_ident)
{
	int pipe_ends[2];

	if (0 != pipe(pipe_ends)) {
		flog(LOG_ERR, "unable to create pipe: %s", strerror(errno));
		exit(-1);
	}

	pid_t pid = fork();

	if (-1 == pid) {
		flog(LOG_ERR, "unable to fork in daemonp");
		exit(-1);
	} else if (0 == pid) {
		/* Child process, detached.. */
		pid = getpid();
		close(pipe_ends[0]);
		if (0 != write_pid_file(daemon_pid_file_ident, pid)) {
			flog(LOG_ERR, "failure writing pid file");
			exit(-1);
		}
		if (sizeof(pid) != write(pipe_ends[1], &pid, sizeof(pid))) {
			flog(LOG_ERR, "failure piping pid to parent process");
		}

		umask(0);
		if (-1 == setsid()) {
			flog(LOG_ERR, "unable to become a session leader: %s", strerror(errno));
			exit(-1);
		}

		if (nochdir == 0) {
			if (chdir("/") == -1) {
				perror("chdir");
				exit(1);
			}
		}
		if (noclose == 0) {
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			if (open("/dev/null", O_RDONLY) == -1) {
				flog(LOG_ERR, "unable to redirect stdin to /dev/null");
				exit(-1);
			}
			if (open("/dev/null", O_WRONLY) == -1) {
				flog(LOG_ERR, "unable to redirect stdout to /dev/null");
				exit(-1);
			}
			if (open("/dev/null", O_RDWR) == -1) {
				flog(LOG_ERR, "unable to redirect stderr to /dev/null");
				exit(-1);
			}
		}
	} else {
		/* Parent.  Make sure the pid file is written before exiting. */
		close(pipe_ends[1]);
		pid_t msg = -1;
		ssize_t rc = read(pipe_ends[0], &msg, sizeof(msg));
		if (rc != sizeof(msg)) {
			flog(LOG_ERR, "child failed to signal pid file written: %s", strerror(errno));
			exit(-1);
		} else if (msg != pid) {
			flog(LOG_ERR, "child wrote wrong pid to pid file: %d", msg);
			exit(-1);
		} else {
			dlog(LOG_DEBUG, 5, "child signaled pid file written: %d", msg);
		}
		close(pipe_ends[0]);

		return pid;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int c;
	int log_method = L_STDERR_SYSLOG;
	char *logfile = PATH_RADVD_LOG;
	int facility = LOG_FACILITY;
	char *username = NULL;
	char *chrootdir = NULL;
	int configtest = 0;
	int daemonize = 1;

	char const *pname = ((pname = strrchr(argv[0], '/')) != NULL) ? pname + 1 : argv[0];

	srand((unsigned int)time(NULL));

	char const *conf_path = PATH_RADVD_CONF;
	char const *daemon_pid_file_ident = PATH_RADVD_PID;

	/* parse args */
#define OPTIONS_STR "d:C:l:m:p:t:u:vhcn"
#ifdef HAVE_GETOPT_LONG
	int opt_idx;
	while ((c = getopt_long(argc, argv, OPTIONS_STR, prog_opt, &opt_idx)) > 0)
#else
	while ((c = getopt(argc, argv, OPTIONS_STR)) > 0)
#endif
	{
		switch (c) {
		case 'C':
			conf_path = optarg;
			break;
		case 'd':
			set_debuglevel(atoi(optarg));
			break;
		case 'f':
			facility = atoi(optarg);
			break;
		case 'l':
			logfile = optarg;
			break;
		case 'p':
			daemon_pid_file_ident = optarg;
			break;
		case 'm':
			if (!strcmp(optarg, "syslog")) {
				log_method = L_SYSLOG;
			} else if (!strcmp(optarg, "stderr_syslog")) {
				log_method = L_STDERR_SYSLOG;
			} else if (!strcmp(optarg, "stderr")) {
				log_method = L_STDERR;
			} else if (!strcmp(optarg, "logfile")) {
				log_method = L_LOGFILE;
			} else if (!strcmp(optarg, "none")) {
				log_method = L_NONE;
			} else {
				fprintf(stderr, "%s: unknown log method: %s\n", pname, optarg);
				exit(1);
			}
			break;
		case 't':
			chrootdir = strdup(optarg);
			break;
		case 'u':
			username = strdup(optarg);
			break;
		case 'v':
			version();
			break;
		case 'c':
			configtest = 1;
			break;
		case 'n':
			daemonize = 0;
			break;
		case 'h':
			usage(pname);
#ifdef HAVE_GETOPT_LONG
		case ':':
			fprintf(stderr, "%s: option %s: parameter expected\n", pname, prog_opt[opt_idx].name);
			exit(1);
#endif
		case '?':
			exit(1);
		}
	}

	/* TODO: Seems like this chroot'ing should happen *after* daemonizing for
	 * the sake of the PID file. */
	if (chrootdir) {
		if (!username) {
			fprintf(stderr, "Chroot as root is not safe, exiting\n");
			exit(1);
		}

		if (chroot(chrootdir) == -1) {
			perror("chroot");
			exit(1);
		}

		if (chdir("/") == -1) {
			perror("chdir");
			exit(1);
		}
		/* username will be switched later */
	}

	if (configtest) {
		set_debuglevel(1);
		log_method = L_STDERR;
	}

	if (log_open(log_method, pname, logfile, facility) < 0) {
		perror("log_open");
		exit(1);
	}

	if (!configtest) {
		flog(LOG_INFO, "version %s started", VERSION);
	}

	/* check that 'other' cannot write the file
	 * for non-root, also that self/own group can't either
	 */
	if (check_conffile_perm(username, conf_path) != 0) {
		if (get_debuglevel() == 0) {
			flog(LOG_ERR, "exiting, permissions on conf_file invalid");
			exit(1);
		} else
			flog(LOG_WARNING, "Insecure file permissions, but continuing anyway");
	}

	/* parse config file */
	struct Interface *ifaces = NULL;
	if ((ifaces = readin_config(conf_path)) == 0) {
		flog(LOG_ERR, "exiting, failed to read config file");
		exit(1);
	}

	if (configtest) {
		free_ifaces(ifaces);
		exit(0);
	}

	/* get a raw socket for sending and receiving ICMPv6 messages */
	int sock = open_icmpv6_socket();
	if (sock < 0) {
		perror("open_icmpv6_socket");
		exit(1);
	}

	/* if we know how to do it, check whether forwarding is enabled */
	if (check_ip6_forwarding()) {
		flog(LOG_WARNING, "IPv6 forwarding seems to be disabled, but continuing anyway");
	}

	int const pidfd = open_and_lock_pid_file(daemon_pid_file_ident);

	/*
	 * okay, config file is read in, socket and stuff is setup, so
	 * lets fork now...
	 */
	if (daemonize) {
		pid_t pid = do_daemonize(log_method, daemon_pid_file_ident);
		if (pid != 0 && pid != -1) {
			/* We want to see clean output from valgrind, so free username, chrootdir,
			 * and ifaces in the child process. */
			if (ifaces)
				free_ifaces(ifaces);

			if (username)
				free(username);

			if (chrootdir)
				free(chrootdir);

			exit(0);
		}
	} else {
		if (0 != write_pid_file(daemon_pid_file_ident, getpid())) {
			flog(LOG_ERR, "failure writing pid file detected");
			exit(-1);
		}
	}

	check_pid_file(daemon_pid_file_ident);

#ifdef __linux__
	/* for privsep */ {
		dlog(LOG_DEBUG, 3, "initializing privsep");

		int pipefds[2];

		if (pipe(pipefds) != 0) {
			flog(LOG_ERR, "Couldn't create privsep pipe.");
			return -1;
		}

		pid_t pid = fork();

		if (pid == -1) {
			flog(LOG_ERR, "Couldn't fork for privsep.");
			return -1;
		}

		if (pid == 0) {
			/* We want to see clean output from valgrind, so free username, chrootdir,
			 * and ifaces in the child process. */
			if (ifaces)
				free_ifaces(ifaces);

			if (username)
				free(username);

			if (chrootdir)
				free(chrootdir);

			close(pipefds[1]);

			privsep_init(pipefds[0]);
			_exit(0);
		}

		dlog(LOG_DEBUG, 3, "radvd privsep PID is %d", pid);

		/* Continue execution (will drop privileges soon) */
		close(pipefds[0]);
		privsep_set_write_fd(pipefds[1]);
	}
#endif

	if (username) {
		if (drop_root_privileges(username) < 0) {
			perror("drop_root_privileges");
			flog(LOG_ERR, "unable to drop root privileges");
			exit(1);
		}
		dlog(LOG_DEBUG, 3, "running as user: %s", username);
	}

	setup_ifaces(sock, ifaces);
	ifaces = main_loop(sock, ifaces, conf_path);
	stop_adverts(sock, ifaces);

	flog(LOG_INFO, "removing %s", daemon_pid_file_ident);
	unlink(daemon_pid_file_ident);
	close(pidfd);

	if (ifaces)
		free_ifaces(ifaces);

	if (chrootdir)
		free(chrootdir);

	if (username)
		free(username);

	flog(LOG_INFO, "returning from radvd main");
	log_close();

	return 0;
}

static struct Interface * main_loop(int sock, struct Interface *ifaces, char const *conf_path)
{
	struct pollfd fds[2];
	sigset_t sigmask;
	sigset_t sigempty;
	struct sigaction sa;

	sigemptyset(&sigempty);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGHUP);
	sigaddset(&sigmask, SIGTERM);
	sigaddset(&sigmask, SIGINT);
	sigaddset(&sigmask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigmask, NULL);

	sa.sa_handler = sighup_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGHUP, &sa, 0);

	sa.sa_handler = sigterm_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, 0);

	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, 0);

	sa.sa_handler = sigusr1_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, 0);

	memset(fds, 0, sizeof(fds));

	fds[0].fd = sock;
	fds[0].events = POLLIN;

#if HAVE_NETLINK
	fds[1].fd = netlink_socket();
	fds[1].events = POLLIN;
#else
	fds[1].fd = -1;
#endif

	for (;;) {
		struct timespec *tsp = 0;

		struct Interface *next_iface_to_expire = find_iface_by_time(ifaces);
		if (next_iface_to_expire) {
			static struct timespec ts;
			int timeout = next_time_msec(next_iface_to_expire);
			ts.tv_sec = timeout / 1000;
			ts.tv_nsec = (timeout - 1000 * ts.tv_sec) * 1000000;
			tsp = &ts;
			dlog(LOG_DEBUG, 1, "polling for %g second(s), next iface is %s", timeout / 1000.0,
			     next_iface_to_expire->props.name);
		} else {
			dlog(LOG_DEBUG, 1, "no iface is next. Polling indefinitely");
		}
#ifdef HAVE_PPOLL
		int rc = ppoll(fds, sizeof(fds) / sizeof(fds[0]), tsp, &sigempty);
#else
		int rc = poll(fds, sizeof(fds) / sizeof(fds[0]), 1000*tsp->tv_sec);
#endif

		if (rc > 0) {
#ifdef HAVE_NETLINK
			if (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				flog(LOG_WARNING, "socket error on fds[1].fd");
			} else if (fds[1].revents & POLLIN) {
				process_netlink_msg(fds[1].fd, ifaces);
			}
#endif

			if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				flog(LOG_WARNING, "socket error on fds[0].fd");
			} else if (fds[0].revents & POLLIN) {
				int len, hoplimit;
				struct sockaddr_in6 rcv_addr;
				struct in6_pktinfo *pkt_info = NULL;
				unsigned char msg[MSG_SIZE_RECV];
				unsigned char chdr[CMSG_SPACE(sizeof(struct in6_pktinfo)) + CMSG_SPACE(sizeof(int))];

				len = recv_rs_ra(sock, msg, &rcv_addr, &pkt_info, &hoplimit, chdr);
				if (len > 0 && pkt_info) {
					process(sock, ifaces, msg, len, &rcv_addr, pkt_info, hoplimit);
				} else if (!pkt_info) {
					dlog(LOG_INFO, 4, "recv_rs_ra returned null pkt_info");
				} else if (len <= 0) {
					dlog(LOG_INFO, 4, "recv_rs_ra returned len <= 0: %d", len);
				}
			}
		} else if (rc == 0) {
			if (next_iface_to_expire)
				timer_handler(sock, next_iface_to_expire);
		} else if (rc == -1) {
			dlog(LOG_INFO, 3, "poll returned early: %s", strerror(errno));
		}

		if (sigint_received) {
			flog(LOG_WARNING, "exiting, %d sigint(s) received", sigint_received);
			break;
		}

		if (sigterm_received) {
			flog(LOG_WARNING, "exiting, %d sigterm(s) received", sigterm_received);
			break;
		}

		if (sighup_received) {
			dlog(LOG_INFO, 3, "sig hup received");
			ifaces = reload_config(sock, ifaces, conf_path);
			sighup_received = 0;
		}

		if (sigusr1_received) {
			dlog(LOG_INFO, 3, "sig usr1 received");
			reset_prefix_lifetimes(ifaces);
			sigusr1_received = 0;
		}

	}

	return ifaces;
}

static pid_t do_daemonize(int log_method, char const * daemon_pid_file_ident)
{
	pid_t pid = -1;

	if (L_STDERR_SYSLOG == log_method || L_STDERR == log_method) {
		pid = daemonp(1, 1, daemon_pid_file_ident);
	} else {
		pid = daemonp(0, 0, daemon_pid_file_ident);
	}

	if (-1 == pid) {
		flog(LOG_ERR, "unable to daemonize: %s", strerror(errno));
		exit(-1);
	}
	return pid;
}

static int open_pid_file(char const * daemon_pid_file_ident)
{
	int pidfd = open(daemon_pid_file_ident, O_SYNC | O_CREAT | O_RDWR, 0644);
	if (-1 == pidfd) {
		flog(LOG_ERR, "unable to open pid file, %s: %s", daemon_pid_file_ident, strerror(errno));
		exit(-1);
	} else {
		dlog(LOG_DEBUG, 5, "opened pid file %s", daemon_pid_file_ident);
	}
	return pidfd;
}

static int open_and_lock_pid_file(char const * daemon_pid_file_ident)
{
	dlog(LOG_DEBUG, 3, "radvd startup PID is %d", getpid());

	int pidfd = open_pid_file(daemon_pid_file_ident);

	int lock = flock(pidfd, LOCK_EX | LOCK_NB);
	if (0 != lock) {
		flog(LOG_ERR, "unable to lock pid file, %s: %s", daemon_pid_file_ident, strerror(errno));
		exit(-1);
	} else {
		dlog(LOG_DEBUG, 4, "locked pid file %s", daemon_pid_file_ident);
	}

	return pidfd;
}

static int write_pid_file(char const * daemon_pid_file_ident, pid_t pid)
{
	int pidfd = open_pid_file(daemon_pid_file_ident);
	char pid_str[20] = {""};
	sprintf(pid_str, "%d", pid);
	dlog(LOG_DEBUG, 3, "radvd PID is %s", pid_str);
	size_t len = strlen(pid_str);
	int rc = write(pidfd, pid_str, len);
	if (rc != (int)len) {
		return -1;
	}
	char newline[] = {"\n"};
	len = strlen(newline);
	rc = write(pidfd, newline, len);
	if (rc != (int)len) {
		return -1;
	}
	rc = fsync(pidfd);
	if (rc != 0) {
		dlog(LOG_DEBUG, 4, "failed to fsync pid file: %s", daemon_pid_file_ident);
	}
	rc = close(pidfd);
	if (rc != 0) {
		dlog(LOG_DEBUG, 4, "failed to close pid file: %s", daemon_pid_file_ident);
	}
	char * dirstrcopy = strdup(daemon_pid_file_ident);
	char * dirstr = dirname(dirstrcopy);
	int dirfd = open(dirstr, O_RDONLY);
	rc = fsync(dirfd);
	if (rc != 0) {
		dlog(LOG_DEBUG, 4, "failed to fsync pid dir: %s", dirstr);
	}
	rc = close(dirfd);
	if (rc != 0) {
		dlog(LOG_DEBUG, 4, "failed to close pid dir: %s", dirstr);
	}
	free(dirstrcopy);
	dlog(LOG_DEBUG, 4, "wrote pid %d to pid file: %s", pid, daemon_pid_file_ident);
	return rc;
}

static void check_pid_file(char const * daemon_pid_file_ident)
{
	FILE * pidfile = fopen(daemon_pid_file_ident, "r");

	if (!pidfile) {
		flog(LOG_ERR, "unable to open pid file, %s: %s", daemon_pid_file_ident, strerror(errno));
		exit(-1);
	}

	pid_t pid = -1;

	int rc = fscanf(pidfile, "%d", &pid);
	fclose(pidfile);

	if (rc != 1) {
		flog(LOG_ERR, "unable to read pid from pid file: %s", daemon_pid_file_ident);
		exit(-1);
	}

	if (pid != getpid()) {
		flog(LOG_ERR, "pid in file, %s, doesn't match getpid(): %d != %d", daemon_pid_file_ident, pid, getpid());
		exit(-1);
	}
	dlog(LOG_DEBUG, 4, "validated pid file, %s: %d", daemon_pid_file_ident, pid);
}


static void timer_handler(int sock, struct Interface *iface)
{
	dlog(LOG_DEBUG, 1, "timer_handler called for %s", iface->props.name);

	if (send_ra_forall(sock, iface, NULL) != 0) {
		dlog(LOG_DEBUG, 4, "send_ra_forall failed on interface %s", iface->props.name);
	}

	double next = rand_between(iface->MinRtrAdvInterval, iface->MaxRtrAdvInterval);

	reschedule_iface(iface, next);
}

static void config_interface(struct Interface *iface)
{
	if (iface->AdvLinkMTU)
		set_interface_linkmtu(iface->props.name, iface->AdvLinkMTU);
	if (iface->ra_header_info.AdvCurHopLimit)
		set_interface_curhlim(iface->props.name, iface->ra_header_info.AdvCurHopLimit);
	if (iface->ra_header_info.AdvReachableTime)
		set_interface_reachtime(iface->props.name, iface->ra_header_info.AdvReachableTime);
	if (iface->ra_header_info.AdvRetransTimer)
		set_interface_retranstimer(iface->props.name, iface->ra_header_info.AdvRetransTimer);
}

/*
 *      send initial advertisement and set timers
 */
static void kickoff_adverts(int sock, struct Interface *iface)
{
	clock_gettime(CLOCK_MONOTONIC, &iface->times.last_ra_time);

	if (iface->UnicastOnly)
		return;

	clock_gettime(CLOCK_MONOTONIC, &iface->times.last_multicast);

	/* send an initial advertisement */
	if (send_ra_forall(sock, iface, NULL) != 0) {
		dlog(LOG_DEBUG, 4, "send_ra_forall failed on interface %s", iface->props.name);
	}

	double next = min(MAX_INITIAL_RTR_ADVERT_INTERVAL, iface->MaxRtrAdvInterval);
	reschedule_iface(iface, next);
}

static void stop_advert_foo(struct Interface *iface, void *data)
{
	if (!iface->UnicastOnly) {
		/* send a final advertisement with zero Router Lifetime */
		dlog(LOG_DEBUG, 4, "stopping all adverts on %s", iface->props.name);
		iface->state_info.cease_adv = 1;
		int sock = *(int *)data;
		send_ra_forall(sock, iface, NULL);
	}
}

static void stop_adverts(int sock, struct Interface *ifaces)
{
	flog(LOG_INFO, "sending stop adverts");
	/*
	 *      send final RA (a SHOULD in RFC4861 section 6.2.5)
	 */
	for_each_iface(ifaces, stop_advert_foo, &sock);
}

static void setup_iface_foo(struct Interface *iface, void *data)
{
	int sock = *(int *)data;

	if (setup_iface(sock, iface) < 0) {
		if (iface->IgnoreIfMissing) {
			dlog(LOG_DEBUG, 4, "interface %s does not exist or is not set up properly, ignoring the interface",
			     iface->props.name);
			return;
		} else {
			flog(LOG_ERR, "interface %s does not exist or is not set up properly", iface->props.name);
			exit(1);
		}
	}

	config_interface(iface);
	kickoff_adverts(sock, iface);
}

static void setup_ifaces(int sock, struct Interface *ifaces)
{
	for_each_iface(ifaces, setup_iface_foo, &sock);
}

static struct Interface *reload_config(int sock, struct Interface *ifaces, char const *conf_path)
{
	free_ifaces(ifaces);

	flog(LOG_INFO, "attempting to reread config file");

	/* reread config file */
	ifaces = readin_config(conf_path);
	if (!ifaces) {
		flog(LOG_ERR, "exiting, failed to read config file");
		exit(1);
	}
	setup_ifaces(sock, ifaces);

	flog(LOG_INFO, "resuming normal operation");

	return ifaces;
}

static void sighup_handler(int sig)
{
	sighup_received = 1;
}

static void sigterm_handler(int sig)
{
	++sigterm_received;

	if (sigterm_received > 2) {
		abort();
	}
}

static void sigint_handler(int sig)
{
	++sigint_received;

	if (sigint_received > 2) {
		abort();
	}
}

static void sigusr1_handler(int sig)
{
	sigusr1_received = 1;
}

static void reset_prefix_lifetimes_foo(struct Interface *iface, void *data)
{
	flog(LOG_INFO, "Resetting prefix lifetimes on %s", iface->props.name);

	for (struct AdvPrefix * prefix = iface->AdvPrefixList; prefix; prefix = prefix->next) {
		if (prefix->DecrementLifetimesFlag) {
			char pfx_str[INET6_ADDRSTRLEN];
			addrtostr(&prefix->Prefix, pfx_str, sizeof(pfx_str));
			dlog(LOG_DEBUG, 4, "%s/%u%%%s plft reset from %u to %u secs", pfx_str, prefix->PrefixLen,
			     iface->props.name, prefix->curr_preferredlft, prefix->AdvPreferredLifetime);
			dlog(LOG_DEBUG, 4, "%s/%u%%%s vlft reset from %u to %u secs", pfx_str, prefix->PrefixLen,
			     iface->props.name, prefix->curr_validlft, prefix->AdvValidLifetime);
			prefix->curr_validlft = prefix->AdvValidLifetime;
			prefix->curr_preferredlft = prefix->AdvPreferredLifetime;
		}
	}
}

static void reset_prefix_lifetimes(struct Interface *ifaces)
{
	for_each_iface(ifaces, reset_prefix_lifetimes_foo, 0);
}

static int drop_root_privileges(const char *username)
{
	struct passwd *pw = getpwnam(username);
	if (pw) {
		if (initgroups(username, pw->pw_gid) != 0 || setgid(pw->pw_gid) != 0 || setuid(pw->pw_uid) != 0) {
			flog(LOG_ERR, "Couldn't change to '%.32s' uid=%d gid=%d", username, pw->pw_uid, pw->pw_gid);
			return -1;
		}
	} else {
		flog(LOG_ERR, "Couldn't find user '%.32s'", username);
		return -1;
	}
	return 0;
}

static int check_conffile_perm(const char *username, const char *conf_file)
{
	FILE *fp = fopen(conf_file, "r");
	if (fp == NULL) {
		flog(LOG_ERR, "can't open %s: %s", conf_file, strerror(errno));
		return -1;
	}
	fclose(fp);

	if (!username)
		username = "root";

	struct passwd *pw = getpwnam(username);
	if (!pw) {
		return -1;
	}

	struct stat stbuf;
	if (0 != stat(conf_file, &stbuf)) {
		return -1;
	}

	if (stbuf.st_mode & S_IWOTH) {
		flog(LOG_ERR, "Insecure file permissions (writable by others): %s", conf_file);
		return -1;
	}

	/* for non-root: must not be writable by self/own group */
	if (strncmp(username, "root", 5) != 0 && ((stbuf.st_mode & S_IWGRP && pw->pw_gid == stbuf.st_gid)
						  || (stbuf.st_mode & S_IWUSR && pw->pw_uid == stbuf.st_uid))) {
		flog(LOG_ERR, "Insecure file permissions (writable by self/group): %s", conf_file);
		return -1;
	}

	return 0;
}

static void version(void)
{
	fprintf(stderr, "Version: %s\n\n", VERSION);
	fprintf(stderr, "Compiled in settings:\n");
	fprintf(stderr, "  default config file		\"%s\"\n", PATH_RADVD_CONF);
	fprintf(stderr, "  default pidfile		\"%s\"\n", PATH_RADVD_PID);
	fprintf(stderr, "  default logfile		\"%s\"\n", PATH_RADVD_LOG);
	fprintf(stderr, "  default syslog facility	%d\n", LOG_FACILITY);
	fprintf(stderr, "Please send bug reports or suggestions to %s.\n", CONTACT_EMAIL);

	exit(1);
}

static void usage(char const *pname)
{
	fprintf(stderr, "usage: %s %s\n", pname, usage_str);
	exit(1);
}
