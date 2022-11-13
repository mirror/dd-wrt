/*
 * ddns.c
 *
 * Copyright (C) 2006-2022 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <syslog.h>
#include <sys/wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <rc.h>
#include <services.h>

/*
 * inadyn scripts by lawnmowerguy1 
 */

char _username[] = "ddns_username_XX";
char _passwd[] = "ddns_passwd_XX";
char _hostname[] = "ddns_hostname_XX";
char _wildcard[] = "ddns_wildcard_XX";
char _ssl[] = "ddns_ssl_XX";
char _cache_file[128];
static int init_ddns(FILE * fp)
{

	int flag = nvram_geti("ddns_enable");
	if (flag > 32 || flag < 1)
		return -1;
	char *providers[] = {
		NULL,
		"default@dyndns.org",
		"default@freedns.afraid.org",
		"default@zoneedit.com",
		"default@no-ip.com",
		NULL,
		"dyndns@3322.org",
		"default@easydns.com",
		"default@dnsomatic.com",
		"default@selfhost.de",
		"dyndns@he.net",
		"default@duiadns.net",
		"default@tunnelbroker.net",
		"default@spdyn.de",
		"ipv4@nsupdate.info",
		"default@loopia.com",
		"default@domains.google.com",
		"default@dynu.com",
		"default@changeip.com",
		"default@ovh.com",
		"default@strato.com",
		"default@cloudflare.com",
		"default@cloudxns.net",
		"default@ddnss.de",
		"default@dhis.org",
		"default@dnsexit.com",
		"default@dnspod.cn",
		"default@duckdns.org",
		"default@freemyip.com",
		"default@gira.de",
		"default@sitelutions.com",
		"default@pdd.yandex.ru",
		"default@ipv4.dynv6.com",
	};
	char *provider = providers[flag];
	snprintf(_ssl, sizeof(_hostname), "%s", "ddns_ssl");
	if (flag == 1) {
		snprintf(_username, sizeof(_username), "%s", "ddns_username");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname");
		snprintf(_wildcard, sizeof(_hostname), "%s", "ddns_wildcard");
		snprintf(_cache_file, sizeof(_cache_file), "/tmp/ddns/%s.cache", nvram_safe_get("ddns_hostname"));
	} else {
		snprintf(_username, sizeof(_username), "%s_%d", "ddns_username", flag);
		snprintf(_passwd, sizeof(_passwd), "%s_%d", "ddns_passwd", flag);
		snprintf(_hostname, sizeof(_hostname), "%s_%d", "ddns_hostname", flag);
		snprintf(_wildcard, sizeof(_hostname), "%s_%d", "ddns_wildcard", flag);
		snprintf(_cache_file, sizeof(_cache_file), "/tmp/ddns/%s.cache", nvram_nget("ddns_hostname_%d", flag));
	}
	if (fp) {
		if (flag == 5)
			fprintf(fp, "custom namecheap {\n");
		else
			fprintf(fp, "provider %s {\n", provider);
		if (flag != 28 && flag != 11)
			fprintf(fp, "username = \"%s\"\n", nvram_safe_get(_username));
		if (flag == 27)
			fprintf(fp, "password = \"nopasswd\"\n");
		else
			fprintf(fp, "password = \"%s\"\n", nvram_safe_get(_passwd));
		char *next;
		char var[128];
		char *hn = nvram_safe_get(_hostname);
		fprintf(fp, "hostname = {");
		int idx = 0;
		foreach(var, hn, next) {
			if (idx)
				fprintf(fp, ", ");
			fprintf(fp, "\"%s\"", var);
			idx++;
		}
		fprintf(fp, "}\n");
#ifdef HAVE_USE_OPENSSL
		fprintf(fp, "ssl = %s\n", nvram_match(_ssl, "1") ? "true" : "false");
#endif
		if (nvram_match(_wildcard, "1"))
			fprintf(fp, "wildcard = true\n");
		if (flag == 5) {
			fprintf(fp, "ddns-server = \"%s\"\n", nvram_safe_get("ddns_custom_5"));
			fprintf(fp, "ddns-path = \"%s\"\n", nvram_safe_get("ddns_path_5"));
		}
		if (nvram_match("ddns_wan_ip", "1")) {
			fprintf(fp, "checkip-command = \"/sbin/service checkwanip main\"\n");
		}
		fprintf(fp, "}\n");
	}
	return 0;
}

