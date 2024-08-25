/*
 * static_route.c
 *
 * Copyright (C) 2005 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>

#include <broadcom.h>
static void _show_ruleif(webs_t wp, int argc, char_t **argv, char *page, char *rules, int index, int any)
{
	int which;
	char word[256];
	char *next = NULL;
	char ifnamecopy[32];
	char bufferif[512];

	which = websGetVari(wp, page, 0);
	strcpy(ifnamecopy, "br0");

	char *sroute = nvram_safe_get(rules);

	foreach(word, sroute, next)
	{
		if (which-- == 0) {
			GETENTRYBYIDX_DEL(ifname, word, index, ":");
			if (!ifname)
				break;
			strcpy(ifnamecopy, ifname);
		}
	}

	bzero(bufferif, 512);
	getIfList(bufferif, NULL);
	websWrite(wp, "<option value=\"lan\" %s >LAN &amp; WLAN</option>\n",
		  nvram_match("lan_ifname", ifnamecopy) ? "selected=\"selected\"" : "");
	websWrite(wp, "<option value=\"wan\" %s >WAN</option>\n",
		  nvram_match("wan_ifname", ifnamecopy) ? "selected=\"selected\"" : "");
	if (any)
		websWrite(wp, "<option value=\"any\" %s >ANY</option>\n",
			  strcmp("any", ifnamecopy) == 0 ? "selected=\"selected\"" : "");
	bzero(word, 256);
	next = NULL;
	foreach(word, bufferif, next)
	{
		if (nvram_match("lan_ifname", word))
			continue;
		if (nvram_match("wan_ifname", word))
			continue;
		if (isbridged(word))
			continue;
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", word,
			  strcmp(word, ifnamecopy) == 0 ? "selected=\"selected\"" : "", getNetworkLabel(wp, word));
	}
}

EJ_VISIBLE void ej_show_routeif(webs_t wp, int argc, char_t **argv)
{
	_show_ruleif(wp, argc, argv, "route_page", "static_route", 4, 1);
}

#ifndef HAVE_MICRO
EJ_VISIBLE void ej_show_ruleiif(webs_t wp, int argc, char_t **argv)
{
	_show_ruleif(wp, argc, argv, "rule_page", "pbr_rule", 9, 0);
}

EJ_VISIBLE void ej_show_ruleoif(webs_t wp, int argc, char_t **argv)
{
	_show_ruleif(wp, argc, argv, "rule_page", "pbr_rule", 15, 0);
}
#endif
/*
 * Example: 
 * static_route=192.168.2.0:255.255.255.0:192.168.1.2:1:br0
 * <% static_route("ipaddr", 0); %> produces "192.168.2.0"
 * <% static_route("lan", 0); %> produces "selected" if nvram_match("lan_ifname", "br0")
 */
