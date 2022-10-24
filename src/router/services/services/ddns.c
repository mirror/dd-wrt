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
char _dyndnstype[] = "ddns_dyndnstype_XX";
char _wildcard[] = "ddns_wildcard_XX";
char _url[] = "ddns_url_XX";
char _conf[] = "ddns_conf_XX";

int init_ddns(FILE * fp)
{

	int flag = nvram_geti("ddns_enable");

	switch (flag) {
	case 0:		// ddns disabled
		return -1;
		break;

	case 1:
		if (fp) {
			if (nvram_matchi("ddns_dyndnstype", 2))
				fprintf(fp, "provider statdns@dyndns.org {\n");
			else if (nvram_matchi("ddns_dyndnstype", 3))
				fprintf(fp, "provider custom@dyndns.org {\n");
			else
				fprintf(fp, "provider dyndns@dyndns.org {\n");

			fprintf(fp, "username = %s\n", nvram_safe_get("ddns_username"));
			fprintf(fp, "password = %s\n", nvram_safe_get("ddns_passwd"));
			fprintf(fp, "hostname = %s\n", nvram_safe_get("ddns_hostname"));
			fprintf(fp, "}\n");
		}
		snprintf(_username, sizeof(_username), "%s", "ddns_username");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname");
//              snprintf(_dyndnstype, sizeof(_dyndnstype), "%s", "ddns_dyndnstype");
//              snprintf(_wildcard, sizeof(_wildcard), "%s", "ddns_wildcard");
		break;

	case 2:
		if (fp) {
			fprintf(fp, "provider default@freedns.afraid.org {\n");
			fprintf(fp, "username = %s\n", nvram_safe_get("ddns_username_2"));
			fprintf(fp, "password = %s\n", nvram_safe_get("ddns_passwd_2"));
			fprintf(fp, "hostname = %s\n", nvram_safe_get("ddns_hostname_2"));
			fprintf(fp, "}\n");
		}
		snprintf(_username, sizeof(_username), "%s", "ddns_username_2");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_2");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname_2");

		break;

	case 3:

		if (fp) {
			fprintf(fp, "provider default@zoneedit.com	 {\n");
			fprintf(fp, "username = %s\n", nvram_safe_get("ddns_username_3"));
			fprintf(fp, "password = %s\n", nvram_safe_get("ddns_passwd_3"));
			fprintf(fp, "hostname = %s\n", nvram_safe_get("ddns_hostname_3"));
			fprintf(fp, "}\n");
		}
		snprintf(_username, sizeof(_username), "%s", "ddns_username_3");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_3");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname_3");
		break;

	case 4:

		if (fp) {
			fprintf(fp, "provider default@no-ip.com	 {\n");
			fprintf(fp, "username = %s\n", nvram_safe_get("ddns_username_4"));
			fprintf(fp, "password = %s\n", nvram_safe_get("ddns_passwd_4"));
			fprintf(fp, "hostname = %s\n", nvram_safe_get("ddns_hostname_4"));
			fprintf(fp, "}\n");
		}
		snprintf(_username, sizeof(_username), "%s", "ddns_username_4");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_4");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname_4");
		break;

	case 5:

		if (fp) {
			fprintf(fp, "custom namecheap {\n");
			fprintf(fp, "username = %s\n", nvram_safe_get("ddns_username_5"));
			fprintf(fp, "password = %s\n", nvram_safe_get("ddns_passwd_5"));
			fprintf(fp, "hostname = %s\n", nvram_safe_get("ddns_hostname_5"));
			fprintf(fp, "ddns-server = %s\n", nvram_safe_get("ddns_url"));
			fprintf(fp, "ddns-path = %s\n", nvram_safe_get("ddns_conf"));
			fprintf(fp, "}\n");
		}
		snprintf(_username, sizeof(_username), "%s", "ddns_username_5");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_5");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname_5");

		break;

	case 6:

		if (fp) {
			fprintf(fp, "provider dyndns@3322.org {\n");
			fprintf(fp, "username = %s\n", nvram_safe_get("ddns_username_6"));
			fprintf(fp, "password = %s\n", nvram_safe_get("ddns_passwd_6"));
			fprintf(fp, "hostname = %s\n", nvram_safe_get("ddns_hostname_6"));
			fprintf(fp, "}\n");
		}
		snprintf(_username, sizeof(_username), "%s", "ddns_username_6");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_6");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname_6");

		//???
