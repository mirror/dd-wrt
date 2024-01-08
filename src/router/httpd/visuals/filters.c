/*
 * filters.c
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
#include <string.h>
#include <signal.h>

#include <broadcom.h>

/*
 * Format: filter_rule{1...10}=$STAT:1$NAME:test1$ (1=>disable 2=>enable)
 * 
 * Format: filter_tod{1...10} = hr:min hr:min wday filter_tod_buf{1...10} =
 * sun mon tue wed thu fri sat //only for web page read Example: Everyday and 
 * 24-hour filter_todXX = 0:0 23:59 0-0 filter_tod_bufXX = 7 (for web)
 * 
 * From 9:55 to 22:00 every sun, wed and thu filter_todXX = 9:55 22:00 0,3-4
 * filter_tod_bufXX = 1 0 1 1 0 0 0 (for web)
 * 
 * Format: filter_ip_grp{1...10} = ip1 ip2 ip3 ip4 ip5 ip6 ip_r1-ipr2
 * ip_r3-ip_r4 filter_ip_mac{1...10} = 00:11:22:33:44:55 00:12:34:56:78:90
 * 
 * Format: filter_port=udp:111-222 both:22-33 disable:22-333 tcp:11-22222
 * 
 * Converting Between AM/PM and 24 Hour Clock: Converting from AM/PM to 24
 * hour clock: 12:59 AM -> 0059 (between 12:00 AM and 12:59 AM, subtract 12
 * hours) 10:00 AM -> 1000 (between 1:00 AM and 12:59 PM, a straight
 * conversion) 10:59 PM -> 2259 (between 1:00 PM and 11:59 PM, add 12 hours)
 * Converting from 24 hour clock to AM/PM 0059 -> 12:59 AM (between 0000 and
 * 0059, add 12 hours) 0100 -> 1:00 AM (between 0100 and 1159, straight
 * converion to AM) 1259 -> 12:59 PM (between 1200 and 1259, straight
 * converion to PM) 1559 -> 3:59 PM (between 1300 and 2359, subtract 12
 * hours)
 * 
 */

/*
 * Example: 100-200 250-260 (ie. 192.168.1.100-192.168.1.200
 * 192.168.1.250-192.168.1.260) 
 */

