/*
 * dhcpforward.c
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
#ifdef HAVE_DHCPFORWARD
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

void start_dhcpfwd(void)
{
	if (getWET())	// dont 
		// start 
		// any 
		// dhcp 
		// services 
		// in 
		// bridge 
		// mode
	{
		nvram_set("lan_proto", "static");
		return;
	}
#ifdef HAVE_DHCPFORWARD
	FILE *fp;

	if (nvram_match("dhcpfwd_enable", "1")) {
		mkdir("/tmp/dhcp-fwd", 0700);
		mkdir("/var/run/dhcp-fwd", 0700);
		fp = fopen("/tmp/dhcp-fwd/dhcp-fwd.conf", "wb");
		fprintf(fp, "user		root\n"
			"group		root\n"
			"chroot		/var/run/dhcp-fwd\n"
			"logfile		/tmp/dhcp-fwd.log\n"
			"loglevel	1\n"
			"pidfile		/var/run/dhcp-fwd.pid\n"
			"ulimit core	0\n"
			"ulimit stack	64K\n"
			"ulimit data	32K\n"
			"ulimit rss	200K\n"
			"ulimit nproc	0\n"
			"ulimit nofile	0\n"
			"ulimit as	0\n"
			"if	%s	true	false	true\n",
			nvram_safe_get("lan_ifname"));

		char *wan_proto = nvram_safe_get("wan_proto");
		char *wan_ifname = nvram_safe_get("wan_ifname");

		if (getSTA()) {
			wan_ifname = getSTA();	// returns eth1/eth2 for broadcom and 
			// ath0 for atheros
		}
#ifdef HAVE_PPPOE
		if (strcmp(wan_proto, "pppoe") == 0) {
			fprintf(fp, "if	ppp0	false	true	true\n");
		}
#else
		if (0) {
		}
#endif
		else if (getWET()) {
			// nothing
		} else if (strcmp(wan_proto, "dhcp") == 0
			   || strcmp(wan_proto, "static") == 0) {
			fprintf(fp, "if	%s	false	true	true\n",
				wan_ifname);
		}
#ifdef HAVE_3G
		else if (strcmp(wan_proto, "3g") == 0) {
			fprintf(fp, "if	ppp0	false	true	true\n");
		}
#endif
#ifdef HAVE_PPTP
		else if (strcmp(wan_proto, "pptp") == 0) {
			fprintf(fp, "if	ppp0	false	true	true\n");
		}
#endif
#ifdef HAVE_L2TP
		else if (strcmp(wan_proto, "l2tp") == 0) {
			fprintf(fp, "if	ppp0	false	true	true\n");
		}
#endif
#ifdef HAVE_HEARTBEAT
		else if (strcmp(wan_proto, "heartbeat") == 0) {
			fprintf(fp, "if	ppp0	false	true	true\n");
		}
#endif
		else {
			fprintf(fp, "if	%s	false	true	true\n",
				wan_ifname);
		}

		fprintf(fp, "name	%s	ws-c\n"
			"server	ip	%s\n",
			nvram_safe_get("lan_ifname"),
			nvram_safe_get("dhcpfwd_ip"));
		fclose(fp);
		eval("dhcpfwd", "-c", "/tmp/dhcp-fwd/dhcp-fwd.conf");
		syslog(LOG_INFO,
		       "dhcpfwd : dhcp forwarder daemon successfully started\n");
		return;
	}
#endif
#ifdef HAVE_DHCPRELAY
	if (nvram_match("dhcpfwd_enable", "1")) {
		eval("dhcrelay", "-i", nvram_safe_get("lan_ifname"),
		     nvram_safe_get("dhcpfwd_ip"));
		syslog(LOG_INFO,
		       "dhcrelay : dhcp relay successfully started\n");
	}
#endif
	return;
}

void stop_dhcpfwd(void)
{
#ifdef HAVE_DHCPFORWARD
	if (pidof("dhcpfwd") > 0) {
		syslog(LOG_INFO,
		       "dhcpfwd : dhcp forwarder daemon successfully stopped\n");
		killall("dhcpfwd", SIGTERM);	// kill also dhcp forwarder if
		// available
	}
#endif
#ifdef HAVE_DHCPRELAY
	if (pidof("dhcrelay") > 0) {
		syslog(LOG_INFO,
		       "dhcrelay : dhcp relay successfully stopped\n");
		killall("dhcrelay", SIGTERM);
	}
#endif
}
#endif
