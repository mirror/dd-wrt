/*
 * firewall.c
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

// #define DEVELOPE_ENV
// #define XBOX_SUPPORT /* Define Microsoft XBox, game machine, support */
#define AOL_SUPPORT		/* Define AOL support */
// #define FLOOD_PROTECT /* Define flooding protection */
// #define REVERSE_RULE_ORDER /* If it needs to reverse the rule's
// sequential. It is used
// when the MARK match/target be using. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <net/if.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <l7protocols.h>

#ifndef DEVELOPE_ENV
#include <bcmnvram.h>
#include <shutils.h>
#include <rc.h>
#include <code_pattern.h>
#include <utils.h>
#include <wlutils.h>
#include <cy_conf.h>
#endif				/* DEVELOPE_ENV */
#include <services.h>

/*
 * Same as the file "linux/netfilter_ipv4/ipt_webstr.h" 
 */
#define BLK_JAVA				0x01
#define BLK_ACTIVE				0x02
#define BLK_COOKIE				0x04
#define BLK_PROXY				0x08

/*
 * possible files path 
 */
#define IPTABLES_SAVE_FILE		"/tmp/.ipt"
#define CRONTAB 			"/tmp/crontab"
#define IPTABLES_RULE_STAT		"/tmp/.rule"

/*
 * Known port 
 */
#define DNS_PORT		53	/* UDP */
#define TFTP_PORT		69	/* UDP */
#define ISAKMP_PORT 	500	/* UDP */
#define RIP_PORT		520	/* UDP */
#define L2TP_PORT		1701	/* UDP */

#define HTTP_PORT		80	/* TCP */
#define IDENT_PORT		113	/* TCP */
#define HTTPS_PORT		443	/* TCP */
#define PPTP_PORT		1723	/* TCP */

#define IP_MULTICAST			"224.0.0.0/4"

/*
 * Limitation definition 
 */
#define NR_IPGROUPS 	5
#define NR_MACGROUPS	5

/*
 * MARK number in mangle table 
 */
#define MARK_OFFSET 	0x10
// #define MARK_MASK 0xf0
#define MARK_DROP		0x1e
// #define MARK_ACCEPT 0x1f
// #define MARK_HTTP 0x30
#define MARK_LAN2WAN	0x100	/* For HotSpot */

#ifdef FLOOD_PROTECT
#define FLOOD_RATE		200
#define TARG_PASS		"limaccept"	/* limited of accepting chain 
						 */
#define TARG_RST		"logreject"
#else
#define TARG_PASS		"ACCEPT"
#define TARG_RST		"REJECT --reject-with tcp-reset"
#endif

#if 0
#define DEBUG printf
#else
#define DEBUG(format, args...)
#endif

static char *suspense = NULL;
static unsigned int count = 0;
static char log_accept[15];
static char log_drop[15];
static char log_reject[64];
static int web_lanport = HTTP_PORT;

static unsigned int now_wday, now_hrmin;

static void va_save2file(char *prefix, const char *fmt, va_list args)
{
	FILE *fp;
	if ((fp = fopen(IPTABLES_SAVE_FILE, "a")) == NULL) {
		printf("Can't open /tmp/.ipt\n");
		exit(1);
	}

	if (prefix)
		fprintf(fp, "%s ", prefix);
	vfprintf(fp, fmt, args);
	putc('\n', fp);
	fclose(fp);
}

static void save2file(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	va_save2file(NULL, fmt, args);
	va_end(args);
}

static void save2file_A_prerouting(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	va_save2file("-A PREROUTING", fmt, args);
	va_end(args);
}

static void save2file_A_postrouting(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	va_save2file("-A POSTROUTING", fmt, args);
	va_end(args);
}

static void save2file_I_postrouting(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	va_save2file("-I POSTROUTING", fmt, args);
	va_end(args);
}

static void save2file_A_forward(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	va_save2file("-A FORWARD", fmt, args);
	va_end(args);
}

static void save2file_I_forward(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	va_save2file("-I FORWARD", fmt, args);
	va_end(args);
}

static void save2file_A_input(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	va_save2file("-A INPUT", fmt, args);
	va_end(args);
}

static void save2file_A(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	va_save2file("-A", fmt, args);
	va_end(args);
}

static int isstandalone(char *name)
{
	return (nvram_nmatch("0", "%s_bridged", name) || isbridge(name)) ? 1 : 0;
}

#if 0
#define DEBUG printf
#else
#define DEBUG(format, args...)
#endif

#define IPTABLES_RULE_STAT	"/tmp/.rule"

/****************** Below is for 'filter' command *******************/

/*
 * update_bitmap:
 *
 * Update bitmap file for activative rule when we insert/delete
 * rule. This file is for tracking the status of filter setting.
 *
 * PARAM - mode 0 : delete
 *                              1 : insert
 *
 * RETURN - The rule order.
 *
 * Example:
 *      mode = 1, seq = 7
 *      before = 0,1,1,0,1,0,0,0,1,1,
 *      after  = 0,1,1,0,1,0,1,0,1,1,
 *      return = 3
 */
static int update_bitmap(int mode, int seq)
{
	FILE *fd;
	char buf[100];
	char sep[] = ",";
	char *token;

	int k, i = 1, order = 0;
	int array[100];

#if defined(REVERSE_RULE_ORDER)
	seq = (NR_RULES + 1) - seq;
#endif
	/*
	 * Read active-rule bitmap 
	 */
	if ((fd = fopen(IPTABLES_RULE_STAT, "r")) == NULL) {
		cprintf("Can't open %s\n", IPTABLES_RULE_STAT);
		exit(1);
	}
	fgets(buf, sizeof(buf), fd);

	token = strtok(buf, sep);
	while (token != NULL) {
		if (*token != '0' && *token != '1')
			break;

		array[i] = atoi(token);

		if (i < seq)
			order += array[i];
		i++;
		token = strtok(NULL, sep);
	}

	fclose(fd);

	/*
	 * Modify setting 
	 */
	if (mode == 1) {	/* add */
//              if (array[seq] == 1)
//                      return -1;
		array[seq] = 1;
	} else {		/* delete */
//              if (array[seq] == 0)
//                      return -1;
		array[seq] = 0;
	}

	/*
	 * Write back active-rule bitmap 
	 */
	if ((fd = fopen(IPTABLES_RULE_STAT, "w")) == NULL) {
		cprintf("Can't open %s\n", IPTABLES_RULE_STAT);
		exit(1);
	}
	for (k = 1; k < i; k++)
		fprintf(fd, "%d,", array[k]);

	fclose(fd);

	return order;
}

static int ip2cclass(char *ipaddr, char *new, int count)
{
	int ip[4];

	if (sscanf(ipaddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) != 4)
		return 0;

	return snprintf(new, count, "%d.%d.%d.", ip[0], ip[1], ip[2]);
}

static void parse_port_forward(char *wanaddr, char *lan_cclass, char *wordlist, int dmzenable)
{
	char var[256], *next;
	char buff[256], ip2[16];
	int flag_dis = 0;
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
	char *wan_proto = nvram_safe_get("wan_proto");
	char *wan_iface = nvram_safe_get("wan_ifname");
	if (!*wan_iface)
		wan_iface = nvram_safe_get("wan_ifname2");
#endif

	/*
	 * name:enable:proto:port>ip name:enable:proto:port>ip 
	 */
	foreach(var, wordlist, next) {
		GETENTRYBYIDX(name, var, 0);
		GETENTRYBYIDX(enable, var, 1);
		GETENTRYBYIDX(proto, var, 2);
		GETENTRYBYIDX(from, var, 3);
		GETENTRYBYIDX(to, var, 4);
		GETENTRYBYIDX(ip, var, 5);
		if (!name || !enable || !proto || !from || !to || !ip)
			continue;

		/*
		 * skip if it's disabled 
		 */
		if (strcmp(enable, "off") == 0)
			flag_dis = 1;
		else
			flag_dis = 0;

		/*
		 * Sveasoft fix for old single number format 
		 */
		if (!sv_valid_ipaddr(ip) && sv_valid_range(ip, 1, 254)) {
			snprintf(ip2, 15, "%s%s", lan_cclass, ip);
			ip = ip2;
		}

		/*
		 * -A PREROUTING -i eth1 -p tcp --dport 8899:88 -j DNAT
		 * --to-destination 192.168.1.88:0 -A PREROUTING -i eth1 -p tcp -m
		 * tcp --dport 9955:99 -j DNAT --to-destination 192.168.1.99:0 
		 */
		if (!strcmp(proto, "tcp") || !strcmp(proto, "both")) {
			bzero(buff, sizeof(buff));

			if (flag_dis == 0) {
				save2file_A_prerouting("-p tcp -d %s --dport %s:%s -j DNAT --to-destination %s", wanaddr, from, to, ip);
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
				if (!strcmp(wan_proto, "pppoe_dual") || (!strcmp(wan_proto, "pptp") && nvram_matchi("wan_dualaccess", 1)) || (!strcmp(wan_proto, "l2tp") && nvram_matchi("wan_dualaccess", 1))
				    )
					save2file_A_prerouting("-i %s -p tcp --dport %s:%s -j DNAT --to-destination %s", wan_iface, from, to, ip);
#endif
				snprintf(buff, sizeof(buff), "-A FORWARD -p tcp -m tcp -d %s --dport %s:%s -j %s\n", ip, from, to, log_accept);
			} else {
				if ((!dmzenable)
				    || (dmzenable && strcmp(ip, nvram_safe_get("dmz_ipaddr")))) {
					snprintf(buff, sizeof(buff), "-A FORWARD -p tcp -m tcp -d %s --sport %s:%s -j %s\n", ip, from, to, log_drop);
				}
			}

			count += strlen(buff) + 1;
			suspense = realloc(suspense, count);
			strcat(suspense, buff);
		}

		if (!strcmp(proto, "udp") || !strcmp(proto, "both")) {
			bzero(buff, sizeof(buff));
			if (flag_dis == 0) {
				save2file_A_prerouting("-p udp -d %s --dport %s:%s -j DNAT --to-destination %s", wanaddr, from, to, ip);
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
				if (!strcmp(wan_proto, "pppoe_dual") || (!strcmp(wan_proto, "pptp") && nvram_matchi("wan_dualaccess", 1)) || (!strcmp(wan_proto, "l2tp") && nvram_matchi("wan_dualaccess", 1))
				    )
					save2file_A_prerouting("-i %s -p udp -m udp --dport %s:%s -j DNAT --to-destination %s", wan_iface, from, to, ip);
#endif
				snprintf(buff, sizeof(buff), "-A FORWARD -p udp -m udp -d %s --dport %s:%s -j %s\n", ip, from, to, log_accept);
			} else {
				if ((!dmzenable)
				    || (dmzenable && strcmp(ip, nvram_safe_get("dmz_ipaddr")))) {
					snprintf(buff, sizeof(buff), "-A FORWARD -p udp -m udp -d %s --dport %s:%s -j %s\n", ip, from, to, log_drop);

				}
			}
			count += strlen(buff) + 1;
			suspense = realloc(suspense, count);
			strcat(suspense, buff);
		}
	}
}

#ifdef HAVE_UPNP
static void parse_upnp_forward(char *wanface, char *wanaddr, char *lan_cclass)
{
	char name[32];		// = "forward_portXXXXXXXXXX";
	char value[1000];
	char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1, *proto;
	char *enable, *desc;
	char buff[256];
	int i;

	if (nvram_invmatchi("upnp_enable", 1))
		return;

	if (nvram_matchi("upnp_clear", 1)) {	// tofu10
		nvram_unset("upnp_clear");
		for (i = 0; i < 1000; ++i) {
			sprintf(name, "forward_port%d", i);
			nvram_unset(name);
		}
		nvram_seti("forward_cur", 0);
		return;
	}

	/*
	 * Set
	 * wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1,proto,enable,desc 
	 */
	for (i = 0; i < 50; i++) {
		snprintf(name, sizeof(name), "forward_port%d", i);

		strncpy(value, nvram_safe_get(name), sizeof(value));

		/*
		 * Check for LAN IP address specification 
		 */
		lan_ipaddr = value;
		wan_port0 = strsep(&lan_ipaddr, ">");
		if (!lan_ipaddr)
			continue;

		/*
		 * Check for LAN destination port specification 
		 */
		lan_port0 = lan_ipaddr;
		lan_ipaddr = strsep(&lan_port0, ":");
		if (!lan_port0)
			continue;

		/*
		 * Check for protocol specification 
		 */
		proto = lan_port0;
		lan_port0 = strsep(&proto, ":,");
		if (!proto)
			continue;

		/*
		 * Check for enable specification 
		 */
		enable = proto;
		proto = strsep(&enable, ":,");
		if (!enable)
			continue;

		/*
		 * Check for description specification (optional) 
		 */
		desc = enable;
		enable = strsep(&desc, ":,");

		/*
		 * Check for WAN destination port range (optional) 
		 */
		wan_port1 = wan_port0;
		wan_port0 = strsep(&wan_port1, "-");
		if (!wan_port1)
			wan_port1 = wan_port0;

		/*
		 * Check for LAN destination port range (optional) 
		 */
		lan_port1 = lan_port0;
		lan_port0 = strsep(&lan_port1, "-");
		if (!lan_port1)
			lan_port1 = lan_port0;

		/*
		 * skip if it's disabled 
		 */
		if (strcmp(enable, "off") == 0)
			continue;

		/*
		 * skip if it's illegal ip 
		 */
		if (get_single_ip(lan_ipaddr, 3) == 0 || get_single_ip(lan_ipaddr, 3) == 255)
			continue;

		/*
		 * -A PREROUTING -p tcp --dport 823 -j DNAT --to-destination
		 * 192.168.1.88:23 
		 */
		if (!*wanface) {
			wanface = "br0";
		}

		if (!strcmp(proto, "tcp") || !strcmp(proto, "both")) {
			save2file_A_prerouting("-i %s -p tcp -d %s --dport %s -j DNAT --to-destination %s%d:%s", wanface, wanaddr, wan_port0, lan_cclass, get_single_ip(lan_ipaddr, 3), lan_port0);

			snprintf(buff, sizeof(buff), "-A FORWARD -p tcp -m tcp -d %s%d --dport %s -j %s\n", lan_cclass, get_single_ip(lan_ipaddr, 3), lan_port0, log_accept);

			count += strlen(buff) + 1;
			suspense = realloc(suspense, count);
			strcat(suspense, buff);
		}
		if (!strcmp(proto, "udp") || !strcmp(proto, "both")) {
			save2file_A_prerouting("-i %s -p udp -d %s --dport %s -j DNAT --to-destination %s%d:%s", wanface, wanaddr, wan_port0, lan_cclass, get_single_ip(lan_ipaddr, 3), lan_port0);

			snprintf(buff, sizeof(buff), "-A FORWARD -p udp -m udp -d %s%d --dport %s -j %s\n", lan_cclass, get_single_ip(lan_ipaddr, 3), lan_port0, log_accept);

			count += strlen(buff) + 1;
			suspense = realloc(suspense, count);
			strcat(suspense, buff);
		}
	}
}
#endif
static void create_spec_forward(char *proto, char *src, char *wanaddr, char *from, char *ip, char *to)
{
	char buff[256];
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
	char *wan_proto = nvram_safe_get("wan_proto");
	char *wan_iface = nvram_safe_get("wan_ifname");
	if (!*wan_iface)
		wan_iface = nvram_safe_get("wan_ifname2");
#endif
	if (src && *src) {
		save2file_A_prerouting("-p %s -m %s -s %s -d %s --dport %s -j DNAT --to-destination %s:%s", proto, proto, src, wanaddr, from, ip, to);
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
		if (!strcmp(wan_proto, "pppoe_dual") || (!strcmp(wan_proto, "pptp") && nvram_matchi("wan_dualaccess", 1)) || (!strcmp(wan_proto, "l2tp") && nvram_matchi("wan_dualaccess", 1))
		    )
			save2file_A_prerouting("-i %s -p %s -m %s --dport %s -j DNAT --to-destination %s:%s", wan_iface, proto, proto, from, ip, to);
#endif
		if (!strcmp(nvram_safe_get("lan_ipaddr"), ip)) {
			snprintf(buff, sizeof(buff), "-I INPUT -p %s -m %s -s %s -d %s --dport %s -j %s\n", proto, proto, src, ip, to, log_accept);
		} else {
			snprintf(buff, sizeof(buff), "-A FORWARD -p %s -m %s -s %s -d %s --dport %s -j %s\n", proto, proto, src, ip, to, log_accept);
		}
	} else {
		save2file_A_prerouting("-p %s -m %s -d %s --dport %s -j DNAT --to-destination %s:%s", proto, proto, wanaddr, from, ip, to);
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
		if (!strcmp(wan_proto, "pppoe_dual") || (!strcmp(wan_proto, "pptp") && nvram_matchi("wan_dualaccess", 1)) || (!strcmp(wan_proto, "l2tp") && nvram_matchi("wan_dualaccess", 1))
		    )
			save2file_A_prerouting("-i %s -p %s -m %s --dport %s -j DNAT --to-destination %s:%s", wan_iface, proto, proto, from, ip, to);
#endif
		if (!strcmp(nvram_safe_get("lan_ipaddr"), ip)) {
			snprintf(buff, sizeof(buff), "-I INPUT -p %s -m %s -d %s --dport %s -j %s\n", proto, proto, ip, to, log_accept);
		} else {
			snprintf(buff, sizeof(buff), "-A FORWARD -p %s -m %s -d %s --dport %s -j %s\n", proto, proto, ip, to, log_accept);
		}
	}
	count += strlen(buff) + 1;
	suspense = realloc(suspense, count);
	strcat(suspense, buff);

}

