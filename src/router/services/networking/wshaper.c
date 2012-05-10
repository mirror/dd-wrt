/*
 * wshaper.c Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details. You should have received a copy of the GNU 
 * General Public License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA. $Id: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>

#include <bcmdevs.h>
#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <services.h>

extern char *get_mtu_val(void);
extern void add_client_mac_srvfilter(char *name, char *type, char *data, char *level, int base, char *client);
extern void add_client_ip_srvfilter(char *name, char *type, char *data, char *level, int base, char *client);


static char *get_wanface(void)
{
	char *dev = get_wan_face();
	
	if (!strcmp(dev, "br0"))
		dev = NULL;
	return dev;
}

static int client_bridged_enabled(void)
{
	// enumerate all possible interfaces
	char iflist[256];
	iflist[0] = 0; // workaround for bug in getIfList()
	getIfList(iflist, NULL);
	
	static char word[256];
	char *next;
	int bridged_clients=0;
	
	// any interface in client_bridged mode?
	foreach(word, iflist, next)
		if (nvram_nmatch("wet", "%s_mode", word))
			bridged_clients++;
	
	return bridged_clients;
}

#ifdef HAVE_SVQOS
void svqos_reset_ports(void)
{
#ifndef HAVE_XSCALE
#ifndef HAVE_MAGICBOX
#ifndef HAVE_RB600
#ifndef HAVE_FONERA
#ifndef HAVE_RT2880
#ifndef HAVE_LS2
#ifndef HAVE_SOLO51
#ifndef HAVE_LS5
#ifndef HAVE_X86
#ifndef HAVE_WHRAG108
#ifndef HAVE_CA8
#ifndef HAVE_PB42
#ifndef HAVE_LSX
#ifndef HAVE_DANUBE
#ifndef HAVE_STORM
#ifndef HAVE_LAGUNA
#ifndef HAVE_OPENRISC
#ifndef HAVE_ADM5120
#ifndef HAVE_TW6600
	if (nvram_match("portprio_support", "1")) {
		writeproc("/proc/switch/eth0/port/1/enable","1");
		writeproc("/proc/switch/eth0/port/2/enable","1");
		writeproc("/proc/switch/eth0/port/3/enable","1");
		writeproc("/proc/switch/eth0/port/4/enable","1");

		writeproc("/proc/switch/eth0/port/1/prio-enable","0");
		writeproc("/proc/switch/eth0/port/2/prio-enable","0");
		writeproc("/proc/switch/eth0/port/3/prio-enable","0");
		writeproc("/proc/switch/eth0/port/4/prio-enable","0");

		writeproc("/proc/switch/eth0/port/1/media","AUTO");
		writeproc("/proc/switch/eth0/port/2/media","AUTO");
		writeproc("/proc/switch/eth0/port/3/media","AUTO");
		writeproc("/proc/switch/eth0/port/4/media","AUTO");

		writeproc("/proc/switch/eth0/port/1/bandwidth","FULL");
		writeproc("/proc/switch/eth0/port/2/bandwidth","FULL");
		writeproc("/proc/switch/eth0/port/3/bandwidth","FULL");
		writeproc("/proc/switch/eth0/port/4/bandwidth","FULL");
	}
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
}

int svqos_set_ports(void)
{
#ifndef HAVE_XSCALE
#ifndef HAVE_MAGICBOX
#ifndef HAVE_RB600
#ifndef HAVE_FONERA
#ifndef HAVE_RT2880
#ifndef HAVE_LS2
#ifndef HAVE_LS5
#ifndef HAVE_WHRAG108
#ifndef HAVE_CA8
#ifndef HAVE_SOLO51
#ifndef HAVE_X86
#ifndef HAVE_LAGUNA
#ifndef HAVE_TW6600
#ifndef HAVE_PB42
#ifndef HAVE_LSX
#ifndef HAVE_DANUBE
#ifndef HAVE_STORM
#ifndef HAVE_OPENRISC
#ifndef HAVE_ADM5120
	if (nvram_match("portprio_support", "1")) {
		int loop = 1;
		char nvram_var[32] = {
			0
		}, *level;

		svqos_reset_ports();

		for (loop = 1; loop < 5; loop++) {
			snprintf(nvram_var, 31, "svqos_port%dbw", loop);

			if (strcmp("0", nvram_safe_get(nvram_var)))
				writevaproc(nvram_safe_get(nvram_var),"/proc/switch/eth0/port/%d/bandwidth",loop);
			else
				writevaproc("0","/proc/switch/eth0/port/%d/enable",loop);

				writevaproc("1","/proc/switch/eth0/port/%d/prio-enable",loop);
			level = nvram_nget("svqos_port%dprio", loop);
			char lvl[32];
			sprintf(lvl,"%d",atoi(level) / 10 - 1);
			writevaproc(lvl,"/proc/switch/eth0/port/%d/prio",loop);
		}
	}
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

	 return 0;
}

#ifndef HAVE_AQOS
static inline void
add_client_classes(unsigned int base, unsigned int level)
{
	char *wan_dev = get_wan_face();
	
	unsigned int uplimit = atoi(nvram_get("wshaper_uplink"));
	unsigned int downlimit = atoi(nvram_get("wshaper_downlink"));
	unsigned long lanlimit = 1000000;

	unsigned int quantum = atoi(get_mtu_val()) + 14;

	unsigned int uprate=0, downrate=0;
	
	switch (level) {
		case 100:
			uprate = uplimit;
			downrate = downlimit;
			break;
		case 10:
			uprate = uplimit/100*75;
			downrate = downlimit/100*75;
			break;
		case 20:
			uprate = uplimit/100*15;
			downrate = downlimit/100*15;
			break;
		case 30:
			uprate = uplimit/100*10;
			downrate = downlimit/100*10;
			break;
		case 40:
			uprate = 1;
			downrate = downlimit/100*1;
			break;
	}
	
	if (nvram_match("qos_type", "0")) {		// HTB
		
		sysprintf		// interior
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d",
			 wan_dev, 2, base, uprate, uplimit, quantum);
		sysprintf		// exempt
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0",
			 wan_dev, base, base+1, uprate, uplimit, quantum);
		sysprintf		// premium
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
			 wan_dev, base, base+2, (uprate/100*75)+1, uplimit, quantum);
		sysprintf		// express
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
			 wan_dev, base, base+3, (uprate/100*15)+1, uplimit, quantum);
		sysprintf		// standart
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
			 wan_dev, base, base+4, (uprate/100*10)+1, uplimit, quantum);
		sysprintf		// bulk
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
			 wan_dev, base, base+5, 1, uplimit, quantum);
		
		sysprintf		// interior
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d",
			 "imq0", 2, base, downrate, downlimit, quantum);
		sysprintf		// exempt
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0",
			 "imq0", base, base+1, downrate, downlimit, quantum);
		sysprintf		// premium
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
			 "imq0", base, base+2, (downrate/100*75)+1, downlimit, quantum);
		sysprintf		// express
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
			 "imq0", base, base+3, (downrate/100*15)+1, downlimit, quantum);
		sysprintf		// standard
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
			 "imq0", base, base+4, (downrate/100*10)+1, downlimit, quantum);
		sysprintf		// bulk
			("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
			 "imq0", base, base+5, 1, downlimit, quantum);
		
		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf		// interior
				("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d",
				 "imq1", 2, base, lanlimit, lanlimit, quantum);
			sysprintf		// exempt
				("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0",
				 "imq1", base, base+1, lanlimit, lanlimit, quantum);
			sysprintf		// premium
				("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
				 "imq1", base, base+2, lanlimit/100*75, lanlimit, quantum);
			sysprintf		// express
				("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
				 "imq1", base, base+3, lanlimit/100*15, lanlimit, quantum);
			sysprintf		// standard
				("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
				 "imq1", base, base+4, lanlimit/100*10, lanlimit, quantum);
			sysprintf		// bulk
				("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 1",
				 "imq1", base, base+5, 1, lanlimit, quantum);
		}	
	} else {	// HFSC
		sysprintf		// interior
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 wan_dev, 1, base, uprate, uplimit);
		sysprintf		// exempt
			("tc class add dev %s parent 1:%d classid 1:%d hfsc rt umax 1500b dmax 30ms rate 100kbit ls rate %dkbit ul rate %dkbit",
			 wan_dev, base, base+1, uprate, uplimit);
		sysprintf		// premium
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 wan_dev, base, base+2, uprate/100*75, uplimit);
		sysprintf		// express
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 wan_dev, base, base+3, uprate/100*15, uplimit);
		sysprintf		// standard
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 wan_dev, base, base+4, uprate/100*10, uplimit);
		sysprintf		// bulk
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 wan_dev, base, base+5, 1, uplimit);
		
		sysprintf		// interior
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 "imq0", 1, base, downrate, downlimit);
		sysprintf		// exempt
			("tc class add dev %s parent 1:%d classid 1:%d hfsc rt umax 1500b dmax 30ms rate 100kbit ls rate %dkbit ul rate %dkbit",
			 "imq0", base, base+1, downrate, downlimit);
		sysprintf		// premium
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 "imq0", base, base+2, downrate/100*75, downlimit);
		sysprintf		// express
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 "imq0", base, base+3, downrate/100*15, downlimit);
		sysprintf		// standard
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 "imq0", base, base+4, downrate/100*10, downlimit);
		sysprintf		// bulk
			("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
			 "imq0", base, base+5, 1, downlimit);

		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf		// interior
				("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
				 "imq1", 1, base, lanlimit, lanlimit);
			sysprintf		// exempt
				("tc class add dev %s parent 1:%d classid 1:%d hfsc rt umax 1500b dmax 30ms rate 100kbit ls rate %dkbit ul rate %dkbit",
				 "imq1", base, base+1, lanlimit, lanlimit);
			sysprintf		// premium
				("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
				 "imq1", base, base+2, lanlimit/100*75, lanlimit);
			sysprintf		// express
				("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
				 "imq1", base, base+3, lanlimit/100*15, lanlimit);
			sysprintf		// standard
				("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
				 "imq1", base, base+4, lanlimit/100*10, lanlimit);
			sysprintf
				("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit",
				 "imq1", base, base+5, 1, lanlimit);
		}
	}

	// filter rules
	sysprintf
		("tc filter add dev %s protocol ip pref 1 handle 0x%x fw classid 1:%d",
		 wan_dev, base, base+1);
	sysprintf
		("tc filter add dev %s protocol ip pref 3 handle 0x%x fw classid 1:%d",
		 wan_dev, base+1, base+2);
	sysprintf
		("tc filter add dev %s protocol ip pref 5 handle 0x%x fw classid 1:%d",
		 wan_dev, base+2, base+3);
	sysprintf
		("tc filter add dev %s protocol ip pref 8 handle 0x%x fw classid 1:%d",
		 wan_dev, base+3, base+4);
	sysprintf
		("tc filter add dev %s protocol ip pref 9 handle 0x%x fw classid 1:%d",
		 wan_dev, base+4, base+5);
	
	sysprintf
		("tc filter add dev %s protocol ip pref 1 handle 0x%x fw classid 1:%d",
		 "imq0", base, base+1);
	sysprintf
		("tc filter add dev %s protocol ip pref 3 handle 0x%x fw classid 1:%d",
		 "imq0", base+1, base+2);
	sysprintf
		("tc filter add dev %s protocol ip pref 5 handle 0x%x fw classid 1:%d",
		 "imq0", base+2, base+3);
	sysprintf
		("tc filter add dev %s protocol ip pref 8 handle 0x%x fw classid 1:%d",
		 "imq0", base+3, base+4);
	sysprintf
		("tc filter add dev %s protocol ip pref 9 handle 0x%x fw classid 1:%d",
		 "imq0", base+4, base+5);
	
	if (nvram_match("wshaper_dev", "LAN")) {
		sysprintf
			("tc filter add dev %s protocol ip pref 1 handle 0x%x fw classid 1:%d",
			 "imq1", base, base+1);
		sysprintf
			("tc filter add dev %s protocol ip pref 3 handle 0x%x fw classid 1:%d",
			 "imq1", base+1, base+2);
		sysprintf
			("tc filter add dev %s protocol ip pref 5 handle 0x%x fw classid 1:%d",
			 "imq1", base+2, base+3);
		sysprintf
			("tc filter add dev %s protocol ip pref 8 handle 0x%x fw classid 1:%d",
			 "imq1", base+3, base+4);
		sysprintf
			("tc filter add dev %s protocol ip pref 9 handle 0x%x fw classid 1:%d",
			 "imq1", base+4, base+5);
	}
	
	// leaf qdiscs
	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 wan_dev, base+1, base+1, quantum);
	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 wan_dev, base+2, base+2, quantum);
	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 wan_dev, base+3, base+3, quantum);
	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 wan_dev, base+4, base+4, quantum);
	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 wan_dev, base+5, base+5, quantum);

	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 "imq0", base+1, base+1, quantum);
	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 "imq0", base+2, base+2, quantum);
	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 "imq0", base+3, base+3, quantum);
	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 "imq0", base+4, base+4, quantum);
	sysprintf
		("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
		 "imq0", base+5, base+5, quantum);

	if (nvram_match("wshaper_dev", "LAN")) {
		sysprintf
			("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
			 "imq1", base+1, base+1, quantum);
		sysprintf
			("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
			 "imq1", base+2, base+2, quantum);
		sysprintf
			("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
			 "imq1", base+3, base+3, quantum);
		sysprintf
			("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
			 "imq1", base+4, base+4, quantum);
		sysprintf
			("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10",
			 "imq1", base+5, base+5, quantum);
	}
}

#elif HAVE_AQOS
#ifdef HAVE_OPENVPN
static inline int is_in_bridge(char *interface)
{
#define BUFFER_SIZE	256
	
	FILE *fd = NULL;;
	char buffer[BUFFER_SIZE];
	
	if (!interface)
		return 0;
	
	system2("/usr/sbin/brctl show > /tmp/bridges");
	
	fd = fopen("/tmp/bridges", "r");
	if (fd != NULL) {
		while (fgets(buffer, BUFFER_SIZE, fd) != NULL) {
			if (strstr(buffer, interface) != NULL) {
				fclose(fd);
				return 1;
			}
		}
		fclose(fd);
	}
	return 0;
}
#endif

extern void add_client_classes(unsigned int base, unsigned int uprate, unsigned int downrate, unsigned long lanrate);
//extern void add_userip(char *ip, int idx, char *upstream, char *downstream, char *lanstream);
//extern void add_usermac(char *mac, int idx, char *upstream, char *downstream, char *lanstream);

void aqos_tables(void)
{
	FILE *outips = fopen("/tmp/aqos_ips", "wb");
	FILE *outmacs = fopen("/tmp/aqos_macs", "wb");

	char *qos_ipaddr = nvram_safe_get("svqos_ips");
	char *qos_mac = nvram_safe_get("svqos_macs");
	char *qos_svcs = NULL;
	
	char level[32], level2[32], level3[32], data[32], type[32];
	char srvname[32], srvtype[32], srvdata[32], srvlevel[32];
	
	int base = 210;
	int ret = 0;

	do {
		ret = sscanf(qos_mac, "%31s %31s %31s %31s %31s |", data, level, level2, type, level3);
		if (ret < 5)
			break;
		
		fprintf(outmacs, "%s\n", data);
		add_client_classes(base, atoi(level), atoi(level2), atol(level3));
		
		qos_svcs = nvram_safe_get("svqos_svcs");
		do {
			if (sscanf
				(qos_svcs, "%31s %31s %31s %31s ", srvname, srvtype, srvdata,
				 srvlevel) < 4)
				break;
		
			add_client_mac_srvfilter(srvname, srvtype, srvdata, srvlevel, base, data);
		} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

		// not service-prioritized, then default class		
		sysprintf
			("iptables -t mangle -A FILTER_IN -m mac --mac-source %s -m mark --mark 1 -j MARK --set-mark %d",
			 data, base+3);
		
		base += 10;
	}
	while ((qos_mac = strpbrk(++qos_mac, "|")) && qos_mac++);

	do {
		ret = sscanf(qos_ipaddr, "%31s %31s %31s %31s |", data, level, level2, level3);
		if (ret < 4)
			break;
		
		fprintf(outips, "%s\n", data);
		add_client_classes(base, atoi(level), atoi(level2), atol(level3));
	
		qos_svcs = nvram_safe_get("svqos_svcs");
		do {
			if (sscanf
				(qos_svcs, "%31s %31s %31s %31s ", srvname, srvtype, srvdata,
				 srvlevel) < 4)
				break;
			
			add_client_ip_srvfilter(srvname, srvtype, srvdata, srvlevel, base, data);
		} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);
		
		// not service-prioritized, then default class		
		sysprintf
			("iptables -t mangle -A FILTER_OUT -s %s -m mark --mark 0 -j MARK --set-mark %d",
			 data, base+3);
		sysprintf
			("iptables -t mangle -A FILTER_OUT -d %s -m mark --mark 0 -j MARK --set-mark %d",
			 data, base+3);
		sysprintf
			("iptables -t mangle -A FILTER_IN -s %s -m mark --mark 1 -j MARK --set-mark %d",
			 data, base+3);
		sysprintf
			("iptables -t mangle -A FILTER_IN -d %s -m mark --mark 1 -j MARK --set-mark %d",
			 data, base+3);
		
		base += 10;
	}
	while ((qos_ipaddr = strpbrk(++qos_ipaddr, "|")) && qos_ipaddr++);
	
	fclose(outips);
	fclose(outmacs);
}
#endif

int svqos_iptables(void)
{
	char *qos_svcs = nvram_safe_get("svqos_svcs");	
	char name[32], type[32], data[32], level[32];
	
	char *wshaper_dev = nvram_get("wshaper_dev");	
	char *wan_dev = get_wanface();
	
	insmod("ipt_mark");
	insmod("xt_mark");
	insmod("ipt_CONNMARK");
	insmod("xt_CONNMARK");
	insmod("ipt_mac");
	insmod("xt_mac");
	
#ifdef HAVE_OPENDPI	
	insmod("/lib/opendpi/xt_opendpi.ko");
#endif
	insmod("ipt_layer7");

	// set-up mark/filter tables
	
	system2("iptables -t mangle -F SVQOS_SVCS");
	system2("iptables -t mangle -X SVQOS_SVCS");
	system2("iptables -t mangle -N SVQOS_SVCS");
	
	// chain to circumvent an problem with client specific service matches
	system2("iptables -t mangle -F FOO");
	system2("iptables -t mangle -X FOO");
	system2("iptables -t mangle -N FOO");
	
	system2("iptables -t mangle -F FILTER_OUT");
	system2("iptables -t mangle -X FILTER_OUT");
	system2("iptables -t mangle -N FILTER_OUT");
	system2("iptables -t mangle -A FILTER_OUT -j CONNMARK --restore");
	system2("iptables -t mangle -A FILTER_OUT -m mark --mark 0 -j SVQOS_SVCS");

	system2("iptables -t mangle -F FILTER_IN");
	system2("iptables -t mangle -X FILTER_IN");
	system2("iptables -t mangle -N FILTER_IN");
	system2("iptables -t mangle -A FILTER_IN -j FOO");
	system2("iptables -t mangle -A FILTER_IN -j CONNMARK --restore");
	system2("iptables -t mangle -A FILTER_IN -m mark --mark 0 -j SVQOS_SVCS");

	sysprintf("iptables -t mangle -D PREROUTING -j FILTER_IN");
	sysprintf("iptables -t mangle -I PREROUTING -j FILTER_IN");
	sysprintf("iptables -t mangle -D POSTROUTING -j FILTER_OUT");
	sysprintf("iptables -t mangle -I POSTROUTING -j FILTER_OUT");
	
	system2("iptables -t mangle -A POSTROUTING -m dscp --dscp ! 0 -j DSCP --set-dscp 0");

	if (!strcmp(wshaper_dev, "WAN")) {
		sysprintf("iptables -t mangle -D INPUT -i %s -j IMQ --todev 0", wan_dev);
		sysprintf("iptables -t mangle -A INPUT -i %s -j IMQ --todev 0", wan_dev);
		sysprintf("iptables -t mangle -D FORWARD -i %s -j IMQ --todev 0", wan_dev);
		sysprintf("iptables -t mangle -A FORWARD -i %s -j IMQ --todev 0", wan_dev);
	}	
	if (!strcmp(wshaper_dev, "LAN")) 
	{
		if ( !client_bridged_enabled() && nvram_invmatch("wan_proto", "disabled") ) 
		{
			sysprintf("iptables -t mangle -D INPUT -i %s -j IMQ --todev 0", wan_dev);
			sysprintf("iptables -t mangle -A INPUT -i %s -j IMQ --todev 0", wan_dev);
			sysprintf("iptables -t mangle -D FORWARD -i %s -j IMQ --todev 0", wan_dev);
			sysprintf("iptables -t mangle -A FORWARD -i %s -j IMQ --todev 0", wan_dev);
			
			sysprintf("iptables -t mangle -D INPUT -i ! %s -j IMQ --todev 1", wan_dev);
			sysprintf("iptables -t mangle -A INPUT -i ! %s -j IMQ --todev 1", wan_dev);
			sysprintf("iptables -t mangle -D FORWARD -i ! %s -o ! %s -j IMQ --todev 1", wan_dev, wan_dev);
			sysprintf("iptables -t mangle -A FORWARD -i ! %s -o ! %s -j IMQ --todev 1", wan_dev, wan_dev);
		} else {
			sysprintf("iptables -t mangle -D INPUT -j IMQ --todev 1");
			sysprintf("iptables -t mangle -A INPUT -j IMQ --todev 1");
			sysprintf("iptables -t mangle -D FORWARD -j IMQ --todev 1");
			sysprintf("iptables -t mangle -A FORWARD -j IMQ --todev 1");
		}
	}
	
	/* add openvpn filter rules */
