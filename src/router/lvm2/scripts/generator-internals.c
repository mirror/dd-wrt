// This file contains the unit testable parts of
// lvm2_activation_generator_systemd_red_hat

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>		/* For PATH_MAX for musl libc */
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//----------------------------------------------------------------

static void _error(const char *format, ...) __attribute__ ((format(printf, 1, 2)));

//----------------------------------------------------------------

// I'm rolling my own version of popen() here because I do not want to
// go through the shell.

struct child_process {
	pid_t pid;
	FILE *fp;
};

static bool _open_child(struct child_process *child, const char *cmd, const char *argv[])
{
	int r, pipe_fd[2];

	r = pipe(pipe_fd);
	if (r < 0) {
		_error("call to pipe() failed: %d\n", r);
		return false;
	}

	child->pid = fork();
	if (child->pid < 0) {
		(void) close(pipe_fd[0]);
		(void) close(pipe_fd[1]);
		_error("call to fork() failed: %d\n", r);
		return false;
	}

	if (child->pid == 0) {
		// child
		(void) close(pipe_fd[0]);
		if (pipe_fd[1] != STDOUT_FILENO) {
			(void) dup2(pipe_fd[1], STDOUT_FILENO);
			(void) close(pipe_fd[1]);
		}

		if (execv(cmd, (char *const *) argv) < 0)
			_error("execv failed: %s\n", strerror(errno));
		// Shouldn't get here unless exec failed.
		exit(1);
	} else {
		// parent
		(void) close(pipe_fd[1]);
		child->fp = fdopen(pipe_fd[0], "r");
		if (!child->fp) {
			_error("call to fdopen() failed\n");
			return false;
		}
	}

	return true;
}

// Returns the child's exit status
static bool _close_child(struct child_process *child)
{
	int status;

	(void) fclose(child->fp);

	while (waitpid(child->pid, &status, 0) < 0)
		if (errno != EINTR)
			return -1;

	if (WIFEXITED(status) && !WEXITSTATUS(status))
		return true;

	return false;
}

//----------------------------------------------------------------
// Aquiring config from the lvmconfig process

#define LVM_CONF_EVENT_ACTIVATION "global/event_activation"
#define LVM_CONF_USE_LVMPOLLD	  "global/use_lvmpolld"

struct config {
	bool event_activation;
	bool sysinit_needed;
};

static bool _begins_with(const char *line, const char *prefix, const char **rest)
{
	size_t len = strlen(prefix);

	if (strlen(line) < len)
		return false;

	if (strncmp(line, prefix, len))
		return false;

	*rest = line + len;

	return true;
}

static bool _parse_bool(const char *val, bool * result)
{
	const char *b = val, *e;

	while (*b && isspace(*b))
		b++;

	if (!*b)
		goto parse_error;

	e = b;
	while (*e && !isspace(*e))
		e++;

	if ((e - b) != 1)
		goto parse_error;

	// We only handle '1', or '0'
	if (*b == '1') {
		*result = true;
		return true;

	} else if (*b == '0') {
		*result = false;
		return true;
	}
	// Fallthrough

 parse_error:
	_error("couldn't parse bool value '%s'\n", val);
	return false;
}

static bool _parse_line(const char *line, struct config *cfg)
{
	const char *val;

	if (_begins_with(line, "event_activation=", &val)) {
		return _parse_bool(val, &cfg->event_activation);

	} else if (_begins_with(line, "use_lvmpolld=", &val)) {
		bool r;
		if (!_parse_bool(val, &r))
			return false;
		cfg->sysinit_needed = !r;
		return true;
	}

	return false;
}

static bool _get_config(struct config *cfg, const char *lvmconfig_path)
{
	static const char *_argv[] = {
		"lvmconfig", LVM_CONF_EVENT_ACTIVATION, LVM_CONF_USE_LVMPOLLD, NULL
	};

	bool r = true;
	char buffer[256];
	struct child_process child;

	cfg->event_activation = false;
	cfg->sysinit_needed = true;

	if (!_open_child(&child, lvmconfig_path, _argv)) {
		_error("couldn't open lvmconfig process\n");
		return false;
	}

	while (fgets(buffer, sizeof(buffer), child.fp)) {
		if (!_parse_line(buffer, cfg)) {
			_error("_parse_line() failed\n");
			r = false;
		}
	}

	if (!_close_child(&child)) {
		_error("lvmconfig failed\n");
		r = false;
	}

	return r;
}
