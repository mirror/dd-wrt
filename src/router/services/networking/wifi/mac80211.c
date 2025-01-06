/*
 * madwifi_ath9k.c 
 * Copyright (C) 2010 Christian Scheele <chris@dd-wrt.com>
 * Copyright (C) 2010 - 2024 Sebastian Gottschall <brainslayer@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifdef HAVE_ATH9K
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <utils.h>
#include <wlutils.h>
#include <shutils.h>
#include <unistd.h>
#include <glob.h>

#include <services.h>

#include "unl.h"
#include <nl80211.h>

#define ENTER syslog(LOG_DEBUG, "mac80211: start %s", __func__)
#define LEAVE syslog(LOG_DEBUG, "mac80211: leave %s", __func__)
#define MAC80211DEBUG() \
	do {            \
	} while (0)

#ifndef MAC80211DEBUG
#define MAC80211DEBUG() syslog(LOG_DEBUG, "mac80211: %s:%d", __func__, __LINE__)
#endif

static int cur_freq;
static int cur_freq2;
static int cur_channeloffset;
static int cur_iht;
static char *cur_caps;
void check_cryptomod(const char *prefix);

void setupHostAP_ath9k(char *maininterface, int isfirst, int vapid, int aoss);
static void setupSupplicant_ath9k(const char *prefix, char *ssidoverride, int isadhoc);
void setupHostAP_generic_ath9k(const char *prefix, FILE *fp, int isrepeater, int aoss);

#ifndef HAVE_SUPERCHANNEL
int inline issuperchannel(void)
{
	return 0;
}
#else
int issuperchannel(void);
#endif

static int cansuperchannel(const char *prefix)
{
	return (issuperchannel() && nvram_nmatch("0", "%s_regulatory", prefix) && nvram_nmatch("ddwrt", "%s_fwtype", prefix));
}

static const char *gethtmode(const char *prefix)
{
	ENTER;
	char *netmode = nvram_nget("%s_net_mode", prefix);
	char *akm = nvram_nget("%s_akm", prefix);
	char *ht = "HT20";
	int iht;
	int isath5k = is_ath5k(prefix);
	char bw[32];
	sprintf(bw, "%s_channelbw", prefix);
	int usebw = 20;
	if (nvram_matchi(bw, 40))
		usebw = 40;
	if (nvram_matchi(bw, 5))
		usebw = 5;
	if (nvram_matchi(bw, 10))
		usebw = 10;
	if (nvram_matchi(bw, 2040))
		usebw = 40;
	if (nvram_matchi(bw, 80))
		usebw = 80;
	if (nvram_matchi(bw, 160))
		usebw = 160;
	if (nvram_match(bw, "80+80"))
		usebw = 8080;

	if ((!strcmp(netmode, "ng-only") || //
	     !strcmp(netmode, "na-only") || //
	     !strcmp(netmode, "n2-only") || //
	     !strcmp(netmode, "n5-only") || //
	     !strcmp(netmode, "axg-only") || //
	     !strcmp(netmode, "ac-only") || //
	     !strcmp(netmode, "acn-mixed") || //
	     !strcmp(netmode, "ax-only") || //
	     !strcmp(netmode, "xacn-mixed") || //
	     !strcmp(netmode, "mixed")) &&
	    strcmp(akm, "wep")) {
		char sb[32];
		sprintf(sb, "%s_nctrlsb", prefix);
		switch (usebw) {
		case 40:
			if (nvram_default_match(sb, "ull", "luu") || nvram_match(sb, "upper"))
				ht = "HT40+";
			else
				ht = "HT40-";
			break;
		case 80:
			ht = "80MHz";
			break;
		case 8080:
			ht = "80+80MHz";
			break;
		case 160:
			ht = "160MHz";
			break;
		case 20:
		case 10:
		case 5:
		default:
			ht = "HT20";
			break;
		}
	} else {
		ht = "NOHT";
	}
	LEAVE;
	return ht;
}

static void setRTS(char *use)
{
	ENTER;
	char rts[32];

	sprintf(rts, "%s_protmode", use);
	nvram_default_get(rts, "None");

	sprintf(rts, "%s_rts", use);
	nvram_default_get(rts, "0");

	sprintf(rts, "%s_rtsvalue", use);
	nvram_default_get(rts, "500");

	if (nvram_nmatch("1", "%s_rts", use)) {
		eval("iwconfig", use, "rts", nvram_nget("%s_rtsvalue", use));
	} else {
		eval("iwconfig", use, "rts", "off");
	}
	LEAVE;
}

void delete_ath9k_devices(char *physical_iface)
{
	ENTER;
	glob_t globbuf;
	char globstring[1024];
	char tmp[256];
	int globresult;
	if (physical_iface)
		sprintf(globstring, "/sys/class/ieee80211/%s/device/net/*", physical_iface);
	else
		sprintf(globstring, "/sys/class/ieee80211/phy*/device/net/*");
	globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
	int i;
	for (i = 0; i < globbuf.gl_pathc; i++) {
		char *ifname;
		ifname = strrchr(globbuf.gl_pathv[i], '/');
		if (!ifname)
			continue;
		char dev[32];
		sprintf(dev, "%s", ifname + 1);
		if (has_ad(dev)) {
			sysprintf("echo 0 > /sys/kernel/debug/ieee80211/phy2/wil6210/led_cfg");
			sysprintf("echo 10000 0 10000 0 10000 0 > /sys/kernel/debug/ieee80211/phy2/wil6210/led_blink_time");
			br_del_interface(getBridge("wlan2", tmp), dev);
		} else
			br_del_interface(getBridge(dev, tmp), dev);
		eval("ifconfig", dev, "down");
		eval("iw", dev, "del");
	}
	LEAVE;
}

void deconfigure_single_ath9k(int count)
{
	ENTER;
	int idx = get_ath9k_phy_idx(count);
	fprintf(stderr, "ath9k deconfigure_single: phy%d wlan%d\n", idx, count);
	char wif[10];
	sprintf(wif, "phy%d", idx);
	sysprintf("rm -f /tmp/wlan%d_hostapd.conf", idx);
	sysprintf("rm -f /tmp/wlan%d_wpa_supplicant.conf", idx);
	delete_ath9k_devices(wif);
	LEAVE;
}

static void mesh_param(char *dev, char *name, char *def)
{
	char mparam[64];
	sprintf(mparam, "%s_%s", dev, name);
	eval("iw", "dev", dev, "set", "mesh_param", name, nvram_default_get(mparam, def));
}

void set_mesh_params(char *dev)
{
	ENTER;
	mesh_param(dev, "mesh_fwding", "1");
	mesh_param(dev, "mesh_retry_timeout", "100");
	mesh_param(dev, "mesh_confirm_timeout", "100");
	mesh_param(dev, "mesh_holding_timeout", "100");
	mesh_param(dev, "mesh_max_peer_links", "3");
	mesh_param(dev, "mesh_max_retries", "3");
	mesh_param(dev, "mesh_ttl", "5");
	mesh_param(dev, "mesh_element_ttl", "3");
	mesh_param(dev, "mesh_auto_open_plinks", "1");
	mesh_param(dev, "mesh_hwmp_max_preq_retries", "2");
	mesh_param(dev, "mesh_path_refresh_time", "1000");
	mesh_param(dev, "mesh_min_discovery_timeout", "100");
	mesh_param(dev, "mesh_hwmp_active_path_timeout", "5000");
	mesh_param(dev, "mesh_hwmp_preq_min_interval", "10");
	mesh_param(dev, "mesh_hwmp_net_diameter_traversal_time", "50");
	mesh_param(dev, "mesh_hwmp_rootmode", "0");
	mesh_param(dev, "mesh_hwmp_rann_interval", "5000");
	mesh_param(dev, "mesh_gate_announcements", "0");
	mesh_param(dev, "mesh_sync_offset_max_neighor", "10");
	mesh_param(dev, "mesh_rssi_threshold", "-80");
	mesh_param(dev, "mesh_hwmp_active_path_to_root_timeout", "6000");
	mesh_param(dev, "mesh_hwmp_confirmation_interval", "5000");
	mesh_param(dev, "mesh_power_mode", "active");
	mesh_param(dev, "mesh_awake_window", "10");
	mesh_param(dev, "mesh_plink_timeout", "0");
	mesh_param(dev, "mesh_nolearn", "0");
	mesh_param(dev, "mesh_connected_to_gate", "0");
	mesh_param(dev, "mesh_connected_to_as", "0");
	LEAVE;
}

void mesh_params_main(int argc, char *argv[])
{
	ENTER;
	int c = getdevicecount();
	char dev[32];
	char var[32], *next;
	int i;
	for (i = 0; i < c; i++) {
		sprintf(dev, "wlan%d", i);
		if (nvram_nmatch("disabled", "%s_net_mode", dev))
			continue;
		if (nvram_nmatch("disabled", "%s_mode", dev))
			continue;
		sleep(3);
		if (nvram_nmatch("mesh", "%s_mode", dev))
			set_mesh_params(dev);
		char vifs[32];
		sprintf(vifs, "wlan%d_vifs", i);
		char *vaps = nvram_safe_get(vifs);
		foreach(var, vaps, next)
		{
			if (nvram_nmatch("disabled", "%s_mode", var))
				continue;
			if (nvram_nmatch("mesh", "%s_mode", var))
				set_mesh_params(var);
		}
	}
	LEAVE;
}

static void setchanbw(char *wif, char *driver, int bw)
{
	sysprintf("echo %d > /sys/kernel/debug/ieee80211/%s/%s/chanbw", bw, wif, driver);
}

int iscpe(void);
void configure_single_ath9k(int count)
{
	char *next;
	char var[80];
	char mode[80];
	int cnt = 0;
	char dev[10];
	char wif[10];
	int phy_idx = get_ath9k_phy_idx(count);
	char channel[32];
	char ssid[32];
	char net[32];
	char wifivifs[32];
	char broadcast[32];
	char sens[32];
	char basedev[32];
	char diversity[32];
	char athmac[32];
	char wl_poll[32];
	char rxantenna[32];
	char txantenna[32];
	static int vapcount = 0;
	int isath5k = 0;
	int isath10k = 0;
	int ismt7615 = 0;
	int ismt7915 = 0;
	int ismt7921 = 0;
	char *apm;
	char isolate[32];
	char primary[32] = { 0 };
	char regdomain[32];
	char *country;
	int isadhoc = 0;
	ENTER;
	sprintf(dev, "wlan%d", count);
	isath5k = is_ath5k(dev);
	isath10k = is_ath10k(dev);
	ismt7615 = is_mt7615(dev);
	ismt7915 = is_mt7915(dev);
	ismt7921 = is_mt7921(dev);
	// sprintf(regdomain, "%s_regdomain", dev);
	// country = nvram_default_get(regdomain, "US");
	// sysprintf("iw reg set %s", getIsoName(country));
	// // sleep(3);
	if (count == 0)
		vapcount = 0;
	sprintf(wif, "phy%d", phy_idx);
	sprintf(wifivifs, "wlan%d_vifs", count);
	fprintf(stderr, "ath9k configure_single: phy%d wlan%d\n", phy_idx, count);
	sprintf(channel, "wlan%d_channel", count);
	sprintf(sens, "wlan%d_distance", count);
	sprintf(diversity, "wlan%d_diversity", count);
	sprintf(athmac, "wlan%d_hwaddr", count);
	sprintf(rxantenna, "wlan%d_rxantenna", count);
	sprintf(txantenna, "wlan%d_txantenna", count);
	// create base device
	cprintf("configure base interface %d / %s\n", count, dev);
	sprintf(net, "%s_net_mode", dev);
	char *netmode = nvram_default_get(net, "mixed");
	if (!strncmp(dev, "wlan0", 4)) {
		led_control(LED_WLAN0, LED_OFF);
	}
	if (!strncmp(dev, "wlan1", 4)) {
		led_control(LED_WLAN1, LED_OFF);
	}
	if (!strcmp(netmode, "disabled")) {
		return;
	}
	if (!strncmp(dev, "wlan0", 4)) {
		led_control(LED_WLAN0, LED_ON);
	}
	if (!strncmp(dev, "wlan1", 4)) {
		led_control(LED_WLAN1, LED_ON);
	}
	MAC80211DEBUG();
	if (nvram_nmatch("1", "%s_turbo_qam", dev)) {
		sysprintf("echo 1 > /sys/kernel/debug/ieee80211/%s/ath10k/turboqam", wif);
		sysprintf("echo 1 > /sys/kernel/debug/ieee80211/%s/mt76/turboqam", wif);
		sysprintf("echo 1 > /sys/kernel/debug/ieee80211/%s/brcmfmac/turboqam", wif);
		sysprintf("echo 1 > /sys/kernel/debug/ieee80211/%s/turboqam", wif);
	} else {
		sysprintf("echo 0 > /sys/kernel/debug/ieee80211/%s/ath10k/turboqam", wif);
		sysprintf("echo 0 > /sys/kernel/debug/ieee80211/%s/mt76/turboqam", wif);
		sysprintf("echo 0 > /sys/kernel/debug/ieee80211/%s/turboqam", wif);
		sysprintf("echo 0 > /sys/kernel/debug/ieee80211/%s/brcmfmac/turboqam", wif);
	}
	if (isath10k && has_fwswitch(dev)) {
		char fwtype[32];
		char regulatory[32];
		char fwtype_use[32];
		sprintf(fwtype, "%s_fwtype", dev);
		sprintf(fwtype_use, "%s_fwtype_use", dev);
		sprintf(regulatory, "%s_regulatory", dev);
		if (nvram_match(regulatory, "0")) {
			nvram_set(fwtype, "ddwrt");
		}
		if (!nvram_default_match(fwtype, nvram_default_get(fwtype_use, "ddwrt"), "ddwrt")) {
			nvram_set(fwtype_use, nvram_safe_get(fwtype));
			//                      nvram_set(dualband_use, nvram_safe_get(dualband));
			//                      sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath10k/dualband", nvram_safe_get(dualband), wif);
			if (nvram_match(fwtype, "vanilla"))
				sysprintf("echo vanilla > /sys/kernel/debug/ieee80211/%s/ath10k/fw_post", wif);
			else
				sysprintf("echo > /sys/kernel/debug/ieee80211/%s/ath10k/fw_post", wif);
			sysprintf("echo fw-reload > /sys/kernel/debug/ieee80211/%s/ath10k/simulate_fw_crash", wif);
		}
	}
	MAC80211DEBUG();
#ifdef HAVE_REGISTER
	int cpeonly = iscpe();
#else
	int cpeonly = 0;
#endif
	int maxrxchain = mac80211_get_avail_rx_antenna(dev);
	int maxtxchain = mac80211_get_avail_tx_antenna(dev);
	int txchain;
	int rxchain;
	if (!*(nvram_safe_get(rxantenna)) || !*(nvram_safe_get(txantenna))) {
		txchain = maxtxchain;
		rxchain = maxrxchain;
	} else {
		txchain = nvram_geti(txantenna);
		rxchain = nvram_geti(rxantenna);
	}
	int reset = 0;
	if (txchain > maxtxchain) {
		reset = 1;
		txchain = maxtxchain;
	}
	if (rxchain > maxrxchain) {
		reset = 1;
		rxchain = maxrxchain;
	}

	char rxdefstr[32];
	char txdefstr[32];
	sprintf(txdefstr, "%d", rxchain);
	sprintf(rxdefstr, "%d", txchain);
	if (reset) {
		nvram_set(txantenna, txdefstr);
		nvram_set(rxantenna, rxdefstr);
	} else {
		nvram_default_get(txantenna, txdefstr);
		nvram_default_get(rxantenna, rxdefstr);
	}
	if (!has_ad(dev))
		mac80211_set_antennas(dev, txchain, rxchain);

	char wl[32];
	sprintf(wl, "wlan%d_mode", count);
	apm = nvram_default_get(wl, "ap");

	if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap") || !strcmp(apm, "apup") || !strcmp(apm, "sta") || !strcmp(apm, "wet")) {
		eval("iw", wif, "interface", "add", dev, "type", "managed");
		strcpy(primary, dev);
	} else if (!strcmp(apm, "wdssta")) {
		eval("iw", wif, "interface", "add", dev, "type", "managed", "4addr", "on", "mtikwds", "off");
		strcpy(primary, dev);
	} else if (!strcmp(apm, "wdssta_mtik")) {
		eval("iw", wif, "interface", "add", dev, "type", "managed", "4addr", "on", "mtikwds", "on");
		strcpy(primary, dev);
	} else if (!strcmp(apm, "mesh")) {
		char akm[32];
		sprintf(akm, "%s_akm", dev);
		nvram_default_get(akm, "disabled");
		int iht, channeloffset;
		get_channeloffset(dev, &iht, &channeloffset);
		char farg[32];
		char *farg2 = "5775";
		char *freq = nvram_nget("%s_channel", dev);
		sprintf(farg, "%d", atoi(freq) + (channeloffset * 5));

		const char *htmode = gethtmode(dev);
		//todo 80+80 center2_freq

		if (nvhas(akm, "psk") || nvhas(akm, "psk2") || nvhas(akm, "psk3")) {
			eval("iw", wif, "interface", "add", dev, "type", "mp");
			if (!strcmp(htmode, "NOHT") || !strncmp(htmode, "HT", 2))
				eval("iw", "dev", dev, "set", "freq", freq, htmode);
			else if (!strcmp(htmode, "80+80"))
				eval("iw", "dev", dev, "set", "freq", freq, htmode, farg, farg2);
			else
				eval("iw", "dev", dev, "set", "freq", freq, htmode, farg);
		} else {
			eval("iw", wif, "interface", "add", dev, "type", "mp", "mesh_id", nvram_nget("%s_ssid", dev));
			if (!strcmp(htmode, "NOHT") || !strncmp(htmode, "HT", 2))
				eval("iw", "dev", dev, "set", "freq", freq, htmode);
			else if (!strcmp(htmode, "80+80"))
				eval("iw", "dev", dev, "set", "freq", freq, htmode, farg, farg2);
			else
				eval("iw", "dev", dev, "set", "freq", freq, htmode, farg);
		}

		strcpy(primary, dev);
	} else {
		char akm[32];
		sprintf(akm, "%s_akm", dev);
		eval("iw", wif, "interface", "add", dev, "type", "ibss");
		isadhoc = 1;
		//              if (nvhas(akm, "psk") || nvhas(akm, "psk2") || nvhas(akm, "psk3")) {
		//                      // setup and join does wpa_supplicant
		//              } else
		{
			char *freq = nvram_nget("%s_channel", dev);
			eval("ifconfig", dev, "up");
			const char *htmode = gethtmode(dev);
			int iht, channeloffset;
			get_channeloffset(dev, &iht, &channeloffset);
			char farg[32];
			//todo 80+80 center2_freq
			char *farg2 = "5775";
			sprintf(farg, "%d", atoi(freq) + (channeloffset * 5));
			if (!strcmp(htmode, "NOHT") || !strncmp(htmode, "HT", 2))
				eval("iw", "dev", dev, "ibss", "join", nvram_nget("%s_ssid", dev), freq, htmode);
			else if (!strcmp(htmode, "80+80"))
				eval("iw", "dev", dev, "ibss", "join", nvram_nget("%s_ssid", dev), freq, htmode, farg, farg2);
			else
				eval("iw", "dev", dev, "ibss", "join", nvram_nget("%s_ssid", dev), freq, htmode, farg);
			cprintf("handle ibss join");
		}
	}
#ifdef HAVE_IPQ6018
	eval("tc", "qdisc", "replace", "dev", dev, "root", "noqueue");
#endif

	MAC80211DEBUG();
	char macaddr[32];
	// interface is created at this point, so that should work
#if defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_IPQ6018)
	int board = getRouterBrand();
	if (board == ROUTER_ASROCK_G10) {
		getMacAddr(dev, macaddr, sizeof(macaddr));
	} else if (board == ROUTER_ASUS_AC58U || board == ROUTER_LINKSYS_EA8300) {
		getMacAddr(dev, macaddr, sizeof(macaddr));
	} else {
		getWirelessMac(macaddr, count);
		set_hwaddr(dev, macaddr);
	}
#else
	getMacAddr(dev, macaddr, sizeof(macaddr));
#endif
	if (!*nvram_safe_get(athmac))
		nvram_set(athmac, macaddr);
	int distance = nvram_default_geti(sens, 500); // to meter
	char dist[32];
	if (distance > 0)
		sprintf(dist, "%d", distance);
	else
		sprintf(dist, "auto");
	eval("iw", "phy", wif, "set", "distance", dist);
#ifdef HAVE_ATH10K
//      if (is_ath10k(dev) && !is_mvebu(dev)) { // evil hack for QCA
//              set_ath10kdistance(dev, distance);
//      }
#endif
	//      experimental frame compression for internal testing only right now

	// das scheint noch aerger zu machen
	eval("iw", "dev", dev, "set", "power_save", "off");
	setRTS(dev);

	cprintf("done()\n");

	MAC80211DEBUG();
	cprintf("setup encryption");
	// setup encryption
	int isfirst = 1;
	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wdssta_mtik") && strcmp(apm, "wet") &&
	    strcmp(apm, "infra") && strcmp(apm, "mesh") && strcmp(apm, "tdma")) {
		setupHostAP_ath9k(dev, isfirst, 0, 0);
		isfirst = 0;
	} else {
		char *clonename = "def_whwaddr";
		if (!strcmp(apm, "sta"))
			clonename = "def_hwaddr";
		if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch(clonename, "00:00:00:00:00:00") &&
		    nvram_invmatch(clonename, "")) {
			set_hwaddr(dev, nvram_safe_get(clonename));
		}
		//              char akm[32];
		//              sprintf(akm, "%s_akm", dev);
		//              if (strcmp(apm, "infra") || nvhas(akm, "psk") || nvhas(akm, "psk2") || nvhas(akm, "psk3"))
		setupSupplicant_ath9k(dev, NULL, isadhoc);
	}
	char *vifs = nvram_safe_get(wifivifs);
	int countvaps = 1;
	foreach(var, vifs, next)
	{
		countvaps++;
	}
	MAC80211DEBUG();
	if (countvaps < 4)
		countvaps = 4;
	if (countvaps > vapcount)
		vapcount = countvaps;
	int counter = 1;
	char compr[32];
	if (*vifs) {
		foreach(var, vifs, next)
		{
			if (nvram_nmatch("disabled", "%s_mode", var)) {
				counter++;
				continue;
			}
			// create the first main hostapd interface when this is repeater mode
			if (!nvram_nmatch("mesh", "%s_mode", var)) {
				if (isfirst)
					eval("iw", wif, "interface", "add", var, "type", "managed");
				setupHostAP_ath9k(dev, isfirst, counter, 0);
			} else {
				char akm[32];
				sprintf(akm, "%s_akm", var);
				if (nvhas(akm, "psk") || nvhas(akm, "psk2") || nvhas(akm, "psk3"))
					eval("iw", wif, "interface", "add", var, "type", "mp", wif, var);
				else
					eval("iw", wif, "interface", "add", var, "type", "mp", "mesh_id",
					     nvram_nget("%s_ssid", var));
				setupSupplicant_ath9k(var, NULL, 0);
			}
#ifdef HAVE_IPQ6018
			eval("tc", "qdisc", "replace", "dev", var, "root", "noqueue");
#endif
			sprintf(compr, "%s_fc_th", var);
			char *threshold = nvram_default_get(compr,
							    "512"); // minimum framesize frequired for compression
			sprintf(compr, "%s_fc", var);

			if (nvram_default_matchi(compr, 1, 0)) {
				eval("iw", "dev", var, "set", "compr", "lzo", threshold);
			} else if (nvram_default_matchi(compr, 2, 0)) {
				eval("iw", "dev", var, "set", "compr", "lzma", threshold);
			} else if (nvram_default_matchi(compr, 3, 0)) {
				eval("iw", "dev", var, "set", "compr", "lz4", threshold);
			} else if (nvram_default_matchi(compr, 4, 0)) {
				eval("iw", "dev", var, "set", "compr", "zstd", threshold);
			} else {
				eval("iw", "dev", var, "set", "compr", "off");
			}
			setRTS(var);

			isfirst = 0;
			counter++;
		}
	}
	if (has_ad(dev)) {
		sysprintf("echo 0 > /sys/kernel/debug/ieee80211/phy2/wil6210/led_polarity");
		sysprintf("echo 1 > /sys/kernel/debug/ieee80211/phy2/wil6210/led_cfg");
		sysprintf("echo 10000 0 200 200 100 100 > /sys/kernel/debug/ieee80211/phy2/wil6210/led_blink_time");
	}
	sprintf(compr, "%s_fc_th", dev);
	char *threshold = nvram_default_get(compr, "512"); // minimum framesize frequired for compression
	sprintf(compr, "%s_fc", dev);
	MAC80211DEBUG();

	if (nvram_default_matchi(compr, 1, 0)) {
		eval("iw", "dev", dev, "set", "compr", "lzo", threshold);
	} else if (nvram_default_matchi(compr, 2, 0)) {
		eval("iw", "dev", dev, "set", "compr", "lzma", threshold);
	} else if (nvram_default_matchi(compr, 3, 0)) {
		eval("iw", "dev", dev, "set", "compr", "lz4", threshold);
	} else if (nvram_default_matchi(compr, 4, 0)) {
		eval("iw", "dev", dev, "set", "compr", "zstd", threshold);
	} else {
		eval("iw", "dev", dev, "set", "compr", "off");
	}
}

void get_pairwise(const char *prefix, char *pwstring, char *grpstring, int isadhoc, int ismesh);

void setupHostAP_generic_ath9k(const char *prefix, FILE *fp, int isrepeater, int aoss)
{
	MAC80211DEBUG();
	int freq = 0;
	char nfreq[32];
	int freq2 = 0;
	char nfreq2[32];
	int i = 0;
	char *caps;
	int isath5k = is_ath5k(prefix);
	fprintf(fp, "driver=nl80211\n");
	fprintf(fp, "ctrl_interface=/var/run/hostapd\n");
	fprintf(fp, "wmm_ac_bk_cwmin=4\n");
	fprintf(fp, "wmm_ac_bk_cwmax=10\n");
	fprintf(fp, "wmm_ac_bk_aifs=7\n");
	fprintf(fp, "wmm_ac_bk_txop_limit=0\n");
	fprintf(fp, "wmm_ac_bk_acm=0\n");
	fprintf(fp, "wmm_ac_be_aifs=3\n");
	fprintf(fp, "wmm_ac_be_cwmin=4\n");
	fprintf(fp, "wmm_ac_be_cwmax=10\n");
	fprintf(fp, "wmm_ac_be_acm=0\n");
	fprintf(fp, "wmm_ac_vi_aifs=2\n");
	fprintf(fp, "wmm_ac_vi_cwmin=3\n");
	fprintf(fp, "wmm_ac_vi_cwmax=4\n");
	fprintf(fp, "wmm_ac_vi_txop_limit=94\n");
	fprintf(fp, "wmm_ac_vi_acm=0\n");
	fprintf(fp, "wmm_ac_vo_aifs=2\n");
	fprintf(fp, "wmm_ac_vo_cwmin=2\n");
	fprintf(fp, "wmm_ac_vo_cwmax=3\n");
	fprintf(fp, "wmm_ac_vo_txop_limit=47\n");
	fprintf(fp, "wmm_ac_vo_acm=0\n");
	fprintf(fp, "tx_queue_data3_aifs=7\n");
	fprintf(fp, "tx_queue_data3_cwmin=15\n");
	fprintf(fp, "tx_queue_data3_cwmax=1023\n");
	fprintf(fp, "tx_queue_data3_burst=0\n");
	fprintf(fp, "tx_queue_data2_aifs=3\n");
	fprintf(fp, "tx_queue_data2_cwmin=15\n");
	fprintf(fp, "tx_queue_data2_cwmax=63\n");
	fprintf(fp, "tx_queue_data1_aifs=1\n");
	fprintf(fp, "tx_queue_data1_cwmin=7\n");
	fprintf(fp, "tx_queue_data1_cwmax=15\n");
	fprintf(fp, "tx_queue_data1_burst=3.0\n");
	fprintf(fp, "tx_queue_data0_aifs=1\n");
	fprintf(fp, "tx_queue_data0_cwmin=3\n");
	fprintf(fp, "tx_queue_data0_cwmax=7\n");
	fprintf(fp, "tx_queue_data0_burst=1.5\n");
	const char *country = getIsoName(nvram_default_get("wlan0_regdomain", "UNITED_STATES"));
	if (!country)
		country = "DE";
	fprintf(fp, "country_code=%s\n", country);
	char *netmode = nvram_nget("%s_net_mode", prefix);

	if (isath5k || !strcmp(netmode, "ac-only") || !strcmp(netmode, "mixed") || !strcmp(netmode, "acn-mixed") ||
	    !strcmp(netmode, "ax-only") || !strcmp(netmode, "axg-only") || !strcmp(netmode, "xacn-mixed")) {
		fprintf(fp, "tx_queue_data2_burst=2.0\n");
		fprintf(fp, "wmm_ac_be_txop_limit=0\n");
	} else {
		fprintf(fp, "tx_queue_data2_burst=0\n");
		fprintf(fp, "wmm_ac_be_txop_limit=64\n");
	}

	char *akm = nvram_nget("%s_akm", prefix);
	char *crypto = nvram_nget("%s_crypto", prefix);
	const char *ht = NULL;
	int iht = 0;
	int channeloffset = 6;
	char bw[32];
	sprintf(bw, "%s_channelbw", prefix);
	int usebw = 20;
	if (nvram_matchi(bw, 40))
		usebw = 40;
	if (nvram_matchi(bw, 10))
		usebw = 10;
	if (nvram_matchi(bw, 5))
		usebw = 5;
	if (nvram_matchi(bw, 2040))
		usebw = 40;
	if (nvram_matchi(bw, 80))
		usebw = 80;
	if (nvram_matchi(bw, 160))
		usebw = 160;
	if (nvram_match(bw, "80+80"))
		usebw = 8080;

	MAC80211DEBUG();
	if (has_ac(prefix)) {
		if (strcmp(netmode, "acn-mixed") && //
		    strcmp(netmode, "ac-only") && //
		    strcmp(netmode, "ax-only") && //
		    strcmp(netmode, "xacn-mixed") && //
		    strcmp(netmode, "mixed")) {
			fprintf(fp, "ieee80211ac=0\n");
		}
	}
	if (has_ax(prefix)) {
		if (strcmp(netmode, "xacn-mixed") && //
		    strcmp(netmode, "ax-only ") && //
		    strcmp(netmode, "axg-only ") && //
		    strcmp(netmode, "mixed")) {
			fprintf(fp, "ieee80211ax=0\n");
		}
	}
	if ((!strcmp(netmode, "ng-only") || //
	     !strcmp(netmode, "na-only") || //
	     !strcmp(netmode, "n2-only") || //
	     !strcmp(netmode, "n5-only") || //
	     !strcmp(netmode, "ac-only") || //
	     !strcmp(netmode, "ax-only") || //
	     !strcmp(netmode, "axg-only") || //
	     !strcmp(netmode, "acn-mixed") || //
	     !strcmp(netmode, "xacn-mixed") || //
	     !strcmp(netmode, "mixed")) &&
	    strcmp(akm, "wep") && !aoss) {
		if (strcmp(netmode, "mixed") && strcmp(netmode, "ng-only") && strcmp(netmode, "na-only")) {
			if (!isath5k)
				fprintf(fp, "require_ht=1\n");
		}
		if (!isath5k && !has_ad(prefix)) {
			fprintf(fp, "ieee80211n=1\n");
			if (nvram_matchi(bw, 2040)) {
				fprintf(fp, "dynamic_ht40=1\n");
			} else {
				fprintf(fp, "noscan=1\n");
				fprintf(fp, "dynamic_ht40=0\n");
			}
		}
		char sb[32];
		sprintf(sb, "%s_nctrlsb", prefix);
		switch (usebw) {
		case 40:
			ht = get_channeloffset(prefix, &iht, NULL);
			break;
		case 80:
		case 8080:
		case 160:
			ht = get_channeloffset(prefix, &iht, &channeloffset);
			break;
		case 20:
		case 10:
		case 5:
		default:
			ht = "HT20";
			break;
		}
	} else {
		ht = "HT20";
	}
	MAC80211DEBUG();
	char regdomain[32];
	sprintf(regdomain, "%s_regdomain", prefix);

	if (isrepeater) {
		// for ht40- take second channel otherwise hostapd is unhappy (and does not start)
		if (has_2ghz(prefix)) {
			if (iht == -1) {
				i = 4; // in 2.4 mhz channel spacing is different
			}
		}
		if (has_5ghz(prefix)) {
			if (iht == -1) {
				i = 1;
			}
		}
		switch (usebw) {
		case 160:
			ht = "HT40+";
			iht = 1;
			channeloffset = 14;
			freq = 5500;
			break;
		case 80:
		case 8080:
			freq2 = 5775;
			ht = "HT40+";
			channeloffset = 6;
			iht = 1;
			freq = 5500;
			break;
		case 40:
			ht = "HT40+";
			if (has_2ghz(prefix)) {
				freq = 2437;
			}
			if (has_5ghz(prefix)) {
				freq = 5500;
			}
			break;
		case 20:
		case 10:
		case 5:
			ht = "HT20";
		default:
			if (has_2ghz(prefix)) {
				freq = 2437;
			}
			if (has_5ghz(prefix)) {
				freq = 5500;
			}
			break;
		}
	} else {
		// also we still should take care on the selected mode
		sprintf(nfreq, "%s_channel", prefix);
		freq = nvram_default_geti(nfreq, 0);
		sprintf(nfreq2, "%s_channel2", prefix);
		freq2 = nvram_default_geti(nfreq2, 0);

		if (freq == 0) {
			if (has_ad(prefix)) {
				freq = 53320;
			} else {
				struct mac80211_ac *acs;
				fprintf(stderr, "call mac80211autochannel for interface: %s\n", prefix);
				eval("ifconfig", prefix, "up");
				switch (usebw) {
				case 40:
					acs = mac80211autochannel(prefix, NULL, 2, 0, AUTO_FORCEHT40);
					break;
				case 80:
					acs = mac80211autochannel(prefix, NULL, 2, 0, AUTO_FORCEVHT80);
					break;
				case 160:
				case 8080:
					acs = mac80211autochannel(prefix, NULL, 2, 0, AUTO_FORCEVHT160);
					break;
				default:
					acs = mac80211autochannel(prefix, NULL, 2, 0, AUTO_ALL);
				}
				if (acs != NULL) {
					struct wifi_channels *chan = mac80211_get_channels_simple(prefix, country, usebw, 0xff);
					freq = acs->freq;
					fprintf(stderr, "mac80211autochannel interface: %s frequency: %d\n", prefix, freq);
					int i = 0;
					while (chan[i].freq != -1) {
						if (chan[i].freq == freq)
							break;
						i++;
					}
					switch (usebw) {
					case 40:
						if (chan[i].luu && acs->luu) {
							ht = "HT40-";
							iht = -1;
						}
						if (chan[i].ull && acs->ull) {
							ht = "HT40+";
							iht = 1;
						}
						break;
					case 80:
					case 8080:
						if (chan[i].luu && acs->luu) {
							ht = "HT40+";
							channeloffset = 2;
							iht = -1;
						}
						if (chan[i].ulu && acs->ulu) {
							ht = "HT40+";
							iht = 1;
							channeloffset = 6;
						}
						if (chan[i].lul && acs->lul) {
							ht = "HT40-";
							channeloffset = 6;
							iht = -1;
						}
						if (chan[i].ull && acs->ull) {
							ht = "HT40-";
							iht = 1;
							channeloffset = 2;
						}
						break;
					case 160:
						if (chan[i].uuu && acs->uuu) {
							ht = "HT40+";
							iht = 1;
							channeloffset = 14;
						}
						if (chan[i].uul && acs->uul) {
							ht = "HT40-";
							iht = 1;
							channeloffset = 10;
						}
						if (chan[i].ulu && acs->ulu) {
							ht = "HT40+";
							iht = 1;
							channeloffset = 6;
						}
						if (chan[i].ull && acs->ull) {
							ht = "HT40-";
							iht = 1;
							channeloffset = 2;
						}
						if (chan[i].luu && acs->luu) {
							ht = "HT40+";
							iht = -1;
							channeloffset = 2;
						}
						if (chan[i].lul && acs->lul) {
							ht = "HT40-";
							iht = -1;
							channeloffset = 6;
						}
						if (chan[i].llu && acs->llu) {
							ht = "HT40+";
							iht = -1;
							channeloffset = 10;
						}
						if (chan[i].lll && acs->lll) {
							ht = "HT40-";
							iht = -1;
							channeloffset = 14;
						}
						break;
					default:
					case 20:
					case 10:
					case 5:
						ht = "HT20";
						break;
					}
					free_mac80211_ac(acs);
				} else {
					if (has_2ghz(prefix)) {
						freq = 2437;
					}
					if (has_5ghz(prefix)) {
						switch (usebw) {
						case 8080:
						case 80:
							ht = "HT40+";
							iht = 1;
							channeloffset = 6;
							freq = 5500;
							freq2 = 5775;
							break;
						case 160:
							ht = "HT40+";
							iht = 1;
							channeloffset = 14;
							freq = 5500;
							break;
						case 40:
							ht = "HT40+";
							iht = 1;
							freq = 5500;
							break;
						case 20:
						default:
							ht = "HT20";
							freq = 5500;
							break;
						}
					}
				}
			}
		}
	}
	MAC80211DEBUG();
	if (!isath5k && !has_ad(prefix)) {
		char shortgi[32];
		sprintf(shortgi, "%s_shortgi", prefix);
		char greenfield[32];
		sprintf(greenfield, "%s_gf", prefix);
		char ldpc[32];
		sprintf(ldpc, "%s_ldpc", prefix);
		int i_ldpc = nvram_default_geti(ldpc, 1);
		if ((!strcmp(netmode, "mixed") || //
		     !strcmp(netmode, "ac-only") || !strcmp(netmode, "acn-mixed") || !strcmp(netmode, "axg-only") ||
		     !strcmp(netmode, "xacn-mixed") || !strcmp(netmode, "ax-only"))) {
			i_ldpc = 1;
		}
		int smps = nvram_ngeti("%s_smps", prefix);
		caps = mac80211_get_caps(prefix, nvram_default_matchi(shortgi, 1, 1) ? 1 : 0,
					 nvram_default_matchi(greenfield, 1, 0) ? 1 : 0, usebw > 20, i_ldpc, smps);
		if (ht) {
			if (smps == 1 && has_static_smps(prefix))
				fprintf(fp, "ht_capab=[%s]%s[SMPS-STATIC]\n", ht, caps);
			else if (smps == 2 && has_dynamic_smps(prefix))
				fprintf(fp, "ht_capab=[%s]%s[SMPS-DYNAMIC]\n", ht, caps);
			else
				fprintf(fp, "ht_capab=[%s]%s\n", ht, caps);
		} else {
			if (smps == 1 && has_static_smps(prefix))
				fprintf(fp, "ht_capab=%s[SMPS-STATIC]\n", caps);
			else if (smps == 2 && has_dynamic_smps(prefix))
				fprintf(fp, "ht_capab=%s[SMPS-DYNAMIC]\n", caps);
		}
		free(caps);
	}
	MAC80211DEBUG();
	cur_freq = freq;
	cur_freq2 = freq2;
	cur_channeloffset = channeloffset;
	cur_iht = iht;
	char shortgi[32];
	sprintf(shortgi, "%s_shortgi", prefix);
	char mubf[32];
	sprintf(mubf, "%s_mubf", prefix);
	char subf[32];
	sprintf(subf, "%s_subf", prefix);

	caps = mac80211_get_vhtcaps(prefix, nvram_default_matchi(shortgi, 1, 1) ? 1 : 0,
				    (usebw == 80 || usebw == 160 || usebw == 8080) ? 1 : 0, usebw == 160 ? 1 : 0,
				    usebw == 8080 ? 1 : 0, nvram_default_matchi(subf, 1, DEFAULT_BF),
				    nvram_default_matchi(mubf, 1, DEFAULT_BF));
	cur_caps = caps;
	if (has_ac(prefix) && has_5ghz(prefix)) {
		if (freq >= 4000 && (!strcmp(netmode, "mixed") || //
				     !strcmp(netmode, "ac-only") || !strcmp(netmode, "acn-mixed") || !strcmp(netmode, "ax-only") ||
				     !strcmp(netmode, "xacn-mixed"))) {
			if (*caps) {
				fprintf(fp, "vht_capab=%s\n", caps);
				if (!strcmp(netmode, "ac-only")) {
					fprintf(fp, "ieee80211ac=1\n");
					fprintf(fp, "require_vht=1\n");
					fprintf(fp, "ieee80211d=1\n");
					fprintf(fp, "ieee80211h=1\n");
					//might be needed for dfs
					//fprintf(fp, "spectrum_mgmt_required=1\n");
					//fprintf(fp, "local_pwr_constraint=3\n");
				}

				if (!strcmp(netmode, "acn-mixed")) {
					fprintf(fp, "ieee80211ac=1\n");
					fprintf(fp, "require_ht=1\n");
					fprintf(fp, "ieee80211d=1\n");
					fprintf(fp, "ieee80211h=1\n");
				}

				if (has_ax(prefix)) {
					if (!strcmp(netmode, "xacn-mixed") || !strcmp(netmode, "ax-only")) {
						fprintf(fp, "ieee80211ax=1\n");
						fprintf(fp, "ieee80211ac=1\n");
						fprintf(fp, "require_ht=1\n");
						fprintf(fp, "ieee80211d=1\n");
						fprintf(fp, "ieee80211h=1\n");
					}
					if (!strcmp(netmode, "ax-only")) {
						fprintf(fp, "require_vht=1\n");
						fprintf(fp, "require_he=1\n");
					}
				}

				if (!strcmp(netmode, "mixed")) {
					if (has_ax(prefix)) {
						fprintf(fp, "ieee80211ax=1\n");
					}
					fprintf(fp, "ieee80211ac=1\n");
					fprintf(fp, "ieee80211d=1\n");
					fprintf(fp, "ieee80211h=1\n");
				}
				if (has_ax(prefix)) {
					if (!strcmp(netmode, "mixed") || !strcmp(netmode, "xacn-mixed") ||
					    !strcmp(netmode, "ax-only")) {
						if (nvram_match(mubf, "1")) {
							fprintf(fp, "he_mu_beamformer=1\n");
						}
						if (nvram_match(subf, "1")) {
							fprintf(fp, "he_su_beamformer=1\n");
							fprintf(fp, "he_su_beamformee=1\n");
						}
						fprintf(fp, "he_default_pe_duration=4\n");
						fprintf(fp, "he_rts_threshold=1023\n");
						fprintf(fp, "he_mu_edca_qos_info_param_count=0\n");
						fprintf(fp, "he_mu_edca_qos_info_q_ack=0\n");
						fprintf(fp, "he_mu_edca_qos_info_queue_request=0\n");
						fprintf(fp, "he_mu_edca_qos_info_txop_request=0\n");
						fprintf(fp, "he_mu_edca_ac_be_aifsn=8\n");
						fprintf(fp, "he_mu_edca_ac_be_aci=0\n");
						fprintf(fp, "he_mu_edca_ac_be_ecwmin=9\n");
						fprintf(fp, "he_mu_edca_ac_be_ecwmax=10\n");
						fprintf(fp, "he_mu_edca_ac_be_timer=255\n");
						fprintf(fp, "he_mu_edca_ac_bk_aifsn=15\n");
						fprintf(fp, "he_mu_edca_ac_bk_aci=1\n");
						fprintf(fp, "he_mu_edca_ac_bk_ecwmin=9\n");
						fprintf(fp, "he_mu_edca_ac_bk_ecwmax=10\n");
						fprintf(fp, "he_mu_edca_ac_bk_timer=255\n");
						fprintf(fp, "he_mu_edca_ac_vi_ecwmin=5\n");
						fprintf(fp, "he_mu_edca_ac_vi_ecwmax=7\n");
						fprintf(fp, "he_mu_edca_ac_vi_aifsn=5\n");
						fprintf(fp, "he_mu_edca_ac_vi_aci=2\n");
						fprintf(fp, "he_mu_edca_ac_vi_timer=255\n");
						fprintf(fp, "he_mu_edca_ac_vo_aifsn=5\n");
						fprintf(fp, "he_mu_edca_ac_vo_aci=3\n");
						fprintf(fp, "he_mu_edca_ac_vo_ecwmin=5\n");
						fprintf(fp, "he_mu_edca_ac_vo_ecwmax=7\n");
						fprintf(fp, "he_mu_edca_ac_vo_timer=255\n");
						char color[32];
						sprintf(color, "%s_bss_color", prefix);
						int c = nvram_default_geti(color, 128);
						if (c > 0) {
							fprintf(fp, "he_bss_color=%d\n", c);
						}
						fprintf(fp, "he_bss_color_partial=%d\n",
							nvram_nmatch("1", "%s_bss_color_partial", prefix) ? 1 : 0);
						fprintf(fp, "he_twt_required=%d\n",
							nvram_nmatch("1", "%s_twt_required", prefix) ? 1 : 0);
					}
				}
				fprintf(fp, "no_country_ie=1\n");

				switch (usebw) {
				case 40:
					fprintf(fp, "vht_oper_chwidth=0\n");
					fprintf(fp, "vht_oper_centr_freq_seg0_idx_freq=%d\n", freq + (10 * iht));
					break;
				case 80:
					fprintf(fp, "vht_oper_chwidth=1\n");
					fprintf(fp, "vht_oper_centr_freq_seg0_idx_freq=%d\n", freq + ((channeloffset * 5) * iht));
					break;
				case 160:
					fprintf(fp, "vht_oper_chwidth=2\n");
					fprintf(fp, "vht_oper_centr_freq_seg0_idx_freq=%d\n", freq + ((channeloffset * 5) * iht));
					break;
				case 8080:
					fprintf(fp, "vht_oper_chwidth=3\n");
					fprintf(fp, "vht_oper_centr_freq_seg0_idx_freq=%d\n", freq + ((channeloffset * 5) * iht));
					fprintf(fp, "vht_oper_centr_freq_seg1_idx_freq=%d\n", freq2);
					break;
				default:
					fprintf(fp, "vht_oper_chwidth=0\n");
					break;
				}
				if (has_ax(prefix) &&
				    (!strcmp(netmode, "ax-only") || !strcmp(netmode, "xacn-mixed") || !strcmp(netmode, "mixed"))) {
					switch (usebw) {
					case 40:
						fprintf(fp, "he_oper_chwidth=0\n");
						fprintf(fp, "he_oper_centr_freq_seg0_idx_freq=%d\n", freq + (10 * iht));
						break;
					case 80:
						fprintf(fp, "he_oper_chwidth=1\n");
						fprintf(fp, "he_oper_centr_freq_seg0_idx_freq=%d\n",
							freq + ((channeloffset * 5) * iht));
						break;
					case 160:
						fprintf(fp, "he_oper_chwidth=2\n");
						fprintf(fp, "he_oper_centr_freq_seg0_idx_freq=%d\n",
							freq + ((channeloffset * 5) * iht));
						break;
					case 8080:
						fprintf(fp, "he_oper_chwidth=3\n");
						fprintf(fp, "he_oper_centr_freq_seg0_idx_freq=%d\n",
							freq + ((channeloffset * 5) * iht));
						fprintf(fp, "he_oper_centr_freq_seg1_idx_freq=%d\n", freq2);
						break;
					default:
						fprintf(fp, "he_oper_chwidth=0\n");
						break;
					}
				}
			}
		}
	}
	nvram_default_nget("0", "%s_cell_density", prefix);
	nvram_default_nget("1", "%s_legacy", prefix);
	int density = nvram_ngeti("%s_cell_density", prefix);
	int legacy = nvram_ngeti("%s_legacy", prefix);
	if (has_ad(prefix)) {
		fprintf(fp, "hw_mode=ad\n");
	} else if (freq < 4000) {
		if (!strcmp(netmode, "b-only")) {
			fprintf(fp, "hw_mode=b\n");

			if (density == 1) {
				fprintf(fp, "supported_rates=55 110\n");
				fprintf(fp, "basic_rates=55 110\n");
			} else if (density > 2) {
				fprintf(fp, "supported_rates=110\n");
				fprintf(fp, "basic_rates=110\n");
			}

		} else if (!strcmp(netmode, "bg-mixed")) {
			fprintf(fp, "hw_mode=g\n");

			switch (density) {
			case 0:
			case 1:
				if (legacy) {
					fprintf(fp, "supported_rates=10 20 55 60 90 110 120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=55 60 110 120 240\n");
				} else {
					fprintf(fp, "supported_rates=60 90 120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=60 90 120 240\n");
				}
				break;
			case 2:
				if (legacy) {
					fprintf(fp, "supported_rates=110 120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=110 120 240\n");
				} else {
					fprintf(fp, "supported_rates=120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=120 240\n");
				}
				break;
			default:
				fprintf(fp, "supported_rates=240 360 480 540\n");
				fprintf(fp, "basic_rates=240\n");
				break;
			}
		} else if (!strcmp(netmode, "mixed") || !strcmp(netmode, "axg-only")) {
			if (has_ax(prefix)) {
				if (!strcmp(netmode, "ax-only")) {
					fprintf(fp, "require_he=1\n");
				}
				if (nvram_match(mubf, "1")) {
					fprintf(fp, "he_mu_beamformer=1\n");
				}
				if (nvram_match(subf, "1")) {
					fprintf(fp, "he_su_beamformer=1\n");
					fprintf(fp, "he_su_beamformee=1\n");
				}
				fprintf(fp, "ieee80211ax=1\n");
				fprintf(fp, "he_default_pe_duration=4\n");
				fprintf(fp, "he_rts_threshold=1023\n");
				fprintf(fp, "he_mu_edca_qos_info_param_count=0\n");
				fprintf(fp, "he_mu_edca_qos_info_q_ack=0\n");
				fprintf(fp, "he_mu_edca_qos_info_queue_request=0\n");
				fprintf(fp, "he_mu_edca_qos_info_txop_request=0\n");
				fprintf(fp, "he_mu_edca_ac_be_aifsn=8\n");
				fprintf(fp, "he_mu_edca_ac_be_aci=0\n");
				fprintf(fp, "he_mu_edca_ac_be_ecwmin=9\n");
				fprintf(fp, "he_mu_edca_ac_be_ecwmax=10\n");
				fprintf(fp, "he_mu_edca_ac_be_timer=255\n");
				fprintf(fp, "he_mu_edca_ac_bk_aifsn=15\n");
				fprintf(fp, "he_mu_edca_ac_bk_aci=1\n");
				fprintf(fp, "he_mu_edca_ac_bk_ecwmin=9\n");
				fprintf(fp, "he_mu_edca_ac_bk_ecwmax=10\n");
				fprintf(fp, "he_mu_edca_ac_bk_timer=255\n");
				fprintf(fp, "he_mu_edca_ac_vi_ecwmin=5\n");
				fprintf(fp, "he_mu_edca_ac_vi_ecwmax=7\n");
				fprintf(fp, "he_mu_edca_ac_vi_aifsn=5\n");
				fprintf(fp, "he_mu_edca_ac_vi_aci=2\n");
				fprintf(fp, "he_mu_edca_ac_vi_timer=255\n");
				fprintf(fp, "he_mu_edca_ac_vo_aifsn=5\n");
				fprintf(fp, "he_mu_edca_ac_vo_aci=3\n");
				fprintf(fp, "he_mu_edca_ac_vo_ecwmin=5\n");
				fprintf(fp, "he_mu_edca_ac_vo_ecwmax=7\n");
				fprintf(fp, "he_mu_edca_ac_vo_timer=255\n");
				char color[32];
				sprintf(color, "bss_color", prefix);
				int c = nvram_default_geti(color, 128);
				if (c > 0) {
					fprintf(fp, "he_bss_color=%d\n", c);
				}
				fprintf(fp, "he_bss_color_partial=%d\n", nvram_nmatch("1", "%s_bss_color_partial", prefix) ? 1 : 0);
				fprintf(fp, "he_twt_required=%d\n", nvram_nmatch("1", "%s_twt_required", prefix) ? 1 : 0);
			}
			fprintf(fp, "hw_mode=g\n");
			switch (density) {
			case 0:
			case 1:
				if (legacy) {
					fprintf(fp, "supported_rates=10 20 55 60 90 110 120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=55 60 110 120 240\n");
				} else {
					fprintf(fp, "supported_rates=60 90 120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=60 90 120 240\n");
				}
				break;
			case 2:
				if (legacy) {
					fprintf(fp, "supported_rates=110 120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=110 120 240\n");
				} else {
					fprintf(fp, "supported_rates=120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=120 240\n");
				}
				break;
			default:
				fprintf(fp, "supported_rates=240 360 480 540\n");
				fprintf(fp, "basic_rates=240\n");
				break;
			}

		} else {
			fprintf(fp, "hw_mode=g\n");

			switch (density) {
			case 0:
			case 1:
				if (legacy) {
					fprintf(fp, "supported_rates=10 20 55 60 90 110 120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=55 60 110 120 240\n");
				} else {
					fprintf(fp, "supported_rates=60 90 120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=60 90 120 240\n");
				}
				break;
			case 2:
				if (legacy) {
					fprintf(fp, "supported_rates=110 120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=110 120 240\n");
				} else {
					fprintf(fp, "supported_rates=120 180 240 360 480 540\n");
					fprintf(fp, "basic_rates=120 240\n");
				}
				break;
			default:
				fprintf(fp, "supported_rates=240 360 480 540\n");
				fprintf(fp, "basic_rates=240\n");
				break;
			}
		}
	} else {
		fprintf(fp, "hw_mode=a\n");

		switch (density) {
		case 1:
			fprintf(fp, "supported_rates=60 90 120 180 240 360 480 540\n");
			fprintf(fp, "basic_rates=60 120 240\n");
			break;

		case 2:
			fprintf(fp, "supported_rates=120 180 240 360 480 540\n");
			fprintf(fp, "basic_rates=120 240\n");
			break;

		case 3:
			fprintf(fp, "supported_rates=240 360 480 540\n");
			fprintf(fp, "basic_rates=240\n");
			break;
		}
	}

	MAC80211DEBUG();
	fprintf(fp, "channel=%d\n", ieee80211_mhz2ieee(freq));
	//	if (!has_ad(prefix))
	fprintf(fp, "frequency=%d\n", freq);
	char bcn[32];
	sprintf(bcn, "%s_bcn", prefix);
#ifdef HAVE_ATH9K
	char *vifs = nvram_nget("%s_vifs", prefix);
	int intval = atoi(nvram_default_get(bcn, "100"));
	if (*vifs && has_beacon_limit(prefix)) {
		if (intval < 100)
			intval = 100;
	}
#endif
	fprintf(fp, "beacon_int=%d\n", intval);
#ifdef HAVE_WPA3
	char airtime[32];
	sprintf(airtime, "%s_at_policy", prefix);
	if (nvram_matchi(airtime, 0)) {
		fprintf(fp, "airtime_mode=0\n");
	}
	if (nvram_matchi(airtime, 1)) {
		fprintf(fp, "airtime_mode=1\n");
	}
	if (nvram_matchi(airtime, 2)) {
		fprintf(fp, "airtime_mode=2\n");
	}
#endif
	fprintf(fp, "\n");
}

static void setMacFilter(FILE *fp, char *iface)
{
	ENTER;
	char *next;
	char var[32];
	char nvvar[32];
	sprintf(nvvar, "%s_macmode", iface);
	if (nvram_match(nvvar, "deny")) {
		fprintf(fp, "deny_mac_file=/tmp/%s_deny\n", iface);
		fprintf(fp, "macaddr_acl=0\n");
		char nvlist[32];
		sprintf(nvlist, "%s_maclist", iface);
		char name[32];
		sprintf(name, "/tmp/%s_deny", iface);
		FILE *out = fopen(name, "wb");
		char *list = nvram_safe_get(nvlist);
		foreach(var, list, next)
		{
			fprintf(out, "%s\n", var);
		}
		fclose(out);
	} else if (nvram_match(nvvar, "allow")) {
		fprintf(fp, "accept_mac_file=/tmp/%s_accept\n", iface);
		fprintf(fp, "macaddr_acl=1\n");
		char nvlist[32];
		sprintf(nvlist, "%s_maclist", iface);
		char name[32];
		sprintf(name, "/tmp/%s_accept", iface);
		FILE *out = fopen(name, "wb");
		char *list = nvram_safe_get(nvlist);
		foreach(var, list, next)
		{
			fprintf(out, "%s\n", var);
		}
		fclose(out);
	}
	LEAVE;
}
int isregistered(void);

static int ieee80211_aton(char *str, unsigned char mac[6])
{
	unsigned int addr[6];
	int i;
	if (sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x", &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]) != 6)
		return -1;
	/*
	 * sscanf needs an unsigned int, but mac address bytes cannot exceed 0xff
	 */
	for (i = 0; i < 6; i++)
		mac[i] = addr[i] & 0xff;
	return 0;
}

