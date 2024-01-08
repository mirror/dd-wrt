/*
 * validators.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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

#define VALIDSOURCE 1

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif /* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <dd_defs.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>

//debug
//#include <syslog.h>

#ifdef FILTER_DEBUG
extern FILE *debout;

#define D(a)                        \
	fprintf(debout, "%s\n", a); \
	fflush(debout);
#else
#define D(a)
#endif

/*void validate_cgi(webs_t fp)
{
	fp->p->env->validate_cgi(fp);
}

char *validate_websGetVar(webs_t wp, char *var, char *d)
{
	return wp->p->env->websGetVar(wp, var, d);
}

int validate_websGetVari(webs_t wp, char *var, int d)
{
	return wp->p->env->websGetVari(wp, var, d);
}
*/
char *copytonv(webs_t wp, const char *fmt, ...);
char *copytonv_prefix(webs_t wp, const char *var, const char *prefix);

char *copymergetonv(webs_t wp, const char *fmt, ...);

/*
 * Example: ISASCII("", 0); return true; ISASCII("", 1); return false;
 * ISASCII("abc123", 1); return true; 
 */
int ISASCII(char *value, int flag, int unwanted)
{
	int i, tag = TRUE;

#if COUNTRY == JAPAN
	return tag; // don't check for japan version
#endif

	if (!*value) {
		if (flag)
			return FALSE; // null
		else
			return TRUE;
	}
	if (unwanted && strpbrk(value, "'\"´`")) { // filter some unwanted characters
		return FALSE;
	}
	for (i = 0; *(value + i); i++) {
		if (!isascii(*(value + i))) {
			tag = FALSE;
			break;
		}
	}
	return tag;
}

/*
 * Example: legal_hwaddr("00:11:22:33:44:aB"); return true;
 * legal_hwaddr("00:11:22:33:44:5"); return false;
 * legal_hwaddr("00:11:22:33:44:HH"); return false; 
 */
int legal_hwaddr(char *value)
{
	unsigned int hwaddr[6];
	int tag = TRUE;
	int i, count;

	/*
	 * Check for bad, multicast, broadcast, or null address 
	 */
	for (i = 0, count = 0; *(value + i); i++) {
		if (*(value + i) == ':') {
			if ((i + 1) % 3 != 0) {
				tag = FALSE;
				break;
			}
			count++;
		} else if (isxdigit(*(value + i))) /* one of 0 1 2 3 4 5 6 7 8 9 
							 * a b c d e f A B C D E F */
			continue;
		else {
			tag = FALSE;
			break;
		}
	}

	if (!tag || i != 17 || count != 5) /* must have 17's characters and 5's
						 * ':' */
		tag = FALSE;
	else if (sscanf(value, "%x:%x:%x:%x:%x:%x", &hwaddr[0], &hwaddr[1], &hwaddr[2], &hwaddr[3], &hwaddr[4], &hwaddr[5]) != 6) {
		// (hwaddr[0] & 1) || // the bit 7 is 1
		// (hwaddr[0] & hwaddr[1] & hwaddr[2] & hwaddr[3] & hwaddr[4] &
		// hwaddr[5]) == 0xff ){ // FF:FF:FF:FF:FF:FF
		// (hwaddr[0] | hwaddr[1] | hwaddr[2] | hwaddr[3] | hwaddr[4] |
		// hwaddr[5]) == 0x00){ // 00:00:00:00:00:00
		tag = FALSE;
	} else
		tag = TRUE;

	return tag;
}

/*
 * Example: 255.255.255.0 (111111111111111111111100000000) is a legal netmask
 * 255.255.0.255 (111111111111110000000011111111) is an illegal netmask 
 */
