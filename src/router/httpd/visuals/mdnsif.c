/*
 * Copyright (C) 2022 EGC
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
 */

// for popen warning in Ubuntu
//#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>  //for isdigit and isalpha
#include <errno.h>
#include <stdarg.h>

//for ddwrt
// /*
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
//#include <services.h>

#include <broadcom.h>

// */


EJ_VISIBLE void ej_show_mdnsif(webs_t wp, int argc, char_t ** argv)
{
	char bufferif[256];
	int count = 1;
	int maxcount = 3;
	char temp[64];
	char word[32];
	char *next;
	bzero(bufferif, 256);
	
	getIfList(bufferif, NULL);
	
	
	websWrite(wp, "<fieldset>\n");
	show_caption_legend(wp, "service.mdns_legend");
	//websWrite(wp, "<legend><% tran(\"service.mdns_legend\"); %></legend>\n");
		snprintf(temp, sizeof(temp), "mdns_enable");
		websWrite(wp, "<div class=\"setting\">\n");
		{
			show_caption(wp, "label", "service.mdns_legend", NULL);
			websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" onclick=\"show_layer_ext(this, 'idmdns', true)\" value=\"1\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 1, 1) ? "checked=\"checked\"" : ""));
			show_caption(wp, NULL, "share.enable", "&nbsp;");
			websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" onclick=\"show_layer_ext(this, 'idmdns', false)\" value=\"0\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 0, 1) ? "checked=\"checked\"" : ""));
			show_caption_simple(wp, "share.disable");
		}
		websWrite(wp, "</div>\n");
		
		websWrite(wp, "<div id=\"idmdns\">\n");
			snprintf(temp, sizeof(temp), "mdns_domain");
			websWrite(wp, "<div class=\"setting\">\n");
			{
				show_caption(wp, "label", "service.mdns_domain", NULL);
				websWrite(wp, "<input size=\"15\" maxlength=\"24\" class=\"text\" name=\"%s\" value=\"%s\" />\n", temp, nvram_safe_get(temp));
			}
			websWrite(wp, "</div>\n");
			snprintf(temp, sizeof(temp), "mdns_reflector");
			websWrite(wp, "<div class=\"setting\">\n");
			{
				show_caption(wp, "label", "service.mdns_reflector", NULL);
				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 1, 1) ? "checked=\"checked\"" : ""));
				show_caption(wp, NULL, "share.enable", "&nbsp;");
				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 0, 1) ? "checked=\"checked\"" : ""));
				show_caption_simple(wp, "share.disable");
			}
			websWrite(wp, "</div>\n");
			/*
			websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "service.mdns_interfaces", NULL);
				websWrite(wp, "<input size=\"15\" maxlength=\"32\" class=\"text\" name=\"mdns_interfaces\" value=\"%s\" />\n", nvram_safe_get("mdns_interfaces"));
			websWrite(wp, "</div>\n");
			*/

			//interfaces
			websWrite(wp, "<fieldset>\n");
				show_caption_legend(wp, "service.mdns_interfaces");
				//show_caption(wp, "label", "service.mdns_interfaces", NULL);
				websWrite(wp, "<table>\n");
				websWrite(wp, "<tr>\n");
				foreach(word, bufferif, next) {
					if ( strncmp(word, "aux", 3) && strcmp(word, "lo") && strcmp(word, "ctf0") && !strchr(word, ':')) {
						snprintf(temp, sizeof(temp), "mdnsif_%s", word);
						{
							websWrite(wp, "<td>&nbsp;&nbsp;\n");
							websWrite(wp, "&nbsp;&nbsp;\n");
							websWrite(wp, "<label for=\"%s\">%s</label>", temp, word);
							websWrite(wp, "</td>\n");
							websWrite(wp, "<td>\n");
							websWrite(wp, "<input type=\"hidden\" name=\"%s\" id=\"%s\" value=\"0\" />\n", temp, temp);
							websWrite(wp, "<input class=\"spaceradio\" type=\"checkbox\" name=\"%s\" value=\"1\" %s />\n", temp, (nvram_default_matchi(temp, 1, 0) ? "checked=\"checked\"" : ""));
							websWrite(wp, "</td>\n");
							if (count++ > maxcount) {
								websWrite(wp, "</tr>\n");
								websWrite(wp, "<tr>\n");
								maxcount += 4;
							}
						}
					}
				}
				websWrite(wp, "</tr>\n");
				websWrite(wp, "</table>\n");
			websWrite(wp, "</fieldset>\n");
			//end interfaces
		websWrite(wp, "</div>\n"); //end hide show
	websWrite(wp, "</fieldset>\n");


	websWrite(wp, "<div class=\"center\">\n");
	{
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp,
			  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"mdnsif_save_button\\\" value=\\\"\" + share.add + \"\\\" onclick=\\\"mdnsif_save(this.form)\\\" />\");\n");
		websWrite(wp, "//]]>\n</script>\n");
	}
	websWrite(wp, "</div>\n");
	
	return;
}
