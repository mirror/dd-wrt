/*
 * bridging.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

EJ_VISIBLE void ej_show_bridgenames(webs_t wp, int argc, char_t **argv)
{
	char buffer[256];
	int count = 0;
	int br0found = 0;
	char word[256];
	char *next, *wordlist;
	char *stp = word;
	char *bridge, *prio, *mtu, *mcast, *mac;
	char bridge_name[32];

	bzero(buffer, 256);
	getIfList(buffer, NULL);
	int realcount = nvram_default_geti("bridges_count", 0);

	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(bridge, word, 0);
		if (bridge && !strcmp(bridge, "br0")) {
			br0found = 1;
			break;
		}
	}

	websWrite(wp, "<table cellspacing=\"5\" summary=\"bridges\" id=\"Bridge_table\" class=\"table\"><thead><tr>\n");
	websWrite(wp, "<th><script type=\"text/javascript\">Capture(nas.raidnametbl)</script></th>\n");
	show_caption_pp(wp, NULL, "networking.stp", "<th>", "</th>\n");
#ifdef HAVE_MSTP
	char *stpoptions = "STP RSTP MSTP Off";
	char *stpoptions_trans[] = { "share.stp", "share.rstp", "share.mstp", "share.off" };
#else
	char *stpoptions = "STP Off";
	char *stpoptions_trans[] = { "share.stp", "share.off" };
#endif
	show_caption_pp(wp, NULL, "networking.snooping", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.prio", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.forward_delay", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.max_age", "<th>", "</th>\n");
	websWrite(wp, "<th>MTU</th>\n");
	websWrite(wp, "<th>Root MAC</th>\n");
	show_caption_pp(wp, NULL, "share.actiontbl", "<th class=\"center\" width=\"10%%\">", "</th></thead>\n");

	if (!br0found) {
		sprintf(bridge_name, "bridgename%d", count);
		websWrite(wp, "<tbody><tr><td><input class=\"num\" name=\"%s\" size=\"3\" value=\"br0\" /></td>\n", bridge_name);

		sprintf(bridge_name, "bridgestp%d", count);
		websWrite(wp, "<td>");
		showOptions_trans(wp, bridge_name, stpoptions, stpoptions_trans, "Off");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgemcastbr%d", count);
		websWrite(wp, "<td>");
		showOptions_trans(wp, bridge_name, "On Off", (char *[]){ "share.on", "share.off" },
				  nvram_default_matchi("br0_mcast", 1, 0) ? "On" : "Off");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgeprio%d", count);
		websWrite(wp, "<td>");
		showOptions(wp, bridge_name,
			    "0 4096 8192 12288 16384 20480 24576 28672 32768 36864 40960 45056 49152 53248 57344 61440", "32768");
		websWrite(wp, "</td>");

		sprintf(bridge_name, "bridgeforward_delay%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"2\" value=\"15\" /></td>\n", bridge_name);
		sprintf(bridge_name, "bridgemax_age%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"2\" value=\"20\" /></td>\n", bridge_name);

		// Bridges are bridges, Ports are ports, show it again HERE
		sprintf(bridge_name, "bridgemtu%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"3\" value=\"1500\" /></td>\n", bridge_name);

		sprintf(bridge_name, "lan_hwaddr");
		websWrite(wp, "<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"16\" value=\"%s\" /></td>\n",
			  bridge_name, nvram_safe_get(bridge_name));

		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" disabled />\");\n//]]>\n</script></td></tr>\n");

		count++;
	}
	foreach(word, wordlist, next)
	{
		int sub = 0;
		if (*word == '>')
			sub = 1;
		GETENTRYBYIDX(bridge, word, 0);
		if (sub)
			bridge = "";
		GETENTRYBYIDX(stp, word, 1 - sub);
		GETENTRYBYIDX(prio, word, 2 - sub);
		GETENTRYBYIDX(mtu, word, 3 - sub);
		if (!mtu) {
			mtu = "1500";
		}
		GETENTRYBYIDX(forward_delay, word, 4 - sub);
		if (!forward_delay) {
			forward_delay = "15";
		}
		GETENTRYBYIDX(max_age, word, 5 - sub);
		if (!max_age) {
			max_age = "20";
		}
		if (!bridge || !stp)
			break;
		sprintf(bridge_name, "bridgename%d", count);
		websWrite(wp, "<tr><td><input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" /></td>\n", bridge_name, bridge);
		sprintf(bridge_name, "bridgestp%d", count);
		websWrite(wp, "<td>");
		showOptions_trans(wp, bridge_name, stpoptions, stpoptions_trans, stp);
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgemcastbr%d", count);
		char mcast[32];
		sprintf(mcast, "%s_mcast", bridge);
		websWrite(wp, "<td>");
		showOptions_trans(wp, bridge_name, "On Off", (char *[]){ "share.on", "share.off" },
				  nvram_default_matchi(mcast, 1, 0) ? "On" : "Off");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgeprio%d", count);
		websWrite(wp, "<td>");
		showOptions(wp, bridge_name,
			    "0 4096 8192 12288 16384 20480 24576 28672 32768 36864 40960 45056 49152 53248 57344 61440",
			    prio != NULL ? prio : "32768");
		websWrite(wp, "</td>");

		sprintf(bridge_name, "bridgeforward_delay%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"2\" value=\"%s\" /></td>\n", bridge_name, forward_delay);
		sprintf(bridge_name, "bridgemax_age%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"2\" value=\"%s\" /></td>\n", bridge_name, max_age);

		// Bridges are bridges, Ports are ports, show it again HERE
		sprintf(bridge_name, "bridgemtu%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" /></td>\n", bridge_name,
			  mtu != NULL ? mtu : "1500");
		if (!strcmp(bridge, "br0")) {
			sprintf(bridge_name, "lan_hwaddr");
			websWrite(wp, "<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"16\" value=\"%s\" /></td>\n",
				  bridge_name, nvram_safe_get(bridge_name));
		} else {
			char macbuf[32];
			char *hwmac = get_hwaddr(bridge, macbuf);
			mac = nvram_nget("%s_hwaddr", bridge);
			if (hwmac && !*(mac))
				nvram_nset(hwmac, "%s_hwaddr", bridge);
			mac = nvram_nget("%s_hwaddr", bridge);
			if (!strcmp(mac, "")) {
				websWrite(wp, "<td class=\"center\">...</td>\n");
			} else {
				websWrite(
					wp,
					"<td class=\"center\"><input class=\"num\" name=\"%s_hwaddr\" size=\"16\" value=\"%s\" /></td>\n",
					bridge, mac);
			}
		}

		if (strcmp(bridge, "br0")) {
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"bridge_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
				count);
			// don't show that here, since that is under Basic Setup
			/*			websWrite(wp, "<tr><td colspan=\"7\" class=\"center\">");
			show_ipnetmask(wp, bridge);
			websWrite(wp, "</td></tr>");*/
		} else {
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" disabled />\");\n//]]>\n</script></td></tr>\n");
		}
		count++;
	}
	int i;
	int totalcount = count;

	for (i = count; i < realcount; i++) {
		sprintf(bridge_name, "bridgename%d", i);
		websWrite(wp, "<tr><td><input class=\"num\" name=\"%s\" size=\"3\" /></td>\n", bridge_name);
		sprintf(bridge_name, "bridgestp%d", i);
		websWrite(wp, "<td>");
		showOptions_trans(wp, bridge_name, stpoptions, stpoptions_trans, "STP");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgemcastbr%d", count);
		websWrite(wp, "<td>");
		showOptions_trans(wp, bridge_name, "On Off", (char *[]){ "share.on", "share.off" }, "Off");
		websWrite(wp, "</td>");
		sprintf(bridge_name, "bridgeprio%d", i);
		websWrite(wp, "<td>");
		showOptions(wp, bridge_name,
			    "0 4096 8192 12288 16384 20480 24576 28672 32768 36864 40960 45056 49152 53248 57344 61440", "32768");
		websWrite(wp, "</td>");

		sprintf(bridge_name, "bridgeforward_delay%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"2\" value=\"15\" /></td>\n", bridge_name);
		sprintf(bridge_name, "bridgemax_age%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"2\" value=\"20\" /></td>\n", bridge_name);

		sprintf(bridge_name, "bridgemtu%d", count);
		websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" /></td>\n", bridge_name, "1500");
		websWrite(wp, "<td></td>");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"bridge_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
			i);
		totalcount++;
	}

	websWrite(wp, "<tr>");
	websWrite(wp, "<td colspan=\"8\"></td>\n");
	websWrite(wp, "<td class=\"center\">\n");

	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"bridge_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	char var[32];
	websWrite(wp, "</td>");
	websWrite(wp, "</tr>");

	sprintf(var, "%d", totalcount);
	nvram_set("bridges_count", var);
	websWrite(wp, "</tbody></table>\n");
}