int legal_netmask(char *value)
{
	struct in_addr ipaddr;
	int ip[4] = { 0, 0, 0, 0 };
	int i, j;
	int match0 = -1;
	int match1 = -1;
	int ret, tag;

	ret = sscanf(value, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

	if (ret == 4 && inet_aton(value, &ipaddr)) {
		for (i = 3; i >= 0; i--) {
			for (j = 1; j <= 8; j++) {
				if ((ip[i] % 2) == 0)
					match0 = (3 - i) * 8 + j;
				else if (((ip[i] % 2) == 1) && match1 == -1)
					match1 = (3 - i) * 8 + j;
				ip[i] = ip[i] / 2;
			}
		}
	}

	if (match0 >= match1)
		tag = FALSE;
	else
		tag = TRUE;

	return tag;
}

/*
 * Example: legal_ipaddr("192.168.1.1"); return true;
 * legal_ipaddr("192.168.1.1111"); return false; 
 */
int legal_ipaddr(char *value)
{
	struct in_addr ipaddr;
	int ip[4];
	int ret, tag;

	ret = sscanf(value, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

	if (ret != 4 || !inet_aton(value, &ipaddr))
		tag = FALSE;
	else
		tag = TRUE;

	return tag;
}

int valid_wep_key(webs_t wp, char *value, struct variable *v)
{
	int i;

	switch (*(value)) {
	case 5:
	case 13:
		for (i = 0; *(value + i); i++) {
			if (isascii(*(value + i))) {
				continue;
			} else {
				websDebugWrite(wp, "Invalid <b>%s</b> %s: must be ascii code<br>", v->longname, value);
				return FALSE;
			}
		}
		break;
	case 10:
	case 26:
		for (i = 0; *(value + i); i++) {
			if (isxdigit(*(value + i))) { /* one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B 
							 * C D E F */
				continue;
			} else {
				websDebugWrite(wp, "Invalid <b>%s</b> %s: must be hexadecimal digits<br>", v->longname, value);
				return FALSE;
			}
		}
		break;

	default:
		websDebugWrite(wp, "Invalid <b>%s</b>: must be 5 or 13 ASCII characters or 10 or 26 hexadecimal digits<br>",
			       v->longname);
		return FALSE;
	}

	/*
	 * for(i=0 ; *(value+i) ; i++){ if(isxdigit(*(value+i))){ continue; }
	 * else{ websDebugWrite(wp, "Invalid <b>%s</b> %s: must be hexadecimal
	 * digits<br>", v->longname, value); return FALSE; } }
	 * 
	 * if (i != length) { websDebugWrite(wp, "Invalid <b>%s</b> %s: must be
	 * %d characters<br>", v->longname, value,length); return FALSE; } 
	 */
	return TRUE;
}

EJ_VISIBLE void validate_statics(webs_t wp, char *value, struct variable *v)
{
	if (!sv_valid_statics(value)) {
		websDebugWrite(wp, "Invalid <b>%s</b> %s: not a legal statics entry<br>", v->longname, value);
		return;
	}

	nvram_set(v->name, value);
}

int valid_netmask(webs_t wp, char *value, struct variable *v)
{
	if (!legal_netmask(value)) {
		websDebugWrite(wp, "Invalid <b>%s</b> %s: not a legal netmask<br>", v->longname, value);
		return FALSE;
	}

	return TRUE;
}

EJ_VISIBLE void validate_netmask(webs_t wp, char *value, struct variable *v)
{
	if (valid_netmask(wp, value, v))
		nvram_set(v->name, value);
}

EJ_VISIBLE void validate_merge_netmask(webs_t wp, char *value, struct variable *v)
{
	char netmask[20], maskname[30];
	char *mask;
	int i;

	strcpy(netmask, "");
	for (i = 0; i < 4; i++) {
		snprintf(maskname, sizeof(maskname), "%s_%d", v->name, i);
		mask = websGetVar(wp, maskname, NULL);
		if (mask) {
			strcat(netmask, mask);
			if (i < 3)
				strcat(netmask, ".");
		} else {
			return;
		}
	}

	if (valid_netmask(wp, netmask, v))
		nvram_set(v->name, netmask);
}

// Added by Daniel(2004-07-29) for EZC
// char webs_buf[5000];
// int webs_buf_offset = 0;

static void validate_list(webs_t wp, char *value, struct variable *v, int (*valid)(webs_t, char *, struct variable *))
{
	int n, i;
	char name[100];
	char buf[1000] = "", *cur = buf;

	n = atoi(value);

	for (i = 0; i < n; i++) {
		snprintf(name, sizeof(name), "%s%d", v->name, i);
		if (!(value = websGetVar(wp, name, NULL)))
			return;
		if (!*value && v->nullok)
			continue;
		if (!valid(wp, value, v))
			continue;
		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s", cur == buf ? "" : " ", value);
	}
	nvram_set(v->name, buf);
}

int valid_ipaddr(webs_t wp, char *value, struct variable *v)
{
	struct in_addr netaddr, netmask;

	if (!legal_ipaddr(value)) {
		websDebugWrite(wp, "Invalid <b>%s</b> %s: not an IP address<br>", v->longname, value);
		return FALSE;
	}

	if (v->argv) {
		if (!strcmp(v->argv[0], "lan")) {
			if (*(value + strlen(value) - 2) == '.' && *(value + strlen(value) - 1) == '0') {
				websDebugWrite(wp, "Invalid <b>%s</b> %s: not an IP address<br>", v->longname, value);
				return FALSE;
			}
		}

		else if (!legal_ip_netmask(v->argv[0], v->argv[1], value)) {
			(void)inet_aton(nvram_safe_get(v->argv[0]), &netaddr);
			(void)inet_aton(nvram_safe_get(v->argv[1]), &netmask);
			return FALSE;
		}
	}

	return TRUE;
}

EJ_VISIBLE void validate_ipaddr(webs_t wp, char *value, struct variable *v)
{
	if (valid_ipaddr(wp, value, v))
		nvram_set(v->name, value);
}

EJ_VISIBLE void validate_ipaddrs(webs_t wp, char *value, struct variable *v)
{
	validate_list(wp, value, v, valid_ipaddr);
}

int valid_merge_ip_4(webs_t wp, char *value, struct variable *v)
{
	char ipaddr[20];

	if (atoi(value) == 255) {
		websDebugWrite(wp, "Invalid <b>%s</b> %s: out of range 0 - 254 <br>", v->longname, value);
		return FALSE;
	}

	sprintf(ipaddr, "%d.%d.%d.%s", get_single_ip(nvram_safe_get("lan_ipaddr"), 0),
		get_single_ip(nvram_safe_get("lan_ipaddr"), 1), get_single_ip(nvram_safe_get("lan_ipaddr"), 2), value);

	if (!valid_ipaddr(wp, ipaddr, v)) {
		return FALSE;
	}

	return TRUE;
}

/*
 * static void validate_merge_ip_4 (webs_t wp, char *value, struct variable
 * *v) { if (!strcmp (value, "")) { nvram_set (v->name, "0"); return; }
 * 
 * if (valid_merge_ip_4 (wp, value, v)) nvram_set (v->name, value); } 
 */

/*
 * Example: lan_ipaddr_0 = 192 lan_ipaddr_1 = 168 lan_ipaddr_2 = 1
 * lan_ipaddr_3 = 1 get_merge_ipaddr("lan_ipaddr", ipaddr); produces
 * ipaddr="192.168.1.1" 
 */
int get_merge_ipaddr(webs_t wp, char *name, char *ipaddr, char *value, char *netmask)
{
	char ipname[30];
	int i;
	char buf[50] = { 0 };
	char *ip[4];
	char *tmp;

	// cprintf("ip addr\n");
	strcpy(ipaddr, "");
	// cprintf("safe get\n");
	char *ipa = value;
	if (!ipa)
		ipa = nvram_safe_get(name);

	// cprintf("strcpy\n");
	if (ipa == NULL)
		strcpy(buf, "0.0.0.0");
	else
		strcpy(buf, ipa);
	// cprintf("strsep\n");
	char *b = (char *)&buf;
	ip[0] = strsep(&b, ".");
	ip[1] = strsep(&b, ".");
	ip[2] = strsep(&b, ".");
	ip[3] = b;
	unsigned int _ip;
	unsigned int _nm;
	if (netmask && *b) {
		char *nm[4];
		char _n[50];
		char *n = (char *)&_n;
		strcpy(n, netmask);
		nm[0] = strsep(&n, ".");
		nm[1] = strsep(&n, ".");
		nm[2] = strsep(&n, ".");
		nm[3] = n;
		_ip = (atoi(ip[0]) << 24) | (atoi(ip[1]) << 16) | (atoi(ip[2]) << 8) | atoi(ip[3]);
		_nm = (atoi(nm[0]) << 24) | (atoi(nm[1]) << 16) | (atoi(nm[2]) << 8) | atoi(nm[3]);
	}
	unsigned int target = 0;
	for (i = 0; i < 4; i++) {
		// cprintf("merge %s_%d\n",name,i);
		snprintf(ipname, sizeof(ipname), "%s_%d", name, i);
		tmp = websGetVar(wp, ipname, ip[i]);
		if (tmp == NULL)
			return 0;
		target |= atoi(tmp) << ((3 - i) * 8);
	}
	if (netmask && *b) {
		target &= ~_nm;
		_ip &= _nm;
		target |= _ip;
	}

	for (i = 0; i < 4; i++) {
		char t[50];
		sprintf(t, "%d", (target >> ((3 - i) * 8)) & 0xff);
		strcat(ipaddr, t);
		if (i < 3)
			strcat(ipaddr, ".");
	}
	return 1;
}

char *cidr_to_nm(char *netmask, size_t len, unsigned int netmask_cidr)
{
	strcpy(netmask, "");
	unsigned int nm =
		(((unsigned long long)1 << 32) -
		 ((unsigned long long)((unsigned long long)1 << ((unsigned long long)32 - (unsigned long long)netmask_cidr))));
	snprintf(netmask, len, "%d.%d.%d.%d", (nm >> 24) & 0xff, (nm >> 16) & 0xff, (nm >> 8) & 0xff, nm & 0xff);
	return netmask;
}

int get_merge_ipaddr_cidr(webs_t wp, char *name, char *ipaddr, char *value, int netmask_cidr)
{
	char netmask[20];
	return get_merge_ipaddr(wp, name, ipaddr, value, cidr_to_nm(netmask, sizeof(netmask), netmask_cidr));
}

EJ_VISIBLE void validate_merge_ipaddrs(webs_t wp, char *value, struct variable *v)
{
	char ipaddr[20];

	get_merge_ipaddr(wp, v->name, ipaddr, NULL, NULL);

	if (valid_ipaddr(wp, ipaddr, v))
		nvram_set(v->name, ipaddr);
}

EJ_VISIBLE void validate_merge_dhcpstart(webs_t wp, char *value, struct variable *v)
{
	char ipaddr[20];

	char *tmp = websGetVar(wp, "lan_proto", "static");
	if (!strcmp(tmp, "static"))
		get_merge_ipaddr(wp, v->name, ipaddr, nvram_safe_get("dhcp_start"), nvram_safe_get("lan_netmask"));
	else
		get_merge_ipaddr(wp, v->name, ipaddr, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

	if (valid_ipaddr(wp, ipaddr, v))
		nvram_set(v->name, ipaddr);
}

/*
 * Example: wan_mac_0 = 00 wan_mac_1 = 11 wan_mac_2 = 22 wan_mac_3 = 33
 * wan_mac_4 = 44 wan_mac_5 = 55 get_merge_mac("wan_mac",mac); produces
 * mac="00:11:22:33:44:55" 
 */
int get_merge_mac(webs_t wp, char *name, char *macaddr)
{
	char macname[30];
	char *mac;
	int i;

	strcpy(macaddr, "");

	for (i = 0; i < 6; i++) {
		snprintf(macname, sizeof(macname), "%s_%d", name, i);
		mac = websGetVar(wp, macname, "00");
		if (strlen(mac) == 1)
			strcat(macaddr, "0");
		strcat(macaddr, mac);
		if (i < 5)
			strcat(macaddr, ":");
	}

	return 1;
}

EJ_VISIBLE void validate_merge_mac(webs_t wp, char *value, struct variable *v)
{
	char macaddr[20];

	get_merge_mac(wp, v->name, macaddr);

	if (valid_hwaddr(wp, macaddr, v))
		nvram_set(v->name, macaddr);
}

EJ_VISIBLE void validate_dns(webs_t wp, char *value, struct variable *v)
{
	char buf[100] = "", *cur = buf;
	char ipaddr[20], ipname[30];
	char *ip;
	int i, j;

	for (j = 0; j < 3; j++) {
		strcpy(ipaddr, "");
		for (i = 0; i < 4; i++) {
			snprintf(ipname, sizeof(ipname), "%s%d_%d", v->name, j, i);
			ip = websGetVar(wp, ipname, NULL);
			if (ip) {
				strcat(ipaddr, ip);
				if (i < 3)
					strcat(ipaddr, ".");
			} else
				return;
		}

		if (!strcmp(ipaddr, "0.0.0.0"))
			continue;
		if (!valid_ipaddr(wp, ipaddr, v))
			continue;
		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s", cur == buf ? "" : " ", ipaddr);
	}
	nvram_set(v->name, buf);

	dns_to_resolv();
}

int valid_choice(webs_t wp, char *value, struct variable *v)
{
	char **choice;

	for (choice = v->argv; *choice; choice++) {
		if (!strcmp(value, *choice))
			return TRUE;
	}

	websDebugWrite(wp, "Invalid <b>%s</b> %s: not one of ", v->longname, value);
	for (choice = v->argv; *choice; choice++)
		websDebugWrite(wp, "%s%s", choice == v->argv ? "" : "/", *choice);
	websDebugWrite(wp, "<br>");
	return FALSE;
}

EJ_VISIBLE void validate_choice(webs_t wp, char *value, struct variable *v)
{
	if (valid_choice(wp, value, v))
		nvram_set(v->name, value);
}

EJ_VISIBLE void validate_noack(webs_t wp, char *value, struct variable *v)
{
	char *wme;

	/*
	 * return if wme is not enabled 
	 */
	if (!(wme = websGetVar(wp, "wl_wme", NULL)))
		return;
	else if (strcmp(wme, "on"))
		return;

	validate_choice(wp, value, v);
}

int valid_range(webs_t wp, char *value, struct variable *v)
{
	int n, start, end;

	n = atoi(value);
	start = atoi(v->argv[0]);
	end = atoi(v->argv[1]);

	if (!ISDIGIT(value, 1) || n < start || n > end) {
		websDebugWrite(wp, "Invalid <b>%s</b> %s: out of range %d-%d<br>", v->longname, value, start, end);
		return FALSE;
	}

	return TRUE;
}

EJ_VISIBLE void validate_range(webs_t wp, char *value, struct variable *v)
{
	char buf[20];
	int range;

	if (valid_range(wp, value, v)) {
		range = atoi(value);
		snprintf(buf, sizeof(buf), "%d", range);
		nvram_set(v->name, buf);
	}
}

int valid_name(webs_t wp, char *value, struct variable *v, int unwanted)
{
	int n, max;

	n = atoi(value);

	if (!ISASCII(value, 1, unwanted)) {
		return FALSE;
	}
	if (v) {
		max = atoi(v->argv[0]);
		if (strlen(value) > max) {
			return FALSE;
		}
	}
	return TRUE;
}

EJ_VISIBLE void validate_name(webs_t wp, char *value, struct variable *v)
{
	if (valid_name(wp, value, v, 1))
		nvram_set(v->name, value);
}

EJ_VISIBLE void validate_reboot(webs_t wp, char *value, struct variable *v)
{
	if (value && v) {
		nvram_set(v->name, value);
		nvram_seti("do_reboot", 1);
	}
}

/*
 * the html always show "d6nw5v1x2pc7st9m" so we must filter it. 
 */
EJ_VISIBLE void validate_password(webs_t wp, char *value, struct variable *v)
{
#ifdef HAVE_IAS
	if (!strcmp(value, "http_username")) {
		nvram_set("http_userpln", value);
		fprintf(stderr, "[SET USERNAME] %s\n", value);
	} else if (!strcmp(value, "http_passwd")) {
		nvram_set("http_pwdpln", value);
		fprintf(stderr, "[SET PASSWORD] %s\n", value);
	}
#endif
	if (strcmp(value, TMP_PASSWD) && valid_name(wp, value, v, 0)) {
		char passout[MD5_OUT_BUFSIZE];
		nvram_set(v->name, zencrypt(value, passout));

		eval("/sbin/setpasswd");
	}
}

EJ_VISIBLE void validate_password2(webs_t wp, char *value, struct variable *v)
{
	if (strcmp(value, TMP_PASSWD) && valid_name(wp, value, v, 0)) {
		nvram_set(v->name, value);
	}
}

int valid_hwaddr(webs_t wp, char *value, struct variable *v)
{
	/*
	 * Make exception for "NOT IMPLELEMENTED" string 
	 */
	if (!strcmp(value, "NOT_IMPLEMENTED"))
		return (TRUE);

	/*
	 * Check for bad, multicast, broadcast, or null address 
	 */
	if (!legal_hwaddr(value)) {
		websDebugWrite(wp, "Invalid <b>%s</b> %s: not a legal MAC address<br>", v->longname, value);
		return FALSE;
	}

	return TRUE;
}

EJ_VISIBLE void validate_hwaddr(webs_t wp, char *value, struct variable *v)
{
	if (valid_hwaddr(wp, value, v))
		nvram_set(v->name, value);
}

EJ_VISIBLE void validate_hwaddrs(webs_t wp, char *value, struct variable *v)
{
	validate_list(wp, value, v, valid_hwaddr);
}

EJ_VISIBLE void validate_wan_ipaddr(webs_t wp, char *value, struct variable *v)
{
	char wan_ipaddr[20], wan_netmask[20], wan_gateway[20], pptp_wan_gateway[20], l2tp_wan_gateway[20];
	char wan_ipaddr_static[20], wan_netmask_static[20];
	char *wan_proto = websGetVar(wp, "wan_proto", NULL);
	char *pptp_use_dhcp = websGetVar(wp, "pptp_use_dhcp", NULL);
	char *l2tp_use_dhcp = websGetVar(wp, "l2tp_use_dhcp", NULL);

	int pptp_skip_check = FALSE;

	struct variable wan_variables[] = {
		{ NULL },
		{ NULL },
	      { argv:ARGV("wan_ipaddr", "wan_netmask") },
	}, *which;

	which = &wan_variables[0];

	get_merge_ipaddr(wp, "wan_ipaddr", wan_ipaddr, NULL, NULL);
	cidr_to_nm(wan_netmask, sizeof(wan_netmask), websGetVari(wp, "wan_netmask", 0));
	get_merge_ipaddr(wp, "wan_gateway", wan_gateway, NULL, NULL);
	get_merge_ipaddr(wp, "pptp_wan_gateway", pptp_wan_gateway, NULL, NULL);
	get_merge_ipaddr(wp, "l2tp_wan_gateway", l2tp_wan_gateway, NULL, NULL);
	get_merge_ipaddr(wp, "wan_ipaddr_static", wan_ipaddr_static, NULL, NULL);
	cidr_to_nm(wan_netmask_static, sizeof(wan_netmask_static), websGetVari(wp, "wan_netmask_static", 0));
	if (!strcmp(wan_proto, "pptp")) {
		nvram_seti("pptp_pass", 0); // disable pptp passthrough
	}

	if (!strcmp(wan_proto, "pptp") && !strcmp("0.0.0.0",
						  wan_ipaddr)) { // Sveasoft: allow 0.0.0.0 for pptp IP addr
		pptp_skip_check = TRUE;
		nvram_seti("pptp_use_dhcp", 1);
	} else
		nvram_seti("pptp_use_dhcp", 0);

	if (strcmp(wan_proto, "pppoe_dual")) {
		if (FALSE == pptp_skip_check && !valid_ipaddr(wp, wan_ipaddr, &which[0]))
			return;
	} else {
		if (!valid_ipaddr(wp, wan_ipaddr_static, &which[0]))
			return;
	}

	nvram_set("wan_ipaddr_buf", nvram_safe_get("wan_ipaddr"));
	nvram_set("wan_ipaddr", wan_ipaddr);
	nvram_set("wan_ipaddr_static", wan_ipaddr_static);

	if (strcmp(wan_proto, "pppoe_dual")) {
		if (FALSE == pptp_skip_check && !valid_netmask(wp, wan_netmask, &which[1]))
			return;
	} else {
		if (!valid_netmask(wp, wan_netmask_static, &which[1]))
			return;
	}

	nvram_set("wan_netmask", wan_netmask);
	nvram_set("wan_netmask_static", wan_netmask_static);

	if (strcmp(pptp_wan_gateway, "0.0.0.0")) {
		nvram_set("pptp_wan_gateway", pptp_wan_gateway);
	}

	if (strcmp(l2tp_wan_gateway, "0.0.0.0")) {
		nvram_set("l2tp_wan_gateway", l2tp_wan_gateway);
	}

	if (!strcmp(wan_gateway, "0.0.0.0"))
		return;

	nvram_set("wan_gateway", wan_gateway);
}

#ifdef HAVE_PORTSETUP
EJ_VISIBLE void validate_portsetup(webs_t wp, char *value, struct variable *v)
{
	char *next;
	char var[64];
	char eths[256];

	getIfLists(eths, 256);
	foreach(var, eths, next)
	{
		copytonv_prefix(wp, "label", var);
		copytonv_prefix(wp, "hwaddr", var);
		char *bridged = copytonv_prefix(wp, "bridged", var);
		copytonv_prefix(wp, "multicast", var);
		copytonv_prefix(wp, "multicast_to_unicast", var);
		copytonv_prefix(wp, "nat", var);
		copytonv_prefix(wp, "bloop", var);
		copytonv_prefix(wp, "isolation", var);
#ifdef HAVE_TOR
		copytonv_prefix(wp, "tor", var);
#endif
		copytonv_prefix(wp, "dns_redirect", var);
		copymergetonv(wp, "%s_dns_ipaddr", var);
		char val[64];

		sprintf(val, "%s_mtu", var);
		char *mtu = websGetVar(wp, val, NULL);
		if (mtu)
			nvram_set(val, mtu);
		else
			nvram_seti(val, 1500);

		sprintf(val, "%s_txq", var);
		char *txq = websGetVar(wp, val, NULL);
		if (txq)
			nvram_set(val, txq);
		else
			nvram_seti(val, 1000);

#ifdef HAVE_TMK
		sprintf(val, "%s_r1x", var);
		nvram_set(val, websGetVar(wp, val, "0"));
		sprintf(val, "%s_r1x_server", var);
		nvram_set(val, websGetVar(wp, val, ""));
		sprintf(val, "%s_r1x_port", var);
		nvram_set(val, websGetVar(wp, val, ""));
		sprintf(val, "%s_r1x_ss", var);
		nvram_set(val, websGetVar(wp, val, ""));
		sprintf(val, "%s_r1x_st", var);
		nvram_set(val, websGetVar(wp, val, "0"));
		sprintf(val, "%s_r1x_wl", var);
		nvram_set(val, websGetVar(wp, val, ""));
#endif

		if (bridged && strcmp(bridged, "0") == 0) {
			copymergetonv(wp, "%s_ipaddr", var);
			char buf[32];
			char temp[32];
			sprintf(temp, "%s_netmask", var);
			nvram_set(temp, cidr_to_nm(buf, sizeof(buf), websGetVari(wp, temp, 0)));
#if defined(HAVE_BKM) || defined(HAVE_TMK)
			if (1) {
				copytonv(wp, "nld_%s_enable", var);
				copytonv(wp, "nld_%s_bridge", var);
				//NSMD
				copytonv(wp, "nsmd_%s_enable", var);
			}
#endif
#if defined(HAVE_BATMANADV)
			if (1) {
				copytonv(wp, "bat_%s_enable", var);
				copytonv(wp, "bat_%s_bridge", var);
			}
#endif
		}
	}
	next = websGetVar(wp, "wan_ifname", NULL);
	if (next) {
		nvram_set("wan_ifname2", next);
	}
}
#endif

EJ_VISIBLE void validate_lan_ipaddr(webs_t wp, char *value, struct variable *v)
{
	char lan_ipaddr[20], lan_netmask[20];
	cidr_to_nm(lan_netmask, sizeof(lan_netmask), websGetVari(wp, "lan_netmask", 0));
	get_merge_ipaddr(wp, v->name, lan_ipaddr, NULL, NULL);

	if (!valid_ipaddr(wp, lan_ipaddr, v))
		return;

	if (strcmp(nvram_safe_get("lan_ipaddr"), lan_ipaddr)) {
		unlink("/tmp/dnsmasq.leases");
		unlink("/jffs/dnsmasq.leases");
	}
	if (strcmp(nvram_safe_get("lan_netmask"), lan_netmask)) {
		unlink("/tmp/dnsmasq.leases");
		unlink("/jffs/dnsmasq.leases");
	}

	/*     if (strcmp(lan_ipaddr, nvram_safe_get("lan_ipaddr")) || strcmp(lan_netmask, nvram_safe_get("lan_netmask")))
	   lan_ip_changed = 1;
	   else
	   lan_ip_changed = 0; */

	nvram_set(v->name, lan_ipaddr);
	nvram_set("lan_netmask", lan_netmask);
}

EJ_VISIBLE void validate_remote_ip(webs_t wp, char *value, struct variable *v)
{
	char from[20], *to;
	char remote_ip[254];
	char name[32];

	get_merge_ipaddr(wp, v->name, from, NULL, NULL);

	snprintf(name, sizeof(name), "%s_4", v->name);
	to = websGetVar(wp, name, NULL);

	if (!valid_ipaddr(wp, from, v))
		return;

	snprintf(remote_ip, sizeof(remote_ip), "%s %s", from, to);

	nvram_set(v->name, remote_ip);
}

#define SRL_VALID(v) (((v) > 0) && ((v) <= 15))
#define SFBL_VALID(v) (((v) > 0) && ((v) <= 15))
#define LRL_VALID(v) (((v) > 0) && ((v) <= 15))
#define LFBL_VALID(v) (((v) > 0) && ((v) <= 15))

EJ_VISIBLE void validate_wl_wme_tx_params(webs_t wp, char *value, struct variable *v)
{
	int srl = 0, sfbl = 0, lrl = 0, lfbl = 0, max_rate = 0, nmode = 0;
	char *s, *errmsg;
	char tmp[256];

	/* return if wme is not enabled */
	if (!(s = websGetVar(wp, "wl0_wme", NULL))) {
		return;
	} else if (!strcmp(s, "off")) {
		return;
	}

	/* return if afterburner enabled */
	if ((s = websGetVar(wp, "wl0_afterburner", NULL)) && (!strcmp(s, "auto"))) {
		return;
	}

	if (!value || atoi(value) != 5) { /* Number of INPUTs */
		return;
	}

	s = nvram_safe_get(v->name);

	if (*s)
		sscanf(s, "%d %d %d %d %d", &srl, &sfbl, &lrl, &lfbl, &max_rate);

	if ((value = websGetVar(wp, strcat_r(v->name, "0", tmp), NULL)) != NULL)
		srl = atoi(value);

	if (!SRL_VALID(srl)) {
		errmsg = "Short Retry Limit must be in the range 1 to 15";
		return;
	}

	if ((value = websGetVar(wp, strcat_r(v->name, "1", tmp), NULL)) != NULL)
		sfbl = atoi(value);

	if (!SFBL_VALID(sfbl)) {
		errmsg = "Short Fallback Limit must be in the range 1 to 15";
		return;
	}

	if ((value = websGetVar(wp, strcat_r(v->name, "2", tmp), NULL)) != NULL)
		lrl = atoi(value);

	if (!LRL_VALID(lrl)) {
		errmsg = "Long Retry Limit must be in the range 1 to 15";
		return;
	}

	if ((value = websGetVar(wp, strcat_r(v->name, "3", tmp), NULL)) != NULL)
		lfbl = atoi(value);

	if (!LFBL_VALID(lfbl)) {
		errmsg = "Long Fallback Limit must be in the range 1 to 15";
		return;
	}

	if ((value = websGetVar(wp, strcat_r(v->name, "4", tmp), NULL)) != NULL)
		max_rate = atoi(value);

	s = nvram_safe_get("wl0_nmode");
	if (*s)
		nmode = atoi(s);

	sprintf(tmp, "%d %d %d %d %d", srl, sfbl, lrl, lfbl, max_rate);

	nvram_set(v->name, tmp);

	return;
}

EJ_VISIBLE void validate_wl_wme_params(webs_t wp, char *value, struct variable *v)
{
	int n, i;
	int cwmin = 0, cwmax = 0;
	char *wme, *afterburner;
	char name[100];
	char buf[1000] = "", *cur = buf;
	struct {
		char *name;
		int range;
		char *arg1;
		char *arg2;
	} field_attrib[] = { { "WME AC CWmin", 1, "0", "32767" },     { "WME AC CWmax", 1, "0", "32767" },
			     { "WME AC AIFSN", 1, "1", "15" },	      { "WME AC TXOP(b)", 1, "0", "65504" },
			     { "WME AC TXOP(a/g)", 1, "0", "65504" }, { "WME AC Admin Forced", 0, "on", "off" } };

	/*
	 * return if wme is not enabled 
	 */
	if (!(wme = websGetVar(wp, "wl_wme", NULL)))
		return;
	else if (strcmp(wme, "on"))
		return;

	/*
	 * return if afterburner enabled 
	 */
	if ((afterburner = websGetVar(wp, "wl_afterburner", NULL)) && (!strcmp(afterburner, "auto")))
		return;

	n = atoi(value) + 1;

	for (i = 0; i < n; i++) {
		snprintf(name, sizeof(name), "%s%d", v->name, i);
		if (!(value = websGetVar(wp, name, NULL)))
			return;
		if (!*value && v->nullok)
			continue;

		if (i == 0)
			cwmin = atoi(value);
		else if (i == 1) {
			cwmax = atoi(value);
			if (cwmax < cwmin) {
				websDebugWrite(wp, "Invalid <b>%s</b> %d: greater than <b>%s</b> %d<br>", field_attrib[0].name,
					       cwmin, field_attrib[i].name, cwmax);
				return;
			}
		}
		if (field_attrib[i].range) {
			if (atoi(value) < atoi(field_attrib[i].arg1) || atoi(value) > atoi(field_attrib[i].arg2)) {
				websDebugWrite(wp, "Invalid <b>%s</b> %d: should be in range %s to %s<br>", field_attrib[i].name,
					       atoi(value), field_attrib[i].arg1, field_attrib[i].arg2);
				return;
			}
		} else {
			if (strcmp(value, field_attrib[i].arg1) && strcmp(value, field_attrib[i].arg2)) {
				websDebugWrite(wp, "Invalid <b>%s</b> %s: should be %s or %s<br>", field_attrib[i].name, value,
					       field_attrib[i].arg1, field_attrib[i].arg2);
			}
		}

		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s", cur == buf ? "" : " ", value);
	}

	nvram_set(v->name, buf);
}

EJ_VISIBLE void validate_wl_key(webs_t wp, char *value, struct variable *v)
{
	char *c;

	switch (*(value)) {
	case 5:
	case 13:
		break;
	case 10:
	case 26:
		for (c = value; *c; c++) {
			if (!isxdigit(*c)) {
				websDebugWrite(wp, "Invalid <b>%s</b>: character %c is not a hexadecimal digit<br>", v->longname,
					       *c);
				return;
			}
		}
		break;
	default:
		websDebugWrite(wp, "Invalid <b>%s</b>: must be 5 or 13 ASCII characters or 10 or 26 hexadecimal digits<br>",
			       v->longname);
		return;
	}

	nvram_set(v->name, value);
}

#ifndef GMODE_AFTERBURNER
#define GMODE_AFTERBURNER 7
#endif

EJ_VISIBLE void validate_wl_wep(webs_t wp, char *value, struct variable *v)
{
	if (!valid_choice(wp, value, v))
		return;
#ifdef ABURN_WSEC_CHECK
	if (strcmp(value, "off") && nvram_geti("wl_gmode") == GMODE_AFTERBURNER) {
		websDebugWrite(wp, "<br>Invalid <b>%s</b>: must be set to <b>Off</b> when 54g Mode is AfterBurner.", v->longname);
		return;
	}
#endif
	nvram_set(v->name, value);
}

EJ_VISIBLE void validate_auth_mode(webs_t wp, char *value, struct variable *v)
{
	if (!valid_choice(wp, value, v))
		return;
	nvram_set(v->name, value);
}

EJ_VISIBLE void validate_wpa_psk(webs_t wp, char *value, struct variable *v)
{
	int len = strlen(value);
	char *c;

	if (len == 64) {
		for (c = value; *c; c++) {
			if (!isxdigit((int)*c)) {
				websDebugWrite(wp, "Invalid <b>%s</b>: character %c is not a hexadecimal digit<br>", v->longname,
					       *c);
				return;
			}
		}
	} else if (len < 8 || len > 63) {
		websDebugWrite(wp, "Invalid <b>%s</b>: must be between 8 and 63 ASCII characters or 64 hexadecimal digits<br>",
			       v->longname);
		return;
	}

	nvram_set(v->name, value);
}

EJ_VISIBLE void validate_wl_wep_key(webs_t wp, char *value, struct variable *v)
{
	char buf[200] = "";
	int error_value = 0;
	struct variable wl_wep_variables[] = {
	      { argv:ARGV("16") },
	      { argv:ARGV("5", "10") },
		// for 64 bit
	      { argv:ARGV("13", "26") },
		// for 128 bit
	      { argv:ARGV("1", "4") },
	}, *which;

	char *wep_bit = "", *wep_passphrase = "", *wep_key1 = "", *wep_key2 = "", *wep_key3 = "", *wep_key4 = "", *wep_tx = "";
	char new_wep_passphrase[50] = "", new_wep_key1[30] = "", new_wep_key2[30] = "", new_wep_key3[30] = "",
	     new_wep_key4[30] = "";
	int index;

	which = &wl_wep_variables[0];

	wep_bit = websGetVar(wp, "wl_wep_bit", NULL); // 64 or 128
	if (!wep_bit)
		return;
	if (strcmp(wep_bit, "64") && strcmp(wep_bit, "128"))
		return;

	wep_passphrase = websGetVar(wp, "wl_passphrase", "");
	// if(!wep_passphrase) return ;

	// strip_space(wep_passphrase);
	if (strcmp(wep_passphrase, "")) {
		if (!valid_name(wp, wep_passphrase, &which[0], 1)) {
			error_value = 1;
		} else {
			httpd_filter_name(wep_passphrase, new_wep_passphrase, sizeof(new_wep_passphrase), SET);
		}
	}

	wep_key1 = websGetVar(wp, "wl_key1", "");
	wep_key2 = websGetVar(wp, "wl_key2", "");
	wep_key3 = websGetVar(wp, "wl_key3", "");
	wep_key4 = websGetVar(wp, "wl_key4", "");
	wep_tx = websGetVar(wp, "wl_key", NULL);

	if (!wep_tx) {
		error_value = 1;
		return;
	}

	index = (atoi(wep_bit) == 64) ? 1 : 2;

	if (strcmp(wep_key1, "")) {
		if (!valid_wep_key(wp, wep_key1, &which[index])) {
			error_value = 1;
		} else {
			httpd_filter_name(wep_key1, new_wep_key1, sizeof(new_wep_key1), SET);
		}
	}
	if (strcmp(wep_key2, "")) {
		if (!valid_wep_key(wp, wep_key2, &which[index])) {
			error_value = 1;
		} else {
			httpd_filter_name(wep_key2, new_wep_key2, sizeof(new_wep_key2), SET);
		}
	}
	if (strcmp(wep_key3, "")) {
		if (!valid_wep_key(wp, wep_key3, &which[index])) {
			error_value = 1;
		} else {
			httpd_filter_name(wep_key3, new_wep_key3, sizeof(new_wep_key3), SET);
		}
	}
	if (strcmp(wep_key4, "")) {
		if (!valid_wep_key(wp, wep_key4, &which[index])) {
			error_value = 1;
		} else {
			httpd_filter_name(wep_key4, new_wep_key4, sizeof(new_wep_key4), SET);
		}
	}

	if (!error_value) {
		snprintf(buf, sizeof(buf), "%s:%s:%s:%s:%s:%s", new_wep_passphrase, new_wep_key1, new_wep_key2, new_wep_key3,
			 new_wep_key4, wep_tx);
		nvram_set("wl_wep_bit", wep_bit);
		nvram_set("wl_wep_buf", buf);

		nvram_set("wl_passphrase", wep_passphrase);
		nvram_set("wl_key", wep_tx);
		nvram_set("wl_key1", wep_key1);
		nvram_set("wl_key2", wep_key2);
		nvram_set("wl_key3", wep_key3);
		nvram_set("wl_key4", wep_key4);

		if (!strcmp(wep_key1, "") && !strcmp(wep_key2, "") && !strcmp(wep_key3, "") && !strcmp(wep_key4, "")) // Allow
			// null
			// wep
			nvram_set("wl_wep", "off");
		else
			nvram_set("wl_wep", "restricted");
	}
}

EJ_VISIBLE void validate_wl_auth(webs_t wp, char *value, struct variable *v)
{
	if (!valid_choice(wp, value, v))
		return;
	/*
	 * // not to check , spec for linksys if (atoi(value) == 1) { char
	 * wl_key[] = "wl_keyXXX";
	 * 
	 * snprintf(wl_key, sizeof(wl_key), "wl_key%s",
	 * nvram_safe_get("wl_key")); if (!strlen(nvram_safe_get(wl_key))) {
	 * websDebugWrite(wp, "Invalid <b>%s</b>: must first specify a valid
	 * <b>Network Key</b><br>", v->longname); return; } } 
	 */
	nvram_set(v->name, value);
}

/*
 * Example: 00:11:22:33:44:55=1 00:12:34:56:78:90=0 (ie 00:11:22:33:44:55 if
 * filterd, and 00:12:34:56:78:90 is not) wl_maclist = "00:11:22:33:44:55" 
 */
EJ_VISIBLE void validate_wl_hwaddrs(webs_t wp, char *value, struct variable *v)
{
	int i;
	int error_value = 0;
	char *buf = safe_malloc((19 * WL_FILTER_MAC_NUM * WL_FILTER_MAC_PAGE) + 1);
	if (buf == NULL)
		return; //out of memory
	char *cur = buf;
	unsigned int m[6];
	char *ifname2 = websGetVar(wp, "ifname", NULL); // 64 or 128
	char ifname[32];
	strcpy(ifname, ifname2);
	rep(ifname, 'X', '.');
	bzero(buf, 19 * WL_FILTER_MAC_NUM * WL_FILTER_MAC_PAGE);
	if (ifname == NULL) {
		debug_free(buf);
		return;
	}
#ifdef HAVE_SPOTPASS
	int count, wildcard, wildcard_valid;
#endif
	for (i = 0; i < WL_FILTER_MAC_NUM * WL_FILTER_MAC_PAGE; i++) {
		char filter_mac[] = "wlan10.99_macXXX";
		char *mac = NULL;
		char mac1[21];

		snprintf(filter_mac, sizeof(filter_mac), "%s%s%d", ifname2, "_mac", i);

		mac = websGetVar(wp, filter_mac, NULL);

		if (!mac || !strcmp(mac, "0") || !strcmp(mac, "")) {
			continue;
		}
#ifdef HAVE_SPOTPASS
		count = strlen(mac) - 1;
		wildcard = 0;
		wildcard_valid = 0;

		for (count; count >= 0; count--) {
			if (mac[count] == '*') {
				if (count == strlen(mac) - 1) {
					wildcard_valid = 1;
					wildcard++;
					mac[count] = '0';
				} else if (wildcard_valid == 0) {
					wildcard_valid = -1;
					break;
				} else {
					wildcard++;
					mac[count] = '0';
				}
			} else if (mac[count] != ':' && wildcard_valid == 1) {
				wildcard_valid = 0;
			}
		}

		if (wildcard_valid == -1) {
			// validation error - skip MAC
			continue;
		}
#endif
		if (strlen(mac) == 12) {
			sscanf(mac, "%02X%02X%02X%02X%02X%02X", &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]);
			sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
		} else if (strlen(mac) == 17) {
			sscanf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]);
			sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
		} else {
			mac1[0] = 0;
		}

		if (!valid_hwaddr(wp, mac1, v)) {
			error_value = 1;
			continue;
		}
#ifdef HAVE_SPOTPASS
		if (wildcard_valid == 0 && wildcard > 0) {
			sprintf(mac1, "%s\/%d", mac1, wildcard * 4);
		}
#endif
		cur += snprintf(cur, buf + ((strlen(mac1) + 1) * WL_FILTER_MAC_NUM * WL_FILTER_MAC_PAGE) - cur, "%s%s",
				cur == buf ? "" : " ", mac1);
	}

	if (!error_value) {
		char mlist[32];
		sprintf(mlist, "%s_maclist", ifname);
		nvram_set(mlist, buf);
		char acmac[32];
		sprintf(acmac, "%s_active_mac", ifname);
		nvram_set(acmac, "");
		if (!strcmp(ifname, "wl0"))
			nvram_set("wl_active_mac", "");
	}
	debug_free(buf);
}