extern char *hostapd_eap_get_types(void);
extern void addWPS(FILE *fp, char *prefix, int configured);
extern void setupHS20(FILE *fp, char *prefix);
void setupHostAPPSK(FILE *fp, char *prefix, int isfirst);
void setupHostAP_ath9k(char *maininterface, int isfirst, int vapid, int aoss)
{
#ifdef HAVE_REGISTER
	if (!isregistered())
		return;
#endif
	MAC80211DEBUG();
	char psk[32];
	char akm[32];
	char mfp[32];
	char ft[32];
	char fstr[32];
	char preamble[32];
	char lowack[32];
	char uapsd[32];
	FILE *fp = NULL;
	char *ssid;
	char nssid[32];
	char maxassoc[32];
	char ifname[32];
	int isrepeater = 0;
	unsigned char hwbuff[32];
	char macaddr[32];
	char *types;
	char *debug;
	char tmp[256];
	if (isfirst && !vapid) {
		sprintf(ifname, "%s", maininterface);
	} else {
		sprintf(ifname, "%s.%d", maininterface, vapid);
		if (!nvram_nmatch("mesh", "%s_mode", maininterface))
			isrepeater = 1;
	}
#ifdef HAVE_WZRHPAG300NH
	if (aoss) {
		if (!strncmp(ifname, "wlan0", 4))
			sprintf(ifname, "aossg");
		else
			sprintf(ifname, "aossa");
	}
#else
	if (aoss)
		sprintf(ifname, "aoss");
#endif
	MAC80211DEBUG();
	sprintf(akm, "%s_akm", ifname);
	sprintf(ft, "%s_ft", ifname);
	sprintf(mfp, "%s_mfp", ifname);
	sprintf(preamble, "%s_preamble", ifname);
	sprintf(lowack, "%s_d_lowack", ifname);
	sprintf(uapsd, "%s_uapsd", ifname);
	check_cryptomod(ifname);
	if (nvram_match(akm, "8021X"))
		return;
	sprintf(fstr, "/tmp/%s_hostap.conf", maininterface);
	if (isfirst) {
		fp = fopen(fstr, "wb");
		setupHostAP_generic_ath9k(maininterface, fp, isrepeater, aoss);
		if (has_ad(ifname))
			fprintf(fp, "interface=giwifi0\n");
		else
			fprintf(fp, "interface=%s\n", ifname);
	} else {
		fp = fopen(fstr, "ab");
		fprintf(fp, "bss=%s\n", ifname);
	}
	if (nvram_default_nmatchi(1, 0, "%s_m2u", ifname))
		fprintf(fp, "multicast_to_unicast=1\n");
	else
		fprintf(fp, "multicast_to_unicast=0\n");

	char bw[32];
	sprintf(bw, "%s_channelbw", maininterface);
	int usebw = 20;
	if (nvram_matchi(bw, 40))
		usebw = 40;
	if (nvram_matchi(bw, 10))
		usebw = 10;
	if (nvram_matchi(bw, 5))
		usebw = 5;
	if (nvram_matchi(bw, 2040))
		usebw = 40;
	if (nvram_matchi(bw, 80))
		usebw = 80;
	if (nvram_matchi(bw, 160))
		usebw = 160;
	if (nvram_match(bw, "80+80"))
		usebw = 8080;
	if (isfirst && has_qam256(ifname) && has_2ghz(ifname) && (usebw < 80 || cansuperchannel(maininterface))) {
		if (nvram_nmatch("1", "%s_turbo_qam", maininterface)) {
			fprintf(fp, "vht_capab=%s\n", cur_caps);
			fprintf(fp, "ieee80211ac=1\n");
			switch (usebw) {
			case 40:
				fprintf(fp, "vht_oper_chwidth=0\n");
				fprintf(fp, "vht_oper_centr_freq_seg0_idx_freq=%d\n", cur_freq + ((2 * 5) * cur_iht));
				break;
			case 80:
				fprintf(fp, "vht_oper_chwidth=1\n");
				fprintf(fp, "vht_oper_centr_freq_seg0_idx_freq=%d\n",
					cur_freq + ((cur_channeloffset * 5) * cur_iht));
				break;
			case 160:
				fprintf(fp, "vht_oper_chwidth=2\n");
				fprintf(fp, "vht_oper_centr_freq_seg0_idx_freq=%d\n",
					cur_freq + ((cur_channeloffset * 5) * cur_iht));
				break;
			case 8080:
				fprintf(fp, "vht_oper_chwidth=3\n");
				fprintf(fp, "vht_oper_centr_freq_seg0_idx_freq=%d\n",
					cur_freq + ((cur_channeloffset * 5) * cur_iht));
				fprintf(fp, "vht_oper_centr_freq_seg1_idx_freq=%d\n", cur_freq2);
				break;
			default:
				fprintf(fp, "vht_oper_chwidth=0\n");
				break;
			}
		}
	}
	fprintf(fp, "no_country_ie=1\n");
	if (has_qam256(ifname) && has_2ghz(ifname) && (usebw < 80 || cansuperchannel(maininterface))) {
		if (nvram_nmatch("1", "%s_turbo_qam", maininterface)) {
			fprintf(fp, "vendor_vht=1\n");
		}
	}
	MAC80211DEBUG();
	if (!vapid)
		fprintf(fp, "preamble=%s\n", nvram_default_get(preamble, "1"));
#ifdef HAVE_MVEBU
	fprintf(fp, "disassoc_low_ack=%s\n", nvram_default_get(lowack, "0"));
#else
	fprintf(fp, "disassoc_low_ack=%s\n", nvram_default_get(lowack, "1"));
#endif
	char *mode = nvram_nget("%s_mode", ifname);
	if (!strcmp(mode, "wdsap") || !strcmp(mode, "apup"))
		fprintf(fp, "wds_sta=1\n");
	if (!strcmp(mode, "apup"))
		fprintf(fp, "apup=1\n");
	char wmm[32];
	sprintf(wmm, "%s_wmm", ifname);
	fprintf(fp, "wmm_enabled=%s\n", nvram_default_get(wmm, "1"));
	if (has_uapsd(ifname)) {
		fprintf(fp, "uapsd_advertisement_enabled=%s\n", nvram_default_get(uapsd, "0"));
	}
	if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") &&
	    nvram_invmatch("def_whwaddr", "") && !strcmp(maininterface, "wlan0")) {
		strcpy(macaddr, nvram_safe_get("def_whwaddr"));
	} else {
		char *wifimac = nvram_nget("%s_hwaddr", maininterface);
		if (!*wifimac || ieee80211_aton(wifimac, hwbuff) < 0) {
			wl_hwaddr(maininterface, hwbuff);
			sprintf(macaddr, "%02X:%02X:%02X:%02X:%02X:%02X", hwbuff[0], hwbuff[1], hwbuff[2], hwbuff[3], hwbuff[4],
				hwbuff[5]);
		} else {
			strcpy(macaddr, wifimac);
		}
	}

	if (vapid > 0) {
		char *wifimac = nvram_nget("%s_hwaddr", ifname);
		if (!*wifimac) {
			int brand = getRouterBrand();
			if (brand == ROUTER_WRT_3200ACM || brand == ROUTER_WRT_32X) {
				hwbuff[0] |= 0x2;
				hwbuff[5] += vapid & 0xf;
			} else {
				hwbuff[0] ^= ((vapid - 1) << 2) | 0x2;
			}
			sprintf(macaddr, "%02X:%02X:%02X:%02X:%02X:%02X", hwbuff[0], hwbuff[1], hwbuff[2], hwbuff[3], hwbuff[4],
				hwbuff[5]);
		} else {
			strcpy(macaddr, wifimac);
		}
	}
	MAC80211DEBUG();
	//              MAC_ADD(macaddr);
	if (!has_ad(maininterface) && !is_brcmfmac(maininterface)) {
		fprintf(fp, "bssid=%s\n", macaddr);
	}
	char vathmac[32];
	sprintf(vathmac, "%s_hwaddr", ifname);
	nvram_set(vathmac, macaddr);
	fprintf(stderr, "setup %s %s\n", ifname, macaddr);
	setMacFilter(fp, ifname);
	char isolate[32];
	char broadcast[32];
	sprintf(isolate, "%s_ap_isolate", ifname);
	if (nvram_default_matchi(isolate, 1, 0))
		fprintf(fp, "ap_isolate=1\n");
	sprintf(broadcast, "%s_closed", ifname);
	if (nvram_default_matchi(broadcast, 1, 0))
		fprintf(fp, "ignore_broadcast_ssid=1\n");
	else
		fprintf(fp, "ignore_broadcast_ssid=0\n");
	sprintf(maxassoc, "%s_maxassoc", ifname);
	fprintf(fp, "max_num_sta=%s\n", nvram_default_get(maxassoc, "256"));
	char dtim[32];
	sprintf(dtim, "%s_dtim", ifname);
	fprintf(fp, "dtim_period=%s\n", nvram_default_get(dtim, "2"));
	if (aoss) {
		if (!strncmp(ifname, "aossa", 5))
			ssid = "ESSID-AOSS-1";
		else
			ssid = "ESSID-AOSS";
	} else {
		sprintf(nssid, "%s_ssid", ifname);
#if defined(HAVE_TMK)
		ssid = nvram_default_get(nssid, "KMT_vap");
#elif defined(HAVE_BKM)
		ssid = nvram_default_get(nssid, "BKM_vap");
#else
		ssid = nvram_default_get(nssid, "dd-wrt");
#endif
	}

	fprintf(fp, "ssid=%s\n", ssid);
	int ispsk2 = nvhas(akm, "psk2");
	int ispsk = nvhas(akm, "psk");
	int ispsk3 = nvhas(akm, "psk3");
	int iswpa = nvhas(akm, "wpa");
	int iswpa2 = nvhas(akm, "wpa2");
	int iswpa3 = nvhas(akm, "wpa3");
	int isowe = nvhas(akm, "owe");
	int iswpa3_128 = nvhas(akm, "wpa3-128");
	int iswpa3_192 = nvhas(akm, "wpa3-192");
	int iswpa2sha256 = nvhas(akm, "wpa2-sha256");
	int iswpa2sha384 = nvhas(akm, "wpa2-sha384");
	int ispsk2sha256 = nvhas(akm, "psk2-sha256");
	int iswep = nvhas(akm, "wep");
	MAC80211DEBUG();
	if (nvram_nmatch("1", "%s_bridged", ifname))
		fprintf(fp, "bridge=%s\n", getBridge(ifname, tmp));
	// wep key support
	if (iswep || aoss) {
		//              if (!isfirst || aoss)
		//                      fprintf(fp, "ieee80211n=0\n");

		if (!aoss) {
			if (!strncmp(ifname, "wlan0", 4))
				led_control(LED_SEC0, LED_ON);
			if (!strncmp(ifname, "wlan1", 4))
				led_control(LED_SEC1, LED_ON);
		}
		fprintf(fp, "logger_syslog=-1\n");
		debug = nvram_nget("%s_wpa_debug", ifname);
		if (debug != NULL) {
			if (!strcmp(debug, "1"))
				fprintf(fp, "logger_syslog_level=1\n");
			else if (!strcmp(debug, "2"))
				fprintf(fp, "logger_syslog_level=2\n");
			else if (!strcmp(debug, "3"))
				fprintf(fp, "logger_syslog_level=0\n");
		} else
			fprintf(fp, "logger_syslog_level=2\n");
		fprintf(fp, "logger_stdout=-1\n");
		fprintf(fp, "logger_stdout_level=2\n");
		char *authmode = nvram_nget("%s_authmode", ifname);
		if (aoss)
			authmode = "auto";
		if (!strcmp(authmode, "shared"))
			fprintf(fp, "auth_algs=2\n");
		else if (!strcmp(authmode, "auto"))
			fprintf(fp, "auth_algs=3\n");
		else
			fprintf(fp, "auth_algs=1\n");
		int i;
		if (aoss) {
			for (i = 1; i < 5; i++) {
				fprintf(fp, "wep_key%d=4D454C434F\n", i - 1);
			}
			fprintf(fp, "wep_default_key=0\n");
		} else {
			for (i = 1; i < 5; i++) {
				char *athkey = nvram_nget("%s_key%d", ifname, i);
				if (athkey != NULL && *athkey) {
					fprintf(fp, "wep_key%d=%s\n", i - 1, athkey);
				}
			}
			fprintf(fp, "wep_default_key=%d\n", atoi(nvram_nget("%s_key", ifname)) - 1);
			addWPS(fp, ifname, 1);
		}
	} else if (nvram_match(akm, "disabled")) {
		addWPS(fp, ifname, 0);
	} else if (ispsk || ispsk2 || ispsk3 || isowe || iswpa || iswpa2 || iswpa3 || iswpa3_128 || iswpa3_192 || iswpa2sha256 ||
		   iswpa2sha384 || ispsk2sha256) {
		setupHostAPPSK(fp, ifname, isfirst);
	} else if (nvhas(akm, "radius")) {
		fprintf(fp, "ieee8021x=1\n");
		char local_ip[32];
		sprintf(local_ip, "%s_local_ip", ifname);
		char *lip = nvram_default_get(local_ip, "0.0.0.0");
		if (strcmp(lip, "0.0.0.0")) {
			fprintf(fp, "radius_client_addr=%s\n", lip);
			fprintf(fp, "own_ip_addr=%s\n", lip);
		} else {
			if (nvram_match("wan_proto", "disabled"))
				fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr"));
			else {
				char *wip = get_wan_ipaddr();
				if (*wip)
					fprintf(fp, "own_ip_addr=%s\n", wip);
				else
					fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr"));
			}
		}

		fprintf(fp, "eap_server=0\n");
		fprintf(fp, "auth_algs=1\n");
		char retry[32];
		sprintf(retry, "%s_radius_retry", ifname);
		fprintf(fp, "radius_retry_primary_interval=%s\n", nvram_default_get(retry, "600"));
		types = hostapd_eap_get_types();
		fprintf(fp, "%s", types);
		free(types);
		fprintf(fp, "auth_server_addr=%s\n", nvram_nget("%s_radius_ipaddr", ifname));
		fprintf(fp, "auth_server_port=%s\n", nvram_nget("%s_radius_port", ifname));
		fprintf(fp, "auth_server_shared_secret=%s\n", nvram_nget("%s_radius_key", ifname));
		char check[64];
		sprintf(check, "%s_radius2_ipaddr", ifname);
		nvram_default_get(check, "0.0.0.0");
		if (!nvram_nmatch("", "%s_radius2_ipaddr", ifname) && !nvram_nmatch("0.0.0.0", "%s_radius2_ipaddr", ifname) &&
		    !nvram_nmatch("", "%s_radius2_port", ifname)) {
			fprintf(fp, "auth_server_addr=%s\n", nvram_nget("%s_radius2_ipaddr", ifname));
			fprintf(fp, "auth_server_port=%s\n", nvram_nget("%s_radius2_port", ifname));
			fprintf(fp, "auth_server_shared_secret=%s\n", nvram_nget("%s_radius2_key", ifname));
		}
		if (nvram_nmatch("1", "%s_acct", ifname)) {
			fprintf(fp, "acct_server_addr=%s\n", nvram_nget("%s_acct_ipaddr", ifname));
			fprintf(fp, "acct_server_port=%s\n", nvram_nget("%s_acct_port", ifname));
			fprintf(fp, "acct_server_shared_secret=%s\n", nvram_nget("%s_acct_key", ifname));
		}
	}
	// fprintf (fp, "jumpstart_p1=1\n");

	MAC80211DEBUG();
	/* low signal drop */
	char signal[32];
	sprintf(signal, "%s_connect", ifname);
	fprintf(fp, "signal_connect=%s\n", nvram_default_get(signal, "-128"));
	sprintf(signal, "%s_stay", ifname);
	fprintf(fp, "signal_stay=%s\n", nvram_default_get(signal, "-128"));
	sprintf(signal, "%s_poll_time", ifname);
	fprintf(fp, "signal_poll_time=%s\n", nvram_default_get(signal, "10"));
	sprintf(signal, "%s_strikes", ifname);
	fprintf(fp, "signal_strikes=%s\n", nvram_default_get(signal, "3"));
