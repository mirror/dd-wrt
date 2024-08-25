/*
 * bonding.c
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
EJ_VISIBLE void ej_show_bondings(webs_t wp, int argc, char_t **argv)
{
	char buffer[256];
	char bufferif[512];
	char bondnames[256];
	int count = 0;
	char word[256];
	char *next, *wordlist;

	bzero(buffer, 256);
	bzero(bondnames, 256);
	bzero(bufferif, 512);
	// todo rework that shit. each bond can have a individual type. this should be considered here
	show_caption_pp(wp, NULL, "networking.bonding", "<h2>", "</h2>\n");
	websWrite(wp, "<fieldset>\n");
	show_caption_pp(wp, NULL, "networking.bonding", "<legend>", "</legend>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "networking.bondtype", NULL);
	showOptions(wp, "bonding_type", "balance-rr active-backup balance-xor broadcast 802.3ad balance-tlb balance-alb",
		    nvram_default_get("bonding_type", "balance-rr"));

	show_caption_pp(wp, NULL, "networking.bondpolicy", "&nbsp;", "&nbsp;");
	showOptions(wp, "bonding_policy", "layer2 layer2+3 layer3+4 encap2+3 encap3+4",
		    nvram_default_get("bonding_policy", "layer2"));
	show_caption_pp(wp, NULL, "networking.bondifaces", "&nbsp;", "&nbsp;");
	websWrite(wp, "<input class=\"num\" name=\"bonding_number\" size=\"5\" value=\"%s\" />\n",
		  nvram_default_get("bonding_number", "1"));
	websWrite(wp, "</div>\n");

	getIfListNoPorts(bufferif, "eth");
	int i;

#ifdef HAVE_XSCALE
	bzero(buffer, 256);
	getIfListNoPorts(buffer, "ixp");
	strcat(bufferif, " ");
	strcat(bufferif, buffer);
#endif
	bzero(buffer, 256);
	getIfListB(buffer, NULL, 1, 1, 1);
	strcat(bufferif, " ");
	strcat(bufferif, buffer);

	bzero(buffer, 256);
	getIfListNoPorts(buffer, "vlan");

	strcat(bufferif, " ");
	strcat(bufferif, buffer);

#ifdef HAVE_MADWIFI
	int c = getdevicecount();

	for (i = 0; i < c; i++) {
		char ath[32];

		sprintf(ath, "wlan%d_bridged", i);
		if (nvram_default_matchi(ath, 0, 1)) {
			sprintf(bufferif, "%s wlan%d", bufferif, i);
			char vifs[32];

			sprintf(vifs, "wlan%d_vifs", i);
			strcat(bufferif, " ");
			strcat(bufferif, nvram_safe_get(vifs));
		}
	}
#endif

	for (i = 0; i < nvram_geti("bonding_number"); i++) {
		sprintf(bondnames, "%s bond%d", bondnames, i);
	}
	int totalcount = 0;

	int realcount = nvram_default_geti("bonding_count", 0);

	wordlist = nvram_safe_get("bondings");
	foreach(word, wordlist, next)
	{
		char *port = word;
		char *tag = strsep(&port, ">");

		if (!tag || !port)
			break;
		char vlan_name[32];

		// sprintf (vlan_name, "%s.%s", tag, port);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(networking.bonding) + Capture(\" %d \") + Capture(networking.assign)</script></div>\n",
			count);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.bond)</script>&nbsp;");
		sprintf(vlan_name, "bondingifname%d", count);
		showOptions(wp, vlan_name, bondnames, tag);
		sprintf(vlan_name, "bondingattach%d", count);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.slave)</script>&nbsp;");
		showIfOptions(wp, vlan_name, bufferif, port);
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bond_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			count);
		websWrite(wp, "</div>\n");
		count++;
	}
	totalcount = count;
	for (i = count; i < realcount; i++) {
		char vlan_name[32];

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(networking.bonding) + Capture(\" %d \")  + Capture(networking.iface)</script></div>\n",
			i);
		show_caption_pp(wp, NULL, "networking.bond", "&nbsp;", "&nbsp;");
		sprintf(vlan_name, "bondingifname%d", i);
		showOptions(wp, vlan_name, bondnames, "");
		sprintf(vlan_name, "bondingattach%d", i);
		show_caption_pp(wp, NULL, "networking.slave", "&nbsp;", "&nbsp;");
		showIfOptions(wp, vlan_name, bufferif, "");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bond_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("bonding_count", var);
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"bond_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</fieldset><br />\n");
}
