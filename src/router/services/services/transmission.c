/*
 * transmission.c
 *
 * Copyright (C) 2013 - 2019 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
	int alloc;
	int type;
};

static struct config_val config[] = { { "alt-speed-down", "50", 0, 0 },
				      { "alt-speed-enabled", "false", 0, 0 },
				      { "alt-speed-time-begin", "540", 0, 0 },
				      { "alt-speed-time-day", "127", 0, 0 },
				      { "alt-speed-time-enabled", "false", 0, 0 },
				      { "alt-speed-time-end", "1020", 0, 0 },
				      { "alt-speed-up", "50", 0, 0 },
				      { "bind-address-ipv4", "0.0.0.0", 0, 0 },
				      { "bind-address-ipv6", "::", 0, 0 },
				      { "blocklist-enabled", "false", 0, 0 },
				      { "blocklist-url", "http://www.example.com/blocklist", 0, 0 },
				      { "cache-size-mb", "4", 0, 0 },
				      { "dht-enabled", "true", 0, 0 },
				      { "download-dir", "", 0, 0 },
				      { "download-limit", "100", 0, 0 },
				      { "download-limit-enabled", "0", 0, 0 },
				      { "download-queue-enabled", "true", 0, 0 },
				      { "download-queue-size", "5", 0, 0 },
				      { "encryption", "1", 0, 0 },
				      { "idle-seeding-limit", "30", 0, 0 },
				      { "idle-seeding-limit-enabled", "false", 0, 0 },
				      { "incomplete-dir", "", 0, 0 },
				      { "incomplete-dir-enabled", "false", 0, 0 },
				      { "lpd-enabled", "true", 0, 0 },
				      { "max-peers-global", "200", 0, 0 },
				      { "message-level", "4", 0, 0 },
				      { "peer-congestion-algorithm", "", 0, 0 },
				      { "peer-id-ttl-hours", "6", 0, 0 },
				      { "peer-limit-global", "200", 0, 0 },
				      { "peer-limit-per-torrent", "50", 0, 0 },
				      { "peer-port", "62708", 0, 0 },
				      { "peer-port-random-high", "65535", 0, 0 },
				      { "peer-port-random-low", "49152", 0, 0 },
				      { "peer-port-random-on-start", "true", 0, 0 },
				      { "peer-socket-tos", "default", 0, 0 },
				      { "pex-enabled", "true", 0, 0 },
				      { "port-forwarding-enabled", "true", 0, 0 },
				      { "preallocation", "1", 0, 0 },
				      { "prefetch-enabled", "true", 0, 0 },
				      { "queue-stalled-enabled", "true", 0, 0 },
				      { "queue-stalled-minutes", "30", 0, 0 },
				      { "ratio-limit", "2", 0, 0 },
				      { "ratio-limit-enabled", "false", 0, 0 },
				      { "rename-partial-files", "true", 0, 0 },
				      { "rpc-authentication-required", "true", 0, 0 },
				      { "rpc-bind-address", "0.0.0.0", 0, 0 },
				      { "rpc-enabled", "true", 0, 0 },
				      { "rpc-host-whitelist", "", 0, 0 },
				      { "rpc-host-whitelist-enabled", "true", 0, 0 },
				      { "rpc-password", "{572778e48a0d0f5104d970ed9fc69f6f98f03fd6ozr1fmao", 0, 0 },
				      { "rpc-port", "9091", 0, 0 },
				      { "rpc-socket-mode", "0750", 0, 0 },
				      { "rpc-url", "/transmission/", 0, 0 },
				      { "rpc-username", "transmission", 0, 0 },
				      { "rpc-whitelist", "127.0.0.1", 0, 0 },
				      { "rpc-whitelist-enabled", "true", 0, 0 },
				      { "scrape-paused-torrents-enabled", "true", 0, 0 },
				      { "script-torrent-done-enabled", "false", 0, 0 },
				      { "script-torrent-done-filename", "", 0, 0 },
				      { "seed-queue-enabled", "false", 0, 0 },
				      { "seed-queue-size", "10", 0, 0 },
				      { "speed-limit-down", "100", 0, 0 },
				      { "speed-limit-down-enabled", "false", 0, 0 },
				      { "speed-limit-up", "100", 0, 0 },
				      { "speed-limit-up-enabled", "false", 0, 0 },
				      { "start-added-torrents", "true", 0, 0 },
				      { "tcp-enabled", "true", 0, 0 },
				      { "torrent-added-verify-mode", "fast", 0, 0 },
				      { "trash-original-torrent-files", "false", 0, 0 },
				      { "umask", "022", 0, 0 },
				      { "upload-limit", "100", 0, 0 },
				      { "upload-limit-enabled", "0", 0, 0 },
				      { "upload-slots-per-torrent", "14", 0, 0 },
				      { "utp-enabled", "true", 0, 0 },
				      { "watch-dir", "", 0, 0 },
				      { "watch-dir-enabled", "false", 0, 0 } };

static int isnum(char *str)
{
	int len = strlen(str);
	int i;
	if (!len)
		return 0;
	if (str[0] == '0')
		return 0;
	for (i = 0; i < len; i++) {
		if (!isdigit(str[i]))
			return 0;
	}
	return 1;
}

static void set_config_alloc(char *name, char *val, int alloc, int type)
{
	int count = sizeof(config) / sizeof(struct config_val);
	int i;
	for (i = 0; i < count; i++) {
		if (!strcmp(name, config[i].name)) {
			config[i].val = val;
			config[i].alloc = alloc;
			config[i].type = type;
		}
	}
}

static void set_config(char *name, char *val, int type)
{
	set_config_alloc(name, val, 0, type);
}

static void parse_config(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/settings.json", nvram_safe_get("transmission_dir"));
	FILE *fp = fopen(path, "rb");
	if (fp) {
		while (!feof(fp)) {
			char str[512] = { 0 };
			fgets(str, sizeof(str), fp);
			char *name = strchr(str, '"');
			if (!name)
				continue;
			name++;
			char *val = strchr(name, '"');
			if (!val)
				continue;
			val[0] = 0;
			val += 3;
			if (strchr(val, '"'))
				val++;
			char *end = strchr(val, '"');
			if (end)
				end[0] = 0;
			else {
				end = strchr(val, ',');
				if (end)
					end[0] = 0;
				else
					end = strchr(val, '\n');
				if (end)
					end[0] = 0;
			}
			if (!strcmp(name, "rpc-password") || !strcmp(name, "rpc-username"))
				set_config_alloc(name, strdup(val), 1, 1);
			else
				set_config_alloc(name, strdup(val), 1, 0);
		}
		fclose(fp);
	}
}

static void writeconfig(FILE *fp)
{
	int i;
	int count = sizeof(config) / sizeof(struct config_val);
	fprintf(fp, "{\n");
	for (i = 0; i < count; i++) {
		char *name = config[i].name;
		if (!config[i].val[0])
			fprintf(fp, "\t\"%s\": \"\"", name);
		else if (config[i].type)
			fprintf(fp, "\t\"%s\": \"%s\"", name, config[i].val);
		else if (!strcmp(config[i].val, "false") || !strcmp(config[i].val, "true") || isnum(config[i].val))
			fprintf(fp, "\t\"%s\": %s", name, config[i].val);
		else
			fprintf(fp, "\t\"%s\": \"%s\"", name, config[i].val);
		if (i == count - 1)
			fprintf(fp, "\n");
		else
			fprintf(fp, ",\n");
	}
	fprintf(fp, "}\n");
}

char *transmission_deps(void)
{
	return "transmission_enable transmission_dir lan_ipaddr transmission_download transmission_whitelist transmission_rpc transmission_username transmission_password";
}

char *transmission_proc(void)
{
	return "transmissiond";
}

void stop_transmission(void);

void start_transmission(void)
{
	stop_transmission(); // write config if present
	if (!nvram_matchi("transmission_enable", 1))
		return;
	parse_config(); // read it back and parse it

	eval("mkdir", "-p", nvram_safe_get("transmission_dir"));

	char allowed[64];
	strcpy(allowed, nvram_safe_get("lan_ipaddr"));
	char *p = strrchr(allowed, '.');
	if (p) {
		p[1] = '*';
		p[2] = 0;
	}
	int count = sizeof(config) / sizeof(struct config_val);
	int i;
	char path[256];
	snprintf(path, sizeof(path), "%s/settings.json", nvram_safe_get("transmission_dir"));
	FILE *fp = fopen(path, "wb");
	if (fp) {
		set_config("download-dir", nvram_safe_get("transmission_download"), 1);
		char inc[512];
		snprintf(inc, sizeof(inc), "%s/incomplete", nvram_safe_get("transmission_download"));
		set_config("incomplete-dir", strdup(inc), 1);
		char allow[512];
		snprintf(allow, sizeof(allow), "127.0.0.1,%s", allowed);
		char *whitelist = nvram_safe_get("transmission_whitelist");
		if (*whitelist)
			sprintf(allow, "%s,%s", allow, whitelist);
		set_config_alloc("rpc-whitelist", strdup(allow), 1, 0);
		set_config("rpc-whitelist-enabled", "true", 0);
		set_config_alloc("rpc-port", strdup(nvram_safe_get("transmission_rpc")), 1, 0);
		set_config_alloc("rpc-username", strdup(nvram_safe_get("transmission_username")), 1, 1);
		set_config_alloc("rpc-password", strdup(nvram_safe_get("transmission_password")), 1, 1);
		if (!nvram_match("transmission_script", "")) {
			set_config("script-torrent-done-enabled", "true", 0);
			set_config_alloc("script-torrent-done-filename", strdup(nvram_safe_get("transmission_script")), 1, 1);
		}
		char *down = nvram_safe_get("transmission_down");
		if (*down && *down != '\r' && *down != '\n') {
			set_config("speed-limit-down-enabled", "true", 0);
			set_config_alloc("speed-limit-down", strdup(down), 1, 0);
		}
		char *up = nvram_safe_get("transmission_up");
		if (*up && *up != '\r' && *up != '\n') {
			set_config("speed-limit-up-enabled", "true", 0);
			set_config_alloc("speed-limit-up", strdup(up), 1, 0);
		}
		set_config("rpc-authentication-required", "true", 0);
		writeconfig(fp);
	}
	if (fp)
		fclose(fp);
	for (i = 0; i < count; i++) {
		if (config[i].alloc)
			free(config[i].val);
	}
	writeprocsysnet("core/rmem_max", "16777216");
	writeprocsysnet("core/wmem_max", "4194304");
	writeprocsysnet("ipv4/tcp_adv_win_scale", "4");

	char *web = nvram_default_get("transmission_style", "default");
	sysprintf("export TRANSMISSION_WEB_HOME=\"/usr/share/transmission/%s\" && transmissiond --config-dir \"%s\"", web,
		  nvram_safe_get("transmission_dir"));
	dd_loginfo("transmission", "daemon successfully started\n");

	return;
}

void stop_transmission(void)
{
	stop_process_timeout("transmissiond", "daemon", 10);
	nvram_delstates(transmission_deps());
}

#endif

#ifdef TEST
int main(int argc, char *argv[])
{
	start_transmission();
}
#endif