#ifdef HAVE_WPA3
	char airtime[32];
	sprintf(airtime, "%s_at_policy", maininterface);
	if (nvram_matchi(airtime, 1) || nvram_matchi(airtime, 2)) {
		char at_weight[32];
		sprintf(at_weight, "%s_at_weight", ifname);
		fprintf(fp, "airtime_bss_weight=%d\n", nvram_default_geti(at_weight, 1));
	}
	if (nvram_matchi(airtime, 2)) {
		char at_limit[32];
		sprintf(at_limit, "%s_at_limit", ifname);
		fprintf(fp, "airtime_bss_limit=%d\n", nvram_default_geti(at_limit, 0));
	}
#endif

	MAC80211DEBUG();
	char rts[32];

	sprintf(rts, "%s_protmode", ifname);
	nvram_default_get(rts, "None");

	sprintf(rts, "%s_rts", ifname);
	nvram_default_get(rts, "0");

	sprintf(rts, "%s_rtsvalue", ifname);
	nvram_default_get(rts, "500");

	if (nvram_nmatch("1", "%s_rts", ifname)) {
		fprintf(fp, "rts_threshold=%s\n", nvram_nget("%s_rtsvalue", ifname));
	}
#ifdef HAVE_HOTSPOT20
	setupHS20(fp, ifname);
