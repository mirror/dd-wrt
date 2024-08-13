/*
 * ejs.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>

#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <bcmnvram.h>
#include <revision.h>
#include <shutils.h>
#ifdef HAVE_SAMBA_SERVER
#include <jansson.h>
#endif
#include <glob.h>
int iscpe(void);

struct onload onloads[] = {
	// { "Filters", filter_onload },
	{ "WL_ActiveTable", wl_active_onload },
	{ "MACClone", macclone_onload },
	{ "FilterSummary", filtersummary_onload },
	{ "Ping", ping_onload },
	// {"Traceroute", traceroute_onload},
};

EJ_VISIBLE void ej_onload(webs_t wp, int argc, char_t **argv)
{
	char *type, *arg;
	struct onload *v;

	type = argv[0];
	arg = argv[1];

	for (v = onloads; v < &onloads[STRUCT_LEN(onloads)]; v++) {
		if (!strcmp(v->name, type)) {
			v->go(wp, arg);
			return;
		}
	}

	return;
}

/*
 * Meta tag command that will no allow page cached by browsers. The will
 * force the page to be refreshed when visited.
 */
EJ_VISIBLE void ej_no_cache(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "<meta http-equiv=\"expires\" content=\"0\">\n");
	websWrite(wp, "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n");
	websWrite(wp, "<meta http-equiv=\"pragma\" content=\"no-cache\">\n");

	return;
}

/*
 * Example:
 * lan_ipaddr=192.168.1.1
 * <% prefix_ip_get("lan_ipaddr",1); %> produces "192.168.1."
 */
