/*
 * wshaper.c Copyright (C) 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <bcmdevs.h>
#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <services.h>
#include <broadcom.h>

static char *get_wanface(char *buf)
{
	char *dev = safe_get_wan_face(buf);

	if (!strcmp(dev, "br0"))
		return NULL;
	return dev;
}

int client_bridged_enabled(void)
{
	// enumerate all possible interfaces
	char iflist[512];
	iflist[0] = 0; // workaround for bug in getIfList()
	getIfList(iflist, NULL);

	char word[256];
	char *next;
	int bridged_clients = 0;

	// any interface in client_bridged mode?
	foreach(word, iflist, next) if (nvram_nmatch("wet", "%s_mode", word))
		bridged_clients++;

	return bridged_clients;
}

#ifdef HAVE_IPV6
#define evalip6(cmd, args...)                          \
	{                                              \
		if (nvram_match("ipv6_enable", "1")) { \
			eval_va(cmd, ##args, NULL);    \
		}                                      \
	}
#else
#define evalip6(...)
#endif

#ifdef HAVE_SVQOS

extern int get_mtu_val(void);
extern void add_client_mac_srvfilter(char *name, char *type, char *data,
				     int level, int base, char *client);
extern void add_client_ip_srvfilter(char *name, char *type, char *data,
				    int level, int base, char *client);
extern char *get_NFServiceMark(char *buffer, size_t len, char *service,
			       uint32 mark);

#if !defined(ARCH_broadcom) || defined(HAVE_BCMMODERN)
extern char *get_tcfmark(char *tcfmark, uint32 mark, int seg);
#endif

extern void add_client_classes(unsigned int base, unsigned int uprate,
			       unsigned int downrate, unsigned int lanrate,
			       unsigned int level);

static void svqos_reset_ports(void)
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
#ifndef HAVE_VENTANA
#ifndef HAVE_NEWPORT
#ifndef HAVE_EROUTER
#ifndef HAVE_OPENRISC
#ifndef HAVE_ADM5120
#ifndef HAVE_TW6600
	if (nvram_matchi("portprio_support", 1)) {
		writeproc("/proc/switch/eth0/port/1/enable", "1");
		writeproc("/proc/switch/eth0/port/2/enable", "1");
		writeproc("/proc/switch/eth0/port/3/enable", "1");
		writeproc("/proc/switch/eth0/port/4/enable", "1");

		writeproc("/proc/switch/eth0/port/1/prio-enable", "0");
		writeproc("/proc/switch/eth0/port/2/prio-enable", "0");
		writeproc("/proc/switch/eth0/port/3/prio-enable", "0");
		writeproc("/proc/switch/eth0/port/4/prio-enable", "0");

		writeproc("/proc/switch/eth0/port/1/media", "AUTO");
		writeproc("/proc/switch/eth0/port/2/media", "AUTO");
		writeproc("/proc/switch/eth0/port/3/media", "AUTO");
		writeproc("/proc/switch/eth0/port/4/media", "AUTO");

		writeproc("/proc/switch/eth0/port/1/bandwidth", "FULL");
		writeproc("/proc/switch/eth0/port/2/bandwidth", "FULL");
		writeproc("/proc/switch/eth0/port/3/bandwidth", "FULL");
		writeproc("/proc/switch/eth0/port/4/bandwidth", "FULL");
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
#endif
#endif
#endif
}

static int svqos_set_ports(void)
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
#ifndef HAVE_VENTANA
#ifndef HAVE_NEWPORT
#ifndef HAVE_EROUTER
#ifndef HAVE_TW6600
#ifndef HAVE_PB42
#ifndef HAVE_LSX
#ifndef HAVE_DANUBE
#ifndef HAVE_STORM
#ifndef HAVE_OPENRISC
#ifndef HAVE_ADM5120
	if (nvram_matchi("portprio_support", 1)) {
		int loop = 1;
		char nvram_var[32] = { 0 }, *level;

		svqos_reset_ports();

		for (loop = 1; loop < 5; loop++) {
			snprintf(nvram_var, 31, "svqos_port%dbw", loop);

			if (!nvram_matchi(nvram_var, 0))
				writevaproc(
					nvram_safe_get(nvram_var),
					"/proc/switch/eth0/port/%d/bandwidth",
					loop);
			else
				writevaproc("0",
					    "/proc/switch/eth0/port/%d/enable",
					    loop);

			writevaproc("1",
				    "/proc/switch/eth0/port/%d/prio-enable",
				    loop);
			level = nvram_nget("svqos_port%dprio", loop);
			char lvl[32];
			sprintf(lvl, "%d", atoi(level) / 10 - 1);
			writevaproc(lvl, "/proc/switch/eth0/port/%d/prio",
				    loop);
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
#endif
#endif
#endif

	return 0;
}

#ifdef HAVE_OPENVPN
static inline int is_in_bridge(char *interface)
{
#define BUFFER_SIZE 256

	FILE *fd = NULL;
	;
	char buffer[BUFFER_SIZE];

	if (!interface)
		return 0;

	system("brctl show > /tmp/bridges");

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

static char *s_downlist = NULL;
static char *s_iflist = NULL;
static int s_hasIF(char *list, char *ifname)
{
	char word[256];
	char *next;
	if (!list)
		return 0;
	foreach(word, list, next)
	{
		if (!strcmp(word, ifname))
			return 1;
	}
	return 0;
}

static void s_addIF(char **list, char *ifname)
{
	if (s_hasIF(*list, ifname))
		return;

	if (!*list) {
		*list = strdup(ifname);
	} else {
		*list = realloc(*list, strlen(*list) + strlen(ifname) + 2);
		strcat(*list, " ");
		strcat(*list, ifname);
	}
}

static void s_clearIF(char **list)
{
	if (*list)
		free(*list);
	*list = NULL;
}

#define addIF(ifname) s_addIF(&s_iflist, ifname)
#define hasIF(ifname) s_hasIF(s_iflist, ifname)
#define clearIF() s_clearIF(&s_iflist)

#define down_addIF(ifname) s_addIF(&s_downlist, ifname)
#define down_hasIF(ifname) s_hasIF(s_downlist, ifname)
#define down_clearIF() s_clearIF(&s_downlist)
static void down_upIF(void)
{
	char word[256];
	char *next;
	if (!s_downlist)
		return;
	foreach(word, s_downlist, next)
	{
		if (!is_mac80211(word)) {
			eval("ifconfig", word, "up");
		}
	}
	start_set_routes();
}

static void aqos_tables(void)
{
	FILE *outips = fopen("/tmp/aqos_ips", "wb");
	FILE *outmacs = fopen("/tmp/aqos_macs", "wb");

	char *qos_ipaddr = nvram_safe_get("svqos_ips");
	char *qos_mac = nvram_safe_get("svqos_macs");
	char *qos_svcs = NULL;

	char data[32], type[32], proto1[32], proto2[32], proto3[32], proto4[32],
		proto[32];
	char srvname[32], srvtype[32], srvdata[32];
	int srvlevel;
	int level, level2, level3, prio;
	char nullmask[24];
	char buffer[32];
	strcpy(nullmask, qos_nfmark(buffer, sizeof(buffer), 0));
	insmod("xt_physdev");

	int base = 210;
	int ret = 0;

	do {
		ret = sscanf(qos_mac, "%31s %d %d %31s %d %d |", data, &level,
			     &level2, type, &level3, &prio);
		if (ret < 6)
			break;

		fprintf(outmacs, "%s\n", data);
		add_client_classes(base, level, level2, level3, prio);

		qos_svcs = nvram_safe_get("svqos_svcs");
		do {
			if (sscanf(qos_svcs, "%31s %31s %31s %d ", srvname,
				   srvtype, srvdata, &srvlevel) < 4)
				break;

			add_client_mac_srvfilter(srvname, srvtype, srvdata,
						 srvlevel, base, data);
		} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

		// not service-prioritized, then default class

		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac",
		     "--mac-source", data, "-m", "mark", "--mark", nullmask,
		     "-j", "MARK", "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + 3));
		evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-m",
			"mac", "--mac-source", data, "-m", "mark", "--mark",
			nullmask, "-j", "MARK", "--set-mark",
			qos_nfmark(buffer, sizeof(buffer), base + 3));

		base += 10;
	} while ((qos_mac = strpbrk(++qos_mac, "|")) && qos_mac++);

	do {
		ret = sscanf(qos_ipaddr, "%31s %d %d %d %d |", data, &level,
			     &level2, &level3, &prio);
		if (ret < 5)
			break;

		fprintf(outips, "%s\n", data);
		add_client_classes(base, level, level2, level3, prio);

		qos_svcs = nvram_safe_get("svqos_svcs");
		do {
			if (sscanf(qos_svcs, "%31s %31s %31s %d ", srvname,
				   srvtype, srvdata, &srvlevel) < 4)
				break;

			add_client_ip_srvfilter(srvname, srvtype, srvdata,
						srvlevel, base, data);
		} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

		// not service-prioritized, then default class
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", data,
		     "-m", "mark", "--mark", nullmask, "-j", "MARK",
		     "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + 3));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", data,
		     "-m", "mark", "--mark", nullmask, "-j", "MARK",
		     "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + 3));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", data,
		     "-m", "mark", "--mark", nullmask, "-j", "MARK",
		     "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + 3));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", data,
		     "-m", "mark", "--mark", nullmask, "-j", "MARK",
		     "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + 3));

		base += 10;
	} while ((qos_ipaddr = strpbrk(++qos_ipaddr, "|")) && qos_ipaddr++);

	char *qos_devs = nvram_safe_get("svqos_devs");
	do {
		bzero(proto, sizeof(proto));
		ret = sscanf(qos_devs, "%31s %d %d %d %d %31s |", data, &level,
			     &level2, &level3, &prio, proto);
		if (ret < 5)
			break;
		if (!strcmp(proto, "|") || !strcmp(proto, "none")) {
			/* mark interface interface as priority / bandwidth limits based without any service filter */
			addIF(data);
		}
		char chainname_in[32];
		sprintf(chainname_in, "FILTER_%s_IN", data);
		char chainname_out[32];
		sprintf(chainname_out, "FILTER_%s_OUT", data);

		eval("iptables", "-t", "mangle", "-F", chainname_in);
		eval("iptables", "-t", "mangle", "-X", chainname_in);
		eval("iptables", "-t", "mangle", "-N", chainname_in);

		eval("iptables", "-t", "mangle", "-F", chainname_out);
		eval("iptables", "-t", "mangle", "-X", chainname_out);
		eval("iptables", "-t", "mangle", "-N", chainname_out);

		evalip6("ip6tables", "-t", "mangle", "-F", chainname_in);
		evalip6("ip6tables", "-t", "mangle", "-X", chainname_in);
		evalip6("ip6tables", "-t", "mangle", "-N", chainname_in);

		evalip6("ip6tables", "-t", "mangle", "-F", chainname_out);
		evalip6("ip6tables", "-t", "mangle", "-X", chainname_out);
		evalip6("ip6tables", "-t", "mangle", "-N", chainname_out);

		//              eval("iptables", "-t", "mangle", "-A", chainname_in, "-j", "CONNMARK", "--restore-mark");
		//              eval("iptables", "-t", "mangle", "-A", chainname_out, "-j", "CONNMARK", "--restore-mark");

		if (nvram_match("wshaper_dev", "LAN")) {
			if (nvram_nmatch("1", "%s_bridged", data)) {
				if (!is_mac80211(data))
					eval("ifconfig", data, "down");
				down_addIF(data);
				eval("iptables", "-t", "mangle", "-D", "INPUT",
				     "-m", "physdev", "--physdev-in", data,
				     "-j", "IMQ", "--todev", "0");
				eval("iptables", "-t", "mangle", "-A", "INPUT",
				     "-m", "physdev", "--physdev-in", data,
				     "-j", "IMQ", "--todev", "0");
				eval("iptables", "-t", "mangle", "-D",
				     "FORWARD", "-m", "physdev", "--physdev-in",
				     data, "-j", "IMQ", "--todev", "0");
				eval("iptables", "-t", "mangle", "-A",
				     "FORWARD", "-m", "physdev", "--physdev-in",
				     data, "-j", "IMQ", "--todev", "0");

				evalip6("ip6tables", "-t", "mangle", "-D",
					"INPUT", "-m", "physdev",
					"--physdev-in", data, "-j", "IMQ",
					"--todev", "0");
				evalip6("ip6tables", "-t", "mangle", "-A",
					"INPUT", "-m", "physdev",
					"--physdev-in", data, "-j", "IMQ",
					"--todev", "0");
				evalip6("ip6tables", "-t", "mangle", "-D",
					"FORWARD", "-m", "physdev",
					"--physdev-in", data, "-j", "IMQ",
					"--todev", "0");
				evalip6("ip6tables", "-t", "mangle", "-A",
					"FORWARD", "-m", "physdev",
					"--physdev-in", data, "-j", "IMQ",
					"--todev", "0");
			} else {
				eval("iptables", "-t", "mangle", "-A", "INPUT",
				     "-i", data, "-j", "IMQ", "--todev", "0");
				eval("iptables", "-t", "mangle", "-A",
				     "FORWARD", "-i", data, "-j", "IMQ",
				     "--todev", "0");

				evalip6("ip6tables", "-t", "mangle", "-A",
					"INPUT", "-i", data, "-j", "IMQ",
					"--todev", "0");
				evalip6("ip6tables", "-t", "mangle", "-A",
					"FORWARD", "-i", data, "-j", "IMQ",
					"--todev", "0");
			}
		}

		if (nvram_nmatch("1", "%s_bridged", data)) {
			//                      eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-m", "mark", "--mark", nullmask, "-m", "physdev", "--physdev-in", data, "-j", chainname_in);
			//                      eval("iptables", "-t", "mangle", "-D", "FILTER_OUT", "-m", "mark", "--mark", nullmask, "-m", "physdev", "--physdev-is-bridged", "--physdev-out", data, "-j", chainname_out);
			//                      eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "2", "-m", "mark", "--mark", nullmask, "-m", "physdev", "--physdev-in", data, "-j", chainname_in);
			//                      eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "2", "-m", "mark", "--mark", nullmask, "-m", "physdev", "--physdev-is-bridged", "--physdev-out", data, "-j", chainname_out);

			if (!is_mac80211(data))
				eval("ifconfig", data, "down");
			down_addIF(data);
			eval("iptables", "-t", "mangle", "-D", "FILTER_IN",
			     "-m", "physdev", "--physdev-in", data, "-j",
			     chainname_in);
			eval("iptables", "-t", "mangle", "-D", "FILTER_OUT",
			     "-m", "physdev", "--physdev-is-bridged",
			     "--physdev-out", data, "-j", chainname_out);
			eval("iptables", "-t", "mangle", "-A", "FILTER_IN",
			     "-m", "physdev", "--physdev-in", data, "-j",
			     chainname_in);
			eval("iptables", "-t", "mangle", "-A", "FILTER_OUT",
			     "-m", "physdev", "--physdev-is-bridged",
			     "--physdev-out", data, "-j", chainname_out);

			evalip6("ip6tables", "-t", "mangle", "-D", "FILTER_IN",
				"-m", "physdev", "--physdev-in", data, "-j",
				chainname_in);
			evalip6("ip6tables", "-t", "mangle", "-D", "FILTER_OUT",
				"-m", "physdev", "--physdev-is-bridged",
				"--physdev-out", data, "-j", chainname_out);
			evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN",
				"-m", "physdev", "--physdev-in", data, "-j",
				chainname_in);
			evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_OUT",
				"-m", "physdev", "--physdev-is-bridged",
				"--physdev-out", data, "-j", chainname_out);
		} else {
			//                      eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-m", "mark", "--mark", nullmask, "-i", data, "-j", chainname_in);
			//                      eval("iptables", "-t", "mangle", "-D", "FILTER_OUT", "-m", "mark", "--mark", nullmask, "-o", data, "-j", chainname_out);

			//                      eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mark", "--mark", nullmask, "-i", data, "-j", chainname_in);
			//                      eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-m", "mark", "--mark", nullmask, "-o", data, "-j", chainname_out);

			eval("iptables", "-t", "mangle", "-D", "FILTER_IN",
			     "-i", data, "-j", chainname_in);
			eval("iptables", "-t", "mangle", "-D", "FILTER_OUT",
			     "-o", data, "-j", chainname_out);

			eval("iptables", "-t", "mangle", "-A", "FILTER_IN",
			     "-i", data, "-j", chainname_in);
			eval("iptables", "-t", "mangle", "-A", "FILTER_OUT",
			     "-o", data, "-j", chainname_out);

			evalip6("ip6tables", "-t", "mangle", "-D", "FILTER_IN",
				"-i", data, "-j", chainname_in);
			evalip6("ip6tables", "-t", "mangle", "-D", "FILTER_OUT",
				"-o", data, "-j", chainname_out);

			evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN",
				"-i", data, "-j", chainname_in);
			evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_OUT",
				"-o", data, "-j", chainname_out);
		}

	} while ((qos_devs = strpbrk(++qos_devs, "|")) && qos_devs++);

	qos_devs = nvram_safe_get("svqos_devs");
	int oldbase = base;
	do {
		bzero(proto, sizeof(proto));
		ret = sscanf(qos_devs, "%31s %d %d %d %d %31s |", data, &level,
			     &level2, &level3, &prio, proto);
		if (ret < 5)
			break;
		if (!strcmp(proto, "|") || !strcmp(proto, "none")) {
			bzero(proto, sizeof(proto));
		}

		bzero(proto1, sizeof(proto1));
		bzero(proto2, sizeof(proto2));
		bzero(proto3, sizeof(proto3));
		bzero(proto4, sizeof(proto4));

		qos_svcs = nvram_safe_get("svqos_svcs");
		add_client_classes(base, level, level2, level3, prio);

		char chainname_in[32];
		sprintf(chainname_in, "FILTER_%s_IN", data);
		char chainname_out[32];
		sprintf(chainname_out, "FILTER_%s_OUT", data);
		if (*proto) {
			size_t plen = strlen(qos_svcs) + 128 + 2;
			char *svcs = malloc(plen);
			char *m = svcs;
			filters *s_filters = get_filters_list();
			int count = 0;
			while (s_filters[count].name != NULL) {
				if (!strcmp(s_filters[count].name, proto)) {
					char *protos[6] = { "tcp",  "udp",
							    "both", "l7",
							    "dpi",  "p2p",
							    "risk" };
					strcpy(proto2,
					       protos[s_filters[count].proto -
						      1]);
					strcpy(proto1, s_filters[count].name);
					sprintf(proto3, "%d:%d",
						s_filters[count].portfrom,
						s_filters[count].portto);
					sprintf(proto4, "%d", prio);
					break;
				}
				count++;
			}
			free_filters(s_filters);
			snprintf(svcs, plen, "%s %s %s %s", proto1, proto2,
				 proto3, proto4);
			do {
				if (sscanf(svcs, "%31s %31s %31s %d ", srvname,
					   srvtype, srvdata, &srvlevel) < 4)
					break;

				add_client_dev_srvfilter(srvname, srvtype,
							 srvdata, srvlevel,
							 base, chainname_in);
				add_client_dev_srvfilter(srvname, srvtype,
							 srvdata, srvlevel,
							 base, chainname_out);
			} while ((svcs = strpbrk(++svcs, "|")) && svcs++);

			free(m);
		}
		/* 
		 * check if interface has a none entry for interface based bandwidth limits or priorities. 
		 * in this case global level based services must take care if these limits 
		 */
		if (hasIF(data) && !*proto && *qos_svcs) {
			do {
				if (sscanf(qos_svcs, "%31s %31s %31s %d ",
					   srvname, srvtype, srvdata,
					   &srvlevel) < 4)
					break;

				add_client_dev_srvfilter(srvname, srvtype,
							 srvdata, srvlevel,
							 base, chainname_in);
				add_client_dev_srvfilter(srvname, srvtype,
							 srvdata, srvlevel,
							 base, chainname_out);

			} while ((qos_svcs = strpbrk(++qos_svcs, "|")) &&
				 qos_svcs++);
		}
		// not service-prioritized, then default class
		base += 10;

	} while ((qos_devs = strpbrk(++qos_devs, "|")) && qos_devs++);
	clearIF();

	qos_devs = nvram_safe_get("svqos_devs");
	base = oldbase;
	do {
		bzero(proto, sizeof(proto));
		ret = sscanf(qos_devs, "%31s %d %d %d %d %31s |", data, &level,
			     &level2, &level3, &prio, proto);
		if (ret < 5)
			break;

		char chainname_in[32];
		sprintf(chainname_in, "FILTER_%s_IN", data);
		char chainname_out[32];
		sprintf(chainname_out, "FILTER_%s_OUT", data);

		if (!strcmp(proto, "|") || !strcmp(proto, "none")) {
			bzero(proto, sizeof(proto));
			eval("iptables", "-t", "mangle", "-D", chainname_in,
			     "-m", "mark", "--mark", nullmask, "-j", "MARK",
			     "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), base + 3));
			eval("iptables", "-t", "mangle", "-D", chainname_out,
			     "-m", "mark", "--mark", nullmask, "-j", "MARK",
			     "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), base + 3));
			eval("iptables", "-t", "mangle", "-A", chainname_in,
			     "-m", "mark", "--mark", nullmask, "-j", "MARK",
			     "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), base + 3));
			eval("iptables", "-t", "mangle", "-A", chainname_out,
			     "-m", "mark", "--mark", nullmask, "-j", "MARK",
			     "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), base + 3));

			evalip6("ip6tables", "-t", "mangle", "-D", chainname_in,
				"-m", "mark", "--mark", nullmask, "-j", "MARK",
				"--set-mark",
				qos_nfmark(buffer, sizeof(buffer), base + 3));
			evalip6("ip6tables", "-t", "mangle", "-D",
				chainname_out, "-m", "mark", "--mark", nullmask,
				"-j", "MARK", "--set-mark",
				qos_nfmark(buffer, sizeof(buffer), base + 3));
			evalip6("ip6tables", "-t", "mangle", "-A", chainname_in,
				"-m", "mark", "--mark", nullmask, "-j", "MARK",
				"--set-mark",
				qos_nfmark(buffer, sizeof(buffer), base + 3));
			evalip6("ip6tables", "-t", "mangle", "-A",
				chainname_out, "-m", "mark", "--mark", nullmask,
				"-j", "MARK", "--set-mark",
				qos_nfmark(buffer, sizeof(buffer), base + 3));
		}
		eval("iptables", "-t", "mangle", "-D", chainname_in, "-j",
		     "RETURN");
		eval("iptables", "-t", "mangle", "-D", chainname_out, "-j",
		     "RETURN");
		eval("iptables", "-t", "mangle", "-A", chainname_in, "-j",
		     "RETURN");
		eval("iptables", "-t", "mangle", "-A", chainname_out, "-j",
		     "RETURN");

		evalip6("ip6tables", "-t", "mangle", "-D", chainname_in, "-j",
			"RETURN");
		evalip6("ip6tables", "-t", "mangle", "-D", chainname_out, "-j",
			"RETURN");
		evalip6("ip6tables", "-t", "mangle", "-A", chainname_in, "-j",
			"RETURN");
		evalip6("ip6tables", "-t", "mangle", "-A", chainname_out, "-j",
			"RETURN");
		base += 10;

	} while ((qos_devs = strpbrk(++qos_devs, "|")) && qos_devs++);

	fclose(outips);
	fclose(outmacs);
}

static int svqos_iptables(void)
{
	char wan_if_buffer[33];
	char *qos_pkts = nvram_safe_get("svqos_pkts");
	char *qos_svcs = nvram_safe_get("svqos_svcs");
	char name[32], type[32], data[32], pkt_filter[5];
	int level;
	char *wshaper_dev = nvram_safe_get("wshaper_dev");
	char ifbuf[16 + 1];
	char *wan_dev = get_wanface(ifbuf);
	char inv_wan_dev[33];
	char buffer[32];
	char nullmask[24];
	snprintf(inv_wan_dev, sizeof(inv_wan_dev), "!%s", wan_dev);
	strcpy(nullmask, qos_nfmark(buffer, sizeof(buffer), 0));

	insmod("ipt_mark");
	insmod("xt_mark");
	insmod("ipt_CONNMARK");
	insmod("xt_CONNMARK");
	insmod("ipt_mac");
	insmod("xt_mac");
	if (!strcmp(wshaper_dev, "LAN")) {
		// don't let packages pass to iptables without ebtables loaded
		writeprocsysnet("bridge/bridge-nf-call-arptables", "1");
		writeprocsysnet("bridge/bridge-nf-call-ip6tables", "1");
		writeprocsysnet("bridge/bridge-nf-call-iptables", "1");

		insmod("ebtables");
	}
#if !defined(ARCH_broadcom) || defined(HAVE_BCMMODERN)
	// if kernel version later then 2.4, overwrite all old tc filter
	sysprintf("tc filter del dev %s", wan_dev);
	sysprintf("tc filter del dev %s", "imq0");
	sysprintf("tc filter del dev %s", "imq1");
	if (nvram_match("wshaper_dev", "WAN") && wan_dev != NULL) {
		char tcfmark[32] = { 0 };
		char tcfmark2[32] = { 0 };
		//              eval("tc", "filter", "add", "dev", wan_dev, "protocol", "ip", "parent", "1:", "prio", "13", "u32", "match", "mark", get_tcfmark(tcfmark, 1000, 1), get_tcfmark(tcfmark2, 1000, 2), "flowid", "1:1000");
		eval("tc", "filter", "add", "dev", wan_dev, "protocol", "ip",
		     "parent", "1:", "prio", "15", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 100, 1),
		     get_tcfmark(tcfmark2, 100, 2), "flowid", "1:100");
		eval("tc", "filter", "add", "dev", wan_dev, "protocol", "ip",
		     "parent", "1:", "prio", "13", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 10, 1), get_tcfmark(tcfmark2, 10, 2),
		     "flowid", "1:10");
		eval("tc", "filter", "add", "dev", wan_dev, "protocol", "ip",
		     "parent", "1:", "prio", "11", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 20, 1), get_tcfmark(tcfmark2, 20, 2),
		     "flowid", "1:20");
		eval("tc", "filter", "add", "dev", wan_dev, "protocol", "ip",
		     "parent", "1:", "prio", "9", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 30, 1), get_tcfmark(tcfmark2, 30, 2),
		     "flowid", "1:30");
		eval("tc", "filter", "add", "dev", wan_dev, "protocol", "ip",
		     "parent", "1:", "prio", "3", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 40, 1), get_tcfmark(tcfmark2, 40, 2),
		     "flowid", "1:40");

		evalip6("tc", "filter", "add", "dev", wan_dev, "protocol",
			"ipv6", "parent", "1:", "prio", "16", "u32", "match",
			"mark", get_tcfmark(tcfmark, 100, 1),
			get_tcfmark(tcfmark2, 100, 2), "flowid", "1:100");
		evalip6("tc", "filter", "add", "dev", wan_dev, "protocol",
			"ipv6", "parent", "1:", "prio", "14", "u32", "match",
			"mark", get_tcfmark(tcfmark, 10, 1),
			get_tcfmark(tcfmark2, 10, 2), "flowid", "1:10");
		evalip6("tc", "filter", "add", "dev", wan_dev, "protocol",
			"ipv6", "parent", "1:", "prio", "12", "u32", "match",
			"mark", get_tcfmark(tcfmark, 20, 1),
			get_tcfmark(tcfmark2, 20, 2), "flowid", "1:20");
		evalip6("tc", "filter", "add", "dev", wan_dev, "protocol",
			"ipv6", "parent", "1:", "prio", "10", "u32", "match",
			"mark", get_tcfmark(tcfmark, 30, 1),
			get_tcfmark(tcfmark2, 30, 2), "flowid", "1:30");
		evalip6("tc", "filter", "add", "dev", wan_dev, "protocol",
			"ipv6", "parent", "1:", "prio", "4", "u32", "match",
			"mark", get_tcfmark(tcfmark, 40, 1),
			get_tcfmark(tcfmark2, 40, 2), "flowid", "1:40");

		init_ackprio(wan_dev);
	}

	{
		char tcfmark[32] = { 0 };
		char tcfmark2[32] = { 0 };
		//              eval("tc", "filter", "add", "dev", "imq0", "protocol", "ip", "parent", "1:", "prio", "13", "u32", "match", "mark", get_tcfmark(tcfmark, 1000, 1), get_tcfmark(tcfmark2, 1000, 2), "flowid", "1:1000");
		eval("tc", "filter", "add", "dev", "imq0", "protocol", "ip",
		     "parent", "1:", "prio", "15", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 100, 1),
		     get_tcfmark(tcfmark2, 100, 2), "flowid", "1:100");
		eval("tc", "filter", "add", "dev", "imq0", "protocol", "ip",
		     "parent", "1:", "prio", "13", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 10, 1), get_tcfmark(tcfmark2, 10, 2),
		     "flowid", "1:10");
		eval("tc", "filter", "add", "dev", "imq0", "protocol", "ip",
		     "parent", "1:", "prio", "11", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 20, 1), get_tcfmark(tcfmark2, 20, 2),
		     "flowid", "1:20");
		eval("tc", "filter", "add", "dev", "imq0", "protocol", "ip",
		     "parent", "1:", "prio", "9", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 30, 1), get_tcfmark(tcfmark2, 30, 2),
		     "flowid", "1:30");
		eval("tc", "filter", "add", "dev", "imq0", "protocol", "ip",
		     "parent", "1:", "prio", "3", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 40, 1), get_tcfmark(tcfmark2, 40, 2),
		     "flowid", "1:40");

		evalip6("tc", "filter", "add", "dev", "imq0", "protocol",
			"ipv6", "parent", "1:", "prio", "16", "u32", "match",
			"mark", get_tcfmark(tcfmark, 100, 1),
			get_tcfmark(tcfmark2, 100, 2), "flowid", "1:100");
		evalip6("tc", "filter", "add", "dev", "imq0", "protocol",
			"ipv6", "parent", "1:", "prio", "14", "u32", "match",
			"mark", get_tcfmark(tcfmark, 10, 1),
			get_tcfmark(tcfmark2, 10, 2), "flowid", "1:10");
		evalip6("tc", "filter", "add", "dev", "imq0", "protocol",
			"ipv6", "parent", "1:", "prio", "12", "u32", "match",
			"mark", get_tcfmark(tcfmark, 20, 1),
			get_tcfmark(tcfmark2, 20, 2), "flowid", "1:20");
		evalip6("tc", "filter", "add", "dev", "imq0", "protocol",
			"ipv6", "parent", "1:", "prio", "10", "u32", "match",
			"mark", get_tcfmark(tcfmark, 30, 1),
			get_tcfmark(tcfmark2, 30, 2), "flowid", "1:30");
		evalip6("tc", "filter", "add", "dev", "imq0", "protocol",
			"ipv6", "parent", "1:", "prio", "4", "u32", "match",
			"mark", get_tcfmark(tcfmark, 40, 1),
			get_tcfmark(tcfmark2, 40, 2), "flowid", "1:40");
	}
	if (nvram_match("wshaper_dev", "LAN")) {
		char tcfmark[32] = { 0 };
		char tcfmark2[32] = { 0 };

		//              eval("tc", "filter", "add", "dev", "imq1", "protocol", "ip", "parent", "1:", "prio", "13", "u32", "match", "mark", get_tcfmark(tcfmark, 1000, 1), get_tcfmark(tcfmark2, 1000, 2), "flowid", "1:1000");
		eval("tc", "filter", "add", "dev", "imq1", "protocol", "ip",
		     "parent", "1:", "prio", "15", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 100, 1),
		     get_tcfmark(tcfmark2, 100, 2), "flowid", "1:100");
		eval("tc", "filter", "add", "dev", "imq1", "protocol", "ip",
		     "parent", "1:", "prio", "13", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 10, 1), get_tcfmark(tcfmark2, 10, 2),
		     "flowid", "1:10");
		eval("tc", "filter", "add", "dev", "imq1", "protocol", "ip",
		     "parent", "1:", "prio", "11", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 20, 1), get_tcfmark(tcfmark2, 20, 2),
		     "flowid", "1:20");
		eval("tc", "filter", "add", "dev", "imq1", "protocol", "ip",
		     "parent", "1:", "prio", "9", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 30, 1), get_tcfmark(tcfmark2, 30, 2),
		     "flowid", "1:30");
		eval("tc", "filter", "add", "dev", "imq1", "protocol", "ip",
		     "parent", "1:", "prio", "3", "u32", "match", "mark",
		     get_tcfmark(tcfmark, 40, 1), get_tcfmark(tcfmark2, 40, 2),
		     "flowid", "1:40");

		evalip6("tc", "filter", "add", "dev", "imq1", "protocol",
			"ipv6", "parent", "1:", "prio", "16", "u32", "match",
			"mark", get_tcfmark(tcfmark, 100, 1),
			get_tcfmark(tcfmark2, 100, 2), "flowid", "1:100");
		evalip6("tc", "filter", "add", "dev", "imq1", "protocol",
			"ipv6", "parent", "1:", "prio", "14", "u32", "match",
			"mark", get_tcfmark(tcfmark, 10, 1),
			get_tcfmark(tcfmark2, 10, 2), "flowid", "1:10");
		evalip6("tc", "filter", "add", "dev", "imq1", "protocol",
			"ipv6", "parent", "1:", "prio", "12", "u32", "match",
			"mark", get_tcfmark(tcfmark, 20, 1),
			get_tcfmark(tcfmark2, 20, 2), "flowid", "1:20");
		evalip6("tc", "filter", "add", "dev", "imq1", "protocol",
			"ipv6", "parent", "1:", "prio", "10", "u32", "match",
			"mark", get_tcfmark(tcfmark, 30, 1),
			get_tcfmark(tcfmark2, 30, 2), "flowid", "1:30");
		evalip6("tc", "filter", "add", "dev", "imq1", "protocol",
			"ipv6", "parent", "1:", "prio", "4", "u32", "match",
			"mark", get_tcfmark(tcfmark, 40, 1),
			get_tcfmark(tcfmark2, 40, 2), "flowid", "1:40");
	}