static char *filter_ip_get(webs_t wp, char *type, int which, char *word, char *tgt, size_t len)
{
	char *start, *end, *wordlist, *next;
	char filter_ip[] = "filter_ip_grpXXX";
	int temp = which;
	snprintf(filter_ip, sizeof(filter_ip), "filter_ip_grp%d", wp->p->filter_id);

	wordlist = nvram_safe_get(filter_ip);
	if (!wordlist)
		return "0";

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			if (temp == 6) {
				end = word;
				start = strsep(&end, "-");
				int isip1, isip2, isip3, isip4;
				if (sscanf(start, "%d.%d.%d.%d", &isip1, &isip2, &isip3, &isip4) != 4) {
					sscanf(start, "%d", &isip4);
					if (isip4 != 0)
						sscanf(nvram_safe_get("lan_ipaddr"), "%d.%d.%d", &isip1, &isip2, &isip3);
					else {
						isip1 = 0;
						isip2 = 0;
						isip3 = 0;
					}
				}
				if (!strcmp(type, "ip_range0_0")) {
					snprintf(tgt, len, "%d", isip1);
					return tgt;
				}
				if (!strcmp(type, "ip_range0_1")) {
					snprintf(tgt, len, "%d", isip2);
					return tgt;
				}
				if (!strcmp(type, "ip_range0_2")) {
					snprintf(tgt, len, "%d", isip3);
					return tgt;
				}
				if (!strcmp(type, "ip_range0_3")) {
					snprintf(tgt, len, "%d", isip4);
					return tgt;
				}
				int ieip1, ieip2, ieip3, ieip4;
				if (sscanf(end, "%d.%d.%d.%d", &ieip1, &ieip2, &ieip3, &ieip4) != 4) {
					sscanf(start, "%d", &ieip4);
					if (ieip4 != 0)
						sscanf(nvram_safe_get("lan_ipaddr"), "%d.%d.%d", &ieip1, &ieip2, &ieip3);
					else {
						ieip1 = 0;
						ieip2 = 0;
						ieip3 = 0;
					}
				}

				if (!strcmp(type, "ip_range0_4")) {
					snprintf(tgt, len, "%d", ieip1);
					return tgt;
				}
				if (!strcmp(type, "ip_range0_5")) {
					snprintf(tgt, len, "%d", ieip2);
					return tgt;
				}
				if (!strcmp(type, "ip_range0_6")) {
					snprintf(tgt, len, "%d", ieip3);
					return tgt;
				}
				if (!strcmp(type, "ip_range0_7")) {
					snprintf(tgt, len, "%d", ieip4);
					return tgt;
				}

			} else if (temp == 7) {
				end = word;
				start = strsep(&end, "-");
				int isip1, isip2, isip3, isip4;
				if (sscanf(start, "%d.%d.%d.%d", &isip1, &isip2, &isip3, &isip4) != 4) {
					sscanf(start, "%d", &isip4);
					if (isip4 != 0)
						sscanf(nvram_safe_get("lan_ipaddr"), "%d.%d.%d", &isip1, &isip2, &isip3);
					else {
						isip1 = 0;
						isip2 = 0;
						isip3 = 0;
					}
				}

				if (!strcmp(type, "ip_range1_0")) {
					snprintf(tgt, len, "%d", isip1);
					return tgt;
				}
				if (!strcmp(type, "ip_range1_1")) {
					snprintf(tgt, len, "%d", isip2);
					return tgt;
				}
				if (!strcmp(type, "ip_range1_2")) {
					snprintf(tgt, len, "%d", isip3);
					return tgt;
				}
				if (!strcmp(type, "ip_range1_3")) {
					snprintf(tgt, len, "%d", isip4);
					return tgt;
				}

				int ieip1, ieip2, ieip3, ieip4;
				if (sscanf(end, "%d.%d.%d.%d", &ieip1, &ieip2, &ieip3, &ieip4) != 4) {
					sscanf(start, "%d", &ieip4);
					if (ieip4 != 0)
						sscanf(nvram_safe_get("lan_ipaddr"), "%d.%d.%d", &ieip1, &ieip2, &ieip3);
					else {
						ieip1 = 0;
						ieip2 = 0;
						ieip3 = 0;
					}
				}

				if (!strcmp(type, "ip_range1_4")) {
					snprintf(tgt, len, "%d", ieip1);
					return tgt;
				}
				if (!strcmp(type, "ip_range1_5")) {
					snprintf(tgt, len, "%d", ieip2);
					return tgt;
				}
				if (!strcmp(type, "ip_range1_6")) {
					snprintf(tgt, len, "%d", ieip3);
					return tgt;
				}
				if (!strcmp(type, "ip_range1_7")) {
					snprintf(tgt, len, "%d", ieip4);
					return tgt;
				}
			}
			return word;
		}
	}
	return "0";
}

static char *filter_port_get(char *list, char *type, int which, char *buf, size_t len)
{
	char *wordlist, *next;
	char word[256];
	char *start, *end, *proto;
	char *protos[] = { "disable", "both", "tcp", "udp", "l7" };

	wordlist = nvram_safe_get(list);
	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			start = word;
			proto = strsep(&start, ":");
			end = start;
			start = strsep(&end, "-");
			int i;
			for (i = 0; i < sizeof(protos) / sizeof(char *); i++) {
				if (!strcmp(type, protos[i])) {
					if (!strcmp(proto, protos[i]))
						return "selected";
					else
						return " ";
				}
			}
			if (!strcmp(type, "start")) {
				strncpy(buf, len, start);
				return buf;
			}
			if (!strcmp(type, "end")) {
				strncpy(buf, len, end);
				return buf;
			}
		}
	}
	if (!strcmp(type, "start") || !strcmp(type, "end"))
		return "0";
	else
		return "";
}

