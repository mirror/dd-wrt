/*
 * portsetup.c
 *
 * Copyright (C) 2005 - 2016 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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
void ej_portsetup(webs_t wp, int argc, char_t ** argv)
{
	char ssid[64];
	char *next, *bnext;
	char var[64];
	char eths[256];
	static char bword[256];
	char bufferif[512];

	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(idx.portsetup)</script></h2>\n");
	websWrite(wp, "<fieldset>\n");

	char *wanifname = nvram_safe_get("wan_ifname2");

	if (strlen(wanifname) == 0)
		wanifname = nvram_safe_get("wan_ifname");
	memset(eths, 0, 256);
	getIfLists(eths, 256);
	if (strlen(wanifname) > 0) {

		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(idx.portsetup)</script></legend>\n");
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(idx.wanport)</script></div>\n");
		websWrite(wp, "<select name=\"wan_ifname\">\n");

		websWrite(wp, "<option value=\"\" %s ><script type=\"text/javascript\">Capture(share.disabled);</script></option>\n", strlen(wanifname) == 0 ? "selected=\"selected\"" : "");
		foreach(var, eths, next) {
			websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, !strcmp(wanifname, var) ? "selected=\"selected\"" : "", getNetworkLabel(var));
		}
		websWrite(wp, "</select></div>\n");
	}
	memset(bufferif, 0, 256);
	getIfList(bufferif, "br");
	foreach(var, eths, next) {
		int isbridge = 0;
		if (!strcmp("etherip0", var))
			continue;
		if (strchr(var, '.') == NULL) {
			if (!strcmp(get_wan_face(), var))
				continue;
			if (!strcmp(nvram_safe_get("lan_ifname"), var))
				continue;
			foreach(bword, bufferif, bnext) {
				if (!strcmp(bword, var)) {
					isbridge = 1;
				}
			}
		}

		char layer[64];
		strcpy(layer, var);
		rep(layer, '.', 'X');
		sprintf(ssid, "%s_bridged", var);
		// nvram_nset("0", "%s_bridged", var);
		websWrite(wp, "<fieldset>\n");
		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(wl_basic.network)</script> %s</legend>\n", getNetworkLabel(var));
		// label here
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(idx.label)</script></div>\n");
		websWrite(wp, "<input maxlength=\"32\" size=\"25\" name=\"%s_label\" value=\"%s\" /></div>\n", var, nvram_nget("%s_label", var));
		// qlen here

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">%s</div>\n", live_translate("idx.txqlen"));
		char txq[32];
		sprintf(txq, "%s_txq", var);
		websWrite(wp, "<input class=\"num\" maxlength=\"4\" onblur=\"valid_range(this,0,10000,idx.txqlen)\" size=\"5\" name=\"%s\" value=\"%s\" />\n", txq, nvram_default_get(txq, getTXQ(var)));
		websWrite(wp, "</div>\n");

		// qlen end
		if (!isbridge) {
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(networking.bridgeassign)</script></div>\n");
			websWrite(wp,
				  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idnet', true);\" name=\"%s_bridged\" %s /><script type=\"text/javascript\">Capture(wl_basic.unbridged)</script>&nbsp;\n",
				  layer, var, nvram_default_match(ssid, "0", "1") ? "checked=\"checked\"" : "");
			websWrite(wp,
				  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idnet', false);\" name=\"%s_bridged\" %s /><script type=\"text/javascript\">Capture(share.deflt)</script>\n",
				  layer, var, nvram_default_match(ssid, "1", "1") ? "checked=\"checked\"" : "");
			websWrite(wp, "</div>\n");
		}
		if (!isbridge) {
			websWrite(wp, "<div id=\"%s_idnet\">\n", layer);
		}
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">%s</div>\n", live_translate("idx.mtu"));
		char mtu[32];
		sprintf(mtu, "%s_mtu", var);
		websWrite(wp, "<input class=\"num\" maxlength=\"4\" onblur=\"valid_mtu(this)\" size=\"5\" name=\"%s\" value=\"%s\" />\n", mtu, nvram_default_get(mtu, "1500"));
		websWrite(wp, "</div>\n");

		char mcast[32];

		sprintf(mcast, "%s_multicast", var);
		nvram_default_get(mcast, "0");
		showRadio(wp, "wl_basic.multicast", mcast);

		if (has_gateway()) {
			sprintf(mcast, "%s_nat", var);
			nvram_default_get(mcast, "1");
			showRadio(wp, "wl_basic.masquerade", mcast);
		}

		char isolation[32];
		sprintf(isolation, "%s_isolation", var);
		nvram_default_get(isolation, "0");
		showRadio(wp, "wl_basic.isolation", isolation);

		char redirect[32];
		sprintf(redirect, "%s_dns_redirect", var);
		nvram_default_get(redirect, "0");

		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(idx.force_dnsmasq)</script></div>\n");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idredirect', true);\" name=\"%s_dns_redirect\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			  layer, var, nvram_default_match(redirect, "1", "0") ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idredirect', false);\" name=\"%s_dns_redirect\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			  layer, var, nvram_default_match(redirect, "0", "0") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div id=\"%s_idredirect\">\n", layer);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dns_redirect)</script></div>\n");
		char dnsip[32];
		sprintf(dnsip, "%s_dns_ipaddr", var);
		char *ipv = nvram_default_get(dnsip, "0.0.0.0");
		websWrite(wp, "<input type=\"hidden\" name=\"%s_dns_ipaddr\" value=\"4\" />\n", var);
		websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,223,share.ip)\" name=\"%s_dns_ipaddr_0\" value=\"%d\" />.", var, get_single_ip(ipv, 0));
		websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_dns_ipaddr_1\" value=\"%d\" />.", var, get_single_ip(ipv, 1));
		websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_dns_ipaddr_2\" value=\"%d\" />.", var, get_single_ip(ipv, 2));
		websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_dns_ipaddr_3\" value=\"%d\" />\n", var, get_single_ip(ipv, 3));
		websWrite(wp, "</div>\n");

		websWrite(wp, "</div>\n");

		show_ipnetmask(wp, var);
#if defined(HAVE_BKM) || defined(HAVE_TMK)
#ifdef HAVE_REGISTER
		if (registered_has_cap(21))
#endif
		{
			char nld_enable[32], nld_bridge[32], bufferif[256];
			char word[256];
			char *next;

			sprintf(nld_enable, "nld_%s_enable", var);
			websWrite(wp, "<div class=\"setting\">\n<div class=\"label\">ZCM enable</div>\n");
			websWrite(wp, "<input class=\"spaceradio\" type=\"checkbox\" name=\"nld_%s_enable\" value=\"1\" %s /></div>\n", var, nvram_match(nld_enable, "1") ? "checked=\"checked\"" : "");

			sprintf(nld_bridge, "nld_%s_bridge", var);
			nvram_default_get(nld_bridge, "br0");
			websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">/*Capture(idx.wanport)*/</script>ZCM Bridge</div>\n");
			websWrite(wp, "<select name=\"nld_%s_bridge\">\n", var);
			websWrite(wp, "  <option value=\"\">none</option>\n");
			memset(bufferif, 0, 256);
			getIfList(bufferif, "br");
			foreach(word, bufferif, next) {
				// if( strcmp( word, "br0" ) ) {
				websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", word, nvram_match(nld_bridge, word) ? "selected=\"selected\"" : "", word);
				// }
			}
			websWrite(wp, "</select>\n</div>\n");
		}