static void parse_spec_forward(char *wanaddr, char *wordlist)
{
	char var[256], *next;
	char buff[256];

	/*
	 * name:enable:proto:ext_port>ip:int_port
	 * name:enable:proto:ext_port>ip:int_port 
	 */
	foreach(var, wordlist, next) {
		GETENTRYBYIDX(name, var, 0);
		GETENTRYBYIDX(enable, var, 1);
		GETENTRYBYIDX(proto, var, 2);
		GETENTRYBYIDX(from, var, 3);
		GETENTRYBYIDX(ip, var, 4);
		GETENTRYBYIDX(to, var, 5);
		GETENTRYBYIDX(src, var, 6);
		if (!name || !enable || !proto || !from || !to || !ip)
			continue;
		// cprintf("%s %s %s %s %s\n",enable,proto,from,ip,to);

		/*
		 * skip if it's disabled 
		 */
		if (strcmp(enable, "off") == 0)
			continue;

		/*
		 * -A PREROUTING -i eth1 -p tcp -d 192.168.88.11 --dport 823
		 * -j DNAT --to-destination 192.168.1.88:23 
		 */

		if (!strcmp(proto, "tcp") || !strcmp(proto, "both")) {
			create_spec_forward("tcp", src, wanaddr, from, ip, to);
		}
		if (!strcmp(proto, "udp") || !strcmp(proto, "both")) {
			create_spec_forward("udp", src, wanaddr, from, ip, to);
		}
	}
}

static void nat_prerouting_bridged(char *wanface, char *vifs)
{
	char var[256], *wordlist, *next;
#ifdef HAVE_TOR
	if (nvram_matchi("tor_enable", 1)) {
		if (nvram_matchi("tor_transparent", 1)) {
			save2file_A_prerouting("-i %s -p udp --dport 53 -j DNAT --to %s:5353", "br0", nvram_safe_get("lan_ipaddr"));
			save2file_A_prerouting("-i %s -p udp --dport 5353 -j DNAT --to %s:5353", "br0", nvram_safe_get("lan_ipaddr"));
			save2file_A_prerouting("-i %s -p tcp --syn -j DNAT --to %s:9040", "br0", nvram_safe_get("lan_ipaddr"));
		}

		char vif_ip[32];
		foreach(var, vifs, next) {
			if ((!wanface || strcmp(wanface, var))
			    && strcmp(nvram_safe_get("lan_ifname"), var)) {
				if (nvram_nmatch("1", "%s_tor", var) && isstandalone(var)) {
					save2file_A_prerouting("-i %s -p udp --dport 53 -j DNAT --to %s:5353", var, nvram_safe_get("lan_ipaddr"));
					save2file_A_prerouting("-i %s -p udp --dport 5353 -j DNAT --to %s:5353", var, nvram_safe_get("lan_ipaddr"));
					save2file_A_prerouting("-i %s -p tcp --syn -j DNAT --to %s:9040", var, nvram_safe_get("lan_ipaddr"));
				}
			}
		}
	}
#endif
	if (nvram_matchi("dns_redirect", 1) && nvram_matchi("dnsmasq_enable", 1)) {
		save2file_A_prerouting("-i %s -p udp --dport 53 -j DNAT --to %s", nvram_safe_get("lan_ifname"), nvram_safe_get("lan_ipaddr"));
		save2file_A_prerouting("-i %s -p tcp --dport 53 -j DNAT --to %s", nvram_safe_get("lan_ifname"), nvram_safe_get("lan_ipaddr"));
	}
	foreach(var, vifs, next) {
		if ((!wanface || strcmp(wanface, var))
		    && strcmp(nvram_safe_get("lan_ifname"), var)) {
			if (nvram_nmatch("1", "%s_dns_redirect", var)) {
				char *target = nvram_nget("%s_dns_ipaddr", var);
				if (*target && sv_valid_ipaddr(target) && strcmp(target, "0.0.0.0")) {
					save2file_A_prerouting("-i %s -p udp --dport 53 -j DNAT --to %s", var, target);
					save2file_A_prerouting("-i %s -p tcp --dport 53 -j DNAT --to %s", var, target);
				} else {
					save2file_A_prerouting("-i %s -p udp --dport 53 -j DNAT --to %s", var, nvram_safe_get("lan_ipaddr"));
					save2file_A_prerouting("-i %s -p tcp --dport 53 -j DNAT --to %s", var, nvram_safe_get("lan_ipaddr"));
				}
			}
		}
	}

}

static void nat_prerouting(char *wanface, char *wanaddr, char *lan_cclass, int dmzenable, int remotessh, int remotetelnet, int remotemanage, char *vifs)
{
	char var[256], *wordlist, *next;
	char from[100], to[100];
	char *remote_ip_any = nvram_default_get("remote_ip_any", "1");
	char *remote_ip = nvram_default_get("remote_ip", "0.0.0.0 0");
	char *lan_ip = nvram_safe_get("lan_ipaddr");
	int remote_any = 0;

	if (!strcmp(remote_ip_any, "1") || !strncmp(remote_ip, "0.0.0.0", 7))
		remote_any = 1;

	nat_prerouting_bridged(wanface, vifs);

	/*
	 * Block ads on all http requests
	 */
#ifdef HAVE_PRIVOXY
	if (nvram_matchi("privoxy_transp_enable", 1) && nvram_matchi("privoxy_enable", 1)) {

		char vif_ip[32];
		foreach(var, vifs, next) {
			if ((!wanface || strcmp(wanface, var))
			    && strcmp(nvram_safe_get("lan_ifname"), var)) {
				if (nvram_nmatch("1", "%s_isolation", var)) {
					save2file_A_prerouting("-i %s -d %s/%s -j RETURN", var, lan_ip, nvram_safe_get("lan_netmask"));
					sprintf(vif_ip, "%s_ipaddr", var);
					save2file_A_prerouting("-i %s -d %s -j RETURN", var, nvram_safe_get(vif_ip));
				}
			}
		}

		/* no gui setting yet - redirect all except this IP */
		if (*(nvram_safe_get("privoxy_transp_exclude"))) {
			save2file_A_prerouting("-p tcp -s %s --dport 80 -j ACCEPT", nvram_safe_get("privoxy_transp_exclude"));
		}
		/* block access from privoxy to webif */
		save2file_A_prerouting("-p tcp -s %s -d %s --dport %d -j DROP", lan_ip, lan_ip, web_lanport);
		/* do not filter access to the webif from lan */
		save2file_A_prerouting("-p tcp -s %s/%s -d %s --dport %d -j ACCEPT", lan_ip, nvram_safe_get("lan_netmask"), lan_ip, web_lanport);
		/* go through proxy */
		save2file_A_prerouting("-p tcp -d ! %s --dport 80 -j DNAT --to %s:8118", wanaddr, lan_ip);
	}
#endif

	/*
	 * Enable remote Web GUI management 
	 */
	char tmp[1024];
	if (remotemanage) {
		if (remote_any) {
			save2file_A_prerouting("-p tcp -d %s --dport %s -j DNAT --to-destination %s:%d", wanaddr, nvram_safe_get("http_wanport"), lan_ip, web_lanport);
		} else {
			sscanf(remote_ip, "%s %s", from, to);
			wordlist = range(from, get_complete_ip(from, to), tmp);

			foreach(var, wordlist, next) {
				save2file_A_prerouting("-p tcp -s %s -d %s --dport %s -j DNAT --to-destination %s:%d", var, wanaddr, nvram_safe_get("http_wanport"), lan_ip, web_lanport);
			}
		}
	}
#ifdef HAVE_SSHD
	/*
	 * Enable remote ssh management : Botho 03-05-2006 
	 */
	if (remotessh) {
		if (remote_any) {
			save2file_A_prerouting("-p tcp -d %s --dport %s -j DNAT --to-destination %s:%s", wanaddr, nvram_safe_get("sshd_wanport"), lan_ip, nvram_safe_get("sshd_port"));
		} else {
			sscanf(remote_ip, "%s %s", from, to);

			wordlist = range(from, get_complete_ip(from, to), tmp);

			foreach(var, wordlist, next) {
				save2file_A_prerouting("-p tcp -s %s -d %s --dport %s -j DNAT --to-destination %s:%s", var, wanaddr, nvram_safe_get("sshd_wanport"), lan_ip, nvram_safe_get("sshd_port"));
			}
		}
	}
#endif

#ifdef HAVE_TELNET
	/*
	 * Enable remote telnet management 
	 */
	if (remotetelnet) {
		if (remote_any) {
			save2file_A_prerouting("-p tcp -d %s --dport %s -j DNAT --to-destination %s:23", wanaddr, nvram_safe_get("telnet_wanport"), lan_ip);
		} else {
			sscanf(remote_ip, "%s %s", from, to);

			wordlist = range(from, get_complete_ip(from, to), tmp);

			foreach(var, wordlist, next) {
				save2file_A_prerouting("-p tcp -s %s -d %s --dport %s -j DNAT --to-destination %s:23", var, wanaddr, nvram_safe_get("telnet_wanport"), lan_ip);
			}
		}
	}
#endif

	/*
	 * ICMP packets are always redirected to INPUT chains 
	 */
	save2file_A_prerouting("-p icmp -d %s -j DNAT --to-destination %s", wanaddr, lan_ip);

#ifdef HAVE_TFTP
	/*
	 * Enable remote upgrade 
	 */
	if (nvram_matchi("remote_upgrade", 1))
		save2file_A_prerouting("-p udp -d %s --dport %d -j DNAT --to-destination %s", wanaddr, TFTP_PORT, lan_ip);
#endif

	/*
	 * Initiate suspense string for parse_port_forward() 
	 */
	suspense = malloc(1);
	*suspense = 0;
	count = 1;

	if (has_gateway()) {
		writeprocsysnet("netfilter/nf_conntrack_helper", "1");	// kerne 4.7 uses 0 as new default which disables several nat helpers

		/*
		 * Port forwarding 
		 */
#ifdef HAVE_UPNP
		parse_upnp_forward(wanface, wanaddr, lan_cclass);
#endif
		parse_spec_forward(wanaddr, nvram_safe_get("forward_spec"));
		parse_port_forward(wanaddr, lan_cclass, nvram_safe_get("forward_port"), dmzenable);
		/*
		 * DD-WRT addition by Eric Sauvageau 
		 */
		save2file_A_prerouting("-d %s -j TRIGGER --trigger-type dnat", wanaddr);
		/*
		 * DD-WRT addition end 
		 */
	}

	/*
	 * DMZ forwarding 
	 */
	if (dmzenable)
		save2file_A_prerouting("-d %s -j DNAT --to-destination %s%s", wanaddr, lan_cclass, nvram_safe_get("dmz_ipaddr"));

}