EJ_VISIBLE void ej_static_route_setting(webs_t wp, int argc, char_t **argv)
{
	char *arg;
	int which, count;
	char word[256];
	char *next, *page;
	char name[50] = "";
	char new_name[200];
	arg = argv[0];

	which = websGetVari(wp, "route_page", 0);

	if (!strcmp(arg, "name")) {
		char *sroutename = nvram_safe_get("static_route_name");

		foreach(word, sroutename, next)
		{
			if (which-- == 0 || (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-")))) {
				find_match_pattern(name, sizeof(name), word, "$NAME:", "");
				httpd_filter_name(name, new_name, sizeof(new_name), GET);
				websWrite(wp, new_name);
				return;
			}
		}
	}
	char *sroute = nvram_safe_get("static_route");

	foreach(word, sroute, next)
	{
		//if (which-- == 0) {
		if (which-- == 0 || (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-")))) {
			GETENTRYBYIDX_DEL(s_flags, word, 6, ":");
			int flags = 0;
			if (s_flags)
				sscanf(s_flags, "%X", &flags);
			int src_en = flags & 0x1;
			int scope_en = flags & 0x2;
			int table_en = flags & 0x4;
			int mtu_en = flags & 0x8;
			int advmss_en = flags & 0x10;
			if (!strcmp(arg, "ipaddr")) {
				GETENTRYBYIDX_DEL(ipaddr, word, 0, ":");
				if (ipaddr) {
					websWrite(wp, "%d", get_single_ip(ipaddr, atoi(argv[1])));
					return;
				}
			} else if (!strcmp(arg, "netmask")) {
				GETENTRYBYIDX_DEL(netmask, word, 1, ":");
				if (netmask) {
					websWrite(wp, "%d", getmask(netmask));
					return;
				}
			} else if (!strcmp(arg, "gateway")) {
				GETENTRYBYIDX_DEL(gateway, word, 2, ":");
				if (gateway) {
					websWrite(wp, "%d", get_single_ip(gateway, atoi(argv[1])));
					return;
				}
			} else if (!strcmp(arg, "metric")) {
				GETENTRYBYIDX_DEL(metric, word, 3, ":");
				if (metric) {
					websWrite(wp, metric);
					return;
				}
			} else if (!strcmp(arg, "nat")) {
				GETENTRYBYIDX_DEL(nat, word, 5, ":");
				if (nat && !strcmp(nat, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "lan")) {
				GETENTRYBYIDX_DEL(ifname, word, 4, ":");
				if (nvram_match("lan_ifname", ifname)) {
					websWrite(wp, "selected=\"selected\"");
				}
				return;
			} else if (!strcmp(arg, "wan")) {
				GETENTRYBYIDX_DEL(ifname, word, 4, ":");
				if (nvram_match("wan_ifname", ifname)) {
					websWrite(wp, "selected=\"selected\"");
				}
				return;
			} else if (!strcmp(arg, "scope_en")) {
				if (scope_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "scope")) {
				GETENTRYBYIDX_DEL(scope, word, 8, ":");
				if (scope && !strcmp(argv[1], scope)) {
					websWrite(wp, "selected=\"selected\"");
				}
				return;
			} else if (!strcmp(arg, "mtu_en")) {
				if (mtu_en)
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(arg, "mtu")) {
				GETENTRYBYIDX_DEL(mtu, word, 10, ":");
				if (mtu) {
					websWrite(wp, mtu);
					return;
				}
			} else if (!strcmp(arg, "advmss_en")) {
				if (advmss_en)
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(arg, "advmss")) {
				GETENTRYBYIDX_DEL(advmss, word, 11, ":");
				if (advmss) {
					websWrite(wp, advmss);
					return;
				}
			} else if (!strcmp(arg, "table_en")) {
				if (table_en)
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(arg, "table")) {
				GETENTRYBYIDX_DEL(table, word, 9, ":");
				if (table) {
					websWrite(wp, table);
					return;
				}
			} else if (!strcmp(arg, "src_en")) {
				if (src_en)
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(arg, "src")) {
				GETENTRYBYIDX_DEL(src, word, 7, ":");
				if (src) {
					websWrite(wp, "%d", get_single_ip(src, atoi(argv[1])));
					return;
				}
			}
			break;
		}
	}
	if (!strcmp(arg, "ipaddr") || !strcmp(arg, "netmask") || !strcmp(arg, "src") || !strcmp(arg, "gateway"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "metric"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "mtu"))
		websWrite(wp, "1500");
	else if (!strcmp(arg, "advmss"))
		websWrite(wp, "1460");
	else if (!strcmp(arg, "table"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "scope"))
		websWrite(wp, "global");

	return;
}

#ifndef HAVE_MICRO
EJ_VISIBLE void ej_pbr_rule_setting(webs_t wp, int argc, char_t **argv)
{
	char *arg;
	int which, count;
	char word[256];
	char *next, *page;
	char name[50] = "";
	char new_name[200];
	arg = argv[0];

	which = websGetVari(wp, "rule_page", 0);

	if (!strcmp(arg, "name")) {
		char *sroutename = nvram_safe_get("pbr_rule_name");

		foreach(word, sroutename, next)
		{
			if (which-- == 0 || (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-")))) {
				find_match_pattern(name, sizeof(name), word, "$NAME:", "");
				httpd_filter_name(name, new_name, sizeof(new_name), GET);
				websWrite(wp, new_name);
				return;
			}
		}
	}
	char *sroute = nvram_safe_get("pbr_rule");

	foreach(word, sroute, next)
	{
		//if (which-- == 0) {
		if (which-- == 0 || (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-")))) {
			GETENTRYBYIDX_DEL(s_flags, word, 0, ":");
			int flags = 0;
			if (s_flags)
				sscanf(s_flags, "%X", &flags);
			int not = flags & 0x1;
			int from_en = flags & 0x2;
			int to_en = flags & 0x4;
			int priority_en = flags & 0x8;
			int tos_en = flags & 0x10;
			int fwmark_en = flags & 0x20;
			int realms_en = flags & 0x40;
			int table_en = flags & 0x80;
			int suppress_prefixlength_en = flags & 0x100;
			int iif_en = flags & 0x200;
			int nat_en = flags & 0x400;
			int type_en = flags & 0x800;
			int ipproto_en = flags & 0x1000;
			int sport_en = flags & 0x2000;
			int dport_en = flags & 0x4000;
			int oif_en = flags & 0x8000;

			if (!strcmp(arg, "not")) {
				if (not )
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "from_en")) {
				if (from_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "to_en")) {
				if (to_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "priority_en")) {
				if (priority_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "tos_en")) {
				if (tos_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "fwmark_en")) {
				if (fwmark_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "realms_en")) {
				if (realms_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "table_en")) {
				if (table_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "suppress_prefixlength_en")) {
				if (suppress_prefixlength_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "iif_en")) {
				if (iif_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "oif_en")) {
				if (oif_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "nat_en")) {
				if (nat_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "type_en")) {
				if (type_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "sport_en")) {
				if (sport_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "dport_en")) {
				if (dport_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "ipproto_en")) {
				if (ipproto_en)
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "from")) {
				GETENTRYBYIDX_DEL(from, word, 1, ":");
				if (from) {
					websWrite(wp, "%d", get_single_ip(from, atoi(argv[1])));

					return;
				}
			} else if (!strcmp(arg, "to")) {
				GETENTRYBYIDX_DEL(to, word, 2, ":");
				if (to) {
					websWrite(wp, "%d", get_single_ip(to, atoi(argv[1])));
					return;
				}
			} else if (!strcmp(arg, "priority")) {
				GETENTRYBYIDX_DEL(priority, word, 3, ":");
				if (priority) {
					websWrite(wp, priority);
					return;
				}
			} else if (!strcmp(arg, "tos")) {
				GETENTRYBYIDX_DEL(tos, word, 4, ":");
				if (tos) {
					websWrite(wp, tos);
					return;
				}
			} else if (!strcmp(arg, "fwmark")) {
				GETENTRYBYIDX_DEL(fwmark, word, 5, ":");
				if (fwmark) {
					char *slash = strchr(fwmark, '/');
					if (slash)
						*slash = '\0';
					if (!strcmp(argv[1], "0"))
						websWrite(wp, fwmark);
					else {
						if (slash)
							websWrite(wp, slash + 1);
						else
							websWrite(wp, "0xffffffff");
					}
					return;
				}
			} else if (!strcmp(arg, "realms")) {
				GETENTRYBYIDX_DEL(realms, word, 6, ":");
				if (realms) {
					websWrite(wp, realms);
					return;
				}
			} else if (!strcmp(arg, "table")) {
				GETENTRYBYIDX_DEL(table, word, 7, ":");
				if (table) {
					websWrite(wp, table);
					return;
				}
			} else if (!strcmp(arg, "suppress_prefixlength")) {
				GETENTRYBYIDX_DEL(suppress_prefixlength, word, 8, ":");
				if (suppress_prefixlength) {
					websWrite(wp, suppress_prefixlength);
					return;
				}
			} else if (!strcmp(arg, "iif")) {
				GETENTRYBYIDX_DEL(iif, word, 9, ":");
				if (iif && !strcmp(argv[1], iif)) {
					websWrite(wp, "selected=\"selected\"");
					return;
				}
			} else if (!strcmp(arg, "oif")) {
				GETENTRYBYIDX_DEL(oif, word, 15, ":");
				if (oif && !strcmp(argv[1], oif)) {
					websWrite(wp, "selected=\"selected\"");
					return;
				}
			} else if (!strcmp(arg, "nat")) {
				GETENTRYBYIDX_DEL(nat, word, 10, ":");
				if (nat) {
					websWrite(wp, "%d", get_single_ip(nat, atoi(argv[1])));
					return;
				}
			} else if (!strcmp(arg, "type")) {
				GETENTRYBYIDX_DEL(type, word, 11, ":");
				if (type && !strcmp(argv[1], type)) {
					websWrite(wp, "selected=\"selected\"");
					return;
				}
			} else if (!strcmp(arg, "sport")) {
				GETENTRYBYIDX_DEL(sport, word, 13, ":");
				if (sport) {
					int from, to;
					sscanf(sport, "%d-%d", &from, &to);
					if (!strcmp(argv[1], "0"))
						websWrite(wp, "%d", from);
					else
						websWrite(wp, "%d", to);
					return;
				}
			} else if (!strcmp(arg, "dport")) {
				GETENTRYBYIDX_DEL(dport, word, 14, ":");
				if (dport) {
					int from, to;
					sscanf(dport, "%d-%d", &from, &to);
					if (!strcmp(argv[1], "0"))
						websWrite(wp, "%d", from);
					else
						websWrite(wp, "%d", to);
					return;
				}
			} else if (!strcmp(arg, "ipproto")) {
				GETENTRYBYIDX_DEL(ipproto, word, 12, ":");
				if (ipproto) {
					if (!strcmp(argv[1], ipproto))
						websWrite(wp, "selected=\"selected\"");
					return;
				}
			}
			break;
		}
	}

	if (!strcmp(arg, "nat") || !strcmp(arg, "from") || !strcmp(arg, "to"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "priority"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "tos"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "fwmark")) {
		if (!strcmp(argv[1], "0"))
			websWrite(wp, "0");
		else
			websWrite(wp, "0xffffffff");
	} else if (!strcmp(arg, "realms"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "type"))
		websWrite(wp, "unicast");
	else if (!strcmp(arg, "suppress_prefixlength"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "table"))
		websWrite(wp, "254");
	else if (!strcmp(arg, "sport"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "dport"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "ipproto"))
		websWrite(wp, "0");
	return;
}
#endif

EJ_VISIBLE void ej_static_route_table(webs_t wp, int argc, char_t **argv)
{
	int i, page, tmp = 0;
	int which;
	char *type;
	char word[256], *next;
	if (argc < 1)
		return;
	type = argv[0];

	page = websGetVari(wp, "route_page", 0); // default to 0

	if (!strcmp(type, "select")) {
		char *sroutename = nvram_safe_get("static_route_name");

		for (i = 0; i < STATIC_ROUTE_PAGE; i++) {
			char name[50] = " ";
			char new_name[80] = " ";
			char buf[80] = "";

			which = i;
			foreach(word, sroutename, next)
			{
				if (which-- == 0) {
					find_match_pattern(name, sizeof(name), word, "$NAME:", " ");
					httpd_filter_name(name, new_name, sizeof(new_name), GET);
					if (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-"))) {
						page = i;
					}
				}
			}
			snprintf(buf, sizeof(buf), "(%s)", new_name);

			websWrite(wp, "\t\t<option value=\"%d\" %s> %d %s</option>\n", i,
				  ((i == page) && !tmp) ? "selected=\"selected\"" : "", i + 1, buf);

			if (i == page)
				tmp = 1;
		}
	}

	return;
}

#ifndef HAVE_MICRO
EJ_VISIBLE void ej_pbr_rule_table(webs_t wp, int argc, char_t **argv)
{
	int i, page, tmp = 0;
	int which;
	char *type;
	char word[256], *next;
	if (argc < 1)
		return;
	type = argv[0];

	page = websGetVari(wp, "rule_page", 0); // default to 0

	if (!strcmp(type, "select")) {
		char *srulename = nvram_safe_get("pbr_rule_name");

		for (i = 0; i < STATIC_ROUTE_PAGE; i++) {
			char name[50] = " ";
			char new_name[80] = " ";
			char buf[80] = "";

			which = i;
			foreach(word, srulename, next)
			{
				if (which-- == 0) {
					find_match_pattern(name, sizeof(name), word, "$NAME:", " ");
					httpd_filter_name(name, new_name, sizeof(new_name), GET);
					if (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-"))) {
						page = i;
					}
				}
			}
			snprintf(buf, sizeof(buf), "(%s)", new_name);

			websWrite(wp, "\t\t<option value=\"%d\" %s> %d %s</option>\n", i,
				  ((i == page) && !tmp) ? "selected=\"selected\"" : "", i + 1, buf);

			if (i == page)
				tmp = 1;
		}
	}

	return;
}
#endif
