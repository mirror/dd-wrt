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
#define NR_RULES		10
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

static char *suspense;
static unsigned int count = 0;
static char log_accept[15];
static char log_drop[15];
static char log_reject[64];
static char wanface[IFNAMSIZ];
static char lanface[IFNAMSIZ];
static char lan_cclass[] = "xxx.xxx.xxx.";
static char wanaddr[] = "xxx.xxx.xxx.xxx";
static int web_lanport = HTTP_PORT;

static FILE *ifd;		/* /tmp/.rule */
static FILE *cfd;		/* /tmp/crontab */

static unsigned int now_wday, now_hrmin;
static int webfilter = 0;
static int dmzenable = 0;
static int remotemanage = 0;

#ifdef HAVE_SSHD
static int remotessh = 0;	/* Botho 03-05-2006 */
#endif

#ifdef HAVE_TELNET
static int remotetelnet = 0;
#endif

static void save2file(const char *fmt, ...)
{
	char buf[10240];
	va_list args;
	FILE *fp;

	if ((fp = fopen(IPTABLES_SAVE_FILE, "a")) == NULL) {
		printf("Can't open /tmp/.ipt\n");
		exit(1);
	}

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	va_start(args, fmt);
	fprintf(fp, "%s", buf);
	va_end(args);

	fclose(fp);
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
		if (array[seq] == 1)
			return -1;
		array[seq] = 1;
	} else {		/* delete */
		if (array[seq] == 0)
			return -1;
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

static void parse_port_forward(char *wordlist)
{
	char var[256], *next;
	char *name, *enable, *proto, *port, *ip;
	char buff[256], ip2[16];
	int flag_dis = 0;
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
	char *wan_proto = nvram_safe_get("wan_proto");
#endif

	/*
	 * name:enable:proto:port>ip name:enable:proto:port>ip 
	 */
	foreach(var, wordlist, next) {
		enable = var;
		name = strsep(&enable, ":");
		if (!name || !enable)
			continue;
		proto = enable;
		enable = strsep(&proto, ":");
		if (!enable || !proto)
			continue;
		port = proto;
		proto = strsep(&port, ":");
		if (!proto || !port)
			continue;
		ip = port;
		port = strsep(&ip, ">");
		if (!port || !ip)
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
				save2file("-A PREROUTING -p tcp -d %s --dport %s -j DNAT --to-destination %s\n", wanaddr, port, ip);
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
				if (!strcmp(wan_proto, "pptp") ||
				    !strcmp(wan_proto, "l2tp") ||
				    !strcmp(wan_proto, "pppoe_dual") ) 
					save2file("-A PREROUTING -i %s -p tcp -d %s --dport %s -j DNAT --to-destination %s\n", nvram_safe_get("wan_ifname"), wanaddr, port, ip);
#endif 
				snprintf(buff, sizeof(buff), "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n", ip, port, log_accept);
			} else {
				if ((!dmzenable)
				    || (dmzenable && strcmp(ip, nvram_safe_get("dmz_ipaddr")))) {
					snprintf(buff, sizeof(buff), "-A FORWARD -p tcp -m tcp -d %s --sport %s -j %s\n", ip, port, log_drop);
				}
			}

			count += strlen(buff) + 1;
			suspense = realloc(suspense, count);
			strcat(suspense, buff);
		}

		if (!strcmp(proto, "udp") || !strcmp(proto, "both")) {
			bzero(buff, sizeof(buff));
			if (flag_dis == 0) {
				save2file("-A PREROUTING -p udp -d %s --dport %s -j DNAT --to-destination %s\n", wanaddr, port, ip);
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
				if (!strcmp(wan_proto, "pptp") ||
				    !strcmp(wan_proto, "l2tp") ||
				    !strcmp(wan_proto, "pppoe_dual") )
					save2file("-A PREROUTING -i %s -p udp -d %s --dport %s -j DNAT --to-destination %s\n", nvram_safe_get("wan_ifname"), wanaddr, port, ip);
#endif 
				snprintf(buff, sizeof(buff), "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", ip, port, log_accept);
			} else {
				if ((!dmzenable)
				    || (dmzenable && strcmp(ip, nvram_safe_get("dmz_ipaddr")))) {
					snprintf(buff, sizeof(buff), "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", ip, port, log_drop);

				}
			}
			count += strlen(buff) + 1;
			suspense = realloc(suspense, count);
			strcat(suspense, buff);
		}
	}
}

