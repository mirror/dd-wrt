#define VISUALSOURCE 1
/*
 * dd-wrt.c
 *
 * Copyright (C) 2005 - 2007 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>
#include <cymac.h>
#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>
//#include <l7protocols.h>

#ifdef HAVE_OVERCLOCKING
static unsigned int type2_clocks[7] = { 200, 240, 252, 264, 300, 330, 0 };
static unsigned int type3_clocks[3] = { 150, 200, 0 };
static unsigned int type4_clocks[10] = { 192, 200, 216, 228, 240, 252, 264, 280, 300, 0 };
static unsigned int type7_clocks[10] = { 183, 187, 198, 200, 216, 225, 233, 237, 250, 0 };
static unsigned int type8_clocks[9] = { 200, 300, 400, 500, 600, 632, 650, 662, 0 };

static unsigned int type9_clocks[7] =	// 1200 seem to be the last value which works stable
{ 600, 800, 1000, 1200, 1400, 1600, 0 };

static unsigned int type10_clocks[7] = { 300, 333, 400, 480, 500, 533, 0 };

#endif

#ifdef HAVE_RT2880
#define IFMAP(a) getRADev(a)
#else
#define IFMAP(a) (a)
#endif

void show_ipnetmask(webs_t wp, char *var)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n");

	char *ipv = nvram_nget("%s_ipaddr", var);

	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,1,223,share.ip)\" name=\"%s_ipaddr_0\" value=\"%d\" />.", var, get_single_ip(ipv, 0));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_1\" value=\"%d\" />.", var, get_single_ip(ipv, 1));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_2\" value=\"%d\" />.", var, get_single_ip(ipv, 2));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_3\" value=\"%d\" />\n", var, get_single_ip(ipv, 3));
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n");
	ipv = nvram_nget("%s_netmask", var);

	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_0\" value=\"%d\" />.", var, get_single_ip(ipv, 0));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_1\" value=\"%d\" />.", var, get_single_ip(ipv, 1));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_2\" value=\"%d\" />.", var, get_single_ip(ipv, 2));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_3\" value=\"%d\" />", var, get_single_ip(ipv, 3));
	websWrite(wp, "</div>\n");

}

#ifdef HAVE_OVERCLOCKING
void ej_show_clocks(webs_t wp, int argc, char_t ** argv)
{
	int rev = cpu_plltype();
	unsigned int *c;

	if (rev == 2)
		c = type2_clocks;
	else if (rev == 3)
		c = type3_clocks;
	else if (rev == 4)
		c = type4_clocks;
	else if (rev == 7)
		c = type7_clocks;
	else if (rev == 8)
		c = type8_clocks;
	else if (rev == 9)
		c = type9_clocks;
	else if (rev == 10)
		c = type10_clocks;
	else {
		websWrite(wp, "<script type=\"text/javascript\">Capture(management.clock_support)</script>\n</div>\n");
		return;
	}

	char *oclk = nvram_safe_get("overclocking");

	int cclk = atoi(oclk);

	int i = 0;
	int in_clock_array = 0;
	
	//check if cpu clock list contains current clkfreq
	while (c[i] != 0) {
		if(c[i]== cclk) {
			in_clock_array = 1;
		}
		i++;
	}
	
	if(in_clock_array || nvram_get("clkfreq") != NULL) {
	
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(management.clock_frq)</script></div>\n");
		websWrite(wp, "<select name=\"overclocking\">\n");
		i = 0;
		while (c[i] != 0) {
			websWrite(wp, "<option value=\"%d\" %s >%d MHz</option>\n", c[i], c[i] == cclk ? "selected=\"selected\"" : "", c[i]);
			i++;
		}
		
		websWrite(wp, "</select>\n</div>\n");
	} else {
		websWrite(wp, "<script type=\"text/javascript\">Capture(management.clock_support)</script>\n</div>\n");
		fprintf(stderr, "CPU frequency list for rev: %d does not contain current clkfreq: %d.", rev, cclk);
	}
}
#endif

void ej_show_routing(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"gateway\\\" %s >\" + share.gateway + \"</option>\");\n", nvram_selmatch(wp, "wk_mode", "gateway") ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_BIRD
	websWrite(wp, "document.write(\"<option value=\\\"bgp\\\" %s >BGP</option>\");\n", nvram_selmatch(wp, "wk_mode", "bgp") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"router\\\" %s >\" + route.rip2_mod + \"</option>\");\n", nvram_selmatch(wp, "wk_mode", "router") ? "selected=\\\"selected\\\"" : "");
#endif
#ifdef HAVE_QUAGGA
	websWrite(wp, "document.write(\"<option value=\\\"bgp\\\" %s >BGP</option>\");\n", nvram_selmatch(wp, "wk_mode", "bgp") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"router\\\" %s >\" + route.rip2_mod + \"</option>\");\n", nvram_selmatch(wp, "wk_mode", "router") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"ospf\\\" %s >\" + route.ospf_mod + \"</option>\");\n", nvram_selmatch(wp, "wk_mode", "ospf") ? "selected=\\\"selected\\\"" : "");
#endif
#ifdef HAVE_QUAGGA
	websWrite(wp, "document.write(\"<option value=\\\"ospf bgp rip router\\\" %s >vtysh OSPF BGP RIP router</option>\");\n", nvram_selmatch(wp, "wk_mode", "ospf bgp rip router") ? "selected=\\\"selected\\\"" : "");
#endif
#ifdef HAVE_OLSRD
	websWrite(wp, "document.write(\"<option value=\\\"olsr\\\" %s >\" + route.olsrd_mod + \"</option>\");\n", nvram_selmatch(wp, "wk_mode", "olsr") ? "selected=\\\"selected\\\"" : "");
#endif
	websWrite(wp, "document.write(\"<option value=\\\"static\\\" %s >\" + share.router + \"</option>\");\n", nvram_selmatch(wp, "wk_mode", "static") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n");
	return;

}

#ifdef HAVE_BUFFALO
extern void *getUEnv(char *name);
#endif

void ej_show_connectiontype(webs_t wp, int argc, char_t ** argv)
{

	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"disabled\\\" %s >\" + share.disabled + \"</option>\");\n", nvram_selmatch(wp, "wan_proto", "disabled") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"static\\\" %s >\" + idx.static_ip + \"</option>\");\n", nvram_selmatch(wp, "wan_proto", "static") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"dhcp\\\" %s >\" + idx.dhcp + \"</option>\");\n", nvram_selmatch(wp, "wan_proto", "dhcp") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "\n//]]>\n</script>\n");

#ifdef HAVE_MODEMBRIDGE
	websWrite(wp, "<option value=\"bridge\" %s >DSL Modem Bridge</option>\n", nvram_selmatch(wp, "wan_proto", "bridge") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_PPPOE
	websWrite(wp, "<option value=\"pppoe\" %s >PPPoE</option>\n", nvram_selmatch(wp, "wan_proto", "pppoe") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_PPPOATM
	websWrite(wp, "<option value=\"pppoa\" %s >PPPoA</option>\n", nvram_selmatch(wp, "wan_proto", "pppoa") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_PPTP
	websWrite(wp, "<option value=\"pptp\" %s >PPTP</option>\n", nvram_selmatch(wp, "wan_proto", "pptp") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_L2TP
	websWrite(wp, "<option value=\"l2tp\" %s >L2TP</option>\n", nvram_selmatch(wp, "wan_proto", "l2tp") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_HEARTBEAT
	websWrite(wp, "<option value=\"heartbeat\" %s >HeartBeat Signal</option>\n", nvram_selmatch(wp, "wan_proto", "heartbeat") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_IPETH
	websWrite(wp, "<option value=\"iphone\" %s >IPhone Tethering</option>\n", nvram_selmatch(wp, "wan_proto", "iphone") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_3G
#ifdef HAVE_BUFFALO
	char *region = getUEnv("region");
	if (!region) {
		region = "US";
	}
	if (!strcmp(region, "EU") || !strcmp(region, "DE")
	    || nvram_match("umts_override", "1")) {
#endif
		websWrite(wp, "<option value=\"3g\" %s >Mobile Broadband</option>\n", nvram_selmatch(wp, "wan_proto", "3g") ? "selected=\"selected\"" : "");
#ifdef HAVE_BUFFALO
	}
#endif
#endif

	return;
}

void ej_show_infopage(webs_t wp, int argc, char_t ** argv)
{
	/*
	 * #ifdef HAVE_NEWMEDIA websWrite(wp,"<dl>\n"); websWrite(wp,"<dd
	 * class=\"definition\">GGEW net GmbH</dd>\n"); websWrite(wp,"<dd
	 * class=\"definition\">Dammstrasse 68</dd>\n"); websWrite(wp,"<dd
	 * class=\"definition\">64625 Bensheim</dd>\n"); websWrite(wp,"<dd
	 * class=\"definition\"><a href=\"http://ggew-net.de\"><img
	 * src=\"images/ggewlogo.gif\" border=\"0\"/></a></dd>\n");
	 * websWrite(wp,"<dd class=\"definition\"> </dd>\n"); websWrite(wp,"<dd
	 * class=\"definition\"><a href=\"http://ggew-net.de\"/></dd>\n");
	 * websWrite(wp,"<dd class=\"definition\"> </dd>\n"); websWrite(wp,"<dd
	 * class=\"definition\">In Kooperation mit NewMedia-NET GmbH</dd>\n");
	 * websWrite(wp,"<dd class=\"definition\"><a
	 * href=\"http://www.newmedia-net.de\"/></dd>\n");
	 * websWrite(wp,"</dl>\n"); #endif
	 */
	return;
}

void ej_dumpmeminfo(webs_t wp, int argc, char_t ** argv)
{
	FILE *fcpu = fopen("/proc/meminfo", "r");

	if (fcpu == NULL) {
		return;
	}
	char buf[128];
	int n = 0;

      rept:;
	if (n == EOF) {
		fclose(fcpu);
		return;
	}
	if (n)
		websWrite(wp, "'%s'", buf);
	n = fscanf(fcpu, "%s", buf);
	if (n != EOF)
		websWrite(wp, ",");
	goto rept;
}

#ifdef HAVE_STORM
#define HARDFREQ "300"
#elif HAVE_OPENRISC
#define HARDFREQ "166"
#elif HAVE_LAGUNA
#define FREQLINE 2
#elif HAVE_RT3052
#ifdef HAVE_HOTPLUG2
#define FREQLINE 7
#else
#define FREQLINE 4
#endif
#elif defined(HAVE_DANUBE)
#define FREQLINE 6
#elif HAVE_RT2880
#ifdef HAVE_HOTPLUG2
#define FREQLINE 7
#else
#define FREQLINE 4
#endif
#elif HAVE_XSCALE
#define FREQLINE 2
#elif defined(HAVE_MAGICBOX) || defined(HAVE_RB600)
#define FREQLINE 3
#elif defined(HAVE_FONERA) || defined(HAVE_SOLO51) || defined(HAVE_ADM5120) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_LS5) || defined(HAVE_WHRAG108) || defined(HAVE_TW6600) || defined(HAVE_CA8) || defined(HAVE_RB500)
#ifdef HAVE_HOTPLUG2
#define FREQLINE 5
#else
#define FREQLINE 4
#endif
#elif defined(HAVE_PB42) || defined(HAVE_LSX)
#define FREQLINE 5
#elif HAVE_VENTANA
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq", "rb");
	if (fp) {
		int freq;
		fscanf(fp, "%d", &freq);
		fclose(fp);
		websWrite(wp, "%d", freq / 1000);
	} else {
		websWrite(wp, "unknown");
		return;
	}
}
#elif HAVE_X86
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp = fopen("/proc/cpuinfo", "rb");

	if (fp == NULL) {
		websWrite(wp, "unknown");
		return;
	}
	int cnt = 0;
	int b = 0;

	while (b != EOF) {
		b = getc(fp);
		if (b == ':')
			cnt++;
		if (cnt == 8) {
			getc(fp);
			char cpuclk[32];
			int i = 0;

			b = getc(fp);
			while (b != 0xa && b != 0xd && b != 0x20) {
				cpuclk[i++] = b;
				b = getc(fp);
			}
			cpuclk[i++] = 0;
			websWrite(wp, cpuclk);
			fclose(fp);
			return;
		}
	}

	fclose(fp);
	websWrite(wp, "unknown");
	return;
}
#else
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	char *clk = nvram_get("clkfreq");

	if (clk == NULL) {
		if (getcpurev() == 0)	//BCM4710
			websWrite(wp, "125");
		else if (getcpurev() == 29)	//BCM5354
			websWrite(wp, "240");
		else
			websWrite(wp, "unknown");
		return;
	}
	char buf[64];

	strcpy(buf, clk);
	int i = 0;

	while (buf[i++] != 0) {
		if (buf[i] == ',')
			buf[i] = 0;
	}
	websWrite(wp, buf);
	return;
}
#endif

#if defined(FREQLINE)
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp = fopen("/proc/cpuinfo", "rb");

	if (fp == NULL) {
		websWrite(wp, "unknown");
		return;
	}
	int cnt = 0;
	int b = 0;

	while (b != EOF) {
		b = getc(fp);
		if (b == ':')
			cnt++;

		if (cnt == FREQLINE) {
			getc(fp);
			char cpuclk[7];
			int i;
			for (i = 0; i < 6; i++) {
				int c = getc(fp);
				if (c == EOF || c == '\n' || c == '.' || c == 0)
					break;
				cpuclk[i] = c;
			}
			cpuclk[i] = 0;
			websWrite(wp, cpuclk);
			fclose(fp);
			return;
		}
	}

	fclose(fp);
	websWrite(wp, "unknown");
	return;
}

#undef FREQLINE
#elif defined(HARDFREQ)
void ej_get_clkfreq(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, HARDFREQ);
	return;
}

#undef HARDFREQ
#endif

void ej_show_cpuinfo(webs_t wp, int argc, char_t ** argv)
{

#ifdef HAVE_IPR
	char *str = "rev 1.2";
#else
	char *str = cpustring();
#endif
	if (!str) {
		websWrite(wp, "Not Detected!\n");
		return;
	}
	websWrite(wp, str);
}

void ej_show_cpucores(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp = popen("cat /proc/cpuinfo|grep processor|wc -l", "rb");
	int count, dcount;
	fscanf(fp, "%d", &count);
	fclose(fp);
	fp = popen("cat /proc/cpuinfo|grep \"compatible processor\"|wc -l", "rb");
	fscanf(fp, "%d", &dcount);
	fclose(fp);
	count -= dcount;

	if (!count)
		count = 1;

	websWrite(wp, "%d", count);
}

#define ASSOCLIST_TMP	"/tmp/.wl_assoclist"
#define RSSI_TMP	"/tmp/.rssi"
#define ASSOCLIST_CMD	"wl assoclist"
#define RSSI_CMD	"wl rssi"
#define NOISE_CMD	"wl noise"

void ej_show_wds_subnet(webs_t wp, int argc, char_t ** argv)
{
	int index = -1;
	char *interface;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d %s", &index, &interface);
#else
	if (ejArgs(argc, argv, "%d %s", &index, &interface) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	char br1[32];

	sprintf(br1, "%s_br1_enable", interface);
	if (nvram_invmatch(br1, "1"))
		return;
	char buf[16];

	sprintf(buf, "%s_wds%d_enable", interface, index);
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"2\\\" %s >\" + wds.subnet + \"</option>\");\n//]]>\n</script>\n",
		  nvram_selmatch(wp, buf, "2") ? "selected=\\\"selected\\\"" : "");
	return;
}

#ifdef HAVE_SKYTRON
void ej_active_wireless2(webs_t wp, int argc, char_t ** argv)
{
	int rssi = 0, noise = 0;
	FILE *fp, *fp2;
	char *mode;
	char mac[30];
	char list[2][30];
	char line[80];

	unlink(ASSOCLIST_TMP);
	unlink(RSSI_TMP);

	mode = nvram_safe_get("wl_mode");
	sysprintf("%s > %s", ASSOCLIST_CMD, ASSOCLIST_TMP);

	int connected = 0;

	if ((fp = fopen(ASSOCLIST_TMP, "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (sscanf(line, "%s %s", list[0], mac) != 2)	// assoclist
				// 00:11:22:33:44:55
				continue;

			if (strcmp(list[0], "assoclist"))
				break;

			rssi = 0;
			noise = 0;
			// get rssi value
			if (strcmp(mode, "ap"))
				sysprintf("%s > %s", RSSI_CMD, RSSI_TMP);
			else
				sysprintf("%s \"%s\" > %s", RSSI_CMD, mac, RSSI_TMP);

			// get noise value if not ap mode
			if (strcmp(mode, "ap"))
				sysprintf("%s >> %s", NOISE_CMD, RSSI_TMP);

			fp2 = fopen(RSSI_TMP, "r");
			if (fgets(line, sizeof(line), fp2) != NULL) {

				// get rssi
				if (sscanf(line, "%s %s %d", list[0], list[1], &rssi) != 3)
					continue;

				// get noise for client/wet mode
				if (strcmp(mode, "ap") && fgets(line, sizeof(line), fp2) != NULL && sscanf(line, "%s %s %d", list[0], list[1], &noise) != 3)
					continue;

				fclose(fp2);
			}
			if (nvram_match("maskmac", "1")) {
				mac[0] = 'x';
				mac[1] = 'x';
				mac[3] = 'x';
				mac[4] = 'x';
				mac[6] = 'x';
				mac[7] = 'x';
				mac[9] = 'x';
				mac[10] = 'x';
			}
			if (strcmp(mode, "ap") != 0) {
				connected = 1;
				websWrite(wp, "<tr>\n");
				websWrite(wp,
					  "<td bgcolor=\"#B2B2B2\" valign=\"middle\" align=\"right\" width=\"200\" height=\"25\"><font face=\"Arial\" color=\"#000000\" size=\"2\"><b>Verbindungsstatus</b></font></td>\n");
				websWrite(wp, "<td bgcolor=\"#B2B2B2\"></td>\n");
				websWrite(wp, "<td bgcolor=\"#FFFFFF\"></td>\n");
				websWrite(wp, "<td colspan=\"2\" bgcolor=\"#FFFFFF\" valign=\"middle\" align=\"left\"><font face=\"Arial\" color=\"#000000\" size=\"2\">Verbunden</font></td>\n");
				websWrite(wp, "</tr>\n");
				websWrite(wp, "<tr>\n");
				websWrite(wp, "<td bgcolor=\"#B2B2B2\" valign=\"middle\" align=\"right\" width=\"200\" height=\"25\"><font face=\"Arial\" color=\"#000000\" size=\"2\">Signal</font></td>\n");
				websWrite(wp, "<td bgcolor=\"#B2B2B2\"></td>\n");
				websWrite(wp, "<td bgcolor=\"#FFFFFF\"></td>\n");
				websWrite(wp, "<td colspan=\"2\" bgcolor=\"#FFFFFF\" valign=\"middle\" align=\"left\"><font face=\"Arial\" color=\"#000000\" size=\"2\">%d dBm</font></td>\n", rssi);
				websWrite(wp, "</tr>\n");
				websWrite(wp, "<tr>\n");
				websWrite(wp, "<td bgcolor=\"#B2B2B2\" valign=\"middle\" align=\"right\" width=\"200\" height=\"25\"><font face=\"Arial\" color=\"#000000\" size=\"2\">Rauschen</font></td>\n");
				websWrite(wp, "<td bgcolor=\"#B2B2B2\"></td>\n");
				websWrite(wp, "<td bgcolor=\"#FFFFFF\"></td>\n");
				websWrite(wp, "<td colspan=\"2\" bgcolor=\"#FFFFFF\" valign=\"middle\" align=\"left\"><font face=\"Arial\" color=\"#000000\" size=\"2\">%d dBm</font></td>\n", noise);
				websWrite(wp, "</tr>\n");
			}
		}
		fclose(fp);
	}

	unlink(ASSOCLIST_TMP);
	unlink(RSSI_TMP);
	if (!connected) {
		connected = 1;
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td bgcolor=\"#B2B2B2\" valign=\"middle\" align=\"right\" width=\"200\" height=\"25\"><font face=\"Arial\" color=\"#000000\" size=\"2\"><b>Verbindungsstatus</b></font></td>\n");
		websWrite(wp, "<td bgcolor=\"#B2B2B2\"></td>\n");
		websWrite(wp, "<td bgcolor=\"#FFFFFF\"></td>\n");
		websWrite(wp, "<td colspan=\"2\" bgcolor=\"#FFFFFF\" valign=\"middle\" align=\"left\"><font face=\"Arial\" color=\"#000000\" size=\"2\">Nicht Verbunden</font></td>\n");
		websWrite(wp, "</tr>\n");

	}

	return 0;
}
#endif

void ej_show_paypal(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_DDLAN
	websWrite(wp, "<a href=\"mailto:support@mcdd.de\">support@mcdd.de</a><br />");
#endif
#ifdef HAVE_CORENET
	websWrite(wp, "<a href=\"http://www.corenetsolutions.com\">http://www.corenetsolutions.com</a><br />");
#endif

#ifndef CONFIG_BRANDING
#ifndef HAVE_REGISTER
	websWrite(wp, "<a href=\"http://www.dd-wrt.com/\">DD-WRT</a><br />");
	websWrite(wp, "<form action=\"https://www.paypal.com/cgi-bin/webscr\" method=\"post\" target=\"_blank\">");
	websWrite(wp, "<input type=\"hidden\" name=\"cmd\" value=\"_xclick\" />");
	websWrite(wp, "<input type=\"hidden\" name=\"business\" value=\"paypal@dd-wrt.com\" />");
	websWrite(wp, "<input type=\"hidden\" name=\"item_name\" value=\"DD-WRT Development Support\" />");
	websWrite(wp, "<input type=\"hidden\" name=\"no_note\" value=\"1\" />");
	websWrite(wp, "<input type=\"hidden\" name=\"currency_code\" value=\"EUR\" />");
	websWrite(wp, "<input type=\"hidden\" name=\"lc\" value=\"en\" />");
	websWrite(wp, "<input type=\"hidden\" name=\"tax\" value=\"0\" />");
	websWrite(wp, "<input type=\"image\" src=\"images/paypal.png\" name=\"submit\" />");
	websWrite(wp, "</form>");
	//websWrite(wp,
	//        "<br /><script type=\"text/javascript\">Capture(donate.mb)</script><br />\n");
	//websWrite(wp,
	//        "<a href=\"https://www.moneybookers.com/app/send.pl\" target=\"_blank\">\n");
	// #ifdef HAVE_MICRO
	// websWrite (wp,
	// "<img style=\"border-width: 1px; border-color: #8B8583;\"
	// src=\"http://www.moneybookers.com/images/banners/88_en_interpayments.gif\" 
	// alt=\"donate thru moneybookers\" />\n");
	// #else
	//websWrite(wp,
	//        "<img style=\"border-width: 1px; border-color: #8B8583;\" src=\"images/88_en_interpayments.png\" alt=\"donate thru interpayments\" />\n");
	// #endif
	//websWrite(wp, "</a>\n");
#endif
#endif
	return;
}

#ifdef HAVE_RADLOCAL

void ej_show_iradius_check(webs_t wp, int argc, char_t ** argv)
{
	char *sln = nvram_safe_get("iradius_count");

	if (sln == NULL || strlen(sln) == 0)
		return;
	int leasenum = atoi(sln);
	int i;

	for (i = 0; i < leasenum; i++) {
		websWrite(wp, "if(F._iradius%d_active)\n", i);
		websWrite(wp, "if(F._iradius%d_active.checked == true)\n", i);
		websWrite(wp, "F.iradius%d_active.value=1\n", i);
		websWrite(wp, "else\n");
		websWrite(wp, "F.iradius%d_active.value=0\n", i);

		websWrite(wp, "if(F._iradius%d_delete)\n", i);
		websWrite(wp, "if(F._iradius%d_delete.checked == true)\n", i);
		websWrite(wp, "F.iradius%d_delete.value=1\n", i);
		websWrite(wp, "else\n");
		websWrite(wp, "F.iradius%d_delete.value=0\n", i);
	}

}

void ej_show_iradius(webs_t wp, int argc, char_t ** argv)
{
	char *sln = nvram_safe_get("iradius_count");

	if (sln == NULL || strlen(sln) == 0)
		return;
	int leasenum = atoi(sln);

	if (leasenum == 0)
		return;
	int i;
	char username[32];
	char *o, *userlist;

	cprintf("get collection\n");
	char *u = nvram_get_collection("iradius");

	cprintf("collection result %s", u);
	if (u != NULL) {
		userlist = (char *)safe_malloc(strlen(u) + 1);
		strcpy(userlist, u);
		free(u);
		o = userlist;
	} else {
		userlist = NULL;
		o = NULL;
	}
	cprintf("display = chain\n");
	struct timeval now;

	gettimeofday(&now, NULL);
	for (i = 0; i < leasenum; i++) {
		snprintf(username, 31, "iradius%d_name", i);
		char *sep = NULL;

		if (userlist)
			sep = strsep(&userlist, " ");
		websWrite(wp, "<tr><td>\n");
		websWrite(wp, "<input name=\"%s\" type=\"hidden\" />", username);
		websWrite(wp, "<input name=\"%s\" value=\"%s\" size=\"25\" maxlength=\"63\" />\n", username, sep != NULL ? sep : "");
		websWrite(wp, "</td>\n");
		if (userlist)
			sep = strsep(&userlist, " ");

		char active[32];

		snprintf(active, 31, "iradius%d_active", i);

		websWrite(wp, "<td>\n");
		websWrite(wp, "<input name=\"%s\" type=\"hidden\" />", active);
		websWrite(wp, "<input type=\"checkbox\" value=\"%s\" name=\"_%s\" %s />\n", sep, active, sep != NULL ? strcmp(sep, "1") == 0 ? "checked=\"checked\"" : "" : "");
		websWrite(wp, "</td>\n");
		websWrite(wp, "<td>\n");
		if (userlist)
			sep = strsep(&userlist, " ");
		long t = atol(sep);

		if (t != -1) {
			t -= now.tv_sec;
			t /= 60;
		}

		snprintf(active, 31, "iradius%d_lease", i);
		char st[32];

		if (t >= 0)
			sprintf(st, "%d", t);
		else
			sprintf(st, "over");
		websWrite(wp, "<input type=\"num\" name=\"%s\" value='%s' />\n", active, st);
		websWrite(wp, "</td>\n");

		websWrite(wp, "<td>\n");
		snprintf(active, 31, "iradius%d_delete", i);
		websWrite(wp, "<input name=\"%s\" type=\"hidden\" />", active);
		websWrite(wp, "<input type=\"checkbox\" name=\"_%s\"/>\n", active);
		websWrite(wp, "</td></tr>\n");
	}
	if (o != NULL)
		free(o);
	return;
}

#endif

#ifdef HAVE_CHILLILOCAL

void ej_show_userlist(webs_t wp, int argc, char_t ** argv)
{
	char *sln = nvram_safe_get("fon_usernames");

	if (sln == NULL || strlen(sln) == 0)
		return;
	int leasenum = atoi(sln);

	if (leasenum == 0)
		return;
	int i;
	char username[32];
	char password[32];
	char *u = nvram_safe_get("fon_userlist");
	char *userlist = (char *)safe_malloc(strlen(u) + 1);

	strcpy(userlist, u);
	char *o = userlist;

	for (i = 0; i < leasenum; i++) {
		snprintf(username, 31, "fon_user%d_name", i);
		char *sep = strsep(&userlist, "=");

		websWrite(wp, "<tr><td>\n");
		websWrite(wp, "<input name=\"%s\" value=\"%s\" size=\"25\" maxlength=\"63\" />\n", username, sep != NULL ? sep : "");
		websWrite(wp, "</td>\n");
		sep = strsep(&userlist, " ");
		snprintf(password, 31, "fon_user%d_password", i);
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input type=\"password\" name=\"%s\" value=\"blahblahblah\" size=\"25\" maxlength=\"63\" />\n", password);
		websWrite(wp, "</td></tr>\n");
	}
	free(o);
	return;
}

#endif

void ej_show_staticleases(webs_t wp, int argc, char_t ** argv)
{
	int i;

	// cprintf("get static leasenum");

	char *sln = nvram_safe_get("static_leasenum");

	// cprintf("check null");
	if (sln == NULL || strlen(sln) == 0)
		return;
	// cprintf("atoi");

	int leasenum = atoi(sln);

	// cprintf("leasenum==0");
	if (leasenum == 0)
		return;
	// cprintf("get leases");
	char *nvleases = nvram_safe_get("static_leases");
	char *leases = (char *)safe_malloc(strlen(nvleases) + 1);
	char *originalpointer = leases;	// strsep destroys the pointer by

	// moving it
	strcpy(leases, nvleases);
	for (i = 0; i < leasenum; i++) {
		char *sep = strsep(&leases, "=");

		websWrite(wp, "<tr><td><input name=\"lease%d_hwaddr\" value=\"%s\" size=\"18\" maxlength=\"18\" onblur=\"valid_name(this,share.mac,SPACE_NO)\" /></td>", i, sep != NULL ? sep : "");
		sep = strsep(&leases, "=");
		websWrite(wp, "<td><input name=\"lease%d_hostname\" value=\"%s\" size=\"24\" maxlength=\"24\" onblur=\"valid_name(this,share.hostname,SPACE_NO)\" /></td>", i, sep != NULL ? sep : "");
		sep = strsep(&leases, "=");
		websWrite(wp, "<td><input name=\"lease%d_ip\" value=\"%s\" size=\"15\" maxlength=\"15\" class=\"num\" onblur=\"valid_name(this,share.ip,SPACE_NO)\" /></td>\n", i, sep != NULL ? sep : "");
		sep = strsep(&leases, " ");
		websWrite(wp,
			  "<td><input name=\"lease%d_time\" value=\"%s\" size=\"10\" maxlength=\"10\" class=\"num\" onblur=\"valid_name(this,share.time,SPACE_NO)\" /><script type=\"text/javascript\">Capture(share.minutes)</script></td></tr>\n",
			  i, sep != NULL ? sep : "");
	}
	free(originalpointer);
	return;
}

void ej_show_control(webs_t wp, int argc, char_t ** argv)
{
#ifdef CONFIG_BRANDING
	websWrite(wp, "Control Panel");
#else
	websWrite(wp, "DD-WRT Control Panel");
#endif
	return;
}

#ifdef HAVE_AQOS
void ej_show_default_level(webs_t wp, int argc, char_t ** argv)
{
	char *defaults = nvram_safe_get("svqos_defaults");

	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend><script type=\"text/javascript\">Capture(qos.legend6)</script></legend>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(qos.enabledefaultlvls)</script></div>\n");
	websWrite(wp, "<input type=\"checkbox\" onclick=\"defaultlvl_grey(this.checked,this.form)\" name=\"svqos_defaults\" value=\"1\" %s />\n", nvram_match("svqos_defaults", "1") ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">WAN <script type=\"text/javascript\">Capture(qos.bandwidth)</script> Up</div>\n");
	websWrite(wp, "<input type=\"num\" name=\"default_uplevel\" size=\"6\" value=\"%s\" %s/>\n", nvram_safe_get("default_uplevel"), (!strcmp(defaults, "1")) ? "" : "disabled");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">WAN <script type=\"text/javascript\">Capture(qos.bandwidth)</script> Down</div>\n");
	websWrite(wp, "<input type=\"num\" name=\"default_downlevel\" size=\"6\" value=\"%s\" %s/>\n", nvram_safe_get("default_downlevel"), (!strcmp(defaults, "1")) ? "" : "disabled");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">LAN <script type=\"text/javascript\">Capture(qos.bandwidth)</script></div>\n");
	websWrite(wp, "<input type=\"num\" name=\"default_lanlevel\" size=\"6\" value=\"%s\" %s/>\n", nvram_default_get("default_lanlevel", "100000"), (!strcmp(defaults, "1")) ? "" : "disabled");
	websWrite(wp, "</div>\n");
	websWrite(wp, "</fieldset><br />\n");
	return;
}
#endif

