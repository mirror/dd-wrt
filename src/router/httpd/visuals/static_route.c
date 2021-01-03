/*
 * static_route.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <gottschall@dd-wrt.com>
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

EJ_VISIBLE void ej_show_routeif(webs_t wp, int argc, char_t ** argv)
{
	int which;
	char word[256];
	char *next = NULL;
	char ifnamecopy[32];
	char bufferif[512];

	which = websGetVari(wp, "route_page", 0);
	strcpy(ifnamecopy, "br0");

	char *sroute = nvram_safe_get("static_route");

	foreach(word, sroute, next) {
		if (which-- == 0) {
			GETENTRYBYIDX(ifname, word, 4);
			if (!ifname)
				break;
			strcpy(ifnamecopy, ifname);
		}
	}

	bzero(bufferif, 512);
	getIfList(bufferif, NULL);
	websWrite(wp, "<option value=\"lan\" %s >LAN &amp; WLAN</option>\n", nvram_match("lan_ifname", ifnamecopy) ? "selected=\"selected\"" : "");
	websWrite(wp, "<option value=\"wan\" %s >WAN</option>\n", nvram_match("wan_ifname", ifnamecopy) ? "selected=\"selected\"" : "");
	websWrite(wp, "<option value=\"any\" %s >ANY</option>\n", strcmp("any", ifnamecopy) == 0 ? "selected=\"selected\"" : "");
	bzero(word, 256);
	next = NULL;
	foreach(word, bufferif, next) {
		if (nvram_match("lan_ifname", word))
			continue;
		if (nvram_match("wan_ifname", word))
			continue;
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", word, strcmp(word, ifnamecopy) == 0 ? "selected=\"selected\"" : "", getNetworkLabel(wp, word));
	}
}

#ifndef HAVE_MICRO
EJ_VISIBLE void ej_show_ruleiif(webs_t wp, int argc, char_t ** argv)
{
	int which;
	char word[256];
	char *next = NULL;
	char ifnamecopy[32];
	char bufferif[512];

	which = websGetVari(wp, "rule_page", 0);
	strcpy(ifnamecopy, "br0");

	char *sroute = nvram_safe_get("pbr_rule");

	foreach(word, sroute, next) {
		if (which-- == 0) {
			GETENTRYBYIDX(ifname, word, 18);
			if (!ifname)
				break;
			strcpy(ifnamecopy, ifname);
		}
	}

	bzero(bufferif, 512);
	getIfList(bufferif, NULL);
	websWrite(wp, "<option value=\"lan\" %s >LAN &amp; WLAN</option>\n", nvram_match("lan_ifname", ifnamecopy) ? "selected=\"selected\"" : "");
	websWrite(wp, "<option value=\"wan\" %s >WAN</option>\n", nvram_match("wan_ifname", ifnamecopy) ? "selected=\"selected\"" : "");
	websWrite(wp, "<option value=\"any\" %s >ANY</option>\n", strcmp("any", ifnamecopy) == 0 ? "selected=\"selected\"" : "");
	bzero(word, 256);
	next = NULL;
	foreach(word, bufferif, next) {
		if (nvram_match("lan_ifname", word))
			continue;
		if (nvram_match("wan_ifname", word))
			continue;
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", word, strcmp(word, ifnamecopy) == 0 ? "selected=\"selected\"" : "", getNetworkLabel(wp, word));
	}
}
#endif
/*
 * Example: 
 * static_route=192.168.2.0:255.255.255.0:192.168.1.2:1:br0
 * <% static_route("ipaddr", 0); %> produces "192.168.2.0"
 * <% static_route("lan", 0); %> produces "selected" if nvram_match("lan_ifname", "br0")
 */
