/*
 * vlantagging.c
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
void ej_show_vlantagging(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	int count = 0;
	char word[256];
	char *next, *wordlist;

	memset(buffer, 0, 256);
	getIfList(buffer, NULL);
	int totalcount = 0;
	int realcount = atoi(nvram_default_get("vlan_tagcount", "0"));

	wordlist = nvram_safe_get("vlan_tags");
	foreach(word, wordlist, next) {

		char *port = word;
		char *tag = strsep(&port, ">");
		char *prio = port;
		strsep(&prio, ">");

		if (!tag || !port)
			break;
		if (!prio)
			prio = "0";
		char vlan_name[32];

		// sprintf (vlan_name, "%s.%s", tag, port);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(networking.vlan)+document.write(\" %d \")+Capture(networking.iface)</script></div>\n", count);
		sprintf(vlan_name, "vlanifname%d", count);
		showIfOptions(wp, vlan_name, buffer, tag);
		//tag number
		sprintf(vlan_name, "vlantag%d", count);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.tg_number);</script>&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" />\n", vlan_name, port);
		//priority
		sprintf(vlan_name, "vlanprio%d", count);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.prio);</script>&nbsp;");
		showOptions(wp, vlan_name, "0 1 2 3 4 5 6 7", prio);

		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"vlan_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		count++;
	}
	totalcount = count;
	int i;

	for (i = count; i < realcount; i++) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(networking.vlan)+document.write(\" %d \")+Capture(networking.iface)</script></div>\n", i);
		char vlan_name[32];

		sprintf(vlan_name, "vlanifname%d", i);
		showIfOptions(wp, vlan_name, buffer, "");
		sprintf(vlan_name, "vlantag%d", i);
		//tag number
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.tg_number);</script>&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"0\" />\n", vlan_name);
		//priority
		sprintf(vlan_name, "vlanprio%d", i);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.prio);</script>&nbsp;");
		showOptions(wp, vlan_name, "0 1 2 3 4 5 6 7", "0");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"vlan_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("vlan_tagcount", var);
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"vlan_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
}