static char *selmatch(char *var, char *is, char *ret)
{
	if (nvram_match(var, is))
		return ret;
	return "";
}

static void show_security_prefix(webs_t wp, int argc, char_t ** argv, char *prefix, int primary)
{
	static char var[80];
	static char sta[80];

	// char p2[80];
	cprintf("show security prefix\n");
	sprintf(var, "%s_security_mode", prefix);
	// strcpy(p2,prefix);
	// rep(p2,'X','.');
	// websWrite (wp, "<input type=\"hidden\"
	// name=\"%s_security_mode\"/>\n",p2);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.secmode)</script></div>\n");
	websWrite(wp, "<select name=\"%s_security_mode\" onchange=\"SelMode('%s_security_mode',this.form.%s_security_mode.selectedIndex,this.form)\">\n", prefix, prefix, prefix);
	websWrite(wp,
#ifdef HAVE_IAS
		  "<option value=\"disabled\" %s>%s</option>\n", selmatch(var, "psk", "selected=\"selected\""), ias_enc_label("disabled"));
#else
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"disabled\\\" %s >\" + share.disabled + \"</option>\");\n//]]>\n</script>\n",
		  selmatch(var, "disabled", "selected=\\\"selected\\\""));
#endif
#ifdef HAVE_IAS
	websWrite(wp, "<option value=\"psk\" %s>%s</option>\n", selmatch(var, "psk", "selected=\"selected\""), ias_enc_label("psk"));
#else
	websWrite(wp, "<option value=\"psk\" %s>WPA Personal</option>\n", selmatch(var, "psk", "selected=\"selected\""));
#endif
	sprintf(sta, "%s_mode", prefix);
	if (!primary || nvram_match(sta, "ap") || nvram_match(sta, "wdsap")) {
		websWrite(wp,
#ifdef HAVE_IAS
			  "<option value=\"wpa\" %s>%s</option>\n", selmatch(var, "wpa", "selected=\"selected\""), ias_enc_label("wpa"));
#else
			  "<option value=\"wpa\" %s>WPA Enterprise</option>\n", selmatch(var, "wpa", "selected=\"selected\""));
#endif
	}
#ifdef HAVE_IAS
	websWrite(wp, "<option value=\"psk2\" %s>%s</option>\n", selmatch(var, "psk2", "selected=\"selected\""), ias_enc_label("psk2"));
#else
	websWrite(wp, "<option value=\"psk2\" %s>WPA2 Personal</option>\n", selmatch(var, "psk2", "selected=\"selected\""));
#endif
	if (!primary || nvram_match(sta, "ap") || nvram_match(sta, "wdsap")) {
		websWrite(wp,
#ifdef HAVE_IAS
			  "<option value=\"wpa2\" %s>%s</option>\n", selmatch(var, "wpa2", "selected=\"selected\""), ias_enc_label("wpa2"));
#else
			  "<option value=\"wpa2\" %s>WPA2 Enterprise</option>\n", selmatch(var, "wpa2", "selected=\"selected\""));
#endif
	}
#ifdef HAVE_RT2880
	if (!primary || nvram_match(sta, "ap"))
#endif
		websWrite(wp,
#ifdef HAVE_IAS
			  "<option value=\"psk psk2\" %s>%s</option>\n", selmatch(var, "psk psk2", "selected=\"selected\""), ias_enc_label("psk psk2"));
#else
			  "<option value=\"psk psk2\" %s>WPA2 Personal Mixed</option>\n", selmatch(var, "psk psk2", "selected=\"selected\""));
#endif

	if (!primary || nvram_match(sta, "ap") || nvram_match(sta, "wdsap")) {
		websWrite(wp,
#ifdef HAVE_IAS
			  "<option value=\"wpa wpa2\" %s>%s</option>\n", selmatch(var, "wpa wpa2", "selected=\"selected\""), ias_enc_label("wpa wpa2"));
#else
			  "<option value=\"wpa wpa2\" %s>WPA2 Enterprise Mixed</option>\n", selmatch(var, "wpa wpa2", "selected=\"selected\""));
#endif

#ifdef HAVE_IAS
#ifdef HAVE_ATH9K
		if (!is_ath9k(prefix))
// disabled -> not implemented for newer wireless drivers
#endif
			websWrite(wp, "<option value=\"radius\" %s>%s</option>\n", selmatch(var, "radius", "selected=\"selected\""), ias_enc_label("radius"));
#else
		websWrite(wp, "<option value=\"radius\" %s>RADIUS</option>\n", selmatch(var, "radius", "selected=\"selected\""));
#endif
	}
#ifdef HAVE_IAS
	websWrite(wp, "<option value=\"wep\" %s>%s</option>\n", selmatch(var, "wep", "selected=\"selected\""), ias_enc_label("wep"));
#else
	websWrite(wp, "<option value=\"wep\" %s>WEP</option>\n", selmatch(var, "wep", "selected=\"selected\""));
#endif
#ifdef HAVE_WPA_SUPPLICANT
#ifndef HAVE_MICRO
#ifndef HAVE_RT2880
	if (nvram_match(sta, "sta") || nvram_match(sta, "wdssta")
	    || nvram_match(sta, "apsta") || nvram_match(sta, "wet")) {
		websWrite(wp, "<option value=\"8021X\" %s>802.1x</option>\n", selmatch(var, "8021X", "selected=\"selected\""));
	}
#else
#ifndef HAVE_RT61
	if (nvram_match(sta, "sta") || nvram_match(sta, "wet")) {
		websWrite(wp, "<option value=\"8021X\" %s>802.1x</option>\n", selmatch(var, "8021X", "selected=\"selected\""));
	}
#endif
#endif
#endif
#endif

	websWrite(wp, "</select></div>\n");
	rep(prefix, 'X', '.');
	cprintf("ej show wpa\n");
	ej_show_wpa_setting(wp, argc, argv, prefix);

}

static void ej_show_security_single(webs_t wp, int argc, char_t ** argv, char *prefix)
{
	char *next;
	char var[80];
	char ssid[80];
	char mac[16];

#ifdef HAVE_GUESTPORT
	char guestport[16];
	sprintf(guestport, "guestport_%s", prefix);
#endif
	sprintf(mac, "%s_hwaddr", prefix);
	char *vifs = nvram_nget("%s_vifs", prefix);

	if (vifs == NULL)
		return;
	sprintf(ssid, "%s_ssid", prefix);
	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(wpa.h2)</script> %s</h2>\n", prefix);
	websWrite(wp, "<fieldset>\n");
	// cprintf("getting %s %s\n",ssid,nvram_safe_get(ssid));
	websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.pintrface)</script> %s SSID [", IFMAP(prefix));
	tf_webWriteESCNV(wp, ssid);	// fix for broken html page if ssid
	// contains html tag
	websWrite(wp, "] HWAddr [%s]</legend>\n", nvram_safe_get(mac));
	show_security_prefix(wp, argc, argv, prefix, 1);
	websWrite(wp, "</fieldset>\n<br />\n");
	foreach(var, vifs, next) {
#ifdef HAVE_GUESTPORT
		if (nvram_match(guestport, var))
			continue;
#endif
		sprintf(ssid, "%s_ssid", var);
		websWrite(wp, "<fieldset>\n");
		// cprintf("getting %s %s\n", ssid,nvram_safe_get(ssid));
		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.vintrface)</script> %s SSID [", IFMAP(var));
		tf_webWriteESCNV(wp, ssid);	// fix for broken html page if ssid
		// contains html tag
		sprintf(mac, "%s_hwaddr", var);
		if (nvram_get(mac))
			websWrite(wp, "] HWAddr [%s", nvram_safe_get(mac));

		websWrite(wp, "]</legend>\n");
		rep(var, '.', 'X');
		show_security_prefix(wp, argc, argv, var, 0);
		websWrite(wp, "</fieldset>\n<br />\n");
	}
#ifdef HAVE_GUESTPORT
	foreach(var, vifs, next) {
		if (nvram_match(guestport, var)) {
			websWrite(wp, "<h2>Guestport</h2>\n");

			sprintf(ssid, "%s_ssid", var);
			websWrite(wp, "<fieldset>\n");
			websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.vintrface)</script> %s SSID [", IFMAP(var));
			tf_webWriteESCNV(wp, ssid);	// fix for broken html page if ssid
			// contains html tag
			sprintf(mac, "%s_hwaddr", var);
			if (nvram_get(mac))
				websWrite(wp, "] HWAddr [%s", nvram_safe_get(mac));

			websWrite(wp, "]</legend>\n");
			rep(var, '.', 'X');
			show_security_prefix(wp, argc, argv, var, 0);
			websWrite(wp, "</fieldset>\n<br />\n");
		}
	}
#endif
}

void ej_show_security(webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "wl%d", i);
		ej_show_security_single(wp, argc, argv, buf);
	}
	return;
#else
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "ath%d", i);
		ej_show_security_single(wp, argc, argv, buf);
	}
	return;
#endif
}

void ej_show_dhcpd_settings(webs_t wp, int argc, char_t ** argv)
{
	int i;

	if (getWET())		// dhcpd settings disabled in client bridge mode, so we wont display it
		return;

	websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(idx.dhcp_legend)</script></legend>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_type)</script></div>\n");
	websWrite(wp, "<select class=\"num\" size=\"1\" name=\"dhcpfwd_enable\" onchange=SelDHCPFWD(this.form.dhcpfwd_enable.selectedIndex,this.form)>\n");
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + idx.dhcp_srv + \"</option>\");\n", nvram_match("dhcpfwd_enable", "0") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + idx.dhcp_fwd + \"</option>\");\n", nvram_match("dhcpfwd_enable", "1") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
	if (nvram_match("dhcpfwd_enable", "1")) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_srv)</script></div>\n");
		char *ipfwd = nvram_safe_get("dhcpfwd_ip");

		websWrite(wp,
			  "<input type=\"hidden\" name=\"dhcpfwd_ip\" value=\"4\" /><input class=\"num\" maxlength=\"3\" size=\"3\" name=\"dhcpfwd_ip_0\" onblur=\"valid_range(this,0,255,idx.dhcp_srv)\" value=\"%d\" />.<input class=\"num\" maxlength=\"3\" size=\"3\" name=\"dhcpfwd_ip_1\" onblur=\"valid_range(this,0,255,idx.dhcp_srv)\" value=\"%d\" />.<input class=\"num\" maxlength=\"3\" name=\"dhcpfwd_ip_2\" size=\"3\" onblur=\"valid_range(this,0,255,idx.dhcp_srv)\" value=\"%d\" />.<input class=\"num\" maxlength=\"3\" name=\"dhcpfwd_ip_3\" size=\"3\" onblur=\"valid_range(this,0,254,idx.dhcp_srv)\" value=\"%d\"\" /></div>\n",
			  get_single_ip(ipfwd, 0), get_single_ip(ipfwd, 1), get_single_ip(ipfwd, 2), get_single_ip(ipfwd, 3));
	} else {
		char buf[20];

		prefix_ip_get("lan_ipaddr", buf, 1);
		websWrite(wp, "<div class=\"setting\">\n");
		// char *nv = nvram_safe_get ("wan_wins");
		websWrite(wp,
			  "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_srv)</script></div><input class=\"spaceradio\" type=\"radio\" name=\"lan_proto\" value=\"dhcp\" onclick=SelDHCP('dhcp',this.form) %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			  nvram_match("lan_proto", "dhcp") ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" name=\"lan_proto\" value=\"static\" onclick=\"SelDHCP('static',this.form)\" %s /><script type=\"text/javascript\">Capture(share.disable)</script></div><input type=\"hidden\" name=\"dhcp_check\" /><div class=\"setting\">\n",
			  nvram_match("lan_proto", "static") ? "checked=\"checked\"" : "");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_start)</script></div>%s", buf);
		websWrite(wp, "<input class=\"num\" name=\"dhcp_start\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,1,254,idx.dhcp_start)\" value=\"%s\" />", nvram_safe_get("dhcp_start"));
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp,
			  "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_maxusers)</script></div><input class=\"num\" name=\"dhcp_num\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,253,idx.dhcp_maxusers)\" value=\"%s\" /></div>\n",
			  nvram_safe_get("dhcp_num"));
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp,
			  "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_lease)</script></div><input class=\"num\" name=\"dhcp_lease\" size=\"5\" maxlength=\"5\" onblur=\"valid_range(this,0,99999,idx.dhcp_lease)\" value=\"%s\" > <script type=\"text/javascript\">Capture(share.minutes)</script></input></div>\n",
			  nvram_safe_get("dhcp_lease"));
		if (nvram_invmatch("wan_proto", "static")) {
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx_static.dns)</script> 1</div>");
			websWrite(wp, "<input type=\"hidden\" name=\"wan_dns\" value=\"4\" />");
			for (i = 0; i < 4; i++)
				websWrite(wp,
					  "<input class=\"num\" name=\"wan_dns0_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,idx_static.dns)\" value=\"%d\" />%s",
					  i, i == 3 ? 254 : 255, get_dns_ip("wan_dns", 0, i), i < 3 ? "." : "");

			websWrite(wp, "\n</div>\n<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx_static.dns)</script> 2</div>");
			for (i = 0; i < 4; i++)
				websWrite(wp,
					  "<input class=\"num\" name=\"wan_dns1_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,idx_static.dns)\" value=\"%d\" />%s",
					  i, i == 3 ? 254 : 255, get_dns_ip("wan_dns", 1, i), i < 3 ? "." : "");

			websWrite(wp, "\n</div>\n<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx_static.dns)</script> 3</div>");
			for (i = 0; i < 4; i++)
				websWrite(wp,
					  "<input class=\"num\" name=\"wan_dns2_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,idx_static.dns)\" value=\"%d\" />%s",
					  i, i == 3 ? 254 : 255, get_dns_ip("wan_dns", 2, i), i < 3 ? "." : "");
			websWrite(wp, "\n</div>");
		}
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">WINS</div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"wan_wins\" value=\"4\" />\n");
		char *wins = nvram_safe_get("wan_wins");

		for (i = 0; i < 4; i++) {
			websWrite(wp,
				  "<input class=\"num\" name=\"wan_wins_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,&#34;WINS&#34;)\" value=\"%d\" />%s",
				  i, i == 3 ? 254 : 255, get_single_ip(wins, i), i < 3 ? "." : "");
		}

		websWrite(wp, "</div>\n<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_dnsmasq)</script></div>\n");
		websWrite(wp, "<input type=\"checkbox\" name=\"_dhcp_dnsmasq\" value=\"1\" onclick=\"setDNSMasq(this.form)\" %s />\n", nvram_match("dhcp_dnsmasq", "1") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dns_dnsmasq)</script></div>\n");
		websWrite(wp, "<input type=\"checkbox\" name=\"_dns_dnsmasq\" value=\"1\" %s />\n", nvram_match("dns_dnsmasq", "1") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.auth_dnsmasq)</script></div>\n");
		websWrite(wp, "<input type=\"checkbox\" name=\"_auth_dnsmasq\" value=\"1\" %s />\n", nvram_match("auth_dnsmasq", "1") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
	}

	websWrite(wp, "</fieldset><br />\n");
	return;
}

#ifdef HAVE_MADWIFI
void ej_show_wifiselect(webs_t wp, int argc, char_t ** argv)
{
	char *next;
	char var[32];
	int count = getdevicecount();

	if (count < 1)
		return;

	if (count == 1 && strlen(nvram_safe_get("ath0_vifs")) == 0)
		return;

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.intrface)</script></div>\n");
	websWrite(wp, "<select name=\"wifi_display\" onchange=\"refresh(this.form)\">\n");
	int i;

	for (i = 0; i < count; i++) {
		sprintf(var, "ath%d", i);
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, nvram_match("wifi_display", var) ? "selected=\"selected\"" : "", var);
		char *names = nvram_nget("ath%d_vifs", i);

		foreach(var, names, next) {
			websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, nvram_match("wifi_display", var) ? "selected=\"selected\"" : "", var);
		}
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

}
#else
void ej_show_wifiselect(webs_t wp, int argc, char_t ** argv)
{
	char *next;
	char var[32];
	int count = get_wl_instances();

	if (count < 2)
		return;
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.intrface)</script></div>\n");
	websWrite(wp, "<select name=\"wifi_display\" onchange=\"refresh(this.form)\">\n");
	int i;

	for (i = 0; i < count; i++) {
		sprintf(var, "wl%d", i);
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, nvram_match("wifi_display", var) ? "selected=\"selected\"" : "", var);
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
}

#endif
#if 0
static void showOption(webs_t wp, char *propname, char *nvname)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(%s)</script></div>\n<select name=\"%s\">\n", propname, nvname);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + share.disabled + \"</option>\");\n", nvram_default_match(nvname, "0", "0") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + share.enabled + \"</option>\");\n", nvram_default_match(nvname, "1", "0") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");

}
#endif
void showRadio(webs_t wp, char *propname, char *nvname)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(%s)</script></div>\n", propname);
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		  nvname, nvram_default_match(nvname, "1", "0") ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
		  nvname, nvram_default_match(nvname, "0", "0") ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
}

#ifdef HAVE_MADWIFI
void showAutoOption(webs_t wp, char *propname, char *nvname)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(%s)</script></div>\n<select name=\"%s\">\n", propname, nvname);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"-1\\\" %s >\" + share.auto + \"</option>\");\n", nvram_default_match(nvname, "0", "-1") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + share.enabled + \"</option>\");\n", nvram_default_match(nvname, "1", "-1") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + share.disabled + \"</option>\");\n", nvram_default_match(nvname, "0", "-1") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");

}
#endif

static void showOptions(webs_t wp, char *propname, char *names, char *select)
{
	char *next;
	char var[80];

	websWrite(wp, "<select name=\"%s\">\n", propname);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	foreach(var, names, next) {
		websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", var, !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", var);
	}
	websWrite(wp, "//]]>\n</script>\n</select>\n");
}

static void showOptionsChoose(webs_t wp, char *propname, char *names, char *select)
{
	char *next;
	char var[80];

	websWrite(wp, "<select name=\"%s\">\n", propname);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"null\\\" >Please choose...</option>\");\n");
	foreach(var, names, next) {
		websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", var, !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", var);
	}
	websWrite(wp, "//]]>\n</script>\n</select>\n");
}

static void showOptionsLabel(webs_t wp, char *labelname, char *propname, char *names, char *select)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(%s)</script></div>", labelname);
	showOptions(wp, propname, names, select);
	websWrite(wp, "</div>\n");

}

void show_inputlabel(webs_t wp, char *labelname, char *propertyname, int propertysize, char *inputclassname, int inputmaxlength)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(%s)</script></div>", labelname);
	websWrite(wp, "<input class=\"%s\" size=\"%d\" maxlength=\"%d\" name=\"%s\" value=\"%s\" />\n", inputclassname, propertysize, inputmaxlength, propertyname, nvram_safe_get(propertyname));
	websWrite(wp, "</div>\n");
}

void show_custominputlabel(webs_t wp, char *labelname, char *propertyname, char *property, int propertysize)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">%s</div>", labelname);
	websWrite(wp, "<input size=\"%d\" name=\"%s\" value=\"%s\" />\n", propertysize, propertyname, property);
	websWrite(wp, "</div>\n");
}

#ifdef HAVE_USB
void ej_show_usb_diskinfo(webs_t wp, int argc, char_t ** argv)
{
	char buff[512];
	FILE *fp;
	int mounted = 0;
	if (!nvram_match("usb_automnt", "1"))
		return;

	if ((fp = fopen("/tmp/disktype.dump", "r"))) {
		while (fgets(buff, sizeof(buff), fp)) {
			if (strcmp(buff, "\n"))
				websWrite(wp, "%s<br />", buff);
		}
		fclose(fp);
		mounted = 1;
	}
	if ((fp = fopen("/tmp/parttype.dump", "r"))) {
		while (fgets(buff, sizeof(buff), fp)) {
			if (strcmp(buff, "\n"))
				websWrite(wp, "%s<br />", buff);
		}
		fclose(fp);
		mounted = 1;
	}

	if (!mounted)
		websWrite(wp, "%s", live_translate("status_router.notavail"));

	return;
}
#endif

#ifdef HAVE_MMC
void ej_show_mmc_cardinfo(webs_t wp, int argc, char_t ** argv)
{
	char buff[512];
	FILE *fp;

	if (!nvram_match("mmc_enable0", "1"))
		return;

	if ((fp = fopen("/proc/mmc/status", "rb"))) {
		while (fgets(buff, sizeof(buff), fp)) {
			if (strcmp(buff, "\n"))
				websWrite(wp, "%s<br />", buff);
		}
		fclose(fp);
	} else
		websWrite(wp, "%s", live_translate("status_router.notavail"));

	return;
}
#endif

void show_legend(webs_t wp, char *labelname, int translate)
{
	/*
	 * char buf[2]; sprintf(buf,"%d",translate); websWrite (wp,
	 * "<legend>%s%s%s</legend>\n", !strcmp (buf, "1") ? "<script
	 * type=\"text/javascript\">Capture(" : "", labelname, !strcmp (buf, "1") 
	 * ? ")</script>" : ""); 
	 */
	if (translate)
		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(%s)</script></legend>\n", labelname);
	else
		websWrite(wp, "<legend>%s</legend>\n", labelname);

}

#ifdef HAVE_OLSRD

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
		showOptions(wp, "olsrd_ifname", buffer, "");
		websWrite(wp, "&nbsp;&nbsp;");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"olsrd_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</fieldset><br />\n");
	}
}
#endif

#ifdef HAVE_VLANTAGGING
#ifdef HAVE_BONDING

void ej_show_bondings(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	char bufferif[512];
	char bondnames[256];
	int count = 0;
	static char word[256];
	char *next, *wordlist;

	memset(buffer, 0, 256);
	memset(bondnames, 0, 256);
	memset(bufferif, 0, 512);
	websWrite(wp, "<h2>Bonding</h2>\n");
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend>Bonding</legend>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">Bonding Type</div>\n", count);
	showOptions(wp, "bonding_type", "balance-rr active-backup balance-xor broadcast 802.3ad balance-tlb balance-alb weighted-rr duplex", nvram_default_get("bonding_type", "balance-rr"));
	websWrite(wp, "&nbsp;Bonding Interfaces&nbsp;");
	websWrite(wp, "<input class=\"num\" name=\"bonding_number\"size=\"5\" value=\"%s\" />\n", nvram_default_get("bonding_number", "1"));
	websWrite(wp, "</div>\n");

	getIfList(bufferif, "eth");
	int i;

#ifdef HAVE_XSCALE
	memset(buffer, 0, 256);
	getIfList(buffer, "ixp");
	sprintf(bufferif, "%s %s", bufferif, buffer);
#endif
	memset(buffer, 0, 256);
	getIfList(buffer, "br");
	sprintf(bufferif, "%s %s", bufferif, buffer);
#ifdef HAVE_MADWIFI
	int c = getdevicecount();

	for (i = 0; i < c; i++) {
		char ath[32];

		sprintf(ath, "ath%d_bridged", i);
		if (nvram_default_match(ath, "0", "1")) {
			sprintf(bufferif, "%s ath%d", bufferif, i);
			char vifs[32];

			sprintf(vifs, "ath%d_vifs", i);
			sprintf(bufferif, "%s %s", bufferif, nvram_safe_get(vifs));
		}
	}
#endif

	for (i = 0; i < atoi(nvram_safe_get("bonding_number")); i++) {
		sprintf(bondnames, "%s bond%d", bondnames, i);
	}
	int totalcount = 0;
	int realcount = atoi(nvram_default_get("bonding_count", "0"));

	wordlist = nvram_safe_get("bondings");
	foreach(word, wordlist, next) {
		char *port = word;
		char *tag = strsep(&port, ">");

		if (!tag || !port)
			break;
		char vlan_name[32];

		// sprintf (vlan_name, "%s.%s", tag, port);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">Bonding %d Assignment</div>\n", count);
		websWrite(wp, "&nbsp;Bond&nbsp;");
		sprintf(vlan_name, "bondingifname%d", count);
		showOptions(wp, vlan_name, bondnames, tag);
		sprintf(vlan_name, "bondingattach%d", count);
		websWrite(wp, "&nbsp;Slave&nbsp;");
		showOptions(wp, vlan_name, bufferif, port);
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bond_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		count++;
	}
	totalcount = count;
	for (i = count; i < realcount; i++) {
		char vlan_name[32];

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">Bonding %d Interface</div>\n", i);
		websWrite(wp, "&nbsp;Bond&nbsp;");
		sprintf(vlan_name, "bondingifname%d", i);
		showOptions(wp, vlan_name, bondnames, "");
		sprintf(vlan_name, "bondingattach%d", i);
		websWrite(wp, "&nbsp;Slave&nbsp;");
		showOptions(wp, vlan_name, bufferif, "");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bond_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("bonding_count", var);
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"bond_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</fieldset><br />\n");
}
#endif

void ej_show_vlantagging(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	int count = 0;
	static char word[256];
	char *next, *wordlist;

	memset(buffer, 0, 256);
	getIfList(buffer, NULL);
	int totalcount = 0;
	int realcount = atoi(nvram_default_get("vlan_tagcount", "0"));

	wordlist = nvram_safe_get("vlan_tags");
	foreach(word, wordlist, next) {

		char *port = word;
		char *tag = strsep(&port, ">");
		char *prio = port;
		strsep(&prio, ">");

		if (!tag || !port)
			break;
		if (!prio)
			prio = "0";
		char vlan_name[32];

		// sprintf (vlan_name, "%s.%s", tag, port);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">VLAN%d Interface</div>\n", count);
		sprintf(vlan_name, "vlanifname%d", count);
		showOptions(wp, vlan_name, buffer, tag);
		//tag number
		sprintf(vlan_name, "vlantag%d", count);
		websWrite(wp, "&nbsp;Tag Number&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" />\n", vlan_name, port);
		//priority
		sprintf(vlan_name, "vlanprio%d", count);
		websWrite(wp, "&nbsp;Prio&nbsp;");
		showOptions(wp, vlan_name, "0 1 2 3 4 5 6 7", prio);

		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"vlan_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		count++;
	}
	totalcount = count;
	int i;

	for (i = count; i < realcount; i++) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">VLAN%d Interface</div>\n", i);
		char vlan_name[32];

		sprintf(vlan_name, "vlanifname%d", i);
		showOptions(wp, vlan_name, buffer, "");
		sprintf(vlan_name, "vlantag%d", i);
		//tag number
		websWrite(wp, "&nbsp;Tag Number&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"0\" />\n", vlan_name);
		//priority
		sprintf(vlan_name, "vlanprio%d", i);
		websWrite(wp, "&nbsp;Prio&nbsp;");
		showOptions(wp, vlan_name, "0 1 2 3 4 5 6 7", "0");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"vlan_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("vlan_tagcount", var);
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"vlan_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
}

void ej_show_mdhcp(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	int count = 0;
	static char word[256];
	char *next, *wordlist;

	websWrite(wp, "<h2>%s</h2>\n<fieldset>\n", live_translate("networking.h5"));
	websWrite(wp, "<legend>%s</legend>\n", live_translate("networking.legend5"));

	memset(buffer, 0, 256);
	getIfList(buffer, NULL);
	int totalcount = 0;
	int realcount = atoi(nvram_default_get("mdhcpd_count", "0"));

	wordlist = nvram_safe_get("mdhcpd");
	foreach(word, wordlist, next) {
		char *interface = word;
		char *dhcpon = interface;

		interface = strsep(&dhcpon, ">");
		char *start = dhcpon;

		dhcpon = strsep(&start, ">");
		char *max = start;

		start = strsep(&max, ">");
		char *leasetime = max;

		max = strsep(&leasetime, ">");
		if (max == NULL) {
			max = leasetime;
			leasetime = "3660";
		}
		if (!interface || !start || !dhcpon || !max || !leasetime)
			break;
		char vlan_name[32];

		// interface
		char *ipaddr = nvram_nget("%s_ipaddr", interface);
		char *netmask = nvram_nget("%s_netmask", interface);

		if (strlen(ipaddr) > 0 && strlen(netmask) > 0) {
			websWrite(wp, "Interface %s: IP %s/%s\n", interface, ipaddr, netmask);
		}
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">DHCP %d</div>\n", count);
		sprintf(vlan_name, "mdhcpifname%d", count);
		showOptions(wp, vlan_name, buffer, interface);
		// on off
		sprintf(vlan_name, "mdhcpon%d", count);
		showOptions(wp, vlan_name, "On Off", dhcpon);
		// start
		sprintf(vlan_name, "mdhcpstart%d", count);
		websWrite(wp, "&nbsp;Start&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, start);
		// max
		sprintf(vlan_name, "mdhcpmax%d", count);
		websWrite(wp, "&nbsp;Max&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, max);
		sprintf(vlan_name, "mdhcpleasetime%d", count);
		websWrite(wp, "&nbsp;Leasetime&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n", vlan_name, leasetime);
		// 
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"mdhcp_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		count++;
	}
	totalcount = count;
	int i;

	for (i = count; i < realcount; i++) {
		char vlan_name[32];

		// sprintf (mdhcp_name, "%s.%s", tag, port);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">DHCP %d</div>\n", totalcount);
		// interface
		sprintf(vlan_name, "mdhcpifname%d", totalcount);
		showOptions(wp, vlan_name, buffer, "");
		// on off
		sprintf(vlan_name, "mdhcpon%d", totalcount);
		showOptions(wp, vlan_name, "On Off", "");
		// start
		sprintf(vlan_name, "mdhcpstart%d", totalcount);
		websWrite(wp, "&nbsp;Start&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, "100");
		// max
		sprintf(vlan_name, "mdhcpmax%d", totalcount);
		websWrite(wp, "&nbsp;Max&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, "50");
		sprintf(vlan_name, "mdhcpleasetime%d", totalcount);
		websWrite(wp, "&nbsp;Leasetime&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n", vlan_name, "3600");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"mdhcp_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("mdhcpd_count", var);
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"mdhcp_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</fieldset><br />\n");

}

void ej_show_bridgenames(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	int count = 0;
	int br0found = 0;
	static char word[256];
	char *next, *wordlist;
	char *stp = word;
	char *bridge, *prio, *mtu;
	char bridge_name[32];

	memset(buffer, 0, 256);
	getIfList(buffer, NULL);
	int realcount = atoi(nvram_default_get("bridges_count", "0"));

	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next) {
		bridge = strsep(&stp, ">");
		if (!strcmp(bridge, "br0")) {
			br0found = 1;
			break;
		}
	}
	if (!br0found) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">Bridge %d</div>\n", count);
		sprintf(bridge_name, "bridgename%d", count);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"br0\" />\n", bridge_name);
#ifdef HAVE_MSTP
		websWrite(wp, "&nbsp;MSTP&nbsp;");
#else
		websWrite(wp, "&nbsp;STP&nbsp;");
#endif
		sprintf(bridge_name, "bridgestp%d", count);
		showOptions(wp, bridge_name, "On Off", "Off");
		websWrite(wp, "&nbsp;Prio&nbsp;");
		sprintf(bridge_name, "bridgeprio%d", count);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"32768\" />\n", bridge_name);
		websWrite(wp, "&nbsp;MTU&nbsp;");
		// Bridges are bridges, Ports are ports, show it again HERE          
		sprintf(bridge_name, "bridgemtu%d", count);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"1500\" />\n", bridge_name);
		websWrite(wp, "</div>\n");
		// don't show that here, since that is under Basic Setup
		// show_ipnetmask(wp, bridge);
		count++;
	}

	foreach(word, wordlist, next) {

		stp = word;
		bridge = strsep(&stp, ">");
		prio = stp;

		stp = strsep(&prio, ">");
		mtu = prio;

		prio = strsep(&mtu, ">");
		if (!prio) {
			prio = mtu;
			mtu = "1500";
		}

/*	char *stp = word;
	char *bridge = strsep( &stp, ">" );
	char *mtu = stp;
	char *prio = strsep( &mtu, ">" );
*/
		if (!bridge || !stp)
			break;

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">Bridge %d</div>\n", count);
		sprintf(bridge_name, "bridgename%d", count);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" />\n", bridge_name, bridge);
