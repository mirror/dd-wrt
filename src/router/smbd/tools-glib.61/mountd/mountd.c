// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#define _GNU_SOURCE
#include <fcntl.h>

#include <tools.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "ipc.h"
#include "management/share.h"
#include "config_parser.h"

static void usage(int status)
{
	printf(
		"Usage: ksmbd.mountd [-v] [-p PORT] [-n[WAY]] [-C CONF] [-P PWDDB]\n");

	if (status != EXIT_SUCCESS)
		printf("Try `ksmbd.mountd --help' for more information.\n");
	else
		printf(
			"\n"
			"  -p, --port=PORT         bind to PORT instead of TCP port 445\n"
			"  -n, --nodetach[=WAY]    do not detach process from foreground;\n"
			"                          if WAY is 1, become process group leader (default);\n"
			"                          if WAY is 0, detach\n"
			"  -C, --config=CONF       use CONF as configuration file instead of\n"
			"                          `" PATH_SMBCONF "'\n"
			"  -P, --pwddb=PWDDB       use PWDDB as user database instead of\n"
			"                          `" PATH_PWDDB "'\n"
			"  -v, --verbose           be verbose\n"
			"  -V, --version           output version information and exit\n"
			"  -h, --help              display this help and exit\n"
			"\n"
			"See ksmbd.mountd(8) for more details.\n");
}

static struct option opts[] = {
	{"port",	required_argument,	NULL,	'p' },
	{"nodetach",	optional_argument,	NULL,	'n' },
	{"config",	required_argument,	NULL,	'C' },
	{"pwddb",	required_argument,	NULL,	'P' },
	{"verbose",	no_argument,		NULL,	'v' },
	{"version",	no_argument,		NULL,	'V' },
	{"help",	no_argument,		NULL,	'h' },
	{NULL,		0,			NULL,	 0  }
};

#define LIST_FMT_SS "%28s    %s"

static void __list_config_share_cb(struct ksmbd_share *share, int *wfd)
{
	dprintf(*wfd, LIST_FMT_SS "\n", share->name, share->comment ?: "");
}

static int list_config(int wfd)
{
	dprintf(wfd, "\e[1m" LIST_FMT_SS "\e[m" "\n", "Name", "Comment");
	shm_iter_shares((share_cb)__list_config_share_cb, &wfd);

	return kill(global_conf.pid, SIGUSR1) < 0 ? -errno : 0;
}

static void worker_sa_sigaction(int signo, siginfo_t *siginfo, void *ucontext)
{
	switch (signo) {
	case SIGIO:
	case SIGPIPE:
	case SIGCHLD:
		return;
	case SIGHUP:
		ksmbd_health_status |= KSMBD_SHOULD_RELOAD_CONFIG;
		return;
	case SIGUSR1:
		ksmbd_health_status |= KSMBD_SHOULD_LIST_CONFIG;
		return;
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:
		ksmbd_health_status &= ~KSMBD_HEALTH_RUNNING;
		return;
	}

	_Exit(128 + signo);
}

static void worker_init_sa_handler(sigset_t sigset)
{
	int signo;

	for (signo = 1; signo < _NSIG; signo++)
		if (sigismember(&sigset, signo)) {
			struct sigaction act = {
				.sa_sigaction = worker_sa_sigaction,
				.sa_flags = SA_SIGINFO,
			};

			sigfillset(&act.sa_mask);
			sigaction(signo, &act, NULL);
		}
}

static void __splice_pipe(int rfd, int wfd)
{
	if (wfd >= 0) {
		while (splice(rfd, NULL, wfd, NULL, PIPE_BUF, 0) > 0)
			;
	} else {
		char buf[PIPE_BUF];

		while (read(rfd, buf, sizeof(buf)) > 0)
			;
	}
}

