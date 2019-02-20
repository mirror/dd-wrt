/*
 * Copyright (C) 2011-2012 Red Hat, Inc.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "tools/tool.h"

#include "daemon-server.h"
#include "daemon-log.h"
#include "libdaemon/client/daemon-io.h"

#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>

#include <syslog.h> /* FIXME. For the global closelog(). */

#if 0
/* Create a device monitoring thread. */
static int _pthread_create(pthread_t *t, void *(*fun)(void *), void *arg, int stacksize)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	/*
	 * We use a smaller stack since it gets preallocated in its entirety
	 */
	pthread_attr_setstacksize(&attr, stacksize + getpagesize());
	return pthread_create(t, &attr, fun, arg);
}
#endif

static volatile sig_atomic_t _shutdown_requested = 0;
static int _systemd_activation = 0;

static void _exit_handler(int sig __attribute__((unused)))
{
	_shutdown_requested = 1;
}

#define EXIT_ALREADYRUNNING 13

#ifdef __linux__

#include <stddef.h>

/*
 * Kernel version 2.6.36 and higher has
 * new OOM killer adjustment interface.
 */
#  define OOM_ADJ_FILE_OLD "/proc/self/oom_adj"
#  define OOM_ADJ_FILE "/proc/self/oom_score_adj"

/* From linux/oom.h */
/* Old interface */
#  define OOM_DISABLE (-17)
#  define OOM_ADJUST_MIN (-16)
/* New interface */
#  define OOM_SCORE_ADJ_MIN (-1000)

/* Systemd on-demand activation support */
#  define SD_ACTIVATION_ENV_VAR_NAME "SD_ACTIVATION"
#  define SD_LISTEN_PID_ENV_VAR_NAME "LISTEN_PID"
#  define SD_LISTEN_FDS_ENV_VAR_NAME "LISTEN_FDS"
#  define SD_LISTEN_FDS_START 3
#  define SD_FD_SOCKET_SERVER SD_LISTEN_FDS_START

static int _is_idle(daemon_state s)
{
	return s.idle && s.idle->is_idle && !s.threads->next;
}

static struct timeval *_get_timeout(daemon_state s)
{
	return s.idle ? s.idle->ptimeout : NULL;
}

static void _reset_timeout(daemon_state s)
{
	if (s.idle) {
		s.idle->ptimeout->tv_sec = 1;
		s.idle->ptimeout->tv_usec = 0;
	}
}

static unsigned _get_max_timeouts(daemon_state s)
{
	return s.idle ? s.idle->max_timeouts : 0;
}

static int _set_oom_adj(const char *oom_adj_path, int val)
{
	FILE *fp;

	if (!(fp = fopen(oom_adj_path, "w"))) {
		perror("oom_adj: fopen failed");
		return 0;
	}

	fprintf(fp, "%i", val);

	if (dm_fclose(fp))
		perror("oom_adj: fclose failed");

	return 1;
}

/*
 * Protection against OOM killer if kernel supports it
 */
static int _protect_against_oom_killer(void)
{
	struct stat st;

	if (stat(OOM_ADJ_FILE, &st) == -1) {
		if (errno != ENOENT)
			perror(OOM_ADJ_FILE ": stat failed");

		/* Try old oom_adj interface as a fallback */
		if (stat(OOM_ADJ_FILE_OLD, &st) == -1) {
			if (errno == ENOENT)
				perror(OOM_ADJ_FILE_OLD " not found");
			else
				perror(OOM_ADJ_FILE_OLD ": stat failed");
			return 1;
		}

		return _set_oom_adj(OOM_ADJ_FILE_OLD, OOM_DISABLE) ||
		       _set_oom_adj(OOM_ADJ_FILE_OLD, OOM_ADJUST_MIN);
	}

	return _set_oom_adj(OOM_ADJ_FILE, OOM_SCORE_ADJ_MIN);
}

union sockaddr_union {
	struct sockaddr sa;
	struct sockaddr_un un;
};

