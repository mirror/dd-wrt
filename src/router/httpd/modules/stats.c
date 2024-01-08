/*
 * stats.c
 *
 * Copyright (C) 2005 - 2021 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <bcmnvram.h>
#include <shutils.h>

extern int http_post(const char *server, char *buf, size_t count);

#define BUFSPACE 50000
int nvram_size(void);

int http_stats(const char *url)
{
	char *buf, *s;
	char **cur;
	char *secrets[] = { "os_server", "stats_server", "http_passwd", NULL };
	char *files[] = { "/proc/version",  "/proc/meminfo",
			  "/proc/cpuinfo",  "/proc/interrupts",
			  "/proc/net/dev",  "/proc/net/pppoe",
			  "/proc/net/snmp", NULL };
	char *contents;
	int NVRAMSPACE = nvram_size();
	if (!(buf = safe_malloc(NVRAMSPACE)))
		return errno;

	/*
	 * Get NVRAM variables 
	 */
	nvram_getall(buf, NVRAMSPACE);
	for (s = buf; *s; s++) {
		for (cur = secrets; *cur; cur++) {
			if (!strncmp(s, *cur, strlen(*cur))) {
				s += strlen(*cur) + 1;
				while (*s)
					*s++ = '*';
				break;
			}
		}
		*(s += strlen(s)) = '&';
	}

	/*
	 * Dump interesting /proc entries 
	 */
	for (cur = files; *cur; cur++) {
		if ((contents = file2str(*cur))) {
			s += snprintf(s, buf + BUFSPACE - s, "%s=%s&", *cur,
				      contents);
			debug_free(contents);
		}
	}

	/*
	 * Report uptime 
	 */
	s += snprintf(s, buf + BUFSPACE - s, "uptime=%lu&",
		      (unsigned long)time(NULL));

	/*
	 * Save 
	 */
	s += snprintf(s, buf + BUFSPACE - s, "action=save");
	buf[BUFSPACE - 1] = '\0';

	/*
	 * Post to server 
	 */
	http_post(url ?: nvram_safe_get("stats_server"), buf, BUFSPACE);

	debug_free(buf);
	return 0;
}