static int worker_init_wait(pid_t pid, sigset_t sigset, int rfd)
{
	int ret = -ECHILD, wfd = -1;

	if (fcntl(rfd, F_SETFL, fcntl(rfd, F_GETFL) | O_ASYNC) < 0 ||
	    fcntl(rfd, F_SETOWN, global_conf.pid) < 0) {
		ret = -errno;
		pr_err("Can't control pipe: %m\n");
		return ret;
	}

	pr_info("Started worker\n");

	for (;;) {
		siginfo_t siginfo;

		if (sigwaitinfo(&sigset, &siginfo) < 0)
			continue;

		if (siginfo.si_signo == SIGIO) {
			__splice_pipe(rfd, wfd);
			continue;
		}

		if (siginfo.si_signo == SIGPIPE) {
			if (wfd >= 0) {
				close(wfd);
				wfd = -1;
			}
			continue;
		}

		if (siginfo.si_signo == SIGCHLD) {
			if (siginfo.si_code == CLD_KILLED)
				siginfo.si_status += 128;
			else if (siginfo.si_code != CLD_EXITED &&
				 siginfo.si_code != CLD_DUMPED)
				continue;
			if (siginfo.si_status > 128) {
				int signo = siginfo.si_status - 128;

				if (!sigismember(&sigset, signo))
					ret = -EIO;
				pr_err("Worker " "%s" "killed: %s\n",
				       ret == -EIO ? "fatally " : "",
				       strsignal(signo));
			} else if (siginfo.si_status != EXIT_SUCCESS) {
				ret = -EIO;
			}
			__splice_pipe(rfd, wfd);
			if (wfd >= 0)
				close(wfd);
			return ret;
		}

		if (siginfo.si_signo == SIGINT ||
		    siginfo.si_signo == SIGQUIT ||
		    siginfo.si_signo == SIGTERM) {
			ret = 0;
		} else if (siginfo.si_signo == SIGUSR1 &&
			   siginfo.si_pid == pid) {
			__splice_pipe(rfd, wfd);
			if (wfd >= 0) {
				close(wfd);
				wfd = -1;
			}
			continue;
		}

		if (kill(pid, siginfo.si_signo) < 0)
			continue;

		if (siginfo.si_signo == SIGUSR1) {
			g_autofree char *fifo_path =
				g_strdup_printf("%s.%d",
						PATH_FIFO,
						siginfo.si_pid);

			if (wfd >= 0)
				continue;

			wfd = open(fifo_path, O_WRONLY | O_NONBLOCK);
			if (wfd < 0)
				pr_err("Can't open `%s': %m\n", fifo_path);
		}
	}
}

static int worker_init(void)
{
	sigset_t sigset;
	pid_t pid;
	int fds[2], ret;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGIO);
	sigaddset(&sigset, SIGPIPE);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGHUP);
	sigaddset(&sigset, SIGUSR1);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGQUIT);
	sigaddset(&sigset, SIGTERM);
	sigaddset(&sigset, SIGABRT);
	pthread_sigmask(SIG_BLOCK, &sigset, NULL);

	if (pipe2(fds, O_NONBLOCK) < 0) {
		ret = -errno;
		pr_err("Can't create pipe: %m\n");
		return ret;
	}

	pid = fork();
	if (pid < 0) {
		ret = -errno;
		pr_err("Can't fork worker: %m\n");
		close(fds[1]);
		close(fds[0]);
		return ret;
	}
	if (pid > 0) {
		close(fds[1]);
		ret = worker_init_wait(pid, sigset, fds[0]);
		close(fds[0]);
		return ret;
	}

	close(fds[0]);
	worker_init_sa_handler(sigset);

	ret = load_config(global_conf.pwddb, global_conf.smbconf);
	if (ret)
		goto out;

	for (;;) {
		pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);

		ret = ipc_process_event();

		pthread_sigmask(SIG_BLOCK, &sigset, NULL);

		if (ret || !(ksmbd_health_status & KSMBD_HEALTH_RUNNING))
			goto out;

		if (ksmbd_health_status & KSMBD_SHOULD_RELOAD_CONFIG) {
			ret = load_config(global_conf.pwddb,
					  global_conf.smbconf);
			if (!ret) {
				pr_info("Reloaded config\n");
				ksmbd_health_status &=
					~KSMBD_SHOULD_RELOAD_CONFIG;
			}
		}

		if (ksmbd_health_status & KSMBD_SHOULD_LIST_CONFIG) {
			ret = list_config(fds[1]);
			if (!ret) {
				pr_info("Listed config\n");
				ksmbd_health_status &=
					~KSMBD_SHOULD_LIST_CONFIG;
			}
		}
	}

out:
	close(fds[1]);
	remove_config();
	return ret;
}