static void nat_postrouting(char *wanface, char *wanaddr, char *vifs)
{
	char word[80], *tmp;
	if (has_gateway()) {

		// added for logic test
		int loopmask = 0;
		char *nmask = nvram_safe_get("lan_netmask");	// assuming
		loopmask = getmask(nmask);

		// if (strlen (wanface) > 0)
		// save2file
		// ("-A POSTROUTING -p udp -o %s --sport 5060:5070 -j
		// MASQUERADE "
		// "--to-ports 5056-5071\n", wanface);
		if (nvram_matchi("dtag_vlan8", 1) && nvram_matchi("wan_vdsl", 1)) {
			save2file_A_postrouting("-o %s -j SNAT --to-source %s", nvram_safe_get("tvnicfrom"), nvram_safe_get("tvnicaddr"));
		}
		if (*wanface && wanactive(wanaddr)
		    && !nvram_matchi("br0_nat", 0))
			save2file_A_postrouting("-s %s/%d -o %s -j SNAT --to-source %s", nvram_safe_get("lan_ipaddr"), loopmask, wanface, wanaddr);
		char *sr = nvram_safe_get("static_route");
		foreach(word, sr, tmp) {
			GETENTRYBYIDX(ipaddr, word, 0);
			GETENTRYBYIDX(netmask, word, 1);
			GETENTRYBYIDX(nat, word, 5);
			if (nat && !strcmp(nat, "1")) {
				save2file_A_postrouting("-s %s/%d -o %s -j SNAT --to-source %s", ipaddr, getmask(netmask), wanface, wanaddr);
			}

		}

		char *wan_ifname_tun = nvram_safe_get("wan_ifname");
		if (isClient()) {
			wan_ifname_tun = getSTA();
		}
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
		char *wan_proto = nvram_safe_get("wan_proto");
		if (!strcmp(wan_proto, "pppoe_dual") || (!strcmp(wan_proto, "pptp") && nvram_matchi("wan_dualaccess", 1)) || (!strcmp(wan_proto, "l2tp") && nvram_matchi("wan_dualaccess", 1))) {
			struct in_addr ifaddr;
			osl_ifaddr(wan_ifname_tun, &ifaddr);
			save2file_A_postrouting("-o %s -j SNAT --to-source %s", wan_ifname_tun, inet_ntoa(ifaddr));
		}
#endif
/*
		if (nvram_match("wan_proto", "pptp")) {
			struct in_addr ifaddr;
			osl_ifaddr(nvram_safe_get("pptp_ifname"), &ifaddr);
			save2file_A_postrouting("-o %s -j SNAT --to-source %s", nvram_safe_get("pptp_ifname"), inet_ntoa(ifaddr));
		}
		if (nvram_match("wan_proto", "l2tp")) {
			struct in_addr ifaddr;
			osl_ifaddr(wan_ifname_tun, &ifaddr);
			save2file_A_postrouting("-o %s -j SNAT --to-source %s", wan_ifname_tun, inet_ntoa(ifaddr));
		}
		if (nvram_match("wan_proto", "pppoe_dual")) {
			struct in_addr ifaddr;
			osl_ifaddr(wan_ifname_tun, &ifaddr);
			save2file_A_postrouting("-o %s -j SNAT --to-source %s", wan_ifname_tun, inet_ntoa(ifaddr));
		}
*/

		char *next;
		char dev[16];
		char var[80];

		// char *vifs = nvram_safe_get ("lan_ifnames");
		// if (vifs != NULL)
		foreach(var, vifs, next) {
			if (strcmp(wanface, var)
			    && strcmp(nvram_safe_get("lan_ifname"), var)) {
				if (isstandalone(var)) {

					char nat[32];
					sprintf(nat, "%s_nat", var);
					nvram_default_get(nat, "1");
					if (nvram_matchi(nat, 1)) {
						save2file_A_postrouting("-s %s/%d -o %s -j SNAT --to-source %s", nvram_nget("%s_ipaddr", var), getmask(nvram_nget("%s_netmask", var)), wanface, wanaddr);
					}

				}
			}
		}
		if (nvram_matchi("block_loopback", 0) || nvram_match("filter", "off")) {
			save2file_A_postrouting("-m mark --mark %s -j MASQUERADE", get_NFServiceMark("FORWARD", 1));
		}
		if (nvram_matchi("block_loopback", 0) || nvram_match("filter", "off"))
			writeprocsysnet("ipv4/conf/br0/loop", "1");

//              if (!nvram_match("wan_proto", "pptp") && !nvram_match("wan_proto", "l2tp") && nvram_matchi("wshaper_enable", 0)) {
		//eval("iptables", "-t", "raw", "-A", "PREROUTING", "-p", "tcp", "-j", "CT", "--helper", "ddtb");       //this speeds up networking alot on slow systems 
		//eval("iptables", "-t", "raw", "-A", "PREROUTING", "-p", "udp", "-j", "CT", "--helper", "ddtb");       //this speeds up networking alot on slow systems 
//              }
	} else {
//              if (!nvram_match("wan_proto", "pptp") && !nvram_match("wan_proto", "l2tp") && nvram_matchi("wshaper_enable", 0)) {
		//eval("iptables", "-t", "raw", "-A", "PREROUTING", "-p", "tcp", "-j", "CT", "--helper", "ddtb");       //this speeds up networking alot on slow systems 
		//eval("iptables", "-t", "raw", "-A", "PREROUTING", "-p", "udp", "-j", "CT", "--helper", "ddtb");       //this speeds up networking alot on slow systems 
//              }
#ifdef HAVE_SFE
		if (!nvram_match("sfe", "1"))
#endif
			eval("iptables", "-t", "raw", "-A", "PREROUTING", "-j", "NOTRACK");	//this speeds up networking alot on slow systems 
		/* the following code must be used in future kernel versions, not yet used. we still need to test it */
//              eval("iptables", "-t", "raw", "-A", "PREROUTING", "-j", "CT","--notrack");      //this speeds up networking alot on slow systems 
		if (*wanface && wanactive(wanaddr))
			if (nvram_matchi("wl_br1_enable", 1))
				save2file_A_postrouting("-o %s -j SNAT --to-source %s", wanface, wanaddr);
	}
}

static void parse_port_filter(char *lanface, char *wordlist)
{
	char var[256], *next;

	/*
	 * Parse protocol:lan_port0-lan_port1 ... 
	 */
	foreach(var, wordlist, next) {
		GETENTRYBYIDX(protocol, var, 0);
		GETENTRYBYIDX(lan_port0, var, 1);
		GETENTRYBYIDX(lan_port1, var, 2);

		if (!protocol || !lan_port0 || !lan_port1)
			continue;

		if (!strcmp(protocol, "disable"))
			continue;

		/*
		 * -A FORWARD -i br0 -p tcp --dport 0:655 -j logdrop -A
		 * FORWARD -i br0 -p udp --dport 0:655 -j logdrop 
		 */
		if (!strcmp(protocol, "tcp") || !strcmp(protocol, "both")) {
			save2file_A_forward("-i %s -p tcp --dport %s:%s -j %s", lanface, lan_port0, lan_port1, log_drop);
		}
		if (!strcmp(protocol, "udp") || !strcmp(protocol, "both")) {
			save2file_A_forward("-i %s -p udp --dport %s:%s -j %s", lanface, lan_port0, lan_port1, log_drop);
		}
	}
}

static int match_wday(char *wday)
{
	int wd[7] = { 0, 0, 0, 0, 0, 0, 0 };
	char sep[] = ",";
	char *token;
	int st, end;
	int i;

	token = strtok(wday, sep);
	while (token != NULL) {
		if (sscanf(token, "%d-%d", &st, &end) == 2)
			for (i = st; i <= end; i++)
				wd[i] = 1;
		else
			wd[atoi(token)] = 1;

		token = strtok(NULL, sep);
	}

	DEBUG("week map=%d%d%d%d%d%d%d\n", wd[0], wd[1], wd[2], wd[3], wd[4], wd[5], wd[6]);
	DEBUG("now_wday=%d, match_wday()=%d\n", now_wday, wd[now_wday]);
	return wd[now_wday];
}

static int match_hrmin(int hr_st, int mi_st, int hr_end, int mi_end)
{
	unsigned int hm_st, hm_end;

	/*
	 * convert into %d%2d format 
	 */
	hm_st = hr_st * 100 + mi_st;
	hm_end = hr_end * 100 + mi_end;

	if (hm_st < hm_end) {
		if (now_hrmin < hm_st || now_hrmin > hm_end)
			return 0;
	} else {		// time rotate
		if (now_hrmin < hm_st && now_hrmin > hm_end)
			return 0;
	}

	return 1;
}

/*
 * PARAM - seq : Seqence number.
 *
 * RETURN - 0 : Data error or be disabled until in scheduled time.
 *                      1 : Enabled.
 */
static int schedule_by_tod(FILE * cfd, int seq)
{
	char *todvalue;
	int sched = 0, allday = 0;
	int hr_st, hr_end;	/* hour */
	int mi_st, mi_end;	/* minute */
	char wday[128];
	int intime = 0;

	/*
	 * Get the NVRAM data 
	 */
	todvalue = nvram_nget("filter_tod%d", seq);

	if (strcmp(todvalue, "") == 0)
		return 0;

	/*
	 * Is it anytime or scheduled ? 
	 */
	if (strcmp(todvalue, "0:0 23:59 0-0") == 0 || strcmp(todvalue, "0:0 23:59 0-6") == 0) {
		sched = 0;
	} else {
		sched = 1;
		if (strcmp(todvalue, "0:0 23:59") == 0)
			allday = 1;
		if (sscanf(todvalue, "%d:%d %d:%d %s", &hr_st, &mi_st, &hr_end, &mi_end, wday) != 5)
			return 0;	/* error format */
	}

	DEBUG("sched=%d, allday=%d\n", sched, allday);
	/*
	 * Anytime 
	 */
	if (!sched) {
		save2file_A("lan2wan -j grp_%d", seq);
		return 1;
	}

	/*
	 * Scheduled 
	 */
	if (allday) {		/* 24-hour, but not everyday */
		char wday_st[64], wday_end[64];	/* for crontab */
		int rotate = 0;	/* wday continugoue */
		char sep[] = ",";	/* wday seperate character */
		char *token;
		int st, end;

		/*
		 * If its format looks like as "0-1,3,5-6" 
		 */
		if (*wday == '0')
			if (*(wday + strlen(wday) - 1) == '6')
				rotate = 1;

		/*
		 * Parse the 'wday' format for crontab 
		 */
		token = strtok(wday, sep);
		while (token != NULL) {
			/*
			 * which type of 'wday' ? 
			 */
			if (sscanf(token, "%d-%d", &st, &end) != 2)
				st = end = atoi(token);

			if (rotate == 1 && st == 0)
				sprintf(wday_end + strlen(wday_end), ",%d", end);
			else if (rotate == 1 && end == 6)
				sprintf(wday_st + strlen(wday_st), ",%d", st);
			else {
				sprintf(wday_st + strlen(wday_st), ",%d", st);
				sprintf(wday_end + strlen(wday_end), ",%d", end);
			}

			token = strtok(NULL, sep);
		}

		/*
		 * Write to crontab for triggering the event 
		 */
		/*
		 * "wday_xx + 1" can ignor the first character ',' 
		 */
		fprintf(cfd, "%02d %2d * * %s root /sbin/filter add %d\n", mi_st, hr_st, wday_st + 1, seq);
		fprintf(cfd, "%02d %2d * * %s root /sbin/filter del %d\n", mi_end, hr_end, wday_end + 1, seq);
		if (match_wday(wday))
			intime = 1;
	} else {		/* Nither 24-hour, nor everyday */
		/*
		 * Write to crontab for triggering the event 
		 */
		fprintf(cfd, "%02d %2d * * %s root /sbin/filter add %d\n", mi_st, hr_st, wday, seq);
		fprintf(cfd, "%02d %2d * * %s root /sbin/filter del %d\n", mi_end, hr_end, wday, seq);
		if (match_wday(wday)
		    && match_hrmin(hr_st, mi_st, hr_end, mi_end))
			intime = 1;
	}

	/*
	 * Would it be enabled now ? 
	 */
	DEBUG("intime=%d\n", intime);
	if (intime) {
		save2file_A("lan2wan -j grp_%d", seq);
		return 1;
	}

	return 0;
}

static void macgrp_chain(int seq, int urlenable, char *iflist, char *target)
{
	char var[256], *next;
	char *wordlist;
	wordlist = nvram_nget("filter_mac_grp%d", seq);
	if (strcmp(wordlist, "") == 0)
		return;

	insmod("ipt_mac xt_mac");

	foreach(var, wordlist, next) {

		char ifname[32];
		char *nextif;
		if (iflist) {
			foreach(ifname, iflist, nextif) {
				save2file_A("grp_%d -i %s -m mac --mac-source %s -j %s", seq, ifname, var, target);
				save2file_A("grp_%d -i %s -m mac --mac-destination %s -j %s", seq, ifname, var, target);
			}
		} else {
			save2file_A("grp_%d -m mac --mac-source %s -j %s", seq, var, target);
			save2file_A("grp_%d -m mac --mac-destination %s -j %s", seq, var, target);

		}
	}
}

static void ipgrp_chain(char *lan_cclass, int seq, int urlenable, char *iflist, char *target)
{
	char buf[256];
	char var1[256], *wordlist1, *next1;
	char var2[256], *wordlist2, *next2;
	char from[100], to[100];
	char tmp[1024];
	int a1 = 0, a2 = 0;
	char s1[32], s2[32];

	wordlist1 = nvram_nget("filter_ip_grp%d", seq);
	if (strcmp(wordlist1, "") == 0)
		return;

	foreach(var1, wordlist1, next1) {
		if (strchr(var1, '-')) {
			char *end = var1;
			char *start = strsep(&end, "-");
			if (start && (!strchr(start, '.') || !strchr(end, '.'))) {
				if (atoi(start) == 0 && atoi(end) == 0)
					continue;
				//convert old style 
				char newstart[32];
				sprintf(newstart, "%s%s", lan_cclass, start);
				start = newstart;
				char newend[32];
				sprintf(newend, "%s%s", lan_cclass, end);
				end = newend;
			}
			if (!strcmp(start, "0.0.0.0")
			    && !strcmp(end, "0.0.0.0"))
				continue;
			// if(a1 == 0) /* from 1 */
			// a1 = 1;

			snprintf(from, sizeof(from), "%s", start);
			snprintf(to, sizeof(to), "%s", end);
			/*
			 * The return value of range() is global string array 
			 */
			wordlist2 = range(from, to, tmp);
		} else if (sscanf(var1, "%d", &a1) == 1) {
			if (a1 == 0)	/* unset */
				continue;

			snprintf(buf, sizeof(buf), "%s%d", lan_cclass, a1);
			wordlist2 = buf;
		} else
			continue;

		DEBUG("range=%s\n", wordlist2);

		foreach(var2, wordlist2, next2) {
			char ifname[32];
			char *nextif;
			if (iflist) {
				foreach(ifname, iflist, nextif) {
					save2file_A("grp_%d -i %s -s %s -j %s", seq, ifname, var2, target);
					save2file_A("grp_%d -i %s -d %s -j %s", seq, ifname, var2, target);
				}

			} else {
				save2file_A("grp_%d -s %s -j %s", seq, var2, target);
				save2file_A("grp_%d -d %s -j %s", seq, var2, target);
			}
		}
	}
}

static void portgrp_chain(int seq, int urlenable, char *iflist, char *target)
{
	char var[256], *next;
	char *wordlist;

	wordlist = nvram_nget("filter_dport_grp%d", seq);
	if (strcmp(wordlist, "") == 0)
		return;

	/*
	 * Parse protocol:lan_port0-lan_port1 ... 
	 */
	foreach(var, wordlist, next) {
		GETENTRYBYIDX(protocol, var, 0);
		GETENTRYBYIDX(lan_port0, var, 1);
		GETENTRYBYIDX(lan_port1, var, 2);
		if (!protocol || !lan_port0 || !lan_port1)
			continue;

		if (!strcmp(protocol, "disable"))
			continue;

		char ifname[32];
		char *nextif;
		if (iflist) {
			foreach(ifname, iflist, nextif) {
				if (!strcmp(protocol, "tcp") || !strcmp(protocol, "both")) {
					save2file_A("grp_%d -i %s -p tcp --dport %s:%s -j %s", seq, ifname, lan_port0, lan_port1, target);
				}
				if (!strcmp(protocol, "udp") || !strcmp(protocol, "both")) {
					save2file_A("grp_%d -i %s -p udp --dport %s:%s -j %s", seq, ifname, lan_port0, lan_port1, target);
				}
			}

		} else {
			/*
			 * -A grp_* -p tcp --dport 0:655 -j logdrop -A grp_* -p udp -m 
			 * udp --dport 0:655 -j logdrop 
			 */
			if (!strcmp(protocol, "tcp") || !strcmp(protocol, "both")) {
				save2file_A("grp_%d -p tcp --dport %s:%s -j %s", seq, lan_port0, lan_port1, target);
			}
			if (!strcmp(protocol, "udp") || !strcmp(protocol, "both")) {
				save2file_A("grp_%d -p udp --dport %s:%s -j %s", seq, lan_port0, lan_port1, target);
			}
		}
	}
}