EJ_VISIBLE void ej_show_bridgetable(webs_t wp, int argc, char_t **argv)
{
	FILE *f;
	char buf[512];
	char brname[32];
	char brstp[8];
	char brif[16];
	int count = 0;

	if ((f = popen("brctl show", "r")) != NULL) {
		while (fgets(buf, sizeof(buf), f)) {
			if (count) // skip line 0
			{
				strcpy(brname, "");
				strcpy(brstp, "");
				strcpy(brif, "");

				if (strncmp(buf, "\t\t\t", 3) != 0) {
					if (count != 1)
						websWrite(wp, "\',"); // close
					sscanf(buf, "%s %*s %s %s", brname, brstp, brif);
					websWrite(wp, "\'%s\',\'%s\',\'%s ", brname, brstp, brif);
				} else {
					sscanf(buf, "%s", brif);
					websWrite(wp, "%s ", brif);
				}
			}

			count++;
		}

		websWrite(wp, "\'"); // close
		pclose(f);
	}

	return;
}

static void show_bridgeifname(webs_t wp, char *bridges, char *devs, int count, char *bridge, char *port, char *stp, char *prio,
			      char *hairpin, char *cost)
{
	char vlan_name[32];
	websWrite(wp, "<tr>\n");
	websWrite(wp, "<td>");
	sprintf(vlan_name, "bridge%d", count);
	showIfOptions(wp, vlan_name, bridges, bridge);
	websWrite(wp, "</td>");

	websWrite(wp, "<td>");
	sprintf(vlan_name, "bridgeif%d", count);
	showIfOptions(wp, vlan_name, devs, port);
	websWrite(wp, "</td>");

#ifdef HAVE_MSTP
	int hasstp;
	char word[256];
	if (bridge && *(bridge))
		hasstp = getBridgeSTP(bridge, word);
	else
		hasstp = 0;
	websWrite(wp, "<td>");
	sprintf(vlan_name, "bridgeifstp%d", count);
	if (hasstp) {
		showOptions_trans(wp, vlan_name, "On Off", (char *[]){ "share.on", "share.off" },
				  stp ? (!strcmp(stp, "1") ? "On" : "Off") : "On");
	} else {
		showOptions_ext_trans(wp, vlan_name, "Off", (char *[]){ "share.off" }, "Off", 1);
	}
	websWrite(wp, "</td>");
#endif
	sprintf(vlan_name, "bridgeifprio%d", count);
	websWrite(wp, "<td>");
	showOptions(wp, vlan_name, "0 16 32 48 64 80 96 112 128 144 160 176 192 208 224 240", prio != NULL ? prio : "128");
	websWrite(wp, "</td>");

	sprintf(vlan_name, "bridgeifcost%d", count);
	websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" /></td>\n", vlan_name, cost ? cost : "100");

	websWrite(wp, "<td class=\"center\">");
	sprintf(vlan_name, "bridgeifhairpin%d", count);
	websWrite(wp, "<input type=\"checkbox\" name=\"%s\" value=\"1\" %s/>\n", vlan_name,
		  (hairpin && !strcmp(hairpin, "1")) ? "checked=\"checked\"" : "");
	websWrite(wp, "</td>");

	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"bridgeif_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
		count);
	websWrite(wp, "</td>");
	websWrite(wp, "</tr>\n");
}