static int _handle_preloaded_socket(int fd, const char *path)
{
	struct stat st_fd;
	union sockaddr_union sockaddr = { .sa.sa_family = 0 };
	int type = 0;
	socklen_t len = sizeof(type);
	size_t path_len = strlen(path);

	if (fd < 0)
		return 0;

	if (fstat(fd, &st_fd) < 0 || !S_ISSOCK(st_fd.st_mode))
		return 0;

	if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) < 0 ||
	    len != sizeof(type) || type != SOCK_STREAM)
		return 0;

	len = sizeof(sockaddr);
	if (getsockname(fd, &sockaddr.sa, &len) < 0 ||
	    len < sizeof(sa_family_t) ||
	    sockaddr.sa.sa_family != PF_UNIX)
		return 0;

	if (!(len >= offsetof(struct sockaddr_un, sun_path) + path_len + 1 &&
	      memcmp(path, sockaddr.un.sun_path, path_len) == 0))
		return 0;

	return 1;
}

static int _systemd_handover(struct daemon_state *ds)
{
	const char *e;
	char *p;
	unsigned long env_pid, env_listen_fds;
	int r = 0;

	/* SD_ACTIVATION must be set! */
	if (!(e = getenv(SD_ACTIVATION_ENV_VAR_NAME)) || strcmp(e, "1"))
		goto out;

	/* LISTEN_PID must be equal to our PID! */
	if (!(e = getenv(SD_LISTEN_PID_ENV_VAR_NAME)))
		goto out;

	errno = 0;
	env_pid = strtoul(e, &p, 10);
	if (errno || !p || *p || env_pid <= 0 ||
	    getpid() != (pid_t) env_pid)
		goto out;

	/* LISTEN_FDS must be 1 and the fd must be a socket! */
	if (!(e = getenv(SD_LISTEN_FDS_ENV_VAR_NAME)))
		goto out;

	errno = 0;
	env_listen_fds = strtoul(e, &p, 10);
	if (errno || !p || *p || env_listen_fds != 1)
		goto out;

	/* Check and handle the socket passed in */
	if ((r = _handle_preloaded_socket(SD_FD_SOCKET_SERVER, ds->socket_path)))
		ds->socket_fd = SD_FD_SOCKET_SERVER;

out:
	unsetenv(SD_ACTIVATION_ENV_VAR_NAME);
	unsetenv(SD_LISTEN_PID_ENV_VAR_NAME);
	unsetenv(SD_LISTEN_FDS_ENV_VAR_NAME);
	return r;
}

#endif

static int _open_socket(daemon_state s)
{
	int fd;
	int file_created = 0;
	struct sockaddr_un sockaddr = { .sun_family = AF_UNIX };
	struct stat buf;
	mode_t old_mask;

	(void) dm_prepare_selinux_context(s.socket_path, S_IFSOCK);
	old_mask = umask(0077);

	/* Open local socket */
	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("Can't create local socket.");
		goto error;
	}

	/* Set non-blocking */
	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK))
		fprintf(stderr, "setting O_NONBLOCK on socket fd %d failed: %s\n", fd, strerror(errno));

	fprintf(stderr, "[D] creating %s\n", s.socket_path);
	if (!dm_strncpy(sockaddr.sun_path, s.socket_path, sizeof(sockaddr.sun_path))) {
		fprintf(stderr, "%s: daemon socket path too long.\n", s.socket_path);
		goto error;
	}

	if (bind(fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr))) {
		if (errno != EADDRINUSE) {
			perror("can't bind local socket");
			goto error;
		}

		/* Socket already exists. If it's stale, remove it. */
		if (lstat(sockaddr.sun_path, &buf)) {
			perror("stat failed");
			goto error;
		}

		if (!S_ISSOCK(buf.st_mode)) {
			fprintf(stderr, "%s: not a socket\n", sockaddr.sun_path);
			goto error;
		}

		if (buf.st_uid || (buf.st_mode & (S_IRWXG | S_IRWXO))) {
			fprintf(stderr, "%s: unrecognised permissions\n", sockaddr.sun_path);
			goto error;
		}

		if (!connect(fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr))) {
			fprintf(stderr, "Socket %s already in use\n", sockaddr.sun_path);
			goto error;
		}

		fprintf(stderr, "removing stale socket %s\n", sockaddr.sun_path);

		if (unlink(sockaddr.sun_path) && (errno != ENOENT)) {
			perror("unlink failed");
			goto error;
		}

		if (bind(fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr))) {
			perror("local socket bind failed after unlink");
			goto error;
		}
	}

	file_created = 1;

	if (listen(fd, 1) != 0) {
		perror("listen local");
		goto error;
	}

