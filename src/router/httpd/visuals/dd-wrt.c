/*
 * dd-wrt.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>

#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>
//#include <l7protocols.h>

#if defined(HAVE_80211AC) || (defined(HAVE_BRCMFMAC) && defined(HAVE_NORTHSTAR))
#define COUNTRYLIST "EU DE GB FR NL ES IT CN US JP AU SG BR RU TW CA KR LA"
#else
#define COUNTRYLIST NULL
#endif

#ifdef HAVE_OVERCLOCKING
#ifdef HAVE_ALPINE
static unsigned int alpine_clocks[] = {
	533, 800, 1200, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 0
}; //i tested up to 2200, but it hard on the limit
#else
static unsigned int type2_clocks[7] = { 200, 240, 252, 264, 300, 330, 0 };
static unsigned int type3_clocks[3] = { 150, 200, 0 };
static unsigned int type4_clocks[10] = { 192, 200, 216, 228, 240, 252, 264, 280, 300, 0 };
static unsigned int type7_clocks[10] = { 183, 187, 198, 200, 216, 225, 233, 237, 250, 0 };
static unsigned int type8_clocks[9] = { 200, 300, 400, 500, 600, 632, 650, 662, 0 };

static unsigned int type10_clocks[9] = { 200, 266, 300, 333, 400, 480, 500, 533, 0 };

#ifdef HAVE_NORTHSTAR
static unsigned int ns_type11_clocks[4] = { 600, 800, 900, 0 };
static unsigned int ns_type10_clocks[4] = { 600, 800, 1000, 0 };
static unsigned int ns_type9_clocks[3] = { 600, 800, 0 };
static unsigned int ns_type8_clocks[6] = { 600, 800, 1000, 1200, 1400, 0 };
static unsigned int ns_type7_clocks[4] = { 600, 800, 1000, 0 };
#endif
#endif
#endif

#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
#define IFMAP(a) getRADev(a)
#else
#define IFMAP(a) (a)
#endif

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
#else
#define issuperchannel() wp->issuperchannel
#endif

static int cansuperchannel(webs_t wp, char *prefix)
{
	return (issuperchannel() && nvram_nmatch("0", "%s_regulatory", prefix) && nvram_nmatch("ddwrt", "%s_fwtype", prefix));
}

#if 0
static struct timeval before, after, r;

static void START(void)
{
	gettimeofday(&before, NULL);
}

static void s_END(char *service, int line)
{
	gettimeofday(&after, NULL);
	timersub(&after, &before, &r);
	fprintf(stderr, "DEBUG: %s:%d duration %ld.%06ld\n", service, line, (long int)r.tv_sec, (long int)r.tv_usec);
	START();
}

#define END() s_END(__func__, __LINE__);
#endif

int is_ap(char *prefix)
{
	char ap[16];
	sprintf(ap, "%s_mode", prefix);
	return nvram_match(ap, "ap") || nvram_match(ap, "wdsap");
}

int is_station(char *prefix)
{
	char sta[16];
	sprintf(sta, "%s_mode", prefix);
	return nvram_match(sta, "sta") || nvram_match(sta, "wdssta") || nvram_match(sta, "wdssta_mtik");
}

int is_supplicant(char *prefix)
{
	char sta[16];
	sprintf(sta, "%s_mode", prefix);
	return is_station(prefix) || nvram_match(sta, "mesh");
}

void show_caption_pp(webs_t wp, const char *class, const char *caption, const char *pre, const char *post)
{
	char *buf;
	if (class)
		asprintf(&buf, "%s<div class=\"%s\"><script type=\"text/javascript\">Capture(%s)</script></div>%s\n",
			 pre ? pre : "", class, caption, post ? post : "");
	else {
		asprintf(&buf, "%s<script type=\"text/javascript\">Capture(%s)</script>%s", pre ? pre : "", caption,
			 post ? post : "");
	}
	if (buf) {
		websWrite(wp, buf);
		debug_free(buf);
	}
}

#ifdef HAVE_OVERCLOCKING
EJ_VISIBLE void ej_show_clocks(webs_t wp, int argc, char_t **argv)
{
	int rev = cpu_plltype();
	unsigned int *c;
	char *oclk = nvram_safe_get("overclocking");
#if defined(HAVE_ALPINE)
	if (!*oclk) {
		oclk = "1700";
		nvram_set("clkfreq", "1700");
	}
	c = alpine_clocks;
#elif defined(HAVE_NORTHSTAR)
	switch (rev) {
	case 11:
		c = ns_type11_clocks;
		break;
	case 10:
		c = ns_type10_clocks;
		break;
	case 9:
		c = ns_type9_clocks;
		break;
	case 8:
		c = ns_type8_clocks;
		break;
	case 7:
		c = ns_type7_clocks;
		break;
	default:
		show_caption(wp, NULL, "management.clock_support", "\n");
		return;
	}
#else
	switch (rev) {
	case 2:
		c = type2_clocks;
		break;
	case 3:
		c = type3_clocks;
		break;
	case 4:
		c = type4_clocks;
		break;
	case 7:
		c = type7_clocks;
		break;
	case 8:
		c = type8_clocks;
		break;
	case 10:
		c = type10_clocks;
		break;
	default:
		show_caption(wp, NULL, "management.clock_support", "</div>\n");
		return;
	}
#endif

	int cclk = atoi(oclk);

	int i = 0;
	int in_clock_array = 0;

	//check if cpu clock list contains current clkfreq
	while (c[i] != 0) {
		if (c[i] == cclk) {
			in_clock_array = 1;
		}
		i++;
	}

	if (in_clock_array && nvram_exists("clkfreq")) {
		show_caption(wp, "label", "management.clock_frq", NULL);
		websWrite(wp, "<select name=\"overclocking\">\n");
		i = 0;
		while (c[i] != 0) {
			websWrite(wp, "<option value=\"%d\" %s >%d ", c[i], c[i] == cclk ? "selected=\"selected\"" : "", c[i]);
			show_caption(wp, NULL, "wl_basic.mhz", "</option>\n");
			i++;
		}
		websWrite(wp, "</select>\n</div>\n");
	} else {
		show_caption(wp, NULL, "management.clock_support", "</div>\n");
		fprintf(stderr, "CPU frequency list for rev: %d does not contain current clkfreq: %d.", rev, cclk);
	}
}
#endif

EJ_VISIBLE void ej_show_routing(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"gateway\\\" %s >\" + share.gateway + \"</option>\");\n",
		  nvram_selmatch(wp, "wk_mode", "gateway") ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_BIRD
	websWrite(wp, "document.write(\"<option value=\\\"bgp\\\" %s >BGP</option>\");\n",
		  nvram_selmatch(wp, "wk_mode", "bgp") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"router\\\" %s >\" + route.rip2_mod + \"</option>\");\n",
		  nvram_selmatch(wp, "wk_mode", "router") ? "selected=\\\"selected\\\"" : "");
#endif
#ifdef HAVE_QUAGGA
	websWrite(wp, "document.write(\"<option value=\\\"bgp\\\" %s >BGP</option>\");\n",
		  nvram_selmatch(wp, "wk_mode", "bgp") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"router\\\" %s >\" + route.rip2_mod + \"</option>\");\n",
		  nvram_selmatch(wp, "wk_mode", "router") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"ospf\\\" %s >\" + route.ospf_mod + \"</option>\");\n",
		  nvram_selmatch(wp, "wk_mode", "ospf") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"ospf router\\\" %s >\" + route.ospf_rip2_mod + \"</option>\");\n",
		  nvram_selmatch(wp, "wk_mode", "ospf router") ? "selected=\\\"selected\\\"" : "");
	if (nvram_match("wk_mode", "ospf bgp rip router")) {
		websWrite(wp,
			  "document.write(\"<option value=\\\"ospf bgp rip router\\\" %s >vtysh OSPF BGP RIP router</option>\");\n",
			  nvram_selmatch(wp, "wk_mode", "ospf bgp rip router") ? "selected=\\\"selected\\\"" : "");
	}
#endif
#ifdef HAVE_OLSRD
	websWrite(wp, "document.write(\"<option value=\\\"olsr\\\" %s >\" + route.olsrd_mod + \"</option>\");\n",
		  nvram_selmatch(wp, "wk_mode", "olsr") ? "selected=\\\"selected\\\"" : "");
#endif
	websWrite(wp, "document.write(\"<option value=\\\"static\\\" %s >\" + share.router + \"</option>\");\n",
		  nvram_selmatch(wp, "wk_mode", "static") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n");
	return;
}

EJ_VISIBLE void ej_has_routing(webs_t wp, int argc, char_t **argv)
{
	char var[32], *next;
	char *sub = websGetVar(wp, "wk_mode", NULL);
	if (sub == NULL)
		sub = nvram_safe_get("wk_mode");

	foreach(var, sub, next)
	{
		if (!strcmp(argv[0], "zebra")) {
			if (!strcmp(var, "bgp"))
				return;
			if (!strcmp(var, "router"))
				return;
			if (!strcmp(var, "ospf"))
				return;
		}
		if (!strcmp(var, argv[0]))
			return;
	}
	websWrite(wp, "%s", argv[1]);
}

#ifdef HAVE_BUFFALO
extern void *getUEnv(char *name);
#endif

EJ_VISIBLE void ej_show_connectiontype(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"disabled\\\" %s >\" + share.disabled + \"</option>\");\n",
		  nvram_selmatch(wp, "wan_proto", "disabled") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"static\\\" %s >\" + idx.static_ip + \"</option>\");\n",
		  nvram_selmatch(wp, "wan_proto", "static") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"dhcp\\\" %s >\" + idx.dhcp + \"</option>\");\n",
		  nvram_selmatch(wp, "wan_proto", "dhcp") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"dhcp_auth\\\" %s >\" + idx.dhcp_auth + \"</option>\");\n",
		  nvram_selmatch(wp, "wan_proto", "dhcp_auth") ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "\n//]]>\n</script>\n");

#ifdef HAVE_MODEMBRIDGE
	websWrite(wp, "<option value=\"bridge\" %s ><script type=\"text/javascript\">Capture(idx.dsl_mdm_bdg)</script></option>\n",
		  nvram_selmatch(wp, "wan_proto", "bridge") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_PPPOE
	websWrite(wp, "<option value=\"pppoe\" %s >PPPoE</option>\n",
		  nvram_selmatch(wp, "wan_proto", "pppoe") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_PPPOEDUAL
	websWrite(wp,
		  "<option value=\"pppoe_dual\" %s><script type=\"text/javascript\">Capture(idx.pppoe_dual)</script></option>\n",
		  nvram_selmatch(wp, "wan_proto", "pppoe_dual") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_PPPOATM
	websWrite(wp, "<option value=\"pppoa\" %s >PPPoA</option>\n",
		  nvram_selmatch(wp, "wan_proto", "pppoa") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_PPTP
	websWrite(wp, "<option value=\"pptp\" %s >PPTP</option>\n",
		  nvram_selmatch(wp, "wan_proto", "pptp") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_L2TP
	websWrite(wp, "<option value=\"l2tp\" %s >L2TP</option>\n",
		  nvram_selmatch(wp, "wan_proto", "l2tp") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_HEARTBEAT
	websWrite(wp,
		  "<option value=\"heartbeat\" %s ><script type=\"text/javascript\">Capture(idx.heartbeat_sig)</script></option>\n",
		  nvram_selmatch(wp, "wan_proto", "heartbeat") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_IPETH
	websWrite(wp,
		  "<option value=\"iphone\" %s ><script type=\"text/javascript\">Capture(idx.iphone_tether)</script></option>\n",
		  nvram_selmatch(wp, "wan_proto", "iphone") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_3G
#ifdef HAVE_BUFFALO
	char *region = getUEnv("region");
	if (!region) {
		region = "US";
	}
	if (!strcmp(region, "EU") || !strcmp(region, "DE") || nvram_matchi("umts_override", 1)) {
#endif
		websWrite(wp,
			  "<option value=\"3g\" %s ><script type=\"text/javascript\">Capture(idx.mobile_bb)</script></option>\n",
			  nvram_selmatch(wp, "wan_proto", "3g") ? "selected=\"selected\"" : "");
#ifdef HAVE_BUFFALO
	}
#endif
#endif

	return;
}

EJ_VISIBLE void ej_show_infopage(webs_t wp, int argc, char_t **argv)
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

EJ_VISIBLE void ej_dumpmeminfo(webs_t wp, int argc, char_t **argv)
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

#include "cpucores.c"

#define ASSOCLIST_TMP "/tmp/.wl_assoclist"
#define RSSI_TMP "/tmp/.rssi"
#define ASSOCLIST_CMD "wl assoclist"
#define RSSI_CMD "wl rssi"
#define NOISE_CMD "wl noise"

EJ_VISIBLE void ej_show_wds_subnet(webs_t wp, int argc, char_t **argv)
{
	int index = atoi(argv[0]);
	char *interface = argv[1];

	char br1[32];

	sprintf(br1, "%s_br1_enable", interface);
	if (nvram_invmatchi(br1, 1))
		return;
	char buf[22];

	sprintf(buf, "%s_wds%d_enable", interface, index);
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"2\\\" %s >\" + wds.subnet + \"</option>\");\n//]]>\n</script>\n",
		nvram_selmatch(wp, buf, "2") ? "selected=\\\"selected\\\"" : "");
	return;
}

EJ_VISIBLE void ej_show_paypal(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_DDLAN
	websWrite(wp, "<a href=\"mailto:support@mcdd.de\">support@mcdd.de</a><br />");
#endif
#ifdef HAVE_CORENET
	websWrite(wp, "<a href=\"http://www.corenetsolutions.com\">http://www.corenetsolutions.com</a><br />");
#endif

#ifndef HAVE_BRANDING
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
	websWrite(wp,
		  "<input type=\"image\" alt=\"Paypal DD-WRT development support\" src=\"images/paypal.png\" name=\"submit\" />");
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

EJ_VISIBLE void ej_show_iradius_check(webs_t wp, int argc, char_t **argv)
{
	char *sln = nvram_safe_get("iradius_count");

	if (sln == NULL || *(sln) == 0)
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

EJ_VISIBLE void ej_show_iradius(webs_t wp, int argc, char_t **argv)
{
	char *sln = nvram_safe_get("iradius_count");

	if (sln == NULL || *(sln) == 0)
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
		userlist = strdup(u);
		debug_free(u);
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
		websWrite(wp, "<input name=\"%s\" value=\"%s\" size=\"25\" maxlength=\"63\" />\n", username,
			  sep != NULL ? sep : "");
		websWrite(wp, "</td>\n");
		if (userlist)
			sep = strsep(&userlist, " ");

		char active[32];

		snprintf(active, 31, "iradius%d_active", i);

		websWrite(wp, "<td>\n");
		websWrite(wp, "<input name=\"%s\" type=\"hidden\" />", active);
		websWrite(wp, "<input type=\"checkbox\" value=\"%s\" name=\"_%s\" %s />\n", sep, active,
			  sep != NULL ? strcmp(sep, "1") == 0 ? "checked=\"checked\"" : "" : "");
		websWrite(wp, "</td>\n");
		websWrite(wp, "<td>\n");
		if (userlist)
			sep = strsep(&userlist, " ");

		long t = 0;

		if (sep != NULL)
			t = atol(sep);

		if (t != -1) {
			t -= now.tv_sec;
			t /= 60;
		}

		snprintf(active, 31, "iradius%d_lease", i);
		char st[32];

		if (t >= 0)
			sprintf(st, "%ld", t);
		else
			sprintf(st, "over");
		websWrite(wp, "<input class=\"num\" name=\"%s\" value='%s' />\n", active, st);
		websWrite(wp, "</td>\n");

		websWrite(wp, "<td>\n");
		snprintf(active, 31, "iradius%d_delete", i);
		websWrite(wp, "<input name=\"%s\" type=\"hidden\" />", active);
		websWrite(wp, "<input type=\"checkbox\" name=\"_%s\"/>\n", active);
		websWrite(wp, "</td></tr>\n");
	}
	if (o != NULL)
		debug_free(o);
	return;
}

#endif

#ifdef HAVE_CHILLI
#ifdef HAVE_CHILLILOCAL

EJ_VISIBLE void ej_show_userlist(webs_t wp, int argc, char_t **argv)
{
	char *sln = nvram_safe_get("fon_usernames");

	if (sln == NULL || *(sln) == 0)
		return;
	int leasenum = atoi(sln);

	if (leasenum == 0)
		return;
	int i;
	char username[32];
	char password[32];
	char *u = nvram_safe_get("fon_userlist");
	char *userlist = strdup(u);
	char *o = userlist;

	for (i = 0; i < leasenum; i++) {
		snprintf(username, 31, "fon_user%d_name", i);
		char *sep = strsep(&userlist, "=");

		websWrite(wp, "<tr><td>\n");
		websWrite(wp, "<input name=\"%s\" autocomplete=\"new-password\" value=\"%s\" size=\"25\" maxlength=\"63\" />\n",
			  username, sep != NULL ? sep : "");
		websWrite(wp, "</td>\n");
		char *pass = userlist;
		sep = strsep(&userlist, " ");
		snprintf(password, 31, "fon_user%d_password", i);
		websWrite(wp, "<td>\n");
		websWrite(
			wp,
			"<input type=\"password\" autocomplete=\"new-password\" name=\"%s\" value=\"%s\" size=\"25\" maxlength=\"63\" />\n",
			password, pass);
		websWrite(wp, "</td></tr>\n");
	}
	debug_free(o);
	return;
}

#endif
#endif

EJ_VISIBLE void ej_show_openvpnuserpass(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *sln = nvram_safe_get("openvpn_userpassnum");
	if (sln == NULL || *(sln) == 0) // check for NULL pointer (which should never happen in this case) or empty string
		return;
	int userpassnum = atoi(sln);
	if (userpassnum == 0)
		return;
	char *nvuserpass = nvram_safe_get("openvpn_userpass");
	char *userpass = strdup(nvuserpass);
	char *originalpointer = userpass; // strsep destroys the pointer by moving it
	for (i = 0; i < userpassnum; i++) {
		char *sep = strsep(&userpass, "=");
		websWrite(
			wp,
			"<tr><td><input name=\"openvpn%d_usrname\" value=\"%s\" size=\"32\" maxlength=\"32\" onblur=\"valid_name(this,share.usrname,SPACE_NO)\" /></td>",
			i, sep != NULL ? sep : "");
		sep = strsep(&userpass, " ");
		websWrite(
			wp,
			"<td><input type=\"password\" name=\"openvpn%d_passwd\" value=\"%s\" size=\"48\" maxlength=\"64\" onmouseover=\"this.type='text'\" onmouseout=\"this.type='password'\" onblur=\"valid_name(this,share.passwd,SPACE_NO)\" /></td>",
			i, sep != NULL ? sep : "");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"userpass_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n</td></tr>",
			i);
	}
	debug_free(originalpointer);
	return;
}

EJ_VISIBLE void ej_show_staticleases(webs_t wp, int argc, char_t **argv)
{
	int i;

	// cprintf("get static leasenum");

	char *sln = nvram_safe_get("static_leasenum");

	// cprintf("check null");
	if (sln == NULL || *(sln) == 0)
		return;
	// cprintf("atoi");

	int leasenum = atoi(sln);

	// cprintf("leasenum==0");
	if (leasenum == 0)
		return;
	// cprintf("get leases");
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
			i, sep != NULL ? sep : "");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"lease_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n</td></tr>",
			i);
	}
	debug_free(originalpointer);
	return;
}

EJ_VISIBLE void ej_show_control(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_BRANDING
	websWrite(wp, "Control Panel");
#else
	websWrite(wp, "DD-WRT Control Panel");
#endif
	return;
}

EJ_VISIBLE void ej_show_default_level(webs_t wp, int argc, char_t **argv)
{
	char *defaults = nvram_safe_get("svqos_defaults");

	websWrite(wp, "<fieldset>\n");
	show_caption_legend(wp, "qos.legend6");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "qos.enabledefaultlvls", NULL);
	websWrite(
		wp,
		"<input type=\"checkbox\" onclick=\"defaultlvl_grey(this.checked,this.form)\" name=\"svqos_defaults\" value=\"1\" %s />\n",
		nvram_matchi("svqos_defaults", 1) ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">WAN <script type=\"text/javascript\">document.write(qos.bandwidth)</script></div>\n");
	websWrite(wp, "<input class\"num\" type=\"number\" size=\"10\" name=\"default_downlevel\" value=\"%s\" %s/>\n",
		  nvram_safe_get("default_downlevel"), (!strcmp(defaults, "1")) ? "" : "disabled");
	websWrite(wp, "&nbsp<script type=\"text/javascript\">document.write(qos.speed+\" \"+qos.down)</script></div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">WAN <script type=\"text/javascript\">document.write(qos.bandwidth)</script></div>\n");
	websWrite(wp, "<input class\"num\" type=\"number\" size=\"10\" name=\"default_uplevel\" value=\"%s\" %s/>\n",
		  nvram_safe_get("default_uplevel"), (!strcmp(defaults, "1")) ? "" : "disabled");
	websWrite(wp, "&nbsp<script type=\"text/javascript\">document.write(qos.speed+\" \"+qos.up)</script></div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">LAN <script type=\"text/javascript\">Capture(qos.bandwidth)</script></div>\n");
	websWrite(wp, "<input class\"num\" type=\"number\" size=\"10\" name=\"default_lanlevel\" value=\"%s\" %s/>\n",
		  nvram_default_get("default_lanlevel", "100000"), (!strcmp(defaults, "1")) ? "" : "disabled");
	websWrite(wp, "&nbsp<script type=\"text/javascript\">document.write(qos.speed)</script></div>\n");
	websWrite(wp, "</fieldset><br />\n");
	return;
}

static char *selmatch(char *var, char *is, char *ret)
{
	if (nvram_match(var, is))
		return ret;
	return "";
}

static char *wpa_enc_label(char *buf, size_t len, char *value)
{
#ifdef HAVE_IAS
	return ias_enc_label(value);
#else
	/*
	   I'm not sure if consumers can handle the new names for encryption types here, but at least we can change it back quick if required
	 */
	if (value) {
		if (!strcmp(value, "disabled")) {
			return tran_string(buf, len, "share.disabled");
		} else if (!strcmp(value, "psk")) {
			return "WPA-PSK";
		} else if (!strcmp(value, "psk2")) {
			return "WPA2-PSK";
		} else if (!strcmp(value, "psk2-sha256")) {
			return "WPA2-PSK-SHA256";
		} else if (!strcmp(value, "psk3")) {
			return "WPA3-PSK";
		} else if (!strcmp(value, "psk psk2")) {
			return "WPA2-PSK/WPA-PSK";
		} else if (!strcmp(value, "psk2 psk3")) {
			return "WPA2-PSK/WPA3-PSK";
#ifndef HAVE_MADWIFI
		} else if (!strcmp(value, "wpa")) {
			return "WPA-EAP";
#else
		} else if (!strcmp(value, "wpa")) {
			return "WPA";
#endif
		} else if (!strcmp(value, "wpa2")) {
			return "WPA2-EAP";
		} else if (!strcmp(value, "wpa2-sha256")) {
			return "WPA2-EAP-SHA256";
		} else if (!strcmp(value, "wpa3")) {
			return "WPA3-EAP-SUITE-B";
		} else if (!strcmp(value, "wpa3-192")) {
			return "WPA3-EAP-SUITE-B-192";
		} else if (!strcmp(value, "wpa wpa2")) {
			return "WPA2-EAP/WPA-EAP";
		} else if (!strcmp(value, "wpa2 wpa3")) {
			return "WPA2-EAP/WPA3-EAP-SUITE-B";
		} else if (!strcmp(value, "wpa2 wpa3-192")) {
			return "WPA2-EAP/WPA3-EAP-SUITE-B-192";
		} else if (!strcmp(value, "wep")) {
			return "WEP";
		} else if (!strcmp(value, "owe")) {
			return "OWE";
		} else if (!strcmp(value, "radius")) {
			return "RADIUS";
		} else {
			return value;
		}
	}
	return "";
#endif
}

static void show_security_prefix(webs_t wp, int argc, char_t **argv, char *prefix, int primary)
{
	char var[80];
	char sta[80];
	char buf[128];

	// char p2[80];
	cprintf("show security prefix\n");
	sprintf(var, "%s_security_mode", prefix);
	// strcpy(p2,prefix);
	// rep(p2,'X','.');
	// websWrite (wp, "<input type=\"hidden\"
	// name=\"%s_security_mode\"/>\n",p2);
#ifdef HAVE_MADWIFI
	websWrite(wp, "<fieldset>\n");
#endif
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wpa.secmode", NULL);
	websWrite(
		wp,
		"<select name=\"%s_security_mode\" onchange=\"SelMode('%s', '%s_security_mode',this.form.%s_security_mode.selectedIndex,this.form)\">\n",
		prefix, prefix, prefix, prefix);
	websWrite(wp, "<option value=\"disabled\" %s>%s</option>\n", selmatch(var, "psk", "selected=\"selected\""),
		  wpa_enc_label(buf, sizeof(buf), "disabled"));

	sprintf(sta, "%s_mode", prefix);
#if 0
	if (has_wpa3(prefix)) {
		if (!has_qtn(prefix)) {
			if (!primary || is_ap(prefix)) {
				websWrite(wp, "<option value=\"owe\" %s>%s</option>\n", selmatch(var, "owe", "selected=\"selected\""), wpa_enc_label(buf, sizeof(buf), "owe"));
			}
		}
	}
#endif
#ifndef HAVE_MADWIFI
	websWrite(wp, "<option value=\"psk\" %s>%s</option>\n", selmatch(var, "psk", "selected=\"selected\""),
		  wpa_enc_label(buf, sizeof(buf), "psk"));
#else
	if (nvhas(var, "psk") || nvhas(var, "psk2") || nvhas(var, "psk3") || nvhas(var, "wpa") || nvhas(var, "wpa2") ||
	    nvhas(var, "wpa3") || nvhas(var, "wpa3-192"))
		websWrite(wp, "<option value=\"wpa\" %s>%s</option>\n", "selected=\"selected\"",
			  wpa_enc_label(buf, sizeof(buf), "wpa"));
	else
		websWrite(wp, "<option value=\"wpa\" >%s</option>\n", wpa_enc_label(buf, sizeof(buf), "wpa"));
#endif
#ifndef HAVE_MADWIFI
	websWrite(wp, "<option value=\"psk2\" %s>%s</option>\n", selmatch(var, "psk2", "selected=\"selected\""),
		  wpa_enc_label(buf, sizeof(buf), "psk2"));
	if (has_wpa3(prefix))
		websWrite(wp, "<option value=\"psk3\" %s>%s</option>\n", selmatch(var, "psk3", "selected=\"selected\""),
			  wpa_enc_label(buf, sizeof(buf), "psk3"));
#ifdef HAVE_RT2880
	if (!primary || nvram_match(sta, "ap"))
#endif
	{
		websWrite(wp, "<option value=\"psk psk2\" %s>%s</option>\n", selmatch(var, "psk psk2", "selected=\"selected\""),
			  wpa_enc_label(buf, sizeof(buf), "psk psk2"));
		if (has_wpa3(prefix))
			websWrite(wp, "<option value=\"psk2 psk3\" %s>%s</option>\n",
				  selmatch(var, "psk2 psk3", "selected=\"selected\""),
				  wpa_enc_label(buf, sizeof(buf), "psk2 psk3"));
	}
#endif
	if (!has_qtn(prefix)) {
		if (!primary || is_ap(prefix)) {
#ifndef HAVE_MADWIFI
			websWrite(wp, "<option value=\"wpa\" %s>%s</option>\n", selmatch(var, "wpa", "selected=\"selected\""),
				  wpa_enc_label(buf, sizeof(buf), "wpa"));
			websWrite(wp, "<option value=\"wpa2\" %s>%s</option>\n", selmatch(var, "wpa2", "selected=\"selected\""),
				  wpa_enc_label(buf, sizeof(buf), "wpa2"));
			if (has_wpa3(prefix)) {
				websWrite(wp, "<option value=\"wpa3\" %s>%s</option>\n",
					  selmatch(var, "wpa3", "selected=\"selected\""), wpa_enc_label(buf, sizeof(buf), "wpa3"));
				websWrite(wp, "<option value=\"wpa3-192\" %s>%s</option>\n",
					  selmatch(var, "wpa3-192", "selected=\"selected\""),
					  wpa_enc_label(buf, sizeof(buf), "wpa3-192"));
			}
			websWrite(wp, "<option value=\"wpa wpa2\" %s>%s</option>\n",
				  selmatch(var, "wpa wpa2", "selected=\"selected\""), wpa_enc_label(buf, sizeof(buf), "wpa wpa2"));
			if (has_wpa3(prefix)) {
				websWrite(wp, "<option value=\"wpa2 wpa3\" %s>%s</option>\n",
					  selmatch(var, "wpa2 wpa3", "selected=\"selected\""),
					  wpa_enc_label(buf, sizeof(buf), "wpa2 wpa3"));
				websWrite(wp, "<option value=\"wpa2 wpa3-192\" %s>%s</option>\n",
					  selmatch(var, "wpa2 wpa3-192", "selected=\"selected\""),
					  wpa_enc_label(buf, sizeof(buf), "wpa2 wpa3-192"));
			}
#endif
			websWrite(wp, "<option value=\"radius\" %s>%s</option>\n", selmatch(var, "radius", "selected=\"selected\""),
				  wpa_enc_label(buf, sizeof(buf), "radius"));
		}
		if (!nvram_match(sta, "mesh"))
			websWrite(wp, "<option value=\"wep\" %s>%s</option>\n", selmatch(var, "wep", "selected=\"selected\""),
				  wpa_enc_label(buf, sizeof(buf), "wep"));
	}
#ifdef HAVE_WPA_SUPPLICANT
#ifndef HAVE_MICRO
#if !defined(HAVE_RT2880) || defined(HAVE_MT76)
	if (nvram_match(sta, "sta") || nvram_match(sta, "wdssta") || nvram_match(sta, "apsta") || nvram_match(sta, "wet")) {
#ifdef HAVE_MADWIFI
		if (nvhas(var, "peap") || nvhas(var, "leap") || nvhas(var, "tls") || nvhas(var, "ttls") || nvhas(var, "8021X"))
			websWrite(wp, "<option value=\"8021X\" %s>802.1X / EAP</option>\n", "selected=\"selected\"");
		else
			websWrite(wp, "<option value=\"8021X\" %s>802.1X / EAP</option>\n", "");

#else
		websWrite(wp, "<option value=\"8021X\" %s>802.1X / EAP</option>\n",
			  selmatch(var, "8021X", "selected=\"selected\""));
#endif
	}
#else
#ifndef HAVE_RT61
	if (nvram_match(sta, "sta") || nvram_match(sta, "wet")) {
		websWrite(wp, "<option value=\"8021X\" %s>802.1X / EAP</option>\n",
			  selmatch(var, "8021X", "selected=\"selected\""));
	}
#endif
#endif
#endif
#endif

	websWrite(wp, "</select></div>\n");
	rep(prefix, 'X', '.');
	cprintf("ej show wpa\n");
	internal_ej_show_wpa_setting(wp, argc, argv, prefix);
}

static void ej_show_security_single(webs_t wp, int argc, char_t **argv, char *prefix)
{
	char *next;
	char var[80];
	char ssid[80];
	char mac[18];

#ifdef HAVE_GUESTPORT
	char guestport[16];
	sprintf(guestport, "guestport_%s", prefix);
#endif
	sprintf(mac, "%s_hwaddr", prefix);
	char *vifs = nvram_nget("%s_vifs", prefix);

	if (vifs == NULL)
		return;
	sprintf(ssid, "%s_ssid", prefix);
	if (!nvram_nmatch("disabled", "%s_net_mode", prefix) && !nvram_nmatch("disabled", "%s_mode", prefix)) {
		websWrite(wp, "<h2><script type=\"text/javascript\">Capture(wpa.h2)</script> %s</h2>\n", prefix);
		websWrite(wp, "<fieldset>\n");
		// cprintf("getting %s %s\n",ssid,nvram_safe_get(ssid));
		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.pintrface)</script> %s SSID [",
			  getNetworkLabel(wp, IFMAP(prefix)));
		tf_webWriteESCNV(wp, ssid); // fix for broken html page if ssid
		// contains html tag
		websWrite(wp, "] HWAddr [%s]</legend>\n", nvram_safe_get(mac));
		show_security_prefix(wp, argc, argv, prefix, 1);
		websWrite(wp, "</fieldset>\n<br />\n");
	}
	foreach(var, vifs, next)
	{
		if (nvram_nmatch("disabled", "%s_mode", var))
			continue;

		sprintf(ssid, "%s_ssid", var);
		websWrite(wp, "<fieldset>\n");
		// cprintf("getting %s %s\n", ssid,nvram_safe_get(ssid));
		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.vintrface)</script> %s SSID [",
			  getNetworkLabel(wp, IFMAP(var)));
		tf_webWriteESCNV(wp, ssid); // fix for broken html page if ssid
		// contains html tag
		sprintf(mac, "%s_hwaddr", var);
		if (nvram_exists(mac))
			websWrite(wp, "] HWAddr [%s", nvram_safe_get(mac));

		websWrite(wp, "]</legend>\n");
		rep(var, '.', 'X');
		show_security_prefix(wp, argc, argv, var, 0);
		websWrite(wp, "</fieldset>\n<br />\n");
	}
}

EJ_VISIBLE void ej_show_security(webs_t wp, int argc, char_t **argv)
{
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];
		sprintf(buf, WIFINAME "%d", i);

		ej_show_security_single(wp, argc, argv, buf);
	}
	return;
}

