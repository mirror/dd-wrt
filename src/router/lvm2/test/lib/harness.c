/*
 * Copyright (C) 2010-2013 Red Hat, Inc. All rights reserved.
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

#include "configure.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/klog.h>
#include <sys/resource.h> /* rusage */
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

static pid_t pid;
static int fds[2];

#define MAX 1024
#define MAX_LOG_SIZE (32*1024*1024) /* Default max size of test log */
#define WRITE_TIMEOUT (180 * 2)	/* 3 minutes */

struct stats {
	int nfailed;
	int nskipped;
	int npassed;
	int nknownfail;
	int nwarned;
	int ninterrupted;
	int status[MAX];
};

static struct stats s;

static char *readbuf = NULL;
static size_t readbuf_sz = 0, readbuf_used = 0;

static int die = 0;
static int verbose = 0; /* >1 with timestamps */
static int interactive = 0; /* disable all redirections */
static int quiet = 0;
static const char *results;
static unsigned fullbuffer = 0;
static int unlimited = 0;
static int write_timeout = WRITE_TIMEOUT;

static time_t harness_start;

static FILE *outfile = NULL;
char testdirdebug[PATH_MAX];

struct subst {
	const char *key;
	char *value;
};

static struct subst subst[2];

enum {
	UNKNOWN,
	FAILED,
	INTERRUPTED,
	KNOWNFAIL,
	PASSED,
	SKIPPED,
	TIMEOUT,
	WARNED,
};

static void handler( int sig ) {
	signal( sig, SIG_DFL );
	kill( -pid, sig );
	die = sig;
}

static int outline(FILE *out, char *buf, int start, int force) {
	char *from = buf + start;
	char *next = strchr(buf + start, '\n');

	if (!next && !force) /* not a complete line yet... */
		return start;

	if (!next)
		next = from + strlen(from);
	else
		++next;

	if (!strncmp(from, "@TESTDIR=", 9)) {
		subst[0].key = "@TESTDIR@";
		free(subst[0].value);
		subst[0].value = strndup(from + 9, next - from - 9 - 1);
		snprintf(testdirdebug, sizeof(testdirdebug), "%s/debug.log", subst[0].value);
	} else if (!strncmp(from, "@PREFIX=", 8)) {
		subst[1].key = "@PREFIX@";
		free(subst[1].value);
		subst[1].value = strndup(from + 8, next - from - 8 - 1);
	} else {
		char *line = strndup(from, next - from);
		char *a = line, *b;
		do {
			int idx = -1;
			int i;
			b = line + strlen(line);
			for ( i = 0; i < 2; ++i ) {
				if (subst[i].key) {
					// printf("trying: %s -> %s\n", subst[i].value, subst[i].key);
					char *stop = strstr(a, subst[i].value);
					if (stop && stop < b) {
						idx = i;
						b = stop;
					}
				}
			}
			fwrite(a, 1, b - a, out);
			a = b;

			if ( idx >= 0 ) {
				fprintf(out, "%s", subst[idx].key);
				a += strlen(subst[idx].value);
			}
		} while (b < line + strlen(line));
		free(line);
	}

	return next - buf + (force ? 0 : 1);
}

static void dump(void) {
	int counter_last = -1, counter = 0;

	while ((counter < (int) readbuf_used) && (counter != counter_last)) {
		counter_last = counter;
		counter = outline( stdout, readbuf, counter, 1 );
	}
}

static void trickle(FILE *out, int *last, int *counter) {
	if (*last > (int) readbuf_used) {
		*last = -1;
		*counter = 0;
	}
	while ((*counter < (int) readbuf_used) && (*counter != *last)) {
		*last = *counter;
		*counter = outline( out, readbuf, *counter, 1 );
	}
}

static void clear(void) {
	readbuf_used = 0;
	fullbuffer = 0;
}

static int64_t _get_time_us(void)
{
       struct timeval tv;

       (void) gettimeofday(&tv, 0);
       return (int64_t) tv.tv_sec * 1000000 + (int64_t) tv.tv_usec;
}