out:
	umask(old_mask);
	(void) dm_prepare_selinux_context(NULL, 0);
	return fd;

error:
	if (fd >= 0) {
		if (close(fd))
			perror("close failed");
		if (file_created && unlink(s.socket_path))
			perror("unlink failed");
		fd = -1;
	}
	goto out;
}

static void _remove_lockfile(const char *file)
{
	if (unlink(file))
		perror("unlink failed");
}

static void _daemonise(daemon_state s)
{
	int child_status;
	int fd;
	pid_t pid;
	struct rlimit rlim;
	struct timeval tval;
	sigset_t my_sigset;

	if ((fd = open("/dev/null", O_RDWR)) == -1) {
		fprintf(stderr, "Unable to open /dev/null.\n");
		exit(EXIT_FAILURE);
	}

	sigemptyset(&my_sigset);
	if (sigprocmask(SIG_SETMASK, &my_sigset, NULL) < 0) {
		fprintf(stderr, "Unable to restore signals.\n");
		exit(EXIT_FAILURE);
	}
	signal(SIGTERM, &_exit_handler);

	switch (pid = fork()) {
	case -1:
		perror("fork failed:");
		exit(EXIT_FAILURE);

	case 0:		/* Child */
		break;

	default:
		(void) close(fd);
		/* Wait for response from child */
		while (!waitpid(pid, &child_status, WNOHANG) && !_shutdown_requested) {
			tval.tv_sec = 0;
			tval.tv_usec = 250000;	/* .25 sec */
			select(0, NULL, NULL, NULL, &tval);
		}

		if (_shutdown_requested) /* Child has signaled it is ok - we can exit now */
			exit(0);

		switch (WEXITSTATUS(child_status)) {
		case EXIT_ALREADYRUNNING:
			fprintf(stderr, "Failed to acquire lock on %s. Already running?\n", s.pidfile);
			break;
		default:
			/* Problem with child.  Determine what it is by exit code */
			fprintf(stderr, "Child exited with code %d\n", WEXITSTATUS(child_status));
		}
		exit(WEXITSTATUS(child_status));
	}

	if (chdir("/")) {
		perror("Cannot chdir to /");
		exit(1);
	}

	if ((dup2(fd, STDIN_FILENO) == -1) ||
	    (dup2(fd, STDOUT_FILENO) == -1) ||
	    (dup2(fd, STDERR_FILENO) == -1)) {
		perror("Error setting terminal FDs to /dev/null");
		exit(2);
	}

	if ((fd > STDERR_FILENO) && close(fd)) {
		perror("Failed to close /dev/null descriptor");
		exit(3);
	}

	/* Switch to sysconf(_SC_OPEN_MAX) ?? */
	if (getrlimit(RLIMIT_NOFILE, &rlim) < 0)
		fd = 256; /* just have to guess */
	else
		fd = rlim.rlim_cur;

	for (--fd; fd > STDERR_FILENO; fd--) {
#ifdef __linux__
		/* Do not close fds preloaded by systemd! */
		if (_systemd_activation && fd == SD_FD_SOCKET_SERVER)
			continue;
#endif
		(void) close(fd);
	}

	setsid();
}

response daemon_reply_simple(const char *id, ...)
{
	va_list ap;
	response res = { .cft = NULL };

	va_start(ap, id);

	buffer_init(&res.buffer);
	if (!buffer_append_f(&res.buffer, "response = %s", id, NULL)) {
		res.error = ENOMEM;
		goto end;
	}
	if (!buffer_append_vf(&res.buffer, ap)) {
		res.error = ENOMEM;
		goto end;
	}

end:
	va_end(ap);
	return res;
}