EJ_VISIBLE void ej_getWET(webs_t wp, int argc, char_t **argv)
{
	if (getWET())
		websWrite(wp, "1");
	else
		websWrite(wp, "0");
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

#ifdef HAVE_MADWIFI
EJ_VISIBLE void ej_show_wifiselect(webs_t wp, int argc, char_t **argv)
{
	char *next;
	char var[32];
	char eths[256];
	bzero(eths, 256);

	int count = getdevicecount();

	if (count < 1)
		return;
	getIfList(eths, "wlan0.sta");

	if (count == 1 && *(nvram_safe_get("wlan0_vifs")) == 0 && *(eths) == 0)
		return;

	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.intrface", NULL);
	websWrite(wp, "<select name=\"wifi_display\" onchange=\"refresh(this.form)\">\n");
	int i;

	for (i = 0; i < count; i++) {
		sprintf(var, WIFINAME "%d", i);
		if (nvram_nmatch("disabled", "%s_mode", var))
			continue;
		if (has_ad(var))
			websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var,
				  nvram_match("wifi_display", "giwifi0") ? "selected=\"selected\"" : "", getNetworkLabel(wp, var));
		else
			websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var,
				  nvram_match("wifi_display", var) ? "selected=\"selected\"" : "", getNetworkLabel(wp, var));
		char *names = nvram_nget(WIFINAME "%d_vifs", i);

		foreach(var, names, next)
		{
			if (nvram_nmatch("disabled", "%s_net_mode", var))
				continue;
			if (has_ad(var))
				websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var,
					  nvram_match("wifi_display", "giwifi0") ? "selected=\"selected\"" : "",
					  getNetworkLabel(wp, var));
			else
				websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var,
					  nvram_match("wifi_display", var) ? "selected=\"selected\"" : "",
					  getNetworkLabel(wp, var));
		}

		char ifname[32];
		sprintf(ifname, "wlan%d.sta", i);
		bzero(eths, 256);
		getIfList(eths, ifname);
		foreach(var, eths, next)
		{
			websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var,
				  nvram_match("wifi_display", var) ? "selected=\"selected\"" : "", getNetworkLabel(wp, var));
		}
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
}
#else
EJ_VISIBLE void ej_show_wifiselect(webs_t wp, int argc, char_t **argv)
{
	char *next;
	char var[32];
	int count = get_wl_instances();

	if (count < 2)
		return;
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.intrface", NULL);
	websWrite(wp, "<select name=\"wifi_display\" onchange=\"refresh(this.form)\">\n");
	int i;

	for (i = 0; i < count; i++) {
		sprintf(var, WIFINAME "%d", i);
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var,
			  nvram_match("wifi_display", var) ? "selected=\"selected\"" : "", getNetworkLabel(wp, var));
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
}

#endif

void show_bgscan_options(webs_t wp, char *prefix)
{
	char signal[32];
	websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(wl_adv.bgscan)</script></legend>");
	sprintf(signal, "%s_bgscan_mode", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	char buf1[128];
	char buf2[128];
	char *modes[] = { "\" + share.off +\"", "\" + wl_adv.bgscan_simple + \"", "\" + wl_adv.bgscan_learn + \"" };
	showOptionsNames(wp, "wl_adv.bgscan_mode", signal, "off simple learn", modes, nvram_default_get(signal, "off"));
	websWrite(wp, "</div>\n");
	sprintf(signal, "%s_bgscan_short_int", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_adv.bgscan_short_int", NULL);
	websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" value=\"%s\" />\n", signal,
		  nvram_default_get(signal, "30"));
	websWrite(wp, "</div>\n");
	sprintf(signal, "%s_bgscan_threshold", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_adv.bgscan_threshold", NULL);
	websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" value=\"%s\" />\n", signal,
		  nvram_default_get(signal, "-45"));
	websWrite(wp, "</div>\n");
	sprintf(signal, "%s_bgscan_long_int", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_adv.bgscan_long_int", NULL);
	websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" value=\"%s\" />\n", signal,
		  nvram_default_get(signal, "300"));
	websWrite(wp, "</div>\n");
	websWrite(wp, "</fieldset><br/>\n");
}

#ifdef HAVE_USB
EJ_VISIBLE void ej_show_usb_diskinfo(webs_t wp, int argc, char_t **argv)
{
	char line[512];
	char used[32];
	char avail[32];
	char per[16];
	char mp[128];
	char *pos;
	FILE *fp;
	int mounted = 0;
	if (!nvram_matchi("usb_automnt", 1))
		return;
	//exclude proftpd bind mount points and don't display the first 3 lines which are header and rootfs

	if ((fp = popen("df -P -h | grep -v proftpd | awk '{ print $3 \" \" $2 \" \" $5 \" \" $6}' | tail -n +3", "rb"))) {
		while (!feof(fp) && fgets(line, sizeof(line), fp)) {
			if (strlen(line) > 2) {
				bzero(used, sizeof(used));
				bzero(avail, sizeof(avail));
				bzero(per, sizeof(per));
				bzero(mp, sizeof(mp));
				if (sscanf(line, "%s %s %s %s", used, avail, per, mp) == 4) {
					if (!strncmp(mp, "/dev", 4))
						continue;
					websWrite(wp, "<div class=\"setting\">");
					websWrite(wp, "<div class=\"label\">%s %s</div>", live_translate(wp, "usb.usb_diskspace"),
						  mp);
					websWrite(wp, "<span id=\"usage\">");
					websWrite(wp, "<div class=\"meter\"><div class=\"bar\" style=\"width:%s;\"></div>", per);
					websWrite(wp, "<div class=\"text\">%s</div></div>", per);
					websWrite(wp, "%s / %s </span></div>", used, avail);
				}
			}
		}
		websWrite(wp, "<hr>");
		pclose(fp);
	}
	websWrite(wp, "<div class=\"setting\">");
	if ((fp = fopen("/tmp/disktype.dump", "r"))) {
		while (!feof(fp) && fgets(line, sizeof(line), fp)) {
			if (strcmp(line, "\n"))
				websWrite(wp, "%s<br />", line);
		}
		fclose(fp);
		mounted = 1;
	}
	if ((fp = fopen("/tmp/parttype.dump", "r"))) {
		while (!feof(fp) && fgets(line, sizeof(line), fp)) {
			if (strcmp(line, "\n"))
				websWrite(wp, "%s<br />", line);
		}
		fclose(fp);
		mounted = 1;
	}
	websWrite(wp, "</div>");

	if (!mounted) {
		websWrite(wp, "%s", live_translate(wp, "status_router.notavail"));
		websWrite(wp, "<br>");
	}
	return;
}
#endif

#ifdef HAVE_MMC
EJ_VISIBLE void ej_show_mmc_cardinfo(webs_t wp, int argc, char_t **argv)
{
	char buff[512];
	FILE *fp;

	if (!nvram_matchi("mmc_enable0", 1))
		return;

	if ((fp = fopen("/proc/mmc/status", "rb"))) {
		while (fgets(buff, sizeof(buff), fp)) {
			if (strcmp(buff, "\n"))
				websWrite(wp, "%s<br />", buff);
		}
		fclose(fp);
	} else {
		show_caption_simple(wp, "status_router.notavail");
		websWrite(wp, "<br>");
	}
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
		show_caption_pp(wp, NULL, labelname, "<legend>", "</legend>\n");
	else
		websWrite(wp, "<legend>%s</legend>\n", labelname);
}

#ifdef HAVE_OLSRD
#include "olsrd.c"
#endif

#ifdef HAVE_VLANTAGGING
#ifdef HAVE_BONDING
#include "bonding.c"
#endif

#include "vlantagging.c"
#include "mdhcp.c"
#include "bridging.c"
#endif

#ifdef HAVE_IPVS
#include "ipvs.c"
#endif
#if 0
static void showDynOption(webs_t wp, char *propname, char *nvname, char *options[], char *names[])
{
	int i;

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">%s</div><select name=\"%s\">\n", propname, nvname);
	for (i = 0; options[i] != NULL; i++) {
		websWrite(wp, "<option value=\"%s\" %s><script type=\"text/javascript\">Capture(share.off)</script></option>\n", names[i], nvram_match(nvname, options[i]) ? "selected=\"selected\"" : "");
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
	if (is_ap(prefix) || nvram_match(wl_mode, "infra") || nvram_match(wl_mode, "mesh")) {
		char wl_channel[16];
		char wl_channel2[16];
		char wl_wchannel[16];

		sprintf(wl_channel, "%s_channel", prefix);
		sprintf(wl_channel2, "%s_channel2", prefix);
		sprintf(wl_wchannel, "%s_wchannel", prefix);
		char wl_nbw[16];

		nvram_default_get(wl_wchannel, "0");
		sprintf(wl_nbw, "%s_nbw", prefix);

		websWrite(wp, "<div class=\"setting\">\n");
		//wl_basic.vht80p80chan="Wireless Channel 2 (80+80)";
		show_caption(wp, "label", "wl_basic.label4", NULL);
		if (is_mac80211(prefix))
			websWrite(
				wp,
				"<select name=\"%s\" rel=\"mac80211\" onfocus=\"check_action(this,0)\" onchange=\"setChannelProperties(this);\"><script type=\"text/javascript\">\n//<![CDATA[\n",
				wl_channel);
		else
			websWrite(
				wp,
				"<select name=\"%s\" onfocus=\"check_action(this,0)\"><script type=\"text/javascript\">\n//<![CDATA[\n",
				wl_channel);
#ifdef HAVE_MADWIFI
		struct wifi_channels *chan;
		char cn[128];
		char fr[32];
		int gotchannels = 0;
		int channelbw = 20;
		if (is_mac80211(prefix)) {
			// temp must be replaced with the actual selected country
			char regdomain[32];
			char *country;
			int checkband = 255;
#ifdef HAVE_ATH9K
			sprintf(regdomain, "%s_regdomain", "wlan0");
#else
			sprintf(regdomain, "%s_regdomain", prefix);
#endif
			country = nvram_default_get(regdomain, "UNITED_STATES");
			// temp end

			if (nvram_nmatch("ng-only", "%s_net_mode", prefix) || nvram_nmatch("n2-only", "%s_net_mode", prefix) ||
			    nvram_nmatch("bg-mixed", "%s_net_mode", prefix) || nvram_nmatch("ng-mixed", "%s_net_mode", prefix) ||
			    nvram_nmatch("b-only", "%s_net_mode", prefix) || nvram_nmatch("g-only", "%s_net_mode", prefix) ||
			    nvram_nmatch("axg-only", "%s_net_mode", prefix)) {
				checkband = 2;
			}
			if (nvram_nmatch("a-only", "%s_net_mode", prefix) || nvram_nmatch("na-only", "%s_net_mode", prefix) ||
			    nvram_nmatch("ac-only", "%s_net_mode", prefix) || nvram_nmatch("acn-mixed", "%s_net_mode", prefix) ||
			    nvram_nmatch("ax-only", "%s_net_mode", prefix) || nvram_nmatch("xacn-mixed", "%s_net_mode", prefix) ||
			    nvram_nmatch("n5-only", "%s_net_mode", prefix)) {
				checkband = 5;
			}
			if (nvram_nmatch("80", "%s_channelbw", prefix))
				channelbw = 80;
			if (nvram_nmatch("80+80", "%s_channelbw", prefix))
				channelbw = 80;
			if (nvram_nmatch("160", "%s_channelbw", prefix))
				channelbw = 160;
			if (nvram_nmatch("40", "%s_channelbw", prefix))
				channelbw = 40;
			if (nvram_nmatch("2040", "%s_channelbw", prefix))
				channelbw = 40;
			chan = mac80211_get_channels_simple(prefix, getIsoName(country), channelbw, checkband);
			/* if (chan == NULL)
			   chan =
			   list_channels_ath9k(dev, "DE", 40,
			   0xff); */
			gotchannels = 1;
		}
		if (!gotchannels) {
			chan = list_channels(prefix);
			if (chan == NULL)
				chan = list_channels(dev);
		}
#ifdef HAVE_ENEO
		chan = NULL;
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s>\" + share.auto + \"</option>\");\n",
			  !strcmp("0", "0") ? "selected=\\\"selected\\\"" : "");

#endif
		if (chan) {
			char *wlc = nvram_safe_get(wl_channel);
			int offset = get_freqoffset(prefix);
			// int cnt = getchannelcount ();
			if (!nvram_nmatch("mesh", "%s_mode", prefix) && !nvram_nmatch("infra", "%s_mode", prefix))
				websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s>\" + share.auto + \"</option>\");\n",
					  !strcmp(wlc, "0") ? "selected=\\\"selected\\\"" : "");
			int i = 0;
			while (chan[i].freq != -1) {
#ifdef HAVE_BUFFALO
				if (chan[i].dfs == 1) {
					i++;
					continue;
				}
#endif

				if (is_mvebu(prefix) &&
				    ((chan[i].channel == 161 || chan[i].channel == 153 || chan[i].channel == 64) &&
				     channelbw == 80)) {
					i++;
					continue;
				}

				if (channelbw == 40 && !chan[i].luu && !chan[i].ull) {
					i++;
					continue; // do not show channels where bandwidth is not available
				}

				if (channelbw == 80 && !chan[i].lul && !chan[i].ull && !chan[i].ulu && !chan[i].luu) {
					i++;
					continue; // do not show channels where bandwidth is not available
				}

				if (channelbw == 160 && !chan[i].lll && !chan[i].llu && !chan[i].lul && !chan[i].luu &&
				    !chan[i].ull && !chan[i].ulu && !chan[i].uul && !chan[i].uuu) {
					i++;
					continue; // do not show channels where bandwidth is not available
				}
				sprintf(cn, "%d", chan[i].channel);
				sprintf(fr, "%d", chan[i].freq);
				int freq = chan[i].freq;
				if (freq != -1) {
					if (is_mac80211(prefix) && !is_ath5k(prefix)) {
						websWrite(
							wp,
							"document.write(\"<option value=\\\"%s\\\" rel=\\\'{\\\"lll\\\":%d,\\\"llu\\\":%d,\\\"lul\\\":%d,\\\"luu\\\":%d,\\\"ull\\\":%d,\\\"ulu\\\":%d,\\\"uul\\\":%d,\\\"uuu\\\":%d}\\\'%s>%s - %d \"+wl_basic.mhz+\"</option>\");\n",
							fr, chan[i].lll, chan[i].llu, chan[i].lul, chan[i].luu, chan[i].ull,
							chan[i].ulu, chan[i].uul, chan[i].uuu,
							!strcmp(wlc, fr) ? " selected=\\\"selected\\\"" : "", cn, (freq + offset));
					} else {
						websWrite(
							wp,
							"document.write(\"<option value=\\\"%s\\\" %s>%s - %d \"+wl_basic.mhz+\"</option>\");\n",
							fr, !strcmp(wlc, fr) ? "selected=\\\"selected\\\"" : "", cn,
							(freq + offset));
					}
				}
				i++;
			}
		}
		websWrite(wp, "//]]>\n</script></select></div>\n");
		if (has_vht80plus80(prefix) && nvram_nmatch("80+80", "%s_channelbw", prefix) && chan) {
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.vht80p80chan", NULL);
			char *wlc = nvram_safe_get(wl_channel2);
			int base = nvram_geti(wl_channel);
			int offset = get_freqoffset(prefix);
			websWrite(
				wp,
				"<select name=\"%s\" onfocus=\"check_action(this,0)\" ><script type=\"text/javascript\">\n//<![CDATA[\n",
				wl_channel2);
			/*
			todo, develop algorithm for best quality 80+80 secondary channel
*/
			//                      websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s>\" + share.auto + \"</option>\");\n", !strcmp(wlc, "0") ? "selected=\\\"selected\\\"" : "");
			int i = 0;
			int iht;
			int channeloffset;
			get_channeloffset(prefix, &iht, &channeloffset);
			while (chan[i].freq != -1) {
				if (is_mvebu(prefix) &&
				    ((chan[i].channel == 161 || chan[i].channel == 153 || chan[i].channel == 64) &&
				     channelbw == 80)) {
					i++;
					continue;
				}
				int freq = chan[i].freq + 10;
				sprintf(cn, "%d", chan[i++].channel + 2);
				sprintf(fr, "%d", freq);
				if (iht > 0) {
					int sub = (base + 14 * 5) - freq;
					//                                      fprintf(stderr, "[%c] freq %d and base %d  sub %d\n", sub > 0 && sub < 70 ? 'x' : 'o', freq, (base + 14 * 5), sub);
					if (sub > 0 && sub < 70)
						continue;
				} else {
					int sub = freq - (base - 14 * 5);
					//                                      fprintf(stderr, "[%c] freq %d and base %d  sub %d\n", sub > 0 && sub < 70 ? 'x' : 'o', freq, (base - 14 * 5), sub);
					if (sub > 0 && sub < 70)
						continue;
				}
				int look = freq - 30;
				int i2 = 0;
				while (chan[i2].freq != -1) {
					if (look == chan[i2].freq) {
						//                                              fprintf(stderr, "add %d\n", look);
						goto found;
					}
					i2++;
				}
				continue;
found:;
				if (freq + offset >= 5310 && freq + offset < 5500)
					continue;
				if (!chan[i].lul && !chan[i].ull && !chan[i].ulu && !chan[i].luu)
					continue;
				websWrite(wp,
					  "document.write(\"<option value=\\\"%s\\\" %s>%s - %d \"+wl_basic.mhz+\"</option>\");\n",
					  fr, !strcmp(wlc, fr) ? " selected=\\\"selected\\\"" : "", cn, (freq + offset));
			}
			websWrite(wp, "//]]>\n</script></select></div>\n");
		}
		if (gotchannels)
			debug_free(chan);
#else
		int instance = 0;

		if (!strcmp(prefix, "wl1"))
			instance = 1;
		if (!strcmp(prefix, "wl2"))
			instance = 2;
		{
			unsigned int chanlist[128] = { 0 };
			char *ifn = get_wl_instance_name(instance);
			int chancount = getchannels(chanlist, ifn);
			int net_is_a = 0;

			if (chanlist[0] > 25)
				net_is_a = 1;

			int i, j;

			// supported 5GHz channels for IEEE 802.11n 40MHz
			int na_upper[16] = { 40, 48, 56, 64, 104, 112, 120, 128, 136, 153, 161, 0, 0, 0, 0, 0 };
			int na_lower[16] = { 36, 44, 52, 60, 100, 108, 116, 124, 132, 149, 157, 0, 0, 0, 0, 0 };

			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s>\" + share.auto + \"</option>\");\n",
				  nvram_nmatch("0", "%s_channel", prefix) ? "selected=\\\"selected\\\"" : "");
			for (i = 0; i < chancount; i++) {
				int ofs;

				if (chanlist[i] < 25)
					ofs = 2407;
				else
					ofs = 5000;
				ofs += (chanlist[i] * 5);
				if (ofs == 2477)
					ofs = 2484; // fix: ch 14 is 2.484, not 2.477 GHz
				//              websWrite( wp, ", \"%0.3f\"", ofs );
				char channelstring[32];

				int showit = 1;

				if (nvram_match(wl_net_mode, "a-only") || nvram_match(wl_net_mode, "na-only") ||
				    nvram_match(wl_net_mode, "n5-only") || nvram_match(wl_net_mode, "ac-only") ||
				    nvram_match(wl_net_mode, "acn-mixed") || nvram_match(wl_net_mode, "ax-only") ||
				    nvram_match(wl_net_mode, "xacn-mixed") || (net_is_a && nvram_match(wl_net_mode, "mixed"))) {
					if (chanlist[i] < 25)
						showit = 0;
				} else {
					if (chanlist[i] > 25)
						showit = 0;
				}

				if ((nvram_match(wl_net_mode, "na-only") || nvram_match(wl_net_mode, "ac-only") ||
				     nvram_match(wl_net_mode, "acn-mixed") || nvram_match(wl_net_mode, "ax-only") ||
				     nvram_match(wl_net_mode, "xacn-mixed") || (net_is_a && nvram_match(wl_net_mode, "mixed")) ||
				     nvram_match(wl_net_mode, "n5-only")) &&
				    nvram_matchi(wl_nbw, 40)) {
					showit = 0;
					j = 0;
					if (nvram_nmatch("upper", "%s_nctrlsb", prefix) ||
					    nvram_nmatch("uu", "%s_nctrlsb", prefix) || nvram_nmatch("lu", "%s_nctrlsb", prefix)) {
						while (na_upper[j]) {
							if (chanlist[i] == na_upper[j]) {
								showit = 1;
								break;
							}
							j++;
						}
					} else if (nvram_nmatch("lower", "%s_nctrlsb", prefix) ||
						   nvram_nmatch("ll", "%s_nctrlsb", prefix) ||
						   nvram_nmatch("ul", "%s_nctrlsb", prefix)) {
						while (na_lower[j]) {
							if (chanlist[i] == na_lower[j]) {
								showit = 1;
								break;
							}
							j++;
						}
					}
				}

				if ((nvram_match(wl_net_mode, "n-only") || nvram_match(wl_net_mode, "n2-only") ||
				     nvram_match(wl_net_mode, "ng-only") || (!net_is_a && nvram_match(wl_net_mode, "mixed"))) &&
				    nvram_matchi(wl_nbw, 40)) {
					showit = 0;
					if (nvram_nmatch("upper", "%s_nctrlsb", prefix) ||
					    nvram_nmatch("uu", "%s_nctrlsb", prefix) || nvram_nmatch("lu", "%s_nctrlsb", prefix)) {
						if (chanlist[i] >= 5 && chanlist[i] <= 13) {
							showit = 1;
						}
					} else if (nvram_nmatch("lower", "%s_nctrlsb", prefix) ||
						   nvram_nmatch("ll", "%s_nctrlsb", prefix) ||
						   nvram_nmatch("ul", "%s_nctrlsb", prefix)) {
						if (chanlist[i] <= 9) {
							showit = 1;
						}
					}
				}

				sprintf(channelstring, "%d", chanlist[i]);
				if (showit) {
					websWrite(
						wp,
						"document.write(\"<option value=\\\"%d\\\" %s>%d - %d.%d \"+wl_basic.ghz+\"</option>\");\n",
						chanlist[i],
						nvram_nmatch(channelstring, "%s_channel", prefix) ? "selected=\\\"selected\\\"" :
												    "",
						chanlist[i], ofs / 1000, ofs % 1000);
				}
			}
		}
		websWrite(wp, "//]]>\n</script></select></div>\n");
#endif
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
		show_caption(wp, "label", "wl_adv.label21", NULL);
		websWrite(wp, "<select name=\"%s_maxrate\">\n", prefix);
	} else {
		show_caption(wp, "label", "wl_adv.label23", NULL);
		websWrite(wp, "<select name=\"%s_minrate\">\n", prefix);
	}
	websWrite(wp, "<script type=\"text/javascript\">\n");
	websWrite(wp, "//<![CDATA[\n");
	char srate[32];

	sprintf(srate, "%s_minrate", prefix);
	char mxrate[32];

	sprintf(mxrate, "%s_maxrate", prefix);
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + share.auto + \"</option>\");\n",
		  nvram_matchi(srate, 0) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n");
	websWrite(wp, "</script>\n");
	char **rate;
	char **showrates = NULL;
	int len = 0;
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
		if (nvram_matchi(bw, 40)) {
			showrates = turbo_rates;
		}
		if (nvram_matchi(bw, 10)) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(char *);
		}
		if (nvram_matchi(bw, 5)) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(char *);
		}
		if (nvram_matchi(bw, 2)) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(char *);
		}
	}
	if (nvram_match(mode, "a-only")) {
		rate = ag_rates;
		len = sizeof(ag_rates) / sizeof(char *);
		if (nvram_matchi(bw, 40)) {
			showrates = turbo_rates;
		}
		if (nvram_matchi(bw, 10)) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(char *);
		}
		if (nvram_matchi(bw, 5)) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(char *);
		}
		if (nvram_matchi(bw, 2)) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(char *);
		}
	}
	if (nvram_match(mode, "bg-mixed")) {
		rate = bg_rates;
		len = sizeof(bg_rates) / sizeof(char *);
		if (nvram_matchi(bw, 10)) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(char *);
		}
		if (nvram_matchi(bw, 5)) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(char *);
		}
		if (nvram_matchi(bw, 2)) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(char *);
		}
	}
	if (nvram_match(mode, "mixed")) {
		rate = bg_rates;
		len = sizeof(bg_rates) / sizeof(char *);
		if (nvram_matchi(bw, 40)) {
			rate = ag_rates;
			len = sizeof(ag_rates) / sizeof(char *);
			showrates = turbo_rates;
		}
		if (nvram_matchi(bw, 10)) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(char *);
		}
		if (nvram_matchi(bw, 5)) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(char *);
		}
		if (nvram_matchi(bw, 2)) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(char *);
		}
	}
	int i;

	for (i = 0; i < len; i++) {
		if (maxrate) {
			int offset = 0;

			if (nvram_match(mode, "g-only") && nvram_matchi(bw, 20))
				offset = 4;
			char comp[32];

			sprintf(comp, "%d", i + 1 + offset);
			if (showrates)
				websWrite(wp, "<option value=\"%d\" %s >%s Mbit/s</option>\n", i + 1 + offset,
					  nvram_match(mxrate, comp) ? "selected=\"selected\"" : "", showrates[i]);
			else
				websWrite(wp, "<option value=\"%d\" %s >%s Mbit/s</option>\n", i + 1 + offset,
					  nvram_match(mxrate, comp) ? "selected=\"selected\"" : "", rate[i]);
		} else {
			int offset = 0;

			if (nvram_match(mode, "g-only") && nvram_matchi(bw, 20))
				offset = 4;
			char comp[32];

			sprintf(comp, "%d", i + 1 + offset);
			if (showrates)
				websWrite(wp, "<option value=\"%d\" %s >%s Mbit/s</option>\n", i + 1 + offset,
					  nvram_match(srate, comp) ? "selected=\"selected\"" : "", showrates[i]);
			else
				websWrite(wp, "<option value=\"%d\" %s >%s Mbit/s</option>\n", i + 1 + offset,
					  nvram_match(srate, comp) ? "selected=\"selected\"" : "", rate[i]);
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
	char wl_net_mode[32];

	sprintf(wl_net_mode, "%s_net_mode", prefix);

	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_basic.label2", NULL);
	websWrite(wp, "<select name=\"%s\">\n", wl_net_mode);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"disabled\\\" %s>\" + share.disabled + \"</option>\");\n",
		  nvram_match(wl_net_mode, "disabled") ? "selected=\\\"selected\\\"" : "");
	if (!has_ad(prefix))
		websWrite(wp, "document.write(\"<option value=\\\"mixed\\\" %s>\" + wl_basic.mixed + \"</option>\");\n",
			  nvram_match(wl_net_mode, "mixed") ? "selected=\\\"selected\\\"" : "");
	if (has_mimo(prefix) && has_2ghz(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"bg-mixed\\\" %s>\" + wl_basic.bg + \"</option>\");\n",
			  nvram_match(wl_net_mode, "bg-mixed") ? "selected=\\\"selected\\\"" : "");
	}
#ifdef HAVE_WHRAG108
	if (!strcmp(prefix, "wlan1"))
#endif
#ifdef HAVE_TW6600
		if (!strcmp(prefix, "wlan1"))
#endif

			if (has_2ghz(prefix)) {
				websWrite(wp,
					  "document.write(\"<option value=\\\"b-only\\\" %s>\" + wl_basic.b + \"</option>\");\n",
					  nvram_match(wl_net_mode, "b-only") ? "selected=\\\"selected\\\"" : "");
			}
#ifdef HAVE_MADWIFI
	if (has_2ghz(prefix)) {
#ifdef HAVE_WHRAG108
		if (!strcmp(prefix, "wlan1"))
#endif
#ifdef HAVE_TW6600
			if (!strcmp(prefix, "wlan1"))
#endif
				websWrite(wp,
					  "document.write(\"<option value=\\\"g-only\\\" %s>\" + wl_basic.g + \"</option>\");\n",
					  nvram_match(wl_net_mode, "g-only") ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_WHRAG108
		if (!strcmp(prefix, "wlan1"))
#endif
#ifdef HAVE_TW6600
			if (!strcmp(prefix, "wlan1"))
#endif
#if !defined(HAVE_LS5) || defined(HAVE_EOC5610)
				websWrite(wp,
					  "document.write(\"<option value=\\\"bg-mixed\\\" %s>\" + wl_basic.bg + \"</option>\");\n",
					  nvram_match(wl_net_mode, "bg-mixed") ? "selected=\\\"selected\\\"" : "");
#endif
	}
#else
#ifdef HAVE_WHRAG108
	if (!strcmp(prefix, "wlan1"))
#endif
#if !defined(HAVE_LS5) || defined(HAVE_EOC5610)

		if (has_2ghz(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"g-only\\\" %s>\" + wl_basic.g + \"</option>\");\n",
				  nvram_match(wl_net_mode, "g-only") ? "selected=\\\"selected\\\"" : "");
		}
	if (has_mimo(prefix) && has_2ghz(prefix) && !is_ath5k(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"ng-only\\\" %s>\" + wl_basic.ng + \"</option>\");\n",
			  nvram_match(wl_net_mode, "ng-only") ? "selected=\\\"selected\\\"" : "");
	}
#endif
#endif
	if (has_mimo(prefix) && has_2ghz(prefix) && !is_ath5k(prefix)) {
		if (has_5ghz(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"n2-only\\\" %s>\" + wl_basic.n2 + \"</option>\");\n",
				  nvram_match(wl_net_mode, "n2-only") ? "selected=\\\"selected\\\"" : "");
		} else {
			websWrite(wp, "document.write(\"<option value=\\\"n-only\\\" %s>\" + wl_basic.n + \"</option>\");\n",
				  nvram_match(wl_net_mode, "n-only") ? "selected=\\\"selected\\\"" : "");
		}
	}
	if (has_ax(prefix) && has_2ghz(prefix)) {
		if (has_5ghz(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"axg-only\\\" %s>\" + wl_basic.axg + \"</option>\");\n",
				  nvram_match(wl_net_mode, "n2-only") ? "selected=\\\"selected\\\"" : "");
		} else {
			websWrite(wp, "document.write(\"<option value=\\\"axg-only\\\" %s>\" + wl_basic.ax + \"</option>\");\n",
				  nvram_match(wl_net_mode, "n-only") ? "selected=\\\"selected\\\"" : "");
		}
	}
#if !defined(HAVE_FONERA) && !defined(HAVE_LS2) && !defined(HAVE_MERAKI)
#ifndef HAVE_MADWIFI

	if (has_5ghz(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"a-only\\\" %s>\" + wl_basic.a + \"</option>\");\n",
			  nvram_match(wl_net_mode, "a-only") ? "selected=\\\"selected\\\"" : "");
	}
	if (has_mimo(prefix) && has_5ghz(prefix) && !is_ath5k(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"na-only\\\" %s>\" + wl_basic.na + \"</option>\");\n",
			  nvram_match(wl_net_mode, "na-only") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"n5-only\\\" %s>\" + wl_basic.n5 + \"</option>\");\n",
			  nvram_match(wl_net_mode, "n5-only") ? "selected=\\\"selected\\\"" : "");
	}
	if (has_ac(prefix) && has_5ghz(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"acn-mixed\\\" %s>\" + wl_basic.acn + \"</option>\");\n",
			  nvram_match(wl_net_mode, "acn-mixed") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"ac-only\\\" %s>\" + wl_basic.ac + \"</option>\");\n",
			  nvram_match(wl_net_mode, "ac-only") ? "selected=\\\"selected\\\"" : "");
	}
	if (has_ax(prefix) && has_5ghz(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"xacn-mixed\\\" %s>\" + wl_basic.xacn + \"</option>\");\n",
			  nvram_match(wl_net_mode, "xacn-mixed") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"ax-only\\\" %s>\" + wl_basic.ax + \"</option>\");\n",
			  nvram_match(wl_net_mode, "xac-only") ? "selected=\\\"selected\\\"" : "");
	}
	if (has_ad(prefix)) {
		websWrite(wp, "document.write(\"<option value=\\\"ad-only\\\" %s>\" + wl_basic.ad + \"</option>\");\n",
			  nvram_match(wl_net_mode, "ad-only") ? "selected=\\\"selected\\\"" : "");
	}
#else
#if HAVE_WHRAG108
	if (!strcmp(prefix, "wlan0"))
#endif
#ifdef HAVE_TW6600
		if (!strcmp(prefix, "wlan0"))
#endif
			if (has_5ghz(prefix)) {
				websWrite(wp,
					  "document.write(\"<option value=\\\"a-only\\\" %s>\" + wl_basic.a + \"</option>\");\n",
					  nvram_match(wl_net_mode, "a-only") ? "selected=\\\"selected\\\"" : "");
			}
#endif

#endif
	if (is_mac80211(prefix)) {
		if (has_2ghz(prefix) && !is_ath5k(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"ng-only\\\" %s>\" + wl_basic.ng + \"</option>\");\n",
				  nvram_match(wl_net_mode, "ng-only") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"n2-only\\\" %s>\" + wl_basic.n2 + \"</option>\");\n",
				  nvram_match(wl_net_mode, "n2-only") ? "selected=\\\"selected\\\"" : "");
		}
		if (has_5ghz(prefix) && !is_ath5k(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"na-only\\\" %s>\" + wl_basic.na + \"</option>\");\n",
				  nvram_match(wl_net_mode, "na-only") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"n5-only\\\" %s>\" + wl_basic.n5 + \"</option>\");\n",
				  nvram_match(wl_net_mode, "n5-only") ? "selected=\\\"selected\\\"" : "");
		}
		if ((is_ath10k(prefix) || is_ath11k(prefix) || is_mvebu(prefix) || has_vht80(prefix)) && has_5ghz(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"acn-mixed\\\" %s>\" + wl_basic.acn + \"</option>\");\n",
				  nvram_match(wl_net_mode, "acn-mixed") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"ac-only\\\" %s>\" + wl_basic.ac + \"</option>\");\n",
				  nvram_match(wl_net_mode, "ac-only") ? "selected=\\\"selected\\\"" : "");
		}
		if (is_ath11k(prefix) && has_5ghz(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"xacn-mixed\\\" %s>\" + wl_basic.xacn + \"</option>\");\n",
				  nvram_match(wl_net_mode, "xacn-mixed") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"ax-only\\\" %s>\" + wl_basic.ax + \"</option>\");\n",
				  nvram_match(wl_net_mode, "ax-only") ? "selected=\\\"selected\\\"" : "");
		}
		if (has_ad(prefix)) {
			websWrite(wp, "document.write(\"<option value=\\\"ad-only\\\" %s>\" + wl_basic.ad + \"</option>\");\n",
				  nvram_match(wl_net_mode, "ad-only") ? "selected=\\\"selected\\\"" : "");
		}
	}

	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
	if (nvram_nmatch("n-only", "%s_net_mode", prefix)) {
		char wl_greenfield[32];

		sprintf(wl_greenfield, "%s_greenfield", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.label7", NULL);
		websWrite(wp, "<select name=\"%s\" >\n", wl_greenfield);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s>\" + wl_basic.mixed + \"</option>\");\n",
			  nvram_default_matchi(wl_greenfield, 0, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s>\" + wl_basic.greenfield + \"</option>\");\n",
			  nvram_default_matchi(wl_greenfield, 1, 0) ? "selected=\\\"selected\\\"" : "");
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
	char wl_protmode[32];
	sprintf(wl_protmode, "%s_protmode", var);
	showOptionsLabel(wp, "wl_basic.protmode", wl_protmode, "None CTS RTS/CTS", nvram_default_get(wl_protmode, "None"));

	strcpy(vvar, var);
	rep(vvar, '.', 'X');
	sprintf(ssid, "%s_rts", var);
	websWrite(
		wp,
		"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.rts)</script></div>\n");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idrts', true);\" name=\"%s_rts\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		vvar, var, nvram_default_matchi(ssid, 1, 0) ? "checked=\"checked\"" : "");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idrts', false);\" name=\"%s_rts\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
		vvar, var, nvram_default_matchi(ssid, 0, 0) ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div id=\"%s_idrts\">\n", vvar);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_basic.rtsvalue", NULL);
	char ip[32];

	sprintf(ip, "%s_rtsvalue", var);
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"4\" size=\"4\" onblur=\"valid_range(this,1,2346,share.ip)\" name=\"%s_rtsvalue\" value=\"%s\" />",
		var, nvram_default_get(ip, "500"));
	websWrite(wp, "</div>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<script>\n//<![CDATA[\n ");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_rts\"), \"%s_idrts\", %s);\n", var, vvar,
		  nvram_matchi(ssid, 1) ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");
}