EJ_VISIBLE void ej_prefix_ip_get(webs_t wp, int argc, char_t **argv)
{
	char *name;
	int type;

	name = argv[0];
	type = atoi(argv[1]);

	char *val = nvram_safe_get(name);

	if (type == 1)
		websWrite(wp, "%d.%d.%d.", get_single_ip(val, 0), get_single_ip(val, 1), get_single_ip(val, 2));
	if (type == 2)
		websWrite(wp, "%d.%d.", get_single_ip(val, 0), get_single_ip(val, 1));
	if (type == 3)
		websWrite(wp, "%d.", get_single_ip(val, 0));

	return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1
 * <% nvram_get("lan_ipaddr"); %> produces "192.168.1.1"
 */
EJ_VISIBLE void ej_nvram_get(webs_t wp, int argc, char_t **argv)
{
#if COUNTRY == JAPAN
	websWrite(wp, "%s", nvram_safe_get(argv[0]));
#else

	tf_webWriteESCNV(wp, argv[0]); // test: buffered version of above

	return;
#endif

	return;
}

EJALIAS(ej_nvram_get, ej_nvg);

extern struct nvram_param *srouter_defaults;

EJ_VISIBLE void ej_nvram_default_get(webs_t wp, int argc, char_t **argv)
{
	struct nvram_param *t;
	for (t = srouter_defaults; t->name; t++) {
		if (!strcmp(t->name, argv[0]))
			websWrite(wp, "%s", t->value);
	}
}

EJALIAS(ej_nvram_default_get, ej_nvdg);
EJ_VISIBLE void ej_nvram_real_get(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "%s", nvram_safe_get(argv[0]));

	return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1, gozila_action = 0
 * <% nvram_selget("lan_ipaddr"); %> produces "192.168.1.1"
 * lan_ipaddr = 192.168.1.1, gozila_action = 1, websGetVar(wp, "lan_proto", NULL) = 192.168.1.2;
 * <% nvram_selget("lan_ipaddr"); %> produces "192.168.1.2"
 */
EJ_VISIBLE void ej_nvram_selget(webs_t wp, int argc, char_t **argv)
{
	char *name;

	name = argv[0];
	if (wp->gozila_action) {
		char *buf = websGetVar(wp, name, NULL);

		if (buf) {
			websWrite(wp, "%s", buf);
			return;
		}
	}
	tf_webWriteESCNV(wp, name); // test: buffered version of above

	return;
}

/*
 * Example:
 * wan_mac = 00:11:22:33:44:55
 * <% nvram_mac_get("wan_mac"); %> produces "00-11-22-33-44-55"
 */
EJ_VISIBLE void ej_nvram_mac_get(webs_t wp, int argc, char_t **argv)
{
	char *c;
	char *mac;
	int i;

	c = nvram_safe_get(argv[0]);

	if (c) {
		mac = strdup(c);
		for (i = 0; *(mac + i); i++) {
			if (*(mac + i) == ':')
				*(mac + i) = '-';
		}
		websWrite(wp, "%s", mac);
		debug_free(mac); // leak, thx tofu
	}

	return;
}

/*
 * Example:
 * wan_proto = dhcp; gozilla = 0;
 * <% nvram_gozila_get("wan_proto"); %> produces "dhcp"
 *
 * wan_proto = dhcp; gozilla = 1; websGetVar(wp, "wan_proto", NULL) = static;
 * <% nvram_gozila_get("wan_proto"); %> produces "static"
 */
EJ_VISIBLE void ej_nvram_gozila_get(webs_t wp, int argc, char_t **argv)
{
	char *type;

	type = GOZILA_GET(wp, argv[0]);
	if (type == NULL)
		type = nvram_safe_get(argv[0]);

	websWrite(wp, "%s", type);
}

EJ_VISIBLE void ej_webs_get(webs_t wp, int argc, char_t **argv)
{
	char *value;

	value = websGetVar(wp, argv[0], NULL);

	if (value)
		websWrite(wp, "%s", value);

	return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1
 * <% get_single_ip("lan_ipaddr","1"); %> produces "168"
 */
EJ_VISIBLE void ej_get_single_ip(webs_t wp, int argc, char_t **argv)
{
	char *c;

	c = nvram_safe_get(argv[0]);
	if (c) {
		if (!strcmp(c, PPP_PSEUDO_IP) || !strcmp(c, PPP_PSEUDO_GW))
			c = "0.0.0.0";
		else if (!strcmp(c, PPP_PSEUDO_NM))
			c = "255.255.255.0";

		websWrite(wp, "%d", get_single_ip(c, atoi(argv[1])));
	} else
		websWrite(wp, "0");

	return;
}

EJ_VISIBLE void ej_get_cidr_mask(webs_t wp, int argc, char_t **argv)
{
	char *c;
	c = nvram_safe_get(argv[0]);
	if (*c) {
		websWrite(wp, "%d", getmask(c));
	} else
		websWrite(wp, "0");
}

EJ_VISIBLE void ej_get_single_nm(webs_t wp, int argc, char_t **argv)
{
	char *c;

	c = nvram_safe_get(argv[0]);
	if (*c) {
		websWrite(wp, "%d", get_single_ip(c, atoi(argv[1])));
	} else
		websWrite(wp, "0");

	return;
}

/*
 * Example: wan_mac = 00:11:22:33:44:55 get_single_mac("wan_mac", 1);
 * produces "11"
 */
int get_single_mac(char *macaddr, int which)
{
	int mac[6] = { 0, 0, 0, 0, 0, 0 };
	int ret;

	ret = sscanf(macaddr, "%2X:%2X:%2X:%2X:%2X:%2X", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
	return mac[which];
}

/*
 * Example:
 * wan_mac = 00:11:22:33:44:55
 * <% get_single_mac("wan_mac","1"); %> produces "11"
 */
EJ_VISIBLE void ej_get_single_mac(webs_t wp, int argc, char_t **argv)
{
	char *c;
	int mac;

	c = nvram_safe_get(argv[0]);
	if (c) {
		mac = get_single_mac(c, atoi(argv[1]));
		websWrite(wp, "%02X", mac);
	} else
		websWrite(wp, "00");

	return;
}

/*
 * Example:
 * wan_proto = dhcp; gozilla = 0;
 * <% nvram_selmatch("wan_proto", "dhcp", "selected"); %> produces "selected"
 *
 * wan_proto = dhcp; gozilla = 1; websGetVar(wp, "wan_proto", NULL) = static;
 * <% nvram_selmatch("wan_proto", "static", "selected"); %> produces "selected"
 */

int nvram_selmatch(webs_t wp, char *name, char *match)
{
	char *type = GOZILA_GET(wp, name);

	if (!type) {
		if (nvram_match(name, match)) {
			return 1;
		}
	} else {
		if (!strcmp(type, match)) {
			return 1;
		}
	}
	return 0;
}

EJ_VISIBLE void ej_nvram_selmatch(webs_t wp, int argc, char_t **argv)
{
	if (nvram_selmatch(wp, argv[0], argv[1])) {
		websWrite(wp, argv[2]);
	}
	return;
}

EJALIAS(ej_nvram_selmatch, ej_nvsm);

EJ_VISIBLE void ej_nvram_listselmatch(webs_t wp, int argc, char_t **argv)
{
	char value[64];
	char *next;
	char *list = argv[1];

	foreach(value, list, next)
	{
		if (nvram_selmatch(wp, argv[0], value)) {
			websWrite(wp, argv[2]);
			return;
		}
	}
	return;
}

EJALIAS(ej_nvram_listselmatch, ej_nvlsm);

EJ_VISIBLE void ej_nvram_listselmatch_not_v6(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_IPV6
	if (nvram_matchi("ipv6_enable", 1)) {
		char value[64];
		char *next;
		char *list = argv[1];

		foreach(value, list, next)
		{
			if (nvram_selmatch(wp, argv[0], value)) {
				websWrite(wp, argv[2]);
				return;
			}
		}
	} else
#endif
		websWrite(wp, argv[2]);
	return;
}

EJALIAS(ej_nvram_listselmatch_not_v6, ej_nvlsmn6);

EJ_VISIBLE void ej_nvram_else_selmatch(webs_t wp, int argc, char_t **argv)
{
	char *type;

	type = GOZILA_GET(wp, argv[0]);

	if (!type) {
		if (nvram_match(argv[0], argv[1])) {
			websWrite(wp, argv[2]);
			return;
		}
	} else {
		if (!strcmp(type, argv[1])) {
			websWrite(wp, argv[2]);
			return;
		}
	}
	websWrite(wp, argv[3]);

	return;
}

EJALIAS(ej_nvram_else_selmatch, ej_nvesm);

EJ_VISIBLE void ej_nvram_else_listselmatch(webs_t wp, int argc, char_t **argv)
{
	char value[64];
	char *next;
	char *list = argv[1];
	char *type;

	type = GOZILA_GET(wp, argv[0]);
	foreach(value, list, next)
	{
		if (!type) {
			if (nvram_match(argv[0], value)) {
				websWrite(wp, argv[2]);
				return;
			}
		} else {
			if (!strcmp(type, value)) {
				websWrite(wp, argv[2]);
				return;
			}
		}
	}
	websWrite(wp, argv[3]);
}

EJALIAS(ej_nvram_else_listselmatch, ej_nvelsm);

EJ_VISIBLE void ej_selchecked(webs_t wp, int argc, char_t **argv)
{
	char *type;

	type = websGetVar(wp, argv[0], "0");

	if (type) {
		if (!strcmp(type, argv[1]))
			websWrite(wp, "checked=\"checked\"");
	}

	return;
}

EJ_VISIBLE void ej_else_selmatch(webs_t wp, int argc, char_t **argv)
{
	char *type;

	type = websGetVar(wp, argv[0], NULL);

	if (type) {
		if (!strcmp(type, argv[1])) {
			websWrite(wp, argv[2]);
		} else
			websWrite(wp, argv[3]);
	} else {
		websWrite(wp, argv[3]);
	}

	return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_else_match("wan_proto", "dhcp", "0","1"); %> produces "0"
 * <% nvram_else_match("wan_proto", "static", "0","1"); %> produces "1"
 */
EJ_VISIBLE void ej_nvram_else_match(webs_t wp, int argc, char_t **argv)
{
	if (nvram_match(argv[0], argv[1]))
		websWrite(wp, argv[2]);
	else
		websWrite(wp, argv[3]);

	return;
}

EJALIAS(ej_nvram_else_match, ej_nvem);

EJ_VISIBLE void ej_startswith(webs_t wp, int argc, char_t **argv)
{
	if (startswith(nvram_safe_get(argv[0]), argv[1]))
		websWrite(wp, argv[2]);

	return;
}

typedef struct defrule {
	char *name;
	int (*rulefn)(char *name);
};

static int rule_ismini(char *name)
{
	if (startswith(nvram_safe_get("dist_type"), "mini")) {
		return 1;
	}
	return 0;
}

static int rule_isvpn(char *name)
{
	if (startswith(nvram_safe_get("dist_type"), "vpn")) {
		return 1;
	}
	return 0;
}

static int rule_iswet(char *name)
{
	return getWET() ? 1 : 0;
}

static int rule_issta(char *name)
{
	return getSTA() ? 1 : 0;
}

static int rule_haswifi(char *name)
{
	return haswifi() ? 1 : 0;
}

static int rule_wanvlan(char *name)
{
	char *wan_iface = nvram_safe_get("wan_ifname");
	if (!*wan_iface)
		wan_iface = nvram_safe_get("wan_ifname2");
	return isvlan(wan_iface);
}

static int rule_fa(char *name)
{
	char *fa = nvram_safe_get("ctf_fa_mode");
	return *fa ? ((nvram_match("wan_proto", "dhcp") || nvram_match("wan_proto", "static")) &&
		      nvram_match("wshaper_enable", "0")) :
		     0;
}

static int rule_ctf(char *name)
{
	char *ctf = nvram_safe_get("has_ctf");
	return *ctf ? 1 : 0;
}

static int rule_sfe(char *name)
{
	return !rule_ctf(name);
}

static int rule_nss(char *name)
{
#if defined(HAVE_QCA_NSS)
	return 1;
#else
	return 0;
#endif
}

static int rule_afterburner(char *name)
{
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	int afterburner = 0;
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_SMLEN];
	char *ifname;
	name = name + 11;
	if (!strncmp(name, "_wl0", 4))
		ifname = nvram_safe_get("wl0_ifname");
	else if (!strncmp(name, "_wl1", 4))
		ifname = nvram_safe_get("wl1_ifname");
	else // "_wl1"
		ifname = nvram_safe_get("wl2_ifname");
	char *next;

	if (wl_iovar_get(ifname, "cap", (void *)caps, WLC_IOCTL_SMLEN) == 0) {
		foreach(cap, caps, next)
		{
			if (!strcmp(cap, "afterburner"))
				return 1;
		}
	}
#endif
	return 0;
}

static int rule_poeswitch(char *name)
{
	return led_control(POE_GPIO, GPIO_CHECK);
}

#ifdef HAVE_USB
static int rule_usb_rc(char *name)
{
	return !nvram_match("rc_usb", "");
}
#endif

#include <linux/version.h>
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))

static struct defrule s_conditions[] = {
#ifdef HAVE_MICRO
	{ "MICRO", NULL },
#endif
#ifdef HAVE_EXTHELP
	{ "EXTHELP", NULL },
#endif
#ifdef HAVE_MULTICAST
	{ "MULTICAST", NULL },
#endif
#ifdef HAVE_SNMP
	{ "SNMP", NULL },
#endif
#ifdef HAVE_WIVIZ
	{ "WIVIZ", NULL },
#endif
#ifdef HAVE_RSTATS
	{ "RSTATS", NULL },
#endif
#ifdef HAVE_ACK
	{ "ACK", NULL },
#endif
#ifdef HAVE_SSHD
	{ "SSHD", NULL },
#endif
#ifdef HAVE_PPTPD
	{ "PPTPD", NULL },
#endif
#ifdef HAVE_QUAGGA
	{ "QUAGGA", NULL },
#endif
#ifdef HAVE_SAMBA
	{ "SAMBA", NULL },
#endif
#ifdef HAVE_SAMBA3
	{ "SAMBA3", NULL },
#endif
#if defined(HAVE_JFFS2) || defined(HAVE_X86) || defined(HAVE_VENTANA)
	{ "JFFS2", NULL },
#endif
#ifdef HAVE_GPSI
	{ "GPSI", NULL },
#endif
#ifdef HAVE_MMC
	{ "MMC", NULL },
#endif
#ifdef HAVE_SPUTNIK_APD
	{ "SPUTNIK_APD", NULL },
#endif
#ifdef HAVE_RFLOW
	{ "RFLOW", NULL },
#endif
#ifdef HAVE_USB
	{ "USB", NULL },
	{ "USB_rc", rule_usb_rc },
#endif
#ifdef HAVE_RADIUSPLUGIN
	{ "RADIUSPLUGIN", NULL },
#endif
#ifdef HAVE_PPPOESERVER
	{ "PPPOESERVER", NULL },
#endif
#ifdef HAVE_MILKFISH
	{ "MILKFISH", NULL },
#endif
#ifdef HAVE_LANGUAGE
	{ "LANGUAGE", NULL },
#endif
#ifdef HAVE_BUFFALO
	{ "HAVE_BUFFALO", NULL },
#endif
#ifdef HAVE_WPS
	{ "HAVE_WPS", NULL },
#endif
#ifdef HAVE_IPV6
	{ "HAVE_IPV6", NULL },
#endif
#ifdef HAVE_ATH9K
	{ "HAVE_ATH9K", NULL },
#endif
#ifdef HAVE_USBIP
	{ "USBIP", NULL },
#endif
#ifdef HAVE_USB_ADV
	{ "USBADV", NULL },
#endif
#ifdef HAVE_80211AC
	{ "80211AC", NULL },
#endif
#ifdef HAVE_DNSSEC
	{ "DNSSEC", NULL },
#endif
#ifdef HAVE_DNSCRYPT
	{ "DNSCRYPT", NULL },
#endif
#ifdef HAVE_SMARTDNS
	{ "SMARTDNS", NULL },
#endif
#if defined(HAVE_BKM) || defined(HAVE_TMK)
	{ "MULTISIM", NULL },
#endif
#ifdef HAVE_ATH9K
	{ "MAC80211", NULL },
#endif
#ifdef HAVE_HOTSPOT
	{ "HAVE_HOTSPOT", NULL },
#endif
#ifdef HAVE_ANTAIRA
	{ "HAVE_ANTAIRA", NULL },
#endif
#ifdef HAVE_SWCONFIG
	{ "HAVE_SWCONFIG", NULL },
#endif
#ifdef HAVE_ANTAIRA_MINI
	{ "HAVE_ANTAIRA_MINI", NULL },
#endif
#ifdef HAVE_HTTPS
	{ "HTTPS", NULL },
#endif
#ifdef HAVE_SPEEDTEST_CLI
	{ "SPEEDTEST_CLI", NULL },
#endif
#ifdef HAVE_REAL_OPENSSL
	{ "OPENSSL", NULL },
#endif
#ifdef HAVE_ANTAIRA_AGENT
	{ "ANTAIRA_AGENT", NULL },
#endif
	{ "MINI", rule_ismini },
	{ "VPN", rule_isvpn },
	{ "WET", rule_iswet },
	{ "WET", rule_issta },
	{ "WANVLAN", rule_wanvlan },
	{ "HASWIFI", rule_haswifi },
	{ "AFTERBURNER", rule_afterburner },
#ifndef HAVE_MICRO
	{ "HAVE_PBR", NULL },
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
	{ "HAVE_EXT_IPROUTE", NULL },
#endif
#endif
	{ "POESWITCH", rule_poeswitch },
#if !defined(ARCH_broadcom) || defined(HAVE_BCMMODERN)
	{ "FA", rule_fa },
	{ "CTF", rule_ctf },
	{ "SFE", rule_sfe },
	{ "NSS", rule_nss },
#endif
	{ NULL, NULL }
};

EJ_VISIBLE void ej_ifdef(webs_t wp, int argc, char_t **argv)
{
	char *name, *output;

	name = argv[0];
	output = argv[1];
	int cnt = 0;
	while (s_conditions[cnt].name) {
		if (!strcmp(name, s_conditions[cnt].name)) {
			if (s_conditions[cnt].rulefn && !s_conditions[cnt].rulefn(name)) {
				return;
			}
			websWrite(wp, output);
			return;
		}
		cnt++;
	}

	return;
}

EJ_VISIBLE void ej_ifndef(webs_t wp, int argc, char_t **argv)
{
	char *name, *output;

	name = argv[0];
	output = argv[1];

	int cnt = 0;
	while (s_conditions[cnt].name) {
		if (!strcmp(name, s_conditions[cnt].name)) {
			if (!s_conditions[cnt].rulefn || (s_conditions[cnt].rulefn && s_conditions[cnt].rulefn(name))) {
				return;
			}
			websWrite(wp, output);
			return;
		}
		cnt++;
	}
	websWrite(wp, output);
	return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
EJ_VISIBLE void ej_nvram_match(webs_t wp, int argc, char_t **argv)
{
	if (nvram_match(argv[0], argv[1]))
		websWrite(wp, argv[2]);

	return;
}

EJALIAS(ej_nvram_match, ej_nvm);

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_invmatch("wan_proto", "dhcp", "disabled"); %> does not produce
 * <% nvram_invmatch("wan_proto", "static", "disabled"); %> produces "disabled"
 */
EJ_VISIBLE void ej_nvram_invmatch(webs_t wp, int argc, char_t **argv)
{
	if (!nvram_match(argv[0], argv[1]))
		websWrite(wp, argv[2]);
	return;
}

EJALIAS(ej_nvram_invmatch, ej_nvim);

/*
 * Example:
 * filter_mac=00:12:34:56:78:00 00:87:65:43:21:00
 * <% nvram_list("filter_mac", 1); %> produces "00:87:65:43:21:00"
 * <% nvram_list("filter_mac", 100); %> produces ""
 */
EJ_VISIBLE void ej_nvram_list(webs_t wp, int argc, char_t **argv)
{
	int which;
	char word[256], *next;
	which = atoi(argv[1]);
	char *list = nvram_safe_get(argv[0]);

	foreach(word, list, next)
	{
		if (which-- == 0)
			websWrite(wp, word);
	}

	return;
}

/*
 * Example: wan_dns = 1.2.3.4 10.20.30.40 15.25.35.45 get_dns_ip("wan_dns",
 * 1, 2); produces "20"
 */
int get_dns_ip(char *name, int which, int count)
{
	char word[256];
	char *next;
	int ip;
	char *list = nvram_safe_get(name);

	foreach(word, list, next)
	{
		if (which-- == 0) {
			ip = get_single_ip(word, count);
			return ip;
		}
	}
	return 0;
}

/*
 * Example: wan_dns = 168.95.1.1 210.66.161.125 168.95.192.1 <%
 * get_dns_ip("wan_dns", "1", "2"); %> produces "161" <%
 * get_dns_ip("wan_dns", "2", "3"); %> produces "1"
 */
EJ_VISIBLE void ej_get_dns_ip(webs_t wp, int argc, char_t **argv)
{
	int which;
	char word[256], *next;

	which = atoi(argv[1]);
	char *list = nvram_safe_get(argv[0]);

	foreach(word, list, next)
	{
		if (which-- == 0) {
			websWrite(wp, "%d", get_single_ip(word, atoi(argv[2])));
			return;
		}
	}

	websWrite(wp, "0"); // not find
}

EJ_VISIBLE void ej_get_http_prefix(webs_t wp, int argc, char_t **argv)
{
	char http[10];
	char ipaddr[20];
	char port[10];

	int http_enable = websGetVari(wp, "http_enable", 0);

#ifdef HAVE_HTTPS
	int https_enable = websGetVari(wp, "https_enable", 0);

	if (DO_SSL(wp) && !http_enable && !https_enable) {
		strcpy(http, "https");
	} else if (DO_SSL(wp) && https_enable) {
		strcpy(http, "https");
	} else if (!DO_SSL(wp) && !http_enable && https_enable) {
		strcpy(http, "https");
	} else
#endif
		strcpy(http, "http");

	if (wp->browser_method == USE_LAN) { // Use LAN to browser
		if (nvram_matchi("restore_defaults", 1) || nvram_matchi("sv_restore_defaults", 1)) {
			strcpy(http, "http");
		} else
			strcpy(ipaddr, nvram_safe_get("lan_ipaddr"));
		strcpy(port, "");
	} else {
		if (nvram_match("wan_proto", "pptp"))
			strcpy(ipaddr, nvram_safe_get("pptp_get_ip"));
		else if (nvram_match("wan_proto", "l2tp"))
			strcpy(ipaddr, nvram_safe_get("l2tp_get_ip"));
		else
			strcpy(ipaddr, nvram_safe_get("wan_ipaddr"));

		sprintf(port, ":%s", nvram_safe_get("http_wanport"));
	}

	websWrite(wp, "%s://%s%s/", http, ipaddr, port);

	return;
}

EJ_VISIBLE void ej_get_mtu(webs_t wp, int argc, char_t **argv)
{
	struct mtu_lists *mtu_list;
	char *type;
	char *proto = GOZILA_GET(wp, "wan_proto");

	type = argv[1];
	if (proto == NULL)
		proto = nvram_safe_get("wan_proto");
	mtu_list = get_mtu(proto);

	if (!strcmp(type, "min"))
		websWrite(wp, "%s", mtu_list->min);
	else if (!strcmp(type, "max"))
		websWrite(wp, "%s", mtu_list->max);

	return;
}

EJ_VISIBLE void ej_show_forward(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("forward_entries");
	if (count == NULL || *(count) == 0 || (c = atoi(count)) <= 0) {
		// return -1; botho 07/03/06 add "- None -" if empty
		websWrite(
			wp,
			"<tr>\n<td colspan=\"7\" class=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n</tr>\n");
	}
	for (i = 0; i < c; i++) {
		websWrite(
			wp,
			"<tr><td>\n<input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		port_forward_table(wp, "name", i);
		websWrite(
			wp,
			"\" /></td>\n<td>\n<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
			i);
		port_forward_table(wp, "from", i);
		websWrite(
			wp,
			"\" /></td>\n<td>\n<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
			i);
		port_forward_table(wp, "to", i);
		websWrite(wp, "\"/></td>\n<td><select size=\"1\" name=\"pro%d\">\n<option value=\"tcp\" ", i);
		port_forward_table(wp, "sel_tcp", i);
		websWrite(wp, ">TCP</option>\n<option value=\"udp\" ");
		port_forward_table(wp, "sel_udp", i);
		websWrite(
			wp,
			">UDP</option>\n<script type=\"text/javascript\">\n//<![CDATA[\ndocument.write(\"<option value=\\\"both\\\" ");
		port_forward_table(wp, "sel_both", i);
		websWrite(
			wp,
			" >\" + share.both + \"</option>\");\n\n//]]>\n</script>\n</select></td>\n<td>\n<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"ip%d\" value=\"",
			i);
		port_forward_table(wp, "ip", i);
		websWrite(wp, "\" /></td>\n<td class=\"center\">\n<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
		port_forward_table(wp, "enable", i);
		websWrite(wp, " /></td>\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"forward_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n</td></tr>\n",
			i);
	}
	return;
}

EJ_VISIBLE void ej_show_forward_spec(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("forwardspec_entries");
	if (count == NULL || *(count) == 0 || (c = atoi(count)) <= 0) {
		// return -1; botho 07/03/06 add "- None -" if empty
		// websWrite (wp, "<tr></tr><tr></tr>\n");
		websWrite(
			wp,
			"<tr>\n<td colspan=\"8\" class=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n</tr>\n");
	}
	for (i = 0; i < c; i++) {
		//name
		websWrite(
			wp,
			"<tr><td>\n<input maxlength=\"12\" size=\"10\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		port_forward_spec(wp, "name", i);
		websWrite(wp, "\" /></td>\n<td><select size=\"1\" name=\"pro%d\">\n<option value=\"tcp\" ", i);
		port_forward_spec(wp, "sel_tcp", i);
		websWrite(wp, ">TCP</option>\n<option value=\"udp\" ");
		port_forward_spec(wp, "sel_udp", i);
		websWrite(
			wp,
			">UDP</option>\n<script type=\"text/javascript\">\n//<![CDATA[\ndocument.write(\"<option value=\\\"both\\\" ");
		port_forward_spec(wp, "sel_both", i);
		websWrite(
			wp,
			" >\" + share.both + \"</option>\");\n"
			"\n//]]>\n</script>\n</select></td>\n<td>\n<input class=\"num\" maxlength=\"18\" size=\"18\" name=\"src%d\" value=\"",
			i);
		port_forward_spec(wp, "src", i);
		websWrite(
			wp,
			"\" /></td>\n<td>\n<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
			i);
		port_forward_spec(wp, "from", i);
		websWrite(wp, "\" /></td>\n<td>\n<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"ip%d\" value=\"", i);
		port_forward_spec(wp, "ip", i);
		websWrite(
			wp,
			"\" /></td>\n<td>\n<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
			i);
		port_forward_spec(wp, "to", i);
		websWrite(wp, "\" /></td>\n<td class=\"center\">\n<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
		port_forward_spec(wp, "enable", i);
		websWrite(wp, " /></td>\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"forward_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n</td></tr>\n",
			i);
	}
	return;
}
void ip_forward(webs_t wp, char *type, int which);

EJ_VISIBLE void ej_show_forward_ip(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("forwardip_entries");
	if (count == NULL || *(count) == 0 || (c = atoi(count)) <= 0) {
		websWrite(
			wp,
			"<tr>\n<td colspan=\"5\" class=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n</tr>\n");
	}
	for (i = 0; i < c; i++) {
		websWrite(
			wp,
			"<tr><td>\n<input maxlength=\"12\" size=\"10\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		ip_forward(wp, "name", i);
		websWrite(wp, "\" /></td>\n<td>\n<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"src%d\" value=\"", i);
		ip_forward(wp, "src", i);
		websWrite(wp, "\" /></td>\n<td>\n<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"dest%d\" value=\"", i);
		ip_forward(wp, "dest", i);
		websWrite(wp, "\" /></td>\n<td class=\"center\">\n<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
		ip_forward(wp, "enable", i);
		websWrite(wp, " /></td>\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"forward_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n</td></tr>\n",
			i);
	}
}

EJ_VISIBLE void ej_show_triggering(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("trigger_entries");
	if (count == NULL || *(count) == 0 || (c = atoi(count)) <= 0) {
		websWrite(
			wp,
			"<tr>\n<td colspan=\"8\" class=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n</tr>\n");
	}
	for (i = 0; i < c; i++) {
		websWrite(
			wp,
			"<tr>\n<td><input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		port_trigger_table(wp, "name", i);
		websWrite(
			wp,
			"\" /></td>\n<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"i_from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
			i);
		port_trigger_table(wp, "i_from", i);
		websWrite(
			wp,
			"\" /></td>\n<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"i_to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
			i);
		port_trigger_table(wp, "i_to", i);
		websWrite(wp, "\" /></td>\n<td><select size=\"1\" name=\"pro%d\">\n<option value=\"tcp\" ", i);
		port_trigger_table(wp, "sel_tcp", i);
		websWrite(wp, ">TCP</option>\n<option value=\"udp\" ");
		port_trigger_table(wp, "sel_udp", i);
		websWrite(
			wp,
			">UDP</option>\n<script type=\"text/javascript\">\n//<![CDATA[\ndocument.write(\"<option value=\\\"both\\\" ");
		port_trigger_table(wp, "sel_both", i);
		websWrite(
			wp,
			" >\" + share.both + \"</option>\");\n"
			"\n//]]>\n</script>\n</select></td>\n<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"o_from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
			i);
		port_trigger_table(wp, "o_from", i);
		websWrite(
			wp,
			"\" /></td>\n<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"o_to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
			i);
		port_trigger_table(wp, "o_to", i);
		websWrite(wp, "\" /></td>\n<td class=\"center\"><input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
		port_trigger_table(wp, "enable", i);
		websWrite(wp, " /></td>\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"trigger_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n</td></tr>\n",
			i);
	}
	return;
}

#include "styles.c"

#include "../webs.h"

extern const websRomPageIndexType websRomPageIndex[];

#ifdef HAVE_LANGUAGE
EJ_VISIBLE void ej_show_languages(webs_t wp, int argc, char_t **argv)
{
	char buf[256];

	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	int i = 0;

	while (websRomPageIndex[i].path != NULL) {
		cprintf("checking %s\n", websRomPageIndex[i].path);
		if (!strncmp(websRomPageIndex[i].path, "lang_pack/", (sizeof("lang_pack/") - 1))) {
			cprintf("found language\n");
			if (strlen(websRomPageIndex[i].path) < 14)
				continue;
			strcpy(buf, websRomPageIndex[i].path);
			char *mybuf = &buf[sizeof("lang_pack/") - 1];

			if (strchr(mybuf, (int)'-') == NULL) {
				mybuf[strlen(mybuf) - 3] = 0; // strip .js
				websWrite(
					wp,
					"document.write(\"<option value=\\\"%s\\\" %s >\" + management.lang_%s + \"</option>\");\n",
					mybuf, nvram_match("language", mybuf) ? "selected=\\\"selected\\\"" : "", mybuf);
			}
		}
		i++;
	}
	websWrite(wp, "//]]>\n</script>\n");
	return;
}
#endif

static char *directories[] = {
	"/jffs/etc/config",
	"/mmc/etc/config",
	"/etc/config",
};

static int checkandadd(char *name, char **lst)
{
	char *list = *lst;
	if (!name) {
		if (list)
			debug_free(list);
		list = NULL;
		return 0;
	}
	if (!list) {
		list = strdup(name);
	} else {
		char cap[128];
		char *next;
		foreach(cap, list, next)
		{
			if (!strcmp(cap, name)) {
				return 1;
			}
		}
		list = realloc(list, strlen(list) + 1 + strlen(name) + 1);
		strcat(list, " ");
		strcat(list, name);
	}
	return 0;
}

EJ_VISIBLE void ej_show_modules(webs_t wp, int argc, char_t **argv)
{
	char buf[256];
	struct dirent *entry;
	DIR *directory;
	int resultcount = 0;
	char *result[256];
	result[0] = NULL;
	char *list = NULL;

	// display modules
	int idx;

	for (idx = 0; idx < 3; idx++) {
		directory = opendir(directories[idx]);
		if (directory == NULL)
			continue;
		// list all files in this directory
		while ((entry = readdir(directory)) != NULL) {
			if (checkandadd(entry->d_name, &list))
				continue;
			if (argc > 0) {
				if (argc == 3) {
					if (!nvram_match(argv[1], argv[2])) {
						closedir(directory);
						return;
					}
				}

				if (endswith(entry->d_name, argv[0])) {
#if defined(HAVE_ERC)
					if (strcmp(entry->d_name,
						   "base.webconfig") &&
					    !wp->userid) //show only base.webconfig for this user and nothing else
					{
						continue;
					}
#endif
					sprintf(buf, "%s/%s", directories[idx], entry->d_name);
					result[resultcount] = strdup(entry->d_name);
					resultcount++;
					result[resultcount] = NULL;
				}
			} else {
				if (endswith(entry->d_name, ".webconfig")) {
					sprintf(buf, "%s/%s", directories[idx], entry->d_name);
					result[resultcount] = strdup(entry->d_name);
					resultcount++;
					result[resultcount] = NULL;
				}
			}
		}
		/* now sort entries to solve EXT2 unsorted problem */
		int i, a;
		for (a = 0; a < resultcount; a++) {
			int change = 0;
			for (i = 0; i < resultcount - 1; i++) {
				int step = 0;
again:;
				if (!result[i][step] || !result[i + 1][step])
					continue;
				if (result[i][step] == result[i + 1][step]) {
					step++;
					goto again;
				}
				if (result[i][step] > result[i + 1][step]) {
					char *temp = result[i + 1];
					result[i + 1] = result[i];
					result[i] = temp;
					step = 0;
					change++;
				}
			}
			if (!change)
				break; //no more sortable entries found, so just break up here
		}
		for (i = 0; i < resultcount; i++) {
			sprintf(buf, "%s/%s", directories[idx], result[i]);
			do_ej(METHOD_GET, NULL, buf, wp);
		}
		for (i = 0; i < resultcount; i++) {
			debug_free(result[i]);
		}
		resultcount = 0;
		result[0] = NULL;
		closedir(directory);
	}
	checkandadd(NULL, &list);
	return;
}

#define getRouterName() nvram_exists(NVROUTER_ALT) ? nvram_safe_get(NVROUTER_ALT) : nvram_safe_get(NVROUTER)
EJ_VISIBLE void ej_get_sysmodel(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_XIOCOM
	websWrite(wp, "XWR");
#elif HAVE_ONNET
#ifdef HAVE_ONNET_BLANK
	if (nvram_match(NVROUTER, "Atheros Hornet")) {
		websWrite(wp, "9331");
	} else if (nvram_match(NVROUTER, "Compex WPE72")) {
		websWrite(wp, "E72");
	} else if (nvram_match(NVROUTER, "ACCTON AC622")) {
		if (iscpe()) {
			websWrite(wp, "7240-2");
		} else {
			websWrite(wp, "7240-2");
		}
	} else if (nvram_match(NVROUTER, "ACCTON AC722")) {
		if (iscpe()) {
			websWrite(wp, "OTAi 7240-5");
		} else {
			websWrite(wp, "OTAi 7240-5");
		}
	} else if (nvram_match(NVROUTER, "Compex WP546")) {
		websWrite(wp, "546");
	} else if (nvram_match(NVROUTER, "Yuncore XD9531")) {
		websWrite(wp, "AP-9531");
	} else if (nvram_match(NVROUTER, "Yuncore XD3200")) {
		websWrite(wp, "AP-9563AC");
	} else if (nvram_match(NVROUTER, "Yuncore SR3200")) {
		websWrite(wp, "AP-1200AC");
	} else if (nvram_match(NVROUTER, "Yuncore CPE880")) {
		websWrite(wp, "AP-9344");
	} else if (nvram_match(NVROUTER, "Yuncore CPE890")) {
		websWrite(wp, "AP-5900AC");
	} else if (nvram_match(NVROUTER, "Alfa AP120C")) {
		websWrite(wp, "AP-600dbdc");
	} else {
		websWrite(wp, "%s", nvram_safe_get(NVROUTER));
	}
#else
	if (nvram_match(NVROUTER, "Atheros Hornet")) {
		websWrite(wp, "OTAi 9331");
	} else if (nvram_match(NVROUTER, "Compex WPE72")) {
		websWrite(wp, "OTAi 724");
	} else if (nvram_match(NVROUTER, "ACCTON AC622")) {
		if (iscpe()) {
			websWrite(wp, "OTAi 724S");
		} else {
			websWrite(wp, "OTAi 724AP");
		}
	} else if (nvram_match(NVROUTER, "ACCTON AC722")) {
		if (iscpe()) {
			websWrite(wp, "OTAi 724S");
		} else {
			websWrite(wp, "OTAi 724AP");
		}
	} else if (nvram_match(NVROUTER, "Compex WP546")) {
		websWrite(wp, "OTAi 724S");
	} else if (nvram_match(NVROUTER, "Yuncore XD9531")) {
		websWrite(wp, "OtAi 9531");
	} else if (nvram_match(NVROUTER, "Yuncore XD3200")) {
		websWrite(wp, "OTAi 9563-AC");
	} else if (nvram_match(NVROUTER, "Yuncore SR3200")) {
		websWrite(wp, "OTAi 1200AC");
	} else if (nvram_match(NVROUTER, "Yuncore CPE880")) {
		websWrite(wp, "OTAi-9344");
	} else if (nvram_match(NVROUTER, "Yuncore CPE890")) {
		websWrite(wp, "OTAi 5900AC");
	} else if (nvram_match(NVROUTER, "Alfa AP120C")) {
		websWrite(wp, "OTAi 600dbdc");
	} else {
		websWrite(wp, "OTAi %s", getRouterName());
	}
#endif
#elif HAVE_SANSFIL
	websWrite(wp, "%s", "SANSFIL");
#elif HAVE_KORENRON
	websWrite(wp, "KORENRON %s", getRouterName());
#elif HAVE_TESTEM
	websWrite(wp, "TESTEM %s", getRouterName());
#elif HAVE_HOBBIT
	websWrite(wp, "HQ-NDS %s", getRouterName());
#elif HAVE_RAYTRONIK
	websWrite(wp, "RN-150M %s", getRouterName());
#else
	websWrite(wp, "%s", getRouterName());
#endif
}

#undef getRouterName
EJ_VISIBLE void ej_get_syskernel(webs_t wp, int argc, char_t **argv)
{
	struct utsname name;
	uname(&name);
	websWrite(wp, "%s %s %s %s", name.sysname, name.release, name.version, name.machine);
}

EJ_VISIBLE void ej_get_totaltraff(webs_t wp, int argc, char_t **argv)
{
	char wan_if_buffer[33];
	char *type;
	char wanface[32];
	char line[256];
	unsigned long long rcvd = 0, sent = 0, megcounti = 0, megcounto = 0;
	FILE *in;
	int ifl;

	type = argv[0];

	if (!nvram_matchi("ttraff_enable", 1))
		return;

	if (nvram_match("ttraff_iface", "") || !nvram_exists("ttraff_iface"))
		strlcpy(wanface, safe_get_wan_face(wan_if_buffer), sizeof(wanface) - 1);
	else
		strlcpy(wanface, nvram_safe_get("ttraff_iface"), sizeof(wanface) - 1);
	strcat(wanface, ":");

	in = fopen("/proc/net/dev", "rb");
	if (in == NULL)
		return;

	/* eat first two lines */
	fgets(line, sizeof(line), in);
	fgets(line, sizeof(line), in);
	while (fgets(line, sizeof(line), in) != NULL) {
		ifl = 0;

		if (strstr(line, wanface)) {
			while (line[ifl] != ':')
				ifl++;
			line[ifl] = 0;

			sscanf(line + ifl + 1,
			       "%llu %*llu %*llu %*llu %*llu %*llu %*llu %*llu %llu %*llu %*llu %*llu %*llu %*llu %*llu %*llu",
			       &rcvd, &sent);
		}
	}

	fclose(in);

	rcvd >>= 20; // output in MiB
	sent >>= 20;

	if ((in = fopen("/tmp/.megc", "r")) != NULL) {
		fgets(line, sizeof(line), in);
		sscanf(line, "%llu:%llu", &megcounti, &megcounto);
		rcvd += megcounti;
		sent += megcounto;
		fclose(in);
	}

	if (!strcmp(type, "in")) {
		websWrite(wp, "%llu", rcvd); // output in MiB
	} else if (!strcmp(type, "out")) {
		websWrite(wp, "%llu", sent);
	}
	return;
}

void show_bwif(webs_t wp, char *ifname, char *name)
{
	char buf[128];
	websWrite(wp, "<h2>%s - %s</h2>\n", tran_string(buf, sizeof(buf), "status_band.h2"), name);
	websWrite(wp, "<fieldset>\n");
	websWrite(
		wp,
		"<iframe title=\"\" src=\"/graph_if.svg?%s\" width=\"100%%\" height=\"275\" frameborder=\"0\" type=\"image/svg+xml\">\n",
		ifname);
	websWrite(wp, "</iframe>\n"
		      "</fieldset>\n"
		      "<br />\n");
}

EJ_VISIBLE void ej_show_bandwidth(webs_t wp, int argc, char_t **argv)
{
	char wan_if_buffer[33];

	char name[180];
	char *next, *bnext;
	char var[80];
	char eths[256];
	char eths2[256];
	char bword[256];
	glob_t globbuf;
	char *globstring;
	int globresult;
	int c;
	show_bwif(wp, nvram_safe_get("lan_ifname"), "LAN");
	bzero(eths, sizeof(eths));
	getIfLists(eths, sizeof(eths));
	//add ppp interfacs
	bzero(eths2, sizeof(eths2));
	getIfList(eths2, "ppp");
	strcat(eths, " ");
	strcat(eths, eths2);
	//add tun interfaces
	bzero(eths2, sizeof(eths2));
	getIfList(eths2, "tun");
	strcat(eths, " ");
	strcat(eths, eths2);
#ifndef HAVE_MADWIFI
	int cnt = get_wl_instances();
#endif
	foreach(var, eths, next)
	{
		if (!nvram_match("wan_proto", "disabled")) {
			if (!strcmp(safe_get_wan_face(wan_if_buffer), var))
				continue;
			if (!strcmp(nvram_safe_get("wan_ifname2"), var))
				continue;
		}
		if (!strcmp("etherip0", var))
			continue;
		if (!strncmp("wl", var, 2))
			continue;
		if (!strcmp(nvram_safe_get("lan_ifname"), var))
			continue;
#ifndef HAVE_MADWIFI
		for (c = 0; c < cnt; c++) {
			if (!strcmp(get_wl_instance_name(c), var))
				goto skip;
		}
#endif
		if (isbridge(var)) {
			snprintf(name, sizeof(name), "BRIDGE (%s)", getNetworkLabel(wp, var));
		} else
			snprintf(name, sizeof(name), "LAN (%s)", getNetworkLabel(wp, var));

		show_bwif(wp, var, name);
skip:;
	}
	char buf[128];
	if (!nvram_match("wan_proto", "disabled")) {
		if (getSTA()) {
			snprintf(name, sizeof(name), "%s WAN (%s)", tran_string(buf, sizeof(buf), "share.wireless"),
				 getNetworkLabel(wp, safe_get_wan_face(wan_if_buffer)));
		} else
			snprintf(name, sizeof(name), "WAN (%s)", getNetworkLabel(wp, safe_get_wan_face(wan_if_buffer)));

		show_bwif(wp, safe_get_wan_face(wan_if_buffer), name);

		if (nvram_matchi("dtag_vlan8", 1) && nvram_matchi("dtag_bng", 0)) {
			if (getRouterBrand() == ROUTER_WRT600N || getRouterBrand() == ROUTER_WRT610N)
				show_bwif(wp, "eth2.0008", "IPTV");
			else
				show_bwif(wp, "eth0.0008", "IPTV");
		}
	}
#ifdef HAVE_MADWIFI
	c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char dev[32];
		sprintf(dev, "wlan%d", i);
		if (nvram_nmatch("disabled", "%s_net_mode", dev))
			continue;
		snprintf(name, sizeof(name), "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"), getNetworkLabel(wp, dev));
		show_bwif(wp, dev, name);
		char *vifs = nvram_nget("%s_vifs", dev);

		if (vifs == NULL)
			continue;
		foreach(var, vifs, next)
		{
			if (nvram_nmatch("disabled", "%s_mode", var))
				continue;
			snprintf(name, sizeof(name), "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"),
				 getNetworkLabel(wp, var));
			show_bwif(wp, var, name);
		}
		int s;

		for (s = 1; s <= 10; s++) {
			char *wdsdev;

			wdsdev = nvram_nget("%s_wds%d_if", dev, s);
			if (*(wdsdev) == 0)
				continue;
			if (nvram_nmatch("0", "%s_wds%d_enable", dev, s))
				continue;
			snprintf(name, sizeof(name), "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"),
				 getNetworkLabel(wp, wdsdev));
			show_bwif(wp, wdsdev, name);
		}

		if (is_mac80211(dev)) {
			asprintf(&globstring, "/sys/class/ieee80211/phy*/device/net/%s.sta*", dev);
			globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
			int awdscount;
			for (awdscount = 0; awdscount < globbuf.gl_pathc; awdscount++) {
				char *ifname;
				ifname = strrchr(globbuf.gl_pathv[awdscount], '/');
				if (!ifname) {
					debug_free(globstring);
					continue;
				}
				sprintf(name, "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"), ifname + 1);
				show_bwif(wp, ifname + 1, name);
			}
			globfree(&globbuf);
			debug_free(globstring);
		}
	}

#else
	for (c = 0; c < cnt; c++) {
		if (nvram_nmatch("disabled", "wl%d_net_mode", c))
			continue;
		snprintf(name, sizeof(name), "%s (wl%d)", tran_string(buf, sizeof(buf), "share.wireless"), c);
		show_bwif(wp, get_wl_instance_name(c), name);
		char *vifs = nvram_nget("wl%d_vifs", c);
		if (vifs == NULL)
			continue;
		foreach(var, vifs, next)
		{
			if (nvram_nmatch("disabled", "%s_mode", var))
				continue;
			snprintf(name, sizeof(name), "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"),
				 getNetworkLabel(wp, var));
			show_bwif(wp, var, name);
		}
	}
#endif
#ifdef HAVE_WAVESAT

	sprintf(name, "%s", tran_string(buf, sizeof(buf), "wl_wimax.titl"));
	show_bwif(wp, "ofdm", name);
#endif
}

#include "menu.c"
#include "pagehead.c"

EJ_VISIBLE void ej_show_timeoptions(webs_t wp, int argc, char_t **argv) // Eko
{
	int i;

	for (i = 0; (allTimezones[i].tz_name != NULL); i++) {
		websWrite(wp, "<option value=\"%s\" %s>%s</option>\n", allTimezones[i].tz_name,
			  nvram_match("time_zone", allTimezones[i].tz_name) ? "selected=\"selected\"" : "",
			  allTimezones[i].tz_name);
	}
}

#ifdef HAVE_IPV6
EJ_VISIBLE void ej_show_ipv6options(webs_t wp, int argc, char_t **argv)
{
	websWrite(
		wp,
		"<option value=\"ipv6native\" %s><script type=\"text/javascript\">Capture(management.ipv6_native)</script></option>\n",
		nvram_match("ipv6_typ", "ipv6native") ? "selected=\"selected\"" : "");
	websWrite(
		wp,
		"<option value=\"ipv6pd\" %s><script type=\"text/javascript\">Capture(management.ipv6_px_del)</script></option>\n",
		nvram_match("ipv6_typ", "ipv6pd") ? "selected=\"selected\"" : "");
	websWrite(
		wp,
		"<option value=\"ipv6in4\" %s><script type=\"text/javascript\">Capture(management.ipv6_6in4st)</script></option>\n",
		nvram_match("ipv6_typ", "ipv6in4") ? "selected=\"selected\"" : "");
	//websWrite(wp, "<option value=\"ipv6to4\" %s>6to4 Anycast Releay</option>\n", nvram_match("ipv6_typ", "ipv6to4") ? "selected=\"selected\"" : "");
}
#endif
EJ_VISIBLE void ej_show_wanipinfo(webs_t wp, int argc, char_t **argv) // Eko
{
	char wan_if_buffer[33];

#ifdef HAVE_IPV6
	char buf[INET6_ADDRSTRLEN];
#endif
	char *wan_ipaddr;
	int wan_link;
	static char *disabled = NULL;
	if (!disabled)
		disabled = strdup(live_translate(wp, "share.disabled"));
	if (getWET() || nvram_match("wan_proto", "disabled") || nvram_match("wan_proto", "bridge")) {
		websWrite(wp, ": %s", disabled);
		return;
	}

	wan_link = check_wan_link(0);
	char *wan_proto = nvram_safe_get("wan_proto");
	if (!strcmp(wan_proto, "pptp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("pptp_get_ip") : nvram_safe_get("wan_ipaddr");
	} else if (!strcmp(wan_proto, "pppoe")
#ifdef HAVE_PPPOATM
		   || !strcmp(wan_proto, "pppoa")
#endif
#ifdef HAVE_PPPOEDUAL
		   || !strcmp(wan_proto, "pppoe_dual")
#endif
#ifdef HAVE_3G
		   || !strcmp(wan_proto, "3g")
#endif
#ifdef HAVE_IPETH
		   || !strcmp(wan_proto, "iphone") || !strcmp(wan_proto, "android")
#endif
	) {
		wan_ipaddr = wan_link ? nvram_safe_get("wan_ipaddr") : "0.0.0.0";
#ifdef HAVE_L2TP
	} else if (nvram_match("wan_proto", "l2tp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("l2tp_get_ip") : nvram_safe_get("wan_ipaddr");
#endif
	} else {
		wan_ipaddr = nvram_safe_get("wan_ipaddr");
	}

#ifdef HAVE_IPV6
	if (nvram_match("ipv6_typ", "ipv6in4") || nvram_match("ipv6_typ", "ipv6pd") || nvram_match("ipv6_typ", "ipv6native"))
		websWrite(wp, "&nbsp;IPv4: %s", wan_ipaddr);
	else
#endif
		websWrite(wp, "&nbsp;IP: %s", wan_ipaddr);
#ifdef HAVE_IPV6
	const char *ipv6addr = NULL;
	if (nvram_match("ipv6_typ", "ipv6native"))
		ipv6addr = getifaddr(buf, safe_get_wan_face(wan_if_buffer), AF_INET6, 0);
	if (nvram_match("ipv6_typ", "ipv6in4"))
		ipv6addr = getifaddr(buf, "ip6tun", AF_INET6, 0);
	if (nvram_match("ipv6_typ", "ipv6pd"))
		ipv6addr = getifaddr(buf, nvram_safe_get("lan_ifname"), AF_INET6, 0);
	if (nvram_match("ipv6_typ", "ipv6in4") || nvram_match("ipv6_typ", "ipv6pd") || nvram_match("ipv6_typ", "ipv6native")) {
		if (!ipv6addr)
			ipv6addr = getifaddr(buf, safe_get_wan_face(wan_if_buffer), AF_INET6,
					     0); // try wan if all other fails
		if (ipv6addr)
			websWrite(wp, "&nbsp;IPv6: %s", ipv6addr);
	}
#endif

	return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_selected("wan_proto", "dhcp",); %> produces: selected="selected"
 * <% nvram_selected_js("wan_proto", "dhcp"); %> produces: selected=\"selected\"
 * <% nvram_selected("wan_proto", "static"); %> does not produce
 */
EJ_VISIBLE void ej_nvram_selected(webs_t wp, int argc, char_t **argv)
{
	if (nvram_match(argv[0], argv[1])) {
		websWrite(wp, "selected=\"selected\"");
	}
	return;
}

EJALIAS(ej_nvram_selected, ej_nvs);

EJ_VISIBLE void ej_nvram_selected_js(webs_t wp, int argc, char_t **argv)
{
	if (nvram_match(argv[0], argv[1])) {
		websWrite(wp, "selected=\\\"selected\\\"");
	}
	return;
}

EJALIAS(ej_nvram_selected_js, ej_nvsjs);

EJ_VISIBLE void ej_getboottime(webs_t wp, int argc, char_t **argv)
{
	char *end = nvram_safe_get("end_time");
	if (!*end) {
		websWrite(wp, "30");
		return;
	}
	time_t endtime = atol(end);
	time_t starttime = nvram_default_geti("start_time", 0);
	if (starttime <= endtime)
		websWrite(wp, "30");
	else {
		time_t boottime = starttime - endtime;
		if (boottime > 300)
			boottime = 20; //something is strange
		websWrite(wp, "%ld", boottime + 10); //aprox boot time
	}
}

char *tran_string(char *buf, size_t len, char *str)
{
	snprintf(buf, len - 1, "<script type=\"text/javascript\">Capture(%s)</script>", str);
	return buf;
}

EJ_VISIBLE void ej_tran(webs_t wp, int argc, char_t **argv)
{
	char buf[128];
	websWrite(wp, "%s", tran_string(buf, sizeof(buf), argv[0]));
}

EJ_VISIBLE void ej_live_tran(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "%s", live_translate(wp, argv[0]));
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_checked("wan_proto", "dhcp"); %> produces: checked="checked"
 * <% nvram_checked_js("wan_proto", "dhcp"); %> produces: checked=\"checked\"
 * <% nvram_checked("wan_proto", "static"); %> does not produce
 */

EJ_VISIBLE void ej_nvram_checked(webs_t wp, int argc, char_t **argv)
{
	if (nvram_match(argv[0], argv[1])) {
		websWrite(wp, "checked=\"checked\"");
	}

	return;
}

EJALIAS(ej_nvram_checked, ej_nvc);

EJ_VISIBLE void ej_nvram_checked_js(webs_t wp, int argc, char_t **argv)
{
	if (nvram_match(argv[0], argv[1])) {
		websWrite(wp, "checked=\\\"checked\\\"");
	}

	return;
}

EJ_VISIBLE void ej_make_time_list(webs_t wp, int argc, char_t **argv)
{
	int i, st, en;
	char ic[16];

	st = atoi(argv[1]);
	en = atoi(argv[2]);

	for (i = st; i <= en; i++) {
		sprintf(ic, "%d", i);
		websWrite(wp, "<option value=\"%d\" %s >%02d</option>\n", i,
			  nvram_match(argv[0], ic) ? "selected=\"selected\"" : "", i);
	}

	return;
}

#ifdef HAVE_QCN
#include <qcnapi.h>
#endif

EJ_VISIBLE void ej_get_service_state(webs_t wp, int argc, char_t **argv)
{
	char buf[128];
	websWrite(wp, "<div class=\"setting\">");
	show_caption(wp, "label", "idx.dhcp_srv", NULL);
	if (nvram_match("lan_proto", "dhcp")) {
		websWrite(wp, "%s", tran_string(buf, sizeof(buf), "share.enabled"));
		if (pidof("dnsmasq") > 0) {
			websWrite(wp, " - %s", tran_string(buf, sizeof(buf), "diag.running"));
		} else {
			websWrite(wp, " - %s", tran_string(buf, sizeof(buf), "diag.stopped"));
		}
	} else {
		websWrite(wp, "%s", tran_string(buf, sizeof(buf), "share.disabled"));
	}
	websWrite(wp, "&nbsp;</div>");

#ifdef HAVE_SAMBA_SERVER
	websWrite(wp, "<div class=\"setting\"><div class=\"label\">%s</div>", tran_string(buf, sizeof(buf), "nas.samba3"));
	if (nvram_matchi("samba3_enable", 1)) {
		websWrite(wp, "%s", tran_string(buf, sizeof(buf), "share.enabled"));
#ifdef HAVE_SMBD
		if (pidof("ksmbd.mountd") > 0) {
#else
		if (pidof("smbd") > 0) {
#endif
			websWrite(wp, " - %s", tran_string(buf, sizeof(buf), "diag.running"));
		} else {
			websWrite(wp, " - %s", tran_string(buf, sizeof(buf), "diag.stopped"));
		}
	} else {
		websWrite(wp, "%s", tran_string(buf, sizeof(buf), "share.disabled"));
	}
	websWrite(wp, "&nbsp;</div>");
#endif
}

#include "cputemp.c"
#include "voltage.c"

static void showencstatus(webs_t wp, char *prefix)
{
	char akm[64];
	char sec[64];
	char *enc = NULL;
	sprintf(akm, "%s_akm", prefix);
	sprintf(sec, "%s_security_mode", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.encrypt)</script>&nbsp;-&nbsp;<script type=\"text/javascript\">Capture(share.intrface)</script>&nbsp;%s</div>\n",
		prefix);
	websWrite(wp, "<script type=\"text/javascript\">");
	if (nvram_match(akm, "radius"))
		enc = strdup("RADIUS");
	else if (nvram_match(akm, "8021X"))
		enc = strdup("802.1x");
	else if (nvhas(akm, "peap") || nvhas(akm, "leap") || nvhas(akm, "tls") || nvhas(akm, "ttls")) {
		char type[128] = { 0 };
		if (nvhas(akm, "wpa"))
			sprintf(type, "WPA");
		if (nvhas(akm, "wpa2"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA2");
		if (nvhas(akm, "wpa2-sha256"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA2-SHA256");
		if (nvhas(akm, "wpa2-sha384"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA2-SHA384");
		if (nvhas(akm, "wpa3"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA3");
		if (nvhas(akm, "wpa3-128"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA3-SUITE-B");
		if (nvhas(akm, "wpa3-192"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA3-SUITE-B-192");
		if (nvhas(akm, "wep"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "802.1X");
		asprintf(&enc, "%s%s%s%s%s", type[0] ? type : "UNKNOWN", nvhas(akm, "peap") ? "-PEAP" : "",
			 nvhas(akm, "leap") ? "-LEAP" : "", nvhas(akm, "tls") ? "-TLS" : "", nvhas(akm, "ttls") ? "-TTLS" : "");
	} else {
		char type[128] = { 0 };
		if (nvhas(akm, "owe"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "OWE");
		if (nvhas(akm, "psk"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA-PSK");
		if (nvhas(akm, "psk2"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA2-PSK");
		if (nvhas(akm, "psk2-sha256"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA2-PSK-SHA256");
		if (nvhas(akm, "psk3"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA3-PSK");
		if (nvhas(akm, "wpa"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA-EAP");
		if (nvhas(akm, "wpa2"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA2-EAP");
		if (nvhas(akm, "wpa2-sha256"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA2-EAP-SHA256");
		if (nvhas(akm, "wpa2-sha384"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA2-EAP-SHA384");
		if (nvhas(akm, "wpa3"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA3-EAP");
		if (nvhas(akm, "wpa3-128"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA3-EAP-SUITE-B");
		if (nvhas(akm, "wpa3-192"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WPA3-EAP-SUITE-B-192");
		if (nvhas(akm, "wep"))
			sprintf(type, "%s%s%s", type, type[0] ? "/" : "", "WEP");
		if (type[0])
			enc = strdup(type);
	}
	if (enc) {
		websWrite(wp, "Capture(share.enabled)");
		websWrite(wp, "</script>,&nbsp;");
		websWrite(wp, enc);
		debug_free(enc);
	} else {
		websWrite(wp, "Capture(share.disabled)");
		websWrite(wp, "</script>");
	}

	websWrite(wp, "\n</div>\n");
	return;
}

EJ_VISIBLE void ej_get_txpower(webs_t wp, int argc, char_t **argv)
{
	char txpwr[32];
	char m[32];
	int txpower;
	char mode[32];
	char net_mode[32];

	strncpy(m, nvram_safe_get("wifi_display"), 5);
	m[5] = 0;
	sprintf(net_mode, "%s_net_mode", m);
	sprintf(mode, "%s_mode", m);
	if (nvram_match(net_mode, "disabled") || nvram_match(mode, "disabled")) {
		txpower = 0;
		websWrite(wp, "%s", live_translate(wp, "wl_basic.radio_off"));
	} else {
		sprintf(txpwr, "%s_txpwr", m);
#ifdef HAVE_MADWIFI
		txpower = wifi_gettxpower(m);
#elif HAVE_RT2880
		txpower = nvram_geti(txpwr);
#else //broadcom
		txpower = bcm_gettxpower(m);
#endif
#ifdef HAVE_BUFFALO
		get_txpower_extended(wp, txpower, m);
#else
#ifdef HAVE_MADWIFI
		websWrite(wp, "%d dBm", txpower);
#elif HAVE_RT2880
		websWrite(wp, "%d mW", txpower);
#else
#ifdef HAVE_80211AC //broadcom
		if (txpower == 1496) {
			websWrite(wp, "Auto");
		} else
#endif
		{
			websWrite(wp, "%d mW", txpower);
		}
#endif
#endif
	}
}

EJ_VISIBLE void ej_getencryptionstatus(webs_t wp, int argc, char_t **argv)
{
	char *mode = nvram_safe_get("wifi_display");

	showencstatus(wp, mode);
}

EJ_VISIBLE void ej_getwirelessstatus(webs_t wp, int argc, char_t **argv)
{
	char var[32];
	char m[32];
	int showap = 0, showcli = 0;

	strncpy(m, nvram_safe_get("wifi_display"), 5);
	m[5] = 0;
	sprintf(var, "%s_mode", m);

	if (is_ap(m)) {
		showap = 1; // "Clients"
	} else {
		showcli = 1; // "Access Point"
		sprintf(var, "%s_vifs", m);
		if (*(nvram_safe_get(var)))
			showap = 1; // " & Clients"
	}

	if (showcli)
		websWrite(wp, "<script type=\"text/javascript\">Capture(info.ap)</script>");
	if (showcli && showap)
		websWrite(wp, " & ");
	if (showap)
		websWrite(wp, "<script type=\"text/javascript\">Capture(status_wireless.legend3)</script>");
}

EJ_VISIBLE void ej_getwirelessssid(webs_t wp, int argc, char_t **argv)
{
	char ssid[32];

	char *ifname = nvram_safe_get("wifi_display");
	if (has_ad(ifname))
		ifname = "wlan2";
	sprintf(ssid, "%s_ssid", ifname);
	tf_webWriteESCNV(wp, ssid);
}

EJ_VISIBLE void ej_getwirelessmode(webs_t wp, int argc, char_t **argv)
{
	char mode[32];

	char *ifname = nvram_safe_get("wifi_display");
	if (has_ad(ifname))
		ifname = "wlan2";
	sprintf(mode, "%s_mode", ifname);

	websWrite(wp, "<script type=\"text/javascript\">");
	if (nvram_match(mode, "disabled"))
		websWrite(wp, "Capture(share.disabled)");
	if (nvram_match(mode, "wet"))
		websWrite(wp, "Capture(wl_basic.clientBridge)");
	if (nvram_match(mode, "ap"))
		websWrite(wp, "Capture(wl_basic.ap)");
	if (nvram_match(mode, "sta"))
		websWrite(wp, "Capture(wl_basic.client)");
	if (nvram_match(mode, "infra"))
		websWrite(wp, "Capture(wl_basic.adhoc)");
	if (nvram_match(mode, "apsta"))
		websWrite(wp, "Capture(wl_basic.repeater)");
	if (nvram_match(mode, "apstawet"))
		websWrite(wp, "Capture(wl_basic.repeaterbridge)");
	if (nvram_match(mode, "wdssta"))
		websWrite(wp, "Capture(wl_basic.wdssta)");
	if (nvram_match(mode, "wdssta_mtik"))
		websWrite(wp, "Capture(wl_basic.wdssta_mtik)");
	if (nvram_match(mode, "wdsap"))
		websWrite(wp, "Capture(wl_basic.wdsap)");
	if (nvram_match(mode, "apup"))
		websWrite(wp, "Capture(wl_basic.apup)");
	if (nvram_match(mode, "mesh"))
		websWrite(wp, "Capture(wl_basic.mesh)");
	if (nvram_match(mode, "tdma"))
		websWrite(wp, "Capture(wl_basic.tdma)");
	websWrite(wp, "</script>&nbsp;\n");
}

EJ_VISIBLE void ej_getwirelessnetmode(webs_t wp, int argc, char_t **argv)
{
	char netmode[32];
	char mode[32];
	char m[32];

	char *ifname = nvram_safe_get("wifi_display");
	if (has_ad(ifname))
		ifname = "wlan2";
	strncpy(m, ifname, 5);
	m[5] = 0;
	sprintf(netmode, "%s_net_mode", m);

	websWrite(wp, "<script type=\"text/javascript\">");
	if (nvram_match(netmode, "disabled") || nvram_match(mode, "disabled"))
		websWrite(wp, "Capture(share.disabled)");
	else {
		if (nvram_match(netmode, "mixed"))
			websWrite(wp, "Capture(wl_basic.mixed)");
		if (nvram_match(netmode, "bg-mixed"))
			websWrite(wp, "Capture(wl_basic.bg)");
		if (nvram_match(netmode, "g-only"))
			websWrite(wp, "Capture(wl_basic.g)");
		if (nvram_match(netmode, "b-only"))
			websWrite(wp, "Capture(wl_basic.b)");
		if (nvram_match(netmode, "n-only"))
			websWrite(wp, "Capture(wl_basic.n)");
		if (nvram_match(netmode, "a-only"))
			websWrite(wp, "Capture(wl_basic.a)");
		if (nvram_match(netmode, "na-only"))
			websWrite(wp, "Capture(wl_basic.na)");
		if (nvram_match(netmode, "ng-only"))
			websWrite(wp, "Capture(wl_basic.ng)");
		if (nvram_match(netmode, "n2-only"))
			websWrite(wp, "Capture(wl_basic.n2)");
		if (nvram_match(netmode, "n5-only"))
			websWrite(wp, "Capture(wl_basic.n5)");
		if (nvram_match(netmode, "ac-only"))
			websWrite(wp, "Capture(wl_basic.ac)");
		if (nvram_match(netmode, "ax-only"))
			websWrite(wp, "Capture(wl_basic.ax)");
		if (nvram_match(netmode, "axg-only"))
			websWrite(wp, "Capture(wl_basic.axg)");
		if (nvram_match(netmode, "ad-only"))
			websWrite(wp, "Capture(wl_basic.ad)");
		if (nvram_match(netmode, "acn-mixed"))
			websWrite(wp, "Capture(wl_basic.acn)");
		if (nvram_match(netmode, "xacn-mixed"))
			websWrite(wp, "Capture(wl_basic.xacn)");
	}
	websWrite(wp, "</script>&nbsp;\n");
}

#ifdef HAVE_OPENVPN
EJ_VISIBLE void ej_show_openvpn_status(webs_t wp, int argc, char_t **argv)
{
	websWrite(
		wp,
		"<fieldset class=\"dark_fs_bg\">\n<legend><script type=\"text/javascript\">Capture(share.state)</script></legend>\n");
	char *buffer = malloc(4096);
	int len;

	FILE *in = popen("/etc/openvpnstate.sh", "r");

	while ((len = fread(buffer, 1, 4095, in)) == 4095) {
		buffer[len] = 0;
		wfputs(buffer, wp);
	}
	if (len) {
		buffer[len] = 0;
		wfputs(buffer, wp);
	}

	pclose(in);
	websWrite(wp, "</fieldset><br />");
	websWrite(
		wp,
		"<fieldset class=\"dark_fs_bg\">\n<legend><script type=\"text/javascript\">Capture(share.statu)</script></legend>\n");
	in = popen("/etc/openvpnstatus.sh", "r");
	while ((len = fread(buffer, 1, 4095, in)) == 4095) {
		buffer[len] = 0;
		wfputs(buffer, wp);
	}
	if (len) {
		buffer[len] = 0;
		wfputs(buffer, wp);
	}
	pclose(in);
	websWrite(wp, "</fieldset><br />");
	websWrite(
		wp,
		"<fieldset class=\"dark_fs_bg\">\n<legend><script type=\"text/javascript\">Capture(log.legend)</script></legend>\n");
	in = popen("/etc/openvpnlog.sh", "r");
	while ((len = fread(buffer, 1, 4095, in)) == 4095) {
		buffer[len] = 0;
		wfputs(buffer, wp);
	}
	if (len) {
		buffer[len] = 0;
		wfputs(buffer, wp);
	}
	pclose(in);
	debug_free(buffer);
	websWrite(wp, "</fieldset><br />");
}
#endif

EJ_VISIBLE void ej_radio_on(webs_t wp, int argc, char_t **argv)
{
	int radiooff = -1;

#ifdef HAVE_MADWIFI
	char *ifname = nvram_safe_get("wifi_display");

	if (*(ifname)) {
		int state = get_radiostate(ifname);

		switch (state) {
		case 1:
			websWrite(wp, "1");
			break;
		default:
			websWrite(wp, "0");
			break;
		}
	} else {
		websWrite(wp, "0");
	}
#elif HAVE_RT2880

	int state = get_radiostate(nvram_safe_get("wifi_display"));

	switch (state) {
	case 1:
		websWrite(wp, "1");
		break;
	default:
		websWrite(wp, "0");
		break;
	}
#else
	char name[32];
	sprintf(name, "%s_ifname", nvram_safe_get("wifi_display"));

	char *ifname = nvram_safe_get(name);

	wl_ioctl(ifname, WLC_GET_RADIO, &radiooff, sizeof(int));

	switch ((radiooff & WL_RADIO_SW_DISABLE)) {
	case 0:
		websWrite(wp, "1");
		break;
	default:
		websWrite(wp, "0");
		break;
	}
#endif
}

static void get_radio_state(char *buf)
{
	int radiooff = -1;
	char *wifi = nvram_safe_get("wifi_display");
#ifdef HAVE_MADWIFI
	char *ifname = wifi;

	if (*(ifname)) {
		int state = get_radiostate(ifname);

		switch (state) {
		case 1:
			strcpy(buf, "wl_basic.radio_on");
			break;
		case -1:
			strcpy(buf, "share.unknown");
			break;
		default: // 1: software disabled, 2: hardware
			// disabled, 3: both are disabled
			strcpy(buf, "wl_basic.radio_off");
			break;
		}
	} else {
		strcpy(buf, "share.unknown");
	}
#elif HAVE_RT2880

	int state = get_radiostate(wifi);

	switch (state) {
	case 1:
		strcpy(buf, "wl_basic.radio_on");
		break;
	case -1:
		strcpy(buf, "share.unknown");
		break;
	default: // 1: software disabled, 2: hardware
		// disabled, 3: both are disabled
		strcpy(buf, "wl_basic.radio_off");
		break;
	}
#else
#ifdef HAVE_QTN
	if (!strcmp(wifi, "wl1")) {
		char status[16];
		if (!rpc_qtn_ready()) {
			strcpy(buf, "share.unknown");
			return;
		}
		qcsapi_interface_get_status("wifi0", status);
		if (!strcmp(status, "Up"))
			strcpy(buf, "wl_basic.radio_on");
		else
			strcpy(buf, "wl_basic.radio_off");
		return;
	}
#endif

	char name[32];
	sprintf(name, "%s_ifname", wifi);

	char *ifname = nvram_safe_get(name);

	wl_ioctl(ifname, WLC_GET_RADIO, &radiooff, sizeof(int));

	switch ((radiooff & WL_RADIO_SW_DISABLE)) {
	case 0:
		strcpy(buf, "wl_basic.radio_on");
		break;
	case -1:
		strcpy(buf, "share.unknown");
		break;
	default: // 1: software disabled, 2: hardware
		// disabled, 3: both are disabled
		strcpy(buf, "wl_basic.radio_off");
		break;
	}
#endif
}

EJ_VISIBLE void ej_get_radio_state(webs_t wp, int argc, char_t **argv)
{
	char buf[64];
	bzero(buf, sizeof(buf));
	get_radio_state(buf);
	websWrite(wp, "%s", live_translate(wp, buf));
}

EJ_VISIBLE void ej_get_radio_statejs(webs_t wp, int argc, char_t **argv)
{
	char buf[64];
	bzero(buf, sizeof(buf));
	get_radio_state(buf);
	websWrite(wp, "<script type=\"text/javascript\">Capture(%s)</script>&nbsp;", buf);
}

#include "arp.c"

#ifdef HAVE_PPPOESERVER

EJ_VISIBLE void ej_dumppppoe(webs_t wp, int argc, char_t **argv)
{
	FILE *in = fopen("/tmp/pppoe_connected", "rb");
	if (!in)
		return;
	char pid[32];
	char ifname[32];
	char local[32];
	char remote[32];
	char peer[64];
	int count = 0;
	while (fscanf(in, "%s %s %s %s", pid, ifname, local, peer) == 4) {
		websWrite(wp, "%c\"%s\",\"%s\",\"%s\",\"%s\"", count ? ',' : ' ', ifname, peer, local, pid);
		count++;
		if (feof(in))
			break;
	}
	fclose(in);
	return;
}

#endif

int tf_webWriteESC(webs_t wp, const char *value)
{
	char buf[512];
	int n;
	int r;
	const char *c;

	n = 0;
	r = 0;
	for (c = value; *c; c++) {
		if (n < (sizeof(buf) - 1) && (isprint(*c)) && (*c != '"') && (*c != '&') && (*c != '<') && (*c != '>') &&
		    (*c != '\'') && (*c != '\\')) {
			buf[n++] = *c;
		} else {
			n += snprintf(&buf[n], sizeof(buf) - n, "&#%d;", *c);
		}
		if (n > (sizeof(buf) - 10) && n < (sizeof(buf) - 1)) { // ! extra space for &...
			buf[n] = 0;
			n = 0;
			r += wfputs(buf, wp);
		}
	}
	if (n > 0 && n < (sizeof(buf) - 1)) {
		buf[n] = 0;
		r += wfputs(buf, wp);
	}
	return r;
}

int tf_webWriteESCNV(webs_t wp, const char *nvname)
{
	return tf_webWriteESC(wp, nvram_safe_get(nvname));
}

int tf_webWriteJS(webs_t wp, const char *s)
{
	char buf[512];
	int n;
	int r;
	unsigned char *c = (unsigned char *)s;

	n = 0;
	r = 0;
	for (; *c; c++) {
		if (*c == 0xc3) {
			c++;
			switch (*c) {
			case 0xa3:
				n += snprintf(buf + n, sizeof(buf) - n, "&auml;");
				break;
			case 0xb6:
				n += snprintf(buf + n, sizeof(buf) - n, "&ouml;");
				break;
			case 0xbc:
				n += snprintf(buf + n, sizeof(buf) - n, "&uuml;");
				break;
			case 0xc4:
				n += snprintf(buf + n, sizeof(buf) - n, "&Auml;");
				break;
			case 0xd6:
				n += snprintf(buf + n, sizeof(buf) - n, "&Ouml;");
				break;
			case 0xdc:
				n += snprintf(buf + n, sizeof(buf) - n, "&Uuml;");
				break;
			case 0xdf:
				n += snprintf(buf + n, sizeof(buf) - n, "&szlig;");
				break;
			default:
				s--;
			}
		} else if (*c == '<') {
			n += snprintf(buf + n, sizeof(buf) - n, "&lt;");
		} else if (*c == '\'') {
			continue;
		} else if (*c == '>') {
			n += snprintf(buf + n, sizeof(buf) - n, "&gt;");
		} else if (n < (sizeof(buf) - 1) && (*c != '"') && (*c != '\\') && (*c != '/') && (*c != '*') && (isprint(*c))) {
			buf[n++] = *c;
		} else {
			n += snprintf(&buf[n], sizeof(buf) - n, "\\x%02x", *c);
		}
		if (n > (sizeof(buf) - 10) && n < (sizeof(buf) - 1)) { // ! extra space for \xHH
			buf[n] = 0;
			n = 0;
			r += wfputs(buf, wp);
		}
	}
	if (n > 0 && n < (sizeof(buf) - 1)) {
		buf[n] = 0;
		r += wfputs(buf, wp);
	}
	return r;
}

#ifdef HAVE_UPNP
// changed by steve
// writes javascript-string safe text

// <% tf_upnp(); %>
// returns all "forward_port#" nvram entries containing upnp port forwardings
EJ_VISIBLE void ej_tf_upnp(webs_t wp, int argc, char_t **argv)
{
	int i;
	int len, pos, count;
	char *temp;

	if (nvram_matchi("upnp_enable", 1)) {
		for (i = 0; i < 50; i++) {
			websWrite(wp, (i > 0) ? ",'" : "'");

			// fix: some entries are missing the desc. - this breaks the
			// upnp.asp page, so we add ,*
			temp = nvram_nget("forward_port%d", i);
			count = 0;
			len = strlen(temp);

			for (pos = len; pos != 0; pos--) {
				if (temp[pos] == ',')
					count++;
			}
			tf_webWriteJS(wp, temp);
			if (count == 2)
				websWrite(wp, ",*");

			websWrite(wp, "'");
		}
	}
}

// end changed by steve
#endif

//extern void show_onlineupdates(webs_t wp, int argc, char_t ** argv);

EJ_VISIBLE void ej_show_upgrade_options(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_BUFFALO
#ifndef HAVE_FREECWMP
	show_onlineupdates(wp, argc, argv);
#endif
#endif
}
EJ_VISIBLE void ej_getsetuppage(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_BUFFALO
	if (endswith(wp->request_url, ".asp") || endswith(wp->request_url, ".htm") || endswith(wp->request_url, ".html")) {
		websWrite(wp, "%s", wp->request_url);
	} else {
		websWrite(wp, "SetupAssistant.asp");
	}
#else
	websWrite(wp, "Info.htm");
#endif
}
EJ_VISIBLE void ej_wan_if_status(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_DSL_CPE_CONTROL
	char *annex = nvram_safe_get("annex");
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "  <legend>DSL Status</legend>\n");
	websWrite(wp, "  <div class=\"setting\">\n");
	websWrite(wp, "    <div class=\"label\"><script type=\"text/javascript\">Capture(dsl.annex)</script></div>\n");
	websWrite(wp, "    <span>%c</span>\n", toupper(annex[0]));
	websWrite(wp, "  </div>\n");
	websWrite(wp, "  <div class=\"setting\">\n");
	websWrite(wp, "    <div class=\"label\"><script type=\"text/javascript\">Capture(dsl.iface_status)</script></div>\n");
	websWrite(wp, "    <span id=\"dsl_iface_status\">%s</span>\n", nvram_safe_get("dsl_iface_status"));
	websWrite(wp, "  </div>\n");
	websWrite(wp, "  <div class=\"setting\">\n");
	websWrite(wp, "    <div class=\"label\"><script type=\"text/javascript\">Capture(dsl.datarate)</script></div>\n");
	websWrite(wp, "    <span id=\"dsl_datarate_ds\">%11.2f</span> MBit / <span id=\"dsl_datarate_us\">%11.2f</span> MBit\n",
		  atof(nvram_safe_get("dsl_datarate_ds")), atof(nvram_safe_get("dsl_datarate_us")));
	websWrite(wp, "  </div>\n");
	websWrite(wp, "  <div class=\"setting\">\n");
	websWrite(wp, "    <div class=\"label\"><script type=\"text/javascript\">Capture(dsl.snr)</script></div>\n");
	websWrite(wp, "    <span id=\"dsl_snr_up\">%d</span> dB / <span id=\"dsl_snr_down\">%d</span> dB\n",
		  nvram_geti("dsl_snr_up"), nvram_geti("dsl_snr_down"));
	websWrite(wp, "  </div>\n");
	websWrite(wp, "</fieldset>\n");
	websWrite(wp, "<br />\n");
#endif
}

#ifdef HAVE_SPOTPASS
EJ_VISIBLE void ej_spotpass_servers(webs_t wp, int argc, char_t **argv)
{
	char url[128], proto[8], ports[64];
	char dummy1[1], dummy2[8];
	int port1, port2;
	char *ptr;
	char *serverlist = strdup(nvram_safe_get("spotpass_servers"));
	ptr = strtok(serverlist, "|");
	while (ptr != NULL) {
		if (sscanf(ptr, "%s %s %s %s %d %d", &dummy1, &url, &proto, &dummy2, &port1, &port2) == 6) {
			websWrite(wp, "%s %s %d,%d", url, proto, port1, port2);
		} else if (sscanf(ptr, "%s %s %s %d %d", &dummy1, &url, &proto, &port1, &port2) == 5) {
			websWrite(wp, "%s %s %d,%d", url, proto, port1, port2);
		} else if (sscanf(ptr, "%s %s %s", &url, &proto, &ports) == 3) {
			websWrite(wp, "%s %s %s", url, proto, ports);
		}
		ptr = strtok(NULL, "|");
		if (ptr != NULL) {
			websWrite(wp, "\n");
		}
	}
}
#endif
