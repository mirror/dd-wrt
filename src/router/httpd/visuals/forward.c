/*
 * forward.c
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

#include <broadcom.h>
#include <cy_conf.h>

#define NAME 0
#define ENABLE 1
#define SEL_TCP 2
#define SEL_UDP 3
#define SEL_BOTH 4
#define I_FROM 5
#define I_TO 6
#define O_FROM 7
#define O_TO 8
#define SRC 9
#define DEST 10
#define FROM 11
#define TO 12
#define UDP 13
#define TCP 14
#define IP 15

static int name_to_type(char *type)
{
	if (!strcmp(type, "name"))
		return NAME;
	if (!strcmp(type, "enable"))
		return ENABLE;
	if (!strcmp(type, "sel_tcp"))
		return SEL_TCP;
	if (!strcmp(type, "sel_udp"))
		return SEL_UDP;
	if (!strcmp(type, "sel_both"))
		return SEL_BOTH;
	if (!strcmp(type, "i_from"))
		return I_FROM;
	if (!strcmp(type, "i_to"))
		return I_TO;
	if (!strcmp(type, "o_from"))
		return O_FROM;
	if (!strcmp(type, "o_to"))
		return O_TO;
	if (!strcmp(type, "src"))
		return SRC;
	if (!strcmp(type, "dest"))
		return DEST;
	if (!strcmp(type, "from"))
		return FROM;
	if (!strcmp(type, "to"))
		return TO;
	if (!strcmp(type, "udp"))
		return UDP;
	if (!strcmp(type, "tcp"))
		return TCP;
	if (!strcmp(type, "ip"))
		return IP;
	return -1;
}

/*
 * Example: name:[on|off]:[tcp|udp|both]:8000:80>100 
 */

void port_forward_table(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char new_name[200];

	int t = name_to_type(type);
	wordlist = nvram_safe_get("forward_port");

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			switch (t) {
			case NAME: {
				GETENTRYBYIDX(name, word, 0);
				if (!name)
					continue;
				httpd_filter_name(name, new_name, sizeof(new_name), GET);
				websWrite(wp, "%s", new_name);
			} break;
			case FROM: {
				GETENTRYBYIDX(from, word, 3);
				if (!from)
					continue;
				websWrite(wp, "%s", from);
			} break;
			case TO: {
				GETENTRYBYIDX(to, word, 4);
				if (!to)
					continue;
				websWrite(wp, "%s", to);
			} break;
			case TCP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "udp"))
					websWrite(wp, "checked=\"checked\"");
			} break;
			case UDP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "tcp"))
					websWrite(wp, "checked=\"checked\"");
			} break;
			case SEL_TCP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "udp"))
					websWrite(wp, "selected=\"selected\"");
			} break;
			case SEL_UDP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "tcp"))
					websWrite(wp, "selected=\"selected\"");
			} break;
			case SEL_BOTH: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (!strcmp(proto, "both"))
					websWrite(wp, "selected=\\\"selected\\\"");
			} break;
			case IP: {
				GETENTRYBYIDX(ip, word, 5);
				if (!ip)
					continue;
				websWrite(wp, "%s", ip);
			} break;
			case ENABLE: {
				GETENTRYBYIDX(enable, word, 1);
				if (!enable)
					continue;
				if (!strcmp(enable, "on"))
					websWrite(wp, "checked=\"checked\"");
			} break;
			}
			return;
		}
	}
	switch (t) {
	case FROM:
	case TO:
		websWrite(wp, "0");
		break;
	case IP:
		websWrite(wp, "0.0.0.0");
		break;
	case SEL_BOTH:
		websWrite(wp, "selected=\\\"selected\\\"");
	}
}