static response _builtin_handler(daemon_state s, client_handle h, request r)
{
	const char *rq = daemon_request_str(r, "request", "NONE");
	response res = { .error = EPROTO };

	if (!strcmp(rq, "hello")) {
		return daemon_reply_simple("OK", "protocol = %s", s.protocol ?: "default",
					   "version = %" PRId64, (int64_t) s.protocol_version, NULL);
	}

	buffer_init(&res.buffer);
	return res;
}

static void *_client_thread(void *state)
{
	thread_state *ts = state;
	request req;
	response res;

	buffer_init(&req.buffer);

	while (1) {
		if (!buffer_read(ts->client.socket_fd, &req.buffer))
			goto fail;

		req.cft = config_tree_from_string_without_dup_node_check(req.buffer.mem);

		if (!req.cft)
			fprintf(stderr, "error parsing request:\n %s\n", req.buffer.mem);
		else
			daemon_log_cft(ts->s.log, DAEMON_LOG_WIRE, "<- ", req.cft->root);

		res = _builtin_handler(ts->s, ts->client, req);

		if (res.error == EPROTO) /* Not a builtin, delegate to the custom handler. */
			res = ts->s.handler(ts->s, ts->client, req);

		if (!res.buffer.mem) {
			if (!dm_config_write_node(res.cft->root, buffer_line, &res.buffer))
				goto fail;
			if (!buffer_append(&res.buffer, "\n\n"))
				goto fail;
			dm_config_destroy(res.cft);
		}

		if (req.cft)
			dm_config_destroy(req.cft);
		buffer_destroy(&req.buffer);

		daemon_log_multi(ts->s.log, DAEMON_LOG_WIRE, "-> ", res.buffer.mem);
		buffer_write(ts->client.socket_fd, &res.buffer);

		buffer_destroy(&res.buffer);
	}
fail:
	/* TODO what should we really do here? */
	if (close(ts->client.socket_fd))
		perror("close");
	buffer_destroy(&req.buffer);
	ts->active = 0;
	return NULL;
}

static int _handle_connect(daemon_state s)
{
	thread_state *ts;
	struct sockaddr_un sockaddr;
	client_handle client = { .thread_id = 0 };
	socklen_t sl = sizeof(sockaddr);

	client.socket_fd = accept(s.socket_fd, (struct sockaddr *) &sockaddr, &sl);
	if (client.socket_fd < 0) {
		if (errno != EAGAIN || !_shutdown_requested)
			ERROR(&s, "Failed to accept connection: %s.", strerror(errno));
		return 0;
	}

	 if (fcntl(client.socket_fd, F_SETFD, FD_CLOEXEC))
		WARN(&s, "setting CLOEXEC on client socket fd %d failed", client.socket_fd);

	if (!(ts = malloc(sizeof(thread_state)))) {
		if (close(client.socket_fd))
			perror("close");
		ERROR(&s, "Failed to allocate thread state");
		return 0;
	}

	ts->next = s.threads->next;
	s.threads->next = ts;

	ts->active = 1;
	ts->s = s;
	ts->client = client;

	if ((errno = pthread_create(&ts->client.thread_id, NULL, _client_thread, ts))) {
		ERROR(&s, "Failed to create client thread: %s.", strerror(errno));
		return 0;
	}

	return 1;
}

static void _reap(daemon_state s, int waiting)
{
	thread_state *last = s.threads, *ts = last->next;
	void *rv;

	while (ts) {
		if (waiting || !ts->active) {
			if ((errno = pthread_join(ts->client.thread_id, &rv)))
				ERROR(&s, "pthread_join failed: %s", strerror(errno));
			last->next = ts->next;
			free(ts);
		} else
			last = ts;
		ts = last->next;
	}
}

