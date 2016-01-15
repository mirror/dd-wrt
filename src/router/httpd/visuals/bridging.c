/*
 * bridging.c
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
void ej_show_bridgenames(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	int count = 0;
	int br0found = 0;
	char word[256];
	char *next, *wordlist;
	char *stp = word;
	char *bridge, *prio, *mtu, *mcast, *mac;
	char bridge_name[32];

	memset(buffer, 0, 256);
	getIfList(buffer, NULL);
	int realcount = atoi(nvram_default_get("bridges_count", "0"));

	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next) {
		bridge = strsep(&stp, ">");
		if (!strcmp(bridge, "br0")) {
			br0found = 1;
			break;
		}
	}

	websWrite(wp, "<table cellspacing=\"5\" summary=\"bridges\" id=\"Bridge_table\" class=\"table center\"><tr>\n");
	websWrite(wp, "<th>Name</th>\n");
#ifdef HAVE_MSTP
	websWrite(wp, "<th>MSTP</th>\n");
#else
	websWrite(wp, "<th>STP</th>\n");
#endif
	websWrite(wp, "<th><script type=\"text/javascript\">Capture(networking.snooping)</script></th>\n");
	websWrite(wp, "<th><script type=\"text/javascript\">Capture(networking.prio)</script></th>\n");
	websWrite(wp, "<th>MTU</th>\n");
	websWrite(wp, "<th>Root MAC</th>\n");
	websWrite(wp, "<th>&nbsp;</th></tr>\n");

	if (!br0found) {

		sprintf(bridge_name, "bridgename%d", count);
		websWrite(wp, "<tr><td><input class=\"num\" name=\"%s\"size=\"3\" value=\"br0\" /></td>\n", bridge_name);

		sprintf(bridge_name, "bridgestp%d", count);
		websWrite(wp, "<td>");
		showOptions(wp, bridge_name, "On Off", "Off");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgemcastbr%d", count);
		websWrite(wp, "<td>");
		showOptions(wp, bridge_name, "On Off", nvram_default_match("br0_mcast", "1", "0") ? "On" : "Off");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgeprio%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\"size=\"5\" value=\"32768\" /></td>\n", bridge_name);
		// Bridges are bridges, Ports are ports, show it again HERE          
		sprintf(bridge_name, "bridgemtu%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\"size=\"3\" value=\"1500\" /></td>\n", bridge_name);

		sprintf(bridge_name, "lan_hwaddr");
		websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"16\" value=\"%s\" /></td>\n", bridge_name, nvram_safe_get(bridge_name));

		websWrite(wp,
			  "<td><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" disabled />\");\n//]]>\n</script></td></tr>\n");

		// don't show that here, since that is under Basic Setup
		// show_ipnetmask(wp, bridge);
		count++;
	}

	foreach(word, wordlist, next) {

		stp = word;
		bridge = strsep(&stp, ">");
		prio = stp;

		stp = strsep(&prio, ">");
		mtu = prio;

		prio = strsep(&mtu, ">");
		if (!prio) {
			prio = mtu;
			mtu = "1500";
		}

/*	char *stp = word;
	char *bridge = strsep( &stp, ">" );
	char *mtu = stp;
	char *prio = strsep( &mtu, ">" );
*/
		if (!bridge || !stp)
			break;

		sprintf(bridge_name, "bridgename%d", count);
		websWrite(wp, "<tr><td><input class=\"num\" name=\"%s\"size=\"3\" value=\"%s\" /></td>\n", bridge_name, bridge);
		sprintf(bridge_name, "bridgestp%d", count);
		websWrite(wp, "<td>");
		showOptions(wp, bridge_name, "On Off", stp);
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgemcastbr%d", count);
		char mcast[32];
		sprintf(mcast, "%s_mcast", bridge);
		websWrite(wp, "<td>");
		showOptions(wp, bridge_name, "On Off", nvram_default_match(mcast, "1", "0") ? "On" : "Off");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgeprio%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" /></td>\n", bridge_name, prio != NULL ? prio : "32768");
		// Bridges are bridges, Ports are ports, show it again HERE          
		sprintf(bridge_name, "bridgemtu%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\"size=\"3\" value=\"%s\" /></td>\n", bridge_name, mtu != NULL ? mtu : "1500");
		if (!strcmp(bridge, "br0")) {
			sprintf(bridge_name, "lan_hwaddr");
			websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"16\" value=\"%s\" /></td>\n", bridge_name, nvram_safe_get(bridge_name));
		} else {
			sprintf(bridge_name, "%s_hwaddr", bridge);
			mac = nvram_safe_get(bridge_name);
			if (!strcmp(mac, "")) {
				websWrite(wp, "<td align=\"center\">...</td>\n");
			} else {
				websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"16\" value=\"%s\" /></td>\n", bridge_name, nvram_safe_get(bridge_name));
			}
		}

		if (strcmp(bridge, "br0")) {
			websWrite(wp,
				  "<td><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bridge_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
				  count);
			// don't show that here, since that is under Basic Setup
			websWrite(wp, "<tr><td colspan=\"7\" align=\"center\">");
			show_ipnetmask(wp, bridge);
			websWrite(wp, "</td></tr>");
		} else {
			websWrite(wp,
				  "<td><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" disabled />\");\n//]]>\n</script></td></tr>\n");
		}
		count++;
	}
	int i;
	int totalcount = count;

	for (i = count; i < realcount; i++) {

		sprintf(bridge_name, "bridgename%d", i);
		websWrite(wp, "<tr><td><input class=\"num\" name=\"%s\"size=\"3\" /></td>\n", bridge_name);
		sprintf(bridge_name, "bridgestp%d", i);
		websWrite(wp, "<td>");
		showOptions(wp, bridge_name, "On Off", "On");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgemcastbr%d", count);
		websWrite(wp, "<td>");
		showOptions(wp, bridge_name, "On Off", "Off");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgeprio%d", i);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" /></td>\n", bridge_name, "32768");
		sprintf(bridge_name, "bridgemtu%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\"size=\"3\" value=\"%s\" /></td>\n", bridge_name, "1500");
		websWrite(wp, "<td></td><td>");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bridge_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
			  i);
		totalcount++;
	}
	websWrite(wp, "</table>");

	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"bridge_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("bridges_count", var);
}

