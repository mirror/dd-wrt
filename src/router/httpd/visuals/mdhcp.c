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

EJ_VISIBLE void ej_show_mdhcp(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	char buf[128];
	int count = 0;
	char word[256];
	char *next, *wordlist;

	websWrite(wp, "<h2>%s</h2>\n<fieldset>\n", tran_string(buf, "networking.h5"));
	websWrite(wp, "<legend>%s</legend>\n", tran_string(buf, "networking.legend5"));

	bzero(buffer, 256);
	getIfList(buffer, NULL);
	int totalcount = 0;
	int realcount = nvram_default_geti("mdhcpd_count", 0);

	wordlist = nvram_safe_get("mdhcpd");
	foreach(word, wordlist, next) {
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

		if (*ipaddr && *netmask) {
			show_caption_simple(wp, "networking.iface");
			websWrite(wp, " %s: IP %s/%s\n", getNetworkLabel(wp, interface), ipaddr, netmask);
		}
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "DHCP %d &nbsp;\n", count);
		sprintf(vlan_name, "mdhcpifname%d", count);
		showIfOptions(wp, vlan_name, buffer, interface);
		// on off
		sprintf(vlan_name, "mdhcpon%d", count);
		showOptions(wp, vlan_name, "On Off", dhcpon);
		// start
		sprintf(vlan_name, "mdhcpstart%d", count);
		show_caption_pp(wp, NULL, "share.start", "&nbsp;", "&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, start);
		// max
		sprintf(vlan_name, "mdhcpmax%d", count);
		show_caption_pp(wp, NULL, "networking.max", "&nbsp;", "&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, max);
		sprintf(vlan_name, "mdhcpleasetime%d", count);
		show_caption_pp(wp, NULL, "networking.leasetime", "&nbsp;", "&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n", vlan_name, leasetime);
		// 
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"mdhcp_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		count++;
	}
	totalcount = count;
	int i;

	for (i = count; i < realcount; i++) {
		char vlan_name[32];

		// sprintf (mdhcp_name, "%s.%s", tag, port);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "DHCP %d\n", totalcount);
		// interface
		sprintf(vlan_name, "mdhcpifname%d", totalcount);
		showIfOptions(wp, vlan_name, buffer, "");
		// on off
		sprintf(vlan_name, "mdhcpon%d", totalcount);
		showOptions(wp, vlan_name, "On Off", "");
		// start
		sprintf(vlan_name, "mdhcpstart%d", totalcount);
		websWrite(wp, "&nbsp;Start&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, "100");
		// max
		sprintf(vlan_name, "mdhcpmax%d", totalcount);
		websWrite(wp, "&nbsp;Max&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, "50");
		sprintf(vlan_name, "mdhcpleasetime%d", totalcount);
		websWrite(wp, "&nbsp;Leasetime&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n", vlan_name, "1440");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"mdhcp_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("mdhcpd_count", var);
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"mdhcp_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</fieldset><br />\n");

}