char *fw_get_filter_services(void)
{

	l7filters *filters = filters_list;
	char temp[128] = "";
	char *proto[] = { "l7", "p2p", "dpi" };
	char *services = NULL;

	while (filters->name)	// add l7 and p2p filters
	{
		sprintf(temp, "$NAME:%03d:%s$PROT:%03d:%s$PORT:003:0:0<&nbsp;>", strlen(filters->name), filters->name, filters->protocol == 0 ? 2 : 3, proto[filters->protocol]);
		if (!services) {
			services = malloc(strlen(temp) + 1);
			services[0] = 0;
		} else
			services = realloc(services, strlen(services) + strlen(temp) + 1);
		strcat(services, temp);
		filters++;
	}
	services = realloc(services, strlen(services) + strlen(nvram_safe_get("filter_services")) + 1);
	strcat(services, nvram_safe_get("filter_services"));	// this is
	// user
	// defined
	// filters
	services = realloc(services, strlen(services) + strlen(nvram_safe_get("filter_services_1")) + 1);
	strcat(services, nvram_safe_get("filter_services_1"));

	return services;
}

/*
 * char * fw_get_filter_services (void) { static char services[8192] = "",
 * svcs_var[31] = "filter_services0"; int index = 0;
 * 
 * while (strlen (nvram_safe_get (svcs_var)) > 0 && index < 8) { snprintf
 * (svcs_var, 31, "filter_services%d", index); strcat (services,
 * nvram_safe_get (svcs_var)); index++;
 * 
 * 
 * }
 * 
 * return services; } 
 */
static void advgrp_chain(int seq, int urlenable, char *ifname)
{
	char *wordlist, word[1024], *next;
	char *services, srv[1024], *next2;
	char delim[] = "<&nbsp;>";

	cprintf("add advgrp_chain\n");

	/*
	 * filter_services=$NAME:006:My
	 * ICQ$PROT:002:17$PORT:009:5000:5010<&nbsp;>.. 
	 */
	// services = fw_get_filter_services ();
	// //nvram_safe_get("filter_services");

	services = fw_get_filter_services();

	/*
	 * filter_port_grp5=My ICQ<&nbsp;>Game boy 
	 */
	wordlist = nvram_nget("filter_port_grp%d", seq);
	split(word, wordlist, next, delim) {

		split(srv, services, next2, delim) {
			int len = 0;
			char *name, *prot, *port;
			char protocol[100], ports[100], realname[100];

			if ((name = strstr(srv, "$NAME:")) == NULL || (prot = strstr(srv, "$PROT:")) == NULL || (port = strstr(srv, "$PORT:")) == NULL)
				continue;

			/*
			 * $NAME 
			 */
			if (sscanf(name, "$NAME:%3d:", &len) != 1 || strlen(word) != len)
				continue;
			if (memcmp(name + sizeof("$NAME:nnn:") - 1, word, len)
			    != 0)
				continue;

			strncpy(realname, name + sizeof("$NAME:nnn:") - 1, len);
			realname[len] = '\0';

			/*
			 * $PROT 
			 */
			if (sscanf(prot, "$PROT:%3d:", &len) != 1)
				continue;
			strncpy(protocol, prot + sizeof("$PROT:nnn:") - 1, len);
			protocol[len] = '\0';

			/*
			 * $PORT 
			 */
			if (sscanf(port, "$PORT:%3d:", &len) != 1)
				continue;
			strncpy(ports, port + sizeof("$PORT:nnn:") - 1, len);
			ports[len] = '\0';

			cprintf("match:: name=%s, protocol=%s, ports=%s\n", word, protocol, ports);
			if (!strcmp(protocol, "tcp")
			    || !strcmp(protocol, "both"))
				save2file_A("advgrp_%d -p tcp --dport %s -j %s", seq, ports, log_drop);
			if (!strcmp(protocol, "udp")
			    || !strcmp(protocol, "both"))
				save2file_A("advgrp_%d -p udp --dport %s -j %s", seq, ports, log_drop);
			if (!strcmp(protocol, "icmp"))
				save2file_A("advgrp_%d -p icmp -j %s", seq, log_drop);
			if (!strcmp(protocol, "l7")) {
				int i;

				for (i = 0; i < strlen(realname); i++)
					realname[i] = tolower(realname[i]);
				insmod("ipt_layer7 xt_layer7");

				save2file_A("advgrp_%d -m layer7 --l7proto %s -j %s", seq, realname, log_drop);
			}
#ifdef HAVE_OPENDPI
			if (!strcmp(protocol, "dpi")) {
				insmod("xt_ndpi");
				save2file_A("advgrp_%d -m ndpi --%s -j %s", seq, realname, log_drop);
			}
#endif
			if (!strcmp(protocol, "p2p")) {
				char *proto = NULL;
				/*commonly used protocols, decending */
				if (!strcasecmp(realname, "bittorrent"))
					proto = "bit";
				if (!strcasecmp(realname, "edonkey"))
					proto = "edk";
				if (!strcasecmp(realname, "gnutella"))
					proto = "gnu";
				if (!strcasecmp(realname, "ares"))
					proto = "ares";
				/*atm rarly used protocols */
				if (!strcasecmp(realname, "applejuice"))
					proto = "apple";
				if (!strcasecmp(realname, "bearshare"))
					proto = "gnu";
				if (!strcasecmp(realname, "directconnect"))
					proto = "dc";
				if (!strcasecmp(realname, "kazaa"))
					proto = "kazaa";
				if (!strcasecmp(realname, "mute"))
					proto = "mute";
				if (!strcasecmp(realname, "soulseek"))
					proto = "soul";
				if (!strcasecmp(realname, "waste"))
					proto = "waste";
				if (!strcasecmp(realname, "winmx"))
					proto = "winmx";
				if (!strcasecmp(realname, "xdcc"))
					proto = "xdcc";
				if (proto)	//avoid null pointer, if realname isnt matched
				{
					insmod("ipt_ipp2p xt_ipp2p");
					save2file_A("advgrp_%d -m ipp2p --%s -j %s", seq, proto, log_drop);
					if (!strcmp(proto, "bit")) {
						/* bittorrent detection enhanced */
#ifdef HAVE_OPENDPI
						insmod("xt_ndpi");
						save2file_A("advgrp_%d -m ndpi --bittorrent -j %s", seq, log_drop);
#else
						insmod("ipt_layer7 xt_layer7");
#ifdef HAVE_MICRO
						save2file_A("advgrp_%d -m layer7 --l7proto bt -j %s", seq, log_drop);
#else
						save2file_A("advgrp_%d -m length --length 0:550 -m layer7 --l7proto bt -j %s", seq, log_drop);
#endif
						save2file_A("advgrp_%d -m layer7 --l7proto bt1 -j %s", seq, log_drop);
						save2file_A("advgrp_%d -m layer7 --l7proto bt2 -j %s", seq, log_drop);
#endif
					}
				}
			}

		}
	}
	/*
	 * p2p catchall 
	 */
	if (nvram_nmatch("1", "filter_p2p_grp%d", seq)) {
		insmod("ipt_layer7 xt_layer7 ipt_ipp2p xt_ipp2p");
		save2file_A("advgrp_%d -m ipp2p --edk -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --dc -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --gnu -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --kazaa -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --bit -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --apple -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --soul -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --winmx -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --ares -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --mute -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --waste -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ipp2p --xdcc -j %s", seq, log_drop);

		/* p2p detection enhanced */
#ifdef HAVE_OPENDPI
		insmod("xt_ndpi");
		/*commonly used protocols, decending */
		save2file_A("advgrp_%d -m ndpi --bittorrent -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ndpi --edonkey -j %s", seq, log_drop);
		/*atm rarly used protocols */
		save2file_A("advgrp_%d -p tcp -m ndpi --applejuice -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m ndpi --directconnect -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ndpi --fasttrack -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m ndpi --filetopia -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ndpi --gnutella -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ndpi --imesh -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m ndpi --openft -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m ndpi --pando_media_booster -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m ndpi --soulseek -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m ndpi --winmx -j %s", seq, log_drop);
#else
#ifdef HAVE_MICRO
		save2file_A("advgrp_%d -m layer7 --l7proto bt -j %s", seq, log_drop);
#else
		save2file_A("advgrp_%d -m length --length 0:550 -m layer7 --l7proto bt -j %s", seq, log_drop);
#endif
#endif
		/* commonly used protocols, decending */
/*		save2file_A("advgrp_%d -m layer7 --l7proto bt -j %s", seq, log_drop); */
		save2file_A("advgrp_%d  -p tcp -m layer7 --l7proto ares -j %s", seq, log_drop);
#ifndef HAVE_OPENDPI
		save2file_A("advgrp_%d -m layer7 --l7proto bt4 -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto bt1 -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto bittorrent -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto bt2 -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m layer7 --l7proto gnutella -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto applejuice -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m layer7 --l7proto directconnect -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m layer7 --l7proto soulseek -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m layer7 --l7proto openft -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto fasttrack -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto imesh -j %s", seq, log_drop);
#endif
		save2file_A("advgrp_%d -m layer7 --l7proto audiogalaxy -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m layer7 --l7proto bearshare -j %s", seq, log_drop);
		/* atm rarly used protocols */
		save2file_A("advgrp_%d -m layer7 --l7proto edonkey -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto freenet -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto gnucleuslan -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto goboogy -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto hotline -j %s", seq, log_drop);
/*	 	save2file_A("advgrp_%d -m layer7 --l7proto kugoo -j %s", seq, log_drop);// xunlei, kugoo, winmx block websurfing */
		save2file_A("advgrp_%d -p tcp -m layer7 --l7proto mute -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m layer7 --l7proto napster -j %s", seq, log_drop);
		save2file_A("advgrp_%d -p tcp -m layer7 --l7proto soribada -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto tesla -j %s", seq, log_drop);
/*		save2file_A("advgrp_%d -p tcp -m layer7 --l7proto winmx -j %s", seq, log_drop);
		save2file_A("advgrp_%d -m layer7 --l7proto xunlei -j %s", seq, log_drop); */
	}
	free(services);
	/*
	 * filter_web_host2=hello<&nbsp;>world<&nbsp;>friend 
	 */
	wordlist = nvram_nget("filter_web_host%d", seq);
	if (wordlist && strcmp(wordlist, "")) {
		insmod("ipt_webstr");
		save2file_A("advgrp_%d -p tcp -m webstr --host \"%s\" -j %s", seq, wordlist, log_reject);
#if !defined(ARCH_broadcom) && !defined(HAVE_RTG32) || defined(HAVE_BCMMODERN)
		char var[256];
		char *next;
		char *pos = wordlist;
		int len = strlen(wordlist);
		while ((wordlist - pos) < len && (next = strstr(wordlist, "<&nbsp;>"))) {
			bzero(var, 256);
			strncpy(var, wordlist, (next - wordlist));
			int offset = 0;
			if (strstr(var, "https://"))
				offset = 8;
			save2file_A("advgrp_%d -p tcp -m string --string \"%s\" --algo bm --from 1 --to 600 -j %s", seq, &var[offset], log_reject);
			wordlist = next + 8;
		}

#endif

	}
	/*
	 * filter_web_url3=hello<&nbsp;>world<&nbsp;>friend 
	 */
	wordlist = nvram_nget("filter_web_url%d", seq);
	if (wordlist && strcmp(wordlist, "")) {
		insmod("ipt_webstr");
		save2file_A("advgrp_%d -p tcp -m webstr --url \"%s\" -j %s", seq, wordlist, log_reject);
#if !defined(ARCH_broadcom) || defined(HAVE_BCMMODERN) && !defined(HAVE_RTG32)
		char var[256];
		char *next;
		char *pos = wordlist;
		int len = strlen(wordlist);
		while ((wordlist - pos) < len && (next = strstr(wordlist, "<&nbsp;>"))) {
			bzero(var, 256);
			strncpy(var, wordlist, (next - wordlist));
			int offset = 0;
			if (strstr(var, "https://"))
				offset = 8;
			save2file_A("advgrp_%d -p tcp -m string --string \"%s\" --algo bm --from 1 --to 600 -j %s", seq, &var[offset], log_reject);
			wordlist = next + 8;
		}
#endif

	}
	/*
	 * Others will be accepted 
	 */
	// save2file ("-A advgrp_%d -j %s", seq, log_accept);
}