#ifdef HAVE_MSTP
		websWrite(wp, "&nbsp;MSTP&nbsp;");
#else
		websWrite(wp, "&nbsp;STP&nbsp;");
#endif
		sprintf(bridge_name, "bridgestp%d", count);
		showOptions(wp, bridge_name, "On Off", stp);
		websWrite(wp, "&nbsp;Prio&nbsp;");
		sprintf(bridge_name, "bridgeprio%d", count);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" />\n", bridge_name, prio != NULL ? prio : "32768");
		websWrite(wp, "&nbsp;MTU&nbsp;");
		// Bridges are bridges, Ports are ports, show it again HERE          
		sprintf(bridge_name, "bridgemtu%d", count);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" />\n", bridge_name, mtu != NULL ? mtu : "1500");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bridge_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		// don't show that here, since that is under Basic Setup
		if (strcmp(bridge, "br0")) {
			show_ipnetmask(wp, bridge);
		}
		count++;
	}
	int i;
	int totalcount = count;

	for (i = count; i < realcount; i++) {

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">Bridge %d</div>\n", i);
		sprintf(bridge_name, "bridgename%d", i);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" />\n", bridge_name);
#ifdef HAVE_MSTP
		websWrite(wp, "&nbsp;MSTP&nbsp;");
#else
		websWrite(wp, "&nbsp;STP&nbsp;");
#endif
		sprintf(bridge_name, "bridgestp%d", i);
		showOptions(wp, bridge_name, "On Off", "On");
		websWrite(wp, "&nbsp;Prio&nbsp;");
		sprintf(bridge_name, "bridgeprio%d", i);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" />\n", bridge_name, "32768");
		websWrite(wp, "&nbsp;MTU&nbsp;");
		sprintf(bridge_name, "bridgemtu%d", count);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" />\n", bridge_name, "1500");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bridge_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"bridge_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("bridges_count", var);
}

void ej_show_bridgetable(webs_t wp, int argc, char_t ** argv)
{

	FILE *f;
	char buf[128];
	char brname[32];
	char brstp[8];
	char brif[16];
	int count = 0;

	system2("brctl show > /tmp/.brtable");

	if ((f = fopen("/tmp/.brtable", "r")) != NULL) {

		while (fgets(buf, sizeof(buf), f)) {

			if (count)	// skip line 0
			{
				strcpy(brname, "");
				strcpy(brstp, "");
				strcpy(brif, "");

				if (strncmp(buf, "\t\t\t", 3) != 0) {
					if (count != 1)
						websWrite(wp, "\',");	// close
					sscanf(buf, "%s %*s %s %s", brname, brstp, brif);
					websWrite(wp, "\'%s\',\'%s\',\'%s ", brname, brstp, brif);
				} else {
					sscanf(buf, "%s", brif);
					websWrite(wp, "%s ", brif);
				}
			}
			count++;
		}

		websWrite(wp, "\'");	// close
		fclose(f);
		unlink("/tmp/.brtable");
	}
	return;
}

void ej_show_bridgeifnames(webs_t wp, int argc, char_t ** argv)
{
	char bufferif[512];
	char bufferif2[256];
	char finalbuffer[512];
	int count = 0;
	int c = 0;
	static char word[256];
	char *next, *wordlist;

	memset(bufferif, 0, 512);
	memset(bufferif2, 0, 256);
	getIfList(bufferif, "eth");
#ifdef HAVE_GATEWORX
	getIfList(bufferif2, "ixp");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);
#endif

	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "vlan");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);

	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "wl");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);

	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "ofdm");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);

#ifdef HAVE_RT2880
	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "ra");
	sprintf(bufferif, "%s %s", bufferif, bufferif2);
#endif
	memset(bufferif2, 0, 256);
	getIfList(bufferif2, "br");
	foreach(word, bufferif2, next) {
		if (contains(word, '.'))
			sprintf(bufferif, "%s %s", bufferif, word);
	}
	int i;

#ifdef HAVE_MADWIFI
//      memset(bufferif2, 0, 256);
//      getIfList(bufferif2, "ath");
//      foreach(word, bufferif2, next) {
//              if (contains(word, '.'))
//                      sprintf(bufferif, "%s %s", bufferif, word);
//      }
	c = getdevicecount();

	for (i = 0; i < c; i++) {
		char ath[32];

//              sprintf(ath, "ath%d_bridged", i);
//              if (nvram_default_match(ath, "1", "1")) 
		{
			sprintf(bufferif, "%s ath%d", bufferif, i);
			char vifs[32];

			sprintf(vifs, "ath%d_vifs", i);
			sprintf(bufferif, "%s %s", bufferif, nvram_safe_get(vifs));
		}
	}
#endif
#ifdef HAVE_BONDING
	c = atoi(nvram_default_get("bonding_number", "1"));
	for (i = 0; i < c; i++) {
		sprintf(bufferif, "%s bond%d", bufferif, i);
	}
#endif
#ifdef HAVE_EOP_TUNNEL
	for (i = 1; i < 11; i++) {
		char EOP[32];

		if (nvram_nmatch("1", "oet%d_en", i)
		    && nvram_nmatch("1", "oet%d_bridged", i)) {
			sprintf(EOP, "oet%d", i);
			sprintf(bufferif, "%s %s", bufferif, EOP);
		}
	}
#endif
	char buffer[256];

	memset(buffer, 0, 256);
	getIfList(buffer, "br");

	memset(finalbuffer, 0, 256);
	foreach(word, buffer, next) {
		if (!contains(word, '.'))
			sprintf(finalbuffer, "%s %s", finalbuffer, word);
	}
	char *checkbuffer = safe_malloc(strlen(finalbuffer) + 6);
	memset(checkbuffer, 0, strlen(finalbuffer) + 6);
	strcpy(checkbuffer, "none ");
	strcat(checkbuffer, finalbuffer);
	strcpy(finalbuffer, checkbuffer);
	free(checkbuffer);
	int realcount = atoi(nvram_default_get("bridgesif_count", "0"));

	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next) {
		char *port = word;
		char *tag = strsep(&port, ">");
		char *prio = port;

		strsep(&prio, ">");
		if (!tag || !port)
			break;
		char vlan_name[32];

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">Assignment %d</div>\n", count);
		sprintf(vlan_name, "bridge%d", count);
		showOptions(wp, vlan_name, finalbuffer, tag);
		websWrite(wp, "&nbsp;Interface&nbsp;");
		sprintf(vlan_name, "bridgeif%d", count);
		showOptions(wp, vlan_name, bufferif, port);
		websWrite(wp, "&nbsp;Prio&nbsp;");
		sprintf(vlan_name, "bridgeifprio%d", count);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"3\" value=\"%s\" />\n", vlan_name, prio != NULL ? prio : "63");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bridgeif_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		count++;
	}
	int totalcount = count;

	for (i = count; i < realcount; i++) {
		char vlan_name[32];

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">Assignment %d</div>\n", i);
		sprintf(vlan_name, "bridge%d", i);
		showOptions(wp, vlan_name, finalbuffer, "");
		websWrite(wp, "&nbsp;Interface&nbsp;");
		sprintf(vlan_name, "bridgeif%d", i);
		showOptions(wp, vlan_name, bufferif, "");
		websWrite(wp, "&nbsp;Prio&nbsp;");
		sprintf(vlan_name, "bridgeifprio%d", i);
		websWrite(wp, "<input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" />\n", vlan_name, "63");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bridgeif_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"bridgeif_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("bridgesif_count", var);
}

#endif
#if 0
static void showDynOption(webs_t wp, char *propname, char *nvname, char *options[], char *names[])
{
	int i;

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">%s</div><select name=\"%s\">\n", propname, nvname);
	for (i = 0; options[i] != NULL; i++) {
		websWrite(wp, "<option value=\"%s\" %s>Off</option>\n", names[i], nvram_match(nvname, options[i]) ? "selected=\"selected\"" : "");
	}
	websWrite(wp, "</div>\n");

}
#endif

static void show_channel(webs_t wp, char *dev, char *prefix, int type)
{
	char wl_mode[16];

	sprintf(wl_mode, "%s_mode", prefix);
	char wl_net_mode[16];

	sprintf(wl_net_mode, "%s_net_mode", prefix);
	if (nvram_match(wl_net_mode, "disabled"))
		return;
	if (nvram_match(wl_mode, "ap") || nvram_match(wl_mode, "wdsap")
	    || nvram_match(wl_mode, "infra")) {
		char wl_channel[16];

		sprintf(wl_channel, "%s_channel", prefix);
		char wl_wchannel[16];

		sprintf(wl_wchannel, "%s_wchannel", prefix);
		char wl_nbw[16];

		nvram_default_get(wl_wchannel, "0");
		sprintf(wl_nbw, "%s_nbw", prefix);

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label4)</script></div>\n");
#ifdef HAVE_ATH9K
		if (is_ath9k(prefix))
			websWrite(wp, "<select name=\"%s\" rel=\"ath9k\" onfocus=\"check_action(this,0)\" onchange=\"setChannelProperties(this);\"><script type=\"text/javascript\">\n//<![CDATA[\n", wl_channel);
		else
#endif
			websWrite(wp, "<select name=\"%s\" onfocus=\"check_action(this,0)\"><script type=\"text/javascript\">\n//<![CDATA[\n", wl_channel);
#ifdef HAVE_MADWIFI
		struct wifi_channels *chan;
		char cn[128];
		char fr[32];
		int gotchannels = 0;

#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
		if (is_ath11n(prefix)) {
#ifdef HAVE_MADWIFI_MIMO
			if (is_ar5008(prefix)) {
				chan = list_channels_11n(prefix);
				if (chan == NULL)
					chan = list_channels_11n(dev);
				gotchannels = 1;
			}
#endif
#ifdef HAVE_ATH9K
			if (is_ath9k(prefix)) {
				// temp must be replaced with the actual selected country
				char regdomain[16];
				char *country;
				sprintf(regdomain, "%s_regdomain", prefix);
				country = nvram_default_get(regdomain, "UNITED_STATES");
				// temp end
				chan = mac80211_get_channels(prefix, getIsoName(country), 40, 0xff);
				/* if (chan == NULL)
				   chan =
				   list_channels_ath9k(dev, "DE", 40,
				   0xff); */
				gotchannels = 1;
			}
#endif
		}
#endif
		if (!gotchannels) {
			chan = list_channels(prefix);
			if (chan == NULL)
				chan = list_channels(dev);
		}

		if (chan != NULL) {
			// int cnt = getchannelcount ();
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s>\" + share.auto + \"</option>\");\n", nvram_match(wl_channel, "0") ? "selected=\\\"selected\\\"" : "");
			int i = 0;

			while (chan[i].freq != -1) {
#ifdef HAVE_BUFFALO
				if (chan[i].dfs == 1) {
					i++;
					continue;
				}
#endif

				cprintf("%d\n", chan[i].channel);
				cprintf("%d\n", chan[i].freq);

#ifdef HAVE_ATH9K
				if (nvram_match("ath9k_channeldebug", "1")) {
					sprintf(cn, "%d (-%d,+%d,o%d,d%d,m%d,nofdm%d)", chan[i].channel, chan[i].ht40minus, chan[i].ht40plus, chan[i].no_outdoor, chan[i].dfs, chan[i].max_eirp, chan[i].no_ofdm);
				} else
					sprintf(cn, "%d", chan[i].channel);
#else
				sprintf(cn, "%d", chan[i].channel);
#endif
				sprintf(fr, "%d", chan[i].freq);
				int freq = get_wififreq(prefix, chan[i].freq);
				if (freq != -1)
#ifdef HAVE_ATH9K
					if (is_ath9k(prefix)) {
						websWrite(wp,
							  "document.write(\"<option value=\\\"%s\\\" rel=\\\'{\\\"HT40minus\\\":%d,\\\"HT40plus\\\":%d}\\\'%s>%s - %d MHz</option>\");\n",
							  fr, chan[i].ht40minus, chan[i].ht40plus, nvram_match(wl_channel, fr) ? " selected=\\\"selected\\\"" : "", cn, (freq));
					} else
#endif
					{
						websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s>%s - %d MHz</option>\");\n", fr, nvram_match(wl_channel, fr) ? "selected=\\\"selected\\\"" : "", cn, (freq));
					}
				i++;
			}
			free(chan);
		}
#else
		int instance = 0;

		if (!strcmp(prefix, "wl1"))
			instance = 1;
/* 
#if 0 
		if (type == 1 && !nvram_match(wl_net_mode, "g-only")
		    && !nvram_match(wl_net_mode, "a-only")
		    && !nvram_match(wl_net_mode, "na-only")
		    && !nvram_match(wl_net_mode, "bg-mixed")
		    && nvram_match(wl_nbw, "40")) {
			int ch = atoi(nvram_nget("wl%d_wchannel", instance));

			websWrite(wp, "var max_channel = 2;\n");
			websWrite(wp, "var wl%d_channel = '%s';\n", instance,
				  nvram_safe_get(wl_wchannel));
			websWrite(wp,
				  "var freq = new Array(\"Auto\",\"2.412\",\"2.417\",\"2.422\",\"2.427\",\"2.432\",\"2.437\",\"2.442\",\"2.447\",\"2.452\",\"2.457\",\"2.462\",\"2.467\",\"2.472\",\"2.484\");\n");
			char *sel = "";

			if (ch == 0)	//wchannel = Auto
			{
				websWrite(wp,
					  " document.write(\"<option value=\\\"0\\\" selected=\\\"selected\\\">\"+ share.auto +\"</option>\");\n");
			} else {
				if (nvram_nmatch
				    ("lower", "wl%d_nctrlsb", instance))
					sel = "selected=\\\"selected\\\"";

				websWrite(wp,
					  " document.write(\"<option value=\\\"%d\\\" %s>%d - \"+freq[%d]+\" GHz</option>\");\n",
					  ch - 2, sel, ch - 2, ch - 2);
				sel = "";
				if (nvram_nmatch
				    ("upper", "wl%d_nctrlsb", instance))
					sel = "selected=\\\"selected\\\"";
				websWrite(wp,
					  " document.write(\"<option value=\\\"%d\\\" %s>%d - \"+freq[%d]+\" GHz</option>\");\n",
					  ch + 2, sel, ch + 2, ch + 2);
			}

		} else
#endif 
*/
		{

			unsigned int chanlist[128];
			char *ifn = get_wl_instance_name(instance);
			int chancount = getchannels(chanlist, ifn);
			int net_is_a = 0;

			if (chanlist[0] > 25)
				net_is_a = 1;

//          websWrite( wp, "var max_channel = %d;\n", chancount );
//          websWrite( wp, "var wl%d_channel = '%s';\n", instance, nvram_safe_get( wl_channel ) );
//          websWrite( wp, "var offset = %d;\n", chanlist[0] );
//          websWrite( wp, "var buf = \"\";\n" );
//      websWrite( wp, "var freq = new Array(\"Auto\"" );
			int i, j;

			// supported 5GHz channels for IEEE 802.11n 40MHz
			int na_upper[16] = { 40, 48, 56, 64, 104, 112, 120, 128, 136, 153, 161,
				0, 0, 0, 0, 0
			};
			int na_lower[16] = { 36, 44, 52, 60, 100, 108, 116, 124, 132, 149, 157,
				0, 0, 0, 0, 0
			};

			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s>\" + share.auto + \"</option>\");\n", nvram_nmatch("0", "%s_channel", prefix) ? "selected=\\\"selected\\\"" : "");
			for (i = 0; i < chancount; i++) {
				float ofs;

				if (chanlist[i] < 25)
					ofs = 2.407f;
				else
					ofs = 5.000f;
				ofs += (float)(chanlist[i] * 0.005f);
				if (ofs == 2.477f)
					ofs = 2.484f;	// fix: ch 14 is 2.484, not 2.477 GHz
//              websWrite( wp, ", \"%0.3f\"", ofs );
				char channelstring[32];

				int showit = 1;

				if (nvram_match(wl_net_mode, "a-only")
				    || nvram_match(wl_net_mode, "na-only")
				    || nvram_match(wl_net_mode, "n5-only")
				    || nvram_match(wl_net_mode, "ac-only")
				    || (net_is_a && nvram_match(wl_net_mode, "mixed"))) {
					if (chanlist[i] < 25)
						showit = 0;
				} else {
					if (chanlist[i] > 25)
						showit = 0;
				}

				if ((nvram_match(wl_net_mode, "na-only") || nvram_match(wl_net_mode, "ac-only")

				     || (net_is_a && nvram_match(wl_net_mode, "mixed"))
				     || nvram_match(wl_net_mode, "n5-only"))
				    && nvram_match(wl_nbw, "40")) {
					showit = 0;
					j = 0;
					if (nvram_nmatch("upper", "%s_nctrlsb", prefix)) {
						while (na_upper[j]) {
							if (chanlist[i] == na_upper[j]) {
								showit = 1;
								break;
							}
							j++;
						}
					} else if (nvram_nmatch("lower", "%s_nctrlsb", prefix)) {
						while (na_lower[j]) {
							if (chanlist[i] == na_lower[j]) {
								showit = 1;
								break;
							}
							j++;
						}
					}
				}

				if ((nvram_match(wl_net_mode, "n-only")
				     || nvram_match(wl_net_mode, "n2-only")
				     || nvram_match(wl_net_mode, "ng-only")
				     || (!net_is_a && nvram_match(wl_net_mode, "mixed")))
				    && nvram_match(wl_nbw, "40")) {
					showit = 0;
					if (nvram_nmatch("upper", "%s_nctrlsb", prefix)) {
						if (chanlist[i] >= 5 && chanlist[i] <= 13) {
							showit = 1;
						}
					} else if (nvram_nmatch("lower", "%s_nctrlsb", prefix)) {
						if (chanlist[i] <= 9) {
							showit = 1;
						}
					}
				}

				sprintf(channelstring, "%d", chanlist[i]);
				if (showit) {
					websWrite(wp,
						  "document.write(\"<option value=\\\"%d\\\" %s>%d - %0.3f GHz</option>\");\n",
						  chanlist[i], nvram_nmatch(channelstring, "%s_channel", prefix) ? "selected=\\\"selected\\\"" : "", chanlist[i], ofs);
				}
			}
//          websWrite( wp, ");\n" );
//          websWrite( wp, "for(i=0; i<=max_channel ; i++) {\n" );
//          websWrite( wp, "    if(i == wl%d_channel) buf = \"selected\";\n",
//                     instance );
//          websWrite( wp, "    else buf = \"\";\n" );
//          websWrite( wp, "    if (i==0)\n" );
//          websWrite( wp,
//                     "                document.write(\"<option value=\"+i+\" \"+buf+\">\" + share.auto + \"</option>\");\n" );
//          websWrite( wp, "    else\n" );
//          websWrite( wp,
//                     "                document.write(\"<option value=\"+i+\" \"+buf+\">\"+(i+offset-1)+\" - \"+freq[i]+\" GHz</option>\");\n" );
//          websWrite( wp, "}\n" );
		}
#endif
		websWrite(wp, "//]]>\n</script></select></div>\n");
	}
}

#ifdef HAVE_MADWIFI
static char *ag_rates[] = { "6", "9", "12", "18", "24", "36", "48", "54" };
static char *turbo_rates[] = { "12", "18", "24", "36", "48", "72", "96", "108" };
static char *b_rates[] = { "1", "2", "5.5", "11" };
static char *bg_rates[] = { "1", "2", "5.5", "6", "9", "11", "12", "18", "24", "36", "48", "54" };

// static char *g_rates[] = { "1", "2", "5.5", "11", "12", "18", "24", "36",
// "48", "54" };
//static char *xr_rates[] =
//    { "0.25", "0.5", "1", "2", "3", "6", "9", "12", "18", "24", "36", "48",
//    "54"
//};
static char *half_rates[] = { "3", "4.5", "6", "9", "12", "18", "24", "27" };
static char *quarter_rates[] = { "1.5", "2", "3", "4.5", "6", "9", "12", "13.5" };
static char *subquarter_rates[] = { "0.75", "1", "1.5", "2.25", "3", "4.5", "6", "6.75" };

void show_rates(webs_t wp, char *prefix, int maxrate)
{
	websWrite(wp, "<div class=\"setting\">\n");
	if (maxrate) {
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label21)</script></div>\n");
		websWrite(wp, "<select name=\"%s_maxrate\">\n", prefix);
	} else {
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label23)</script></div>\n");
		websWrite(wp, "<select name=\"%s_minrate\">\n", prefix);
	}
	websWrite(wp, "<script type=\"text/javascript\">\n");
	websWrite(wp, "//<![CDATA[\n");
	char srate[32];

	sprintf(srate, "%s_minrate", prefix);
	char mxrate[32];

	sprintf(mxrate, "%s_maxrate", prefix);
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + share.auto + \"</option>\");\n", nvram_match(srate, "0") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n");
	websWrite(wp, "</script>\n");
	char **rate;
	char **showrates = NULL;
	int len;
	char mode[32];
	char bw[16];

	sprintf(bw, "%s_channelbw", prefix);

	sprintf(mode, "%s_net_mode", prefix);
	if (nvram_match(mode, "b-only")) {
		rate = b_rates;
		len = sizeof(b_rates) / sizeof(char *);
	}
	if (nvram_match(mode, "g-only")) {
		rate = ag_rates;
		len = sizeof(ag_rates) / sizeof(char *);
		if (nvram_match(bw, "40")) {
			showrates = turbo_rates;
		}
		if (nvram_match(bw, "10")) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(char *);
		}
		if (nvram_match(bw, "5")) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(char *);
		}
		if (nvram_match(bw, "2")) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(char *);
		}
	}
	if (nvram_match(mode, "a-only")) {
		rate = ag_rates;
		len = sizeof(ag_rates) / sizeof(char *);
		if (nvram_match(bw, "40")) {
			showrates = turbo_rates;
		}
		if (nvram_match(bw, "10")) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(char *);
		}
		if (nvram_match(bw, "5")) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(char *);
		}
		if (nvram_match(bw, "2")) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(char *);
		}
	}
	if (nvram_match(mode, "bg-mixed")) {
		rate = bg_rates;
		len = sizeof(bg_rates) / sizeof(char *);
		if (nvram_match(bw, "10")) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(char *);
		}
		if (nvram_match(bw, "5")) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(char *);
		}
		if (nvram_match(bw, "2")) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(char *);
		}
	}
	if (nvram_match(mode, "mixed")) {
		rate = bg_rates;
		len = sizeof(bg_rates) / sizeof(char *);
		if (nvram_match(bw, "40")) {
			rate = ag_rates;
			len = sizeof(ag_rates) / sizeof(char *);
			showrates = turbo_rates;
		}
		if (nvram_match(bw, "10")) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(char *);
		}
		if (nvram_match(bw, "5")) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(char *);
		}
		if (nvram_match(bw, "2")) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(char *);
		}
	}
	int i;

	for (i = 0; i < len; i++) {
		if (maxrate) {
			int offset = 0;

			if (nvram_match(mode, "g-only")
			    && nvram_match(bw, "20"))
				offset = 4;
			char comp[32];

			sprintf(comp, "%d", i + 1 + offset);
			if (showrates)
				websWrite(wp, "<option value=\"%d\" %s >%s Mbps</option>\n", i + 1 + offset, nvram_match(mxrate, comp) ? "selected=\"selected\"" : "", showrates[i]);
			else
				websWrite(wp, "<option value=\"%d\" %s >%s Mbps</option>\n", i + 1 + offset, nvram_match(mxrate, comp) ? "selected=\"selected\"" : "", rate[i]);
		} else {
			int offset = 0;

			if (nvram_match(mode, "g-only")
			    && nvram_match(bw, "20"))
				offset = 4;
			char comp[32];

			sprintf(comp, "%d", i + 1 + offset);
			if (showrates)
				websWrite(wp, "<option value=\"%d\" %s >%s Mbps</option>\n", i + 1 + offset, nvram_match(srate, comp) ? "selected=\"selected\"" : "", showrates[i]);
			else
				websWrite(wp, "<option value=\"%d\" %s >%s Mbps</option>\n", i + 1 + offset, nvram_match(srate, comp) ? "selected=\"selected\"" : "", rate[i]);

		}
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "<span class=\"default\">\n");
	websWrite(wp, "<script type=\"text/javascript\">\n");
	websWrite(wp, "//<![CDATA[\n");
	websWrite(wp, "document.write(\"(\" + share.deflt + \": \" + share.auto + \")\");\n");
	websWrite(wp, "//]]\n");
	websWrite(wp, "</script></span></div>\n");

}
#endif
static void show_netmode(webs_t wp, char *prefix)
{
	char wl_net_mode[16];

	sprintf(wl_net_mode, "%s_net_mode", prefix);

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label2)</script></div><select name=\"%s\">\n", wl_net_mode);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"disabled\\\" %s>\" + share.disabled + \"</option>\");\n", nvram_match(wl_net_mode, "disabled") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"mixed\\\" %s>\" + wl_basic.mixed + \"</option>\");\n", nvram_match(wl_net_mode, "mixed") ? "selected=\\\"selected\\\"" : "");

	if (has_mimo(prefix) && has_2ghz(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"bg-mixed\\\" %s>\" + wl_basic.bg + \"</option>\");\n", nvram_match(wl_net_mode, "bg-mixed") ? "selected=\\\"selected\\\"" : "");
	}
#ifdef HAVE_WHRAG108
	if (!strcmp(prefix, "ath1"))
#endif
#ifdef HAVE_TW6600
		if (!strcmp(prefix, "ath1"))
#endif

			if (has_2ghz(prefix)) {
				websWrite(wp, "document.write(\"<option value=\\\"b-only\\\" %s>\" + wl_basic.b + \"</option>\");\n", nvram_match(wl_net_mode, "b-only") ? "selected=\\\"selected\\\"" : "");
			}
#ifdef HAVE_MADWIFI
	if (has_2ghz(prefix)) {

#ifdef HAVE_WHRAG108
		if (!strcmp(prefix, "ath1"))
#endif
#ifdef HAVE_TW6600
			if (!strcmp(prefix, "ath1"))
#endif
				websWrite(wp, "document.write(\"<option value=\\\"g-only\\\" %s>\" + wl_basic.g + \"</option>\");\n", nvram_match(wl_net_mode, "g-only") ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_WHRAG108
		if (!strcmp(prefix, "ath1"))
#endif
#ifdef HAVE_TW6600
			if (!strcmp(prefix, "ath1"))
#endif
#if !defined(HAVE_LS5) || defined(HAVE_EOC5610)
				websWrite(wp, "document.write(\"<option value=\\\"bg-mixed\\\" %s>\" + wl_basic.bg + \"</option>\");\n", nvram_match(wl_net_mode, "bg-mixed") ? "selected=\\\"selected\\\"" : "");
#endif
	}
#else
#ifdef HAVE_WHRAG108
	if (!strcmp(prefix, "ath1"))
#endif
#if !defined(HAVE_LS5) || defined(HAVE_EOC5610)

		if (has_2ghz(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"g-only\\\" %s>\" + wl_basic.g + \"</option>\");\n", nvram_match(wl_net_mode, "g-only") ? "selected=\\\"selected\\\"" : "");
		}
	if (has_mimo(prefix) && has_2ghz(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"ng-only\\\" %s>\" + wl_basic.ng + \"</option>\");\n", nvram_match(wl_net_mode, "ng-only") ? "selected=\\\"selected\\\"" : "");
	}
#endif
#endif
	if (has_mimo(prefix) && has_2ghz(prefix)) {
		if (has_5ghz(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"n2-only\\\" %s>\" + wl_basic.n2 + \"</option>\");\n", nvram_match(wl_net_mode, "n2-only") ? "selected=\\\"selected\\\"" : "");
		} else {
			websWrite(wp, "document.write(\"<option value=\\\"n-only\\\" %s>\" + wl_basic.n + \"</option>\");\n", nvram_match(wl_net_mode, "n-only") ? "selected=\\\"selected\\\"" : "");
		}
	}
#if !defined(HAVE_FONERA) && !defined(HAVE_LS2) && !defined(HAVE_MERAKI)
#ifndef HAVE_MADWIFI

	if (has_5ghz(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"a-only\\\" %s>\" + wl_basic.a + \"</option>\");\n", nvram_match(wl_net_mode, "a-only") ? "selected=\\\"selected\\\"" : "");
	}
	if (has_mimo(prefix) && has_5ghz(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"na-only\\\" %s>\" + wl_basic.na + \"</option>\");\n", nvram_match(wl_net_mode, "na-only") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"n5-only\\\" %s>\" + wl_basic.n5 + \"</option>\");\n", nvram_match(wl_net_mode, "n5-only") ? "selected=\\\"selected\\\"" : "");
	}
	if (has_ac(prefix) && has_5ghz(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"ac-only\\\" %s>\" + wl_basic.ac + \"</option>\");\n", nvram_match(wl_net_mode, "ac-only") ? "selected=\\\"selected\\\"" : "");
	}
#else
#if HAVE_WHRAG108
	if (!strcmp(prefix, "ath0"))
#endif
#ifdef HAVE_TW6600
		if (!strcmp(prefix, "ath0"))
#endif
			if (has_5ghz(prefix)) {
				websWrite(wp, "document.write(\"<option value=\\\"a-only\\\" %s>\" + wl_basic.a + \"</option>\");\n", nvram_match(wl_net_mode, "a-only") ? "selected=\\\"selected\\\"" : "");
			}
#endif

#endif
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
	if (is_ath11n(prefix)) {
		if (has_2ghz(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"ng-only\\\" %s>\" + wl_basic.ng + \"</option>\");\n", nvram_match(wl_net_mode, "ng-only") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"n2-only\\\" %s>\" + wl_basic.n2 + \"</option>\");\n", nvram_match(wl_net_mode, "n2-only") ? "selected=\\\"selected\\\"" : "");
		}
		if (has_5ghz(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"na-only\\\" %s>\" + wl_basic.na + \"</option>\");\n", nvram_match(wl_net_mode, "na-only") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"n5-only\\\" %s>\" + wl_basic.n5 + \"</option>\");\n", nvram_match(wl_net_mode, "n5-only") ? "selected=\\\"selected\\\"" : "");
		}
	}
#endif

	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

#ifdef HAVE_RT2880
	if (nvram_nmatch("n-only", "%s_net_mode", prefix)) {
		char wl_greenfield[32];

		sprintf(wl_greenfield, "%s_greenfield", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label7)</script></div><select name=\"%s\" >\n", wl_greenfield);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s>\" + wl_basic.mixed + \"</option>\");\n", nvram_default_match(wl_greenfield, "0", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s>\" + wl_basic.greenfield + \"</option>\");\n", nvram_default_match(wl_greenfield, "1", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}
#endif
}

#ifdef HAVE_MADWIFI
static void showrtssettings(webs_t wp, char *var)
{
	char ssid[32];
	char vvar[32];

	strcpy(vvar, var);
	rep(vvar, '.', 'X');
	sprintf(ssid, "%s_rts", var);
	websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.rts)</script></div>\n");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idrts', true);\" name=\"%s_rts\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>\n",
		  vvar, var, nvram_default_match(ssid, "1", "0") ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idrts', false);\" name=\"%s_rts\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
		  vvar, var, nvram_default_match(ssid, "0", "0") ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div id=\"%s_idrts\">\n", vvar);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.rtsvalue)</script></div>\n");
	char ip[32];

	sprintf(ip, "%s_rtsvalue", var);
	websWrite(wp, "<input class=\"num\" maxlength=\"4\" size=\"4\" onblur=\"valid_range(this,1,2346,share.ip)\" name=\"%s_rtsvalue\" value=\"%s\" />", var, nvram_default_get(ip, "2346"));
	websWrite(wp, "</div>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<script>\n//<![CDATA[\n ");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_rts\"), \"%s_idrts\", %s);\n", var, vvar, nvram_match(ssid, "1") ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");

}
#endif
static void showbridgesettings(webs_t wp, char *var, int mcast, int dual)
{

	char ssid[32];

	sprintf(ssid, "%s_bridged", var);
	char vvar[32];

	strcpy(vvar, var);
	rep(vvar, '.', 'X');
	websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.network)</script></div>\n");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idnetvifs', true);\" name=\"%s_bridged\" %s><script type=\"text/javascript\">Capture(wl_basic.unbridged)</script></input>&nbsp;\n",
		  vvar, var, nvram_default_match(ssid, "0", "1") ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idnetvifs', false);\" name=\"%s_bridged\" %s><script type=\"text/javascript\">Capture(wl_basic.bridged)</script></input>\n",
		  vvar, var, nvram_default_match(ssid, "1", "1") ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div id=\"%s_idnetvifs\">\n", vvar);
	if (mcast) {
		char mcastvar[32];

		sprintf(mcastvar, "%s_multicast", var);
		nvram_default_get(mcastvar, "0");
		showRadio(wp, "wl_basic.multicast", mcastvar);
	}
	if (has_gateway()) {
		char natvar[32];
		sprintf(natvar, "%s_nat", var);
		nvram_default_get(natvar, "1");
		showRadio(wp, "wl_basic.masquerade", natvar);
	}
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n");
	char ip[32];

	sprintf(ip, "%s_ipaddr", var);
	char *ipv = nvram_safe_get(ip);

	websWrite(wp, "<input type=\"hidden\" name=\"%s_ipaddr\" value=\"4\" />\n", var);
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,1,223,share.ip)\" name=\"%s_ipaddr_0\" value=\"%d\" />.", var, get_single_ip(ipv, 0));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_1\" value=\"%d\" />.", var, get_single_ip(ipv, 1));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_2\" value=\"%d\" />.", var, get_single_ip(ipv, 2));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_3\" value=\"%d\" />\n", var, get_single_ip(ipv, 3));
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n");
	sprintf(ip, "%s_netmask", var);
	ipv = nvram_safe_get(ip);

	websWrite(wp, "<input type=\"hidden\" name=\"%s_netmask\" value=\"4\" />\n", var);
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_0\" value=\"%d\" />.", var, get_single_ip(ipv, 0));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_1\" value=\"%d\" />.", var, get_single_ip(ipv, 1));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_2\" value=\"%d\" />.", var, get_single_ip(ipv, 2));
	websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_3\" value=\"%d\" />.", var, get_single_ip(ipv, 3));
	websWrite(wp, "</div>\n");

