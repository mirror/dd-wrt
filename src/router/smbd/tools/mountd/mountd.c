// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <ksmbdtools.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <ipc.h>
#include <rpc.h>
#include <worker.h>
#include <config_parser.h>
#include <management/user.h>
#include <management/share.h>
#include <management/session.h>
#include <management/tree_conn.h>

static int no_detach;
int usmbd_health_status;
static pid_t worker_pid;
static int lock_fd = -1;
static char *pwddb = PATH_PWDDB;
static char *smbconf = PATH_SMBCONF;

typedef int (*worker_fn)(void);

static void usage(void)
{
	fprintf(stderr, "Usage: usmbd\n");
	fprintf(stderr, "\t--p=NUM | --port=NUM              TCP port NUM\n");
	fprintf(stderr, "\t--c=smb.conf | --config=smb.conf  config file\n");
	fprintf(stderr, "\t--u=pwd.db | --users=pwd.db       Users DB\n");
	fprintf(stderr, "\t--n | --nodetach                  Don't detach\n");
	fprintf(stderr, "\t--s | --systemd                   Service mode\n");
	fprintf(stderr, "\t-V | --version                    Show version\n");
	fprintf(stderr, "\t-h | --help                       Show help\n");

	exit(EXIT_FAILURE);
}

static void show_version(void)
{
	printf("ksmbd-tools version : %s\n", KSMBD_TOOLS_VERSION);
	exit(EXIT_FAILURE);
}

static int handle_orphaned_lock_file(void)
{
	char proc_ent[64] = {0, };
	char manager_pid[10] = {0, };
	int pid = 0;
	int fd;

	fd = open(USMBD_LOCK_FILE, O_RDONLY);
	if (fd < 0)
		return -EINVAL;

	if (read(fd, &manager_pid, sizeof(manager_pid)) == -1) {
		pr_debug("Unable to read main PID: %s\n", strerr(errno));
		close(fd);
		return -EINVAL;
	}

	close(fd);

	pid = strtol(manager_pid, NULL, 10);
	snprintf(proc_ent, sizeof(proc_ent), "/proc/%d", pid);
	fd = open(proc_ent, O_RDONLY);
	if (fd < 0) {
		pr_info("Unlink orphaned '%s'\n", USMBD_LOCK_FILE);
		return unlink(USMBD_LOCK_FILE);
	}

	close(fd);
	pr_info("File '%s' belongs to pid %d\n", USMBD_LOCK_FILE, pid);
	return -EINVAL;
}