#endif
	MAC80211DEBUG();
	if (has_wpa3(ifname)) {
		char *owe_ifname = nvram_nget("%s_owe_ifname", ifname);
		if (*owe_ifname) {
			char oif[32];
			char *next;
			int oexists = 0;
			char *vifs = nvram_nget("%s_vifs", maininterface);
			foreach(oif, vifs, next)
			{
				if (!strcmp(oif, owe_ifname) && !nvram_nmatch("disabled", "%s_net_mode", owe_ifname) &&
				    !nvram_nmatch("disabled", "%s_mode", owe_ifname)) {
					oexists = 1;
				}
			}

			if (oexists && nvram_nmatch("1", "%s_owe", owe_ifname)) {
				fprintf(fp, "owe_transition_ifname=%s\n", owe_ifname);
			}
		}
	}
#ifdef HAVE_WPA3
	if (nvram_nmatch("1", "%s_80211v", ifname)) {
		fprintf(fp, "wnm_sleep_mode=%d\n", nvram_nmatch("1", "%s_wnm_sleep_mode", ifname) ? 1 : 0);
		fprintf(fp, "wnm_sleep_mode_no_keys=%d\n", nvram_nmatch("1", "%s_wnm_sleep_mode_no_keys", ifname) ? 1 : 0);
		fprintf(fp, "bss_transition=%d\n", nvram_nmatch("1", "%s_bss_transition", ifname) ? 1 : 0);
		if (nvram_nmatch("1", "%s_proxy_arp", ifname) && nvram_nmatch("1", "%s_bridged", ifname)) {
			fprintf(fp, "ap_isolate=1\n");
			fprintf(fp, "proxy_arp=1\n");
		} else {
			fprintf(fp, "proxy_arp=0\n");
		}
		if (nvram_nmatch("1", "%s_time_advertisement", ifname)) {
			fprintf(fp, "time_advertisement=2\n");
			char *tz;
			tz = nvram_nget("%s_time_zone", ifname);
			int i;
			const char *zone = "Europe/Berlin";
			for (i = 0; allTimezones[i].tz_name != NULL; i++) {
				if (!strcmp(allTimezones[i].tz_name, tz)) {
					zone = allTimezones[i].tz_string;
					break;
				}
			}
			fprintf(fp, "time_zone=%s\n", zone);
		} else {
			fprintf(fp, "time_advertisement=0\n");
		}
	}
	if (nvram_nmatch("1", "%s_80211k", ifname)) {
		fprintf(fp, "rrm_neighbor_report=%d\n", nvram_nmatch("1", "%s_rrm_neighbor_report", ifname) ? 1 : 0);
		fprintf(fp, "rrm_beacon_report=%d\n", nvram_nmatch("1", "%s_rrm_beacon_report", ifname) ? 1 : 0);
	}
	if (nvram_nmatch("1", "%s_mbo", ifname)) {
		fprintf(fp, "mbo=1\n");
		char mbo_pref[64];
		sprintf(mbo_pref, "%s_mbo_cell_data_conn_pref", ifname);
		fprintf(fp, "mbo_cell_data_conn_pref=%s\n", nvram_default_get(mbo_pref, "0"));
	}
