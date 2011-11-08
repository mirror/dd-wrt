/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <xfs/libxfs.h>
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