#ifdef HAVE_AQOS
#ifdef HAVE_OPENVPN
	if (   nvram_invmatch("openvpn_enable", "0")
		|| nvram_invmatch("openvpncl_enable", "0"))
	{
		char iflist[256];
		static char word[256];
		char *next;
		bool unbridged_tap = 0;

		insmod("xt_dscp");
		insmod("xt_DSCP");

		system2("iptables -t mangle -F VPN_IN");
		system2("iptables -t mangle -X VPN_IN");
		system2("iptables -t mangle -N VPN_IN");
		system2("iptables -t mangle -A VPN_IN -j CONNMARK --save");

		system2("iptables -t mangle -F VPN_OUT");
		system2("iptables -t mangle -X VPN_OUT");
		system2("iptables -t mangle -N VPN_OUT");
		
		system2("iptables -t mangle -F VPN_DSCP");
		system2("iptables -t mangle -X VPN_DSCP");
		system2("iptables -t mangle -N VPN_DSCP");
		system2("iptables -t mangle -A VPN_DSCP -m dscp --dscp 10 -j MARK --set-mark 100");
		system2("iptables -t mangle -A VPN_DSCP -m dscp --dscp 1 -j MARK --set-mark 10");
		system2("iptables -t mangle -A VPN_DSCP -m dscp --dscp 2 -j MARK --set-mark 20");
		system2("iptables -t mangle -A VPN_DSCP -m dscp --dscp 3 -j MARK --set-mark 30");
		system2("iptables -t mangle -A VPN_DSCP -m dscp --dscp 4 -j MARK --set-mark 40");
		system2("iptables -t mangle -A VPN_DSCP -m dscp --dscp ! 0 -j DSCP --set-dscp 0");
		system2("iptables -t mangle -A VPN_DSCP -j RETURN");
		
		// look for present tun-devices
		if(getifcount("tun")) {
			system2("iptables -t mangle -I PREROUTING 2 -i tun+ -j VPN_IN");
			system2("iptables -t mangle -I INPUT 1 -i tun+ -j IMQ --todev 0");
			system2("iptables -t mangle -I FORWARD 1 -i tun+ -j IMQ --todev 0");
			system2("iptables -t mangle -I POSTROUTING 1 -o tun+ -j VPN_OUT");
		}

		// look for present tap-devices
		if (getifcount("tap"))
		{
			writeproc("/proc/sys/net/bridge/bridge-nf-call-arptables","1"); 
			writeproc("/proc/sys/net/bridge/bridge-nf-call-ip6tables","1"); 
			writeproc("/proc/sys/net/bridge/bridge-nf-call-iptables","1");
			
			insmod("xt_physdev");
			insmod("ebtables");
			
			getIfList(iflist, "tap");
			foreach(word, iflist, next) {
				if (is_in_bridge(word)) {
					sysprintf("iptables -t mangle -I PREROUTING 2 -m physdev --physdev-in %s -j VPN_IN", word);
					sysprintf("iptables -t mangle -I INPUT 1 -m physdev --physdev-in %s -j IMQ --todev 0", word);
					sysprintf("iptables -t mangle -I FORWARD 1 -m physdev --physdev-in %s -j IMQ --todev 0", word);
					sysprintf("iptables -t mangle -I POSTROUTING -m physdev --physdev-out %s -j VPN_OUT", word);
				} else
					unbridged_tap = 1;
			}

			if (unbridged_tap) {
				system2("iptables -t mangle -I PREROUTING 2 -i tap+ -j VPN_IN");
				system2("iptables -t mangle -I INPUT 1 -i tap+ -j IMQ --todev 0");
				system2("iptables -t mangle -I FORWARD 1 -i tap+ -j IMQ --todev 0");
				system2("iptables -t mangle -I POSTROUTING 1 -o tap+ -j VPN_OUT");
			}
		}

		//system2("iptables -t mangle -A POSTROUTING -m dscp --dscp ! 0 -j DSCP --set-dscp 0");

		char *qos_vpn = nvram_safe_get("svqos_vpns");

		/*
		 *  vpn format is "interface level | interface level |" ..etc 
		*/
		do {
			if (sscanf(qos_vpn, "%32s %32s |", data, level) < 2)
				break;
			
			/* incomming data */
			sysprintf("iptables -t mangle -I VPN_IN 1 -i %s -j MARK --set-mark %s",
					  data, level);

			/* outgoing data */
			if (is_in_bridge(data))
				sysprintf("iptables -t mangle -I VPN_OUT 1 -m physdev --physdev-out %s -j DSCP --set-dscp %d",
							data, atoi(level)/10 );
			else
				sysprintf("iptables -t mangle -I VPN_OUT 1 -o %s -j DSCP --set-dscp %d",
							data, atoi(level)/10 );
			
		} while ((qos_vpn = strpbrk(++qos_vpn, "|")) && qos_vpn++);
	}