EJ_VISIBLE void ej_show_bridgeifnames(webs_t wp, int argc, char_t **argv)
{
	char bufferif[512];
	char finalbuffer[512];
	int count = 0;
	int c = 0;
	char word[256];
	char *next, *wordlist;
	bzero(bufferif, 512);
	bzero(finalbuffer, 512);
	getIfList(bufferif, NULL);
	foreach(word, bufferif, next)
	{
		if (!isbridge(word) && strcmp(word, "lo") && !strchr(word, ':')) {
			strcat(finalbuffer, " ");
			strcat(finalbuffer, word);
		}
	}
	strcpy(bufferif, finalbuffer);
	int i;

	bzero(finalbuffer, 512);
	getIfListB(finalbuffer, NULL, 1, 1, 0);
	char *checkbuffer = calloc(strlen(finalbuffer) + 6, 1);
	strcpy(checkbuffer, "none ");
	strcat(checkbuffer, finalbuffer);
	strcpy(finalbuffer, checkbuffer);
	debug_free(checkbuffer);
	int realcount = nvram_default_geti("bridgesif_count", 0);
	websWrite(
		wp,
		"<table cellspacing=\"5\" summary=\"bridgeassignments\" id=\"bridgeassignments_table\" class=\"table\"><thead><tr>\n");
	show_caption_pp(wp, NULL, "networking.assign", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.iface", "<th>", "</th>\n");
#ifdef HAVE_MSTP
	show_caption_pp(wp, NULL, "networking.stp", "<th>", "</th>\n");
#endif
	show_caption_pp(wp, NULL, "networking.prio", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.pathcost", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.hairpin", "<th class=\"center\">", "</th>\n");
	show_caption_pp(wp, NULL, "share.actiontbl", "<th class=\"center\" width=\"10%%\">", "</th></thead>\n");

	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		GETENTRYBYIDX(hairpin, word, 3);
		GETENTRYBYIDX(stp, word, 4);
		GETENTRYBYIDX(pathcost, word, 5);
		if (!hairpin)
			hairpin = "0";
		if (!stp)
			stp = "1";
		if (!pathcost)
			pathcost = "100";

		show_bridgeifname(wp, finalbuffer, bufferif, count, tag, port, stp, prio, hairpin, pathcost);
		count++;
	}
	int totalcount = count;

	for (i = count; i < realcount; i++) {
		show_bridgeifname(wp, finalbuffer, bufferif, i, "", "", NULL, NULL, NULL, NULL);
		totalcount++;
	}
	websWrite(wp, "<tbody><tr>\n");
	websWrite(wp, "<td colspan=\"2\">&nbsp;</td>\n");
#ifdef HAVE_MSTP
	websWrite(wp, "<td>&nbsp;</td>\n");
#endif
	websWrite(wp, "<td colspan=\"3\">&nbsp;</td>\n");
	websWrite(wp, "<td class=\"center\">\n");
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"bridgeif_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</td>\n");
	websWrite(wp, "</tr>\n");
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("bridgesif_count", var);
	websWrite(wp, "</tbody></table>\n");
}
