/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
 \********************************************************************/

/**
  @file safe.c
  @brief Safe versions of stdlib/string functions that error out and exit if memory allocation fails
  @author Copyright (C) 2005 Mina Naguib <mina@ilesansfil.org>
  @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
 */

#define _GNU_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#include "safe.h"
#include "debug.h"

void * safe_calloc (size_t size)
{
	void * retval = NULL;
	retval = calloc(1, size);
	if (!retval) {
		debug(LOG_CRIT, "Failed to calloc %d bytes of memory: %s. Bailing out.", size, strerror(errno));
	}
	return (retval);
}

void * safe_malloc (size_t size)
{
	void * retval = NULL;
	retval = malloc(size);
	if (!retval) {
		debug(LOG_CRIT, "Failed to malloc %d bytes of memory: %s. Bailing out.", size, strerror(errno));
	}
	return (retval);
}

char * safe_strdup(const char s[])
{
	char * retval = NULL;
	if (!s) {
		debug(LOG_CRIT, "safe_strdup called with NULL which would have crashed strdup. Bailing out.");
		return ("error");
	}
	retval = strdup(s);
	if (!retval) {
		debug(LOG_CRIT, "Failed to duplicate a string: %s. Bailing out.", strerror(errno));
		return ("error");
	}
	return (retval);
}

int safe_asprintf(char **strp, const char *fmt, ...)
{
	va_list ap;
	int retval;

	va_start(ap, fmt);
	retval = safe_vasprintf(strp, fmt, ap);
	va_end(ap);

	return (retval);
}

int safe_vasprintf(char **strp, const char *fmt, va_list ap)
{
	int retval;

	retval = vasprintf(strp, fmt, ap);

	if (retval == -1) {
		debug(LOG_CRIT, "Failed to vasprintf: %s.  Bailing out", strerror(errno));
		return (retval);
	}

	return (retval);
}

pid_t safe_fork(void)
{
	pid_t result;
	result = fork();

	if (result == -1) {
		debug(LOG_CRIT, "Failed to fork: %s. Unable to continue - Exiting......", strerror(errno));
		sleep (5);
		abort();
	} else if (result == 0) {
		// I'm the child - do some cleanup
	}

	return result;
}
