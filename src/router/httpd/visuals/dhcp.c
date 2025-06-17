/*
 * dhcp.c
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
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <ctype.h>

#include <broadcom.h>

#define DHCP_MAX_COUNT 254
#define EXPIRES_NEVER 0xFFFFFFFF /* static lease */

static int landhcp(void)
{
	if (!getWET())
		if (nvram_match("lan_proto", "dhcp") && nvram_matchi("dhcpfwd_enable", 0) && !*nvram_safe_get("lan_dhcpaddr"))
			return 1;
	return 0;
}

static int hasmdhcp(void)
{
	if (nvram_exists("mdhcpd_count")) {
		int mdhcpcount = nvram_geti("mdhcpd_count");
		return mdhcpcount > 0 ? 1 : 0;
	}
	return 0;
}

EJ_VISIBLE void ej_dhcpenabled(webs_t wp, int argc, char_t **argv)
{
	if (landhcp() || hasmdhcp())
		websWrite(wp, argv[0]);
	else
		websWrite(wp, argv[1]);
}

static char *dhcp_reltime(char *buf, size_t len, time_t t, int sub)
{
	if (sub) {
		time_t now = time(NULL);
		t -= now;
	}
	if (t < 0)
		t = 0;
	int days = t / 86400;
	int min = t / 60;

	snprintf(buf, len, "%d day%s %02d:%02d:%02d", days, ((days == 1) ? "" : "s"), ((min / 60) % 24), (min % 60), (int)(t % 60));
	return buf;
}

/*
 * dump in array: hostname,mac,ip,expires read leases from leasefile as:
 * expires mac ip hostname 
 */
EJ_VISIBLE void ej_dumpleases(webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	unsigned long expires;
	int i;
	int count = 0;
	int macmask;
	char ip[32];
	char hostname[256];
	char clid[256];
	int ifidx = -1;

	char buf[512];
	char *p;
	char *buff;
	struct lease_t lease;
	struct in_addr addr;
	char *ipaddr, mac[32] = "", expires_time[256] = "";

	if (argc < 1) {
		return;
	}
	macmask = atoi(argv[0]);
	if (landhcp() || hasmdhcp()) {
		if (nvram_invmatchi("dhcpd_usenvram", 1)) {
			/*
			 * Parse leases file 
			 */
			if (nvram_matchi("dhcpd_usejffs", 1)) {
				fp = fopen("/jffs/dnsmasq.leases", "r");
				if (!fp) {
					fp = fopen("/tmp/dnsmasq.leases", "r");
				}
				if (!fp) {
					return;
				}
			} else {
				fp = fopen("/tmp/dnsmasq.leases", "r");
				if (!fp) {
					fp = fopen("/jffs/dnsmasq.leases", "r");
				}
			}
			if (!fp) {
				return;
			}

			if (fp) {
				while (fgets(buf, sizeof(buf), fp)) {
					ifidx = -1;
					if (sscanf(buf, "%lu %17s %15s %255s %255s %d", &expires, mac, ip, hostname, clid, &ifidx) <
					    4)
						continue;
					p = mac;
					while ((*p = toupper(*p)) != 0)
						++p;
					if ((p = strrchr(ip, '.')) == NULL)
						continue;
					if (nvram_matchi("maskmac", 1) && macmask) {
						mac[0] = 'x';
						mac[1] = 'x';
						mac[3] = 'x';
						mac[4] = 'x';
						mac[6] = 'x';
						mac[7] = 'x';
						mac[9] = 'x';
						mac[10] = 'x';
					}
					char ifname[32] = { 0 };
					if (ifidx != -1) {
						getIfByIdx(ifname, ifidx);
					}
					websWrite(wp, "%c'%s','%s','%s','%s','%s','%s','%s'", (count ? ',' : ' '),
						  (hostname[0] ? hostname : live_translate(wp, "share.unknown")), ip, mac,
						  ((expires == 0) ? live_translate(wp, "share.sttic") :
								    dhcp_reltime(buf, sizeof(buf), expires, 1)),
						  p + 1, ifname, nvram_nget("%s_label", ifname));
					++count;
				}
				fclose(fp);
			}
		} else {
			for (i = 0; i < DHCP_MAX_COUNT; ++i) {
				sprintf(buf, "dnsmasq_lease_%d.%d.%d.%d", get_single_ip(nvram_safe_get("lan_ipaddr"), 0),
					get_single_ip(nvram_safe_get("lan_ipaddr"), 1),
					get_single_ip(nvram_safe_get("lan_ipaddr"), 2), i);

				buff = nvram_safe_get(buf);
				if (sscanf(buff, "%lu %17s %15s %255s", &expires, mac, ip, hostname) < 4)
					continue;
				p = mac;
				while ((*p = toupper(*p)) != 0)
					++p;
				if ((p = strrchr(ip, '.')) == NULL)
					continue;
				if (nvram_matchi("maskmac", 1) && macmask) {
					mac[0] = 'x';
					mac[1] = 'x';
					mac[3] = 'x';
					mac[4] = 'x';
					mac[6] = 'x';
					mac[7] = 'x';
					mac[9] = 'x';
					mac[10] = 'x';
				}
				websWrite(wp, "%c'%s','%s','%s','%s','%s','N/A',''", (count ? ',' : ' '),
					  (hostname[0] ? hostname : live_translate(wp, "share.unknown")), ip, mac,
					  ((expires == 0) ? live_translate(wp, "share.sttic") :
							    dhcp_reltime(buf, sizeof(buf), expires, 1)),
					  p + 1);
				++count;
			}
		}
	}
	return;
}

