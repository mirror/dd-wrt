/*
 * forward.c
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

#include <broadcom.h>
#include <cy_conf.h>

/*
 * Example: name:[on|off]:[tcp|udp|both]:8000:80>100 
 */

void port_forward_table(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char new_name[200];
	int temp;

	wordlist = nvram_safe_get("forward_port");
	temp = which;

	foreach(word, wordlist, next) {
		if (which-- == 0) {
			GETENTRYBYIDX(name, word, 0);
			GETENTRYBYIDX(enable, word, 1);
			GETENTRYBYIDX(proto, word, 2);
			GETENTRYBYIDX(from, word, 3);
			GETENTRYBYIDX(to, word, 4);
			GETENTRYBYIDX(ip, word, 5);
			if (!name || !enable || !proto || !from || !to || !ip)
				continue;

			if (!strcmp(type, "name")) {
				httpd_filter_name(name, new_name, sizeof(new_name), GET);
				websWrite(wp, "%s", new_name);
			} else if (!strcmp(type, "from"))
				websWrite(wp, "%s", from);
			else if (!strcmp(type, "to"))
				websWrite(wp, "%s", to);
			else if (!strcmp(type, "tcp")) {	// use checkbox
				if (!strcmp(proto, "udp"))
					websWrite(wp, "");
				else
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(type, "udp")) {	// use checkbox
				if (!strcmp(proto, "tcp"))
					websWrite(wp, "");
				else
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(type, "sel_tcp")) {	// use select
				if (!strcmp(proto, "udp"))
					websWrite(wp, "");
				else
					websWrite(wp, "selected=\"selected\"");
			} else if (!strcmp(type, "sel_udp")) {	// use select
				if (!strcmp(proto, "tcp"))
					websWrite(wp, "");
				else
					websWrite(wp, "selected=\"selected\"");
			} else if (!strcmp(type, "sel_both")) {	// use select
				if (!strcmp(proto, "both"))
					websWrite(wp, "selected=\\\"selected\\\"");
				else
					websWrite(wp, "");
			} else if (!strcmp(type, "ip"))
				websWrite(wp, "%s", ip);
			else if (!strcmp(type, "enable")) {
				if (!strcmp(enable, "on"))
					websWrite(wp, "checked=\"checked\"");
				else
					websWrite(wp, "");
			}
			return;
		}
	}
	if (!strcmp(type, "from") || !strcmp(type, "to"))
		websWrite(wp, "0");
	else if (!strcmp(type, "ip"))
		websWrite(wp, "0.0.0.0");
	else if (!strcmp(type, "sel_both"))
		websWrite(wp, "selected=\\\"selected\\\"");
	else
		websWrite(wp, "");
}