void daemon_start(daemon_state s)
{
	int failed = 0;
	log_state _log = { { 0 } };
	thread_state _threads = { .next = NULL };
	unsigned timeout_count = 0;
	fd_set in;

	/*
	 * Switch to C locale to avoid reading large locale-archive file used by
	 * some glibc (on some distributions it takes over 100MB). Some daemons
	 * need to use mlockall().
	 */
	if (setenv("LC_ALL", "C", 1))
		perror("Cannot set LC_ALL to C");

#ifdef __linux__
	_systemd_activation = _systemd_handover(&s);
#endif

	if (!s.foreground)
		_daemonise(s);

	s.log = &_log;
	s.log->name = s.name;
	s.threads = &_threads;

	/* Log important things to syslog by default. */
	daemon_log_enable(s.log, DAEMON_LOG_OUTLET_SYSLOG, DAEMON_LOG_FATAL, 1);
	daemon_log_enable(s.log, DAEMON_LOG_OUTLET_SYSLOG, DAEMON_LOG_ERROR, 1);

	if (s.pidfile) {
		(void) dm_prepare_selinux_context(s.pidfile, S_IFREG);

		/*
		 * NB. Take care to not keep stale locks around. Best not exit(...)
		 * after this point.
		 */
		if (dm_create_lockfile(s.pidfile) == 0) {
			ERROR(&s, "Failed to acquire lock on %s. Already running?\n", s.pidfile);
			exit(EXIT_ALREADYRUNNING);
		}

		(void) dm_prepare_selinux_context(NULL, 0);
	}

	/* Set normal exit signals to request shutdown instead of dying. */
	signal(SIGINT, &_exit_handler);
	signal(SIGHUP, &_exit_handler);
	signal(SIGQUIT, &_exit_handler);
	signal(SIGTERM, &_exit_handler);
	signal(SIGALRM, &_exit_handler);
	signal(SIGPIPE, SIG_IGN);

#ifdef __linux__
	/* Systemd has adjusted oom killer for us already */
	if (s.avoid_oom && !_systemd_activation && !_protect_against_oom_killer())
		ERROR(&s, "Failed to protect against OOM killer");
#endif

	if (!_systemd_activation && s.socket_path) {
		s.socket_fd = _open_socket(s);
		if (s.socket_fd < 0)
			failed = 1;
	}

	/* Set Close-on-exec */
	if (!failed && fcntl(s.socket_fd, F_SETFD, 1))
		ERROR(&s, "setting CLOEXEC on socket fd %d failed: %s\n", s.socket_fd, strerror(errno));

	/* Signal parent, letting them know we are ready to go. */
	if (!s.foreground)
		kill(getppid(), SIGTERM);

	/*
	 * Use daemon_main for daemon-specific init and polling, or
	 * use daemon_init for daemon-specific init and generic lib polling.
	 */

	if (s.daemon_main) {
		if (!s.daemon_main(&s))
			failed = 1;
		goto out;
	}

	if (s.daemon_init)
		if (!s.daemon_init(&s))
			failed = 1;

	while (!failed) {
		_reset_timeout(s);
		FD_ZERO(&in);
		FD_SET(s.socket_fd, &in);
		if (select(FD_SETSIZE, &in, NULL, NULL, _get_timeout(s)) < 0 && errno != EINTR)
			perror("select error");
		if (FD_ISSET(s.socket_fd, &in)) {
			timeout_count = 0;
			_handle_connect(s);
		}

		_reap(s, 0);

		if (_shutdown_requested && !s.threads->next)
			break;

		/* s.idle == NULL equals no shutdown on timeout */
		if (_is_idle(s)) {
			DEBUGLOG(&s, "timeout occured");
			if (++timeout_count >= _get_max_timeouts(s)) {
				INFO(&s, "Inactive for %d seconds. Exiting.", timeout_count);
				break;
			}
		}
	}

	INFO(&s, "%s waiting for client threads to finish", s.name);
	_reap(s, 1);
out:
	/* If activated by systemd, do not unlink the socket - systemd takes care of that! */
	if (!_systemd_activation && s.socket_fd >= 0)
		if (unlink(s.socket_path))
			perror("unlink error");

	if (s.socket_fd >= 0)
		if (close(s.socket_fd))
			perror("socket close");

	if (s.daemon_fini)
		if (!s.daemon_fini(&s))
			failed = 1;

	INFO(&s, "%s shutting down", s.name);

	closelog(); /* FIXME */
	if (s.pidfile)
		_remove_lockfile(s.pidfile);
	if (failed)
		exit(1);
}
