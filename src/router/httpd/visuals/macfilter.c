/*
 * macfilter.c
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
static void show_macfilter_if(webs_t wp, char *ifname)
{
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend>%s - %s</legend>\n", getNetworkLabel(IFMAP(ifname)), live_translate("wl_mac.legend"));
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">%s</div>\n", live_translate("wl_mac.label"));
	char macmode[32];

	sprintf(macmode, "%s_macmode1", ifname);
	rep(macmode, '.', 'X');
	if (nvram_get(macmode) == NULL)
		nvram_set(macmode, "disabled");
	char id[32];

	sprintf(id, "idmac%s", ifname);
	rep(id, '.', 'X');
	char mycopy[256];

	strcpy(mycopy, live_translate("share.enable"));
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"other\" name=\"%s\" %s onclick=\"show_layer_ext(this, '%s', true)\" />%s&nbsp;\n",
		  macmode, nvram_match(macmode, "other") ? "checked=\"checked\"" : "", id, mycopy);
	strcpy(mycopy, live_translate("share.disable"));
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"disabled\" name=\"%s\" %s onclick=\"show_layer_ext(this, '%s', false)\" />%s\n",
		  macmode, nvram_match(macmode, "disabled") ? "checked=\"checked\"" : "", id, mycopy);
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\" id=\"%s\">\n", id);
	websWrite(wp, "<div class=\"label\">%s<br />&nbsp;</div>\n", live_translate("wl_mac.label2"));
	sprintf(macmode, "%s_macmode", ifname);
	if (nvram_get(macmode) == NULL)
		nvram_set(macmode, "disabled");
	strcpy(mycopy, live_translate("wl_mac.deny"));
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"deny\" name=\"%s\" %s />%s&nbsp;\n", macmode, nvram_invmatch(macmode, "allow") ? "checked=\"checked\"" : "", mycopy);
	websWrite(wp, "<br />\n");
	strcpy(mycopy, live_translate("wl_mac.allow"));
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"allow\" name=\"%s\" %s />%s\n", macmode, nvram_match(macmode, "allow") ? "checked=\"checked\"" : "", mycopy);
	websWrite(wp, "</div><br />\n");
	websWrite(wp, "<div class=\"center\">\n");
	websWrite(wp, "<script type=\"text/javascript\">\n");
	websWrite(wp, "//<![CDATA[\n");
	websWrite(wp,
		  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"mac_filter_button\\\" value=\\\"\" + sbutton.filterMac + \"\\\" onclick=\\\"openWindow('WL_FilterTable-%s.asp', 1000, 740,'MACList');\\\" />\");\n",
		  ifname);
	websWrite(wp, "//]]>\n");
	websWrite(wp, "</script>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "</fieldset><br />\n");
}

void ej_list_mac_layers(webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
	int c = get_wl_instances();
	char devs[32];
	int i;

	for (i = 0; i < c; i++) {
		char macmode[32];
		char id[32];

		sprintf(devs, "wl%d", i);
		sprintf(macmode, "%s_macmode1", devs);
		sprintf(id, "idmac%s", devs);
		rep(id, '.', 'X');
		rep(macmode, '.', 'X');
		websWrite(wp, "show_layer_ext(document.wireless.%s, '%s', \"%s\" == \"other\");\n", macmode, id, nvram_match(macmode, "other") ? "other" : "disabled");
	}

#else

	int c = getdevicecount();
	char devs[32];
	int i;

	for (i = 0; i < c; i++) {
		char macmode[32];
		char id[32];

		sprintf(devs, "ath%d", i);
		sprintf(macmode, "%s_macmode1", devs);
		sprintf(id, "idmac%s", devs);
		rep(id, '.', 'X');
		rep(macmode, '.', 'X');
		websWrite(wp, "show_layer_ext(document.wireless.%s, '%s', \"%s\" == \"other\");\n", macmode, id, nvram_match(macmode, "other") ? "other" : "disabled");
		// show_macfilter_if (wp, devs);
		char vif[32];

		sprintf(vif, "%s_vifs", devs);
		char var[80], *next;
		char *vifs = nvram_safe_get(vif);

		if (vifs != NULL)
			foreach(var, vifs, next) {
			sprintf(macmode, "%s_macmode1", var);
			sprintf(id, "idmac%s", var);
			rep(id, '.', 'X');
			rep(macmode, '.', 'X');
			websWrite(wp, "show_layer_ext(document.wireless.%s, '%s', \"%s\" == \"other\");\n", macmode, id, nvram_match(macmode, "other") ? "other" : "disabled");
			}
	}

#endif
}

void ej_show_macfilter(webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
	int c = get_wl_instances();
	char devs[32];
	int i;

	for (i = 0; i < c; i++) {
		sprintf(devs, "wl%d", i);
		show_macfilter_if(wp, devs);
	}
#else
	int c = getdevicecount();
	char devs[32];
	int i;

	for (i = 0; i < c; i++) {
		sprintf(devs, "ath%d", i);
		show_macfilter_if(wp, devs);
		char vif[32];

		sprintf(vif, "%s_vifs", devs);
		char var[80], *next;
		char *vifs = nvram_safe_get(vif);

		if (vifs != NULL)
			foreach(var, vifs, next) {
			show_macfilter_if(wp, var);
			}
	}

#endif
}