#endif
#if defined(HAVE_BATMANADV)
#ifdef HAVE_REGISTER
		if (registered_has_cap(19))
#endif
		{
			char bat_enable[32], bat_bridge[32], bufferif[256];
			char word[256];
			char *next;

			sprintf(bat_enable, "bat_%s_enable", var);
			websWrite(wp, "<div class=\"setting\">\n<div class=\"label\">L2Mesh enable</div>\n");
			websWrite(wp, "<input class=\"spaceradio\" type=\"checkbox\" name=\"bat_%s_enable\" value=\"1\" %s /></div>\n", var, nvram_match(bat_enable, "1") ? "checked=\"checked\"" : "");

			sprintf(bat_bridge, "bat_%s_bridge", var);
			nvram_default_get(bat_bridge, "br0");
			websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">/*Capture(idx.wanport)*/</script>L2Mesh Bridge</div>\n");
			websWrite(wp, "<select name=\"bat_%s_bridge\">\n", var);
			websWrite(wp, "  <option value=\"\">none</option>\n");
			memset(bufferif, 0, 256);
			getIfList(bufferif, "br");
			foreach(word, bufferif, next) {
				// if( strcmp( word, "br0" ) ) {
				websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", word, nvram_match(bat_bridge, word) ? "selected=\"selected\"" : "", word);
				// }
			}
			websWrite(wp, "</select>\n</div>\n");
		}
#endif
		websWrite(wp, "<br />\n");
		if (!isbridge) {
			websWrite(wp, "</div>\n");
		}
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n ");
		if (!isbridge)
			websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_bridged\"), \"%s_idnet\", %s);\n", var, layer, nvram_match(ssid, "0") ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_dns_redirect\"), \"%s_idredirect\", %s);\n", var, layer, nvram_match(redirect, "1") ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</fieldset>\n");
	      skip:;
	}
	websWrite(wp, "</fieldset><br />\n");
}