EJ_VISIBLE void ej_filter_dport_get(webs_t wp, int argc, char_t **argv)
{
	int which;
	char *type;
	char name[] = "filter_dport_grpXXX";
	char buf[64];
	sprintf(name, "filter_dport_grp%d", wp->p->filter_id);

	type = argv[0];
	which = atoi(argv[1]);
	websWrite(wp, "%s", filter_port_get(name, type, which, buf, sizeof(buf)));

	return;
}

EJ_VISIBLE void ej_filter_port_get(webs_t wp, int argc, char_t **argv)
{
	int which;
	char *type;
	char buf[64];
	type = argv[0];
	which = atoi(argv[1]);
	websWrite(wp, "%s", filter_port_get(type, "filter_port", which, buf, sizeof(buf)));

	return;
}

/*
 * Example: 00:11:22:33:44:55 00:11:22:33:44:56 
 */

static char *filter_mac_get(webs_t wp, int which, char *word)
{
	char *wordlist, *next;
	char *mac;
	char filter_mac[] = "filter_mac_grpXXX";

	snprintf(filter_mac, sizeof(filter_mac), "filter_mac_grp%d", wp->p->filter_id);

	wordlist = nvram_safe_get(filter_mac);
	if (!wordlist)
		return "";

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			mac = word;
			return mac;
		}
	}
	return "00:00:00:00:00:00";
}

EJ_VISIBLE void ej_filter_ip_get(webs_t wp, int argc, char_t **argv)
{
	int which;
	char *type;
	char word[256];
	char buf[64];

	type = argv[0];
	which = atoi(argv[1]);
	websWrite(wp, "%s", filter_ip_get(wp, type, which, word, buf, sizeof(buf)));

	return;
}

EJ_VISIBLE void ej_filter_mac_get(webs_t wp, int argc, char_t **argv)
{
	int which;
	char word[256];

	which = atoi(argv[0]);
	websWrite(wp, "%s", filter_mac_get(wp, which, word));
	return;
}

EJ_VISIBLE void ej_filter_policy_select(webs_t wp, int argc, char_t **argv)
{
	int i;
	for (i = 1; i <= NR_RULES; i++) {
		char filter[] = "filter_ruleXXX";
		char *data = "";
		char name[50] = "";

		snprintf(filter, sizeof(filter), "filter_rule%d", i);
		data = nvram_safe_get(filter);

		if (data && strcmp(data, "")) {
			find_match_pattern(name, sizeof(name), data, "$NAME:", ""); // get
			// name
			// value
		}
		websWrite(wp, "<option value=%d %s>%d ( %s ) </option>\n", i,
			  (wp->p->filter_id == i ? "selected=\"selected\" " : ""), i, name);
	}
	return;
}

EJ_VISIBLE void ej_show_filterif(webs_t wp, int argc, char_t **argv)
{
	char wan_if_buffer[33];
	if (argc < 1)
		return;
	char *ifname = argv[0];
	char ifs[50] = "";
	char filter[] = "filter_ruleXXX";
	char *data = "";

	snprintf(filter, sizeof(filter), "filter_rule%d", wp->p->filter_id);
	data = nvram_safe_get(filter);
	find_match_pattern(ifs, sizeof(ifs), data, "$IF:", ""); // get

	websWrite(wp, "<select name=\"%s\">\n", ifname);
	int i;
	for (i = 1; i < argc; i++) {
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", argv[i],
			  nvram_match(ifname, argv[i]) ? "selected=\"selected\"" : "", argv[i]);
	}
	char *wanface = safe_get_wan_face(wan_if_buffer);
	websWrite(wp, "<option value=\"%s\" %s >LAN</option>\n", nvram_safe_get("lan_ifname"),
		  !strcmp(ifs, nvram_safe_get("lan_ifname")) ? "selected=\"selected\"" : "");
	char *next;
	char var[80];
	char eths[256];
	char eth2[256];
	bzero(eths, 256);
	getIfLists(eths, 256);
	bzero(eth2, 256);
	getIfListNoPorts(eth2, "ppp");
	strcat(eths, " ");
	strcat(eths, eth2);
	foreach(var, eths, next)
	{
		if (!strcmp(wanface, var))
			continue;
		if (!strcmp(nvram_safe_get("lan_ifname"), var))
			continue;
		if (nvram_nmatch("1", "%s_bridged", var) && !isbridge(var))
			continue;
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, !strcmp(ifs, var) ? "selected=\"selected\"" : "",
			  getNetworkLabel(wp, var));
	}

	websWrite(wp, "</select>\n");
}

