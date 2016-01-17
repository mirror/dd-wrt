/*
 * conntrack.c
 *
 * Copyright (C) 2005 - 2016 Sebastian Gottschall <gottschall@dd-wrt.com>
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
void ej_dumpip_conntrack(webs_t wp, int argc, char_t ** argv)
{
	int ip_count = 0;
	FILE *fp;
	int c;

	fp = fopen("/proc/net/ip_conntrack", "rb");
	if (fp == NULL)
		return;
	while (!feof(fp)) {
		c = getc(fp);
		if (c == EOF)
			break;
		if (c == 0xa)
			ip_count++;
	}

	websWrite(wp, "%d", ip_count);

	fclose(fp);

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
	lineLen = strlen(line);
	searchLen = strlen(search);

	if (searchLen > lineLen)
		return (1);	// this can't match, invalid data?

	for (i = 0; i < lineLen - searchLen; i++) {
		if (!strncasecmp((char *)&line[i], search, searchLen))
			break;	// we got hit
	}

	for (j = i + searchLen; j < i + 15 + searchLen; j++) {
		if (j >= lineLen)
			break;	// end of line may be a delimiter too
		// return(1); // incomplete data
		if (line[j] == ' ')
			break;	// we reach _space_ delimiter
	}
	memcpy(ret, &line[i + searchLen], j - (i + searchLen));
	ret[j - (i + searchLen)] = 0;
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
		return (0);	// this can't match
	}
	int slen = strlen(string);

	for (i = 0; i < slen - searchLen; i++) {	// +1 removed.
		if (!strncasecmp((char *)&string[i], search, searchLen)) {
			return (1);	// we got hit
		}
	}
	return (0);
}

void ej_ip_conntrack_table(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp;
	int ip_count = 1;
	char line[512];
	char protocol[16] = "";
	int timeout = 0;
	char srcip[16] = "";
	char dstip[16] = "";
	int _dport;
	struct servent *servp;
	char dstport[6] = "";
	char state[12] = "";
	char dum1[32];
	int dum2;
	char *lanip = nvram_get("lan_ipaddr");

	fp = fopen("/proc/net/ip_conntrack", "rb");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp) != NULL) {

		websWrite(wp, "<tr>\n");

		// Nb
		websWrite(wp, "<td align=\"right\">%d</td>", ip_count);

		// Proto
		if (string_search(line, "tcp"))
			sprintf(protocol, "TCP");
		else if (string_search(line, "udp"))
			sprintf(protocol, "UDP");
		else if (string_search(line, "icmp"))
			sprintf(protocol, "ICMP");
		else
			sprintf(protocol, live_translate("share.unknown"));
		websWrite(wp, "<td>%s</td>", protocol);

		// Timeout
		sscanf(line, "%s %d %d", &dum1[0], &dum2, &timeout);
		websWrite(wp, "<td align=\"right\">%d</td>", timeout);

		// src
		search_hit("src=", line, srcip);
		// char buf[200];
		// getHostName (buf, srcip);
		// websWrite (wp, "<td align=\"right\" onmouseover='DisplayDiv(this,
		// event, 15, 15, \"%s\")' onmouseout=\"unDisplayDiv()\">%s</td>",
		// buf != "unknown" ? buf : live_translate ("share.unknown") ,
		// srcip);
		if (!strcmp(srcip, lanip))
			websWrite(wp, "<td align=\"right\">%s</td>", srcip);
		else
			websWrite(wp, "<td align=\"right\"><a title=\"Geotool\" href=\"javascript:openGeotool('%s')\">%s</a></td>", srcip, srcip);

		// dst
		search_hit("dst=", line, dstip);
		// getHostName (buf, dstip);
		// websWrite (wp, "<td align=\"right\" onmouseover='DisplayDiv(this,
		// event, 15, 15, \"%s\")' onmouseout=\"unDisplayDiv()\">%s</td>",
		// buf != "unknown" ? buf : live_translate ("share.unknown") ,
		// dstip);
		if (!strcmp(dstip, lanip))
			websWrite(wp, "<td align=\"right\">%s</td>", dstip);
		else
			websWrite(wp, "<td align=\"right\"><a title=\"Geotool\" href=\"javascript:openGeotool('%s')\">%s</a></td>", dstip, dstip);

		// service
		search_hit("dport=", line, dstport);
		_dport = atoi(dstport);
		servp = my_getservbyport(htons(_dport), protocol);
		websWrite(wp, "<td align=\"right\">%s</td>", servp ? servp->s_name : dstport);

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

	fclose(fp);

	return;
}