static void lan2wan_chains(char *lan_cclass)
{
	time_t ct;		/* Calendar time */
	struct tm *bt;		/* Broken time */
	int seq;
	char buf[] = "filter_rulexxx";
	char *data;
	unsigned int offset = 0, len = 0;
	unsigned int mark = 0;
	int up = 0;
	int urlfilter = 1;
	FILE *cfd;
	FILE *ifd;
	// char urlhost[] ="filter_url_hostxxx";
	// char urlkeywd[]="filter_url_keywdxxx";

	/*
	 * Get local calendar time 
	 */
	time(&ct);
	bt = localtime(&ct);

	/*
	 * Convert to 3-digital format 
	 */
	now_hrmin = bt->tm_hour * 100 + bt->tm_min;
	now_wday = bt->tm_wday;

	/*
	 * keep the status using bitmap 
	 */
	if ((ifd = fopen(IPTABLES_RULE_STAT, "w")) == NULL) {
		cprintf("Can't open %s\n", IPTABLES_RULE_STAT);
		exit(1);
	}

	/*
	 * Open the crontab file for modification 
	 */
	if ((cfd = fopen(CRONTAB, "w")) == NULL) {
		cprintf("Can't open %s\n", CRONTAB);
		exit(1);
	}
	// fprintf (cfd, "PATH=/sbin:/bin:/usr/sbin:/usr/bin\n\n");

#if defined(REVERSE_RULE_ORDER)
	for (seq = NR_RULES; seq >= 1; seq--) {
#else
	for (seq = 1; seq <= NR_RULES; seq++) {
#endif
		data = nvram_nget("filter_rule%d", seq);

		if (strcmp(data, "") == 0)
			up = 0;
		else
			up = schedule_by_tod(cfd, seq);

		fprintf(ifd, "%d,", up);
	}

	fclose(cfd);
	fclose(ifd);

	for (seq = 1; seq <= NR_RULES; seq++) {
		data = nvram_nget("filter_rule%d", seq);
		if (data && strcmp(data, "") == 0)
			continue;

		/*
		 * Check if it is enabled 
		 */
		char ifs[40];
		char *iflist = NULL;
		find_match_pattern(ifs, sizeof(ifs), data, "$IF:", "");	// get 
		find_match_pattern(buf, sizeof(buf), data, "$STAT:", "");	// get 

		if (!strcmp(buf, ""))
			continue;	/* error format */
		if (*ifs) {
			if (strcmp(ifs, "Any"))
				iflist = ifs;
		}
		DEBUG("STAT: %s\n", buf);
		switch (atoi(buf)) {
		case 1:	/* Drop it */
			mark = MARK_DROP;
			break;
		case 2:	/* URL checking */
			mark = MARK_OFFSET + seq;
			break;
		default:	/* jump to next iteration */
			continue;
		}

		/*
		 * sprintf(urlhost, "filter_url_host%d", seq); sprintf(urlkeywd,
		 * "filter_url_keywd%d", seq); if (nvram_match(urlhost, "") &&
		 * nvram_match(urlkeywd, "")) urlfilter = 0;
		 * 
		 * DEBUG("host=%s, keywd=%s\n", urlhost, urlkeywd); 
		 */
		char target[100];
		if (mark == MARK_DROP)
			strncpy(target, log_drop, sizeof(log_drop));
		else
			sprintf(target, "advgrp_%d", seq);

		macgrp_chain(seq, urlfilter, iflist, target);
		ipgrp_chain(lan_cclass, seq, urlfilter, iflist, target);
		portgrp_chain(seq, urlfilter, iflist, target);
		advgrp_chain(seq, urlfilter, iflist);
	}
}

#ifndef HAVE_MICRO
#include <pthread.h>
static pthread_mutex_t mutex_unl = PTHREAD_MUTEX_INITIALIZER;
static char *lastlock;
static char *lastunlock;
#define lock() pthread_mutex_lock(&mutex_unl)
#define unlock() pthread_mutex_unlock(&mutex_unl)
#else
#define lock()
#define unlock()
#endif

/*
 *
 * mode 0 : delete
 *              1 : insert
 */
static int update_filter(int mode, int seq)
{
	char target_ip[20];
	char order[10];
	int ord;
	lock();
	ord = update_bitmap(mode, seq);
	sprintf(target_ip, "grp_%d", seq);
	sprintf(order, "%d", ord * 1 + 1);
	/*
	 * iptables -t mangle -I lan2wan 3 -j macgrp_9 
	 */
	if (mode == 1) {	/* insert */
		eval("iptables", "-D", "lan2wan", "-j", target_ip);
		eval("iptables", "-I", "lan2wan", "-j", target_ip);
	} else {		/* delete */
		eval("iptables", "-D", "lan2wan", "-j", target_ip);
	}
	unlock();
	cprintf("done\n");
	return 0;
}

int filter_main(int argc, char *argv[])
{

	if (argc > 2) {
		int num = 0;
		if ((num = atoi(argv[2])) > 0) {
			if (strcmp(argv[1], "add") == 0) {
				update_filter(1, num);
				goto out;
			} else if (strcmp(argv[1], "del") == 0) {
				update_filter(0, num);
				goto out;
			}
		}
	} else {
		fprintf(stderr, "usage: filter [add|del] number\n");
		return -1;
	}
      out:;
	return 0;
}

/*
 * PARAM - seq : Seqence number.
 *
 * RETURN - 0 : It's not in time.
 *                      1 : in time and anytime
 *                      2 : in time
 */
int if_tod_intime(int seq)
{
	char *todvalue;
	int sched = 0, allday = 0;
	int hr_st, hr_end;	/* hour */
	int mi_st, mi_end;	/* minute */
	char wday[128];
	int intime = 0;
	/*
	 * Get the NVRAM data 
	 */
	todvalue = nvram_nget("filter_tod%d", seq);
	DEBUG("%s: %s\n", todname, todvalue);
	if (strcmp(todvalue, "") == 0)
		return 0;
	/*
	 * Is it anytime or scheduled ? 
	 */
	if (strcmp(todvalue, "0:0 23:59 0-0") == 0 || strcmp(todvalue, "0:0 23:59 0-6") == 0) {
		sched = 0;
	} else {
		sched = 1;
		if (strcmp(todvalue, "0:0 23:59") == 0)
			allday = 1;
		if (sscanf(todvalue, "%d:%d %d:%d %s", &hr_st, &mi_st, &hr_end, &mi_end, wday) != 5)
			return 0;	/* error format */
	}

	DEBUG("sched=%d, allday=%d\n", sched, allday);
	/*
	 * Anytime 
	 */
	if (!sched)
		return 1;
	/*
	 * Scheduled 
	 */
	if (allday) {		/* 24-hour, but not everyday */

		if (match_wday(wday))
			intime = 1;
	} else {		/* Nither 24-hour, nor everyday */

		if (match_wday(wday)
		    && match_hrmin(hr_st, mi_st, hr_end, mi_end))
			intime = 1;
	}
	DEBUG("intime=%d\n", intime);
	/*
	 * Would it be enabled now ? 
	 */
	if (intime)
		return 2;
	return 0;
}

int filtersync_main(int argc, char *argv[])
{
	time_t ct;		/* Calendar time */
	struct tm *bt;		/* Broken time */
	int seq;
	int ret;
	/*
	 * Get local calendar time 
	 */
	time(&ct);
	bt = localtime(&ct);
	/*
	 * Convert to 3-digital format 
	 */
	now_hrmin = bt->tm_hour * 100 + bt->tm_min;
	now_wday = bt->tm_wday;
	for (seq = 1; seq <= NR_RULES; seq++) {
		if (if_tod_intime(seq) > 0)
			update_filter(1, seq);
		else
			update_filter(0, seq);
		DEBUG("seq=%d, ret=%d\n", seq, ret);
	}
	return 0;
}

static void parse_trigger_out(char *wordlist)
{
	char var[256], *next;
	/*
	 * port_trigger=name:[on|off]:[tcp|udp|both]:wport0-wport1>lport0-lport1 
	 */
	foreach(var, wordlist, next) {
		GETENTRYBYIDX(name, var, 0);
		GETENTRYBYIDX(enable, var, 1);
		GETENTRYBYIDX(proto, var, 2);
		GETENTRYBYIDX(wport0, var, 3);
		GETENTRYBYIDX(wport1, var, 4);
		GETENTRYBYIDX(lport0, var, 5);
		GETENTRYBYIDX(lport1, var, 6);
		/*
		 * skip if it's disabled 
		 */
		if (strcmp(enable, "off") == 0)
			continue;
		if (!strcmp(proto, "tcp") || !strcmp(proto, "udp")) {
			save2file_A("trigger_out -p %s -m %s --dport %s:%s "
				    "-j TRIGGER --trigger-type out --trigger-proto %s --trigger-match %s-%s --trigger-relate %s-%s", proto, proto, wport0, wport1, proto, wport0, wport1, lport0, lport1);
		} else if (!strcmp(proto, "both")) {
			save2file_A("trigger_out -p tcp --dport %s:%s " "-j TRIGGER --trigger-type out --trigger-proto all --trigger-match %s-%s --trigger-relate %s-%s", wport0, wport1, wport0, wport1, lport0, lport1);
			save2file_A("trigger_out -p udp --dport %s:%s " "-j TRIGGER --trigger-type out --trigger-proto all --trigger-match %s-%s --trigger-relate %s-%s", wport0, wport1, wport0, wport1, lport0, lport1);
		}
	}
}

#ifdef HAVE_VLANTAGGING
static void add_bridges(char *wanface, char *chain, int forward)
{
	char word[256];
	char *next, *wordlist;
	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next) {
		GETENTRYBYIDX(tag, word, 0);
		if (!tag)
			break;
		char ipaddr[32];
		sprintf(ipaddr, "%s_ipaddr", tag);
		char netmask[32];
		sprintf(netmask, "%s_netmask", tag);
		if (ifexists(tag)) {
			if (nvram_exists(ipaddr) && nvram_exists(netmask)
			    && !nvram_match(ipaddr, "0.0.0.0")
			    && !nvram_match(netmask, "0.0.0.0")) {
				eval("ifconfig", tag, nvram_safe_get(ipaddr), "netmask", nvram_safe_get(netmask), "up");
			} else {
				eval("ifconfig", tag, "up");
			}
			if (forward && wanface && *wanface)
				save2file_A_forward("-i %s -o %s -j %s", tag, wanface, log_accept);
			else {
				if (!strcmp(chain, "OUTPUT"))
					save2file_A("%s -o %s -j %s", chain, tag, log_accept);
				else
					save2file_A("%s -i %s -j %s", chain, tag, log_accept);
			}
		}
	}

}

#endif
static void filter_input(char *wanface, char *lanface, char *wanaddr, int remotessh, int remotetelnet, int remotemanage, char *vifs)
{

	char *next, *iflist, buff[16];
	/*
	 * Filter known SPI state 
	 */
	/*
	 * most of what was here has been moved to the end 
	 */
	save2file_A_input("-m state --state RELATED,ESTABLISHED -j %s", log_accept);
	if (nvram_matchi("dtag_vlan8", 1) && nvram_matchi("wan_vdsl", 1)) {
		save2file_A_input("-i %s -j %s", nvram_safe_get("tvnicfrom"), log_accept);
	}
#ifdef HAVE_PPTP
	/*
	 * Impede DoS/Bruteforce, redcuce possilbe bruteforce on pptp server
	 */
#ifndef HAVE_MICRO
	if (nvram_matchi("pptpd_enable", 1)) {
		if (nvram_matchi("limit_pptp", 1)) {
			save2file_A_input("-i %s -p tcp --dport %d -j logbrute", wanface, PPTP_PORT);
			save2file_A_input("-i %s -p tcp --dport %d -j %s", wanface, PPTP_PORT, log_accept);
		} else {
			save2file_A_input("-i %s -p tcp --dport %d -j %s", wanface, PPTP_PORT, log_accept);
		}
	}
#endif
	if (nvram_match("wan_proto", "dhcp") || nvram_match("wan_proto", "dhcp_auth"))
		save2file_A_input("-i %s -p udp --sport 67 --dport 68 -j %s", wanface, log_accept);
	if (nvram_matchi("pptpd_enable", 1)
	    || nvram_matchi("pptpd_client_enable", 1)
	    || nvram_match("wan_proto", "pptp")) {
		save2file_A_input("-p tcp --dport %d -j %s", PPTP_PORT, log_accept);
		save2file_A_input("-p 47 -j %s", log_accept);
		if (nvram_matchi("pptpd_lockdown", 1)) {
			save2file_A_input("-i %s -p udp --sport 67 --dport 68 -j %s", lanface, log_accept);
			save2file_A_input("-i %s -j %s", lanface, log_drop);
		}
	}
#endif
#ifdef HAVE_FTP
	if (nvram_matchi("proftpd_enable", 1)
	    && nvram_matchi("proftpd_wan", 1)) {
		if (nvram_matchi("limit_ftp", 1)) {
			save2file_A_input("-i %s -p tcp --dport %s -j logbrute", wanface, nvram_safe_get("proftpd_port"));
			save2file_A_input("-i %s -p tcp --dport %s -j %s", wanface, nvram_safe_get("proftpd_port"), log_accept);
		} else {
			save2file_A_input("-i %s -p tcp --dport %s -j %s", wanface, nvram_safe_get("proftpd_port"), log_accept);
		}
	}
#endif
#ifdef HAVE_WEBSERVER
	if (nvram_matchi("lighttpd_enable", 1) && nvram_matchi("lighttpd_wan", 1)) {
		save2file_A_input("-i %s -p tcp --dport %s -j %s", wanface, nvram_safe_get("lighttpd_port"), log_accept);
		save2file_A_input("-i %s -p tcp --dport %s -j %s", wanface, nvram_safe_get("lighttpd_sslport"), log_accept);
	}
#endif
/*#ifdef HAVE_AP_SERV
	save2file_A_input("-i %s -p udp --dport 22359 -j ACCEPT\n",lanface);
	save2file_A_input("-i %s -p udp --sport 22359 -j ACCEPT\n",lanface);
	save2file_A("OUTPUT -p udp --sport 22359 -j ACCEPT\n");
#endif*/
	/*
	 * Routing protocol, RIP, accept 
	 */
	/*
	 * lonewolf mods for multiple VLANs / interfaces 
	 */
#ifdef HAVE_OPENVPN
	//check if ovpn server is running
	if (nvram_matchi("openvpn_enable", 1)) {
		save2file_A_input("-p %s --dport %s -j %s", nvram_match("openvpn_proto", "udp") ? "udp" : "tcp", nvram_safe_get("openvpn_port"), log_accept);
		if (nvram_match("openvpn_tuntap", "tun")) {
//                      if (strlen(nvram_safe_get("openvpn_ccddef")) = 0) {
			save2file_A_input("-i %s2 -j %s", nvram_safe_get("openvpn_tuntap"), log_accept);
			save2file_A_forward("-i %s2 -j %s", nvram_safe_get("openvpn_tuntap"), log_accept);
			save2file_A_forward("-o %s2 -j %s", nvram_safe_get("openvpn_tuntap"), log_accept);
//                      }
		}
	}
#endif
	if (wanactive(wanaddr)) {
		if (nvram_invmatchi("dr_wan_rx", 0))
			save2file_A_input("-p udp -i %s --dport %d -j %s", wanface, RIP_PORT, log_accept);
		else
			save2file_A_input("-p udp -i %s --dport %d -j %s", wanface, RIP_PORT, log_drop);
	}
	if (nvram_invmatchi("dr_lan_rx", 0))
		save2file_A_input("-p udp -i %s --dport %d -j %s", lanface, RIP_PORT, log_accept);
	else
		save2file_A_input("-p udp -i %s --dport %d -j %s", lanface, RIP_PORT, log_drop);
	iflist = nvram_safe_get("no_route_if");
	foreach(buff, iflist, next) {
		save2file_A_input("-p udp -i %s --dport %d -j %s", buff, RIP_PORT, log_drop);
	}

	save2file_A_input("-p udp --dport %d -j %s", RIP_PORT, log_accept);
	/*
	 * end lonewolf mods 
	 */
	/*
	 * Wolf mod - accept protocol 41 for IPv6 tunneling 
	 */
#ifdef HAVE_IPV6
	if (nvram_matchi("ipv6_enable", 1))
		save2file_A_input("-p 41 -j %s", log_accept);
#endif
	/*
	 * Sveasoft mod - accept OSPF protocol broadcasts 
	 */
	if (nvram_match("wk_mode", "ospf"))
		save2file_A_input("-p ospf -j %s", log_accept);
	if (nvram_match("wk_mode", "bgp"))
		save2file_A_input("-p tcp --dport 179 -j %s", log_accept);
#ifdef HAVE_OLSRD
	if (nvram_match("wk_mode", "olsr"))
		save2file_A_input("-p udp --dport 698 -j %s", log_accept);
#endif
	/*
	 * Sveasoft mod - default for br1/separate subnet WDS type 
	 */
	if (nvram_matchi("wl0_br1_enable", 1)
	    && nvram_invmatchi("wl0_br1_nat", 1)
	    && nvram_invmatchi("wl0_br1_nat", 2))
		save2file_A_input("-i br1 -j %s", log_accept);
	if (nvram_matchi("wl1_br1_enable", 1)
	    && nvram_invmatchi("wl1_br1_nat", 1)
	    && nvram_invmatchi("wl1_br1_nat", 2))
		save2file_A_input("-i br1 -j %s", log_accept);
#ifdef HAVE_VLANTAGGING
	add_bridges(wanface, "INPUT", 0);
#endif
	/*
	 * Remote Web GUI Management Use interface name, destination address, and 
	 * port to make sure that it's redirected from WAN 
	 */
	if (remotemanage) {
		save2file_A_input("-p tcp -d %s --dport %d -j %s", nvram_safe_get("lan_ipaddr"), web_lanport, log_accept);
	}
#ifdef HAVE_SSHD
	/*
	 * Impede DoS/Bruteforce, reduce load on ssh
	 */
#ifndef HAVE_MICRO
	if (remotessh) {

		if (nvram_matchi("limit_ssh", 1))
			save2file_A_input("-i %s -p tcp -d %s --dport %s -j logbrute", wanface, nvram_safe_get("lan_ipaddr"), nvram_safe_get("sshd_port"));
	}
#endif
	/*
	 * Remote Web GUI Management Botho 03-05-2006 : remote ssh & remote GUI
	 * management are not linked anymore 
	 */
	if (remotessh) {
		save2file_A_input("-i %s -p tcp -d %s --dport %s -j %s", wanface, nvram_safe_get("lan_ipaddr"), nvram_safe_get("sshd_port"), log_accept);
	}
#endif

#ifdef HAVE_TELNET
	/*
	 * Impede DoS/Bruteforce, reduce load on Telnet
	 */
#ifndef HAVE_MICRO
	if (remotetelnet) {
		if (nvram_matchi("limit_telnet", 1))
			save2file_A_input("-i %s -p tcp -d %s --dport 23 -j logbrute", wanface, nvram_safe_get("lan_ipaddr"));
	}
#endif
	if (remotetelnet) {
		save2file_A_input("-i %s -p tcp -d %s --dport 23 -j %s", wanface, nvram_safe_get("lan_ipaddr"), log_accept);
	}
#endif
	/*
	 * ICMP request from WAN interface 
	 */
	if (wanactive(wanaddr)) {
		if (nvram_invmatch("filter", "off"))
			save2file_A_input("-i %s -p icmp -j %s", wanface, nvram_matchi("block_wan", 1) ? log_drop : log_accept);
		else
			save2file_A_input("-i %s -p icmp -j %s", wanface, log_accept);
	}
	/*
	 * IGMP query from WAN interface 
	 */
	save2file_A_input("-p igmp -j %s", doMultiCast() == 0 ? log_drop : log_accept);
#ifdef HAVE_UDPXY
	if (wanactive(wanaddr) && nvram_matchi("udpxy_enable", 1) && nvram_exists("tvnicfrom"))
		save2file_A_input("-i %s -p udp -d %s -j %s", nvram_safe_get("tvnicfrom"), IP_MULTICAST, log_accept);
#endif
#ifndef HAVE_MICRO
	/*
	 * SNMP access from WAN interface 
	 */
	if (nvram_matchi("snmpd_enable", 1) && (nvram_matchi("block_snmp", 0) || nvram_match("filter", "off"))) {
		save2file_A_input("-i %s -p udp --dport 161 -j %s", wanface, log_accept);
	}
#endif

#ifdef HAVE_TFTP
	/*
	 * Remote Upgrade 
	 */
	if (nvram_matchi("remote_upgrade", 1))
		save2file_A_input("-p udp --dport %d -j %s", TFTP_PORT, log_accept);
#endif
#ifdef HAVE_MILKFISH
	if (*wanface && nvram_matchi("milkfish_enabled", 1))
		save2file_A_input("-p udp -i %s --dport 5060 -j %s", wanface, log_accept);
	// save2file ("-A INPUT -m udp -p udp -i %s --dport 35000 36000 -j
	// ACCEPT\n", wanface);
#endif
#ifdef HAVE_VNCREPEATER
	if (nvram_matchi("vncr_enable", 1) && *wanface) {
		save2file_A_input("-p tcp -i %s --dport 5900 -j %s", wanface, log_accept);
		save2file_A_input("-p tcp -i %s --dport 5500 -j %s", wanface, log_accept);
	}
#endif
#ifdef HAVE_TOR
	if (nvram_matchi("tor_enable", 1)) {
		if (nvram_matchi("tor_relay", 1))
			save2file_A_input("-p tcp -i %s --dport 9001 -j %s", wanface, log_accept);
		if (nvram_matchi("tor_dir", 1))
			save2file_A_input("-p tcp -i %s --dport 9030 -j %s", wanface, log_accept);
	}
#endif
	/*
	 * Ident request backs by telnet or IRC server 
	 */
	if (nvram_matchi("block_ident", 0) || nvram_match("filter", "off"))
		save2file_A_input("-p tcp --dport %d -j %s", IDENT_PORT, log_accept);
	/*
	 * Filter known SPI state 
	 */
	// removed first rule: -A INPUT -m state --state INVALID -j DROP
	// (wolfiR)
	save2file_A_input("-i lo -m state --state NEW -j ACCEPT");
	save2file_A_input("-i %s -m state --state NEW -j %s", lanface, log_accept);
	/*
	 * lonewolf mods for extra VLANs / interfaces 
	 */
	iflist = nvram_safe_get("no_firewall_if");
	foreach(buff, iflist, next) {
		save2file_A_input("-i %s -m state --state NEW -j %s", buff, log_accept);
	}
	char var[80];
	// char *vifs = nvram_safe_get ("lan_ifnames");
	// if (vifs != NULL)
	foreach(var, vifs, next) {
		if (strcmp(get_wan_face(), var)
		    && strcmp(nvram_safe_get("lan_ifname"), var)) {
			if (nvram_nmatch("1", "%s_isolation", var)) {
				save2file_A_input("-i %s -p udp --dport 67 -j %s", var, log_accept);
				save2file_A_input("-i %s -p udp --dport 53 -j %s", var, log_accept);
				save2file_A_input("-i %s -p tcp --dport 53 -j %s", var, log_accept);
				save2file_A_input("-i %s -m state --state NEW -j %s", var, log_drop);
			}
			if (isstandalone(var)) {
				save2file_A_input("-i %s -j %s", var, log_accept);
			}

		}
	}

	/*
	 * end lonewolf mods 
	 */

	/*
	 * Drop those packets we are NOT recognizable 
	 */
	save2file_A_input("-j %s", log_drop);
}