#ifdef HAVE_MADWIFI
/*if (dual)
{
    char dl[32];
    sprintf(dl,"%s_duallink",var);
    websWrite( wp,
	       "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.duallink)</script></div>\n" );
    websWrite( wp,
	       "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idduallink', true);\" name=\"%s_duallink\" %s><script type=\"text/javascript\">Capture(shared.enable)</script></input>&nbsp;\n",
	       var, var, nvram_default_match( dl, "1",
					       "0" ) ? "checked=\"checked\"" :
	       "" );
    websWrite( wp,
	       "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idduallink', false);\" name=\"%s_duallink\" %s><script type=\"text/javascript\">Capture(shared.disable)</script></input>\n",
	       var, var, nvram_default_match( dl, "0",
					       "0" ) ? "checked=\"checked\"" :
	       "" );
    websWrite( wp, "</div>\n" );

    websWrite( wp, "<div id=\"%s_iddualink\">\n", var );

    sprintf( ip, "%s_duallink_parent", var );
    websWrite( wp, "<div class=\"setting\">\n" );
    websWrite( wp,"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.parent)</script></div>\n" );
    ipv = nvram_default_get( ip,"0.0.0.0" );
    websWrite( wp,
	       "<input type=\"hidden\" name=\"%s_duallink_parent\" value=\"4\" />\n",
	       var );
    websWrite( wp,
	       "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_duallink_parent_0\" value=\"%d\" />.",
	       var, get_single_ip( ipv, 0 ) );
    websWrite( wp,
	       "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_duallink_parent_1\" value=\"%d\" />.",
	       var, get_single_ip( ipv, 1 ) );
    websWrite( wp,
	       "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_duallink_parent_2\" value=\"%d\" />.",
	       var, get_single_ip( ipv, 2 ) );
    websWrite( wp,
	       "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_duallink_parent_3\" value=\"%d\" />.",
	       var, get_single_ip( ipv, 3 ) );
    websWrite( wp, "</div>\n" );

    websWrite( wp, "</div>\n" );

    websWrite( wp, "<script>\n//<![CDATA[\n " );
    websWrite( wp,
	       "show_layer_ext(document.getElementsByName(\"%s_duallink\"), \"%s_idduallink\", %s);\n",
	       var, vvar, nvram_match( dl, "1" ) ? "true" : "false" );
    websWrite( wp, "//]]>\n</script>\n" );
}*/
#endif

	websWrite(wp, "</div>\n");

	websWrite(wp, "<script>\n//<![CDATA[\n ");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_bridged\"), \"%s_idnetvifs\", %s);\n", var, vvar, nvram_match(ssid, "0") ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");

}

#ifdef HAVE_MADWIFI
static void show_chanshift(webs_t wp, char *prefix)
{
	char wl_chanshift[32];
	char wl_channelbw[32];

	sprintf(wl_channelbw, "%s_channelbw", prefix);
	sprintf(wl_chanshift, "%s_chanshift", prefix);
	if (atoi(nvram_safe_get(wl_channelbw)) > 2 && (atoi(nvram_safe_get(wl_chanshift)) & 0xf) > 10)
		nvram_set(wl_chanshift, "10");
	if (atoi(nvram_safe_get(wl_channelbw)) > 5 && (atoi(nvram_safe_get(wl_chanshift)) & 0xf) > 10)
		nvram_set(wl_chanshift, "10");
	if (atoi(nvram_safe_get(wl_channelbw)) > 10 && (atoi(nvram_safe_get(wl_chanshift)) & 0xf) > 0)
		nvram_set(wl_chanshift, "0");

	if (nvram_match(wl_channelbw, "5")
	    || nvram_match(wl_channelbw, "10")
	    || nvram_match(wl_channelbw, "2")) {

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.chanshift)</script></div>\n<select name=\"%s\">\n", wl_chanshift);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		if (nvram_match(wl_channelbw, "5")
		    || nvram_match(wl_channelbw, "2"))
			websWrite(wp, "document.write(\"<option value=\\\"-15\\\" %s >-15 MHz</option>\");\n", nvram_default_match(wl_chanshift, "-15", "0") ? "selected=\\\"selected\\\"" : "");
		if (nvram_match(wl_channelbw, "5")
		    || nvram_match(wl_channelbw, "10")
		    || nvram_match(wl_channelbw, "2"))
			websWrite(wp, "document.write(\"<option value=\\\"-10\\\" %s >-10 MHz</option>\");\n", nvram_default_match(wl_chanshift, "-10", "0") ? "selected=\\\"selected\\\"" : "");
		if (nvram_match(wl_channelbw, "5")
		    || nvram_match(wl_channelbw, "10")
		    || nvram_match(wl_channelbw, "2"))
			websWrite(wp, "document.write(\"<option value=\\\"-5\\\" %s >-5 MHz</option>\");\n", nvram_default_match(wl_chanshift, "-5", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >0 MHz</option>\");\n", nvram_default_match(wl_chanshift, "0", "0") ? "selected=\\\"selected\\\"" : "");
		if (nvram_match(wl_channelbw, "5")
		    || nvram_match(wl_channelbw, "10")
		    || nvram_match(wl_channelbw, "2"))
			websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >+5 MHz</option>\");\n", nvram_default_match(wl_chanshift, "5", "0") ? "selected=\\\"selected\\\"" : "");
		if (nvram_match(wl_channelbw, "5")
		    || nvram_match(wl_channelbw, "10")
		    || nvram_match(wl_channelbw, "2"))
			websWrite(wp, "document.write(\"<option value=\\\"10\\\" %s >+10 MHz</option>\");\n", nvram_default_match(wl_chanshift, "10", "0") ? "selected=\\\"selected\\\"" : "");
		if (nvram_match(wl_channelbw, "5")
		    || nvram_match(wl_channelbw, "2"))
			websWrite(wp, "document.write(\"<option value=\\\"15\\\" %s >+15 MHz</option>\");\n", nvram_default_match(wl_chanshift, "15", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
	}
}
#endif
static int show_virtualssid(webs_t wp, char *prefix)
{
	char *next;
	char var[80];
	char ssid[80];
	char vif[16];
	char power[32];
#ifdef HAVE_GUESTPORT
	char guestport[16];
	sprintf(guestport, "guestport_%s", prefix);
#endif

#ifdef HAVE_MADWIFI
	char wmm[32];
	char wl_protmode[32];
#endif
	sprintf(vif, "%s_vifs", prefix);
	char *vifs = nvram_safe_get(vif);

	if (vifs == NULL)
		return 0;
#ifndef HAVE_MADWIFI
	if (!nvram_nmatch("ap", "%s_mode", prefix)
	    && !nvram_nmatch("apsta", "%s_mode", prefix)
	    && !nvram_nmatch("apstawet", "%s_mode", prefix))
		return 0;
#endif
	int count = 1;

	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(wl_basic.h2_vi)</script></h2>\n");
	foreach(var, vifs, next) {
#ifdef HAVE_GUESTPORT
		if (nvram_match(guestport, var)) {
			count++;
			continue;
		}
#endif
		sprintf(ssid, "%s_ssid", var);
		websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(share.vintrface)</script> %s SSID [", IFMAP(var));
		tf_webWriteESCNV(wp, ssid);	// fix for broken html page if ssid
		// contains html tag
		char wl_macaddr[16];
		sprintf(wl_macaddr, "%s_hwaddr", var);
		if (nvram_get(wl_macaddr))
			websWrite(wp, "] HWAddr [%s", nvram_safe_get(wl_macaddr));
		websWrite(wp, "]</legend>\n");
		websWrite(wp, "<div class=\"setting\">\n");
#if !defined(HAVE_EASY_WIRELESS_CONFIG) || defined(HAVE_BCMMODERN)
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label3)</script></div>\n");

		websWrite(wp, "<input name=\"%s_ssid\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.label3)\" value=\"%s\" /></div>\n", var, nvram_safe_get(ssid));

#ifdef HAVE_MADWIFI
//      sprintf( wl_chanshift, "%s_chanshift", var );
//      show_chanshift( wp, wl_chanshift );

		sprintf(wl_protmode, "%s_protmode", var);
		showOptionsLabel(wp, "wl_basic.protmode", wl_protmode, "None CTS RTS/CTS", nvram_default_get(wl_protmode, "None"));
		showrtssettings(wp, var);
#endif

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label5)</script></div>");
		sprintf(ssid, "%s_closed", var);
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			  var, nvram_match(ssid, "0") ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			  var, nvram_match(ssid, "1") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		char wl_mode[16];

#ifdef HAVE_MADWIFI
		sprintf(wl_mode, "%s_mode", var);
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n", wl_mode);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");\n", nvram_match(wl_mode, "ap") ? "selected=\\\"selected\\\"" : "");
		// websWrite (wp,
		// "document.write(\"<option value=\\\"wdssta\\\" %s >\" +
		// wl_basic.wdssta + \"</option>\");\n",
		// nvram_match (wl_mode,
		// "wdssta") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");\n", nvram_match(wl_mode, "wdsap") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
//              sprintf(wmm, "%s_wmm", var);
//              showRadio(wp, "wl_adv.label18", wmm);
#endif

#else				// start EASY_WIRELESS_SETUP

// wireless mode
		char wl_mode[16];

#ifdef HAVE_MADWIFI
		sprintf(wl_mode, "%s_mode", var);
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n", wl_mode);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");\n", nvram_match(wl_mode, "ap") ? "selected=\\\"selected\\\"" : "");
		// websWrite (wp,
		// "document.write(\"<option value=\\\"wdssta\\\" %s >\" +
		// wl_basic.wdssta + \"</option>\");\n",
		// nvram_match (wl_mode,
		// "wdssta") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");\n", nvram_match(wl_mode, "wdsap") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label3)</script></div>\n");

		websWrite(wp, "<input name=\"%s_ssid\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.label3)\" value=\"%s\" /></div>\n", var, nvram_safe_get(ssid));
// broadcast wireless ssid
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label5)</script></div>");
		sprintf(ssid, "%s_closed", var);
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			  var, nvram_match(ssid, "0") ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			  var, nvram_match(ssid, "1") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
#endif

#ifdef HAVE_IFL
// label
		char wl_label[16];
		sprintf(wl_label, "%s_label", var);
		websWrite(wp,
			  "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.if_label)</script></div><input type=\"text\" name=\"%s\" value=\"%s\" maxlength=\"20\"></div>\n",
			  wl_label, nvram_safe_get(wl_label));

#endif

// WIRELESS Advanced
		char advanced_label[32];
		char maskvar[32];
		strcpy(maskvar, var);
		rep(maskvar, '.', 'X');
		sprintf(advanced_label, "%s_wl_advanced", maskvar);
		websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.legend)</script></div>\n");
		websWrite(wp, "<input type=\"checkbox\" name=\"%s\" onclick=\"toggle_layer(this,'%s_layer')\"%s>", advanced_label, advanced_label, websGetVar(wp, advanced_label, NULL) ? " checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div id=\"%s_layer\"%s>\n", advanced_label, websGetVar(wp, advanced_label, NULL) ? "" : " style=\"display: none;\"");

#ifdef HAVE_IFL

		char wl_note[16];
		sprintf(wl_note, "%s_note", var);
		websWrite(wp,
			  "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.if_info)</script></div><textarea name=\"%s\" cols=\"60\" rows=\"3\">%s</textarea></div>\n",
			  wl_note, nvram_safe_get(wl_note));
#endif

#ifdef HAVE_MADWIFI
//      sprintf( wl_chanshift, "%s_chanshift", var );
//      show_chanshift( wp, wl_chanshift );

		sprintf(wl_protmode, "%s_protmode", var);
		showOptionsLabel(wp, "wl_basic.protmode", wl_protmode, "None CTS RTS/CTS", nvram_default_get(wl_protmode, "None"));
		showrtssettings(wp, var);

#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
		if (!is_ath11n(prefix))
#endif
		{
			sprintf(wmm, "%s_wmm", var);
			showRadio(wp, "wl_adv.label18", wmm);
		}
#endif

#endif				// end BUFFALO

		sprintf(ssid, "%s_ap_isolate", var);
		showRadio(wp, "wl_adv.label11", ssid);
#ifdef HAVE_MADWIFI

		if (nvram_nmatch("ap", "%s_mode", var)
		    || nvram_nmatch("wdsap", "%s_mode", var)
		    || nvram_nmatch("infra", "%s_mode", var)) {
			sprintf(power, "%s_maxassoc", var);
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label10)</script></div>\n");
			websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,256,wl_adv.label10)\" value=\"%s\" />\n", power, nvram_default_get(power, "256"));

			websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 256 \" + share.user + \")\");\n//]]>\n</script></span>\n");
			websWrite(wp, "</div>\n");
		}

		sprintf(power, "%s_mtikie", var);
		nvram_default_get(power, "0");
		showRadio(wp, "wl_basic.mtikie", power);
#endif
#ifdef HAVE_RT2880
		showbridgesettings(wp, getRADev(var), 1, 0);
#else
		showbridgesettings(wp, var, 1, 0);
#endif
#if defined(HAVE_EASY_WIRELESS_CONFIG) && !defined(HAVE_BCMMODERN)
		websWrite(wp, "</div>\n");
#endif
		websWrite(wp, "</fieldset><br />\n");
		count++;
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
		if (count == 4 && isap8x()) {
			websWrite(wp, "<div class=\"warning\">\n");
			websWrite(wp, "  <p><script type=\"text/javascript\">Capture(wl_basic.ap83_vap_note)</script></p>\n");
			websWrite(wp, "</div>\n<br>\n");
		}
#endif
	}
	websWrite(wp, "<div class=\"center\">\n");
#ifdef HAVE_MADWIFI
	if (count < 8)
#elif HAVE_RT2880
	if (count < 7)
#else
	if (count < WL_MAXBSSCFG)
#endif
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"vifs_add_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
			  prefix);

	if (count > 1)
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.remove + \"\\\" onclick=\\\"vifs_remove_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
			  prefix);

	websWrite(wp, "</div><br />\n");

#ifdef HAVE_GUESTPORT
	int gpfound = 0;
	websWrite(wp, "<h2>Guestport</h2>\n");
	foreach(var, vifs, next) {
		if (nvram_match(guestport, var)) {
			gpfound = 1;

			sprintf(ssid, "%s_ssid", var);
			websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(share.vintrface)</script> %s SSID [", IFMAP(var));
			tf_webWriteESCNV(wp, ssid);	// fix for broken html page if ssid
			// contains html tag
			char wl_macaddr[16];
			sprintf(wl_macaddr, "%s_hwaddr", var);
			if (nvram_get(wl_macaddr))
				websWrite(wp, "] HWAddr [%s", nvram_safe_get(wl_macaddr));
			websWrite(wp, "]</legend>\n");

			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label3)</script></div>\n");
			//fprintf(stderr, "[GUESTPORT] %s\n", ssid);
			//fprintf(stderr, "[GUESTPORT] %s\n",nvram_safe_get(ssid));
			websWrite(wp, "<input name=\"%s_ssid\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.label3)\" value=\"%s\" /></div>\n", var, nvram_safe_get(ssid));

			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label5)</script></div>");
			sprintf(ssid, "%s_closed", var);
			websWrite(wp,
				  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
				  var, nvram_match(ssid, "0") ? "checked=\"checked\"" : "");
			websWrite(wp,
				  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
				  var, nvram_match(ssid, "1") ? "checked=\"checked\"" : "");
			websWrite(wp, "</div>\n");

			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n");
			char ip[32];

			sprintf(ip, "%s_ipaddr", var);
			char *ipv = nvram_safe_get(ip);

			websWrite(wp, "<input type=\"hidden\" name=\"%s_ipaddr\" value=\"4\" />\n", var);
			websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,1,223,share.ip)\" name=\"%s_ipaddr_0\" value=\"%d\" />.", var, get_single_ip(ipv, 0));
			websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_1\" value=\"%d\" />.", var, get_single_ip(ipv, 1));
			websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_2\" value=\"%d\" />.", var, get_single_ip(ipv, 2));
			websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_3\" value=\"%d\" />\n", var, get_single_ip(ipv, 3));
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n");
			sprintf(ip, "%s_netmask", var);
			ipv = nvram_safe_get(ip);

			websWrite(wp, "<input type=\"hidden\" name=\"%s_netmask\" value=\"4\" />\n", var);
			websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_0\" value=\"%d\" />.", var, get_single_ip(ipv, 0));
			websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_1\" value=\"%d\" />.", var, get_single_ip(ipv, 1));
			websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_2\" value=\"%d\" />.", var, get_single_ip(ipv, 2));
			websWrite(wp, "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_3\" value=\"%d\" />", var, get_single_ip(ipv, 3));
			websWrite(wp, "</div>\n");

			sprintf(ssid, "%s_ap_isolate", var);
			showRadio(wp, "wl_adv.label11", ssid);
#ifdef HAVE_MADWIFI

			if (nvram_nmatch("ap", "%s_mode", var)
			    || nvram_nmatch("wdsap", "%s_mode", var)
			    || nvram_nmatch("infra", "%s_mode", var)) {
				sprintf(power, "%s_maxassoc", var);
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label10)</script></div>\n");
				websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,256,wl_adv.label10)\" value=\"%s\" />\n", power, nvram_default_get(power, "256"));

				websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 256 \" + share.user + \")\");\n//]]>\n</script></span>\n");
				websWrite(wp, "</div>\n");
			}
#endif				// GUESTPORT
			websWrite(wp, "</fieldset><br />\n");
		}
	}
	websWrite(wp, "<div class=\"center\">\n");
#ifdef HAVE_MADWIFI
	if (count < 8 && gpfound == 0)
#elif HAVE_RT2880
	if (count < 7 && gpfound == 0)
#else
	if (count < WL_MAXBSSCFG && gpfound == 0)
#endif
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"$('gp_modify').value='add';vifs_add_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
			  prefix);

	if (gpfound == 1)
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.remove + \"\\\" onclick=\\\"$('gp_modify').value='remove';vifs_remove_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
			  prefix);

	websWrite(wp, "</div><br />\n");
#endif
	return 0;
}

void ej_getdefaultindex(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_BUFFALO
	websWrite(wp, "SetupAssistant.asp");
#else
	websWrite(wp, "index.asp");
#endif
}

void ej_showad(webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_FON
#ifndef CONFIG_BRANDING
#ifdef HAVE_CHILLI
	// if (nvram_invmatch ("fon_enable", "1"))
	// websWrite (wp,
	// "<a href=\"fon.cgi\"><img src=\"images/turn.gif\" border=0 /></a>");
#endif
#endif
#endif

#ifndef HAVE_NOAD
	/*
	 * if (nvram_match("wanup","1")) { websWrite(wp,"<script
	 * type=\"text/javascript\"><!--\n//<![CDATA[\n ");
	 * websWrite(wp,"google_ad_client = \"pub-8308593183433068\";\n");
	 * websWrite(wp,"google_ad_width = 728;\n");
	 * websWrite(wp,"google_ad_height = 90;\n");
	 * websWrite(wp,"google_ad_format = \"728x90_as\";\n");
	 * websWrite(wp,"google_ad_type = \"text_image\";\n");
	 * websWrite(wp,"google_ad_channel =\"8866414571\";\n");
	 * websWrite(wp,"google_color_border = \"333333\";\n");
	 * websWrite(wp,"google_color_bg = \"000000\";\n");
	 * websWrite(wp,"google_color_link = \"FFFFFF\";\n");
	 * websWrite(wp,"google_color_url = \"999999\";\n");
	 * websWrite(wp,"google_color_text = \"CCCCCC\";\n");
	 * websWrite(wp,"//-->//]]>\n</script>\n"); websWrite(wp,"<script
	 * type=\"text/javascript\"\n"); websWrite(wp,"
	 * src=\"http://pagead2.googlesyndication.com/pagead/show_ads.js\">\n");
	 * websWrite(wp,"</script>\n"); }
	 */
#endif
	return;
}

#ifndef HAVE_SUPERCHANNEL
int inline issuperchannel(void)
{
#if defined(HAVE_MAKSAT) && defined(HAVE_MR3202A)
	return 0;
#elif defined(HAVE_MAKSAT) && defined(HAVE_ALPHA)
	return 0;
#elif HAVE_MAKSAT
	return 1;
#else
	return 0;
#endif
}
#endif

void ej_show_countrylist(webs_t wp, int argc, char_t ** argv)
{
	if (argc < 1) {
		return;
	}
	char *list = getCountryList();

	showOptionsChoose(wp, argv[0], list, nvram_safe_get(argv[0]));
}

