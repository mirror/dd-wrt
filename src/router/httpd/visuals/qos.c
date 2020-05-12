/*
 * qos.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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
 * Note: VLAN #16 designates tagging.  There is no VLAN #16 (only 0-15) 
 */

void ej_show_qos_aqd(webs_t wp, int argc, char_t ** argv)
{
#if defined(HAVE_CODEL) || defined(HAVE_FQ_CODEL)

	char *aqd = nvram_safe_get("svqos_aqd");

	websWrite(wp, "<div class=\"setting\">\n"
		  "<div class=\"label\"><script type=\"text/javascript\">Capture(qos.aqd)</script></div>\n"
		  "<select name=\"qos_aqd\">\n" "<option value=\"sfq\" %s><script type=\"text/javascript\">Capture(qos.aqd_sfq)</script></option>\n", strcmp(aqd, "sfq") == 0 ? "selected" : "");
#ifdef HAVE_CODEL
	websWrite(wp, "<option value=\"codel\" %s><script type=\"text/javascript\">Capture(qos.aqd_codel)</script></option>\n", strcmp(aqd, "codel") == 0 ? "selected" : "");
#endif
#ifdef HAVE_FQ_CODEL
	websWrite(wp, "<option value=\"fq_codel\" %s><script type=\"text/javascript\">Capture(qos.aqd_fqcodel)</script></option>\n", strcmp(aqd, "fq_codel") == 0 ? "selected" : "");
#endif
#ifdef HAVE_FQ_CODEL_FAST
	websWrite(wp, "<option value=\"fq_codel_fast\" %s><script type=\"text/javascript\">Capture(qos.aqd_fqcodel_fast)</script></option>\n", strcmp(aqd, "fq_codel_fast") == 0 ? "selected" : "");
#endif
#ifdef HAVE_PIE
	websWrite(wp, "<option value=\"pie\" %s><script type=\"text/javascript\">Capture(qos.aqd_pie)</script></option>\n", strcmp(aqd, "pie") == 0 ? "selected" : "");
#endif
#ifdef HAVE_CAKE
	websWrite(wp, "<option value=\"cake\" %s><script type=\"text/javascript\">Capture(qos.aqd_cake)</script></option>\n", strcmp(aqd, "cake") == 0 ? "selected" : "");
#endif

	websWrite(wp, "</select>\n</div>\n");

#else
	websWrite(wp, "<input type=\"hidden\" name=\"qos_aqd\" </input>");
#endif
}

void ej_get_qospkts(webs_t wp, int argc, char_t ** argv)
{
	char *qos_pkts = nvram_safe_get("svqos_pkts");
	char pkt_filter[4];

	websWrite(wp, "<tr>\n"
		  "<td><input type=\"checkbox\" name=\"svqos_pktack\" value=\"ACK\" %s><script type=\"text/javascript\">Capture(qos.pktack)</script></input></td>\n"
		  "<td><input type=\"checkbox\" name=\"svqos_pktsyn\" value=\"SYN\" %s><script type=\"text/javascript\">Capture(qos.pktsyn)</script></input></td>\n"
		  "<td><input type=\"checkbox\" name=\"svqos_pktfin\" value=\"FIN\" %s><script type=\"text/javascript\">Capture(qos.pktfin)</script></input></td>\n"
		  "<td><input type=\"checkbox\" name=\"svqos_pktrst\" value=\"RST\" %s><script type=\"text/javascript\">Capture(qos.pktrst)</script></input></td>\n"
		  "<td><input type=\"checkbox\" name=\"svqos_pkticmp\" value=\"ICMP\" %s><script type=\"text/javascript\">Capture(qos.pkticmp)</script></input></td>\n"
		  "</tr>\n", strstr(qos_pkts, "ACK") ? "checked" : "", strstr(qos_pkts, "SYN") ? "checked" : "", strstr(qos_pkts, "FIN") ? "checked" : "", strstr(qos_pkts, "RST") ? "checked" : "", strstr(qos_pkts,
																									    "ICMP") ?
		  "checked" : "");
}

void getpacketcounts(char *table, char *chain, unsigned long long *counts, int len);