void filter_output(char *wanface)
{
	/*
	 * Sveasoft mod - default for br1/separate subnet WDS type 
	 */
	if (nvram_matchi("wl0_br1_enable", 1)
	    && nvram_invmatchi("wl0_br1_nat", 1)
	    && nvram_invmatchi("wl_br1_nat", 2))
		save2file_A("OUTPUT -o br1 -j %s", log_accept);
	if (nvram_matchi("wl1_br1_enable", 1)
	    && nvram_invmatchi("wl1_br1_nat", 1)
	    && nvram_invmatchi("wl_br1_nat", 2))
		save2file_A("OUTPUT -o br1 -j %s", log_accept);
#ifdef HAVE_VLANTAGGING
	add_bridges(wanface, "OUTPUT", 0);
#endif
}

static void filter_forward(char *wanface, char *lanface, char *lan_cclass, int dmzenable, int webfilter, char *vifs)
{
	char *filter_web_hosts, *filter_web_urls, *filter_rule;
	char *next;
	char var[80];
	int i = 0;
	int filter_host_url = 0;
	while (i < 20 && filter_host_url == 0) {
		i++;
		filter_web_hosts = nvram_nget("filter_web_host%d", i);
		filter_web_urls = nvram_nget("filter_web_url%d", i);
		filter_rule = nvram_nget("filter_rule%d", i);
		if ((filter_web_hosts && strcmp(filter_web_hosts, ""))
		    || (filter_web_urls && strcmp(filter_web_urls, ""))
		    || (filter_rule && !strncmp(filter_rule, "$STAT:1", 7))
		    || (filter_rule && !strncmp(filter_rule, "$STAT:2", 7))) {
			filter_host_url = 1;
		}
	}

	if (nvram_matchi("dtag_vlan8", 1) && nvram_matchi("wan_vdsl", 1)) {
		save2file_A_forward("-i %s -j %s", nvram_safe_get("tvnicfrom"), log_accept);
		save2file_A_forward("-o %s -j %s", nvram_safe_get("tvnicfrom"), log_accept);
	}

	/*
	 * Drop the wrong state, INVALID, packets 
	 */
	if (!has_gateway())
		save2file_A_forward("-m state --state INVALID -j %s", log_drop);
	foreach(var, vifs, next) {
		if (strcmp(get_wan_face(), var)
		    && strcmp(nvram_safe_get("lan_ifname"), var)) {
			if (isstandalone(var) && nvram_nmatch("1", "%s_nat", var)) {
				save2file_A_forward("-i %s -j lan2wan", var);
			}
		}
	}

	save2file_A_forward("-j lan2wan");
	/*
	 * Filter Web application 
	 */
	// if (webfilter)
	// save2file ("-A FORWARD -i %s -o %s -p tcp --dport %d "
	// "-m webstr --content %d -j %s",
	// lanface, wanface, HTTP_PORT, webfilter, log_reject);
	if (nvram_invmatch("filter", "off") && webfilter && *wanface) {
		insmod("ipt_webstr");
		save2file_A_forward("-i %s -o %s -p tcp -m webstr --content %d -j %s", lanface, wanface, webfilter, log_reject);
	}

	/*
	 * If webfilter is used this rule must be evaluated after webstr rule
	 */
	if (filter_host_url)
		save2file_A_forward("-m state --state RELATED,ESTABLISHED -j %s", log_accept);
	/*
	 * Accept the redirect, might be seen as INVALID, packets 
	 */
	save2file_A_forward("-i %s -o %s -j %s", lanface, lanface, log_accept);
	/*
	 * Drop all traffic from lan 
	 */
	if (nvram_matchi("pptpd_lockdown", 1))
		save2file_A_forward("-i %s -j %s", lanface, log_drop);
	/*
	 * Filter by destination ports "filter_port" if firewall on 
	 */
	if (nvram_invmatch("filter", "off"))
		parse_port_filter(lanface, nvram_safe_get("filter_port"));
	/*
	 * Sveasoft mods - accept OSPF protocol broadcasts 
	 */
	if (nvram_match("wk_mode", "ospf")) {
		save2file_A_forward("-p ospf -j %s", log_accept);
	}
	if (nvram_match("wk_mode", "bgp")) {
		save2file_A_forward("-p tcp --sport 179 -j %s", log_accept);	// BGP 
		// port
		save2file_A_forward("-p tcp --dport 179 -j %s", log_accept);	// BGP 
		// port
	}
#ifdef HAVE_OLSRD
	if (nvram_match("wk_mode", "olsr")) {
		save2file_A_forward("-p udp --dport 698 -j %s", log_accept);
		save2file_A_forward("-p udp --sport 698 -j %s", log_accept);
	}
#endif

	/*
	 * Sveasoft mod - FORWARD br1 to br0, protecting br0 
	 */
	if (nvram_matchi("wl0_br1_enable", 1)) {

		if (nvram_matchi("wl0_br1_nat", 1)) {
			save2file_A_forward("-i br0 -o br1 -j %s", log_accept);
			save2file_A_forward("-o br0 -i br1 -m state --state ESTABLISHED,RELATED -j %s", log_accept);
		}

		/*
		 * Sveasoft mod - FORWARD br0 to br1, protecting br1 
		 */
		else if (nvram_matchi("wl0_br1_nat", 2)) {
			save2file_A_forward("-o br0 -i br1 -j %s", log_accept);
			save2file_A_forward("-i br0 -o br1 -m state --state ESTABLISHED,RELATED -j %s", log_accept);
		}
		/*
		 * Sveasoft mod - default for br1/separate subnet WDS type 
		 */
		else
			save2file_A_forward("-i br1 -o br0 -j %s", log_accept);
		char *wan = get_wan_face();
		if (wan && *wan)
			save2file_A_forward("-i br1 -o %s -j %s", wan, log_accept);
	}
#ifdef HAVE_VLANTAGGING
	add_bridges(wanface, "FORWARD", 1);
#endif
	// unload_vpn_modules ();
	if (nvram_invmatch("filter", "off") && *wanface) {

		/*
		 * DROP packets for PPTP pass through. 
		 */
		if (nvram_matchi("pptp_pass", 0))
			save2file_A_forward("-o %s -p tcp --dport %d -j %s", wanface, PPTP_PORT, log_drop);
		/*
		 * DROP packets for L2TP pass through. 
		 */
		if (nvram_matchi("l2tp_pass", 0))
			save2file_A_forward("-o %s -p udp --dport %d -j %s", wanface, L2TP_PORT, log_drop);
		/*
		 * DROP packets for IPsec pass through 
		 */
		if (nvram_matchi("ipsec_pass", 0))
			save2file_A_forward("-o %s -p udp --dport %d -j %s", wanface, ISAKMP_PORT, log_drop);
	}
	if (nvram_states(vpn_modules_deps()))
		start_vpn_modules();
	// load_vpn_modules ();
	if (nvram_invmatch("filter", "off")) {
		if (nvram_matchi("pptp_pass", 1)) {
			if (*wanface) {
				save2file_I_forward("-o %s -s %s/%d -p tcp --dport %d -j %s", wanface, nvram_safe_get("lan_ipaddr"), getmask(nvram_safe_get("lan_netmask")), PPTP_PORT, log_accept);
				save2file_I_forward("-o %s -s %s/%d -p gre -j %s", wanface, nvram_safe_get("lan_ipaddr"), getmask(nvram_safe_get("lan_netmask")), log_accept);
			}
		}
	}
	/*
	 * ACCEPT packets for Multicast pass through 
	 */
	if (nvram_matchi("dtag_vlan8", 1) && nvram_matchi("wan_vdsl", 1)) {
		if (doMultiCast() > 0)
			save2file_A_forward("-i %s -p udp --destination %s -j %s", nvram_safe_get("tvnicfrom"), IP_MULTICAST, log_accept);
#ifdef HAVE_PPTP
	} else if (nvram_match("wan_proto", "pptp") && nvram_matchi("pptp_iptv", 1) && nvram_exists("tvnicfrom")) {
		if (doMultiCast() > 0)
			save2file_A_forward("-i %s -p udp --destination %s -j %s", nvram_safe_get("tvnicfrom"), IP_MULTICAST, log_accept);
#endif
#ifdef HAVE_L2TP
	} else if (nvram_match("wan_proto", "l2tp") && nvram_matchi("pptp_iptv", 1) && nvram_exists("tvnicfrom")) {
		if (doMultiCast() > 0)
			save2file_A_forward("-i %s -p udp --destination %s -j %s", nvram_safe_get("tvnicfrom"), IP_MULTICAST, log_accept);
#endif
#ifdef HAVE_PPPOEDUAL
	} else if (nvram_match("wan_proto", "pppoe_dual") && nvram_matchi("pptp_iptv", 1) && nvram_exists("tvnicfrom")) {
		if (doMultiCast() > 0)
			save2file_A_forward("-i %s -p udp --destination %s -j %s", nvram_safe_get("tvnicfrom"), IP_MULTICAST, log_accept);
#endif
	} else {
		if (doMultiCast() > 0 && *wanface)
			save2file_A_forward("-i %s -p udp --destination %s -j %s", wanface, IP_MULTICAST, log_accept);
	}
	/*
	 * port-forwarding accepting rules 
	 */
	if (*suspense != 0)
		save2file("%s", suspense);
	free(suspense);
	/*
	 * Port trigger by user definition 
	 */
	/*
	 * Incoming connection will be accepted, if it match the port-ranges. 
	 */
	if (*wanface) {
		save2file_A_forward("-i %s -o %s -j TRIGGER --trigger-type in", wanface, lanface);
		save2file_A_forward("-i %s -j trigger_out", lanface);
	}
	/*
	 * DMZ forwarding 
	 */
	if (dmzenable)
		save2file_A_forward("-o %s -d %s%s -j %s", lanface, lan_cclass, nvram_safe_get("dmz_ipaddr"), log_accept);
	foreach(var, vifs, next) {
		if (strcmp(get_wan_face(), var)
		    && strcmp(nvram_safe_get("lan_ifname"), var)) {
			if (nvram_nmatch("1", "%s_isolation", var)) {
				save2file_I_forward("-i %s -d %s/%s -m state --state NEW -j %s", var, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"), log_drop);
				save2file_A_forward("-i br0 -o %s -m state --state NEW -j %s", var, log_drop);
				if (nvram_matchi("privoxy_transp_enable", 1)) {
					save2file("-I INPUT -i %s -d %s/%s -p tcp --dport 8118 -j %s", var, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"), log_accept);
				}
			}
			if (*wanface) {
				save2file_A_forward("-i %s -o %s -j TRIGGER --trigger-type in", wanface, var);
				save2file_A_forward("-i %s -j trigger_out", var);
				save2file_A_forward("-i %s -m state --state NEW -j %s", var, log_accept);
			}
		}
	}

	/*
	 * Accept new connections 
	 */
	save2file_A_forward("-i %s -m state --state NEW -j %s", lanface, log_accept);
	/*
	 * ...otherwise drop if firewall on 
	 */
	if (nvram_invmatch("filter", "off"))
		save2file_A_forward("-j %s", log_drop);
	lan2wan_chains(lan_cclass);
	parse_trigger_out(nvram_safe_get("port_trigger"));
	/*
	 * If webfilter is not used we can put this rule on top in order to increase WAN<->LAN throughput
	 */
	if (!filter_host_url)
		save2file_I_forward("-m state --state RELATED,ESTABLISHED -j %s", log_accept);
}

/*
 *      Mangle table
 */
static void mangle_table(char *wanface, char *wanaddr, char *vifs)
{
	save2file("*mangle\n:PREROUTING ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]");
#ifndef HAVE_MICRO
	if (nvram_matchi("wan_priority", 1) && isvlan(wanface)) {
		eval("vconfig", "set_egress_map", wanface, "0", "6");
		eval("vconfig", "set_egress_map", wanface, "1", "0");
		insmod("nf_defrag_ipv6 nf_log_ipv6 ip6_tables nf_conntrack_ipv6 ip6table_filter ip6table_mangle xt_DSCP xt_CLASSIFY");
		save2file_A_prerouting("-i %s -j MARK --set-mark 0x100000", wanface);
		save2file_A_postrouting("-o %s -j MARK --set-mark 0x100000", wanface);
		save2file_A_postrouting("-m mark --mark 0x100000 -j CLASSIFY --set-class 0:1");
		save2file_A_postrouting("-o %s -p udp --dport 67 -j CLASSIFY --set-class 0:0", wanface);

		eval("ip6tables", "-t", "mangle", "-D", "PREROUTING", "-i", wanface, "-j", "MARK", "--set-mark", "0x100000");
		eval("ip6tables", "-t", "mangle", "-A", "PREROUTING", "-i", wanface, "-j", "MARK", "--set-mark", "0x100000");
		eval("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-o", wanface, "-j", "MARK", "--set-mark", "0x100000");
		eval("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-o", wanface, "-j", "MARK", "--set-mark", "0x100000");
		eval("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-m", "mark", "--mark", "0x100000", "-j", "CLASSIFY", "--set-class", "0:1");
		eval("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-m", "mark", "--mark", "0x100000", "-j", "CLASSIFY", "--set-class", "0:1");
		eval("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-o", wanface, "-p", "udp", "--dport", "547", "-j", "CLASSIFY", "--set-class", "0:0");
		eval("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-o", wanface, "-p", "udp", "--dport", "547", "-j", "CLASSIFY", "--set-class", "0:0");

	}
	if (nvram_matchi("filter_tos", 1)) {
		insmod("xt_DSCP");
		if (!nvram_matchi("wan_priority", 1) || !isvlan(wanface)) {
			save2file_A_prerouting("-i %s -j MARK --set-mark 0x100000", wanface);
			save2file_A_postrouting("-o %s -j MARK --set-mark 0x100000", wanface);
		}
		save2file_A_postrouting("-m mark --mark 0x100000 -j TOS --set-tos 0x00");
		eval("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-m", "mark", "--mark", "0x100000", "-j", "TOS", "--set-tos", "0x00");
		eval("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-m", "mark", "--mark", "0x100000", "-j", "TOS", "--set-tos", "0x00");
	}
#endif
	if (strcmp(wanface, "wwan0") && nvram_matchi("wshaper_enable", 0)) {
		if (wanactive(wanaddr) && (nvram_matchi("block_loopback", 0) || nvram_match("filter", "off"))) {
			insmod("ipt_mark xt_mark ipt_CONNMARK xt_CONNMARK xt_connmark");
			save2file_A_prerouting("-i ! %s -d %s -j MARK --set-mark %s", wanface, wanaddr, get_NFServiceMark("FORWARD", 1));
			save2file_A_prerouting("-j CONNMARK --save-mark");
		}
	}
	/*
	 * Clamp TCP MSS to PMTU of WAN interface 
	 */
	save2file_I_forward("-p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu");
#ifdef HAVE_PRIVOXY
	if ((nvram_matchi("privoxy_enable", 1)) && (nvram_matchi("wshaper_enable", 1))) {
		save2file("-I OUTPUT -p tcp --sport 8118 -j IMQ --todev 0");
	}
#endif

	/*
	 * Sveasoft add - avoid the "mark everything" rule, Reformed's PPPoE code 
	 * should take care of this 
	 */
	/*
	 * For PPPoE Connect On Demand, to reset idle timer. add by honor
	 * (2003-04-17) Reference driver/net/ppp_generic.c 
	 */
	// save2file_A_prerouting("-i %s -m mark ! --mark 0 -j MARK --set-mark
	// %d\n", lanface, MARK_LAN2WAN);
	save2file("COMMIT");
}

/*
 *      NAT table
 */
static void nat_table(char *wanface, char *wanaddr, char *lan_cclass, int dmzenable, int remotessh, int remotetelnet, int remotemanage, char *vifs)
{
	save2file("*nat\n:PREROUTING ACCEPT [0:0]\n:POSTROUTING ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]");
	if (wanactive(wanaddr)) {
		nat_prerouting(wanface, wanaddr, lan_cclass, dmzenable, remotessh, remotetelnet, remotemanage, vifs);
		nat_postrouting(wanface, wanaddr, vifs);
	} else {
		nat_prerouting_bridged(NULL, vifs);
	}
	save2file("COMMIT");
}

/*
 *      Filter table
 */
static void filter_table(char *wanface, char *lanface, char *wanaddr, char *lan_cclass, int dmzenable, int webfilter, int remotessh, int remotetelnet, int remotemanage, char *vifs)
{
	save2file("*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n:logreject - [0:0]\n"
#ifdef FLOOD_PROTECT
		  ":limaccept - [0:0]"
#endif
		  ":trigger_out - [0:0]\n:lan2wan - [0:0]");
	int seq;
	for (seq = 1; seq <= NR_RULES; seq++) {
		save2file(":grp_%d - [0:0]", seq);
		save2file(":advgrp_%d - [0:0]", seq);
	}
#ifndef HAVE_MICRO
	if ((nvram_matchi("limit_telnet", 1)) || (nvram_matchi("limit_pptp", 1)) || (nvram_matchi("limit_ssh", 1)) || (nvram_matchi("limit_ftp", 1))) {
		save2file(":logbrute - [0:0]");
		save2file_A("logbrute -m recent --set --name BRUTEFORCE --rsource");
		save2file_A("logbrute -m recent ! --update --seconds 60 --hitcount 4 --name BRUTEFORCE --rsource -j RETURN");
		// -m limit rule is a fallback in case -m recent isn't included in a build
		save2file_A("logbrute -m limit --limit 1/min --limit-burst 1 -j RETURN");
		if ((nvram_matchi("log_enable", 1))
		    && (nvram_matchi("log_dropped", 1)))
			save2file_A("logbrute -j LOG --log-prefix \"[DROP BRUTEFORCE] : \" --log-tcp-options --log-ip-options");
		save2file_A("logbrute -j %s", log_drop);
	}
#endif

	if (wanactive(wanaddr)) {
		/*
		 * Does it disable the filter? 
		 */
		if (nvram_match("filter", "off")
		    || !has_gateway()) {

			/*
			 * Make sure remote management ports are filtered if it is disabled 
			 */
			if (!remotemanage && *wanface) {
				save2file_A_input("-p tcp -i %s --dport %s -j %s", wanface, nvram_safe_get("http_wanport"), log_drop);
				save2file_A_input("-p tcp -i %s --dport 80 -j %s", wanface, log_drop);
				save2file_A_input("-p tcp -i %s --dport 443 -j %s", wanface, log_drop);
				save2file_A_input("-p tcp -i %s --dport 69 -j %s", wanface, log_drop);
			}
			/*
			 * Make sure remote ssh/telnet port is filtered if it is disabled :
			 * Botho 03-05-2006 
			 */
#ifdef HAVE_SSHD
			if (!remotessh && *wanface) {
				save2file_A_input("-i %s -p tcp --dport %s -j %s", wanface, nvram_safe_get("sshd_port"), log_drop);
			}
#endif

#ifdef HAVE_TELNET
			if (!remotetelnet && *wanface) {
				save2file_A_input("-p tcp -i %s --dport 23 -j %s", wanface, log_drop);
			}
#endif
			filter_forward(wanface, lanface, lan_cclass, dmzenable, webfilter, vifs);
		} else {

			filter_input(wanface, lanface, wanaddr, remotessh, remotetelnet, remotemanage, vifs);
			filter_output(wanface);
			filter_forward(wanface, lanface, lan_cclass, dmzenable, webfilter, vifs);
		}
	} else {
		char var[80];
		char vifs[256];
		char *next;
		foreach(var, vifs, next) {
			if (strcmp(get_wan_face(), var)
			    && strcmp(nvram_safe_get("lan_ifname"), var)) {
				if (nvram_nmatch("1", "%s_isolation", var)) {
					save2file_A_input("-i %s -p udp --dport 67 -j %s", var, log_accept);
					save2file_A_input("-i %s -p udp --dport 53 -j %s", var, log_accept);
					save2file_A_input("-i %s -p tcp --dport 53 -j %s", var, log_accept);
					save2file_A_input("-i %s -m state --state NEW -j %s", var, log_drop);
				}
				if (isstandalone(var)) {
					save2file_A_input("-i %s -j %s", var, log_accept);
				}

			}
		}

	}

	/*
	 * logaccept chain 
	 */
#ifdef FLOOD_PROTECT
	if ((nvram_matchi("log_enable", 1))
	    && (nvram_matchi("log_accepted", 1)))
		save2file_A("logaccept -i %s -m state --state NEW -m limit --limit %d -j LOG --log-prefix \"FLOOD \" --log-tcp-sequence --log-tcp-options --log-ip-options", wanface, FLOOD_RATE);
	save2file_A("logaccept -i %s -m state --state NEW -m limit --limit %d -j %s", wanface, FLOOD_RATE, log_drop);
#endif
	if ((nvram_matchi("log_enable", 1))
	    && (nvram_matchi("log_accepted", 1)))
		save2file_A("logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" --log-tcp-sequence --log-tcp-options --log-ip-options");
	save2file_A("logaccept -j ACCEPT");
	/*
	 * logdrop chain 
	 */
	if (has_gateway()) {
		if ((nvram_matchi("log_enable", 1))
		    && (nvram_matchi("log_dropped", 1)))
			save2file_A("logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" --log-tcp-sequence --log-tcp-options --log-ip-options");
		save2file_A("logdrop -m state --state INVALID -j LOG --log-prefix \"DROP \" --log-tcp-sequence --log-tcp-options --log-ip-options");
	} else {
		if ((nvram_matchi("log_enable", 1))
		    && (nvram_matchi("log_dropped", 1)))
			save2file_A("logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" --log-tcp-sequence --log-tcp-options --log-ip-options");
	}
	save2file_A("logdrop -j DROP");
	/*
	 * logreject chain 
	 */
	if ((nvram_matchi("log_enable", 1))
	    && (nvram_matchi("log_rejected", 1)))
		save2file_A("logreject -j LOG --log-prefix \"WEBDROP \" --log-tcp-sequence --log-tcp-options --log-ip-options");
	save2file_A("logreject -p tcp -j REJECT --reject-with tcp-reset");
#ifdef FLOOD_PROTECT
	/*
	 * limaccept chain 
	 */
	if ((nvram_matchi("log_enable", 1))
	    && (nvram_matchi("log_accepted", 1)))
		save2file_A("limaccept -i %s -m state --state NEW -m limit --limit %d -j LOG --log-prefix \"FLOOD \" --log-tcp-sequence --log-tcp-options --log-ip-options");
	save2file_A("limaccept -i %s -m state --state NEW -m limit --limit %d -j %s\n-A limaccept -j ACCEPT", wanface, FLOOD_RATE, wanface, FLOOD_RATE, log_drop);
#endif
	save2file("COMMIT");
}

static void create_restore_file(char *wanface, char *lanface, char *wanaddr, char *lan_cclass, int dmzenable, int webfilter, int remotessh, int remotetelnet, int remotemanage, char *vifs)
{
	mangle_table(wanface, wanaddr, vifs);
	nat_table(wanface, wanaddr, lan_cclass, dmzenable, remotessh, remotetelnet, remotemanage, vifs);
	filter_table(wanface, lanface, wanaddr, lan_cclass, dmzenable, webfilter, remotessh, remotetelnet, remotemanage, vifs);
}

#ifdef HAVE_GUESTPORT
void set_gprules(char *iface)
{
	char gport[32];
	char giface[16];
	char gvar[32];
	char gipaddr[32];
	char gnetmask[32];
	sprintf(gport, "guestport_%s", iface);
	if (nvram_exists(gport)) {
		sprintf(giface, nvram_safe_get(gport));
		sprintf(gvar, "%s_ipaddr", giface);
		sprintf(gipaddr, nvram_safe_get(gvar));
		sprintf(gvar, "%s_netmask", giface);
		sprintf(gnetmask, nvram_safe_get(gvar));
		sysprintf("iptables -I INPUT -i %s -d %s/%s -m state --state NEW -j DROP", giface, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
		sysprintf("iptables -I INPUT -i %s -d %s/255.255.255.255 -m state --state NEW -j DROP", giface, gipaddr);
		sysprintf("iptables -I INPUT -i %s -d %s/255.255.255.255 -p udp --dport 67 -j %s", giface, gipaddr, TARG_PASS);
		sysprintf("iptables -I INPUT -i %s -d %s/255.255.255.255 -p udp --dport 53 -j %s", giface, gipaddr, TARG_PASS);
		sysprintf("iptables -I INPUT -i %s -d %s/255.255.255.255 -p tcp --dport 53 -j %s", giface, gipaddr, TARG_PASS);
		sysprintf("iptables -I FORWARD -i %s -m state --state NEW -j %s", giface, TARG_PASS);
		sysprintf("iptables -I FORWARD -i %s -o br0 -m state --state NEW -j DROP", giface);
		sysprintf("iptables -I FORWARD -i br0 -o %s -m state --state NEW -j DROP", giface);
	}
}
#endif

int isregistered_real(void);
#ifdef HAVE_IPV6
void start_firewall6(void)
{

	if (nvram_matchi("ipv6_enable", 0))
		return;
	fprintf(stderr, "start firewall6\n");
	insmod("nf_defrag_ipv6 nf_log_ipv6 ip6_tables nf_conntrack_ipv6 ip6table_filter");
	eval("ip6tables", "-F", "INPUT");
	eval("ip6tables", "-F", "FORWARD");
	eval("ip6tables", "-F", "OUTPUT");
	eval("ip6tables", "-A", "INPUT", "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
	eval("ip6tables", "-A", "INPUT", "-p", "icmpv6", "-j", "ACCEPT");
	eval("ip6tables", "-A", "INPUT", "-s", "fe80::/64", "-j", "ACCEPT");
	eval("ip6tables", "-A", "INPUT", "-i", "br0", "-j", "ACCEPT");
	if (nvram_match("ipv6_typ", "ipv6rd") || nvram_match("ipv6_typ", "ipv6in4") || nvram_match("ipv6_typ", "ipv6to4")) {

		eval("iptables", "-I", "INPUT", "-p", "41", "-j", "ACCEPT");
		eval("ip6tables", "-A", "FORWARD", "-p", "tcp", "-m", "tcp", "--tcp-flags", "SYN,RST", "SYN", "-j", "TCPMSS", "--clamp-mss-to-pmtu");
	}

	/* accept ICMP requests from the remote tunnel endpoint */
	if (nvram_match("ipv6_typ", "ipv6in4")) {
		char *ip = nvram_safe_get("ipv6_tun_end_ipv4");
		if (*ip && strcmp(ip, "0.0.0.0") != 0)
			eval("iptables", "-I", "INPUT", "-p", "icmp", "-s", ip, "-j", "ACCEPT");
	}

	if (nvram_match("ipv6_typ", "ipv6in4") || nvram_match("ipv6_typ", "ipv6native"))
		eval("ip6tables", "-A", "INPUT", "-p", "udp", "--dport", "546", "-j", "ACCEPT");
	eval("ip6tables", "-A", "INPUT", "-j", "DROP");
	eval("ip6tables", "-A", "FORWARD", "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
	if (nvram_match("ipv6_typ", "ipv6native") || nvram_match("ipv6_typ", "ipv6pd")) {
		if (nvram_match("wan_proto", "disabled")) {
			eval("ip6tables", "-A", "FORWARD", "-o", nvram_safe_get("lan_ifname"), "-j", "ACCEPT");
		} else {
			eval("ip6tables", "-A", "FORWARD", "-o", get_wan_face(), "-j", "ACCEPT");
		}
	}

	if (nvram_match("ipv6_typ", "ipv6in4"))
		eval("ip6tables", "-A", "FORWARD", "-o", "ip6tun", "-j", "ACCEPT");
	eval("ip6tables", "-A", "FORWARD", "-p", "icmpv6", "--icmpv6-type", "echo-request", "-m", "limit", "--limit", "2/s", "-j", "ACCEPT");
	eval("ip6tables", "-A", "FORWARD", "-j", "DROP");
}
#endif

void start_loadfwmodules(void)
{
	insmod("iptable_raw iptable_mangle nf_conntrack_h323 xt_NFLOG"
	       " xt_length xt_REDIRECT xt_CT xt_limit xt_TCPMSS"
	       " xt_connbytes xt_connlimit"
	       " xt_CLASSIFY xt_recent"
	       " xt_conntrack xt_state"
	       " xt_string xt_LOG xt_iprange xt_tcpmss" " xt_NETMAP compat_xtables" " ipt_MASQUERADE iptable_filter nf_reject_ipv4" " ipt_REJECT nf_nat_h323" " ipt_TRIGGER nf_nat_masquerade_ipv4 ipt_ah");
}

int client_bridged_enabled(void);

#ifdef DEVELOPE_ENV
int main(void)
#else
void start_firewall(void)
#endif
{
	DIR *dir;
	struct dirent *file;
	FILE *fp;
	char name[NAME_MAX];
	struct stat statbuff;
	int log_level = 0;
	char wanface[IFNAMSIZ];
	char lanface[IFNAMSIZ];
	char lan_cclass[] = "xxx.xxx.xxx.";
	char wanaddr[] = "xxx.xxx.xxx.xxx";
	int dmzenable;
	int remotessh = 0;
	int remotetelnet = 0;
	int remotemanage = 0;
	lock();
#ifdef HAVE_REGISTER
#ifndef HAVE_ERC
	if (isregistered_real())
#endif
#endif
	{
		runStartup(".prewall");
	}
	start_loadfwmodules();
	system("cat /proc/net/ip_conntrack_flush 2>&1");
	system("cat /proc/sys/net/netfilter/nf_conntrack_flush 2>&1");
	if (nvram_matchi("arp_spoofing", 1))
		writeproc("/proc/net/arp_spoofing_enable", "1");
	else
		writeproc("/proc/net/arp_spoofing_enable", "0");
	writeint("/sys/fast_classifier/skip_to_bridge_ingress", 1);
#ifndef	HAVE_80211AC
	/*
	 * Improve WAN<->LAN Performance on K26
	 */
	writeprocsysnet("core/netdev_max_backlog", "120");
#endif
	char vifs[256];
	getIfLists(vifs, 256);
	/*
	 * Block obviously spoofed IP addresses 
	 */
	DEBUG("start firewall()...........\n");
#if 0
	if (!(dir = opendir("/proc/sys/net/ipv4/conf")))
		perror("/proc/sys/net/ipv4/conf");
	while (dir && (file = readdir(dir))) {
		if (strncmp(file->d_name, ".", NAME_MAX) != 0 && strncmp(file->d_name, "..", NAME_MAX) != 0) {
			sprintf(name, "/proc/sys/net/ipv4/conf/%s/rp_filter", file->d_name);
			if (!(fp = fopen(name, "r+"))) {
				perror(name);
				break;
			}
			fputc('1', fp);
			fclose(fp);
		}
	}
	closedir(dir);
#endif
	/*
	 * Determine LOG level 
	 */
	DEBUG("start firewall()........1\n");
	log_level = nvram_geti("log_level");
	// sprintf(log_drop, "%s", (log_level & 1) ? "logdrop" : "DROP");
	// sprintf(log_accept, "%s", (log_level & 2) ? "logaccept" : TARG_PASS);
	// sprintf(log_reject, "%s", (log_level & 1) ? "logreject" : TARG_RST);
	if (log_level >= 1)
		sprintf(log_drop, "%s", "logdrop");
	else
		sprintf(log_drop, "%s", "DROP");
	if (log_level >= 2)
		sprintf(log_accept, "%s", "logaccept");
	else
		sprintf(log_accept, "%s", TARG_PASS);
	if (log_level >= 1)
		sprintf(log_reject, "%s", "logreject");
	else
		sprintf(log_reject, "%s", TARG_RST);
	/*
	 * Get NVRAM value into globle variable 
	 */
	DEBUG("start firewall()........2\n");
	strncpy(lanface, nvram_safe_get("lan_ifname"), IFNAMSIZ);
	strncpy(wanface, get_wan_face(), IFNAMSIZ);
	strncpy(wanaddr, get_wan_ipaddr(), sizeof(wanaddr));
	if (nvram_match("wan_proto", "pptp")) {
		if (!*(nvram_safe_get("pptp_get_ip")))	// for initial dhcp ip
		{
			if (getSTA())
				strncpy(wanface, getSTA(), IFNAMSIZ);
			else
				strncpy(wanface, nvram_safe_get("wan_ifname"), IFNAMSIZ);
		}
	}

	ip2cclass(nvram_safe_get("lan_ipaddr"), &lan_cclass[0], sizeof(lan_cclass));
	/*
	 * Run Webfilter ? 
	 */
	int webfilter = 0;	/* Reset, clear the late setting */
	if (nvram_matchi("block_cookie", 1))
		webfilter |= BLK_COOKIE;
	if (nvram_matchi("block_java", 1))
		webfilter |= BLK_JAVA;
	if (nvram_matchi("block_activex", 1))
		webfilter |= BLK_ACTIVE;
	if (nvram_matchi("block_proxy", 1))
		webfilter |= BLK_PROXY;
	/*
	 * Run DMZ forwarding ? 
	 */
	if (has_gateway()
	    && nvram_matchi("dmz_enable", 1)
	    && nvram_invmatch("dmz_ipaddr", "")
	    && nvram_invmatchi("dmz_ipaddr", 0))
		dmzenable = 1;
	else
		dmzenable = 0;
	/*
	 * Remote Web GUI management 
	 */
	if (nvram_matchi("remote_management", 1) && nvram_invmatch("http_wanport", "") && nvram_invmatchi("http_wanport", 0))
		remotemanage = 1;
	else
		remotemanage = 0;
#ifdef HAVE_SSHD
	/*
	 * Remote ssh management : Botho 03-05-2006 
	 */
	if (nvram_matchi("remote_mgt_ssh", 1) && nvram_invmatch("sshd_wanport", "") && nvram_invmatchi("sshd_wanport", 0) && nvram_matchi("sshd_enable", 1))
		remotessh = 1;
	else
		remotessh = 0;
#endif
#ifdef HAVE_TELNET
	/*
	 * Remote telnet management 
	 */
	if (nvram_matchi("remote_mgt_telnet", 1) && nvram_invmatch("telnet_wanport", "") && nvram_invmatchi("telnet_wanport", 0) && nvram_matchi("telnetd_enable", 1))
		remotetelnet = 1;
	else
		remotetelnet = 0;
#endif
#ifdef HAVE_HTTPS
	if (nvram_matchi("remote_mgt_https", 1))
		web_lanport = nvram_geti("https_lanport") > 0 ? nvram_geti("https_lanport") : HTTPS_PORT;
	else
#endif
		web_lanport = nvram_geti("http_lanport") > 0 ? nvram_geti("http_lanport") : HTTP_PORT;
	/*
	 * Remove existent file 
	 */
	DEBUG("start firewall()........3\n");
	if (stat(IPTABLES_SAVE_FILE, &statbuff) == 0)
		unlink(IPTABLES_SAVE_FILE);
	/*
	 * Create file for iptables-restore 
	 */
	DEBUG("start firewall()........4\n");

	create_restore_file(wanface, lanface, wanaddr, lan_cclass, dmzenable, webfilter, remotessh, remotetelnet, remotemanage, vifs);
#ifndef DEVELOPE_ENV
	/*
	 * Insert the rules into kernel 
	 */
	DEBUG("start firewall()........5\n");
	eval("iptables-restore", IPTABLES_SAVE_FILE);
	// unlink(IPTABLES_SAVE_FILE);
#endif
#ifdef HAVE_IPV6
	start_firewall6();
#endif
	/*
	 * begin Sveasoft add 
	 */
	/*
	 * run rc_firewall script 
	 */
	cprintf("Exec RC Filewall\n");
	int runfw = 0;
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
	cprintf("Ready\n");
	/*
	 * end Sveasoft add 
	 */
	// run wanup scripts
	start_wanup();
	/*
	 * Turn on the DMZ-LED, if enabled.(from service.c) 
	 */
	cprintf("enable DMZ\n");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	if (dmzenable)
		diag_led(DMZ, START_LED);
	else
		diag_led(DMZ, STOP_LED);
#endif
	cprintf("done");
	cprintf("Start firewall\n");
	/*
	 * We don't forward packet until those policies are set. 
	 */
	DEBUG("start firewall()........6\n");
	if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "r+"))) {
		fputc('1', fp);
		fclose(fp);
	} else
		perror("/proc/sys/net/ipv4/ip_forward");
	cprintf("start ipv6\n");
#ifdef HAVE_IPV6
	if (nvram_matchi("ipv6_enable", 1)) {
		writeprocsysnet("ipv6/conf/all/forwarding", "1");
	}
#endif
#ifdef HAVE_GGEW
	char *wordlist = nvram_safe_get("ral");
	char var[256], *next;
	foreach(var, wordlist, next) {
		sysprintf("iptables -I INPUT -s %s -j %s", var, log_accept);
	}
#endif
#ifdef HAVE_IAS
	if (nvram_matchi("ias_startup", 3))
		sysprintf("iptables -t nat -I PREROUTING -i br0 -p udp --dport 53 -j DNAT --to 192.168.11.1:55300");
#endif
#ifdef HAVE_GUESTPORT
	set_gprules("ath0");
#ifdef HAVE_WZRHPAG300NH
	set_gprules("ath1");
#endif
#endif
/*
 *	Services restart.
 * 	Should be always at the end. 
 */
#ifdef HAVE_WIFIDOG
	if (nvram_matchi("wd_enable", 1)) {
		stop_wifidog();
		start_wifidog();
	}
#endif
#ifdef HAVE_CHILLI
	if (nvram_matchi("chilli_enable", 1)
	    || nvram_matchi("hotss_enable", 1)) {
		stop_chilli();
		start_chilli();
	}
#endif
#ifdef HAVE_PPPOESERVER
	if (nvram_matchi("pppoeserver_enabled", 1)) {
		stop_pppoeserver();
		start_pppoeserver();
	}
#endif
	unlock();
	cprintf("ready");
	cprintf("done\n");
}

#ifdef HAVE_IPV6
void stop_firewall6(void)
{
	if (nvram_matchi("ipv6_enable", 0))
		return;
	//eval("ip", "-6", "addr", "flush", "scope", "global");
}
#endif

void stop_firewall(void)
{
	lock();
	eval("iptables", "-t", "raw", "-F");
//      stop_anchorfree();
	/*
	 * Make sure the DMZ-LED is off (from service.c) 
	 */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	diag_led(DMZ, STOP_LED);
#endif
	char num[32];
	int i;
	eval("iptables", "-F");
	eval("iptables", "-t", "nat", "-F");
	eval("iptables", "-t", "mangle", "-F");
	rmmod("ipt_webstr");
	rmmod("ipt_layer7");
	rmmod("xt_layer7");
	rmmod("ipt_ipp2p");
	rmmod("xt_ipp2p");
	if (nvram_invmatchi("apd_enable", 0)) {
		rmmod("ipt_mark");
		rmmod("xt_mark");
	}
	if (nvram_invmatchi("apd_enable", 0)) {
		rmmod("ipt_mac");
		rmmod("xt_mac");
	}
	cprintf("done\n");
#ifdef HAVE_IPV6
	stop_firewall6();
#endif
	unlock();
	return;
}