void ej_show_wireless_single(webs_t wp, char *prefix)
{
	char wl_mode[16];
	char wl_macaddr[16];
	char wl_ssid[16];
	char frequencies[32];

	sprintf(wl_mode, "%s_mode", prefix);
	sprintf(wl_macaddr, "%s_hwaddr", prefix);
	sprintf(wl_ssid, "%s_ssid", prefix);
	// check the frequency capabilities;
	if (has_2ghz(prefix) && has_5ghz(prefix) && has_ac(prefix)) {
		sprintf(frequencies, " [2.4 GHz/5 GHz/802.11ac]");
	} else if (has_5ghz(prefix) && has_ac(prefix)) {
		sprintf(frequencies, " [5 GHz/802.11ac]");
	} else if (has_5ghz(prefix) && has_2ghz(prefix)) {
		sprintf(frequencies, " [2.4/5 GHz]");
	} else if (has_5ghz(prefix)) {
		sprintf(frequencies, " [5 GHz]");
	} else if (has_2ghz(prefix)) {
		sprintf(frequencies, " [2.4 GHz]");
	} else {
		frequencies[0] = 0;
	}

	// wireless mode
	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(wl_basic.h2_v24)</script> %s%s</h2>\n", prefix, frequencies);
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.pintrface)</script> %s - SSID [", IFMAP(prefix));
	tf_webWriteESCNV(wp, wl_ssid);	// fix 
	websWrite(wp, "] HWAddr [%s]</legend>\n", nvram_safe_get(wl_macaddr));
	char power[16];

#if !defined(HAVE_EASY_WIRELESS_CONFIG) || defined(HAVE_BCMMODERN)
	// char maxpower[16];
#ifdef HAVE_MADWIFI
#ifndef HAVE_MAKSAT
#ifndef HAVE_DDLINK

	if (isXR36(prefix)) {
		char wl_cardtype[32];
		sprintf(wl_cardtype, "%s_cardtype", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.cardtype)</script></div>\n<select name=\"%s\">\n", wl_cardtype);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Ubiquiti XR3.3</option>\");\n", nvram_default_match(wl_cardtype, "0", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Ubiquiti XR3.6</option>\");\n", nvram_default_match(wl_cardtype, "1", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Ubiquiti XR3.7</option>\");\n", nvram_default_match(wl_cardtype, "2", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
	}

	if (isEMP(prefix)) {
		char wl_cardtype[32];
		sprintf(wl_cardtype, "%s_cardtype", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.cardtype)</script></div>\n<select name=\"%s\">\n", wl_cardtype);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Atheros Generic</option>\");\n", nvram_default_match(wl_cardtype, "0", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >Alfa Networks AWPCI085H</option>\");\n", nvram_default_match(wl_cardtype, "5", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"6\\\" %s >Alfa Networks AWPCI085P</option>\");\n", nvram_default_match(wl_cardtype, "6", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >Doodle Labs DLM105</option>\");\n", nvram_default_match(wl_cardtype, "7", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >MakSat MAK27</option>\");\n", nvram_default_match(wl_cardtype, "4", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Senao EMP-8602</option>\");\n", nvram_default_match(wl_cardtype, "1", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Senao EMP-8603-S</option>\");\n", nvram_default_match(wl_cardtype, "2", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >Senao EMP-8603</option>\");\n", nvram_default_match(wl_cardtype, "3", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
	}
#endif
#endif				// ! HAVE MAKSAT
#ifndef HAVE_NOCOUNTRYSEL
	char wl_regdomain[16];

	sprintf(wl_regdomain, "%s_regdomain", prefix);
	if (is_ath9k(prefix) || nvram_nmatch("1", "%s_regulatory", prefix) || !issuperchannel()) {
		websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.regdom)</script></div>\n");
		char *list = getCountryList();

		showOptions(wp, wl_regdomain, list, nvram_safe_get(wl_regdomain));
		websWrite(wp, "</div>\n");
	}
#endif				// ! HAVE MAKSAT
	/*
	 * while (regdomains[domcount].name != NULL) { char domcode[16]; sprintf
	 * (domcode, "%d", regdomains[domcount].code); websWrite (wp, "<option
	 * value=\"%d\" %s>%s</option>\n", regdomains[domcount].code, nvram_match 
	 * (wl_regdomain, domcode) ? " selected=\"selected\"" : "",
	 * regdomains[domcount].name); domcount++; } websWrite (wp,
	 * "</select>\n"); websWrite (wp, "</div>\n");
	 */
	// power adjustment
	sprintf(power, "%s_txpwrdbm", prefix);
	// sprintf (maxpower, "%s_maxpower", prefix);
	if (issuperchannel())	// show
		// client
		// only on
		// first
		// interface
	{

		char regulatory[32];
		sprintf(regulatory, "%s_regulatory", prefix);
		nvram_default_get(regulatory, "0");
		websWrite(wp, " 	<div class=\"setting\">\n");
		websWrite(wp, " 		<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.regulatory)</script></div>\n");
		websWrite(wp,
			  " 		<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_regulatory\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			  prefix, nvram_match(regulatory, "0") ? "checked" : "");
		websWrite(wp,
			  " 		<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_regulatory\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			  prefix, nvram_match(regulatory, "1") ? "checked" : "");
		websWrite(wp, " 	</div>\n");

	}
	int txpower = atoi(nvram_safe_get(power));
#ifdef HAVE_ESPOD
#ifdef HAVE_SUB3
	if (txpower > 28) {
		txpower = 28;
		nvram_set(power, "28");
	}
#else
	if (txpower > 30) {
		txpower = 28;
		nvram_set(power, "30");
	}
#endif
#endif
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp,
		  "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.TXpower)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%d\" /> dBm\n",
		  power, txpower + wifi_gettxpoweroffset(prefix));
	websWrite(wp, "</div>\n");
	sprintf(power, "%s_antgain", prefix);
#ifndef HAVE_MAKSAT
	if (nvram_nmatch("1", "%s_regulatory", prefix))
#endif
	{
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp,
			  "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.AntGain)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%s\" /> dBi\n",
			  power, nvram_default_get(power, "0"));
		websWrite(wp, "</div>\n");
	}
#endif

#ifdef HAVE_MADWIFI
	// if (!strcmp (prefix, "ath0"))
#endif
	{
		// #ifdef HAVE_MADWIFI
		// if (!strcmp (prefix, "ath0")) //show client only on first
		// interface
		// #endif
		{
#ifdef HAVE_MADWIFI
			// if (!strcmp (prefix, "ath0")) //show client only on first
			// interface
			// if (nvram_match ("ath0_mode", "wdsap")
			// || nvram_match ("ath0_mode", "wdssta"))
			// showOption (wp, "wl_basic.wifi_bonding", "wifi_bonding");
#endif
#ifdef HAVE_REGISTER
			int cpeonly = iscpe();
#else
			int cpeonly = 0;
#endif
			websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n", wl_mode);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			if (!cpeonly) {
				websWrite(wp, "document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");\n", nvram_match(wl_mode, "ap") ? "selected=\\\"selected\\\"" : "");
			}
#ifndef HAVE_RT61
			websWrite(wp, "document.write(\"<option value=\\\"sta\\\" %s >\" + wl_basic.client + \"</option>\");\n", nvram_match(wl_mode, "sta") ? "selected=\\\"selected\\\"" : "");
#endif
#ifndef HAVE_RT2880
#ifdef HAVE_RELAYD
			websWrite(wp, "document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientRelayd + \"</option>\");\n",
#else
			websWrite(wp, "document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientBridge + \"</option>\");\n",
#endif
				  nvram_match(wl_mode, "wet") ? "selected=\\\"selected\\\"" : "");
#endif
			if (!cpeonly)
				websWrite(wp, "document.write(\"<option value=\\\"infra\\\" %s >\" + wl_basic.adhoc + \"</option>\");\n", nvram_match(wl_mode, "infra") ? "selected=\\\"selected\\\"" : "");
#ifndef HAVE_MADWIFI
			if (!cpeonly) {
				websWrite(wp, "document.write(\"<option value=\\\"apsta\\\" %s >\" + wl_basic.repeater + \"</option>\");\n", nvram_match(wl_mode, "apsta") ? "selected=\\\"selected\\\"" : "");
//#ifndef HAVE_RT2880
				websWrite(wp, "document.write(\"<option value=\\\"apstawet\\\" %s >\" + wl_basic.repeaterbridge + \"</option>\");\n", nvram_match(wl_mode, "apstawet") ? "selected=\\\"selected\\\"" : "");
			}
//#endif
#else
			websWrite(wp, "document.write(\"<option value=\\\"wdssta\\\" %s >\" + wl_basic.wdssta + \"</option>\");\n", nvram_match(wl_mode, "wdssta") ? "selected=\\\"selected\\\"" : "");
			if (!cpeonly) {
				websWrite(wp, "document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");\n", nvram_match(wl_mode, "wdsap") ? "selected=\\\"selected\\\"" : "");
			}
#endif
			websWrite(wp, "//]]>\n</script>\n");
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
		}
		/*
		 * #ifdef HAVE_MADWIFI else {
		 * 
		 * 
		 * websWrite (wp, "<div class=\"setting\"><div
		 * class=\"label\"><script
		 * type=\"text/javascript\">Capture(wl_basic.label)</script></div><select 
		 * name=\"%s\">\n", wl_mode); websWrite (wp, "<script
		 * type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option
		 * value=\\\"ap\\\" %s >\" + wl_basic.ap +
		 * \"</option>\");\n//]]>\n</script>\n", nvram_match (wl_mode, "ap")
		 * ? "selected=\\\"selected\\\"" : ""); websWrite (wp, "<script
		 * type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option
		 * value=\\\"infra\\\" %s >\" + wl_basic.adhoc +
		 * \"</option>\");\n//]]>\n</script>\n", nvram_match (wl_mode,
		 * "infra") ? "selected=\\\"selected\\\"" : ""); websWrite (wp,
		 * "<script type=\"text/javascript\">\n//<![CDATA[\n
		 * document.write(\"<option value=\\\"wdssta\\\" %s >\" +
		 * wl_basic.wdssta + \"</option>\");\n//]]>\n</script>\n",
		 * nvram_match (wl_mode, "wdssta") ? "selected=\\\"selected\\\"" :
		 * ""); websWrite (wp, "<script
		 * type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option
		 * value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap +
		 * \"</option>\");\n//]]>\n</script>\n", nvram_match (wl_mode,
		 * "wdsap") ? "selected=\\\"selected\\\"" : ""); websWrite (wp,
		 * "</select>\n"); websWrite (wp, "</div>\n"); } #endif
		 */
	}
	// wireless net mode
	show_netmode(wp, prefix);
	// turbo options
#ifdef HAVE_MADWIFI

	// char wl_xchanmode[16];
	char wl_outdoor[16];
	char wl_diversity[16];
	char wl_rxantenna[16];
	char wl_txantenna[16];
	char wl_width[16];
	char wl_preamble[16];
	char wl_xr[16];
	char wl_comp[32];
	char wl_ff[16];
	char wmm[32];
	char wl_isolate[32];
	char wl_sifstime[32];
	char wl_preambletime[32];
	char wl_intmit[32];
	char wl_noise_immunity[32];
	char wl_ofdm_weak_det[32];
	char wl_protmode[32];
	char wl_doth[32];
	char wl_csma[32];

	sprintf(wl_csma, "%s_csma", prefix);
	sprintf(wl_doth, "%s_doth", prefix);
	sprintf(wl_protmode, "%s_protmode", prefix);
	sprintf(wl_outdoor, "%s_outdoor", prefix);
	sprintf(wl_diversity, "%s_diversity", prefix);
	sprintf(wl_rxantenna, "%s_rxantenna", prefix);
	sprintf(wl_txantenna, "%s_txantenna", prefix);
	sprintf(wl_width, "%s_channelbw", prefix);
//    sprintf( wl_comp, "%s_compression", prefix );
	sprintf(wl_ff, "%s_ff", prefix);
	sprintf(wl_preamble, "%s_preamble", prefix);
	sprintf(wl_preambletime, "%s_preambletime", prefix);
	sprintf(wl_sifstime, "%s_sifstime", prefix);
	sprintf(wl_xr, "%s_xr", prefix);

	sprintf(wl_intmit, "%s_intmit", prefix);
	sprintf(wl_noise_immunity, "%s_noise_immunity", prefix);
	sprintf(wl_ofdm_weak_det, "%s_ofdm_weak_det", prefix);

#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
	if (!is_ath11n(prefix))
#endif
	{
		showAutoOption(wp, "wl_basic.intmit", wl_intmit);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.noise_immunity)</script></div>\n<select name=\"%s\">\n", wl_noise_immunity);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >0</option>\");\n", nvram_default_match(wl_noise_immunity, "0", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n", nvram_default_match(wl_noise_immunity, "1", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >2</option>\");\n", nvram_default_match(wl_noise_immunity, "2", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >3</option>\");\n", nvram_default_match(wl_noise_immunity, "3", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >4</option>\");\n", nvram_default_match(wl_noise_immunity, "4", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");

		showRadio(wp, "wl_basic.ofdm_weak_det", wl_ofdm_weak_det);
	}

	showOptionsLabel(wp, "wl_basic.protmode", wl_protmode, "None CTS RTS/CTS", nvram_default_get(wl_protmode, "None"));
	showrtssettings(wp, prefix);
	if (!is_ath11n(prefix)) {
		show_rates(wp, prefix, 0);
		show_rates(wp, prefix, 1);
	}
	showRadio(wp, "wl_basic.preamble", wl_preamble);
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
	if (!is_ath11n(prefix))
#endif
	{
		showRadio(wp, "wl_basic.extrange", wl_xr);
		showRadio(wp, "wl_basic.supergff", wl_ff);
	}
#if 0
	showRadio(wp, "wl_basic.csma", wl_csma);
#endif
	// showOption (wp, "wl_basic.extchannel", wl_xchanmode);
#ifndef HAVE_BUFFALO
#if !defined(HAVE_FONERA) && !defined(HAVE_LS2) && !defined(HAVE_MERAKI)
	if (has_5ghz(prefix)) {
		if (nvram_nmatch("1", "%s_regulatory", prefix)
		    || !issuperchannel()) {
			showRadio(wp, "wl_basic.outband", wl_outdoor);
		}
	}
#endif
#endif
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div><select name=\"%s\" >\n", wl_width);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");

#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
	if (is_ath11n(prefix)) {

#if defined(HAVE_ATH9K)
		if (!is_ath9k(prefix) || has_ht40(prefix))
#endif
			if ((nvram_nmatch("n-only", "%s_net_mode", prefix)
			     || nvram_nmatch("ng-only", "%s_net_mode", prefix)
			     || nvram_nmatch("n2-only", "%s_net_mode", prefix)
			     || nvram_nmatch("mixed", "%s_net_mode", prefix)
			     || nvram_nmatch("n5-only", "%s_net_mode", prefix)
			     || nvram_nmatch("na-only", "%s_net_mode", prefix))) {
				websWrite(wp, "document.write(\"<option value=\\\"2040\\\" %s >\" + share.dynamicturbo + \"</option>\");\n", nvram_match(wl_width, "2040") ? "selected=\\\"selected\\\"" : "");
				fprintf(stderr, "[CHANNEL WIDTH] 20/40 (1)\n");
			}
	}
	if (!is_ath11n(prefix) || is_ath9k(prefix) || ||(is_ath11n(prefix)
							 && (nvram_nmatch("n-only", "%s_net_mode", prefix)
							     || nvram_nmatch("ng-only", "%s_net_mode", prefix)
							     || nvram_nmatch("n2-only", "%s_net_mode", prefix)
							     || nvram_nmatch("mixed", "%s_net_mode", prefix)
							     || nvram_nmatch("n5-only", "%s_net_mode", prefix)
							     || nvram_nmatch("na-only", "%s_net_mode", prefix))))
#endif
	{
#if defined(HAVE_ATH9K)
		if (!is_ath9k(prefix) || has_ht40(prefix))
#endif
			websWrite(wp, "document.write(\"<option value=\\\"40\\\" %s >\" + share.turbo + \"</option>\");\n", nvram_match(wl_width, "40") ? "selected=\\\"selected\\\"" : "");
	}
	websWrite(wp, "document.write(\"<option value=\\\"20\\\" %s >\" + share.full + \"</option>\");\n", nvram_match(wl_width, "20") ? "selected=\\\"selected\\\"" : "");

#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
}
#endif
#if !defined(HAVE_BUFFALO)
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K) && !defined(HAVE_MADIFI_MIMO)
{
	websWrite(wp, "document.write(\"<option value=\\\"10\\\" %s >\" + share.half + \"</option>\");\n", nvram_match(wl_width, "10") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >\" + share.quarter + \"</option>\");\n", nvram_match(wl_width, "5") ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_SUBQUARTER
	if (registered_has_subquarter()) {
		/* will be enabled once it is tested and the spectrum analyse is done */
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + share.subquarter + \"</option>\");\n", nvram_match(wl_width, "2") ? "selected=\\\"selected\\\"" : "");
	}
#endif
}
#endif
#endif
websWrite(wp, "//]]>\n</script>\n");
websWrite(wp, "</select>\n");
websWrite(wp, "</div>\n");
/*#if defined(HAVE_EOC5610)
	websWrite(wp,
		  "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label25)</script></div><select name=\"%s\" >\n",
		  wl_txantenna);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp,
		  "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.ghz5 + \"</option>\");\n",
		  nvram_match(wl_txantenna,
			      "1") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp,
		  "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.ghz24 + \"</option>\");\n",
		  nvram_match(wl_txantenna,
			      "2") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n");

	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");*/
#if defined(HAVE_PICO2) || defined(HAVE_PICO2HP) || defined(HAVE_PICO5)

/*    websWrite( wp,
	       "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label25)</script></div><select name=\"%s\" >\n",
	       wl_txantenna );
    websWrite( wp, "<script type=\"text/javascript\">\n//<![CDATA[\n" );
    websWrite( wp,
	       "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.internal + \"</option>\");\n",
	       nvram_match( wl_txantenna,
			    "1" ) ? "selected=\\\"selected\\\"" : "" );
    websWrite( wp,
	       "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.external + \"</option>\");\n",
	       nvram_match( wl_txantenna,
			    "2" ) ? "selected=\\\"selected\\\"" : "" );
    websWrite( wp, "//]]>\n</script>\n" );

    websWrite( wp, "</select>\n" );
    websWrite( wp, "</div>\n" );*/
//#elif defined(HAVE_EOC1650)
/*    websWrite( wp,
	       "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label25)</script></div><select name=\"%s\" >\n",
	       wl_txantenna );
    websWrite( wp, "<script type=\"text/javascript\">\n//<![CDATA[\n" );
    websWrite( wp,
	       "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.internal + \"</option>\");\n",
	       nvram_match( wl_txantenna,
			    "2" ) ? "selected=\\\"selected\\\"" : "" );
    websWrite( wp,
	       "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.external + \"</option>\");\n",
	       nvram_match( wl_txantenna,
			    "1" ) ? "selected=\\\"selected\\\"" : "" );
    websWrite( wp, "//]]>\n</script>\n" );

    websWrite( wp, "</select>\n" );
    websWrite( wp, "</div>\n" );*/
#elif defined(HAVE_NS2) || defined(HAVE_NS5) || defined(HAVE_LC2) || defined(HAVE_LC5) || defined(HAVE_NS3)

websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label24)</script></div><select name=\"%s\" >\n", wl_txantenna);
websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.vertical + \"</option>\");\n", nvram_match(wl_txantenna, "0") ? "selected=\\\"selected\\\"" : "");
websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.horizontal + \"</option>\");\n", nvram_match(wl_txantenna, "1") ? "selected=\\\"selected\\\"" : "");
websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >\" + wl_basic.adaptive + \"</option>\");\n", nvram_match(wl_txantenna, "3") ? "selected=\\\"selected\\\"" : "");
#if defined(HAVE_NS5) || defined(HAVE_NS2) || defined(HAVE_NS3)
websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.external + \"</option>\");\n", nvram_match(wl_txantenna, "2") ? "selected=\\\"selected\\\"" : "");
#endif
websWrite(wp, "//]]>\n</script>\n");

websWrite(wp, "</select>\n");
websWrite(wp, "</div>\n");

#else
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
if (!is_ath11n(prefix))
#endif
{
	showRadio(wp, "wl_basic.diversity", wl_diversity);
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label12)</script></div><select name=\"%s\" >\n", wl_txantenna);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");\n", nvram_match(wl_txantenna, "0") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");\n", nvram_match(wl_txantenna, "1") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");\n", nvram_match(wl_txantenna, "2") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label13)</script></div><select name=\"%s\" >\n", wl_rxantenna);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");\n", nvram_match(wl_rxantenna, "0") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");\n", nvram_match(wl_rxantenna, "1") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");\n", nvram_match(wl_rxantenna, "2") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
}
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
else {
	int maxrx = 7;
	int maxtx = 7;
#ifdef HAVE_ATH9K
	int prefixcount;
	sscanf(prefix, "ath%d", &prefixcount);
	int phy_idx = get_ath9k_phy_idx(prefixcount);
	maxrx = mac80211_get_avail_rx_antenna(phy_idx);
	maxtx = mac80211_get_avail_tx_antenna(phy_idx);
#endif
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.txchainmask)</script></div><select name=\"%s\" >\n", wl_txantenna);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n", nvram_match(wl_txantenna, "1") ? "selected=\\\"selected\\\"" : "");
	if (maxtx > 1)
		websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >1+2</option>\");\n", nvram_match(wl_txantenna, "3") ? "selected=\\\"selected\\\"" : "");
	if (maxtx > 3)
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >1+3</option>\");\n", nvram_match(wl_txantenna, "5") ? "selected=\\\"selected\\\"" : "");
	if (maxtx > 5)
		websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >1+2+3</option>\");\n", nvram_match(wl_txantenna, "7") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.rxchainmask)</script></div><select name=\"%s\" >\n", wl_rxantenna);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n", nvram_match(wl_rxantenna, "1") ? "selected=\\\"selected\\\"" : "");
	if (maxrx > 1)
		websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >1+2</option>\");\n", nvram_match(wl_rxantenna, "3") ? "selected=\\\"selected\\\"" : "");
	if (maxrx > 3)
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >1+3</option>\");\n", nvram_match(wl_rxantenna, "5") ? "selected=\\\"selected\\\"" : "");
	if (maxrx > 5)
		websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >1+2+3</option>\");\n", nvram_match(wl_rxantenna, "7") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

}
#endif
#endif
#endif
#ifdef HAVE_MADWIFI
sprintf(wl_isolate, "%s_ap_isolate", prefix);
showRadio(wp, "wl_adv.label11", wl_isolate);

#if 0
websWrite(wp, "<div class=\"setting\">\n");
websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.sifstime)</script></div>\n");
websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,99999999,wl_basic.sifstime)\" value=\"%s\" />\n", wl_sifstime, nvram_default_get(wl_sifstime, "16"));
websWrite(wp, "</div>\n");

websWrite(wp, "<div class=\"setting\">\n");
websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.preambletime)</script></div>\n");
websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,99999999,wl_basic.preambletime)\" value=\"%s\" />\n", wl_preambletime, nvram_default_get(wl_preambletime, "20"));
websWrite(wp, "</div>\n");
#endif
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
if (!is_ath11n(prefix))
#endif
{
	sprintf(wmm, "%s_wmm", prefix);
	showRadio(wp, "wl_adv.label18", wmm);
}
#endif

websWrite(wp, "<div class=\"setting\">\n");
websWrite(wp,
	  "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label3)</script></div><input name=\"%s\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.label3)\" value=\"%s\" /></div>\n",
	  wl_ssid, nvram_safe_get(wl_ssid));

#ifdef HAVE_MADWIFI
#ifndef HAVE_BUFFALO
if (has_5ghz(prefix)) {
	showRadio(wp, "wl_basic.radar", wl_doth);
}
show_chanshift(wp, prefix);
#endif
#endif
#ifdef HAVE_RT2880
#else
if (nvram_match(wl_mode, "ap") || nvram_match(wl_mode, "wdsap")
    || nvram_match(wl_mode, "infra"))
