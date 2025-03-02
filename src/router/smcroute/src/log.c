/* System logging API
 *
 * Copyright (C) 2011-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#define SYSLOG_NAMES

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "log.h"
#include "conf.h"
#include "util.h"

int  log_level = LOG_NOTICE;
char log_message[128];

/**
 * loglvl - Convert log level string to value
 * @level: String from user, debug, error, warning, etc.
 *
 * Returns:
 * Matching %LOG_DEBUG, %LOG_ERR, etc.
 */
int loglvl(const char *level)
{
	int i;

	for (i = 0; prioritynames[i].c_name; i++) {
		size_t len = MIN(strlen(prioritynames[i].c_name), strlen(level));

		if (!strncasecmp(prioritynames[i].c_name, level, len))
			return prioritynames[i].c_val;
	}

	return atoi(level);
}

/**
 * smclog - Log message to syslog or stderr
 * @severity: Standard syslog() severity levels
 * @fmt:      Standard printf() formatted message to log
 *
 * Logs a standard printf() formatted message to syslog and stderr when
 * @severity is greater than the @log_level threshold.  When @code is
 * set it is appended to the log, along with the error message.
 *
 * When @severity is %LOG_ERR or worse this function will call exit().
 */
void smclog(int severity, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if (!conf_vrfy) {
		vsnprintf(log_message, sizeof(log_message), fmt, args);
	} else {
		if (severity <= log_level) {
			vprintf(fmt, args);
			puts("");
		}
	}
	va_end(args);

	syslog(severity, "%s", log_message);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