#endif
	
	aqos_tables();
#endif
	
#ifndef HAVE_AQOS
	
	char *qos_mac = nvram_safe_get("svqos_macs");
	char *qos_ipaddr = nvram_safe_get("svqos_ips");
	
	char srvname[32], srvtype[32], srvdata[32], srvlevel[32];
	
	int base = 210;
	
	/*
	 *	mac format is "mac level | mac level |" ..etc 
	 */
	do {
		if (sscanf(qos_mac, "%31s %31s |", data, level) < 2)
			break;

		add_client_classes(base, atoi(level));

		qos_svcs = nvram_safe_get("svqos_svcs");	
		do {
			if (sscanf
				(qos_svcs, "%31s %31s %31s %31s ", srvname, srvtype, srvdata,
				 srvlevel) < 4)
				break;

			add_client_mac_srvfilter(srvname, srvtype, srvdata, srvlevel, base, data);
		} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);
		
		// not service-prioritized, then default class		
		sysprintf
			("iptables -t mangle -A FILTER_IN -m mark --mark 0 -m mac --mac-source %s -j MARK --set-mark %d",
			 data, base+3);
		
		base += 10;
	}
	while ((qos_mac = strpbrk(++qos_mac, "|")) && qos_mac++);

	/*
	 * ipaddr format is "ipaddr level | ipaddr level |" ..etc 
	 */
	do {
		
		if (sscanf(qos_ipaddr, "%31s %31s |", data, level) < 2)
			break;
		
		add_client_classes(base, atoi(level));

		qos_svcs = nvram_safe_get("svqos_svcs");
		do {
			if (sscanf
				(qos_svcs, "%31s %31s %31s %31s ", srvname, srvtype, srvdata,
				 srvlevel) < 4)
				break;

			add_client_ip_srvfilter(srvname, srvtype, srvdata, srvlevel, base, data);
		} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);
		
		// not service-prioritized, then default class		
		sysprintf
			("iptables -t mangle -A FILTER_OUT -s %s -m mark --mark 0 -j MARK --set-mark %d",
			 data, base+3);
		sysprintf
			("iptables -t mangle -A FILTER_OUT -d %s -m mark --mark 0 -j MARK --set-mark %d",
			 data, base+3);
		sysprintf
			("iptables -t mangle -A FILTER_IN -m mark --mark 0 -s %s -j MARK --set-mark %d",
			 data, base+3);
		sysprintf
			("iptables -t mangle -A FILTER_IN -m mark --mark 0 -d %s -j MARK --set-mark %d",
			 data, base+3);
		
		base += 10;
	}
	while ((qos_ipaddr = strpbrk(++qos_ipaddr, "|")) && qos_ipaddr++);