EJ_VISIBLE void ej_dhcp_remaining_time(webs_t wp, int argc, char_t **argv)
{
	// tofu12
	if (nvram_invmatch("wan_proto", "dhcp") && nvram_invmatch("wan_proto", "dhcp_auth"))
		return;

	long exp;
	char buf[128];
	struct sysinfo si;
	long n;

	exp = 0;
	if (file_to_buf("/tmp/udhcpc.expires", buf, sizeof(buf))) {
		n = atol(buf);
		if (n > 0) {
			sysinfo(&si);
			exp = n - si.uptime;
		}
	}
	websWrite(wp, dhcp_reltime(buf, sizeof(buf), exp, 0));

	return;
}

EJ_VISIBLE void ej_show_staticleases(webs_t wp, int argc, char_t **argv)
{
	int i;

	char *sln = nvram_safe_get("static_leasenum");

	if (sln == NULL || *(sln) == 0)
		return;

	int leasenum = atoi(sln);

	if (leasenum == 0)
		return;
	char *nvleases = nvram_safe_get("static_leases");
	char *leases = strdup(nvleases);
	char *originalpointer = leases; // strsep destroys the pointer by

	// moving it
	for (i = 0; i < leasenum; i++) {
		char *sep = strsep(&leases, "=");

		websWrite(
			wp,
			"<tr><td><input name=\"lease%d_hwaddr\" value=\"%s\" size=\"18\" maxlength=\"18\" onblur=\"valid_name(this,share.mac,SPACE_NO)\" /></td>",
			i, sep != NULL ? sep : "");
		sep = strsep(&leases, "=");
		websWrite(
			wp,
			"<td><input name=\"lease%d_hostname\" value=\"%s\" size=\"24\" maxlength=\"24\" onblur=\"valid_name(this,share.hostname,SPACE_NO)\" /></td>",
			i, sep != NULL ? sep : "");
		sep = strsep(&leases, "=");
		websWrite(
			wp,
			"<td><input name=\"lease%d_ip\" value=\"%s\" size=\"15\" maxlength=\"15\" class=\"num\" onblur=\"valid_name(this,share.ip,SPACE_NO)\" /></td>\n",
			i, sep != NULL ? sep : "");
		sep = strsep(&leases, " ");
		websWrite(
			wp,
			"<td><input name=\"lease%d_time\" value=\"%s\" size=\"10\" maxlength=\"10\" class=\"num\" onblur=\"valid_name(this,share.time,SPACE_NO)\" />&nbsp;<script type=\"text/javascript\">Capture(share.minutes)</script></td>\n",
			i, sep != NULL ? sep : "3600");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"lease_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n</td></tr>",
			i);
	}
	debug_free(originalpointer);
	return;
}

