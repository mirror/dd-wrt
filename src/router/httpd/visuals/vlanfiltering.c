/*
 * vlanfiltering.c
 *
 * Copyright (C) 2005 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_VLAN_FILTERING
#include <libbridge.h>
EJ_VISIBLE void ej_show_vlanfiltering(webs_t wp, int argc, char_t **argv)
{
	char buffer[256];
	int count = 0;
	char word[256];
	const char *next, *wordlist;
	int has_vlan = br_has_vlan_filtering();
	if (has_vlan) {
		websWrite(wp, "<h2><script type=\"text/javascript\">Capture(networking.hfiltering)</script></h2>");
		websWrite(wp,
			  "<fieldset>\n<legend><script type=\"text/javascript\">Capture(networking.flegend)</script></legend>\n");

		getIfList(buffer, sizeof(buffer), NULL);
		int totalcount = 0;
		int realcount = nvram_default_geti("vlan_filtercount", 0);
		websWrite(wp, "<table cellspacing=\"4\" summary=\"filters\" id=\"filters_table\" class=\"table\"><thead><tr>\n");
		show_caption_pp(wp, NULL, "networking.iface", "<th>", "</th>\n");
		show_caption_pp(wp, NULL, "networking.tg_number", "<th>", "</th>\n");
		show_caption_pp(wp, NULL, "networking.pvid", "<th>", "</th>\n");
		show_caption_pp(wp, NULL, "networking.untagged", "<th>", "</th>\n");
		show_caption_pp(wp, NULL, "share.actiontbl", "<th class=\"center\" width=\"10%%\">", "</th></thead><tbody>\n");

		wordlist = nvram_safe_get("vlan_filters");
		foreach(word, wordlist, next) {
			GETENTRYBYIDX(ifname, word, 0);
			GETENTRYBYIDX(vlan, word, 1);
			GETENTRYBYIDX(pvid, word, 2);
			GETENTRYBYIDX(untagged, word, 3);
			if (!ifname || !vlan)
				break;
			char vlan_name[32];
			websWrite(wp, "<tr>\n");
			websWrite(wp, "<td>");
			sprintf(vlan_name, "filterifname%d", count);
			showIfOptions(wp, vlan_name, buffer, ifname);
			websWrite(wp, "</td>\n");
			//tag number
			sprintf(vlan_name, "filtertag%d", count);
			websWrite(wp, "<td>");
			websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n", vlan_name, vlan);
			websWrite(wp, "</td>\n");
			//pvid
			sprintf(vlan_name, "filterpvid%d", count);
			websWrite(wp, "<td>");
			websWrite(wp, "<input type=\"checkbox\" name=\"%s\" value=\"1\" %s>\n", vlan_name,
				  !strcmp(pvid, "1") ? "checked" : "");
			websWrite(wp, "</td>\n");
			//untagged
			sprintf(vlan_name, "filteruntagged%d", count);
			websWrite(wp, "<td>");
			websWrite(wp, "<input type=\"checkbox\" name=\"%s\" value=\"1\" %s>\n", vlan_name,
				  !strcmp(untagged, "1") ? "checked" : "");
			websWrite(wp, "</td>\n");
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"filter_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
				count);
			websWrite(wp, "</td>\n");
			websWrite(wp, "</tr>\n");

			count++;
		}
		totalcount = count;
		int i;

		for (i = count; i < realcount; i++) {
			char vlan_name[32];
			websWrite(wp, "<tr>\n");
			websWrite(wp, "<td>");

			sprintf(vlan_name, "filterifname%d", i);
			showIfOptions(wp, vlan_name, buffer, "");
			websWrite(wp, "</td>\n");

			//tag number
			sprintf(vlan_name, "filtertag%d", count);
			websWrite(wp, "<td>");
			websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"0\" />\n", vlan_name);
			websWrite(wp, "</td>\n");

			//pvid
			sprintf(vlan_name, "filterpvid%d", count);
			websWrite(wp, "<td>");
			websWrite(wp, "<input type=\"checkbox\" name=\"%s\" value=\"1\">\n", vlan_name);
			websWrite(wp, "</td>\n");

			//untagged
			sprintf(vlan_name, "filteruntagged%d", count);
			websWrite(wp, "<td>");
			websWrite(wp, "<input type=\"checkbox\" name=\"%s\" value=\"1\">\n", vlan_name);
			websWrite(wp, "</td>\n");

			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"filter_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
				i);
			websWrite(wp, "</td>\n");
			websWrite(wp, "</tr>\n");
			totalcount++;
		}
		char var[32];

		sprintf(var, "%d", totalcount);
		nvram_set("vlan_filtercount", var);
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td colspan=\"4\">&nbsp;</td>\n");
		websWrite(wp, "<td class=\"center\">\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"filter_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
		websWrite(wp, "</td>\n");
		websWrite(wp, "</tr>\n");
		websWrite(wp, "</tbody></table>\n");

		websWrite(wp, "</fieldset><br />\n");
	}
}
#endif