#endif
	
	// if OSPF is active put it into the Express bucket for outgoing QoS
	if (nvram_match("wk_mode", "ospf"))
		system2
		    ("iptables -t mangle -A FILTER_OUT -p ospf -j MARK --set-mark 20");

	if(!strcmp(wshaper_dev, "LAN")) {
		// don't let packages pass to iptables without ebtables loaded
		writeproc("/proc/sys/net/bridge/bridge-nf-call-arptables","1");
		writeproc("/proc/sys/net/bridge/bridge-nf-call-ip6tables","1");
		writeproc("/proc/sys/net/bridge/bridge-nf-call-iptables","1");
		
		insmod("ebtables");
	}

	qos_svcs = nvram_safe_get("svqos_svcs");	
	
	/*
	 * services format is "name type data level | name type data level |"
	 * ..etc 
	 */
	do {
		
		if (sscanf
		    (qos_svcs, "%31s %31s %31s %31s ", name, type, data,
		     level) < 4)
			break;
		
		if (strstr(type, "udp") || strstr(type, "both")) 
		{
			sysprintf
				("iptables -t mangle -A SVQOS_SVCS -p udp -m udp --dport %s -j MARK --set-mark %s",
				 data, level);
			sysprintf
				("iptables -t mangle -A SVQOS_SVCS -p udp -m udp --sport %s -j MARK --set-mark %s",
				 data, level);
			
			// workaround for non functional iptables matches
			sysprintf
				("iptables -t mangle -I FOO -p udp -m udp --dport %s -j MARK --set-mark 0x0",
				 data);
			sysprintf
				("iptables -t mangle -I FOO -p udp -m udp --sport %s -j MARK --set-mark 0x0",
				 data);
		}
		
		if (strstr(type, "tcp") || strstr(type, "both")) 
		{
			sysprintf
				("iptables -t mangle -A SVQOS_SVCS -p tcp -m tcp --dport %s -j MARK --set-mark %s",
				 data, level);
			sysprintf
				("iptables -t mangle -A SVQOS_SVCS -p tcp -m tcp --sport %s -j MARK --set-mark %s",
				 data, level);
			
			// workaround for non functional iptables matches
			sysprintf
				("iptables -t mangle -I FOO -p tcp -m tcp --dport %s -j MARK --set-mark 0x0",
				 data);
			sysprintf
				("iptables -t mangle -I FOO -p tcp -m tcp --sport %s -j MARK --set-mark 0x0",
				 data);
		}
		
		if (strstr(type, "l7")) 
		{
			sysprintf
				("iptables -t mangle -A SVQOS_SVCS -m layer7 --l7proto %s -j MARK --set-mark %s",
				 name, level);
			
			// workaround for non functional iptables matches
			sysprintf
				("iptables -t mangle -I FOO -m layer7 --l7proto %s -j MARK --set-mark 0x0",
				 name);
		}
		
#ifdef HAVE_OPENDPI
		if (strstr(type, "dpi")) 
		{
			sysprintf
				("iptables -t mangle -A SVQOS_SVCS -m opendpi --%s -j MARK --set-mark %s",
				 name, level);
			
			// workaround for non functional iptables matches
			sysprintf
				("iptables -t mangle -I FOO -m opendpi --%s -j MARK --set-mark 0x0",
				 name);
		}
#endif
		
		if (strstr(type, "p2p")) 
		{
			char *proto = NULL;
			char *realname = name;
			
			if (!strcasecmp(realname, "applejuice"))
				proto = "apple";
			else if (!strcasecmp(realname, "ares"))
				proto = "ares";
			else if (!strcasecmp(realname, "bearshare"))
				proto = "gnu";
			else if (!strcasecmp(realname, "bittorrent"))
				proto = "bit";
			else if (!strcasecmp(realname, "directconnect"))
				proto = "dc";
			else if (!strcasecmp(realname, "edonkey"))
				proto = "edk";
			else if (!strcasecmp(realname, "gnutella"))
				proto = "gnu";
			else if (!strcasecmp(realname, "kazaa"))
				proto = "kazaa";
			else if (!strcasecmp(realname, "mute"))
				proto = "mute";
			else if (!strcasecmp(realname, "soulseek"))
				proto = "soul";
			else if (!strcasecmp(realname, "waste"))
				proto = "waste";
			else if (!strcasecmp(realname, "winmx"))
				proto = "winmx";
			else if (!strcasecmp(realname, "xdcc"))
				proto = "xdcc";
			if (proto) 
			{
				insmod("ipt_ipp2p");
				sysprintf
					("iptables -t mangle -A SVQOS_SVCS -p tcp -m ipp2p --%s -j MARK --set-mark %s",
					 proto, level);
				
				// workaround for non functional iptables matches
				sysprintf
					("iptables -t mangle -I FOO -p tcp -m ipp2p --%s -j MARK --set-mark 0x0",
					 proto);
				
				if (!strcmp(proto, "bit")) 
				{
					// bittorrent detection enhanced 
#ifdef HAVE_MICRO
				sysprintf
					("iptables -t mangle -A SVQOS_SVCS -m layer7 --l7proto bt -j MARK --set-mark %s\n",
					 level);
					
				// workaround for non functional iptables matches
				sysprintf
					("iptables -t mangle -I FOO -m layer7 --l7proto bt -j MARK --set-mark 0x0");
#else
				sysprintf
					("iptables -t mangle -A SVQOS_SVCS -m length --length 0:550 -m layer7 --l7proto bt -j MARK --set-mark %s\n",
					 level);
					
				// workaround for non functional iptables matches
				sysprintf
					("iptables -t mangle -I FOO -m length --length 0:550 -m layer7 --l7proto bt -j MARK --set-mark 0x0");
#endif
				sysprintf
					("iptables -t mangle -A SVQOS_SVCS -m layer7 --l7proto bt2 -j MARK --set-mark %s\n",
					 level);

				// workaround for non functional iptables matches
				sysprintf
					("iptables -t mangle -I FOO -m layer7 --l7proto bt2 -j MARK --set-mark 0x0");
				}
			}
		}
	} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);
			
	// close mark-tables 
	system2("iptables -t mangle -A FILTER_IN -j CONNMARK --save");
	system2("iptables -t mangle -A FILTER_IN -p tcp -m length --length 0:64 --tcp-flags ACK ACK -j MARK --set-mark 0x64");
	system2("iptables -t mangle -A FILTER_IN -j RETURN");
	
	system2("iptables -t mangle -A FILTER_OUT -p tcp -m length --length 0:64 --tcp-flags ACK ACK -j CLASSIFY --set-class 1:100");
	system2("iptables -t mangle -A FILTER_OUT -m layer7 --l7proto dns -j CLASSIFY --set-class 1:100");
	system2("iptables -t mangle -A FILTER_OUT -j VPN_DSCP");
	system2("iptables -t mangle -A FILTER_OUT -j CONNMARK --save");
	system2("iptables -t mangle -A FILTER_OUT -j RETURN");

	system2("iptables -t mangle -A SVQOS_SVCS -j RETURN");
	
	// set port priority and port bandwidth
	svqos_set_ports();

	return 0;
}
#endif

