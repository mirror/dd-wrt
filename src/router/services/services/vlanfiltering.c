/*
 * vlanfiltering.c
 *
 * Copyright (C) 2007 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/sysinfo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ddnvram.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>
#include <services.h>
#include <libbridge.h>

#ifdef HAVE_VLAN_FILTERING

void start_vlanfiltering(void)
{
	char word[256];
	const char *next, *wordlist;

	wordlist = nvram_safe_get("vlan_filters");
	foreach(word, wordlist, next) {
		GETENTRYBYIDX(ifname, word, 0);
		GETENTRYBYIDX(vlan, word, 1);
		GETENTRYBYIDX(pvid, word, 2);
		GETENTRYBYIDX(untagged, word, 3);
		if (!ifname || !vlan || !pvid || !untagged) {
			break;
		}
		char *args[4] = { NULL, NULL, NULL, NULL };
		int cnt = 0;
		if (!strcmp(pvid, "1"))
			args[cnt++] = "pvid";
		if (!strcmp(untagged, "1"))
			args[cnt++] = "untagged";
		args[cnt++] = "master";
		eval("bridge", "vlan", "add", "dev", getBridge(ifname), "vid", vlan, "self");
		eval("bridge", "vlan", "add", "dev", ifname, "vid", vlan, args[0], args[1], args[2]);
	}
}

void stop_vlanfiltering(void)
{
	char word[256];
	const char *next, *wordlist;

	wordlist = nvram_safe_get("vlan_filters");
	foreach(word, wordlist, next) {
		GETENTRYBYIDX(ifname, word, 0);
		GETENTRYBYIDX(vlan, word, 1);
		GETENTRYBYIDX(pvid, word, 2);
		GETENTRYBYIDX(untagged, word, 3);
		if (!ifname || !vlan || !pvid || !untagged) {
			break;
		}
		eval("bridge", "vlan", "del", "dev", getBridge(ifname), "vid", vlan);
		eval("bridge", "vlan", "del", "dev", ifname, "vid", vlan);
	}
}

#endif