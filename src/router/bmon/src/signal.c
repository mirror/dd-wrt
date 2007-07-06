/*
 * signal.c           Signal Handling
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <bmon/bmon.h>
#include <bmon/conf.h>
#include <bmon/utils.h>

static int sig_state = 0;

static RETSIGTYPE sig_usr1(int unused)
{
	sig_state = 1;
	signal(SIGUSR1, sig_usr1);
}

int is_signal_recvd(void)
{
	int ret = sig_state;
	sig_state = 0;
	return ret;
}

#if defined PRAGMA_FALLBACK
#pragma init (init_signal)
#endif

static void __init init_signal(void)
{
	if (signal(SIGUSR1, sig_usr1) == SIG_ERR) {
		perror("signal(SIGUSR1) failed");
		exit(1);
	}
}

void send_signal(const char *arg)
{
	if (*arg != '-') {
		long pid = strtol(arg, NULL, 10);
		
		if (pid != LONG_MIN && pid != LONG_MAX) {
			if (kill((int) pid, SIGUSR1) < 0)
				quit("kill(%d, SIGUSR1) failed: %s\n",
					(int) pid, strerror(errno));
		} else
			quit("Invalid pid.\n");
	} else {
		FILE *f;
		char buf[256], hdr[256], bmon_procs[10][256];
		int cnt = 0, pid_to_kill = -1;
		int my_uid = getuid();
		char cmd[64];
		
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd)-1, "ps -U %d -o pid,uid,args", my_uid);
		
		if (!(f = popen(cmd, "r"))) {
			fprintf(stderr, "popen(%s: %s\n", cmd, strerror(errno));
			quit("Your ps is probably not POSIX compatible, use kill\n");
		}

		fgets(hdr, sizeof(hdr), f);
		
		while(fgets(buf, sizeof(buf), f)) {
			int pid, uid, n;
			char args[256], cmd[256];

			if (!strstr(buf, "bmon") || !strstr(buf, "-w") || strstr(buf, "-S"))
				continue;

			n = sscanf(buf, "%d %d %s %[^\n\r]s", &pid, &uid, cmd, args);
			
			if (n != 4)
				continue;
			
			if (uid == my_uid) {
				if (cnt <= 9) {
					snprintf(bmon_procs[cnt], sizeof(bmon_procs[cnt])-1,
						"%5d %5d %s %s", pid, uid, cmd, args);
					pid_to_kill = pid;
					cnt++;
				} else
					break;
			}
		}
		
		pclose(f);
		
		if (cnt == 1) {
			if (pid_to_kill >= 0)
				if (kill((int) pid_to_kill, SIGUSR1) < 0)
					quit("kill(%d, SIGUSR1) failed: %s\n",
						(int) pid_to_kill, strerror(errno));
		} else {
			int n;
			
			fprintf(stderr, "%d instances of bmon are running, " \
				"use bmon -S <pid>\n", cnt);
			
			printf("%s", hdr);
			for (n=0; n < cnt; n++)
				printf("%s\n", bmon_procs[n]);
		}
	}
}