static void _append_buf(const char *buf, size_t len)
{
	if ((readbuf_used + len) >= readbuf_sz) {
		if ((readbuf_sz >= MAX_LOG_SIZE) && !unlimited) {
			if (fullbuffer++ ==  0)
				kill(-pid, SIGINT);
			return;
		}
		readbuf_sz = 2 * (readbuf_used + len + readbuf_sz);
		readbuf = realloc(readbuf, readbuf_sz);
	}

	if (!readbuf)
		exit(205);

	memcpy(readbuf + readbuf_used, buf, len);
	readbuf_used += len;
}

static const char *_append_with_stamp(const char *buf, int stamp)
{
	static const char spaces[] = "                 ";
	static int64_t t_last;
	static int64_t t_start = 0;
	int64_t t_now;
	char stamp_buf[32]; /* Bigger to always fit both numbers */
	const char *be;
	const char *bb = buf;
	size_t len;

	while ((be = strchr(bb, '\n'))) {
		if (stamp++ == 0) {
			t_now = _get_time_us();
			if (!t_start)
				t_start = t_last = t_now;
			len = snprintf(stamp_buf, sizeof(stamp_buf),
				       "%8.3f%8.4f ",
				       (t_now - t_start) / 1000000.f,
				       (t_now - t_last) / 1000000.f);
			_append_buf(stamp_buf, (len < (sizeof(spaces) - 1)) ?
				    len : (sizeof(spaces) - 1));
			t_last = t_now;
		}

		_append_buf(bb, be + 1 - bb);
		bb = be + 1;

		if (stamp > 0 && bb[0])
			_append_buf(spaces, sizeof(spaces) - 1);
	}

	return bb;
}

static int drain(int fd)
{
	char buf[2 * 1024 * 1024 + 1]; /* try to capture large sysrq trace */
	const char *bp;
	int stamp = 0;
	int sz;

	static int stdout_last = -1, stdout_counter = 0;
	static int outfile_last = -1, outfile_counter = 0;

	if ((sz = read(fd, buf, sizeof(buf) - 1)) > 0) {
		buf[sz] = '\0';
		bp = (verbose < 2) ? buf : _append_with_stamp(buf, stamp);
		if (sz > (bp - buf)) {
			_append_buf(bp, sz - (bp - buf));
			stamp = -1; /* unfinished line */
		} else
			stamp = 0;

		readbuf[readbuf_used] = 0;

		if (verbose)
			trickle(stdout, &stdout_last, &stdout_counter);
		if (outfile)
			trickle(outfile, &outfile_last, &outfile_counter);
	}

	return sz;
}

static int drain_fds(int fd1, int fd2, long timeout)
{
	return -1;
}

#define SYSLOG_ACTION_READ_CLEAR     4
#define SYSLOG_ACTION_CLEAR          5

static void clear_dmesg(void)
{
	klogctl(SYSLOG_ACTION_CLEAR, 0, 0);
}

static void drain_dmesg(void)
{
	char buf[1024 * 1024 + 1];
	int sz = klogctl(SYSLOG_ACTION_READ_CLEAR, buf, sizeof(buf) - 1);
	if (sz > 0) {
		buf[sz] = 0;
		_append_buf(buf, sz);
	}
}

static const char *duration(time_t start, const struct rusage *usage)
{
	static char buf[100];
	int t = (int)(time(NULL) - start);

	int p = sprintf(buf, "%2d:%02d", t / 60, t % 60);

	if (usage)
		sprintf(buf + p, "   %2ld:%02ld.%03ld/%ld:%02ld.%03ld%5ld%8ld/%ld",
			usage->ru_utime.tv_sec / 60, usage->ru_utime.tv_sec % 60,
			usage->ru_utime.tv_usec / 1000,
			usage->ru_stime.tv_sec / 60, usage->ru_stime.tv_sec % 60,
			usage->ru_stime.tv_usec / 1000,
			usage->ru_maxrss / 1024,
			usage->ru_inblock, usage->ru_oublock);

	return buf;
}