//              snprintf(_dyndnstype, sizeof(_dyndnstype), "%s", "ddns_dyndnstype_6");
//              snprintf(_wildcard, sizeof(_wildcard), "%s", "ddns_wildcard_6");
		break;

	case 7:

		if (fp) {
			fprintf(fp, "provider default@easydns.com {\n");
			fprintf(fp, "username = %s\n", nvram_safe_get("ddns_username_7"));
			fprintf(fp, "password = %s\n", nvram_safe_get("ddns_passwd_7"));
			fprintf(fp, "hostname = %s\n", nvram_safe_get("ddns_hostname_7"));
			fprintf(fp, "}\n");
		}
		snprintf(_username, sizeof(_username), "%s", "ddns_username_7");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_7");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname_7");

//      ???
//              snprintf(_wildcard, sizeof(_wildcard), "%s", "ddns_wildcard_7");
		break;

/*	case 8:
		strcpy(service, "default@tzo.com");

		snprintf(_username, sizeof(_username), "%s", "ddns_username_8");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_8");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname_8");
		break;

	case 9:
		strcpy(service, "default@dynsip.org");

		snprintf(_username, sizeof(_username), "%s", "ddns_username_9");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_9");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname_9");
		break;

	case 10:
		strcpy(service, "default@dtdns.com");

		snprintf(_username, sizeof(_username), "%s", "ddns_username_10");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_10");
		snprintf(_hostname, sizeof(_username), "%s", "ddns_username_10");
		break;
*/
	case 11:

		if (fp) {
			fprintf(fp, "provider default@duiadns.net {\n");
			fprintf(fp, "username = %s\n", nvram_safe_get("ddns_username_11"));
			fprintf(fp, "password = %s\n", nvram_safe_get("ddns_passwd_11"));
			fprintf(fp, "hostname = %s\n", nvram_safe_get("ddns_hostname_11"));
			fprintf(fp, "}\n");
		}
		snprintf(_username, sizeof(_username), "%s", "ddns_username_11");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd_11");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname_11");

		break;

	default:
		return -1;
	}

	return 0;
}

void start_ddns(void)
{
	int ret;
	FILE *fp;

	/*
	 * We don't want to update, if user don't input below field 
	 */
	if (nvram_match(_username, "") || nvram_match(_passwd, "") || nvram_match(_hostname, ""))
		return;

	mkdir("/tmp/ddns", 0744);

	if (strcmp(nvram_safe_get("ddns_enable_buf"), nvram_safe_get("ddns_enable")) ||	// ddns 
	    // mode 
	    // change
	    strcmp(nvram_safe_get("ddns_username_buf"), nvram_safe_get(_username)) ||	// ddns 
	    // username 
	    // chane
	    strcmp(nvram_safe_get("ddns_passwd_buf"), nvram_safe_get(_passwd)) ||	// ddns 
	    // password 
	    // change
	    strcmp(nvram_safe_get("ddns_hostname_buf"), nvram_safe_get(_hostname)) ||	// ddns 
	    // hostname 
	    // change
	    strcmp(nvram_safe_get("ddns_dyndnstype_buf"), nvram_safe_get(_dyndnstype)) ||	// ddns 
	    // dyndnstype 
	    // change
	    strcmp(nvram_safe_get("ddns_wildcard_buf"), nvram_safe_get(_wildcard)) ||	// ddns 
	    // wildcard 
	    // change
	    strcmp(nvram_safe_get("ddns_url_buf"), nvram_safe_get(_url)) ||	// ddns 
	    // url 
	    // change
	    strcmp(nvram_safe_get("ddns_conf_buf"), nvram_safe_get(_conf)) ||	// ddns 
	    // conf 
	    // change
	    strcmp(nvram_safe_get("ddns_custom_5_buf"), nvram_safe_get("ddns_custom_5"))) {
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
			fprintf(fp, "period %s\n", "900");	// check ip
		else
			fprintf(fp, "period %s\n", "600");	// check ip
		fprintf(fp, "forced-update %d\n", nvram_geti("ddns_force") * 24 * 60 * 60);	// force 
		fprintf(fp, "cache-dir %s\n", "/tmp/ddns");	// cache dir
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
			fprintf(fp, " --update_period_sec %s", "900");	// check ip
		else
			fprintf(fp, " --update_period_sec %s", "600");	// check ip
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
	dd_logstart("ddns", eval("inadyn", "-e", "ddns_success", "--exec-mode=compat", "-f", "/tmp/ddns/inadyn.conf"));

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
	nvram_set("ddns_dyndnstype_buf", nvram_safe_get(_dyndnstype));
	nvram_set("ddns_wildcard_buf", nvram_safe_get(_wildcard));
	nvram_set("ddns_conf_buf", nvram_safe_get(_conf));
	nvram_set("ddns_url_buf", nvram_safe_get(_url));
	nvram_set("ddns_custom_5_buf", nvram_safe_get("ddns_custom_5"));

	nvram_async_commit();

	cprintf("done\n");

	return 0;
}
