/************************************************************************\
 * This program is free software; you can redistribute it and/or	*
 * modify it under the terms of the GNU General Public License as	*
 * published by the Free:Software Foundation; either version 2 of	*
 * the License, or (at your option) any later version.			*
 *									*
 * This program is distributed in the hope that it will be useful,	*
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	*
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.		*
 * See the GNU General Public License for more details.		*
\************************************************************************/

/** @internal
 * @file http_microhttpd_utils.c
 * @brief a httpd implementation using libmicrohttpd
 * @author Copyright (C) 2015 Alexander Couzens <lynxis@fe80.eu>
 * @author Copyright (C) 2015-2023 The openNDS contributors <opennds@blue-wave.net>
 * @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
 */


#ifndef _UHTTPD_UTILS_

#include <sys/stat.h>

#include <stdarg.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <alloca.h>
#include <time.h>
#include <unistd.h>

/*struct uh_addr {
	uint8_t family;
	uint16_t port;
	union {
		struct in_addr in;
		struct in6_addr in6;
	};
};
*/

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))

#define array_size(x) \
	(sizeof(x) / sizeof(x[0]))

#define fd_cloexec(fd) \
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC)

#ifdef __GNUC__
#define __printf(a, b) __attribute__((format(printf, a, b)))
#else
#define __printf(a, b)
#endif

int htmlentityencode(char *buf, int blen, const char *src, int slen);
int uh_urldecode(char *buf, int blen, const char *src, int slen);
int uh_urlencode(char *buf, int blen, const char *src, int slen);
int uh_b64decode(char *buf, int blen, const void *src, int slen);
int b64_encode(char *buf, int blen, const void *src, int slen);

#endif