#endif

	// set-up mark/filter tables

	eval("iptables", "-t", "mangle", "-F", "SVQOS_SVCS");
	eval("iptables", "-t", "mangle", "-X", "SVQOS_SVCS");
	eval("iptables", "-t", "mangle", "-N", "SVQOS_SVCS");

	eval("iptables", "-t", "mangle", "-F", "FILTER_OUT");
	eval("iptables", "-t", "mangle", "-X", "FILTER_OUT");
	eval("iptables", "-t", "mangle", "-N", "FILTER_OUT");
	eval("iptables", "-t", "mangle", "-F", "FILTER_IN");
	eval("iptables", "-t", "mangle", "-X", "FILTER_IN");
	eval("iptables", "-t", "mangle", "-N", "FILTER_IN");
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "CONNMARK",
	     "--restore-mark");
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "CONNMARK",
	     "--restore-mark");

	eval("iptables", "-t", "mangle", "-D", "PREROUTING", "-j", "FILTER_IN");
	eval("iptables", "-t", "mangle", "-I", "PREROUTING", "-j", "FILTER_IN");
	eval("iptables", "-t", "mangle", "-D", "POSTROUTING", "-j",
	     "FILTER_OUT");
	eval("iptables", "-t", "mangle", "-I", "POSTROUTING", "-j",
	     "FILTER_OUT");

	evalip6("ip6tables", "-t", "mangle", "-F", "SVQOS_SVCS");
	evalip6("ip6tables", "-t", "mangle", "-X", "SVQOS_SVCS");
	evalip6("ip6tables", "-t", "mangle", "-N", "SVQOS_SVCS");

	evalip6("ip6tables", "-t", "mangle", "-F", "FILTER_OUT");
	evalip6("ip6tables", "-t", "mangle", "-X", "FILTER_OUT");
	evalip6("ip6tables", "-t", "mangle", "-N", "FILTER_OUT");
	evalip6("ip6tables", "-t", "mangle", "-F", "FILTER_IN");
	evalip6("ip6tables", "-t", "mangle", "-X", "FILTER_IN");
	evalip6("ip6tables", "-t", "mangle", "-N", "FILTER_IN");
	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_OUT", "-j",
		"CONNMARK", "--restore-mark");
	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-j",
		"CONNMARK", "--restore-mark");

	evalip6("ip6tables", "-t", "mangle", "-D", "PREROUTING", "-j",
		"FILTER_IN");
	evalip6("ip6tables", "-t", "mangle", "-I", "PREROUTING", "-j",
		"FILTER_IN");
	evalip6("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-j",
		"FILTER_OUT");
	evalip6("ip6tables", "-t", "mangle", "-I", "POSTROUTING", "-j",
		"FILTER_OUT");

	//      insmod("xt_dscp");
	//      insmod("xt_DSCP");
	//      eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-m", "dscp", "--dscp", "!", "0", "-j", "DSCP", "--set-dscp", "0");

	if (!strcmp(wshaper_dev, "WAN") && wan_dev != NULL) {
		eval("iptables", "-t", "mangle", "-D", "INPUT", "-i", wan_dev,
		     "-j", "IMQ", "--todev", "0");
		eval("iptables", "-t", "mangle", "-A", "INPUT", "-i", wan_dev,
		     "-j", "IMQ", "--todev", "0");
		eval("iptables", "-t", "mangle", "-D", "FORWARD", "-i", wan_dev,
		     "-j", "IMQ", "--todev", "0");
		eval("iptables", "-t", "mangle", "-A", "FORWARD", "-i", wan_dev,
		     "-j", "IMQ", "--todev", "0");

		evalip6("ip6tables", "-t", "mangle", "-D", "INPUT", "-i",
			wan_dev, "-j", "IMQ", "--todev", "0");
		evalip6("ip6tables", "-t", "mangle", "-A", "INPUT", "-i",
			wan_dev, "-j", "IMQ", "--todev", "0");
		evalip6("ip6tables", "-t", "mangle", "-D", "FORWARD", "-i",
			wan_dev, "-j", "IMQ", "--todev", "0");
		evalip6("ip6tables", "-t", "mangle", "-A", "FORWARD", "-i",
			wan_dev, "-j", "IMQ", "--todev", "0");
	}
	if (!strcmp(wshaper_dev, "LAN")) {
		if (!client_bridged_enabled() &&
		    nvram_invmatch("wan_proto", "disabled")) {
			if (wan_dev != NULL) {
				eval("iptables", "-t", "mangle", "-D", "INPUT",
				     "-i", wan_dev, "-j", "IMQ", "--todev",
				     "0");
				eval("iptables", "-t", "mangle", "-A", "INPUT",
				     "-i", wan_dev, "-j", "IMQ", "--todev",
				     "0");
				eval("iptables", "-t", "mangle", "-D",
				     "FORWARD", "-i", wan_dev, "-j", "IMQ",
				     "--todev", "0");
				eval("iptables", "-t", "mangle", "-A",
				     "FORWARD", "-i", wan_dev, "-j", "IMQ",
				     "--todev", "0");

				eval("iptables", "-t", "mangle", "-D", "INPUT",
				     "-i", inv_wan_dev, "-j", "IMQ", "--todev",
				     "1");
				eval("iptables", "-t", "mangle", "-A", "INPUT",
				     "-i", inv_wan_dev, "-j", "IMQ", "--todev",
				     "1");

				eval("iptables", "-t", "mangle", "-D",
				     "FORWARD", "-i", inv_wan_dev, "-o",
				     inv_wan_dev, "-j", "IMQ", "--todev", "1");
				eval("iptables", "-t", "mangle", "-A",
				     "FORWARD", "-i", inv_wan_dev, "-o",
				     inv_wan_dev, "-j", "IMQ", "--todev", "1");

				evalip6("ip6tables", "-t", "mangle", "-D",
					"INPUT", "-i", wan_dev, "-j", "IMQ",
					"--todev", "0");
				evalip6("ip6tables", "-t", "mangle", "-A",
					"INPUT", "-i", wan_dev, "-j", "IMQ",
					"--todev", "0");
				evalip6("ip6tables", "-t", "mangle", "-D",
					"FORWARD", "-i", wan_dev, "-j", "IMQ",
					"--todev", "0");
				evalip6("ip6tables", "-t", "mangle", "-A",
					"FORWARD", "-i", wan_dev, "-j", "IMQ",
					"--todev", "0");

				evalip6("ip6tables", "-t", "mangle", "-D",
					"INPUT", "-i", inv_wan_dev, "-j", "IMQ",
					"--todev", "1");
				evalip6("ip6tables", "-t", "mangle", "-A",
					"INPUT", "-i", inv_wan_dev, "-j", "IMQ",
					"--todev", "1");

				evalip6("ip6tables", "-t", "mangle", "-D",
					"FORWARD", "-i", inv_wan_dev, "-o",
					inv_wan_dev, "-j", "IMQ", "--todev",
					"1");
				evalip6("ip6tables", "-t", "mangle", "-A",
					"FORWARD", "-i", inv_wan_dev, "-o",
					inv_wan_dev, "-j", "IMQ", "--todev",
					"1");
			}
		} else {
			eval("iptables", "-t", "mangle", "-D", "INPUT", "-j",
			     "IMQ", "--todev", "1");
			eval("iptables", "-t", "mangle", "-A", "INPUT", "-j",
			     "IMQ", "--todev", "1");
			eval("iptables", "-t", "mangle", "-D", "FORWARD", "-j",
			     "IMQ", "--todev", "1");
			eval("iptables", "-t", "mangle", "-A", "FORWARD", "-j",
			     "IMQ", "--todev", "1");

			evalip6("ip6tables", "-t", "mangle", "-D", "INPUT",
				"-j", "IMQ", "--todev", "1");
			evalip6("ip6tables", "-t", "mangle", "-A", "INPUT",
				"-j", "IMQ", "--todev", "1");
			evalip6("ip6tables", "-t", "mangle", "-D", "FORWARD",
				"-j", "IMQ", "--todev", "1");
			evalip6("ip6tables", "-t", "mangle", "-A", "FORWARD",
				"-j", "IMQ", "--todev", "1");
		}
	}
	//      /* add openvpn filter rules */
	//#ifdef HAVE_OPENVPN
	//      if (nvram_invmatchi("openvpn_enable", 0) || nvram_invmatchi("openvpncl_enable", 0)) {
	//              char iflist[256];
	//              char word[256];
	//              char *next;
	//              bool unbridged_tap = 0;
	//
	//              insmod("xt_dscp");
	//              insmod("xt_DSCP");
	//
	//              eval("iptables", "-t", "mangle", "-F", "VPN_IN");
	//              eval("iptables", "-t", "mangle", "-X", "VPN_IN");
	//              eval("iptables", "-t", "mangle", "-N", "VPN_IN");
	//              eval("iptables", "-t", "mangle", "-A", "VPN_IN", "-j", "CONNMARK", "--save-mark");
	//
	//              eval("iptables", "-t", "mangle", "-F", "VPN_OUT");
	//              eval("iptables", "-t", "mangle", "-X", "VPN_OUT");
	//              eval("iptables", "-t", "mangle", "-N", "VPN_OUT");
	//
	//              eval("iptables", "-t", "mangle", "-F", "VPN_DSCP");
	//              eval("iptables", "-t", "mangle", "-X", "VPN_DSCP");
	//              eval("iptables", "-t", "mangle", "-N", "VPN_DSCP");
	//              eval("iptables", "-t", "mangle", "-A", "VPN_DSCP", "-m", "dscp", "--dscp", "10", "-j", "MARK", "--set-mark", qos_nfmark(buffer,sizeof(buffer),100));
	//              eval("iptables", "-t", "mangle", "-A", "VPN_DSCP", "-m", "dscp", "--dscp", "1", "-j", "MARK", "--set-mark", qos_nfmark(buffer,sizeof(buffer),10));
	//              eval("iptables", "-t", "mangle", "-A", "VPN_DSCP", "-m", "dscp", "--dscp", "2", "-j", "MARK", "--set-mark", qos_nfmark(buffer,sizeof(buffer),20));
	//              eval("iptables", "-t", "mangle", "-A", "VPN_DSCP", "-m", "dscp", "--dscp", "3", "-j", "MARK", "--set-mark", qos_nfmark(buffer,sizeof(buffer),30));
	//              eval("iptables", "-t", "mangle", "-A", "VPN_DSCP", "-m", "dscp", "--dscp", "4", "-j", "MARK", "--set-mark", qos_nfmark(buffer,sizeof(buffer),40));
	//              eval("iptables", "-t", "mangle", "-A", "VPN_DSCP", "-m", "dscp", "--dscp", "!", "0", "-j", "DSCP", "--set-dscp", "0");
	//              eval("iptables", "-t", "mangle", "-A", "VPN_DSCP", "-j", "RETURN");
	//
	//              // look for present tun-devices
	//              if (getifcount("tun")) {
	//                      eval("iptables", "-t", "mangle", "-A", "PREROUTING", "-i", "tun+", "-j", "VPN_IN");
	//                      eval("iptables", "-t", "mangle", "-A", "INPUT", "-i", "tun+", "-j", "IMQ", "--todev", "0");
	//                      eval("iptables", "-t", "mangle", "-A", "FORWARD", "-i", "tun+", "-j", "IMQ", "--todev", "0");
	//                      eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-o", "tun+", "-j", "VPN_OUT");
	//              }
	//              insmod("xt_physdev");
	//              // look for present tap-devices
	//              if (getifcount("tap")) {
	//                      writeprocsysnet("bridge/bridge-nf-call-arptables", "1");
	//                      writeprocsysnet("bridge/bridge-nf-call-ip6tables", "1");
	//                      writeprocsysnet("bridge/bridge-nf-call-iptables", "1");
	//
	//                      insmod("ebtables");
	//
	//                      getIfList(iflist, "tap");
	//                      foreach(word, iflist, next) {
	//                              if (is_in_bridge(word)) {
	//                                      if (!is_mac80211(word))
	//                                              eval("ifconfig", word, "down");
	//                                      down_addIF(word);
	//                                      eval("iptables", "-t", "mangle", "-A", "PREROUTING", "-m", "physdev", "--physdev-in", word, "-j", "VPN_IN");
	//                                      eval("iptables", "-t", "mangle", "-A", "INPUT", "-m", "physdev", "--physdev-is-bridged", "--physdev-in", word, "-j", "IMQ", "--todev", "0");
	//                                      eval("iptables", "-t", "mangle", "-A", "FORWARD", "-m", "physdev", "--physdev-in", word, "-j", "IMQ", "--todev", "0");
	//                                      eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-m", "physdev", "--physdev-is-bridged", "--physdev-out", word, "-j", "VPN_OUT");
	//                              } else
	//                                      unbridged_tap = 1;
	//                      }
	//
	//                      if (unbridged_tap) {
	//                              eval("iptables", "-t", "mangle", "-A", "PREROUTING", "-i", "tap+", "-j", "VPN_IN");
	//                              eval("iptables", "-t", "mangle", "-A", "INPUT", "-i", "tap+", "-j", "IMQ", "--todev", "0");
	//                              eval("iptables", "-t", "mangle", "-A", "FORWARD", "-i", "tap+", "-j", "IMQ", "--todev", "0");
	//                              eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-o", "tap+", "-j", "VPN_OUT");
	//                      }
	//              }
	//              //system("iptables -t mangle -A POSTROUTING -m dscp --dscp ! 0 -j DSCP --set-dscp 0");
	//
	//              char *qos_vpn = nvram_safe_get("svqos_vpns");
	//              insmod("xt_dscp");
	//              insmod("xt_DSCP");
	//
	//              /*
	//               *  vpn format is "interface level | interface level |" ..etc
	//               */
	//              do {
	//                      if (sscanf(qos_vpn, "%32s %d |", data, &level) < 2)
	//                              break;
	//
	//                      /* incomming data */
	//                      eval("iptables", "-t", "mangle", "-A", "VPN_IN", "-i", data, "-j", "MARK", "--set-mark", qos_nfmark(buffer,sizeof(buffer),level));
	//                      char s_level[32];
	//                      sprintf(s_level, "%d", level / 10);
	//                      /* outgoing data */
	//                      if (is_in_bridge(data)) {
	//                              if (!is_mac80211(data))
	//                                      eval("ifconfig", data, "down");
	//                              down_addIF(data);
	//                              eval("iptables", "-t", "mangle", "-A", "VPN_OUT", "-m", "physdev", "--physdev-is-bridged", "--physdev-out", data, "-j", "DSCP", "--set-dscp", s_level);
	//                      } else
	//                              eval("iptables", "-t", "mangle", "-A", "VPN_OUT", "-o", data, "-j", "DSCP", "--set-dscp", s_level);
	//
	//              } while ((qos_vpn = strpbrk(++qos_vpn, "|")) && qos_vpn++);
	//      }
	//#endif

	aqos_tables();

	if (wan_dev && strcmp(wan_dev, "wwan0")) {
		if (wanactive(get_wan_ipaddr()) &&
		    (nvram_matchi("block_loopback", 0) ||
		     nvram_match("filter", "off"))) {
			char *wan_face = safe_get_wan_face(wan_if_buffer);
			char inv_wan_face[33];
			snprintf(inv_wan_face, sizeof(inv_wan_face), "!%s",
				 wan_face);
			eval("iptables", "-t", "mangle", "-A", "PREROUTING",
			     "-i", inv_wan_face, "-d", get_wan_ipaddr(), "-j",
			     "MARK", "--set-mark",
			     get_NFServiceMark(buffer, sizeof(buffer),
					       "FORWARD", 1));
			eval("iptables", "-t", "mangle", "-A", "PREROUTING",
			     "-j", "CONNMARK", "--save-mark");

			evalip6("ip6tables", "-t", "mangle", "-A", "PREROUTING",
				"-i", inv_wan_face, "-d", get_wan_ipaddr(),
				"-j", "MARK", "--set-mark",
				get_NFServiceMark(buffer, sizeof(buffer),
						  "FORWARD", 1));
			evalip6("ip6tables", "-t", "mangle", "-A", "PREROUTING",
				"-j", "CONNMARK", "--save-mark");
		}
	}
	// if OSPF is active put it into the Express bucket for outgoing QoS
	if (nvram_match("wk_mode", "ospf")) {
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p",
		     "ospf", "-j", "MARK", "--set-mark", nullmask);
		evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_OUT", "-p",
			"ospf", "-j", "MARK", "--set-mark", nullmask);
	}
	qos_svcs = nvram_safe_get("svqos_svcs");

	/*
	 * services format is "name type data level | name type data level |"
	 * ..etc 
	 */
	do {
		if (sscanf(qos_svcs, "%31s %31s %31s %d ", name, type, data,
			   &level) < 4)
			break;

		if (strstr(type, "udp") || strstr(type, "both")) {
			eval("iptables", "-t", "mangle", "-A", "SVQOS_SVCS",
			     "-p", "udp", "-m", "udp", "--dport", data, "-j",
			     "MARK", "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), level));
			eval("iptables", "-t", "mangle", "-A", "SVQOS_SVCS",
			     "-p", "udp", "-m", "udp", "--sport", data, "-j",
			     "MARK", "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), level));

			evalip6("ip6tables", "-t", "mangle", "-A", "SVQOS_SVCS",
				"-p", "udp", "-m", "udp", "--dport", data, "-j",
				"MARK", "--set-mark",
				qos_nfmark(buffer, sizeof(buffer), level));
			evalip6("ip6tables", "-t", "mangle", "-A", "SVQOS_SVCS",
				"-p", "udp", "-m", "udp", "--sport", data, "-j",
				"MARK", "--set-mark",
				qos_nfmark(buffer, sizeof(buffer), level));
		}

		if (strstr(type, "tcp") || strstr(type, "both")) {
			eval("iptables", "-t", "mangle", "-A", "SVQOS_SVCS",
			     "-p", "tcp", "-m", "tcp", "--dport", data, "-j",
			     "MARK", "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), level));
			eval("iptables", "-t", "mangle", "-A", "SVQOS_SVCS",
			     "-p", "tcp", "-m", "tcp", "--sport", data, "-j",
			     "MARK", "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), level));

			evalip6("ip6tables", "-t", "mangle", "-A", "SVQOS_SVCS",
				"-p", "tcp", "-m", "tcp", "--dport", data, "-j",
				"MARK", "--set-mark",
				qos_nfmark(buffer, sizeof(buffer), level));
			evalip6("ip6tables", "-t", "mangle", "-A", "SVQOS_SVCS",
				"-p", "tcp", "-m", "tcp", "--sport", data, "-j",
				"MARK", "--set-mark",
				qos_nfmark(buffer, sizeof(buffer), level));
		}
		if (name && (!strcmp(name, "windows-telemetry") ||
			     !strcmp(name, "ubnt-telemetry")))
			continue;
		if (strstr(type, "l7")) {
			insmod("ipt_layer7");
			insmod("xt_layer7");
			eval("iptables", "-t", "mangle", "-A", "SVQOS_SVCS",
			     "-m", "layer7", "--l7proto", name, "-j", "MARK",
			     "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), level));
			evalip6("ip6tables", "-t", "mangle", "-A", "SVQOS_SVCS",
				"-m", "layer7", "--l7proto", name, "-j", "MARK",
				"--set-mark",
				qos_nfmark(buffer, sizeof(buffer), level));
		}
#ifdef HAVE_OPENDPI
		if (strstr(type, "dpi")) {
			insmod("xt_ndpi");
			eval("iptables", "-t", "mangle", "-A", "SVQOS_SVCS",
			     "-m", "ndpi", "--proto", name, "-j", "MARK",
			     "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), level));
			evalip6("ip6tables", "-t", "mangle", "-A", "SVQOS_SVCS",
				"-m", "ndpi", "--proto", name, "-j", "MARK",
				"--set-mark",
				qos_nfmark(buffer, sizeof(buffer), level));
		}
		if (strstr(type, "risk")) {
			insmod("xt_ndpi");
			int risk = get_risk_by_name(name);
			char *dep = get_dep_by_name(name);
			char lvl[32];
			sprintf(lvl, "%d", risk);
			eval("iptables", "-t", "mangle", "-A", "SVQOS_SVCS",
			     "-m", "ndpi", "--proto", dep, "--risk", lvl, "-j",
			     "MARK", "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), level));
			evalip6("ip6tables", "-t", "mangle", "-A", "SVQOS_SVCS",
				"-m", "ndpi", "--proto", dep, "--risk", lvl,
				"-j", "MARK", "--set-mark",
				qos_nfmark(buffer, sizeof(buffer), level));
		}
#endif

		if (strstr(type, "p2p")) {
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
			if (proto) {
				insmod("ipt_ipp2p");
				insmod("xt_ipp2p");
				char s_proto[32];
				sprintf(s_proto, "--%s", proto);
				eval("iptables", "-t", "mangle", "-A",
				     "SVQOS_SVCS", "-p", "tcp", "-m", "ipp2p",
				     s_proto, "-j", "MARK", "--set-mark",
				     qos_nfmark(buffer, sizeof(buffer), level));
				evalip6("ip6tables", "-t", "mangle", "-A",
					"SVQOS_SVCS", "-p", "tcp", "-m",
					"ipp2p", s_proto, "-j", "MARK",
					"--set-mark",
					qos_nfmark(buffer, sizeof(buffer),
						   level));

				if (!strcmp(proto, "bit")) {
					// bittorrent detection enhanced
#ifdef HAVE_MICRO
					eval("iptables", "-t", "mangle", "-A",
					     "SVQOS_SVCS", "-m", "layer7",
					     "--l7proto", "bt", "-j", "MARK",
					     "--set-mark",
					     qos_nfmark(buffer, sizeof(buffer),
							level));
#else
					eval("iptables", "-t", "mangle", "-A",
					     "SVQOS_SVCS", "-m", "length",
					     "--length", "0:550", "-m",
					     "layer7", "--l7proto", "bt", "-j",
					     "MARK", "--set-mark",
					     qos_nfmark(buffer, sizeof(buffer),
							level));
#endif
					eval("iptables", "-t", "mangle", "-A",
					     "SVQOS_SVCS", "-m", "layer7",
					     "--l7proto", "bt2", "-j", "MARK",
					     "--set-mark",
					     qos_nfmark(buffer, sizeof(buffer),
							level));

					evalip6("ip6tables", "-t", "mangle",
						"-A", "SVQOS_SVCS", "-m",
						"length", "--length", "0:550",
						"-m", "layer7", "--l7proto",
						"bt", "-j", "MARK",
						"--set-mark",
						qos_nfmark(buffer,
							   sizeof(buffer),
							   level));
					evalip6("ip6tables", "-t", "mangle",
						"-A", "SVQOS_SVCS", "-m",
						"layer7", "--l7proto", "bt2",
						"-j", "MARK", "--set-mark",
						qos_nfmark(buffer,
							   sizeof(buffer),
							   level));
				}
			}
		}
	} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

	// close mark-tables
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mark",
	     "--mark", nullmask, "-j", "SVQOS_SVCS");
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "CONNMARK",
	     "--save-mark");
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "RETURN");
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-m", "mark",
	     "--mark", nullmask, "-j", "SVQOS_SVCS");

	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mark",
		"--mark", nullmask, "-j", "SVQOS_SVCS");
	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-j",
		"CONNMARK", "--save-mark");
	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-j", "RETURN");
	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_OUT", "-m", "mark",
		"--mark", nullmask, "-j", "SVQOS_SVCS");