#ifdef HAVE_WPA3
static void showairtimepolicy(webs_t wp, char *var, char *sub)
{
	if (nvram_nmatch("sta", "%s_mode", sub))
		return;
	if (nvram_nmatch("wdssta", "%s_mode", sub))
		return;
	if (nvram_nmatch("wdssta_mtik", "%s_mode", sub))
		return;
	char vvar[32];
	char wl_airtime[32];
	if (has_airtime_policy(var)) {
		strcpy(vvar, var);
		rep(vvar, '.', 'X');
		sprintf(wl_airtime, "%s_at_policy", var);
		nvram_default_get(wl_airtime, "0");

		if (!strcmp(var, sub)) {
			char *vifs = nvram_nget("%s_vifs", sub);
			char *m = malloc(strlen(vifs) + 1);
			strcpy(m, vifs);
			rep(m, '.', 'X');

			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.airtime_policy", NULL);
			websWrite(wp,
				  "<select name=\"%s_at_policy\" onclick=\"show_airtime_policy(this.form, '%s', '%s', '%s');\">\n",
				  var, var, vvar, m);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + share.disabled + \"</option>\");\n",
				  nvram_match(wl_airtime, "0") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp,
				  "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.airtime_dynamic + \"</option>\");\n",
				  nvram_match(wl_airtime, "1") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp,
				  "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.airtime_limit + \"</option>\");\n",
				  nvram_match(wl_airtime, "2") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script>\n</select>\n");
			websWrite(wp, "</div>\n");
			debug_free(m);
		}
		websWrite(wp, "<div id=\"%s_idairtimeweight\">\n", vvar);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.airtime_weight", NULL);
		char ip[32];
		sprintf(ip, "%s_at_weight", var);
		websWrite(
			wp,
			"<input class=\"num\" maxlength=\"4\" size=\"4\" onblur=\"valid_range(this,0,65536,share.ip)\" name=\"%s_at_weight\" value=\"%s\" />",
			var, nvram_default_get(ip, "1"));
		websWrite(wp, "</div>\n");
		websWrite(wp, "</div>\n");

		sprintf(ip, "%s_at_limit", var);
		websWrite(wp, "<div id=\"%s_idairtimelimit\">\n", vvar);
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.airtime_dolimit)</script></div>\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_at_limit\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			var, nvram_default_matchi(ip, 1, 0) ? "checked=\"checked\"" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_at_limit\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
			var, nvram_default_matchi(ip, 0, 0) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</div>\n");

		websWrite(wp, "<script>\n//<![CDATA[\n ");
		websWrite(wp, "show_airtime_policy(document.wireless, \"%s\", \"%s\", '');\n", sub, vvar);
		websWrite(wp, "//]]>\n</script>\n");
	}
}
#else
#define showairtimepolicy(wp, var, sub)
#endif

#endif
static void showbridgesettings(webs_t wp, char *var, int mcast, int dual)
{
	char ssid[32];

	sprintf(ssid, "%s_bridged", var);
	char vvar[32];

	strcpy(vvar, var);
	rep(vvar, '.', 'X');
	int iswds = 0;
	if (!strncmp(var, "wlan", 4) && strstr(var, ".sta"))
		iswds = 1;
	if (!iswds && has_multicast_to_unicast(var) && !nvram_nmatch("0", "%s_bridged", var)) {
		char unicast[32];
		sprintf(unicast, "%s_multicast_to_unicast", var);
		nvram_default_get(unicast, "0");
		showRadio(wp, "networking.unicast", unicast);
	}
	websWrite(
		wp,
		"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.network)</script></div>\n");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idnetvifs', false);\" name=\"%s_bridged\" %s><script type=\"text/javascript\">Capture(wl_basic.bridged)</script></input>\n",
		vvar, var, nvram_default_matchi(ssid, 1, 1) ? "checked=\"checked\"" : "");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idnetvifs', true);\" name=\"%s_bridged\" %s><script type=\"text/javascript\">Capture(wl_basic.unbridged)</script></input>\n",
		vvar, var, nvram_default_matchi(ssid, 0, 1) ? "checked=\"checked\"" : "");
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

		sprintf(natvar, "%s_bloop", var);
		nvram_default_get(natvar, nvram_safe_get("block_loopback"));
		showRadio(wp, "filter.nat", natvar);
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
		vvar, var, nvram_default_matchi(redirect, 1, 0) ? "checked=\"checked\"" : "");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idredirect', false);\" name=\"%s_dns_redirect\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
		vvar, var, nvram_default_matchi(redirect, 0, 0) ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div id=\"%s_idredirect\">\n", vvar);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "idx.dns_redirect", NULL);
	websWrite(wp, "<input type=\"hidden\" name=\"%s_dns_ipaddr\" value=\"4\" />\n", var);
	show_ip(wp, var, "dns_ipaddr", 0, 0, "share.ip");
	websWrite(wp, "</div>\n");

	websWrite(wp, "</div>\n");
	websWrite(wp, "<script>\n//<![CDATA[\n ");
	websWrite(wp, "//]]>\n</script>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.ip", NULL);
	websWrite(wp, "<input type=\"hidden\" name=\"%s_ipaddr\" value=\"4\" />\n", var);
	char netmask[32];
	sprintf(netmask, "%s_netmask", var);
	show_ip_cidr(wp, var, "ipaddr", 0, "share.ip", netmask, "share.subnet");
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
	       var, vvar, nvram_matchi( dl,1)) ? "true" : "false" );
    websWrite( wp, "//]]>\n</script>\n" );
}*/
#endif

	websWrite(wp, "</div>\n");

	websWrite(wp, "<script>\n//<![CDATA[\n ");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_bridged\"), \"%s_idnetvifs\", %s);\n", var, vvar,
		  nvram_matchi(ssid, 0) ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_dns_redirect\"), \"%s_idredirect\", %s);\n", var, vvar,
		  nvram_matchi(redirect, 1) ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");
}

#ifdef HAVE_MADWIFI
static void show_chanshift(webs_t wp, char *prefix)
{
	char wl_chanshift[32];
	char wl_channelbw[32];

	sprintf(wl_channelbw, "%s_channelbw", prefix);
	sprintf(wl_chanshift, "%s_chanshift", prefix);
	if (nvram_geti(wl_channelbw) > 2 && (nvram_geti(wl_chanshift) & 0xf) > 10)
		nvram_seti(wl_chanshift, 10);
	if (nvram_geti(wl_channelbw) > 5 && (nvram_geti(wl_chanshift) & 0xf) > 10)
		nvram_seti(wl_chanshift, 10);
	if (nvram_geti(wl_channelbw) > 10 && (nvram_geti(wl_chanshift) & 0xf) > 0)
		nvram_seti(wl_chanshift, 0);

	if (nvram_matchi(wl_channelbw, 5) || nvram_matchi(wl_channelbw, 10) || nvram_matchi(wl_channelbw, 2)) {
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.chanshift", NULL);
		websWrite(wp, "<select name=\"%s\">\n", wl_chanshift);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		if (nvram_matchi(wl_channelbw, 5) || nvram_matchi(wl_channelbw, 2))
			websWrite(wp, "document.write(\"<option value=\\\"-15\\\" %s >-15 \"+wl_basic.mhz+\"</option>\");\n",
				  nvram_default_matchi(wl_chanshift, -15, 0) ? "selected=\\\"selected\\\"" : "");
		if (nvram_matchi(wl_channelbw, 5) || nvram_matchi(wl_channelbw, 10) || nvram_matchi(wl_channelbw, 2))
			websWrite(wp, "document.write(\"<option value=\\\"-10\\\" %s >-10 \"+wl_basic.mhz+\"</option>\");\n",
				  nvram_default_matchi(wl_chanshift, -10, 0) ? "selected=\\\"selected\\\"" : "");
		if (nvram_matchi(wl_channelbw, 5) || nvram_matchi(wl_channelbw, 10) || nvram_matchi(wl_channelbw, 2))
			websWrite(wp, "document.write(\"<option value=\\\"-5\\\" %s >-5 \"+wl_basic.mhz+\"</option>\");\n",
				  nvram_default_matchi(wl_chanshift, -5, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >0 \"+wl_basic.mhz+\"</option>\");\n",
			  nvram_default_matchi(wl_chanshift, 0, 0) ? "selected=\\\"selected\\\"" : "");
		if (nvram_matchi(wl_channelbw, 5) || nvram_matchi(wl_channelbw, 10) || nvram_matchi(wl_channelbw, 2))
			websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >+5 \"+wl_basic.mhz+\"</option>\");\n",
				  nvram_default_matchi(wl_chanshift, 5, 0) ? "selected=\\\"selected\\\"" : "");
		if (nvram_matchi(wl_channelbw, 5) || nvram_matchi(wl_channelbw, 10) || nvram_matchi(wl_channelbw, 2))
			websWrite(wp, "document.write(\"<option value=\\\"10\\\" %s >+10 \"+wl_basic.mhz+\"</option>\");\n",
				  nvram_default_matchi(wl_chanshift, 10, 0) ? "selected=\\\"selected\\\"" : "");
		if (nvram_matchi(wl_channelbw, 5) || nvram_matchi(wl_channelbw, 2))
			websWrite(wp, "document.write(\"<option value=\\\"15\\\" %s >+15 \"+wl_basic.mhz+\"</option>\");\n",
				  nvram_default_matchi(wl_chanshift, 15, 0) ? "selected=\\\"selected\\\"" : "");
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
	char uapsd[32];
	char wl_protmode[32];
#endif
	sprintf(vif, "%s_vifs", prefix);
	char *vifs = nvram_safe_get(vif);
	if (vifs == NULL)
		return 0;
#ifndef HAVE_MADWIFI
	if (!nvram_nmatch("ap", "%s_mode", prefix) && !nvram_nmatch("apsta", "%s_mode", prefix) &&
	    !nvram_nmatch("apstawet", "%s_mode", prefix))
		return 0;
#endif
	int count = 1;

#ifdef HAVE_MADWIFI
	if ((is_mac80211(prefix) && getmaxvaps(prefix) > 1) || !is_mac80211(prefix))
#endif
		websWrite(wp, "<h2><script type=\"text/javascript\">Capture(wl_basic.h2_vi)</script></h2>\n");
	foreach(var, vifs, next)
	{
#ifdef HAVE_GUESTPORT
		if (nvram_match(guestport, var)) {
			count++;
			continue;
		}
#endif
		sprintf(ssid, "%s_ssid", var);
		websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(share.vintrface)</script> %s SSID [",
			  getNetworkLabel(wp, IFMAP(var)));
		tf_webWriteESCNV(wp, ssid); // fix for broken html page if ssid
		// contains html tag
		char wl_macaddr[18];
		sprintf(wl_macaddr, "%s_hwaddr", var);
		if (nvram_exists(wl_macaddr))
			websWrite(wp, "] HWAddr [%s", nvram_safe_get(wl_macaddr));
		websWrite(wp, "]</legend>\n");
		websWrite(wp, "<div class=\"setting\">\n");
#if (!defined(HAVE_EASY_WIRELESS_CONFIG) || defined(HAVE_BCMMODERN)) && !defined(HAVE_BRCMFMAC)
		show_caption(wp, "label", "wl_basic.ssid", NULL);
		websWrite(wp,
			  "<input name=\"%s_ssid\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.ssid)\" value=\"",
			  var);
		tf_webWriteESCNV(wp, ssid);
		websWrite(wp, "\" /></div>\n");

#ifdef HAVE_MADWIFI
		//      sprintf( wl_chanshift, "%s_chanshift", var );
		//      show_chanshift( wp, wl_chanshift );

		showrtssettings(wp, var);
		showairtimepolicy(wp, var, prefix);
#endif

		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.label5", NULL);
		sprintf(ssid, "%s_closed", var);
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			var, nvram_matchi(ssid, 0) ? "checked=\"checked\"" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			var, nvram_matchi(ssid, 1) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		char wl_mode[16];

#ifdef HAVE_MADWIFI
		sprintf(wl_mode, "%s_mode", var);
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n",
			wl_mode);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");\n",
			  nvram_match(wl_mode, "ap") ? "selected=\\\"selected\\\"" : "");
		if (has_wdsap(var)) {
			websWrite(wp, "document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");\n",
				  nvram_match(wl_mode, "wdsap") ? "selected=\\\"selected\\\"" : "");
		}
#if 0
		if (has_mesh(var))
			websWrite(wp, "document.write(\"<option value=\\\"mesh\\\" %s >\" + wl_basic.mesh + \"</option>\");\n", nvram_match(wl_mode, "mesh") ? "selected=\\\"selected\\\"" : "");
#endif
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
//              sprintf(wmm, "%s_wmm", var);
//              showRadio(wp, "wl_adv.label18", wmm);
#endif
		char webfilter[32];
		sprintf(webfilter, "%s_web_filter", var);
		showRadioDefaultOn(wp, "wl_adv.label17", webfilter);
#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
		showbridgesettings(wp, getRADev(var), 1, 0);
#else
		showbridgesettings(wp, var, 1, 0);
#endif

#else // start EASY_WIRELESS_SETUP

		// wireless mode
		char wl_mode[16];

#ifdef HAVE_MADWIFI
		sprintf(wl_mode, "%s_mode", var);
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n",
			wl_mode);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"disabled\\\" %s >\" + share.disabled + \"</option>\");\n",
			  nvram_match(wl_mode, "disabled") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");\n",
			  nvram_match(wl_mode, "ap") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");\n",
			  nvram_match(wl_mode, "wdsap") ? "selected=\\\"selected\\\"" : "");
#if 0
		if (has_mesh(var))
			websWrite(wp, "document.write(\"<option value=\\\"mesh\\\" %s >\" + wl_basic.mesh + \"</option>\");\n", nvram_match(wl_mode, "mesh") ? "selected=\\\"selected\\\"" : "");
#endif
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");

		show_caption(wp, "label", "wl_basic.ssid", NULL);
		websWrite(wp,
			  "<input name=\"%s_ssid\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.ssid)\" value=\"",
			  var);
		tf_webWriteESCNV(wp, ssid);
		websWrite(wp, "\" /></div>\n");

		// broadcast wireless ssid
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.label5", NULL);
		sprintf(ssid, "%s_closed", var);
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			var, nvram_matchi(ssid, 0) ? "checked=\"checked\"" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			var, nvram_matchi(ssid, 1) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
#endif

#ifdef HAVE_IFL
		// label
		char wl_label[16];
		sprintf(wl_label, "%s_label", var);
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.if_label)</script></div><input type=\"text\" name=\"%s\" value=\"%s\" maxlength=\"20\"></div>\n",
			wl_label, nvram_safe_get(wl_label));

#endif

		// WIRELESS Advanced
		char advanced_label[32];
		char maskvar[32];
		strcpy(maskvar, var);
		rep(maskvar, '.', 'X');
		sprintf(advanced_label, "%s_wl_advanced", maskvar);
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.legend)</script></div>\n");
		websWrite(wp, "<input type=\"checkbox\" name=\"%s\" onclick=\"toggle_layer(this,'%s_layer')\"%s>", advanced_label,
			  advanced_label, websGetVar(wp, advanced_label, NULL) ? " checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div id=\"%s_layer\"%s>\n", advanced_label,
			  websGetVar(wp, advanced_label, NULL) ? "" : " style=\"display: none;\"");

#ifdef HAVE_IFL

		char wl_note[16];
		sprintf(wl_note, "%s_note", var);
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.if_info)</script></div><textarea name=\"%s\" cols=\"60\" rows=\"3\">%s</textarea></div>\n",
			wl_note, nvram_safe_get(wl_note));
#endif

#ifdef HAVE_MADWIFI
		//      sprintf( wl_chanshift, "%s_chanshift", var );
		//      show_chanshift( wp, wl_chanshift );

		showrtssettings(wp, var);
		showairtimepolicy(wp, var, prefix);
#ifdef HAVE_MAC80211_COMPRESS
		if (is_mac80211(prefix) && !is_mvebu(prefix)) {
			char wl_fc[16];
			sprintf(wl_fc, "%s_fc", var);
			FILE *fp = fopen("/proc/modules", "rb");
			char line[245];
			int zstd = 0;
			if (fp) {
				while (!feof(fp) && fgets(line, sizeof(line), fp)) {
					if (strstr(line, "zstd")) {
						zstd = 1;
						break;
					}
				}
				fclose(fp);
			}
			char *names_zstd[] = { "\" + share.disabled + \"", "LZO", "LZ4", "LZMA", "ZSTD" };
			char *names[] = {
				"\" + share.disabled + \"",
				"LZO",
				"LZ4",
				"LZMA",
			};
			if (zstd)
				showOptionsNames(wp, "wl_basic.fc", wl_fc, "0 1 3 2 4", names_zstd, nvram_default_get(wl_fc, "0"));
			else
				showOptionsNames(wp, "wl_basic.fc", wl_fc, "0 1 3 2", names, nvram_default_get(wl_fc, "0"));
		}
#endif
		sprintf(wmm, "%s_wmm", var);
		if (is_mac80211(var))
			showRadioDefaultOn(wp, "wl_adv.label18", wmm);
		else
			showRadio(wp, "wl_adv.label18", wmm);
		sprintf(uapsd, "%s_uapsd", var);
		if (has_uapsd(prefix)) {
			showRadio(wp, "wl_basic.uapsd", uapsd);
		}
#endif

#endif // end BUFFALO
		sprintf(ssid, "%s_ap_isolate", var);
		showRadio(wp, "wl_adv.label11", ssid);
#if 0 //def HAVE_80211AC
#ifndef HAVE_NOAC
		if (!has_qtn(var)) {
			char wl_igmp[16];
			sprintf(wl_igmp, "%s_wmf_bss_enable", var);
			nvram_default_get(wl_igmp, "0");
			showRadio(wp, "wl_basic.igmpsnooping", wl_igmp);
		}
#endif
#endif
#ifdef HAVE_MADWIFI

		if (is_ap(var) || nvram_nmatch("mesh", "%s_mode", var) || nvram_nmatch("infra", "%s_mode", var)) {
			sprintf(power, "%s_maxassoc", var);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_adv.label10", NULL);
			websWrite(
				wp,
				"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,256,wl_adv.label10)\" value=\"%s\" />\n",
				power, nvram_default_get(power, "256"));

			websWrite(
				wp,
				"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 256 \" + share.user + \")\");\n//]]>\n</script></span>\n");
			websWrite(wp, "</div>\n");
		}

		char dtim[32];
		sprintf(dtim, "%s_dtim", var);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_adv.label7", NULL);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,1,255,wl_adv.label7)\" value=\"%s\" />\n",
			dtim, nvram_default_get(dtim, "2"));
		websWrite(wp, "</div>\n");

		if (is_mac80211(var)) {
			if (is_ap(var)) {
				websWrite(
					wp,
					"<fieldset><legend><script type=\"text/javascript\">Capture(wl_adv.droplowsignal)</script></legend>");
				char signal[32];
				sprintf(signal, "%s_connect", var);
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_adv.connect", NULL);
				websWrite(
					wp,
					"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,-128,0,wl_adv.connect)\" value=\"%s\" />\n",
					signal, nvram_default_get(signal, "-128"));
				websWrite(wp, "</div>\n");

				sprintf(signal, "%s_stay", var);
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_adv.stay", NULL);
				websWrite(
					wp,
					"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,-128,0,wl_adv.stay)\" value=\"%s\" />\n",
					signal, nvram_default_get(signal, "-128"));
				websWrite(wp, "</div>\n");

				sprintf(signal, "%s_poll_time", var);
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_adv.poll_time", NULL);
				websWrite(
					wp,
					"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,1,3600,wl_adv.poll_time)\" value=\"%s\" />\n",
					signal, nvram_default_get(signal, "10"));
				websWrite(wp, "</div>\n");

				sprintf(signal, "%s_strikes", var);
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_adv.strikes", NULL);
				websWrite(
					wp,
					"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,1,60,wl_adv.strikes)\" value=\"%s\" />\n",
					signal, nvram_default_get(signal, "3"));
				websWrite(wp, "</div>\n");

				websWrite(wp, "</fieldset><br/>\n");
			} else if (is_supplicant(var)) {
				show_bgscan_options(wp, var);
			}
		}
		char webfilter[32];
		sprintf(webfilter, "%s_web_filter", var);
		showRadioDefaultOn(wp, "wl_adv.label17", webfilter);
#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
		showbridgesettings(wp, getRADev(var), 1, 0);
#else
		showbridgesettings(wp, var, 1, 0);
#endif
		websWrite(wp, "</div>\n");
#endif

		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.remove + \"\\\" onclick=\\\"vifs_remove_submit(this.form,'%s','%d')\\\" />\");\n//]]>\n</script>\n",
			prefix, count - 1);
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + share.copy + \"\\\" onclick=\\\"copy_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
			var);
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + share.paste + \"\\\" onclick=\\\"paste_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
			var);
		websWrite(wp, "</fieldset><br />\n");
		count++;
		if (is_ap8x(prefix) && count == 4) {
			websWrite(wp, "<div class=\"warning\">\n");
			websWrite(wp, "<p><script type=\"text/javascript\">Capture(wl_basic.ap83_vap_note)</script></p>\n");
			websWrite(wp, "</div>\n<br>\n");
		}
	}
#ifndef HAVE_GUESTPORT
	websWrite(wp, "<div class=\"center\">\n");
#ifdef HAVE_MADWIFI
	if ((is_mac80211(prefix) && count < getmaxvaps(prefix)) || (!is_mac80211(prefix) && count < 8))
#elif HAVE_RT2880
	if (count < 7)
#else
	int max = get_maxbssid(prefix);
	if (has_qtn(prefix))
		max = 3;
	if (count < max)
#endif
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + wl_basic.add + \"\\\" onclick=\\\"vifs_add_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
			prefix);
	websWrite(wp, "</div><br />\n");
#endif
#ifdef HAVE_GUESTPORT
	int gpfound = 0;
	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(share.guest_port)</script></h2>\n");
	foreach(var, vifs, next)
	{
		if (nvram_match(guestport, var)) {
			gpfound = 1;
			sprintf(ssid, "%s_ssid", var);
			websWrite(wp,
				  "<fieldset><legend><script type=\"text/javascript\">Capture(share.vintrface)</script> %s SSID [",
				  getNetworkLabel(wp, IFMAP(var)));
			tf_webWriteESCNV(wp, ssid); // fix for broken html page if ssid
			// contains html tag
			char wl_macaddr[18];
			sprintf(wl_macaddr, "%s_hwaddr", var);
			if (nvram_exists(wl_macaddr))
				websWrite(wp, "] HWAddr [%s", nvram_safe_get(wl_macaddr));
			websWrite(wp, "]</legend>\n");
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.ssid", NULL);
			websWrite(
				wp,
				"<input name=\"%s_ssid\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.ssid)\" value=\"",
				var);
			tf_webWriteESCNV(wp, ssid);
			websWrite(wp, "\" /></div>\n");
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.label5", NULL);
			sprintf(ssid, "%s_closed", var);
			websWrite(
				wp,
				"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
				var, nvram_matchi(ssid, 0) ? "checked=\"checked\"" : "");
			websWrite(
				wp,
				"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
				var, nvram_matchi(ssid, 1) ? "checked=\"checked\"" : "");
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "share.ip", NULL);
			show_ip(wp, var, "ipaddr", 0, 0, "share.ip");
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "share.subnet", NULL);
			show_ip(wp, var, "ipaddr", 1, 0, "share.subnet");
			websWrite(wp, "</div>\n");
			sprintf(ssid, "%s_ap_isolate", var);
			showRadio(wp, "wl_adv.label11", ssid);
#ifdef HAVE_MADWIFI
			if (is_ap(var) || nvram_nmatch("mesh", "%s_mode", var) || nvram_nmatch("infra", "%s_mode", var)) {
				sprintf(power, "%s_maxassoc", var);
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_adv.label10", NULL);
				websWrite(
					wp,
					"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,256,wl_adv.label10)\" value=\"%s\" />\n",
					power, nvram_default_get(power, "256"));
				websWrite(
					wp,
					"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 256 \" + share.user + \")\");\n//]]>\n</script></span>\n");
				websWrite(wp, "</div>\n");
			}
#endif // GUESTPORT
			websWrite(wp, "</fieldset><br />\n");
		}
	}

#ifdef HAVE_MADWIFI
	if (gpfound == 0 && ((is_mac80211(prefix) && count < getmaxvaps(prefix)) || (!is_mac80211(prefix) && count < 8)))
#elif HAVE_RT2880
	if (count < 7 && gpfound == 0)
#else
	int max = get_maxbssid(prefix);
	if (has_qtn(prefix))
		max = 3;
	if (count < max && gpfound == 0)
#endif
		websWrite(wp, "<div class=\"center\">\n");
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + wl_basic.add + \"\\\" onclick=\\\"$('gp_modify').value='add';vifs_add_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
		prefix);
	websWrite(wp, "</div><br />\n");
#endif
	return 0;
}

EJ_VISIBLE void ej_getdefaultindex(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_BUFFALO
	websWrite(wp, "SetupAssistant.asp");
#else
	websWrite(wp, "index.asp");
#endif
}

EJ_VISIBLE void ej_show_countrylist(webs_t wp, int argc, char_t **argv)
{
	if (argc < 1) {
		return;
	}
	if (nvram_matchi("nocountrysel", 1))
		return;
	char *list = getCountryList(NULL);
	showOptionsChoose(wp, argv[0], list, NULL, nvram_safe_get(argv[0]));
}

static void mesh_num(webs_t wp, char *prefix, char *name, int len, int def)
{
	char mparam[64];
	sprintf(mparam, "%s_%s", prefix, name);
	nvram_default_geti(mparam, def);
	char label[64];
	sprintf(label, "wl_basic.%s", name);
	show_inputlabel(wp, label, mparam, len, "num", len);
}

static void mesh_radio(webs_t wp, char *prefix, char *name, int def)
{
	char mparam[64];
	sprintf(mparam, "%s_%s", prefix, name);
	char label[64];
	sprintf(label, "wl_basic.%s", name);
	showRadioNoDef(wp, label, mparam, nvram_default_geti(mparam, def));
}

static void internal_ej_show_wireless_single(webs_t wp, char *prefix)
{
	char wl_mode[16];
	char wl_macaddr[18];
	char wl_ssid[16];
	char frequencies[256];
	char wl_outdoor[16];
	char wl_diversity[32];
	char wl_rxantenna[32];
	char wl_txantenna[32];
	char wl_width[32];
	char wl_preamble[16];
	char wl_xr[16];
	char wl_comp[32];
	char wl_ff[16];
	char wmm[32];
	char wl_uapsd[32];
	char wl_lowack[32];
	char wl_ldpc[32];
	char wl_smps[32];
	char wl_isolate[32];
	char wl_intmit[32];
	char wl_qboost[32];
	char wl_autoburst[32];
	char wl_sifs_trigger_time[32];
	char wl_noise_immunity[32];
	char wl_ofdm_weak_det[32];
	char wl_protmode[32];
	char wl_doth[32];
	char wl_csma[32];
	char wl_shortgi[32];
	char wl_subf[32];
	char wl_mubf[32];
	char wl_bssid[32];
	char *chipset = getWifiDeviceName(prefix, NULL);

#ifdef HAVE_MADWIFI
	int maxvaps = getmaxvaps(prefix);
#elif HAVE_RT2880
	int maxvaps = 7;
#else
	int maxvaps = get_maxbssid(prefix);
	if (has_qtn(prefix))
		maxvaps = 3;
#endif

	sprintf(wl_mode, "%s_mode", prefix);
	sprintf(wl_macaddr, "%s_hwaddr", prefix);
	sprintf(wl_ssid, "%s_ssid", prefix);
	// check the frequency capabilities;
	if (has_ad(prefix)) {
		sprintf(frequencies,
			" <script type=\"text/javascript\">document.write(\"[60\"+wl_basic.ghz+\" 802.11ad]%s%s - Max Vaps(%d)\");</script>",
			chipset ? " - " : "", chipset ? chipset : "", maxvaps);
	} else if (has_2ghz(prefix) && has_5ghz(prefix) && has_ac(prefix)) {
		sprintf(frequencies,
			" <script type=\"text/javascript\">document.write(\"[2.4\"+wl_basic.ghz+\"/5 \"+wl_basic.ghz+\"/802.11ac]%s%s - Max Vaps(%d)\");</script>",
			chipset ? " - " : "", chipset ? chipset : "", maxvaps);
	} else if (has_5ghz(prefix) && has_ac(prefix)) {
		sprintf(frequencies,
			" <script type=\"text/javascript\">document.write(\"[5 \"+wl_basic.ghz+\"/802.11ac]%s%s - Max Vaps(%d)\");</script>",
			chipset ? " - " : "", chipset ? chipset : "", maxvaps);
	} else if (has_5ghz(prefix) && has_2ghz(prefix)) {
		sprintf(frequencies,
			" <script type=\"text/javascript\">document.write(\"[2.4 \"+wl_basic.ghz+\"/5 \"+wl_basic.ghz+\"]%s%s - Max Vaps(%d)\");</script>",
			chipset ? " - " : "", chipset ? chipset : "", maxvaps);
	} else if (has_5ghz(prefix)) {
		sprintf(frequencies,
			" <script type=\"text/javascript\">document.write(\"[5 \"+wl_basic.ghz+\"]%s%s - Max Vaps(%d)\")</script>",
			chipset ? " - " : "", chipset ? chipset : "", maxvaps);
	} else if (has_2ghz(prefix) && has_ac(prefix)) {
		sprintf(frequencies,
			" <script type=\"text/javascript\">document.write(\"[2.4 \"+wl_basic.ghz+\" \"+wl_basic.tbqam+\"]%s%s - Max Vaps(%d)\")</script>",
			chipset ? " - " : "", chipset ? chipset : "", maxvaps);
	} else if (has_2ghz(prefix)) {
		sprintf(frequencies,
			" <script type=\"text/javascript\">document.write(\"[2.4 \"+wl_basic.ghz+\"]%s%s - Max Vaps(%d)\")</script>",
			chipset ? " - " : "", chipset ? chipset : "", maxvaps);
	} else {
		frequencies[0] = 0;
	}
	// wireless mode
	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(wl_basic.h2_v24)</script> %s%s</h2>\n", prefix, frequencies);
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.pintrface)</script> %s - SSID [",
		  getNetworkLabel(wp, IFMAP(prefix)));
	tf_webWriteESCNV(wp, wl_ssid); // fix
	sprintf(wl_macaddr, "%s_hwaddr", prefix);
	websWrite(wp, "] HWAddr [%s]</legend>\n", nvram_safe_get(wl_macaddr));
	char power[16];