static int create_lock_file(void)
{
	char manager_pid[10];
	size_t sz;

retry:
	lock_fd = open(USMBD_LOCK_FILE, O_CREAT | O_EXCL | O_WRONLY,
			S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
	if (lock_fd < 0) {
		if (handle_orphaned_lock_file())
			return -EINVAL;
		goto retry;
	}

	if (flock(lock_fd, LOCK_EX | LOCK_NB) != 0)
		return -EINVAL;

	sz = snprintf(manager_pid, sizeof(manager_pid), "%d", getpid());
	if (write(lock_fd, manager_pid, sz) == -1)
		pr_err("Unable to record main PID: %s\n", strerr(errno));
	return 0;
}

static void delete_lock_file(void)
{
	if (lock_fd == -1)
		return;

	flock(lock_fd, LOCK_UN);
	close(lock_fd);
	lock_fd = -1;
	remove(USMBD_LOCK_FILE);
}

static int wait_group_kill(int signo)
{
	pid_t pid;
	int status;

	if (kill(worker_pid, signo) != 0)
		pr_err("can't execute kill %d: %s\n",
			worker_pid,
			strerr(errno));

	while (1) {
		pid = waitpid(-1, &status, 0);
		if (pid != 0) {
			pr_debug("detected pid %d termination\n", pid);
			break;
		}
		sleep(1);
	}
	return 0;
}

static int setup_signal_handler(int signo, sighandler_t handler)
{
	int status;
	sigset_t full_set;
	struct sigaction act = {};

	sigfillset(&full_set);

	act.sa_handler = handler;
	act.sa_mask = full_set;

	status = sigaction(signo, &act, NULL);
	if (status != 0)
		pr_err("Unable to register %s signal handler: %s",
				strsignal(signo), strerr(errno));
	return status;
}

static int setup_signals(sighandler_t handler)
{
	if (setup_signal_handler(SIGHUP, handler) != 0)
		return -EINVAL;
	int airbag_init(void);
	airbag_init();

/*	if (setup_signal_handler(SIGINT, handler) != 0)
		return -EINVAL;

	if (setup_signal_handler(SIGTERM, handler) != 0)
		return -EINVAL;

	if (setup_signal_handler(SIGABRT, handler) != 0)
		return -EINVAL;

	if (setup_signal_handler(SIGQUIT, handler) != 0)
		return -EINVAL;

	if (setup_signal_handler(SIGHUP, handler) != 0)
		return -EINVAL;

	if (setup_signal_handler(SIGSEGV, handler) != 0)
		return -EINVAL;
*/
	return 0;
}

static int parse_configs(char *pwddb, char *smbconf)
{
	int ret;

	ret = cp_parse_pwddb(pwddb);
	if (ret == -ENOENT) {
		pr_err("User database file does not exist. %s\n",
			"Only guest sessions (if permitted) will work.");
	} else if (ret) {
		pr_err("Unable to parse user database\n");
		return ret;
	}

	ret = cp_parse_smbconf(smbconf);
	if (ret) {
		pr_err("Unable to parse smb configuration file\n");
		return ret;
	}
	return 0;
}

static void worker_process_free(void)
{
	/*
	 * NOTE, this is the final release, we don't look at ref_count
	 * values. User management should be destroyed last.
	 */
	ipc_destroy();
	rpc_destroy();
	sm_destroy();
	wp_destroy();
	shm_destroy();
	usm_destroy();
}

static void child_sig_handler(int signo)
{
	static volatile int fatal_delivered;

	if (signo == SIGHUP) {
		/*
		 * This is a signal handler, we can't take any locks, set
		 * a flag and wait for normal execution context to re-read
		 * the configs.
		 */
		usmbd_health_status |= USMBD_SHOULD_RELOAD_CONFIG;
		pr_debug("Scheduled a config reload action.\n");
		return;
	}

	pr_err("Child received signal: %d (%s)\n",
		signo, strsignal(signo));

	if (!atomic_int_compare_and_exchange(&fatal_delivered, 0, 1))
		return;

	usmbd_health_status &= ~USMBD_HEALTH_RUNNING;
	worker_process_free();
	exit(EXIT_SUCCESS);
}

static void manager_sig_handler(int signo)
{
	/*
	 * Pass SIGHUP to worker, so it will reload configs
	 */
	if (signo == SIGHUP) {
		if (!worker_pid)
			return;

		usmbd_health_status |= USMBD_SHOULD_RELOAD_CONFIG;
		if (kill(worker_pid, signo))
			pr_err("Unable to send SIGHUP to %d: %s\n",
				worker_pid, strerr(errno));
		return;
	}

	setup_signals(SIG_DFL);
	wait_group_kill(signo);
	pr_info("Exiting. Bye!\n");
	delete_lock_file();
	kill(0, SIGINT);
}

static int parse_reload_configs(const char *pwddb, const char *smbconf)
{
	int ret;

	ret = cp_parse_pwddb(pwddb);
	if (ret == -ENOENT) {
		pr_err("User database file does not exist. %s\n",
			"Only guest sessions (if permitted) will work.");
	} else if (ret) {
		pr_err("Unable to parse user database\n");
		return ret;
	}

	ret = cp_parse_reload_smbconf(smbconf);
	if (ret)
		pr_err("Unable to parse smb.conf\n");
	return ret;
}

static int worker_process_init(void)
{
	int ret;

	setup_signals(child_sig_handler);
	set_logger_app_name("usmbd-worker");
	ret = usm_init();
	if (ret) {
		pr_err("Failed to init user management\n");
		goto out;
	}
	ret = wp_init();
	if (ret) {
		pr_err("Failed to init worker\n");
		goto out;
	}
	ret = shm_init();
	if (ret) {
		pr_err("Failed to init net share management\n");
		goto out;
	}

	ret = parse_configs(pwddb, smbconf);
	if (ret) {
		pr_err("Failed to parse configuration files\n");
		goto out;
	}

	ret = sm_init();
	if (ret) {
		pr_err("Failed to init user session management\n");
		goto out;
	}

	ret = rpc_init();
	if (ret) {
		pr_err("Failed to init RPC subsystem\n");
		goto out;
	}

	ret = ipc_init();
	if (ret) {
		pr_err("Failed to init IPC subsystem\n");
		goto out;
	}

	while (usmbd_health_status & USMBD_HEALTH_RUNNING) {
		if (usmbd_health_status & USMBD_SHOULD_RELOAD_CONFIG) {
			ret = parse_reload_configs(pwddb, smbconf);
			if (ret)
				pr_err("Failed to reload configs. "
					"Continue with the old one.\n");
			usmbd_health_status &= ~USMBD_SHOULD_RELOAD_CONFIG;
		}

		ret = ipc_process_event();
		if (ret == -USMBD_STATUS_IPC_FATAL_ERROR) {
			ret = USMBD_STATUS_IPC_FATAL_ERROR;
			break;
		}
	}
out:
	worker_process_free();
	return ret;
}

static pid_t start_worker_process(worker_fn fn)
{
	int status = 0;
	pid_t __pid;

	__pid = fork();
	if (__pid < 0) {
		pr_err("Can't fork child process: `%s'\n", strerr(errno));
		return -EINVAL;
	}
	if (__pid == 0) {
		status = fn();
		exit(status);
	}
	return __pid;
}

static int manager_process_init(void)
{
	/*
	 * Do not chdir() daemon()'d process to '/'.
	 */
	int nochdir = 1;

	setup_signals(manager_sig_handler);
	if (no_detach == 0) {
		pr_logger_init(PR_LOGGER_SYSLOG);
		if (daemon(nochdir, 0) != 0) {
			pr_err("Daemonization failed\n");
			goto out;
		}
	} else {
		/*
		 * Make ourselves a process group leader; if we are
		 * the group leader already then the function will do
		 * nothing (apart from setting errnor to EPERM).
		 */
		if (no_detach == 1)
			setsid();
	}

	if (create_lock_file()) {
		pr_err("Failed to create lock file: %s\n", strerr(errno));
		goto out;
	}

	worker_pid = start_worker_process(worker_process_init);
	if (worker_pid < 0)
		goto out;

	while (1) {
		int status;
		pid_t child;

		child = waitpid(-1, &status, 0);
		if (usmbd_health_status & USMBD_SHOULD_RELOAD_CONFIG &&
				errno == EINTR) {
			usmbd_health_status &= ~USMBD_SHOULD_RELOAD_CONFIG;
			continue;
		}

		pr_err("WARNING: child process exited abnormally: %d\n",
				child);
		if (child == -1) {
			pr_err("waitpid() returned error code: %s\n",
				strerr(errno));
			goto out;
		}

		if (WIFEXITED(status) &&
			WEXITSTATUS(status) == USMBD_STATUS_IPC_FATAL_ERROR) {
			pr_err("Fatal IPC error. Terminating. Check dmesg.\n");
			goto out;
		}

		/* Ratelimit automatic restarts */
		sleep(1);
		worker_pid = start_worker_process(worker_process_init);
		if (worker_pid < 0)
			goto out;
	}
out:
	delete_lock_file();
	kill(0, SIGTERM);
	return 0;
}

static int manager_systemd_service(void)
{
	pid_t __pid;

	__pid = start_worker_process(manager_process_init);
	if (__pid < 0)
		return -EINVAL;

	return 0;
}

static struct option opts[] = {
	{"port",	required_argument,	NULL,	'p' },
	{"config",	required_argument,	NULL,	'c' },
	{"users",	required_argument,	NULL,	'u' },
	{"systemd",	no_argument,		NULL,	's' },
	{"nodetach",	optional_argument,	NULL,	'n' },
	{"help",	no_argument,		NULL,	'h' },
	{"?",		no_argument,		NULL,	'?' },
	{"version",	no_argument,		NULL,	'V' },
	{NULL,		0,			NULL,	 0  }
};

#ifdef MULTICALL
int usmbd_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int systemd_service = 0;
	int c;
	int airbag_init(void);
	airbag_init();
	set_logger_app_name("usmbd-manager");
	memset(&global_conf, 0x00, sizeof(struct smbconf_global));
	pr_logger_init(PR_LOGGER_STDIO);

	opterr = 0;
	while (1) {
		c = getopt_long(argc, argv, "n::p:c:u:sVh", opts, NULL);

		if (c < 0)
			break;

		switch (c) {
		case 0: /* getopt_long() set a variable, just keep going */
			break;
		case 1:
			break;
		case 'p':
			global_conf.tcp_port = cp_get_group_kv_long(optarg);
			pr_debug("TCP port option override\n");
			break;
		case 'c':
			smbconf = strdup(optarg);
			break;
		case 'u':
			pwddb = strdup(optarg);
			break;
		case 'n':
			if (!optarg)
				no_detach = 1;
			else
				no_detach = cp_get_group_kv_long(optarg);
			break;
		case 's':
			systemd_service = 1;
			break;
		case 'V':
			show_version();
			break;
		case ':':
			pr_err("Missing option argument\n");
			/* Fall through */
		case '?':
		case 'h':
			/* Fall through */
		default:
			usage();
		}
	}

	if (!smbconf || !pwddb) {
		pr_err("Out of memory\n");
		exit(EXIT_FAILURE);
	}

	setup_signals(manager_sig_handler);
	if (!systemd_service)
		return manager_process_init();
	return manager_systemd_service();
}
