/*
 * eoptunnel.c
 *
 * Copyright (C) 2005 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

void show_cidr(webs_t wp, char *prefix, char *var, int nm, char *type, char *nmtype, char *nmname);
void show_ip(webs_t wp, char *prefix, char *var, int nm, int invalid, char *type);
void show_caption(webs_t wp, const char *class, const char *cap, const char *ext);
void show_caption_simple(webs_t wp, const char *cap);

static void show_oet_checkbox(webs_t wp, const char *label, const char *fmt, int id, int def, const char *ext)
{
	char temp[64];
	snprintf(temp, sizeof(temp), fmt, id);
	websWrite(wp, "<div class=\"setting\">\n");
	{
		show_caption(wp, "label", label, NULL);
		websWrite(wp, "<input type=\"hidden\" name=\"%s\" id=\"%s\" value=\"0\" />\n", temp, temp);
		char attrib[64] = { 0 };
		if (ext)
			snprintf(attrib, sizeof(attrib), ext, id);
		websWrite(wp, "<input class=\"spaceradio\" type=\"checkbox\" name=\"%s\" value=\"1\" %s %s/>\n", temp,
			  (nvram_default_matchi(temp, 1, def) ? "checked=\"checked\"" : ""), attrib);
	}
	websWrite(wp, "</div>\n");
}

static void show_oet_checkbox_peer(webs_t wp, const char *label, const char *fmt, int id, int peer, int def, const char *ext)
{
	char temp[64];
	snprintf(temp, sizeof(temp), fmt, id, peer);
	websWrite(wp, "<div class=\"setting\">\n");
	{
		show_caption(wp, "label", label, NULL);
		websWrite(wp, "<input type=\"hidden\" name=\"%s\" id=\"%s\" value=\"0\" />\n", temp, temp);
		char attrib[64] = { 0 };
		if (ext)
			snprintf(attrib, sizeof(attrib), ext, id);
		websWrite(wp, "<input class=\"spaceradio\" type=\"checkbox\" name=\"%s\" value=\"1\" %s %s/>\n", temp,
			  (nvram_default_matchi(temp, 1, def) ? "checked=\"checked\"" : ""), attrib);
	}
	websWrite(wp, "</div>\n");
}

static void show_oet_radio(webs_t wp, const char *label, const char *fmt, int id, int def, const char *exttrue,
			   const char *extfalse)
{
	char temp[64];

	snprintf(temp, sizeof(temp), fmt, id);
	websWrite(wp, "<div class=\"setting\">\n");
	{
		char attrib1[128] = { 0 };
		char attrib2[128] = { 0 };
		if (exttrue)
			snprintf(attrib1, sizeof(attrib1), exttrue, id, id);
		if (extfalse)
			snprintf(attrib2, sizeof(attrib2), extfalse, id, id);
		show_caption(wp, "label", label, NULL);
		websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s %s />", temp, attrib1,
			  (nvram_default_matchi(temp, 1, def) ? "checked=\"checked\"" : ""));
		show_caption(wp, NULL, "share.enable", "&nbsp;");
		websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s %s />", temp, attrib2,
			  (nvram_default_matchi(temp, 0, def) ? "checked=\"checked\"" : ""));
		show_caption_simple(wp, "share.disable");
	}
	websWrite(wp, "</div>\n");
}

static void show_oet_radio_peer(webs_t wp, const char *label, const char *fmt, int id, int peer, int def, const char *exttrue,
				const char *extfalse)
{
	char temp[64];

	snprintf(temp, sizeof(temp), fmt, id, peer);
	websWrite(wp, "<div class=\"setting\">\n");
	{
		char attrib1[128] = { 0 };
		char attrib2[128] = { 0 };
		if (exttrue)
			snprintf(attrib1, sizeof(attrib1), exttrue, id, peer);
		if (extfalse)
			snprintf(attrib2, sizeof(attrib2), extfalse, id, peer);
		show_caption(wp, "label", label, NULL);
		websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s %s />", temp, attrib1,
			  (nvram_default_matchi(temp, 1, def) ? "checked=\"checked\"" : ""));
		show_caption(wp, NULL, "share.enable", "&nbsp;");
		websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s %s />", temp, attrib2,
			  (nvram_default_matchi(temp, 0, def) ? "checked=\"checked\"" : ""));
		show_caption_simple(wp, "share.disable");
	}
	websWrite(wp, "</div>\n");
}

static void show_oet_num(webs_t wp, const char *label, int size, int maxlength, int def, int id, const char *ext, const char *fmt,
			 ...)
{
	va_list args;
	va_start(args, fmt);
	char *buf;

	websWrite(wp, "<div class=\"setting\">\n");
	{
		show_caption(wp, "label", label, NULL);
		char attrib[128] = { 0 };
		if (ext)
			snprintf(attrib, sizeof(attrib), ext, id);
		vasprintf(&buf, fmt, args);
		if (def != -1)
			websWrite(wp, "<input size=\"%d\" maxlength=\"%d\" name=\"%s\" class=\"num\" value=\"%d\" %s/>\n", size,
				  maxlength, buf, nvram_default_geti(buf, def), attrib);
		else
			websWrite(wp, "<input size=\"%d\" maxlength=\"%d\" name=\"%s\" class=\"num\" value=\"%s\" %s/>\n", size,
				  maxlength, buf, nvram_safe_get(buf), attrib);
		free(buf);
	}
	websWrite(wp, "</div>\n");
	va_end(args);
}

static void show_oet_textfield(webs_t wp, int pass, const char *label, int size, int maxlength, const char *def, int id,
			       const char *ext, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char *buf;
	websWrite(wp, "<div class=\"setting\">\n");
	{
		char attrib[128] = { 0 };
		vasprintf(&buf, fmt, args);
		if (ext)
			snprintf(attrib, sizeof(attrib), ext, id);
		show_caption(wp, "label", label, NULL);
		if (pass)
			websWrite(wp, "<input type=\"password\" size=\"%d\" maxlength=\"%d\" name=\"%s\" value=\"%s\" %s/>\n", size,
				  maxlength, buf, nvram_default_get(buf, def), attrib);
		else
			websWrite(wp, "<input size=\"%d\" maxlength=\"%d\" name=\"%s\" value=\"%s\" %s/>\n", size, maxlength, buf,
				  nvram_default_get(buf, def), attrib);
		free(buf);
	}
	websWrite(wp, "</div>\n");
	va_end(args);
}

EJ_VISIBLE void ej_show_eop_tunnels(webs_t wp, int argc, char_t **argv)
{
	int tun;
	char temp[128];
	char temp2[128];
	int tunnels = nvram_default_geti("oet_tunnels", 0) + 1;

	for (tun = 1; tun < tunnels; tun++) {
		char oet[32];
		sprintf(oet, "oet%d", tun);
		websWrite(wp, "<fieldset>\n");
		//name legend
		snprintf(temp, sizeof(temp), "oet%d_label", tun);
		{
			websWrite(wp, "<legend>");
			show_caption_simple(wp, "eoip.tunnel");
			//websWrite(wp, " %s</legend>\n", getNetworkLabel(wp, oet));
			websWrite(wp, " %s - <input type=\"text\" size=\"14\" id=\"%s\" name=\"%s\" value=\"%s\" ></legend>\n", oet,
				  temp, temp, nvram_safe_get(temp));
		}
		show_oet_radio(wp, "eoip.srv", "oet%d_en", tun, 0, "onclick=\"show_layer_ext(this, 'idoet%d', true)\"",
			       "onclick=\"show_layer_ext(this, 'idoet%d', false)\"");
		websWrite(wp, "<div id=\"idoet%d\">\n", tun);
		{
			snprintf(temp, sizeof(temp), "oet%d_proto", tun);
			websWrite(wp, "<div class=\"setting\">\n");
			{
				show_caption(wp, "label", "eoip.proto", NULL);
				websWrite(wp, "<select name=\"%s\" onclick=\"changeproto(this, %d, this.value, %s, %s, %s)\">\n",
					  temp, tun, nvram_nget("oet%d_bridged", tun), nvram_nget("vxlan%d_bridged", tun),
					  nvram_nmatchi(1, "oet%d_mcast", tun));

				websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
				websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\"  + eoip.etherip + \"</option>\");\n",
					  nvram_matchi(temp, 0) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\"  + eoip.mtik + \"</option>\");\n",
					  nvram_matchi(temp, 1) ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_WIREGUARD
				websWrite(wp,
					  "document.write(\"<option value=\\\"2\\\" %s >\"  + eoip.wireguard + \"</option>\");\n",
					  nvram_matchi(temp, 2) ? "selected=\\\"selected\\\"" : "");
#endif
#ifdef HAVE_IPV6
				websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >\"  + eoip.vxlan + \"</option>\");\n",
					  nvram_matchi(temp, 3) ? "selected=\\\"selected\\\"" : "");
#endif
				websWrite(wp, "//]]>\n</script>\n");
				websWrite(wp, "</select>\n");
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idmtik%d\">\n", tun);
			{
				show_oet_num(wp, "eoip.tunnelID", 4, 3, -1, tun,
					     "onblur=\"valid_range(this,0,65535,eoip.tunnelID)\"", "oet%d_id", tun);
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idvxlan%d\">\n", tun);
			{
				show_oet_num(wp, "eoip.tunnelID", 8, 8, -1, tun,
					     "onblur=\"valid_range(this,0,16777215,eoip.tunnelID)\"", "oet%d_id", tun);
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idwireguard%d\">\n", tun);
			{
#ifdef HAVE_WIREGUARD
				show_oet_checkbox(wp, "service.vpn_mit", "oet%d_mit", tun, 0, NULL);
				show_oet_checkbox(wp, "eoip.wireguard_oet_natout", "oet%d_natout", tun, 0, NULL);
				show_oet_radio(wp, "eoip.wireguard_obfuscation", "oet%d_obf", tun, 0,
					       "onclick=\"show_layer_ext(this, 'idoet%d_showobf', true)\"",
					       "onclick=\"show_layer_ext(this, 'idoet%d_showobf', false)\"");
				websWrite(wp, "<div id=\"idoet%d_showobf\">\n",
					  tun); //for show or hide advanced options
				{
					show_oet_textfield(
						wp, 1, "share.key", 32, 32, "", tun,
						"onmouseover=\"this.type=\'text\'\" onmouseout=\"this.type=\'password\'\"",
						"oet%d_obfkey", tun);
				}
				websWrite(wp, "</div>\n");

				show_oet_num(wp, "eoip.wireguard_localport", 5, 5, 51820, tun, NULL, "oet%d_port", tun);

				{
					int mtu = 1500;
					if (!nvram_match("wan_proto", "disabled"))
						mtu = nvram_geti("wan_mtu");
					mtu -= nvram_matchi("ipv6_enable", 1) ? 80 : 60;
					show_oet_num(wp, "idx.mtu", 5, 5, mtu, tun, NULL, "oet%d_mtu", tun);
				}

				websWrite(wp, "<div class=\"center\">\n");
				{
					websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
					websWrite(
						wp,
						"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"gen_key_button\\\" value=\\\"\" + eoip.genkey + \"\\\" onclick=\\\"gen_wg_key(this.form,%d)\\\" />\");\n",
						tun);
					websWrite(wp, "//]]>\n</script>\n");
				}
				websWrite(wp, "</div>\n");

				//public key show
				show_oet_textfield(wp, 0, "eoip.wireguard_localkey", 48, 48, "", tun, "readonly=\"readonly\"",
						   "oet%d_public", tun);

				//egc: DNS
				show_oet_textfield(wp, 0, "eoip.wireguard_oet_dns", 32, 48, "", tun, NULL, "oet%d_dns", tun);

				//Inbound firewall controlled by nvram parameter: oet${i}_firewallin default 0 = disabled, Checkbox
				show_oet_checkbox(wp, "eoip.wireguard_firewallin", "oet%d_firewallin", tun, 0, NULL);

				//egc Kill switch nvram param: oet%d_killswitch, Checkbox
				show_oet_checkbox(wp, "eoip.wireguard_killswitch", "oet%d_killswitch", tun, 0, NULL);

				//show or hide advanced options
				show_oet_radio(wp, "eoip.wireguard_showadvanced", "oet%d_showadvanced", tun, 0,
					       "onclick=\"show_layer_ext(this, 'idoet%d_showadvanced', true)\"",
					       "onclick=\"show_layer_ext(this, 'idoet%d_showadvanced', false)\"");
				//end show

				websWrite(wp, "<div id=\"idoet%d_showadvanced\">\n",
					  tun); //for show or hide advanced options
				{
					//websWrite(wp, "<div id=\"idoet%d_showadvanced\" style=\"display:none\">\n", tun);  //for show or hide advanced options

					//egc set private key
					show_oet_textfield(
						wp, 1, "eoip.wireguard_localprivatekey", 48, 48, "", tun,
						"onmouseover=\"this.type=\'text\'\" onmouseout=\"this.type=\'password\'\"",
						"oet%d_private", tun);

					//route up script
					show_oet_textfield(wp, 0, "eoip.wireguard_rtupscript", 48, 64, "", tun, NULL,
							   "oet%d_rtupscript", tun);

					//route down script
					show_oet_textfield(wp, 0, "eoip.wireguard_rtdownscript", 48, 64, "", tun, NULL,
							   "oet%d_rtdownscript", tun);

					//fwmark
					show_oet_num(wp, "eoip.wireguard_fwmark", 5, 5, -1, tun, NULL, "oet%d_fwmark", tun);

					//egc full wan access, NAT over WAN, nvram param: oet%d_wanac, Checkbox
					show_oet_checkbox(wp, "service.vpnd_allowcnwan", "oet%d_wanac", tun, 1, NULL);

					//egc full lan access, NAT over br0 nvram param: oet%d_lanac, Checkbox
					show_oet_checkbox(wp, "eoip.wireguard_lanac", "oet%d_lanac", tun, 0, NULL);

					//egc Reverse PBR switch nvram param: oet%d_spbr
					snprintf(temp, sizeof(temp), "oet%d_spbr", tun);
					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_spbr", NULL);
						websWrite(wp, "<select name=\"%s\" onclick=\"changespbr(this, %d, this.value)\">\n",
							  temp, tun);
						websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
						websWrite(
							wp,
							"document.write(\"<option value=\\\"0\\\" %s >\"  + eoip.wireguard_spbr0 + \"</option>\");\n",
							nvram_matchi(temp, 0) ? "selected=\\\"selected\\\"" : "");
						websWrite(
							wp,
							"document.write(\"<option value=\\\"1\\\" %s >\"  + eoip.wireguard_spbr1 + \"</option>\");\n",
							nvram_matchi(temp, 1) ? "selected=\\\"selected\\\"" : "");
						websWrite(
							wp,
							"document.write(\"<option value=\\\"2\\\" %s >\"  + eoip.wireguard_spbr2 + \"</option>\");\n",
							nvram_matchi(temp, 2) ? "selected=\\\"selected\\\"" : "");
						websWrite(wp, "//]]>\n</script>\n");
						websWrite(wp, "</select>\n");
					}
					websWrite(wp,
						  "</div>\n"); //end spbr switch

					websWrite(wp, "<div id=\"idoet%d_spbr\">\n",
						  tun); //for show or hide spbr input
					{ //egc: SPBR input box
						show_oet_textfield(wp, 0, "eoip.wireguard_oet_spbr_ip", 78, 1024, "", tun, NULL,
								   "oet%d_spbr_ip", tun);
						show_oet_checkbox(wp, "eoip.wireguard_dnspbr", "oet%d_dnspbr", tun, 0,
								  "onclick=\"changedns46(this, %d)\"");
						websWrite(wp, "<div id=\"idoet%d_dns46\">\n",
							  tun); //for show or hide dns4 and dns6
						{
							show_oet_textfield(wp, 0, "eoip.wireguard_dns4", 16, 16, "9.9.9.9", tun,
									   NULL, "oet%d_dns4", tun);
							if (nvram_matchi("ipv6_enable", 1)) {
								snprintf(temp, sizeof(temp), "oet%d_dns6", tun);
								show_oet_textfield(wp, 0, "eoip.wireguard_dns6", 40, 40,
										   "2620:fe::9", tun, NULL, "oet%d_dns6", tun);
							}
						}
						websWrite(wp, "</div>\n");
					}
					websWrite(wp,
						  "</div>\n"); //end show hide spbr

					//egc destination PBR switch nvram param: oet%d_dpbr
					snprintf(temp, sizeof(temp), "oet%d_dpbr", tun);
					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_dpbr", NULL);
						websWrite(wp, "<select name=\"%s\" onclick=\"changedpbr(this, %d, this.value)\">\n",
							  temp, tun);
						websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
						websWrite(
							wp,
							"document.write(\"<option value=\\\"0\\\" %s >\"  + eoip.wireguard_dpbr0 + \"</option>\");\n",
							nvram_matchi(temp, 0) ? "selected=\\\"selected\\\"" : "");
						websWrite(
							wp,
							"document.write(\"<option value=\\\"1\\\" %s >\"  + eoip.wireguard_dpbr1 + \"</option>\");\n",
							nvram_matchi(temp, 1) ? "selected=\\\"selected\\\"" : "");
						websWrite(
							wp,
							"document.write(\"<option value=\\\"2\\\" %s >\"  + eoip.wireguard_dpbr2 + \"</option>\");\n",
							nvram_matchi(temp, 2) ? "selected=\\\"selected\\\"" : "");
						websWrite(wp, "//]]>\n</script>\n");
						websWrite(wp, "</select>\n");
					}
					websWrite(wp,
						  "</div>\n"); //end dpbr switch

					websWrite(wp, "<div id=\"idoet%d_dpbr\">\n",
						  tun); //for show or hide dpbr
					{ //egc: DPBR input box
						show_oet_textfield(wp, 0, "eoip.wireguard_oet_dpbr_ip", 78, 1024, "", tun, NULL,
								   "oet%d_dpbr_ip", tun);
					}
					websWrite(wp,
						  "</div>\n"); //end show hide dpbr

					//egc Check if tunnel is part of fail group, Checkbox
					websWrite(wp, "<div id=\"idoet%d_failgrp\">\n",
						  tun); //show/hide failgrp
					{
						show_oet_radio(wp, "eoip.wireguard_failgrp", "oet%d_failgrp", tun, 0,
							       "onclick=\"failover_show(this, %d, this.value)\"",
							       "onclick=\"failover_show(this, %d, this.value)\"");

						websWrite(wp, "<div id=\"idoet%d_tunnelstate\">\n",
							  tun); //show or hide tunnel state only if failgrp is checked
						{
							websWrite(wp, "<div class=\"setting\">\n");
							snprintf(temp, sizeof(temp), "oet%d_failstate", tun);
							show_caption(wp, "label", "eoip.wireguard_failstate", NULL);
							websWrite(wp, "<select name=\"%s\">\n", temp);
							websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
							websWrite(
								wp,
								"document.write(\"<option value=\\\"0\\\" %s >\"  + eoip.wireguard_standby + \"</option>\");\n",
								nvram_matchi(temp, 0) ? "selected=\\\"selected\\\"" : "");
							websWrite(
								wp,
								"document.write(\"<option value=\\\"2\\\" %s >\"  + eoip.wireguard_running + \"</option>\");\n",
								nvram_matchi(temp, 2) ? "selected=\\\"selected\\\"" : "");
							websWrite(
								wp,
								"document.write(\"<option value=\\\"1\\\" %s >\"  + eoip.wireguard_failed + \"</option>\");\n",
								nvram_matchi(temp, 1) ? "selected=\\\"selected\\\"" : "");
							websWrite(wp, "//]]>\n</script>\n");
							websWrite(wp, "</select>\n");
							websWrite(wp, "</div>\n");
							//end tunnel state
						}
						websWrite(wp,
							  "</div>\n"); //end show hide tunnelstae
					}
					websWrite(wp,
						  "</div>\n"); //end show hide failgrp

					websWrite(wp, "<div id=\"idoet%d_wdog2\">\n", tun);
					{
						show_oet_radio(wp, "service.vpn_wdog", "oet%d_wdog", tun, 0,
							       "onclick=\"wdog_show(this, %d, this.value)\"",
							       "onclick=\"wdog_show(this, %d, this.value)\"");
					}
					websWrite(wp, "</div>\n");

					//Watchdog
					websWrite(wp, "<div id=\"idoet%d_wdog\">\n", tun);
					{
						websWrite(wp, "<fieldset>\n");
						show_caption_legend(wp, "service.vpn_wdog");
						show_oet_textfield(wp, 0, "share.srvipname", 20, 48, "8.8.8.8", tun, NULL,
								   "oet%d_failip", tun);
						show_oet_textfield(wp, 0, "alive.timeout", 5, 5, "30", tun,
								   "onblur=\"valid_range(this,10,300,alive.timeout)\"",
								   "oet%d_failtime", tun);
						websWrite(wp, "</fieldset>\n");
					}
					websWrite(wp, "</div>\n");
				}
				websWrite(wp,
					  "</div>\n"); //end show hide Advanced settings

				snprintf(temp, sizeof(temp), "oet%d_peers", tun);
				int peers = nvram_default_geti(temp, 0);
				int peer;
				for (peer = 0; peer < peers; peer++) {
					websWrite(wp, "<br><fieldset>\n");

					//name legend
					snprintf(temp, sizeof(temp), "oet%d_namep%d", tun, peer);
					{
						websWrite(
							wp,
							"<legend>%d. <input type=\"text\" size=\"14\" id=\"%s\" name=\"%s\" value=\"%s\" ></legend>\n",
							peer + 1, temp, temp, nvram_safe_get(temp));
					}
					//Box for items in Client config file, definition of show_ip, show_caption etc is in src/router/httpd/visuals/ddwrt.c
					websWrite(wp, "<fieldset>\n");
					//websWrite(wp, "<legend>Client config file</legend>\n");
					show_caption_legend(wp, "eoip.wireguard_cllegend");
					//Show/Hide Client Config
					show_oet_radio_peer(wp, "eoip.wireguard_cllegend", "oet%d_clconfig%d", tun, peer, 0,
							    "onclick=\"show_layer_ext(this, 'idclconfig%d_peer%d', true)\"",
							    "onclick=\"show_layer_ext(this, 'idclconfig%d_peer%d', false)\"");

					//end show Client config
					websWrite(wp, "<div id=\"idclconfig%d_peer%d\">\n", tun, peer);
					{
						show_oet_textfield(wp, 0, "eoip.wireguard_peerip", 48, 128, "0.0.0.0/0", tun, NULL,
								   "oet%d_clip%d", tun, peer);
						show_oet_textfield(wp, 0, "eoip.wireguard_peerdns", 48, 128, "0.0.0.0", tun, NULL,
								   "oet%d_cldns%d", tun, peer);
						show_oet_textfield(wp, 0, "eoip.wireguard_clend", 40, 48, "", tun, NULL,
								   "oet%d_clend%d", tun, peer);
						show_oet_num(wp, "eoip.wireguard_clka", 5, 5, 25, tun, NULL, "oet%d_clka%d", tun,
							     peer);
					}
					websWrite(wp,
						  "</div>\n"); // end show/hide idclconfig
					websWrite(wp, "</fieldset><br>\n");

					show_oet_radio_peer(wp, "eoip.wireguard_endpoint", "oet%d_endpoint%d", tun, peer, 0,
							    "onclick=\"show_layer_ext(this, 'idendpoint%d_peer%d', true)\"",
							    "onclick=\"show_layer_ext(this, 'idendpoint%d_peer%d', false)\"");
					websWrite(wp, "<div id=\"idendpoint%d_peer%d\">\n", tun, peer);
					{
						snprintf(temp2, sizeof(temp2), "oet%d_peerport%d", tun, peer);
						snprintf(temp, sizeof(temp), "oet%d_rem%d", tun, peer);
						websWrite(wp, "<div class=\"setting\">\n");
						{
							show_caption(wp, "label", "eoip.wireguard_peer", NULL);
							websWrite(
								wp,
								"<input size=\"40\" maxlength=\"48\" name=\"%s\" value=\"%s\" />:<input size=\"5\" maxlength=\"5\" name=\"%s\" class=\"num\" value=\"%s\" />\n\n",
								temp, nvram_safe_get(temp), temp2, nvram_safe_get(temp2));
						}
						websWrite(wp, "</div>\n");
						show_oet_radio_peer(
							wp, "eoip.wireguard_obfuscation", "oet%d_obf%d", tun, peer, 0,
							"onclick=\"show_layer_ext(this, 'idshowobf%d_peer%d', true)\"",
							"onclick=\"show_layer_ext(this, 'idshowobf%d_peer%d', false)\"");
						websWrite(wp, "<div id=\"idshowobf%d_peer%d\">\n", tun,
							  peer); //for show or hide advanced options
						{
							show_oet_textfield(
								wp, 1, "share.key", 32, 32, "", tun,
								"onmouseover=\"this.type=\'text\'\" onmouseout=\"this.type=\'password\'\"",
								"oet%d_obfkey%d", tun, peer);
						}
						websWrite(wp, "</div>\n");
					}
					websWrite(wp, "</div>\n");
					snprintf(temp, sizeof(temp), "oet%d_aip%d", tun, peer);
					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_allowedips", NULL);
						websWrite(wp, "<input size=\"72\" maxlength=\"1024\" name=\"%s\" value=\"%s\" />\n",
							  temp, nvram_default_get(temp, "0.0.0.0/0"));
					}
					websWrite(wp, "</div>\n");

					//egc: route allowed IP's, controlled by nvram oet${i}_aip_rten${p}
					show_oet_checkbox_peer(wp, "eoip.wireguard_route_allowedip", "oet%d_aip_rten%d", tun, peer,
							       1, NULL);

					show_oet_num(wp, "eoip.wireguard_ka", 5, 5, -1, tun, NULL, "oet%d_ka%d", tun, peer);
					//public key peer input
					show_oet_textfield(wp, 0, "eoip.wireguard_peerkey", 48, 48, "", tun, NULL,
							   "oet%d_peerkey%d", tun, peer);

					show_oet_radio_peer(wp, "eoip.wireguard_usepsk", "oet%d_usepsk%d", tun, peer, 0,
							    "onclick=\"show_layer_ext(this, 'idpsk%d_peer%d', true)\"",
							    "onclick=\"show_layer_ext(this, 'idpsk%d_peer%d', false)\"");

					websWrite(wp, "<div id=\"idpsk%d_peer%d\">\n", tun, peer);
					{
						websWrite(wp, "<div class=\"center\">\n");
						{
							websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
							websWrite(
								wp,
								"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"gen_psk_button\\\" value=\\\"\" + eoip.wireguard_genpsk + \"\\\" onclick=\\\"gen_wg_psk(this.form,%d,%d)\\\" />\");\n",
								tun, peer);
							websWrite(wp, "//]]>\n</script>\n");
						}
						websWrite(wp, "</div>\n");
						show_oet_textfield(wp, 0, "eoip.wireguard_psk", 48, 48, "", tun, NULL,
								   "oet%d_psk%d", tun, peer);
					}
					websWrite(wp, "</div>\n");
					int hasqr = 0;
					if (nvram_nexists("oet%d_peerpk%d", tun, peer)) {
						char svgpath[64];
						char s_tun[32];
						char s_peer[32];
						char confpath[64];
						sprintf(s_tun, "%d", tun);
						sprintf(s_peer, "%d", peer);
						sprintf(confpath, "/tmp/wireguard/oet%d_peer%d_conf", tun, peer);
						eval("makewgconfig", s_tun, s_peer);
						sprintf(svgpath, "/tmp/wireguard/oet%d_peer%d_svg", tun, peer);
						eval("qrencode", "-t", "svg", "--rle", "-r", confpath, "-o", svgpath);
						FILE *svg = fopen(svgpath, "rb");
						if (svg) {
							fseek(svg, 0, SEEK_END);
							int len = ftell(svg);
							rewind(svg);
							char *buf = calloc(1, len + 1);
							fread(buf, len, 1, svg);
							fclose(svg);
							websWrite(wp, "<div class=\"setting\">\n");
							wfputs(buf, wp);
							websWrite(wp, "<br><script type=\"text/javascript\">\n");
							websWrite(wp, "//<![CDATA[\n");
							websWrite(
								wp,
								"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"config_button\\\" value=\\\"\" + sbutton.download_config + \"\\\" onclick=\\\"window.location.href='/wireguard_config_oet%d_peer%d.conf';\\\" />\");\n",
								tun, peer);
							websWrite(wp, "//]]>\n");
							websWrite(wp, "</script>\n");
							websWrite(wp, "</div>\n");
							debug_free(buf);
							hasqr = 1;
						}
					}
					//Status box
					websWrite(wp, "<div id=\"idoet%d_statusbox\">\n",
						  tun); //naming for periodic refresh need also peer
					{
						websWrite(wp, "<div class=\"setting\">\n");
						{
							snprintf(temp, sizeof(temp), "oet%d_peerkey%d", tun, peer);
							char buffer[1024] = "Press F5 to refresh";
							int len = 0;
							char command[128];
							snprintf(command, sizeof(command),
								 "/usr/bin/wireguard-state.sh %d %s 2>/dev/null", tun,
								 nvram_safe_get(temp));
							FILE *in = popen(command, "r");
							if (in != NULL) {
								while ((len = fread(buffer, 1, 1023, in)) == 1023) {
									buffer[len] = 0;
								}
								if (len == 0) {
									char temp3[64];
									snprintf(temp3, sizeof(temp3), "oet%d_namep%d", tun, peer);
									sprintf(buffer, "No connection present for %s ",
										nvram_safe_get(temp3));
								}
							} else {
								sprintf(buffer, "Unable to retrieve WireGuardStatus oet%d", tun);
							}
							show_caption(wp, "label", "eoip.wireguard_oet_status", NULL);
							websWrite(
								wp,
								"<textarea cols=\"32\" rows=\"3\" id=\"oet%d_status%d\" name=\"oet%d_status%d\" disabled=\"disabled\"></textarea>",
								tun, peer, tun, peer);
							websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
							websWrite(wp,
								  "document.getElementById(\"oet%d_status%d\").value =\"%s\";\n",
								  tun, peer, buffer);
							websWrite(wp, "//]]>\n</script>\n");
							pclose(in);
						}
						websWrite(wp, "</div>\n");
					}
					websWrite(wp, "</div>\n");
					//end status box

					websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
					websWrite(wp, "show_layer_ext(this, 'idclconfig%d_peer%d',%s);\n", tun, peer,
						  nvram_nmatchi(1, "oet%d_clconfig%d", tun, peer) ? "true" : "false");
					websWrite(wp, "show_layer_ext(this, 'idpsk%d_peer%d',%s);\n", tun, peer,
						  nvram_nmatchi(1, "oet%d_usepsk%d", tun, peer) ? "true" : "false");
					websWrite(wp, "show_layer_ext(this, 'idendpoint%d_peer%d',%s);\n", tun, peer,
						  nvram_nmatchi(1, "oet%d_endpoint%d", tun, peer) ? "true" : "false");
					websWrite(wp, "show_layer_ext(this, 'idshowobf%d_peer%d',%s);\n", tun, peer,
						  nvram_nmatchi(1, "oet%d_obf%d", tun, peer) ? "true" : "false");
					websWrite(
						wp,
						"document.write(\"<input class=\\\"button red_btn\\\" type=\\\"button\\\" name=\\\"delete_peer\\\" value=\\\"\" + eoip.wireguard_delpeer + \"\\\" onclick=\\\"del_peer(this.form,%d,%d)\\\" />\");\n",
						tun, peer);
					websWrite(
						wp,
						"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"gen_qr\\\" value=\\\"\" + eoip.wireguard_makeclient + \"\\\" onclick=\\\"gen_wg_client(this.form,%d,%d)\\\" />\");\n",
						tun, peer);
					if (hasqr) {
						websWrite(
							wp,
							"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"del_qr\\\" value=\\\"\" + eoip.wireguard_cleanqr + \"\\\" onclick=\\\"del_wg_client(this.form,%d,%d)\\\" />\");\n",
							tun, peer);
					}
					websWrite(wp, "//]]>\n</script>\n");
					websWrite(wp, "</fieldset>\n");
				}

				websWrite(wp, "<br>\n");

				websWrite(wp, "<div class=\"center\">\n");
				{
					websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
					websWrite(
						wp,
						"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"add_peer_button\\\" value=\\\"\" + eoip.wireguard_addpeer + \"\\\" onclick=\\\"add_peer(this.form,%d)\\\" />\");\n",
						tun);
					websWrite(wp, "//]]>\n</script>\n");
				}
				websWrite(wp, "</div>\n");

				websWrite(wp, "<br>\n");

#endif
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idlocalip%d\">\n", tun);
			{
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.localIP", NULL);
					websWrite(wp, "<input type=\"hidden\" name=\"oet%d_local\" value=\"0.0.0.0\"/>\n", tun);
					snprintf(temp, sizeof(temp), "oet%d_local", tun);
					show_ip(wp, NULL, temp, 1, 0, "eoip.localIP");
				}
				websWrite(wp, "</div>\n");
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.remoteIP", NULL);
					websWrite(wp, "<input type=\"hidden\" name=\"oet%d_rem\" value=\"0.0.0.0\"/>\n", tun);
					snprintf(temp, sizeof(temp), "oet%d_rem", tun);
					show_ip(wp, NULL, temp, 0, 0, "eoip.remoteIP");
				}
				websWrite(wp, "</div>\n");
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idshowmcast%d\">\n", tun);
			{
				show_oet_radio(wp, "eoip.mcast", "oet%d_mcast", tun, 0,
					       "onclick=\"toggle_layer_ext(this, 'idmcast%d', 'idremoteip6%d', true)\"",
					       "onclick=\"toggle_layer_ext(this, 'idmcast%d', 'idremoteip6%d', false)\"");
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idmcast%d\">\n", tun);
			{
				show_oet_textfield(wp, 0, "eoip.mcast_group", 48, 128, "", tun, NULL, "oet%d_group", tun);
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idremoteip6%d\">\n", tun);
			{
				show_oet_textfield(wp, 0, "eoip.remoteIP", 48, 128, "", tun, NULL, "oet%d_remip6", tun);
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idlocalip6%d\">\n", tun);
			{
				show_oet_textfield(wp, 0, "eoip.localIP", 48, 128, "", tun, NULL, "oet%d_localip6", tun);
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idvxlansettings%d\">\n", tun);
			{
				char bufferif[512];
				char wan_if_buffer[33];
				char word[32];
				char *next;
				snprintf(temp, sizeof(temp), "oet%d_dev", tun);
				char *cmp = nvram_default_get(temp, "any");
				char *wanface = safe_get_wan_face(wan_if_buffer);
				bzero(bufferif, 512);
				getIfList(bufferif, NULL);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "share.intrface", NULL);
					websWrite(wp, "<select name=\"%s\">\n", temp);
					websWrite(wp, "<option value=\"any\" %s >ANY</option>\n",
						  !strcmp("any", cmp) ? "selected=\"selected\"" : "");
					websWrite(wp, "<option value=\"%s\" %s >LAN &amp; WLAN</option>\n",
						  nvram_safe_get("lan_ifname"),
						  nvram_match("lan_ifname", cmp) ? "selected=\"selected\"" : "");
					if (strcmp(wanface, "br0")) {
						websWrite(wp, "<option value=\"%s\" %s >WAN</option>\n", wanface,
							  !strcmp(wanface, cmp) ? "selected=\"selected\"" : "");
					}
					foreach(word, bufferif, next)
					{
						if (nvram_match("lan_ifname", word))
							continue;
						if (strcmp(wanface, "br0")) {
							if (!strcmp(wanface, cmp))
								continue;
						}
						if (!strncmp(word, "vxlan", 5))
							continue;
						if (isbridged(word))
							continue;
						websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", word,
							  strcmp(word, cmp) == 0 ? "selected=\"selected\"" : "",
							  getNetworkLabel(wp, word));
					}
					websWrite(wp, "</select>\n");
				}
				websWrite(wp, "</div>\n");
				show_oet_num(wp, "eoip.dport", 5, 5, 4789, tun, NULL, "oet%d_dstport", tun);

				char min[32];
				char max[32];
				snprintf(min, sizeof(min), "oet%d_srcmin", tun);
				snprintf(max, sizeof(max), "oet%d_srcmax", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.sportrange", NULL);
					websWrite(
						wp,
						"<input name=\"%s\" size=\"5\" maxlength=\"5\" class=\"num\" value=\"%s\" /> ~ <input name=\"%s\" size=\"5\" maxlength=\"5\" class=\"num\" value=\"%s\" />\n",
						min, nvram_default_get(min, "0"), max, nvram_default_get(max, "0"));
				}
				websWrite(wp, "</div>\n");
				show_oet_num(wp, "eoip.ttl", 5, 5, 0, tun, NULL, "oet%d_ttl", tun);
				show_oet_num(wp, "eoip.tos", 5, 5, 0, tun, NULL, "oet%d_tos", tun);

				show_oet_checkbox(wp, "eoip.lrn", "oet%d_lrn", tun, 1, NULL);
				show_oet_checkbox(wp, "eoip.proxy", "oet%d_proxy", tun, 0, NULL);
				show_oet_checkbox(wp, "eoip.rsc", "oet%d_rsc", tun, 0, NULL);
				show_oet_checkbox(wp, "eoip.l2miss", "oet%d_l2miss", tun, 0, NULL);
				show_oet_checkbox(wp, "eoip.l3miss", "oet%d_l3miss", tun, 0, NULL);
				/* kernel 4.4 or higher */
				show_oet_checkbox(wp, "eoip.udpcsum", "oet%d_udpcsum", tun, 0, NULL);
				show_oet_checkbox(wp, "eoip.udp6zerocsumtx", "oet%d_udp6zerocsumtx", tun, 0, NULL);
				show_oet_checkbox(wp, "eoip.udp6zerocsumrx", "oet%d_udp6zerocsumrx", tun, 0, NULL);
				/* kernel 3.10 or higher */
				show_oet_checkbox(wp, "eoip.df", "oet%d_df", tun, 0, NULL);

				show_oet_num(wp, "eoip.ageing", 5, 5, 300, tun, NULL, "oet%d_ageing", tun);
				show_oet_num(wp, "eoip.flowlabel", 5, 5, -1, tun, NULL, "oet%d_fl", tun);
				show_oet_radio(wp, "eoip.bridging", "vxlan%d_bridged", tun, 0,
					       "onclick=\"show_layer_ext(this, 'idvxlanbridged%d', false)\"",
					       "onclick=\"show_layer_ext(this, 'idvxlanbridged%d', true)\"");
			}
			websWrite(wp, "</div>\n");

			websWrite(wp, "<div id=\"idl2support%d\">\n", tun);
			{
				show_oet_radio(wp, "eoip.bridging", "oet%d_bridged", tun, 0,
					       "onclick=\"show_layer_ext(this, 'idbridged%d', false)\"",
					       "onclick=\"show_layer_ext(this, 'idbridged%d', true)\"");
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idbridged%d\">\n", tun);
			{
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "share.ip", NULL);
					websWrite(wp, "<input type=\"hidden\" name=\"oet%d_ipaddr\" value=\"0.0.0.0\"/>\n", tun);
					snprintf(temp, sizeof(temp), "oet%d_ipaddr", tun);
					snprintf(temp2, sizeof(temp2), "oet%d_netmask", tun);
					show_ip_cidr(wp, NULL, temp, 0, "share.ip", temp2, "share.subnet");
				}
				websWrite(wp, "</div>\n");
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idvxlanbridged%d\">\n", tun);
			{
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "share.ip", NULL);
					websWrite(wp, "<input type=\"hidden\" name=\"vxlan%d_ipaddr\" value=\"0.0.0.0\"/>\n", tun);
					snprintf(temp, sizeof(temp), "vxlan%d_ipaddr", tun);
					snprintf(temp2, sizeof(temp2), "vxlan%d_netmask", tun);
					show_ip_cidr(wp, NULL, temp, 0, "share.ip", temp2, "share.subnet");
				}
				websWrite(wp, "</div>\n");
			}
			websWrite(wp, "</div>\n");
			// Alternative input for ipaddress and netmask to add multiple addresses including IPv6 e.g.: 10.0.0.2/16, fc00::2/96
			// nvram variable oet%d_ipaddrmask
			websWrite(wp, "<div id=\"idwginput%d\">\n", tun);
			{
				show_oet_textfield(wp, 0, "share.ipaddrmask", 48, 128, "", tun, NULL, "oet%d_ipaddrmask", tun);
				//end alternative input
			}
			websWrite(wp, "</div>\n");
		}
		websWrite(wp, "</div>\n");
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(
			wp,
			"document.write(\"<input class=\\\"button red_btn\\\" type=\\\"button\\\" name=\\\"del_button\\\" value=\\\"\" + eoip.del + \"\\\" onclick=\\\"del_tunnel(this.form,%d)\\\" />\");\n",
			tun);
		websWrite(wp, "toggle_layer_ext(document.eop.oet%d_en, 'idmcast%d', 'idremoteip6%d', %s);\n", tun, tun, tun,
			  nvram_nmatchi(1, "oet%d_mcast", tun) ? "true" : "false");
		websWrite(wp, "changeproto(document.eop.oet%d_proto, %d, %s, %s, %s, %s);\n", tun, tun,
			  nvram_nget("oet%d_proto", tun), nvram_nget("oet%d_bridged", tun), nvram_nget("vxlan%d_bridged", tun),
			  nvram_nmatchi(1, "oet%d_mcast", tun));
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d', %s);\n", tun, tun,
			  nvram_nmatchi(1, "oet%d_en", tun) ? "true" : "false");
		//hide or show advanced settings
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_showadvanced',%s);\n", tun, tun,
			  nvram_nmatchi(1, "oet%d_showadvanced", tun) ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idshowmcast%d',%s);\n", tun, tun,
			  nvram_nmatchi(3, "oet%d_proto", tun) ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idremoteip6%d',%s);\n", tun, tun,
			  nvram_nmatchi(3, "oet%d_proto", tun) ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idlocalip6%d',%s);\n", tun, tun,
			  nvram_nmatchi(3, "oet%d_proto", tun) ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idvxlansettings%d',%s);\n", tun, tun,
			  nvram_nmatchi(3, "oet%d_proto", tun) ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_showobf',%s);\n", tun, tun,
			  nvram_nmatchi(1, "oet%d_obf", tun) ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_tunnelstate', %s);\n", tun, tun,
			  nvram_nmatchi(1, "oet%d_failgrp", tun) ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_wdog', %s);\n", tun, tun,
			  (nvram_nmatchi(1, "oet%d_failgrp", tun) || nvram_nmatchi(1, "oet%d_wdog", tun)) ? "true" : "false");
		//websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_wdog', %s);\n", tun, tun, nvram_nmatchi(1, "oet%d_failgrp", tun) ? "true" : "false");
		//websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_wdog', %s);\n", tun, tun, nvram_nmatchi(1, "oet%d_wdog", tun) ? "true" : "false");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_wdog2', %s);\n", tun, tun,
			  nvram_nmatchi(1, "oet%d_failgrp", tun) ? "false" : "true");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_failgrp', %s);\n", tun, tun,
			  nvram_nmatchi(1, "oet%d_wdog", tun) ? "false" : "true");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_spbr',%s);\n", tun, tun,
			  nvram_nmatchi(0, "oet%d_spbr", tun) ? "false" : "true");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_dpbr',%s);\n", tun, tun,
			  nvram_nmatchi(0, "oet%d_dpbr", tun) ? "false" : "true");
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_dns46',%s);\n", tun, tun,
			  nvram_nmatchi(0, "oet%d_dnspbr", tun) ? "false" : "true");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</fieldset><br>\n");
	}
	websWrite(wp, "<div class=\"center\">\n");
	{
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(
			wp,
			"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"add_button\\\" value=\\\"\" + eoip.add + \"\\\" onclick=\\\"add_tunnel(this.form)\\\" />\");\n");
		//show filepicker
		websWrite(
			wp,
			"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"import_tunnel_button\\\" value=\\\"\" + eoip.importt + \"\\\" onclick=\\\"show_layer_ext(this.form, 'idwgimport', true);\\\" />\");\n");
		websWrite(wp, "//]]>\n</script>\n");
	}
	websWrite(wp, "</div>\n");

	//make hidden filepicker
	websWrite(wp, "<div id=\"idwgimport\" class=\"setting\">\n");
	{
		websWrite(wp, "<fieldset>\n");
		websWrite(wp, "<legend>");
		show_caption_simple(wp, "eoip.importt");
		websWrite(wp, "</legend>\n");
		show_caption(wp, "label", "eoip.filepicker", NULL);
		websWrite(
			wp,
			"<input id=\"wgimportfile\" type=\"file\" accept=\".conf\" onchange=\"import_tunnel(this.form,id,%d);\" />\n",
			tun);
		websWrite(wp, "</fieldset>\n");
	}
	websWrite(wp, "</div>\n");
	//hide file picker
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	//this.form only for *elements* to refer to the form, use 'this' or 'document.eop' or better document.forms['eop']  without an element
	websWrite(wp, "show_layer_ext(this, 'idwgimport', false);\n");
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "<br>\n");
}
#endif