#if !defined(HAVE_EASY_WIRELESS_CONFIG) || defined(HAVE_BCMMODERN) && !defined(HAVE_BRCMFMAC)
	// char maxpower[16];
	if (is_mac80211(prefix)) {
		if (isFXXN_PRO(prefix) == 1) {
			char wl_cardtype[32];
			sprintf(wl_cardtype, "%s_cardtype", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.cardtype", NULL);
			websWrite(wp, "<select name=\"%s\">\n", wl_cardtype);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
#ifdef HAVE_ONNET
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Atheros 2458</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 0, 0) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Atheros 3336</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 1, 0) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Atheros 5964</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 2, 0) ? "selected=\\\"selected\\\"" : "");
#else
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Atheros Generic</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 0, 0) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >DBII F36N-PRO</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 1, 0) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >DBII F64N-PRO</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 2, 0) ? "selected=\\\"selected\\\"" : "");
#endif
			websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
		}
	}
#ifdef HAVE_MADWIFI
#ifndef HAVE_MAKSAT
#ifndef HAVE_DDLINK

	if (isXR36(prefix)) {
		char wl_cardtype[32];
		sprintf(wl_cardtype, "%s_cardtype", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.cardtype", NULL);
		websWrite(wp, "<select name=\"%s\">\n", wl_cardtype);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Ubiquiti XR3.3</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 0, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Ubiquiti XR3.6</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 1, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Ubiquiti XR3.7</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 2, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
	}

	if (isEMP(prefix)) {
		char wl_cardtype[32];
		sprintf(wl_cardtype, "%s_cardtype", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.cardtype", NULL);
		websWrite(wp, "<select name=\"%s\">\n", wl_cardtype);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Atheros Generic</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 0, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >Alfa Networks AWPCI085H</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 5, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"6\\\" %s >Alfa Networks AWPCI085P</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 6, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >Doodle Labs DLM105</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 7, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >MakSat MAK27</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 4, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Senao EMP-8602</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 1, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Senao EMP-8603-S</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 2, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >Senao EMP-8603</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 3, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
	}
#endif
#endif // ! HAVE MAKSAT

	if (is_ath10k(prefix) && has_fwswitch(prefix) && nvram_nmatch("1", "%s_regulatory", prefix)) {
		char fw_type[32];
		sprintf(fw_type, "%s_fwtype", prefix);
		char *fw_names[] = { "DD-WRT", "VANILLA" };
		showOptionsNames(wp, "wl_basic.fw_type", fw_type, "ddwrt vanilla", fw_names, nvram_default_get(fw_type, "ddwrt"));
	}

#ifndef HAVE_NOCOUNTRYSEL
	if (!nvram_matchi("nocountrysel", 1)) {
		char wl_regdomain[32];
#ifdef HAVE_ATH9K
		if (!strcmp(prefix, "wlan0"))
#endif
		{
			sprintf(wl_regdomain, "%s_regdomain", prefix);

			if (is_mac80211(prefix) || nvram_nmatch("1", "%s_regulatory", prefix) || !issuperchannel()) {
				char *list = getCountryList(COUNTRYLIST);
				showOptionsLabel(wp, "wl_basic.regdom", wl_regdomain, list, nvram_safe_get(wl_regdomain));
			}
		}
	}
#endif // ! HAVE MAKSAT
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
	if (issuperchannel()) // show
	// client
	// only on
	// first
	// interface
	{
		char regulatory[32];
		sprintf(regulatory, "%s_regulatory", prefix);
		nvram_default_get(regulatory, "0");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.regulatory", NULL);
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_regulatory\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			prefix, nvram_matchi(regulatory, 0) ? "checked" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_regulatory\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			prefix, nvram_matchi(regulatory, 1) ? "checked" : "");
		websWrite(wp, "</div>\n");
	}
	int txpower = nvram_geti(power);
#ifdef HAVE_ESPOD
#ifdef HAVE_SUB3
	if (txpower > 28) {
		txpower = 28;
		nvram_seti(power, 28);
	}
#else
	if (txpower > 30) {
		txpower = 28;
		nvram_seti(power, 30);
	}
#endif
#endif
#if !defined(HAVE_WZR450HP2) || !defined(HAVE_BUFFALO) || !defined(HAVE_IDEXX)

#ifdef HAVE_ATH9K
	if (is_ath10k(prefix) && nvram_nmatch("ddwrt", "%s_fwtype", prefix)) {
		char wl_po[32];
		sprintf(wl_po, "%s_power_override", prefix);
		nvram_default_get(wl_po, "0");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.power_override", NULL);
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_power_override\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			prefix, nvram_matchi(wl_po, 1) ? "checked" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_power_override\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			prefix, nvram_matchi(wl_po, 0) ? "checked" : "");
		websWrite(wp, "</div>\n");
	}
#endif
	websWrite(wp, "<div class=\"setting\">\n");
#ifdef HAVE_ATH9K

	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.TXpower)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%d\" /> dBm (Max %d)\n",
		power, txpower + wifi_gettxpoweroffset(prefix), mac80211_get_maxpower(prefix) + wifi_gettxpoweroffset(prefix));
#else
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.TXpower)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%d\" /> dBm\n",
		power, txpower + wifi_gettxpoweroffset(prefix));

#endif
	websWrite(wp, "</div>\n");
	sprintf(power, "%s_antgain", prefix);
#ifndef HAVE_MAKSAT
	if (nvram_nmatch("1", "%s_regulatory", prefix))
#endif
	{
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.AntGain)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%s\" /> dBi\n",
			power, nvram_default_get(power, "0"));
		websWrite(wp, "</div>\n");
	}
#endif
#endif
	if (has_ax(prefix)) {
		char *netmode = nvram_nget("%s_net_mode", prefix);
		if (!strcmp(netmode, "mixed") || !strcmp(netmode, "xacn-mixed") || !strcmp(netmode, "ax-only") ||
		    !strcmp(netmode, "axg-only")) {
			char color[32];
			sprintf(color, "%s_he_bss_color", prefix);
			showInputNum(wp, "wl_basic.he_bss_color", color, 2, 2, 0);
		}
	}

#ifdef HAVE_MADWIFI
	// if (!strcmp (prefix, "wlan0"))
#endif
	{
		// #ifdef HAVE_MADWIFI
		// if (!strcmp (prefix, "wlan0")) //show client only on first
		// interface
		// #endif
		{
#ifdef HAVE_MADWIFI
			// if (!strcmp (prefix, "wlan0")) //show client only on first
			// interface
			// if (nvram_match ("wlan0_mode", "wdsap")
			// || nvram_match ("wlan0_mode", "wdssta"))
			// showOption (wp, "wl_basic.wifi_bonding", "wifi_bonding");
#endif
#ifdef HAVE_REGISTER
			int cpeonly = iscpe();
#else
			int cpeonly = 0;
#endif
			websWrite(
				wp,
				"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n",
				wl_mode);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			if (!has_no_apmode(prefix)) {
				if (!cpeonly) {
					websWrite(
						wp,
						"document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");\n",
						nvram_match(wl_mode, "ap") ? "selected=\\\"selected\\\"" : "");
				}
			}
#ifdef HAVE_MADWIFI
			if (has_wdsap(prefix)) {
				if (!cpeonly) {
					websWrite(
						wp,
						"document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");\n",
						nvram_match(wl_mode, "wdsap") ? "selected=\\\"selected\\\"" : "");
				}
			}
#endif
#if (!defined(HAVE_RT61) && !defined(HAVE_DIR860)) || defined(HAVE_MT76)
			websWrite(wp, "document.write(\"<option value=\\\"sta\\\" %s >\" + wl_basic.client + \"</option>\");\n",
				  nvram_match(wl_mode, "sta") ? "selected=\\\"selected\\\"" : "");
#endif
#if !defined(HAVE_RT2880) || defined(HAVE_MT76)
#ifdef HAVE_RELAYD
			websWrite(wp,
				  "document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientRelayd + \"</option>\");\n",
#else
			websWrite(wp,
				  "document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientBridge + \"</option>\");\n",
#endif
				  nvram_match(wl_mode, "wet") ? "selected=\\\"selected\\\"" : "");
#endif
#ifndef HAVE_MADWIFI
			if (!cpeonly) {
				websWrite(
					wp,
					"document.write(\"<option value=\\\"apsta\\\" %s >\" + wl_basic.repeater + \"</option>\");\n",
					nvram_match(wl_mode, "apsta") ? "selected=\\\"selected\\\"" : "");
				//#ifndef HAVE_RT2880
				websWrite(
					wp,
					"document.write(\"<option value=\\\"apstawet\\\" %s >\" + wl_basic.repeaterbridge + \"</option>\");\n",
					nvram_match(wl_mode, "apstawet") ? "selected=\\\"selected\\\"" : "");
			}
//#endif
#else
			if (has_wdsap(prefix)) {
				websWrite(
					wp,
					"document.write(\"<option value=\\\"wdssta\\\" %s >\" + wl_basic.wdssta + \"</option>\");\n",
					nvram_match(wl_mode, "wdssta") ? "selected=\\\"selected\\\"" : "");
				websWrite(
					wp,
					"document.write(\"<option value=\\\"wdssta_mtik\\\" %s >\" + wl_basic.wdssta_mtik + \"</option>\");\n",
					nvram_match(wl_mode, "wdssta_mtik") ? "selected=\\\"selected\\\"" : "");
			}
#endif
			if (!cpeonly && has_ibss(prefix))
				websWrite(
					wp,
					"document.write(\"<option value=\\\"infra\\\" %s >\" + wl_basic.adhoc + \"</option>\");\n",
					nvram_match(wl_mode, "infra") ? "selected=\\\"selected\\\"" : "");
			if (has_mesh(prefix)) {
				websWrite(wp,
					  "document.write(\"<option value=\\\"mesh\\\" %s >\" + wl_basic.mesh + \"</option>\");\n",
					  nvram_match(wl_mode, "mesh") ? "selected=\\\"selected\\\"" : "");
			}
			websWrite(wp, "//]]>\n</script>\n");
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
		}
	}
	// wireless net mode
	show_netmode(wp, prefix);
	// turbo options
#ifdef HAVE_MADWIFI
	// char wl_xchanmode[16];
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
	sprintf(wl_shortgi, "%s_shortgi", prefix);
	sprintf(wl_subf, "%s_subf", prefix);
	sprintf(wl_mubf, "%s_mubf", prefix);
	sprintf(wl_xr, "%s_xr", prefix);
	sprintf(wl_intmit, "%s_intmit", prefix);
	sprintf(wl_qboost, "%s_qboost", prefix);
	sprintf(wl_autoburst, "%s_autoburst", prefix);
	sprintf(wl_sifs_trigger_time, "%s_sifs_trigger_time", prefix);
	sprintf(wl_noise_immunity, "%s_noise_immunity", prefix);
	sprintf(wl_ofdm_weak_det, "%s_ofdm_weak_det", prefix);
	sprintf(wl_uapsd, "%s_uapsd", prefix);
	sprintf(wl_lowack, "%s_d_lowack", prefix);
	sprintf(wl_ldpc, "%s_ldpc", prefix);
	if (has_ldpc(prefix)) {
		char *netmode = nvram_nget("%s_net_mode", prefix);
		if ((strcmp(netmode, "mixed") && //
		     strcmp(netmode, "ac-only") && strcmp(netmode, "acn-mixed") && strcmp(netmode, "ax-only") &&
		     strcmp(netmode, "axg-only") && strcmp(netmode, "xacn-mixed")))
			showRadioInv(wp, "wl_basic.ldpc", wl_ldpc);
	}
	if (has_uapsd(prefix)) {
		showRadio(wp, "wl_basic.uapsd", wl_uapsd);
	}
	if (is_ap(prefix))
#ifdef HAVE_MVEBU
		showRadioDefaultOff(wp, "wl_basic.disassoc_low_ack", wl_lowack);
#else
		showRadioDefaultOn(wp, "wl_basic.disassoc_low_ack", wl_lowack);
#endif
	if (has_smps(prefix)) {
		sprintf(wl_smps, "%s_smps", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.smps", NULL);
		websWrite(wp, "<select name=\"%s\">\n", wl_smps);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + share.off + \"</option>\");\n", "0",
			  nvram_default_match(wl_smps, "0", "0") ? "selected=\\\"selected\\\"" : "");
		if (has_static_smps(prefix))
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + share.sttic + \"</option>\");\n", "1",
				  nvram_match(wl_smps, "1") ? "selected=\\\"selected\\\"" : "");
		if (has_dynamic_smps(prefix))
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + share.dynamic + \"</option>\");\n", "2",
				  nvram_match(wl_smps, "2") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n");
		websWrite(wp, "</div>\n");
	}
	if (!is_mvebu(prefix)) {
		if (is_mac80211(prefix)) {
			showRadio(wp, "wl_basic.intmit", wl_intmit);
		} else {
			showAutoOption(wp, "wl_basic.intmit", wl_intmit, 0);
		}
		if (nvram_match("experimental", "1")) {
			if (has_wave2(prefix)) {
				showRadio(wp, "wl_basic.dynamic_auto_bursting", wl_autoburst);
			}
		}
		if (has_qboost(prefix)) {
			if (has_qboost_tdma(prefix)) {
				showRadio(wp, "wl_basic.qboost_tdma", wl_qboost);
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_basic.sifs_trigger_time", NULL);
				websWrite(
					wp,
					"<input class=\"num\" name=\"%s\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,20,wl_basic.sifs_trigger_time)\" value=\"%s\" />\n",
					wl_sifs_trigger_time, nvram_default_get(wl_sifs_trigger_time, "0"));
				websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(share.ms)</script></div>\n");
			} else
				showRadio(wp, "wl_basic.qboost", wl_qboost);
		}
		if (!is_mac80211(prefix)) {
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.noise_immunity", NULL);
			websWrite(wp, "<select name=\"%s\">\n", wl_noise_immunity);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >0</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 0, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 1, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >2</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 2, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >3</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 3, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >4</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 4, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
			showRadio(wp, "wl_basic.ofdm_weak_det", wl_ofdm_weak_det);
		}
	}

	showrtssettings(wp, prefix);
	showairtimepolicy(wp, prefix, prefix);
	if (!is_mac80211(prefix)) {
		show_rates(wp, prefix, 0);
		show_rates(wp, prefix, 1);
	}

	showRadioDefaultOn(wp, "wl_basic.preamble", wl_preamble);
	if (!is_mac80211(prefix)) {
		showRadio(wp, "wl_basic.extrange", wl_xr);
		showRadio(wp, "wl_basic.supergff", wl_ff);
	}
#if 0
	showRadio(wp, "wl_basic.csma", wl_csma);
#endif
	// showOption (wp, "wl_basic.extchannel", wl_xchanmode);
	if (has_shortgi(prefix)) {
		nvram_default_get(wl_shortgi, "1");
		showRadio(wp, "wl_basic.shortgi", wl_shortgi);
	}
	if ((has_5ghz(prefix) && has_ac(prefix)) || has_ax(prefix)) {
		if (has_subeamforming(prefix)) {
			nvram_default_get(wl_subf, "0");
			showRadio(wp, "wl_basic.subf", wl_subf);
		}
		if (has_mubeamforming(prefix)) {
			nvram_default_get(wl_mubf, "0");
			showRadio(wp, "wl_basic.mubf", wl_mubf);
		}
	}
#ifndef HAVE_BUFFALO
#if !defined(HAVE_FONERA) && !defined(HAVE_LS2) && !defined(HAVE_MERAKI)
	if (!is_mac80211(prefix)) {
		if (has_5ghz(prefix)) {
			if (nvram_nmatch("1", "%s_regulatory", prefix) || !issuperchannel()) {
				showRadio(wp, "wl_basic.outband", wl_outdoor);
			}
		}
	}
#endif
#endif
	if (is_mac80211(prefix) && has_dualband(prefix)) {
		char dualband[32];
		sprintf(dualband, "%s_dualband", prefix);
		nvram_default_get(dualband, "0");
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.dualband)</script></div><select name=\"%s\">\n",
			dualband);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + share.deflt + \"</option>\");\n",
			  nvram_matchi(dualband, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.bghz24 + \"</option>\");\n",
			  nvram_matchi(dualband, 2) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >\" + wl_basic.bghz5 + \"</option>\");\n",
			  nvram_matchi(dualband, 5) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}

	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div><select name=\"%s\" onchange=\"to_submit(this.form);\">\n",
		wl_width);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"20\\\" %s >\" + share.full + \"</option>\");\n",
		  nvram_matchi(wl_width, 20) ? "selected=\\\"selected\\\"" : "");
	int canht40 = can_ht40(prefix);
	int canvht80 = can_vht80(prefix);
	if (!is_mac80211(prefix) || canht40)
		if ((nvram_nmatch("n-only", "%s_net_mode", prefix) || nvram_nmatch("ng-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("n2-only", "%s_net_mode", prefix) || nvram_nmatch("mixed", "%s_net_mode", prefix) ||
		     nvram_nmatch("n5-only", "%s_net_mode", prefix) || nvram_nmatch("ac-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("acn-mixed", "%s_net_mode", prefix) || nvram_nmatch("ax-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("axg-only", "%s_net_mode", prefix) || nvram_nmatch("xacn-mixed", "%s_net_mode", prefix) ||
		     nvram_nmatch("na-only", "%s_net_mode", prefix))) {
			websWrite(wp, "document.write(\"<option value=\\\"2040\\\" %s >\" + share.dynamicturbo + \"</option>\");\n",
				  nvram_matchi(wl_width, 2040) ? "selected=\\\"selected\\\"" : "");
			fprintf(stderr, "[CHANNEL WIDTH] 20/40 (1)\n");
		}
	if (is_mac80211(prefix) &&
	    (nvram_nmatch("n-only", "%s_net_mode", prefix) || nvram_nmatch("ng-only", "%s_net_mode", prefix) ||
	     nvram_nmatch("n2-only", "%s_net_mode", prefix) || nvram_nmatch("mixed", "%s_net_mode", prefix) ||
	     nvram_nmatch("n5-only", "%s_net_mode", prefix) || nvram_nmatch("ac-only", "%s_net_mode", prefix) ||
	     nvram_nmatch("acn-mixed", "%s_net_mode", prefix) || nvram_nmatch("ax-only", "%s_net_mode", prefix) ||
	     nvram_nmatch("axg-only", "%s_net_mode", prefix) || nvram_nmatch("xacn-mixed", "%s_net_mode", prefix) ||
	     nvram_nmatch("na-only", "%s_net_mode", prefix))) {
		if (!is_mac80211(prefix) || is_ath5k(prefix))
			websWrite(wp, "document.write(\"<option value=\\\"40\\\" %s >\" + share.turbo + \"</option>\");\n",
				  nvram_matchi(wl_width, 40) ? "selected=\\\"selected\\\"" : "");
		else if (canht40)
			websWrite(wp, "document.write(\"<option value=\\\"40\\\" %s >\" + share.ht40 + \"</option>\");\n",
				  nvram_matchi(wl_width, 40) ? "selected=\\\"selected\\\"" : "");
		if ((is_ath11k(prefix) ||is_ath10k(prefix) || is_mvebu(prefix) || has_vht80(prefix)) && ((has_5ghz(prefix) || (cansuperchannel(wp, prefix) && nvram_nmatch("1", "%s_turbo_qam",prefix))) && nvram_nmatch("mixed", "%s_net_mode", prefix)
		    || nvram_nmatch("ac-only", "%s_net_mode", prefix) || nvram_nmatch("ax-only", "%s_net_mode", prefix)
		    || nvram_nmatch("acn-mixed", "%s_net_mode", prefix) || nvram_nmatch("xacn-mixed", "%s_net_mode", prefix) || (cansuperchannel(wp, prefix) && nvram_nmatch("1", "%s_turbo_qam",prefix))) {
			if (canvht80)
				websWrite(wp, "document.write(\"<option value=\\\"80\\\" %s >\" + share.vht80 + \"</option>\");\n",
					  nvram_matchi(wl_width, 80) ? "selected=\\\"selected\\\"" : "");
			if ((has_vht160(prefix) || has_he160(prefix)) && can_vht160(prefix) && has_5ghz(prefix))
				websWrite(wp,
					  "document.write(\"<option value=\\\"160\\\" %s >\" + share.vht160 + \"</option>\");\n",
					  nvram_matchi(wl_width, 160) ? "selected=\\\"selected\\\"" : "");
			if (has_vht80plus80(prefix) && canvht80 && has_5ghz(prefix))
				websWrite(
					wp,
					"document.write(\"<option value=\\\"80+80\\\" %s >\" + share.vht80plus80 + \"</option>\");\n",
					nvram_match(wl_width, "80+80") ? "selected=\\\"selected\\\"" : "");
		}
	}
#if !defined(HAVE_BUFFALO)
#if defined(HAVE_MADWIFI)
	if (has_half(prefix))
		websWrite(wp, "document.write(\"<option value=\\\"10\\\" %s >\" + share.half + \"</option>\");\n",
			  nvram_matchi(wl_width, 10) ? "selected=\\\"selected\\\"" : "");
	if (has_quarter(prefix))
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >\" + share.quarter + \"</option>\");\n",
			  nvram_matchi(wl_width, 5) ? "selected=\\\"selected\\\"" : "");
	if (registered_has_subquarter() && has_subquarter(prefix)) {
		/* will be enabled once it is tested and the spectrum analyse is done */
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + share.subquarter + \"</option>\");\n",
			  nvram_matchi(wl_width, 2) ? "selected=\\\"selected\\\"" : "");
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
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label24)</script></div><select name=\"%s\" >\n",
		wl_txantenna);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.vertical + \"</option>\");\n",
		  nvram_matchi(wl_txantenna, 0) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.horizontal + \"</option>\");\n",
		  nvram_matchi(wl_txantenna, 1) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >\" + wl_basic.adaptive + \"</option>\");\n",
		  nvram_matchi(wl_txantenna, 3) ? "selected=\\\"selected\\\"" : "");
#if defined(HAVE_NS5) || defined(HAVE_NS2) || defined(HAVE_NS3)
	websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.external + \"</option>\");\n",
		  nvram_matchi(wl_txantenna, 2) ? "selected=\\\"selected\\\"" : "");
#endif
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
#else
	if (!is_mac80211(prefix)) {
		showRadio(wp, "wl_basic.diversity", wl_diversity);
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label12)</script></div><select name=\"%s\" >\n",
			wl_txantenna);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");\n",
			  nvram_matchi(wl_txantenna, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");\n",
			  nvram_matchi(wl_txantenna, 1) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");\n",
			  nvram_matchi(wl_txantenna, 2) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label13)</script></div><select name=\"%s\" >\n",
			wl_rxantenna);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");\n",
			  nvram_matchi(wl_rxantenna, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");\n",
			  nvram_matchi(wl_rxantenna, 1) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");\n",
			  nvram_matchi(wl_rxantenna, 2) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}
#if defined(HAVE_ATH9K)
	else {
		int maxrx = 7;
		int maxtx = 7;
#ifdef HAVE_ATH9K
		maxrx = mac80211_get_avail_rx_antenna(prefix);
		maxtx = mac80211_get_avail_tx_antenna(prefix);
#endif
		if (maxtx > 1) {
			websWrite(
				wp,
				"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.txchainmask)</script></div><select name=\"%s\" >\n",
				wl_txantenna);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n",
				  nvram_matchi(wl_txantenna, 1) ? "selected=\\\"selected\\\"" : "");
			if (maxtx > 1)
				websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >1+2</option>\");\n",
					  nvram_matchi(wl_txantenna, 3) ? "selected=\\\"selected\\\"" : "");
			if (maxtx > 3)
				websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >1+3</option>\");\n",
					  nvram_matchi(wl_txantenna, 5) ? "selected=\\\"selected\\\"" : "");
			if (maxtx > 5)
				websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >1+2+3</option>\");\n",
					  nvram_matchi(wl_txantenna, 7) ? "selected=\\\"selected\\\"" : "");
			if (maxtx > 7)
				websWrite(wp, "document.write(\"<option value=\\\"15\\\" %s >1+2+3+4</option>\");\n",
					  nvram_matchi(wl_txantenna, 15) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script>\n");
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
		}

		if (maxrx > 0) {
			websWrite(
				wp,
				"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.rxchainmask)</script></div><select name=\"%s\" >\n",
				wl_rxantenna);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n",
				  nvram_matchi(wl_rxantenna, 1) ? "selected=\\\"selected\\\"" : "");
			if (maxrx > 1)
				websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >1+2</option>\");\n",
					  nvram_matchi(wl_rxantenna, 3) ? "selected=\\\"selected\\\"" : "");
			if (maxrx > 3)
				websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >1+3</option>\");\n",
					  nvram_matchi(wl_rxantenna, 5) ? "selected=\\\"selected\\\"" : "");
			if (maxrx > 5)
				websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >1+2+3</option>\");\n",
					  nvram_matchi(wl_rxantenna, 7) ? "selected=\\\"selected\\\"" : "");
			if (maxrx > 7)
				websWrite(wp, "document.write(\"<option value=\\\"15\\\" %s >1+2+3+4</option>\");\n",
					  nvram_matchi(wl_rxantenna, 15) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script>\n");
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
		}
	}