#endif
{

	if (has_mimo(prefix)
	    && (nvram_nmatch("n-only", "%s_net_mode", prefix)
		|| nvram_nmatch("ng-only", "%s_net_mode", prefix)
		|| nvram_nmatch("mixed", "%s_net_mode", prefix)
		|| nvram_nmatch("n2-only", "%s_net_mode", prefix)
		|| nvram_nmatch("n5-only", "%s_net_mode", prefix)
		|| nvram_nmatch("ac-only", "%s_net_mode", prefix)
		|| nvram_nmatch("na-only", "%s_net_mode", prefix))) {
		show_channel(wp, prefix, prefix, 1);

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div>\n");
		websWrite(wp, "<select name=\"%s_nbw\">\n", prefix);
#ifdef HAVE_RT2880
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"20\\\" %s >20 MHz</option>\");\n//]]>\n</script>\n",
			  nvram_nmatch("20", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "<option value=\"40\" %s>40 MHz</option>", nvram_nmatch("40", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
#else
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\" %s >\" + share.auto + \"</option>\");\n//]]>\n</script>\n",
			  nvram_nmatch("0", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "<option value=\"20\" %s>20 MHz</option>\n", nvram_nmatch("20", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "<option value=\"40\" %s>40 MHz</option>\n", nvram_nmatch("40", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
		if (has_ac(prefix) && has_5ghz(prefix)) {
			websWrite(wp, "<option value=\"80\" %s>80 MHz</option>\n", nvram_nmatch("80", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");

		}
#endif
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");

		if (nvram_nmatch("40", "%s_nbw", prefix)) {
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
			websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
			websWrite(wp, "<option value=\"upper\" %s>upper</option>\n", nvram_nmatch("upper", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "<option value=\"lower\" %s>lower</option>\n", nvram_nmatch("lower", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "</select>\n");

			websWrite(wp, "</div>\n");
		}
		if (nvram_nmatch("80", "%s_nbw", prefix)) {	// 802.11ac
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
			websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
			websWrite(wp, "<option value=\"ll\" %s>lower lower</option>\n", nvram_nmatch("ll", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "<option value=\"lu\" %s>lower upper</option>\n", nvram_nmatch("lu", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "<option value=\"ul\" %s>upper lower</option>\n", nvram_nmatch("ul", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "<option value=\"uu\" %s>upper upper</option>\n", nvram_nmatch("uu", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
		}
	} else {

		show_channel(wp, prefix, prefix, 0);
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
		if (is_ath11n(prefix)
		    && (nvram_match(wl_width, "40")
			|| nvram_match(wl_width, "2040"))) {
			fprintf(stderr, "[CHANNEL WIDTH] 20/40 (2)\n");
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
			websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
			websWrite(wp, "<option value=\"upper\" %s>upper</option>\n", nvram_nmatch("upper", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "<option value=\"lower\" %s>lower</option>\n", nvram_nmatch("lower", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "</select>\n");

			websWrite(wp, "</div>\n");
		}
#endif
	}

	char wl_closed[16];

	sprintf(wl_closed, "%s_closed", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label5)</script></div>\n");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		  wl_closed, nvram_match(wl_closed, "0") ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
		  wl_closed, nvram_match(wl_closed, "1") ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
}
#ifdef HAVE_MADWIFI
	// if (nvram_match (wl_mode, "sta") || nvram_match (wl_mode, "wdssta")
	// || nvram_match (wl_mode, "wet"))
{
	char wl_scanlist[32];

	sprintf(wl_scanlist, "%s_scanlist", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.scanlist)</script></div>\n");
	websWrite(wp, "<input name=\"%s\" size=\"32\" maxlength=\"512\" value=\"%s\" />\n", wl_scanlist, nvram_default_get(wl_scanlist, "default"));
	websWrite(wp, "</div>\n");
}
#endif

	// ACK timing
#if defined(HAVE_ACK) || defined(HAVE_MADWIFI)	// temp fix for v24 broadcom
	// ACKnot working

sprintf(power, "%s_distance", prefix);
	//websWrite(wp, "<br />\n");
websWrite(wp, "<div class=\"setting\">\n");
websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label6)</script></div>\n");
websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"8\" maxlength=\"8\" onblur=\"valid_range(this,0,99999999,wl_basic.label6)\" value=\"%s\" />\n", power, nvram_default_get(power, "2000"));
websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 2000 \" + share.meters + \")\");\n//]]>\n</script></span>\n");
websWrite(wp, "</div>\n");
	// end ACK timing
#endif
#ifdef HAVE_MADWIFI
if (nvram_nmatch("ap", "%s_mode", prefix)
    || nvram_nmatch("wdsap", "%s_mode", prefix)
    || nvram_nmatch("infra", "%s_mode", prefix)) {
	sprintf(power, "%s_maxassoc", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label10)</script></div>\n");
	websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,256,wl_basic.label6)\" value=\"%s\" />\n", power, nvram_default_get(power, "256"));
	websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 256 \" + status_wireless.legend3 + \")\");\n//]]>\n</script></span>\n");
	websWrite(wp, "</div>\n");
}
sprintf(power, "%s_mtikie", prefix);
nvram_default_get(power, "0");
showRadio(wp, "wl_basic.mtikie", power);

showbridgesettings(wp, prefix, 1, 1);
#elif HAVE_RT2880
showbridgesettings(wp, "ra0", 1, 1);
#else
if (!strcmp(prefix, "wl0"))
	showbridgesettings(wp, get_wl_instance_name(0), 1, 1);
if (!strcmp(prefix, "wl1"))
	showbridgesettings(wp, get_wl_instance_name(1), 1, 1);
#endif
#else
// BUFFALO Basic
#ifdef HAVE_MADWIFI
	// if (!strcmp (prefix, "ath0"))
#endif
	{
		// #ifdef HAVE_MADWIFI
		// if (!strcmp (prefix, "ath0")) //show client only on first
		// interface
		// #endif
		{
#ifdef HAVE_MADWIFI
			// if (!strcmp (prefix, "ath0")) //show client only on first
			// interface
			// if (nvram_match ("ath0_mode", "wdsap")
			// || nvram_match ("ath0_mode", "wdssta"))
			// showOption (wp, "wl_basic.wifi_bonding", "wifi_bonding");
#endif

#ifdef HAVE_REGISTER
			int cpeonly = iscpe();
#else
			int cpeonly = 0;
#endif
			websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n", wl_mode);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			if (!cpeonly) {
				websWrite(wp, "document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");\n", nvram_match(wl_mode, "ap") ? "selected=\\\"selected\\\"" : "");
			}
#ifndef HAVE_RT61
			websWrite(wp, "document.write(\"<option value=\\\"sta\\\" %s >\" + wl_basic.client + \"</option>\");\n", nvram_match(wl_mode, "sta") ? "selected=\\\"selected\\\"" : "");
#endif
#ifndef HAVE_RT2880
#ifdef HAVE_RELAYD
			websWrite(wp, "document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientRelayd + \"</option>\");\n",
#else
			websWrite(wp, "document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientBridge + \"</option>\");\n",
#endif
				  nvram_match(wl_mode, "wet") ? "selected=\\\"selected\\\"" : "");
#endif
#ifndef HAVE_BUFFALO
			if (!cpeonly)
#else
			if (!cpeonly && !has_5ghz(prefix))
#endif
				websWrite(wp, "document.write(\"<option value=\\\"infra\\\" %s >\" + wl_basic.adhoc + \"</option>\");\n", nvram_match(wl_mode, "infra") ? "selected=\\\"selected\\\"" : "");
#ifndef HAVE_MADWIFI
			if (!cpeonly) {
				websWrite(wp, "document.write(\"<option value=\\\"apsta\\\" %s >\" + wl_basic.repeater + \"</option>\");\n", nvram_match(wl_mode, "apsta") ? "selected=\\\"selected\\\"" : "");
//#ifndef HAVE_RT2880
				websWrite(wp, "document.write(\"<option value=\\\"apstawet\\\" %s >\" + wl_basic.repeaterbridge + \"</option>\");\n", nvram_match(wl_mode, "apstawet") ? "selected=\\\"selected\\\"" : "");
			}
//#endif
#else
			websWrite(wp, "document.write(\"<option value=\\\"wdssta\\\" %s >\" + wl_basic.wdssta + \"</option>\");\n", nvram_match(wl_mode, "wdssta") ? "selected=\\\"selected\\\"" : "");
			if (!cpeonly) {
				websWrite(wp, "document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");\n", nvram_match(wl_mode, "wdsap") ? "selected=\\\"selected\\\"" : "");
			}
#endif
			websWrite(wp, "//]]>\n</script>\n");
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
		}
	}

// RELAYD OPTIONAL SETTINGS
#ifdef HAVE_RELAYD
	if (nvram_match(wl_mode, "wet")) {
		char wl_relayd[32];
		int ip[4] = { 0, 0, 0, 0 };

		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.clientRelaydDefaultGwMode)</script></div>");
		sprintf(wl_relayd, "%s_relayd_gw_auto", prefix);
		websWrite(wp,
			  " 		<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_relayd_gw_auto\" onclick=\"show_layer_ext(this, '%s_relayd_gw_ipaddr', false)\" %s /><script type=\"text/javascript\">Capture(share.auto)</script>&nbsp;(DHCP)&nbsp;\n",
			  prefix, prefix, nvram_default_match(wl_relayd, "1", "1") ? "checked" : "");
		websWrite(wp,
			  " 		<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_relayd_gw_auto\" onclick=\"show_layer_ext(this, '%s_relayd_gw_ipaddr', true)\" %s/><script type=\"text/javascript\">Capture(share.manual)</script>\n",
			  prefix, prefix, nvram_default_match(wl_relayd, "0", "1") ? "checked" : "");
		websWrite(wp, "</div>\n");

		sprintf(wl_relayd, "%s_relayd_gw_ipaddr", prefix);
		sscanf(nvram_safe_get(wl_relayd), "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
		sprintf(wl_relayd, "%s_relayd_gw_auto", prefix);
		websWrite(wp, "\
	<div id=\"%s_relayd_gw_ipaddr\" class=\"setting\"%s>\n\
	          <input type=\"hidden\" name=\"%s_relayd_gw_ipaddr\" value=\"4\">\n\
	          <div class=\"label\"><script type=\"text/javascript\">Capture(share.gateway)</script></div>\n\
	          <input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_0\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_1\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_2\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_3\" value=\"%d\" onblur=\"valid_range(this,1,254,'IP')\" class=\"num\">\n\
       </div>\n", prefix, nvram_default_match(wl_relayd, "1", "0") ? " style=\"display: none; visibility: hidden;\"" : "", prefix, prefix, ip[0], prefix, ip[1], prefix, ip[2], prefix, ip[3]);
	}
#endif

	// wireless net mode
	show_netmode(wp, prefix);

	// turbo options
#ifdef HAVE_MADWIFI

	// char wl_xchanmode[16];
	char wl_outdoor[16];
	char wl_diversity[16];
	char wl_rxantenna[16];
	char wl_txantenna[16];
	char wl_width[16];
	char wl_preamble[16];
	char wl_xr[16];
	char wl_comp[32];
	char wl_ff[16];
	char wmm[32];
	char wl_isolate[32];
	char wl_sifstime[32];
	char wl_preambletime[32];
	char wl_intmit[32];
	char wl_noise_immunity[32];
	char wl_ofdm_weak_det[32];
	char wl_protmode[32];
	char wl_doth[32];
	char wl_csma[32];

	sprintf(wl_csma, "%s_csma", prefix);
	sprintf(wl_doth, "%s_doth", prefix);
	sprintf(wl_protmode, "%s_protmode", prefix);
	sprintf(wl_outdoor, "%s_outdoor", prefix);
	sprintf(wl_diversity, "%s_diversity", prefix);
	sprintf(wl_rxantenna, "%s_rxantenna", prefix);
	sprintf(wl_txantenna, "%s_txantenna", prefix);
	sprintf(wl_width, "%s_channelbw", prefix);
//    sprintf( wl_comp, "%s_compression", prefix );
	sprintf(wl_ff, "%s_ff", prefix);
	sprintf(wl_preamble, "%s_preamble", prefix);
	sprintf(wl_preambletime, "%s_preambletime", prefix);
	sprintf(wl_sifstime, "%s_sifstime", prefix);
	sprintf(wl_xr, "%s_xr", prefix);

	sprintf(wl_intmit, "%s_intmit", prefix);
	sprintf(wl_noise_immunity, "%s_noise_immunity", prefix);
	sprintf(wl_ofdm_weak_det, "%s_ofdm_weak_det", prefix);

#if 0
	showRadio(wp, "wl_basic.csma", wl_csma);
#endif
	// showOption (wp, "wl_basic.extchannel", wl_xchanmode);
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div><select name=\"%s\" >\n", wl_width);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
/* limit channel options by mode */
#if defined(HAVE_ATH9K)
	if (!is_ath9k(prefix) || has_ht40(prefix))
#endif
		if (is_ath11n(prefix)) {
			if ((nvram_nmatch("n-only", "%s_net_mode", prefix)
			     || nvram_nmatch("ng-only", "%s_net_mode", prefix)
			     || nvram_nmatch("n2-only", "%s_net_mode", prefix)
			     || nvram_nmatch("mixed", "%s_net_mode", prefix)
			     || nvram_nmatch("n5-only", "%s_net_mode", prefix)
			     || nvram_nmatch("na-only", "%s_net_mode", prefix)))
				websWrite(wp, "document.write(\"<option value=\\\"2040\\\" %s >\" + share.dynamicturbo + \"</option>\");\n", nvram_match(wl_width, "2040") ? "selected=\\\"selected\\\"" : "");

		}
	if (!is_ath11n(prefix)
	    || (is_ath11n(prefix)
		&& (nvram_nmatch("n-only", "%s_net_mode", prefix)
		    || nvram_nmatch("ng-only", "%s_net_mode", prefix)
		    || nvram_nmatch("n2-only", "%s_net_mode", prefix)
		    || nvram_nmatch("mixed", "%s_net_mode", prefix)
		    || nvram_nmatch("n5-only", "%s_net_mode", prefix)
		    || nvram_nmatch("na-only", "%s_net_mode", prefix))))
#endif
	{
#if defined(HAVE_ATH9K)
		if (!is_ath9k(prefix) || has_ht40(prefix))
#endif
			websWrite(wp, "document.write(\"<option value=\\\"40\\\" %s >\" + share.turbo + \"</option>\");\n", nvram_match(wl_width, "40") ? "selected=\\\"selected\\\"" : "");
	}
	websWrite(wp, "document.write(\"<option value=\\\"20\\\" %s >\" + share.full + \"</option>\");\n", nvram_match(wl_width, "20") ? "selected=\\\"selected\\\"" : "");

#if !defined(HAVE_BUFFALO)
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K) && !defined(HAVE_MADIFI_MIMO)
	{
		websWrite(wp, "document.write(\"<option value=\\\"10\\\" %s >\" + share.half + \"</option>\");\n", nvram_match(wl_width, "10") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >\" + share.quarter + \"</option>\");\n", nvram_match(wl_width, "5") ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_SUBQUARTER
		if (registered_has_subquarter()) {
			/* will be enabled once it is tested and the spectrum analyse is done */
			websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + share.subquarter + \"</option>\");\n", nvram_match(wl_width, "2") ? "selected=\\\"selected\\\"" : "");
		}
#endif
	}
#endif
#endif
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

// test
#ifdef HAVE_RT2880
#else
	if (nvram_match(wl_mode, "ap") || nvram_match(wl_mode, "wdsap")
	    || nvram_match(wl_mode, "infra"))
#endif
	{

		if (has_mimo(prefix)
		    && (nvram_nmatch("n-only", "%s_net_mode", prefix)
			|| nvram_nmatch("ng-only", "%s_net_mode", prefix)
			|| nvram_nmatch("mixed", "%s_net_mode", prefix)
			|| nvram_nmatch("n2-only", "%s_net_mode", prefix)
			|| nvram_nmatch("n5-only", "%s_net_mode", prefix)
			|| nvram_nmatch("na-only", "%s_net_mode", prefix))) {
			show_channel(wp, prefix, prefix, 1);

			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div>\n");
			websWrite(wp, "<select name=\"%s_nbw\">\n", prefix);
#ifdef HAVE_RT2880
			websWrite(wp,
				  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"20\\\" %s >20 MHz</option>\");\n//]]>\n</script>\n",
				  nvram_nmatch("20", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "<option value=\"40\" %s>40 MHz</option>", nvram_nmatch("40", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
#else
			websWrite(wp,
				  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\" %s >\" + share.auto + \"</option>\");\n//]]>\n</script>\n",
				  nvram_nmatch("0", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "<option value=\"20\" %s>20 MHz</option>\n", nvram_nmatch("20", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "<option value=\"40\" %s>40 MHz</option>\n", nvram_nmatch("40", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
			if (has_ac(prefix) && has_5ghz(prefix)) {
				websWrite(wp, "<option value=\"80\" %s>80 MHz</option>\n", nvram_nmatch("80", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");

			}
#endif
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");

			if (nvram_nmatch("40", "%s_nbw", prefix)) {
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
				websWrite(wp, "<option value=\"upper\" %s>upper</option>\n", nvram_nmatch("upper", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "<option value=\"lower\" %s>lower</option>\n", nvram_nmatch("lower", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "</select>\n");

				websWrite(wp, "</div>\n");
			}
			if (nvram_nmatch("80", "%s_nbw", prefix)) {	// 802.11ac
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
				websWrite(wp, "<option value=\"ll\" %s>lower lower</option>\n", nvram_nmatch("ll", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "<option value=\"lu\" %s>lower upper</option>\n", nvram_nmatch("lu", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "<option value=\"ul\" %s>upper lower</option>\n", nvram_nmatch("ul", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "<option value=\"uu\" %s>upper upper</option>\n", nvram_nmatch("uu", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "</select>\n");
				websWrite(wp, "</div>\n");
			}
		} else {

			show_channel(wp, prefix, prefix, 0);
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
			if (is_ath11n(prefix)
			    && (nvram_match(wl_width, "40")
				|| nvram_match(wl_width, "2040"))) {
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
				websWrite(wp, "<option value=\"upper\" %s>upper</option>\n", nvram_nmatch("upper", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "<option value=\"lower\" %s>lower</option>\n", nvram_nmatch("lower", "%s_nctrlsb", prefix) ? "selected=\\\"selected\\\"" : "");
				websWrite(wp, "</select>\n");

				websWrite(wp, "</div>\n");
			}
#endif
		}
	}
// wireless ssid
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp,
		  "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label3)</script></div><input name=\"%s\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.label3)\" value=\"",
		  wl_ssid);
	tf_webWriteESCNV(wp, wl_ssid);
	websWrite(wp, "\" /></div>\n");

#ifdef HAVE_RT2880
	if (nvram_match(wl_mode, "ap") || nvram_match(wl_mode, "wdsap")
	    || nvram_match(wl_mode, "infra") || nvram_match(wl_mode, "apsta")
	    || nvram_match(wl_mode, "apstawet"))
#else
	if (nvram_match(wl_mode, "ap") || nvram_match(wl_mode, "wdsap")
	    || nvram_match(wl_mode, "infra"))
#endif
	{
// ssid broadcast
		char wl_closed[16];

		sprintf(wl_closed, "%s_closed", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label5)</script></div>\n");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			  wl_closed, nvram_match(wl_closed, "0") ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			  wl_closed, nvram_match(wl_closed, "1") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
	}
#ifdef HAVE_IFL
// label
	char wl_label[16];
	sprintf(wl_label, "%s_label", prefix);
	websWrite(wp,
		  "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.if_label)</script></div><input type=\"text\" name=\"%s\" value=\"%s\" maxlength=\"20\"></div>\n",
		  wl_label, nvram_safe_get(wl_label));

#endif
// WIRELESS Advanced
	char advanced_label[32];
	sprintf(advanced_label, "%s_wl_advanced", prefix);
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.legend)</script></div>\n");
	websWrite(wp, "<input type=\"checkbox\" name=\"%s\" onclick=\"toggle_layer(this,'%s_layer')\"%s>", advanced_label, advanced_label, websGetVar(wp, advanced_label, NULL) ? " checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div id=\"%s_layer\"%s>\n", advanced_label, websGetVar(wp, advanced_label, NULL) ? "" : " style=\"display: none;\"");
#ifdef HAVE_IFL

	char wl_note[16];
	sprintf(wl_note, "%s_note", prefix);
	websWrite(wp,
		  "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.if_info)</script></div><textarea name=\"%s\" cols=\"60\" rows=\"3\">%s</textarea></div>\n",
		  wl_note, nvram_safe_get(wl_note));
#endif

#ifdef HAVE_MADWIFI
#ifndef HAVE_MAKSAT
#ifndef HAVE_DDLINK

	if (isXR36(prefix)) {
		char wl_cardtype[32];
		sprintf(wl_cardtype, "%s_cardtype", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.cardtype)</script></div>\n<select name=\"%s\">\n", wl_cardtype);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Ubiquiti XR3.3</option>\");\n", nvram_default_match(wl_cardtype, "0", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Ubiquiti XR3.6</option>\");\n", nvram_default_match(wl_cardtype, "1", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Ubiquiti XR3.7</option>\");\n", nvram_default_match(wl_cardtype, "2", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
	}

	if (isEMP(prefix)) {
		char wl_cardtype[32];
		sprintf(wl_cardtype, "%s_cardtype", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.cardtype)</script></div>\n<select name=\"%s\">\n", wl_cardtype);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Atheros Generic</option>\");\n", nvram_default_match(wl_cardtype, "0", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >Alfa Networks AWPCI085H</option>\");\n", nvram_default_match(wl_cardtype, "5", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"6\\\" %s >Alfa Networks AWPCI085P</option>\");\n", nvram_default_match(wl_cardtype, "6", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >Doodle Labs DLM105</option>\");\n", nvram_default_match(wl_cardtype, "7", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >MakSat MAK27</option>\");\n", nvram_default_match(wl_cardtype, "4", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Senao EMP-8602</option>\");\n", nvram_default_match(wl_cardtype, "1", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Senao EMP-8603-S</option>\");\n", nvram_default_match(wl_cardtype, "2", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >Senao EMP-8603</option>\");\n", nvram_default_match(wl_cardtype, "3", "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
	}
#endif
#endif				// ! HAVE MAKSAT

#ifndef HAVE_NOCOUNTRYSEL
	char wl_regdomain[16];

	sprintf(wl_regdomain, "%s_regdomain", prefix);
	if (1 || nvram_nmatch("1", "%s_regulatory", prefix) || !issuperchannel()) {
		websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.regdom)</script></div>\n");
		char *list = getCountryList();

		showOptions(wp, wl_regdomain, list, nvram_safe_get(wl_regdomain));
		websWrite(wp, "</div>\n");
	}
#endif				// ! HAVE MAKSAT
	// power adjustment
	sprintf(power, "%s_txpwrdbm", prefix);
	// sprintf (maxpower, "%s_maxpower", prefix);
	if (issuperchannel())	// show
		// client
		// only on
		// first
		// interface
	{

		char regulatory[32];
		sprintf(regulatory, "%s_regulatory", prefix);
		nvram_default_get(regulatory, "0");
		websWrite(wp, " 	<div class=\"setting\">\n");
		websWrite(wp, " 		<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.regulatory)</script></div>\n");
		websWrite(wp,
			  " 		<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_regulatory\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			  prefix, nvram_match(regulatory, "0") ? "checked" : "");
		websWrite(wp,
			  " 		<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_regulatory\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			  prefix, nvram_match(regulatory, "1") ? "checked" : "");
		websWrite(wp, " 	</div>\n");

	}
	int txpower = atoi(nvram_safe_get(power));
#ifdef HAVE_ESPOD
#ifdef HAVE_SUB3
	if (txpower > 28) {
		txpower = 28;
		nvram_set(power, "28");
	}
#else
	if (txpower > 30) {
		txpower = 30;
		nvram_set(power, "30");
	}
#endif
#endif
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp,
		  "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.TXpower)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%d\" /> dBm\n",
		  power, txpower + wifi_gettxpoweroffset(prefix));
	websWrite(wp, "</div>\n");
	sprintf(power, "%s_antgain", prefix);
#ifndef HAVE_MAKSAT
	if (nvram_nmatch("1", "%s_regulatory", prefix))
#endif
	{
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp,
			  "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.AntGain)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%s\" /> dBi\n",
			  power, nvram_default_get(power, "0"));
		websWrite(wp, "</div>\n");
	}
#endif

	// channel options
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
	if (!is_ath11n(prefix))
#endif
	{
		showAutoOption(wp, "wl_basic.intmit", wl_intmit);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.noise_immunity)</script></div>\n<select name=\"%s\">\n", wl_noise_immunity);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >0</option>\");\n", nvram_default_match(wl_noise_immunity, "0", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n", nvram_default_match(wl_noise_immunity, "1", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >2</option>\");\n", nvram_default_match(wl_noise_immunity, "2", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >3</option>\");\n", nvram_default_match(wl_noise_immunity, "3", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >4</option>\");\n", nvram_default_match(wl_noise_immunity, "4", "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");

		showRadio(wp, "wl_basic.ofdm_weak_det", wl_ofdm_weak_det);
	}

	showOptionsLabel(wp, "wl_basic.protmode", wl_protmode, "None CTS RTS/CTS", nvram_default_get(wl_protmode, "None"));
	showrtssettings(wp, prefix);
	if (!is_ath11n(prefix)) {
		show_rates(wp, prefix, 0);
		show_rates(wp, prefix, 1);
	}
	showRadio(wp, "wl_basic.preamble", wl_preamble);
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
	if (!is_ath11n(prefix))
#endif
	{
		showRadio(wp, "wl_basic.extrange", wl_xr);
		showRadio(wp, "wl_basic.supergff", wl_ff);
	}
#ifndef HAVE_BUFFALO
#if !defined(HAVE_FONERA) && !defined(HAVE_LS2) && !defined(HAVE_MERAKI)
	if (has_5ghz(prefix)) {
		if (nvram_nmatch("1", "%s_regulatory", prefix)
		    || !issuperchannel()) {
			showRadio(wp, "wl_basic.outband", wl_outdoor);
		}
	}
#endif
#endif

// antenna settings
#if defined(HAVE_PICO2) || defined(HAVE_PICO2HP) || defined(HAVE_PICO5)
	// do nothing
#elif defined(HAVE_NS2) || defined(HAVE_NS5) || defined(HAVE_LC2) || defined(HAVE_LC5) || defined(HAVE_NS3)

	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label24)</script></div><select name=\"%s\" >\n", wl_txantenna);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.vertical + \"</option>\");\n", nvram_match(wl_txantenna, "0") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.horizontal + \"</option>\");\n", nvram_match(wl_txantenna, "1") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >\" + wl_basic.adaptive + \"</option>\");\n", nvram_match(wl_txantenna, "3") ? "selected=\\\"selected\\\"" : "");
#if defined(HAVE_NS5) || defined(HAVE_NS2) || defined(HAVE_NS3)
	websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.external + \"</option>\");\n", nvram_match(wl_txantenna, "2") ? "selected=\\\"selected\\\"" : "");
#endif
	websWrite(wp, "//]]>\n</script>\n");

	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

#else
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
	if (!is_ath11n(prefix))
#endif
	{
		showRadio(wp, "wl_basic.diversity", wl_diversity);
		websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label12)</script></div><select name=\"%s\" >\n", wl_txantenna);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");\n", nvram_match(wl_txantenna, "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");\n", nvram_match(wl_txantenna, "1") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");\n", nvram_match(wl_txantenna, "2") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label13)</script></div><select name=\"%s\" >\n", wl_rxantenna);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");\n", nvram_match(wl_rxantenna, "0") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");\n", nvram_match(wl_rxantenna, "1") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");\n", nvram_match(wl_rxantenna, "2") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
	else {
		int maxrx = 7;
		int maxtx = 7;
#ifdef HAVE_ATH9K
		int prefixcount;
		sscanf(prefix, "ath%d", &prefixcount);
		int phy_idx = get_ath9k_phy_idx(prefixcount);
		maxrx = mac80211_get_avail_rx_antenna(phy_idx);
		maxtx = mac80211_get_avail_tx_antenna(phy_idx);
#endif
		websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.txchainmask)</script></div><select name=\"%s\" >\n", wl_txantenna);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n", nvram_match(wl_txantenna, "1") ? "selected=\\\"selected\\\"" : "");
		if (maxtx > 1)
			websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >1+2</option>\");\n", nvram_match(wl_txantenna, "3") ? "selected=\\\"selected\\\"" : "");
		if (maxtx > 3)
			websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >1+3</option>\");\n", nvram_match(wl_txantenna, "5") ? "selected=\\\"selected\\\"" : "");
		if (maxtx > 5)
			websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >1+2+3</option>\");\n", nvram_match(wl_txantenna, "7") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.rxchainmask)</script></div><select name=\"%s\" >\n", wl_rxantenna);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n", nvram_match(wl_rxantenna, "1") ? "selected=\\\"selected\\\"" : "");
		if (maxrx > 1)
			websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >1+2</option>\");\n", nvram_match(wl_rxantenna, "3") ? "selected=\\\"selected\\\"" : "");
		if (maxrx > 3)
			websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >1+3</option>\");\n", nvram_match(wl_rxantenna, "5") ? "selected=\\\"selected\\\"" : "");
		if (maxrx > 5)
			websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >1+2+3</option>\");\n", nvram_match(wl_rxantenna, "7") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");

	}
#endif
#endif
#endif

// ap isolation
#ifdef HAVE_MADWIFI
	sprintf(wl_isolate, "%s_ap_isolate", prefix);
	showRadio(wp, "wl_adv.label11", wl_isolate);

#if 0
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.sifstime)</script></div>\n");
	websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,99999999,wl_basic.sifstime)\" value=\"%s\" />\n", wl_sifstime, nvram_default_get(wl_sifstime, "16"));
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.preambletime)</script></div>\n");
	websWrite(wp,
		  "<input class=\"num\" name=\"%s\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,99999999,wl_basic.preambletime)\" value=\"%s\" />\n", wl_preambletime, nvram_default_get(wl_preambletime, "20"));
	websWrite(wp, "</div>\n");
#endif
// wmm
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
	if (!is_ath11n(prefix))
#endif
	{
		sprintf(wmm, "%s_wmm", prefix);
		showRadio(wp, "wl_adv.label18", wmm);
	}
#endif

// radar detection
#ifdef HAVE_MADWIFI
#ifndef HAVE_BUFFALO
	if (has_5ghz(prefix)) {
		showRadio(wp, "wl_basic.radar", wl_doth);
	}
	show_chanshift(wp, prefix);
#endif
#endif

// scanlist
#ifdef HAVE_MADWIFI
	// if (nvram_match (wl_mode, "sta") || nvram_match (wl_mode, "wdssta")
	// || nvram_match (wl_mode, "wet"))
	{
		char wl_scanlist[32];

		sprintf(wl_scanlist, "%s_scanlist", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.scanlist)</script></div>\n");
		websWrite(wp, "<input name=\"%s\" size=\"32\" maxlength=\"512\" value=\"%s\" />\n", wl_scanlist, nvram_default_get(wl_scanlist, "default"));
		websWrite(wp, "</div>\n");
	}
#endif

	// ACK timing
#if defined(HAVE_ACK) || defined(HAVE_MADWIFI)	// temp fix for v24 broadcom
	// ACKnot working

	sprintf(power, "%s_distance", prefix);
	//websWrite(wp, "<br />\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label6)</script></div>\n");
	websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"8\" maxlength=\"8\" onblur=\"valid_range(this,0,99999999,wl_basic.label6)\" value=\"%s\" />\n", power, nvram_default_get(power, "2000"));
	websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 2000 \" + share.meters + \")\");\n//]]>\n</script></span>\n");
	websWrite(wp, "</div>\n");
	// end ACK timing
#endif
#ifdef HAVE_MADWIFI
	if (nvram_nmatch("ap", "%s_mode", prefix)
	    || nvram_nmatch("wdsap", "%s_mode", prefix)
	    || nvram_nmatch("infra", "%s_mode", prefix)) {
		sprintf(power, "%s_maxassoc", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label10)</script></div>\n");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,256,wl_basic.label6)\" value=\"%s\" />\n", power, nvram_default_get(power, "256"));
		websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 256 \" + status_wireless.legend3 + \")\");\n//]]>\n</script></span>\n");
		websWrite(wp, "</div>\n");
	}
	sprintf(power, "%s_mtikie", prefix);
	nvram_default_get(power, "0");
	showRadio(wp, "wl_basic.mtikie", power);

	showbridgesettings(wp, prefix, 1, 1);
#elif HAVE_RT2880
	showbridgesettings(wp, "ra0", 1, 1);
#else
	if (!strcmp(prefix, "wl0"))
		showbridgesettings(wp, get_wl_instance_name(0), 1, 1);
	if (!strcmp(prefix, "wl1"))
		showbridgesettings(wp, get_wl_instance_name(1), 1, 1);
#endif

	websWrite(wp, "</div>\n");
#endif				// end BUFFALO
	websWrite(wp, "</fieldset>\n");
	websWrite(wp, "<br />\n");
#ifdef HAVE_REGISTER
	if (!iscpe())
#endif
		show_virtualssid(wp, prefix);
}

void ej_show_wireless(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_MADWIFI
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "ath%d", i);
		ej_show_wireless_single(wp, buf);
	}
#else
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "wl%d", i);
		ej_show_wireless_single(wp, buf);
	}
#endif
#ifdef HAVE_GUESTPORT
	websWrite(wp, "<input type=\"hidden\" name=\"gp_modify\" id=\"gp_modify\" value=\"\">\n");
#endif
	return;
}

void show_preshared(webs_t wp, char *prefix)
{
	char var[80];

	cprintf("show preshared");
	sprintf(var, "%s_crypto", prefix);
	websWrite(wp, "<div><div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.algorithms)</script></div>\n");
	websWrite(wp, "<select name=\"%s_crypto\">\n", prefix);
	websWrite(wp, "<option value=\"tkip\" %s>TKIP</option>\n", selmatch(var, "tkip", "selected=\"selected\""));
	websWrite(wp, "<option value=\"aes\" %s>AES</option>\n", selmatch(var, "aes", "selected=\"selected\""));
	websWrite(wp, "<option value=\"tkip+aes\" %s>TKIP+AES</option>\n", selmatch(var, "tkip+aes", "selected=\"selected\""));
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.shared_key)</script></div>\n");
	sprintf(var, "%s_wpa_psk", prefix);
	websWrite(wp,
#ifdef HAVE_BUFFALO
		  "<input type=\"password\" id=\"%s_wpa_psk\" name=\"%s_wpa_psk\" class=\"no-check\" onblur=\"valid_wpa_psk(this, true);\" maxlength=\"64\" size=\"32\" value=\"",
#else
		  "<input type=\"password\" id=\"%s_wpa_psk\" name=\"%s_wpa_psk\" class=\"no-check\" onblur=\"valid_psk_length(this);\" maxlength=\"64\" size=\"32\" value=\"",
#endif
		  prefix, prefix);
	tf_webWriteESCNV(wp, var);
	websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
	websWrite(wp,
		  "<input type=\"checkbox\" name=\"%s_wl_unmask\" value=\"0\" onclick=\"setElementMask('%s_wpa_psk', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
		  prefix, prefix);
	websWrite(wp, "</div>\n");

	if (nvram_nmatch("ap", "%s_mode", prefix)
	    || nvram_nmatch("wdsap", "%s_mode", prefix)) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.rekey)</script></div>\n");
		sprintf(var, "%s_wpa_gtk_rekey", prefix);
		websWrite(wp, "<input class=\"num\" name=\"%s_wpa_gtk_rekey\" maxlength=\"5\" size=\"5\" onblur=\"valid_range(this,0,99999,wpa.rekey)\" value=\"%s\" />\n", prefix, nvram_default_get(var, "3600"));
		websWrite(wp,
#ifdef HAVE_BUFFALO
			  "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 3600, \" + share.range + \": 0 - 99999)\");\n//]]>\n</script></span>\n");
#else
			  "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 3600, \" + share.range + \": 1 - 99999)\");\n//]]>\n</script></span>\n");
#endif
		websWrite(wp, "</div>\n");
	}
	websWrite(wp, "</div>\n");
}

void show_radius(webs_t wp, char *prefix, int showmacformat, int backup)
{
	char var[80];

	cprintf("show radius\n");
	if (showmacformat) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label2)</script></div>\n");
		websWrite(wp, "<select name=\"%s_radmactype\">\n", prefix);
		websWrite(wp, "<option value=\"0\" %s >aabbcc-ddeeff</option>\n", nvram_prefix_match("radmactype", prefix, "0") ? "selected" : "");
		websWrite(wp, "<option value=\"1\" %s >aabbccddeeff</option>\n", nvram_prefix_match("radmactype", prefix, "1") ? "selected" : "");
		websWrite(wp, "<option value=\"2\" %s >aa:bb:cc:dd:ee:ff</option>\n", nvram_prefix_match("radmactype", prefix, "2") ? "selected" : "");
		websWrite(wp, "<option value=\"3\" %s >aa-bb-cc-dd-ee-ff</option>\n", nvram_prefix_match("radmactype", prefix, "3") ? "selected" : "");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}
	char *rad = nvram_nget("%s_radius_ipaddr", prefix);

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label3)</script></div>\n");
	websWrite(wp, "<input type=\"hidden\" name=\"%s_radius_ipaddr\" value=\"4\" />\n", prefix);
	websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_0\" onblur=\"valid_range(this,0,255,radius.label3)\" class=\"num\" value=\"%d\" />.", prefix, get_single_ip(rad, 0));
	websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_1\" onblur=\"valid_range(this,0,255,radius.label3)\" class=\"num\" value=\"%d\" />.", prefix, get_single_ip(rad, 1));
	websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_2\" onblur=\"valid_range(this,0,255,radius.label3)\" class=\"num\" value=\"%d\" />.", prefix, get_single_ip(rad, 2));
	websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_3\" onblur=\"valid_range(this,1,254,radius.label3)\" class=\"num\" value=\"%d\" />\n", prefix, get_single_ip(rad, 3));
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label4)</script></div>\n");
	sprintf(var, "%s_radius_port", prefix);
	websWrite(wp, "<input name=\"%s_radius_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label4)\" value=\"%s\" />\n", prefix, nvram_default_get(var, "1812"));
	websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 1812)\");\n//]]>\n</script></span>\n</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label7)</script></div>\n");
	sprintf(var, "%s_radius_key", prefix);
	websWrite(wp, "<input type=\"password\" id=\"%s_radius_key\" name=\"%s_radius_key\" maxlength=\"79\" size=\"32\" value=\"", prefix, prefix);

	tf_webWriteESCNV(wp, var);
	websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
	websWrite(wp,
		  "<input type=\"checkbox\" name=\"%s_radius_unmask\" value=\"0\" onclick=\"setElementMask('%s_radius_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
		  prefix, prefix);

	if (backup) {
		rad = nvram_nget("%s_radius2_ipaddr", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label23)</script></div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"%s_radius2_ipaddr\" value=\"4\" />\n", prefix);
		websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_radius2_ipaddr_0\" onblur=\"valid_range(this,0,255,radius.label23)\" class=\"num\" value=\"%d\" />.", prefix, get_single_ip(rad, 0));
		websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_radius2_ipaddr_1\" onblur=\"valid_range(this,0,255,radius.label23)\" class=\"num\" value=\"%d\" />.", prefix, get_single_ip(rad, 1));
		websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_radius2_ipaddr_2\" onblur=\"valid_range(this,0,255,radius.label23)\" class=\"num\" value=\"%d\" />.", prefix, get_single_ip(rad, 2));
		websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_radius2_ipaddr_3\" onblur=\"valid_range(this,1,254,radius.label23)\" class=\"num\" value=\"%d\" />\n", prefix, get_single_ip(rad, 3));
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label24)</script></div>\n");
		sprintf(var, "%s_radius2_port", prefix);
		websWrite(wp, "<input name=\"%s_radius2_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label24)\" value=\"%s\" />\n", prefix, nvram_default_get(var, "1812"));
		websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 1812)\");\n//]]>\n</script></span>\n</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label27)</script></div>\n");
		sprintf(var, "%s_radius2_key", prefix);
		websWrite(wp, "<input type=\"password\" id=\"%s_radius2_key\" name=\"%s_radius2_key\" maxlength=\"79\" size=\"32\" value=\"", prefix, prefix);

		tf_webWriteESCNV(wp, var);
		websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
		websWrite(wp,
			  "<input type=\"checkbox\" name=\"%s_radius2_unmask\" value=\"0\" onclick=\"setElementMask('%s_radius2_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
			  prefix, prefix);
	}
	websWrite(wp, "</div>\n");
#ifdef HAVE_MADWIFI
	if (!showmacformat) {
		char acct[32];
		char vvar[32];

		strcpy(vvar, prefix);
		rep(vvar, '.', 'X');
		sprintf(acct, "%s_acct", prefix);
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label18)</script></div>\n");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idacct', true);\" name=\"%s_acct\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>\n",
			  vvar, prefix, nvram_default_match(acct, "1", "0") ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idacct', false);\" name=\"%s_acct\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
			  vvar, prefix, nvram_default_match(acct, "0", "0") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		char *rad = nvram_nget("%s_acct_ipaddr", prefix);

		websWrite(wp, "<div id=\"%s_idacct\">\n", vvar);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label13)</script></div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"%s_acct_ipaddr\" value=\"4\" />\n", prefix);
		websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_acct_ipaddr_0\" onblur=\"valid_range(this,0,255,radius.label13)\" class=\"num\" value=\"%d\" />.", prefix, get_single_ip(rad, 0));
		websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_acct_ipaddr_1\" onblur=\"valid_range(this,0,255,radius.label13)\" class=\"num\" value=\"%d\" />.", prefix, get_single_ip(rad, 1));
		websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_acct_ipaddr_2\" onblur=\"valid_range(this,0,255,radius.label13)\" class=\"num\" value=\"%d\" />.", prefix, get_single_ip(rad, 2));
		websWrite(wp, "<input size=\"3\" maxlength=\"3\" name=\"%s_acct_ipaddr_3\" onblur=\"valid_range(this,1,254,radius.label13)\" class=\"num\" value=\"%d\" />\n", prefix, get_single_ip(rad, 3));
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label14)</script></div>\n");
		sprintf(var, "%s_acct_port", prefix);
		websWrite(wp, "<input name=\"%s_acct_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label14)\" value=\"%s\" />\n", prefix, nvram_default_get(var, "1813"));
		websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 1813)\");\n//]]>\n</script></span>\n</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label17)</script></div>\n");
		sprintf(var, "%s_acct_key", prefix);
		websWrite(wp, "<input type=\"password\" id=\"%s_acct_key\" name=\"%s_acct_key\" maxlength=\"79\" size=\"32\" value=\"", prefix, prefix);
		tf_webWriteESCNV(wp, var);
		websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
		websWrite(wp,
			  "<input type=\"checkbox\" name=\"%s_acct_unmask\" value=\"0\" onclick=\"setElementMask('%s_acct_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
			  prefix, prefix);
		websWrite(wp, "</div>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<script>\n//<![CDATA[\n ");
		websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_acct\"), \"%s_idacct\", %s);\n", prefix, vvar, nvram_match(acct, "1") ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");
	}
#endif
}

#ifdef HAVE_WPA_SUPPLICANT
#ifndef HAVE_MICRO
static void init_80211x_layers(webs_t wp, char *prefix)
{
	if (nvram_prefix_match("8021xtype", prefix, "tls")) {
		websWrite(wp, "enable_idtls(\"%s\");\n", prefix);
	}
	if (nvram_prefix_match("8021xtype", prefix, "leap")) {
		websWrite(wp, "enable_idleap(\"%s\");\n", prefix);
	}
	if (nvram_prefix_match("8021xtype", prefix, "ttls")) {
		websWrite(wp, "enable_idttls(\"%s\");\n", prefix);
	}
	if (nvram_prefix_match("8021xtype", prefix, "peap")) {
		websWrite(wp, "enable_idpeap(\"%s\");\n", prefix);
	}
}

void ej_init_80211x_layers(webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "wl%d", i);
		init_80211x_layers(wp, buf);
	}
	return;
#else
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "ath%d", i);
		if (nvram_nmatch("8021X", "%s_security_mode", buf))
			init_80211x_layers(wp, buf);
	}
	return;
#endif

}

