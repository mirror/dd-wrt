/*
 * switch.c
 *
 * Copyright (C) 2005 - 2019 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
 * todo stylesheet compatible code 
 */
/*
 * lonewolf additions 
 */

// Note that there is no VLAN #16.  It's just a convieniant way of denoting a
// "Tagged" port
EJ_VISIBLE void ej_port_vlan_table(webs_t wp, int argc, char_t **argv)
{
	/*
	 * vlans[x][y] where x 0-15 are VLANS x 16 is tagging, 17 is
	 * auto-negotiation, 18 is 100/10 Mbit, and 19 is Full/Half duplex y 0-4
	 * are switch ports (port 5 is set automaticly) y 5 it the bridge device
	 * (x 16 dosn't apply) 
	 */

	int i, j, *vlans[10], tmp, wl_br;
	char *c, *next, buff[32], portvlan[32];
	int a, *vlanlist;
	int lanports = 4;
	int cpuports = 0;
	if (nvram_exists("sw_lan6"))
		lanports = 6;

	if (*nvram_safe_get("sw_lan1")) {
		if (!*nvram_safe_get("sw_lan4"))
			lanports = 3;
		if (!*nvram_safe_get("sw_lan3"))
			lanports = 2;
		if (!*nvram_safe_get("sw_lan2"))
			lanports = 1;
	}
	if (*nvram_safe_get("sw_lancpuport") && *nvram_safe_get("sw_wancpuport"))
		cpuports = 2;
	else if (*nvram_safe_get("sw_cpuport"))
		cpuports = 1;
	int blen = nvram_default_geti("portvlan_count", 3);
	char *deflist = malloc((blen * 5) + 1);
	for (i = 0; i < 10; i++)
		vlans[i] = malloc(sizeof(int) * (blen + 9));
	vlanlist = malloc(sizeof(int) * (blen + 17));

	if (getRouterBrand() == ROUTER_UBNT_UNIFIAC)
		lanports = 2;
	if (getRouterBrand() == ROUTER_UBNT_NANOAC)
		lanports = 2;
	c = nvram_safe_get("portvlanlist");
	for (i = 0; i < blen; i++)
		vlanlist[i] = i;
	i = 0;
	foreach(portvlan, c, next)
	{
		vlanlist[i++] = atoi(portvlan);
	}
	for (i = 0; i < blen + 9; i++)
		for (j = 0; j < lanports + 2 + cpuports; j++)
			vlans[j][i] = -1;

	wl_br = -1;

	for (i = 0; i < lanports + 2 + cpuports; i++) {
		if (i < (lanports + 1 + cpuports))
			snprintf(buff, 31, "port%dvlans", i);
		else if (i == 5)
			snprintf(buff, 31, "%s", "lan_ifnames");
		else
			snprintf(buff, 31, "ub%d_ifnames", i - 5);

		c = nvram_safe_get(buff);

		if (c) {
			foreach(portvlan, c, next)
			{
				if (portvlan[0] == 'e' && portvlan[1] == 't' && portvlan[2] == 'h' && portvlan[3] == '1')
					wl_br = i - (lanports + 1);
				if (ISDIGIT(portvlan, 1) ||
				    (portvlan[0] == 'v' && portvlan[1] == 'l' && portvlan[2] == 'a' && portvlan[3] == 'n')) {
					if (ISDIGIT(portvlan, 1))
						tmp = atoi(portvlan);
					else {
						portvlan[0] = portvlan[4];
						portvlan[1] = portvlan[5];
						portvlan[2] = '\0';
						if (ISDIGIT(portvlan, 1))
							tmp = atoi(portvlan);
						else
							continue;
					}
					if (tmp >= 16000) {
						tmp = blen + ((tmp - 16000) / 1000);
					}
					if (i < lanports + 1 + cpuports) {
						vlans[i][tmp] = 1;
						//fprintf(stderr, "assign port %d flag %d\n", i, tmp);
					} else {
						vlans[lanports + 1 + cpuports][tmp] = i - (lanports + 1 + cpuports);
					}
				}
			}
		}
	}

	int nowan = 0;
#ifdef HAVE_SWCONFIG
	nowan = nvram_match("sw_wan", "-1");
	websWrite(wp, "<tr>\n");
	websWrite(wp, "<th rowspan=\"2\"><script type=\"text/javascript\">Capture(vlan.legend)</script></th>\n");
	websWrite(wp, "<th colspan=\"%d\" class=\"center\"><script type=\"text/javascript\">Capture(share.port)</script></th>\n",
		  lanports + cpuports + !nowan);
	websWrite(
		wp,
		"<th rowspan=\"2\" class=\"center\" width=\"10%%\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n");
	websWrite(wp, "</tr>\n");
	websWrite(wp, "<tr>\n");
	if (cpuports == 2) {
		websWrite(wp, "<th class=\"center\">WAN CPUPORT</th>\n");
		websWrite(wp, "<th class=\"center\">LAN CPUPORT</th>\n");
	} else if (cpuports == 1) {
		websWrite(wp, "<th class=\"center\">CPUPORT</th>\n");
	}
	if (!nowan)
		websWrite(wp, "<th class=\"center\">WAN</th>\n");
	for (a = 1; a < lanports + 1; a++) {
		websWrite(wp, "<th class=\"center\">%d</th>\n", a);
	}
	websWrite(wp, "</tr>\n");

	websWrite(wp, "<tr>\n");
	websWrite(wp, "<td>&nbsp;</td>\n");
	if (cpuports == 2) {
		websWrite(wp, "<td class=\"status_green center\">%d</td>\n", getPortStatus(nvram_geti("sw_wancpuport")));
		websWrite(wp, "<td class=\"status_green center\">%d</td>\n", getPortStatus(nvram_geti("sw_lancpuport")));
	} else if (cpuports == 1) {
		websWrite(wp, "<td class=\"status_green center\">%d</td>\n", getPortStatus(nvram_geti("sw_cpuport")));
	}
	for (a = nowan; a < lanports + 1; a++) {
		int status = 0;
		if (a == 0)
			status = getPortStatus(nvram_geti("sw_wan"));
		else
			status = getPortStatus(nvram_ngeti("sw_lan%d", a));

		char cstatus[32];
		if (status < 10)
			sprintf(cstatus, "status_red");
		else if (status == 10)
			sprintf(cstatus, "status_orange");
		else if (status == 100)
			sprintf(cstatus, "status_yellow");
		else if (status == 1000)
			sprintf(cstatus, "status_green");
		else
			sprintf(cstatus, "status_red");
		if (status >= 10)
			websWrite(wp, "<td class=\"%s center\">%d</td>\n", cstatus, status);
		else
			websWrite(wp,
				  "<td class=\"%s center\"><script type=\"text/javascript\">Capture(share.down)</script></td>\n",
				  cstatus);
	}

	websWrite(wp, "<td></td>\n");
	websWrite(wp, "</tr>\n");
#else

	websWrite(wp, "<tr>\n");
	websWrite(wp, "<th rowspan=\"2\"><script type=\"text/javascript\">Capture(vlan.legend)</script></th>\n");
	websWrite(wp, "<th colspan=\"5\" class=\"center\"><script type=\"text/javascript\">Capture(share.port)</script></th>\n");
	websWrite(wp,
		  "<th rowspan=\"2\" class=\"center\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n");
	websWrite(wp, "</tr>\n");
	websWrite(wp, "<tr>\n");
	websWrite(wp, "<th class=\"center\">WAN</th>\n");
	websWrite(wp, "<th class=\"center\">1</th>\n");
	websWrite(wp, "<th class=\"center\">2</th>\n");
	websWrite(wp, "<th class=\"center\">3</th>\n");
	websWrite(wp, "<th class=\"center\">4</th>\n");
	websWrite(wp, "</tr>\n");

	// Status header
	char status[32];

	char *ifname = "eth0";
	if (f_exists("/proc/switch/eth0/enable"))
		ifname = "eth0";
	if (f_exists("/proc/switch/eth1/enable"))
		ifname = "eth1";
	if (f_exists("/proc/switch/eth2/enable"))
		ifname = "eth2";
	char portstatus[64];
	snprintf(portstatus, sizeof(portstatus), "/proc/switch/%s/port/0/status", ifname);
	FILE *fp = fopen(portstatus, "rb");
	if (fp) {
		fclose(fp);

		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td>&nbsp;</td>\n");

		int vlanmap[6] = { 0, 1, 2, 3, 4, 5 }; // 0=wan; 1,2,3,4=lan; 5=internal
		getPortMapping(vlanmap);

		for (a = 0; a < 5; a++) {
			snprintf(portstatus, sizeof(portstatus), "/proc/switch/%s/port/%d/status", ifname, vlanmap[a]);
			char cstatus[32];
			bzero(cstatus, 32);
			FILE *fp = fopen(portstatus, "rb");
			if (fp) {
				fgets(cstatus, 31, fp);
				fclose(fp);
			}
			int speed = 0;
			if (!strncmp(cstatus, "disc", 4))
				sprintf(status, "status_red");

			if (!strncmp(cstatus, "10", 3)) {
				speed = 10;
				sprintf(status, "status_orange");
			}

			if (!strncmp(cstatus, "100", 3)) {
				speed = 100;
				sprintf(status, "status_yellow");
			}

			if (!strncmp(cstatus, "1000", 4)) {
				speed = 1000;
				sprintf(status, "status_green");
			}
			if (speed)
				websWrite(wp, "<td class=\"%s center\">%d</td>\n", status, speed);
			else
				websWrite(wp, "<td class=\"%s center\">down</td>\n", status);
		}
		websWrite(wp, "<td></td>\n");
		websWrite(wp, "</tr>\n");
	}
#endif
	int len = blen;
#if defined(HAVE_SWCONFIG) && !defined(HAVE_ALPINE)
	if (has_igmpsnooping())
		len += 3;
#endif
#if !defined(HAVE_ALPINE)
	len += 6;
#endif
	for (i = 0; i < len; i++) {
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td>");
		int flag = i;
		if (i >= blen) {
			flag = ((i - blen) * 1000) + 16000;
		}

		switch (flag) {
		case 16000:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.tagged)</script>");
			break;
		case 17000:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.negociate)</script>");
			break;
		case 18000:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.gigabit)</script>");
			break;
		case 19000:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.fullspeed)</script>");
			break;
		case 20000:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.fullduplex)</script>");
			break;
		case 21000:
			websWrite(wp, "<script type=\"text/javascript\">Capture(share.enabled)</script>");
			break;
		case 22000:
			websWrite(wp, "<script type=\"text/javascript\">Capture(networking.snooping)</script>");
			break;
		case 23000:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.eee)</script>");
			break;
		case 24000:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.flow)</script>");
			break;
		default:
			if (!i)
				snprintf(deflist, (blen * 5) + 1, "%d", i);
			else
				snprintf(deflist, (blen * 5) + 1, "%s %d", deflist, i);
			websWrite(wp, "<input class=\"num\" maxlength=\"4\" size=\"4\" name=\"portvlan%dlist\" value=\"%d\" />", i,
				  vlanlist[i]);
			break;
		}

		websWrite(wp, "</td>\n");
		int j1;
		for (j1 = nowan; j1 < lanports + 1 + cpuports; j1++) {
			if (cpuports == 1) {
				if (j1 == nowan)
					j = lanports + 1;
				else
					j = j1 - 1;
			} else if (cpuports == 2) {
				if (j1 == nowan)
					j = lanports + 1;
				else if (j1 == nowan + 1)
					j = lanports + 2;
				else
					j = j1 - 2;

			} else
				j = j1;
			//                      fprintf(stderr, "port %d %d\n", j, vlans[j][i]);
			if (i >= blen)
				snprintf(buff, 31, "\"port%dvlan%d\"", j, ((i - blen) * 1000) + 16000);
			else
				snprintf(buff, 31, "\"port%dvlan%d\"", j, i);
			// todo. disable config fields for cpu ports
			websWrite(wp, "<td");

			if (j1 % 2 == 0)
				websWrite(wp, " class=\"odd\"");
			if (j > lanports && flag > 16000 && flag != 22000) {
				websWrite(wp, " height=\"20\">&nbsp</td>\n");
				continue;
			}
			char aria[64];
			if (flag < 17000) {
				sprintf(aria, "%s %d %s %d", live_translate(wp, "share.port"), j, live_translate(wp, "vlan.legend"),
					i);
			} else {
				if (flag == 16000)
					snprintf(aria, sizeof(aria), "%s %d %s", live_translate(wp, "share.port"), j,
						 live_translate(wp, "vlan.tagged"));
				if (flag == 17000)
					snprintf(aria, sizeof(aria), "%s %d %s", live_translate(wp, "share.port"), j,
						 live_translate(wp, "vlan.negociate"));
				if (flag == 18000)
					snprintf(aria, sizeof(aria), "%s %d %s", live_translate(wp, "share.port"), j,
						 live_translate(wp, "vlan.gigabit"));
				if (flag == 19000)
					snprintf(aria, sizeof(aria), "%s %d %s", live_translate(wp, "share.port"), j,
						 live_translate(wp, "vlan.fullspeed"));
				if (flag == 20000)
					snprintf(aria, sizeof(aria), "%s %d %s", live_translate(wp, "share.port"), j,
						 live_translate(wp, "vlan.fullduplex"));
				if (flag == 21000)
					snprintf(aria, sizeof(aria), "%s %d %s", live_translate(wp, "share.port"), j,
						 live_translate(wp, "share.enabled"));
				if (flag == 22000)
					snprintf(aria, sizeof(aria), "%s %d %s", live_translate(wp, "share.port"), j,
						 live_translate(wp, "networking.snooping"));
				if (flag == 23000)
					snprintf(aria, sizeof(aria), "%s %d %s", live_translate(wp, "share.port"), j,
						 live_translate(wp, "vlan.eee"));
				if (flag == 24000)
					snprintf(aria, sizeof(aria), "%s %d %s", live_translate(wp, "share.port"), j,
						 live_translate(wp, "vlan.flow"));
			}
			websWrite(
				wp,
				" height=\"20\"><div class=\"center\"><input type=\"checkbox\" value=\"on\" aria-label=\"%s\" name=%s ",
				aria, buff);

			//                      fprintf(stderr, "port %d, line %d flags %d %d\n", j, i, vlans[j][i], flag);
			if (flag < 17000 || flag > 21000) {
				if (vlans[j][i] == 1)
					websWrite(wp, "checked=\"checked\" ");
			} else {
				if (vlans[j][i] == -1)
					websWrite(wp, "checked=\"checked\" ");
			}
			if (flag < 17000) {
				snprintf(buff, sizeof(buff), "\"SelVLAN(this.form,'port%d')\"", j);
				websWrite(wp, "onclick=%s", buff);
			} else if (flag == 17000 || flag == 18000 || flag == 21000 || flag == 22000 || flag == 23000 ||
				   flag == 24000) {
				snprintf(buff, sizeof(buff), "\"SelSpeed(this.form,'port%d')\"", j);
				websWrite(wp, "onclick=%s", buff);
			}
			websWrite(wp, " /></div></td>\n");
		}

		if (flag < 16000 && flag > 2) {
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"vlan_remove(this.form,'%d')\\\" />\");\n//]]>\n</script></td>\n",
				i);
		} else {
			websWrite(wp, "<td>&nbsp;</td>\n");
		}

		websWrite(wp, "</tr>\n");
		if (i == (blen - 1)) {
			websWrite(
				wp,
				"<tr><td colspan=\"%d\">&nbsp;</td><td class=\"center\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"vlan_add(this.form,'%d')\\\" />\");\n//]]>\n</script></td></tr>\n",
				6 + cpuports, i);
		}
		if (flag == 20000 || flag == 16000) {
			websWrite(wp, "<tr><td>&nbsp;</td></tr>\n");
		}
	}

