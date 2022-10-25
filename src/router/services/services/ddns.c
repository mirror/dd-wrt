/*
 * ddns.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
char _url[] = "ddns_url_XX";
char _conf[] = "ddns_conf_XX";

int init_ddns(FILE * fp)
{

	int flag = nvram_geti("ddns_enable");
	if (flag > 31 || flag < 1)
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
		"default@cloudxns.com",
		"default@ddnss.de",
		"default@dhis.org",
		"default@dnsexit.com",
		"default@dnspod.cn",
		"default@duckdns.org",
		"default@freemyip.com",
		"default@gira.de",
		"default@sitelutions.com",
		"default@pdd.yandex.ru",
	};
	char *provider = providers[flag];
	snprintf(_ssl, sizeof(_hostname), "%s", "ddns_ssl");
	if (flag == 1) {
		snprintf(_username, sizeof(_username), "%s", "ddns_username");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname");
		snprintf(_wildcard, sizeof(_hostname), "%s", "ddns_wildcard");
	} else {
		snprintf(_username, sizeof(_username), "%s_%d", "ddns_username", flag);
		snprintf(_passwd, sizeof(_passwd), "%s_%d", "ddns_passwd", flag);
		snprintf(_hostname, sizeof(_hostname), "%s_%d", "ddns_hostname", flag);
		snprintf(_wildcard, sizeof(_hostname), "%s_%d", "ddns_wildcard", flag);
	}
	if (fp) {
		if (flag == 5)
			fprintf(fp, "custom namecheap {\n");
		else
			fprintf(fp, "provider %s {\n", provider);
		fprintf(fp, "username = %s\n", nvram_safe_get(_username));
		fprintf(fp, "password = %s\n", nvram_safe_get(_passwd));
		fprintf(fp, "hostname = %s\n", nvram_safe_get(_hostname));
#ifdef HAVE_OPENSSL
		fprintf(fp, "ssl = %s\n", nvram_match(_ssl, "1") ? "true" : "false");
#endif
		if (nvram_match(_wildcard, "1"))
			fprintf(fp, "wildcard = true\n");
		if (flag == 5) {
			fprintf(fp, "ddns-server = %s\n", nvram_safe_get("ddns_url"));
			fprintf(fp, "ddns-path = %s\n", nvram_safe_get("ddns_conf"));
		}
		if (nvram_match("ddns_wan_ip", "1")) {
			fprintf(fp, "checkip-command = /sbin/service checkwanip main\n");
		}
		fprintf(fp, "}\n");
	}
	return 0;
}

void start_ddns(void)
{
	int ret;
	FILE *fp;

	nvram_set("ddns_status", "0");

	/*
	 * We don't want to update, if user don't input below field 
	 */
	if (nvram_match(_username, "") || nvram_match(_passwd, "") || nvram_match(_hostname, ""))
		return;

	mkdir("/tmp/ddns", 0744);

	if (strcmp(nvram_safe_get("ddns_enable_buf"), nvram_safe_get("ddns_enable")) ||
	    strcmp(nvram_safe_get("ddns_username_buf"), nvram_safe_get(_username)) ||
	    strcmp(nvram_safe_get("ddns_passwd_buf"), nvram_safe_get(_passwd)) ||
	    strcmp(nvram_safe_get("ddns_hostname_buf"), nvram_safe_get(_hostname)) ||
	    strcmp(nvram_safe_get("ddns_wildcard_buf"), nvram_safe_get(_wildcard)) || strcmp(nvram_safe_get("ddns_url_buf"), nvram_safe_get(_url)) || strcmp(nvram_safe_get("ddns_conf_buf"), nvram_safe_get(_conf)) ||
#ifdef HAVE_OPENSSL
	    strcmp(nvram_safe_get("ddns_ssl_buf"), nvram_safe_get(_ssl)) ||
#endif
	    strcmp(nvram_safe_get("ddns_custom_buf"), nvram_safe_get("ddns_custom_5"))) {
		/*
		 * If the user changed anything in the GUI, delete all cache and log 
		 */
		nvram_unset("ddns_cache");
		nvram_unset("ddns_time");
		unlink("/tmp/ddns/ddns.log");
		unlink("/tmp/ddns/inadyn_ip.cache");
		unlink("/tmp/ddns/inadyn_time.cache");
	}

	/*
	 * Generate ddns configuration file 
	 */
	if ((fp = fopen("/tmp/ddns/inadyn.conf", "w"))) {
		if (nvram_matchi("ddns_enable", 7))
			fprintf(fp, "period %s\n", "900");
		else
			fprintf(fp, "period %s\n", "600");
		fprintf(fp, "forced-update %d\n", nvram_geti("ddns_force") * 24 * 60 * 60);
		fprintf(fp, "cache-dir %s\n", "/tmp/ddns");
		if (init_ddns(fp) < 0)
			return;

/*	
		fprintf(fp, "--background");
		fprintf(fp, " --dyndns_system %s", service);	// service
		fprintf(fp, " -a %s", nvram_safe_get(_hostname));	// alias/hostname
		if (nvram_matchi("ddns_enable", 1)
		    || nvram_matchi("ddns_enable", 6)
		    || nvram_matchi("ddns_enable", 7)) {
			if (nvram_matchi(_wildcard, 1))
				fprintf(fp, " --wildcard");
		}
		if (nvram_matchi("ddns_enable", 7))
			fprintf(fp, " --update_period_sec %s", "900");	
		else
			fprintf(fp, " --update_period_sec %s", "600");	
		fprintf(fp, " --forced_update_period %d", nvram_geti("ddns_force") * 24 * 60 * 60);	// force 
		fprintf(fp, " --log_file %s", "/tmp/ddns/ddns.log");	// log to
		fprintf(fp, " --cache_dir %s", "/tmp/ddns");	// cache dir
		fprintf(fp, " --exec %s", "ddns_success");	// run after update
		if (nvram_matchi("ddns_enable", 5)) {
			fprintf(fp, " --dyndns_server_name %s", nvram_safe_get("ddns_custom_5"));
			if (nvram_invmatch(_url, ""))
				fprintf(fp, " --dyndns_server_url %s", nvram_safe_get(_url));
			if (nvram_invmatch(_conf, ""))
				fprintf(fp, " %s", nvram_safe_get(_conf));
		}
*/
		fprintf(fp, "\n");
		fclose(fp);
	} else {
		perror("/tmp/ddns/inadyn.conf");
		return;
	}

	/*
	 * Restore cache data to file from NV 
	 */
	if (nvram_invmatch("ddns_cache", "")
	    && nvram_invmatch("ddns_time", "")) {
		nvram2file("ddns_cache", "/tmp/ddns/inadyn_ip.cache");
		nvram2file("ddns_time", "/tmp/ddns/inadyn_time.cache");
	}
	dd_logstart("ddns", eval("inadyn", "-e", "ddns_success", "--exec-mode=compat", "-f", "/tmp/ddns/inadyn.conf", "-P", "/var/run/inadyn.pid"));

	cprintf("done\n");

	return;
}

