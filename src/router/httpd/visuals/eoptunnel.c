/*
 * eoptunnel.c
 *
 * Copyright (C) 2005 - 2013 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_EOP_TUNNEL

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>
#include <cymac.h>

void ej_show_eop_tunnels(webs_t wp, int argc, char_t ** argv)
{

	int tun;
	char temp[32];

	for (tun = 1; tun < 11; tun++) {

		websWrite(wp, "<fieldset>\n");
		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(eoip.tunnel)</script> %d</legend>\n", tun);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.srv)</script></div>\n");
		sprintf(temp, "oet%d_en", tun);
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d', true)\" /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			  temp, (nvram_match(temp, "1") ? "checked=\"checked\"" : ""), tun);
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d', false)\" /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			  temp, (nvram_match(temp, "0") ? "checked=\"checked\"" : ""), tun);
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div id=\"idoet%d\">\n", tun);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.remoteIP)</script></div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"oet%d_rem\" value=\"0.0.0.0\"/>\n", tun);
		sprintf(temp, "oet%d_rem", tun);
		websWrite(wp,
			  "<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_0\" onblur=\"valid_range(this,0,255,eoip.remoteIP)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_1\" onblur=\"valid_range(this,0,255,eoip.tunnelID)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_2\" onblur=\"valid_range(this,0,255,eoip.tunnelID)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_3\" onblur=\"valid_range(this,1,254,eoip.tunnelID)\" value=\"%d\" />\n",
			  temp, get_single_ip(nvram_safe_get(temp), 0), temp, get_single_ip(nvram_safe_get(temp), 1), temp, get_single_ip(nvram_safe_get(temp), 2), temp, get_single_ip(nvram_safe_get(temp), 3));
		websWrite(wp, "</div>\n");
/*
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.tunnelID)</script></div>\n" );
		  	sprintf( temp, "oet%d_id", tun );
	websWrite( wp,
		   "<input size=\"4\" maxlength=\"3\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,999,eoip.tunnelID)\" value=\"%s\" />\n",
		   temp, nvram_get( temp ) );
	websWrite( wp, "</div>\n" );

	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.comp)</script></div>\n" );
	sprintf( temp, "oet%d_comp", tun );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		   temp,
		   ( nvram_match( temp, "1" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		   temp,
		   ( nvram_match( temp, "0" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp, "</div>\n" );
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.passtos)</script></div>\n" );
	sprintf( temp, "oet%d_pt", tun );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		   temp,
		   ( nvram_match( temp, "1" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		   temp,
		   ( nvram_match( temp, "0" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp, "</div>\n" );
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.frag)</script></div>\n" );
	sprintf( temp, "oet%d_fragment", tun );
	websWrite( wp,
		   "<input size=\"4\" maxlength=\"4\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,1500,eoip.frag)\" value=\"%s\" />\n",
		   temp, nvram_get( temp ) );
	websWrite( wp, "</div>\n" );
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.mssfix)</script></div>\n" );
	sprintf( temp, "oet%d_mssfix", tun );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		   temp,
		   ( nvram_match( temp, "1" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		   temp,
		   ( nvram_match( temp, "0" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp, "</div>\n" );
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.shaper)</script></div>\n" );
	sprintf( temp, "oet%d_shaper", tun );
	websWrite( wp,
		   "<input size=\"6\" maxlength=\"6\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,100000,eoip.shaper)\" value=\"%s\" />\n",
		   temp, nvram_get( temp ) );
	websWrite( wp, "</div>\n" );
*/
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.bridging)</script></div>\n");
		sprintf(temp, "oet%d_bridged", tun);
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idbridged%d', false)\" /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			  temp, (nvram_match(temp, "1") ? "checked=\"checked\"" : ""), tun);
		websWrite(wp,
			  " <input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idbridged%d', true)\" /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			  temp, (nvram_match(temp, "0") ? "checked=\"checked\"" : ""), tun);
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div id=\"idbridged%d\">\n", tun);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"oet%d_ipaddr\" value=\"0.0.0.0\"/>\n", tun);
		sprintf(temp, "oet%d_ipaddr", tun);
		websWrite(wp,
			  "<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_0\" onblur=\"valid_range(this,0,255,share.ip)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_1\" onblur=\"valid_range(this,0,255,share.ip)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_2\" onblur=\"valid_range(this,0,255,share.ip)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_3\" onblur=\"valid_range(this,1,254,share.ip)\" value=\"%d\" />\n",
			  temp, get_single_ip(nvram_safe_get(temp), 0), temp, get_single_ip(nvram_safe_get(temp), 1), temp, get_single_ip(nvram_safe_get(temp), 2), temp, get_single_ip(nvram_safe_get(temp), 3));
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"oet%d_netmask\" value=\"0.0.0.0\"/>\n", tun);
		sprintf(temp, "oet%d_netmask", tun);
		websWrite(wp,
			  "<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_0\" onblur=\"valid_range(this,0,255,share.subnet)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_1\" onblur=\"valid_range(this,0,255,share.subnet)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_2\" onblur=\"valid_range(this,0,255,share.subnet)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_3\" onblur=\"valid_range(this,0,254,share.subnet)\" value=\"%d\" />\n",
			  temp, get_single_ip(nvram_safe_get(temp), 0), temp, get_single_ip(nvram_safe_get(temp), 1), temp, get_single_ip(nvram_safe_get(temp), 2), temp, get_single_ip(nvram_safe_get(temp), 3));
		websWrite(wp, "</div>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</fieldset><br/>\n");
	}
}
#endif