/*
 * Example: name:[on|off]:[tcp|udp|both]:8000:80>100 
 */

EJ_VISIBLE void validate_forward_proto(webs_t wp, char *value, struct variable *v)
{
	int i, error = 0;
	char *buf, *cur;
	int count, sof;
	struct variable forward_proto_variables[] = {
	      { argv:ARGV("12") },
	      { argv:ARGV("0", "65535") },
	      { argv:ARGV("0", "65535") },
		{ NULL },
	}, *which;
	buf = nvram_safe_get("forward_entries");
	if (buf == NULL || *(buf) == 0)
		return;
	count = atoi(buf);
	sof = (count * 128) + 1;
	buf = (char *)safe_malloc(sof);
	cur = buf;
	buf[0] = 0;

	for (i = 0; i < count; i++) {
		char forward_name[] = "nameXXX";
		char forward_from[] = "fromXXX";
		char forward_to[] = "toXXX";
		char forward_ip[] = "ipXXX";
		char forward_tcp[] = "tcpXXX"; // for checkbox
		char forward_udp[] = "udpXXX"; // for checkbox
		char forward_pro[] = "proXXX"; // for select, cisco style UI
		char forward_enable[] = "enableXXX";
		char *name = "", new_name[200] = "", *from = "", *to = "", *ip = "", *tcp = "", *udp = "", *enable = "", proto[10],
		     *pro = "";

		snprintf(forward_name, sizeof(forward_name), "name%d", i);
		snprintf(forward_from, sizeof(forward_from), "from%d", i);
		snprintf(forward_to, sizeof(forward_to), "to%d", i);
		snprintf(forward_ip, sizeof(forward_ip), "ip%d", i);
		snprintf(forward_tcp, sizeof(forward_tcp), "tcp%d", i);
		snprintf(forward_udp, sizeof(forward_udp), "udp%d", i);
		snprintf(forward_enable, sizeof(forward_enable), "enable%d", i);
		snprintf(forward_pro, sizeof(forward_pro), "pro%d", i);

		name = websGetVar(wp, forward_name, "");
		from = websGetVar(wp, forward_from, "0");
		to = websGetVar(wp, forward_to, "0");
		ip = websGetVar(wp, forward_ip, "0");
		tcp = websGetVar(wp, forward_tcp, NULL); // for checkbox
		udp = websGetVar(wp, forward_udp, NULL); // for checkbox
		pro = websGetVar(wp, forward_pro, NULL); // for select option
		enable = websGetVar(wp, forward_enable, "off");

		which = &forward_proto_variables[0];

		if (!*from && !*to && !*ip)
			continue;
		if (!strcmp(ip, "0") || !strcmp(ip, ""))
			continue;
		if ((!strcmp(from, "0") || !strcmp(from, "")) && (!strcmp(to, "0") || !strcmp(to, "")) &&
		    (!strcmp(ip, "0") || !strcmp(ip, ""))) {
			continue;
		}

		/*
		 * check name 
		 */
		if (strcmp(name, "")) {
			if (!valid_name(wp, name, &which[0], 1)) {
				continue;
			} else {
				httpd_filter_name(name, new_name, sizeof(new_name), SET);
			}
		}

		if (!strcmp(from, ""))
			from = to;
		if (!strcmp(to, ""))
			to = from;

		if (atoi(from) > atoi(to)) {
			SWAP(from, to);
		}

		if (!valid_range(wp, from, &which[1]) || !valid_range(wp, to, &which[2])) {
			continue;
		}

		if (pro) { // use select option
			strcpy(proto, pro);
		} else { // use checkbox
			if (tcp && udp)
				strcpy(proto, "both");
			else if (tcp && !udp)
				strcpy(proto, "tcp");
			else if (!tcp && udp)
				strcpy(proto, "udp");
		}
		/*
		 * check ip address 
		 */

		if (!*ip) {
			error = 1;
			// websWrite(wp, "Invalid <b>%s</b> : must specify a
			// ip<br>",which[4].longname);
			continue;
		}

		/*
		 * Sveasoft add - new format allows full IP address 
		 */
		if (sv_valid_ipaddr(ip)) {
			cur += snprintf(cur, buf + sof - cur, "%s%s:%s:%s:%d:%d>%s", cur == buf ? "" : " ", new_name, enable, proto,
					atoi(from), atoi(to), ip);
		}
		/*
		 * Sveasoft - for backwords compatability allow single number 
		 */
		else if (sv_valid_range(ip, 0, 254)) {
			char fullip[16] = { 0 };
			int f_ip[4];

			sscanf(nvram_safe_get("lan_ipaddr"), "%d.%d.%d.%d", &f_ip[0], &f_ip[1], &f_ip[2], &f_ip[3]);
			snprintf(fullip, 15, "%d.%d.%d.%d", f_ip[0], f_ip[1], f_ip[2], atoi(ip));
			cur += snprintf(cur, buf + sof - cur, "%s%s:%s:%s:%d:%d>%s", cur == buf ? "" : " ", new_name, enable, proto,
					atoi(from), atoi(to), fullip);

		} else {
			error = 1;
			continue;
		}
	}
	if (!error)
		nvram_set(v->name, buf);
	debug_free(buf);
}

