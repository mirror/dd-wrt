/*
 * arpd.c
 *
 * Copyright (C) 2009 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <ddnvram.h>
#include <shutils.h>
#include <services.h>

void start_arpd(void)
{
	char vifs[512];
	char *ifnames[128] = { "arpd", "-b", NULL, "-a", "3", "-k" };
	int cnt = 0;
#if defined(HAVE_MVEBU) || defined(HAVE_IPQ6018)
	getIfListNoPorts(vifs, sizeof(vifs), "ixp vlan wan lan 10g-sfp 10g-copper");
#else
	getIfListNoPorts(vifs, sizeof(vifs), "eth ixp vlan wan lan 10g-sfp 10g-copper");
#endif
	char var[32], *wordlist;
	const char *next;
	char *dbname;
	if (!jffs_mounted() && (freediskSpace("/jffs") < 8 * 1024 * 1024))
		ifnames[2] = "/tmp/arpd.db";
	else
		ifnames[2] = "/jffs/arpd.db";
	cnt = 6;
	if (nvram_match("arpd_enable", "1"))
		ifnames[cnt++] = strdup("br0");
	int active = 0;
	foreach(var, vifs, next) {
		if (cnt == ARRAY_SIZE(ifnames))
			break;
		if (nvram_nmatch("1", "%s_arpd", var))
			ifnames[cnt++] = strdup(var);
		active = 1;
	}
	if (active)
		_log_evalpid(ifnames, NULL, 0, NULL);
	int i;
	for (i = 6; i < cnt; i++)
		free(ifnames[i]);
	return;
}

void restart_arpd(void)
{
	stop_arpd();
	start_arpd();
}

void stop_arpd(void)
{
	stop_process("arpd", "daemon");

	return;
}