EJ_VISIBLE void ej_static_route_setting(webs_t wp, int argc, char_t ** argv)
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

		foreach(word, sroutename, next) {
			if (which-- == 0 || (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-")))) {
				find_match_pattern(name, sizeof(name), word, "$NAME:", "");
				httpd_filter_name(name, new_name, sizeof(new_name), GET);
				websWrite(wp, new_name);
				return;
			}

		}
	}
	char *sroute = nvram_safe_get("static_route");

	foreach(word, sroute, next) {
		//if (which-- == 0) {
		if (which-- == 0 || (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-")))) {
			GETENTRYBYIDX(ipaddr, word, 0);
			GETENTRYBYIDX(netmask, word, 1);
			GETENTRYBYIDX(gateway, word, 2);
			GETENTRYBYIDX(metric, word, 3);
			GETENTRYBYIDX(ifname, word, 4);
			GETENTRYBYIDX(nat, word, 5);
			GETENTRYBYIDX(src_en, word, 6);
			GETENTRYBYIDX(src, word, 7);
			GETENTRYBYIDX(scope_en, word, 8);
			GETENTRYBYIDX(scope, word, 9);
			GETENTRYBYIDX(table_en, word, 10);
			GETENTRYBYIDX(table, word, 11);
			GETENTRYBYIDX(mtu_en, word, 12);
			GETENTRYBYIDX(mtu, word, 13);
			GETENTRYBYIDX(advmss_en, word, 14);
			GETENTRYBYIDX(advmss, word, 15);
			if (!ipaddr || !netmask || !gateway || !metric || !ifname)
				continue;

			if (!strcmp(arg, "ipaddr")) {
				websWrite(wp, "%d", get_single_ip(ipaddr, atoi(argv[1])));
				return;
			} else if (!strcmp(arg, "netmask")) {
				websWrite(wp, "%d", get_single_ip(netmask, atoi(argv[1])));
				return;
			} else if (!strcmp(arg, "gateway")) {
				websWrite(wp, "%d", get_single_ip(gateway, atoi(argv[1])));
				return;
			} else if (!strcmp(arg, "metric")) {
				websWrite(wp, metric);
				return;
			} else if (!strcmp(arg, "nat")) {
				if (nat && !strcmp(nat, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "lan")
				   && nvram_match("lan_ifname", ifname)) {
				websWrite(wp, "selected=\"selected\"");
				return;
			} else if (!strcmp(arg, "wan")
				   && nvram_match("wan_ifname", ifname)) {
				websWrite(wp, "selected=\"selected\"");
				return;
			} else if (!strcmp(arg, "scope_en")) {
				if (scope_en && !strcmp(scope_en, "1"))
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(arg, "scope") && !strcmp(argv[1],scope)) {
				websWrite(wp, "selected=\"selected\"");
				return;
			} else if (!strcmp(arg, "mtu_en")) {
				if (mtu_en && !strcmp(mtu_en, "1"))
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(arg, "mtu")) {
				websWrite(wp, mtu);
				return;
			} else if (!strcmp(arg, "advmss_en")) {
				if (advmss_en && !strcmp(advmss_en, "1"))
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(arg, "advmss")) {
				websWrite(wp, advmss);
				return;
			} else if (!strcmp(arg, "table_en")) {
				if (table_en && !strcmp(table_en, "1"))
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(arg, "table")) {
				websWrite(wp, table);
				return;
			} else if (!strcmp(arg, "src")) {
				websWrite(wp, "%d", get_single_ip(src, atoi(argv[1])));
				return;
			}	
			return;
		}
	}

	if (!strcmp(arg, "ipaddr") || !strcmp(arg, "netmask") || !strcmp(arg, "src")
	    || !strcmp(arg, "gateway"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "metric"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "mtu"))
		websWrite(wp, "1500");
	else if (!strcmp(arg, "advmss"))
		websWrite(wp, "1460");
	else if (!strcmp(arg, "table"))
		websWrite(wp, "0");
	return;
}

#ifndef HAVE_MICRO
EJ_VISIBLE void ej_pbr_rule_setting(webs_t wp, int argc, char_t ** argv)
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

		foreach(word, sroutename, next) {
			if (which-- == 0 || (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-")))) {
				find_match_pattern(name, sizeof(name), word, "$NAME:", "");
				httpd_filter_name(name, new_name, sizeof(new_name), GET);
				websWrite(wp, new_name);
				return;
			}

		}
	}
	char *sroute = nvram_safe_get("pbr_rule");

	foreach(word, sroute, next) {
		//if (which-- == 0) {
		if (which-- == 0 || (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-")))) {
			GETENTRYBYIDX(not, word, 0);
			GETENTRYBYIDX(from_en, word, 1);
			GETENTRYBYIDX(from, word, 2);
			GETENTRYBYIDX(to_en, word, 3);
			GETENTRYBYIDX(to, word, 4);
			GETENTRYBYIDX(priority_en, word, 5);
			GETENTRYBYIDX(priority, word, 6);
			GETENTRYBYIDX(tos_en, word, 7);
			GETENTRYBYIDX(tos, word, 8);
			GETENTRYBYIDX(fwmark_en, word, 9);
			GETENTRYBYIDX(fwmark, word, 10);
			GETENTRYBYIDX(realms_en, word, 11);
			GETENTRYBYIDX(realms, word, 12);
			GETENTRYBYIDX(table_en, word, 13);
			GETENTRYBYIDX(table, word, 14);
			GETENTRYBYIDX(suppress_prefixlength_en, word, 15);
			GETENTRYBYIDX(suppress_prefixlength, word, 16);
			GETENTRYBYIDX(iif_en, word, 17);
			GETENTRYBYIDX(iif, word, 18);
			GETENTRYBYIDX(nat_en, word, 19);
			GETENTRYBYIDX(nat, word, 20);
			GETENTRYBYIDX(type_en, word, 21);
			GETENTRYBYIDX(type, word, 22);
			
			if (!strcmp(arg, "not")) {
				if (not && !strcmp(not, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "from_en")) {
				if (from_en && !strcmp(from_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "to_en")) {
				if (to_en && !strcmp(to_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "priority_en")) {
				if (priority_en && !strcmp(priority_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "tos_en")) {
				if (tos_en && !strcmp(tos_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "fwmark_en")) {
				if (fwmark_en && !strcmp(fwmark_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "realms_en")) {
				if (realms_en && !strcmp(realms_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "table_en")) {
				if (table_en && !strcmp(table_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "suppress_prefixlength_en")) {
				if (suppress_prefixlength_en && !strcmp(suppress_prefixlength_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "iif_en")) {
				if (iif_en && !strcmp(iif_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "nat_en")) {
				if (nat_en && !strcmp(nat_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "type_en")) {
				if (type_en && !strcmp(type_en, "1"))
					websWrite(wp, "checked=\"checked\"");
				return;
			} else if (!strcmp(arg, "from")) {
				websWrite(wp, "%d", get_single_ip(from, atoi(argv[1])));
				return;
			} else if (!strcmp(arg, "to")) {
				websWrite(wp, "%d", get_single_ip(to, atoi(argv[1])));
				return;
			} else if (!strcmp(arg, "priority")) {
				websWrite(wp, priority);
				return;
			} else if (!strcmp(arg, "tos")) {
				websWrite(wp, tos);
				return;
			} else if (!strcmp(arg, "fwmark")) {
				websWrite(wp, fwmark);
				return;
			} else if (!strcmp(arg, "realms")) {
				websWrite(wp, realms);
				return;
			} else if (!strcmp(arg, "table")) {
				websWrite(wp, table);
				return;
			} else if (!strcmp(arg, "suppress_prefixlength")) {
				websWrite(wp, suppress_prefixlength);
				return;
			} else if (!strcmp(arg, "iif")) {
				websWrite(wp, iif);
				return;
			} else if (!strcmp(arg, "nat")) {
				websWrite(wp, "%d", get_single_ip(nat, atoi(argv[1])));
				return;
			} else if (!strcmp(arg, "type")) {
				websWrite(wp, type);
				return;
			}
			return;
		}
	}

	if (!strcmp(arg, "nat") || !strcmp(arg, "from") || !strcmp(arg, "to"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "priority"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "tos"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "fwmark"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "realms"))
		websWrite(wp, "0");
	else if (!strcmp(arg, "type"))
		websWrite(wp, "unicast");
	else if (!strcmp(arg, "suppress_prefixlength"))
		websWrite(wp, "unicast");
	else if (!strcmp(arg, "table"))
		websWrite(wp, "254");
	return;
}
#endif

EJ_VISIBLE void ej_static_route_table(webs_t wp, int argc, char_t ** argv)
{
	int i, page, tmp = 0;
	int which;
	char *type;
	char word[256], *next;
	if (argc < 1)
		return;
	type = argv[0];

	page = websGetVari(wp, "route_page", 0);	// default to 0

	if (!strcmp(type, "select")) {
		char *sroutename = nvram_safe_get("static_route_name");

		for (i = 0; i < STATIC_ROUTE_PAGE; i++) {
			char name[50] = " ";
			char new_name[80] = " ";
			char buf[80] = "";

			which = i;
			foreach(word, sroutename, next) {
				if (which-- == 0) {
					find_match_pattern(name, sizeof(name), word, "$NAME:", " ");
					httpd_filter_name(name, new_name, sizeof(new_name), GET);
					if (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-"))) {
						page = i;
					}
				}
			}
			snprintf(buf, sizeof(buf), "(%s)", new_name);

			websWrite(wp, "\t\t<option value=\"%d\" %s> %d %s</option>\n", i, ((i == page) && !tmp) ? "selected=\"selected\"" : "", i + 1, buf);

			if (i == page)
				tmp = 1;
		}
	}

	return;
}
#ifndef HAVE_MICRO
EJ_VISIBLE void ej_pbr_rule_table(webs_t wp, int argc, char_t ** argv)
{
	int i, page, tmp = 0;
	int which;
	char *type;
	char word[256], *next;
	if (argc < 1)
		return;
	type = argv[0];

	page = websGetVari(wp, "rule_page", 0);	// default to 0

	if (!strcmp(type, "select")) {
		char *srulename = nvram_safe_get("pbr_rule_name");

		for (i = 0; i < STATIC_ROUTE_PAGE; i++) {
			char name[50] = " ";
			char new_name[80] = " ";
			char buf[80] = "";

			which = i;
			foreach(word, srulename, next) {
				if (which-- == 0) {
					find_match_pattern(name, sizeof(name), word, "$NAME:", " ");
					httpd_filter_name(name, new_name, sizeof(new_name), GET);
					if (next == NULL && !strcmp("", websGetVar(wp, "change_action", "-"))) {
						page = i;
					}
				}
			}
			snprintf(buf, sizeof(buf), "(%s)", new_name);

			websWrite(wp, "\t\t<option value=\"%d\" %s> %d %s</option>\n", i, ((i == page) && !tmp) ? "selected=\"selected\"" : "", i + 1, buf);

			if (i == page)
				tmp = 1;
		}
	}

	return;
}
#endif