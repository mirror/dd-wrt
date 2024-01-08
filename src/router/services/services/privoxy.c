/*
 * privoxy.c
 *
 * Copyright (C) 2013 Richard Schneidt
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

#ifdef HAVE_PRIVOXY
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

void start_privoxy(void)
{
	char wan_if_buffer[33];
	if (!nvram_matchi("privoxy_enable", 1))
		return;

	int mode = 0;
	int whitelist = 0;
	char *next;
	char var[80];
	char vifs[256];
	char *ip = nvram_safe_get("lan_ipaddr");
	char *mask = nvram_safe_get("lan_netmask");
	char *webif_port = nvram_safe_get("http_lanport");

	sysprintf("grep -q nobody /etc/passwd || echo \"nobody:*:65534:65534:nobody:/var:/bin/false\" >> /etc/passwd");
	mkdir("/var/log/privoxy", 0777);

	char *wan = get_wan_ipaddr();
	if (nvram_matchi("privoxy_transp_enable", 1)) {
		sysprintf("iptables -t nat -D PREROUTING -p tcp ! -d %s --dport 80 -j DNAT --to %s:8118", wan, ip);
		sysprintf("iptables -t nat -I PREROUTING -p tcp ! -d %s --dport 80 -j DNAT --to %s:8118", wan, ip);
		//		sysprintf("iptables -t nat -D PREROUTING -p tcp ! -d %s --dport 443 -j DNAT --to %s:8118", wan, ip);
		//		sysprintf("iptables -t nat -I PREROUTING -p tcp ! -d %s --dport 443 -j DNAT --to %s:8118", wan, ip);
		sysprintf("iptables -t nat -D PREROUTING -p tcp -s %s/%s -d %s --dport %s -j ACCEPT", ip, mask, ip, webif_port);
		sysprintf("iptables -t nat -I PREROUTING -p tcp -s %s/%s -d %s --dport %s -j ACCEPT", ip, mask, ip, webif_port);
		sysprintf("iptables -t nat -D PREROUTING -p tcp -s %s -d %s --dport %s -j DROP", ip, ip, webif_port);
		sysprintf("iptables -t nat -I PREROUTING -p tcp -s %s -d %s --dport %s -j DROP", ip, ip, webif_port);
		char *transp = nvram_safe_get("privoxy_transp_exclude");
		/* no gui setting yet - redirect all except this IP */
		if (*transp) {
			sysprintf("iptables -t nat -D PREROUTING -p tcp -s %s --dport 80 -j ACCEPT", transp);
			sysprintf("iptables -t nat -I PREROUTING -p tcp -s %s --dport 80 -j ACCEPT", transp);
			//			sysprintf("iptables -t nat -D PREROUTING -p tcp -s %s --dport 443 -j ACCEPT", transp);
			//			sysprintf("iptables -t nat -I PREROUTING -p tcp -s %s --dport 443 -j ACCEPT", transp);
		}
		mode = 1;
		getIfLists(vifs, 256);
		char vif_ip[32];
		foreach(var, vifs, next)
		{
			if (strcmp(safe_get_wan_face(wan_if_buffer), var) && strcmp(nvram_safe_get("lan_ifname"), var)) {
				if (nvram_nmatch("1", "%s_isolation", var)) {
					sysprintf("iptables -t nat -D PREROUTING -i %s -d %s/%s -j RETURN", var, ip, mask);
					sysprintf("iptables -t nat -I PREROUTING -i %s -d %s/%s -j RETURN", var, ip, mask);
					sprintf(vif_ip, "%s_ipaddr", var);
					sysprintf("iptables -t nat -D PREROUTING -i %s -d %s -j RETURN", var,
						  nvram_safe_get(vif_ip));
					sysprintf("iptables -t nat -I PREROUTING -i %s -d %s -j RETURN", var,
						  nvram_safe_get(vif_ip));
				}
			}
		}
	}

	if (nvram_invmatch("privoxy_whitel", "")) {
		eval("cp", "/etc/privoxy/user.action", "/tmp/user.action");
		FILE *fpuser = fopen("/tmp/user.action", "ab");
		fprintf(fpuser, "%s", "{ -block }\n");
		fprintf(fpuser, "%s", nvram_safe_get("privoxy_whitel"));
		fclose(fpuser);
		whitelist = 1;
	}

	FILE *fp = fopen("/tmp/privoxy.conf", "wb");

	if (nvram_matchi("privoxy_advanced", 1) && nvram_invmatch("privoxy_conf", "")) {
		fprintf(fp, "%s", nvram_safe_get("privoxy_conf"));
	} else {
		fprintf(fp,
			"confdir /etc/privoxy\n"
			"logdir /var/log/privoxy\n"
			"actionsfile match-all.action\n"
			"actionsfile default.action\n"
			"actionsfile %s\n"
			"filterfile default.filter\n"
			"filterfile user.filter\n"
			"logfile logfile\n"
			"listen-address  %s:8118\n"
			"toggle  1\n"
			"enable-remote-toggle  0\n"
			"enable-remote-http-toggle  0\n"
			"enable-edit-actions 0\n"
			"tolerate-pipelining 1\n"
			"buffer-limit 4096\n"
			"accept-intercepted-requests %d\n"
			"split-large-forms 0\n"
			"socket-timeout 60\n"
			"max-client-connections %d\n"
			"handle-as-empty-doc-returns-ok 1\n",
			whitelist ? "/tmp/user.action" : "user.action", ip, mode, nvram_geti("privoxy_maxclient"));
	}
	fclose(fp);
	log_eval("privoxy", "/tmp/privoxy.conf");
	return;
}

void stop_privoxy(void)
{
	char wan_if_buffer[33];
	char *ip = nvram_safe_get("lan_ipaddr");
	char *wan = get_wan_ipaddr();
	char *mask = nvram_safe_get("lan_netmask");
	char *webif_port = nvram_safe_get("http_lanport");
	char *transp = nvram_safe_get("privoxy_transp_exclude");
	char *next;
	char var[80];
	char vifs[256];

	sysprintf("iptables -t nat -D PREROUTING -p tcp ! -d %s --dport 80 -j DNAT --to %s:8118", wan, ip);
	//	sysprintf("iptables -t nat -D PREROUTING -p tcp ! -d %s --dport 443 -j DNAT --to %s:8118", wan, ip);
	sysprintf("iptables -t nat -D PREROUTING -p tcp -s %s/%s -d %s --dport %s -j ACCEPT", ip, mask, ip, webif_port);
	sysprintf("iptables -t nat -D PREROUTING -p tcp -s %s -d %s --dport %s -j DROP", ip, ip, webif_port);
	if (*transp) {
		sysprintf("iptables -t nat -D PREROUTING -p tcp -s %s --dport 80 -j ACCEPT");
		//		sysprintf("iptables -t nat -D PREROUTING -p tcp -s %s --dport 443 -j ACCEPT");
	}
	getIfLists(vifs, 256);
	char vif_ip[32];
	foreach(var, vifs, next)
	{
		if (strcmp(safe_get_wan_face(wan_if_buffer), var) && strcmp(nvram_safe_get("lan_ifname"), var)) {
			if (nvram_nmatch("1", "%s_isolation", var)) {
				sysprintf("iptables -t nat -D PREROUTING -i %s -d %s/%s -j RETURN", var, ip, mask);
				sprintf(vif_ip, "%s_ipaddr", var);
				sysprintf("iptables -t nat -D PREROUTING -i %s -d %s -j RETURN", var, nvram_safe_get(vif_ip));
			}
		}
	}
	stop_process("privoxy", "daemon");
}
#endif
