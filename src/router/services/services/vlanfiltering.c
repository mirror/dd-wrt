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

#define iterate_filters(command)                                      \
	{                                                             \
		wordlist = nvram_safe_get("vlan_filters");            \
		foreach(word, wordlist, next) {                       \
			GETENTRYBYIDX(ifname, word, 0);               \
			GETENTRYBYIDX(vlan, word, 1);                 \
			GETENTRYBYIDX(pvid, word, 2);                 \
			GETENTRYBYIDX(untagged, word, 3);             \
			if (!ifname || !vlan || !pvid || !untagged) { \
				break;                                \
			}                                             \
			command;                                      \
		}                                                     \
	}

void start_vlanfiltering(void)
{
	char word[256];
	const char *next, *wordlist;
	char *args[4] = { NULL, NULL, NULL, NULL };

	iterate_filters({
		if (!isbridge(ifname)) {
			eval("bridge", "vlan", "del", "dev", ifname, "vid", "1"); /* del default pvid */
		}
	});
	iterate_filters({
		if (!isbridge(ifname)) {
			int cnt = 0;
			if (!strcmp(pvid, "1"))
				args[cnt++] = "pvid";
			if (!strcmp(untagged, "1"))
				args[cnt++] = "untagged";
			args[cnt++] = "master";
			args[cnt++] = NULL;
			char tmp[256];
			eval("bridge", "vlan", "del", "dev", getBridge(ifname, tmp), "vid", "1"); /* del default pvid if custom vlans are defined */
//			eval("bridge", "vlan", "add", "dev", getBridge(ifname, tmp), "vid", vlan,
//			     "self"); /* allow bridge to receive vlan with vid X */
			eval("bridge", "vlan", "add", "dev", ifname, "vid", vlan, args[0], args[1], args[2]); /* add vlan to port */
		}
	});
	iterate_filters({
		if (isbridge(ifname)) {
			eval("bridge", "vlan", "del", "dev", ifname, "vid",
			     vlan); /* if a custom filter for bridges is defined, remove it, this can also be used to override the default pvid */
		}
	});
	iterate_filters({
		if (isbridge(ifname)) {
			int cnt = 0;
			if (!strcmp(pvid, "1"))
				args[cnt++] = "pvid";
			if (!strcmp(untagged, "1"))
				args[cnt++] = "untagged";
			args[cnt++] = "self";
			args[cnt++] = NULL;
			eval("bridge", "vlan", "add", "dev", ifname, "vid", vlan, args[0], args[1],
			     args[2]); /* if a custom filter for brisges is defined, now add them all here */
		}
	});
}

void stop_vlanfiltering(void)
{
	char word[256];
	const char *next, *wordlist;

	iterate_filters({
		char tmp[256];
		eval("bridge", "vlan", "del", "dev", getBridge(ifname, tmp), "vid", vlan);
		eval("bridge", "vlan", "del", "dev", ifname, "vid", vlan);
		if (!nvram_nmatch("1", "%s_trunk", getBridge(ifname, tmp)))
			eval("bridge", "vlan", "add", "dev", ifname, "vid", "1", "pvid", "untagged"); /* add default pvid */
	});
}

#endif