//      if (nvram_invmatchi("openvpn_enable", 0) || nvram_invmatchi("openvpncl_enable", 0)) {
//              eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "VPN_DSCP");
//      }
#if 1
#ifdef HAVE_80211AC
	if (nvram_match("bcmdebug", "1"))
#endif
	{
		// seems to crash northstar

		// http://svn.dd-wrt.com:8000/ticket/2803 && http://svn.dd-wrt.com/ticket/2811
		do {
			if (sscanf(qos_pkts, "%4s ", pkt_filter) < 1)
				break;
			if (!strcmp(pkt_filter, "ICMP")) {
				eval("iptables", "-t", "mangle", "-A",
				     "FILTER_OUT", "-p", "icmp", "-j",
				     "CLASSIFY", "--set-class", "1:100");
				evalip6("ip6tables", "-t", "mangle", "-A",
					"FILTER_OUT", "-p", "icmp", "-j",
					"CLASSIFY", "--set-class", "1:100");
			} else {
				eval("iptables", "-t", "mangle", "-A",
				     "FILTER_OUT", "-p", "tcp", "-m", "tcp",
				     "--tcp-flags", pkt_filter, pkt_filter,
				     "-m", "length", "--length", "0:64", "-j",
				     "CLASSIFY", "--set-class", "1:100");
				evalip6("ip6tables", "-t", "mangle", "-A",
					"FILTER_OUT", "-p", "tcp", "-m", "tcp",
					"--tcp-flags", pkt_filter, pkt_filter,
					"-m", "length", "--length", "0:64",
					"-j", "CLASSIFY", "--set-class",
					"1:100");
			}
		} while ((qos_pkts = strpbrk(++qos_pkts, "|")) && qos_pkts++);
	}