#ifdef HAVE_UPNP
static void parse_upnp_forward()
{
	char name[32];		// = "forward_portXXXXXXXXXX";
	char value[1000];
	char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1, *proto;
	char *enable, *desc;
	char buff[256];
	int i;

	if (nvram_invmatch("upnp_enable", "1"))
		return;

	if (nvram_match("upnp_clear", "1")) {	// tofu10
		nvram_unset("upnp_clear");
		for (i = 0; i < 50; ++i) {
			sprintf(name, "forward_port%d", i);
			nvram_unset(name);
		}
		nvram_set("forward_cur", "0");
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
		char *wan = wanface;
		if (!strlen(wan)) {
			wan = "br0";
		}

		if (!strcmp(proto, "tcp") || !strcmp(proto, "both")) {
			save2file("-A PREROUTING -i %s -p tcp -d %s --dport %s " "-j DNAT --to-destination %s%d:%s\n", wan, wanaddr, wan_port0, lan_cclass, get_single_ip(lan_ipaddr, 3), lan_port0);

			snprintf(buff, sizeof(buff), "-A FORWARD -p tcp " "-m tcp -d %s%d --dport %s -j %s\n", lan_cclass, get_single_ip(lan_ipaddr, 3), lan_port0, log_accept);

			count += strlen(buff) + 1;
			suspense = realloc(suspense, count);
			strcat(suspense, buff);
		}
		if (!strcmp(proto, "udp") || !strcmp(proto, "both")) {
			save2file("-A PREROUTING -i %s -p udp -d %s --dport %s " "-j DNAT --to-destination %s%d:%s\n", wan, wanaddr, wan_port0, lan_cclass, get_single_ip(lan_ipaddr, 3), lan_port0);

			snprintf(buff, sizeof(buff), "-A FORWARD -p udp " "-m udp -d %s%d --dport %s -j %s\n", lan_cclass, get_single_ip(lan_ipaddr, 3), lan_port0, log_accept);

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
#endif

	if (src && strlen(src) > 0) {
		save2file("-A PREROUTING -p %s -m %s -s %s -d %s --dport %s -j DNAT --to-destination %s:%s\n", proto, proto, src, wanaddr, from, ip, to);
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
		if (!strcmp(wan_proto, "pptp") ||
		    !strcmp(wan_proto, "l2tp") ||
		    !strcmp(wan_proto, "pppoe_dual") ) 
			save2file("-A PREROUTING -i %s -p %s -m %s -s %s -d %s --dport %s -j DNAT --to-destination %s:%s\n", nvram_safe_get("wan_ifname"), proto, proto, src, wanaddr, from, ip, to);
#endif        
		snprintf(buff, sizeof(buff), "-A FORWARD -p %s -m %s -s %s -d %s --dport %s -j %s\n", proto, proto, src, ip, to, log_accept);
	} else {
		save2file("-A PREROUTING -p %s -m %s -d %s --dport %s -j DNAT --to-destination %s:%s\n", proto, proto, wanaddr, from, ip, to);
#if defined (HAVE_PPTP) || defined (HAVE_L2TP) || defined (HAVE_PPPOEDUAL)
		if (!strcmp(wan_proto, "pptp") ||
		    !strcmp(wan_proto, "l2tp") ||
		    !strcmp(wan_proto, "pppoe_dual") ) 
			save2file("-A PREROUTING -i %s -p %s -m %s -d %s --dport %s -j DNAT --to-destination %s:%s\n", nvram_safe_get("wan_ifname"), proto, proto, wanaddr, from, ip, to);
#endif  
		snprintf(buff, sizeof(buff), "-A FORWARD -p %s -m %s -d %s --dport %s -j %s\n", proto, proto, ip, to, log_accept);
	}
	count += strlen(buff) + 1;
	suspense = realloc(suspense, count);
	strcat(suspense, buff);

}

static void parse_spec_forward(char *wordlist)
{
	char var[256], *next;
	char *name, *enable, *proto, *from, *to, *ip, *src;
	char buff[256];

	/*
	 * name:enable:proto:ext_port>ip:int_port
	 * name:enable:proto:ext_port>ip:int_port 
	 */
	foreach(var, wordlist, next) {
		enable = var;
		name = strsep(&enable, ":");
		if (!name || !enable)
			continue;
		proto = enable;
		enable = strsep(&proto, ":");
		if (!enable || !proto)
			continue;
		from = proto;
		proto = strsep(&from, ":");
		if (!proto || !from)
			continue;
		ip = from;
		from = strsep(&ip, ">");
		if (!from || !ip)
			continue;
		to = ip;
		ip = strsep(&to, ":");
		if (!ip || !to)
			continue;

		src = to;
		to = strsep(&src, "<");
		if (!to) {
			to = src;
			src = NULL;
		}
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

static void nat_prerouting(void)
{
	char var[256], *wordlist, *next;
	char from[100], to[100];
	char *remote_ip_any = nvram_default_get("remote_ip_any", "1");
	char *remote_ip = nvram_default_get("remote_ip", "0.0.0.0 0");
	int remote_any = 0;

	if (!strcmp(remote_ip_any, "1") || !strncmp(remote_ip, "0.0.0.0", 7))
		remote_any = 1;
	/*
	* Block ads on all http requests
	*/	
#ifdef HAVE_PRIVOXY
	if (nvram_match("privoxy_transp_enable", "1") && nvram_match("privoxy_enable", "1")){
		save2file("-A PREROUTING -p tcp -d ! %s --dport 80 -j REDIRECT --to-port 8118\n", wanaddr);
	}
#endif
#ifdef HAVE_TOR
	if (nvram_match("tor_transparent", "1") && nvram_match("tor_enable", "1")){
		save2file("-A PREROUTING -i br0 -p udp --dport 53 -j REDIRECT --to-ports 53\n");
		save2file("-A PREROUTING -i br0 -p tcp --syn -j REDIRECT --to-ports 9040\n");
	}
#endif
	/*
	 * Enable remote Web GUI management 
	 */
	if (remotemanage) {
		if (remote_any) {
			save2file("-A PREROUTING -p tcp -d %s --dport %s " "-j DNAT --to-destination %s:%d\n", wanaddr, nvram_safe_get("http_wanport"), nvram_safe_get("lan_ipaddr"), web_lanport);
		} else {
			sscanf(remote_ip, "%s %s", from, to);

			wordlist = range(from, get_complete_ip(from, to));

			foreach(var, wordlist, next) {
				save2file("-A PREROUTING -p tcp -s %s -d %s --dport %s " "-j DNAT --to-destination %s:%d\n", var, wanaddr, nvram_safe_get("http_wanport"), nvram_safe_get("lan_ipaddr"), web_lanport);
			}
		}
	}
#ifdef HAVE_SSHD
	/*
	 * Enable remote ssh management : Botho 03-05-2006 
	 */
	if (remotessh) {
		if (remote_any) {
			save2file("-A PREROUTING -p tcp -d %s --dport %s " "-j DNAT --to-destination %s:%s\n", wanaddr, nvram_safe_get("sshd_wanport"), nvram_safe_get("lan_ipaddr"), nvram_safe_get("sshd_port"));
		} else {
			sscanf(remote_ip, "%s %s", from, to);

			wordlist = range(from, get_complete_ip(from, to));

			foreach(var, wordlist, next) {
				save2file
				    ("-A PREROUTING -p tcp -s %s -d %s --dport %s "
				     "-j DNAT --to-destination %s:%s\n", var, wanaddr, nvram_safe_get("sshd_wanport"), nvram_safe_get("lan_ipaddr"), nvram_safe_get("sshd_port"));
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
			save2file("-A PREROUTING -p tcp -d %s --dport %s " "-j DNAT --to-destination %s:23\n", wanaddr, nvram_safe_get("telnet_wanport"), nvram_safe_get("lan_ipaddr"));
		} else {
			sscanf(remote_ip, "%s %s", from, to);

			wordlist = range(from, get_complete_ip(from, to));

			foreach(var, wordlist, next) {
				save2file("-A PREROUTING -p tcp -s %s -d %s --dport %s " "-j DNAT --to-destination %s:23\n", var, wanaddr, nvram_safe_get("telnet_wanport"), nvram_safe_get("lan_ipaddr"));
			}
		}
	}
#endif

	/*
	 * ICMP packets are always redirected to INPUT chains 
	 */
	save2file("-A PREROUTING -p icmp -d %s -j DNAT --to-destination %s\n", wanaddr, nvram_safe_get("lan_ipaddr"));
	

#ifdef HAVE_TFTP
	/*
	 * Enable remote upgrade 
	 */
	if (nvram_match("remote_upgrade", "1"))
		save2file("-A PREROUTING -p udp -d %s --dport %d " "-j DNAT --to-destination %s\n", wanaddr, TFTP_PORT, nvram_safe_get("lan_ipaddr"));
#endif

	/*
	 * Initiate suspense string for parse_port_forward() 
	 */
	suspense = malloc(1);
	*suspense = 0;
	count = 1;

	if (has_gateway()) {
		/*
		 * Port forwarding 
		 */
#ifdef HAVE_UPNP
		parse_upnp_forward();
#endif
		parse_spec_forward(nvram_safe_get("forward_spec"));
		parse_port_forward(nvram_safe_get("forward_port"));
		/*
		 * DD-WRT addition by Eric Sauvageau 
		 */
		save2file("-A PREROUTING -d %s -j TRIGGER --trigger-type dnat\n", wanaddr);
		/*
		 * DD-WRT addition end 
		 */
	}

	/*
	 * DMZ forwarding 
	 */
	if (dmzenable)
		save2file("-A PREROUTING -d %s -j DNAT --to-destination %s%s\n", wanaddr, lan_cclass, nvram_safe_get("dmz_ipaddr"));

}

static int wanactive(void)
{
	return (!nvram_match("wan_proto", "disabled") && strcmp(wanaddr, "0.0.0.0") && check_wan_link(0));
}

static void nat_postrouting(void)
{
#ifdef HAVE_PPPOESERVER
	if (nvram_match("pppoeserver_enabled", "1")
	    && wanactive())
		save2file("-A POSTROUTING -s %s/%s -j SNAT --to-source=%s\n", nvram_safe_get("pppoeserver_remotenet"), nvram_safe_get("pppoeserver_remotemask"), wanaddr);
#endif
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
		if (nvram_match("dtag_vlan8", "1")
		    && nvram_match("wan_vdsl", "1")) {
			save2file("-A POSTROUTING -o %s -j SNAT --to-source %s\n", nvram_safe_get("tvnicfrom"), nvram_safe_get("tvnicaddr"));
		}
		if (strlen(wanface) > 0 && wanactive()
		    && !nvram_match("br0_nat", "0"))
			save2file("-A POSTROUTING -s %s/%d -o %s -j SNAT --to-source %s\n", nvram_safe_get("lan_ipaddr"), loopmask, wanface, wanaddr);

		char *wan_ifname_tun = nvram_safe_get("wan_ifname");
		if (isClient()) {
			wan_ifname_tun = getSTA();
		}

		if (nvram_match("wan_proto", "pptp")) {
			struct in_addr ifaddr;
			osl_ifaddr(nvram_safe_get("pptp_ifname"), &ifaddr);
			save2file("-A POSTROUTING -o %s -j SNAT --to-source %s\n", nvram_safe_get("pptp_ifname"), inet_ntoa(ifaddr));
		}
		if (nvram_match("wan_proto", "l2tp")) {
			struct in_addr ifaddr;
			osl_ifaddr(wan_ifname_tun, &ifaddr);
			save2file("-A POSTROUTING -o %s -j SNAT --to-source %s\n", wan_ifname_tun, inet_ntoa(ifaddr));
		}

		if (nvram_match("block_loopback", "0")) {
			save2file("-A POSTROUTING -m mark --mark %s -j MASQUERADE\n", get_NFServiceMark("FORWARD", 1));
		}

		char *next;
		char dev[16];
		char var[80];

		char vifs[256];

		getIfLists(vifs, 256);
		// char *vifs = nvram_safe_get ("lan_ifnames");
		// if (vifs != NULL)
		foreach(var, vifs, next) {
			if (strcmp(get_wan_face(), var)
			    && strcmp(nvram_safe_get("lan_ifname"), var)) {
				if (nvram_nmatch("0", "%s_bridged", var)) {

					char nat[32];
					sprintf(nat, "%s_nat", var);
					nvram_default_get(nat, "1");
					if (nvram_match(nat, "1")) {
						save2file("-A POSTROUTING -s %s/%d -o %s -j SNAT --to-source %s\n", nvram_nget("%s_ipaddr", var), getmask(nvram_nget("%s_netmask", var)), wanface, wanaddr);
					}

				}
			}
		}
		if (nvram_match("block_loopback", "0"))
			writeproc("/proc/sys/net/ipv4/conf/br0/loop", "1");

	} else {
		eval("iptables", "-t", "raw", "-A", "PREROUTING", "-j", "NOTRACK");	//this speeds up networking alot on slow systems 

		/* the following code must be used in future kernel versions, not yet used. we still need to test it */
//              eval("iptables", "-t", "raw", "-A", "PREROUTING", "-j", "CT","--notrack");      //this speeds up networking alot on slow systems 
		if (strlen(wanface) > 0 && wanactive())
			if (nvram_match("wl_br1_enable", "1"))
				save2file("-A POSTROUTING -o %s -j SNAT --to-source %s\n", wanface, wanaddr);
	}
}

static void parse_port_filter(char *wordlist)
{
	char var[256], *next;
	char *protocol, *lan_port0, *lan_port1;

	/*
	 * Parse protocol:lan_port0-lan_port1 ... 
	 */
	foreach(var, wordlist, next) {
		lan_port0 = var;
		protocol = strsep(&lan_port0, ":");
		if (!protocol || !lan_port0)
			continue;
		lan_port1 = lan_port0;
		lan_port0 = strsep(&lan_port1, "-");
		if (!lan_port0 || !lan_port1)
			continue;

		if (!strcmp(protocol, "disable"))
			continue;

		/*
		 * -A FORWARD -i br0 -p tcp --dport 0:655 -j logdrop -A
		 * FORWARD -i br0 -p udp --dport 0:655 -j logdrop 
		 */
		if (!strcmp(protocol, "tcp") || !strcmp(protocol, "both")) {
			save2file("-A FORWARD -i %s -p tcp --dport %s:%s -j %s\n", lanface, lan_port0, lan_port1, log_drop);
		}
		if (!strcmp(protocol, "udp") || !strcmp(protocol, "both")) {
			save2file("-A FORWARD -i %s -p udp --dport %s:%s -j %s\n", lanface, lan_port0, lan_port1, log_drop);
		}
	}
}

/*
 * Return 1 for match, 0 for accept, -1 for partial. 
 */
static int find_pattern(const char *data, size_t dlen, const char *pattern, size_t plen, char term, unsigned int *numoff, unsigned int *numlen)
{
	size_t i, j, k;

	// DEBUGP("find_pattern `%s': dlen = %u\n", pattern, dlen);
	if (dlen == 0)
		return 0;

	if (dlen <= plen) {
		/*
		 * Short packet: try for partial? 
		 */
		if (strncmp(data, pattern, dlen) == 0)
			return -1;
		else
			return 0;
	}

	for (i = 0; i <= (dlen - plen); i++) {
		if (memcmp(data + i, pattern, plen) != 0)
			continue;

		/*
		 * patten match !! 
		 */
		*numoff = i + plen;
		for (j = *numoff, k = 0; data[j] != term; j++, k++)
			if (j > dlen)
				return -1;	/* no terminal char */

		*numlen = k;
		return 1;
	}

	return 0;
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
static int schedule_by_tod(int seq)
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
		save2file("-A lan2wan -j grp_%d\n", seq);
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
		save2file("-A lan2wan -j grp_%d\n", seq);
		return 1;
	}

	return 0;
}

static void macgrp_chain(int seq, unsigned int mark, int urlenable)
{
	char var[256], *next;
	char *wordlist;

	wordlist = nvram_nget("filter_mac_grp%d", seq);
	if (strcmp(wordlist, "") == 0)
		return;

	insmod("ipt_mac");
	insmod("xt_mac");

	if (mark == MARK_DROP) {
		foreach(var, wordlist, next) {
			save2file("-A grp_%d -m mac --mac-source %s -j %s\n", seq, var, log_drop);
			save2file("-A grp_%d -m mac --mac-destination %s -j %s\n", seq, var, log_drop);
		}
	} else {
		foreach(var, wordlist, next) {
			save2file("-A grp_%d -m mac --mac-source %s -j advgrp_%d\n", seq, var, seq);
			save2file("-A grp_%d -m mac --mac-destination %s -j advgrp_%d\n", seq, var, seq);

			/*
			 * mark = urlenable ? mark : webfilter ? MARK_HTTP : 0; if (mark) 
			 * { save2file("-A macgrp_%d -p tcp --dport %d -m mac "
			 * "--mac-source %s -j MARK --set-mark %d\n" , seq, HTTP_PORT,
			 * var, mark); } 
			 */
		}
	}
}

static void ipgrp_chain(int seq, unsigned int mark, int urlenable)
{
	char buf[256];
	char var1[256], *wordlist1, *next1;
	char var2[256], *wordlist2, *next2;
	char from[100], to[100];
	int a1 = 0, a2 = 0;
	static char s1[32], s2[32];

	wordlist1 = nvram_nget("filter_ip_grp%d", seq);
	if (strcmp(wordlist1, "") == 0)
		return;

	foreach(var1, wordlist1, next1) {
		if (contains(var1, '-')) {
			char *end = var1;
			char *start = strsep(&end, "-");
			if (!contains(start, '.') || !contains(end, '.')) {
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
			wordlist2 = range(from, to);
		} else if (sscanf(var1, "%d", &a1) == 1) {
			if (a1 == 0)	/* unset */
				continue;

			snprintf(buf, sizeof(buf), "%s%d", lan_cclass, a1);
			wordlist2 = buf;
		} else
			continue;

		DEBUG("range=%s\n", wordlist2);

		if (mark == MARK_DROP) {
			foreach(var2, wordlist2, next2) {
				save2file("-A grp_%d -s %s -j %s\n", seq, var2, log_drop);
			}
		} else {
			foreach(var2, wordlist2, next2) {
				save2file("-A grp_%d -s %s -j advgrp_%d\n", seq, var2, seq);
				save2file("-A grp_%d -d %s -j advgrp_%d\n", seq, var2, seq);

				/*
				 * mark = urlenable ? mark : webfilter ? MARK_HTTP : 0; if
				 * (mark){ save2file("-A ipgrp_%d -p tcp --dport %d -s 
				 * %s " "-j MARK --set-mark %d\n", seq, HTTP_PORT, var2,
				 * mark); } 
				 */
			}
		}
	}
}

static void portgrp_chain(int seq, unsigned int mark, int urlenable)
{
	char var[256], *next;
	char *wordlist;
	char target[100];
	char *protocol, *lan_port0, *lan_port1;

	wordlist = nvram_nget("filter_dport_grp%d", seq);
	if (strcmp(wordlist, "") == 0)
		return;

	/*
	 * Determine the filter target 
	 */
	if (mark == MARK_DROP)
		strncpy(target, log_drop, sizeof(log_drop));
	else
		sprintf(target, "advgrp_%d", seq);

	/*
	 * Parse protocol:lan_port0-lan_port1 ... 
	 */
	foreach(var, wordlist, next) {
		lan_port0 = var;
		protocol = strsep(&lan_port0, ":");
		if (!protocol || !lan_port0)
			continue;
		lan_port1 = lan_port0;
		lan_port0 = strsep(&lan_port1, "-");
		if (!lan_port0 || !lan_port1)
			continue;

		if (!strcmp(protocol, "disable"))
			continue;

		/*
		 * -A grp_* -p tcp --dport 0:655 -j logdrop -A grp_* -p udp -m 
		 * udp --dport 0:655 -j logdrop 
		 */
		if (!strcmp(protocol, "tcp") || !strcmp(protocol, "both")) {
			save2file("-A grp_%d -p tcp --dport %s:%s -j %s\n", seq, lan_port0, lan_port1, target);
		}
		if (!strcmp(protocol, "udp") || !strcmp(protocol, "both")) {
			save2file("-A grp_%d -p udp --dport %s:%s -j %s\n", seq, lan_port0, lan_port1, target);
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
static void advgrp_chain(int seq, unsigned int mark, int urlenable)
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
				save2file("-A advgrp_%d -p tcp --dport %s -j %s\n", seq, ports, log_drop);
			if (!strcmp(protocol, "udp")
			    || !strcmp(protocol, "both"))
				save2file("-A advgrp_%d -p udp --dport %s -j %s\n", seq, ports, log_drop);
			if (!strcmp(protocol, "icmp"))
				save2file("-A advgrp_%d -p icmp -j %s\n", seq, log_drop);
			if (!strcmp(protocol, "l7")) {
				int i;

				for (i = 0; i < strlen(realname); i++)
					realname[i] = tolower(realname[i]);
				insmod("ipt_layer7");
				insmod("xt_layer7");

				save2file("-A advgrp_%d -m layer7 --l7proto %s -j %s\n", seq, realname, log_drop);
			}
#ifdef HAVE_OPENDPI
			if (!strcmp(protocol, "dpi")) {
				insmod("/lib/opendpi/xt_opendpi.ko");
				save2file("-A advgrp_%d -m ndpi --%s -j %s\n", seq, realname, log_drop);
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
					insmod("ipt_ipp2p");
					insmod("xt_ipp2p");
					save2file("-A advgrp_%d -m ipp2p --%s -j %s\n", seq, proto, log_drop);
					if (!strcmp(proto, "bit")) {
						/* bittorrent detection enhanced */
#ifdef HAVE_OPENDPI
						insmod("/lib/opendpi/xt_opendpi.ko");
						save2file("-A advgrp_%d -m ndpi --bittorrent -j %s\n", seq, log_drop);
#else
						insmod("ipt_layer7");
						insmod("xt_layer7");
#ifdef HAVE_MICRO
						save2file("-A advgrp_%d -m layer7 --l7proto bt -j %s\n", seq, log_drop);
#else
						save2file("-A advgrp_%d -m length --length 0:550 -m layer7 --l7proto bt -j %s\n", seq, log_drop);
#endif
						save2file("-A advgrp_%d -m layer7 --l7proto bt1 -j %s\n", seq, log_drop);
						save2file("-A advgrp_%d -m layer7 --l7proto bt2 -j %s\n", seq, log_drop);
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
		insmod("ipt_layer7");
		insmod("xt_layer7");
		insmod("ipt_ipp2p");
		insmod("xt_ipp2p");
		save2file("-A advgrp_%d -m ipp2p --ipp2p -j %s\n", seq, log_drop);
		/* p2p detection enhanced */
#ifdef HAVE_OPENDPI
		insmod("/lib/opendpi/xt_opendpi.ko");
		/*commonly used protocols, decending */
		save2file("-A advgrp_%d -m ndpi --bittorrent -j %s\n", seq, log_drop);
/*  disable till pattern works
		save2file
		    ("-A advgrp_%d -m ndpi --edonkey -j %s\n",
		     seq, log_drop); */
		/*atm rarly used protocols */
		save2file("-A advgrp_%d -m ndpi --apple -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m ndpi --directconnect -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m ndpi --fasttrack -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m ndpi --filetopia -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m ndpi --gnutella -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m ndpi --imesh -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m ndpi --openft -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m ndpi --pando -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m ndpi --soulseek -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m ndpi --winmx -j %s\n", seq, log_drop);
#else
#ifdef HAVE_MICRO
		save2file("-A advgrp_%d -m layer7 --l7proto bt -j %s\n", seq, log_drop);
#else
		save2file("-A advgrp_%d -m length --length 0:550 -m layer7 --l7proto bt -j %s\n", seq, log_drop);
#endif
#endif
		/* commonly used protocols, decending */
		save2file("-A advgrp_%d -m layer7 --l7proto bt4 -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto bt1 -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto bittorrent -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto bt2 -j %s\n", seq, log_drop);
/*		save2file("-A advgrp_%d -m layer7 --l7proto bt -j %s\n", seq, log_drop); */
		save2file("-A advgrp_%d -p tcp -m layer7 --l7proto gnutella -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d  -p tcp -m layer7 --l7proto ares -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto applejuice -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto audiogalaxy -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m layer7 --l7proto bearshare -j %s\n", seq, log_drop);
		/* atm rarly used protocols */
		save2file("-A advgrp_%d -p tcp -m layer7 --l7proto directconnect -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto edonkey -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto fasttrack -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto freenet -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto gnucleuslan -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto goboogy -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto hotline -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto imesh -j %s\n", seq, log_drop);
/*	 	save2file("-A advgrp_%d -m layer7 --l7proto kugoo -j %s\n", seq, log_drop);// xunlei, kugoo, winmx block websurfing */
		save2file("-A advgrp_%d -p tcp -m layer7 --l7proto mute -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m layer7 --l7proto napster -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m layer7 --l7proto openft -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m layer7 --l7proto soribada -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -p tcp -m layer7 --l7proto soulseek -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto tesla -j %s\n", seq, log_drop);
/*		save2file("-A advgrp_%d -p tcp -m layer7 --l7proto winmx -j %s\n", seq, log_drop);
		save2file("-A advgrp_%d -m layer7 --l7proto xunlei -j %s\n", seq, log_drop); */
	}
	free(services);
	/*
	 * filter_web_host2=hello<&nbsp;>world<&nbsp;>friend 
	 */
	wordlist = nvram_nget("filter_web_host%d", seq);
	if (wordlist && strcmp(wordlist, "")) {
		insmod("ipt_webstr");
		save2file("-A advgrp_%d -p tcp -m webstr --host \"%s\" -j %s\n", seq, wordlist, log_reject);
	}
	/*
	 * filter_web_url3=hello<&nbsp;>world<&nbsp;>friend 
	 */
	wordlist = nvram_nget("filter_web_url%d", seq);
	if (wordlist && strcmp(wordlist, "")) {
		insmod("ipt_webstr");
		save2file("-A advgrp_%d -p tcp -m webstr --url \"%s\" -j %s\n", seq, wordlist, log_reject);
	}
	/*
	 * Others will be accepted 
	 */
	// save2file ("-A advgrp_%d -j %s\n", seq, log_accept);
}

static void lan2wan_chains(void)
{
	time_t ct;		/* Calendar time */
	struct tm *bt;		/* Broken time */
	int seq;
	char buf[] = "filter_rulexxx";
	char *data;
	int offset, len;
	unsigned int mark = 0;
	int up = 0;
	int urlfilter = 1;

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
			up = schedule_by_tod(seq);

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
		find_pattern(data, strlen(data), "$STAT:", sizeof("$STAT:") - 1, '$', &offset, &len);

		if (len < 1)
			continue;	/* error format */

		strncpy(buf, data + offset, len);
		*(buf + len) = 0;
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
		macgrp_chain(seq, mark, urlfilter);
		ipgrp_chain(seq, mark, urlfilter);
		portgrp_chain(seq, mark, urlfilter);
		advgrp_chain(seq, mark, urlfilter);
	}
}

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

	if ((ord = update_bitmap(mode, seq)) < 0)
		return -1;

	sprintf(target_ip, "grp_%d", seq);
	sprintf(order, "%d", ord * 1 + 1);
	DEBUG("order=%s\n", order);

	/*
	 * iptables -t mangle -I lan2wan 3 -j macgrp_9 
	 */
	if (mode == 1) {	/* insert */
		DEBUG("iptables -I lan2wan %s -j %s\n", order, target_ip);
		eval("iptables", "-I", "lan2wan", order, "-j", target_ip);
	} else {		/* delete */
		DEBUG("iptables -D lan2wan %s\n", order);
		eval("iptables", "-D", "lan2wan", order);
	}

	cprintf("done\n");
	return 0;
}

void start_filter_add(int seq)
{
	DEBUG("filter_add:\n");
	update_filter(1, seq);

}

void start_filter_del(int seq)
{
	DEBUG("filter_del:\n");
	update_filter(0, seq);
}

void start_filtersync(void)
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
			start_filter_add(seq);
		else
			start_filter_del(seq);
		DEBUG("seq=%d, ret=%d\n", seq, ret);
	}
}

static void parse_trigger_out(char *wordlist)
{
	char var[256], *next;
	char *name, *enable, *proto;
	char *wport0, *wport1, *lport0, *lport1;

	/*
	 * port_trigger=name:[on|off]:[tcp|udp|both]:wport0-wport1>lport0-lport1 
	 */
	foreach(var, wordlist, next) {
		enable = var;
		name = strsep(&enable, ":");
		if (!name || !enable)
			continue;
		proto = enable;
		enable = strsep(&proto, ":");
		if (!enable || !proto)
			continue;
		wport0 = proto;
		proto = strsep(&wport0, ":");
		if (!proto || !wport0)
			continue;
		wport1 = wport0;
		wport0 = strsep(&wport1, "-");
		if (!wport0 || !wport1)
			continue;
		lport0 = wport1;
		wport1 = strsep(&lport0, ">");
		if (!wport1 || !lport0)
			continue;
		lport1 = lport0;
		lport0 = strsep(&lport1, "-");
		if (!lport0 || !lport1)
			continue;

		/*
		 * skip if it's disabled 
		 */
		if (strcmp(enable, "off") == 0)
			continue;

		if (!strcmp(proto, "tcp") || !strcmp(proto, "udp")) {
			save2file("-A trigger_out -p %s -m %s --dport %s:%s "
				  "-j TRIGGER --trigger-type out --trigger-proto %s " "--trigger-match %s-%s --trigger-relate %s-%s\n", proto, proto, wport0, wport1, proto, wport0, wport1, lport0, lport1);
		} else if (!strcmp(proto, "both")) {
			save2file("-A trigger_out -p tcp --dport %s:%s "
				  "-j TRIGGER --trigger-type out --trigger-proto all " "--trigger-match %s-%s --trigger-relate %s-%s\n", wport0, wport1, wport0, wport1, lport0, lport1);
			save2file("-A trigger_out -p udp --dport %s:%s "
				  "-j TRIGGER --trigger-type out --trigger-proto all " "--trigger-match %s-%s --trigger-relate %s-%s\n", wport0, wport1, wport0, wport1, lport0, lport1);
		}
	}
}

#ifdef HAVE_VLANTAGGING
static void add_bridges(char *chain, int forward)
{
	static char word[256];
	char *next, *wordlist;
	char *wan = get_wan_face();
	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next) {
		char *port = word;
		char *tag = strsep(&port, ">");
		char *prio = port;

		strsep(&prio, ">");
		if (!tag || !port)
			break;
		char ipaddr[32];

		sprintf(ipaddr, "%s_ipaddr", tag);
		char netmask[32];

		sprintf(netmask, "%s_netmask", tag);
		if (ifexists(tag)) {
			if (nvram_get(ipaddr) && nvram_get(netmask)
			    && !nvram_match(ipaddr, "0.0.0.0")
			    && !nvram_match(netmask, "0.0.0.0")) {
				eval("ifconfig", tag, nvram_safe_get(ipaddr), "netmask", nvram_safe_get(netmask), "up");
			} else {
				eval("ifconfig", tag, "up");

			}
			if (forward && wan && strlen(wan))
				save2file("-A FORWARD -i %s -o %s -j %s\n", tag, wan, log_accept);
			else {
				if (!strcmp(chain, "OUTPUT"))
					save2file("-A %s -o %s -j %s\n", chain, tag, log_accept);
				else
					save2file("-A %s -i %s -j %s\n", chain, tag, log_accept);
			}
		}
	}

}

#endif
static void filter_input(void)
{

	char *next, *iflist, buff[16];

	/*
	 * Filter known SPI state 
	 */
	/*
	 * most of what was here has been moved to the end 
	 */
	save2file("-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n", log_accept);
	if (nvram_match("dtag_vlan8", "1") && nvram_match("wan_vdsl", "1")) {
		save2file("-A INPUT -i %s -j %s\n", nvram_safe_get("tvnicfrom"), log_accept);
	}
#ifdef HAVE_PPTP
	/*
	 * Impede DoS/Bruteforce, redcuce possilbe bruteforce on pptp server
	 */
#ifndef HAVE_MICRO
	if (nvram_match("pptpd_enable", "1")) {
		if (nvram_match("limit_pptp", "1")) {
			save2file("-A INPUT -i %s -p tcp --dport %d -j logbrute\n", wanface, PPTP_PORT);
		} else {
			save2file("-A INPUT -i %s -p tcp --dport %d -j %s\n", wanface, PPTP_PORT, log_accept);
		}
	}
#endif  
	if (nvram_match("wan_proto", "dhcp") )
		save2file("-A INPUT -i %s -p udp --sport 67 --dport 68 -j %s\n", wanface, log_accept);
	if (nvram_match("pptpd_enable", "1")
	    || nvram_match("pptpd_client_enable", "1")
	    || nvram_match("wan_proto", "pptp")) {
		save2file("-A INPUT -p tcp --dport %d -j %s\n", PPTP_PORT, log_accept);
		save2file("-A INPUT -p 47 -j %s\n", log_accept);
		if (nvram_match("pptpd_lockdown", "1")) {
			save2file("-A INPUT -i %s -p udp --sport 67 --dport 68 -j %s\n", lanface, log_accept);
			save2file("-A INPUT -i %s -j %s\n", lanface, log_drop);
		}
	}
#endif
#ifdef HAVE_FTP
	if (nvram_match("proftpd_enable", "1")
	    && nvram_match("proftpd_wan", "1")) {
		if (nvram_match("limit_ftp", "1")) {
			save2file("-A INPUT -i %s -p tcp --dport %s -j logbrute\n", wanface, nvram_safe_get("proftpd_port"));
		} else {
			save2file("-A INPUT -i %s -p tcp --dport %s -j %s\n", wanface, nvram_safe_get("proftpd_port"), log_accept);
		}
	}
#endif
/*#ifdef HAVE_AP_SERV
	save2file("-A INPUT -i %s -p udp --dport 22359 -j ACCEPT\n",lanface);
	save2file("-A INPUT -i %s -p udp --sport 22359 -j ACCEPT\n",lanface);
	save2file("-A OUTPUT -p udp --sport 22359 -j ACCEPT\n");
#endif*/
	/*
	 * Routing protocol, RIP, accept 
	 */
	/*
	 * lonewolf mods for multiple VLANs / interfaces 
	 */
#ifdef HAVE_OPENVPN
	//check if ovpn server is running
	if (nvram_match("openvpn_enable", "1")
	    && nvram_match("openvpn_switch", "1")) {
		save2file("-A INPUT -p %s --dport %s -j %s\n", nvram_match("openvpn_proto", "udp") ? "udp" : "tcp", nvram_safe_get("openvpn_port"), log_accept);
		if (nvram_match("openvpn_tuntap", "tun")) {
//			if (strlen(nvram_safe_get("openvpn_ccddef")) = 0) {
				save2file("-A INPUT -i %s2 -j %s\n", nvram_safe_get("openvpn_tuntap"), log_accept);
				save2file("-A FORWARD -i %s2 -j %s\n", nvram_safe_get("openvpn_tuntap"), log_accept);
				save2file("-A FORWARD -o %s2 -j %s\n", nvram_safe_get("openvpn_tuntap"), log_accept);
//			}
		}
	}
#endif
	if (wanactive()) {
		if (nvram_invmatch("dr_wan_rx", "0"))
			save2file("-A INPUT -p udp -i %s --dport %d -j %s\n", wanface, RIP_PORT, log_accept);
		else
			save2file("-A INPUT -p udp -i %s --dport %d -j %s\n", wanface, RIP_PORT, log_drop);
	}
	if (nvram_invmatch("dr_lan_rx", "0"))
		save2file("-A INPUT -p udp -i %s --dport %d -j %s\n", lanface, RIP_PORT, log_accept);
	else
		save2file("-A INPUT -p udp -i %s --dport %d -j %s\n", lanface, RIP_PORT, log_drop);

	iflist = nvram_safe_get("no_route_if");
	foreach(buff, iflist, next) {
		save2file("-A INPUT -p udp -i %s --dport %d -j %s\n", buff, RIP_PORT, log_drop);
	}

	save2file("-A INPUT -p udp --dport %d -j %s\n", RIP_PORT, log_accept);

	/*
	 * end lonewolf mods 
	 */

	/*
	 * Wolf mod - accept protocol 41 for IPv6 tunneling 
	 */
	if (nvram_match("ipv6_enable", "1"))
		save2file("-A INPUT -p 41 -j %s\n", log_accept);

	/*
	 * Sveasoft mod - accept OSPF protocol broadcasts 
	 */
	if (nvram_match("wk_mode", "ospf"))
		save2file("-A INPUT -p ospf -j %s\n", log_accept);
	if (nvram_match("wk_mode", "bgp"))
		save2file("-A INPUT -p tcp --dport 179 -j %s\n", log_accept);
#ifdef HAVE_OLSRD
	if (nvram_match("wk_mode", "olsr"))
		save2file("-A INPUT -p udp --dport 698 -j %s\n", log_accept);
#endif
	/*
	 * Sveasoft mod - default for br1/separate subnet WDS type 
	 */
	if (nvram_match("wl0_br1_enable", "1")
	    && nvram_invmatch("wl0_br1_nat", "1")
	    && nvram_invmatch("wl0_br1_nat", "2"))
		save2file("-A INPUT -i br1 -j %s\n", log_accept);
	if (nvram_match("wl1_br1_enable", "1")
	    && nvram_invmatch("wl1_br1_nat", "1")
	    && nvram_invmatch("wl1_br1_nat", "2"))
		save2file("-A INPUT -i br1 -j %s\n", log_accept);
#ifdef HAVE_VLANTAGGING
	add_bridges("INPUT", 0);
#endif

	/*
	 * Remote Web GUI Management Use interface name, destination address, and 
	 * port to make sure that it's redirected from WAN 
	 */
	if (remotemanage) {
		save2file("-A INPUT -p tcp -d %s --dport %d -j %s\n", nvram_safe_get("lan_ipaddr"), web_lanport, log_accept);
	}
#ifdef HAVE_SSHD
	/*
	 * Impede DoS/Bruteforce, reduce load on ssh
	 */
#ifndef HAVE_MICRO
	if (remotessh) {

		if (nvram_match("limit_ssh", "1"))
			save2file("-A INPUT -i %s -p tcp -d %s --dport %s -j logbrute\n", wanface, nvram_safe_get("lan_ipaddr") , nvram_safe_get("sshd_port"));
	}
#endif
	/*
	 * Remote Web GUI Management Botho 03-05-2006 : remote ssh & remote GUI
	 * management are not linked anymore 
	 */
	if (remotessh) {
		save2file("-A INPUT -i %s -p tcp -d %s --dport %s -j %s\n", wanface, nvram_safe_get("lan_ipaddr"), nvram_safe_get("sshd_port"), log_accept);
	}
#endif

#ifdef HAVE_TELNET
	/*
	 * Impede DoS/Bruteforce, reduce load on Telnet
	 */
#ifndef HAVE_MICRO
	if (remotetelnet) {
		if (nvram_match("limit_telnet", "1"))
			save2file("-A INPUT -i %s -p tcp -d %s --dport 23 -j logbrute\n", wanface, nvram_safe_get("lan_ipaddr"));
	}
#endif
	if (remotetelnet) {
			save2file("-A INPUT -i %s -p tcp -d %s --dport 23 -j %s\n", wanface, nvram_safe_get("lan_ipaddr"), log_accept);
	}
#endif
	/*
	 * ICMP request from WAN interface 
	 */
	if (wanactive())
		save2file("-A INPUT -i %s -p icmp -j %s\n", wanface, nvram_match("block_wan", "1") ? log_drop : log_accept);

	/*
	 * IGMP query from WAN interface 
	 */
	save2file("-A INPUT -p igmp -j %s\n", doMultiCast() == 0 ? log_drop : log_accept);

#ifdef HAVE_UDPXY
	if (wanactive() && nvram_match("udpxy_enable", "1") && nvram_get("tvnicfrom"))
		save2file("-A INPUT -i %s -p udp -d %s -j %s\n", nvram_safe_get("tvnicfrom"), IP_MULTICAST, log_accept);
#endif

#ifndef HAVE_MICRO
	/*
	 * SNMP access from WAN interface 
	 */
	if (nvram_match("snmpd_enable", "1") && nvram_match("block_snmp", "0")) {
		save2file("-A INPUT -i %s -p udp --dport 161 -j %s\n", wanface, log_accept);
	}
#endif

#ifdef HAVE_TFTP
	/*
	 * Remote Upgrade 
	 */
	if (nvram_match("remote_upgrade", "1"))
		save2file("-A INPUT -p udp --dport %d -j %s\n", TFTP_PORT, log_accept);
#endif

#ifdef HAVE_MILKFISH
	if (strlen(wanface) && nvram_match("milkfish_enabled", "1"))
		save2file("-A INPUT -p udp -i %s --dport 5060 -j %s\n", wanface, log_accept);
	// save2file ("-A INPUT -m udp -p udp -i %s --dport 35000 36000 -j
	// ACCEPT\n", wanface);
#endif
#ifdef HAVE_VNCREPEATER
	if (nvram_match("vncr_enable", "1") && strlen(wanface)) {
		save2file("-A INPUT -p tcp -i %s --dport 5900 -j %s\n", wanface, log_accept);
		save2file("-A INPUT -p tcp -i %s --dport 5500 -j %s\n", wanface, log_accept);
	}
#endif

	/*
	 * Ident request backs by telnet or IRC server 
	 */
	if (nvram_match("block_ident", "0"))
		save2file("-A INPUT -p tcp --dport %d -j %s\n", IDENT_PORT, log_accept);

	/*
	 * Filter known SPI state 
	 */
	// removed first rule: -A INPUT -m state --state INVALID -j DROP
	// (wolfiR)
	save2file("-A INPUT -i lo -m state --state NEW -j ACCEPT\n");
	save2file("-A INPUT -i %s -m state --state NEW -j %s\n", lanface, log_accept);

	/*
	 * lonewolf mods for extra VLANs / interfaces 
	 */
	iflist = nvram_safe_get("no_firewall_if");
	foreach(buff, iflist, next) {
		save2file("-A INPUT -i %s -m state --state NEW -j %s\n", buff, log_accept);
	}
	char dev[16];
	char var[80];

	char vifs[256];

	getIfLists(vifs, 256);

	// char *vifs = nvram_safe_get ("lan_ifnames");
	// if (vifs != NULL)
	foreach(var, vifs, next) {
		if (strcmp(get_wan_face(), var)
		    && strcmp(nvram_safe_get("lan_ifname"), var)) {
			if (nvram_nmatch("0", "%s_bridged", var)) {
				save2file("-A INPUT -i %s -j %s\n", var, log_accept);
			}
		}
	}

	/*
	 * end lonewolf mods 
	 */

	/*
	 * Drop those packets we are NOT recognizable 
	 */
	save2file("-A INPUT -j %s\n", log_drop);
}

void filter_output(void)
{
	/*
	 * Sveasoft mod - default for br1/separate subnet WDS type 
	 */
	if (nvram_match("wl0_br1_enable", "1")
	    && nvram_invmatch("wl0_br1_nat", "1")
	    && nvram_invmatch("wl_br1_nat", "2"))
		save2file("-A OUTPUT -o br1 -j %s\n", log_accept);
	if (nvram_match("wl1_br1_enable", "1")
	    && nvram_invmatch("wl1_br1_nat", "1")
	    && nvram_invmatch("wl_br1_nat", "2"))
		save2file("-A OUTPUT -o br1 -j %s\n", log_accept);
#ifdef HAVE_VLANTAGGING
	add_bridges("OUTPUT", 0);
#endif
}

static void filter_forward(void)
{
	char *filter_web_hosts, *filter_web_urls;
	char *next;
	char dev[16];
	char var[80];

	char vifs[256];		// 
	
	int i=0;
	int filter_host_url = 0;
	
	
	while( i<15 && filter_host_url == 0)
	{
	  i++;
	  
	  filter_web_hosts = nvram_nget("filter_web_host%d", i);
	  filter_web_urls  = nvram_nget("filter_web_url%d", i);
	  
	  if (filter_web_hosts && strcmp(filter_web_hosts, "") || filter_web_urls && strcmp(filter_web_urls, "")) {
		filter_host_url = 1;
	  }
	}
	
	
	/*
	 * If webfilter is not used we can put this rule on top in order to increase WAN<->LAN throughput
	 */
	if(!filter_host_url)
		save2file("-A FORWARD -m state --state RELATED,ESTABLISHED -j %s\n", log_accept);
	
	if (nvram_match("dtag_vlan8", "1") && nvram_match("wan_vdsl", "1")) {
		save2file("-A FORWARD -i %s -j %s\n", nvram_safe_get("tvnicfrom"), log_accept);
		save2file("-A FORWARD -o %s -j %s\n", nvram_safe_get("tvnicfrom"), log_accept);
	}

	getIfLists(vifs, 256);
	// = nvram_safe_get ("lan_ifnames");
	// if (vifs != NULL)
	foreach(var, vifs, next) {
		if (strcmp(get_wan_face(), var)
		    && strcmp(nvram_safe_get("lan_ifname"), var)) {
			if (nvram_nmatch("0", "%s_bridged", var)
			    && nvram_nmatch("0", "%s_nat", var)) {
				save2file("-A FORWARD -i %s -j %s\n", var, log_accept);
			}
		}
	}

	/*
	 * Drop the wrong state, INVALID, packets 
	 */
	// save2file("-A FORWARD -m state --state INVALID -j DROP\n");

	/*
	 * Sveasoft add - log invalid packets 
	 */
	if (!has_gateway())
		save2file("-A FORWARD -m state --state INVALID -j %s\n", log_drop);

	/*
	 * Filter setting by user definition 
	 */
	// save2file ("-A FORWARD -i %s -j lan2wan\n", lanface);
	save2file("-A FORWARD -j lan2wan\n");

	/*
	 * Clamp TCP MSS to PMTU of WAN interface 
	 */
	save2file("-A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
	
	/*
	 * Filter Web application 
	 */
	// if (webfilter)
	// save2file ("-A FORWARD -i %s -o %s -p tcp --dport %d "
	// "-m webstr --content %d -j %s\n",
	// lanface, wanface, HTTP_PORT, webfilter, log_reject);
	if (webfilter && strlen(wanface)) {
		insmod("ipt_webstr");
		save2file("-A FORWARD -i %s -o %s -p tcp " "-m webstr --content %d -j %s\n", lanface, wanface, webfilter, log_reject);
	}

	/*
	 * If webfilter is used this rule must be evaluated after webstr rule
	 */
	if(filter_host_url)
		save2file("-A FORWARD -m state --state RELATED,ESTABLISHED -j %s\n", log_accept);

	/*
	 * Accept the redirect, might be seen as INVALID, packets 
	 */
	save2file("-A FORWARD -i %s -o %s -j %s\n", lanface, lanface, log_accept);

	/*
	 * Drop all traffic from lan 
	 */
	if (nvram_match("pptpd_lockdown", "1"))
		save2file("-A FORWARD -i %s -j %s\n", lanface, log_drop);


	/*
	 * Filter by destination ports "filter_port" if firewall on 
	 */
	if (nvram_invmatch("filter", "off"))
		parse_port_filter(nvram_safe_get("filter_port"));

	/*
	 * Sveasoft mods - accept OSPF protocol broadcasts 
	 */
	if (nvram_match("wk_mode", "ospf")) {
		save2file("-A FORWARD -p ospf -j %s\n", log_accept);
	}
	if (nvram_match("wk_mode", "bgp")) {
		save2file("-A FORWARD -p tcp --sport 179 -j %s\n", log_accept);	// BGP 
		// port
		save2file("-A FORWARD -p tcp --dport 179 -j %s\n", log_accept);	// BGP 
		// port
	}
#ifdef HAVE_OLSRD
	if (nvram_match("wk_mode", "olsr")) {
		save2file("-A FORWARD -p udp --dport 698 -j %s\n", log_accept);
		save2file("-A FORWARD -p udp --sport 698 -j %s\n", log_accept);
	}
#endif

	/*
	 * Sveasoft mod - FORWARD br1 to br0, protecting br0 
	 */
	if (nvram_match("wl0_br1_enable", "1")) {

		if (nvram_match("wl0_br1_nat", "1")) {
			save2file("-A FORWARD -i br0 -o br1 -j %s\n", log_accept);
			save2file("-A FORWARD -o br0 -i br1 -m state --state ESTABLISHED,RELATED -j %s\n", log_accept);
		}

		/*
		 * Sveasoft mod - FORWARD br0 to br1, protecting br1 
		 */
		else if (nvram_match("wl0_br1_nat", "2")) {
			save2file("-A FORWARD -o br0 -i br1 -j %s\n", log_accept);
			save2file("-A FORWARD -i br0 -o br1 -m state --state ESTABLISHED,RELATED -j %s\n", log_accept);
		}
		/*
		 * Sveasoft mod - default for br1/separate subnet WDS type 
		 */
		else
			save2file("-A FORWARD -i br1 -o br0 -j %s\n", log_accept);

		char *wan = get_wan_face();
		if (wan && strlen(wan))
			save2file("-A FORWARD -i br1 -o %s -j %s\n", wan, log_accept);

	}
#ifdef HAVE_VLANTAGGING
	add_bridges("FORWARD", 1);
#endif
	stop_vpn_modules();
	// unload_vpn_modules ();

	if (nvram_invmatch("filter", "off") && strlen(wanface)) {

		/*
		 * DROP packets for PPTP pass through. 
		 */
		if (nvram_match("pptp_pass", "0"))
			save2file("-A FORWARD -o %s -p tcp --dport %d -j %s\n", wanface, PPTP_PORT, log_drop);

		/*
		 * DROP packets for L2TP pass through. 
		 */
		if (nvram_match("l2tp_pass", "0"))
			save2file("-A FORWARD -o %s -p udp --dport %d -j %s\n", wanface, L2TP_PORT, log_drop);

		/*
		 * DROP packets for IPsec pass through 
		 */
		if (nvram_match("ipsec_pass", "0"))
			save2file("-A FORWARD -o %s -p udp --dport %d -j %s\n", wanface, ISAKMP_PORT, log_drop);

	}
	start_vpn_modules();
	// load_vpn_modules ();
	if (nvram_invmatch("filter", "off")) {
		if (nvram_match("pptp_pass", "1")) {
			if (strlen(wanface)) {
				save2file("-I FORWARD -o %s -s %s/%d -p tcp --dport %d -j %s\n", wanface, nvram_safe_get("lan_ipaddr"), getmask(nvram_safe_get("lan_netmask")), PPTP_PORT, log_accept);
				save2file("-I FORWARD -o %s -s %s/%d -p gre -j %s\n", wanface, nvram_safe_get("lan_ipaddr"), getmask(nvram_safe_get("lan_netmask")), log_accept);
			}
		}
	}
	/*
	 * ACCEPT packets for Multicast pass through 
	 */
	if (nvram_match("dtag_vlan8", "1") && nvram_match("wan_vdsl", "1")) {
		if (doMultiCast() > 0)
			save2file("-A FORWARD -i %s -p udp --destination %s -j %s\n", nvram_safe_get("tvnicfrom"), IP_MULTICAST, log_accept);
#ifdef HAVE_PPTP
	} else if (nvram_match("wan_proto", "pptp") && nvram_match("pptp_iptv", "1") && nvram_get("tvnicfrom")) {
		if (doMultiCast() > 0)
			save2file("-A FORWARD -i %s -p udp --destination %s -j %s\n", nvram_safe_get("tvnicfrom"), IP_MULTICAST, log_accept);
#endif
#ifdef HAVE_L2TP
	} else if (nvram_match("wan_proto", "l2tp") && nvram_match("pptp_iptv", "1") && nvram_get("tvnicfrom")) {
		if (doMultiCast() > 0)
			save2file("-A FORWARD -i %s -p udp --destination %s -j %s\n", nvram_safe_get("tvnicfrom"), IP_MULTICAST, log_accept);
#endif
	} else {
		if (doMultiCast() > 0 && strlen(wanface))
			save2file("-A FORWARD -i %s -p udp --destination %s -j %s\n", wanface, IP_MULTICAST, log_accept);
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
	if (strlen(wanface)) {
		save2file("-A FORWARD -i %s -o %s -j TRIGGER --trigger-type in\n", wanface, lanface);
		save2file("-A FORWARD -i %s -j trigger_out\n", lanface);
	}
	/*
	 * DMZ forwarding 
	 */
	if (dmzenable)
		save2file("-A FORWARD -o %s -d %s%s -j %s\n", lanface, lan_cclass, nvram_safe_get("dmz_ipaddr"), log_accept);

	/*
	 * Accept new connections 
	 */
	save2file("-A FORWARD -i %s -m state --state NEW -j %s\n", lanface, log_accept);
	/*
	 * ...otherwise drop if firewall on 
	 */
	if (nvram_invmatch("filter", "off"))
		save2file("-A FORWARD -j %s\n", log_drop);

	lan2wan_chains();

	parse_trigger_out(nvram_safe_get("port_trigger"));
}

/*
 *      Mangle table
 */
static void mangle_table(void)
{
	save2file("*mangle\n" ":PREROUTING ACCEPT [0:0]\n" ":OUTPUT ACCEPT [0:0]\n");

	if (wanactive() && nvram_match("block_loopback", "0")) {
		insmod("ipt_mark");
		insmod("xt_mark");
		insmod("ipt_CONNMARK");
		insmod("xt_CONNMARK");

		save2file("-A PREROUTING -i ! %s -d %s -j MARK --set-mark %s\n", get_wan_face(), get_wan_ipaddr(), get_NFServiceMark("FORWARD", 1));

		save2file("-A PREROUTING -j CONNMARK --save\n");
	}
	/*
	 * Sveasoft add - avoid the "mark everything" rule, Reformed's PPPoE code 
	 * should take care of this 
	 */
	/*
	 * For PPPoE Connect On Demand, to reset idle timer. add by honor
	 * (2003-04-17) Reference driver/net/ppp_generic.c 
	 */
	// save2file("-A PREROUTING -i %s -m mark ! --mark 0 -j MARK --set-mark
	// %d\n", lanface, MARK_LAN2WAN);
	save2file("COMMIT\n");
}

/*
 *      NAT table
 */
static void nat_table(void)
{
	save2file("*nat\n" ":PREROUTING ACCEPT [0:0]\n" ":POSTROUTING ACCEPT [0:0]\n" ":OUTPUT ACCEPT [0:0]\n");
	if (wanactive()) {
		nat_prerouting();
		nat_postrouting();
	}
	save2file("COMMIT\n");
}

/*
 *      Filter table
 */
static void filter_table(void)
{
	save2file("*filter\n" ":INPUT ACCEPT [0:0]\n" ":FORWARD ACCEPT [0:0]\n" ":OUTPUT ACCEPT [0:0]\n" ":logaccept - [0:0]\n" ":logdrop - [0:0]\n" ":logreject - [0:0]\n"
#ifdef FLOOD_PROTECT
		  ":limaccept - [0:0]\n"
#endif
		  ":trigger_out - [0:0]\n" ":lan2wan - [0:0]\n");

	int seq;

	for (seq = 1; seq <= NR_RULES; seq++) {
		save2file(":grp_%d - [0:0]\n", seq);
		save2file(":advgrp_%d - [0:0]\n", seq);
	}
#ifndef HAVE_MICRO
	if( (nvram_match("limit_telnet", "1")) || (nvram_match("limit_pptp", "1")) || (nvram_match("limit_ssh", "1")) || (nvram_match("limit_ftp", "1")) ){
		save2file(":logbrute - [0:0]\n");
		save2file("-A logbrute -m recent --set --name BRUTEFORCE --rsource\n");
		save2file("-A logbrute -m recent ! --update --seconds 60 --hitcount 4 --name BRUTEFORCE --rsource -j RETURN\n");
		// -m limit rule is a fallback in case -m recent isn't included in a build
		save2file("-A logbrute -m limit --limit 1/min --limit-burst 1 -j RETURN\n");
		if ((nvram_match("log_enable", "1"))
		    && (nvram_match("log_dropped", "1")))
			save2file("-A logbrute -j LOG --log-prefix \"[DROP BRUTEFORCE] : \" --log-tcp-options --log-ip-options\n");
		save2file("-A logbrute -j %s\n", log_drop);
	}
#endif

	if (wanactive()) {
		/*
		 * Does it disable the filter? 
		 */
		if (nvram_match("filter", "off")
		    || !has_gateway()) {

			/*
			 * Make sure remote management ports are filtered if it is disabled 
			 */
			if (!remotemanage && strlen(wanface)) {
				save2file("-A INPUT -p tcp -i %s --dport %s -j %s\n", wanface, nvram_safe_get("http_wanport"), log_drop);
				save2file("-A INPUT -p tcp -i %s --dport 80 -j %s\n", wanface, log_drop);
				save2file("-A INPUT -p tcp -i %s --dport 443 -j %s\n", wanface, log_drop);
				save2file("-A INPUT -p tcp -i %s --dport 69 -j %s\n", wanface, log_drop);
			}
			/*
			 * Make sure remote ssh/telnet port is filtered if it is disabled :
			 * Botho 03-05-2006 
			 */
#ifdef HAVE_SSHD
			if (!remotessh && strlen(wanface) > 0) {
				save2file("-A INPUT -i %s -p tcp --dport %s -j %s\n", wanface, nvram_safe_get("sshd_port"), log_drop);
			}
#endif

#ifdef HAVE_TELNET
			if (!remotetelnet && strlen(wanface) > 0) {
				save2file("-A INPUT -p tcp -i %s --dport 23 -j %s\n", wanface, log_drop);
			}
#endif
			filter_forward();
		} else {

			filter_input();
			filter_output();
			filter_forward();
		}
	}

	/*
	 * logaccept chain 
	 */
#ifdef FLOOD_PROTECT
	if ((nvram_match("log_enable", "1"))
	    && (nvram_match("log_accepted", "1")))
		save2file("-A logaccept -i %s -m state --state NEW -m limit --limit %d -j LOG " "--log-prefix \"FLOOD \" --log-tcp-sequence --log-tcp-options --log-ip-options\n", wanface, FLOOD_RATE);
	save2file("-A logaccept -i %s -m state --state NEW -m limit --limit %d -j %s\n", wanface, FLOOD_RATE, log_drop);
#endif
	if ((nvram_match("log_enable", "1"))
	    && (nvram_match("log_accepted", "1")))
		save2file("-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" " "--log-tcp-sequence --log-tcp-options --log-ip-options\n");
	save2file("-A logaccept -j ACCEPT\n");

	/*
	 * logdrop chain 
	 */
	if (has_gateway()) {
		if ((nvram_match("log_enable", "1"))
		    && (nvram_match("log_dropped", "1")))
			save2file
			    ("-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" "
			     "--log-tcp-sequence --log-tcp-options --log-ip-options\n" "-A logdrop -m state --state INVALID -j LOG --log-prefix \"DROP \" " "--log-tcp-sequence --log-tcp-options --log-ip-options\n");
	} else {
		if ((nvram_match("log_enable", "1"))
		    && (nvram_match("log_dropped", "1")))
			save2file("-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" " "--log-tcp-sequence --log-tcp-options --log-ip-options\n");
	}
	save2file("-A logdrop -j DROP\n");

	/*
	 * logreject chain 
	 */
	if ((nvram_match("log_enable", "1"))
	    && (nvram_match("log_rejected", "1")))
		save2file("-A logreject -j LOG --log-prefix \"WEBDROP \" " "--log-tcp-sequence --log-tcp-options --log-ip-options\n");
	save2file("-A logreject -p tcp -j REJECT --reject-with tcp-reset\n");

#ifdef FLOOD_PROTECT
	/*
	 * limaccept chain 
	 */
	if ((nvram_match("log_enable", "1"))
	    && (nvram_match("log_accepted", "1")))
		save2file("-A limaccept -i %s -m state --state NEW -m limit --limit %d -j LOG " "--log-prefix \"FLOOD \" --log-tcp-sequence --log-tcp-options --log-ip-options\n");
	save2file("-A limaccept -i %s -m state --state NEW -m limit --limit %d -j %s\n" "-A limaccept -j ACCEPT\n", wanface, FLOOD_RATE, wanface, FLOOD_RATE, log_drop);
#endif

	save2file("COMMIT\n");
}

static void create_restore_file(void)
{
	mangle_table();
	nat_table();
	filter_table();
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
	if (nvram_get(gport)) {
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
	system("cat /proc/net/ip_conntrack_flush");

#ifndef	HAVE_80211AC
	/*
	 * Improve WAN<->LAN Performance on K26
	 */
	system("echo 120 > /proc/sys/net/core/netdev_max_backlog");
#endif	
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
	log_level = atoi(nvram_safe_get("log_level"));
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
		if (strlen(nvram_safe_get("pptp_get_ip")) == 0)	// for initial dhcp ip
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
	webfilter = 0;		/* Reset, clear the late setting */
	if (nvram_match("block_cookie", "1"))
		webfilter |= BLK_COOKIE;
	if (nvram_match("block_java", "1"))
		webfilter |= BLK_JAVA;
	if (nvram_match("block_activex", "1"))
		webfilter |= BLK_ACTIVE;
	if (nvram_match("block_proxy", "1"))
		webfilter |= BLK_PROXY;

	/*
	 * Run DMZ forwarding ? 
	 */
	if (has_gateway()
	    && nvram_match("dmz_enable", "1")
	    && nvram_invmatch("dmz_ipaddr", "")
	    && nvram_invmatch("dmz_ipaddr", "0"))
		dmzenable = 1;
	else
		dmzenable = 0;

	/*
	 * Remote Web GUI management 
	 */
	if (nvram_match("remote_management", "1") && nvram_invmatch("http_wanport", "") && nvram_invmatch("http_wanport", "0"))
		remotemanage = 1;
	else
		remotemanage = 0;

#ifdef HAVE_SSHD
	/*
	 * Remote ssh management : Botho 03-05-2006 
	 */
	if (nvram_match("remote_mgt_ssh", "1") && nvram_invmatch("sshd_wanport", "") && nvram_invmatch("sshd_wanport", "0") && nvram_match("sshd_enable", "1"))
		remotessh = 1;
	else
		remotessh = 0;
#endif

#ifdef HAVE_TELNET
	/*
	 * Remote telnet management 
	 */
	if (nvram_match("remote_mgt_telnet", "1") && nvram_invmatch("telnet_wanport", "") && nvram_invmatch("telnet_wanport", "0") && nvram_match("telnetd_enable", "1"))
		remotetelnet = 1;
	else
		remotetelnet = 0;
#endif

#ifdef HAVE_HTTPS
	if (nvram_match("remote_mgt_https", "1"))
		web_lanport = HTTPS_PORT;
	else
#endif
		web_lanport = atoi(nvram_safe_get("http_lanport")) ? : HTTP_PORT;

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
	create_restore_file();

#ifndef DEVELOPE_ENV
	/*
	 * Insert the rules into kernel 
	 */
	DEBUG("start firewall()........5\n");
	eval("iptables-restore", IPTABLES_SAVE_FILE);

	// unlink(IPTABLES_SAVE_FILE);
#endif

	/*
	 * begin Sveasoft add 
	 */
	/*
	 * run rc_firewall script 
	 */
	cprintf("Exec RC Filewall\n");
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
	{
		runStartup("/jffs/etc/config", ".prewall");	// if available
		runStartup("/mmc/etc/config", ".prewall");	// if available
		runStartup("/tmp/etc/config", ".prewall");	// if available
		create_rc_file(RC_FIREWALL);
		if (f_exists("/tmp/.rc_firewall")) {
			setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
			system("/tmp/.rc_firewall");
		}
		runStartup("/etc/config", ".firewall");
		runStartup("/sd/etc/config", ".firewall");
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
	if (dmzenable)
		diag_led(DMZ, START_LED);
	else
		diag_led(DMZ, STOP_LED);
	cprintf("done");
#ifdef XBOX_SUPPORT
	writeproc("/proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout", "65");
	writeproc("/proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeouts", "65 180");
#endif
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
	if (nvram_match("ipv6_enable", "1")) {
		writeproc("/proc/sys/net/ipv6/conf/all/forwarding", "1");
	}
#ifdef HAVE_GGEW
	char *wordlist = nvram_safe_get("ral");
	char var[256], *next;

	foreach(var, wordlist, next) {
		sysprintf("iptables -I INPUT -s %s -j %s", var, log_accept);
	}
#endif
#ifdef HAVE_IAS
	if (nvram_match("ias_startup", "3"))
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
	if (nvram_match("wd_enable", "1")) {
		stop_wifidog();
		start_wifidog();
	}
#endif
#ifdef HAVE_CHILLI
	if (nvram_match("chilli_enable", "1")
	    || nvram_match("hotss_enable", "1")) {
		stop_chilli();
		start_chilli();
	}
#endif
#ifdef HAVE_PPPOESERVER
	if (nvram_match("pppoeserver_enabled", "1")) {
		stop_pppoeserver();
		start_pppoeserver();
	}
#endif

	cprintf("ready");

	cprintf("done\n");
}

void stop_firewall(void)
{
	eval("iptables", "-t", "raw", "-F");
//      stop_anchorfree();
	/*
	 * Make sure the DMZ-LED is off (from service.c) 
	 */
	diag_led(DMZ, STOP_LED);
#ifdef HAVE_GGEW
	char *wordlist = nvram_safe_get("ral");
	char var[256], *next;

	foreach(var, wordlist, next) {
		sysprintf("iptables -D INPUT -s %s -j %s", var, log_accept);
	}
#endif
	char num[32];
	int i;

	for (i = 0; i < 10; i++) {
		eval("iptables", "-F", "lan2wan");
		sprintf(num, "grp_%d", i);
		eval("iptables", "-F", num);
		sprintf(num, "advgrp_%d", i);
		eval("iptables", "-F", num);
	}
	rmmod("ipt_webstr");
	rmmod("ipt_layer7");
	rmmod("xt_layer7");
	rmmod("ipt_ipp2p");
	rmmod("xt_ipp2p");
	if (nvram_invmatch("apd_enable", "0")) {
		rmmod("ipt_mark");
		rmmod("xt_mark");
	}
	if (nvram_invmatch("apd_enable", "0")) {
		rmmod("ipt_mac");
		rmmod("xt_mac");
	}
	cprintf("done\n");
	return;
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