#if 0 //ndef HAVE_SWCONFIG
	websWrite(wp, "<tr>\n");
	websWrite(wp, "<td><script type=\"text/javascript\">Capture(share.wireless)</script></td>\n");

	websWrite(wp, "<td colspan=\"6\"><select name=\"wireless\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"-1\\\"");
	if (wl_br < 0)
		websWrite(wp, " selected=\\\"selected\\\"");
	websWrite(wp, ">\" + share.none + \"</option>\");\n//]]>\n</script><option value=\"0\"");
	if (wl_br == 0)
		websWrite(wp, " selected=\"selected\"");
	websWrite(wp, ">LAN</option></select></td>\n");
	websWrite(wp, "</tr>\n");

	websWrite(wp, "<tr><td>&nbsp;</td></tr>\n");

	websWrite(wp, "<tr>\n");
	websWrite(wp, "<td><script type=\"text/javascript\">Capture(vlan.aggregation)</script></td>\n");

	websWrite(wp,
		  "<td colspan=\"6\"><select name=\"trunking\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\">\" + share.no + \"</option>\");\n//]]>\n</script><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"1\\\"");

	c = nvram_safe_get("trunking");

	snprintf(buff, 5, "%s", c);

	if (atoi(buff) == 1)
		websWrite(wp, " selected=\\\"selected\\\"");

	websWrite(wp, ">\" + vlan.trunk + \"</option>\");\n//]]>\n</script></select></td>\n");
	websWrite(wp, "</tr>");
#endif
	for (i = 0; i < 10; i++)
		debug_free(vlans[i]);
	debug_free(vlanlist);
	nvram_default_get("portvlanlist", deflist);
	debug_free(deflist);
	return;
}