#endif
	MAC80211DEBUG();
	char *v = nvram_nget("%s_config", ifname);
	fprintf(fp, "\n");
	if (v && *v)
		fprintf(fp, "%s", v);
	fprintf(fp, "\n");
	fclose(fp);
}

void addvhtcaps(const char *prefix, FILE *fp);

static char *makescanlist(const char *prefix, char *value)
{
	MAC80211DEBUG();
	char *clone = strdup(value);
	int len = strlen(clone);
	int i;
	char *new = NULL;
	struct wifi_channels *chan;
	char *country;
	country = nvram_default_get("wlan0_regdomain", "UNITED_STATES");
	chan = mac80211_get_channels_simple(prefix, getIsoName(country), 20, 255);
	/* format list */
	for (i = 0; i < len; i++) {
		if (clone[i] == ';')
			clone[i] = ' ';
		if (clone[i] == ',')
			clone[i] = ' ';
	}
	char *next;
	char var[128];
	foreach(var, clone, next)
	{
		char *sep = strchr(var, '-');
		if (!sep) {
			char *old = new;
			if (!new)
				asprintf(&new, "%s", var);
			else {
				asprintf(&new, "%s %s", old, var);
				free(old);
			}
		} else {
			*sep = 0;
			int start = atoi(var);
			int end = atoi(sep + 1);
			if (end < start) {
				int tmp = end;
				end = start;
				start = tmp;
			}
			if (end == start)
				continue;
			int soffset = -1;
			int eoffset = -1;
			i = 0;
			while (chan[i].freq != -1) {
				if (chan[i].freq >= start) {
					soffset = i;
					break;
				}
				i++;
			}
			i = 0;
			while (chan[i].freq != -1) {
				if (chan[i].freq <= end) {
					eoffset = i;
				}
				i++;
			}
			if (eoffset > -1 && soffset > -1 && eoffset > soffset) {
				for (i = soffset; i <= eoffset; i++) {
					char *old = new;
					if (!new)
						asprintf(&new, "%d", chan[i].freq);
					else {
						asprintf(&new, "%s %d", old, chan[i].freq);
						free(old);
					}
				}
			}
		}
	}
	free(clone);
	MAC80211DEBUG();
	return new;
}

