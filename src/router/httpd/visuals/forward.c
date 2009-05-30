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
	static char word[256];
	char *next, *wordlist;
	char *name, *from, *to, *proto, *ip, *enable;
	static char new_name[200];
	int temp;

	wordlist = nvram_safe_get("forward_port");
	temp = which;

	foreach(word, wordlist, next) {
		if (which-- == 0) {
			enable = word;
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
			to = from;
			from = strsep(&to, ":");
			if (!to || !from)
				continue;
			ip = to;
			to = strsep(&ip, ">");
			if (!ip || !to)
				continue;

			if (!strcmp(type, "name")) {
				httpd_filter_name(name, new_name,
						  sizeof(new_name), GET);
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
					websWrite(wp,
						  "selected=\\\"selected\\\"");
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
	static char word[256];
	char *next, *wordlist;
	char *name, *from, *to, *proto, *ip, *enable;
	static char new_name[200];
	int temp;

	wordlist = nvram_safe_get("forward_spec");
	temp = which;

	foreach(word, wordlist, next) {
		if (which-- == 0) {
			enable = word;
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

			if (!strcmp(type, "name")) {
				httpd_filter_name(name, new_name,
						  sizeof(new_name), GET);
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
					websWrite(wp,
						  "selected=\\\"selected\\\"");
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

/*
 * Example: name:on:both:1000-2000>3000-4000 
 */

void port_trigger_table(webs_t wp, char *type, int which)
{

	static char word[256];
	char *next, *wordlist;
	char *name = NULL, *enable = NULL, *proto = NULL, *i_from =
	    NULL, *i_to = NULL, *o_from = NULL, *o_to = NULL;
	static char new_name[200];
	int temp;

	wordlist = nvram_safe_get("port_trigger");
	temp = which;

	foreach(word, wordlist, next) {
		if (which-- == 0) {
			enable = word;
			name = strsep(&enable, ":");
			if (!name || !enable)
				continue;
			proto = enable;
			enable = strsep(&proto, ":");
			if (!enable || !proto)
				continue;
			i_from = proto;
			proto = strsep(&i_from, ":");
			if (!proto || !i_from)
				continue;
			i_to = i_from;
			i_from = strsep(&i_to, "-");
			if (!i_from || !i_to)
				continue;
			o_from = i_to;
			i_to = strsep(&o_from, ">");
			if (!i_to || !o_from)
				continue;
			o_to = o_from;
			o_from = strsep(&o_to, "-");
			if (!o_from || !o_to)
				continue;

			if (!strcmp(type, "name")) {
				if (strcmp(name, "")) {
					httpd_filter_name(name, new_name,
							  sizeof(new_name),
							  GET);
					websWrite(wp, "%s", new_name);
				}
			} else if (!strcmp(type, "enable")) {
				if (!strcmp(enable, "on"))
					websWrite(wp,
						  "checked=\\\"checked\\\"");
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
					websWrite(wp,
						  "selected=\\\"selected\\\"");
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
