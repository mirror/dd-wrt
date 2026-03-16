/* $Id: setproctitle.c,v 1.21 2012/10/22 15:15:58 karls Exp $ */
/* Based on conf.c from UCB sendmail 8.8.8 */

#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif /* HAVE_CONFIG_H */

#include "osdep.h"

/* Based on conf.c from UCB sendmail 8.8.8 */

/*
 * Copyright 2003 Damien Miller
 * Copyright (c) 1983, 1995-1997 Eric P. Allman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#endif

#define SPT_NONE	0	/* don't use it at all */
#define SPT_REUSEARGV	2	/* cover argv with title information */

#ifndef SPT_TYPE
# define SPT_TYPE	SPT_NONE
#endif

#ifndef SPT_PADCHAR
# define SPT_PADCHAR	'\0'
#endif

#if defined(SPT_TYPE) && SPT_TYPE == SPT_REUSEARGV
static char *argv_start = NULL;
static size_t argv_env_len = 0;
#endif /* defined(SPT_TYPE) && SPT_TYPE == SPT_REUSEARGV */

extern char *__progname;

void
initsetproctitle(int argc, char *argv[])
{
#if defined(SPT_TYPE) && SPT_TYPE == SPT_REUSEARGV
	char *lastargv = NULL;
	int i;

	/*
	 * NB: This assumes that argv has already been copied out of the
	 * way. This is true for sshd, but may not be true for other
	 * programs. Beware.
	 */

	if (argc == 0 || argv[0] == NULL)
		return;

	/*
	 * Find the last argv string within our process memory area.
	 */
	for (i = 0; i < argc; i++) {
		if (lastargv == NULL || lastargv + 1 == argv[i])
			lastargv = argv[i] + strlen(argv[i]);
	}
	argv[1] = NULL;
	argv_start = argv[0];
	argv_env_len = lastargv - argv[0];

#endif /* defined(SPT_TYPE) && SPT_TYPE == SPT_REUSEARGV */
}

void
setproctitle(const char *fmt, ...)
{
#if defined(SPT_TYPE) && SPT_TYPE == SPT_REUSEARGV
	va_list ap;
	char buf[1024];
	size_t len;

	if (argv_env_len == 0)
		return;

	strlcpy(buf, __progname, sizeof(buf));

	va_start(ap, fmt);
	if (fmt != NULL) {
		len = strlcat(buf, ": ", sizeof(buf));
		if (len < sizeof(buf))
			vsnprintf(buf + len, sizeof(buf) - len , fmt, ap);
	}
	va_end(ap);

/*	slog(LOG_DEBUG, "setproctitle: copy \"%s\" into len %ld",
	    buf, argv_env_len); */
	len = strlcpy(argv_start, buf, argv_env_len);
	for(; len < argv_env_len; len++)
		argv_start[len] = SPT_PADCHAR;
#endif /* defined(SPT_TYPE) && SPT_TYPE == SPT_REUSEARGV */
}