void ej_show_bridgetable(webs_t wp, int argc, char_t ** argv)
{

	FILE *f;
	char buf[128];
	char brname[32];
	char brstp[8];
	char brif[16];
	int count = 0;

	system2("brctl show > /tmp/.brtable");

	if ((f = fopen("/tmp/.brtable", "r")) != NULL) {

		while (fgets(buf, sizeof(buf), f)) {

			if (count)	// skip line 0
			{
				strcpy(brname, "");
				strcpy(brstp, "");
				strcpy(brif, "");

				if (strncmp(buf, "\t\t\t", 3) != 0) {
					if (count != 1)
						websWrite(wp, "\',");	// close
					sscanf(buf, "%s %*s %s %s", brname, brstp, brif);
					websWrite(wp, "\'%s\',\'%s\',\'%s ", brname, brstp, brif);
				} else {
					sscanf(buf, "%s", brif);
					websWrite(wp, "%s ", brif);
				}
			}
			count++;
		}

		websWrite(wp, "\'");	// close
		fclose(f);
		unlink("/tmp/.brtable");
	}
	return;
}

void ej_show_bridgeifnames(webs_t wp, int argc, char_t ** argv)
{
	char bufferif[512];
	char bufferif2[256];
	char finalbuffer[512];
	int count = 0;
	int c = 0;
	char word[256];
	char *next, *wordlist;

	memset(bufferif, 0, 512);
	memset(bufferif2, 0, 256);
	getIfList(bufferif, "eth");
#ifdef HAVE_GATEWORX
	getIfList(bufferif2, "ixp");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);