void start_wshaper(void)
{	
//	int ret = 0;
	char *dl_val;
	char *ul_val;
	char *mtu_val = "1500";
	
	char *wshaper_dev;
	char *wan_dev;
	char *script_name;
	
	wan_dev = get_wanface();
	if (!wan_dev)
		wan_dev = "xx";
	
	wshaper_dev = nvram_safe_get("wshaper_dev");
	
	if (!nvram_invmatch("qos_type", "0"))
		script_name = "svqos";
	else
		script_name = "svqos2";

	stop_wshaper();
	if (!nvram_invmatch("wshaper_enable", "0"))
		return;
	
	if (!strcmp(wshaper_dev, "WAN") && (nvram_match("wan_proto", "disabled") || client_bridged_enabled()) )
		return;

	if ((dl_val = nvram_safe_get("wshaper_downlink")) == NULL &&
	    atoi(dl_val) > 0)
		return;
	if ((ul_val = nvram_safe_get("wshaper_uplink")) == NULL &&
	    atoi(ul_val) > 0)
		return;
	mtu_val = get_mtu_val();

#ifdef HAVE_SVQOS
	
	insmod("imq");
	insmod("ipt_IMQ");
	insmod("xt_IMQ");
	
	if (!strcmp(wshaper_dev, "WAN"))
		eval(script_name, ul_val, dl_val, wan_dev, mtu_val, "imq0");
	else
		eval(script_name, ul_val, dl_val, wan_dev, mtu_val, "imq0", "imq1");
	
	svqos_iptables();
	
#endif
	nvram_set("qos_done", "1");

 return;
}