/*
 * Example: name:[on|off]:src:dest
 */
void validate_forward_ip(webs_t wp, char *value, struct variable *v)
{
	char *buf, *cur;
	int count, sof, i = 0, error = 0;
	struct variable forward_proto_variables[] = {
	      { argv:ARGV("12") },
		{ NULL },
	}, *which;

	buf = nvram_safe_get("forwardip_entries");
	if (buf == NULL || *(buf) == 0)
		return;
	count = atoi(buf);
	sof = (count * 128) + 1;
	buf = (char *)safe_malloc(sof);
	cur = buf;
	buf[0] = 0;

	for (i = 0; i < count; i++) {
		char forward_name[] = "nameXXX";
		char forward_src[] = "srcXXX";
		char forward_dest[] = "destXXX";
		char forward_enable[] = "enableXXX";
		char *name = "", new_name[200] = "", *src = "", *dest = "", *enable = "";

		snprintf(forward_name, sizeof(forward_name), "name%d", i);
		snprintf(forward_src, sizeof(forward_src), "src%d", i);
		snprintf(forward_dest, sizeof(forward_dest), "dest%d", i);
		snprintf(forward_enable, sizeof(forward_enable), "enable%d", i);

		name = websGetVar(wp, forward_name, "");
		src = websGetVar(wp, forward_src, NULL);
		dest = websGetVar(wp, forward_dest, NULL);
		enable = websGetVar(wp, forward_enable, "off");

		if (!*src && !*dest)
			continue;
		if (!strcmp(src, "0") || !strcmp(src, ""))
			continue;
		if ((!strcmp(dest, "0") || !strcmp(dest, "")))
			continue;

		/*
		 * check name 
		 */
		if (strcmp(name, "")) {
			if (!valid_name(wp, name, &which[0], 1)) {
				continue;
			} else {
				httpd_filter_name(name, new_name, sizeof(new_name), SET);
			}
		}
		/*
		 * check ip address 
		 */

		if (!*src || !*dest) {
			error = 1;
			// websWrite(wp, "Invalid <b>%s</b> : must specify a
			// ip<br>",which[4].longname);
			continue;
		}

		if (sv_valid_ipaddr(src) && sv_valid_ipaddr(dest)) {
			cur += snprintf(cur, buf + sof - cur, "%s%s:%s:%s:%s", cur == buf ? "" : " ", new_name, enable, src, dest);
		} else {
			error = 1;
			continue;
		}
	}
	if (!error)
		nvram_set(v->name, buf);
	debug_free(buf);
}

/*
 * Example: name:[on|off]:[tcp|udp|both]:8000:80>100 
 */

EJ_VISIBLE void validate_forward_spec(webs_t wp, char *value, struct variable *v)
{
	int i, error = 0;
	char *buf, *cur;
	int count, sof;
	struct variable forward_proto_variables[] = {
	      { argv:ARGV("12") },
	      { argv:ARGV("0", "65535") },
	      { argv:ARGV("0", "65535") },
		{ NULL },
	}, *which;
	buf = nvram_safe_get("forwardspec_entries");
	if (buf == NULL || *(buf) == 0)
		return;
	count = atoi(buf);
	sof = (count * 128) + 1;
	buf = (char *)safe_malloc(sof);
	cur = buf;
	buf[0] = 0;

	for (i = 0; i < count; i++) {
		char forward_name[] = "nameXXX";
		char forward_from[] = "fromXXX";
		char forward_to[] = "toXXX";
		char forward_ip[] = "ipXXX";
		char forward_src[] = "srcXXX";
		char forward_tcp[] = "tcpXXX"; // for checkbox
		char forward_udp[] = "udpXXX"; // for checkbox
		char forward_pro[] = "proXXX"; // for select, cisco style UI
		char forward_enable[] = "enableXXX";
		char *name = "", new_name[200] = "", *from = "", *to = "", *ip = "", *tcp = "", *udp = "", *enable = "", proto[10],
		     *pro = "", *src = "";

		snprintf(forward_name, sizeof(forward_name), "name%d", i);
		snprintf(forward_from, sizeof(forward_from), "from%d", i);
		snprintf(forward_to, sizeof(forward_to), "to%d", i);
		snprintf(forward_ip, sizeof(forward_ip), "ip%d", i);
		snprintf(forward_src, sizeof(forward_src), "src%d", i);
		snprintf(forward_tcp, sizeof(forward_tcp), "tcp%d", i);
		snprintf(forward_udp, sizeof(forward_udp), "udp%d", i);
		snprintf(forward_enable, sizeof(forward_enable), "enable%d", i);
		snprintf(forward_pro, sizeof(forward_pro), "pro%d", i);

		name = websGetVar(wp, forward_name, "");
		from = websGetVar(wp, forward_from, "0");
		to = websGetVar(wp, forward_to, "0");
		ip = websGetVar(wp, forward_ip, "0");
		src = websGetVar(wp, forward_src, NULL);
		tcp = websGetVar(wp, forward_tcp, NULL); // for checkbox
		udp = websGetVar(wp, forward_udp, NULL); // for checkbox
		pro = websGetVar(wp, forward_pro, NULL); // for select option
		enable = websGetVar(wp, forward_enable, "off");

		which = &forward_proto_variables[0];

		if (!*from && !*to && !*ip)
			continue;
		if (!strcmp(ip, "0") || !strcmp(ip, ""))
			continue;
		if ((!strcmp(from, "0") || !strcmp(from, "")) && (!strcmp(to, "0") || !strcmp(to, "")) &&
		    (!strcmp(ip, "0") || !strcmp(ip, ""))) {
			continue;
		}

		/*
		 * check name 
		 */
		if (strcmp(name, "")) {
			if (!valid_name(wp, name, &which[0], 1)) {
				continue;
			} else {
				httpd_filter_name(name, new_name, sizeof(new_name), SET);
			}
		}

		if (!strcmp(from, ""))
			from = to;
		if (!strcmp(to, ""))
			to = from;

		/*
		 * if(atoi(from) > atoi(to)){ SWAP(from, to); } 
		 */

		if (!valid_range(wp, from, &which[1]) || !valid_range(wp, to, &which[2])) {
			continue;
		}

		if (pro) { // use select option
			strcpy(proto, pro);
		} else { // use checkbox
			if (tcp && udp)
				strcpy(proto, "both");
			else if (tcp && !udp)
				strcpy(proto, "tcp");
			else if (!tcp && udp)
				strcpy(proto, "udp");
		}
		/*
		 * check ip address 
		 */

		if (!*ip) {
			error = 1;
			// websWrite(wp, "Invalid <b>%s</b> : must specify a
			// ip<br>",which[4].longname);
			continue;
		}

		/*
		 * Sveasoft add - new format allows full IP address 
		 */
		if (sv_valid_ipaddr(ip)) {
			if (!src || *(src) == 0)
				cur += snprintf(cur, buf + sof - cur, "%s%s:%s:%s:%d>%s:%d", cur == buf ? "" : " ", new_name,
						enable, proto, atoi(from), ip, atoi(to));
			else
				cur += snprintf(cur, buf + sof - cur, "%s%s:%s:%s:%d>%s:%d<%s", cur == buf ? "" : " ", new_name,
						enable, proto, atoi(from), ip, atoi(to), src);

		} else {
			error = 1;
			continue;
		}
	}
	if (!error)
		nvram_set(v->name, buf);
	debug_free(buf);
}

EJ_VISIBLE void validate_dynamic_route(webs_t wp, char *value, struct variable *v)
{
	struct variable dr_variables[] = {
	      { argv:ARGV("0", "1", "2", "3") },
	}, *which;
	char *dr_setting;

	which = &dr_variables[0];

	if (valid_choice(wp, value, v))
		nvram_set(v->name, value);

	dr_setting = websGetVar(wp, "dr_setting", NULL);
	if (!dr_setting)
		return;

	if (!valid_choice(wp, dr_setting, &which[0]))
		return;

	nvram_set("dr_setting", dr_setting);

	if (!dr_setting || atoi(dr_setting) == 0) {
		nvram_seti("dr_lan_tx", 0);
		nvram_seti("dr_lan_rx", 0);
		nvram_seti("dr_wan_tx", 0);
		nvram_seti("dr_wan_rx", 0);
	} else if (atoi(dr_setting) == 1) {
		nvram_seti("dr_lan_tx", 0);
		nvram_seti("dr_lan_rx", 0);
		nvram_set("dr_wan_tx", "1 2");
		nvram_set("dr_wan_rx", "1 2");
	} else if (atoi(dr_setting) == 2) {
		nvram_set("dr_lan_tx", "1 2");
		nvram_set("dr_lan_rx", "1 2");
		nvram_seti("dr_wan_tx", 0);
		nvram_seti("dr_wan_rx", 0);
	} else if (atoi(dr_setting) == 3) {
		nvram_set("dr_lan_tx", "1 2");
		nvram_set("dr_lan_rx", "1 2");
		nvram_set("dr_wan_tx", "1 2");
		nvram_set("dr_wan_rx", "1 2");
	} else {
		nvram_seti("dr_lan_tx", 0);
		nvram_seti("dr_lan_rx", 0);
		nvram_seti("dr_wan_tx", 0);
		nvram_seti("dr_wan_rx", 0);
	}

	/*
	 * <lonewolf> 
	 */
	if (websGetVari(wp, "dyn_default", 0) == 1)
		nvram_seti("dyn_default", 1);
	else
		nvram_seti("dyn_default", 0);

	if (nvram_match("wk_mode", "ospf") || nvram_match("wk_mode", "router") || nvram_match("wk_mode", "ospf router")) {
		nvram_set("zebra_conf", websGetVar(wp, "zebra_conf", ""));
		nvram_set("ospfd_conf", websGetVar(wp, "ospfd_conf", ""));
		nvram_set("ripd_conf", websGetVar(wp, "ripd_conf", ""));
		nvram_set("zebra_copt", websGetVar(wp, "zebra_copt", "0"));
		nvram_set("ripd_copt", websGetVar(wp, "ripd_copt", "0"));
		nvram_set("ospfd_copt", websGetVar(wp, "ospfd_copt", "0"));
	}
	/*
	 * </lonewolf> 
	 */
}

EJ_VISIBLE void validate_wl_gmode(webs_t wp, char *value, struct variable *v)
{
	if (!valid_choice(wp, value, v))
		return;
	if (atoi(value) == GMODE_AFTERBURNER) {
		nvram_seti("wl0_lazywds", 0);
		nvram_set("wl0_wds", "");
		nvram_set("wl0_mode", "ap");
	}
	nvram_set(v->name, value);

	return;
	/*
	 * force certain wireless variables to fixed values 
	 */
	if (atoi(value) == GMODE_AFTERBURNER) {
		if (nvram_invmatch("wl0_auth_mode", "disabled") ||
#ifdef ABURN_WSEC_CHECK
		    nvram_invmatch("wl0_wep", "off") ||
#endif
		    nvram_invmatch("wl0_mode", "ap") || nvram_invmatchi("wl0_lazywds", 0) || nvram_invmatch("wl0_wds", "")) {
			/*
			 * notify the user 
			 */
			/*
			 * #ifdef ABURN_WSEC_CHECK websWrite (wp, "Invalid <b>%s</b>:
			 * AfterBurner mode requires:" "<br><b>Network Authentication</b> 
			 * set to <b>Disabled</b>" "<br><b>Data Encryption</b> set to
			 * <b>Off</b>" "<br><b>Mode</b> set to <b>Access Point</b>" //
			 * "<br><b>Bridge Restrict</b> set to <b>Enabled</b>" "<br><b>WDS 
			 * devices</b> disabled." "<br>", v->name); #else websWrite (wp,
			 * "Invalid <b>%s</b>: AfterBurner mode requires:"
			 * "<br><b>Network Authentication</b> set to <b>Disabled</b>"
			 * "<br><b>Mode</b> set to <b>Access Point</b>" // "<br><b>Bridge 
			 * Restrict</b> set to <b>Enabled</b>" "<br><b>WDS devices</b>
			 * disabled." "<br>", v->name); #endif
			 */
			return;
		}
	}
}

/*
 * UI Mode GMODE Afterburner Override Basic Rate Set FrameBurst CTS
 * Protection Mixed 6 - AfterBurner -1 Default ON -1(auto) 54g-Only 6 -
 * AfterBurner -1 ALL ON 0(off) 11b-Only 0 - 54g Legacy B NA Default ON
 * -1(auto) 
 */

/*
 * Sveasoft note: settings for b-only, mixed, and g-mode set back to original 
 * defaults before "afterburner" mods. Afterburner bizarre settings
 * maintained for "speedbooster" mode 
 */

