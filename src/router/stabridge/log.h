/*
 * Copyright (c) 2005 by Wilibox (www.wilibox.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef __LOG_H__
#define __LOG_H__

#include <string.h>
#include <errno.h>
#include <stdio.h>

#ifdef STDERR_LOGGER

#define error(args...) do { \
	fprintf(stderr, "Error: "); \
	fprintf(stderr, args); \
} while (0)

#define error_errno(args...) do { \
	fprintf(stderr, "Error: "); \
	fprintf(stderr, args); \
	fprintf(stderr, "%s\n", strerror(errno)); \
} while (0)

#define warn(args...) do { \
	printf("WARN: "); \
	printf(args); \
} while (0)

#define info(args...) do { \
	printf(args); \
} while (0)

#define debug(args...) do { \
	printf("DEBUG: "); \
	printf(args); \
} while (0)

#else /* #ifdef STDERR_LOGGER */

#include <syslog.h>

#define error(args...) do { \
	syslog(LOG_ERR, args); \
} while (0)

#define error_errno(args...) do { \
	syslog(LOG_ERR, args); \
	syslog(LOG_ERR, "%s", strerror(errno)); \
} while (0)

#define warn(args...) do { \
	syslog(LOG_WARNING, args); \
} while (0)

#define info(args...) do { \
	syslog(LOG_INFO, args); \
} while (0)

#define debug(args...) do { \
	syslog(LOG_DEBUG, args); \
} while (0)

#endif

#endif