void port_forward_spec(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char new_name[200];
	int temp;

	wordlist = nvram_safe_get("forward_spec");
	temp = which;

	foreach(word, wordlist, next) {
		if (which-- == 0) {
			GETENTRYBYIDX(name, word, 0);
			GETENTRYBYIDX(enable, word, 1);
			GETENTRYBYIDX(proto, word, 2);
			GETENTRYBYIDX(from, word, 3);
			GETENTRYBYIDX(ip, word, 4);
			GETENTRYBYIDX(to, word, 5);
			GETENTRYBYIDX(src, word, 6);
			if (!name || !enable || !proto || !from || !to || !ip)
				continue;

			if (!strcmp(type, "name")) {
				httpd_filter_name(name, new_name, sizeof(new_name), GET);
				websWrite(wp, "%s", new_name);
			} else if (!strcmp(type, "from"))
				websWrite(wp, "%s", from);
			else if (!strcmp(type, "to"))
				websWrite(wp, "%s", to);
			else if (!strcmp(type, "tcp")) {	// use checkbox
				if (!strcmp(proto, "udp")
				    || !strcmp(proto, "both"))
					websWrite(wp, "");
				else
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(type, "udp")) {	// use checkbox
				if (!strcmp(proto, "tcp")
				    || !strcmp(proto, "both"))
					websWrite(wp, "");
				else
					websWrite(wp, "checked=\"checked\"");
			} else if (!strcmp(type, "sel_tcp")) {	// use select
				if (!strcmp(proto, "udp")
				    || !strcmp(proto, "both"))
					websWrite(wp, "");
				else
					websWrite(wp, "selected=\"selected\"");
			} else if (!strcmp(type, "sel_udp")) {	// use select
				if (!strcmp(proto, "tcp")
				    || !strcmp(proto, "both"))
					websWrite(wp, "");
				else
					websWrite(wp, "selected=\"selected\"");
			} else if (!strcmp(type, "sel_both")) {	// use select
				if (!strcmp(proto, "both"))
					websWrite(wp, "selected=\\\"selected\\\"");
				else
					websWrite(wp, "");
			} else if (!strcmp(type, "ip"))
				websWrite(wp, "%s", ip);
			else if (!strcmp(type, "src"))
				websWrite(wp, "%s", src == NULL ? "" : src);
			else if (!strcmp(type, "enable")) {
				if (!strcmp(enable, "on"))
					websWrite(wp, "checked=\"checked\"");
				else
					websWrite(wp, "");
			}
			return;
		}
	}
	if (!strcmp(type, "from") || !strcmp(type, "to"))
		websWrite(wp, "0");
	else if (!strcmp(type, "ip"))
		websWrite(wp, "0.0.0.0");
	else if (!strcmp(type, "src"))
		websWrite(wp, "");
	else if (!strcmp(type, "sel_both"))
		websWrite(wp, "selected=\\\"selected\\\"");
	else
		websWrite(wp, "");
}

/*
 * Example: name:on:both:1000-2000>3000-4000 
 */

void port_trigger_table(webs_t wp, char *type, int which)
{

	char word[256];
	char *next, *wordlist;
	char new_name[200];
	int temp;

	wordlist = nvram_safe_get("port_trigger");
	temp = which;

	foreach(word, wordlist, next) {
		if (which-- == 0) {
			GETENTRYBYIDX(name, word, 0);
			GETENTRYBYIDX(enable, word, 1);
			GETENTRYBYIDX(proto, word, 2);
			GETENTRYBYIDX(i_from, word, 3);
			GETENTRYBYIDX(i_to, word, 4);
			GETENTRYBYIDX(o_from, word, 5);
			GETENTRYBYIDX(o_to, word, 6);
			if (!name || !enable || !proto || !i_from || !i_to || !o_from || !o_to)
				continue;

			if (!strcmp(type, "name")) {
				if (strcmp(name, "")) {
					httpd_filter_name(name, new_name, sizeof(new_name), GET);
					websWrite(wp, "%s", new_name);
				}
			} else if (!strcmp(type, "enable")) {
				if (!strcmp(enable, "on"))
					websWrite(wp, "checked=\\\"checked\\\"");
				else
					websWrite(wp, "");
			} else if (!strcmp(type, "sel_tcp")) {	// use select
				if (!strcmp(proto, "udp")
				    || !strcmp(proto, "both"))
					websWrite(wp, "");
				else
					websWrite(wp, "selected=\"selected\"");
			} else if (!strcmp(type, "sel_udp")) {	// use select
				if (!strcmp(proto, "tcp")
				    || !strcmp(proto, "both"))
					websWrite(wp, "");
				else
					websWrite(wp, "selected=\"selected\"");
			} else if (!strcmp(type, "sel_both")) {	// use select
				if (!strcmp(proto, "both"))
					websWrite(wp, "selected=\\\"selected\\\"");
				else
					websWrite(wp, "");
			} else if (!strcmp(type, "i_from"))
				websWrite(wp, "%s", i_from);
			else if (!strcmp(type, "i_to"))
				websWrite(wp, "%s", i_to);
			else if (!strcmp(type, "o_from"))
				websWrite(wp, "%s", o_from);
			else if (!strcmp(type, "o_to"))
				websWrite(wp, "%s", o_to);
			return;
		}
	}
	if (!strcmp(type, "name"))
		websWrite(wp, "");
	else
		websWrite(wp, "0");
	return;
}