void port_forward_spec(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char new_name[200];

	int t = name_to_type(type);
	wordlist = nvram_safe_get("forward_spec");

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			switch (t) {
			case NAME: {
				GETENTRYBYIDX(name, word, 0);
				if (!name)
					continue;
				httpd_filter_name(name, new_name, sizeof(new_name), GET);
				websWrite(wp, "%s", new_name);
			} break;
			case FROM: {
				GETENTRYBYIDX(from, word, 3);
				if (!from)
					continue;
				websWrite(wp, "%s", from);
			} break;
			case TO: {
				GETENTRYBYIDX(to, word, 5);
				if (!to)
					continue;
				websWrite(wp, "%s", to);
			} break;
			case TCP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "udp") && strcmp(proto, "both"))
					websWrite(wp, "checked=\"checked\"");
			} break;
			case UDP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "tcp") && strcmp(proto, "both"))
					websWrite(wp, "checked=\"checked\"");
			} break;
			case SEL_TCP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "udp") && strcmp(proto, "both"))
					websWrite(wp, "selected=\"selected\"");
			} break;
			case SEL_UDP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "tcp") && strcmp(proto, "both"))
					websWrite(wp, "selected=\"selected\"");
			} break;
			case SEL_BOTH: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (!strcmp(proto, "both"))
					websWrite(wp, "selected=\\\"selected\\\"");
			} break;
			case IP: {
				GETENTRYBYIDX(ip, word, 4);
				if (!ip)
					continue;
				websWrite(wp, "%s", ip);
			} break;
			case SRC: {
				GETENTRYBYIDX(src, word, 6);
				if (!src)
					continue;
				websWrite(wp, "%s", src == NULL ? "" : src);
			} break;
			case ENABLE: {
				GETENTRYBYIDX(enable, word, 1);
				if (!enable)
					continue;
				if (!strcmp(enable, "on"))
					websWrite(wp, "checked=\"checked\"");
			} break;
			}
			return;
		}
	}
	switch (t) {
	case FROM:
	case TO:
		websWrite(wp, "0");
		break;
	case IP:
		websWrite(wp, "0.0.0.0");
		break;
	case SEL_BOTH:
		websWrite(wp, "selected=\\\"selected\\\"");
	}
}

void ip_forward(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char new_name[200];
	int t = name_to_type(type);

	wordlist = nvram_safe_get("forward_ip");

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			switch (t) {
			case NAME: {
				GETENTRYBYIDX(name, word, 0);
				if (!name)
					continue;
				httpd_filter_name(name, new_name, sizeof(new_name), GET);
				websWrite(wp, "%s", new_name);
			} break;
			case SRC: {
				GETENTRYBYIDX(src, word, 2);
				if (!src)
					continue;
				websWrite(wp, "%s", src);
			} break;
			case DEST: {
				GETENTRYBYIDX(dest, word, 3);
				if (!dest)
					continue;
				websWrite(wp, "%s", dest);
			} break;
			case ENABLE: {
				GETENTRYBYIDX(enable, word, 1);
				if (!enable)
					continue;
				if (!strcmp(enable, "on"))
					websWrite(wp, "checked=\"checked\"");
			} break;
			}
			return;
		}
	}
}

/*
 * Example: name:on:both:1000-2000>3000-4000 
 */

void port_trigger_table(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char new_name[200];
	int t = name_to_type(type);
	wordlist = nvram_safe_get("port_trigger");

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			switch (t) {
			case NAME: {
				GETENTRYBYIDX(name, word, 0);
				if (!name)
					continue;
				if (strcmp(name, "")) {
					httpd_filter_name(name, new_name, sizeof(new_name), GET);
					websWrite(wp, "%s", new_name);
				}
			} break;
			case ENABLE: {
				GETENTRYBYIDX(enable, word, 1);
				if (!enable)
					continue;
				if (!strcmp(enable, "on"))
					websWrite(wp, "checked=\\\"checked\\\"");
			} break;
			case SEL_TCP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "udp") && strcmp(proto, "both"))
					websWrite(wp, "selected=\"selected\"");
			} break;
			case SEL_UDP: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (strcmp(proto, "tcp") && strcmp(proto, "both"))
					websWrite(wp, "selected=\"selected\"");
			} break;
			case SEL_BOTH: {
				GETENTRYBYIDX(proto, word, 2);
				if (!proto)
					continue;
				if (!strcmp(proto, "both"))
					websWrite(wp, "selected=\\\"selected\\\"");
			} break;
			case I_FROM: {
				GETENTRYBYIDX(i_from, word, 3);
				if (!i_from)
					continue;
				websWrite(wp, "%s", i_from);
			} break;
			case I_TO: {
				GETENTRYBYIDX(i_to, word, 4);
				if (!i_to)
					continue;
				websWrite(wp, "%s", i_to);
			} break;
			case O_FROM: {
				GETENTRYBYIDX(o_from, word, 5);
				if (!o_from)
					continue;
				websWrite(wp, "%s", o_from);
			} break;
			case O_TO: {
				GETENTRYBYIDX(o_to, word, 6);
				if (!o_to)
					continue;
				websWrite(wp, "%s", o_to);
			} break;
			default:
				return;
			}
			return;
		}
	}
	if (t != NAME)
		websWrite(wp, "0");
	return;
}
