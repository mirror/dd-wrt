/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/diag/diag.c#1 $
 */
#include "diag.h"

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

int	signalled;

static void
catchalarm(int signo)
{
	signalled = 1;
}

void
reportstats(FILE *fd, struct statshandler *sh)
{
	sh->getstats(sh, sh->total);
	sh->reportverbose(sh, fd);
}

void
runstats(FILE *fd, struct statshandler *sh)
{
	u_long off;
	int line, omask;

	if (sh->interval < 1)
		sh->interval = 1;
	signal(SIGALRM, catchalarm);
	signalled = 0;
	alarm(sh->interval);
banner:
	sh->printbanner(sh, fd);
	fflush(fd);
	line = 0;
loop:
	if (line != 0) {
		sh->getstats(sh, sh->cur);
		sh->reportdelta(sh, fd);
		sh->update(sh);
	} else {
		sh->getstats(sh, sh->total);
		sh->reporttotal(sh, fd);
	}
	putc('\n', fd);
	fflush(fd);
	omask = sigblock(sigmask(SIGALRM));
	if (!signalled)
		sigpause(0);
	sigsetmask(omask);
	signalled = 0;
	alarm(sh->interval);
	line++;
	if (line == 21)		/* XXX tty line count */
		goto banner;
	else
		goto loop;
	/*NOTREACHED*/
}

void
reportcol(FILE *fd, u_int32_t v, const char *def_fmt,
	u_int32_t max, const char *alt_fmt)
{
	if (v < max)
		fprintf(fd, def_fmt, v);
	else {
		char unit = 'K';

		v /= 1024;
		max /= 10;
		if (v > max) { 
			v /= 1024, unit = 'M';
			if (v > max)
				v /= 1024, unit = 'G';
		}
		fprintf(fd, alt_fmt, v, unit);
	}
}
