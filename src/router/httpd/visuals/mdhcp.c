/*
 * mdhcp.c
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

EJ_VISIBLE void ej_show_mdhcp(webs_t wp, int argc, char_t **argv)
{
	char buffer[256];
	char buf[128];
	int count = 0;
	char word[256];
	char *next, *wordlist;

	websWrite(wp, "<h2>%s</h2>\n<fieldset>\n",
		  tran_string(buf, sizeof(buf), "networking.h5"));
	websWrite(wp, "<legend>%s</legend>\n",
		  tran_string(buf, sizeof(buf), "networking.legend5"));

	websWrite(
		wp,
		"<table id=\"mdhcp_table\" class=\"table\" summary=\"mdhcp\" cellspacing=\"7\"><thead><tr>\n");
	show_caption_pp(wp, NULL, "share.ip", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.iface", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "share.enable", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "share.start", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.max", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.leasetime", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "share.actiontbl",
			"<th class=\"center\" width=\"10%%\">",
			"</th></thead><tbody>\n");
	bzero(buffer, 256);
	getIfListNoPorts(buffer, NULL);
	int totalcount = 0;
	int realcount = nvram_default_geti("mdhcpd_count", 0);

	wordlist = nvram_safe_get("mdhcpd");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(interface, word, 0);
		GETENTRYBYIDX(dhcpon, word, 1);
		GETENTRYBYIDX(start, word, 2);
		GETENTRYBYIDX(max, word, 3);
		GETENTRYBYIDX(leasetime, word, 4);
		if (leasetime == NULL) {
			leasetime = "3660";
		}
		if (!interface || !start || !dhcpon || !max || !leasetime)
			break;
		char vlan_name[32];

		// interface
		char *ipaddr = nvram_nget("%s_ipaddr", interface);
		char *netmask = nvram_nget("%s_netmask", interface);
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td>\n");
		if (*ipaddr && *netmask) {
			websWrite(wp, "%s/%d\n", ipaddr, getmask(netmask));
		} else {
			char buf[128];
			websWrite(wp, "%s",
				  tran_string(buf, sizeof(buf), "share.none"));
		}
		websWrite(wp, "</td>\n");
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpifname%d", count);
		showIfOptions(wp, vlan_name, buffer, interface);
		websWrite(wp, "</td>\n");
		// on off
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpon%d", count);
		showOptions_trans(wp, vlan_name, "On fwd Off",
				  (char *[]){ "share.on", "share.forwarding",
					      "share.off" },
				  dhcpon);
		websWrite(wp, "</td>\n");
		// start
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpstart%d", count);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n",
			vlan_name, start);
		websWrite(wp, "</td>\n");
		// max
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpmax%d", count);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n",
			vlan_name, max);
		websWrite(wp, "</td>\n");
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpleasetime%d", count);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n",
			vlan_name, leasetime);
		websWrite(
			wp,
			"&nbsp;<script type=\"text/javascript\">Capture(share.minutes)</script></td>\n");
		//
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.del + \"\\\" onclick=\\\"mdhcp_del_submit(this.form,%d)\\\" /></td>\");\n//]]>\n</script>\n",
			count);
		websWrite(wp, "</tr>\n");
		count++;
	}
	totalcount = count;
	int i;

	for (i = count; i < realcount; i++) {
		char vlan_name[32];
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td>\n");
		{
			char buf[128];
			websWrite(wp, "%s",
				  tran_string(buf, sizeof(buf), "share.none"));
		}
		websWrite(wp, "</td>\n");
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpifname%d", totalcount);
		showIfOptions(wp, vlan_name, buffer, "");
		websWrite(wp, "</td>\n");
		// on off
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpon%d", totalcount);
		showOptions_trans(wp, vlan_name, "On fwd Off",
				  (char *[]){ "share.on", "share.forwarding",
					      "share.off" },
				  "");
		websWrite(wp, "</td>\n");
		// start
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpstart%d", totalcount);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n",
			vlan_name, "100");
		websWrite(wp, "</td>\n");
		// max
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpmax%d", totalcount);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n",
			vlan_name, "50");
		websWrite(wp, "</td>\n");
		websWrite(wp, "<td>\n");
		sprintf(vlan_name, "mdhcpleasetime%d", totalcount);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n",
			vlan_name, "1440");
		websWrite(
			wp,
			"&nbsp;<script type=\"text/javascript\">Capture(share.minutes)</script></td>\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.del + \"\\\" onclick=\\\"mdhcp_del_submit(this.form,%d)\\\" /></td>\");\n//]]>\n</script>\n",
			i);
		websWrite(wp, "</tr>\n");
		totalcount++;
	}
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("mdhcpd_count", var);
	websWrite(wp, "<tr>\n");
	websWrite(wp, "<td colspan=\"6\">&nbsp;</td>\n");
	websWrite(wp, "<td class=\"center\">\n");
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"mdhcp_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</td>\n");
	websWrite(wp, "</tr>\n");
	websWrite(wp, "</tbody></table>\n");
	websWrite(wp, "</fieldset><br />\n");
}
