/*
 * conntrack.c
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
/*
 * Added by Botho 03.April.06 
 */

#if !defined(HAVE_MICRO) && !defined(__UCLIBC__)
#include <pthread.h>
static char *lastlock;
static char *lastunlock;
#define lock() pthread_mutex_lock(&wp->p->mutex_contr)
#define unlock() pthread_mutex_unlock(&wp->p->mutex_contr)
#else
#define mutex_init()
#define lock()
#define unlock()
#endif

EJ_VISIBLE void ej_dumpip_conntrack(webs_t wp, int argc, char_t **argv)
{
	int ip_count = 0;
	FILE *fp;
	int c;
	lock();
	fp = fopen("/proc/net/nf_conntrack", "rb");
	if (fp == NULL) {
		fp = fopen("/proc/net/ip_conntrack", "rb");
	}
	if (fp == NULL) {
		unlock();
		return;
	}
	while ((c = getc(fp)) != EOF) {
		if (c == 0xa)
			ip_count++;
	}
	fclose(fp);
	unlock();

	websWrite(wp, "%d", ip_count);

	return;
}

/*
 * Added by Botho 28.Oct.06 
 */
static int search_hit(char *search, char *line, char *ret)
{
	unsigned int searchLen;
	unsigned int i;
	unsigned int j;
	unsigned int lineLen;

	if (line == NULL || search == NULL || ret == NULL)
		return 1;
	char *p = strstr(line, search);
	if (!p)
		return 1;
	p += strlen(search);
	char *l = strchr(p, ' ');
	if (!l)
		return 1;
	strlcpy(ret, p, (l - p) + 1);
	return (0);
}

static int string_search(char *string, char *search)
{
	int searchLen;
	int i;

	if (search == NULL)
		return 0;
	searchLen = strlen(search);
	if (string == NULL)
		return 0;
	if (searchLen > strlen(string)) {
		return (0); // this can't match
	}
	int slen = strlen(string);

	for (i = 0; i < slen - searchLen; i++) { // +1 removed.
		if (!strncasecmp((char *)&string[i], search, searchLen)) {
			return (1); // we got hit
		}
	}
	return (0);
}

EJ_VISIBLE void ej_ip_conntrack_table(webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	int ip_count = 1;
	char *line;
	char *protocol;
	int timeout = 0;
	char srcip[64] = "";
	char dstip[64] = "";
	int _dport;
	struct servent *servp = NULL;
	char dstport[32] = "";
	char state[32] = "";
	char dum1[32];
	int dum2;
	char *lanip = nvram_safe_get("lan_ipaddr");
	char buf[128];
	lock();
	fp = fopen("/proc/net/nf_conntrack", "rb");
	if (fp == NULL) {
		fp = fopen("/proc/net/ip_conntrack", "rb");
	}
	if (fp == NULL) {
		unlock();
		return;
	}
	line = malloc(512);
	while (!feof(fp) && fgets(line, 511, fp) != NULL) {
		websWrite(wp, "<tr>\n");

		// Nb
		websWrite(wp, "<td align=\"right\">%d</td>", ip_count);

		// Proto
		if (string_search(line, "tcp"))
			protocol = "TCP";
		else if (string_search(line, "udp"))
			protocol = "UDP";
		else if (string_search(line, "icmp"))
			protocol = "ICMP";
		else if (string_search(line, "gre"))
			protocol = "GRE";
		else if (string_search(line, "sctp"))
			protocol = "SCTP";
		else
			protocol = tran_string(buf, sizeof(buf), "share.unknown");
		websWrite(wp, "<td>%s</td>", protocol);

		// Timeout
		sscanf(line, "%*s %*d %*s %*d %d", &timeout);

		websWrite(wp, "<td align=\"right\">%d</td>", timeout);

		// src
		if (search_hit("src=", line, srcip))
			continue;
		// char buf[200];
		// getHostName (buf, srcip);
		// websWrite (wp, "<td align=\"right\" onmouseover='DisplayDiv(this,
		// event, 15, 15, \"%s\")' onmouseout=\"unDisplayDiv()\">%s</td>",
		// buf != "unknown" ? buf : live_translate ("share.unknown") ,
		// srcip);
		if (!strcmp(srcip, lanip))
			websWrite(wp, "<td align=\"right\">%s</td>", srcip);
		else
			websWrite(
				wp,
				"<td align=\"right\"><a class=\"link\" title=\"Geotool\" href=\"javascript:openGeotool('%s')\">%s</a></td>",
				srcip, srcip);

		// dst
		if (search_hit("dst=", line, dstip))
			continue;
		// getHostName (buf, dstip);
		// websWrite (wp, "<td align=\"right\" onmouseover='DisplayDiv(this,
		// event, 15, 15, \"%s\")' onmouseout=\"unDisplayDiv()\">%s</td>",
		// buf != "unknown" ? buf : live_translate ("share.unknown") ,
		// dstip);
		if (!strcmp(dstip, lanip))
			websWrite(wp, "<td align=\"right\">%s</td>", dstip);
		else
			websWrite(
				wp,
				"<td align=\"right\"><a class=\"link\" title=\"Geotool\" href=\"javascript:openGeotool('%s')\">%s</a></td>",
				dstip, dstip);
		// service
		if (search_hit("dport=", line, dstport))
			continue;
		_dport = atoi(dstport);
		servp = my_getservbyport(htons(_dport), protocol);
		websWrite(wp, "<td align=\"right\">%s</td>", servp ? servp->s_name : dstport);
		if (servp) {
			debug_free(servp->s_proto);
			debug_free(servp->s_name);
			debug_free(servp);
		}
		// State
		if (string_search(line, "ESTABLISHED"))
			sprintf(state, "ESTABLISHED");
		else if (string_search(line, "TIME_WAIT"))
			sprintf(state, "TIME_WAIT");
		else if (string_search(line, "UNREPLIED"))
			sprintf(state, "UNREPLIED");
		else if (string_search(line, "CLOSE"))
			sprintf(state, "CLOSE");
		else if (string_search(line, "ASSURED"))
			sprintf(state, "ASSURED");
		else {
			if (string_search(line, "udp"))
				sprintf(state, "UNREPLIED");
			else
				sprintf(state, "&nbsp;");
		}
		websWrite(wp, "<td>%s</td>\n", state);
		websWrite(wp, "</tr>\n");
		ip_count++;
	}
	debug_free(line);

	fclose(fp);
	unlock();
	return;
}
