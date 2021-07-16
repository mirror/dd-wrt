/*OB
 * eoptunnel.c
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
#ifdef HAVE_EOP_TUNNEL

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>

void show_cidr(webs_t wp, char *prefix, char *var, int nm, char *type, char *nmtype, char *nmname);
void show_ip(webs_t wp, char *prefix, char *var, int nm, char *type);
void show_caption(webs_t wp, const char *class, const char *cap, const char *ext);
void show_caption_simple(webs_t wp, const char *cap);

EJ_VISIBLE void ej_show_eop_tunnels(webs_t wp, int argc, char_t ** argv)
{

	int tun;
	char temp[128];
	char temp2[128];
	int tunnels = nvram_default_geti("oet_tunnels", 0) + 1;

	for (tun = 1; tun < tunnels; tun++) {
		char oet[32];
		sprintf(oet, "oet%d", tun);
		websWrite(wp, "<fieldset>\n");
		websWrite(wp, "<legend>");
		show_caption_simple(wp, "eoip.tunnel");
		websWrite(wp, " %s</legend>\n", getNetworkLabel(wp, oet));
		websWrite(wp, "<div class=\"setting\">\n");
		{
			show_caption(wp, "label", "eoip.srv", NULL);
			snprintf(temp, sizeof(temp), "oet%d_en", tun);
			websWrite(wp,
				  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d', true)\" />", temp, (nvram_matchi(temp, 1) ? "checked=\"checked\"" : ""),
				  tun);
			show_caption(wp, NULL, "share.enable", "&nbsp;");
			websWrite(wp,
				  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d', false)\" />", temp, (nvram_matchi(temp, 0) ? "checked=\"checked\"" : ""),
				  tun);
			show_caption_simple(wp, "share.disable");
		}
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div id=\"idoet%d\">\n", tun);
		{
			snprintf(temp, sizeof(temp), "oet%d_proto", tun);
			websWrite(wp, "<div class=\"setting\">\n");
			{
				show_caption(wp, "label", "eoip.proto", NULL);
				websWrite(wp, "<select name=\"oet%d_proto\" onclick=\"changeproto(this, %d, this.value, %s)\">\n", tun, tun, nvram_nget("oet%d_bridged", tun));

				websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
				websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\"  + eoip.etherip + \"</option>\");\n", nvram_matchi(temp, 0) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\"  + eoip.mtik + \"</option>\");\n", nvram_matchi(temp, 1) ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_WIREGUARD
				websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\"  + eoip.wireguard + \"</option>\");\n", nvram_matchi(temp, 2) ? "selected=\\\"selected\\\"" : "");
#endif
				websWrite(wp, "//]]>\n</script>\n");
				websWrite(wp, "</select>\n");
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idmtik%d\">\n", tun);
			{
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.tunnelID", NULL);
					snprintf(temp, sizeof(temp), "oet%d_id", tun);
					websWrite(wp, "<input size=\"4\" maxlength=\"3\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,65535,eoip.tunnelID)\" value=\"%s\" />\n", temp, nvram_safe_get(temp));
				}
				websWrite(wp, "</div>\n");
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idwireguard%d\">\n", tun);
			{
#ifdef HAVE_WIREGUARD
				snprintf(temp, sizeof(temp), "oet%d_mit", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "service.vpn_mit", NULL);
					websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 1, 1) ? "checked=\"checked\"" : ""));
					show_caption(wp, NULL, "share.enable", "&nbsp;");
					websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 0, 1) ? "checked=\"checked\"" : ""));
					show_caption_simple(wp, "share.disable");
				}
				websWrite(wp, "</div>\n");

				//egc: add nat out via tunnel, controlled by nvram oet${i}_natout 
				snprintf(temp, sizeof(temp), "oet%d_natout", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_oet_natout", NULL);
					websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 1, 1) ? "checked=\"checked\"" : ""));
					show_caption(wp, NULL, "share.enable", "&nbsp;");
					websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 0, 1) ? "checked=\"checked\"" : ""));
					show_caption_simple(wp, "share.disable");
				}
				websWrite(wp, "</div>\n");

				snprintf(temp, sizeof(temp), "oet%d_port", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_localport", NULL);
					websWrite(wp, "<input size=\"5\" maxlength=\"5\" name=\"%s\" class=\"num\" value=\"%d\" />\n", temp, nvram_default_geti(temp, 51820));
				}
				websWrite(wp, "</div>\n");
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "idx.mtu", NULL);
					snprintf(temp, sizeof(temp), "oet%d_mtu", tun);
					int mtu = 1500;
					if (!nvram_match("wan_proto", "disabled"))
						mtu = nvram_geti("wan_mtu");
					mtu -= nvram_matchi("ipv6_enable", 1) ? 80 : 60;
					websWrite(wp, "<input size=\"5\" maxlength=\"5\" name=\"%s\" class=\"num\" value=\"%d\" />\n", temp, nvram_default_geti(temp, mtu));
				}
				websWrite(wp, "</div>\n");

				websWrite(wp, "<div class=\"center\">\n");
				{
					websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
					websWrite(wp,
						  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"gen_key_button\\\" value=\\\"\" + eoip.genkey + \"\\\" onclick=\\\"gen_wg_key(this.form,%d)\\\" />\");\n",
						  tun);
					websWrite(wp, "//]]>\n</script>\n");
				}
				websWrite(wp, "</div>\n");

				//public key show
				snprintf(temp, sizeof(temp), "oet%d_public", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_localkey", NULL);
					websWrite(wp, "<input size=\"48\" maxlength=\"48\" name=\"%s\" value=\"%s\" readonly=\"readonly\"/>\n", temp, nvram_safe_get(temp));
				}
				websWrite(wp, "</div>\n");

				//egc: DNS 
				snprintf(temp, sizeof(temp), "oet%d_dns", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_oet_dns", NULL);
					websWrite(wp, "<input size=\"32\" maxlength=\"48\" name=\"%s\" value=\"%s\" />\n", temp, nvram_default_get(temp, ""));
				}
				websWrite(wp, "</div>\n");
				//end DNS

				//Inbound firewall controlled by nvram parameter: oet${i}_firewallin default 0 = disabled, Checkbox
				snprintf(temp, sizeof(temp), "oet%d_firewallin", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_firewallin", NULL);
					websWrite(wp, "<input type=\"hidden\" name=\"%s\" id=\"%s\" value=\"0\" />\n", temp, temp);
					websWrite(wp, "<input class=\"spaceradio\" type=\"checkbox\" name=\"%s\" value=\"1\" %s />\n", temp, (nvram_default_matchi(temp, 1, 0) ? "checked=\"checked\"" : ""));
				}
				websWrite(wp, "</div>\n");
				//end firewall

				//egc Kill switch nvram param: oet%d_killswitch, Checkbox
				snprintf(temp, sizeof(temp), "oet%d_killswitch", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_killswitch", NULL);
					websWrite(wp, "<input type=\"hidden\" name=\"%s\" id=\"%s\" value=\"0\" />\n", temp, temp);
					websWrite(wp, "<input class=\"spaceradio\" type=\"checkbox\" name=\"%s\" value=\"1\" %s />\n", temp, (nvram_default_matchi(temp, 1, 0) ? "checked=\"checked\"" : ""));
				}
				websWrite(wp, "</div>\n");
				//end kill switch

				//show or hide advanced options
				snprintf(temp, sizeof(temp), "oet%d_showadvanced", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_showadvanced", NULL);
					websWrite(wp,
						  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d_showadvanced', true)\" />", temp,
						  (nvram_default_matchi(temp, 1, 0) ? "checked=\"checked\"" : ""), tun);
					show_caption(wp, NULL, "share.enable", "&nbsp;");
					websWrite(wp,
						  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d_showadvanced', false)\" />", temp,
						  (nvram_default_matchi(temp, 0, 0) ? "checked=\"checked\"" : ""), tun);
					show_caption_simple(wp, "share.disable");
				}
				websWrite(wp, "</div>\n");
				//end show

				websWrite(wp, "<div id=\"idoet%d_showadvanced\">\n", tun);	//for show or hide advanced options
				//websWrite(wp, "<div id=\"idoet%d_showadvanced\" style=\"display:none\">\n", tun);  //for show or hide advanced options

				//egc set private key
				snprintf(temp, sizeof(temp), "oet%d_private", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_localprivatekey", NULL);
					websWrite(wp, "<input type=\"password\" size=\"48\" maxlength=\"48\" name=\"%s\" onmouseover=\"this.type=\'text\'\" onmouseout=\"this.type=\'password\'\"  value=\"%s\"/>\n", temp,
						  nvram_safe_get(temp));
				}
				websWrite(wp, "</div>\n");

				//route up script
				snprintf(temp, sizeof(temp), "oet%d_rtupscript", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_rtupscript", NULL);
					websWrite(wp, "<input size=\"48\" maxlength=\"64\" name=\"%s\" value=\"%s\" />\n", temp, nvram_safe_get(temp));
				}
				websWrite(wp, "</div>\n");

				//route down script
				snprintf(temp, sizeof(temp), "oet%d_rtdownscript", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_rtdownscript", NULL);
					websWrite(wp, "<input size=\"48\" maxlength=\"64\" name=\"%s\" value=\"%s\" />\n", temp, nvram_safe_get(temp));
				}
				websWrite(wp, "</div>\n");

				//fwmark
				snprintf(temp, sizeof(temp), "oet%d_fwmark", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_fwmark", NULL);
					websWrite(wp, "<input size=\"5\" maxlength=\"5\" name=\"%s\" value=\"%s\" />\n", temp, nvram_safe_get(temp));
				}
				websWrite(wp, "</div>\n");

				//egc: PBR input box 
				snprintf(temp, sizeof(temp), "oet%d_pbr", tun);
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.wireguard_oet_pbr", NULL);
					websWrite(wp, "<input size=\"78\" maxlength=\"1024\" name=\"%s\" value=\"%s\" />\n", temp, nvram_default_get(temp, ""));
				}
				websWrite(wp, "</div>\n");
				//end PBR
				websWrite(wp, "</div>\n");	//end show hide

				snprintf(temp, sizeof(temp), "oet%d_peers", tun);
				int peers = nvram_default_geti(temp, 0);
				int peer;
				for (peer = 0; peer < peers; peer++) {

					websWrite(wp, "<br /><fieldset>\n");	//added <br>

					//name legend
					snprintf(temp, sizeof(temp), "oet%d_namep%d", tun, peer);
					{
						websWrite(wp, "<legend>%d. <input type=\"text\" size=\"14\" id=\"%s\" name=\"%s\" value=\"%s\" ></legend>\n", peer + 1, temp, temp, nvram_safe_get(temp));
					}
					//legend
					//websWrite(wp, "<legend>Peer %d</legend>\n", peer + 1);

					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_peerip", NULL);
						websWrite(wp, "<input type=\"hidden\" name=\"oet%d_ip%d\" value=\"0.0.0.0\" />\n", tun, peer);
						snprintf(temp, sizeof(temp), "oet%d_ip%d", tun, peer);
						nvram_default_get(temp, "0.0.0.0");
						show_ip(wp, NULL, temp, 1, "eoip.wireguard_peerip");
					}
					websWrite(wp, "</div>\n");

					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_peerdns", NULL);
						websWrite(wp, "<input type=\"hidden\" name=\"oet%d_dns%d\" value=\"0.0.0.0\" />\n", tun, peer);
						snprintf(temp, sizeof(temp), "oet%d_dns%d", tun, peer);
						nvram_default_get(temp, "0.0.0.0");
						show_ip(wp, NULL, temp, 1, "eoip.wireguard_peerdns");
					}
					websWrite(wp, "</div>\n");

					snprintf(temp, sizeof(temp), "oet%d_endpoint%d", tun, peer);
					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_endpoint", NULL);
						websWrite(wp,
							  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idendpoint%d_peer%d', true)\" />", temp,
							  (nvram_default_matchi(temp, 1, 0) ? "checked=\"checked\"" : ""), tun, peer);
						show_caption(wp, NULL, "share.enable", "&nbsp;");
						websWrite(wp,
							  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idendpoint%d_peer%d', false)\" />", temp,
							  (nvram_default_matchi(temp, 0, 0) ? "checked=\"checked\"" : ""), tun, peer);
						show_caption_simple(wp, "share.disable");
					}
					websWrite(wp, "</div>\n");
					websWrite(wp, "<div id=\"idendpoint%d_peer%d\">\n", tun, peer);
					{
						snprintf(temp2, sizeof(temp2), "oet%d_peerport%d", tun, peer);
						snprintf(temp, sizeof(temp), "oet%d_rem%d", tun, peer);
						websWrite(wp, "<div class=\"setting\">\n");
						{
							show_caption(wp, "label", "eoip.wireguard_peer", NULL);
							websWrite(wp,
								  "<input size=\"20\" maxlength=\"48\" name=\"%s\" value=\"%s\" />:<input size=\"5\" maxlength=\"5\" name=\"%s\" class=\"num\" value=\"%s\" />\n\n",
								  temp, nvram_safe_get(temp), temp2, nvram_safe_get(temp2));
						}
						websWrite(wp, "</div>\n");
					}
					websWrite(wp, "</div>\n");
					snprintf(temp, sizeof(temp), "oet%d_aip%d", tun, peer);
					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_allowedips", NULL);
						websWrite(wp, "<input size=\"48\" maxlength=\"1024\" name=\"%s\" value=\"%s\" />\n", temp, nvram_default_get(temp, "0.0.0.0/0"));
					}
					websWrite(wp, "</div>\n");

					//egc: route allowed IP's, controlled by nvram oet${i}_aip_rten${p}
					snprintf(temp, sizeof(temp), "oet%d_aip_rten%d", tun, peer);
					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_route_allowedip", NULL);
						websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 1, 1) ? "checked=\"checked\"" : ""));
						show_caption(wp, NULL, "share.enable", "&nbsp;");
						websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s />", temp, (nvram_default_matchi(temp, 0, 1) ? "checked=\"checked\"" : ""));
						show_caption_simple(wp, "share.disable");
					}
					websWrite(wp, "</div>\n");

					snprintf(temp, sizeof(temp), "oet%d_ka%d", tun, peer);
					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_ka", NULL);
						websWrite(wp, "<input size=\"5\" maxlength=\"5\" name=\"%s\" class=\"num\" onblur=\"valid_range(this,0,65535,eoip.wireguard_ka)\" value=\"%s\" />\n", temp,
							  nvram_safe_get(temp));
					}
					websWrite(wp, "</div>\n");

					//public key peer input
					snprintf(temp, sizeof(temp), "oet%d_peerkey%d", tun, peer);
					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_peerkey", NULL);
						websWrite(wp, "<input size=\"48\" maxlength=\"48\" name=\"%s\" value=\"%s\" />\n", temp, nvram_safe_get(temp));
					}
					websWrite(wp, "</div>\n");
					snprintf(temp, sizeof(temp), "oet%d_usepsk%d", tun, peer);
					nvram_default_get(temp, "0");
					websWrite(wp, "<div class=\"setting\">\n");
					{
						show_caption(wp, "label", "eoip.wireguard_usepsk", NULL);
						websWrite(wp,
							  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idpsk%d_peer%d', true)\" />", temp,
							  (nvram_matchi(temp, 1) ? "checked=\"checked\"" : ""), tun, peer);

						show_caption(wp, NULL, "share.enable", "&nbsp;");

						websWrite(wp,
							  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idpsk%d_peer%d', false)\" />", temp,
							  (nvram_matchi(temp, 0) ? "checked=\"checked\"" : ""), tun, peer);
						show_caption_simple(wp, "share.disable");
					}
					websWrite(wp, "</div>\n");
					websWrite(wp, "<div id=\"idpsk%d_peer%d\">\n", tun, peer);
					{

						websWrite(wp, "<div class=\"center\">\n");
						{
							websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
							websWrite(wp,
								  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"gen_psk_button\\\" value=\\\"\" + eoip.wireguard_genpsk + \"\\\" onclick=\\\"gen_wg_psk(this.form,%d,%d)\\\" />\");\n",
								  tun, peer);
							websWrite(wp, "//]]>\n</script>\n");
						}
						websWrite(wp, "</div>\n");

						snprintf(temp, sizeof(temp), "oet%d_psk%d", tun, peer);
						websWrite(wp, "<div class=\"setting\">\n");
						{
							show_caption(wp, "label", "eoip.wireguard_psk", NULL);
							websWrite(wp, "<input size=\"48\" maxlength=\"48\" name=\"%s\" value=\"%s\" />\n", temp, nvram_safe_get(temp));
						}
						websWrite(wp, "</div>\n");
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
							websWrite(wp,"</br><script type=\"text/javascript\">\n");
							websWrite(wp,"//<![CDATA[\n");
							websWrite(wp,"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"config_button\\\" value=\\\"\" + sbutton.download_config + \"\\\" onclick=\\\"window.location.href='/wireguard_config_oet%d_peer%d.conf';\\\" />\");\n", tun, peer);
							websWrite(wp,"//]]>\n");
							websWrite(wp,"</script>\n");
							websWrite(wp, "</div>\n");
							free(buf);
							hasqr = 1;
						}
					}
					//Status box
					websWrite(wp, "<div id=\"idoet%d_statusbox\">\n", tun);	//naming for periodic refresh
					{
						websWrite(wp, "<div class=\"setting\">\n");
						{
							snprintf(temp, sizeof(temp), "oet%d_peerkey%d", tun, peer);
							char buffer[1024] = "Press F5 to refresh";
							int len = 0;
							char command[64];
							sprintf(command, "/usr/bin/wireguard-state.sh %d %s 2>/dev/null", tun, nvram_safe_get(temp));
							FILE *in = popen(command, "r");
							if (in != NULL) {
								while ((len = fread(buffer, 1, 1023, in)) == 1023) {
									buffer[len] = 0;
								}
								if (len == 0) {
									char temp3[64];
									snprintf(temp3, sizeof(temp3), "oet%d_namep%d", tun, peer);
									sprintf(buffer, "No connection present for %s ", nvram_safe_get(temp3));
								}
							} else {
								sprintf(buffer, "Unable to retrieve WireGuardStatus oet%d", tun);
							}
							show_caption(wp, "label", "eoip.wireguard_oet_status", NULL);
							websWrite(wp, "<textarea cols=\"32\" rows=\"3\" id=\"oet%d_status%d\" name=\"oet%d_status%d\" disabled=\"disabled\"></textarea>", tun, peer, tun, peer);
							websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
							websWrite(wp, "document.getElementById(\"oet%d_status%d\").value =\"%s\";\n", tun, peer, buffer);
							websWrite(wp, "//]]>\n</script>\n");
							pclose(in);
						}
						websWrite(wp, "</div>\n");
					}
					websWrite(wp, "</div>\n");
					//end status box

					websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
					websWrite(wp, "show_layer_ext(this, 'idpsk%d_peer%d',%s);\n", tun, peer, nvram_nmatchi(1, "oet%d_usepsk%d", tun, peer) ? "true" : "false");
					websWrite(wp, "show_layer_ext(this, 'idendpoint%d_peer%d',%s);\n", tun, peer, nvram_nmatchi(1, "oet%d_endpoint%d", tun, peer) ? "true" : "false");
					websWrite(wp,
						  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"reboot_button\\\" value=\\\"\" + eoip.wireguard_delpeer + \"\\\" onclick=\\\"del_peer(this.form,%d,%d)\\\" />\");\n",
						  tun, peer);
					websWrite(wp,
						  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"gen_qr\\\" value=\\\"\" + eoip.wireguard_makeclient + \"\\\" onclick=\\\"gen_wg_client(this.form,%d,%d)\\\" />\");\n",
						  tun, peer);
					if (hasqr) {
						websWrite(wp,
							  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"del_qr\\\" value=\\\"\" + eoip.wireguard_cleanqr + \"\\\" onclick=\\\"del_wg_client(this.form,%d,%d)\\\" />\");\n",
							  tun, peer);
					}
					websWrite(wp, "//]]>\n</script>\n");
					websWrite(wp, "</fieldset>\n");
				}

				websWrite(wp, "<div class=\"center\">\n");
				{
					websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
					websWrite(wp,
						  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"add_peer_button\\\" value=\\\"\" + eoip.wireguard_addpeer + \"\\\" onclick=\\\"add_peer(this.form,%d)\\\" />\");\n",
						  tun);
					websWrite(wp, "//]]>\n</script>\n");
				}
				websWrite(wp, "</div>\n");

				websWrite(wp, "<br />\n");	//added <br>

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
					show_ip(wp, NULL, temp, 1, "eoip.localIP");
				}
				websWrite(wp, "</div>\n");
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.remoteIP", NULL);
					websWrite(wp, "<input type=\"hidden\" name=\"oet%d_rem\" value=\"0.0.0.0\"/>\n", tun);
					snprintf(temp, sizeof(temp), "oet%d_rem", tun);
					show_ip(wp, NULL, temp, 0, "eoip.remoteIP");
				}
				websWrite(wp, "</div>\n");
			}
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div id=\"idl2support%d\">\n", tun);
			{
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "eoip.bridging", NULL);
					snprintf(temp, sizeof(temp), "oet%d_bridged", tun);
					websWrite(wp,
						  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idbridged%d', false)\" />", temp,
						  (nvram_matchi(temp, 1) ? "checked=\"checked\"" : ""), tun);
					show_caption(wp, NULL, "share.enable", "&nbsp;");
					websWrite(wp,
						  " <input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idbridged%d', true)\" />", temp,
						  (nvram_matchi(temp, 0) ? "checked=\"checked\"" : ""), tun);
					show_caption_simple(wp, "share.disable");
				}
				websWrite(wp, "</div>\n");
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
			// Alternative input for ipaddress and netmask to add multiple addresses including IPv6 e.g.: 10.0.0.2/16, fc00::2/96
			// nvram variable oet%d_ipaddrmask
			websWrite(wp, "<div id=\"idwginput%d\">\n", tun);
			{
				websWrite(wp, "<div class=\"setting\">\n");
				{
					show_caption(wp, "label", "share.ipaddrmask", NULL);
					snprintf(temp, sizeof(temp), "oet%d_ipaddrmask", tun);
					websWrite(wp, "<input size=\"48\" maxlength=\"64\" name=\"%s\" value=\"%s\" />\n", temp, nvram_safe_get(temp));
				}
				websWrite(wp, "</div>\n");
				//end alternative input
			}
			websWrite(wp, "</div>\n");
		}
		websWrite(wp, "</div>\n");
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"del_button\\\" value=\\\"\" + eoip.del + \"\\\" onclick=\\\"del_tunnel(this.form,%d)\\\" />\");\n", tun);
		websWrite(wp, "changeproto(document.eop.oet%d_proto, %d, %s, %s);\n", tun, tun, nvram_nget("oet%d_proto", tun), nvram_nget("oet%d_bridged", tun));
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d', %s);\n", tun, tun, nvram_nmatchi(1, "oet%d_en", tun) ? "true" : "false");
		//hide or show advanced settings
		websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idoet%d_showadvanced',%s);\n", tun, tun, nvram_nmatchi(1, "oet%d_showadvanced", tun) ? "true" : "false");
		//hide ip address and netmask for and show alternative input for WG (proto 2)
//              websWrite(wp, "show_layer_ext(document.eop.oet%d_en, 'idwginput%d', %s);\n", tun, tun, nvram_nmatchi(2, "oet%d_proto", tun) ? "true" : "false");
		//end
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</fieldset><br/>\n");
	}
	websWrite(wp, "<div class=\"center\">\n");
	{
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"add_button\\\" value=\\\"\" + eoip.add + \"\\\" onclick=\\\"add_tunnel(this.form)\\\" />\");\n");
		websWrite(wp, "//]]>\n</script>\n");
	}
	websWrite(wp, "</div>\n");
}
#endif