unsigned long long getpackettotal(char *table, char *chain);

EJ_VISIBLE void ej_filter_getpacketcount(webs_t wp, int argc, char_t **argv)
{
	unsigned long long count;
	char grp[32];
	sprintf(grp, "advgrp_%d", wp->p->filter_id);
	count = getpackettotal("filter", grp);
	websWrite(wp, "%lld", count);
}

EJ_VISIBLE void ej_filter_policy_get(webs_t wp, int argc, char_t **argv)
{
	char *type, *part;

	type = argv[0];
	part = argv[1];

	if (!strcmp(type, "f_id")) {
		websWrite(wp, "%d", wp->p->filter_id);
	} else if (!strcmp(type, "f_name")) {
		char name[50] = "";
		char filter[] = "filter_ruleXXX";
		char *data = "";

		snprintf(filter, sizeof(filter), "filter_rule%d", wp->p->filter_id);
		data = nvram_safe_get(filter);
		if (strcmp(data, "")) {
			find_match_pattern(name, sizeof(name), data, "$NAME:", ""); // get
			// name
			// value
			websWrite(wp, "%s", name);
		}
	} else if (!strcmp(type, "f_status")) {
		char status[50] = "", deny[50] = "";
		char filter[] = "filter_ruleXXX";
		char *data = "";

		snprintf(filter, sizeof(filter), "filter_rule%d", wp->p->filter_id);
		data = nvram_safe_get(filter);
		if (strcmp(data, "")) { // have data
			find_match_pattern(status, sizeof(status), data, "$STAT:", "1"); // get
			// status
			// value
			find_match_pattern(deny, sizeof(deny), data, "$DENY:", ""); // get
			// deny
			// value
			if (!strcmp(deny, "")) { // old format
				if (!strcmp(status, "0") || !strcmp(status, "1"))
					strcpy(deny, "1"); // Deny
				else
					strcpy(deny, "0"); // Allow
			}
#if 0
			if (!strcmp(part, "disable")) {
				if (!strcmp(status, "1"))
					websWrite(wp, "checked=\"checked\" ");
			} else if (!strcmp(part, "enable")) {
				if (!strcmp(status, "2"))
					websWrite(wp, "checked=\"checked\" ");
			}
#endif
			if (!strcmp(part, "disable")) {
				if (!strcmp(status, "0"))
					websWrite(wp, "checked=\"checked\" ");
			} else if (!strcmp(part, "enable")) {
				if (strcmp(status, "0"))
					websWrite(wp, "checked=\"checked\" ");
			} else if (!strcmp(part, "deny")) {
				if (!strcmp(deny, "1"))
					websWrite(wp, "checked=\"checked\" ");
			} else if (!strcmp(part, "allow")) {
				if (!strcmp(deny, "0"))
					websWrite(wp, "checked=\"checked\" ");
			} else if (!strcmp(part, "onload_status")) {
				if (!strcmp(deny, "1"))
					websWrite(wp, "deny");
				else
					websWrite(wp, "allow");

			} else if (!strcmp(part, "init")) {
				if (!strcmp(status, "1"))
					websWrite(wp, "disable");
				else if (!strcmp(status, "2"))
					websWrite(wp, "enable");
				else
					websWrite(wp, "disable");
			}
		} else { // no data
			if (!strcmp(part, "disable"))
				websWrite(wp, "checked=\"checked\" ");
			else if (!strcmp(part,
					 "allow")) // default policy is allow,
				// 2003-10-21
				websWrite(wp, "checked=\"checked\" ");
			else if (!strcmp(part,
					 "onload_status")) // default policy is
				// allow, 2003-10-21
				websWrite(wp, "allow");
			else if (!strcmp(part, "init"))
				websWrite(wp, "disable");
		}
	}
	return;
}