void ej_get_qossvcs(webs_t wp, int argc, char_t ** argv)
{
	char *qos_svcs = nvram_safe_get("svqos_svcs");
	char name[32], type[32], data[32], level[32];
	int no_svcs = 0, i = 0, realno = 0;

	// calc # of services
	// no_svcs = strspn(qos_svcs,"|");

	while ((qos_svcs = strpbrk(qos_svcs, "|"))) {
		no_svcs++;
		qos_svcs++;
		realno++;
		if (sscanf(qos_svcs, "%31s %31s %31s %31s ", name, type, data, level) < 4)
			continue;
		if (!strcmp(type, "both") || !strcmp(type, "udp") || !strcmp(type, "tcp"))
			realno++;
	}

	// write HTML data

	websWrite(wp, "<tr><td colspan=\"4\"><input type=\"hidden\" name=\"svqos_nosvcs\" value=\"%d\" /></td></tr>", no_svcs);

	qos_svcs = nvram_safe_get("svqos_svcs");

	/*
	 * services format is "name type data level | name type data level |"
	 * ..etc 
	 */
	unsigned long long *counts = NULL;
	if (no_svcs) {
		counts = malloc(sizeof(unsigned long long) * (realno + 2));
		memset(counts, 0, sizeof(unsigned long long) * (realno + 2));
		getpacketcounts("mangle", "SVQOS_SVCS", counts, realno);
	}
	int c = 0;
	for (i = 0; i < no_svcs && qos_svcs && qos_svcs[0]; i++) {
		if (sscanf(qos_svcs, "%31s %31s %31s %31s ", name, type, data, level) < 4)
			break;

		websWrite(wp, "<tr>\n"
			  "<td>\n"
			  "<input type=\"checkbox\" name=\"svqos_svcdel%d\" />\n"
			  "<input type=\"hidden\" name=\"svqos_svcname%d\" value=\"%s\" />\n"
			  "<input type=\"hidden\" name=\"svqos_svctype%d\" value=\"%s\" />\n" "</td>\n" "<td><em>%s</em></td>\n" "<td >\n", i, i, name, i, type, name);
#if 0
		websWrite(wp, "<select name=\"svqos_svcprio%d\"> \n"
			  "<script type=\"text/javascript\">\n//<![CDATA[\n"
			  "document.write(\"<option value=\\\"1000\\\" %s >\" + qos.prio_exempt + \"</option>\");\n"
			  "document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n"
			  "document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n"
			  "document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n"
			  "document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n"
			  "document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n</script>\n"
			  "</select>\n"
			  "</td>\n", i, strcmp(level, "1000") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level,
																					    "10") == 0 ? "selected=\\\"selected\\\"" : "",
			  strcmp(level, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "40") == 0 ? "selected=\\\"selected\\\"" : "");
#else
		websWrite(wp, "<select name=\"svqos_svcprio%d\"> \n"
			  "<script type=\"text/javascript\">\n//<![CDATA[\n"
			  "document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n"
			  "document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n"
			  "document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n"
			  "document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n"
			  "document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n</script>\n"
			  "</select>\n"
			  "</td>\n", i, strcmp(level, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "10") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level,
																					  "20") ==
			  0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "40") == 0 ? "selected=\\\"selected\\\"" : "");
#endif
		if (!strcmp(type, "both") || !strcmp(type, "udp") || !strcmp(type, "tcp")) {
			websWrite(wp, "<td>%llu</td>", counts[c] + counts[c + 1]);
			c += 2;
		} else {
			websWrite(wp, "<td>%llu</td>", counts[c++]);
		}
		websWrite(wp, "</tr>\n");
		qos_svcs = strpbrk(++qos_svcs, "|");
		qos_svcs++;

	}
	if (counts)
		free(counts);

	return;
}

