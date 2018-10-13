/*
 * debug.h -- debugging routines for libtirpc
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

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdarg.h>
#include <syslog.h>

extern int libtirpc_debug_level;
extern int  log_stderr;

void    libtirpc_log_dbg(char *format, ...);
void 	libtirpc_set_debug(char *name, int level, int use_stderr);

#define LIBTIRPC_DEBUG(level, msg) \
	do { \
		if (level <= libtirpc_debug_level) \
			libtirpc_log_dbg msg; \
	} while (0)

static inline void 
vlibtirpc_log_dbg(int level, const char *fmt, va_list args)
{
	if (level <= libtirpc_debug_level) {
		if (log_stderr) {
			vfprintf(stderr, fmt, args);
			fprintf(stderr, "\n");
		} else
			vsyslog(LOG_NOTICE, fmt, args);
	}
}
#endif /* _DEBUG_H */