void stop_wshaper(void)
{
	int ret = 0;

	char *wan_dev = get_wan_face();
	if (!wan_dev)
		wan_dev = "xx";

	char *script_name;
	if (!nvram_invmatch("qos_type", "0"))
		script_name = "svqos";
	else
		script_name = "svqos2";
	
	nvram_set("qos_done", "0");

	eval(script_name, "stop", "XX", wan_dev, "XX", "imq0", "imq1");
		
#ifdef HAVE_RB500
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_XSCALE
	ret = eval(script_name, "stop", "XX", "ixp0");
	ret = eval(script_name, "stop", "XX", "ixp1");
	ret = eval(script_name, "stop", "XX", "ath0");
	ret = eval(script_name, "stop", "XX", "ath1");
#elif HAVE_LAGUNA
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
#elif HAVE_MAGICBOX
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_RB600
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
	ret = eval(script_name, "stop", "XX", "eth2");
	ret = eval(script_name, "stop", "XX", "ath0");
	ret = eval(script_name, "stop", "XX", "ath1");
	ret = eval(script_name, "stop", "XX", "ath2");
	ret = eval(script_name, "stop", "XX", "ath3");
	ret = eval(script_name, "stop", "XX", "ath4");
	ret = eval(script_name, "stop", "XX", "ath5");
	ret = eval(script_name, "stop", "XX", "ath6");
	ret = eval(script_name, "stop", "XX", "ath7");
#elif HAVE_NS2
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_LC2
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_BS2
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_PICO2
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_PICO5
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_MS2
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_BS2HP
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_LS2
	ret = eval(script_name, "stop", "XX", "vlan0");
	ret = eval(script_name, "stop", "XX", "vlan2");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_SOLO51
	ret = eval(script_name, "stop", "XX", "vlan0");
	ret = eval(script_name, "stop", "XX", "vlan2");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_LS5
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_WRT54G2
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "vlan2");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_RTG32
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "vlan2");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_DIR300
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "vlan2");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_MR3202A
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "vlan2");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_RT2880
	ret = eval(script_name, "stop", "XX", "eth2");
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "vlan2");
	ret = eval(script_name, "stop", "XX", "ra0");
	ret = eval(script_name, "stop", "XX", "apcli0");
