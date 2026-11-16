/*
 * Copyright (c) Red Hat GmbH.  Author: Phil Sutter <phil@nwl.cc>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>
#include <profiling.h>

#include <sys/signalfd.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int get_signalfd(void)
{
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGINT);

	fd = signalfd(-1, &mask, 0);
	if (fd < 0) {
		perror("signalfd()");
		return fd;
	}
	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("sigprocmask()");
		close(fd);
		return -1;
	}
	return fd;
}

void check_signalfd(int fd)
{
	struct signalfd_siginfo info;

	if (read(fd, &info, sizeof(info)) < (signed)sizeof(info))
		return;

	switch (info.ssi_signo) {
	case SIGTERM:
		exit(143);
	case SIGINT:
		exit(130);
	}
}