EJ_VISIBLE void ej_calcendip(webs_t wp, int argc, char_t **argv)
{
	char *ip = nvram_safe_get("dhcp_start");
	char *netmask = nvram_safe_get("lan_netmask");
	int dhcpnum = atoi(nvram_safe_get("dhcp_num"));
	unsigned int ip1 = get_single_ip(ip, 0);
	unsigned int ip2 = get_single_ip(ip, 1);
	unsigned int ip3 = get_single_ip(ip, 2);
	unsigned int ip4 = get_single_ip(ip, 3);
	//      unsigned int im1 = get_single_ip(netmask, 0);
	//      unsigned int im2 = get_single_ip(netmask, 1);
	//      unsigned int im3 = get_single_ip(netmask, 2);
	//      unsigned int im4 = get_single_ip(netmask, 3);

	unsigned int im1 = 255;
	unsigned int im2 = 255;
	unsigned int im3 = 255;
	unsigned int im4 = 255;
	unsigned int sip = ((ip1 & im1) << 24) + ((ip2 & im2) << 16) + ((ip3 & im3) << 8) + ip4;
	unsigned int eip = sip + dhcpnum - 1;

	websWrite(wp, "%d.%d.%d.%d", (eip >> 24) & 0xff, (eip >> 16) & 0xff, (eip >> 8) & 0xff, eip & 0xff);
}