static void supplicant_common_mesh(FILE *fp, char *prefix, char *ssidoverride, int isadhoc, int ismesh)
{
	MAC80211DEBUG();
	char nfreq[32];
	char nfreq2[32];
	char fwd[32];
	char ht[16];
	char sb[32];
	char bw[32];
	int freq;
	if (ismesh) {
		fprintf(fp, "\tmode=5\n");
		sprintf(fwd, "%s_mesh_fwding", prefix);
		fprintf(fp, "\tmesh_fwding=%d\n", atoi(nvram_default_get(fwd, "1")));
	} else
		fprintf(fp, "\tmode=1\n");
	// autochannel
	sprintf(nfreq, "%s_channel", prefix);
	sprintf(nfreq2, "%s_channel2", prefix);
	freq = atoi(nvram_default_get(nfreq, "0"));
	fprintf(fp, "\tfixed_freq=1\n");
	fprintf(fp, "\tfrequency=%d\n", freq);
	sprintf(bw, "%s_channelbw", prefix);
	sprintf(ht, "20");
	int iht, channeloffset;
	if (nvram_default_matchi(bw, 20, 20)) {
		sprintf(ht, "20");
	} else if (nvram_match(bw, "40") || nvram_match(bw, "2040") || nvram_match(bw, "80") || nvram_match(bw, "80+80") ||
		   nvram_match(bw, "160")) {
		const char *cht = get_channeloffset(prefix, &iht, &channeloffset);
		sprintf(ht, cht + 2);
		fprintf(fp, "\tht40=1\n");
	}
	if (!is_ath5k(prefix))
		// fprintf(fp, "ibss_ht_mode=HT%s\n",ht);
		fprintf(fp, "\thtmode=HT%s\n", ht);
	/* todo. consider mode configuration */
	if (nvram_match(bw, "80") || nvram_match(bw, "80+80") || nvram_match(bw, "160")) {
		fprintf(fp, "\tvht=1\n");
		if (has_ax(prefix))
			fprintf(fp, "\the=1\n");
	}
	if (nvram_match(bw, "80")) {
		fprintf(fp, "\tmax_oper_chwidth=1\n");
		fprintf(fp, "\tvht_center_freq1=%d\n", freq + (channeloffset * 5));
	}
	if (nvram_match(bw, "80+80")) {
		fprintf(fp, "\tmax_oper_chwidth=3\n");
		fprintf(fp, "\tvht_center_freq1=%d\n", freq + (channeloffset * 5));
		fprintf(fp, "\tvht_center_freq2=%d\n",
			nvram_geti(nfreq2)); // todo
	}
	if (nvram_match(bw, "160")) {
		fprintf(fp, "\tmax_oper_chwidth=2\n");
		fprintf(fp, "\tvht_center_freq1=%d\n", freq + (channeloffset * 5));
	}
	if (isadhoc) {
		char *cellid = nvram_nget("%s_cellid", prefix);
		if (*cellid) {
			fprintf(fp, "\tbssid=%s\n", cellid);
		}
#if defined(HAVE_MAKSAT) || defined(HAVE_TMK) || defined(HAVE_BKM)
		else {
			char cellidssid[5];
			memset(cellidssid, 0, 5);
			strncpy(cellidssid, ssidoverride, 5);
			fprintf(fp, "\tbssid=02:%02x:%02x:%02x:%02x:%02x\n", cellidssid[0], cellidssid[1], cellidssid[2],
				cellidssid[3], cellidssid[4]);
		}
#endif
	}
	MAC80211DEBUG();
}

void addbssid(FILE *fp, char *prefix);
void eap_sta_key_mgmt(FILE *fp, char *prefix);
void eap_sta_config(FILE *fp, char *prefix, char *ssidoverride, int addvht);

