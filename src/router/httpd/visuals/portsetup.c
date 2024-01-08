/*
 * portsetup.c
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
extern char *getTXQ(char *ifname);
EJ_VISIBLE void ej_portsetup(webs_t wp, int argc, char_t **argv)
{
	char wan_if_buffer[33];
	char ssid[64];
	char *next, *bnext;
	char var[64];
	char eths[256];
	char bword[256];
	char bufferif[512];
	char buf[128];

	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(idx.interface_h2)</script></h2>\n");
	websWrite(wp, "<fieldset>\n");

	char *wanifname = nvram_safe_get("wan_ifname2");

	if (*(wanifname) == 0)
		wanifname = nvram_safe_get("wan_ifname");
	bzero(eths, 256);
	getIfLists(eths, 256);
	if (*(wanifname)) {
		show_caption_legend(wp, "idx.portsetup");
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(idx.wanport)</script></div>\n");
		websWrite(wp, "<select name=\"wan_ifname\">\n");

		websWrite(wp,
			  "<option value=\"\" %s ><script type=\"text/javascript\">Capture(share.disabled);</script></option>\n",
			  *(wanifname) == 0 ? "selected=\"selected\"" : "");
		foreach(var, eths, next)
		{
			websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var,
				  !strcmp(wanifname, var) ? "selected=\"selected\"" : "", getNetworkLabel(wp, var));
		}
		websWrite(wp, "</select></div>\n");
	}
	bzero(bufferif, sizeof(bufferif));
	getIfListB(bufferif, NULL, 1, 1, 0);
	foreach(var, eths, next)
	{
		int isb = 0;
		if (!strcmp("etherip0", var))
			continue;
		if (strchr(var, '.') == NULL) {
			if (!strcmp(safe_get_wan_face(wan_if_buffer), var))
				continue;
			if (!strcmp(nvram_safe_get("lan_ifname"), var))
				continue;
			if (isbridge(var))
				isb = 1;
		}
#ifdef HAVE_BRCM
		if (!strcmp("eth0", var) && ifexists("vlan1"))
			continue;
#endif
		char layer[64];
		strcpy(layer, var);
		rep(layer, '.', 'X');
		sprintf(ssid, "%s_bridged", var);
		// nvram_nset("0", "%s_bridged", var);
		websWrite(wp, "<fieldset>\n");
		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(wl_basic.network)</script> %s</legend>\n",
			  getNetworkLabel(wp, var));
		// mac address
		if (!isb) {
			unsigned char mac[20];
			char *r = get_hwaddr(var, mac);
			char *nvmac = nvram_nget("%s_hwaddr", var);
			if (r && !*(nvmac) && strncmp(var, "wl", 2))
				nvram_nset(r, "%s_hwaddr", var);
			websWrite(
				wp,
				"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(share.mac)</script></div>\n");
			websWrite(wp,
				  "<input class=\"num\" maxlength=\"17\" size=\"16\" name=\"%s_hwaddr\" value=\"%s\" /></div>\n",
				  var, nvram_nget("%s_hwaddr", var));
		}
		// label here
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(idx.label)</script></div>\n");
		websWrite(wp, "<input maxlength=\"32\" size=\"25\" name=\"%s_label\" value=\"%s\" /></div>\n", var,
			  nvram_nget("%s_label", var));
		// qlen here
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">%s</div>\n", tran_string(buf, sizeof(buf), "idx.txqlen"));
		char txq[32];
		sprintf(txq, "%s_txq", var);
		websWrite(
			wp,
			"<input class=\"num\" maxlength=\"4\" onblur=\"valid_range(this,0,10000,idx.txqlen)\" size=\"5\" name=\"%s\" value=\"%s\" />\n",
			txq, nvram_default_get(txq, getTXQ(var)));
		websWrite(wp, "</div>\n");
		// qlen end
		if (!isb) {
			int iswds = 0;
			if (!strncmp(var, "wlan", 4) && strstr(var, ".sta"))
				iswds = 1;
			if (!iswds && has_multicast_to_unicast(var) && !nvram_nmatch("0", "%s_bridged", var)) {
				char unicast[32];
				sprintf(unicast, "%s_multicast_to_unicast", var);
				nvram_default_get(unicast, "0");
				showRadio(wp, "networking.unicast", unicast);
			}
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "networking.bridgeassign", NULL);
			websWrite(
				wp,
				"<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idnet', false);\" name=\"%s_bridged\" %s /><script type=\"text/javascript\">Capture(share.deflt)</script>\n",
				layer, var, nvram_default_matchi(ssid, 1, 1) ? "checked=\"checked\"" : "");
			websWrite(
				wp,
				"<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idnet', true);\" name=\"%s_bridged\" %s /><script type=\"text/javascript\">Capture(wl_basic.unbridged)</script>\n",
				layer, var, nvram_default_matchi(ssid, 0, 1) ? "checked=\"checked\"" : "");
			websWrite(wp, "</div>\n");
		}
		if (!isb) {
			websWrite(wp, "<div id=\"%s_idnet\">\n", layer);
		}
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">%s</div>\n", tran_string(buf, sizeof(buf), "idx.mtu"));
		char mtu[32];
		sprintf(mtu, "%s_mtu", var);
		websWrite(
			wp,
			"<input class=\"num\" maxlength=\"5\" onblur=\"valid_mtu(this)\" size=\"5\" name=\"%s\" value=\"%s\" />\n",
			mtu, nvram_default_get(mtu, "1500"));
		websWrite(wp, "</div>\n");

		char mcast[32];
		sprintf(mcast, "%s_multicast", var);
		nvram_default_get(mcast, "0");
		showRadio(wp, "wl_basic.multicast", mcast);

		if (has_gateway()) {
			sprintf(mcast, "%s_nat", var);
			nvram_default_get(mcast, "1");
			showRadio(wp, "wl_basic.masquerade", mcast);
			sprintf(mcast, "%s_bloop", var);
			nvram_default_get(mcast, nvram_safe_get("block_loopback"));
			showRadio(wp, "filter.nat", mcast);
		}

		char isolation[32];
		sprintf(isolation, "%s_isolation", var);
		nvram_default_get(isolation, "0");
		showRadio(wp, "wl_basic.isolation", isolation);
#ifdef HAVE_TOR
		if (nvram_matchi("tor_enable", 1)) {
			char tor[32];
			sprintf(tor, "%s_tor", var);
			nvram_default_get(tor, "0");
			showRadio(wp, "wl_basic.tor_anon", tor);
		}
#endif

		char redirect[32];
		sprintf(redirect, "%s_dns_redirect", var);
		nvram_default_get(redirect, "0");

		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(idx.force_dnsmasq)</script></div>\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idredirect', true);\" name=\"%s_dns_redirect\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			layer, var, nvram_default_matchi(redirect, 1, 0) ? "checked=\"checked\"" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idredirect', false);\" name=\"%s_dns_redirect\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			layer, var, nvram_default_matchi(redirect, 0, 0) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div id=\"%s_idredirect\">\n", layer);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "idx.dns_redirect", NULL);
		websWrite(wp, "<input type=\"hidden\" name=\"%s_dns_ipaddr\" value=\"4\" />\n", var);
		show_ip(wp, var, "dns_ipaddr", 0, 0, "share.ip");

		websWrite(wp, "</div>\n");

		websWrite(wp, "</div>\n");

		show_ipnetmask(wp, var);
#if defined(HAVE_BKM) || defined(HAVE_TMK)
#ifdef HAVE_REGISTER
		if (registered_has_cap(21))
#endif
		{
			char nld_enable[32], nld_bridge[32];
			char word[256];
			char *next;

			sprintf(nld_enable, "nld_%s_enable", var);
			websWrite(
				wp,
				"<div class=\"setting\">\n<div class=\"label\">ZCM&nbsp;<script type=\"text/javascript\">Capture(share.enable)</script></div>\n");
			websWrite(wp,
				  "<input class=\"spaceradio\" type=\"checkbox\" name=\"nld_%s_enable\" value=\"1\" %s /></div>\n",
				  var, nvram_matchi(nld_enable, 1) ? "checked=\"checked\"" : "");

			sprintf(nld_bridge, "nld_%s_bridge", var);
			nvram_default_get(nld_bridge, "br0");
			websWrite(
				wp,
				"<div class=\"setting\">\n<div class=\"label\">ZCM&nbsp;<script type=\"text/javascript\">Capture(networking.bridge)</script></div>\n");
			websWrite(wp, "<select name=\"nld_%s_bridge\">\n", var);
			websWrite(wp, "  <option value=\"\">none</option>\n");
			foreach(word, bufferif, next)
			{
				// if( strcmp( word, "br0" ) ) {
				websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", word,
					  nvram_match(nld_bridge, word) ? "selected=\"selected\"" : "", word);
				// }
			}
			websWrite(wp, "</select>\n</div>\n");
			// NSMD
			sprintf(nld_enable, "nsmd_%s_enable", var);
			websWrite(
				wp,
				"<div class=\"setting\">\n<div class=\"label\">NSMD&nbsp;<script type=\"text/javascript\">Capture(share.enable)</script></div>\n");
			websWrite(wp,
				  "<input class=\"spaceradio\" type=\"checkbox\" name=\"nsmd_%s_enable\" value=\"1\" %s /></div>\n",
				  var, nvram_match(nld_enable, "1") ? "checked=\"checked\"" : "");
		}
#endif
#if defined(HAVE_BATMANADV)
#ifdef HAVE_REGISTER
		if (registered_has_cap(19))
#endif
		{
			char bat_enable[32], bat_bridge[32];
			char word[256];
			char *next;

			sprintf(bat_enable, "bat_%s_enable", var);
			websWrite(
				wp,
				"<div class=\"setting\">\n<div class=\"label\">L2Mesh&nbsp;<script type=\"text/javascript\">Capture(share.enable)</script></div>\n");
			websWrite(wp,
				  "<input class=\"spaceradio\" type=\"checkbox\" name=\"bat_%s_enable\" value=\"1\" %s /></div>\n",
				  var, nvram_matchi(bat_enable, 1) ? "checked=\"checked\"" : "");

			sprintf(bat_bridge, "bat_%s_bridge", var);
			nvram_default_get(bat_bridge, "br0");
			websWrite(
				wp,
				"<div class=\"setting\">\n<div class=\"label\">L2Mesh&nbsp;<script type=\"text/javascript\">Capture(networking.bridge)</script></div>\n");
			websWrite(wp, "<select name=\"bat_%s_bridge\">\n", var);
			websWrite(
				wp,
				"<option value=\"none\" %s><script type=\"text/javascript\">Capture(share.none)</script></option>\n",
				nvram_match(bat_bridge, "none") ? "selected=\"selected\"" : "");
			foreach(word, bufferif, next)
			{
				// if( strcmp( word, "br0" ) ) {
				websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", word,
					  nvram_match(bat_bridge, word) ? "selected=\"selected\"" : "", word);
				// }
			}
			websWrite(wp, "</select>\n</div>\n");
		}
#endif
		if (!isb) {
			websWrite(wp, "</div>\n");
		}
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n ");
		if (!isb)
			websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_bridged\"), \"%s_idnet\", %s);\n", var, layer,
				  nvram_matchi(ssid, 0) ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_dns_redirect\"), \"%s_idredirect\", %s);\n", var,
			  layer, nvram_matchi(redirect, 1) ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");
#ifdef HAVE_TMK
		websWrite(wp, "<br />\n");
		websWrite(wp, "<fieldset>\n");
		char r1x_if[32];
		sprintf(r1x_if, "%s_r1x", var);
		nvram_default_get(r1x_if, "0");
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(service.wired_8021x_server)</script</div>\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" onclick=\"show_layer_ext(this, '%s_r1x_block', true);\" type=\"radio\" name=\"%s_r1x\" value=\"1\" %s />\n",
			var, var, nvram_match(r1x_if, "1") ? "checked=\"checked\"" : "");
		websWrite(wp, "<script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" onclick=\"show_layer_ext(this, '%s_r1x_block', false);\" type=\"radio\" name=\"%s_r1x\" value=\"0\" %s />\n",
			var, var, nvram_match(r1x_if, "0") ? "checked=\"checked\"" : "");
		websWrite(wp, "<script type=\"text/javascript\">Capture(share.disable)</script>&nbsp;</div>\n");
		// showRadio(wp, "Use as primary Wan", wan_prim);
		websWrite(wp, "<div id=\"%s_r1x_block\">\n", var);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.radius_ipaddr)</script></div>\n");
		char *ipv = nvram_nget("%s_r1x_server", var);
		websWrite(wp, "<input name=\"%s_r1x_server\" size=\"32\" maxlength=\"255\" value=\"%s\" />", var, ipv);
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.radius_port)</script></div>\n");
		ipv = nvram_nget("%s_r1x_port", var);
		websWrite(wp, "<input name=\"%s_r1x_port\" size=\"5\" maxlength=\"10\" value=\"%s\" />", var, ipv);
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.radius_shared_secret)</script></div>\n");
		ipv = nvram_nget("%s_r1x_ss", var);
		websWrite(wp, "<input name=\"%s_r1x_ss\" size=\"32\" maxlength=\"255\" value=\"%s\" />", var, ipv);
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.session_time)</script></div>\n");
		ipv = nvram_nget("%s_r1x_st", var);
		websWrite(wp, "<input name=\"%s_r1x_st\" size=\"5\" maxlength=\"20\" value=\"%s\" />", var, ipv);
		websWrite(wp, "&nbps;<script type=\"text/javascript\">Capture(share.seconds)</script></div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp,
			  "<div class=\"label\"><script type=\"text/javascript\">Capture(hotspot.nodog_MAClist)</script></div>\n");
		ipv = nvram_nget("%s_r1x_wl", var);
		websWrite(wp, "<input name=\"%s_r1x_wl\" size=\"32\" maxlength=\"255\" value=\"%s\" />", var, ipv);
		websWrite(wp, "</div>\n");

		websWrite(wp, "</fieldset><br />\n");
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n ");
		websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_r1x\"), \"%s_r1x_block\", %s);\n", var, layer,
			  nvram_match(r1x_if, "1") ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");
#endif

		websWrite(wp, "</fieldset><br />\n");
skip:;
	}
	websWrite(wp, "</fieldset><br />\n");
}