static void passed(int i, char *f, time_t t, const struct rusage *usage) {
	if (readbuf && strstr(readbuf, "TEST EXPECT FAIL")) {
		++ s.npassed;
		s.status[i] = PASSED;
		printf("passed (UNEXPECTED). %s\n", duration(t, usage));
	} else if (readbuf && strstr(readbuf, "TEST WARNING")) {
		++s.nwarned;
		s.status[i] = WARNED;
		printf("warnings  %s\n", duration(t, usage));
	} else {
		++ s.npassed;
		s.status[i] = PASSED;
		printf("passed.   %s\n", duration(t, usage));
	}
}

static void interrupted(int i, char *f) {
	++ s.ninterrupted;
	s.status[i] = INTERRUPTED;
	printf("\ninterrupted.\n");
	if (!quiet && !verbose && fullbuffer) {
		printf("-- Interrupted %s ------------------------------------\n", f);
		dump();
		printf("\n-- Interrupted %s (end) ------------------------------\n", f);
	}
}

static void timeout(int i, char *f) {
	++ s.ninterrupted;
	s.status[i] = TIMEOUT;
	printf("timeout.\n");
	if (!quiet && !verbose && readbuf) {
		printf("-- Timed out %s ------------------------------------\n", f);
		dump();
		printf("\n-- Timed out %s (end) ------------------------------\n", f);
	}
}

static void skipped(int i, char *f) {
	++ s.nskipped;
	s.status[i] = SKIPPED;
	printf("skipped.\n");
}

static void failed(int i, char *f, int st) {
	if (readbuf && strstr(readbuf, "TEST EXPECT FAIL")) {
		printf("FAILED (expected).\n");
		s.status[i] = KNOWNFAIL;
		++ s.nknownfail;
		return;
	}

	++ s.nfailed;
	s.status[i] = FAILED;
	printf("FAILED  (status %d).\n", WEXITSTATUS(st));
	if (!quiet && !verbose && readbuf) {
		printf("-- FAILED %s ------------------------------------\n", f);
		dump();
		printf("-- FAILED %s (end) ------------------------------\n", f);
	}
}

