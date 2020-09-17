// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include <signal.h>
#include "sig.h"

static int	gotintr;
static sigset_t	intrset;

static void
interrupt(int sig, siginfo_t *info, void *uc)
{
	gotintr = 1;
}

void
blockint(void)
{
	sigprocmask(SIG_BLOCK, &intrset, NULL);
}

void
clearint(void)
{
	gotintr = 0;
}

void
init_sig(void)
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = interrupt;
	sigaction(SIGINT, &sa, NULL);
	sigemptyset(&intrset);
	sigaddset(&intrset, SIGINT);
}

int
seenint(void)
{
	return gotintr;
}

void
unblockint(void)
{
	sigprocmask(SIG_UNBLOCK, &intrset, NULL);
}