void setupSupplicant_ath9k(const char *prefix, char *ssidoverride, int isadhoc)
{
#ifdef HAVE_REGISTER
	if (!isregistered())
		return;
#endif
	MAC80211DEBUG();
	char akm[32];
	int i;
	char mcr[32];
	char ft[32];
	char mfp[32];
	char *mrate;
	char eapol[32];
	sprintf(eapol, "%s_eapol_version", prefix);
	sprintf(akm, "%s_akm", prefix);
	nvram_default_get(akm, "disabled");
	sprintf(ft, "%s_ft", prefix);
	sprintf(mfp, "%s_mfp", prefix);
	const int _has_wpa3 = has_wpa3(prefix);
	const int ispsk2 = nvhas(akm, "psk2");
	const int ispsk = nvhas(akm, "psk");
	const int ispsk3 = _has_wpa3 ? nvhas(akm, "psk3") : 0;
	const int ispsk2sha256 = _has_wpa3 ? nvhas(akm, "psk2-sha256") : 0;
	const int isleap = nvhas(akm, "leap");
	const int ispeap = nvhas(akm, "peap");
	const int istls = nvhas(akm, "tls");
	const int isttls = nvhas(akm, "ttls");
	const int ismesh = nvram_nmatch("mesh", "%s_mode", prefix);
	if (ispsk)
		nvram_nseti(1, "%s_psk", prefix);
	if (ispsk2)
		nvram_nseti(1, "%s_psk2", prefix);
	if (ispsk2sha256)
		nvram_nseti(1, "%s_psk2-sha256", prefix);
	if (ispsk3)
		nvram_nseti(1, "%s_psk3", prefix);
	MAC80211DEBUG();
	check_cryptomod(prefix);
	if (ispsk || ispsk2 || ispsk3 || ispsk2sha256) {
		char fstr[32];
		char psk[32];
		if (!strncmp(prefix, "wlan0", 4))
			led_control(LED_SEC0, LED_ON);
		if (!strncmp(prefix, "wlan1", 4))
			led_control(LED_SEC1, LED_ON);
		sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
		FILE *fp = fopen(fstr, "wb");
		if (!ismesh) {
			if (isadhoc)
				fprintf(fp, "ap_scan=2\n");
			else if (!ismesh)
				fprintf(fp, "ap_scan=1\n");
		}
		fprintf(fp, "ctrl_interface=/var/run/wpa_supplicant\n");
		fprintf(fp, "fast_reauth=1\n");
		fprintf(fp, "eapol_version=%s\n", nvram_default_get(eapol, "1"));
		if (ispsk3)
			fprintf(fp, "sae_groups=19 20 21\n");
		//              if (ismesh)
		//                      fprintf(fp, "user_mpm=1\n");
		fprintf(fp, "network={\n");
		char *netmode = nvram_nget("%s_net_mode", prefix);
		char *channelbw = nvram_nget("%s_channelbw", prefix);
		if (strcmp(netmode, "ac-only") && strcmp(netmode, "acn-mixed") && strcmp(netmode, "ax-only") &&
		    strcmp(netmode, "xacn-mixed") && strcmp(netmode, "mixed") && strcmp(netmode, "axg-only")) {
			fprintf(fp, "\tdisable_vht=1\n");
			if (has_ax(prefix))
				fprintf(fp, "\tdisable_he=1\n");
		}
		if (has_ax(prefix)) {
			if (strcmp(netmode, "ax-only") && strcmp(netmode, "xacn-mixed") && strcmp(netmode, "mixed") &&
			    strcmp(netmode, "axg-only")) {
				fprintf(fp, "\tdisable_he=1\n");
			}
		}

		if (strcmp(netmode, "n-only") && strcmp(netmode, "n2-only") && strcmp(netmode, "ac-only") &&
		    strcmp(netmode, "acn-mixed") && strcmp(netmode, "xacn-mixed") && strcmp(netmode, "ax-only") &&
		    strcmp(netmode, "n5-only") && strcmp(netmode, "na-only") && strcmp(netmode, "ng-only") &&
		    strcmp(netmode, "mixed") && strcmp(netmode, "axg-only")) {
			fprintf(fp, "\tdisable_ht=1\n");
		} else {
			if (!is_ath5k(prefix))
				fprintf(fp, "\tsmps=%d\n", nvram_default_ngeti(0, "%s_smps", prefix));
		}
		if (atoi(channelbw) < 40) {
			fprintf(fp, "\tdisable_ht40=1\n");
		}

		addvhtcaps(prefix, fp);
		if (!ssidoverride)
			ssidoverride = nvram_nget("%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", ssidoverride);
		if (isadhoc || ismesh) {
			supplicant_common_mesh(fp, prefix, ssidoverride, isadhoc, ismesh);
		} else {
			addbssid(fp, prefix);
			fprintf(fp, "\tscan_ssid=1\n");
		}
		/*		char scanlist[32];
		sprintf(scanlist, "%s_scanlist", prefix);
		char *sl = nvram_default_get(scanlist, "default");
		if (strcmp(sl, "default")) {
			char *scanlist = makescanlist(prefix, sl);
			fprintf(fp, "\tscan_freq=%s\n", scanlist);
			free(scanlist);
		}*/

		if (nvram_nmatch("simple", "%s_bgscan_mode", prefix) || nvram_nmatch("learn", "%s_bgscan_mode", prefix)) {
			int bgscan_short_int = nvram_ngeti("%s_bgscan_short_int", prefix);
			int bgscan_threshold = nvram_ngeti("%s_bgscan_threshold", prefix);
			int bgscan_long_int = nvram_ngeti("%s_bgscan_long_int", prefix);

			if (!bgscan_short_int || bgscan_short_int == 0)
				bgscan_short_int = 30;
			if (!bgscan_threshold || bgscan_threshold == 0)
				bgscan_threshold = -45;
			if (!bgscan_long_int || bgscan_long_int == 0)
				bgscan_long_int = 300;
			if (nvram_nmatch("simple", "%s_bgscan_mode", prefix))
				fprintf(fp, "\tbgscan=\"simple:%d:%d:%d\"\n", bgscan_short_int, bgscan_threshold, bgscan_long_int);
			else {
				char db[32];
				sprintf(db, "/tmp/%s_bgscan.db", prefix);
				fprintf(fp, "\tbgscan=\"learn:%d:%d:%d:%s\"\n", bgscan_short_int, bgscan_threshold, bgscan_long_int,
					db);
			}
		}
		//#ifdef HAVE_UNIWIP
		//              fprintf(fp, "\tbgscan=\"simple:30:-45:300\"\n");
		//#endif
		// fprintf (fp, "\tmode=0\n");
		fprintf(fp, "\tkey_mgmt=");
		if (ispsk2 || ispsk)
			fprintf(fp, "WPA-PSK ");
		if (_has_wpa3 && ispsk2sha256)
			fprintf(fp, "WPA-PSK-SHA256 ");
		if (_has_wpa3 && ispsk3)
			fprintf(fp, "SAE ");
#ifdef HAVE_80211R
		if (_has_wpa3 && nvram_matchi(ft, 1) && ispsk3)
			fprintf(fp, "FT-SAE ");
		if (nvram_matchi(ft, 1) && (ispsk2 || ispsk))
			fprintf(fp, "FT-PSK ");
#endif
		fprintf(fp, "\n");
		char pwstring[128] = { 0 };
		char grpstring[128] = { 0 };
		get_pairwise(prefix, pwstring, grpstring, isadhoc, ismesh);
#ifdef HAVE_80211W
		if (nvram_default_matchi(mfp, 1, 0) || ((ispsk2sha256 || ispsk3) && (!ispsk2 && !ispsk)))
			fprintf(fp, "\tieee80211w=2\n");
		else if (nvram_default_matchi(mfp, -1, 0) || ispsk3 || ispsk2sha256) {
			fprintf(fp, "\tieee80211w=1\n");
		} else if (nvram_default_matchi(mfp, 0, 0))
			fprintf(fp, "\tieee80211w=0\n");
#endif
		if (ismesh)
			fprintf(fp, "\tnoscan=1\n");

		if (!*pwstring) {
			sprintf(psk, "%s_crypto", prefix);
			if (nvram_match(psk, "aes")) {
				nvram_nseti(1, "%s_ccmp", prefix);
				fprintf(fp, "\tpairwise=CCMP\n");
#if defined(HAVE_MAKSAT) || defined(HAVE_TMK) || defined(HAVE_BKM)
				if (isadhoc)
					fprintf(fp, "\tgroup=CCMP\n");
				else
#endif
					if (ismesh)
					fprintf(fp, "\tgroup=CCMP\n");
				else
					fprintf(fp, "\tgroup=CCMP TKIP\n");
			}
			if (nvram_match(psk, "tkip")) {
				nvram_nseti(1, "%s_tkip", prefix);
				fprintf(fp, "\tpairwise=TKIP\n");
				fprintf(fp, "\tgroup=TKIP\n");
			}
			if (nvram_match(psk, "ccmp-256")) {
				nvram_nseti(1, "%s_ccmp-256", prefix);
				fprintf(fp, "\tpairwise=CCMP-256\n");
				fprintf(fp, "\tgroup=CCMP-256\n");
			}
			if (nvram_match(psk, "gcmp")) {
				nvram_nseti(1, "%s_gcmp", prefix);
				fprintf(fp, "\tpairwise=GCMP\n");
				fprintf(fp, "\tgroup=GCMP\n");
			}
			if (nvram_match(psk, "gcmp-256")) {
				nvram_nseti(1, "%s_gcmp-256", prefix);
				fprintf(fp, "\tpairwise=GCMP-256\n");
				fprintf(fp, "\tgroup=GCMP-256\n");
			}
			if (nvram_match(psk, "tkip+aes")) {
				nvram_nseti(1, "%s_tkip", prefix);
				nvram_nseti(1, "%s_ccmp", prefix);
				fprintf(fp, "\tpairwise=CCMP TKIP\n");
				fprintf(fp, "\tgroup=CCMP TKIP\n");
			}
		} else {
			fprintf(fp, "\tpairwise=%s\n", pwstring);
			fprintf(fp, "\tgroup=%s\n", grpstring);
		}
		if (ispsk)
			nvram_nseti(1, "%s_psk", prefix);
		if (ispsk2)
			nvram_nseti(1, "%s_psk2", prefix);
		if (ispsk2sha256)
			nvram_nseti(1, "%s_psk2-sha256", prefix);
		if (ispsk3)
			nvram_nseti(1, "%s_psk3", prefix);
		if ((ispsk2 || ispsk2sha256 || ispsk3) && ispsk)
			fprintf(fp, "\tproto=WPA RSN\n");
		else if (ispsk)
			fprintf(fp, "\tproto=WPA\n");
		else if (ispsk2 || ispsk3 || ispsk2sha256)
			fprintf(fp, "\tproto=RSN\n");
		if (ispsk3 && !ispsk && !ispsk2 && !ispsk2sha256) {
			char *sae_key = nvram_nget("%s_sae_key", prefix);
			fprintf(fp, "\tsae_password=\"%s\"\n", sae_key);
		} else {
			char *wpa_psk = nvram_nget("%s_wpa_psk", prefix);
			if (strlen(wpa_psk) == 64)
				fprintf(fp, "\tpsk=%s\n", wpa_psk);
			else
				fprintf(fp, "\tpsk=\"%s\"\n", wpa_psk);
		}
		fprintf(fp, "}\n");
		if (ispsk3) {
#ifdef HAVE_SSID_PROTECTION
		if (nvram_nmatch("1","%s_ssid_protection",prefix))
			fprintf(fp, "ssid_protection=1");
#endif
		}
		char extra[32];
		sprintf(extra, "%s_supplicantext", prefix);
		fwritenvram(extra, fp);
		fclose(fp);
	} else if (ispeap || isleap || istls || isttls) {
		char fstr[32];
		char psk[64];
		char ath[64];
		if (!strncmp(prefix, "wlan0", 4))
			led_control(LED_SEC0, LED_ON);
		if (!strncmp(prefix, "wlan1", 4))
			led_control(LED_SEC1, LED_ON);
		sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
		FILE *fp = fopen(fstr, "wb");
		fprintf(fp, "ctrl_interface=/var/run/wpa_supplicant\n");
		fprintf(fp, "ap_scan=1\n");
		fprintf(fp, "fast_reauth=1\n");
		fprintf(fp, "eapol_version=%s\n", nvram_default_get(eapol, "1"));
		// fprintf (fp, "ctrl_interface_group=0\n");
		eap_sta_config(fp, prefix, ssidoverride, 1);

		char extra[32];
		sprintf(extra, "%s_supplicantext", prefix);
		fwritenvram(extra, fp);
		fclose(fp);
	} else if (nvram_match(akm, "disabled") || nvram_match(akm, "wep")) {
		char fstr[32];
		char psk[32];
		if (nvram_match(akm, "wep")) {
			if (!strncmp(prefix, "wlan0", 4))
				led_control(LED_SEC0, LED_ON);
			if (!strncmp(prefix, "wlan1", 4))
				led_control(LED_SEC1, LED_ON);
		}
		sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
		FILE *fp = fopen(fstr, "wb");
		fprintf(fp, "ctrl_interface=/var/run/wpa_supplicant\n");
		if (!ismesh) {
			if (isadhoc)
				fprintf(fp, "ap_scan=2\n");
			else if (!ismesh)
				fprintf(fp, "ap_scan=1\n");
		}
		char *netmode = nvram_nget("%s_net_mode", prefix);
		char *channelbw = nvram_nget("%s_channelbw", prefix);
		fprintf(fp, "network={\n");

		if (strcmp(netmode, "ac-only") && strcmp(netmode, "acn-mixed") && strcmp(netmode, "ax-only") &&
		    strcmp(netmode, "xacn-mixed") && strcmp(netmode, "mixed")) {
			fprintf(fp, "\tdisable_vht=1\n");
		}

		if (strcmp(netmode, "n-only") && strcmp(netmode, "n2-only") && strcmp(netmode, "ac-only") &&
		    strcmp(netmode, "acn-mixed") && strcmp(netmode, "ax-only") && strcmp(netmode, "xacn-mixed") &&
		    strcmp(netmode, "n5-only") && strcmp(netmode, "na-only") && strcmp(netmode, "ng-only") &&
		    strcmp(netmode, "mixed") && strcmp(netmode, "axg-only")) {
			fprintf(fp, "\tdisable_ht=1\n");
		} else {
			if (atoi(channelbw) < 40) {
				fprintf(fp, "\tdisable_ht40=1\n");
			}
		}
		addvhtcaps(prefix, fp);
		if (!ssidoverride)
			ssidoverride = nvram_nget("%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", ssidoverride);
#ifdef HAVE_UNIWIP
		fprintf(fp, "\tbgscan=\"simple:30:-45:300\"\n");
#endif
		if (ismesh || isadhoc) {
			supplicant_common_mesh(fp, prefix, ssidoverride, isadhoc, ismesh);
		} else {
			addbssid(fp, prefix);
			fprintf(fp, "\tscan_ssid=1\n");
		}
		fprintf(fp, "\tkey_mgmt=NONE\n");
		if (nvram_match(akm, "wep")) {
			int cnt = 0;
			char *authmode = nvram_nget("%s_authmode", prefix);
			if (!strcmp(authmode, "shared"))
				fprintf(fp, "auth_alg=SHARED\n");
			else
				fprintf(fp, "auth_alg=OPEN\n");
			for (i = 1; i < 5; i++) {
				char *athkey = nvram_nget("%s_key%d", prefix, i);
				if (athkey != NULL && *athkey) {
					fprintf(fp, "wep_key%d=%s\n", i - 1,
						athkey); // setup wep
				}
			}

			sprintf(psk, "%s", nvram_nget("%s_key", prefix));
			fprintf(fp, "wep_tx_keyidx=%d\n", atoi(psk) - 1);
		}
		fprintf(fp, "}\n");
		char extra[32];
		sprintf(extra, "%s_supplicantext", prefix);
		fwritenvram(extra, fp);
		fclose(fp);
	}
	MAC80211DEBUG();
}