static void run(int i, char *f) {
	struct rusage usage;
	char flavour[512], script[512];

	pid = fork();
	if (pid < 0) {
		perror("Fork failed.");
		exit(201);
	} else if (pid == 0) {
		if (!interactive) {
			close(STDIN_FILENO);
			dup2(fds[1], STDOUT_FILENO);
			dup2(fds[1], STDERR_FILENO);
			close(fds[1]);
		}
		close(fds[0]);
		if (strchr(f, ':')) {
			strcpy(flavour, f);
			*strchr(flavour, ':') = 0;
			setenv("LVM_TEST_FLAVOUR", flavour, 1);
			strcpy(script, strchr(f, ':') + 1);
		} else {
			strcpy(script, f);
		}
		setpgid(0, 0);
		execlp("bash", "bash", "-noprofile", "-norc", script, NULL);
		perror("execlp");
		fflush(stderr);
		_exit(202);
	} else {
		int st = -1, w;
		time_t start = time(NULL);
		char buf[128];
		char outpath[PATH_MAX];
		char *c = outpath + strlen(results) + 1;
		struct stat statbuf;
		int runaway = 0;
		int no_write = 0;
		int clobber_dmesg = 0;
		int collect_debug = 0;
		int fd_debuglog = -1;
		int fd_kmsg;
		fd_set set;
		int ret;

		//close(fds[1]);
		testdirdebug[0] = '\0'; /* Capture RUNTESTDIR */
		snprintf(buf, sizeof(buf), "%s ...", f);
		printf("Running %-60s%c", buf, verbose ? '\n' : ' ');
		fflush(stdout);
		snprintf(outpath, sizeof(outpath), "%s/%s.txt", results, f);
		while ((c = strchr(c, '/')))
			*c = '_';
		if (!(outfile = fopen(outpath, "w")))
			perror("fopen");

		/* Mix-in kernel log message */
		if ((fd_kmsg = open("/dev/kmsg", O_RDONLY | O_NONBLOCK)) < 0) {
			if (errno != ENOENT) /* Older kernels (<3.5) do not support /dev/kmsg */
				perror("open /dev/kmsg");
		} else if (lseek(fd_kmsg, 0L, SEEK_END) == (off_t) -1)
			perror("lseek /dev/kmsg");

		if ((fd_kmsg < 0) &&
		    (clobber_dmesg = strcmp(getenv("LVM_TEST_CAN_CLOBBER_DMESG") ? : "0", "0")))
			clear_dmesg();

		while ((w = wait4(pid, &st, WNOHANG, &usage)) == 0) {
			struct timeval selectwait = { .tv_usec = 500000 }; /* 0.5s */

			if ((fullbuffer && fullbuffer++ == 8000) ||
			    (write_timeout > 0 && no_write > write_timeout)) 
			{
			timeout:
				kill(pid, SIGINT);
				sleep(5); /* wait a bit for a reaction */
				if ((w = waitpid(pid, &st, WNOHANG)) == 0) {
					if (write_timeout > 0 && no_write > write_timeout)
						/*
						 * Kernel traces needed, when stuck for
						 * too long in userspace without producing
						 * any output, in other case it should be
						 * user space problem
						 */
						system("echo t > /proc/sysrq-trigger");
					collect_debug = 1;
					kill(-pid, SIGKILL);
					w = pid; // waitpid(pid, &st, NULL);
				}
				runaway = 1;
				break;
			}

			if (clobber_dmesg)
				drain_dmesg();

			FD_ZERO(&set);
			FD_SET(fds[0], &set);
			if (fd_kmsg >= 0)
				FD_SET(fd_kmsg, &set);

			if ((ret = select(fd_kmsg > fds[0] ? fd_kmsg + 1 : fds[0] + 1, &set, NULL, NULL, &selectwait)) <= 0) {
				/* Still checking debug log size if it's not growing too much */
				if (!unlimited && testdirdebug[0] &&
				    (stat(testdirdebug, &statbuf) == 0) &&
				    statbuf.st_size > 32 * 1024 * 1024) { /* 32MB command log size */
					printf("Killing test since debug.log has gone wild (size %ld)\n",
					       statbuf.st_size);
					goto timeout;
				}
				no_write++;
				continue;
			}

			if (FD_ISSET(fds[0], &set) && drain(fds[0]) > 0)
				no_write = 0;
			else if (fd_kmsg >= 0 && FD_ISSET(fd_kmsg, &set) && (drain(fd_kmsg) < 0)) {
				close(fd_kmsg);
				fd_kmsg = -1; /* Likely /dev/kmsg is not readable */
				if ((clobber_dmesg = strcmp(getenv("LVM_TEST_CAN_CLOBBER_DMESG") ? : "0", "0")))
					clear_dmesg();
			}
		}
		if (w != pid) {
			perror("waitpid");
			exit(206);
		}

		while (!fullbuffer && (drain_fds(fds[0], fd_kmsg, 0) > 0))
			/* read out what was left */;

		if (die == 2)
			interrupted(i, f);
		else if (runaway) {
			if (collect_debug &&
			    (fd_debuglog = open(testdirdebug, O_RDONLY)) != -1) {
				runaway = unlimited ? INT32_MAX : 4 * 1024 * 1024;
				while (!fullbuffer && runaway > 0 && (ret = drain(fd_debuglog)) > 0)
					runaway -= ret;
				close(fd_debuglog);
			}
			timeout(i, f);
		} else if (WIFEXITED(st)) {
			if (WEXITSTATUS(st) == 0)
				passed(i, f, start, &usage);
			else if (WEXITSTATUS(st) == 200)
				skipped(i, f);
			else
				failed(i, f, st);
		} else
			failed(i, f, st);

		if (fd_kmsg >= 0)
			close(fd_kmsg);
		else if (clobber_dmesg)
			drain_dmesg();
		if (outfile)
			fclose(outfile);
		if (fullbuffer)
			printf("\nTest was interrupted, output has got too large (>%u) (loop:%u)\n"
			       "Set LVM_TEST_UNLIMITED=1 for unlimited log.\n",
			       (unsigned) readbuf_sz, fullbuffer);
		clear();
	}
}