#elif HAVE_FONERA
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "vlan0");
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_WHRAG108
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
	ret = eval(script_name, "stop", "XX", "ath0");
	ret = eval(script_name, "stop", "XX", "ath1");
#elif HAVE_PB42
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
	ret = eval(script_name, "stop", "XX", "ath0");
	ret = eval(script_name, "stop", "XX", "ath1");
#elif HAVE_WR941
	ret = eval(script_name, "stop", "XX", "vlan0");
	ret = eval(script_name, "stop", "XX", "vlan1");
#elif HAVE_WA901v1
	ret = eval(script_name, "stop", "XX", "eth1");
#elif HAVE_WR741V4 
 	ret = eval(script_name, "stop", "XX", "vlan1"); 
 	ret = eval(script_name, "stop", "XX", "vlan2");
#elif HAVE_WR741
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
#elif HAVE_WR1043
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "vlan2");
#elif HAVE_WZRG450
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "vlan2");
#elif HAVE_DIR632
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
#elif HAVE_WNR2000
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_WHRHPGN
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_LSX
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
	ret = eval(script_name, "stop", "XX", "ath0");
	ret = eval(script_name, "stop", "XX", "ath1");
	ret = eval(script_name, "stop", "XX", "ath2");
#elif HAVE_DANUBE
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_WBD222
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
	ret = eval(script_name, "stop", "XX", "eth2");
	ret = eval(script_name, "stop", "XX", "ath0");
	ret = eval(script_name, "stop", "XX", "ath1");
	ret = eval(script_name, "stop", "XX", "ath2");