int filter_tod_init(webs_t wp, int which)
{
	int ret;
	char *tod_data, *tod_buf_data;
	char filter_tod[] = "filter_todXXX";
	char filter_tod_buf[] = "filter_tod_bufXXX";
	char temp[3][20];

	wp->p->tod_data_null = 0;
	wp->p->day_all = wp->p->week0 = wp->p->week1 = wp->p->week2 = wp->p->week3 = wp->p->week4 = wp->p->week5 = wp->p->week6 = 0;
	wp->p->time_all = wp->p->start_hour = wp->p->start_min = wp->p->start_time = wp->p->end_hour = wp->p->end_min =
		wp->p->end_time = 0;
	wp->p->start_week = wp->p->end_week = 0;
	snprintf(filter_tod, sizeof(filter_tod), "filter_tod%d", which);
	snprintf(filter_tod_buf, sizeof(filter_tod_buf), "filter_tod_buf%d", which);

	/*
	 * Parse filter_tod{1...10} 
	 */
	tod_data = nvram_safe_get(filter_tod);
	if (!tod_data)
		return -1; // no data
	if (strcmp(tod_data, "")) {
		sscanf(tod_data, "%s %s %s", temp[0], temp[1], temp[2]);
		sscanf(temp[0], "%d:%d", &wp->p->start_hour, &wp->p->start_min);
		sscanf(temp[1], "%d:%d", &wp->p->end_hour, &wp->p->end_min);
		ret = sscanf(temp[2], "%d-%d", &wp->p->start_week, &wp->p->end_week);
		if (ret == 1)
			wp->p->end_week = wp->p->start_week;

		if (wp->p->start_hour == 0 && wp->p->start_min == 0 && wp->p->end_hour == 23 && wp->p->end_min == 59) { // 24 Hours
			wp->p->time_all = 1;
			wp->p->start_hour = wp->p->end_hour = 0;
			wp->p->start_min = wp->p->start_time = wp->p->end_min = wp->p->end_time = 0;
		}
		/*
		 * else { // check AM or PM time_all = 0; if (start_hour > 11) {
		 * start_hour = start_hour - 12; start_time = 1; } if (end_hour > 11)
		 * { end_hour = end_hour - 12; end_time = 1; } } 
		 */
	} else { // default Everyday and 24 Hours
		wp->p->tod_data_null = 1;
		wp->p->day_all = 1;
		wp->p->time_all = 1;
	}

	if (wp->p->tod_data_null == 0) {
		/*
		 * Parse filter_tod_buf{1...10} 
		 */
		tod_buf_data = nvram_safe_get(filter_tod_buf);
		if (!strcmp(tod_buf_data, "7")) {
			wp->p->day_all = 1;
		} else if (strcmp(tod_buf_data, "")) {
			sscanf(tod_buf_data, "%d %d %d %d %d %d %d", &wp->p->week0, &wp->p->week1, &wp->p->week2, &wp->p->week3,
			       &wp->p->week4, &wp->p->week5, &wp->p->week6);
			wp->p->day_all = 0;
		}
	}
	return 0;
}