int main(int argc, char **argv) {
	char results_list[PATH_MAX];
	const char *result;
	const char *be_verbose = getenv("VERBOSE"),
		   *be_interactive = getenv("INTERACTIVE"),
		   *be_quiet = getenv("QUIET"),
		   *be_write_timeout = getenv("WRITE_TIMEOUT");
	time_t start = time(NULL);
	int i;
	FILE *list;

	if (argc >= MAX) {
		fprintf(stderr, "Sorry, my head exploded. Please increase MAX.\n");
		exit(1);
	}

	if (be_verbose)
		verbose = atoi(be_verbose);

	if (be_interactive)
		interactive = atoi(be_interactive);

	if (be_quiet)
		quiet = atoi(be_quiet);

	if (be_write_timeout)
		write_timeout = atoi(be_write_timeout) * 2;

	results = getenv("LVM_TEST_RESULTS") ? : "results";
	unlimited = getenv("LVM_TEST_UNLIMITED") ? 1 : 0;
	(void) snprintf(results_list, sizeof(results_list), "%s/list", results);

	//if (pipe(fds)) {
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, fds)) {
		perror("socketpair");
		return 201;
	}

	if (fcntl(fds[0], F_SETFL, O_NONBLOCK ) == -1) {
		perror("fcntl on socket");
		return 202;
	}

	/* set up signal handlers */
	for (i = 0; i <= 32; ++i)
		switch (i) {
		case SIGCHLD: case SIGWINCH: case SIGURG:
		case SIGKILL: case SIGSTOP: break;
		default: signal(i, handler);
		}

	harness_start = time(NULL);
	/* run the tests */
	for (i = 1; !die && i < argc; ++i) {
		run(i, argv[i]);
		if ( time(NULL) - harness_start > 48 * 360 ) { /* 04:48 */
			printf("Nearly 5 hours passed, giving up...\n");
			die = 1;
		}
	}

	free(subst[0].value);
	free(subst[1].value);
	free(readbuf);

	printf("\n## %d tests %s : %d OK, %d warnings, %d failures (%d interrupted), %d known failures; "
	       "%d skipped\n",
	       s.nwarned + s.npassed + s.nfailed + s.nskipped + s.ninterrupted,
	       duration(start, NULL),
	       s.npassed, s.nwarned, s.nfailed + s.ninterrupted, s.ninterrupted,
	       s.nknownfail, s.nskipped);

	/* dump a list to results */
	if ((list = fopen(results_list, "w"))) {
		for (i = 1; i < argc; ++ i) {
			switch (s.status[i]) {
			case FAILED: result = "failed"; break;
			case INTERRUPTED: result = "interrupted"; break;
			case PASSED: result = "passed"; break;
			case SKIPPED: result = "skipped"; break;
			case TIMEOUT: result = "timeout"; break;
			case WARNED: result = "warnings"; break;
			default: result = "unknown"; break;
			}
			fprintf(list, "%s %s\n", argv[i], result);
		}
		fclose(list);
	} else
		perror("fopen result");

	/* print out a summary */
	if (s.nfailed || s.nskipped || s.nknownfail || s.ninterrupted || s.nwarned) {
		for (i = 1; i < argc; ++ i) {
			switch (s.status[i]) {
			case FAILED:
				printf("FAILED: %s\n", argv[i]);
				break;
			case INTERRUPTED:
				printf("INTERRUPTED: %s\n", argv[i]);
				break;
			case KNOWNFAIL:
				printf("FAILED (expected): %s\n", argv[i]);
				break;
			case SKIPPED:
				printf("skipped: %s\n", argv[i]);
				break;
			case TIMEOUT:
				printf("TIMEOUT: %s\n", argv[i]);
				break;
			case WARNED:
				printf("WARNED: %s\n", argv[i]);
				break;
			default: /* do nothing */ ;
			}
		}
		printf("\n");
		return (s.nfailed > 0) || (s.ninterrupted > 0) || die;
	}

	return die;
}
