/*
 * log.c
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
#include <time.h>
#include <sys/klog.h>
#include <netdb.h>

#include <broadcom.h>

#define LOG_BUF 16384 // max buf, total have 64 entries

/*
 * Dump firewall log 
 */
EJ_VISIBLE void ej_dumplog(webs_t wp, int argc, char_t **argv)
{
	char wan_if_buffer[33];
	char *buf, *line, *next, *s;
	int len;
	char *type;

	time_t tm;
	char *verdict, *src, *dst, *proto, *spt, *dpt, *in, *out;
	char src_old[32] = "", dpt_old[32] = "", dst_old[32] = "", proto_old[32] = "";

	int _dport, _sport;
	char *_proto = NULL;
	struct servent *servp;

	// struct servent *d_servp;

	char *wan_if = safe_get_wan_face(wan_if_buffer);
	char *lan_if = nvram_safe_get("lan_ifname");

	int count = 0;

	type = argv[0];

	buf = malloc(LOG_BUF);
	bzero(buf, LOG_BUF);
	// if (klogctl(3, buf, 4096) < 0) {
	if (klogctl(3, buf, LOG_BUF) < 0) {
		websError(wp, 400, "Insufficient memory\n");
		debug_free(buf);
		return;
	}
	cprintf("log: %s\n", buf);
	for (next = buf; (line = strsep(&next, "\n"));) {
		if (!strncmp(line, "<4>DROP", 7))
			verdict = "Dropped";
		else if (!strncmp(line, "<4>ACCEPT", 9))
			verdict = "Accepted";
		else if (!strncmp(line, "<4>REJECT", 9))
			verdict = "Rejected";
		else if (!strncmp(line, "<4>[", 4)) {
			// kernel timings included
			line = strchr(line, ']');
			if (!line)
				continue;
			line += 2;
			if (!strncmp(line, "DROP", 4))
				verdict = "Dropped";
			else if (!strncmp(line, "ACCEPT", 6))
				verdict = "Accepted";
			else if (!strncmp(line, "REJECT", 6))
				verdict = "Rejected";
			else
				continue;
		} else
			continue;

		/*
		 * Parse into tokens 
		 */
		s = line;
		len = strlen(s);
		while (strsep(&s, " "))
			;

		/*
		 * Initialize token values 
		 */
		time(&tm);
		in = out = src = dst = proto = spt = dpt = "n/a";

		/*
		 * Set token values 
		 */
		for (s = line; s < &line[len] && *s; s += strlen(s) + 1) {
			if (!strncmp(s, "TIME=", 5))
				tm = strtoul(&s[5], NULL, 10);
			else if (!strncmp(s, "IN=", 3))
				in = &s[3];
			else if (!strncmp(s, "OUT=", 4))
				out = &s[4];
			else if (!strncmp(s, "SRC=", 4))
				src = &s[4];
			else if (!strncmp(s, "DST=", 4))
				dst = &s[4];
			else if (!strncmp(s, "PROTO=", 6))
				proto = &s[6];
			else if (!strncmp(s, "SPT=", 4))
				spt = &s[4];
			else if (!strncmp(s, "DPT=", 4))
				dpt = &s[4];
		}

		if (!strncmp(dpt, "n/a", 3)) // example: ping
			continue;

		_dport = atoi(dpt);
		_sport = atoi(spt);

		servp = my_getservbyport(htons(_dport), proto);

		if (!strcmp(type, "incoming")) {
			if ((!strncmp(in, "ppp", 3) && !strncmp(in, wan_if, 3)) || (!strcmp(in, wan_if))) {
				if (!strcmp(src, src_old) && !strcmp(dpt, dpt_old) && !strcmp(proto, proto_old)) {
					continue; // skip same record
				} else {
					strlcpy(src_old, src, sizeof(src_old) - 1);
					strlcpy(dpt_old, dpt, sizeof(dpt_old) - 1);
					strlcpy(proto_old, proto, sizeof(proto_old) - 1);
				}

				websWrite(
					wp,
					"<tr height=\"1\">\n<td>%s</td>\n<td class=\"center\">%s</td>\n<td class=\"center\">%s</td>\n<td class=\"center\">%s</td>\n</tr>\n",
					src, proto, servp ? servp->s_name : dpt, verdict);
			}
		} else if (!strcmp(type, "outgoing")) {
			if (!strncmp(in, lan_if, 3) &&
			    ((!strncmp(out, "ppp", 3) && !strncmp(out, wan_if, 3)) || (!strcmp(out, wan_if)))) {
				if (_dport == 53) {
					continue; // skip DNS
				}

				if (!strcmp(src, src_old) && !strcmp(dst, dst_old) && !strcmp(proto, proto_old) &&
				    !strcmp(dpt, dpt_old)) {
					continue; // skip same record
				} else {
					strlcpy(src_old, src, sizeof(src_old) - 1);
					strlcpy(dst_old, dst, sizeof(dst_old) - 1);
					strlcpy(proto_old, proto, sizeof(proto_old) - 1);
					strlcpy(dpt_old, dpt, sizeof(dpt_old) - 1);
				}

				websWrite(
					wp,
					"<tr height=\"1\">\n<td>%s</td>\n<td>%s</td>\n<td class=\"center\">%s</td>\n<td>%s</td>\n<td class=\"center\">%s</td>\n</tr>\n",
					src, dst, proto, servp ? servp->s_name : dpt, verdict);
			}
		} else if (!strcmp(type, "all")) {
			int dir = 0;

			if ((!strncmp(out, "ppp", 3) && !strncmp(out, wan_if, 3)) || (!strcmp(out, wan_if))) // incoming
				dir = 1;
			else if (!strncmp(in, lan_if, 3) &&
				 ((!strncmp(out, "ppp", 3) && !strncmp(out, wan_if, 3)) || (!strcmp(out, wan_if)))) // outgoing
				dir = 2;
			else
				continue;

			if (_dport == 53) {
				continue; // skip DNS
			}

			if (!strcmp(src, src_old) && !strcmp(dpt, dpt_old) && !strcmp(dst, dst_old)) {
				continue; // skip same record
			} else {
				strlcpy(src_old, src, sizeof(src_old) - 1);
				strlcpy(dpt_old, dpt, sizeof(dpt_old) - 1);
				strlcpy(dst_old, dst, sizeof(dst_old) - 1);
			}

			websWrite(wp, "%c'%s','%s','%s','%s','%d'\n", count ? ',' : ' ', proto, src, dst,
				  servp ? servp->s_name : dpt, dir);
			count++;
		}
		if (servp) {
			debug_free(servp->s_name);
			debug_free(servp->s_proto);
			debug_free(servp);
		}
		// if(s_service) debug_free(s_service);
		// if(d_service) debug_free(d_service);
	}
	debug_free(buf);
	return;
}
