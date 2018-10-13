/*
 * debug.c -- debugging routines for libtirpc
 *
 * Copyright (C) 2014  Red Hat, Steve Dickson <steved@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "debug.h"

/* library global debug level */
int libtirpc_debug_level = 0;
int  log_stderr = 1; /* log to stderr instead of systlog */

/*
 * Set the debug level for the entire library.
 * Different area will used the value to determin
 * the verbosity of the debugging output.
 */
void
libtirpc_set_debug(char *name, int level, int use_stderr)
{
	if (level < 0)
		level = 0;

	log_stderr = use_stderr;
	if (!use_stderr)
		openlog(name, LOG_PID, LOG_DAEMON);

	libtirpc_debug_level = level;
	LIBTIRPC_DEBUG(1, ("libtirpc: debug level %d", libtirpc_debug_level));
}

void
libtirpc_log_dbg(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if (log_stderr) {
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
	} else
		vsyslog(LOG_NOTICE, fmt, args);
	va_end(args);
}
