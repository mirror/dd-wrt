/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lib/misc/lib.h"
#include "lib/device/device.h"
#include "lib/locking/locking.h"
#include "lib/misc/lvm-exec.h"
#include "lib/commands/toolcontext.h"

#include <unistd.h>
#include <sys/wait.h>

/*
 * Create verbose string with list of parameters
 */
static char *_verbose_args(const char *const argv[], char *buf, size_t sz)
{
	int pos = 0;
	int len;
	unsigned i;

	buf[0] = '\0';
	for (i = 0; argv[i]; i++) {
		if ((len = dm_snprintf(buf + pos, sz - pos,
				       " %s", argv[i])) < 0)
			/* Truncated */
			break;
		pos += len;
	}

	return buf;
}

/*
 * Execute and wait for external command
 */
int exec_cmd(struct cmd_context *cmd, const char *const argv[],
	     int *rstatus, int sync_needed)
{
	pid_t pid;
	int status;
	char buf[PATH_MAX * 2];

	if (rstatus)
		*rstatus = -1;

	if (!argv[0]) {
		log_error(INTERNAL_ERROR "Missing command.");
		return 0;
	}

	if (sync_needed)
		/* Flush ops and reset dm cookie */
		if (!sync_local_dev_names(cmd)) {
			log_error("Failed to sync local device names before forking.");
			return 0;
		}

	log_verbose("Executing:%s", _verbose_args(argv, buf, sizeof(buf)));

	if ((pid = fork()) == -1) {
		log_sys_error("fork", "");
		return 0;
	}

	if (!pid) {
		/* Child */
		reset_locking();
		/* FIXME Fix effect of reset_locking on cache then include this */
		/* destroy_toolcontext(cmd); */
		/* FIXME Use execve directly */
		execvp(argv[0], (char **) argv);
		log_sys_error("execvp", argv[0]);
		_exit(errno);
	}

	/* Parent */
	if (wait4(pid, &status, 0, NULL) != pid) {
		log_error("wait4 child process %u failed: %s", pid,
			  strerror(errno));
		return 0;
	}

	if (!WIFEXITED(status)) {
		log_error("Child %u exited abnormally", pid);
		return 0;
	}

	if (WEXITSTATUS(status)) {
		if (rstatus) {
			*rstatus = WEXITSTATUS(status);
			log_verbose("%s failed: %u", argv[0], *rstatus);
		} else
			log_error("%s failed: %u", argv[0], WEXITSTATUS(status));
		return 0;
	}

	if (rstatus)
		*rstatus = 0;

	return 1;
}

static int _reopen_fd_to_null(int fd)
{
	int null_fd;
	int r = 0;

	if ((null_fd = open("/dev/null", O_RDWR)) == -1) {
		log_sys_error("open", "/dev/null");
		return 0;
	}

	if (close(fd)) {
		log_sys_error("close", "");
		goto out;
	}

	if (dup2(null_fd, fd) == -1) {
		log_sys_error("dup2", "");
		goto out;
	}

	r = 1;
out:
	if (close(null_fd)) {
		log_sys_error("dup2", "");
		return 0;
	}

	return r;
}

FILE *pipe_open(struct cmd_context *cmd, const char *const argv[],
		int sync_needed, struct pipe_data *pdata)
{
	int pipefd[2];
	char buf[PATH_MAX * 2];

	if (sync_needed)
		/* Flush ops and reset dm cookie */
		if (!sync_local_dev_names(cmd)) {
			log_error("Failed to sync local device names before forking.");
			return 0;
		}

	if (pipe(pipefd)) {
		log_sys_error("pipe", "");
		return 0;
	}

	log_verbose("Piping:%s", _verbose_args(argv, buf, sizeof(buf)));

	if ((pdata->pid = fork()) == -1) {
		log_sys_error("pipe", "");
		return 0;
	}

	if (pdata->pid == 0) {
		/* Child -> writer, convert pipe[0] to STDOUT */
		if (!_reopen_fd_to_null(STDIN_FILENO))
			stack;
		else if (close(pipefd[0 /*read*/]))
			log_sys_error("close", "pipe[0]");
		else if (close(STDOUT_FILENO))
			log_sys_error("close", "STDOUT");
		else if (dup2(pipefd[1 /*write*/], STDOUT_FILENO) == -1)
			log_sys_error("dup2", "STDOUT");
		else if (close(pipefd[1]))
			log_sys_error("close", "pipe[1]");
		else if (argv[0]) {
			execvp(argv[0], (char **) argv);
			log_sys_error("execvp", argv[0]);
		}
		_exit(errno);
	}

	/* Parent -> reader */
	if (close(pipefd[1 /*write*/])) {
		log_sys_error("close", "STDOUT");
		return NULL;
	}

	if (!(pdata->fp = fdopen(pipefd[0 /*read*/],  "r"))) {
		log_sys_error("fdopen", "STDIN");
		if (close(pipefd[0]))
			log_sys_error("close", "STDIN");
		return NULL; /* FIXME: kill */
	}

	return pdata->fp;
}

int pipe_close(struct pipe_data *pdata)
{
	int status;

	if (fclose(pdata->fp))
		log_sys_error("fclose", "STDIN");

	if (waitpid(pdata->pid, &status, 0) != pdata->pid) {
		log_sys_error("waitpid", "");
		return 0;
	}

	return (status == 0) ? 1 : 0;
}