#endif
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "CONNMARK",
	     "--save-mark");
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "RETURN");

	//      /* anything which doesnt match should get default class */
	//      eval("iptables", "-t", "mangle", "-A", "SVQOS_SVCS", "-j", "MARK", "--set-mark", qos_nfmark(buffer,sizeof(buffer),30));

	eval("iptables", "-t", "mangle", "-A", "SVQOS_SVCS", "-j", "RETURN");

	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_OUT", "-j",
		"CONNMARK", "--save-mark");
	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_OUT", "-j",
		"RETURN");
	evalip6("ip6tables", "-t", "mangle", "-A", "SVQOS_SVCS", "-j",
		"RETURN");

	// set port priority and port bandwidth
	svqos_set_ports();

	return 0;
}
#endif

void start_qos(void)
{
	//      int ret = 0;
	int dl;
	int ul;

	char *wshaper_dev;
	char *wan_dev;
	char *aqd;
	char ifbuf[16 + 1];
	wan_dev = get_wanface(ifbuf);
	if (!wan_dev)
		wan_dev = "xx";
	if (!nvram_match("tcp_congestion_control", "bbr"))
		writeprocsysnet("core/default_qdisc", "sfq");

	wshaper_dev = nvram_safe_get("wshaper_dev");

	if (!nvram_matchi("wshaper_enable", 1)) {
#ifdef HAVE_SFE
		start_sfe();
#endif
		return;
	}
#ifdef HAVE_SFE
	else
		stop_sfe();
#endif
	if (!strcmp(wshaper_dev, "WAN") &&
	    (nvram_match("wan_proto", "disabled") || client_bridged_enabled()))
		return;

	writeint("/sys/fast_classifier/skip_to_bridge_ingress", 1);

	if (nvram_empty("wshaper_downlink") || nvram_empty("wshaper_uplink"))
		return;
	dl = nvram_geti("wshaper_downlink");
	ul = nvram_geti("wshaper_uplink");

	int mtu = get_mtu_val();
#ifdef HAVE_SVQOS
	aqd = nvram_safe_get("svqos_aqd");

	insmod("imq");
	insmod("ipt_IMQ");
	insmod("xt_IMQ");
	insmod("sch_red");
	insmod("sch_hfsc");
	insmod("sch_htb");
	insmod("sch_sfq");
	insmod("sch_tbf");
	insmod("sch_cbq");

#ifdef HAVE_CODEL
	if (!strcmp(aqd, "codel")) {
		insmod("sch_codel");
	}
#endif

#ifdef HAVE_FQ_CODEL
	if (!strcmp(aqd, "fq_codel")) {
		insmod("sch_fq_codel");
	}
#endif
#ifdef HAVE_FQ_CODEL_FAST
	if (!strcmp(aqd, "fq_codel_fast")) {
		insmod("sch_fq_codel_fast");
	}
#endif
#ifdef HAVE_PIE
	if (!strcmp(aqd, "pie")) {
		insmod("sch_pie");
	}
#endif
#ifdef HAVE_CAKE
	if (!strcmp(aqd, "cake")) {
		insmod("sch_cake");
	}
#endif

	//under K3 interface defaults are way to high, set some sane values
	eval("ifconfig", "imq0", "down");
	eval("ifconfig", "imq0", "mtu", "1500");
	eval("ifconfig", "imq0", "txqueuelen", "30");
	eval("ifconfig", "imq0", "up");

	if (!strcmp(wshaper_dev, "WAN")) {
		eval("ifconfig", "imq1", "down");
		init_qos(nvram_matchi("qos_type", 0) ? "htb" : "hfsc", ul, dl,
			 wan_dev, mtu, "imq0", aqd, NULL);
	} else {
		eval("ifconfig", "imq1", "down");
		eval("ifconfig", "imq1", "mtu", "1500");
		eval("ifconfig", "imq1", "txqueuelen", "30");
		eval("ifconfig", "imq1", "up");
		init_qos(nvram_matchi("qos_type", 0) ? "htb" : "hfsc", ul, dl,
			 wan_dev, mtu, "imq0", aqd, "imq1");
	}
	svqos_iptables();
#endif
	down_upIF();
	down_clearIF();
	nvram_seti("qos_done", 1);

#ifdef HAVE_REGISTER
#ifndef HAVE_ERC
	if (isregistered_real())
#endif
#endif
	{
		runStartup(".firewall");
		create_rc_file(RC_FIREWALL);
		if (f_exists("/tmp/.rc_firewall")) {
			setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
			system("/tmp/.rc_firewall");
		}
	}
	return;
}

