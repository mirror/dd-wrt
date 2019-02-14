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
#include <ctype.h>
struct config_val {
	char *name;
	char *val;
};

static struct config_val config[] = {
	{"alt-speed-down", "50"},
	{"alt-speed-enabled", "true"},
	{"alt-speed-time-begin", "540"},
	{"alt-speed-time-day", "127"},
	{"alt-speed-time-enabled", "false"},
	{"alt-speed-time-end", "1020"},
	{"alt-speed-up", "50"},
	{"bind-address-ipv4", "0.0.0.0"},
	{"bind-address-ipv6", "::"},
	{"blocklist-enabled", "false"},
	{"blocklist-url", "http://www.example.com/blocklist"},
	{"cache-size-mb", "4"},
	{"dht-enabled", "true"},
	{"download-dir", ""},
	{"download-limit", "100"},
	{"download-limit-enabled", "0"},
	{"download-queue-enabled", "true"},
	{"download-queue-size", "5"},
	{"encryption", "0"},
	{"idle-seeding-limit", "30"},
	{"idle-seeding-limit-enabled", "false"},
	{"incomplete-dir", ""},
	{"incomplete-dir-enabled", "false"},
	{"lpd-enabled", "true"},
	{"max-peers-global", "200"},
	{"message-level", "2"},
	{"peer-congestion-algorithm", ""},
	{"peer-id-ttl-hours", "6"},
	{"peer-limit-global", "200"},
	{"peer-limit-per-torrent", "50"},
	{"peer-port", "62708"},
	{"peer-port-random-high", "65535"},
	{"peer-port-random-low", "49152"},
	{"peer-port-random-on-start", "true"},
	{"peer-socket-tos", "default"},
	{"pex-enabled", "true"},
	{"port-forwarding-enabled", "true"},
	{"preallocation", "1"},
	{"prefetch-enabled", "true"},
	{"queue-stalled-enabled", "true"},
	{"queue-stalled-minutes", "30"},
	{"ratio-limit", "2"},
	{"ratio-limit-enabled", "false"},
	{"rename-partial-files", "true"},
	{"rpc-authentication-required", "true"},
	{"rpc-bind-address", "0.0.0.0"},
	{"rpc-enabled", "true"},
	{"rpc-host-whitelist", ""},
	{"rpc-host-whitelist-enabled", "true"},
	{"rpc-password", "{572778e48a0d0f5104d970ed9fc69f6f98f03fd6ozr1fmao"},
	{"rpc-port", "9091"},
	{"rpc-url", "/transmission/"},
	{"rpc-username", "transmission"},
	{"rpc-whitelist", "127.0.0.1"},
	{"rpc-whitelist-enabled", "true"},
	{"scrape-paused-torrents-enabled", "true"},
	{"script-torrent-done-enabled", "false"},
	{"script-torrent-done-filename", ""},
	{"seed-queue-enabled", "false"},
	{"seed-queue-size", "10"},
	{"speed-limit-down", "100"},
	{"speed-limit-down-enabled", "false"},
	{"speed-limit-up", "100"},
	{"speed-limit-up-enabled", "false"},
	{"start-added-torrents", "true"},
	{"trash-original-torrent-files", "false"},
	{"umask", "18"},
	{"upload-limit", "100"},
	{"upload-limit-enabled", "0"},
	{"upload-slots-per-torrent", "14"},
	{"utp-enabled", "true"}
};

static int isnum(char *str)
{
	int len = strlen(str);
	int i;
	for (i = 0; i < len; i++) {
		if (!isdigit(str[i]))
			return 0;
	}
	return 1;
}

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
			int count = sizeof(config) / sizeof(struct config_val);
			int i;
			fprintf(fp, "{\n");
			for (i = 0; i < count; i++) {
				char *name = config[i].name;
				if (!strcmp(name, "download-dir"))
					fprintf(fp, "\t\"%s\": \"%s\",\n", name, nvram_safe_get("transmission_download"));
				else if (!strcmp(name, "incomplete-dir"))
					fprintf(fp, "\t\"%s\": \"%s/incomplete\",\n", name, nvram_safe_get("transmission_download"));
				else if (!strcmp(name, "rpc-whiteliste"))
					fprintf(fp, "\t\"%s\": \"127.0.0.1, %s\",\n", name, allowed);
				else if (!strcmp(name, "rpc-port"))
					fprintf(fp, "\t\"%s\": %s,\n", name, nvram_safe_get("transmission_rpc"));
				else if (!strcmp(config[i].val, "false"))
					fprintf(fp, "\t\"%s\": false,\n", name);
				else if (!strcmp(config[i].val, "true"))
					fprintf(fp, "\t\"%s\": true,\n", name);
				else if (!isnum(config[i].val))
					fprintf(fp, "\t\"%s\": %s,\n", name, config[i].val);
				else
					fprintf(fp, "\t\"%s\": \"%s\",\n", name, config[i].val);
			}
			fprintf(fp, "}\n");

		}
	}
	if (fp)
		fclose(fp);
	sysprintf("echo 16777216 > /proc/sys/net/core/rmem_max");
	sysprintf("echo 4194304 > /proc/sys/net/core/wmem_max");
	char *web = nvram_default_get("transmission_style", "default");
	sysprintf("export TRANSMISSION_WEB_HOME=\"/usr/share/transmission/%s\" && transmissiond --config-dir \"%s\"", web, nvram_safe_get("transmission_dir"));
	dd_loginfo("transmission", "daemon successfully started\n");
	return;
}

void stop_transmission(void)
{
	stop_process("transmissiond", "daemon");
}
#endif
