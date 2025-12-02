/*
 * bandwidth.c
 *
 * Copyright (C) 2005 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <ddnvram.h>
#include <glob.h>

static void show_bwif_row(webs_t wp, char ifname[4][32], char name[4][128])
{
	int i;
	websWrite(wp, "<table cellspacing=\"4\" summary=\"bandwidth\" id=\"bandwidth_table\" class=\"table\"><thead><tr>\n");
	for (i = 0; i < 4; i++) {
		websWrite(wp, "<th width=\"25%%\">%s</th>\n", name[i][0] ? name[i] : "");
	}
	websWrite(wp, "</tr></thead>\n");
	websWrite(wp, "<tbody>\n");
	websWrite(wp, "<tr>\n");

	char buf[128];
	for (i = 0; i < 4; i++) {
		websWrite(wp, "<td>\n");
		if (ifname[i][0]) {
			websWrite(
				wp,
				"<iframe title=\"\" src=\"/graph_if.svg?%s\" width=\"100%%\" height=\"25%%\" frameborder=\"0\" type=\"image/svg+xml\">\n",
				ifname[i]);
		}
		websWrite(wp, "</iframe>\n");
		websWrite(wp, "</td>\n");
	}
	websWrite(wp, "</tbody>\n");
	websWrite(wp, "</table>\n");
}
struct bwcontext {
	char ifname[4][32];
	char name[4][128];
	int count;
};

static void show_bwif(webs_t wp, struct bwcontext *ctx, char *ifname, char *name)
{
	if (!ctx->count) {
		memset(ctx->ifname, 0, sizeof(ctx->ifname));
		memset(ctx->name, 0, sizeof(ctx->name));
	}
	strlcpy(ctx->ifname[ctx->count], ifname, 32);
	strlcpy(ctx->name[ctx->count], name, 128);
	ctx->count++;
	if (ctx->count == 4) {
		ctx->count = 0;
		show_bwif_row(wp, ctx->ifname, ctx->name);
	}
}

void EJ_VISIBLE ej_show_bandwidth(webs_t wp, int argc, char_t **argv)
{
	char wan_if_buffer[33];
	char name[180];
	const char *next, *bnext;
	char var[80];
	char eths[512];
	char eths2[512];
	char buf[128];
	char bword[256];
	glob_t globbuf;
	char *globstring;
	int globresult;
	int c;
	struct bwcontext ctx;
	memset(&ctx, 0, sizeof(ctx));
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend>%s</legend>\n", tran_string(buf, sizeof(buf), "status_band.h2"));
	show_bwif(wp, &ctx, nvram_safe_get("lan_ifname"), "LAN");
	getIfLists(eths, sizeof(eths));
	//add ppp interfacs
	getIfList(eths2, sizeof(eths2), "ppp tun oet");
	strcat(eths, " ");
	strcat(eths, eths2);
#ifndef HAVE_MADWIFI
	int cnt = get_wl_instances();
#endif
	foreach(var, eths, next) {
		if (!nvram_match("wan_proto", "disabled")) {
			if (!strcmp(safe_get_wan_face(wan_if_buffer), var))
				continue;
		}
		if (!strcmp("etherip0", var))
			continue;
		if (!strncmp("wl", var, 2))
			continue;
		if (!strcmp(nvram_safe_get("lan_ifname"), var))
			continue;
#ifndef HAVE_MADWIFI
		for (c = 0; c < cnt; c++) {
			if (!strcmp(get_wl_instance_name(c), var))
				goto skip;
		}
#endif
		struct portstatus status;
		int r = getLanPortStatus(var, &status);
		if (!r && status.link) {
			if (isbridge(var)) {
				snprintf(name, sizeof(name), "BRIDGE (%s)", getNetworkLabel(wp, var));
			} else
				snprintf(name, sizeof(name), "LAN (%s)", getNetworkLabel(wp, var));

			show_bwif(wp, &ctx, var, name);
		}
skip:;
	}
	if (!nvram_match("wan_proto", "disabled")) {
		const char *wanport = safe_get_wan_face(wan_if_buffer);
		char *sta = getSTA();
		if (sta) {
			snprintf(name, sizeof(name), "%s WAN (%s)", tran_string(buf, sizeof(buf), "share.wireless"),
				 getNetworkLabel(wp, sta));
		}
		else
			snprintf(name, sizeof(name), "WAN (%s)",
				 getNetworkLabel(wp, wanport));

		show_bwif(wp, &ctx, wanport, name);

		if (nvram_matchi("dtag_vlan8", 1) && nvram_matchi("dtag_bng", 0)) {
			if (getRouterBrand() == ROUTER_WRT600N || getRouterBrand() == ROUTER_WRT610N)
				show_bwif(wp, &ctx, "eth2.0008", "IPTV");
			else
				show_bwif(wp, &ctx, "eth0.0008", "IPTV");
		}
	}
#ifdef HAVE_MADWIFI
	c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char dev[32];
		sprintf(dev, "wlan%d", i);
		if (nvram_nmatch("disabled", "%s_net_mode", dev))
			continue;
		snprintf(name, sizeof(name), "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"), getNetworkLabel(wp, dev));
		show_bwif(wp, &ctx, dev, name);
		char *vifs = nvram_nget("%s_vifs", dev);

		if (vifs == NULL)
			continue;
		foreach(var, vifs, next) {
			if (nvram_nmatch("disabled", "%s_mode", var))
				continue;
			snprintf(name, sizeof(name), "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"),
				 getNetworkLabel(wp, var));
			show_bwif(wp, &ctx, var, name);
		}
		int s;

		for (s = 1; s <= 10; s++) {
			char *wdsdev;

			wdsdev = nvram_nget("%s_wds%d_if", dev, s);
			if (*(wdsdev) == 0)
				continue;
			if (nvram_nmatch("0", "%s_wds%d_enable", dev, s))
				continue;
			snprintf(name, sizeof(name), "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"),
				 getNetworkLabel(wp, wdsdev));
			show_bwif(wp, &ctx, wdsdev, name);
		}

		if (is_mac80211(dev)) {
			asprintf(&globstring, "/sys/class/ieee80211/phy*/device/net/%s.sta*", dev);
			globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
			int awdscount;
			char *lastname = NULL;
			for (awdscount = 0; awdscount < globbuf.gl_pathc; awdscount++) {
				char *ifname;
				ifname = strrchr(globbuf.gl_pathv[awdscount], '/');
				if (!ifname) {
					debug_free(globstring);
					continue;
				}
				sprintf(name, "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"), ifname + 1);
				if (lastname && !strcmp(lastname, ifname + 1))
					continue;
				show_bwif(wp, &ctx, ifname + 1, name);
				lastname = ifname + 1;
			}
			globfree(&globbuf);
			debug_free(globstring);
		}
	}

#else
	for (c = 0; c < cnt; c++) {
		if (nvram_nmatch("disabled", "wl%d_net_mode", c))
			continue;
		snprintf(name, sizeof(name), "%s (wl%d)", tran_string(buf, sizeof(buf), "share.wireless"), c);
		show_bwif(wp, &ctx, get_wl_instance_name(c), name);
		char *vifs = nvram_nget("wl%d_vifs", c);
		if (vifs == NULL)
			continue;
		foreach(var, vifs, next) {
			if (nvram_nmatch("disabled", "%s_mode", var))
				continue;
			snprintf(name, sizeof(name), "%s (%s)", tran_string(buf, sizeof(buf), "share.wireless"),
				 getNetworkLabel(wp, var));
			show_bwif(wp, &ctx, var, name);
		}
	}
#endif
#ifdef HAVE_WAVESAT

	sprintf(name, "%s", tran_string(buf, sizeof(buf), "wl_wimax.titl"));
	show_bwif(wp, &ctx, "ofdm", name);
#endif
	if (ctx.count > 0) {
		show_bwif_row(wp, ctx.ifname, ctx.name);
	}
	websWrite(wp, "</fieldset>\n");
}