void stop_qos(void)
{
	char wan_if_buffer[33];

	//if imq is not available we don't have to run
	int ret = 0;

	char *wan_dev = safe_get_wan_face(wan_if_buffer);
	if (!wan_dev)
		wan_dev = "xx";

	nvram_seti("qos_done", 0);
	deinit_qos(wan_dev, "imq0", "imq1");

	char eths2[512];
	char eths[512];
	bzero(eths, 512);
	bzero(eths2, 512);
	bzero(eths, 512);
	getIfList(eths, "ixp");
	bzero(eths2, 512);
	getIfList(eths2, "eth");
	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 512);
	getIfList(eths2, "imq");

	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 512);
	getIfList(eths2, "ppp");
	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 512);
	getIfList(eths2, "tun");
	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 512);
	getIfList(eths2, "tap");
	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 512);
	getIfList(eths2, "vlan");
	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 512);
	getIfList(eths2, "wlan");
	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 512);
	getIfList(eths2, "wl");
	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 512);
	getIfList(eths2, "ra");
	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 512);
	getIfList(eths2, "rb");
	strcat(eths, " ");
	strcat(eths, eths2);

	char *next;
	char var[80];
	char *vifs = eths;
	foreach(var, vifs, next)
	{
		if (ifexists(var)) {
			eval("tc", "qdisc", "del", "dev", var, "root");
		}
	}

	// don't let packages pass to iptables without ebtables loaded
	writeprocsysnet("bridge/bridge-nf-call-arptables", "0");
	writeprocsysnet("bridge/bridge-nf-call-ip6tables", "0");
	writeprocsysnet("bridge/bridge-nf-call-iptables", "0");

	rmmod("xt_IMQ");
	rmmod("ipt_IMQ");
	rmmod("imq");
	rmmod("sch_codel");
	rmmod("sch_fq_codel");
	rmmod("sch_fq_codel_fast");
	rmmod("sch_cake");
	//      rmmod("ebtables");
	rmmod("sch_red");
	rmmod("sch_hfsc");
	rmmod("sch_htb");
	rmmod("sch_sfq");
	rmmod("sch_tbf");
	rmmod("sch_cbq");

	return;
}

#ifdef TEST
void start_set_routes(void)
{
}

void runStartup(char *call)
{
}

int create_rc_file(char *p)
{
}

int main(int argc, char *argv[])
{
	start_wshaper();
}
#endif
