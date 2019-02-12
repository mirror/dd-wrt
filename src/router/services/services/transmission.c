/*
 * transmission.c
 *
 * Copyright (C) 2013 Sebastian Gottschall <gottschall@dd-wrt.com>
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

#ifdef HAVE_TRANSMISSION
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
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>

static char *config = {
	"{\n"
	    "\"alt-speed-down\": 50,\n"
	    "\"alt-speed-enabled\": true,\n"
	    "\"alt-speed-time-begin\": 540,\n"
	    "\"alt-speed-time-day\": 127,\n"
	    "\"alt-speed-time-enabled\": false,\n"
	    "\"alt-speed-time-end\": 1020,\n"
	    "\"alt-speed-up\": 50,\n"
	    "\"bind-address-ipv4\": \"0.0.0.0\",\n"
	    "\"bind-address-ipv6\": \"::\",\n"
	    "\"blocklist-enabled\": false,\n"
	    "\"blocklist-url\": \"http://www.example.com/blocklist\",\n"
	    "\"cache-size-mb\": 4,\n"
	    "\"dht-enabled\": true,\n"
	    "\"download-dir\": \"%s\",\n"
	    "\"download-limit\": 100,\n"
	    "\"download-limit-enabled\": 0,\n"
	    "\"download-queue-enabled\": true,\n"
	    "\"download-queue-size\": 5,\n"
	    "\"encryption\": 0,\n"
	    "\"idle-seeding-limit\": 30,\n"
	    "\"idle-seeding-limit-enabled\": false,\n"
	    "\"incomplete-dir\": \"%s/incomplete\",\n"
	    "\"incomplete-dir-enabled\": false,\n"
	    "\"lpd-enabled\": true,\n"
	    "\"max-peers-global\": 200,\n"
	    "\"message-level\": 2,\n"
	    "\"peer-congestion-algorithm\": \"\",\n"
	    "\"peer-id-ttl-hours\": 6,\n"
	    "\"peer-limit-global\": 200,\n"
	    "\"peer-limit-per-torrent\": 50,\n"
	    "\"peer-port\": 62708,\n"
	    "\"peer-port-random-high\": 65535,\n"
	    "\"peer-port-random-low\": 49152,\n"
	    "\"peer-port-random-on-start\": true,\n"
	    "\"peer-socket-tos\": \"default\",\n"
	    "\"pex-enabled\": true,\n"
	    "\"port-forwarding-enabled\": true,\n"
	    "\"preallocation\": 1,\n"
	    "\"prefetch-enabled\": true,\n"
	    "\"queue-stalled-enabled\": true,\n"
	    "\"queue-stalled-minutes\": 30,\n"
	    "\"ratio-limit\": 2,\n"
	    "\"ratio-limit-enabled\": false,\n"
	    "\"rename-partial-files\": true,\n"
	    "\"rpc-authentication-required\": true,\n"
	    "\"rpc-bind-address\": \"0.0.0.0\",\n"
	    "\"rpc-enabled\": true,\n"
	    "\"rpc-host-whitelist\": \"\",\n"
	    "\"rpc-host-whitelist-enabled\": true,\n"
	    "\"rpc-password\": \"{572778e48a0d0f5104d970ed9fc69f6f98f03fd6ozr1fmao\",\n"
	    "\"rpc-port\": 9091,\n"
	    "\"rpc-url\": \"/transmission/\",\n"
	    "\"rpc-username\": \"transmission\",\n"
	    "\"rpc-whitelist\": \"127.0.0.1 %s\",\n"
	    "\"rpc-whitelist-enabled\": true,\n"
	    "\"scrape-paused-torrents-enabled\": true,\n"
	    "\"script-torrent-done-enabled\": false,\n"
	    "\"script-torrent-done-filename\": \"\",\n"
	    "\"seed-queue-enabled\": false,\n"
	    "\"seed-queue-size\": 10,\n"
	    "\"speed-limit-down\": 100,\n"
	    "\"speed-limit-down-enabled\": false,\n"
	    "\"speed-limit-up\": 100,\n"
	    "\"speed-limit-up-enabled\": false,\n"
	    "\"start-added-torrents\": true,\n"
	    "\"trash-original-torrent-files\": false,\n" 
	    "\"umask\": 18,\n" 
	    "\"upload-limit\": 100,\n" 
	    "\"upload-limit-enabled\": 0,\n" 
	    "\"upload-slots-per-torrent\": 14,\n" 
	    "\"utp-enabled\": true\n" "}"
};

void start_transmission(void)
{
	if (!nvram_matchi("transmission_enable", 1))
		return;

	eval("mkdir", "-p", nvram_safe_get("transmission_dir"));

	char allowed[64];
	strcpy(allowed, nvram_safe_get("lan_ipaddr"));
	char *p = strrchr(allowed, '.');
	if (p) {
		p[1] = '*';
		p[2] = 0;
	}
	char path[256];
	snprintf(path, sizeof(path), "%s/settings.json", nvram_safe_get("transmission_dir"));
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		fp = fopen(path, "wb");
		if (fp) {
			fprintf(fp, config, nvram_safe_get("transmission_download"), nvram_safe_get("transmission_download"), allowed);
		}
	}
	if (fp)
		fclose(fp);
	sysprintf("echo 16777216 > /proc/sys/net/core/rmem_max");
	sysprintf("echo 4194304 > /proc/sys/net/core/wmem_max");
	eval("transmissiond", "--config-dir", nvram_safe_get("transmission_dir"));
	dd_loginfo("transmission", "daemon successfully started\n");
	return;
}

void stop_transmission(void)
{
	stop_process("transmission", "daemon");
}
#endif
