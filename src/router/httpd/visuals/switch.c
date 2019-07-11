/*
 * switch.c
 *
 * Copyright (C) 2005 - 2019 Sebastian Gottschall <gottschall@dd-wrt.com>
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
void ej_port_vlan_table(webs_t wp, int argc, char_t ** argv)
{
	/*
	 * vlans[x][y] where x 0-15 are VLANS x 16 is tagging, 17 is
	 * auto-negotiation, 18 is 100/10 Mbit, and 19 is Full/Half duplex y 0-4
	 * are switch ports (port 5 is set automaticly) y 5 it the bridge device
	 * (x 16 dosn't apply) 
	 */

	int i, j, vlans[22][7], tmp, wl_br;
	char *c, *next, buff[32], portvlan[32];
	int a;
	int lanports = 4;
	if (nvram_exists("sw_lan6"))
		lanports = 6;

	for (i = 0; i < 22; i++)
		for (j = 0; j < lanports + 2; j++)
			vlans[i][j] = -1;

	wl_br = -1;

	for (i = 0; i < lanports + 2; i++) {
		if (i < (lanports + 1))
			snprintf(buff, 31, "port%dvlans", i);
		else if (i == 5)
			snprintf(buff, 31, "%s", "lan_ifnames");
		else
			snprintf(buff, 31, "ub%d_ifnames", i - 5);

		c = nvram_safe_get(buff);

		if (c) {
			foreach(portvlan, c, next) {
				if (portvlan[0] == 'e' && portvlan[1] == 't' && portvlan[2] == 'h' && portvlan[3] == '1')
					wl_br = i - (lanports + 1);
				if (ISDIGIT(portvlan, 1)
				    || (portvlan[0] == 'v' && portvlan[1] == 'l' && portvlan[2] == 'a' && portvlan[3] == 'n')) {
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

					if (i < lanports + 1) {
						vlans[tmp][i] = 1;
					} else {
						vlans[tmp][lanports + 1] = i - (lanports + 1);
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
	if (nowan)
		websWrite(wp, "<th colspan=\"%d\"><script type=\"text/javascript\">Capture(share.port)</script></th>\n", lanports);
	else
		websWrite(wp, "<th colspan=\"%d\"><script type=\"text/javascript\">Capture(share.port)</script></th>\n", lanports + 1);
	websWrite(wp, "<th rowspan=\"2\"><script type=\"text/javascript\">Capture(vlan.bridge)</script></th>\n");
	websWrite(wp, "</tr>\n");
	websWrite(wp, "<tr>\n");
	if (!nowan)
		websWrite(wp, "<th>W</th>\n");
	for (a = 1; a < lanports + 1; a++) {
		websWrite(wp, "<th>%d</th>\n", a);
	}
	websWrite(wp, "</tr>\n");

	websWrite(wp, "              <tr>\n");
	websWrite(wp, "<td><script type=\"text/javascript\">Capture(vlan.linkstatus)</script></td>\n");
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
		websWrite(wp, "<td class=\"%s\">&nbsp;</td>\n", cstatus);
	}

	websWrite(wp, "<td></td>\n");
	websWrite(wp, "              </tr>\n");
#else

	websWrite(wp, "<tr>\n");
	websWrite(wp, "<th rowspan=\"2\"><script type=\"text/javascript\">Capture(vlan.legend)</script></th>\n");
	websWrite(wp, "<th colspan=\"5\"><script type=\"text/javascript\">Capture(share.port)</script></th>\n");
	websWrite(wp, "<th rowspan=\"2\"><script type=\"text/javascript\">Capture(vlan.bridge)</script></th>\n");
	websWrite(wp, "</tr>\n");
	websWrite(wp, "<tr>\n");
	websWrite(wp, "<th>W</th>\n");
	websWrite(wp, "<th>1</th>\n");
	websWrite(wp, "<th>2</th>\n");
	websWrite(wp, "<th>3</th>\n");
	websWrite(wp, "<th>4</th>\n");
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

		websWrite(wp, "              <tr>\n");
		websWrite(wp, "<td><script type=\"text/javascript\">Capture(vlan.linkstatus)</script></td>\n");

		int vlanmap[6] = { 0, 1, 2, 3, 4, 5 };	// 0=wan; 1,2,3,4=lan; 5=internal 
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
			if (!strncmp(cstatus, "disc", 4))
				sprintf(status, "status_red");

			if (!strncmp(cstatus, "100", 3))
				sprintf(status, "status_yellow");

			if (!strncmp(cstatus, "1000", 4))
				sprintf(status, "status_green");

			websWrite(wp, "<td class=\"%s\">&nbsp;</td>\n", status);
		}
		websWrite(wp, "<td></td>\n");
		websWrite(wp, "              </tr>\n");
	}
#endif
	int hasgiga = 1;

	int len = 21;
#ifdef HAVE_SWCONFIG
	hasgiga = 0;
#ifdef HAVE_ALPINE
	len = 16;		// no tag support for now
#else
	len = 17;
	if (has_igmpsnooping())
		len = 18;
#endif
#endif

	for (a = 0; a < len + hasgiga; a++) {
		i = a;
		if (hasgiga) {
			if (a == 18)
				i = 21;
			if (a > 18)
				i = a - 1;
		}

		websWrite(wp, "              <tr>\n");
		websWrite(wp, "<td>");

		switch (i) {
		case 16:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.tagged)</script>");
			break;
		case 17:
#ifdef HAVE_SWCONFIG
			websWrite(wp, "<script type=\"text/javascript\">Capture(networking.snooping)</script>");
#else
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.negociate)</script>");
#endif
			break;
		case 18:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.fullspeed)</script>");
			break;
		case 19:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.fullduplex)</script>");
			break;
		case 20:
			websWrite(wp, "<script type=\"text/javascript\">Capture(share.enabled)</script>");
			break;
		case 21:
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.gigabit)</script>");
			break;
		default:
			snprintf(buff, 31, "%d", i);
			websWrite(wp, buff);
			break;
		}

		websWrite(wp, "</td>\n");

		for (j = nowan; j < lanports + 1; j++) {
			snprintf(buff, 31, "\"port%dvlan%d\"", j, i);
			websWrite(wp, "<td");

			if (j % 2 == 0)
				// websWrite(wp, " bgcolor=\"#CCCCCC\"");
				websWrite(wp, " class=\"odd\"");

			websWrite(wp, " height=\"20\"><div align=\"center\"><input type=\"checkbox\" value=\"on\" name=");
			websWrite(wp, buff);

			if (i < 17 || i > 21) {
				if (vlans[i][j] == 1)
					websWrite(wp, " checked=\"checked\"");
			} else {
				if (vlans[i][j] == -1)
					websWrite(wp, " checked=\"checked\"");
			}
#ifdef HAVE_SWCONFIG
			if (i < 18) {
#else
			if (i < 17) {
#endif
				websWrite(wp, " onclick=");
				snprintf(buff, sizeof(buff), "\"SelVLAN(this.form,'port%d')\"", j);
				websWrite(wp, buff);
			} else if (i == 17 || i == 20 || i == 21) {
				websWrite(wp, " onclick=");
				snprintf(buff, sizeof(buff), "\"SelSpeed(this.form,'port%d')\"", j);
				websWrite(wp, buff);
			}
			websWrite(wp, " /></div></td>\n");
		}

		if (i < 16) {
			websWrite(wp, "			<td><select name=");
			snprintf(buff, 31, "\"vlan%d\"", i);
			websWrite(wp, buff);
			websWrite(wp, "><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"-1\\\"");
			if (vlans[i][lanports + 1] < 0)
				websWrite(wp, " selected=\\\"selected\\\"");
			websWrite(wp, ">\" + share.none + \"</option>\");\n//]]>\n</script><option value=\"0\"");
			if (vlans[i][lanports + 1] == 0)
				websWrite(wp, " selected=\"selected\"");
			websWrite(wp, ">LAN</option></select></td>\n");
		} else {
			websWrite(wp, "<td>&nbsp;</td>\n");
		}

		websWrite(wp, "</tr>\n");

		if (i == 16 || i == 20) {
			websWrite(wp, "<tr><td>&nbsp;</td></tr>\n");
		}
	}

#ifndef HAVE_SWCONFIG
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
	websWrite(wp, "              </tr>");
#endif
	return;
}