void start_ddns(void)
{
	int ret;
	FILE *fp;

	int flag = nvram_geti("ddns_enable");
	if (flag > 32 || flag < 1)
		return -1;

	mkdir("/tmp/ddns", 0744);

	/*
	 * Generate ddns configuration file 
	 */
	if ((fp = fopen("/tmp/ddns/inadyn.conf", "w"))) {
		if (nvram_matchi("ddns_enable", 7))
			fprintf(fp, "period = %s\n", "900");
		else
			fprintf(fp, "period = %s\n", "600");
		fprintf(fp, "forced-update = %d\n", nvram_geti("ddns_force") * 24 * 60 * 60);
		if (init_ddns(fp) < 0)
			return;
		fprintf(fp, "\n");
		fclose(fp);
	} else {
		perror("/tmp/ddns/inadyn.conf");
		return;
	}

	if (strcmp(nvram_safe_get("ddns_enable_buf"), nvram_safe_get("ddns_enable")) ||
	    strcmp(nvram_safe_get("ddns_username_buf"), nvram_safe_get(_username)) ||
	    strcmp(nvram_safe_get("ddns_passwd_buf"), nvram_safe_get(_passwd)) ||
	    strcmp(nvram_safe_get("ddns_hostname_buf"), nvram_safe_get(_hostname)) || strcmp(nvram_safe_get("ddns_wildcard_buf"), nvram_safe_get(_wildcard)) ||
#ifdef HAVE_USE_OPENSSL
	    strcmp(nvram_safe_get("ddns_ssl_buf"), nvram_safe_get(_ssl)) ||
#endif
	    strcmp(nvram_safe_get("ddns_path_buf"), nvram_safe_get("ddns_path_5")) || strcmp(nvram_safe_get("ddns_custom_buf"), nvram_safe_get("ddns_custom_5"))) {
		/*
		 * If the user changed anything in the GUI, delete all cache and log 
		 */
		nvram_unset("ddns_cache");
		unlink("/tmp/ddns/ddns.log");
		unlink(_cache_file);
	}

	/*
	 * Restore cache data to file from NV 
	 */
	if (nvram_invmatch("ddns_cache", "")) {
		nvram2file("ddns_cache", _cache_file);
	}
	dd_logstart("ddns", eval("inadyn", "--cache-dir=/tmp/ddns", "-e", "ddns_success", "--exec-mode=compat", "-f", "/tmp/ddns/inadyn.conf", "-P", "/var/run/inadyn.pid", "-l", "notice"));

	cprintf("done\n");

	return;
}

void stop_ddns(void)
{
	int ret;
	stop_process("inadyn", "dynamic dns daemon");
	if (init_ddns(NULL) == 0) {
		unlink(_cache_file);
	}
	if (nvram_matchi("ddns_enable", 0)) {
		unlink("/tmp/ddns/inadyn.conf");
	}
	unlink("/tmp/ddns/ddns.log");

	cprintf("done\n");

	return;
}

int checkwanip_main(int argc, char *argv[])
{
	char new_ip_str[32];
	int wan_link = check_wan_link(0);
	char *wan_ipaddr = NULL;
	if (nvram_match("wan_proto", "pptp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("pptp_get_ip") : nvram_safe_get("wan_ipaddr");
	} else if (!strcmp(nvram_safe_get("wan_proto"), "pppoe")) {
		wan_ipaddr = wan_link ? nvram_safe_get("wan_ipaddr") : "0.0.0.0";
	} else if (!strcmp(nvram_safe_get("wan_proto"), "3g")) {
		wan_ipaddr = wan_link ? nvram_safe_get("wan_ipaddr") : "0.0.0.0";
	} else if (nvram_match("wan_proto", "l2tp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("l2tp_get_ip") : nvram_safe_get("wan_ipaddr");
	} else if (nvram_match("wan_proto", "disabled")) {
		wan_ipaddr = "0.0.0.0";
	} else {
		wan_ipaddr = nvram_safe_get("wan_ipaddr");
	}
	if (!strcmp(wan_ipaddr, "0.0.0.0")) {
		return -1;
	}
	fprintf(stdout, "%s\n", wan_ipaddr);
	return 0;
}

int ddns_success_main(int argc, char *argv[])
{
	char buf[80];
	char buf2[80];
	FILE *fp;

	if (init_ddns(NULL) == 0) {
		if ((fp = fopen(_cache_file, "r"))) {
			fgets(buf, sizeof(buf), fp);
			fclose(fp);
			nvram_set("ddns_cache", buf);
		}
	}

	nvram_set("ddns_enable_buf", nvram_safe_get("ddns_enable"));
	nvram_set("ddns_username_buf", nvram_safe_get(_username));
	nvram_set("ddns_passwd_buf", nvram_safe_get(_passwd));
	nvram_set("ddns_hostname_buf", nvram_safe_get(_hostname));
#ifdef HAVE_USE_OPENSSL
	nvram_set("ddns_ssl_buf", nvram_safe_get(_ssl));
#endif
	nvram_set("ddns_wildcard_buf", nvram_safe_get(_wildcard));
	nvram_set("ddns_custom_buf", nvram_safe_get("ddns_custom_5"));
	nvram_set("ddns_path_buf", nvram_safe_get("ddns_path_5"));

	nvram_async_commit();

	cprintf("done\n");

	return 0;
}