#elif HAVE_STORM
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_OPENRISC
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
	ret = eval(script_name, "stop", "XX", "eth2");
	ret = eval(script_name, "stop", "XX", "eth3");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_ADM5120
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "eth1");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_TW6600
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
	ret = eval(script_name, "stop", "XX", "ath1");
#elif HAVE_RDAT81
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
	ret = eval(script_name, "stop", "XX", "ath1");
#elif HAVE_RCAA01
	ret = eval(script_name, "stop", "XX", "vlan0");
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "ath0");
	ret = eval(script_name, "stop", "XX", "ath1");
#elif HAVE_CA8PRO
	ret = eval(script_name, "stop", "XX", "vlan0");
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_CA8
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#elif HAVE_X86
	ret = eval(script_name, "stop", "XX", "eth0");
	ret = eval(script_name, "stop", "XX", "ath0");
#else
	ret = eval(script_name, "stop", "XX", "vlan1");
	ret = eval(script_name, "stop", "XX", "eth1");
#endif
//	ret = eval(script_name, "stop", "XX", "ppp0");
	stop_firewall();
	start_firewall();

	// don't let packages pass to iptables without ebtables loaded
	writeproc("/proc/sys/net/bridge/bridge-nf-call-arptables","0");
	writeproc("/proc/sys/net/bridge/bridge-nf-call-ip6tables","0");
	writeproc("/proc/sys/net/bridge/bridge-nf-call-iptables","0");

#ifdef HAVE_OPENVPN
	rmmod("xt_dscp");
	rmmod("xt_DSCP");
	rmmod("xt_physdev");
#endif
	
	rmmod("ipt_mark");
	rmmod("xt_mark");
	rmmod("ipt_CONNMARK");
	rmmod("xt_CONNMARK");
	rmmod("/lib/opendpi/xt_opendpi.ko");
	rmmod("ipt_layer7");
	rmmod("ipt_mac");
	rmmod("xt_mac");		
	rmmod("xt_IMQ");		
	rmmod("ipt_IMQ");		
	rmmod("imq");
	rmmod("ebtables");
	
	return;
}
