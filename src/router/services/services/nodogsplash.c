/*
 * nocat.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_NODOG
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <services.h>
#define NODOG_CONF "nodogsplash.conf"
#define NODOG_CONF_PATH "/tmp/" NODOG_CONF
extern void addHost(char *host, char *ip, int withdomain);

int mk_nodog_conf(void)
{
	FILE *fp;
	int i;

	if (!(fp = fopen(NODOG_CONF_PATH, "w"))) {
		perror(NODOG_CONF_PATH);
		return errno;
	}

	fprintf(fp, "#\n");
	fprintf(fp, "GatewayInterface\t%s\n", nvram_default_get("ND_ifname", nvram_safe_get("lan_ifname")));
	fprintf(fp, "FirewallRuleSet authenticated-users {\n");
	fprintf(fp, "\tFirewallRule allow all\n");
	fprintf(fp, "}\n");
	fprintf(fp, "FirewallRuleSet preauthenticated-users {\n");
	fprintf(fp, "\tFirewallRule allow udp port 53\n");
	fprintf(fp, "\tFirewallRule allow tcp port 53\n");
	char var[64];
	char *next;
	char *list = nvram_safe_get("ND_ExcludePorts");
	foreach(var, list, next)
	{
		fprintf(fp, "\tFirewallRule allow tcp port %s\n", var);
		fprintf(fp, "\tFirewallRule allow udp port %s\n", var);
	}
	fprintf(fp, "}\n");
	fprintf(fp, "FirewallRuleSet users-to-router {\n");
	fprintf(fp, "\tFirewallRule allow udp port 53\n");
	fprintf(fp, "\tFirewallRule allow tcp port 53\n");
	fprintf(fp, "\tFirewallRule allow udp port 67\n");
	fprintf(fp, "\tFirewallRule allow tcp port 22\n");
	fprintf(fp, "\tFirewallRule allow tcp port 80\n");
	fprintf(fp, "\tFirewallRule allow tcp port 443\n");
	fprintf(fp, "}\n");
	fprintf(fp, "GatewayName\t%s\n", nvram_safe_get("ND_GatewayName"));
	nvram_default_get("ND_GatewayAddr", "0.0.0.0");
	if (!nvram_match("ND_GatewayAddr", "0.0.0.0"))
		fprintf(fp, "GatewayAddress\t%s\n", nvram_safe_get("ND_GatewayAddr"));
	if (!nvram_match("ND_GatewayIPRange", "0.0.0.0") && !nvram_match("ND_GatewayIPRange", "") &&
	    !nvram_match("ND_GatewayIPRange_mask", "")) {
		char range[64];
		snprintf(range, sizeof(range), "%s/%s", nvram_safe_get("ND_GatewayIPRange"),
			 nvram_safe_get("ND_GatewayIPRange_mask"));
		fprintf(fp, "GatewayIPRange\t%s\n", range);
	}
	fprintf(fp, "WebRoot\t%s\n", nvram_safe_get("ND_DocumentRoot"));
	fprintf(fp, "StatusPage\t%s\n", nvram_default_get("ND_StatusPage", "status.html"));
	fprintf(fp, "SplashPage\t%s\n", nvram_default_get("ND_SplashPage", "splash.html"));
	if (*nvram_safe_get("ND_HomePage") && nvram_match("ND_ForcedRedirect", "1"))
		fprintf(fp, "RedirectURL\t%s\n", nvram_safe_get("ND_HomePage"));

	fprintf(fp, "GatewayPort\t%s\n", nvram_default_get("ND_GatewayPort", "2050"));
	fprintf(fp, "MaxClients\t%s\n", nvram_default_get("ND_MaxClients", "250"));
	fprintf(fp, "PreAuthIdleTimeout\t%s\n", nvram_default_get("ND_PreAuthIdleTimeout", "30"));
	fprintf(fp, "AuthIdleTimeout\t%s\n", nvram_default_get("ND_LoginTimeout", "120"));
	//      fprintf(fp, "SessionTimeout\t%s\n", nvram_default_get("ND_LoginTimeout", "120"));
	fprintf(fp, "CheckInterval\t%s\n", nvram_default_get("ND_CheckInterval", "600"));
	fprintf(fp, "TrafficControl yes\n");
	fprintf(fp, "DownloadLimit\t%s\n", nvram_default_get("ND_dl", "0"));
	fprintf(fp, "UploadLimit\t%s\n", nvram_default_get("ND_ul", "0"));
	if (*nvram_safe_get("ND_MACWhiteList"))
		fprintf(fp, "AllowedMACList\t%s\n", nvram_safe_get("ND_MACWhiteList"));

	fclose(fp);
	return 0;
}

void start_splashd(void)
{
	char path[128];
	FILE *fp;
	if (!nvram_matchi("ND_enable", 1))
		return;

	insmod("ifb ipt_mark ipt_mac xt_mark xt_mac");
	mk_nodog_conf();
	eval("nodogsplash", "-c", getdefaultconfig("nodogsplash", path, sizeof(path), NODOG_CONF));
	dd_loginfo("nodogsplash", "nocatsplash daemon successfully started");
	return;
}

void stop_splashd(void)
{
	stop_process("nodogsplash", "nodogsplash daemon");
	return;
}

#endif