void convert_wl_gmode(char *value, char *prefix)
{
	/* disabled */
	if (!strcmp(value, "disabled")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(-1, "%s_gmode", prefix);
		nvram_nseti(-1, "%s_nmode", prefix);
		nvram_nseti(0, "%s_nreqd", prefix);
		if (!nvram_nmatch("1", "%s_nband", prefix) && !nvram_nmatch("2", "%s_nband", prefix))
			nvram_nseti(2, "%s_nband", prefix);
		/* bgn-mixed */
	} else if (!strcmp(value, "mixed")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(1, "%s_gmode", prefix);
		nvram_nseti(-1, "%s_nmode", prefix);
		nvram_nset("none", "%s_nctrlsb", prefix);
		nvram_nset("auto", "%s_afterburner", prefix);
		nvram_nset("default", "%s_rateset", prefix);
		nvram_nset("on", "%s_frameburst", prefix);
		if (!has_mimo(prefix))
			nvram_nset("g", "%s_phytype", prefix);
		nvram_nseti(0, "%s_nreqd", prefix);
		nvram_nseti(0, "%s_bss_opmode_cap_reqd", prefix);
		if (has_2ghz(prefix))
			nvram_nseti(2, "%s_nband", prefix);
		else
			nvram_nseti(1, "%s_nband", prefix);
		/* bg-mixed */
	} else if (!strcmp(value, "bg-mixed")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(1, "%s_gmode", prefix);
		nvram_nset("none", "%s_nctrlsb", prefix);
		nvram_nset("auto", "%s_afterburner", prefix);
		nvram_nset("default", "%s_rateset", prefix);
		nvram_nset("on", "%s_frameburst", prefix);
		nvram_nseti(0, "%s_nmode", prefix);
		if (!has_mimo(prefix))
			nvram_nset("g", "%s_phytype", prefix);
		nvram_nseti(0, "%s_nreqd", prefix);
		nvram_nseti(0, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nseti(2, "%s_nband", prefix);
		/* g-only */
	} else if (!strcmp(value, "g-only")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(0, "%s_nmode", prefix);
		nvram_nset("none", "%s_nctrlsb", prefix);
		nvram_nseti(2, "%s_gmode", prefix);
		if (!has_mimo(prefix))
			nvram_nset("g", "%s_phytype", prefix);
		nvram_nseti(0, "%s_nreqd", prefix);
		nvram_nseti(1, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nseti(2, "%s_nband", prefix);
		/* ng-only */
	} else if (!strcmp(value, "ng-only")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(2, "%s_nmode", prefix);
		nvram_nseti(2, "%s_gmode", prefix);
		nvram_nseti(0, "%s_nreqd", prefix);
		nvram_nseti(1, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nseti(2, "%s_nband", prefix);
		/* b-only */
	} else if (!strcmp(value, "b-only")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(0, "%s_gmode", prefix);
		nvram_nset("none", "%s_nctrlsb", prefix);
		nvram_nseti(0, "%s_nmode", prefix);
		nvram_nset("off", "%s_afterburner", prefix);
		nvram_nset("default", "%s_rateset", prefix);
		nvram_nset("on", "%s_frameburst", prefix);
		if (!has_mimo(prefix))
			nvram_nset("g", "%s_phytype", prefix);
		nvram_nseti(0, "%s_nreqd", prefix);
		nvram_nseti(0, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nseti(2, "%s_nband", prefix);
		/* n-only or n2-only */
	} else if (!strcmp(value, "n-only") || !strcmp(value, "n2-only")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(1, "%s_gmode", prefix);
		nvram_nseti(2, "%s_nmode", prefix);
		nvram_nseti(1, "%s_nreqd", prefix);
		nvram_nseti(2, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nset("off", "%s_afterburner", prefix); // From
		// 3.61.13.0
		nvram_nset("n", "%s_phytype", prefix);
		nvram_nseti(2, "%s_nband", prefix);
		/* n5-only */
	} else if (!strcmp(value, "n5-only")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(2, "%s_nmode", prefix);
		nvram_nseti(1, "%s_nreqd", prefix);
		nvram_nseti(2, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nset("off", "%s_afterburner", prefix); // From
		// 3.61.13.0
		nvram_nset("n", "%s_phytype", prefix);
		nvram_nseti(1, "%s_nband", prefix);
		/* na-mixed */
	} else if (!strcmp(value, "na-only")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(2, "%s_nmode", prefix);
		nvram_nseti(0, "%s_nreqd", prefix);
		nvram_nseti(1, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nset("off", "%s_afterburner", prefix); // From
		// 3.61.13.0
		nvram_nset("n", "%s_phytype", prefix);
		nvram_nseti(1, "%s_nband", prefix);
		/* a-only */
	} else if (!strcmp(value, "a-only")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(0, "%s_nmode", prefix);
		if (!has_mimo(prefix))
			nvram_nset("a", "%s_phytype", prefix);
		nvram_nseti(0, "%s_nreqd", prefix);
		nvram_nseti(1, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nseti(1, "%s_nband", prefix);
	} else if (!strcmp(value, "ac-only")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(2, "%s_nmode", prefix);
		nvram_nseti(1, "%s_nreqd", prefix);
		nvram_nseti(3, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nseti(1, "%s_nband", prefix);
	} else if (!strcmp(value, "acn-mixed")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(2, "%s_nmode", prefix);
		nvram_nseti(1, "%s_nreqd", prefix);
		nvram_nseti(2, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nseti(1, "%s_nband", prefix);
	} else if (!strcmp(value, "ax-only")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(2, "%s_nmode", prefix);
		nvram_nseti(1, "%s_nreqd", prefix);
		nvram_nseti(3, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nseti(1, "%s_nband", prefix);
	} else if (!strcmp(value, "xacn-mixed")) {
		nvram_nset(value, "%s_net_mode", prefix);
		nvram_nseti(2, "%s_nmode", prefix);
		nvram_nseti(1, "%s_nreqd", prefix);
		nvram_nseti(2, "%s_bss_opmode_cap_reqd", prefix);
		nvram_nseti(1, "%s_nband", prefix);
	}
}

EJ_VISIBLE void validate_wl_net_mode(webs_t wp, char *value, struct variable *v)
{
	if (!valid_choice(wp, value, v))
		return;

	convert_wl_gmode(value, "wl");

	nvram_set(v->name, value);
}

#ifdef HAVE_PPPOESERVER

EJ_VISIBLE void validate_chaps(webs_t wp, char *value, struct variable *v)
{
	int i, error = 0;
	char *buf, *cur;
	int count, sof;
	struct variable chaps_variables[] = {
	      { argv:ARGV("30") },
	      { argv:ARGV("30") },
		{ NULL },
	}, *which;
	buf = nvram_safe_get("pppoeserver_chapsnum");
	if (buf == NULL || *(buf) == 0)
		return;
	count = atoi(buf);
	sof = (count * 128) + 1;
	buf = (char *)safe_malloc(sof);
	cur = buf;
	buf[0] = 0;

	for (i = 0; i < count; i++) {
		char chap_user[] = "userXXX";
		char chap_pass[] = "passXXX";
		char chap_ip[] = "ipXXX";
		char chap_enable[] = "enableXXX";
		char *user = "", new_user[200] = "", *pass = "", new_pass[200] = "", *ip = "", *enable = "";

		snprintf(chap_user, sizeof(chap_user), "user%d", i);
		snprintf(chap_pass, sizeof(chap_pass), "pass%d", i);
		snprintf(chap_ip, sizeof(chap_ip), "ip%d", i);
		snprintf(chap_enable, sizeof(chap_enable), "enable%d", i);

		user = websGetVar(wp, chap_user, "");
		pass = websGetVar(wp, chap_pass, "");
		ip = websGetVar(wp, chap_ip, "0");
		enable = websGetVar(wp, chap_enable, "off");

		which = &chaps_variables[0];

		if (!*ip)
			continue;
		if (!strcmp(ip, "0") || !strcmp(ip, ""))
			continue;

		/*
		 * check name 
		 */
		if (strcmp(user, "")) {
			if (!valid_name(wp, user, &which[0], 1)) {
				continue;
			} else {
				httpd_filter_name(user, new_user, sizeof(new_user), SET);
			}
		}

		if (strcmp(pass, "")) {
			if (!valid_name(wp, pass, &which[1], 1)) {
				continue;
			} else {
				httpd_filter_name(pass, new_pass, sizeof(new_pass), SET);
			}
		}

		/*
		 * check ip address 
		 */
		if (!*ip) {
			error = 1;
			// websWrite(wp, "Invalid <b>%s</b> : must specify a
			// ip<br>",which[4].longname);
			continue;
		}

		if (sv_valid_ipaddr(ip)) {
			cur += snprintf(cur, buf + sof - cur, "%s%s:%s:%s:%s", cur == buf ? "" : " ", new_user, new_pass, ip,
					enable);
		} else {
			error = 1;
			continue;
		}
	}
	if (!error)
		nvram_set(v->name, buf);
	debug_free(buf);
}
#endif
#ifdef HAVE_MILKFISH

EJ_VISIBLE void validate_aliases(webs_t wp, char *value, struct variable *v)
{
	int i, error = 0;
	char *buf, *cur;
	int count, sof;
	struct variable alias_variables[] = {
	      { argv:ARGV("30") },
	      { argv:ARGV("30") },
		{ NULL },
	}, *which;
	buf = nvram_safe_get("milkfish_ddaliasesnum");
	if (buf == NULL || *(buf) == 0)
		return;
	count = atoi(buf);
	sof = (count * 128) + 1;
	buf = (char *)safe_malloc(sof);
	cur = buf;
	buf[0] = 0;

	for (i = 0; i < count; i++) {
		char alias_user[] = "userXXX";
		char alias_pass[] = "passXXX";
		char *user = "", new_user[200] = "", *pass = "", new_pass[200] = "";

		snprintf(alias_user, sizeof(alias_user), "user%d", i);
		snprintf(alias_pass, sizeof(alias_pass), "pass%d", i);

		user = websGetVar(wp, alias_user, "");
		pass = websGetVar(wp, alias_pass, "");

		which = &alias_variables[0];
		if (strcmp(user, "")) {
			if (!valid_name(wp, user, &which[0], 1)) {
				continue;
			} else {
				httpd_filter_name(user, new_user, sizeof(new_user), SET);
			}
		}

		if (strcmp(pass, "")) {
			if (!valid_name(wp, pass, &which[1], 1)) {
				continue;
			} else {
				httpd_filter_name(pass, new_pass, sizeof(new_pass), SET);
			}
		}
		cur += snprintf(cur, buf + sof - cur, "%s%s:%s", cur == buf ? "" : " ", new_user, new_pass);
	}
	if (!error)
		nvram_set(v->name, buf);
	debug_free(buf);
}

EJ_VISIBLE void validate_subscribers(webs_t wp, char *value, struct variable *v)
{
	int i, error = 0;
	char *buf, *cur;
	int count, sof;
	struct variable subscriber_variables[] = {
	      { argv:ARGV("30") },
	      { argv:ARGV("30") },
		{ NULL },
	}, *which;
	buf = nvram_safe_get("milkfish_ddsubscribersnum");
	if (buf == NULL || *(buf) == 0)
		return;
	count = atoi(buf);
	sof = (count * 128) + 1;
	buf = (char *)safe_malloc(sof);
	cur = buf;
	buf[0] = 0;

	for (i = 0; i < count; i++) {
		char subscriber_user[] = "userXXX";
		char subscriber_pass[] = "passXXX";
		char *user = "", new_user[200] = "", *pass = "", new_pass[200] = "";

		snprintf(subscriber_user, sizeof(subscriber_user), "user%d", i);
		snprintf(subscriber_pass, sizeof(subscriber_pass), "pass%d", i);

		user = websGetVar(wp, subscriber_user, "");
		pass = websGetVar(wp, subscriber_pass, "");

		which = &subscriber_variables[0];
		if (strcmp(user, "")) {
			if (!valid_name(wp, user, &which[0], 1)) {
				continue;
			} else {
				httpd_filter_name(user, new_user, sizeof(new_user), SET);
			}
		}

		if (strcmp(pass, "")) {
			if (!valid_name(wp, pass, &which[1], 1)) {
				continue;
			} else {
				httpd_filter_name(pass, new_pass, sizeof(new_pass), SET);
			}
		}
		cur += snprintf(cur, buf + sof - cur, "%s%s:%s", cur == buf ? "" : " ", new_user, new_pass);
	}
	if (!error)
		nvram_set(v->name, buf);
	debug_free(buf);
}
#endif

#ifdef HAVE_RADLOCAL
EJ_VISIBLE void validate_iradius(webs_t wp, char *value, struct variable *v)
{
	char username[32] = "iradiusxxx_name";
	char active[32] = "iradiusxxx_active";
	char del[32] = "iradiusxxx_delete";

	char *sln = nvram_safe_get("iradius_count");

	if (sln == NULL || *(sln) == 0)
		return;
	int leasenum = atoi(sln);

	if (leasenum == 0)
		return;
	char *leases;
	int i;

	leases = (char *)calloc((128 * leasenum) + 1, 1);
	int leasen = 0;

	cprintf("build mac list\n");

	struct timeval now;

	gettimeofday(&now, NULL);

	for (i = 0; i < leasenum; i++) {
		snprintf(del, sizeof(del), "iradius%d_delete", i);
		char *d = websGetVar(wp, del, "");

		cprintf("radius delete = %s\n", d);
		if (strcmp(d, "1") == 0)
			continue;

		snprintf(username, sizeof(username), "iradius%d_name", i);
		strcat(leases, websGetVar(wp, username, "00:00:00:00:00:00"));
		strcat(leases, " ");

		snprintf(active, sizeof(active), "iradius%d_active", i);
		strcat(leases, websGetVar(wp, active, "0"));
		strcat(leases, " ");

		snprintf(active, sizeof(active), "iradius%d_lease", i);
		char *time = websGetVar(wp, active, "-1");
		int t = -1;

		if (strcmp(time, "over"))
			t = atoi(time);
		if (t == -1) {
			strcat(leases, "-1");
		} else {
			char st[32];

			sprintf(st, "%ld", (now.tv_sec + t * 60));
			strcat(leases, st);
		}
		strcat(leases, " ");

		leasen++;
	}

	cprintf("done %s\n", leases);
	nvram_store_collection("iradius", leases);
	cprintf("stored\n");
	char nr[16];

	sprintf(nr, "%d", leasen);
	nvram_set("iradius_count", nr);
	nvram_async_commit();
	debug_free(leases);
}
#endif

#ifdef HAVE_CHILLI
#ifdef HAVE_CHILLILOCAL
EJ_VISIBLE void validate_userlist(webs_t wp, char *value, struct variable *v)
{
	char username[32] = "fon_userxxx_name";
	char password[32] = "fon_userxxx_password";
	char *sln = nvram_safe_get("fon_usernames");

	if (sln == NULL || *(sln) == 0)
		return;
	int leasenum = atoi(sln);

	if (leasenum == 0)
		return;
	char *leases;
	int i;

	leases = (char *)calloc((128 * leasenum) + 1, 1);
	if (!leases)
		return;
	for (i = 0; i < leasenum; i++) {
		snprintf(username, sizeof(username), "fon_user%d_name", i);
		strcat(leases, websGetVar(wp, username, ""));
		strcat(leases, "=");
		snprintf(password, sizeof(password), "fon_user%d_password", i);
		strcat(leases, websGetVar(wp, password, ""));
		strcat(leases, " ");
	}
	nvram_set("fon_userlist", leases);
	nvram_async_commit();
	debug_free(leases);
}

#endif
#endif

void filterstring(char *str, char character)
{
	if (str == NULL)
		return;
	int c;
	int i;
	int len = strlen(str);

	c = 0;
	for (i = 0; i < len; i++) {
		if (str[i] != character)
			str[c++] = str[i];
	}
	str[c++] = 0;
}

char *buildmac(char *in)
{
	char mac[20];
	char *outmac;

	outmac = safe_malloc(20);
	strlcpy(mac, in, 19);
	filterstring(mac, ':');
	filterstring(mac, '-');
	filterstring(mac, ' ');
	if (strlen(mac) != 12) {
		debug_free(outmac);
		return NULL; // error. invalid mac
	}
	int i;
	int c = 0;

	for (i = 0; i < 12; i += 2) {
		outmac[c++] = mac[i];
		outmac[c++] = mac[i + 1];
		if (i < 10)
			outmac[c++] = ':';
	}
	outmac[c++] = 0;
	return outmac;
}

EJ_VISIBLE void validate_openvpnuserpass(webs_t wp, char *value, struct variable *v)
{
	char openvpn_usrname[32] = "openvpnxxx_usrname";
	char openvpn_passwd[32] = "openvpnxxx_passwd";
	char *sln = nvram_safe_get("openvpn_userpassnum");
	//dd_loginfo("OpenVPN", "OpenVPN userpassnum: %s\n", sln );

	if (*(sln) == 0) //string is empty
		return;
	int userpassnum = atoi(sln);

	if (userpassnum <= 0) {
		nvram_unset("openvpn_userpass");
		nvram_async_commit();
		return;
	}

	char *userpass = NULL;
	int i;

	for (i = 0; i < userpassnum; i++) {
		if (i)
			strcat(userpass, " ");
		snprintf(openvpn_usrname, sizeof(openvpn_usrname), "openvpn%d_usrname", i);
		char *usrname = websGetVar(wp, openvpn_usrname, NULL);

		snprintf(openvpn_passwd, sizeof(openvpn_passwd), "openvpn%d_passwd", i);
		char *passwd = websGetVar(wp, openvpn_passwd, NULL);

		if (usrname == NULL || *(usrname) == 0 || passwd == NULL || *(passwd) == 0)
			break;
		if (userpass == NULL)
			userpass = calloc(1, strlen(usrname) + 1 + strlen(passwd) + 2);
		else
			userpass = realloc(userpass, strlen(userpass) + strlen(usrname) + 1 + strlen(passwd) + 2);

		if (!userpass)
			return;
		strcat(userpass, usrname);
		strcat(userpass, "=");
		strcat(userpass, passwd);
	}
	nvram_set("openvpn_userpass", userpass);
	nvram_async_commit();
	debug_free(userpass);
	// also set openvpn_enuserpass
	copytonv(wp, "openvpn_enuserpass");
}

EJ_VISIBLE void validate_staticleases(webs_t wp, char *value, struct variable *v)
{
	char lease_hwaddr[32] = "leasexxx_hwaddr";
	char lease_hostname[32] = "leasexxx_hostname";
	char lease_ip[32] = "leasexxx_ip";
	char lease_time[32] = "leasexxx_time";
	char *sln = nvram_safe_get("static_leasenum");
	char *hwaddr;

	if (*(sln) == 0)
		return;
	int leasenum = atoi(sln);

	if (leasenum <= 0) {
		nvram_unset("static_leases");
		nvram_async_commit();
		return;
	}
	char *leases = NULL;
	int i;

	for (i = 0; i < leasenum; i++) {
		if (i)
			strcat(leases, " ");
		snprintf(lease_hwaddr, sizeof(lease_hwaddr), "lease%d_hwaddr", i);
		hwaddr = websGetVar(wp, lease_hwaddr, NULL);
		if (hwaddr == NULL)
			break;
		char *mac = buildmac(hwaddr);

		if (mac == NULL) {
			debug_free(leases);
			websError(wp, 400, "%s is not a valid MAC adress\n", hwaddr);
			return;
		}
		snprintf(lease_hostname, sizeof(lease_hostname), "lease%d_hostname", i);
		char *hostname = websGetVar(wp, lease_hostname, NULL);

		snprintf(lease_ip, sizeof(lease_ip), "lease%d_ip", i);
		char *ip = websGetVar(wp, lease_ip, "");

		snprintf(lease_time, sizeof(lease_time), "lease%d_time", i);
		char *time = websGetVar(wp, lease_time, "");

		if (hostname == NULL || *(hostname) == 0 || ip == NULL || *(ip) == 0)
			break;
		if (leases == NULL)
			leases = calloc(1, strlen(mac) + 1 + strlen(hostname) + 1 + strlen(ip) + 1 + strlen(time) + 2);
		else
			leases = realloc(leases, strlen(leases) + strlen(mac) + 1 + strlen(hostname) + 1 + strlen(ip) + 1 +
							 strlen(time) + 2);
		if (!leases)
			return;
		strcat(leases, mac);
		debug_free(mac);
		strcat(leases, "=");
		strcat(leases, hostname);
		strcat(leases, "=");
		strcat(leases, ip);
		strcat(leases, "=");
		strcat(leases, time);
	}
	nvram_set("static_leases", leases);
	nvram_async_commit();
	debug_free(leases);
}

EJ_VISIBLE void validate_dhcp_check(webs_t wp, char *value, struct variable *v)
{
	return; // The udhcpd can valid lease table when
	// re-load udhcpd.leases. by honor 2003-08-05
}

EJ_VISIBLE void validate_wds(webs_t wp, char *value, struct variable *v)
{
#ifdef HAVE_MADWIFI
	int h, i, devcount = 0; // changed from 2 to 3
#else
	int h, i, devcount = 1; // changed from 2 to 3
#endif
	struct variable wds_variables[] = {
		{ argv: NULL }, { argv: NULL }, { argv: NULL }, { argv: NULL }, { argv: NULL },
	};

	char *val = NULL;
	char wds[32] = "";
	char wdsif_var[32] = "";
	char enabled_var[32];
	char hwaddr_var[32] = "";
	char ipaddr_var[32] = "";
	char netmask_var[32] = "";
	char desc_var[32] = "";
	char hwaddr[18] = "";
	char ipaddr[16] = "";
	char netmask[16] = "";
	char desc[48] = "";
	char wds_if[32] = { 0 };
	char wds_list[199] = "";
	char *interface = websGetVar(wp, "interface", NULL);

	if (interface == NULL)
		return;

	char wl0wds[32];

	sprintf(wl0wds, "%s_wds", interface);
	nvram_set(wl0wds, "");
	snprintf(wds, sizeof(wds), "%s_br1", interface);
	snprintf(enabled_var, sizeof(enabled_var), "%s_enable", wds);
	cprintf("wds_validate\n");
	/*
	 * validate separate br1 bridge params 
	 */
	if (nvram_matchi(enabled_var, 1)) {
		bzero(ipaddr, sizeof(ipaddr));
		bzero(netmask, sizeof(netmask));

		// disable until validated
		nvram_seti(enabled_var, 0);

		// subnet params validation
		for (i = 0; i < 4; i++) {
			snprintf(ipaddr_var, sizeof(ipaddr_var), "%s_%s%d", wds, "ipaddr", i);
			val = websGetVar(wp, ipaddr_var, NULL);
			if (val) {
				strcat(ipaddr, val);
				if (i < 3)
					strcat(ipaddr, ".");
			} else
				break;

			snprintf(netmask_var, sizeof(netmask_var), "%s_%s%d", wds, "netmask", i);
			val = websGetVar(wp, netmask_var, NULL);
			if (val) {
				strcat(netmask, val);

				if (i < 3)
					strcat(netmask, ".");
			} else
				break;
		}

		if (!valid_ipaddr(wp, ipaddr, &wds_variables[1]) || !valid_netmask(wp, netmask, &wds_variables[2]))
			return;

		snprintf(ipaddr_var, sizeof(ipaddr_var), "%s_%s", wds, "ipaddr");
		snprintf(netmask_var, sizeof(netmask_var), "%s_%s", wds, "netmask");

		nvram_seti(enabled_var, 1);
		snprintf(ipaddr_var, sizeof(ipaddr_var), "%s_%s%d", wds, "ipaddr", i);
		nvram_set(ipaddr_var, ipaddr);
		snprintf(netmask_var, sizeof(netmask_var), "%s_%s%d", wds, "netmask", i);
		nvram_set(netmask_var, netmask);
	} else
		nvram_seti(enabled_var, 0);

	for (h = 1; h <= MAX_WDS_DEVS; h++) {
		bzero(hwaddr, sizeof(hwaddr));
		bzero(desc, sizeof(desc));
		snprintf(wds, sizeof(wds), "%s_wds%d", interface, h);
		snprintf(enabled_var, sizeof(enabled_var), "%s_enable", wds);

		for (i = 0; i < 6; i++) {
			snprintf(hwaddr_var, sizeof(hwaddr_var), "%s_%s%d", wds, "hwaddr", i);
			val = websGetVar(wp, hwaddr_var, NULL);

			if (val) {
				strcat(hwaddr, val);
				if (i < 5)
					strcat(hwaddr, ":");
			}
		}

		if (!valid_hwaddr(wp, hwaddr, &wds_variables[0])) {
			return;
		}

		snprintf(hwaddr_var, sizeof(hwaddr_var), "%s_%s", wds, "hwaddr");
		nvram_set(hwaddr_var, hwaddr);

		snprintf(desc_var, sizeof(desc_var), "%s_%s", wds, "desc");
		val = websGetVar(wp, desc_var, NULL);
		if (val) {
			strcat(desc, val);
			snprintf(desc_var, sizeof(desc_var), "%s_%s", wds, "desc");
			nvram_set(desc_var, desc);
		}

		/*
		 * <lonewolf> 
		 */
		snprintf(desc_var, sizeof(desc_var), "%s_%s", wds, "ospf");
		val = websGetVar(wp, desc_var, "");
		if (val) {
			snprintf(desc_var, sizeof(desc_var), "%s_%s", wds, "ospf");
			nvram_set(desc_var, val);
		}
		/*
		 * </lonewolf> 
		 */

		if (strcmp(hwaddr, "00:00:00:00:00:00") && nvram_invmatchi(enabled_var, 0)) {
			snprintf(wds_list, sizeof(wds_list), "%s %s", wds_list, hwaddr);
		}

		if (nvram_matchi(enabled_var, 1)) {
			bzero(ipaddr, sizeof(ipaddr));
			bzero(netmask, sizeof(netmask));

			// disable until validated
			nvram_seti(enabled_var, 0);

			// subnet params validation
			for (i = 0; i < 4; i++) {
				snprintf(ipaddr_var, sizeof(ipaddr_var), "%s_%s%d", wds, "ipaddr", i);
				val = websGetVar(wp, ipaddr_var, NULL);
				if (val) {
					strcat(ipaddr, val);
					if (i < 3)
						strcat(ipaddr, ".");
				} else
					break;

				snprintf(netmask_var, sizeof(netmask_var), "%s_%s%d", wds, "netmask", i);
				val = websGetVar(wp, netmask_var, NULL);
				if (val) {
					strcat(netmask, val);

					if (i < 3)
						strcat(netmask, ".");
				} else
					break;
			}

			if (!valid_ipaddr(wp, ipaddr, &wds_variables[1]) || !valid_netmask(wp, netmask, &wds_variables[2])) {
				continue;
			}

			snprintf(ipaddr_var, sizeof(ipaddr_var), "%s_%s", wds, "ipaddr");
			snprintf(netmask_var, sizeof(netmask_var), "%s_%s", wds, "netmask");

			nvram_seti(enabled_var, 1);
			nvram_set(ipaddr_var, ipaddr);
			nvram_set(netmask_var, netmask);
		}

		/*
		 * keep the wds devices in sync w enabled entries 
		 */
		snprintf(wdsif_var, sizeof(wdsif_var), "%s_if", wds);
		if (!nvram_matchi(enabled_var, 0)) {
#ifdef HAVE_MADWIFI
			snprintf(wds_if, sizeof(wds_if), "%s.wds%d", interface, (devcount++));
#else
			// quick and dirty
#ifdef HAVE_RT2880
			if (!strcmp(interface, "wl0"))
				snprintf(wds_if, sizeof(wds_if), "wds%d", (devcount++) - 1);
			else if (!strcmp(interface, "wl1"))
				snprintf(wds_if, sizeof(wds_if), "wds%d", (devcount++) + 10 - 1);
#else
			if (!strcmp(interface, "wl0"))
				snprintf(wds_if, sizeof(wds_if), "wds0.%d", (devcount++));
			else if (!strcmp(interface, "wl1"))
				snprintf(wds_if, sizeof(wds_if), "wds1.%d", (devcount++));
#endif
			else
				snprintf(wds_if, sizeof(wds_if), "wds%d.%d", get_wl_instance(interface), (devcount++));
#endif
			nvram_set(wdsif_var, wds_if);
		} else
			nvram_unset(wdsif_var);
	}

	nvram_set(wl0wds, wds_list);
}

EJ_VISIBLE void validate_filter_ip_grp(webs_t wp, char *value, struct variable *v)
{
	D("validate_filter_ip_grp");
	int i = 0;
	char buf[256] = "";
	char *ip0, *ip1, *ip2, *ip3, *ip4, *ip5;
	char *ip_range0_0, *ip_range0_1, *ip_range0_2, *ip_range0_3, *ip_range0_4, *ip_range0_5, *ip_range0_6, *ip_range0_7,
		*ip_range1_0, *ip_range1_1, *ip_range1_2, *ip_range1_3, *ip_range1_4, *ip_range1_5, *ip_range1_6, *ip_range1_7;
	unsigned char ip[10] = { 0, 0, 0, 0, 0, 0, 0 };
	struct variable filter_ip_variables[] = {
	      { argv:ARGV("0", "255") },
	}, *which;
	char _filter_ip[] = "filter_ip_grpXXX";

	// char _filter_rule[] = "filter_ruleXXX";
	// char _filter_tod[] = "filter_todXXX";

	which = &filter_ip_variables[0];

	ip0 = websGetVar(wp, "ip0", "0");
	ip1 = websGetVar(wp, "ip1", "0");
	ip2 = websGetVar(wp, "ip2", "0");
	ip3 = websGetVar(wp, "ip3", "0");
	ip4 = websGetVar(wp, "ip4", "0");
	ip5 = websGetVar(wp, "ip5", "0");
	ip_range0_0 = websGetVar(wp, "ip_range0_0", "0");
	ip_range0_1 = websGetVar(wp, "ip_range0_1", "0");
	ip_range0_2 = websGetVar(wp, "ip_range0_2", "0");
	ip_range0_3 = websGetVar(wp, "ip_range0_3", "0");
	ip_range0_4 = websGetVar(wp, "ip_range0_4", "0");
	ip_range0_5 = websGetVar(wp, "ip_range0_5", "0");
	ip_range0_6 = websGetVar(wp, "ip_range0_6", "0");
	ip_range0_7 = websGetVar(wp, "ip_range0_7", "0");
	ip_range1_0 = websGetVar(wp, "ip_range1_0", "0");
	ip_range1_1 = websGetVar(wp, "ip_range1_1", "0");
	ip_range1_2 = websGetVar(wp, "ip_range1_2", "0");
	ip_range1_3 = websGetVar(wp, "ip_range1_3", "0");
	ip_range1_4 = websGetVar(wp, "ip_range1_4", "0");
	ip_range1_5 = websGetVar(wp, "ip_range1_5", "0");
	ip_range1_6 = websGetVar(wp, "ip_range1_6", "0");
	ip_range1_7 = websGetVar(wp, "ip_range1_7", "0");

	if (!valid_range(wp, ip0, &which[0]) || !valid_range(wp, ip1, &which[0]) || !valid_range(wp, ip2, &which[0]) ||
	    !valid_range(wp, ip3, &which[0]) || !valid_range(wp, ip4, &which[0]) || !valid_range(wp, ip5, &which[0]) ||
	    !valid_range(wp, ip_range0_0, &which[0]) || !valid_range(wp, ip_range0_1, &which[0]) ||
	    !valid_range(wp, ip_range0_2, &which[0]) || !valid_range(wp, ip_range0_3, &which[0]) ||
	    !valid_range(wp, ip_range0_4, &which[0]) || !valid_range(wp, ip_range0_5, &which[0]) ||
	    !valid_range(wp, ip_range0_6, &which[0]) || !valid_range(wp, ip_range0_7, &which[0]) ||
	    !valid_range(wp, ip_range1_0, &which[0]) || !valid_range(wp, ip_range1_1, &which[0]) ||
	    !valid_range(wp, ip_range1_2, &which[0]) || !valid_range(wp, ip_range1_3, &which[0]) ||
	    !valid_range(wp, ip_range1_4, &which[0]) || !valid_range(wp, ip_range1_5, &which[0]) ||
	    !valid_range(wp, ip_range1_6, &which[0]) || !valid_range(wp, ip_range1_7, &which[0])) {
		D("invalid range, return");
		return;
	}

	if (atoi(ip0))
		ip[i++] = atoi(ip0);
	if (atoi(ip1))
		ip[i++] = atoi(ip1);
	if (atoi(ip2))
		ip[i++] = atoi(ip2);
	if (atoi(ip3))
		ip[i++] = atoi(ip3);
	if (atoi(ip4))
		ip[i++] = atoi(ip4);
	if (atoi(ip5))
		ip[i++] = atoi(ip5);

	//      if (atoi(ip_range0_0) > atoi(ip_range0_1))
	//              SWAP(ip_range0_0, ip_range0_1);

	//      if (atoi(ip_range1_0) > atoi(ip_range1_1))
	//              SWAP(ip_range1_0, ip_range1_1);

	sprintf(buf, "%d %d %d %d %d %d %s.%s.%s.%s-%s.%s.%s.%s %s.%s.%s.%s-%s.%s.%s.%s", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5],
		ip_range0_0, ip_range0_1, ip_range0_2, ip_range0_3, ip_range0_4, ip_range0_5, ip_range0_6, ip_range0_7, ip_range1_0,
		ip_range1_1, ip_range1_2, ip_range1_3, ip_range1_4, ip_range1_5, ip_range1_6, ip_range1_7);

	snprintf(_filter_ip, sizeof(_filter_ip), "filter_ip_grp%d", wp->p->filter_id);
	nvram_set(_filter_ip, buf);

	// snprintf(_filter_rule, sizeof(_filter_rule), "filter_rule%s",
	// wp->p->filter_id);
	// snprintf(_filter_tod, sizeof(_filter_tod), "filter_tod%s",
	// wp->p->filter_id);
	// if(nvram_match(_filter_rule, "")){
	// nvram_set(_filter_rule, "$STAT:1$NAME:$$");
	// nvram_set(_filter_tod, "0:0 23:59 0-6");
	// }
	D("success return");
}

/*
 * Example: tcp:100-200 udp:210-220 both:250-260 
 */

EJ_VISIBLE void validate_filter_port(webs_t wp, char *value, struct variable *v)
{
	int i;
	char buf[1000] = "", *cur = buf;
	struct variable filter_port_variables[] = {
	      { argv:ARGV("0",
		     "65535") },
	      { argv:ARGV("0",
		     "65535") },
	}, *which;
	D("validate_filter_port");
	which = &filter_port_variables[0];

	for (i = 0; i < FILTER_PORT_NUM; i++) {
		char filter_port[] = "protoXXX";
		char filter_port_start[] = "startXXX";
		char filter_port_end[] = "endXXX";
		char *port, *start, *end;
		char *temp;

		snprintf(filter_port, sizeof(filter_port), "proto%d", i);
		snprintf(filter_port_start, sizeof(filter_port_start), "start%d", i);
		snprintf(filter_port_end, sizeof(filter_port_end), "end%d", i);
		port = websGetVar(wp, filter_port, NULL);
		start = websGetVar(wp, filter_port_start, NULL);
		end = websGetVar(wp, filter_port_end, NULL);

		if (!port || !start || !end)
			continue;

		if (!*start && !*end)
			continue;

		if ((!strcmp(start, "0") || !strcmp(start, "")) && (!strcmp(end, "0") || !strcmp(end, "")))
			continue;

		if (!*start || !*end) {
			// websWrite(wp, "Invalid <b>%s</b>: must specify a LAN Port
			// Range<br>", v->longname);
			continue;
		}
		if (!valid_range(wp, start, &which[0]) || !valid_range(wp, end, &which[1])) {
			continue;
		}
		if (atoi(start) > atoi(end)) {
			temp = start;
			start = end;
			end = temp;
		}
		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s:%d-%d", cur == buf ? "" : " ", port, atoi(start), atoi(end));
	}

	nvram_set(v->name, buf);
	D("success return");
}

EJ_VISIBLE void validate_filter_dport_grp(webs_t wp, char *value, struct variable *v)
{
	int i;
	char buf[1000] = "", *cur = buf;
	struct variable filter_port_variables[] = {
	      { argv:ARGV("0",
		     "65535") },
	      { argv:ARGV("0",
		     "65535") },
	}, *which;
	char _filter_port[] = "filter_dport_grpXXX";

	// char _filter_rule[] = "filter_ruleXXX";
	// char _filter_tod[] = "filter_todXXX";
	D("validate_filter-dport-grp");
	which = &filter_port_variables[0];

	for (i = 0; i < FILTER_PORT_NUM; i++) {
		char filter_port[] = "protoXXX";
		char filter_port_start[] = "startXXX";
		char filter_port_end[] = "endXXX";
		char *port, *start, *end;
		char *temp;

		snprintf(filter_port, sizeof(filter_port), "proto%d", i);
		snprintf(filter_port_start, sizeof(filter_port_start), "start%d", i);
		snprintf(filter_port_end, sizeof(filter_port_end), "end%d", i);
		port = websGetVar(wp, filter_port, NULL);
		start = websGetVar(wp, filter_port_start, NULL);
		end = websGetVar(wp, filter_port_end, NULL);

		if (!port || !start || !end)
			continue;

		if (!*start && !*end)
			continue;

		if ((!strcmp(start, "0") || !strcmp(start, "")) && (!strcmp(end, "0") || !strcmp(end, "")))
			continue;

		if (!*start || !*end) {
			// websWrite(wp, "Invalid <b>%s</b>: must specify a LAN Port
			// Range<br>", v->longname);
			continue;
		}
		if (!valid_range(wp, start, &which[0]) || !valid_range(wp, end, &which[1])) {
			continue;
		}
		if (atoi(start) > atoi(end)) {
			temp = start;
			start = end;
			end = temp;
		}
		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s:%d-%d", cur == buf ? "" : " ", port, atoi(start), atoi(end));
	}

	snprintf(_filter_port, sizeof(_filter_port), "filter_dport_grp%d", wp->p->filter_id);
	nvram_set(_filter_port, buf);

	// snprintf(_filter_rule, sizeof(_filter_rule), "filter_rule%s",
	// wp->p->filter_id);
	// snprintf(_filter_tod, sizeof(_filter_tod), "filter_tod%s",
	// wp->p->filter_id);
	// if(nvram_match(_filter_rule, "")){
	// nvram_set(_filter_rule, "$STAT:1$NAME:$$");
	// nvram_set(_filter_tod, "0:0 23:59 0-6");
	// }
	D("success return");
}

/*
 * Example: 2 00:11:22:33:44:55 00:11:22:33:44:56 
 */

EJ_VISIBLE void validate_filter_mac_grp(webs_t wp, char *value, struct variable *v)
{
	int i;
	char buf[1000] = "", *cur = buf;
	char _filter_mac[] = "filter_mac_grpXXX";

	// char _filter_rule[] = "filter_ruleXXX";
	// har _filter_tod[] = "filter_todXXX";
	D("validate_filter__mac_grp");

	for (i = 0; i < FILTER_MAC_NUM; i++) {
		char filter_mac[] = "macXXX";
		char *mac, mac1[20] = "";

		snprintf(filter_mac, sizeof(filter_mac), "mac%d", i);

		mac = websGetVar(wp, filter_mac, NULL);
		if (!mac)
			continue;

		if (strcmp(mac, "") && strcmp(mac, "00:00:00:00:00:00") && strcmp(mac, "000000000000")) {
			if (strlen(mac) == 12) {
				char hex[] = "XX";
				unsigned char h;

				while (*mac) {
					strncpy(hex, mac, 2);
					h = (unsigned char)strtoul(hex, NULL, 16);
					if (*(mac1))
						snprintf(mac1 + strlen(mac1), sizeof(mac1) - strlen(mac1), ":");
					snprintf(mac1 + strlen(mac1), sizeof(mac1) - strlen(mac1), "%02X", h);
					mac += 2;
				}
				mac1[17] = '\0';
			} else if (strlen(mac) == 17) {
				strcpy(mac1, mac);
			}
			if (!valid_hwaddr(wp, mac1, v)) {
				continue;
			}
		} else {
			continue;
		}
		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s", cur == buf ? "" : " ", mac1);
	}

	snprintf(_filter_mac, sizeof(_filter_mac), "filter_mac_grp%d", wp->p->filter_id);
	nvram_set(_filter_mac, buf);

	// snprintf(_filter_rule, sizeof(_filter_rule), "filter_rule%s",
	// wp->p->filter_id);
	// snprintf(_filter_tod, sizeof(_filter_tod), "filter_tod%s",
	// wp->p->filter_id);
	// if(nvram_match(_filter_rule, "")){
	// nvram_set(_filter_rule, "$STAT:1$NAME:$$");
	// nvram_set(_filter_tod, "0:0 23:59 0-6");
	// }
	D("success return");
}

/*
 * Format: url0=www.kimo.com.tw, ...  keywd0=sex, ... 
 */
EJ_VISIBLE void validate_filter_web(webs_t wp, char *value, struct variable *v)
{
	int i;
	char buf[1000] = "", *cur = buf;
	char buf1[1000] = "", *cur1 = buf1;
	char filter_host[] = "filter_web_hostXXX";
	char filter_url[] = "filter_web_urlXXX";

	D("validate_filter_web");
	/*
	 * Handle Website Blocking by URL Address 
	 */
	for (i = 0; i < 15; i++) {
		char filter_host[] = "hostXXX";
		char *host;

		snprintf(filter_host, sizeof(filter_host), "host%d", i);
		host = websGetVar(wp, filter_host, "");

		if (!strcmp(host, ""))
			continue;
		int offset = 0;

		if (startswith(host, "http://"))
			offset = 7;
		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s", cur == buf ? "" : "<&nbsp;>", &host[offset]);
	}

	if (strcmp(buf, ""))
		strcat(buf, "<&nbsp;>");

	snprintf(filter_host, sizeof(filter_host), "filter_web_host%d", wp->p->filter_id);
	nvram_set(filter_host, buf);

	/*
	 * Handle Website Blocking by Keyword 
	 */
	for (i = 0; i < 16; i++) {
		char filter_url[] = "urlXXX";
		char *url;

		snprintf(filter_url, sizeof(filter_url), "url%d", i);
		url = websGetVar(wp, filter_url, "");

		if (!strcmp(url, ""))
			continue;

		cur1 += snprintf(cur1, buf1 + sizeof(buf1) - cur1, "%s%s", cur1 == buf1 ? "" : "<&nbsp;>", url);
	}
	if (strcmp(buf1, ""))
		strcat(buf1, "<&nbsp;>");

	snprintf(filter_url, sizeof(filter_url), "filter_web_url%d", wp->p->filter_id);
	nvram_set(filter_url, buf1);
	D("everything okay");
}

/*
 * Example: name:on:both:1000-2000>3000-4000 
 */

EJ_VISIBLE void validate_port_trigger(webs_t wp, char *value, struct variable *v)
{
	int i, error = 0;
	char *buf, *cur, *newbuf, *entry;
	int count, sof;
	struct variable trigger_variables[] = {
	      { argv:ARGV("12") },
	      { argv:ARGV("0", "65535") },
	      { argv:ARGV("0", "65535") },
	      { argv:ARGV("0", "65535") },
	      { argv:ARGV("0", "65535") },
	}, *which;

	buf = nvram_safe_get("trigger_entries");
	if (buf == NULL || *(buf) == 0)
		return;
	count = atoi(buf);
	sof = (count * 46) + 1;
	buf = (char *)safe_malloc(sof);
	cur = buf;
	bzero(buf, sof);

	for (i = 0; i < count; i++) {
		char trigger_name[] = "nameXXX";
		char trigger_enable[] = "enableXXX";
		char trigger_i_from[] = "i_fromXXX";
		char trigger_i_to[] = "i_toXXX";
		char trigger_o_from[] = "o_fromXXX";
		char trigger_o_to[] = "o_toXXX";
		char trigger_proto[] = "proXXX";
		char *name = "", *enable, new_name[200] = "", *i_from = "", *i_to = "", *o_from = "", *o_to = "", *proto = "both";

		snprintf(trigger_name, sizeof(trigger_name), "name%d", i);
		snprintf(trigger_enable, sizeof(trigger_enable), "enable%d", i);
		snprintf(trigger_i_from, sizeof(trigger_i_from), "i_from%d", i);
		snprintf(trigger_i_to, sizeof(trigger_i_to), "i_to%d", i);
		snprintf(trigger_o_from, sizeof(trigger_o_from), "o_from%d", i);
		snprintf(trigger_o_to, sizeof(trigger_o_to), "o_to%d", i);
		snprintf(trigger_proto, sizeof(trigger_proto), "pro%d", i);

		name = websGetVar(wp, trigger_name, "");
		enable = websGetVar(wp, trigger_enable, "off");
		i_from = websGetVar(wp, trigger_i_from, NULL);
		i_to = websGetVar(wp, trigger_i_to, NULL);
		o_from = websGetVar(wp, trigger_o_from, NULL);
		o_to = websGetVar(wp, trigger_o_to, NULL);
		proto = websGetVar(wp, trigger_proto, "both");
		which = &trigger_variables[0];

		if (!i_from || !i_to || !o_from || !o_to)
			continue;

		if ((!strcmp(i_from, "0") || !strcmp(i_from, "")) && (!strcmp(i_to, "0") || !strcmp(i_to, "")) &&
		    (!strcmp(o_from, "0") || !strcmp(o_from, "")) && (!strcmp(o_to, "0") || !strcmp(o_to, "")))
			continue;

		if (!strcmp(i_from, "0") || !strcmp(i_from, ""))
			i_from = i_to;
		if (!strcmp(i_to, "0") || !strcmp(i_to, ""))
			i_to = i_from;
		if (!strcmp(o_from, "0") || !strcmp(o_from, ""))
			o_from = o_to;
		if (!strcmp(o_to, "0") || !strcmp(o_to, ""))
			o_to = o_from;

		if (atoi(i_from) > atoi(i_to))
			SWAP(i_from, i_to);

		if (atoi(o_from) > atoi(o_to))
			SWAP(o_from, o_to);

		if (strcmp(name, "")) {
			if (!valid_name(wp, name, &which[0], 1)) {
				error = 1;
				continue;
			} else {
				httpd_filter_name(name, new_name, sizeof(new_name), SET);
			}
		}

		if (!valid_range(wp, i_from, &which[1]) || !valid_range(wp, i_to, &which[2]) ||
		    !valid_range(wp, o_from, &which[3]) || !valid_range(wp, o_to, &which[4])) {
			error = 1;
			continue;
		}

		int len = 32;
		len += strlen(new_name) + 1;
		len += strlen(enable) + 1;
		len += strlen(proto) + 1;
		len += strlen(i_from) + 1;
		len += strlen(i_to) + 1;
		len += strlen(o_from) + 1;
		len += strlen(o_to) + 1;

		if (*(buf)) {
			len++;
		}
		entry = (char *)safe_malloc(len);
		snprintf(entry, len, "%s%s:%s:%s:%s-%s>%s-%s", strlen(buf) > 0 ? " " : "", new_name, enable, proto, i_from, i_to,
			 o_from, o_to);

		/*cur += snprintf(cur, buf + sof - cur, "%s%s:%s:%s:%s-%s>%s-%s",
		   cur == buf ? "" : " ", new_name, enable, proto,
		   i_from, i_to, o_from, o_to); */
		newbuf = (char *)safe_malloc(strlen(buf) + len + 1);
		strcpy(newbuf, buf);
		strcat(newbuf, entry);
		debug_free(entry);
		debug_free(buf);
		buf = newbuf;
	}

	if (!error) {
		nvram_set(v->name, buf);
	}
	debug_free(buf);
}

EJ_VISIBLE void validate_blocked_service(webs_t wp, char *value, struct variable *v)
{
	int i;
	char buf[1000] = "", *cur = buf;
	char port_grp[] = "filter_port_grpXXX";

	D("validate_blocked_service");
	char filter[32];
	sprintf(filter, "numfilterservice%d", wp->p->filter_id);
	int numfilters = nvram_default_geti(filter, 4);
	for (i = 0; i < numfilters; i++) {
		char blocked_service[] = "blocked_serviceXXX";
		char *service;

		snprintf(blocked_service, sizeof(blocked_service), "blocked_service%d", i);
		service = websGetVar(wp, blocked_service, NULL);
		if (!service || !strcmp(service, "None"))
			continue;

		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s", service, "<&nbsp;>");
		// cur == buf ? "" : "<&nbsp;>", service);
	}

	snprintf(port_grp, sizeof(port_grp), "filter_port_grp%d", wp->p->filter_id);
	nvram_set(port_grp, buf);
	D("right");
}

/*
 * validates the p2p catchall filter 
 */
EJ_VISIBLE void validate_catchall(webs_t wp, char *value, struct variable *v)
{
	char *p2p;
	char port_grp[] = "filter_p2p_grpXXX";

	p2p = websGetVar(wp, "filter_p2p", NULL);
	if (p2p) {
		snprintf(port_grp, sizeof(port_grp), "filter_p2p_grp%d", wp->p->filter_id);
		nvram_set(port_grp, p2p);
	}

	return;
}

void save_olsrd(webs_t wp);
void addDeletion_route(char *word);
void addDeletion_pbr(char *word);

#define ROUTE_LINE_SIZE \
	sizeof("255.255.255.255:255.255.255.255:255.255.255.255:65536:1234567890123456:1:12345:255.255.255.255:nowhere:2147483648:65536:65536")
#define ROUTE_NAME_SIZE sizeof("$NAME:1234567890123456789012345:$$")
EJ_VISIBLE void validate_static_route(webs_t wp, char *value, struct variable *v)
{
#ifdef HAVE_OLSRD
	save_olsrd(wp);
#endif

	int i, tmp = 1;
	char word[256], *next;
	char backuproute[256];
	struct variable static_route_variables[] = {
		{ argv: NULL },
		{ argv: NULL },
		{ argv: NULL },
		{ argv: ARGV("lan", "wan") },
	};
	char *old;
	char *old_name;
	char *buf;
	char *buf_name;

	char *cur;
	char *cur_name;

	char *name, ipaddr[20], netmask[20], gateway[20], src[20], *metric, *ifname, *page, *nat;
	char new_name[80];
	char temp[60], *val = NULL;

	buf = safe_malloc(STATIC_ROUTE_PAGE * ROUTE_LINE_SIZE + 1);
	buf_name = safe_malloc(STATIC_ROUTE_PAGE * ROUTE_NAME_SIZE + 1);
	old = safe_malloc(STATIC_ROUTE_PAGE * ROUTE_LINE_SIZE + 1);
	old_name = safe_malloc(STATIC_ROUTE_PAGE * ROUTE_NAME_SIZE + 1);
	buf[0] = 0;
	buf_name[0] = 0;
	bzero(old, STATIC_ROUTE_PAGE * ROUTE_LINE_SIZE + 1);
	bzero(old_name, STATIC_ROUTE_PAGE * ROUTE_NAME_SIZE + 1);
	cur = buf;
	cur_name = buf_name;

	name = websGetVar(wp, "route_name", ""); // default empty if no find
	// route_name
	metric = websGetVar(wp, "route_metric", "0");
	/*
	 * validate ip address 
	 */
	strcpy(ipaddr, "");
	for (i = 0; i < 4; i++) {
		snprintf(temp, sizeof(temp), "%s_%d", "route_ipaddr", i);
		val = websGetVar(wp, temp, NULL);
		if (val) {
			strcat(ipaddr, val);
			if (i < 3)
				strcat(ipaddr, ".");
		} else {
			// free (ipaddr);
			debug_free(old_name);
			debug_free(old);
			debug_free(buf_name);
			debug_free(buf);
			return;
		}
	}

	/*
	 * validate netmask 
	 */
	int cidr_nm = websGetVari(wp, "route_netmask", 0);
	cidr_to_nm(netmask, sizeof(netmask), cidr_nm);
	/*
	 * validate gateway 
	 */
	strcpy(gateway, "");
	for (i = 0; i < 4; i++) {
		snprintf(temp, sizeof(temp), "%s_%d", "route_gateway", i);
		val = websGetVar(wp, temp, NULL);
		if (val) {
			strcat(gateway, val);
			if (i < 3)
				strcat(gateway, ".");
		} else {
			// free (gateway);
			// free (netmask);
			// free (ipaddr);
			debug_free(old_name);
			debug_free(old);
			debug_free(buf_name);
			debug_free(buf);
			return;
		}
	}

	strcpy(src, "");
	for (i = 0; i < 4; i++) {
		snprintf(temp, sizeof(temp), "%s_%d", "route_src", i);
		val = websGetVar(wp, temp, NULL);
		if (val) {
			strcat(src, val);
			if (i < 3)
				strcat(src, ".");
		} else {
			strcpy(src, "0.0.0.0");
			break;
		}
	}

	page = websGetVar(wp, "route_page", NULL);
	ifname = websGetVar(wp, "route_ifname", NULL);
	nat = websGetVar(wp, "route_nat", 0);
	int src_en = websGetVari(wp, "src_en", 0);
	int scope_en = websGetVari(wp, "scope_en", 0);
	int table_en = websGetVari(wp, "table_en", 0);
	int mtu_en = websGetVari(wp, "mtu_en", 0);
	int advmss_en = websGetVari(wp, "advmss_en", 0);
	int flags = 0;
	flags |= src_en ? 1 << 0 : 0;
	flags |= scope_en ? 1 << 1 : 0;
	flags |= table_en ? 1 << 2 : 0;
	flags |= mtu_en ? 1 << 3 : 0;
	flags |= advmss_en ? 1 << 4 : 0;
	char *scope = websGetVar(wp, "route_scope", "link");
	char *table = websGetVar(wp, "route_table", "0");
	char *mtu = websGetVar(wp, "route_mtu", "1500");
	char *advmss = websGetVar(wp, "route_advmss", "1460");
	if (!page || !metric || !ifname) {
		debug_free(old_name);
		debug_free(old);
		debug_free(buf_name);
		debug_free(buf);
		return;
	}
	// Allow Defaultroute here
	if (!strcmp(ipaddr, "0.0.0.0") && !strcmp(netmask, "0.0.0.0") && strcmp(gateway, "0.0.0.0")) {
		tmp = 1;
		goto write_nvram;
	}
	if ((!strcmp(ipaddr, "0.0.0.0") || !strcmp(ipaddr, "")) && (!strcmp(netmask, "0.0.0.0") || !strcmp(netmask, "")) &&
	    (!strcmp(gateway, "0.0.0.0") || !strcmp(gateway, ""))) {
		tmp = 0;
		goto write_nvram;
	}
	// if (!valid_choice (wp, ifname, &static_route_variables[3]))
	// {
	// free (gateway);
	// free (netmask);
	// free (ipaddr);

	// return;
	// }
	if (!*ipaddr) {
		websDebugWrite(wp, "Invalid <b>%s</b>: must specify an IP Address<br>", v->longname);
		// free (gateway);
		// free (netmask);
		// free (ipaddr);
		debug_free(old_name);
		debug_free(old);
		debug_free(buf_name);
		debug_free(buf);

		return;
	}
	if (!*netmask) {
		websDebugWrite(wp, "Invalid <b>%s</b>: must specify a Subnet Mask<br>", v->longname);
		// free (gateway);
		// free (netmask);
		// free (ipaddr);
		debug_free(old_name);
		debug_free(old);
		debug_free(buf_name);
		debug_free(buf);

		return;
	}
	if (!valid_ipaddr(wp, ipaddr, &static_route_variables[0]) || !valid_netmask(wp, netmask, &static_route_variables[1]) ||
	    !valid_ipaddr(wp, gateway, &static_route_variables[2])) {
		// free (gateway);
		// free (netmask);
		// free (ipaddr);
		debug_free(old_name);
		debug_free(old);
		debug_free(buf_name);
		debug_free(buf);

		return;
	}

	/*
	 * save old value in nvram 
	 */

write_nvram:
	if (!strcmp(ifname, "lan")) {
		ifname = nvram_safe_get("lan_ifname");
		static_route_variables[2].argv = NULL;
	}
	if (!strcmp(ifname, "wan")) {
		ifname = nvram_safe_get("wan_ifname");
		static_route_variables[2].argv = NULL;
	} else {
		static_route_variables[2].argv = NULL;
	}

	for (i = 0; i < STATIC_ROUTE_PAGE; i++) {
		strcpy(&old[i * ROUTE_LINE_SIZE], "");
		strcpy(&old_name[i * ROUTE_NAME_SIZE], "");
	}
	i = 0;
	foreach(word, nvram_safe_get("static_route"), next)
	{
		strcpy(&old[i * ROUTE_LINE_SIZE], word);
		i++;
	}
	i = 0;
	foreach(word, nvram_safe_get("static_route_name"), next)
	{
		strcpy(&old_name[i * ROUTE_NAME_SIZE], word);
		i++;
	}

	strcpy(backuproute, &old[atoi(page) * ROUTE_LINE_SIZE]);

	if (!tmp) {
		if (*(backuproute)) {
			addDeletion_route(backuproute);
			bzero(backuproute, strlen(backuproute));
		}

		snprintf(&old[atoi(page) * ROUTE_LINE_SIZE], ROUTE_LINE_SIZE, "%s", "");
		snprintf(&old_name[atoi(page) * ROUTE_NAME_SIZE], ROUTE_NAME_SIZE, "%s", "");
	} else {
		snprintf(&old[atoi(page) * ROUTE_LINE_SIZE], ROUTE_LINE_SIZE, "%s:%s:%s:%s:%s:%s:%X:%s:%s:%s:%s:%s", ipaddr,
			 netmask, gateway, metric, ifname, nat, flags, src, scope, table, mtu, advmss);
		httpd_filter_name(name, new_name, sizeof(new_name), SET);
		snprintf(&old_name[atoi(page) * ROUTE_NAME_SIZE], ROUTE_NAME_SIZE, "$NAME:%s$$", new_name);
	}
	if (strcmp(backuproute, &old[atoi(page) * ROUTE_LINE_SIZE])) {
		if (*(backuproute)) {
			//nvram_set("nowebaction","1");
			//addAction("static_route_del");
			addDeletion_route(backuproute);
		}
	}

	for (i = 0; i < STATIC_ROUTE_PAGE; i++) {
		//if (strcmp(old[i], ""))
		//      cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s",
		//                      cur == buf ? "" : " ", old[i]);
		if (strcmp(&old_name[i * ROUTE_NAME_SIZE], "")) {
			cur += snprintf(cur, buf + (STATIC_ROUTE_PAGE * ROUTE_LINE_SIZE) - cur, "%s%s", cur == buf ? "" : " ",
					&old[i * ROUTE_LINE_SIZE]);
			cur_name += snprintf(cur_name, buf_name + (STATIC_ROUTE_PAGE * ROUTE_NAME_SIZE) - cur_name, "%s%s",
					     cur_name == buf_name ? "" : " ", &old_name[i * ROUTE_NAME_SIZE]);
		}
	}

	if (!strcmp(websGetVar(wp, "action", ""), "ApplyTake"))
		delete_old_routes();

	nvram_set(v->name, buf);
	nvram_set("static_route_name", buf_name);
#ifdef HAVE_MICRO
	nvram_async_commit();
#endif
	debug_free(old_name);
	debug_free(old);
	debug_free(buf_name);
	debug_free(buf);

	// if (ipaddr)
	// free (ipaddr);
	// if (netmask)
	// free (netmask);
	// if (gateway)
	// free (gateway);
}

#ifndef HAVE_MICRO
#define PBR_LINE_SIZE \
	sizeof("FFFFF:255.255.255.255/32:255.255.255.255/32:65536:255:0xffffffff/0xffffffff:2147483648:2147483648:2147483648:1234567890123456:255.255.255.255:unreachable:FRAGMENT:65535:65535")
EJ_VISIBLE void validate_pbr_rule(webs_t wp, char *value, struct variable *v)
{
	int i, tmp = 1;
	char word[256], *next;
	char backuproute[256];
	struct variable static_rule_variables[] = {
		{ argv: NULL },
		{ argv: NULL },
		{ argv: NULL },
		{ argv: ARGV("lan", "wan") },
	};
	char *old;
	char *old_name;
	char *buf;
	char *buf_name;

	char *cur;
	char *cur_name;

	char *name;
	char new_name[80];
	char temp[60], *val = NULL;

	buf = safe_malloc(STATIC_ROUTE_PAGE * PBR_LINE_SIZE + 1);
	buf_name = safe_malloc(STATIC_ROUTE_PAGE * ROUTE_NAME_SIZE + 1);
	old = safe_malloc(STATIC_ROUTE_PAGE * PBR_LINE_SIZE + 1);
	old_name = safe_malloc(STATIC_ROUTE_PAGE * ROUTE_NAME_SIZE + 1);
	buf[0] = 0;
	buf_name[0] = 0;
	bzero(old, STATIC_ROUTE_PAGE * PBR_LINE_SIZE + 1);
	bzero(old_name, STATIC_ROUTE_PAGE * ROUTE_NAME_SIZE + 1);
	cur = buf;
	cur_name = buf_name;
	name = websGetVar(wp, "rule_name", ""); // default empty if no find
	int not = websGetVari(wp, "not", 0);
	int from_en = websGetVari(wp, "from_en", 0);
	int to_en = websGetVari(wp, "to_en", 0);
	int priority_en = websGetVari(wp, "priority_en", 0);
	int tos_en = websGetVari(wp, "tos_en", 0);
	int fwmark_en = websGetVari(wp, "fwmark_en", 0);
	int realms_en = websGetVari(wp, "realms_en", 0);
	int table_en = websGetVari(wp, "pbr_table_en", 0);
	int suppress_prefixlength_en = websGetVari(wp, "suppress_prefixlength_en", 0);
	int iif_en = websGetVari(wp, "iif_en", 0);
	int oif_en = websGetVari(wp, "oif_en", 0);
	int nat_en = websGetVari(wp, "nat_en", 0);
	int type_en = websGetVari(wp, "type_en", 0);
	int ipproto_en = websGetVari(wp, "ipproto_en", 0);
	int sport_en = websGetVari(wp, "sport_en", 0);
	int dport_en = websGetVari(wp, "dport_en", 0);

	int flags = 0;
	flags |= not ? 1 << 0 : 0;
	flags |= from_en ? 1 << 1 : 0;
	flags |= to_en ? 1 << 2 : 0;
	flags |= priority_en ? 1 << 3 : 0;
	flags |= tos_en ? 1 << 4 : 0;
	flags |= fwmark_en ? 1 << 5 : 0;
	flags |= realms_en ? 1 << 6 : 0;
	flags |= table_en ? 1 << 7 : 0;
	flags |= suppress_prefixlength_en ? 1 << 8 : 0;
	flags |= iif_en ? 1 << 9 : 0;
	flags |= nat_en ? 1 << 10 : 0;
	flags |= type_en ? 1 << 11 : 0;
	flags |= ipproto_en ? 1 << 12 : 0;
	flags |= sport_en ? 1 << 13 : 0;
	flags |= dport_en ? 1 << 14 : 0;
	flags |= oif_en ? 1 << 15 : 0;
	char *page = websGetVar(wp, "rule_page", NULL);
	if (!name[0] || !flags || !page) {
		debug_free(old_name);
		debug_free(old);
		debug_free(buf_name);
		debug_free(buf);
		return; //nothing stored, we ignore it
	}
	char *priority = websGetVar(wp, "rule_priority", "0");
	char *tos = websGetVar(wp, "rule_tos", "0");
	char *fwmark = websGetVar(wp, "rule_fwmark", "0");
	char *fwmask = websGetVar(wp, "rule_fwmask", NULL);
	char *realms = websGetVar(wp, "rule_realms", "0");
	char *table = websGetVar(wp, "rule_table", "0");
	char *suppress_prefixlength = websGetVar(wp, "rule_suppress_prefixlength", "");
	char *iif = websGetVar(wp, "rule_iif", "none");
	char *oif = websGetVar(wp, "rule_oif", "none");
	char *type = websGetVar(wp, "rule_type", "0");
	char *sport_from = websGetVar(wp, "rule_sport_from", "0");
	char *sport_to = websGetVar(wp, "rule_sport_to", "0");
	char *dport_from = websGetVar(wp, "rule_dport_from", "0");
	char *dport_to = websGetVar(wp, "rule_dport_to", "0");
	char *ipproto = websGetVar(wp, "rule_ipproto", "0");
	char sport[64];
	char dport[64];
	char fw[64];
	sprintf(sport, "%s-%s", sport_from, sport_to);
	sprintf(dport, "%s-%s", dport_from, dport_to);
	if (fwmask)
		sprintf(fw, "0x%X/0x%X", strtoul(fwmark, NULL, 0), strtoul(fwmask, NULL, 0)); // force hex
	else
		sprintf(fw, "0x%X/0x%X", strtoul(fwmark, NULL, 0)); // force hex
	/*
	 * validate ip address 
	 */
	char from[20];
	strcpy(from, "");
	for (i = 0; i < 5; i++) {
		snprintf(temp, sizeof(temp), "%s_%d", "rule_from", i);
		val = websGetVar(wp, temp, NULL);
		if (val) {
			strcat(from, val);
			if (i < 3)
				strcat(from, ".");
			if (i == 3)
				strcat(from, "/");
		} else {
			// free (from);
			debug_free(old_name);
			debug_free(old);
			debug_free(buf_name);
			debug_free(buf);
			return;
		}
	}

	char to[20];
	strcpy(to, "");
	for (i = 0; i < 5; i++) {
		snprintf(temp, sizeof(temp), "%s_%d", "rule_to", i);
		val = websGetVar(wp, temp, NULL);
		if (val) {
			strcat(to, val);
			if (i < 3)
				strcat(to, ".");
			if (i == 3)
				strcat(to, "/");
		} else {
			// free (to);
			debug_free(old_name);
			debug_free(old);
			debug_free(buf_name);
			debug_free(buf);
			return;
		}
	}

	char nat[20];
	strcpy(nat, "");
	for (i = 0; i < 4; i++) {
		snprintf(temp, sizeof(temp), "%s_%d", "rule_nat", i);
		val = websGetVar(wp, temp, NULL);
		if (val) {
			strcat(nat, val);
			if (i < 3)
				strcat(nat, ".");
		} else {
			// free (nat);
			debug_free(old_name);
			debug_free(old);
			debug_free(buf_name);
			debug_free(buf);
			return;
		}
	}
	// Allow Defaultroute here

write_nvram:
	if (!strcmp(iif, "lan")) {
		iif = nvram_safe_get("lan_ifname");
	}
	if (!strcmp(iif, "wan")) {
		iif = nvram_safe_get("wan_ifname");
	}
	if (!strcmp(oif, "lan")) {
		oif = nvram_safe_get("lan_ifname");
	}
	if (!strcmp(oif, "wan")) {
		oif = nvram_safe_get("wan_ifname");
	}

	for (i = 0; i < STATIC_ROUTE_PAGE; i++) {
		strcpy(&old[i * PBR_LINE_SIZE], "");
		strcpy(&old_name[i * ROUTE_NAME_SIZE], "");
	}
	i = 0;
	foreach(word, nvram_safe_get("pbr_rule"), next)
	{
		strcpy(&old[i * PBR_LINE_SIZE], word);
		i++;
	}
	i = 0;
	foreach(word, nvram_safe_get("pbr_rule_name"), next)
	{
		strcpy(&old_name[i * ROUTE_NAME_SIZE], word);
		i++;
	}

	strcpy(backuproute, &old[atoi(page) * PBR_LINE_SIZE]);

	snprintf(&old[atoi(page) * PBR_LINE_SIZE], PBR_LINE_SIZE, "%X:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s", flags, from,
		 to, priority, tos, fw, realms, table, suppress_prefixlength, iif, nat, type, ipproto, sport, dport, oif);

	httpd_filter_name(name, new_name, sizeof(new_name), SET);
	snprintf(&old_name[atoi(page) * ROUTE_NAME_SIZE], ROUTE_NAME_SIZE, "$NAME:%s$$", new_name);

	if (strcmp(backuproute, &old[atoi(page) * PBR_LINE_SIZE])) {
		if (*(backuproute)) {
			//nvram_set("nowebaction","1");
			//addAction("pbr_rule_del");
			addDeletion_pbr(backuproute);
		}
	}

	for (i = 0; i < STATIC_ROUTE_PAGE; i++) {
		if (strcmp(&old_name[i * ROUTE_NAME_SIZE], "")) {
			cur += snprintf(cur, buf + (STATIC_ROUTE_PAGE * PBR_LINE_SIZE) - cur, "%s%s", cur == buf ? "" : " ",
					&old[i * PBR_LINE_SIZE]);
			cur_name += snprintf(cur_name, buf_name + (STATIC_ROUTE_PAGE * ROUTE_NAME_SIZE) - cur_name, "%s%s",
					     cur_name == buf_name ? "" : " ", &old_name[i * ROUTE_NAME_SIZE]);
		}
	}

	if (!strcmp(websGetVar(wp, "action", ""), "ApplyTake"))
		delete_old_pbr();
	FILE *backup = fopen("/tmp/pbr_old", "rb");
	if (!backup) {
		backup = fopen("/tmp/pbr_old", "wb");
		char *bck = nvram_safe_get(v->name);
		int blen = strlen(bck);
		for (i = 0; i < blen; i++)
			putc(bck[i], backup);
	}
	fclose(backup);
	nvram_set(v->name, buf);
	nvram_set("pbr_rule_name", buf_name);
	nvram_async_commit();

	debug_free(old_name);
	debug_free(old);
	debug_free(buf_name);
	debug_free(buf);
}
#endif