void show_80211X(webs_t wp, char *prefix)
{
	/*
	 * fields
	 * _8021xtype
	 * _8021xuser
	 * _8021xpasswd
	 * _8021xca
	 * _8021xpem
	 * _8021xprv
	 * _8021xaddopt
	 */
	char type[32];
	char var[80];

	sprintf(type, "%s_8021xtype", prefix);
	nvram_default_get(type, "ttls");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.xsuptype)</script></div>\n");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"peap\" onclick=\"enable_idpeap('%s')\" %s />Peap&nbsp;\n",
		  prefix, prefix, nvram_prefix_match("8021xtype", prefix, "peap") ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"leap\" onclick=\"enable_idleap('%s')\" %s />Leap&nbsp;\n",
		  prefix, prefix, nvram_prefix_match("8021xtype", prefix, "leap") ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"tls\" onclick=\"enable_idtls('%s')\" %s />TLS&nbsp;\n",
		  prefix, prefix, nvram_prefix_match("8021xtype", prefix, "tls") ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"ttls\" onclick=\"enable_idttls('%s')\" %s />TTLS&nbsp;\n",
		  prefix, prefix, nvram_prefix_match("8021xtype", prefix, "ttls") ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
	// ttls authentication
	websWrite(wp, "<div id=\"idttls%s\">\n", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.user)</script></div>\n");
	websWrite(wp, "<input name=\"%s_ttls8021xuser\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("ttls8021xuser", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.anon)</script></div>\n");
	websWrite(wp, "<input name=\"%s_ttls8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("ttls8021xanon", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.passwd)</script></div>\n");
	websWrite(wp, "<input name=\"%s_ttls8021xpasswd\" type=\"password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("ttls8021xpasswd", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.phase2)</script></div>\n");
	websWrite(wp, "<input name=\"%s_ttls8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("ttls8021xphase2", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.servercertif)</script></div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"6\" id=\"%s_ttls8021xca\" name=\"%s_ttls8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ", prefix, prefix);
	websWrite(wp, "var %s_ttls8021xca = fix_cr( '", prefix);
	char namebuf[64];
	sprintf(namebuf, "%s_ttls8021xca", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_ttls8021xca\").value = %s_ttls8021xca;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.options)</script></div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"3\" id=\"%s_ttls8021xaddopt\" name=\"%s_ttls8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ", prefix, prefix);
	websWrite(wp, "var %s_ttls8021xaddopt = fix_cr( '", prefix);
	sprintf(namebuf, "%s_ttls8021xaddopt", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_ttls8021xaddopt\").value = %s_ttls8021xaddopt;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "</div>\n");

	// peap authentication
	websWrite(wp, "<div id=\"idpeap%s\">\n", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.user)</script></div>\n");
	websWrite(wp, "<input name=\"%s_peap8021xuser\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("peap8021xuser", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.anon)</script></div>\n");
	websWrite(wp, "<input name=\"%s_peap8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("peap8021xanon", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.passwd)</script></div>\n");
	websWrite(wp, "<input name=\"%s_peap8021xpasswd\" type=\"password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("peap8021xpasswd", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.phase2)</script></div>\n");
	websWrite(wp, "<input name=\"%s_peap8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("peap8021xphase2", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.servercertif)</script></div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"6\" id=\"%s_peap8021xca\" name=\"%s_peap8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ", prefix, prefix);

	websWrite(wp, "var %s_peap8021xca = fix_cr( '", prefix);
	sprintf(namebuf, "%s_peap8021xca", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_peap8021xca\").value = %s_peap8021xca;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.options)</script></div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"3\" id=\"%s_peap8021xaddopt\" name=\"%s_peap8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ", prefix, prefix);
	websWrite(wp, "var %s_peap8021xaddopt = fix_cr( '", prefix);
	sprintf(namebuf, "%s_peap8021xaddopt", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_peap8021xaddopt\").value = %s_peap8021xaddopt;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "</div>\n");
	// leap authentication
	websWrite(wp, "<div id=\"idleap%s\">\n", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.user)</script></div>\n");
	websWrite(wp, "<input name=\"%s_leap8021xuser\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("leap8021xuser", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.anon)</script></div>\n");
	websWrite(wp, "<input name=\"%s_leap8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("leap8021xanon", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.passwd)</script></div>\n");
	websWrite(wp, "<input name=\"%s_leap8021xpasswd\" type=\"password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("leap8021xpasswd", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.phase2)</script></div>\n");
	websWrite(wp, "<input name=\"%s_leap8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("leap8021xphase2", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.options)</script></div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"3\" id=\"%s_leap8021xaddopt\" name=\"%s_leap8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ", prefix, prefix);
	websWrite(wp, "var %s_leap8021xaddopt = fix_cr( '", prefix);
	sprintf(namebuf, "%s_leap8021xaddopt", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_leap8021xaddopt\").value = %s_leap8021xaddopt;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "</div>\n");

	// tls authentication
	websWrite(wp, "<div id=\"idtls%s\">\n", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	sprintf(var, "%s_tls8021xkeyxchng", prefix);
	nvram_default_get(var, "wep");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.keyxchng)</script></div>\n");
	websWrite(wp, "<select name=\"%s_tls8021xkeyxchng\"> size=\"1\"\n", prefix);
	websWrite(wp, "<option value=\"wep\" %s>Radius/WEP</option>\n", selmatch(var, "wep", "selected=\"selected\""));
	websWrite(wp, "<option value=\"wpa2\" %s>WPA2 Enterprise</option>\n", selmatch(var, "wpa2", "selected=\"selected\""));
	websWrite(wp, "<option value=\"wpa2mixed\" %s>WPA2 Enterprise (Mixed)</option>\n", selmatch(var, "wpa2mixed", "selected=\"selected\""));
	websWrite(wp, "<option value=\"wpa\" %s>WPA Enterprise</option>\n", selmatch(var, "wpa", "selected=\"selected\""));
	websWrite(wp, "</select></div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.user)</script></div>\n");
	websWrite(wp, "<input name=\"%s_tls8021xuser\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("tls8021xuser", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.anon)</script></div>\n");
	websWrite(wp, "<input name=\"%s_tls8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("tls8021xanon", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.passwd)</script></div>\n");
	websWrite(wp, "<input name=\"%s_tls8021xpasswd\" type=\"password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("tls8021xpasswd", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.phase2)</script></div>\n");
	websWrite(wp, "<input name=\"%s_tls8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix, nvram_prefix_get("tls8021xphase2", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.servercertif)</script></div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xca\" name=\"%s_tls8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ", prefix, prefix);
	websWrite(wp, "var %s_tls8021xca = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xca", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_tls8021xca\").value = %s_tls8021xca;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.clientcertif)</script></div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xpem\" name=\"%s_tls8021xpem\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ", prefix, prefix);
	websWrite(wp, "var %s_tls8021xpem = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xpem", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_tls8021xpem\").value = %s_tls8021xpem;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.privatekey)</script></div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xprv\" name=\"%s_tls8021xprv\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ", prefix, prefix);
	websWrite(wp, "var %s_tls8021xprv = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xprv", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_tls8021xprv\").value = %s_tls8021xprv;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.options)</script></div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"3\" id=\"%s_tls8021xaddopt\" name=\"%s_tls8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ", prefix, prefix);
	websWrite(wp, "var %s_tls8021xaddopt = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xaddopt", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_tls8021xaddopt\").value = %s_tls8021xaddopt;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "</div>\n");
	websWrite(wp, "<script>\n//<![CDATA[\n ");
	// websWrite
	// (wp,"show_layer_ext(document.getElementsByName(\"%s_bridged\"),
	// \"%s_idnetvifs\", %s);\n",var, vvar, nvram_match (ssid, "0") ? "true"
	// : "false");
	char peap[32];

	sprintf(peap, "%s_8021xtype", prefix);
	websWrite(wp, "show_layer_ext(document.wpa.%s_8021xtype, 'idpeap%s', %s);\n", prefix, prefix, nvram_match(peap, "peap") ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.wpa.%s_8021xtype, 'idtls%s', %s);\n", prefix, prefix, nvram_match(peap, "tls") ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.wpa.%s_8021xtype, 'idleap%s', %s);\n", prefix, prefix, nvram_match(peap, "leap") ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");

}
#endif
#endif

void show_wparadius(webs_t wp, char *prefix)
{
	char var[80];

	websWrite(wp, "<div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.algorithms)</script></div>\n");
	websWrite(wp, "<select name=\"%s_crypto\">\n", prefix);
	sprintf(var, "%s_crypto", prefix);
	websWrite(wp, "<option value=\"tkip\" %s>TKIP</option>\n", selmatch(var, "tkip", "selected=\"selected\""));
	websWrite(wp, "<option value=\"aes\" %s>AES</option>\n", selmatch(var, "aes", "selected=\"selected\""));
	websWrite(wp, "<option value=\"tkip+aes\" %s>TKIP+AES</option>\n", selmatch(var, "tkip+aes", "selected=\"selected\""));
	websWrite(wp, "</select></div>\n");
#ifdef HAVE_MADWIFI
	show_radius(wp, prefix, 0, 1);
#else
	show_radius(wp, prefix, 0, 0);
#endif
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.rekey)</script></div>\n");
	sprintf(var, "%s_wpa_gtk_rekey", prefix);
	websWrite(wp, "<input name=\"%s_wpa_gtk_rekey\" maxlength=\"5\" size=\"10\" onblur=\"valid_range(this,0,99999,wpa.rekey)\" value=\"%s\" />", prefix, nvram_default_get(var, "3600"));
	websWrite(wp, "</div>\n");
	websWrite(wp, "</div>\n");
}

void show_wep(webs_t wp, char *prefix)
{
	char var[80];
	char *bit;

	cprintf("show wep\n");
#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880)
	char wl_authmode[16];

	sprintf(wl_authmode, "%s_authmode", prefix);
	nvram_default_get(wl_authmode, "open");
	if (nvram_invmatch(wl_authmode, "auto")) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label)</script></div>\n");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"open\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.openn)</script></input>&nbsp;\n",
			  wl_authmode, nvram_match(wl_authmode, "open") ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"shared\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.share_key)</script></input>\n",
			  wl_authmode, nvram_match(wl_authmode, "shared") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
	}
#endif
	websWrite(wp, "<div><div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wep.defkey)</script></div>");
	websWrite(wp, "<input type=\"hidden\" name=\"%s_WEP_key\" />", prefix);
	websWrite(wp, "<input type=\"hidden\" name=\"%s_wep\" value=\"restricted\" />", prefix);
	sprintf(var, "%s_key", prefix);
	nvram_default_get(var, "1");
	fprintf(stderr, "[WEP] default: %s\n", var);
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_key\" %s />1&nbsp;\n", prefix, selmatch(var, "1", "checked=\"checked\""));
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"2\" name=\"%s_key\" %s />2&nbsp;\n", prefix, selmatch(var, "2", "checked=\"checked\""));
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"3\" name=\"%s_key\" %s />3&nbsp;\n", prefix, selmatch(var, "3", "checked=\"checked\""));
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"4\" name=\"%s_key\" %s />4&nbsp;\n", prefix, selmatch(var, "4", "checked=\"checked\""));
	websWrite(wp, "</div>");
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.encrypt)</script></div>");

	sprintf(var, "%s_wep_bit", prefix);
	bit = nvram_safe_get(var);

	cprintf("bit %s\n", bit);

	websWrite(wp, "<select name=\"%s_wep_bit\" size=\"1\" onchange=keyMode(this.form)>", prefix);
	websWrite(wp, "<option value=\"64\" %s >64 bits 10 hex digits</option>", selmatch(var, "64", "selected=\"selected\""));
	websWrite(wp, "<option value=\"128\" %s >128 bits 26 hex digits</option>", selmatch(var, "128", "selected=\"selected\""));
	websWrite(wp, "</select>\n</div>\n<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wep.passphrase)</script></div>\n");
	websWrite(wp, "<input name=%s_passphrase maxlength=\"16\" size=\"20\" value=\"", prefix);

	char p_temp[128];
	char temp[256];

	sprintf(p_temp, "%s", get_wep_value(temp, "passphrase", bit, prefix));
	nvram_set("passphrase_temp", p_temp);
	if (strcmp(p_temp, nvram_safe_get("passphrase_temp"))) {
		fprintf(stderr, "[NVRAM WRITE ERROR] no match: \"%s\" -> \"%s\"\n", p_temp, nvram_safe_get("passphrase_temp"));
		websWrite(wp, "%s", p_temp);
	} else {
		tf_webWriteESCNV(wp, "passphrase_temp");
	}
	nvram_unset("passphrase_temp");

	websWrite(wp, "\" />");
	websWrite(wp, "<input type=\"hidden\" value=\"Null\" name=\"generateButton\" />\n");
	websWrite(wp, "<input class=\"button\" type=\"button\" value=\"Generate\" onclick=generateKey(this.form,\"%s\") name=wepGenerate />\n</div>", prefix);

	char *mlen = "10";
	char *mlen2 = "12";

	if (!strcmp(bit, "128")) {
		mlen = "26";
		mlen2 = "30";
	}
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 1</div>\n");
	websWrite(wp,
#ifdef HAVE_BUFFALO
		  "<input name=%s_key1 size=\"%s\" maxlength=\"%s\" value=\"%s\" onblur=\"valid_wep(this, 1)\" /></div>\n",
#else
		  "<input name=%s_key1 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
#endif
		  prefix, mlen2, mlen, nvram_nget("%s_key1", prefix));
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 2</div>\n");
	websWrite(wp,
#ifdef HAVE_BUFFALO
		  "<input name=%s_key2 size=\"%s\" maxlength=\"%s\" value=\"%s\" onblur=\"valid_wep(this, 1)\" /></div>\n",
#else
		  "<input name=%s_key2 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
#endif
		  prefix, mlen2, mlen, nvram_nget("%s_key2", prefix));
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 3</div>\n");
	websWrite(wp,
#ifdef HAVE_BUFFALO
		  "<input name=%s_key3 size=\"%s\" maxlength=\"%s\" value=\"%s\" onblur=\"valid_wep(this, 1)\" /></div>\n",
#else
		  "<input name=%s_key3 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
#endif
		  prefix, mlen2, mlen, nvram_nget("%s_key3", prefix));
	websWrite(wp, "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 4</div>\n");
	websWrite(wp,
#ifdef HAVE_BUFFALO
		  "<input name=%s_key4 size=\"%s\" maxlength=\"%s\" value=\"%s\" onblur=\"valid_wep(this, 1)\" /></div>\n",
#else
		  "<input name=%s_key4 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
#endif
		  prefix, mlen2, mlen, nvram_nget("%s_key4", prefix));
	websWrite(wp, "</div>\n");
}

void ej_show_defwpower(webs_t wp, int argc, char_t ** argv)
{
	switch (getRouterBrand()) {
	case ROUTER_ASUS_RTN10:
	case ROUTER_ASUS_RTN10U:
	case ROUTER_ASUS_RTN10PLUSD1:
	case ROUTER_ASUS_RTN12:
	case ROUTER_ASUS_RTN12B:
	case ROUTER_ASUS_RTN53:
	case ROUTER_ASUS_RTN16:
		websWrite(wp, "17");
		break;
	case ROUTER_ASUS_AC66U:
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
	case ROUTER_NETGEAR_R6300:
		websWrite(wp, "40");
		break;
	case ROUTER_LINKSYS_E4200:
		websWrite(wp, "100");
		break;
#ifndef HAVE_BUFFALO
	case ROUTER_BUFFALO_WHRG54S:
		if (nvram_match("DD_BOARD", "Buffalo WHR-HP-G54"))
			websWrite(wp, "28");
		else
			websWrite(wp, "71");
		break;
#endif
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		websWrite(wp, "28");
		break;
	default:
		websWrite(wp, "71");
		break;
	}
}

void ej_get_wds_mac(webs_t wp, int argc, char_t ** argv)
{
	int mac = -1, wds_idx = -1, mac_idx = -1;
	char *c, wds_var[32] = "";

	char *interface;

	if (ejArgs(argc, argv, "%d %d %s", &wds_idx, &mac_idx, &interface) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	} else if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
		return;
	else if (mac_idx < 0 || mac_idx > 5)
		return;

	snprintf(wds_var, 31, "%s_wds%d_hwaddr", interface, wds_idx);

	c = nvram_safe_get(wds_var);

	if (c) {
		mac = get_single_mac(c, mac_idx);
		websWrite(wp, "%02X", mac);
	} else
		websWrite(wp, "00");

	return;

}