void stop_ddns(void)
{
	int ret;
	stop_process("inadyn", "dynamic dns daemon");
	unlink("/tmp/ddns/ddns.log");
	unlink("/tmp/ddns/inadyn_ip.cache");
	unlink("/tmp/ddns/inadyn_time.cache");

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
}

int ddns_success_main(int argc, char *argv[])
{
	char buf[80];
	char buf2[80];
	FILE *fp;

	init_ddns(NULL);

	if ((fp = fopen("/tmp/ddns/inadyn_ip.cache", "r"))) {
		fgets(buf, sizeof(buf), fp);
		fclose(fp);
		nvram_set("ddns_cache", buf);
	}

	if ((fp = fopen("/tmp/ddns/inadyn_time.cache", "r"))) {
		fgets(buf2, sizeof(buf2), fp);
		fclose(fp);
		nvram_set("ddns_time", buf2);
	}

	nvram_set("ddns_enable_buf", nvram_safe_get("ddns_enable"));
	nvram_set("ddns_username_buf", nvram_safe_get(_username));
	nvram_set("ddns_passwd_buf", nvram_safe_get(_passwd));
	nvram_set("ddns_hostname_buf", nvram_safe_get(_hostname));
#ifdef HAVE_OPENSSL
	nvram_set("ddns_ssl_buf", nvram_safe_get(_ssl));
#endif
	nvram_set("ddns_wildcard_buf", nvram_safe_get(_wildcard));
	nvram_set("ddns_conf_buf", nvram_safe_get(_conf));
	nvram_set("ddns_url_buf", nvram_safe_get(_url));
	nvram_set("ddns_custom_5_buf", nvram_safe_get("ddns_custom_5"));
	nvram_set("ddns_status", "1");

	nvram_async_commit();

	cprintf("done\n");

	return 0;
}