EJ_VISIBLE void ej_filter_tod_get(webs_t wp, int argc, char_t **argv)
{
	char *type;
	int i;
	type = argv[0];

	filter_tod_init(wp, wp->p->filter_id);

	if (!strcmp(type, "day_all_init")) {
		if (wp->p->day_all == 0)
			websWrite(wp, "1");
		else
			websWrite(wp, "0");
	} else if (!strcmp(type, "time_all_init")) {
		if (wp->p->time_all == 0)
			websWrite(wp, "1");
		else
			websWrite(wp, "0");
	} else if (!strcmp(type, "day_all")) {
		websWrite(wp, "%s", wp->p->day_all == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "start_week")) {
		websWrite(wp, "%d", wp->p->start_week);
	} else if (!strcmp(type, "end_week")) {
		websWrite(wp, "%d", wp->p->end_week);
	} else if (!strcmp(type, "week0")) { // Sun
		websWrite(wp, "%s", wp->p->week0 == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "week1")) { // Mon
		websWrite(wp, "%s", wp->p->week1 == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "week2")) { // Tue
		websWrite(wp, "%s", wp->p->week2 == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "week3")) { // Wed
		websWrite(wp, "%s", wp->p->week3 == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "week4")) { // Thu
		websWrite(wp, "%s", wp->p->week4 == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "week5")) { // Fri
		websWrite(wp, "%s", wp->p->week5 == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "week6")) { // Sat
		websWrite(wp, "%s", wp->p->week6 == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "time_all_en")) { // for linksys
		websWrite(wp, "%s", wp->p->time_all == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "time_all_dis")) { // for linksys
		websWrite(wp, "%s", wp->p->time_all == 0 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "time_all")) {
		websWrite(wp, "%s", wp->p->time_all == 1 ? "checked=\"checked\" " : "");
	} else if (!strcmp(type, "start_hour_24")) { // 00 -> 23
		for (i = 0; i < 24; i++) {
			websWrite(wp, "<option value=%d %s>%d</option>\n", i,
				  i == wp->p->start_hour ? "selected=\"selected\" " : "", i);
		}
	} else if (!strcmp(type, "start_min_1")) { // 0 1 2 3 4 .... -> 58 59
		for (i = 0; i < 60; i++) {
			websWrite(wp, "<option value=%02d %s>%02d</option>\n", i,
				  i == wp->p->start_min ? "selected=\"selected\" " : "", i);
		}
	} else if (!strcmp(type, "end_hour_24")) { // 00 ->23
		for (i = 0; i < 24; i++) {
			websWrite(wp, "<option value=%d %s>%d</option>\n", i, i == wp->p->end_hour ? "selected=\"selected\" " : "",
				  i);
		}
	} else if (!strcmp(type, "end_min_1")) { // 0 1 2 3 4 .... -> 58 59
		for (i = 0; i < 60; i++) {
			websWrite(wp, "<option value=%02d %s>%02d</option>\n", i,
				  i == wp->p->end_min ? "selected=\"selected\" " : "", i);
		}
	}
	return;
}

/*
 * Format: url0, url1, url2, url3, ....  keywd0, keywd1, keywd2, keywd3,
 * keywd4, keywd5, .... 
 */
EJ_VISIBLE void ej_filter_web_get(webs_t wp, int argc, char_t **argv)
{
	char *type;
	int which;
	char *token = "<&nbsp;>";

	type = argv[0];
	which = atoi(argv[1]);

	if (!strcmp(type, "host")) {
		char *host_data, filter_host[] = "filter_web_hostXXX";
		;
		char host[80];

		snprintf(filter_host, sizeof(filter_host), "filter_web_host%d", wp->p->filter_id);
		host_data = nvram_safe_get(filter_host);
		if (!strcmp(host_data, ""))
			return; // no data
		find_each(host, sizeof(host), host_data, token, which, "");
		websWrite(wp, "%s", host);
	} else if (!strcmp(type, "url")) {
		char *url_data, filter_url[] = "filter_web_urlXXX";
		char url[80];

		snprintf(filter_url, sizeof(filter_url), "filter_web_url%d", wp->p->filter_id);
		url_data = nvram_safe_get(filter_url);
		if (!strcmp(url_data, ""))
			return; // no data
		find_each(url, sizeof(url), url_data, token, which, "");
		websWrite(wp, "%s", url);
	}
	return;
}