void ej_get_qosdevs(webs_t wp, int argc, char_t ** argv)
{
	char *qos_ips = nvram_safe_get("svqos_devs");
	char ip[32], level[32], level2[32], lanlevel[32], prio[32], proto[32];
	int no_ips = 0, i = 0;

	// calc # of ips
	while ((qos_ips = strpbrk(qos_ips, "|"))) {
		no_ips++;
		qos_ips++;
	}
	websWrite(wp, "<tr>\n"	//
		  "<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(share.intrface)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.maxdownrate_b)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.maxuprate_b)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.maxlanrate_b)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.service)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(share.priority)</script></th>\n"	//
		  "</tr>\n");	//

	// write HTML data

	websWrite(wp, "<tr><td colspan=\"3\"><input type=\"hidden\" name=\"svqos_nodevs\" value=\"%d\" /></td></tr>", no_ips);

	qos_ips = nvram_safe_get("svqos_devs");

	/*
	 * IP format is "data level | data level |" ..etc 
	 */

	lanlevel[0] = '\0';
	prio[0] = '\0';
	for (i = 0; i < no_ips && qos_ips && qos_ips[0]; i++) {
		if (sscanf(qos_ips, "%31s %31s %31s", ip, level, level2) < 3)
			break;
		if (sscanf(qos_ips, "%31s %31s %31s %31s %31s %31s", ip, level, level2, lanlevel, prio, proto)) {
			if (!strcmp(lanlevel, "|")) {
				strcpy(lanlevel, "0");
				strcpy(prio, "0");
			}
			if (!strcmp(prio, "|"))
				strcpy(prio, "0");

			if (!strcmp(proto, "|"))
				strcpy(proto, "none");
		}

		websWrite(wp, "<tr>\n" "<td align=\"center\">\n"	//
			  "<input type=\"checkbox\" name=\"svqos_devdel%d\" />\n"	//
			  "<input type=\"hidden\" name=\"svqos_dev%d\" value=\"%s\" />\n"	//
			  "</td>\n", i, i, ip);
		websWrite(wp, "	<td><em>%s</em></td>\n", getNetworkLabel(wp, ip));

		websWrite(wp, "	<td nowrap>\n" "<input name=\"svqos_devdown%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n"	//
			  "</td>\n", i, level2, strcmp(prio, "0") == 0 ? "" : "disabled");
		websWrite(wp, "	<td nowrap>\n" "<input class=\"num\" name=\"svqos_devup%d\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n" "</td>\n", i, level,	//
			  strcmp(prio, "0") == 0 ? "" : "disabled");

		websWrite(wp, "	<td nowrap>\n"	//
			  "<input name=\"svqos_devlanlvl%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n"	//
			  "</td>\n", i, lanlevel, strcmp(prio, "0") == 0 ? "" : "disabled");	//
		/* service */
		filters *services = get_filters_list();
		if (services) {
			int count = 0;
			websWrite(wp, "	<td nowrap>\n");
			websWrite(wp, "<select name=\"svqos_devservice%d\" style=\"overflow:hidden; max-width:100px;\"> size=\"1\"\n", i);
			websWrite(wp, "<option value=\"none\" %s >None</option>\n", !strcmp(proto, "none") ? "selected=\"selected\"" : "");
			while (services[count].name != NULL) {
				websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", services[count].name, !strcmp(proto, services[count].name) ? "selected=\"selected\"" : "", services[count].name);
				count++;
			}
			websWrite(wp, "</select>\n");
			free_filters(services);
		}
		websWrite(wp, "	<td>\n"	//
			  "<select name=\"svqos_devprio%d\" onChange=\"iplvl_grey(%d,this,this.form,false)\"> \n"	//
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\" %s >\" + qos.prio_m + \"</option>\");\n"	//
			  "document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n"	//
			  "document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n"	//
			  "document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n"	//
			  "document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n"	//
			  "document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n"	//
			  "</script>\n"	//
			  "</select>\n"	//
			  "</td>\n"	//
			  "</tr>\n", i, i, strcmp(prio, "0") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio,
																					  "10") == 0 ? "selected=\\\"selected\\\"" : "",
			  strcmp(prio, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "40") == 0 ? "selected=\\\"selected\\\"" : "");

		qos_ips = strpbrk(++qos_ips, "|");
		qos_ips++;

	}

	return;
}

void ej_get_qosips(webs_t wp, int argc, char_t ** argv)
{
	char *qos_ips = nvram_safe_get("svqos_ips");
	char ip[32], level[32], level2[32], lanlevel[32], prio[32];
	int no_ips = 0, i = 0;

	// calc # of ips
	while ((qos_ips = strpbrk(qos_ips, "|"))) {
		no_ips++;
		qos_ips++;
	}
	websWrite(wp, "<tr>\n"	//
		  "<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.ipmask)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.maxdownrate_b)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.maxuprate_b)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.maxlanrate_b)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(share.priority)</script></th>\n"	//
		  "</tr>\n");	//

	// write HTML data

	websWrite(wp, "<tr><td colspan=\"3\"><input type=\"hidden\" name=\"svqos_noips\" value=\"%d\" /></td></tr>", no_ips);

	qos_ips = nvram_safe_get("svqos_ips");

	/*
	 * IP format is "data level | data level |" ..etc 
	 */

	lanlevel[0] = '\0';
	prio[0] = '\0';
	for (i = 0; i < no_ips && qos_ips && qos_ips[0]; i++) {
		if (sscanf(qos_ips, "%31s %31s %31s", ip, level, level2) < 3)
			break;
		if (sscanf(qos_ips, "%31s %31s %31s %31s %31s", ip, level, level2, lanlevel, prio)) {
			if (!strcmp(lanlevel, "|")) {
				strcpy(lanlevel, "0");
				strcpy(prio, "0");
			}
			if (!strcmp(prio, "|"))
				strcpy(prio, "0");
		}

		websWrite(wp, "<tr>\n" "<td align=\"center\">\n" "<input type=\"checkbox\" name=\"svqos_ipdel%d\" />\n" "<input type=\"hidden\" name=\"svqos_ip%d\" value=\"%s\" />\n" "</td>\n", i, i, ip);
		websWrite(wp, "	<td><em>%s</em></td>\n", ip);

		websWrite(wp, "	<td nowrap>\n"
			  "<input name=\"svqos_ipdown%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n" "</td>\n", i, level2, strcmp(prio, "0") == 0 ? "" : "disabled");
		websWrite(wp, "	<td nowrap>\n" "<input class=\"num\" name=\"svqos_ipup%d\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n" "</td>\n", i, level,
			  strcmp(prio, "0") == 0 ? "" : "disabled");

		websWrite(wp, "	<td nowrap>\n"
			  "<input name=\"svqos_iplanlvl%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n"
			  "</td>\n", i, lanlevel, strcmp(prio, "0") == 0 ? "" : "disabled");

		websWrite(wp, "	<td>\n"
			  "<select name=\"svqos_ipprio%d\" onChange=\"iplvl_grey(%d,this,this.form,false)\"> \n"
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\" %s >\" + qos.prio_m + \"</option>\");\n"
			  "document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n"
			  "document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n"
			  "document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n"
			  "document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n"
			  "document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n"
			  "</script>\n"
			  "</select>\n"
			  "</td>\n"
			  "</tr>\n", i, i, strcmp(prio, "0") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio,
																					  "10") ==
			  0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio,
																							       "40") ==
			  0 ? "selected=\\\"selected\\\"" : "");

		qos_ips = strpbrk(++qos_ips, "|");
		qos_ips++;

	}

	return;
}