void ej_showbridgesettings(webs_t wp, int argc, char_t ** argv)
{
	char *interface;
	int mcast;

#ifdef FASTWEB
	ejArgs(argc, argv, "%s %d", &interface, &mcast);
#else
	if (ejArgs(argc, argv, "%s %d", &interface, &mcast) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	showbridgesettings(wp, interface, mcast, 0);
}

void ej_get_wds_ip(webs_t wp, int argc, char_t ** argv)
{
	int ip = -1, wds_idx = -1, ip_idx = -1;
	char *c, wds_var[32] = "";

	char *interface;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d %d %s", &wds_idx, &ip_idx, &interface);
#else
	if (ejArgs(argc, argv, "%d %d %s", &wds_idx, &ip_idx, &interface) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
		return;
	else if (ip_idx < 0 || ip_idx > 3)
		return;

	snprintf(wds_var, 31, "%s_wds%d_ipaddr", interface, wds_idx);

	c = nvram_safe_get(wds_var);

	if (c) {
		ip = get_single_ip(c, ip_idx);
		websWrite(wp, "%d", ip);
	} else
		websWrite(wp, "0");

	return;

}

void ej_get_wds_netmask(webs_t wp, int argc, char_t ** argv)
{
	int nm = -1, wds_idx = -1, nm_idx = -1;
	char *c, wds_var[32] = "";

	char *interface;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d %d %s", &wds_idx, &nm_idx, &interface);
#else
	if (ejArgs(argc, argv, "%d %d %s", &wds_idx, &nm_idx, &interface) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	if (wds_idx < 1 || wds_idx > 6)
		return;
	else if (nm_idx < 0 || nm_idx > 3)
		return;

	snprintf(wds_var, 31, "%s_wds%d_netmask", interface, wds_idx);

	c = nvram_safe_get(wds_var);

	if (c) {
		nm = get_single_ip(c, nm_idx);
		websWrite(wp, "%d", nm);
	} else
		websWrite(wp, "255");

	return;

}

void ej_get_wds_gw(webs_t wp, int argc, char_t ** argv)
{
	int gw = -1, wds_idx = -1, gw_idx = -1;
	char *c, wds_var[32] = "";

	char *interface;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d %d %s", &wds_idx, &gw_idx, &interface);
#else
	if (ejArgs(argc, argv, "%d %d %s", &wds_idx, &gw_idx, &interface) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
		return;
	else if (gw_idx < 0 || gw_idx > 3)
		return;

	snprintf(wds_var, 31, "%s_wds%d_gw", interface, wds_idx);

	c = nvram_safe_get(wds_var);

	if (c) {
		gw = get_single_ip(c, gw_idx);
		websWrite(wp, "%d", gw);
	} else
		websWrite(wp, "0");

	return;

}

void ej_get_br1_ip(webs_t wp, int argc, char_t ** argv)
{
	int ip = -1, ip_idx = -1;
	char *c;

	char *interface;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d %s", &ip_idx, &interface);
#else
	if (ejArgs(argc, argv, "%d %s", &ip_idx, &interface) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (ip_idx < 0 || ip_idx > 3)
		return;
	char br1[32];

	sprintf(br1, "%s_br1_ipaddr", interface);
	c = nvram_safe_get(br1);

	if (c) {
		ip = get_single_ip(c, ip_idx);
		websWrite(wp, "%d", ip);
	} else
		websWrite(wp, "0");

	return;

}

void ej_get_br1_netmask(webs_t wp, int argc, char_t ** argv)
{
	int nm = -1, nm_idx = -1;
	char *c;

	char *interface;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d %s", &nm_idx, &interface);
#else
	if (ejArgs(argc, argv, "%d %s", &nm_idx, &interface) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (nm_idx < 0 || nm_idx > 3)
		return;
	char nms[32];

	sprintf(nms, "%s_br1_netmask", interface);
	c = nvram_safe_get(nms);

	if (c) {
		nm = get_single_ip(c, nm_idx);
		websWrite(wp, "%d", nm);
	} else
		websWrite(wp, "255");

	return;

}

void ej_get_uptime(webs_t wp, int argc, char_t ** argv)
{
	char line[256];
	FILE *fp;

	if ((fp = popen("uptime", "r"))) {
		fgets(line, sizeof(line), fp);
		line[strlen(line) - 1] = '\0';	// replace new line with null
#ifdef HAVE_ESPOD
		char *p;
		p = strtok(line, ",");
		if (p != NULL) {
			websWrite(wp, "%s<br>\n", p);
			p = strtok(NULL, "\0");
			websWrite(wp, "%s", p);
		}
#else
		websWrite(wp, "%s", line);
#endif
		pclose(fp);
	}
	return;
}

void ej_get_wan_uptime(webs_t wp, int argc, char_t ** argv)
{
	float sys_uptime;
	float uptime;
	int days, minutes;
	FILE *fp, *fp2;

	if (nvram_match("wan_proto", "disabled"))
		return;
	if (nvram_match("wan_ipaddr", "0.0.0.0")) {
		websWrite(wp, "%s", live_translate("status_router.notavail"));
		return;
	}
	if (!(fp = fopen("/tmp/.wanuptime", "r"))) {
		websWrite(wp, "%s", live_translate("status_router.notavail"));
		return;
	}
	if (!feof(fp) && fscanf(fp, "%f", &uptime) == 1) {
		fp2 = fopen("/proc/uptime", "r");
		fscanf(fp2, "%f", &sys_uptime);
		fclose(fp2);
		uptime = sys_uptime - uptime;
		days = (int)uptime / (60 * 60 * 24);
		if (days)
			websWrite(wp, "%d day%s, ", days, (days == 1 ? "" : "s"));
		minutes = (int)uptime / 60;
		websWrite(wp, "%d:%02d:%02d", (minutes / 60) % 24, minutes % 60, (int)uptime % 60);
	}
	fclose(fp);

	return;

}

void ej_get_wdsp2p(webs_t wp, int argc, char_t ** argv)
{
	int index = -1, ip[4] = { 0, 0, 0, 0 }, netmask[4] = {
	0, 0, 0, 0};
	char nvramvar[32] = { 0 };
	char *interface;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d %s", &index, &interface);
#else
	if (ejArgs(argc, argv, "%d %s", &index, &interface) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	char wlwds[32];

	sprintf(wlwds, "%s_wds1_enable", interface);
	if (nvram_selmatch(wp, "wk_mode", "ospf") && nvram_selmatch(wp, "expert_mode", "1") && nvram_selmatch(wp, wlwds, "1")) {
		char buf[16];

		sprintf(buf, "%s_wds%d_ospf", interface, index);
		websWrite(wp, "<input name=\"%s\" size=\"2\" maxlength=\"5\" value=\"%s\" />\n", buf, nvram_safe_get(buf));
	}

	snprintf(nvramvar, 31, "%s_wds%d_ipaddr", interface, index);
	sscanf(nvram_safe_get(nvramvar), "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
	snprintf(nvramvar, 31, "%s_wds%d_netmask", interface, index);
	sscanf(nvram_safe_get(nvramvar), "%d.%d.%d.%d", &netmask[0], &netmask[1], &netmask[2], &netmask[3]);
	snprintf(nvramvar, 31, "%s_wds%d_enable", interface, index);

	// set netmask to a suggested default if blank
	if (netmask[0] == 0 && netmask[1] == 0 && netmask[2] == 0 && netmask[3] == 0) {
		netmask[0] = 255;
		netmask[1] = 255;
		netmask[2] = 255;
		netmask[3] = 252;
	}

	if (nvram_match(nvramvar, "1")) {
		websWrite(wp, "\
	<div class=\"setting\">\n\
	          <input type=\"hidden\" name=\"%s_wds%d_ipaddr\" value=\"4\">\n\
	          <div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n\
	          <input size=\"3\" maxlength=\"3\" name=\"%s_wds%d_ipaddr0\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_wds%d_ipaddr1\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_wds%d_ipaddr2\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_wds%d_ipaddr3\" value=\"%d\" onblur=\"valid_range(this,1,254,'IP')\" class=\"num\">\n\
       </div>\n", interface, index, interface, index, ip[0], interface, index, ip[1], interface, index, ip[2], interface, index, ip[3]);

		websWrite(wp, "\
       	  <div class=\"setting\">\n\
       	  <div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n\
	  <input type=\"hidden\" name=\"%s_wds%d_netmask\" value=\"4\">\n\
	  <input name=\"%s_wds%d_netmask0\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>.<input name=\"%s_wds%d_netmask1\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>.<input name=\"%s_wds%d_netmask2\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>.<input name=\"%s_wds%d_netmask3\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>\n\
          </div>\n", interface, index, interface, index, netmask[0], interface, index, netmask[1], interface, index, netmask[2], interface, index, netmask[3]);

	}

	return;

}

void ej_get_clone_wmac(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_RB500
	return 0;
#else

	char *c;
	int mac, which;
	int dofree = 0;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d", &which);
#else
	if (ejArgs(argc, argv, "%d", &which) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	if (nvram_match("def_whwaddr", "00:00:00:00:00:00")) {
		// if (strlen (nvram_safe_get ("il0macaddr")) == 0)
		// {
		// if (nvram_match ("port_swap", "1"))
		// c = strdup (nvram_safe_get ("et1macaddr"));
		// else
		// c = strdup (nvram_safe_get ("et0macaddr"));
		// MAC_ADD (c);
		// }
		// else
		// {
		// c = strdup (nvram_safe_get ("il0macaddr"));
		// }
		// dofree = 1;

		if (nvram_match("port_swap", "1")) {
			if (strlen(nvram_safe_get("et1macaddr")) != 0)	// safe:
				// maybe
				// et1macaddr 
				// not there?
			{
				c = strdup(nvram_safe_get("et1macaddr"));
			} else {
				c = strdup(nvram_safe_get("et0macaddr"));
				MAC_ADD(c);	// et0macaddr +3
			}
		} else {
			c = strdup(nvram_safe_get("et0macaddr"));
		}

		dofree = 1;
		if (c) {
			MAC_ADD(c);
			MAC_ADD(c);
		}

	} else
		c = nvram_safe_get("def_whwaddr");

	if (c) {
		mac = get_single_mac(c, which);
		websWrite(wp, "%02X", mac);
		if (dofree)
			free(c);
	} else
		websWrite(wp, "00");

	return;
#endif
}

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

	int i, j, vlans[22][6], tmp, wl_br;
	char *c, *next, buff[32], portvlan[32];
	int a;

	for (i = 0; i < 22; i++)
		for (j = 0; j < 6; j++)
			vlans[i][j] = -1;

	wl_br = -1;

	for (i = 0; i < 8; i++) {
		if (i < 5)
			snprintf(buff, 31, "port%dvlans", i);
		else if (i == 5)
			snprintf(buff, 31, "%s", "lan_ifnames");
		else
			snprintf(buff, 31, "ub%d_ifnames", i - 5);

		c = nvram_safe_get(buff);

		if (c) {
			foreach(portvlan, c, next) {
				if (portvlan[0] == 'e' && portvlan[1] == 't' && portvlan[2] == 'h' && portvlan[3] == '1')
					wl_br = i - 5;
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

					if (i < 5) {
						vlans[tmp][i] = 1;
					} else {
						vlans[tmp][5] = i - 5;
					}
				}
			}
		}
	}

	// Status header
	char status[32];

	websWrite(wp, "              <tr>\n");
	websWrite(wp, "<td><script type=\"text/javascript\">Capture(vlan.linkstatus)</script></td>\n");

	char *ifname = "eth0";
	if (f_exists("/proc/switch/eth0/enable"))
		ifname = "eth0";
	if (f_exists("/proc/switch/eth1/enable"))
		ifname = "eth1";
	if (f_exists("/proc/switch/eth2/enable"))
		ifname = "eth2";
	char portstatus[64];
	int vlanmap[6] = { 0, 1, 2, 3, 4, 5 };	// 0=wan; 1,2,3,4=lan; 5=internal 
	getPortMapping(vlanmap);

	for (a = 0; a < 5; a++) {
		snprintf(portstatus, sizeof(portstatus), "/proc/switch/%s/port/%d/status", ifname, vlanmap[a]);
		char cstatus[32];
		memset(cstatus, 0, 32);
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

	int hasgiga = 1;

	for (a = 0; a < 21 + hasgiga; a++) {
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
			websWrite(wp, "<script type=\"text/javascript\">Capture(vlan.negociate)</script>");
			break;
		case 18:
			websWrite(wp, "Full Speed");
			break;
		case 19:
			websWrite(wp, "Full Duplex");
			break;
		case 20:
			websWrite(wp, "Enabled");
			break;
		case 21:
			websWrite(wp, "Gigabit");
			break;
		default:
			snprintf(buff, 31, "%d", i);
			websWrite(wp, buff);
			break;
		}

		websWrite(wp, "</td>\n");

		for (j = 0; j < 5; j++) {
			snprintf(buff, 31, "\"port%dvlan%d\"", j, i);
			websWrite(wp, "<td");

			if (j % 2 == 0)
				websWrite(wp, " bgcolor=\"#CCCCCC\"");

			websWrite(wp, " height=\"20\"><div align=\"center\"><input type=\"checkbox\" value=\"on\" name=");
			websWrite(wp, buff);

			if (i < 17 || i > 21) {
				if (vlans[i][j] == 1)
					websWrite(wp, " checked=\"checked\"");
			} else {
				if (vlans[i][j] == -1)
					websWrite(wp, " checked=\"checked\"");
			}

			if (i < 17) {
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
			if (vlans[i][5] < 0)
				websWrite(wp, " selected=\\\"selected\\\"");
			websWrite(wp, ">\" + share.none + \"</option>\");\n//]]>\n</script><option value=\"0\"");
			if (vlans[i][5] == 0)
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

	return;
}

/*
 * Note: VLAN #16 designates tagging.  There is no VLAN #16 (only 0-15) 
 */

void ej_show_qos_aqd(webs_t wp, int argc, char_t ** argv)
{
#if defined(HAVE_CODEL) || defined(HAVE_FQ_CODEL)
	char *aqd = nvram_safe_get("svqos_aqd");

	websWrite(wp, "<div class=\"setting\">\n\
                     <div class=\"label\"><script type=\"text/javascript\">Capture(qos.aqd)</script></div>\n\
                     <select name=\"qos_aqd\">\n\
                        <option value=\"sfq\" %s><script type=\"text/javascript\">Capture(qos.aqd_sfq)</script></option>\n", strcmp(aqd, "sfq") == 0 ? "selected" : "");
#ifdef HAVE_CODEL
	websWrite(wp, "     <option value=\"codel\" %s><script type=\"text/javascript\">Capture(qos.aqd_codel)</script></option>\n", strcmp(aqd, "codel") == 0 ? "selected" : "");
#endif
#ifdef HAVE_FQ_CODEL
	websWrite(wp, "     <option value=\"fq_codel\" %s><script type=\"text/javascript\">Capture(qos.aqd_fqcodel)</script></option>\n", strcmp(aqd, "fq_codel") == 0 ? "selected" : "");
#endif

	websWrite(wp, "  </select>\n\
                   </div>\n");

#else
	websWrite(wp, "<input type=\"hidden\" name=\"qos_aqd\" \>");
#endif
}

void ej_get_qospkts(webs_t wp, int argc, char_t ** argv)
{
	char *qos_pkts = nvram_safe_get("svqos_pkts");
	char pkt_filter[4];

	websWrite(wp, "<tr>\n\
                    <td><input type=\"checkbox\" name=\"svqos_pktack\" value=\"ACK\" %s><script type=\"text/javascript\">Capture(qos.pktack)</script></input></td>\n\
                    <td><input type=\"checkbox\" name=\"svqos_pktsyn\" value=\"SYN\" %s><script type=\"text/javascript\">Capture(qos.pktsyn)</script></input></td>\n\
                    <td><input type=\"checkbox\" name=\"svqos_pktfin\" value=\"FIN\" %s><script type=\"text/javascript\">Capture(qos.pktfin)</script></input></td>\n\
                    <td><input type=\"checkbox\" name=\"svqos_pktrst\" value=\"RST\" %s><script type=\"text/javascript\">Capture(qos.pktrst)</script></input></td>\n\
                  </tr>\n", strstr(qos_pkts, "ACK") ? "checked" : "", strstr(qos_pkts, "SYN") ? "checked" : "", strstr(qos_pkts, "FIN") ? "checked" : "", strstr(qos_pkts, "RST") ? "checked" : "");
}

void ej_get_qossvcs(webs_t wp, int argc, char_t ** argv)
{
	char *qos_svcs = nvram_safe_get("svqos_svcs");
	char name[32], type[32], data[32], level[32];
	int no_svcs = 0, i = 0;

	// calc # of services
	// no_svcs = strspn(qos_svcs,"|");

	while ((qos_svcs = strpbrk(qos_svcs, "|"))) {
		no_svcs++;
		qos_svcs++;
	}

	// write HTML data

	websWrite(wp, "<tr><td colspan=\"3\"><input type=\"hidden\" name=\"svqos_nosvcs\" value=\"%d\" /></td></tr>", no_svcs);

	qos_svcs = nvram_safe_get("svqos_svcs");

	/*
	 * services format is "name type data level | name type data level |"
	 * ..etc 
	 */
	for (i = 0; i < no_svcs && qos_svcs && qos_svcs[0]; i++) {
		if (sscanf(qos_svcs, "%31s %31s %31s %31s ", name, type, data, level) < 4)
			break;

		websWrite(wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_svcdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_svcname%d\" value=\"%s\" />\n\
						<input type=\"hidden\" name=\"svqos_svctype%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td >\n", i, i, name, i, type, name);
		websWrite(wp, "<select name=\"svqos_svcprio%d\"> \n\
							<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n\
							document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n\
							document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n\
							document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n\
							document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n</script>\n\
						</select>\n\
		           </td>\n\
				</tr>\n", i, strcmp(level, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "10") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "40") == 0 ? "selected=\\\"selected\\\"" : "");

		qos_svcs = strpbrk(++qos_svcs, "|");
		qos_svcs++;

	}

	return;
}

#ifndef HAVE_AQOS
void ej_get_qosips(webs_t wp, int argc, char_t ** argv)
{
	char *qos_ips = nvram_safe_get("svqos_ips");
	char ip[32], level[32];
	int no_ips = 0, i = 0;

	// calc # of ips
	while ((qos_ips = strpbrk(qos_ips, "|"))) {
		no_ips++;
		qos_ips++;
	}
	websWrite(wp, "<tr>\n\
  					<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.ipmask)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(share.priority)</script></th>\n\
  				</tr>\n");

	// write HTML data

	websWrite(wp, "<tr><td colspan=\"3\"><input type=\"hidden\" name=\"svqos_noips\" value=\"%d\" /></td></tr>", no_ips);

	qos_ips = nvram_safe_get("svqos_ips");

	/*
	 * IP format is "data level | data level |" ..etc 
	 */
	for (i = 0; i < no_ips && qos_ips && qos_ips[0]; i++) {
		if (sscanf(qos_ips, "%31s %31s ", ip, level) < 2)
			break;

		websWrite(wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_ipdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_ip%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td>\n", i, i, ip, ip);
		websWrite(wp, "<select name=\"svqos_ipprio%d\"> \n\
						<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n\
							document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n\
							document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n\
							document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n\
							document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n</script>\n\
					</select>\n\
				  </td>\n\
				</tr>\n", i, strcmp(level, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "10") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "40") == 0 ? "selected=\\\"selected\\\"" : "");

		qos_ips = strpbrk(++qos_ips, "|");
		qos_ips++;

	}

	return;
}
#else
void ej_get_qosips(webs_t wp, int argc, char_t ** argv)
{
	char *qos_ips = nvram_safe_get("svqos_ips");
	char ip[32], level[32], level2[32], lanlevel[32], prio[32];
	int no_ips = 0, i = 0;

	// calc # of ips
	while ((qos_ips = strpbrk(qos_ips, "|"))) {
		no_ips++;
		qos_ips++;
	}
	websWrite(wp, "<tr>\n\
  					<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.ipmask)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.maxuprate_b)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.maxdownrate_b)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.maxlanrate_b)</script></th>\n\
					<th><script type=\"text/javascript\">Capture(share.priority)</script></th>\n\
  				</tr>\n");

	// write HTML data

	websWrite(wp, "<tr><td colspan=\"3\"><input type=\"hidden\" name=\"svqos_noips\" value=\"%d\" /></td></tr>", no_ips);

	qos_ips = nvram_safe_get("svqos_ips");

	/*
	 * IP format is "data level | data level |" ..etc 
	 */

	lanlevel[0] = '\0';
	prio[0] = '\0';
	for (i = 0; i < no_ips && qos_ips && qos_ips[0]; i++) {
		if (sscanf(qos_ips, "%31s %31s %31s", ip, level, level2) < 3)
			break;
		if (sscanf(qos_ips, "%31s %31s %31s %31s %31s", ip, level, level2, lanlevel, prio)) {
			if (!strcmp(lanlevel, "|")) {
				strcpy(lanlevel, "0");
				strcpy(prio, "0");
			}
			if (!strcmp(prio, "|"))
				strcpy(prio, "0");
		}

		websWrite(wp, "<tr>\n\
						<td align=\"center\">\n\
							<input type=\"checkbox\" name=\"svqos_ipdel%d\" />\n\
							<input type=\"hidden\" name=\"svqos_ip%d\" value=\"%s\" />\n\
						</td>\n", i, i, ip);
		websWrite(wp, "	<td><em>%s</em></td>\n", ip);
		websWrite(wp, "	<td nowrap>\n\
							<input class=\"num\" name=\"svqos_ipup%d\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n\
				  </td>\n", i, level, strcmp(prio, "0") == 0 ? "" : "disabled");

		websWrite(wp, "	<td nowrap>\n\
							<input name=\"svqos_ipdown%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n\
						</td>\n", i, level2, strcmp(prio, "0") == 0 ? "" : "disabled");
		websWrite(wp, "	<td nowrap>\n\
							<input name=\"svqos_iplanlvl%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n\
						</td>\n", i, lanlevel, strcmp(prio, "0") == 0 ? "" : "disabled");

		websWrite(wp, "	<td>\n\
							<select name=\"svqos_ipprio%d\" onChange=\"iplvl_grey(%d,this,this.form,false)\"> \n\
								<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\" %s >\" + qos.prio_m + \"</option>\");\n\
									document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n\
									document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n\
									document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n\
									document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n\
									document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n\
								</script>\n\
							</select>\n\
						</td>\n\
					</tr>\n", i, i, strcmp(prio, "0") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "10") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "40") == 0 ? "selected=\\\"selected\\\"" : "");

		qos_ips = strpbrk(++qos_ips, "|");
		qos_ips++;

	}

	return;
}
#endif
#ifndef HAVE_AQOS
void ej_get_qosmacs(webs_t wp, int argc, char_t ** argv)
{
	char *qos_macs = nvram_safe_get("svqos_macs");
	char mac[32], level[32];
	int no_macs = 0, i = 0;

	// calc # of ips
	while ((qos_macs = strpbrk(qos_macs, "|"))) {
		no_macs++;
		qos_macs++;
	}

	websWrite(wp, "<tr>\n\
  					<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(share.mac)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(share.priority)</script></th>\n\
  				</tr>\n");

	// write HTML data
	websWrite(wp, "<input type=\"hidden\" name=\"svqos_nomacs\" value=\"%d\" />", no_macs);

	qos_macs = nvram_safe_get("svqos_macs");

	/*
	 * IP format is "data level | data level |" ..etc 
	 */
	for (i = 0; i < no_macs && qos_macs && qos_macs[0]; i++) {
		if (sscanf(qos_macs, "%31s %31s ", mac, level) < 2)
			break;

		websWrite(wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_macdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_mac%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td>\n", i, i, mac, mac);
		websWrite(wp, "<select name=\"svqos_macprio%d\"> \n\
							<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n\
							document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n\
							document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n\
							document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n\
							document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n</script>\n\
						</select>\n\
					</td>\n\
				</tr>\n", i, strcmp(level, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "10") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(level, "40") == 0 ? "selected=\\\"selected\\\"" : "");

		qos_macs = strpbrk(++qos_macs, "|");
		qos_macs++;

	}

	return;
}

#else
void ej_get_qosmacs(webs_t wp, int argc, char_t ** argv)
{
	char *qos_macs = nvram_safe_get("svqos_macs");
	char mac[32], level[32], level2[32], lanlevel[32], user[32], prio[32];
	int no_macs = 0, i = 0;

	// calc # of ips
	while ((qos_macs = strpbrk(qos_macs, "|"))) {
		no_macs++;
		qos_macs++;
	}
	websWrite(wp, "<tr>\n\
  					<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(share.mac)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.maxuprate_b)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.maxdownrate_b)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.maxlanrate_b)</script></th>\n\
					<th><script type=\"text/javascript\">Capture(share.priority)</script></th>\n\
  				</tr>\n");

	// write HTML data
	websWrite(wp, "<input type=\"hidden\" name=\"svqos_nomacs\" value=\"%d\" />", no_macs);

	qos_macs = nvram_safe_get("svqos_macs");

	lanlevel[0] = '\0';
	prio[0] = '\0';
	for (i = 0; i < no_macs && qos_macs && qos_macs[0]; i++) {
		if (sscanf(qos_macs, "%31s %31s %31s %31s ", mac, level, level2, user) < 4)
			break;
		if (sscanf(qos_macs, "%31s %31s %31s %31s %31s %31s", mac, level, level2, user, lanlevel, prio)) {
			if (!strcmp(lanlevel, "|")) {
				strcpy(lanlevel, "0");
				strcpy(prio, "0");
			}
			if (!strcmp(prio, "|"))
				strcpy(prio, "0");
		}

		websWrite(wp, "<tr>\n\
					<td align=\"center\">\n\
						<input type=\"checkbox\" name=\"svqos_macdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_mac%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td nowrap>\n\
						<input name=\"svqos_macup%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n\
					</td>\n\
					<td nowrap>\n\
						<input name=\"svqos_macdown%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n\
					</td>\n\
					<td nowrap>\n\
						<input name=\"svqos_maclanlvl%d\" class=\"num\" size=\"5\" maxlength=\"6\" value=\"%s\" style=\"text-align:right;\" %s /> kBits\n\
					</td>\n\
					<td>\n\
						<select name=\"svqos_macprio%d\" onChange=\"maclvl_grey(%d,this,this.form,false)\"> \n\
							<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\" %s >\" + qos.prio_m + \"</option>\");\n\
								document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");\n\
								document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");\n\
								document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");\n\
								document.write(\"<option value=\\\"30\\\" %s >\" + share.standard + \"</option>\");\n\
								document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");\n//]]>\n</script>\n\
						</select>\n\
					</td>\n\
				</tr>\n", i, i, mac, mac, i, level, strcmp(prio, "0") == 0 ? "" : "disabled", i, level2, strcmp(prio, "0") == 0 ? "" : "disabled", i, lanlevel, strcmp(prio, "0") == 0 ? "" : "disabled", i, i, strcmp(prio, "0") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "10") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp(prio, "40") == 0 ? "selected=\\\"selected\\\"" : "");

		qos_macs = strpbrk(++qos_macs, "|");
		qos_macs++;

	}

	return;
}
#endif

/*
 * Added by Botho 03.April.06 
 */
void ej_dumpip_conntrack(webs_t wp, int argc, char_t ** argv)
{
	int ip_count = 0;
	FILE *fp;
	int c;

	fp = fopen("/proc/net/ip_conntrack", "rb");
	if (fp == NULL)
		return;
	while (!feof(fp)) {
		c = getc(fp);
		if (c == EOF)
			break;
		if (c == 0xa)
			ip_count++;
	}

	websWrite(wp, "%d", ip_count);

	fclose(fp);

	return;
}

/*
 * Added by Botho 28.Oct.06 
 */
static int search_hit(char *search, char *line, char *ret)
{
	unsigned int searchLen;
	unsigned int i;
	unsigned int j;
	unsigned int lineLen;

	if (line == NULL || search == NULL || ret == NULL)
		return 1;
	lineLen = strlen(line);
	searchLen = strlen(search);

	if (searchLen > lineLen)
		return (1);	// this can't match, invalid data?

	for (i = 0; i < lineLen - searchLen; i++) {
		if (!strncasecmp((char *)&line[i], search, searchLen))
			break;	// we got hit
	}

	for (j = i + searchLen; j < i + 15 + searchLen; j++) {
		if (j >= lineLen)
			break;	// end of line may be a delimiter too
		// return(1); // incomplete data
		if (line[j] == ' ')
			break;	// we reach _space_ delimiter
	}
	memcpy(ret, &line[i + searchLen], j - (i + searchLen));
	ret[j - (i + searchLen)] = 0;
	return (0);
}

static int string_search(char *string, char *search)
{
	int searchLen;
	int i;

	if (search == NULL)
		return 0;
	searchLen = strlen(search);
	if (string == NULL)
		return 0;
	if (searchLen > strlen(string)) {
		return (0);	// this can't match
	}
	int slen = strlen(string);

	for (i = 0; i < slen - searchLen; i++) {	// +1 removed.
		if (!strncasecmp((char *)&string[i], search, searchLen)) {
			return (1);	// we got hit
		}
	}
	return (0);
}

void ej_ip_conntrack_table(webs_t wp, int argc, char_t ** argv)
{
	FILE *fp;
	int ip_count = 1;
	char line[512];
	char protocol[16] = "";
	int timeout = 0;
	char srcip[16] = "";
	char dstip[16] = "";
	int _dport;
	struct servent *servp;
	char dstport[6] = "";
	char state[12] = "";
	char dum1[32];
	int dum2;
	char *lanip = nvram_get("lan_ipaddr");

	fp = fopen("/proc/net/ip_conntrack", "rb");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp) != NULL) {

		websWrite(wp, "<tr>\n");

		// Nb
		websWrite(wp, "<td align=\"right\">%d</td>", ip_count);

		// Proto
		if (string_search(line, "tcp"))
			sprintf(protocol, "TCP");
		else if (string_search(line, "udp"))
			sprintf(protocol, "UDP");
		else if (string_search(line, "icmp"))
			sprintf(protocol, "ICMP");
		else
			sprintf(protocol, live_translate("share.unknown"));
		websWrite(wp, "<td>%s</td>", protocol);

		// Timeout
		sscanf(line, "%s %d %d", &dum1[0], &dum2, &timeout);
		websWrite(wp, "<td align=\"right\">%d</td>", timeout);

		// src
		search_hit("src=", line, srcip);
		// char buf[200];
		// getHostName (buf, srcip);
		// websWrite (wp, "<td align=\"right\" onmouseover='DisplayDiv(this,
		// event, 15, 15, \"%s\")' onmouseout=\"unDisplayDiv()\">%s</td>",
		// buf != "unknown" ? buf : live_translate ("share.unknown") ,
		// srcip);
		if (!strcmp(srcip, lanip))
			websWrite(wp, "<td align=\"right\">%s</td>", srcip);
		else
			websWrite(wp, "<td align=\"right\"><a title=\"Geotool\" href=\"javascript:openGeotool('%s')\">%s</a></td>", srcip, srcip);

		// dst
		search_hit("dst=", line, dstip);
		// getHostName (buf, dstip);
		// websWrite (wp, "<td align=\"right\" onmouseover='DisplayDiv(this,
		// event, 15, 15, \"%s\")' onmouseout=\"unDisplayDiv()\">%s</td>",
		// buf != "unknown" ? buf : live_translate ("share.unknown") ,
		// dstip);
		if (!strcmp(dstip, lanip))
			websWrite(wp, "<td align=\"right\">%s</td>", dstip);
		else
			websWrite(wp, "<td align=\"right\"><a title=\"Geotool\" href=\"javascript:openGeotool('%s')\">%s</a></td>", dstip, dstip);

		// service
		search_hit("dport=", line, dstport);
		_dport = atoi(dstport);
		servp = my_getservbyport(htons(_dport), protocol);
		websWrite(wp, "<td align=\"right\">%s</td>", servp ? servp->s_name : dstport);

		// State
		if (string_search(line, "ESTABLISHED"))
			sprintf(state, "ESTABLISHED");
		else if (string_search(line, "TIME_WAIT"))
			sprintf(state, "TIME_WAIT");
		else if (string_search(line, "UNREPLIED"))
			sprintf(state, "UNREPLIED");
		else if (string_search(line, "CLOSE"))
			sprintf(state, "CLOSE");
		else if (string_search(line, "ASSURED"))
			sprintf(state, "ASSURED");
		else {
			if (string_search(line, "udp"))
				sprintf(state, "UNREPLIED");
			else
				sprintf(state, "&nbsp;");
		}
		websWrite(wp, "<td>%s</td>\n", state);
		websWrite(wp, "</tr>\n");

		ip_count++;
	}

	fclose(fp);

	return;
}

void ej_gethostnamebyip(webs_t wp, int argc, char_t ** argv)
{
	char buf[200];
	char *argument;

#ifdef FASTWEB
	ejArgs(argc, argv, "%s", &argument);
#else
	if (ejArgs(argc, argv, "%s", &argument) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	if (argc == 1) {
		getHostName(buf, argument);
		websWrite(wp, "%s", strcmp(buf, "unknown") ? buf : live_translate("share.unknown"));
	}

	return;
}

/*
 * BEGIN Added by Botho 10.May.06 
 */
void ej_show_wan_to_switch(webs_t wp, int argc, char_t ** argv)
{

	if (nvram_match("wan_proto", "disabled") || getSTA() || getWET())	// WAN 
		// disabled 
		// OR 
		// Wirelles 
		// is 
		// not 
		// AP
	{
		websWrite(wp, "<fieldset>\n\
							<legend><script type=\"text/javascript\">Capture(idx.legend2)</script></legend>\n\
								<div class=\"setting\">\n\
									<div class=\"label\"><script type=\"text/javascript\">Capture(idx.wantoswitch)</script></div>\n\
									<input class=\"spaceradio\" type=\"checkbox\" name=\"_fullswitch\" value=\"1\" %s />\n\
								</div>\n\
							</fieldset><br />\n", nvram_match("fullswitch", "1") ? "checked=\"checked\"" : "");
	}

	return;
}

#define PROC_DEV "/proc/net/dev"

void ej_wl_packet_get(webs_t wp, int argc, char_t ** argv)
{
	char line[256];
	FILE *fp;

#ifdef HAVE_MADWIFI
	char *ifname = nvram_safe_get("wifi_display");
#elif HAVE_RT2880
	char *ifname = "ra0";
#else
	char name[32];
	sprintf(name, "%s_ifname", nvram_safe_get("wifi_display"));
	char *ifname = nvram_safe_get(name);
#endif
	struct dev_info {
		// unsigned long rx_bytes;
		unsigned long rx_pks;
		unsigned long rx_errs;
		unsigned long rx_drops;
		// unsigned long rx_fifo;
		// unsigned long rx_frame;
		// unsigned long rx_com;
		// unsigned long rx_mcast;
		// unsigned long tx_bytes;
		unsigned long tx_pks;
		unsigned long tx_errs;
		unsigned long tx_drops;
		// unsigned long tx_fifo;
		unsigned long tx_colls;
		// unsigned long tx_carr;
		// unsigned long tx_com;
	} info;

	info.rx_pks = info.rx_errs = info.rx_drops = 0;
	info.tx_pks = info.tx_errs = info.tx_drops = info.tx_colls = 0;

	if ((fp = fopen(PROC_DEV, "r")) == NULL) {
		return;
	} else {
		/*
		 * Inter-| Receive | Transmit face |bytes packets errs drop fifo
		 * frame compressed multicast|bytes packets errs drop fifo colls
		 * carrier compressed lo: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 eth0:
		 * 674829 5501 0 0 0 0 0 0 1249130 1831 0 0 0 0 0 0 eth1: 0 0 0 0 0 0 
		 * 0 0 0 0 0 0 0 0 0 0 eth2: 0 0 0 0 0 719 0 0 1974 16 295 0 0 0 0 0
		 * br0: 107114 1078 0 0 0 0 0 0 910094 1304 0 0 0 0 0 0
		 * 
		 */
		while (fgets(line, sizeof(line), fp) != NULL) {
			int ifl = 0;

			if (!strchr(line, ':'))
				continue;
			while (line[ifl] != ':')
				ifl++;
			line[ifl] = 0;	/* interface */
			char ifnamecopy[32];
			int l = 0;
			int i;
			int len = strlen(line);
			for (i = 0; i < len; i++) {
				if (line[i] == ' ')
					continue;
				ifnamecopy[l++] = line[i];
			}
			ifnamecopy[l] = 0;
			if (!strcmp(ifnamecopy, ifname)) {
				/*
				 * sscanf (line + ifl + 1, "%ld %ld %ld %ld %ld %ld %ld %ld
				 * %ld %ld %ld %ld %ld %ld %ld %ld", &info.rx_bytes,
				 * &info.rx_pks, &info.rx_errs, &info.rx_drops,
				 * &info.rx_fifo, &info.rx_frame, &info.rx_com,
				 * &info.rx_mcast, &info.tx_bytes, &info.tx_pks,
				 * &info.tx_errs, &info.tx_drops, &info.tx_fifo,
				 * &info.tx_colls, &info.tx_carr, &info.tx_com); 
				 */
				sscanf(line + ifl + 1,
				       "%*ld %ld %ld %ld %*ld %*ld %*ld %*ld %*ld %ld %ld %ld %*ld %ld %*ld %*ld",
				       &info.rx_pks, &info.rx_errs, &info.rx_drops, &info.tx_pks, &info.tx_errs, &info.tx_drops, &info.tx_colls);
			}

		}
		fclose(fp);
	}

	websWrite(wp, "SWRXgoodPacket=%ld;", info.rx_pks);
	websWrite(wp, "SWRXerrorPacket=%ld;", info.rx_errs + info.rx_drops);

	websWrite(wp, "SWTXgoodPacket=%ld;", info.tx_pks);
	websWrite(wp, "SWTXerrorPacket=%ld;", info.tx_errs + info.tx_drops + info.tx_colls);

	return;
}

/*
 * END Added by Botho 10.May.06 
 */

void ej_statfs(webs_t wp, int argc, char_t ** argv)
{
	struct statfs sizefs;

	if (argc != 2)
		return;

	if ((statfs(argv[0], &sizefs) != 0)
	    || (sizefs.f_type == 0x73717368))
		memset(&sizefs, 0, sizeof(sizefs));

	websWrite(wp, "var %s = {\n\
  	free: %llu,\n\
  	used: %llu,\n\
  	size: %llu\n\
  	};\n", argv[1], ((uint64_t) sizefs.f_bsize * sizefs.f_bfree), ((uint64_t) sizefs.f_bsize * (sizefs.f_blocks - sizefs.f_bfree)), ((uint64_t) sizefs.f_bsize * sizefs.f_blocks));
}

void ej_statnv(webs_t wp, int argc, char_t ** argv)
{
	int space = 0;
	int used = nvram_used(&space);

	websWrite(wp, "%.2f KB / %d KB", (float)used / 1024, space / 1024);

}

#ifdef HAVE_RSTATS
/*
 * 
 * rstats Copyright (C) 2006 Jonathan Zarate
 * 
 * Licensed under GNU GPL v2 or later.
 * 
 */

void ej_bandwidth(webs_t wp, int argc, char_t ** argv)
{
	char *name;
	int sig;
	char *argument;

#ifdef FASTWEB
	ejArgs(argc, argv, "%s", &argument);
#else
	if (ejArgs(argc, argv, "%s", &argument) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	if (argc == 1) {
		if (strcmp(argument, "speed") == 0) {
			sig = SIGUSR1;
			name = "/var/spool/rstats-speed.js";
		} else {
			sig = SIGUSR2;
			name = "/var/spool/rstats-history.js";
		}
		unlink(name);
		killall("rstats", sig);
		wait_file_exists(name, 5, 0);
		do_file(name, wp, NULL);
		unlink(name);
	}
}
#endif

#ifdef HAVE_PORTSETUP
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

		websWrite(wp, "<option value=\"\" %s >Disabled</option>\n", strlen(wanifname) == 0 ? "selected=\"selected\"" : "");
		foreach(var, eths, next) {
			websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, !strcmp(wanifname, var) ? "selected=\"selected\"" : "", var);
		}
		websWrite(wp, "</select></div>\n");
	}
	memset(bufferif, 0, 256);
	getIfList(bufferif, "br");
	foreach(var, eths, next) {
		if (!strcmp("etherip0", var))
			continue;
		if (strchr(var, '.') == NULL) {
			if (!strcmp(get_wan_face(), var))
				continue;
			if (!strcmp(nvram_safe_get("lan_ifname"), var))
				continue;
			foreach(bword, bufferif, bnext) {
				if (!strcmp(bword, var)) {
					goto skip;
				}
			}
		}

		char layer[64];
		strcpy(layer, var);
		rep(layer, '.', 'X');
		sprintf(ssid, "%s_bridged", var);
		// nvram_nset("0", "%s_bridged", var);
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.network)</script> %s</div>\n", var);
		// qlen here

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\">%s</div>\n", live_translate("idx.txqlen"));
		char txq[32];
		sprintf(txq, "%s_txq", var);
		websWrite(wp, "<input class=\"num\" maxlength=\"4\" onblur=\"valid_range(this,0,10000,idx.txqlen)\" size=\"5\" name=\"%s\" value=\"%s\" />\n", txq, nvram_default_get(txq, getTXQ(var)));
		websWrite(wp, "</div>\n");

		// qlen end
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idnet', true);\" name=\"%s_bridged\" %s /><script type=\"text/javascript\">Capture(wl_basic.unbridged)</script>&nbsp;\n",
			  layer, var, nvram_default_match(ssid, "0", "1") ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idnet', false);\" name=\"%s_bridged\" %s /><script type=\"text/javascript\">Capture(share.deflt)</script>\n",
			  layer, var, nvram_default_match(ssid, "1", "1") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div id=\"%s_idnet\">\n", layer);

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

		show_ipnetmask(wp, var);
#if defined(HAVE_BKM) || defined(HAVE_TMK)
#ifdef HAVE_REGISTER
		if (registered_has_cap(21))
#endif
		{
			char nld_enable[32], nld_bridge[32], bufferif[256];
			static char word[256];
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
			static char word[256];
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
		websWrite(wp, "<br />\n</div>\n");
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n ");
		websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_bridged\"), \"%s_idnet\", %s);\n", var, layer, nvram_match(ssid, "0") ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");
	      skip:;
	}
	websWrite(wp, "</fieldset><br />\n");
}
#endif

static void show_macfilter_if(webs_t wp, char *ifname)
{
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend>%s - %s</legend>\n", IFMAP(ifname), live_translate("wl_mac.legend"));
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
		  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"mac_filter_button\\\" value=\\\"\" + sbutton.filterMac + \"\\\" onclick=\\\"openWindow('WL_FilterTable-%s.asp', 930, 740,'MACList');\\\" />\");\n",
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

void ej_show_congestion(webs_t wp, int argc, char_t ** argv)
{
	char *next;
	char var[80];
	char eths[256];
	FILE *fp = fopen("/proc/sys/net/ipv4/tcp_available_congestion_control", "rb");
	if (fp == NULL) {
		strcpy(eths, "vegas westwood bic");
	} else {
		int c = 0;
		while (1 && c < 255) {
			int v = getc(fp);
			if (v == EOF || v == 0xa)
				break;
			eths[c++] = v;
		}
		eths[c++] = 0;
		fclose(fp);
	}

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(management.net_conctrl)</script></div>\n");
	websWrite(wp, "<select name=\"tcp_congestion_control\">\n");
	foreach(var, eths, next) {
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, nvram_match("tcp_congestion_control", var) ? "selected" : "", var);
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
}

void ej_show_ifselect(webs_t wp, int argc, char_t ** argv)
{
	if (argc < 1)
		return;
	char *ifname = argv[0];
	websWrite(wp, "<select name=\"%s\">\n", ifname);
	int i;
	for (i = 1; i < argc; i++) {
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", argv[i], nvram_match(ifname, argv[i]) ? "selected=\"selected\"" : "", argv[i]);
	}

	websWrite(wp, "<option value=\"%s\" %s >LAN</option>\n", nvram_safe_get("lan_ifname"), nvram_match(ifname, nvram_safe_get("lan_ifname")) ? "selected=\"selected\"" : "");
	char *next;
	char var[80];
	char eths[256];

	memset(eths, 0, 256);
	getIfLists(eths, 256);
	foreach(var, eths, next) {
		if (!strcmp(get_wan_face(), var))
			continue;
		if (!strcmp(nvram_safe_get("lan_ifname"), var))
			continue;
		if (!nvram_nmatch("0", "%s_bridged", var)
		    && strncmp(var, "br", 2))
			continue;
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, nvram_match(ifname, var) ? "selected" : "", var);
	}

	websWrite(wp, "</select>\n");
}

#ifdef HAVE_RFLOW
void ej_show_rflowif(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, "<option value=\"%s\" %s >LAN &amp; WLAN</option>\n", nvram_safe_get("lan_ifname"), nvram_match("rflow_if", nvram_safe_get("lan_ifname"))
		  ? "selected=\"selected\"" : "");

	char *lanifs = nvram_safe_get("lan_ifnames");
	char *next;
	char var[80];

	foreach(var, lanifs, next) {
		if (nvram_match("wan_ifname", var))
			continue;
		if (!ifexists(var))
			continue;
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, nvram_match("rflow_if", var) ? "selected=\"selected\"" : "", var);
	}
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	int cnt = get_wl_instances();
	int c;

	for (c = 0; c < cnt; c++) {
		sprintf(var, "wl%d_ifname", c);
		websWrite(wp, "<option value=\"%s\" %s >WLAN%d</option>\n", nvram_safe_get(var), nvram_match("rflow_if", nvram_safe_get(var))
			  ? "selected=\"selected\"" : "", c);
	}
#endif

	char *wanif = nvram_safe_get("wan_ifname");

	if (strlen(wanif) != 0) {
		websWrite(wp, "<option value=\"%s\" %s >WAN</option>\n", wanif, nvram_match("rflow_if", wanif) ? "selected=\"selected\"" : "");
	}
}
#endif

#ifdef CONFIG_STATUS_GPIO
void ej_show_status_gpio_output(webs_t wp, int argc, char_t ** argv)
{
	char *var, *next;
	char nvgpio[32];

	char *value = websGetVar(wp, "action", "");

	char *gpios = nvram_safe_get("gpio_outputs");
	var = (char *)malloc(strlen(gpios) + 1);
	if (var != NULL) {
		if (gpios != NULL) {
			foreach(var, gpios, next) {
				sprintf(nvgpio, "gpio%s", var);
				nvgpio = nvram_nget("gpio_%s", var);
				if (!nvgpio)
					nvram_set(nvgpio, "0");
				websWrite(wp, "<input type=\"checkbox\" name=\"%s\" value=\"1\" %s />\n", nvgpio, nvram_match(nvgpio, "1") ? "checked=\"checked\"" : "");
				websWrite(wp, "<input type=\"checkbox\" name=\"%s\" value=\"0\" %s />\n", nvgpio, nvram_match(nvgpio, "0") ? "checked=\"checked\"" : "");
			}
		}
		free(var);
	}
}

#endif