static int manager_init_wait(sigset_t sigset)
{
	pr_info("Started manager\n");

	for (;;) {
		siginfo_t siginfo;

		if (sigwaitinfo(&sigset, &siginfo) < 0)
			continue;

		if (siginfo.si_signo == SIGCHLD) {
			if (siginfo.si_code != CLD_KILLED &&
			    siginfo.si_code != CLD_EXITED &&
			    siginfo.si_code != CLD_DUMPED)
				continue;
			pr_err("Can't init manager, check syslog\n");
			return -ECHILD;
		}

		if (siginfo.si_signo == SIGUSR1) {
			if (siginfo.si_pid != global_conf.pid)
				continue;
			return 0;
		}
	}
}

static int manager_init(int nodetach)
{
	int signo;
	sigset_t sigset;
	int ret;

	for (signo = 1; signo < _NSIG; signo++) {
		struct sigaction act = {
			.sa_handler = SIG_DFL,
			.sa_flags = signo == SIGCHLD ? SA_NOCLDWAIT : 0,
		};

		sigfillset(&act.sa_mask);
		sigaction(signo, &act, NULL);
	}

	sigemptyset(&sigset);
	pthread_sigmask(SIG_SETMASK, &sigset, NULL);

	switch (nodetach) {
	case 0:
		sigaddset(&sigset, SIGCHLD);
		sigaddset(&sigset, SIGUSR1);
		pthread_sigmask(SIG_BLOCK, &sigset, NULL);

		global_conf.pid = fork();
		if (global_conf.pid < 0) {
			ret = -errno;
			pr_err("Can't fork manager: %m\n");
			return ret;
		}
		if (global_conf.pid > 0)
			return manager_init_wait(sigset);

		setsid();
		if (!freopen("/dev/null", "r", stdin) ||
		    !freopen("/dev/null", "w", stdout) ||
		    !freopen("/dev/null", "w", stderr)) {
			ret = -errno;
			pr_err("Can't redirect stream: %m\n");
			return ret;
		}
		pr_logger_init(PR_LOGGER_SYSLOG);

		pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
		break;
	case 1:
		setpgid(0, 0);
	}

	ret = cp_parse_lock();
	if (ret)
		return ret;

	if (cp_parse_subauth())
		pr_info("Ignored subauth file\n");

	if (!nodetach) {
		pid_t ppid = getppid();

		if (ppid == 1)
			return -ESRCH;
		if (kill(ppid, SIGUSR1) < 0) {
			ret = -errno;
			pr_err("Can't send SIGUSR1 to PID %d: %m\n", ppid);
			return ret;
		}
	}

	for (;;) {
		ret = worker_init();
		switch (ret) {
		case -ECHILD:
			sleep(1);
			continue;
		default:
			pr_info("Terminated\n");
			return ret;
		}
	}
}

int mountd_main(int argc, char **argv)
{
	int ret = -EINVAL;
	int nodetach = 0;
	int c;

	while ((c = getopt_long(argc, argv, "p:n::C:P:vVh", opts, NULL)) != EOF)
		switch (c) {
		case 'p':
			global_conf.tcp_port = cp_get_group_kv_long(optarg);
			break;
		case 'n':
			nodetach = !optarg ?: cp_get_group_kv_long(optarg);
			break;
		case 'C':
			g_free(global_conf.smbconf);
			global_conf.smbconf = g_strdup(optarg);
			break;
		case 'P':
			g_free(global_conf.pwddb);
			global_conf.pwddb = g_strdup(optarg);
			break;
		case 'v':
			set_log_level(PR_DEBUG);
			break;
		case 'V':
			ret = show_version();
			goto out;
		case 'h':
			ret = 0;
			/* Fall through */
		case '?':
		default:
			usage(ret ? EXIT_FAILURE : EXIT_SUCCESS);
			goto out;
		}

	if (argc > optind) {
		usage(ret ? EXIT_FAILURE : EXIT_SUCCESS);
		goto out;
	}

	if (!global_conf.smbconf)
		global_conf.smbconf = g_strdup(PATH_SMBCONF);

	if (!global_conf.pwddb)
		global_conf.pwddb = g_strdup(PATH_PWDDB);

	ret = manager_init(nodetach);
out:
	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}