EJ_VISIBLE void ej_filter_summary_show(webs_t wp, int argc, char_t **argv)
{
	int i;

#if LANGUAGE == JAPANESE
	char w[7][10] = { "��", "��", "��", "?�", "��", "��", "�y" };
	char am[] = "�ߑO";
	char pm[] = "�ߌ�";
	char _24h[] = "24 ����";
#else
	char w[7][15] = { "share.sun_s1", "share.mon_s1", "share.tue_s1", "share.wed_s1",
			  "share.thu_s1", "share.fri_s1", "share.sat_s1" };
	// char am[] = "AM";
	// char pm[] = "PM";
	char _24h[] = "24 Hours.";
#endif
	for (i = 0; i < NR_RULES; i++) {
		char name[50] = "---";
		char status[5] = "---";
		char filter[] = "filter_ruleXXX";
		char *data = "";
		char time_buf[50] = "---";

		snprintf(filter, sizeof(filter), "filter_rule%d", i + 1);
		data = nvram_safe_get(filter);
		if (data && strcmp(data, "")) {
			find_match_pattern(name, sizeof(name), data, "$NAME:", "&nbsp;"); // get
			// name
			// value
			find_match_pattern(status, sizeof(status), data, "$STAT:", "---"); // get
			// name
			// value
		}

		filter_tod_init(wp, i + 1);

		websWrite(wp,
			  "<tr class=\"table_row_bg center\" >\n"
			  "<td width=\"50\" >%d.</td>\n"
			  "<td width=\"200\" >%s</td>\n"
			  "<td width=\"150\" >\n"
			  "<table width=\"150\" border=\"1\" cellspacing=\"1\" style=\"border-collapse: collapse\" >\n"
			  "<tr class=\"center\">\n",
			  i + 1, name);
		websWrite(wp,
			  "<td class=\"%s\" width=\"17\"><script type=\"text/javascript\">Capture(%s)</script></td>\n"
			  "<td class=\"%s\" width=\"17\"><script type=\"text/javascript\">Capture(%s)</script></td>\n"
			  "<td class=\"%s\" width=\"17\"><script type=\"text/javascript\">Capture(%s)</script></td>\n"
			  "<td class=\"%s\" width=\"17\"><script type=\"text/javascript\">Capture(%s)</script></td>\n",
			  wp->p->tod_data_null == 0 && (wp->p->day_all == 1 || wp->p->week0 == 1) ? "table_bg_br_clr_on" :
												    "table_bg_br_clr_off",
			  w[0],
			  wp->p->tod_data_null == 0 && (wp->p->day_all == 1 || wp->p->week1 == 1) ? "table_bg_br_clr_on" :
												    "table_bg_br_clr_off",
			  w[1],
			  wp->p->tod_data_null == 0 && (wp->p->day_all == 1 || wp->p->week2 == 1) ? "table_bg_br_clr_on" :
												    "table_bg_br_clr_off",
			  w[2],
			  wp->p->tod_data_null == 0 && (wp->p->day_all == 1 || wp->p->week3 == 1) ? "table_bg_br_clr_on" :
												    "table_bg_br_clr_off",
			  w[3]);
		websWrite(wp,
			  "<td class=\"%s\" width=\"17\"><script type=\"text/javascript\">Capture(%s)</script></td>\n"
			  "<td class=\"%s\" width=\"17\"><script type=\"text/javascript\">Capture(%s)</script></td>\n"
			  "<td class=\"%s\" width=\"17\"><script type=\"text/javascript\">Capture(%s)</script></td>\n"
			  "</tr>\n"
			  "</table>\n"
			  "</td>\n",
			  wp->p->tod_data_null == 0 && (wp->p->day_all == 1 || wp->p->week4 == 1) ? "table_bg_br_clr_on" :
												    "table_bg_br_clr_off",
			  w[4],
			  wp->p->tod_data_null == 0 && (wp->p->day_all == 1 || wp->p->week5 == 1) ? "table_bg_br_clr_on" :
												    "table_bg_br_clr_off",
			  w[5],
			  wp->p->tod_data_null == 0 && (wp->p->day_all == 1 || wp->p->week6 == 1) ? "table_bg_br_clr_on" :
												    "table_bg_br_clr_off",
			  w[6]);

		if (wp->p->tod_data_null == 0) {
			if (wp->p->time_all == 1)
				strcpy(time_buf, _24h);
			else {
				snprintf(time_buf, sizeof(time_buf), "%02d:%02d - %02d:%02d", wp->p->start_hour, wp->p->start_min,
					 wp->p->end_hour, wp->p->end_min);
			}
		}
		websWrite(wp,
			  "<td width=\"150\" > %s </td>\n"
			  "<td width=\"70\" ><input type=\"checkbox\" name=\"sum%d\" value=\"1\" ></td>\n"
			  "</tr>\n",
			  time_buf, i + 1);
	}
	return;
}

