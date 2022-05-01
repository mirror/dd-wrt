/*
 * vlantagging.c
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
EJ_VISIBLE void ej_show_vlantagging(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	int count = 0;
	char word[256];
	char *next, *wordlist;

	bzero(buffer, 256);
	getIfList(buffer, NULL);
	int totalcount = 0;
	int realcount = nvram_default_geti("vlan_tagcount", 0);
	websWrite(wp, "<table cellspacing=\"4\" summary=\"vlans\" id=\"vlan_table\" class=\"table center\"><tr>\n");
	show_caption_pp(wp, NULL, "networking.iface", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.tg_number", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.prio", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "share.actiontbl", "<th style=\"text-align: center\" width=\"10%\">", "</th>\n");

	wordlist = nvram_safe_get("vlan_tags");
	foreach(word, wordlist, next) {

		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		if (!tag || !port)
			break;
		if (!prio)
			prio = "0";
		char vlan_name[32];
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td>");
		sprintf(vlan_name, "vlanifname%d", count);
		showIfOptions(wp, vlan_name, buffer, tag);
		websWrite(wp, "</td>\n");
		//tag number
		sprintf(vlan_name, "vlantag%d", count);
		websWrite(wp, "<td>");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n", vlan_name, port);
		websWrite(wp, "</td>\n");
		//priority
		sprintf(vlan_name, "vlanprio%d", count);
		websWrite(wp, "<td>");
		showOptions(wp, vlan_name, "0 1 2 3 4 5 6 7", prio);
		websWrite(wp, "</td>\n");

		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td align=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"vlan_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
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

		sprintf(vlan_name, "vlanifname%d", i);
		showIfOptions(wp, vlan_name, buffer, "");
		websWrite(wp, "</td>\n");

		sprintf(vlan_name, "vlantag%d", i);
		//tag number
		websWrite(wp, "<td>");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"0\" />\n", vlan_name);
		websWrite(wp, "</td>\n");
		//priority
		sprintf(vlan_name, "vlanprio%d", i);
		websWrite(wp, "<td>");
		showOptions(wp, vlan_name, "0 1 2 3 4 5 6 7", "0");
		websWrite(wp, "</td>\n");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td align=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"vlan_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</td>\n");
		websWrite(wp, "</tr>\n");
		totalcount++;
	}
	websWrite(wp, "</table>\n");
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("vlan_tagcount", var);
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"vlan_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
}