void ej_get_qosmacs(webs_t wp, int argc, char_t ** argv)
{
	char *qos_macs = nvram_safe_get("svqos_macs");
	char mac[32], level[32], level2[32], lanlevel[32], user[32], prio[32];
	int no_macs = 0, i = 0;

	// calc # of ips
	while ((qos_macs = strpbrk(qos_macs, "|"))) {
		no_macs++;
		qos_macs++;
	}
	websWrite(wp, "<tr>\n"	//
		  "<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(share.mac)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.maxdownrate_b)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.maxuprate_b)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(qos.maxlanrate_b)</script></th>\n"	//
		  "<th><script type=\"text/javascript\">Capture(share.priority)</script></th>\n"	//
		  "</tr>\n");	//

	// write HTML data
	websWrite(wp, "<input type=\"hidden\" name=\"svqos_nomacs\" value=\"%d\" />", no_macs);

	qos_macs = nvram_safe_get("svqos_macs");

	lanlevel[0] = '\0';
	prio[0] = '\0';
	for (i = 0; i < no_macs && qos_macs && qos_macs[0]; i++) {
		if (sscanf(qos_macs, "%31s %31s %31s %31s ", mac, level, level2, user) < 4)
			break;
		if (sscanf(qos_macs, "%31s %31s %31s %31s %31s %31s", mac, level, level2, user, lanlevel, prio)) {
			if (!strcmp(lanlevel, "|")) {
				strcpy(lanlevel, "0");
				strcpy(prio, "0");
			}
			if (!strcmp(prio, "|"))
				strcpy(prio, "0");
		}

		websWrite(wp, "<tr>\n"
			  "<td align=\"center\">\n"
			  "<input type=\"checkbox\" name=\"svqos_macdel%d\" />\n"
			  "<input type=\"hidden\" name=\"svqos_mac%d\" value=\"%s\" />\n"
			  "</td>\n"
			  "<td><em>%s</em></td>\n"
			  "<td nowrap>\n"
			  "<input name=\"svqos_macdown%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n"
			  "</td>\n"
			  "<td nowrap>\n"
			  "<input name=\"svqos_macup%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n"
			  "</td>\n"
			  "<td nowrap>\n"
			  "<input name=\"svqos_maclanlvl%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n"
			  "</td>\n"
			  "<td>\n", i, i, mac, mac, i, level2, strcmp(prio, "0") == 0 ? "" : "disabled", i, level, strcmp(prio, "0") == 0 ? "" : "disabled", i, lanlevel, strcmp(prio, "0") == 0 ? "" : "disabled");

		websWrite(wp, "<select name=\"svqos_macprio%d\" onChange=\"maclvl_grey(%d,this,this.form,false)\"> \n"
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\" %s >\" + qos.prio_m + \"</option>\");\n"
			  "document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n"
			  "document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n"
			  "document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n"
			  "document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n"
			  "document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n</script>\n"
			  "</select>\n"
			  "</td>\n"
			  "</tr>\n", i, i, strcmp(prio, "0") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio,
																					  "10") ==
			  0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio,
																							       "40") ==
			  0 ? "selected=\\\"selected\\\"" : "");

		qos_macs = strpbrk(++qos_macs, "|");
		qos_macs++;

	}

	return;
}