#endif

	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "vlan");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);

	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "wl");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);

	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "ofdm");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);

#ifdef HAVE_RT2880
	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "ra");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);
#endif

#ifdef HAVE_MADWIFI
	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "ath");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);
#endif
	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "br");
	foreach(word, bufferif2, next) {
		if (contains(word, '.'))
			sprintf(bufferif, "%s %s", bufferif, word);
	}
	int i;

#if 0				//def HAVE_MADWIFI
	c = getdevicecount();

	for (i = 0; i < c; i++) {
		char ath[32];

		sprintf(bufferif, "%s ath%d", bufferif, i);
		char vifs[32];

		sprintf(vifs, "ath%d_vifs", i);
		sprintf(bufferif, "%s %s", bufferif, nvram_safe_get(vifs));
	}
#endif
#ifdef HAVE_BONDING
	c = atoi(nvram_default_get("bonding_number", "1"));
	for (i = 0; i < c; i++) {
		sprintf(bufferif, "%s bond%d", bufferif, i);
	}
#endif
#ifdef HAVE_EOP_TUNNEL
	for (i = 1; i < 11; i++) {
		char EOP[32];

		if (nvram_nmatch("1", "oet%d_en", i)
		    && nvram_nmatch("1", "oet%d_bridged", i)) {
			sprintf(EOP, "oet%d", i);
			sprintf(bufferif, "%s %s", bufferif, EOP);
		}
	}
#endif
	char buffer[256];

	memset(buffer, 0, 256);
	getIfList(buffer, "br");

	memset(finalbuffer, 0, 256);
	foreach(word, buffer, next) {
		if (!contains(word, '.'))
			sprintf(finalbuffer, "%s %s", finalbuffer, word);
	}
	char *checkbuffer = safe_malloc(strlen(finalbuffer) + 6);
	memset(checkbuffer, 0, strlen(finalbuffer) + 6);
	strcpy(checkbuffer, "none ");
	strcat(checkbuffer, finalbuffer);
	strcpy(finalbuffer, checkbuffer);
	free(checkbuffer);
	int realcount = atoi(nvram_default_get("bridgesif_count", "0"));

	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next) {
		char *port = word;
		char *tag = strsep(&port, ">");
		char *prio = port;

		strsep(&prio, ">");
		if (!tag || !port)
			break;
		char vlan_name[32];

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<script type=\"text/javascript\">Capture(networking.assign);</script> %d\n", count);
		sprintf(vlan_name, "bridge%d", count);
		showIfOptions(wp, vlan_name, finalbuffer, tag);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.bridge);</script><script type=\"text/javascript\">Capture(networking.iface);</script>&nbsp;");
		sprintf(vlan_name, "bridgeif%d", count);
		showIfOptions(wp, vlan_name, bufferif, port);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.prio);</script>&nbsp;");
		sprintf(vlan_name, "bridgeifprio%d", count);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"3\" value=\"%s\" />\n", vlan_name, prio != NULL ? prio : "63");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bridgeif_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		count++;
	}
	int totalcount = count;

	for (i = count; i < realcount; i++) {
		char vlan_name[32];

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<script type=\"text/javascript\">Capture(networking.assign)</script> %d\n", i);
		sprintf(vlan_name, "bridge%d", i);
		showIfOptions(wp, vlan_name, finalbuffer, "");
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.iface)</script>&nbsp;");
		sprintf(vlan_name, "bridgeif%d", i);
		showIfOptions(wp, vlan_name, bufferif, "");
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.prio)</script>&nbsp;");
		sprintf(vlan_name, "bridgeifprio%d", i);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" />\n", vlan_name, "63");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bridgeif_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"bridgeif_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("bridgesif_count", var);
}