int vhtcaps_main(int argc, char *argv[])
{
	char *maininterface = argv[1];

	char bw[32];
	sprintf(bw, "%s_channelbw", maininterface);
	int usebw = 20;
	if (nvram_matchi(bw, 40))
		usebw = 40;
	if (nvram_matchi(bw, 10))
		usebw = 10;
	if (nvram_matchi(bw, 5))
		usebw = 5;
	if (nvram_matchi(bw, 2040))
		usebw = 40;
	if (nvram_matchi(bw, 80))
		usebw = 80;
	if (nvram_matchi(bw, 160))
		usebw = 160;
	if (nvram_match(bw, "80+80"))
		usebw = 8080;
	char shortgi[32];
	sprintf(shortgi, "%s_shortgi", maininterface);
	char mubf[32];
	sprintf(mubf, "%s_mubf", maininterface);
	char subf[32];
	sprintf(subf, "%s_subf", maininterface);
	char *caps = mac80211_get_vhtcaps(maininterface, nvram_default_matchi(shortgi, 1, 1) ? 1 : 0,
					  (usebw == 80 || usebw == 160 || usebw == 8080) ? 1 : 0, usebw == 160 ? 1 : 0,
					  usebw == 8080 ? 1 : 0, nvram_default_matchi(subf, 1, DEFAULT_BF),
					  nvram_default_matchi(mubf, 1, DEFAULT_BF));
	fprintf(stdout, "%s: caps = %s\n", argv[0], caps);
}

extern void do_hostapd(char *fstr, char *prefix);
void ath9k_start_supplicant(int count, char *prefix)
{
	// erst hostapd starten fuer repeater mode
	// dann wpa_supplicant mit link zu hostapd
	// dann bridgen und konfiguriren
	MAC80211DEBUG();
	char *next;
	char var[80];
	char fstr[32];
	char bridged[32];
	char mode[80];
	char dev[10];
	char power[32];
	char *apm, *vifs;
	char wl[32];
	char ctrliface[32] = "";
	char wifivifs[32];
	char tmp[256];
	char *background = "-B";
	int debug;
	char subinterface[32];
	char net[32];
	char wmode[32];
	int ctrlifneeded = 0;
	char wif[10];
	sprintf(wif, "phy%d", get_ath9k_phy_idx(count));
	sprintf(wl, "%s_mode", prefix);
	sprintf(dev, "%s", prefix);
	sprintf(net, "%s_net_mode", dev);
	char *netmode = nvram_default_get(net, "mixed");
	if (!strcmp(netmode, "disabled"))
		return;
	char bw[32];
	int isath5k = 0;
	int isath10k = 0;
	int ismt7615 = 0;
	int ismt7915 = 0;
	int ismt7921 = 0;
	isath5k = is_ath5k(dev);
	isath10k = is_ath10k(dev);
	ismt7615 = is_mt7615(dev);
	ismt7915 = is_mt7915(dev);
	ismt7921 = is_mt7921(dev);
	// set channelbw ht40 is also 20!
	sprintf(bw, "%s_channelbw", dev);
	char *driver = "ath9k";
	int bwmax = 20;
	int bwmin = 2;
	if (isath5k) {
		driver = "ath5k";
		bwmax = 40;
	} else if (isath10k)
		driver = "ath10k";
	else if (isath10k)
		driver = "ath11k";
	else if (ismt7615 || ismt7915 || ismt7921) {
		bwmin = 5;
		driver = "mt76";
	}
	int chanbw = nvram_geti(bw);
	if (chanbw < bwmin)
		chanbw = bwmin;
	if (chanbw > bwmax)
		chanbw = bwmax;
	setchanbw(wif, driver, chanbw);

	apm = nvram_safe_get(wl);
	sprintf(wifivifs, "%s_vifs", prefix);
	sprintf(power, "%s_txpwrdbm", prefix);
	vifs = nvram_safe_get(wifivifs);
	sprintf(subinterface, "-i%s", dev);
	if (has_ad(dev))
		sprintf(subinterface, "-igiwifi0");
	sprintf(wmode, "%s_mode", dev);
	sprintf(bridged, "%s_bridged", dev);
	debug = nvram_ngeti("%s_wpa_debug", dev);
	if (debug == 1)
		background = "-Bds";
	else if (debug == 2)
		background = "-Bdds";
	else if (debug == 3)
		background = "-Bddds";
	int wet = 0;
#ifndef HAVE_RELAYD
	wet = nvram_match(wmode, "wet");
#endif
	char pid[64];
	sprintf(pid, "/var/run/%s_wpa_supplicant.pid", dev);
	{
		char pw[32];
		sprintf(pw, "%d", nvram_default_geti(power, 16) * 100);
		eval("iw", "phy", wif, "set", "txpower", "fixed", pw);
	}

	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wdssta_mtik") && strcmp(apm, "infra") &&
	    strcmp(apm, "mesh") && strcmp(apm, "tdma") && strcmp(apm, "wet")) {
		sprintf(fstr, "/tmp/%s_hostap.conf", dev);
		do_hostapd(fstr, dev);
	} else {
		if (*vifs) {
			int ctrl = 0;
			int last = 0;
			foreach(var, vifs, next)
			{
				ctrl++;
				if (nvram_nmatch("disabled", "%s_net_mode", var) || nvram_nmatch("disabled", "%s_mode", var))
					continue;
				last = ctrl;
				if (nvram_nmatch("ap", "%s_mode", var) || nvram_nmatch("wdsap", "%s_mode", var))
					break;
			}
			ctrl = last;
			if (ctrl == 0)
				goto skip;
			if (!nvram_match(wmode, "mesh")) {
				/* do not start hostapd before wpa_supplicant in mesh mode, it will fail to initialize the ap interface once mesh is running */
				sprintf(fstr, "/tmp/%s_hostap.conf", dev);
				do_hostapd(fstr, dev);
			}
			sprintf(ctrliface, "/var/run/hostapd/%s.%d", dev, ctrl);
			sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", dev);
			if (!nvram_match(wmode, "mesh") && !nvram_match(wmode, "infra")) {
				if ((nvram_match(wmode, "wdssta") || nvram_match(wmode, "mesh") ||
				     nvram_match(wmode, "wdsta_mtik") || wet) &&
				    nvram_matchi(bridged, 1))
					log_eval("wpa_supplicant", "-P", pid, "-b", getBridge(dev, tmp), background, "-Dnl80211",
						 subinterface, "-H", ctrliface, "-c", fstr);
				else
					log_eval("wpa_supplicant", "-P", pid, background, "-Dnl80211", subinterface, "-H",
						 ctrliface, "-c", fstr);
			} else {
				/* for mesh mode we dont need ctrl interface since it has a static channel configuration */
				if (nvram_matchi(bridged, 1))
					log_eval("wpa_supplicant", "-P", pid, "-b", getBridge(dev, tmp), background, "-Dnl80211",
						 subinterface, "-c", fstr);
				else
					log_eval("wpa_supplicant", "-P", pid, background, "-Dnl80211", subinterface, "-c", fstr);
			}
			if (nvram_match(wmode, "mesh") || nvram_match(wmode, "infra")) {
				/* now start hostapd once wpa_supplicant has been started */
				sprintf(fstr, "/tmp/%s_hostap.conf", dev);
				do_hostapd(fstr, dev);
			}
		} else {
skip:;
			sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", dev);
			if (nvram_match(wmode, "sta") || nvram_match(wmode, "wdssta") || nvram_match(wmode, "wdssta_mtik") || wet ||
			    nvram_match(wmode, "infra") || nvram_match(wmode, "mesh")) {
				if ((nvram_match(wmode, "wdssta") || nvram_match(wmode, "mesh") ||
				     nvram_match(wmode, "wdsta_mtik") || wet) &&
				    nvram_matchi(bridged, 1)) {
					log_eval("wpa_supplicant", "-P", pid, "-b", getBridge(dev, tmp), background, "-Dnl80211",
						 subinterface, "-c", fstr);
				} else {
					log_eval("wpa_supplicant", "-P", pid, background, "-Dnl80211", subinterface, "-c", fstr);
				}
			}
		}
	}
	if (has_ad(dev))
		sprintf(dev, "giwifi0");
#ifdef HAVE_RELAYD
	if (strcmp(apm, "sta") && strcmp(apm, "wet")) {
#else
	if (strcmp(apm, "sta")) {
#endif
		char bridged[32];
		sprintf(bridged, "%s_bridged", dev);
		if (nvram_default_matchi(bridged, 1, 1)) {
			eval("ifconfig", dev, "0.0.0.0", "up");
			br_add_interface(getBridge(dev, tmp), dev);
			eval("ifconfig", dev, "0.0.0.0", "up");
		} else {
			eval("ifconfig", dev, "mtu", getMTU(dev));
			eval("ifconfig", dev, "txqueuelen", getTXQ(dev));
			eval("ifconfig", dev, nvram_nget("%s_ipaddr", dev), "netmask", nvram_nget("%s_netmask", dev), "up");
		}
	} else {
#ifdef HAVE_RELAYD
		if (!strcmp(apm, "wet")) {
			eval("ifconfig", dev, "0.0.0.0", "up");
			//                      sysprintf("relayd -I %s -I %s -D -B", getBridge(dev),
			//                                dev);
		}
#endif

		char bridged[32];
		sprintf(bridged, "%s_bridged", dev);
		if (nvram_default_matchi(bridged, 0, 1)) {
			eval("ifconfig", dev, "mtu", getMTU(dev));
			eval("ifconfig", dev, "txqueuelen", getTXQ(dev));
			eval("ifconfig", dev, nvram_nget("%s_ipaddr", dev), "netmask", nvram_nget("%s_netmask", dev), "up");
		}
	}

	if (*vifs) {
		foreach(var, vifs, next)
		{
			sprintf(mode, "%s_mode", var);
			char *m2 = nvram_safe_get(mode);
			char bridged[32];
			sprintf(bridged, "%s_bridged", var);
			if (!strcmp(m2, "mesh")) {
				sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", var);
				sprintf(subinterface, "-i%s", var);
				if (nvram_matchi(bridged, 1))
					log_eval("wpa_supplicant", "-b", getBridge(var, tmp), background, "-Dnl80211", subinterface,
						 "-c", fstr);
				else {
					log_eval("wpa_supplicant", background, "-Dnl80211", subinterface, "-c", fstr);
				}
			}

			if (strcmp(m2, "sta")) {
				if (nvram_default_matchi(bridged, 1, 1)) {
					eval("ifconfig", dev, "0.0.0.0", "up");
					br_add_interface(getBridge(var, tmp), var);
				} else {
					eval("ifconfig", var, "mtu", getMTU(var));
					eval("ifconfig", var, "txqueuelen", getTXQ(var));
					eval("ifconfig", var, nvram_nget("%s_ipaddr", var), "netmask",
					     nvram_nget("%s_netmask", var), "up");
				}
			}
		}
	}
	if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap") || !strcmp(apm, "apup")) {
		int s;
		for (s = 1; s <= 10; s++) {
			char wdsvarname[32] = { 0 };
			char wdsdevname[32] = { 0 };
			char wdsmacname[32] = { 0 };
			char *wdsdev;
			char *hwaddr;
			sprintf(wdsvarname, "%s_wds%d_enable", dev, s);
			sprintf(wdsdevname, "%s_wds%d_if", dev, s);
			sprintf(wdsmacname, "%s_wds%d_hwaddr", dev, s);
			wdsdev = nvram_safe_get(wdsdevname);
			if (!*wdsdev)
				continue;
			if (nvram_matchi(wdsvarname, 0))
				continue;
			hwaddr = nvram_safe_get(wdsmacname);
			if (*hwaddr) {
				eval("iw", wif, "interface", "add", wdsdev, "type", "wds");
#ifdef HAVE_IPQ6018
				eval("tc", "qdisc", "replace", "dev", wdsdev, "root", "noqueue");
#endif
				eval("iw", "dev", wdsdev, "set", "peer", hwaddr);
				eval("ifconfig", wdsdev, "0.0.0.0", "up");
			}
		}
	}
	{
		char pw[32];
		sprintf(pw, "%d", nvram_default_geti(power, 16) * 100);
		eval("iw", "dev", dev, "set", "txpower", "fixed", pw);
	}
	if (is_ath10k(dev)) {
		char wl_po[32];
		sprintf(wl_po, "%s_power_override", dev);
		sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath10k/power_override", nvram_default_get(wl_po, "0"), wif);
	}
	char wl_intmit[32];
	sprintf(wl_intmit, "%s_intmit", dev);
	char wl_qboost[32];
	sprintf(wl_qboost, "%s_qboost", dev);
	char wl_autoburst[32];
	sprintf(wl_autoburst, "%s_autoburst", dev);
	char wl_sifs_trigger_time[32];
	sprintf(wl_sifs_trigger_time, "%s_sifs_trigger_time", dev);
	if (is_ath10k(dev)) {
		if (has_qboost(dev)) {
			sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath10k/qboost_enable",
				  nvram_default_get(wl_qboost, "0"), wif);
			if (has_qboost_tdma(dev)) {
				sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath10k/sifs_trigger_time",
					  nvram_default_get(wl_sifs_trigger_time, "0"), wif);
			}
		}
		if (nvram_match("experimental", "1")) {
			if (has_wave2(dev)) {
				sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath10k/dynamic_auto_burst",
					  nvram_default_get(wl_autoburst, "0"), wif);
			}
		}
		sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath10k/ani_enable", nvram_default_get(wl_intmit, "0"), wif);
	}
	if (is_ath9k(dev))
		sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath9k/ani", nvram_default_get(wl_intmit, "1"), wif);

	MAC80211DEBUG();
	if (has_airtime_fairness(dev)) {
		char atf[32];
		sprintf(atf, "%s_atf", dev);
		sysprintf("echo %d > /sys/kernel/debug/ieee80211/%s/airtime_flags", nvram_default_matchi(atf, 1, 1) ? 3 : 0, wif);
	}
	MAC80211DEBUG();
}
#endif