EJ_VISIBLE void ej_show_dhcpd_settings(webs_t wp, int argc, char_t **argv)
{
	int i;

	if (getWET()) // dhcpd settings disabled in client bridge mode, so we wont display it
		return;

	websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(idx.dhcp_legend)</script></legend>\n");
	websWrite(wp, "<div class=\"setting\" name=\"dhcp_settings\">\n");
	show_caption(wp, "label", "idx.dhcp_type", NULL);
	websWrite(
		wp,
		"<select class=\"num\" size=\"1\" name=\"dhcpfwd_enable\" onchange=SelDHCPFWD(this.form.dhcpfwd_enable.selectedIndex,this.form)>\n");
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + idx.dhcp_srv + \"</option>\");\n",
		  nvram_matchi("dhcpfwd_enable", 0) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + idx.dhcp_fwd + \"</option>\");\n",
		  nvram_matchi("dhcpfwd_enable", 1) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
	if (nvram_matchi("dhcpfwd_enable", 1)) {
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "idx.dhcp_srv", NULL);
		websWrite(wp, "<input type=\"hidden\" name=\"dhcpfwd_ip\" value=\"4\" />\n");
		show_ip(wp, NULL, "dhcpfwd_ip", 0, 0, "idx.dhcp_srv");
		websWrite(wp, "</div>\n");
	} else {
		websWrite(wp, "<div class=\"setting\">\n");
		// char *nv = nvram_safe_get ("wan_wins");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_srv)</script></div><input class=\"spaceradio\" type=\"radio\" name=\"lan_proto\" value=\"dhcp\" onclick=\"SelDHCP('dhcp',this.form)\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			nvram_match("lan_proto", "dhcp") ? "checked=\"checked\"" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" name=\"lan_proto\" value=\"static\" onclick=\"SelDHCP('static',this.form)\" %s /><script type=\"text/javascript\">Capture(share.disable)</script></div><input type=\"hidden\" name=\"dhcp_check\" /><div class=\"setting\">\n",
			nvram_match("lan_proto", "static") ? "checked=\"checked\"" : "");
		show_caption(wp, "label", "idx.dhcp_start", NULL);
		char *dhcp_start = nvram_safe_get("dhcp_start");
		websWrite(
			wp,
			"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,%d,%d,%s)\" name=\"%s_0\" value=\"%d\" disabled=\"true\" />.",
			1, 254, "idx.dhcp_start", "dhcp_start", get_single_ip(nvram_safe_get("lan_ipaddr"), 0));
		websWrite(
			wp,
			"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,%s)\" name=\"%s_1\" value=\"%d\" />.",
			"idx.dhcp_start", "dhcp_start", get_single_ip(dhcp_start, 1));
		websWrite(
			wp,
			"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,%s)\" name=\"%s_2\" value=\"%d\" />.",
			"idx.dhcp_start", "dhcp_start", get_single_ip(dhcp_start, 2));
		websWrite(
			wp,
			"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,%s)\" name=\"%s_3\" value=\"%d\" />\n",
			"idx.dhcp_start", "dhcp_start", get_single_ip(dhcp_start, 3));
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_maxusers)</script></div><input class=\"num\" name=\"dhcp_num\" size=\"5\" value=\"%s\" /></div>\n",
			nvram_safe_get("dhcp_num"));
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_lease)</script></div><input class=\"num\" name=\"dhcp_lease\" size=\"5\" maxlength=\"5\" onblur=\"valid_range(this,0,99999,idx.dhcp_lease)\" value=\"%s\" > <script type=\"text/javascript\">Capture(share.minutes)</script></input></div>\n",
			nvram_safe_get("dhcp_lease"));
		if (nvram_invmatch("wan_proto", "static")) {
			websWrite(wp, "<div class=\"setting\" id=\"dhcp_static_dns0\">\n");
			websWrite(wp,
				  "<div class=\"label\"><script type=\"text/javascript\">Capture(idx_static.dns)</script> 1</div>");
			websWrite(wp, "<input type=\"hidden\" name=\"wan_dns\" value=\"4\" />");
			for (i = 0; i < 4; i++)
				websWrite(
					wp,
					"<input class=\"num\" name=\"wan_dns0_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,idx_static.dns)\" value=\"%d\" />%s",
					i, i == 3 ? 254 : 255, get_dns_ip("wan_dns", 0, i), i < 3 ? "." : "");

			websWrite(wp, "\n</div>\n<div class=\"setting\" id=\"dhcp_static_dns1\">\n");
			websWrite(wp,
				  "<div class=\"label\"><script type=\"text/javascript\">Capture(idx_static.dns)</script> 2</div>");
			for (i = 0; i < 4; i++)
				websWrite(
					wp,
					"<input class=\"num\" name=\"wan_dns1_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,idx_static.dns)\" value=\"%d\" />%s",
					i, i == 3 ? 254 : 255, get_dns_ip("wan_dns", 1, i), i < 3 ? "." : "");

			websWrite(wp, "\n</div>\n<div class=\"setting\" id=\"dhcp_static_dns2\">\n");
			websWrite(wp,
				  "<div class=\"label\"><script type=\"text/javascript\">Capture(idx_static.dns)</script> 3</div>");
			for (i = 0; i < 4; i++)
				websWrite(
					wp,
					"<input class=\"num\" name=\"wan_dns2_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,idx_static.dns)\" value=\"%d\" />%s",
					i, i == 3 ? 254 : 255, get_dns_ip("wan_dns", 2, i), i < 3 ? "." : "");
			websWrite(wp, "\n</div>");
		}
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">WINS</div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"wan_wins\" value=\"4\" />\n");

		show_ip(wp, NULL, "wan_wins", 1, 0, "&#34;WINS&#34;");
		websWrite(wp, "</div>\n<div class=\"setting\">\n");
		show_caption(wp, "label", "idx.dns_dnsmasq", NULL);
		websWrite(wp, "<input type=\"checkbox\" name=\"_dns_dnsmasq\" value=\"1\" %s />\n",
			  nvram_matchi("dns_dnsmasq", 1) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n<div class=\"setting\">\n");
		show_caption(wp, "label", "idx.auth_dnsmasq", NULL);
		websWrite(wp, "<input type=\"checkbox\" name=\"_auth_dnsmasq\" value=\"1\" %s />\n",
			  nvram_matchi("auth_dnsmasq", 1) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");

#ifdef HAVE_UNBOUND
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(idx.recursive_dns)</script></div>\n");
		websWrite(wp, "<input type=\"checkbox\" name=\"_recursive_dns\" value=\"1\" %s />\n",
			  nvram_matchi("recursive_dns", 1) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
#endif
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "idx.force_dnsmasq", NULL);
		websWrite(wp, "<input type=\"checkbox\" name=\"_dns_redirect\" value=\"1\" %s />\n",
			  nvram_matchi("dns_redirect", 1) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "idx.force_dnsmasqdot", NULL);
		websWrite(wp, "<input type=\"checkbox\" name=\"_dns_redirectdot\" value=\"1\" %s />\n",
			  nvram_matchi("dns_redirectdot", 1) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
	}

	websWrite(wp, "</fieldset><br />\n");
	return;
}
