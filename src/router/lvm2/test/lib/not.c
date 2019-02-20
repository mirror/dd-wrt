/*
 * Copyright (C) 2010 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

static int _finished(const char *cmd, int status, int pid) {
	int ret;
	if (!strcmp(cmd, "not"))
		return !status;
	if (!strcmp(cmd, "should")) {
		if (status) {
			fprintf(stderr, "TEST WARNING: Ignoring command failure.\n");
			/* TODO: avoid using shell here */
			/* Show log for failing command which should be passing */
			ret = system("ls debug.log*${LVM_LOG_FILE_EPOCH}* 2>/dev/null");
			if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0) {
				printf("## timing off\n<======== Debug log ========>\n"); /* timing off */
				fflush(stdout);
				if (system("sed -e 's,^,## DEBUG: ,' debug.log*${LVM_LOG_FILE_EPOCH}* 2>/dev/null")) {
				    /* Ignore result code */;
				}
				printf("## timing on\n"); /* timing on */
				if (system("rm -f debug.log*${LVM_LOG_FILE_EPOCH}*")) {
				    /* Ignore result code */;
				}
				fflush(stdout);
			}
		}
		return 0;
	} else if (!strcmp(cmd, "invalid")) {
		if (status == 3)
			return 0;
		fprintf(stderr, "Test expected exit code 3 (invalid), but got %d.\n", status);
	} else if (!strcmp(cmd, "fail")) {
		if (status == 5)
			return 0;
		fprintf(stderr, "Test expected exit code 5 (fail), but got %d.\n", status);
	}
	return 6;
}

int main(int args, char **argv) {
	const char *val = NULL;
	pid_t pid;
	int status;
	int FAILURE = 6;

	if (args < 2) {
		fprintf(stderr, "Need args\n");
		return FAILURE;
	}

	pid = fork();
	if (pid == -1) {
		fprintf(stderr, "Could not fork\n");
		return FAILURE;
	} else if (pid == 0) { 	/* child */
		if (!strcmp(argv[0], "not"))
			val = ">1";
		else if (!strcmp(argv[0], "invalid"))
			val = "3";
		else if (!strcmp(argv[0], "fail"))
			val = "5";

		if (val)
			setenv("LVM_EXPECTED_EXIT_STATUS", val, 1);

		execvp(argv[1], &argv[1]);
		/* should not be accessible */
		return FAILURE;
	} else {		/* parent */
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status)) {
			if (WIFSIGNALED(status))
				fprintf(stderr,
					"Process %d died of signal %d.\n",
					pid, WTERMSIG(status));
			/* did not exit correctly */
			return FAILURE;
		}

		return _finished(argv[0], WEXITSTATUS(status), pid);
	}
	/* not accessible */
	return FAILURE;
}