EJ_VISIBLE void ej_filter_init(webs_t wp, int argc, char_t **argv)
{
	return;
}

EJ_VISIBLE void ej_filter_port_services_get(webs_t wp, int argc, char_t **argv)
{
	int which = atoi(argv[1]);
	filter_port_services_get(wp, argv[0], which);
}

void filter_port_services_get(webs_t wp, char *type, int which)
{
	char word[1024], *next;
	char delim[] = "<&nbsp;>";

	char *services;

	// get_filter_services (services);

	if (!strcmp(type, "all_list") || !strcmp(type, "user_list")) {
		if (!strcmp(type, "all_list"))
			services = get_filter_services();
		else // user_list only
		{
			int servicelen = strlen(nvram_safe_get("filter_services")) + strlen(nvram_safe_get("filter_services_1"));
			if (servicelen == 0)
				return;
			services = calloc(servicelen + 1, 1);
			strcat(services,
			       nvram_safe_get("filter_services")); // this
			// is
			// user
			// defined
			// filters
			strcat(services,
			       nvram_safe_get("filter_services_1")); // this
			// is
			// user
			// defined
			// filters
			//
		}

		int count = 0;

		split(word, services, next, delim)
		{
			int len = 0;
			char *name, *prot, *port;
			char protocol[100], ports[100];
			int from = 0, to = 0;

			// int proto;

			if ((name = strstr(word, "$NAME:")) == NULL || (prot = strstr(word, "$PROT:")) == NULL ||
			    (port = strstr(word, "$PORT:")) == NULL)
				continue;

			/*
			 * $NAME 
			 */
			if (sscanf(name, "$NAME:%3d:", &len) != 1)
				continue;
			strncpy(name, name + sizeof("$NAME:nnn:") - 1, len);
			name[len] = '\0';

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
			if (sscanf(ports, "%d:%d", &from, &to) != 2)
				continue;

			// cprintf("match:: name=%s, protocol=%s, ports=%s\n",
			// word, protocol, ports);

			websWrite(wp, "services[%d]=new service(%d, \"%s\", %d, %d, %d, \"%s\");\n", count, count, name, from, to,
				  protocol_to_num(protocol), protocol);
			count++;
		}
		debug_free(services);

		websWrite(wp, "services_length = %d;\n", count);
	} else if (!strcmp(type, "service")) {
		char *port_data, filter_port[] = "filter_port_grpXXX";
		char name[80];

		snprintf(filter_port, sizeof(filter_port), "filter_port_grp%d", wp->p->filter_id);
		port_data = nvram_safe_get(filter_port);
		if (!strcmp(port_data, ""))
			return; // no data
		find_each(name, sizeof(name), port_data, "<&nbsp;>", which, "");
		websWrite(wp, "%s", name);

	} else if (!strcmp(type, "p2p")) {
		char *port_data, filter_port[] = "filter_p2p_grpXXX";

		snprintf(filter_port, sizeof(filter_port), "filter_p2p_grp%d", wp->p->filter_id);
		port_data = nvram_safe_get(filter_port);
		if (!strcmp(port_data, ""))
			return; // no data
		websWrite(wp, "%s", port_data);
	}
	return;
}

void filtersummary_onload(webs_t wp, char *arg)
{
	if (!strcmp(nvram_safe_get("filter_summary"), "1")) {
		websWrite(wp, arg);
	}
}