#endif
#endif
#endif
#ifdef HAVE_MADWIFI
	sprintf(wl_isolate, "%s_ap_isolate", prefix);
	showRadio(wp, "wl_adv.label11", wl_isolate);
	char bcn[32];
	sprintf(bcn, "%s_bcn", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_adv.label6", NULL);
	websWrite(
		wp,
		"<input class=\"num\" name=\"%s\" size=\"5\" maxlength=\"5\" onblur=\"valid_range(this,15,65535,wl_adv.label6)\" value=\"%s\" />\n",
		bcn, nvram_default_get(bcn, "100"));
	websWrite(wp, "</div>\n");
	char dtim[32];
	sprintf(dtim, "%s_dtim", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_adv.label7", NULL);
	websWrite(
		wp,
		"<input class=\"num\" name=\"%s\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,1,255,wl_adv.label7)\" value=\"%s\" />\n",
		dtim, nvram_default_get(dtim, "2"));
	websWrite(wp, "</div>\n");
	if (has_airtime_fairness(prefix)) {
		char wl_atf[16];
		sprintf(wl_atf, "%s_atf", prefix);
		showRadioDefaultOn(wp, "wl_basic.atf", wl_atf);
	}
#ifdef HAVE_MAC80211_COMPRESS
	if (is_mac80211(prefix) && !is_mvebu(prefix)) {
		char wl_fc[16];
		sprintf(wl_fc, "%s_fc", prefix);
		FILE *fp = fopen("/proc/modules", "rb");
		char line[245];
		int zstd = 0;
		if (fp) {
			while (!feof(fp) && fgets(line, sizeof(line), fp)) {
				if (strstr(line, "zstd")) {
					zstd = 1;
					break;
				}
			}
			fclose(fp);
		}
		char *names_zstd[] = { "\" + share.disabled + \"", "LZO", "LZ4", "LZMA", "ZSTD" };
		char *names[] = {
			"\" + share.disabled + \"",
			"LZO",
			"LZ4",
			"LZMA",
		};
		if (zstd)
			showOptionsNames(wp, "wl_basic.fc", wl_fc, "0 1 3 2 4", names_zstd, nvram_default_get(wl_fc, "0"));
		else
			showOptionsNames(wp, "wl_basic.fc", wl_fc, "0 1 3 2", names, nvram_default_get(wl_fc, "0"));
	}
#endif
	sprintf(wmm, "%s_wmm", prefix);
	if (is_mac80211(prefix))
		showRadioDefaultOn(wp, "wl_adv.label18", wmm);
	else
		showRadio(wp, "wl_adv.label18", wmm);
#endif
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_basic.ssid", NULL);
	websWrite(wp, "<input name=\"%s\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.ssid)\" value=\"",
		  wl_ssid);
	tf_webWriteESCNV(wp, wl_ssid);
	websWrite(wp, "\" /></div>\n");

#ifdef HAVE_MADWIFI
	if (is_station(prefix)) {
		sprintf(wl_bssid, "%s_bssid", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.bssid", NULL);
		websWrite(wp,
			  "<input size=\"20\" maxlength=\"17\" name=\"%s_bssid\" onblur=\"valid_macs_all(this)\" value=\"%s\" />",
			  prefix, nvram_safe_get(wl_bssid));
		websWrite(wp, "</div>\n");
	}
#endif

#ifdef HAVE_MADWIFI
#ifndef HAVE_BUFFALO
	if (has_5ghz(prefix)) {
		showRadio(wp, "wl_basic.radar", wl_doth);
	}

	show_chanshift(wp, prefix);
#endif
#endif
#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
#else
	if (is_ap(prefix) || nvram_match(wl_mode, "infra") || nvram_match(wl_mode, "mesh"))
#endif
	{
		if (has_mimo(prefix) &&
		    (nvram_nmatch("n-only", "%s_net_mode", prefix) || nvram_nmatch("ng-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("mixed", "%s_net_mode", prefix) || nvram_nmatch("n2-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("n5-only", "%s_net_mode", prefix) || nvram_nmatch("acn-mixed", "%s_net_mode", prefix) ||
		     nvram_nmatch("ac-only", "%s_net_mode", prefix) || nvram_nmatch("xacn-mixed", "%s_net_mode", prefix) ||
		     nvram_nmatch("ax-only", "%s_net_mode", prefix) || nvram_nmatch("na-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("axg-only", "%s_net_mode", prefix))) {
			show_channel(wp, prefix, prefix, 1);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.channel_width", NULL);
			websWrite(wp, "<select name=\"%s_nbw\">\n", prefix);
			//              websWrite(wp,
			//                        "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\" %s >\" + share.auto + \"</option>\");\n//]]>\n</script>\n",
			//                        nvram_nmatch("0", "%s_nbw", prefix) ? "selected=\\\"selected\\\"" : "");
			websWrite(
				wp,
				"<option value=\"20\" %s>20 <script type=\"text/javascript\">Capture(wl_basic.mhz);</script></option>\n",
				nvram_nmatch("20", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
			websWrite(
				wp,
				"<option value=\"40\" %s><script type=\"text/javascript\">Capture(share.ht40);</script></option>\n",
				nvram_nmatch("40", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
			if (has_ac(prefix) && has_5ghz(prefix) &&
			    (nvram_nmatch("mixed", "%s_net_mode", prefix) || nvram_nmatch("ac-only", "%s_net_mode", prefix) ||
			     nvram_nmatch("acn-mixed", "%s_net_mode", prefix) || nvram_nmatch("ax-only", "%s_net_mode", prefix) ||
			     nvram_nmatch("xacn-mixed", "%s_net_mode", prefix))) {
				websWrite(
					wp,
					"<option value=\"80\" %s><script type=\"text/javascript\">Capture(share.vht80);</script></option>\n",
					nvram_nmatch("80", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
				if (has_vht160(prefix) || has_he160(prefix)) {
					websWrite(
						wp,
						"<option value=\"160\" %s><script type=\"text/javascript\">Capture(share.vht160);</script></option>\n",
						nvram_nmatch("160", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
				}
				if (has_vht80plus80(prefix)) {
					websWrite(
						wp,
						"<option value=\"80+80\" %s><script type=\"text/javascript\">Capture(share.vht80plus80);</script></option>\n",
						nvram_nmatch("80+80", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
				}
			}
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
			if (!is_ath5k(prefix) && nvram_nmatch("40", "%s_nbw", prefix)) {
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_basic.channel_wide", NULL);
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
				websWrite(
					wp,
					"<option value=\"upper\" %s><script type=\"text/javascript\">document.write(wl_basic.lower);</script></option>\n",
					nvram_nmatch("upper", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(
					wp,
					"<option value=\"lower\" %s><script type=\"text/javascript\">document.write(wl_basic.upper);</script></option>\n",
					nvram_nmatch("lower", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "</select>\n");
				websWrite(wp, "</div>\n");
			}
			if (nvram_nmatch("80", "%s_nbw", prefix) || nvram_nmatch("80+80", "%s_nbw",
										 prefix)) { // 802.11ac
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_basic.channel_wide", NULL);
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
				websWrite(
					wp,
					"<option value=\"ll\" %s><script type=\"text/javascript\">document.write(wl_basic.lower+\" \"+wl_basic.lower)</script></option>\n",
					nvram_nmatch("ll", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(
					wp,
					"<option value=\"lu\" %s><script type=\"text/javascript\">document.write(wl_basic.lower+\" \"+wl_basic.upper)</script></option>\n",
					nvram_nmatch("lu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(
					wp,
					"<option value=\"ul\" %s><script type=\"text/javascript\">document.write(wl_basic.upper+\" \"+wl_basic.lower)</script></option>\n",
					nvram_nmatch("ul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(
					wp,
					"<option value=\"uu\" %s><script type=\"text/javascript\">document.write(wl_basic.upper+\" \"+wl_basic.upper)</script></option>\n",
					nvram_nmatch("uu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "</select>\n");
				websWrite(wp, "</div>\n");
			}
#if 0
			if (nvram_nmatch("160", "%s_nbw", prefix)) {
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_basic.channel_wide", NULL);
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
				websWrite(wp, "<option value=\"lll\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lll);</script></option>\n",
					  (nvram_nmatch("lll", "%s_nctrlsb", prefix) || nvram_nmatch("lower", "%s_nctrlsb", prefix)) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"llu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_llu);</script></option>\n",
					  nvram_nmatch("llu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"lul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lul);</script></option>\n",
					  nvram_nmatch("lul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"luu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_luu);</script></option>\n",
					  nvram_nmatch("luu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"ull\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ull);</script></option>\n",
					  nvram_nmatch("ull", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"ulu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ulu);</script></option>\n",
					  nvram_nmatch("ulu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"uul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uul);</script></option>\n",
					  nvram_nmatch("uul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"uuu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uuu);</script></option>\n",
					  (nvram_nmatch("uuu", "%s_nctrlsb", prefix) || nvram_nmatch("upper", "%s_nctrlsb", prefix)) ? "selected=\"selected\"" : "");
				websWrite(wp, "</select>\n");
				websWrite(wp, "</div>\n");
			}
#endif
		} else {
			show_channel(wp, prefix, prefix, 0);
			if (is_mac80211(prefix)) {
				if (!is_ath5k(prefix) && (nvram_matchi(wl_width, 40) || nvram_matchi(wl_width, 2040))) {
					websWrite(wp, "<div class=\"setting\">\n");
					show_caption(wp, "label", "wl_basic.channel_wide", NULL);
					websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
					websWrite(
						wp,
						"<option value=\"ull\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_upr);</script></option>\n",
						(nvram_nmatch("ull", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("upper", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(
						wp,
						"<option value=\"luu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lwr);</script></option>\n",
						(nvram_nmatch("luu", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("lower", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(wp, "</select>\n");
					websWrite(wp, "</div>\n");
				}
				if (nvram_matchi(wl_width, 80) || nvram_match(wl_width, "80+80")) {
					websWrite(wp, "<div class=\"setting\">\n");
					show_caption(wp, "label", "wl_basic.channel_wide", NULL);
					websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
					websWrite(
						wp,
						"<option value=\"lul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ll);</script></option>\n",
						(nvram_nmatch("lul", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("lower", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(
						wp,
						"<option value=\"luu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lu);</script></option>\n",
						nvram_nmatch("luu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"ull\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ul);</script></option>\n",
						nvram_nmatch("ull", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"ulu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uu);</script></option>\n",
						(nvram_nmatch("ulu", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("upper", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(wp, "</select>\n");
					websWrite(wp, "</div>\n");
				}
				if (nvram_matchi(wl_width, 160)) {
					websWrite(wp, "<div class=\"setting\">\n");
					show_caption(wp, "label", "wl_basic.channel_wide", NULL);
					websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
					websWrite(
						wp,
						"<option value=\"lll\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lll);</script></option>\n",
						(nvram_nmatch("lll", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("lower", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(
						wp,
						"<option value=\"llu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_llu);</script></option>\n",
						nvram_nmatch("llu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"lul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lul);</script></option>\n",
						nvram_nmatch("lul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"luu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_luu);</script></option>\n",
						nvram_nmatch("luu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"ull\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ull);</script></option>\n",
						nvram_nmatch("ull", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"ulu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ulu);</script></option>\n",
						nvram_nmatch("ulu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"uul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uul);</script></option>\n",
						nvram_nmatch("uul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"uuu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uuu);</script></option>\n",
						(nvram_nmatch("uuu", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("upper", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(wp, "</select>\n");
					websWrite(wp, "</div>\n");
				}
			}
		}

		char wl_closed[16];
		sprintf(wl_closed, "%s_closed", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.label5", NULL);
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			wl_closed, nvram_matchi(wl_closed, 0) ? "checked=\"checked\"" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			wl_closed, nvram_matchi(wl_closed, 1) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
	}
	if (is_mac80211(prefix)) {
		char wl_overlap[16];
		sprintf(wl_overlap, "%s_overlap", prefix);
		showRadio(wp, "wl_basic.overlap", wl_overlap);
	}
	if (has_qam256(prefix) && has_2ghz(prefix)) {
		char wl_turboqam[32];
		sprintf(wl_turboqam, "%s_turbo_qam", prefix);
		showRadio(wp, "wl_basic.turboqam", wl_turboqam);
	}
#ifdef HAVE_BCMMODERN
	if (nvram_match(wl_mode, "ap") || nvram_match(wl_mode, "sta")) {
		char wl_dwds[32];
		sprintf(wl_dwds, "%s_dwds", prefix);
		nvram_default_get(wl_dwds, "0");
		showRadio(wp, "wl_basic.dwds", wl_dwds);
	}
#endif
#ifdef HAVE_80211AC
#ifndef HAVE_NOAC
#if 0
	if (!has_qtn(prefix)) {
		char wl_igmp[16];
		sprintf(wl_igmp, "%s_wmf_bss_enable", prefix);
		nvram_default_get(wl_igmp, "0");
		showRadio(wp, "wl_basic.igmpsnooping", wl_igmp);
	}
#endif
	if (has_ac(prefix) && has_2ghz(prefix)) {
		char wl_turboqam[32];
		sprintf(wl_turboqam, "%s_turbo_qam", prefix);
		showRadio(wp, "wl_basic.turboqam", wl_turboqam);
	}
	if (has_ac(prefix) && nvram_nmatch("15", "%s_hw_rxchain", prefix)) {
		char wl_nitroqam[16];
		sprintf(wl_nitroqam, "%s_nitro_qam", prefix);
		showRadio(wp, "wl_basic.nitroqam", wl_nitroqam);
	}

	char wl_bft[16];
	sprintf(wl_bft, "%s_txbf", prefix);
	if (has_beamforming(prefix)) {
		showRadio(wp, "wl_basic.bft", wl_bft);
		char wl_bfr[16];
		sprintf(wl_bfr, "%s_txbf_imp", prefix);
		showRadio(wp, "wl_basic.bfr", wl_bfr);
	}
	if (has_mumimo(prefix) && nvram_match(wl_bft, "1")) {
		char wl_mu[16];
		sprintf(wl_mu, "%s_mumimo", prefix);
		nvram_default_get(wl_mu, "0");
		showRadio(wp, "wl_basic.mubf", wl_mu);
	}

	if (has_beamforming(prefix)) {
		char wl_atf[16];
		sprintf(wl_atf, "%s_atf", prefix);
		showRadio(wp, "wl_basic.atf", wl_atf);
	}
#endif
#endif

#ifdef HAVE_MADWIFI
	// if (nvram_match (wl_mode, "sta") || nvram_match (wl_mode, "wdssta")
	// || nvram_match (wl_mode, "wet"))
	{
		char wl_scanlist[32];
		sprintf(wl_scanlist, "%s_scanlist", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.scanlist", NULL);
		websWrite(wp, "<input name=\"%s\" size=\"32\" maxlength=\"512\" value=\"%s\" />\n", wl_scanlist,
			  nvram_default_get(wl_scanlist, "default"));
		websWrite(wp, "</div>\n");
	}
#endif

	if (has_acktiming(prefix)) {
		sprintf(power, "%s_distance", prefix);
		//websWrite(wp, "<br />\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.label6", NULL);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"8\" maxlength=\"8\" onblur=\"valid_range(this,0,99999999,wl_basic.label6)\" value=\"%s\" />\n",
			power, nvram_default_get(power, "500"));
		websWrite(
			wp,
			"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 500 \" + share.meters + \")\");\n//]]>\n</script></span>\n");
		websWrite(wp, "</div>\n");
	}
#ifdef HAVE_MADWIFI
	if (is_ap(prefix) || nvram_nmatch("infra", "%s_mode", prefix) || nvram_nmatch("mesh", "%s_mode", prefix)) {
		sprintf(power, "%s_maxassoc", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_adv.label10", NULL);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,256,wl_adv.label10)\" value=\"%s\" />\n",
			power, nvram_default_get(power, "256"));
		websWrite(
			wp,
			"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 256 \" + status_wireless.legend3 + \")\");\n//]]>\n</script></span>\n");
		websWrite(wp, "</div>\n");
	}
	if (is_mac80211(prefix)) {
		if (is_ap(prefix)) {
			char signal[32];
			websWrite(
				wp,
				"<fieldset><legend><script type=\"text/javascript\">Capture(wl_adv.droplowsignal)</script></legend>");
			sprintf(signal, "%s_connect", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_adv.connect", NULL);
			websWrite(
				wp,
				"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,-128,0,wl_adv.connect)\" value=\"%s\" />\n",
				signal, nvram_default_get(signal, "-128"));
			websWrite(wp, "</div>\n");
			sprintf(signal, "%s_stay", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_adv.stay", NULL);
			websWrite(
				wp,
				"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,-128,0,wl_adv.stay)\" value=\"%s\" />\n",
				signal, nvram_default_get(signal, "-128"));
			websWrite(wp, "</div>\n");
			sprintf(signal, "%s_poll_time", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_adv.poll_time", NULL);
			websWrite(
				wp,
				"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,1,3600,wl_adv.poll_time)\" value=\"%s\" />\n",
				signal, nvram_default_get(signal, "10"));
			websWrite(wp, "</div>\n");
			sprintf(signal, "%s_strikes", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_adv.strikes", NULL);
			websWrite(
				wp,
				"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,1,60,wl_adv.strikes)\" value=\"%s\" />\n",
				signal, nvram_default_get(signal, "3"));
			websWrite(wp, "</div>\n");
			websWrite(wp, "</fieldset><br/>\n");
		} else if (is_supplicant(prefix)) {
			show_bgscan_options(wp, prefix);
		}
	}
	char webfilter[32];
	sprintf(webfilter, "%s_web_filter", prefix);
	showRadioDefaultOn(wp, "wl_adv.label17", webfilter);

	showbridgesettings(wp, prefix, 1, 1);
#elif HAVE_RT2880
	char webfilter[32];
	sprintf(webfilter, "%s_web_filter", prefix);
	showRadioDefaultOn(wp, "wl_adv.label17", webfilter);
	showbridgesettings(wp, getRADev(prefix), 1, 1);
#else
	char webfilter[32];
	sprintf(webfilter, "%s_web_filter", prefix);
	showRadioDefaultOn(wp, "wl_adv.label17", webfilter);
	if (!strcmp(prefix, "wl0"))
		showbridgesettings(wp, get_wl_instance_name(0), 1, 1);
	if (!strcmp(prefix, "wl1"))
		showbridgesettings(wp, get_wl_instance_name(1), 1, 1);
	if (!strcmp(prefix, "wl2"))
		showbridgesettings(wp, get_wl_instance_name(2), 1, 1);
#endif
#else
// BUFFALO Basic
#ifdef HAVE_MADWIFI
	// if (!strcmp (prefix, "wlan0"))
#endif
	{
		// #ifdef HAVE_MADWIFI
		// if (!strcmp (prefix, "wlan0")) //show client only on first
		// interface
		// #endif
		{
#ifdef HAVE_MADWIFI
			// if (!strcmp (prefix, "wlan0")) //show client only on first
			// interface
			// if (nvram_match ("wlan0_mode", "wdsap")
			// || nvram_match ("wlan0_mode", "wdssta"))
			// showOption (wp, "wl_basic.wifi_bonding", "wifi_bonding");
#endif

#ifdef HAVE_REGISTER
			int cpeonly = iscpe();
#else
			int cpeonly = 0;
#endif
			websWrite(
				wp,
				"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n",
				wl_mode);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			if (!has_no_apmode(prefix)) {
				if (!cpeonly) {
					websWrite(
						wp,
						"document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");\n",
						nvram_match(wl_mode, "ap") ? "selected=\\\"selected\\\"" : "");
				}
			}
#ifdef HAVE_MADWIFI
			if (has_wdsap(prefix)) {
				if (!cpeonly) {
					websWrite(
						wp,
						"document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");\n",
						nvram_match(wl_mode, "wdsap") ? "selected=\\\"selected\\\"" : "");
				}
			}
#endif
#if (!defined(HAVE_RT61) && !defined(HAVE_DIR860)) || defined(HAVE_MT76)
			websWrite(wp, "document.write(\"<option value=\\\"sta\\\" %s >\" + wl_basic.client + \"</option>\");\n",
				  nvram_match(wl_mode, "sta") ? "selected=\\\"selected\\\"" : "");
#endif
#if !defined(HAVE_RT2880) || defined(HAVE_MT76)
#ifdef HAVE_RELAYD
			websWrite(wp,
				  "document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientRelayd + \"</option>\");\n",
#else
			websWrite(wp,
				  "document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientBridge + \"</option>\");\n",
#endif
				  nvram_match(wl_mode, "wet") ? "selected=\\\"selected\\\"" : "");
#endif
#ifndef HAVE_MADWIFI
			if (!cpeonly) {
				websWrite(
					wp,
					"document.write(\"<option value=\\\"apsta\\\" %s >\" + wl_basic.repeater + \"</option>\");\n",
					nvram_match(wl_mode, "apsta") ? "selected=\\\"selected\\\"" : "");
				//#ifndef HAVE_RT2880
				websWrite(
					wp,
					"document.write(\"<option value=\\\"apstawet\\\" %s >\" + wl_basic.repeaterbridge + \"</option>\");\n",
					nvram_match(wl_mode, "apstawet") ? "selected=\\\"selected\\\"" : "");
			}
//#endif
#else
			if (has_wdsap(prefix)) {
				websWrite(
					wp,
					"document.write(\"<option value=\\\"wdssta\\\" %s >\" + wl_basic.wdssta + \"</option>\");\n",
					nvram_match(wl_mode, "wdssta") ? "selected=\\\"selected\\\"" : "");
				websWrite(
					wp,
					"document.write(\"<option value=\\\"wdssta_mtik\\\" %s >\" + wl_basic.wdssta_mtik + \"</option>\");\n",
					nvram_match(wl_mode, "wdssta_mtik") ? "selected=\\\"selected\\\"" : "");
			}
			if (has_mesh(prefix)) {
				websWrite(wp,
					  "document.write(\"<option value=\\\"mesh\\\" %s >\" + wl_basic.mesh + \"</option>\");\n",
					  nvram_match(wl_mode, "mesh") ? "selected=\\\"selected\\\"" : "");
			}
#endif
#ifndef HAVE_BUFFALO
			if (!cpeonly && has_ibss(prefix))
#else
			if (!cpeonly && !has_5ghz(prefix) && has_ibss(prefix))
#endif
				websWrite(
					wp,
					"document.write(\"<option value=\\\"infra\\\" %s >\" + wl_basic.adhoc + \"</option>\");\n",
					nvram_match(wl_mode, "infra") ? "selected=\\\"selected\\\"" : "");
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
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.clientRelaydDefaultGwMode)</script></div>");
		sprintf(wl_relayd, "%s_relayd_gw_auto", prefix);
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_relayd_gw_auto\" onclick=\"show_layer_ext(this, '%s_relayd_gw_ipaddr', false)\" %s /><script type=\"text/javascript\">Capture(share.auto)</script>&nbsp;(DHCP)&nbsp;\n",
			prefix, prefix, nvram_default_matchi(wl_relayd, 1, 1) ? "checked" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_relayd_gw_auto\" onclick=\"show_layer_ext(this, '%s_relayd_gw_ipaddr', true)\" %s/><script type=\"text/javascript\">Capture(share.manual)</script>\n",
			prefix, prefix, nvram_default_matchi(wl_relayd, 0, 1) ? "checked" : "");
		websWrite(wp, "</div>\n");
		sprintf(wl_relayd, "%s_relayd_gw_ipaddr", prefix);
		sscanf(nvram_safe_get(wl_relayd), "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
		sprintf(wl_relayd, "%s_relayd_gw_auto", prefix);
		websWrite(
			wp,
			"<div id=\"%s_relayd_gw_ipaddr\" class=\"setting\"%s>\n"
			"<input type=\"hidden\" name=\"%s_relayd_gw_ipaddr\" value=\"4\">\n"
			"<div class=\"label\"><script type=\"text/javascript\">Capture(share.gateway)</script></div>\n"
			"<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_0\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_1\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_2\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_3\" value=\"%d\" onblur=\"valid_range(this,1,254,'IP')\" class=\"num\">\n"
			"</div>\n",
			prefix, nvram_default_matchi(wl_relayd, 1, 0) ? " style=\"display: none; visibility: hidden;\"" : "",
			prefix, prefix, ip[0], prefix, ip[1], prefix, ip[2], prefix, ip[3]);
	}
#endif

	// wireless net mode
	show_netmode(wp, prefix);
	// turbo options
#ifdef HAVE_MADWIFI
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
	sprintf(wl_shortgi, "%s_shortgi", prefix);
	sprintf(wl_subf, "%s_subf", prefix);
	sprintf(wl_mubf, "%s_mubf", prefix);
	sprintf(wl_xr, "%s_xr", prefix);
	sprintf(wl_intmit, "%s_intmit", prefix);
	sprintf(wl_qboost, "%s_qboost", prefix);
	sprintf(wl_autoburst, "%s_autoburst", prefix);
	sprintf(wl_sifs_trigger_time, "%s_sifs_trigger_time", prefix);
	sprintf(wl_noise_immunity, "%s_noise_immunity", prefix);
	sprintf(wl_ofdm_weak_det, "%s_ofdm_weak_det", prefix);
	sprintf(wl_uapsd, "%s_uapsd", prefix);
	sprintf(wl_lowack, "%s_d_lowack", prefix);
	sprintf(wl_ldpc, "%s_ldpc", prefix);
	sprintf(wl_smps, "%s_smps", prefix);
#if 0
	showRadio(wp, "wl_basic.csma", wl_csma);
#endif
	if (is_mac80211(prefix) && has_dualband(prefix)) {
		char dualband[32];
		sprintf(dualband, "%s_dualband", prefix);
		nvram_default_get(dualband, "0");
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.dualband)</script></div><select name=\"%s\">\n",
			dualband);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + share.deflt + \"</option>\");\n",
			  nvram_matchi(dualband, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.bghz24 + \"</option>\");\n",
			  nvram_matchi(dualband, 2) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >\" + wl_basic.bghz5 + \"</option>\");\n",
			  nvram_matchi(dualband, 5) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}
	// showOption (wp, "wl_basic.extchannel", wl_xchanmode);
	if (!has_ad(prefix)) {
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div><select name=\"%s\" onchange=\"to_submit(this.form);\">\n",
			wl_width, prefix);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"20\\\" %s >\" + share.full + \"</option>\");\n",
			  nvram_matchi(wl_width, 20) ? "selected=\\\"selected\\\"" : "");
		/* limit channel options by mode */
		int canht40 = can_ht40(prefix);
		int canvht80 = can_vht80(prefix);
		if (!is_mac80211(prefix) || canht40)
			if (is_mac80211(prefix) && !is_ath5k(prefix)) {
				if ((nvram_nmatch("n-only", "%s_net_mode", prefix) ||
				     nvram_nmatch("ng-only", "%s_net_mode", prefix) ||
				     nvram_nmatch("n2-only", "%s_net_mode", prefix) ||
				     nvram_nmatch("mixed", "%s_net_mode", prefix) ||
				     nvram_nmatch("n5-only", "%s_net_mode", prefix) ||
				     nvram_nmatch("ac-only", "%s_net_mode", prefix) ||
				     nvram_nmatch("acn-mixed", "%s_net_mode", prefix) ||
				     nvram_nmatch("ax-only", "%s_net_mode", prefix) ||
				     nvram_nmatch("axg-only", "%s_net_mode", prefix) ||
				     nvram_nmatch("xacn-mixed", "%s_net_mode", prefix) ||
				     nvram_nmatch("na-only", "%s_net_mode", prefix)))
					websWrite(
						wp,
						"document.write(\"<option value=\\\"2040\\\" %s >\" + share.dynamicturbo + \"</option>\");\n",
						nvram_matchi(wl_width, 2040) ? "selected=\\\"selected\\\"" : "");
			}
		if (!is_mac80211(prefix) ||
		    (is_mac80211(prefix) &&
		     (nvram_nmatch("n-only", "%s_net_mode", prefix) || nvram_nmatch("ng-only", "%s_net_mode", prefix) ||
		      nvram_nmatch("n2-only", "%s_net_mode", prefix) || nvram_nmatch("mixed", "%s_net_mode", prefix) ||
		      nvram_nmatch("n5-only", "%s_net_mode", prefix) || nvram_nmatch("ac-only", "%s_net_mode", prefix) ||
		      nvram_nmatch("acn-mixed", "%s_net_mode", prefix) || nvram_nmatch("ax-only", "%s_net_mode", prefix) ||
		      nvram_nmatch("axg-only", "%s_net_mode", prefix) || nvram_nmatch("xacn-mixed", "%s_net_mode", prefix) ||
		      nvram_nmatch("na-only", "%s_net_mode", prefix)))) {
			if (!is_mac80211(prefix) || is_ath5k(prefix))
				websWrite(wp, "document.write(\"<option value=\\\"40\\\" %s >\" + share.turbo + \"</option>\");\n",
					  nvram_matchi(wl_width, 40) ? "selected=\\\"selected\\\"" : "");
			else if (canht40)
				websWrite(wp, "document.write(\"<option value=\\\"40\\\" %s >\" + share.ht40 + \"</option>\");\n",
					  nvram_matchi(wl_width, 40) ? "selected=\\\"selected\\\"" : "");
			if ((is_ath11k(prefix) || is_ath10k(prefix) || is_mvebu(prefix) || has_vht80(prefix)) &&
			    (has_5ghz(prefix) || (cansuperchannel(wp, prefix) && nvram_nmatch("1", "%s_turbo_qam", prefix))) &&
			    (nvram_nmatch("mixed", "%s_net_mode", prefix) || nvram_nmatch("ac-only", "%s_net_mode", prefix) ||
			     nvram_nmatch("acn-mixed", "%s_net_mode", prefix) || nvram_nmatch("ax-only", "%s_net_mode", prefix) ||
			     nvram_nmatch("xacn-mixed", "%s_net_mode", prefix) ||
			     (cansuperchannel(wp, prefix) && nvram_nmatch("1", "%s_turbo_qam", prefix)))) {
				if (canvht80)
					websWrite(
						wp,
						"document.write(\"<option value=\\\"80\\\" %s >\" + share.vht80 + \"</option>\");\n",
						nvram_matchi(wl_width, 80) ? "selected=\\\"selected\\\"" : "");
				if ((has_vht160(prefix) || has_he160(prefix)) && can_vht160(prefix) && has_5ghz(prefix))
					websWrite(
						wp,
						"document.write(\"<option value=\\\"160\\\" %s >\" + share.vht160 + \"</option>\");\n",
						nvram_matchi(wl_width, 160) ? "selected=\\\"selected\\\"" : "");
				if (has_vht80plus80(prefix) && canvht80 && has_5ghz(prefix))
					websWrite(
						wp,
						"document.write(\"<option value=\\\"80+80\\\" %s >\" + share.vht80plus80 + \"</option>\");\n",
						nvram_match(wl_width, "80+80") ? "selected=\\\"selected\\\"" : "");
			}
		}
#if !defined(HAVE_BUFFALO)
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K) && !defined(HAVE_MADIFI_MIMO)
		if (has_half(prefix))
			websWrite(wp, "document.write(\"<option value=\\\"10\\\" %s >\" + share.half + \"</option>\");\n",
				  nvram_matchi(wl_width, 10) ? "selected=\\\"selected\\\"" : "");
		if (has_quarter(prefix))
			websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >\" + share.quarter + \"</option>\");\n",
				  nvram_matchi(wl_width, 5) ? "selected=\\\"selected\\\"" : "");
		if (registered_has_subquarter() && has_subquarter(prefix)) {
			/* will be enabled once it is tested and the spectrum analyse is done */
			websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + share.subquarter + \"</option>\");\n",
				  nvram_matchi(wl_width, 2) ? "selected=\\\"selected\\\"" : "");
		}
#endif
#endif
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}
// test
#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
#else
	if (is_ap(prefix) || nvram_match(wl_mode, "infra") || nvram_match(wl_mode, "mesh"))
#endif
	{
		if (has_mimo(prefix) &&
		    (nvram_nmatch("n-only", "%s_net_mode", prefix) || nvram_nmatch("ng-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("mixed", "%s_net_mode", prefix) || nvram_nmatch("n2-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("n5-only", "%s_net_mode", prefix) || nvram_nmatch("ac-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("acn-mixed", "%s_net_mode", prefix) || nvram_nmatch("ax-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("axg-only", "%s_net_mode", prefix) || nvram_nmatch("xacn-mixed", "%s_net_mode", prefix) ||
		     nvram_nmatch("na-only", "%s_net_mode", prefix))) {
			show_channel(wp, prefix, prefix, 1);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.channel_width", NULL);
			websWrite(wp, "<select name=\"%s_nbw\">\n", prefix);
			//                      websWrite(wp,
			//                                "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\"0\" %s >\" + share.auto + \"</option>\");\n//]]>\n</script>\n",
			//                                nvram_nmatch("0", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
			websWrite(
				wp,
				"<option value=\"20\" %s>20 <script type=\"text/javascript\">Capture(wl_basic.mhz);</script></option>\n",
				nvram_nmatch("20", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
			websWrite(
				wp,
				"<option value=\"40\" %s>40 <script type=\"text/javascript\">Capture(wl_basic.mhz);</script></option>\n",
				nvram_nmatch("40", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
			if (has_ac(prefix) && has_5ghz(prefix) &&
			    (nvram_nmatch("mixed", "%s_net_mode", prefix) || nvram_nmatch("ac-only", "%s_net_mode", prefix) ||
			     nvram_nmatch("acn-mixed", "%s_net_mode", prefix) || nvram_nmatch("ax-only", "%s_net_mode", prefix) ||
			     nvram_nmatch("xacn-mixed", "%s_net_mode", prefix))) {
				websWrite(
					wp,
					"<option value=\"80\" %s><script type=\"text/javascript\">Capture(share.vht80);</script></option>\n",
					nvram_nmatch("80", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
				if (has_vht160(prefix) || has_he160(prefix)) {
					websWrite(
						wp,
						"<option value=\"160\" %s><script type=\"text/javascript\">Capture(share.vht160);</script></option>\n",
						nvram_nmatch("160", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
				}
				if (has_vht80plus80(prefix)) {
					websWrite(
						wp,
						"<option value=\"80+80\" %s><script type=\"text/javascript\">Capture(share.vht80plus80);</script></option>\n",
						nvram_nmatch("80+80", "%s_nbw", prefix) ? "selected=\"selected\"" : "");
				}
			}
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
			if (!is_ath5k(prefix) && nvram_nmatch("40", "%s_nbw", prefix)) {
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_basic.channel_wide", NULL);
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
				websWrite(
					wp,
					"<option value=\"upper\" %s><script type=\"text/javascript\">Capture(wl_basic.upper);</script></option>\n",
					nvram_nmatch("upper", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(
					wp,
					"<option value=\"lower\" %s><script type=\"text/javascript\">Capture(wl_basic.lower);</script></option>\n",
					nvram_nmatch("lower", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "</select>\n");
				websWrite(wp, "</div>\n");
			}
			if (nvram_nmatch("80", "%s_nbw", prefix) || nvram_nmatch("80+80", "%s_nbw",
										 prefix)) { // 802.11ac
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_basic.channel_wide", NULL);
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
				websWrite(
					wp,
					"<option value=\"ll\" %s><script type=\"text/javascript\">document.write(wl_basic.lower+\" \"+wl_basic.lower)</script></option>\n",
					nvram_nmatch("ll", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(
					wp,
					"<option value=\"lu\" %s><script type=\"text/javascript\">document.write(wl_basic.lower+\" \"+wl_basic.upper)</script></option>\n",
					nvram_nmatch("lu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(
					wp,
					"<option value=\"ul\" %s><script type=\"text/javascript\">document.write(wl_basic.upper+\" \"+wl_basic.lower)</script></option>\n",
					nvram_nmatch("ul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(
					wp,
					"<option value=\"uu\" %s><script type=\"text/javascript\">document.write(wl_basic.upper+\" \"+wl_basic.upper)</script></option>\n",
					nvram_nmatch("uu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "</select>\n");
				websWrite(wp, "</div>\n");
			}
#if 0
			if (nvram_nmatch("160", "%s_nbw", prefix)) {
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_basic.channel_wide", NULL);
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
				websWrite(wp, "<option value=\"lll\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lll);</script></option>\n",
					  (nvram_nmatch("lll", "%s_nctrlsb", prefix) || nvram_nmatch("lower", "%s_nctrlsb", prefix)) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"llu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_llu);</script></option>\n",
					  nvram_nmatch("llu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"lul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lul);</script></option>\n",
					  nvram_nmatch("lul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"luu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_luu);</script></option>\n",
					  nvram_nmatch("luu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"ull\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ull);</script></option>\n",
					  nvram_nmatch("ull", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"ulu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ulu);</script></option>\n",
					  nvram_nmatch("ulu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"uul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uul);</script></option>\n",
					  nvram_nmatch("uul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
				websWrite(wp, "<option value=\"uuu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uuu);</script></option>\n",
					  (nvram_nmatch("uuu", "%s_nctrlsb", prefix) || nvram_nmatch("upper", "%s_nctrlsb", prefix)) ? "selected=\"selected\"" : "");
				websWrite(wp, "</select>\n");
				websWrite(wp, "</div>\n");
			}
#endif
		} else {
			show_channel(wp, prefix, prefix, 0);
			if (is_mac80211(prefix)) {
				if (!is_ath5k(prefix) && (nvram_matchi(wl_width, 40) || nvram_matchi(wl_width, 2040))) {
					websWrite(wp, "<div class=\"setting\">\n");
					show_caption(wp, "label", "wl_basic.channel_wide", NULL);
					websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
					websWrite(
						wp,
						"<option value=\"ull\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_upr);</script></option>\n",
						(nvram_nmatch("ull", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("upper", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(
						wp,
						"<option value=\"luu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lwr);</script></option>\n",
						(nvram_nmatch("luu", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("lower", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(wp, "</select>\n");
					websWrite(wp, "</div>\n");
				}
				if (nvram_matchi(wl_width, 80) || nvram_match(wl_width, "80+80")) {
					websWrite(wp, "<div class=\"setting\">\n");
					show_caption(wp, "label", "wl_basic.channel_wide", NULL);
					websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
					websWrite(
						wp,
						"<option value=\"lul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ll);</script></option>\n",
						(nvram_nmatch("lul", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("lower", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(
						wp,
						"<option value=\"luu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lu);</script></option>\n",
						nvram_nmatch("luu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"ull\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ul);</script></option>\n",
						nvram_nmatch("ull", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"ulu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uu);</script></option>\n",
						(nvram_nmatch("ulu", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("upper", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(wp, "</select>\n");
					websWrite(wp, "</div>\n");
				}
				if (nvram_matchi(wl_width, 160)) {
					websWrite(wp, "<div class=\"setting\">\n");
					show_caption(wp, "label", "wl_basic.channel_wide", NULL);
					websWrite(wp, "<select name=\"%s_nctrlsb\" >\n", prefix);
					websWrite(
						wp,
						"<option value=\"lll\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lll);</script></option>\n",
						(nvram_nmatch("lll", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("lower", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(
						wp,
						"<option value=\"llu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_llu);</script></option>\n",
						nvram_nmatch("llu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"lul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_lul);</script></option>\n",
						nvram_nmatch("lul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"luu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_luu);</script></option>\n",
						nvram_nmatch("luu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"ull\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ull);</script></option>\n",
						nvram_nmatch("ull", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"ulu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_ulu);</script></option>\n",
						nvram_nmatch("ulu", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"uul\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uul);</script></option>\n",
						nvram_nmatch("uul", "%s_nctrlsb", prefix) ? "selected=\"selected\"" : "");
					websWrite(
						wp,
						"<option value=\"uuu\" %s><script type=\"text/javascript\">Capture(wl_basic.ch_pos_uuu);</script></option>\n",
						(nvram_nmatch("uuu", "%s_nctrlsb", prefix) ||
						 nvram_nmatch("upper", "%s_nctrlsb", prefix)) ?
							"selected=\"selected\"" :
							"");
					websWrite(wp, "</select>\n");
					websWrite(wp, "</div>\n");
				}
			}
		}
	}
	if (is_mac80211(prefix)) {
		char wl_overlap[16];
		sprintf(wl_overlap, "%s_overlap", prefix);
		showRadio(wp, "wl_basic.overlap", wl_overlap);
	}
	if (has_qam256(prefix) && has_2ghz(prefix)) {
		char wl_turboqam[32];
		sprintf(wl_turboqam, "%s_turbo_qam", prefix);
		showRadio(wp, "wl_basic.turboqam", wl_turboqam);
	}
	// wireless ssid
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.ssid)</script></div><input name=\"%s\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.ssid)\" value=\"",
		wl_ssid);
	tf_webWriteESCNV(wp, wl_ssid);
	websWrite(wp, "\" /></div>\n");

#ifdef HAVE_MADWIFI
	if (is_station(prefix)) {
		sprintf(wl_bssid, "%s_bssid", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.bssid", NULL);
		websWrite(wp,
			  "<input size=\"20\" maxlength=\"17\" name=\"%s_bssid\" onblur=\"valid_macs_all(this)\" value=\"%s\" />",
			  prefix, nvram_safe_get(wl_bssid));
		websWrite(wp, "</div>\n");
	}
#endif
#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
	if (is_ap(prefix) || nvram_match(wl_mode, "infra") || nvram_match(wl_mode, "apsta") || nvram_match(wl_mode, "apstawet"))
#else
	if (is_ap(prefix) || nvram_match(wl_mode, "infra") || nvram_match(wl_mode, "mesh"))
#endif
	{
		// ssid broadcast
		char wl_closed[16];
		sprintf(wl_closed, "%s_closed", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.label5", NULL);
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			wl_closed, nvram_matchi(wl_closed, 0) ? "checked=\"checked\"" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			wl_closed, nvram_matchi(wl_closed, 1) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
	}
#ifdef HAVE_IFL
	// label
	char wl_label[16];
	sprintf(wl_label, "%s_label", prefix);
	websWrite(
		wp,
		"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.if_label)</script></div><input type=\"text\" name=\"%s\" value=\"%s\" maxlength=\"20\"></div>\n",
		wl_label, nvram_safe_get(wl_label));
#endif
	// WIRELESS Advanced
	char advanced_label[32];
	sprintf(advanced_label, "%s_wl_advanced", prefix);
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.legend)</script></div>\n");
	websWrite(wp, "<input type=\"checkbox\" name=\"%s\" onclick=\"toggle_layer(this,'%s_layer')\"%s>", advanced_label,
		  advanced_label, websGetVar(wp, advanced_label, NULL) ? " checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div id=\"%s_layer\"%s>\n", advanced_label,
		  websGetVar(wp, advanced_label, NULL) ? "" : " style=\"display: none;\"");
#ifdef HAVE_IFL
	char wl_note[16];
	sprintf(wl_note, "%s_note", prefix);
	websWrite(
		wp,
		"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.if_info)</script></div><textarea name=\"%s\" cols=\"60\" rows=\"3\">%s</textarea></div>\n",
		wl_note, nvram_safe_get(wl_note));
#endif
	if (is_mac80211(prefix)) {
		if (isFXXN_PRO(prefix) == 1) {
			char wl_cardtype[32];
			sprintf(wl_cardtype, "%s_cardtype", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.cardtype", NULL);
			websWrite(wp, "<select name=\"%s\">\n", wl_cardtype);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
#ifdef HAVE_ONNET
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Atheros 2458</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 0, 0) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Atheros 3336</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 1, 0) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Atheros 5964</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 2, 0) ? "selected=\\\"selected\\\"" : "");
#else
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Atheros Generic</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 0, 0) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >DBII F36N-PRO</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 1, 0) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >DBII F64N-PRO</option>\");\n",
				  nvram_default_matchi(wl_cardtype, 2, 0) ? "selected=\\\"selected\\\"" : "");
#endif
			websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
		}
	}
#ifdef HAVE_MADWIFI
#ifndef HAVE_MAKSAT
#ifndef HAVE_DDLINK

	if (isXR36(prefix)) {
		char wl_cardtype[32];
		sprintf(wl_cardtype, "%s_cardtype", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.cardtype", NULL);
		websWrite(wp, "<select name=\"%s\">\n", wl_cardtype);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Ubiquiti XR3.3</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 0, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Ubiquiti XR3.6</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 1, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Ubiquiti XR3.7</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 2, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
	}

	if (isEMP(prefix)) {
		char wl_cardtype[32];
		sprintf(wl_cardtype, "%s_cardtype", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.cardtype", NULL);
		websWrite(wp, "<select name=\"%s\">\n", wl_cardtype);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Atheros Generic</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 0, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >Alfa Networks AWPCI085H</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 5, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"6\\\" %s >Alfa Networks AWPCI085P</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 6, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >Doodle Labs DLM105</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 7, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >MakSat MAK27</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 4, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >Senao EMP-8602</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 1, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >Senao EMP-8603-S</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 2, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >Senao EMP-8603</option>\");\n",
			  nvram_default_matchi(wl_cardtype, 3, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
	}
#endif
#endif // ! HAVE MAKSAT

	if (has_mesh(prefix)) {
		if (nvram_nmatch("mesh", "%s_mode", prefix)) {
			websWrite(
				wp,
				"<fieldset><legend><script type=\"text/javascript\">Capture(wl_basic.mesh_settings)</script></legend>");

			mesh_radio(wp, prefix, "mesh_fwding", 1);
			mesh_radio(wp, prefix, "mesh_nolearn", 0);
			mesh_radio(wp, prefix, "mesh_gate_announcements", 0);
			{
				char mparam[64];
				sprintf(mparam, "%s_mesh_hwmp_rootmode", prefix);
				char *names[] = { "\" + wl_basic.mesh_no_root + \"", "\" + wl_basic.mesh_preq_no_prep + \"",
						  "\" + wl_basic.mesh_preq_with_prep + \"", "\" + wl_basic.mesh_rann + \"",
						  "\" + wl_basic.preq_with_prep + \"" };
				showOptionsNames(wp, "wl_basic.mesh_hwmp_rootmode", mparam, "0 2 3 4", names,
						 nvram_default_get(mparam, "0"));
			}
			mesh_num(wp, prefix, "mesh_hwmp_rann_interval", 6, 5000);
			mesh_num(wp, prefix, "mesh_hwmp_max_preq_retries", 4, 4);
			mesh_num(wp, prefix, "mesh_hwmp_active_path_timeout", 6, 5000);
			mesh_num(wp, prefix, "mesh_hwmp_preq_min_interval", 4, 10);
			mesh_num(wp, prefix, "mesh_hwmp_net_diameter_traversal_time", 6, 50);
			mesh_num(wp, prefix, "mesh_hwmp_active_path_to_root_timeout", 6, 6000);
			mesh_num(wp, prefix, "mesh_hwmp_confirmation_interval", 6, 5000);
			mesh_num(wp, prefix, "mesh_retry_timeout", 4, 100);
			mesh_num(wp, prefix, "mesh_confirm_timeout", 4, 100);
			mesh_num(wp, prefix, "mesh_holding_timeout", 4, 100);
			mesh_num(wp, prefix, "mesh_max_peer_links", 4, 255);
			mesh_num(wp, prefix, "mesh_max_retries", 4, 3);
			mesh_num(wp, prefix, "mesh_ttl", 4, 31);
			mesh_num(wp, prefix, "mesh_element_ttl", 4, 31);
			mesh_num(wp, prefix, "mesh_path_refresh_time", 6, 1000);
			mesh_num(wp, prefix, "mesh_min_discovery_timeout", 4, 100);
			mesh_radio(wp, prefix, "mesh_auto_open_plinks", 1);
			mesh_num(wp, prefix, "mesh_sync_offset_max_neighor", 4, 10);
			mesh_num(wp, prefix, "mesh_rssi_threshold", 4, 0);
			{
				char mparam[64];
				sprintf(mparam, "%s_mesh_power_mode", prefix);
				char *names[] = { "\" + wl_basic.mesh_active + \"", "\" + wl_basic.mesh_deep + \"",
						  "\" + wl_basic.mesh_light + \"" };
				showOptionsNames(wp, "wl_basic.mesh_power_mode", mparam, "active light deep", names,
						 nvram_default_get(mparam, "active"));
			}
			mesh_num(wp, prefix, "mesh_awake_window", 6, 10);
			mesh_num(wp, prefix, "mesh_plink_timeout", 6, 0);
			mesh_radio(wp, prefix, "mesh_connected_to_gate", 0);
			mesh_radio(wp, prefix, "mesh_connected_to_as", 0);
			websWrite(wp, "</fieldset><br/>\n");
		}
	}
	if (is_ath10k(prefix) && has_fwswitch(prefix)) {
		char fw_type[32];
		sprintf(fw_type, "%s_fwtype", prefix);
		char *fw_names[] = { "DD-WRT", "VANILLA" };
		showOptionsNames(wp, "wl_basic.fw_type", fw_type, "ddwrt vanilla", fw_names, nvram_default_get(fw_type, "ddwrt"));
	}
#ifndef HAVE_NOCOUNTRYSEL
	if (!nvram_matchi("nocountrysel", 1)) {
		char wl_regdomain[32];
#ifdef HAVE_ATH9K
		if (!strcmp(prefix, "wlan0"))
#endif
		{
			sprintf(wl_regdomain, "%s_regdomain", prefix);
			if (1 || nvram_nmatch("1", "%s_regulatory", prefix) || !issuperchannel()) {
				char *list = getCountryList(COUNTRYLIST);
				showOptionsLabel(wp, "wl_basic.regdom_label", wl_regdomain, list, nvram_safe_get(wl_regdomain));
			}
		}
	}
#endif // ! HAVE MAKSAT \
	// power adjustment
	sprintf(power, "%s_txpwrdbm", prefix);
	// sprintf (maxpower, "%s_maxpower", prefix);
	if (issuperchannel()) // show
	// client
	// only on
	// first
	// interface
	{
		char regulatory[32];
		sprintf(regulatory, "%s_regulatory", prefix);
		nvram_default_get(regulatory, "0");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp,
			  "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.regulatory)</script></div>\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_regulatory\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			prefix, nvram_matchi(regulatory, 0) ? "checked" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_regulatory\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			prefix, nvram_matchi(regulatory, 1) ? "checked" : "");
		websWrite(wp, "</div>\n");
	}
	int txpower = nvram_geti(power);
#ifdef HAVE_ESPOD
#ifdef HAVE_SUB3
	if (txpower > 28) {
		txpower = 28;
		nvram_seti(power, 28);
	}
#else
	if (txpower > 30) {
		txpower = 30;
		nvram_seti(power, 30);
	}
#endif
#endif
#if !defined(HAVE_WZR450HP2) || !defined(HAVE_BUFFALO)
#ifdef HAVE_ATH9K
	if (is_ath10k(prefix) && nvram_nmatch("ddwrt", "%s_fwtype", prefix)) {
		char wl_po[32];
		sprintf(wl_po, "%s_power_override", prefix);
		nvram_default_get(wl_po, "0");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.power_override", NULL);
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_power_override\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			prefix, nvram_matchi(wl_po, 1) ? "checked" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_power_override\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			prefix, nvram_matchi(wl_po, 0) ? "checked" : "");
		websWrite(wp, "</div>\n");
	}
#endif

	websWrite(wp, "<div class=\"setting\">\n");
#ifdef HAVE_ATH9K
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.TXpower)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%d\" /> dBm (Max %d)\n",
		power, txpower + wifi_gettxpoweroffset(prefix), mac80211_get_maxpower(prefix) + wifi_gettxpoweroffset(prefix));
#else
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.TXpower)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%d\" /> dBm\n",
		power, txpower + wifi_gettxpoweroffset(prefix));
#endif
	websWrite(wp, "</div>\n");
	sprintf(power, "%s_antgain", prefix);
#ifndef HAVE_MAKSAT
	if (nvram_nmatch("1", "%s_regulatory", prefix))
#endif
	{
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.AntGain)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%s\" /> dBi\n",
			power, nvram_default_get(power, "0"));
		websWrite(wp, "</div>\n");
	}
#endif
#endif
	if (has_ax(prefix)) {
		char *netmode = nvram_nget("%s_net_mode", prefix);
		if (!strcmp(netmode, "mixed") || !strcmp(netmode, "xacn-mixed") || !strcmp(netmode, "ax-only") ||
		    !strcmp(netmode, "axg-only")) {
			char color[32];
			sprintf(color, "%s_he_bss_color", prefix);
			showInputNum(wp, "wl_basic.he_bss_color", color, 2, 2, 0);
		}
	}
	if (has_ldpc(prefix)) {
		char *netmode = nvram_nget("%s_net_mode", prefix);
		if ((strcmp(netmode, "mixed") && //
		     strcmp(netmode, "ac-only") && strcmp(netmode, "acn-mixed") && strcmp(netmode, "ax-only") &&
		     strcmp(netmode, "axg-only") && strcmp(netmode, "xacn-mixed")))
			showRadioInv(wp, "wl_basic.ldpc", wl_ldpc);
	}
	if (has_uapsd(prefix)) {
		showRadio(wp, "wl_basic.uapsd", wl_uapsd);
	}
	if (is_ap(prefix))
#ifdef HAVE_MVEBU
		showRadioDefaultOff(wp, "wl_basic.disassoc_low_ack", wl_lowack);
#else
		showRadioDefaultOn(wp, "wl_basic.disassoc_low_ack", wl_lowack);
#endif
	if (has_smps(prefix)) {
		sprintf(wl_smps, "%s_smps", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.smps", NULL);
		websWrite(wp, "<select name=\"%s\">\n", wl_smps);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + share.off + \"</option>\");\n", "0",
			  nvram_default_match(wl_smps, "0", "0") ? "selected=\\\"selected\\\"" : "");
		if (has_static_smps(prefix))
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + share.sttic + \"</option>\");\n", "1",
				  nvram_match(wl_smps, "1") ? "selected=\\\"selected\\\"" : "");
		if (has_dynamic_smps(prefix))
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + share.dynamic + \"</option>\");\n", "2",
				  nvram_match(wl_smps, "2") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n");
		websWrite(wp, "</div>\n");
	}
	if (!is_mvebu(prefix)) {
		if (is_mac80211(prefix)) {
			showRadio(wp, "wl_basic.intmit", wl_intmit);
		} else {
			showAutoOption(wp, "wl_basic.intmit", wl_intmit, 0);
		}
		if (nvram_match("experimental", "1")) {
			if (has_wave2(prefix)) {
				showRadio(wp, "wl_basic.dynamic_auto_bursting", wl_autoburst);
			}
		}
		if (has_qboost(prefix)) {
			if (has_qboost_tdma(prefix)) {
				showRadio(wp, "wl_basic.qboost_tdma", wl_qboost);
				websWrite(wp, "<div class=\"setting\">\n");
				show_caption(wp, "label", "wl_basic.sifs_trigger_time", NULL);
				websWrite(
					wp,
					"<input class=\"num\" name=\"%s\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,20,wl_basic.sifs_trigger_time)\" value=\"%s\" />\n",
					wl_sifs_trigger_time, nvram_default_get(wl_sifs_trigger_time, "0"));
				websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(share.ms)</script></div>\n");
			} else
				showRadio(wp, "wl_basic.qboost", wl_qboost);
		}
		if (!is_mac80211(prefix)) {
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_basic.noise_immunity", NULL);
			websWrite(wp, "<select name=\"%s\">\n", wl_noise_immunity);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >0</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 0, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 1, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >2</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 2, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >3</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 3, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >4</option>\");\n",
				  nvram_default_matchi(wl_noise_immunity, 4, 4) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
			showRadio(wp, "wl_basic.ofdm_weak_det", wl_ofdm_weak_det);
		}
	}

	showrtssettings(wp, prefix);
	showairtimepolicy(wp, prefix, prefix);
	if (!is_mac80211(prefix)) {
		show_rates(wp, prefix, 0);
		show_rates(wp, prefix, 1);
	}
	showRadioDefaultOn(wp, "wl_basic.preamble", wl_preamble);
	if (!is_mac80211(prefix)) {
		showRadio(wp, "wl_basic.extrange", wl_xr);
		showRadio(wp, "wl_basic.supergff", wl_ff);
	}
	if (has_shortgi(prefix)) {
		nvram_default_get(wl_shortgi, "1");
		showRadio(wp, "wl_basic.shortgi", wl_shortgi);
	}
	if ((has_5ghz(prefix) && has_ac(prefix)) || has_ax(prefix)) {
		if (has_subeamforming(prefix)) {
			nvram_default_get(wl_subf, "0");
			showRadio(wp, "wl_basic.subf", wl_subf);
		}
		if (has_mubeamforming(prefix)) {
			nvram_default_get(wl_mubf, "0");
			showRadio(wp, "wl_basic.mubf", wl_mubf);
		}
	}
#ifndef HAVE_BUFFALO
#if !defined(HAVE_FONERA) && !defined(HAVE_LS2) && !defined(HAVE_MERAKI)
	if (!is_mac80211(prefix)) {
		if (has_5ghz(prefix)) {
			if (nvram_nmatch("1", "%s_regulatory", prefix) || !issuperchannel()) {
				showRadio(wp, "wl_basic.outband", wl_outdoor);
			}
		}
	}
#endif
#endif

// antenna settings
#if defined(HAVE_PICO2) || defined(HAVE_PICO2HP) || defined(HAVE_PICO5)
	// do nothing
#elif defined(HAVE_NS2) || defined(HAVE_NS5) || defined(HAVE_LC2) || defined(HAVE_LC5) || defined(HAVE_NS3)

	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label24)</script></div><select name=\"%s\" >\n",
		wl_txantenna);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.vertical + \"</option>\");\n",
		  nvram_matchi(wl_txantenna, 0) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.horizontal + \"</option>\");\n",
		  nvram_matchi(wl_txantenna, 1) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >\" + wl_basic.adaptive + \"</option>\");\n",
		  nvram_matchi(wl_txantenna, 3) ? "selected=\\\"selected\\\"" : "");
#if defined(HAVE_NS5) || defined(HAVE_NS2) || defined(HAVE_NS3)
	websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.external + \"</option>\");\n",
		  nvram_matchi(wl_txantenna, 2) ? "selected=\\\"selected\\\"" : "");
#endif
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
#else
	if (!is_mac80211(prefix)) {
		showRadio(wp, "wl_basic.diversity", wl_diversity);
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label12)</script></div><select name=\"%s\" >\n",
			wl_txantenna);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");\n",
			  nvram_matchi(wl_txantenna, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");\n",
			  nvram_matchi(wl_txantenna, 1) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");\n",
			  nvram_matchi(wl_txantenna, 2) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label13)</script></div><select name=\"%s\" >\n",
			wl_rxantenna);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");\n",
			  nvram_matchi(wl_rxantenna, 0) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");\n",
			  nvram_matchi(wl_rxantenna, 1) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");\n",
			  nvram_matchi(wl_rxantenna, 2) ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	} else {
		int maxrx = mac80211_get_avail_rx_antenna(prefix);
		int maxtx = mac80211_get_avail_tx_antenna(prefix);
		if (maxtx > 0) {
			websWrite(
				wp,
				"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.txchainmask)</script></div><select name=\"%s\" >\n",
				wl_txantenna);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n",
				  nvram_matchi(wl_txantenna, 1) ? "selected=\\\"selected\\\"" : "");
			if (maxtx > 1)
				websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >1+2</option>\");\n",
					  nvram_matchi(wl_txantenna, 3) ? "selected=\\\"selected\\\"" : "");
			if (maxtx > 3)
				websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >1+3</option>\");\n",
					  nvram_matchi(wl_txantenna, 5) ? "selected=\\\"selected\\\"" : "");
			if (maxtx > 5)
				websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >1+2+3</option>\");\n",
					  nvram_matchi(wl_txantenna, 7) ? "selected=\\\"selected\\\"" : "");
			if (maxtx > 7)
				websWrite(wp, "document.write(\"<option value=\\\"15\\\" %s >1+2+3+4</option>\");\n",
					  nvram_matchi(wl_txantenna, 15) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script>\n");
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
		}

		if (maxrx > 0) {
			websWrite(
				wp,
				"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.rxchainmask)</script></div><select name=\"%s\" >\n",
				wl_rxantenna);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n",
				  nvram_matchi(wl_rxantenna, 1) ? "selected=\\\"selected\\\"" : "");
			if (maxrx > 1)
				websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >1+2</option>\");\n",
					  nvram_matchi(wl_rxantenna, 3) ? "selected=\\\"selected\\\"" : "");
			if (maxrx > 3)
				websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >1+3</option>\");\n",
					  nvram_matchi(wl_rxantenna, 5) ? "selected=\\\"selected\\\"" : "");
			if (maxrx > 5)
				websWrite(wp, "document.write(\"<option value=\\\"7\\\" %s >1+2+3</option>\");\n",
					  nvram_matchi(wl_rxantenna, 7) ? "selected=\\\"selected\\\"" : "");
			if (maxrx > 7)
				websWrite(wp, "document.write(\"<option value=\\\"15\\\" %s >1+2+3+4</option>\");\n",
					  nvram_matchi(wl_rxantenna, 15) ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script>\n");
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");
		}
	}
#endif
#endif

// ap isolation
#ifdef HAVE_MADWIFI
	sprintf(wl_isolate, "%s_ap_isolate", prefix);
	showRadio(wp, "wl_adv.label11", wl_isolate);
	char bcn[32];
	sprintf(bcn, "%s_bcn", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_adv.label6", NULL);
	websWrite(
		wp,
		"<input class=\"num\" name=\"%s\" size=\"5\" maxlength=\"5\" onblur=\"valid_range(this,15,65535,wl_adv.label6)\" value=\"%s\" />\n",
		bcn, nvram_default_get(bcn, "100"));
	websWrite(wp, "</div>\n");
	char dtim[32];
	sprintf(dtim, "%s_dtim", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_adv.label7", NULL);
	websWrite(
		wp,
		"<input class=\"num\" name=\"%s\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,1,255,wl_adv.label7)\" value=\"%s\" />\n",
		dtim, nvram_default_get(dtim, "2"));
	websWrite(wp, "</div>\n");
	if (has_airtime_fairness(prefix)) {
		char wl_atf[16];
		sprintf(wl_atf, "%s_atf", prefix);
		showRadioDefaultOn(wp, "wl_basic.atf", wl_atf);
	}
#ifdef HAVE_MAC80211_COMPRESS
	if (is_mac80211(prefix) && !is_mvebu(prefix)) {
		char wl_fc[16];
		sprintf(wl_fc, "%s_fc", prefix);
		FILE *fp = fopen("/proc/modules", "rb");
		char line[245];
		int zstd = 0;
		if (fp) {
			while (!feof(fp) && fgets(line, sizeof(line), fp)) {
				if (strstr(line, "zstd")) {
					zstd = 1;
					break;
				}
			}
			fclose(fp);
		}
		char *names_zstd[] = { "\" + share.disabled + \"", "LZO", "LZ4", "LZMA", "ZSTD" };
		char *names[] = {
			"\" + share.disabled + \"",
			"LZO",
			"LZ4",
			"LZMA",
		};
		if (zstd)
			showOptionsNames(wp, "wl_basic.fc", wl_fc, "0 1 3 2 4", names_zstd, nvram_default_get(wl_fc, "0"));
		else
			showOptionsNames(wp, "wl_basic.fc", wl_fc, "0 1 3 2", names, nvram_default_get(wl_fc, "0"));
	}
#endif
	// wmm

	sprintf(wmm, "%s_wmm", prefix);
	if (is_mac80211(prefix))
		showRadioDefaultOn(wp, "wl_adv.label18", wmm);
	else
		showRadio(wp, "wl_adv.label18", wmm);
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
		show_caption(wp, "label", "wl_basic.scanlist", NULL);
		websWrite(wp, "<input name=\"%s\" size=\"32\" maxlength=\"512\" value=\"%s\" />\n", wl_scanlist,
			  nvram_default_get(wl_scanlist, "default"));
		websWrite(wp, "</div>\n");
	}
#endif

	if (has_acktiming(prefix)) {
		sprintf(power, "%s_distance", prefix);
		//websWrite(wp, "<br />\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_basic.label6", NULL);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"8\" maxlength=\"8\" onblur=\"valid_range(this,0,99999999,wl_basic.label6)\" value=\"%s\" />\n",
			power, nvram_default_get(power, "500"));
		websWrite(
			wp,
			"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 500 \" + share.meters + \")\");\n//]]>\n</script></span>\n");
		websWrite(wp, "</div>\n");
	}
#ifdef HAVE_MADWIFI
	if (is_ap(prefix) || nvram_nmatch("infra", "%s_mode", prefix) || nvram_nmatch("mesh", "%s_mode", prefix)) {
		sprintf(power, "%s_maxassoc", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wl_adv.label10", NULL);
		websWrite(
			wp,
			"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,256,wl_adv.label10)\" value=\"%s\" />\n",
			power, nvram_default_get(power, "256"));
		websWrite(
			wp,
			"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 256 \" + status_wireless.legend3 + \")\");\n//]]>\n</script></span>\n");
		websWrite(wp, "</div>\n");
	}
	if (is_mac80211(prefix)) {
		if (is_ap(prefix)) {
			char signal[32];
			websWrite(
				wp,
				"<fieldset><legend><script type=\"text/javascript\">Capture(wl_adv.droplowsignal)</script></legend>");
			sprintf(signal, "%s_connect", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_adv.connect", NULL);
			websWrite(
				wp,
				"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,-128,0,wl_adv.connect)\" value=\"%s\" />\n",
				signal, nvram_default_get(signal, "-128"));
			websWrite(wp, "</div>\n");
			sprintf(signal, "%s_stay", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_adv.stay", NULL);
			websWrite(
				wp,
				"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,-128,0,wl_adv.stay)\" value=\"%s\" />\n",
				signal, nvram_default_get(signal, "-128"));
			websWrite(wp, "</div>\n");
			sprintf(signal, "%s_poll_time", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_adv.poll_time", NULL);
			websWrite(
				wp,
				"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,1,3600,wl_adv.poll_time)\" value=\"%s\" />\n",
				signal, nvram_default_get(signal, "10"));
			websWrite(wp, "</div>\n");
			sprintf(signal, "%s_strikes", prefix);
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "wl_adv.strikes", NULL);
			websWrite(
				wp,
				"<input class=\"num\" name=\"%s\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,1,60,wl_adv.strikes)\" value=\"%s\" />\n",
				signal, nvram_default_get(signal, "3"));
			websWrite(wp, "</div>\n");
			websWrite(wp, "</fieldset><br/>\n");
		} else if (is_supplicant(prefix)) {
			show_bgscan_options(wp, prefix);
		}
	}
	char webfilter[32];
	sprintf(webfilter, "%s_web_filter", prefix);
	showRadioDefaultOn(wp, "wl_adv.label17", webfilter);

	showbridgesettings(wp, prefix, 1, 1);
#elif HAVE_RT2880
	char webfilter[32];
	sprintf(webfilter, "%s_web_filter", var);
	showRadioDefaultOn(wp, "wl_adv.label17", webfilter);
	showbridgesettings(wp, getRADev(prefix), 1, 1);
#else
	char webfilter[32];
	sprintf(webfilter, "%s_web_filter", var);
	showRadioDefaultOn(wp, "wl_adv.label17", webfilter);
	if (!strcmp(prefix, "wl0"))
		showbridgesettings(wp, get_wl_instance_name(0), 1, 1);
	if (!strcmp(prefix, "wl1"))
		showbridgesettings(wp, get_wl_instance_name(1), 1, 1);
	if (!strcmp(prefix, "wl2"))
		showbridgesettings(wp, get_wl_instance_name(2), 1, 1);
#endif
	websWrite(wp, "</div>\n");
#endif // end BUFFALO
#ifdef HAVE_ATH9K
	int inst;
	char radio_timer[32];
	sscanf(prefix, "wlan%d", &inst);
	sprintf(radio_timer, "radio%d_timer_enable", inst);
	show_caption_legend(wp, "wl_basic.legend2");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wl_basic.radiotimer", NULL);
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"radio%d_timer_enable\" %s onclick=\"show_layer_ext(this, 'radio%d', true)\" />",
		inst, nvram_match(radio_timer, "1") ? "checked=\"checked\"" : "", inst);
	websWrite(wp, "<script type=\"text/javascript\">Capture(share.enable)</script>&nbsp\n");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"radio%d_timer_enable\" %s onclick=\"show_layer_ext(this, 'radio%d', false)\" />",
		inst, nvram_match(radio_timer, "0") ? "checked=\"checked\"" : "", inst);
	websWrite(wp, "<script type=\"text/javascript\">Capture(share.disable)</script>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div id=\"radio%d\">\n", inst);
	websWrite(wp, "<table id=\"radio%d_table\"></table>", inst);
	websWrite(wp, "<br />\n");
	websWrite(wp, "<div class=\"center\">\n");
	websWrite(wp, "<script type=\"text/javascript\">\n");
	websWrite(wp, "//<![CDATA[\n");
	websWrite(
		wp,
		"	document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.allways_on + \"\\\"  onclick=\\\"setWlTimer('all',true, %d);\\\" />\");\n",
		inst);
	websWrite(
		wp,
		"	document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.allways_off + \"\\\" onclick=\\\"setWlTimer('all',false, %d);\\\" />\");\n",
		inst);
	websWrite(wp, "//]]>\n");
	websWrite(wp, "</script>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "</div>\n");
#endif
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + share.copy + \"\\\" onclick=\\\"copy_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
		prefix);
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + share.paste + \"\\\" onclick=\\\"paste_submit(this.form,'%s')\\\" />\");\n//]]>\n</script>\n",
		prefix);
	websWrite(wp, "</fieldset>\n");
	websWrite(wp, "<br />\n");
	if (!has_no_apmode(prefix)) {
#ifdef HAVE_REGISTER
		if (!iscpe())
#endif
			show_virtualssid(wp, prefix);
	}
}

EJ_VISIBLE void ej_gen_timer_compute(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_ATH9K
	int c = getdevicecount();
	int i;
	for (i = 0; i < c; i++) {
		websWrite(wp, "F.radio%d_on_time.value = computeWlTimer(%d);\n", i, i);
	}
#endif
}

EJ_VISIBLE void ej_gen_init_timer(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_ATH9K
	int c = getdevicecount();
	int i;
	for (i = 0; i < c; i++) {
		char radio[32];
		sprintf(radio, "radio%d_on_time", i);
		websWrite(wp, "setRadioTable(%d);\n", i);
		websWrite(wp, "initWlTimer('%s',%d);\n", nvram_default_get(radio, "111111111111111111111111"), i);
		websWrite(wp, "show_layer_ext(document.wireless.radio%d_timer_enable, 'radio%d', %d);", i, i,
			  nvram_nmatch("1", "radio%d_timer_enable", i) ? 1 : 0);
	}
#endif
}

EJ_VISIBLE void ej_gen_timer_fields(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_ATH9K
	int c = getdevicecount();
	int i;
	for (i = 0; i < c; i++) {
		websWrite(wp, "<input type=\"hidden\" name=\"radio%d_on_time\">\n", i);
	}
#endif
}

EJ_VISIBLE void ej_show_wireless(webs_t wp, int argc, char_t **argv)
{
#if defined(HAVE_ANTAIRA) && defined(HAVE_HABANERO)
	websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(sbutton.survey)</script></legend>");
	websWrite(wp, "<div class=\"center\">");
	websWrite(
		wp,
		"<input title=\"Site survey (wlan0)\" class=\"button\" type=\"button\" name=\"site_survey\" value=\"Wireless site survey (wlan0)\" onclick=\"openWindow('Site_Survey-wlan0.asp', 760, 700)\"/>");
	websWrite(
		wp,
		"<input title=\"Site survey (wlan1)\" class=\"button\" type=\"button\" name=\"site_survey\" value=\"Wireless site survey (wlan1)\" onclick=\"openWindow('Site_Survey-wlan1.asp', 760, 700)\"/>");
	websWrite(wp, "</div>");
	websWrite(wp, "</fieldset>");
	websWrite(wp, "<br></br>");
#endif

#ifndef HAVE_MADWIFI
#if defined(HAVE_NORTHSTAR) || defined(HAVE_80211AC) && !defined(HAVE_BUFFALO)
	if (!nvram_matchi("nocountrysel", 1)) {
		char wl_regdomain[32];
		sprintf(wl_regdomain, "wl_regdomain");
		websWrite(wp, "<h2><script type=\"text/javascript\">Capture(wl_basic.country_settings)</script></h2>\n");
		websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(wl_basic.regdom)</script></legend>\n");
		char *list = getCountryList(COUNTRYLIST);
		showOptionsLabel(wp, "wl_basic.regdom_label", wl_regdomain, list, nvram_default_get("wl_regdomain", "EUROPE"));
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.regmode)</script></div>\n");
		char *wl_regmode = nvram_default_get("wl_reg_mode", "off");
		websWrite(wp, "<select name=\"wl_reg_mode\">\n");
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"off\\\" %s > \" + share.off + \"</option>\");\n",
			  !strcmp(wl_regmode, "off") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"h\\\" %s >802.11h Loose</option>\");\n",
			  !strcmp(wl_regmode, "h") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"h_strict\\\" %s >802.11h Strict</option>\");\n",
			  !strcmp(wl_regmode, "h_strict") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"d\\\" %s >802.11d</option>\");\n",
			  !strcmp(wl_regmode, "d") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n");
		websWrite(wp, "</div>\n");
		websWrite(
			wp,
			"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.tpcdb)</script></div>\n");
		char *wl_tpcdb = nvram_default_get("wl_tpc_db", "off");
		websWrite(wp, "<select name=\"wl_tpc_db\">\n");
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"off\\\" %s >0 (\" + share.off + \")</option>\");\n",
			  !strcmp(wl_tpcdb, "off") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"2\\\" %s >2</option>\");\n",
			  !strcmp(wl_tpcdb, "2") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"3\\\" %s >3</option>\");\n",
			  !strcmp(wl_tpcdb, "3") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >4\");\n",
			  !strcmp(wl_tpcdb, "4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "//]]>\n</script>\n</select>\n");
		websWrite(wp, "</div>\n</fieldset><br />\n");
	}
#endif
#endif
	int c = getdevicecount();
	int i;
	for (i = 0; i < c; i++) {
		char buf[16];
		sprintf(buf, WIFINAME "%d", i);
		internal_ej_show_wireless_single(wp, buf);
	}
#ifdef HAVE_GUESTPORT
	websWrite(wp, "<input type=\"hidden\" name=\"gp_modify\" id=\"gp_modify\" value=\"\">\n");
#endif
	return;
}

void show_addconfig(webs_t wp, char *prefix)
{
#ifdef HAVE_MADWIFI
	char vvar[32];
	strcpy(vvar, prefix);
	rep(vvar, '.', 'X');
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">Custom Config</div>\n");
	websWrite(wp, "<textarea cols=\"60\" rows=\"4\" id=\"%s_config\" name=\"%s_config\"></textarea>\n", vvar, vvar);
	websWrite(wp, "<script type=\"text/javascript\">\n");
	websWrite(wp, "//<![CDATA[\n");
	websWrite(wp, "var %s_config = fix_cr( '", vvar);
	char varname[32];
	sprintf(varname, "%s_config", prefix);
	tf_webWriteESCNV(wp, varname);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_config\").value = %s_config;\n", vvar, vvar);
	websWrite(wp, "//]]>\n");
	websWrite(wp, "</script>\n");
	websWrite(wp, "</div>\n");
#endif
}

static void show_cryptovar(webs_t wp, char *prefix, char *name, char *var, int selmode, int force)
{
	char nvar[80];
	char gvar[80];
	strcpy(gvar, prefix);
	rep(gvar, '.', 'X');

	sprintf(nvar, "%s_%s", prefix, var);
	if (selmode)
		websWrite(
			wp,
			"<input type=\"checkbox\" name=\"%s\" value=\"1\" onclick=\"SelMode('%s', '%s_security_mode',this.form.%s_security_mode.selectedIndex,this.form)\" %s %s /><script type=\"text/javascript\">Capture(%s)</script>",
			nvar, prefix, gvar, gvar, force ? "checked=\"checked\"" : selmatch(nvar, "1", "checked=\"checked\""),
			force ? "disabled=\"disabled\"" : "", name);
	else
		websWrite(
			wp,
			"<input type=\"checkbox\" name=\"%s\" value=\"1\" %s %s /><script type=\"text/javascript\">Capture(%s)</script>",
			nvar, force ? "checked=\"checked\"" : selmatch(nvar, "1", "checked=\"checked\""),
			force ? "disabled=\"disabled\"" : "", name);
}

typedef struct pair {
	char *name;
	char *nvname;
	int (*valid)(const char *prefix);
	int (*valid2)(const char *prefix);
	int (*valid3)(const char *prefix);
	int (*forcecrypto)(const char *prefix);
};

static int alwaystrue(const char *prefix)
{
	return 1;
}

static int noad(const char *prefix)
{
	return !has_ad(prefix);
}

static int aponly(const char *prefix)
{
	return is_ap(prefix);
}

static int nomesh(const char *prefix)
{
	return (!nvram_nmatch("mesh", "%s_mode", prefix));
}

static int aponly_wpa3(const char *prefix)
{
	return (aponly(prefix) && has_wpa3(prefix));
}

static int aponly_wpa3_gcmp256(const char *prefix)
{
	return (aponly(prefix) && has_wpa3(prefix) && has_gcmp_256(prefix));
}

static int aponly_wpa3_gcmp128(const char *prefix)
{
	return (aponly(prefix) && has_wpa3(prefix) && has_gcmp_128(prefix));
}

static int wpa3_gcmp256(const char *prefix)
{
	return (has_wpa3(prefix) && has_gcmp_256(prefix));
}

static int wpa3_gcmp128(const char *prefix)
{
	return (has_wpa3(prefix) && has_gcmp_128(prefix));
}

static int no_suiteb(const char *prefix)
{
	return !nvram_nmatch("1", "%s_wpa3", prefix);
}

static int suiteb(const char *prefix)
{
	return !no_suiteb(prefix);
}

static int no_suiteb192(const char *prefix)
{
	return !nvram_nmatch("1", "%s_wpa3-192", prefix);
}

static int suiteb192(const char *prefix)
{
	return !no_suiteb192(prefix);
}

static int wpaauth(const char *prefix)
{
	if (nvram_nmatch("8021X", "%s_security_mode", prefix)) {
		return nvram_nmatch("1", "%s_wpa", prefix) || //
		       nvram_nmatch("1", "%s_wpa2", prefix) || //
		       nvram_nmatch("1", "%s_wpa2-sha256", prefix) || //
		       nvram_nmatch("1", "%s_wpa3", prefix) || //
		       nvram_nmatch("1", "%s_wpa3-128", prefix) || //
		       nvram_nmatch("1", "%s_wpa3-192", prefix);
	}
	return nvram_nmatch("1", "%s_psk", prefix) || //
	       nvram_nmatch("1", "%s_psk2", prefix) || //
	       nvram_nmatch("1", "%s_psk2-sha256", prefix) || //
	       nvram_nmatch("1", "%s_psk3", prefix) || //
	       nvram_nmatch("1", "%s_owe", prefix) || //
	       nvram_nmatch("1", "%s_wpa", prefix) || //
	       nvram_nmatch("1", "%s_wpa2", prefix) || //
	       nvram_nmatch("1", "%s_wpa2-sha256", prefix) || //
	       nvram_nmatch("1", "%s_wpa3", prefix) || //
	       nvram_nmatch("1", "%s_wpa3-128", prefix) || //
	       nvram_nmatch("1", "%s_wpa3-192", prefix);
}

static int no_suiteb_no_wpa3(const char *prefix)
{
	return (!nvram_nmatch("1", "%s_wpa3-192", prefix) && //
		!nvram_nmatch("1", "%s_wpa3-128", prefix) && //
		!nvram_nmatch("1", "%s_wpa3", prefix) && //
		!nvram_nmatch("1", "%s_owe", prefix) && //
		!nvram_nmatch("1", "%s_psk3", prefix));
}

static int check_already_owe(const char *owe, const char *prefix)
{
	char *owe_if = nvram_nget("%s_owe_ifname", prefix);
	if (*owe_if) {
		char *m = nvram_nget("%s_owe_ifname", owe_if);
		if (*m) {
			if (nvram_nmatch("1", "%s_owe", owe_if)) {
				if (!strcmp(owe_if, owe))
					return 1;
				else
					return 0;
			}
		}
	}
	return 1;
}

static int owe_possible(const char *prefix)
{
	int possible = 0;
	char var[32];
	char *next;
	char master[32];
	strcpy(master, prefix);
	rep(master, '.', 0);
	char *vifs = nvram_nget("%s_vifs", master);
	if (strcmp(master, prefix) && (nvram_nmatch("disabled", "%s_akm", master) || *nvram_nget("%s_akm", master) == 0)) {
		possible = check_already_owe(prefix, master);
		if (possible)
			return possible && nomesh(prefix);
	}
	foreach(var, vifs, next)
	{
		if (strcmp(var, prefix) && (nvram_nmatch("disabled", "%s_akm", var) || *nvram_nget("%s_akm", var) == 0)) {
			possible = check_already_owe(prefix, var);
			if (possible)
				return possible && nomesh(prefix);
		}
	}
	return 0;
}

#ifdef HAVE_MADWIFI
void show_authtable(webs_t wp, char *prefix, int show80211x)
{
	struct pair s_cryptopair[] = {
		{ "wpa.ccmp", "ccmp", noad, wpaauth, alwaystrue },
		{ "wpa.ccmp_256", "ccmp-256", has_ccmp_256, wpaauth, alwaystrue },
		{ "wpa.tkip", "tkip", noad, wpaauth, no_suiteb_no_wpa3 },
		//              { "wpa.gcmp_128", "gcmp", has_ad, wpaauth, alwaystrue },
		{ "wpa.gcmp_128", "gcmp", has_gcmp_128, wpaauth, alwaystrue, suiteb },
		{ "wpa.gcmp_256", "gcmp-256", has_gcmp_256, wpaauth, alwaystrue, suiteb192 },
	};

	struct pair s_authpair_wpa[] = { { "wpa.psk", "psk", alwaystrue, alwaystrue, nomesh },
					 { "wpa.psk2", "psk2", alwaystrue, alwaystrue, nomesh },
					 { "wpa.psk2_sha256", "psk2-sha256", has_wpa3, is_mac80211, nomesh },
					 { "wpa.psk3", "psk3", has_wpa3, is_mac80211, alwaystrue },
					 { "wpa.wpa", "wpa", aponly, alwaystrue, nomesh },
					 { "wpa.wpa2", "wpa2", aponly, alwaystrue, nomesh },
					 { "wpa.wpa2_sha256", "wpa2-sha256", aponly_wpa3, is_mac80211, nomesh },
					 { "wpa.wpa3", "wpa3", aponly_wpa3, is_mac80211, nomesh },
					 { "wpa.wpa3_128", "wpa3-128", aponly_wpa3_gcmp128, has_gmac_128, nomesh },
					 { "wpa.wpa3_192", "wpa3-192", aponly_wpa3_gcmp256, has_gmac_256, nomesh },
					 { "wpa.owe", "owe", aponly_wpa3, is_mac80211, owe_possible } };
	struct pair s_authpair_80211x[] = { { "wpa.wpa", "wpa", alwaystrue, alwaystrue, alwaystrue },
					    { "wpa.wpa2", "wpa2", alwaystrue, alwaystrue, alwaystrue },
					    { "wpa.wpa2_sha256", "wpa2-sha256", has_wpa3, alwaystrue, alwaystrue },
					    { "wpa.wpa3", "wpa3", has_wpa3, is_mac80211, alwaystrue },
					    { "wpa.wpa3_128", "wpa3-128", wpa3_gcmp128, has_gmac_128, alwaystrue },
					    { "wpa.wpa3_192", "wpa3-192", wpa3_gcmp256, has_gmac_256, alwaystrue },
					    { "wpa.wep_8021x", "802.1x", alwaystrue, alwaystrue, alwaystrue } };
	struct pair s_authmethod[] = {
		{ "wpa.peap", "peap", alwaystrue, alwaystrue, alwaystrue },
		{ "wpa.leap", "leap", alwaystrue, alwaystrue, alwaystrue },
		{ "wpa.tls", "tls", alwaystrue, alwaystrue, alwaystrue },
		{ "wpa.ttls", "ttls", alwaystrue, alwaystrue, alwaystrue },
	};
	struct pair *cryptopair;
	struct pair *authpair_wpa;
	struct pair *authmethod;

	cryptopair = malloc(sizeof(s_cryptopair));
	if (show80211x)
		authpair_wpa = malloc(sizeof(s_authpair_80211x));
	else
		authpair_wpa = malloc(sizeof(s_authpair_wpa));
	authmethod = malloc(sizeof(s_authmethod));

	int i, cnt = 0;
	int alen = 0;
	int clen = 0;
	int mlen = 0;

	for (i = 0; i < sizeof(s_cryptopair) / sizeof(struct pair); i++) {
		if (s_cryptopair[i].valid(prefix) && s_cryptopair[i].valid2(prefix) && s_cryptopair[i].valid3(prefix))
			memcpy(&cryptopair[cnt++], &s_cryptopair[i], sizeof(struct pair));
	}
	clen = cnt;
	cnt = 0;

	for (i = 0; i < sizeof(s_authmethod) / sizeof(struct pair); i++) {
		if (s_authmethod[i].valid(prefix) && s_authmethod[i].valid2(prefix) && s_authmethod[i].valid3(prefix))
			memcpy(&authmethod[cnt++], &s_authmethod[i], sizeof(struct pair));
	}
	mlen = cnt;
	cnt = 0;

	if (!show80211x) {
		for (i = 0; i < sizeof(s_authpair_wpa) / sizeof(struct pair); i++) {
			if (s_authpair_wpa[i].valid(prefix) && s_authpair_wpa[i].valid2(prefix) && s_authpair_wpa[i].valid3(prefix))
				memcpy(&authpair_wpa[cnt++], &s_authpair_wpa[i], sizeof(struct pair));
		}
	} else {
		for (i = 0; i < sizeof(s_authpair_80211x) / sizeof(struct pair); i++) {
			if (s_authpair_80211x[i].valid(prefix) && s_authpair_80211x[i].valid2(prefix) &&
			    s_authpair_80211x[i].valid3(prefix))
				memcpy(&authpair_wpa[cnt++], &s_authpair_80211x[i], sizeof(struct pair));
		}
	}
	alen = cnt;

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<table class=\"table\" summary=\"WPA Algorithms\">\n");
	if (show80211x) {
		websWrite(
			wp,
			"<tr><th><script type=\"text/javascript\">Capture(sec80211x.xsuptype)</script></th><th><script type=\"text/javascript\">Capture(wpa.auth_mode)</script></th>"
			"<th><script type=\"text/javascript\">Capture(wpa.algorithms)</script></th></tr>\n");
	} else {
		websWrite(
			wp,
			"<tr><th><script type=\"text/javascript\">Capture(wpa.auth_mode)</script></th><th><script type=\"text/javascript\">Capture(wpa.algorithms)</script></th></tr>\n");
	}
	int count = 0;
	while (1) {
		int s = 0;
		int c = 0;
		int m = 0;
		if (count < alen) {
			s = 1;
		}

		if (count < clen) {
			c = 1;
		}

		if (show80211x && count < mlen) {
			m = 1;
		}
		if (s || c || m) {
			websWrite(wp, "<tr>\n");
			if (show80211x) {
				websWrite(wp, "<td>");
				if (m) {
					show_cryptovar(wp, prefix, authmethod[count].name, authmethod[count].nvname, 1, 0);
				} else {
					websWrite(wp, "&nbsp;");
				}
				websWrite(wp, "</td>\n");
			}
			websWrite(wp, "<td>");
			if (s) {
				show_cryptovar(wp, prefix, authpair_wpa[count].name, authpair_wpa[count].nvname, 1, 0);
			} else {
				websWrite(wp, "&nbsp;");
			}
			websWrite(wp, "</td>\n");
			websWrite(wp, "<td>");
			if (c) {
				show_cryptovar(wp, prefix, cryptopair[count].name, cryptopair[count].nvname, 0,
					       cryptopair[count].forcecrypto ? cryptopair[count].forcecrypto(prefix) : 0);
			} else
				websWrite(wp, "&nbsp;");
			websWrite(wp, "</td>\n");
			websWrite(wp, "</tr>\n");
		}
		if (!s && !c && !m)
			break;
		count++;
	}

	websWrite(wp, "</table>\n");
	websWrite(wp, "</div>\n");
	debug_free(authpair_wpa);
	debug_free(authmethod);
	debug_free(cryptopair);
}

#endif
void show_owe(webs_t wp, char *prefix)
{
	if (nvram_nmatch("1", "%s_owe", prefix)) {
		char var[32];
		char *next;
		char master[32];
		strcpy(master, prefix);
		rep(master, '.', 0);
		char *vifs = nvram_nget("%s_vifs", master);

		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.owe_ifname)</script></div>\n");
		websWrite(wp, "<select name=\"%s_owe_ifname\">\n", prefix);
		if (strcmp(master, prefix) && (nvram_nmatch("disabled", "%s_akm", master) || *nvram_nget("%s_akm", master) == 0))
			websWrite(wp, "<option value=\"%s\" %s >%s - %s</option>\n", master,
				  nvram_nmatch(master, "%s_owe_ifname", prefix) ? "selected=\"selected\"" : "", master,
				  nvram_nget("%s_ssid", master));
		foreach(var, vifs, next)
		{
			if (strcmp(var, prefix) && (nvram_nmatch("disabled", "%s_akm", var) || *nvram_nget("%s_akm", var) == 0))
				websWrite(wp, "<option value=\"%s\" %s >%s - %s</option>\n", var,
					  nvram_nmatch(var, "%s_owe_ifname", prefix) ? "selected=\"selected\"" : "", var,
					  nvram_nget("%s_ssid", var));
		}
		websWrite(wp, "</select></div>\n");
	}
}

void show_preshared(webs_t wp, char *prefix)
{
	char var[80];
	cprintf("show preshared");

#ifndef HAVE_MADWIFI
	sprintf(var, "%s_crypto", prefix);
	websWrite(wp, "<div><div class=\"setting\">\n");
	show_caption(wp, "label", "wpa.algorithms", NULL);
	websWrite(wp, "<select name=\"%s_crypto\">\n", prefix);
	if (has_ad(prefix)) {
		websWrite(wp, "<option value=\"gcmp\" %s><script type=\"text/javascript\">Capture(wpa.gcmp)</script></option>\n",
			  selmatch(var, "gcmp", "selected=\"selected\""));
	} else {
		websWrite(wp, "<option value=\"aes\" %s><script type=\"text/javascript\">Capture(wpa.ccmp)</script></option>\n",
			  selmatch(var, "aes", "selected=\"selected\""));
		websWrite(
			wp,
			"<option value=\"tkip+aes\" %s><script type=\"text/javascript\">Capture(wpa.tkip_ccmp)</script></option>\n",
			selmatch(var, "tkip+aes", "selected=\"selected\""));
		websWrite(wp, "<option value=\"tkip\" %s><script type=\"text/javascript\">Capture(wpa.tkip)</script></option>\n",
			  selmatch(var, "tkip", "selected=\"selected\""));
		if (has_gcmp(prefix)) {
			websWrite(
				wp,
				"<option value=\"ccmp-256\" %s><script type=\"text/javascript\">Capture(wpa.ccmp_256)</script></option>\n",
				selmatch(var, "ccmp-256", "selected=\"selected\""));
			websWrite(
				wp,
				"<option value=\"gcmp\" %s><script type=\"text/javascript\">Capture(wpa.gcmp)</script></option>\n",
				selmatch(var, "gcmp", "selected=\"selected\""));
			websWrite(
				wp,
				"<option value=\"gcmp-256\" %s><script type=\"text/javascript\">Capture(wpa.gcmp_256)</script></option>\n",
				selmatch(var, "gcmp-256", "selected=\"selected\""));
		}
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
#else
	if (nvram_nmatch("1", "%s_psk3", prefix) && !nvram_nmatch("1", "%s_psk", prefix) && !nvram_nmatch("1", "%s_psk2", prefix) &&
	    !nvram_nmatch("1", "%s_psk2sha256", prefix)) {
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wpa.sae_key", NULL);
		sprintf(var, "%s_sae_key", prefix);
		websWrite(
			wp,
			"<input type=\"password\" id=\"%s_sae_key\" name=\"%s_sae_key\" class=\"no-check\" autocomplete=\"new-password\" size=\"32\" value=\"",
			prefix, prefix);
		tf_webWriteESCNV(wp, var);
		websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
		websWrite(
			wp,
			"<input type=\"checkbox\" name=\"%s_wl_unmask\" value=\"0\" onclick=\"setElementMask('%s_sae_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
			prefix, prefix);
		websWrite(wp, "</div>\n");

	} else
#endif
	{
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wpa.shared_key", NULL);
		sprintf(var, "%s_wpa_psk", prefix);
		websWrite(
			wp,
#ifdef HAVE_BUFFALO
			"<input type=\"password\" id=\"%s_wpa_psk\" name=\"%s_wpa_psk\" class=\"no-check\" autocomplete=\"new-password\" onblur=\"valid_wpa_psk(this, true);\" maxlength=\"64\" size=\"32\" value=\"",
#else
			"<input type=\"password\" id=\"%s_wpa_psk\" name=\"%s_wpa_psk\" class=\"no-check\" autocomplete=\"new-password\" onblur=\"valid_psk_length(this);\" maxlength=\"64\" size=\"32\" value=\"",
#endif
			prefix, prefix);
		tf_webWriteESCNV(wp, var);
		websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
		websWrite(
			wp,
			"<input type=\"checkbox\" name=\"%s_wl_unmask\" value=\"0\" onclick=\"setElementMask('%s_wpa_psk', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
			prefix, prefix);
		websWrite(wp, "</div>\n");
	}
}

void show_radius(webs_t wp, char *prefix, int showmacformat, int backup)
{
	char var[80];
	cprintf("show radius\n");
	if (showmacformat) {
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "radius.label2", NULL);
		websWrite(wp, "<select name=\"%s_radmactype\">\n", prefix);
		websWrite(wp, "<option value=\"0\" %s >aabbcc-ddeeff</option>\n",
			  nvram_prefix_match("radmactype", prefix, "0") ? "selected" : "");
		websWrite(wp, "<option value=\"1\" %s >aabbccddeeff</option>\n",
			  nvram_prefix_match("radmactype", prefix, "1") ? "selected" : "");
		websWrite(wp, "<option value=\"2\" %s >aa:bb:cc:dd:ee:ff</option>\n",
			  nvram_prefix_match("radmactype", prefix, "2") ? "selected" : "");
		websWrite(wp, "<option value=\"3\" %s >aa-bb-cc-dd-ee-ff</option>\n",
			  nvram_prefix_match("radmactype", prefix, "3") ? "selected" : "");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}

	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "radius.label3", NULL);
	websWrite(wp, "<input type=\"hidden\" name=\"%s_radius_ipaddr\" value=\"4\" />\n", prefix);
	show_ip(wp, prefix, "radius_ipaddr", 0, 1, "radius.label3");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "radius.label4", NULL);
	sprintf(var, "%s_radius_port", prefix);
	websWrite(
		wp,
		"<input name=\"%s_radius_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label4)\" value=\"%s\" />\n",
		prefix, nvram_default_get(var, "1812"));
	websWrite(wp, "<span class=\"default\"><script type=\"text/javascript\">\n");
	websWrite(wp, "//<![CDATA[\n");
	websWrite(wp, "document.write(\"(\" + share.deflt + \": 1812)\");\n");
	websWrite(wp, "//]]>\n");
	websWrite(wp, "</script></span>\n</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "radius.label7", NULL);
	sprintf(var, "%s_radius_key", prefix);
	websWrite(
		wp,
		"<input type=\"password\" id=\"%s_radius_key\" autocomplete=\"new-password\" name=\"%s_radius_key\" maxlength=\"79\" size=\"32\" value=\"",
		prefix, prefix);
	tf_webWriteESCNV(wp, var);
	websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
	websWrite(
		wp,
		"<input type=\"checkbox\" name=\"%s_radius_unmask\" value=\"0\" onclick=\"setElementMask('%s_radius_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
		prefix, prefix);
	websWrite(wp, "</div>\n");
	if (backup) {
#ifdef HAVE_MADWIFI
		sprintf(var, "%s_radius_retry", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "radius.retry", NULL);
		websWrite(
			wp,
			"<input name=\"%s_radius_retry\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.retry)\" value=\"%s\" />\n",
			prefix, nvram_default_get(var, "600"));
		websWrite(
			wp,
			"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 600)\");\n//]]>\n</script></span>\n</div>\n");
#endif
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "radius.label23", NULL);
		websWrite(wp, "<input type=\"hidden\" name=\"%s_radius2_ipaddr\" value=\"4\" />\n", prefix);
		show_ip(wp, prefix, "radius2_ipaddr", 0, 1, "radius.label23");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "radius.label24", NULL);
		sprintf(var, "%s_radius2_port", prefix);
		websWrite(
			wp,
			"<input name=\"%s_radius2_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label24)\" value=\"%s\" />\n",
			prefix, nvram_default_get(var, "1812"));
		websWrite(
			wp,
			"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 1812)\");\n//]]>\n</script></span>\n</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "radius.label27", NULL);
		sprintf(var, "%s_radius2_key", prefix);
		websWrite(
			wp,
			"<input type=\"password\" id=\"%s_radius2_key\" name=\"%s_radius2_key\" autocomplete=\"new-password\" maxlength=\"79\" size=\"32\" value=\"",
			prefix, prefix);
		tf_webWriteESCNV(wp, var);
		websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
		websWrite(
			wp,
			"<input type=\"checkbox\" name=\"%s_radius2_unmask\" value=\"0\" onclick=\"setElementMask('%s_radius2_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
			prefix, prefix);
		websWrite(wp, "</div>\n");
	}
#ifdef HAVE_MADWIFI
	if (!showmacformat) {
		char acct[32];
		char vvar[32];
		strcpy(vvar, prefix);
		rep(vvar, '.', 'X');
		sprintf(acct, "%s_acct", prefix);
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label18)</script></div>\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idacct', true);\" name=\"%s_acct\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>\n",
			vvar, prefix, nvram_default_matchi(acct, 1, 0) ? "checked=\"checked\"" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idacct', false);\" name=\"%s_acct\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
			vvar, prefix, nvram_default_matchi(acct, 0, 0) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div id=\"%s_idacct\">\n", vvar);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "radius.label13", NULL);
		websWrite(wp, "<input type=\"hidden\" name=\"%s_acct_ipaddr\" value=\"4\" />\n", prefix);
		show_ip(wp, prefix, "acct_ipaddr", 0, 1, "radius.label13");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "radius.label14", NULL);
		sprintf(var, "%s_acct_port", prefix);
		websWrite(
			wp,
			"<input name=\"%s_acct_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label14)\" value=\"%s\" />\n",
			prefix, nvram_default_get(var, "1813"));
		websWrite(
			wp,
			"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 1813)\");\n//]]>\n</script></span>\n</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "radius.label17", NULL);
		sprintf(var, "%s_acct_key", prefix);
		websWrite(
			wp,
			"<input type=\"password\" autocomplete=\"new-password\" id=\"%s_acct_key\" name=\"%s_acct_key\" maxlength=\"79\" size=\"32\" value=\"",
			prefix, prefix);
		tf_webWriteESCNV(wp, var);
		websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
		websWrite(
			wp,
			"<input type=\"checkbox\" name=\"%s_acct_unmask\" value=\"0\" onclick=\"setElementMask('%s_acct_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
			prefix, prefix);
		websWrite(wp, "</div>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<script>\n//<![CDATA[\n ");
		websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_acct\"), \"%s_idacct\", %s);\n", prefix, vvar,
			  nvram_matchi(acct, 1) ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");
	}
	/* force client ip */
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "radius.local_ip", NULL);
	websWrite(wp, "<input type=\"hidden\" name=\"%s_local_ip\" value=\"4\" />\n", prefix);
	show_ip(wp, prefix, "local_ip", 0, 1, "radius.label3");
	websWrite(wp, "</div>\n");
#endif
}

#ifdef HAVE_WPA_SUPPLICANT
#ifndef HAVE_MICRO
static void init_80211x_layers(webs_t wp, char *prefix)
{
#ifndef HAVE_MADWIFI
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
#endif
}

EJ_VISIBLE void ej_init_80211x_layers(webs_t wp, int argc, char_t **argv)
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
#endif
}

#ifndef HAVE_MADWIFI
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
	show_caption(wp, "label", "sec80211x.xsuptype", NULL);
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"peap\" onclick=\"enable_idpeap('%s')\" %s />Peap&nbsp;\n",
		prefix, prefix, nvram_prefix_match("8021xtype", prefix, "peap") ? "checked=\"checked\"" : "");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"leap\" onclick=\"enable_idleap('%s')\" %s />Leap&nbsp;\n",
		prefix, prefix, nvram_prefix_match("8021xtype", prefix, "leap") ? "checked=\"checked\"" : "");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"tls\" onclick=\"enable_idtls('%s')\" %s />TLS&nbsp;\n",
		prefix, prefix, nvram_prefix_match("8021xtype", prefix, "tls") ? "checked=\"checked\"" : "");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"ttls\" onclick=\"enable_idttls('%s')\" %s />TTLS&nbsp;\n",
		prefix, prefix, nvram_prefix_match("8021xtype", prefix, "ttls") ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
#ifdef HAVE_MADWIFI
#ifdef HAVE_80211R
	char bssft[64];
	sprintf(bssft, "%s_ft", prefix);
	showRadio(wp, "wpa.ft", bssft);
#endif
#endif
	// ttls authentication
	websWrite(wp, "<div id=\"idttls%s\">\n", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.user", NULL);
	websWrite(
		wp,
		"<input name=\"%s_ttls8021xuser\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, nvram_prefix_get("ttls8021xuser", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.anon", NULL);
	websWrite(wp, "<input name=\"%s_ttls8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
		  nvram_prefix_get("ttls8021xanon", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.passwd", NULL);
	websWrite(
		wp,
		"<input name=\"%s_ttls8021xpasswd\" type=\"password\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, nvram_prefix_get("ttls8021xpasswd", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.phase2", NULL);
	websWrite(wp, "<input name=\"%s_ttls8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
		  nvram_prefix_get("ttls8021xphase2", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.servercertif", NULL);
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_ttls8021xca\" name=\"%s_ttls8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_ttls8021xca = fix_cr( '", prefix);
	char namebuf[64];
	sprintf(namebuf, "%s_ttls8021xca", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_ttls8021xca\").value = %s_ttls8021xca;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.options", NULL);
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"3\" id=\"%s_ttls8021xaddopt\" name=\"%s_ttls8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
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
	show_caption(wp, "label", "share.user", NULL);
	websWrite(
		wp,
		"<input name=\"%s_peap8021xuser\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, nvram_prefix_get("peap8021xuser", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.anon", NULL);
	websWrite(wp, "<input name=\"%s_peap8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
		  nvram_prefix_get("peap8021xanon", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.passwd", NULL);
	websWrite(
		wp,
		"<input name=\"%s_peap8021xpasswd\" type=\"password\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, nvram_prefix_get("peap8021xpasswd", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.phase2", NULL);
	websWrite(wp, "<input name=\"%s_peap8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
		  nvram_prefix_get("peap8021xphase2", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.servercertif", NULL);
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_peap8021xca\" name=\"%s_peap8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_peap8021xca = fix_cr( '", prefix);
	sprintf(namebuf, "%s_peap8021xca", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_peap8021xca\").value = %s_peap8021xca;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.options", NULL);
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"3\" id=\"%s_peap8021xaddopt\" name=\"%s_peap8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
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
	show_caption(wp, "label", "share.user", NULL);
	websWrite(
		wp,
		"<input name=\"%s_leap8021xuser\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, nvram_prefix_get("leap8021xuser", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.anon", NULL);
	websWrite(wp, "<input name=\"%s_leap8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
		  nvram_prefix_get("leap8021xanon", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.passwd", NULL);
	websWrite(
		wp,
		"<input name=\"%s_leap8021xpasswd\" type=\"password\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, nvram_prefix_get("leap8021xpasswd", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.phase2", NULL);
	websWrite(wp, "<input name=\"%s_leap8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
		  nvram_prefix_get("leap8021xphase2", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.options", NULL);
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"3\" id=\"%s_leap8021xaddopt\" name=\"%s_leap8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
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
	show_caption(wp, "label", "sec80211x.keyxchng", NULL);
	websWrite(wp, "<select name=\"%s_tls8021xkeyxchng\"> size=\"1\"\n", prefix);
	websWrite(wp, "<option value=\"wep\" %s>Radius/WEP</option>\n", selmatch(var, "wep", "selected=\"selected\""));
	websWrite(wp, "<option value=\"wpa2\" %s>WPA2 Enterprise</option>\n", selmatch(var, "wpa2", "selected=\"selected\""));
	websWrite(wp, "<option value=\"wpa2mixed\" %s>WPA2 Enterprise (Mixed)</option>\n",
		  selmatch(var, "wpa2mixed", "selected=\"selected\""));
	websWrite(wp, "<option value=\"wpa\" %s>WPA Enterprise</option>\n", selmatch(var, "wpa", "selected=\"selected\""));
	websWrite(wp, "</select></div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.user", NULL);
	websWrite(
		wp,
		"<input name=\"%s_tls8021xuser\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, nvram_prefix_get("tls8021xuser", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.anon", NULL);
	websWrite(wp, "<input name=\"%s_tls8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
		  nvram_prefix_get("tls8021xanon", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.passwd", NULL);
	websWrite(
		wp,
		"<input name=\"%s_tls8021xpasswd\" type=\"password\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, nvram_prefix_get("tls8021xpasswd", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.phase2", NULL);
	websWrite(wp, "<input name=\"%s_tls8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
		  nvram_prefix_get("tls8021xphase2", prefix));
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.servercertif", NULL);
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xca\" name=\"%s_tls8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_tls8021xca = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xca", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_tls8021xca\").value = %s_tls8021xca;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.clientcertif", NULL);
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xpem\" name=\"%s_tls8021xpem\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_tls8021xpem = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xpem", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_tls8021xpem\").value = %s_tls8021xpem;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.privatekey", NULL);
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xprv\" name=\"%s_tls8021xprv\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_tls8021xprv = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xprv", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(wp, "document.getElementById(\"%s_tls8021xprv\").value = %s_tls8021xprv;\n", prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "sec80211x.options", NULL);
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"3\" id=\"%s_tls8021xaddopt\" name=\"%s_tls8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
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
	websWrite(wp, "show_layer_ext(document.wpa.%s_8021xtype, 'idpeap%s', %s);\n", prefix, prefix,
		  nvram_match(peap, "peap") ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.wpa.%s_8021xtype, 'idtls%s', %s);\n", prefix, prefix,
		  nvram_match(peap, "tls") ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.wpa.%s_8021xtype, 'idleap%s', %s);\n", prefix, prefix,
		  nvram_match(peap, "leap") ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");
}
#else
void show_80211X(webs_t wp, char *prefix)
{
	char type[32];
	char var[80];
	char namebuf[64];
#ifdef HAVE_MADWIFI
#ifdef HAVE_80211R
	char bssft[64];
	sprintf(bssft, "%s_ft", prefix);
	showRadio(wp, "wpa.ft", bssft);
#endif
#endif
	char akm[32];
	sprintf(akm, "%s_akm", prefix);

	// ttls authentication

	if (nvhas(akm, "ttls")) {
		websWrite(wp, "<fieldset>\n");
		show_caption_legend(wp, "sec80211x.ttls");

		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "share.user", NULL);
		websWrite(
			wp,
			"<input name=\"%s_ttls8021xuser\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
			prefix, nvram_prefix_get("ttls8021xuser", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.anon", NULL);
		websWrite(wp, "<input name=\"%s_ttls8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
			  nvram_prefix_get("ttls8021xanon", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "share.passwd", NULL);
		websWrite(
			wp,
			"<input name=\"%s_ttls8021xpasswd\" type=\"password\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
			prefix, nvram_prefix_get("ttls8021xpasswd", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.phase2", NULL);
		websWrite(wp, "<input name=\"%s_ttls8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
			  nvram_prefix_get("ttls8021xphase2", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.servercertif", NULL);
		websWrite(
			wp,
			"<textarea cols=\"60\" rows=\"6\" id=\"%s_ttls8021xca\" name=\"%s_ttls8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
			prefix, prefix);
		websWrite(wp, "var %s_ttls8021xca = fix_cr( '", prefix);
		sprintf(namebuf, "%s_ttls8021xca", prefix);
		tf_webWriteESCNV(wp, namebuf);
		websWrite(wp, "' );\n");
		websWrite(wp, "document.getElementById(\"%s_ttls8021xca\").value = %s_ttls8021xca;\n", prefix, prefix);
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.options", NULL);
		websWrite(
			wp,
			"<textarea cols=\"60\" rows=\"3\" id=\"%s_ttls8021xaddopt\" name=\"%s_ttls8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
			prefix, prefix);
		websWrite(wp, "var %s_ttls8021xaddopt = fix_cr( '", prefix);
		sprintf(namebuf, "%s_ttls8021xaddopt", prefix);
		tf_webWriteESCNV(wp, namebuf);
		websWrite(wp, "' );\n");
		websWrite(wp, "document.getElementById(\"%s_ttls8021xaddopt\").value = %s_ttls8021xaddopt;\n", prefix, prefix);
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</fieldset><br/>\n");
	}
	// peap authentication
	if (nvhas(akm, "peap")) {
		websWrite(wp, "<fieldset>\n");
		show_caption_legend(wp, "sec80211x.peap");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "share.user", NULL);
		websWrite(
			wp,
			"<input name=\"%s_peap8021xuser\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
			prefix, nvram_prefix_get("peap8021xuser", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.anon", NULL);
		websWrite(wp, "<input name=\"%s_peap8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
			  nvram_prefix_get("peap8021xanon", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "share.passwd", NULL);
		websWrite(
			wp,
			"<input name=\"%s_peap8021xpasswd\" type=\"password\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
			prefix, nvram_prefix_get("peap8021xpasswd", prefix));

		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.phase1", NULL);
		websWrite(wp, "<input name=\"%s_peap8021xphase1\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
			  nvram_prefix_get("peap8021xphase1", prefix));

		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.phase2", NULL);
		websWrite(wp, "<input name=\"%s_peap8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
			  nvram_prefix_get("peap8021xphase2", prefix));

		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.servercertif", NULL);
		websWrite(
			wp,
			"<textarea cols=\"60\" rows=\"6\" id=\"%s_peap8021xca\" name=\"%s_peap8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
			prefix, prefix);
		websWrite(wp, "var %s_peap8021xca = fix_cr( '", prefix);
		sprintf(namebuf, "%s_peap8021xca", prefix);
		tf_webWriteESCNV(wp, namebuf);
		websWrite(wp, "' );\n");
		websWrite(wp, "document.getElementById(\"%s_peap8021xca\").value = %s_peap8021xca;\n", prefix, prefix);
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.options", NULL);
		websWrite(
			wp,
			"<textarea cols=\"60\" rows=\"3\" id=\"%s_peap8021xaddopt\" name=\"%s_peap8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
			prefix, prefix);
		websWrite(wp, "var %s_peap8021xaddopt = fix_cr( '", prefix);
		sprintf(namebuf, "%s_peap8021xaddopt", prefix);
		tf_webWriteESCNV(wp, namebuf);
		websWrite(wp, "' );\n");
		websWrite(wp, "document.getElementById(\"%s_peap8021xaddopt\").value = %s_peap8021xaddopt;\n", prefix, prefix);
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</fieldset><br/>\n");
	}
	// leap authentication
	if (nvhas(akm, "leap")) {
		websWrite(wp, "<fieldset>\n");
		show_caption_legend(wp, "sec80211x.leap");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "share.user", NULL);
		websWrite(
			wp,
			"<input name=\"%s_leap8021xuser\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
			prefix, nvram_prefix_get("leap8021xuser", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.anon", NULL);
		websWrite(wp, "<input name=\"%s_leap8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
			  nvram_prefix_get("leap8021xanon", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "share.passwd", NULL);
		websWrite(
			wp,
			"<input name=\"%s_leap8021xpasswd\" type=\"password\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
			prefix, nvram_prefix_get("leap8021xpasswd", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.phase2", NULL);
		websWrite(wp, "<input name=\"%s_leap8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
			  nvram_prefix_get("leap8021xphase2", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.options", NULL);
		websWrite(
			wp,
			"<textarea cols=\"60\" rows=\"3\" id=\"%s_leap8021xaddopt\" name=\"%s_leap8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
			prefix, prefix);
		websWrite(wp, "var %s_leap8021xaddopt = fix_cr( '", prefix);
		sprintf(namebuf, "%s_leap8021xaddopt", prefix);
		tf_webWriteESCNV(wp, namebuf);
		websWrite(wp, "' );\n");
		websWrite(wp, "document.getElementById(\"%s_leap8021xaddopt\").value = %s_leap8021xaddopt;\n", prefix, prefix);
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</fieldset><br/>\n");
	}
	// tls authentication
	if (nvhas(akm, "tls")) {
		websWrite(wp, "<fieldset>\n");
		show_caption_legend(wp, "sec80211x.tls");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "share.user", NULL);
		websWrite(
			wp,
			"<input name=\"%s_tls8021xuser\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
			prefix, nvram_prefix_get("tls8021xuser", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.anon", NULL);
		websWrite(wp, "<input name=\"%s_tls8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
			  nvram_prefix_get("tls8021xanon", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "share.passwd", NULL);
		websWrite(
			wp,
			"<input name=\"%s_tls8021xpasswd\" type=\"password\" autocomplete=\"new-password\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
			prefix, nvram_prefix_get("tls8021xpasswd", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.phase2", NULL);
		websWrite(wp, "<input name=\"%s_tls8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n", prefix,
			  nvram_prefix_get("tls8021xphase2", prefix));
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.servercertif", NULL);
		websWrite(
			wp,
			"<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xca\" name=\"%s_tls8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
			prefix, prefix);
		websWrite(wp, "var %s_tls8021xca = fix_cr( '", prefix);
		sprintf(namebuf, "%s_tls8021xca", prefix);
		tf_webWriteESCNV(wp, namebuf);
		websWrite(wp, "' );\n");
		websWrite(wp, "document.getElementById(\"%s_tls8021xca\").value = %s_tls8021xca;\n", prefix, prefix);
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.clientcertif", NULL);
		websWrite(
			wp,
			"<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xpem\" name=\"%s_tls8021xpem\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
			prefix, prefix);
		websWrite(wp, "var %s_tls8021xpem = fix_cr( '", prefix);
		sprintf(namebuf, "%s_tls8021xpem", prefix);
		tf_webWriteESCNV(wp, namebuf);
		websWrite(wp, "' );\n");
		websWrite(wp, "document.getElementById(\"%s_tls8021xpem\").value = %s_tls8021xpem;\n", prefix, prefix);
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "share.privatekey", NULL);
		websWrite(
			wp,
			"<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xprv\" name=\"%s_tls8021xprv\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
			prefix, prefix);
		websWrite(wp, "var %s_tls8021xprv = fix_cr( '", prefix);
		sprintf(namebuf, "%s_tls8021xprv", prefix);
		tf_webWriteESCNV(wp, namebuf);
		websWrite(wp, "' );\n");
		websWrite(wp, "document.getElementById(\"%s_tls8021xprv\").value = %s_tls8021xprv;\n", prefix, prefix);
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "sec80211x.options", NULL);
		websWrite(
			wp,
			"<textarea cols=\"60\" rows=\"3\" id=\"%s_tls8021xaddopt\" name=\"%s_tls8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
			prefix, prefix);
		websWrite(wp, "var %s_tls8021xaddopt = fix_cr( '", prefix);
		sprintf(namebuf, "%s_tls8021xaddopt", prefix);
		tf_webWriteESCNV(wp, namebuf);
		websWrite(wp, "' );\n");
		websWrite(wp, "document.getElementById(\"%s_tls8021xaddopt\").value = %s_tls8021xaddopt;\n", prefix, prefix);
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</fieldset><br/>\n");
	}
}
#endif
#endif
#endif

void show_wparadius(webs_t wp, char *prefix)
{
	char var[80];
#ifndef HAVE_MADWIFI
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "wpa.algorithms", NULL);
	websWrite(wp, "<select name=\"%s_crypto\">\n", prefix);
	sprintf(var, "%s_crypto", prefix);
	websWrite(wp, "<option value=\"aes\" %s><script type=\"text/javascript\">Capture(wpa.ccmp)</script></option>\n",
		  selmatch(var, "aes", "selected=\"selected\""));
	websWrite(wp, "<option value=\"tkip+aes\" %s><script type=\"text/javascript\">Capture(wpa.tkip_ccmp)</script></option>\n",
		  selmatch(var, "tkip+aes", "selected=\"selected\""));
	websWrite(wp, "<option value=\"tkip\" %s><script type=\"text/javascript\">Capture(wpa.tkip)</script></option>\n",
		  selmatch(var, "tkip", "selected=\"selected\""));
	websWrite(wp, "</select></div>\n");
#endif
#ifdef HAVE_MADWIFI
	show_radius(wp, prefix, 0, 1);
#else
	show_radius(wp, prefix, 0, 0);
#endif
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
		show_caption(wp, "label", "wl_adv.label", NULL);
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"open\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.openn)</script></input>&nbsp;\n",
			wl_authmode, nvram_match(wl_authmode, "open") ? "checked=\"checked\"" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"shared\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.share_key)</script></input>\n",
			wl_authmode, nvram_match(wl_authmode, "shared") ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
	}
#endif
	websWrite(
		wp,
		"<div><div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wep.defkey)</script></div>");
	websWrite(wp, "<input type=\"hidden\" name=\"%s_WEP_key\" />", prefix);
	websWrite(wp, "<input type=\"hidden\" name=\"%s_wep\" value=\"restricted\" />", prefix);
	sprintf(var, "%s_key", prefix);
	nvram_default_get(var, "1");
	fprintf(stderr, "[WEP] default: %s\n", var);
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_key\" %s />1&nbsp;\n", prefix,
		  selmatch(var, "1", "checked=\"checked\""));
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"2\" name=\"%s_key\" %s />2&nbsp;\n", prefix,
		  selmatch(var, "2", "checked=\"checked\""));
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"3\" name=\"%s_key\" %s />3&nbsp;\n", prefix,
		  selmatch(var, "3", "checked=\"checked\""));
	websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" value=\"4\" name=\"%s_key\" %s />4&nbsp;\n", prefix,
		  selmatch(var, "4", "checked=\"checked\""));
	websWrite(wp, "</div>");
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.encrypt)</script></div>");
	sprintf(var, "%s_wep_bit", prefix);
	bit = nvram_safe_get(var);
	cprintf("bit %s\n", bit);
	websWrite(wp, "<select name=\"%s_wep_bit\" size=\"1\" onchange=keyMode(this.form)>", prefix);
	websWrite(wp, "<option value=\"64\" %s ><script type=\"text/javascript\">Capture(wep.opt_64);</script></option>",
		  selmatch(var, "64", "selected=\"selected\""));
	websWrite(wp, "<option value=\"128\" %s ><script type=\"text/javascript\">Capture(wep.opt_128);</script></option>",
		  selmatch(var, "128", "selected=\"selected\""));
	websWrite(
		wp,
		"</select>\n</div>\n<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wep.passphrase)</script></div>\n");
	websWrite(wp, "<input name=%s_passphrase maxlength=\"16\" size=\"20\" value=\"", prefix);
	char p_temp[128];
	char temp[256];
	sprintf(p_temp, "%s", get_wep_value(wp, temp, "passphrase", bit, prefix));
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
	websWrite(
		wp,
		"<input class=\"button\" type=\"button\" value=\"Generate\" onclick=\"generateKey(this.form,\'%s\')\" name=\"wepGenerate\" id=\"wepGenerate\"/>\n</div>",
		prefix);
	websWrite(wp, "<script type=\"text/javascript\">document.getElementById(\'wepGenerate\').value=wep.generate;</script>");
	char *mlen = "10";
	char *mlen2 = "12";
	if (!strcmp(bit, "128")) {
		mlen = "26";
		mlen2 = "30";
	}
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 1</div>\n");
	websWrite(wp,
#ifdef HAVE_BUFFALO
		  "<input name=%s_key1 size=\"%s\" maxlength=\"%s\" value=\"%s\" onblur=\"valid_wep(this, 1)\" /></div>\n",
#else
		  "<input name=%s_key1 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
#endif
		  prefix, mlen2, mlen, nvram_nget("%s_key1", prefix));
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 2</div>\n");
	websWrite(wp,
#ifdef HAVE_BUFFALO
		  "<input name=%s_key2 size=\"%s\" maxlength=\"%s\" value=\"%s\" onblur=\"valid_wep(this, 1)\" /></div>\n",
#else
		  "<input name=%s_key2 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
#endif
		  prefix, mlen2, mlen, nvram_nget("%s_key2", prefix));
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 3</div>\n");
	websWrite(wp,
#ifdef HAVE_BUFFALO
		  "<input name=%s_key3 size=\"%s\" maxlength=\"%s\" value=\"%s\" onblur=\"valid_wep(this, 1)\" /></div>\n",
#else
		  "<input name=%s_key3 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
#endif
		  prefix, mlen2, mlen, nvram_nget("%s_key3", prefix));
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 4</div>\n");
	websWrite(wp,
#ifdef HAVE_BUFFALO
		  "<input name=%s_key4 size=\"%s\" maxlength=\"%s\" value=\"%s\" onblur=\"valid_wep(this, 1)\" /></div>\n",
#else
		  "<input name=%s_key4 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
#endif
		  prefix, mlen2, mlen, nvram_nget("%s_key4", prefix));
	websWrite(wp, "</div>\n");
}

EJ_VISIBLE void ej_show_defwpower(webs_t wp, int argc, char_t **argv)
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

EJ_VISIBLE void ej_get_wds_mac(webs_t wp, int argc, char_t **argv)
{
	int mac = -1, wds_idx = -1, mac_idx = -1;
	char *c, wds_var[32] = "";
	char *interface;
	wds_idx = atoi(argv[0]);
	mac_idx = atoi(argv[1]);
	interface = argv[2];
	if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
		return;
	if (mac_idx < 0 || mac_idx > 5)
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

EJ_VISIBLE void ej_showbridgesettings(webs_t wp, int argc, char_t **argv)
{
	char *interface;
	int mcast;
	interface = argv[0];
	mcast = atoi(argv[1]);
	showbridgesettings(wp, interface, mcast, 0);
}

EJ_VISIBLE void ej_get_wds_ip(webs_t wp, int argc, char_t **argv)
{
	int ip = -1, wds_idx = -1, ip_idx = -1;
	char *c, wds_var[32] = "";
	char *interface;
	wds_idx = atoi(argv[0]);
	ip_idx = atoi(argv[1]);
	interface = argv[2];
	if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
		return;
	if (ip_idx < 0 || ip_idx > 3)
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

EJ_VISIBLE void ej_get_wds_netmask(webs_t wp, int argc, char_t **argv)
{
	int nm = -1, wds_idx = -1, nm_idx = -1;
	char *c, wds_var[32] = "";
	char *interface;
	wds_idx = atoi(argv[0]);
	nm_idx = atoi(argv[1]);
	interface = argv[2];
	if (wds_idx < 1 || wds_idx > 6)
		return;
	if (nm_idx < 0 || nm_idx > 3)
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

EJ_VISIBLE void ej_get_wds_gw(webs_t wp, int argc, char_t **argv)
{
	int gw = -1, wds_idx = -1, gw_idx = -1;
	char *c, wds_var[32] = "";
	char *interface;
	wds_idx = atoi(argv[0]);
	gw_idx = atoi(argv[1]);
	interface = argv[2];
	if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
		return;
	if (gw_idx < 0 || gw_idx > 3)
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

EJ_VISIBLE void ej_get_br1_ip(webs_t wp, int argc, char_t **argv)
{
	int ip = -1, ip_idx = -1;
	char *c;
	char *interface;
	ip_idx = atoi(argv[0]);
	interface = argv[1];
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

EJ_VISIBLE void ej_get_br1_netmask(webs_t wp, int argc, char_t **argv)
{
	int nm = -1, nm_idx = -1;
	char *c;
	char *interface;
	nm_idx = atoi(argv[0]);
	interface = argv[1];
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

#ifndef FSHIFT
#define FSHIFT 16 /* nr of bits of precision */
#endif
#define FIXED_1 (1 << FSHIFT) /* 1.0 as fixed-point */
#define LOAD_INT(x) (unsigned)((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1 - 1)) * 100)

/* copied from busybox */
EJ_VISIBLE void ej_get_uptime(webs_t wp, int argc, char_t **argv)
{
	unsigned updays, uphours, upminutes;
	struct sysinfo info;
	struct tm current_time;
	time_t current_secs;
	time(&current_secs);
	localtime_r(&current_secs, &current_time);
	sysinfo(&info);
	websWrite(wp, " %02u:%02u:%02u up ", current_time.tm_hour, current_time.tm_min, current_time.tm_sec);
	updays = (unsigned)info.uptime / (unsigned)(60 * 60 * 24);
	if (updays)
		websWrite(wp, "%u day%s, ", updays, (updays != 1) ? "s" : "");
	upminutes = (unsigned)info.uptime / (unsigned)60;
	uphours = (upminutes / (unsigned)60) % (unsigned)24;
	upminutes %= 60;
	if (uphours)
		websWrite(wp, "%2u:%02u", uphours, upminutes);
	else
		websWrite(wp, "%u min", upminutes);
#ifdef HAVE_ESPOD
	websWrite(wp, "<br>");
#else
	websWrite(wp, ",  ");
#endif
	websWrite(wp, "load average: %u.%02u, %u.%02u, %u.%02u", LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]),
		  LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]), LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]));
	return;
}

EJ_VISIBLE void ej_get_wan_uptime(webs_t wp, int argc, char_t **argv)
{
	unsigned sys_uptime;
	unsigned uptime;
	int days, minutes;
	FILE *fp, *fp2;
	int fmt = 0;
	char buf[128];
	if (argc)
		fmt = atoi(argv[0]);
	if (nvram_match("wan_proto", "disabled"))
		return;
	if (nvram_match("wan_ipaddr", "0.0.0.0")) {
		if (fmt)
			show_caption_simple(wp, "status_router.notavail");
		else
			websWrite(wp, "%s", live_translate(wp, "status_router.notavail"));
		return;
	}
	if (!(fp = fopen("/tmp/.wanuptime", "r"))) {
		if (fmt)
			show_caption_simple(wp, "status_router.notavail");
		else
			websWrite(wp, "%s", live_translate(wp, "status_router.notavail"));
		return;
	}
	if (!feof(fp) && fscanf(fp, "%u", &uptime) == 1) {
		struct sysinfo info;
		sysinfo(&info);
		sys_uptime = info.uptime;
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

EJ_VISIBLE void ej_get_wdsp2p(webs_t wp, int argc, char_t **argv)
{
	int index = -1, ip[4] = { 0, 0, 0, 0 }, netmask[4] = { 0, 0, 0, 0 };
	char nvramvar[32] = { 0 };
	char *interface;
	index = atoi(argv[0]);
	interface = argv[1];
	char wlwds[32];
	sprintf(wlwds, "%s_wds1_enable", interface);
	if ((nvram_selmatch(wp, "wk_mode", "ospf") || nvram_selmatch(wp, "wk_mode", "ospf router")) &&
	    nvram_selmatch(wp, "expert_mode", "1") && nvram_selmatch(wp, wlwds, "1")) {
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

	if (nvram_matchi(nvramvar, 1)) {
		websWrite(
			wp,
			"<div class=\"setting\">\n"
			"<input type=\"hidden\" name=\"%s_wds%d_ipaddr\" value=\"4\">\n"
			"<div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n"
			"<input size=\"3\" maxlength=\"3\" name=\"%s_wds%d_ipaddr0\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_wds%d_ipaddr1\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_wds%d_ipaddr2\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_wds%d_ipaddr3\" value=\"%d\" onblur=\"valid_range(this,1,254,'IP')\" class=\"num\">\n"
			"</div>\n",
			interface, index, interface, index, ip[0], interface, index, ip[1], interface, index, ip[2], interface,
			index, ip[3]);
		websWrite(
			wp,
			"<div class=\"setting\">\n"
			"<div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n"
			"<input type=\"hidden\" name=\"%s_wds%d_netmask\" value=\"4\">\n"
			"<input name=\"%s_wds%d_netmask0\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>.<input name=\"%s_wds%d_netmask1\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>.<input name=\"%s_wds%d_netmask2\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>.<input name=\"%s_wds%d_netmask3\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>\n"
			"</div>\n",
			interface, index, interface, index, netmask[0], interface, index, netmask[1], interface, index, netmask[2],
			interface, index, netmask[3]);
	}

	return;
}

EJ_VISIBLE void ej_get_clone_wmac(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_RB500
	return 0;
#else

	char *c;
	int mac, which;
	char buf[32];
	which = atoi(argv[0]);
	if (nvram_match("def_whwaddr", "00:00:00:00:00:00")) {
		if (nvram_matchi("port_swap", 1)) {
			if (*(nvram_safe_get("et1macaddr"))) // safe:
			// maybe
			// et1macaddr
			// not there?
			{
				c = strdup(nvram_safe_get("et1macaddr"));
			} else {
				c = strdup(nvram_safe_get("et0macaddr"));
				MAC_ADD(c); // et0macaddr +3
			}
		} else {
			c = &buf[0];
			getSystemMac(c);
		}

		if (c) {
			MAC_ADD(c);
			MAC_ADD(c);
		}

	} else
		c = nvram_safe_get("def_whwaddr");
	if (c) {
		mac = get_single_mac(c, which);
		websWrite(wp, "%02X", mac);
	} else
		websWrite(wp, "00");
	return;
#endif
}

#include "switch.c"
#include "qos.c"
#include "conntrack.c"

EJ_VISIBLE void ej_gethostnamebyip(webs_t wp, int argc, char_t **argv)
{
	char buf[200];
	char *argument = argv[0];
	if (argc == 1) {
		getHostName(buf, argument);
		websWrite(wp, "%s", strcmp(buf, "unknown") ? buf : tran_string(buf, sizeof(buf), "share.unknown"));
	}

	return;
}

#define PROC_DEV "/proc/net/dev"

EJ_VISIBLE void ej_wl_packet_get(webs_t wp, int argc, char_t **argv)
{
	char line[256];
	FILE *fp;
#ifdef HAVE_MADWIFI
	char *ifname = nvram_safe_get("wifi_display");
#elif HAVE_RT2880
	char *ifname = getRADev(nvram_safe_get("wifi_display"));
#else
	char name[32];
	sprintf(name, "%s_ifname", nvram_safe_get("wifi_display"));
	char *ifname = nvram_safe_get(name);
#endif
	struct dev_info {
		unsigned long long int rx_pks;
		unsigned long long int rx_errs;
		unsigned long long int rx_drops;
		unsigned long long int tx_pks;
		unsigned long long int tx_errs;
		unsigned long long int tx_drops;
		unsigned long long int tx_colls;
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
			line[ifl] = 0; /* interface */
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
				       "%*llu %llu %llu %llu %*llu %*llu %*llu %*llu %*llu %llu %llu %llu %*llu %llu %*llu %*llu",
				       &info.rx_pks, &info.rx_errs, &info.rx_drops, &info.tx_pks, &info.tx_errs, &info.tx_drops,
				       &info.tx_colls);
			}
		}
		fclose(fp);
	}

	websWrite(wp, "SWRXgoodPacket=%llu;", info.rx_pks);
	websWrite(wp, "SWRXerrorPacket=%llu;", info.rx_errs + info.rx_drops);
	websWrite(wp, "SWTXgoodPacket=%llu;", info.tx_pks);
	websWrite(wp, "SWTXerrorPacket=%llu;", info.tx_errs + info.tx_drops + info.tx_colls);
	return;
}

/*
 * END Added by Botho 10.May.06 
 */

EJ_VISIBLE void ej_statfs(webs_t wp, int argc, char_t **argv)
{
	struct statfs sizefs;
	if (argc != 2)
		return;
	if ((statfs(argv[0], &sizefs) != 0) || (sizefs.f_type == 0x73717368) || (sizefs.f_type == 0x74717368) ||
	    (sizefs.f_type == 0x68737174))
		bzero(&sizefs, sizeof(sizefs));
	websWrite(wp, "var %s = {\nfree: %llu,\nused: %llu,\nsize: %llu\n};\n", argv[1],
		  ((uint64_t)sizefs.f_bsize * sizefs.f_bfree), ((uint64_t)sizefs.f_bsize * (sizefs.f_blocks - sizefs.f_bfree)),
		  ((uint64_t)sizefs.f_bsize * sizefs.f_blocks));
}

EJ_VISIBLE void ej_getnumfilters(webs_t wp, int argc, char_t **argv)
{
	char filter[32];
	sprintf(filter, "numfilterservice%d", wp->p->filter_id);
	websWrite(wp, "%s", nvram_default_get(filter, "4"));
}

EJ_VISIBLE void ej_show_filters(webs_t wp, int argc, char_t **argv)
{
	char filter[32];
	sprintf(filter, "numfilterservice%d", wp->p->filter_id);
	int numfilters = nvram_default_geti(filter, 4);
	int i;
	for (i = 0; i < numfilters; i++) {
		websWrite(
			wp,
			"<div class=\"setting\">\n" //
			"<select size=\"1\" name=\"blocked_service%d\" onchange=\"onchange_blockedServices(blocked_service%d.selectedIndex, port%d_start, port%d_end)\">\n" //
			"<option value=\"None\" selected=\"selected\">%s</option>\n" //
			"<script type=\"text/javascript\">\n" //
			"//<![CDATA[\n" //
			"write_service_options(servport_name%d);\n" //
			"//]]>\n" //
			"</script>\n" //
			"</select>\n" //
			"<input maxLength=\"5\" size=\"5\" name=\"port%d_start\" class=\"num\" readonly=\"readonly\" /> ~ <input maxLength=\"5\" size=\"5\" name=\"port%d_end\" class=\"num\" readonly=\"readonly\" />\n" //
			"</div>\n",
			i, i, i, i, "", i, i, i); //
	}
}

EJ_VISIBLE void ej_gen_filters(webs_t wp, int argc, char_t **argv)
{
	char filter[32];
	sprintf(filter, "numfilterservice%d", wp->p->filter_id);
	int numfilters = nvram_default_geti(filter, 4);
	int i;
	for (i = 0; i < numfilters; i++) {
		websWrite(wp, "var servport_name%d = \"", i);
		filter_port_services_get(wp, "service", i);
		websWrite(wp, "\";\n");
	}
}

EJ_VISIBLE void ej_statnv(webs_t wp, int argc, char_t **argv)
{
	int space = 0;
	int used = nvram_used(&space);
	websWrite(wp, "%d KiB / %d KiB", used / 1024, space / 1024);
}

#ifdef HAVE_RSTATS
/*
 * 
 * rstats Copyright (C) 2006 Jonathan Zarate
 * 
 * Licensed under GNU GPL v2 or later.
 * 
 */

EJ_VISIBLE void ej_bandwidth(webs_t wp, int argc, char_t **argv)
{
	char *name;
	int sig;
	char *argument = argv[0];
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
		do_file(NULL, name, wp);
		unlink(name);
	}
}
#endif
char *getNetworkLabel(webs_t wp, char *var)
{
	char *l = nvram_nget("%s_label", var);
	snprintf(wp->label, sizeof(wp->label), "%s%s%s", var, strcmp(l, "") ? " - " : "", l);
	return wp->label;
}

#ifdef HAVE_PORTSETUP
#include "portsetup.c"
#endif
#include "macfilter.c"

#ifdef HAVE_DNSCRYPT
EJ_VISIBLE void ej_show_dnscrypt(webs_t wp, int argc, char_t **argv)
{
	char line[512];
	int lines = 0;
	char name[64], fname[128];
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "service.dns_crypt_resolv", NULL);
	websWrite(wp, "<select name=\"dns_crypt_resolver\">\n");
	FILE *fp = fopen("/etc/dnscrypt/dnscrypt-resolvers.csv", "rb");
	if (fp != NULL) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (!lines++) {
				continue;
			}
			int i;
			bzero(fname, sizeof(fname));
			bzero(name, sizeof(name));
			for (i = 0; i < sizeof(name) - 1; i++) {
				if (line[i] == ',')
					break;
				name[i] = line[i];
			}
			int a, cnt = 0, c = 0;
			i++;
			int check = 0;
			if (line[i] == '"')
				check = 1;
			for (a = i; a < i + sizeof(fname) - 1; a++) {
				if (check) {
					if (line[a] == '"') {
						cnt++;
						continue;
					}
					if (cnt == 2)
						break;
				} else {
					if (line[a] == ',') {
						break;
					}
				}
				fname[c++] = line[a];
			}
			websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", name,
				  nvram_match("dns_crypt_resolver", name) ? "selected" : "", fname);
		}
		fclose(fp);
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
}
#endif
EJ_VISIBLE void ej_show_congestion(webs_t wp, int argc, char_t **argv)
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
	show_caption(wp, "label", "management.net_conctrl", NULL);
	websWrite(wp, "<select name=\"tcp_congestion_control\">\n");
	foreach(var, eths, next)
	{
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var,
			  nvram_match("tcp_congestion_control", var) ? "selected" : "", var);
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
}

EJ_VISIBLE void ej_show_ifselect(webs_t wp, int argc, char_t **argv)
{
	char wan_if_buffer[33];
	if (argc < 1)
		return;
	char *ifname = argv[0];
	int showwan = 0;
	if (argc > 1)
		showwan = atoi(argv[1]);

	websWrite(wp, "<select name=\"%s\">\n", ifname);
	int i;
	for (i = 2; i < argc; i++) {
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", argv[i],
			  nvram_match(ifname, argv[i]) ? "selected=\"selected\"" : "", argv[i]);
	}
	char *wanface = safe_get_wan_face(wan_if_buffer);
	if (showwan & 1) {
		if (strcmp(wanface, "br0")) {
			websWrite(wp, "<option value=\"%s\" %s >WAN</option>\n", wanface,
				  nvram_match(ifname, wanface) ? "selected=\"selected\"" : "");
		}
	}
	websWrite(wp, "<option value=\"%s\" %s >LAN</option>\n", nvram_safe_get("lan_ifname"),
		  nvram_match(ifname, nvram_safe_get("lan_ifname")) ? "selected=\"selected\"" : "");
	char *next;
	char var[80];
	char eths[256];
	char eth2[256];
	bzero(eths, 256);
	getIfLists(eths, 256);
	bzero(eth2, 256);
	getIfList(eth2, "ppp");
	strcat(eths, " ");
	strcat(eths, eth2);
	foreach(var, eths, next)
	{
		if (!strcmp(wanface, var))
			continue;
		if (!strcmp(nvram_safe_get("lan_ifname"), var))
			continue;
		if (nvram_nmatch("1", "%s_bridged", var) && !isbridge(var))
			continue;
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var, nvram_match(ifname, var) ? "selected" : "",
			  getNetworkLabel(wp, var));
	}

	websWrite(wp, "</select>\n");
}

EJ_VISIBLE void ej_show_iflist(webs_t wp, int argc, char_t **argv)
{
	char wan_if_buffer[33];
	char *next;
	char var[80];
	char buffer[256];
	bzero(buffer, 256);
	char *prefix = NULL;
	if (argc > 0)
		prefix = argv[0];
	if (!*prefix)
		prefix = NULL;
	getIfListNoPorts(buffer, prefix);
	foreach(var, buffer, next)
	{
		char *wanface = safe_get_wan_face(wan_if_buffer);
		if (strcmp(wanface, "br0") && nvram_match(wanface, var)) {
			websWrite(wp, "<option value=\"%s\" >WAN</option>\n", var);
			continue;
		}
		if (nvram_match("lan_ifname", var)) {
			websWrite(wp, "<option value=\"%s\">LAN &amp; WLAN</option>\n", var);
			continue;
		}
		websWrite(wp, "<option value=\"%s\" >%s</option>\n", var, getNetworkLabel(wp, var));
	}
}

#ifdef HAVE_RFLOW
EJ_VISIBLE void ej_show_rflowif(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "<option value=\"%s\" %s >LAN &amp; WLAN</option>\n", nvram_safe_get("lan_ifname"),
		  nvram_match("rflow_if", nvram_safe_get("lan_ifname")) ? "selected=\"selected\"" : "");
	char *lanifs = nvram_safe_get("lan_ifnames");
	char *next;
	char var[80];
	foreach(var, lanifs, next)
	{
		if (nvram_match("wan_ifname", var))
			continue;
		if (!ifexists(var))
			continue;
		websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", var,
			  nvram_match("rflow_if", var) ? "selected=\"selected\"" : "", getNetworkLabel(wp, var));
	}
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	int cnt = get_wl_instances();
	int c;
	for (c = 0; c < cnt; c++) {
		sprintf(var, "wl%d_ifname", c);
		websWrite(wp, "<option value=\"%s\" %s >WLAN%d</option>\n", nvram_safe_get(var),
			  nvram_match("rflow_if", nvram_safe_get(var)) ? "selected=\"selected\"" : "", c);
	}
#endif

	char *wanif = nvram_safe_get("wan_ifname");
	if (*(wanif)) {
		websWrite(wp, "<option value=\"%s\" %s >WAN</option>\n", wanif,
			  nvram_match("rflow_if", wanif) ? "selected=\"selected\"" : "");
	}
}
#endif
#ifdef CONFIG_STATUS_GPIO
EJ_VISIBLE void ej_show_status_gpio_output(webs_t wp, int argc, char_t **argv)
{
	char *var, *next;
	char nvgpio[32];
	char *value = websGetVar(wp, "action", "");
	char *gpios = nvram_safe_get("gpio_outputs");
	var = (char *)malloc(strlen(gpios) + 1);
	if (var != NULL) {
		if (gpios != NULL) {
			foreach(var, gpios, next)
			{
				sprintf(nvgpio, "gpio%s", var);
				nvgpio = nvram_nget("gpio_%s", var);
				if (!nvgpio)
					nvram_seti(nvgpio, 0);
				websWrite(wp, "<input type=\"checkbox\" name=\"%s\" value=\"1\" %s />\n", nvgpio,
					  nvram_matchi(nvgpio, 1) ? "checked=\"checked\"" : "");
				websWrite(wp, "<input type=\"checkbox\" name=\"%s\" value=\"0\" %s />\n", nvgpio,
					  nvram_matchi(nvgpio, 0) ? "checked=\"checked\"" : "");
			}
		}
		debug_free(var);
	}
}

#endif
