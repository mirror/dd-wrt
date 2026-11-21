#define _GNU_SOURCE
#define _XOPEN_SOURCE 600

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "macros.h"

#define STATE_WAIT_RETRIES 100
#define EXIT_WAIT_RETRIES 50

static int run_command(const char *cmd, char *output, size_t output_size)
{
	FILE *fp;
	size_t used = 0;
	int status;
	int ch;

	ASSERT(output_size > 0);
	output[0] = '\0';

	fp = popen(cmd, "r");
	ASSERT(fp != NULL);
	while ((ch = fgetc(fp)) != EOF) {
		if (used + 1 < output_size)
			output[used++] = (char)ch;
	}
	output[used] = '\0';
	status = pclose(fp);
	ASSERT(status != -1);
	ASSERT(WIFEXITED(status));
	return WEXITSTATUS(status);
}

static bool have_script(void)
{
	char output[16];

	return run_command("command -v script >/dev/null 2>&1", output, sizeof(output)) == 0;
}

static bool session_has_state(const char *session, const char *state)
{
	char cmd[256];
	char output[4096];
	char *line;
	char *saveptr;

	snprintf(cmd, sizeof(cmd), "LC_ALL=C ./screen -ls 2>&1");
	if (run_command(cmd, output, sizeof(output)) > 1)
		return false;
	for (line = strtok_r(output, "\n", &saveptr); line; line = strtok_r(NULL, "\n", &saveptr)) {
		char *session_pos = strstr(line, session);

		if (session_pos && strstr(session_pos, state))
			return true;
	}
	return false;
}

static bool wait_for_state(const char *session, const char *state, int retries)
{
	struct timespec delay;

	delay.tv_sec = 0;
	delay.tv_nsec = 100000000;
	while (retries-- > 0) {
		if (session_has_state(session, state))
			return true;
		nanosleep(&delay, NULL);
	}
	return false;
}

static pid_t spawn_attached_client(const char *session)
{
	char attach_cmd[256];
	pid_t pid;

	snprintf(attach_cmd, sizeof(attach_cmd), "./screen -r %s", session);

	pid = fork();
	if (pid < 0)
		return -1;
	if (pid == 0) {
		if (setenv("TERM", "xterm", 1) != 0)
			_exit(127);
		execlp("script", "script", "-q", "-c", attach_cmd, "/dev/null", (char *)NULL);
		_exit(127);
	}
	return pid;
}

static void cleanup_session(const char *session)
{
	char cmd[256];
	char output[4096];

	snprintf(cmd, sizeof(cmd), "./screen -S %s -X quit 2>&1", session);
	(void)run_command(cmd, output, sizeof(output));
}

int main(void)
{
	char cmd[256];
	char output[4096];
	char session[64];
	pid_t attached_pid;
	int result;
	int status;
	int retries;
	struct timespec delay;

	attached_pid = -1;
	result = 1;
	if (!have_script()) {
		fprintf(stderr, "SKIP: script not available\n");
		return 77;
	}
	ASSERT(setenv("TERM", "xterm", 1) == 0);
	snprintf(session, sizeof(session), "detach-fix-%ld", (long)getpid());
	delay.tv_sec = 0;
	delay.tv_nsec = 100000000;

	snprintf(cmd, sizeof(cmd), "./screen -dmS %s sh -c 'sleep 30' 2>&1", session);
	ASSERT(run_command(cmd, output, sizeof(output)) == 0);

	attached_pid = spawn_attached_client(session);
	if (attached_pid < 0) {
		fprintf(stderr, "failed to spawn attached client\n");
		goto cleanup;
	}
	if (!wait_for_state(session, "Attached", STATE_WAIT_RETRIES)) {
		fprintf(stderr, "session did not reach Attached state\n");
		goto cleanup;
	}

	snprintf(cmd, sizeof(cmd), "./screen -d %s 2>&1", session);
	ASSERT(run_command(cmd, output, sizeof(output)) == 0);
	ASSERT(strstr(output, "Cannot find terminfo entry for ''") == NULL);
	if (!wait_for_state(session, "Detached", STATE_WAIT_RETRIES)) {
		fprintf(stderr, "session did not reach Detached state\n");
		goto cleanup;
	}

	retries = EXIT_WAIT_RETRIES;
	while (retries-- > 0) {
		if (waitpid(attached_pid, &status, WNOHANG) == attached_pid)
			break;
		nanosleep(&delay, NULL);
	}
	if (retries < 0) {
		kill(attached_pid, SIGTERM);
		fprintf(stderr, "attached client did not exit after remote detach\n");
		goto cleanup;
	}
	ASSERT(WIFEXITED(status) || WIFSIGNALED(status));
	result = 0;

cleanup:
	if (attached_pid > 0 && waitpid(attached_pid, &status, WNOHANG) == 0) {
		kill(attached_pid, SIGTERM);
		waitpid(attached_pid, &status, 0);
	}
	cleanup_session(session);

	return result;
}
