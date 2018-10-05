/*
 * websfreeradius.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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
#ifdef HAVE_RAID
#define VALIDSOURCE 1

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else				/* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif				/* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <dd_defs.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>
#include <stdarg.h>
#include <sha1.h>

void add_raid(webs_t wp)
{
	int idx = 0;
	while (1) {
		char *type = nvram_nget("raidtype%d", idx);
		if (!strlen(type))
			break;
		idx++;
	}
	nvram_nset("mp", "raidtype%d", idx);
}

void del_raid(webs_t wp)
{
	char *val = websGetVar(wp, "del_value", NULL);
	if (!val)
		return;
	int idx = atoi(val);

	int cnt = 0;
	while (1) {
		char *type = nvram_nget("raidtype%d", idx);
		if (!strlen(type)) {
			break;
		}
		cnt++;
	}
	if (idx == 0) {
		nvram_nset(NULL, "raidtype%d", idx);
		nvram_nset(NULL, "raidname%d", idx);
		nvram_nset(NULL, "raidlevel%d", idx);
		nvram_nset(NULL, "raid%d", idx);
		nvram_nset(NULL, "raidfs%d", idx);
		return;
	}
	if (cnt == 0)
		return;
	int i;
	for (i = idx; i < cnt; i++) {
		nvram_nset(nvram_nget("raidtype%d", i), "raidtype%d", i);
		nvram_nset(nvram_nget("raidname%d", i), "raidname%d", i);
		nvram_nset(nvram_nget("raidlevel%d", i), "raidlevel%d", i);
		nvram_nset(nvram_nget("raidraid%d", i), "raidraid%d", i);
		nvram_nset(nvram_nget("raidfs%d", i), "raidfs%d", i);
	}
}

#endif
