/*
 * wifidog.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_WIFIDOG
#include <stdlib.h>
#include <sys/stat.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>
// unfinished. do not use
void start_wifidog(void)
{
	if (nvram_match("wd_enable", "1")) {
		insmod("ipt_mark");
		insmod("ipt_mac");
		insmod("xt_mark");
		insmod("xt_mac");
		mkdir("/tmp/wifidog/", 0744);
		FILE *fp = fopen("/tmp/wifidog/wifidog.conf", "wb");

		if (!strlen(nvram_safe_get("wd_gwid")))
			fprintf(fp, "GatewayID %s\n", nvram_safe_get("router_name"));
		else
			fprintf(fp, "GatewayID %s\n", nvram_safe_get("wd_gwid"));
		if (nvram_invmatch("wan_proto", "disabled"))	//WIP
			fprintf(fp, "ExternalInterface %s\n", get_wan_face());
		else
			fprintf(fp, "ExternalInterface %s\n", nvram_safe_get("wd_extiface"));
		fprintf(fp, "GatewayInterface %s\n", nvram_safe_get("wd_iface"));
		// fprintf (fp, "Portal %s\n", nvram_safe_get ("wd_url"));
		fprintf(fp, "GatewayPort %s\n", nvram_safe_get("wd_gwport"));
		fprintf(fp, "HTTPDMaxConn %s\n", nvram_safe_get("wd_httpdcon"));
		fprintf(fp, "HTTPDName %s\n", nvram_safe_get("wd_httpdname"));
		fprintf(fp, "CheckInterval %s\n", nvram_safe_get("wd_interval"));
		fprintf(fp, "ClientTimeout %s\n", nvram_safe_get("wd_timeout"));
		fprintf(fp, "TrustedMACList %s\n", nvram_safe_get("wd_maclist"));
		fprintf(fp, "AuthServer {\n");
		fprintf(fp, "Hostname %s\n", nvram_safe_get("wd_hostname"));
		fprintf(fp, "SSLAvailable %s\n", nvram_match("wd_sslavailable", "1") ? "yes" : "no");
		fprintf(fp, "SSLPort %s\n", nvram_safe_get("wd_sslport"));
		fprintf(fp, "HTTPPort %s\n", nvram_safe_get("wd_httpport"));
		if (strlen(nvram_safe_get("wd_messagefile")) > 0) {
			fprintf(fp, "HtmlMessageFile %s\n", nvram_safe_get("wd_messagefile"));
		}
		if (nvram_match("wd_auth", "1")) {
			if (strlen(nvram_safe_get("wd_realm")) > 0)
				fprintf(fp, "HTTPDRealm %s\n", nvram_safe_get("wd_realm"));
			fprintf(fp, "HTTPDUserName %s\n", nvram_safe_get("wd_username"));
			fprintf(fp, "HTTPDPassword %s\n", nvram_safe_get("wd_password"));
		}
		fprintf(fp, "Path %s\n", nvram_safe_get("wd_path"));
		fprintf(fp, "}\n");
		if (strlen(nvram_safe_get("wd_config")) > 0) {
			fwritenvram("wd_config", fp);
		} else {
			fprintf(fp, "FirewallRuleSet validating-users {\n");
			fprintf(fp, "FirewallRule allow to 0.0.0.0/0\n");
			fprintf(fp, "}\n");
			fprintf(fp, "FirewallRuleSet known-users {\n");
			fprintf(fp, "FirewallRule allow to 0.0.0.0/0\n");
			fprintf(fp, "}\n");
			fprintf(fp, "FirewallRuleSet unknown-users {\n");
			fprintf(fp, "FirewallRule allow udp port 53\n");
			fprintf(fp, "FirewallRule allow tcp port 53\n");
			fprintf(fp, "FirewallRule allow udp port 67\n");
			fprintf(fp, "FirewallRule allow tcp port 67\n");
			fprintf(fp, "}\n");
			fprintf(fp, "FirewallRuleSet locked-users {\n");
			fprintf(fp, "FirewallRule block to 0.0.0.0/0\n");
			fprintf(fp, "}\n");
		}
		fclose(fp);
		eval("wifidog");
		dd_syslog(LOG_INFO, "wifidog successfully started\n");
		eval("iptables", "-D", "FORWARD", "-i", nvram_safe_get("wd_iface"), "-d", nvram_safe_get("wd_hostname"), "-j", "ACCEPT");
		eval("iptables", "-I", "FORWARD", "-i", nvram_safe_get("wd_iface"), "-d", nvram_safe_get("wd_hostname"), "-j", "ACCEPT");
	}
#ifdef HAVE_TIEXTRA2
	start_mwifidog();
#endif
}

void stop_wifidog(void)
{
	stop_process("wifidog", "WiFiDOG");
}

#endif
