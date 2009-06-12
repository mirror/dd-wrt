/*
 * Broadcom UPnP module main entry of linux platform
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: linux_main.c,v 1.9 2008/06/20 05:27:46 Exp $
 */
#include <errno.h>
#include <error.h>
#include <signal.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <netinet/in.h>
#include <wait.h>
#include <ctype.h>
#include <shutils.h>
#include <upnp.h>

#define BCMUPMP_PID_FILE_PATH	"/tmp/bcmupnp.pid"

char g_wandevs[32];

static void
reap(int sig)
{
	pid_t pid;

	if (sig == SIGPIPE)
		return;

	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
		printf("Reaped %d\n", (int)pid);
}

static int
wan_primary_ifunit(void)
{
	int unit;

	for (unit = 0; unit < 10; unit ++) {
		char tmp[100], prefix[] = "wanXXXXXXXXXX_";
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
			return unit;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	char **argp = &argv[1];
	char *wanif = NULL;
	char var[100], prefix[] = "wanXXXXXXXXXX_";
	int daemonize = 0;
	int usage = 0;

	FILE *pidfile;

	/*
	 * Check whether this process is running
	 */
	if ((pidfile = fopen(BCMUPMP_PID_FILE_PATH, "r"))) {
		fprintf(stderr, "%s: UPnP has been started\n", __FILE__);

		fclose(pidfile);
		return -1;
	}

	/* Cread pid file */
	if ((pidfile = fopen(BCMUPMP_PID_FILE_PATH, "w"))) {
		fprintf(pidfile, "%d\n", getpid());
		fclose(pidfile);
	}
	else {
		perror("pidfile");
		exit(errno);
	}

	/*
	 * Process arguments
	 */
	wanif = get_wan_face(); // uses dd-wrt api

	while (argp < &argv[argc]) {
		if (strcasecmp(*argp, "-W") == 0) {
			wanif = *++argp;
		}
		else if (strcasecmp(*argp, "-D") == 0) {
			daemonize = 1;
		}
		else {
			usage = 1;
		}
		argp++;
	}

	strcpy(g_wandevs, wanif);

	/* Warn if the some arguments are not supported */
	if (usage)
		fprintf(stderr, "usage: %s -D [-W <wanif>]\n", argv[0]);

	/*
	 * Enter mainloop
	 */
	if (daemonize && daemon(1, 1) == -1) {
		perror("daemon");
		exit(errno);
	}

	/*
	 * We need to have a reaper for child processes we may create.
	 * That happens when we send signals to the dhcp process to
	 * release an renew a lease on the external interface.
	 */
	signal(SIGCHLD, reap);

	/* Handle the TCP -EPIPE error */
	signal(SIGPIPE, reap);

	/*
	 * For some reason that I do not understand, this process gets
	 * a SIGTERM after sending SIGUSR1 to the dhcp process (to
	 * renew a lease).  Ignore SIGTERM to avoid being killed when
	 * this happens.
	 */
	/* signal(SIGTERM, SIG_IGN); */
	signal(SIGUSR1, upnp_restart_handler);
	fflush(stdout);

	signal(SIGINT, upnp_stop_handler);
	signal(SIGTERM, upnp_stop_handler);

	/* Read configuration to structure and call mainloop */
	upnp_mainloop();

	/* Destroy pid file */
	unlink(BCMUPMP_PID_FILE_PATH);

	return 0;
}

void
upnp_lock()
{
}

void
upnp_unlock()
{
}
