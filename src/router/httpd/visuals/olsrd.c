/*
 * olsrd.c
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
void ej_show_olsrd(webs_t wp, int argc, char_t ** argv)
{
	char *var = websGetVar(wp, "wk_mode", NULL);

	if (var == NULL)
		var = nvram_safe_get("wk_mode");
	if (!strcmp(var, "olsr")) {
		websWrite(wp, "<fieldset>\n");
		show_legend(wp, "route.olsrd_legend", 1);
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(route.olsrd_gateway)</script></div>\n");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"olsrd_gateway\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>\n",
			  nvram_default_match("olsrd_gateway", "1", "0") ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"olsrd_gateway\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
			  nvram_default_match("olsrd_gateway", "0", "0") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");

		show_inputlabel(wp, "route.olsrd_hna", "olsrd_hna", 32, "num", 32);
		show_inputlabel(wp, "route.olsrd_poll", "olsrd_pollsize", 5, "num", 5);
		showOptionsLabel(wp, "route.olsrd_tc", "olsrd_redundancy", "0 1 2", nvram_default_get("olsrd_redundancy", "2"));
		show_inputlabel(wp, "route.olsrd_mpr", "olsrd_coverage", 5, "num", 5);
		showRadio(wp, "route.olsrd_lqfe", "olsrd_lqfisheye");
		show_inputlabel(wp, "route.olsrd_lqag", "olsrd_lqaging", 5, "num", 5);
#ifdef HAVE_IPV6
		showRadio(wp, "route.olsrd_smartgw", "olsrd_smartgw");
#endif
/*		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp,
			  "<div class=\"label\"><script type=\"text/javascript\">Capture(route.olsrd_lqdmin)</script></div>");
		websWrite(wp,"<input class=\"num\" size=\"5\" maxlength=\"5\" name=\"olsrd_lqdijkstramin\" onblur=\"olsrd_checkDijkstra(this.form)\" value=\"%s\" />\n",nvram_safe_get("olsrd_lqdijkstramin"));
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(route.olsrd_lqdmax)</script></div>");
		websWrite(wp,
			  "<input class=\"num\" size=\"5\" maxlength=\"5\" name=\"olsrd_lqdijkstramax\" onblur=\"olsrd_checkDijkstra(this.form)\" value=\"%s\" />\n",
			  nvram_safe_get("olsrd_lqdijkstramax"));
		websWrite(wp, "</div>\n");*/

		showOptionsLabel(wp, "route.olsrd_lqlvl", "olsrd_lqlevel", "0 1 2", nvram_default_get("olsrd_lqlevel", "2"));
		showRadio(wp, "route.olsrd_hysteresis", "olsrd_hysteresis");
		char *wordlist = nvram_safe_get("olsrd_interfaces");
		char *next;
		char word[128];
		int count = 0;

		foreach(word, wordlist, next) {
			char *interface = word;
			char *hellointerval = interface;

			strsep(&hellointerval, ">");
			char *hellovaliditytime = hellointerval;

			strsep(&hellovaliditytime, ">");
			char *tcinterval = hellovaliditytime;

			strsep(&tcinterval, ">");
			char *tcvaliditytime = tcinterval;

			strsep(&tcvaliditytime, ">");
			char *midinterval = tcvaliditytime;

			strsep(&midinterval, ">");
			char *midvaliditytime = midinterval;

			strsep(&midvaliditytime, ">");
			char *hnainterval = midvaliditytime;

			strsep(&hnainterval, ">");
			char *hnavaliditytime = hnainterval;

			strsep(&hnavaliditytime, ">");
			websWrite(wp, "<fieldset>\n");
			show_legend(wp, interface, 0);
			char valuename[32];

			sprintf(valuename, "%s_hellointerval", interface);
			show_custominputlabel(wp, "Hello Interval", valuename, hellointerval, 5);
			sprintf(valuename, "%s_hellovaliditytime", interface);
			show_custominputlabel(wp, "Hello Validity Time", valuename, hellovaliditytime, 5);

			sprintf(valuename, "%s_tcinterval", interface);
			show_custominputlabel(wp, "TC Interval", valuename, tcinterval, 5);
			sprintf(valuename, "%s_tcvaliditytime", interface);
			show_custominputlabel(wp, "TC Validity Time", valuename, tcvaliditytime, 5);

			sprintf(valuename, "%s_midinterval", interface);
			show_custominputlabel(wp, "MID Interval", valuename, midinterval, 5);
			sprintf(valuename, "%s_midvaliditytime", interface);
			show_custominputlabel(wp, "MID Validity Time", valuename, midvaliditytime, 5);

			sprintf(valuename, "%s_hnainterval", interface);
			show_custominputlabel(wp, "HNA Interval", valuename, hnainterval, 5);
			sprintf(valuename, "%s_hnavaliditytime", interface);
			show_custominputlabel(wp, "HNA Validity Time", valuename, hnavaliditytime, 5);
			websWrite(wp,
				  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"olsrd_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
				  count);

			websWrite(wp, "</fieldset>\n");
			count++;
		}
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(route.olsrd_newiface)</script></div>\n");
		char buffer[256];

		memset(buffer, 0, 256);
		getIfList(buffer, NULL);
		showIfOptions(wp, "olsrd_ifname", buffer, "");
		websWrite(wp, "&nbsp;&nbsp;");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"olsrd_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</fieldset><br />\n");
	}
}

