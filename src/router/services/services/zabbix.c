/*
 * zabbix.c
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

#ifdef HAVE_ZABBIX
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
#include <ddnvram.h>
#include <shutils.h>
#include <services.h>

void start_zabbix(void)
{
	if (!nvram_matchi("zabbix_enable", 1))
		return;
	if (pidof("zabbix_agentd") > 0) {
		//syslog(LOG_INFO, "dlna : minidlna already running\n");
	} else {
		sysprintf("grep -q zabbix /etc/passwd || echo \"zabbix:*:65533:65533:zabbix:/var:/bin/false\" >> /etc/passwd");
		char *HOST;
		if (!*nvram_safe_get("wan_hostname"))
			HOST = nvram_safe_get("DD_BOARD");
		else
			HOST = nvram_safe_get("wan_hostname");
		char *SRVIP = nvram_safe_get("zabbix_serverip");
		char *IP = nvram_safe_get("lan_ipaddr");
		FILE *fp = fopen("/tmp/zabbix.conf", "wb");
		fprintf(fp, "LogType=file\n");

		fprintf(fp, "LogType=file\n");
		fprintf(fp, "LogFile=/var/log/zabbix.log\n");
		fprintf(fp, "LogFileSize=1\n");
		fprintf(fp, "Server=%s\n", SRVIP);
		fprintf(fp, "Hostname=%s\n", HOST);
		fprintf(fp, "ListenIP=%s\n", IP);
		fprintf(fp, "StartAgents=5\n");
		fprintf(fp, "AllowRoot=1\n");
#ifndef HAVE_MADWIFI
		fprintf(fp, "UserParameter=temperature.wl0, /usr/sbin/temps eth1 2> /dev/null\n");
		fprintf(fp, "UserParameter=temperature.wl1, /usr/sbin/temps eth2 2> /dev/null\n");
		fprintf(fp, "UserParameter=temperature.wl2, /usr/sbin/temps eth3 2> /dev/null\n");
		fprintf(fp, "UserParameter=clients.wl0, /usr/sbin/wclients eth1\n");
		fprintf(fp, "UserParameter=clients.wl1, /usr/sbin/wclients eth2\n");
		fprintf(fp, "UserParameter=clients.wl2, /usr/sbin/wclients eth3\n");
#else
		fprintf(fp, "UserParameter=temperature.wlan0, /usr/sbin/temps wlan0 2> /dev/null\n");
		fprintf(fp, "UserParameter=temperature.wlan1, /usr/sbin/temps wlan1 2> /dev/null\n");
		fprintf(fp, "UserParameter=temperature.wlan2, /usr/sbin/temps wlan2 2> /dev/null\n");
		fprintf(fp, "UserParameter=clients.wlan0, /usr/sbin/wclients wlan0\n");
		fprintf(fp, "UserParameter=clients.wlan1, /usr/sbin/wclients wlan1\n");
		fprintf(fp, "UserParameter=clients.wlan2, /usr/sbin/wclients wlan2\n");
#endif
		fprintf(fp, "UserParameter=clients.wired, /usr/sbin/clients\n");
		fprintf(fp, "UserParameter=system.topcpu[*], /usr/sbin/topcpu $1 $2\n");
		fprintf(fp, "UserParameter=listenport[*], netstat -ln 2> /dev/null  | grep -c ':'\n");
		fprintf(fp,
			"UserParameter=net.iptables.cksum, iptables-save | grep -v '^[#:]' | md5sum | tr -cd 0-9 | cut -b1-10\n");
		fprintf(fp, "UserParameter=net.ipv4.cksum,ifconfig | grep -B1 ' inet ' | md5sum | tr -cd 0-9 | cut -b1-10\n");
		fprintf(fp,
			"UserParameter=net.ipv4,ifconfig | grep -B1 ' inet ' | grep -o -e 'addr:[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*' -e '^[a-z0-9:]*'\n");
		fprintf(fp,
			"UserParameter=nvram.cksum, /usr/sbin/nvram show 2>/dev/null | grep -vE '^(forward_|traff|dnsmasq_lease_|http_client_)' | sort | md5sum | tr -cd '0-9' | cut -b1-10\n");
		fprintf(fp, "UserParameter=nvram.free, /usr/sbin/nvram show 2>&1 1>/dev/null | grep -o '(.*' | tr -cd '0-9'\n");
		if (*nvram_safe_get("zabbix_usrpara"))
			fprintf(fp, "\n%s\n", nvram_safe_get("zabbix_usrpara"));
		fclose(fp);

		dd_logstart("zabbix", eval("zabbix_agentd", "-c", "/tmp/zabbix.conf"));
	}

	return;
}

void stop_zabbix(void)
{
	stop_process("zabbix_agentd", "daemon");
	/* kill it once again since stop_process does not close all instances */
	if (pidof("zabbix_agentd") > 0) {
		eval("kill", "-9", pidof("zabbix_agentd"));
	}

	return;
}
#endif
