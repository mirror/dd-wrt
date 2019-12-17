/*
 * madwifi_ath9k.c 
 * Copyright (C) 2010 Christian Scheele <chris@dd-wrt.com>
 * Copyright (C) 2010-2019 Sebastian Gottschall <brainslayer@dd-wrt.com>
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
#include <wlutils.h>
#include <utils.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>
#include <glob.h>

#include <services.h>

#include "unl.h"
#include <nl80211.h>

void check_cryptomod(char *prefix);

void setupHostAP_ath9k(char *maininterface, int isfirst, int vapid, int aoss);
static void setupSupplicant_ath9k(char *prefix, char *ssidoverride, int isadhoc);
void setupHostAP_generic_ath9k(char *prefix, FILE * fp, int isrepeater, int aoss);

static const char *gethtmode(char *prefix)
{
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
	if (nvram_matchi(bw, 2040))
		usebw = 40;
	if (nvram_matchi(bw, 80))
		usebw = 80;
	if (nvram_matchi(bw, 160))
		usebw = 160;
	if (nvram_match(bw, "80+80"))
		usebw = 8080;

	if ((!strcmp(netmode, "ng-only") ||	//
	     !strcmp(netmode, "na-only") ||	//
	     !strcmp(netmode, "n2-only") ||	//
	     !strcmp(netmode, "n5-only") ||	//
	     !strcmp(netmode, "ac-only") ||	//
	     !strcmp(netmode, "acn-mixed") ||	//
	     !strcmp(netmode, "mixed"))
	    && strcmp(akm, "wep")) {

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
			ht = "80";
			break;
		case 8080:
			ht = "80+80";
			break;
		case 160:
			ht = "160";
			break;
		case 20:
		default:
			ht = "HT20";
			break;
		}
	} else {
		ht = "NOHT";
	}
	return ht;
}

static void load_compressor(void)
{
	insmod("xxhash");
	insmod("zstd_compress");
	insmod("zstd_decompress");
	insmod("lzo_compress");
	insmod("lzo_decompress");
	insmod("lz4_compress");
	insmod("lz4_decompress");
	insmod("lzma_compress");
	insmod("lzma_decompress");
	/* this module uses weak symbols, which allows to run the whole mac80211 code 
	 * without any bindings to external libraries which arent in use. this will reduce
	 * resource usage since we dont need to load all the libraries all the time
	 * specially with zstd this is neccessary right now
	 */
	insmod("mac80211_compress");
}

static void setRTS(char *use)
{
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
/*	if (nvram_nmatch("None", "%s_protmode", use))
		eval("iwpriv", use, "protmode", "0");
	if (nvram_nmatch("CTS", "%s_protmode", use))
		eval("iwpriv", use, "protmode", "1");
	if (nvram_nmatch("RTS/CTS", "%s_protmode", use))
		eval("iwpriv", use, "protmode", "2"); */
}

void delete_ath9k_devices(char *physical_iface)
{
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
			br_del_interface(getBridge("ath2", tmp), dev);
		} else
			br_del_interface(getBridge(dev, tmp), dev);
		eval("ifconfig", dev, "down");
		eval("iw", dev, "del");
	}
}

void deconfigure_single_ath9k(int count)
{
	int idx = get_ath9k_phy_idx(count);
	fprintf(stderr, "ath9k deconfigure_single: phy%d ath%d\n", idx, count);
	char wif[10];
	sprintf(wif, "phy%d", idx);
	sysprintf("rm -f /tmp/ath%d_hostapd.conf", idx);
	sysprintf("rm -f /tmp/ath%d_wpa_supplicant.conf", idx);
	delete_ath9k_devices(wif);
}

void configure_single_ath9k(int count)
{
	char *next;
	char var[80];
	char mode[80];
	int cnt = 0;
	char dev[10];
	char wif[10];
	int phy_idx = get_ath9k_phy_idx(count);
	char wl[16];
	char bw[32];
	char channel[16];
	char ssid[16];
	char net[16];
	char atf[16];
	char wifivifs[16];
	char broadcast[16];
	char sens[32];
	char basedev[16];
	char diversity[32];
	char athmac[16];
	char wl_poll[32];
	char rxantenna[32];
	char txantenna[32];
	static int vapcount = 0;
	int isath5k = 0;
	char *apm;
	char isolate[32];
	char primary[32] = { 0 };
	char regdomain[16];
	char *country;
	int isadhoc = 0;

	sprintf(dev, "ath%d", count);
	isath5k = is_ath5k(dev);
	// sprintf(regdomain, "%s_regdomain", dev);
	// country = nvram_default_get(regdomain, "US");
	// sysprintf("iw reg set %s", getIsoName(country));
	// // sleep(3);
	if (count == 0)
		vapcount = 0;
	sprintf(wif, "phy%d", phy_idx);
	sprintf(wifivifs, "ath%d_vifs", count);
	fprintf(stderr, "ath9k configure_single: phy%d ath%d\n", phy_idx, count);
	sprintf(channel, "ath%d_channel", count);
	sprintf(sens, "ath%d_distance", count);
	sprintf(diversity, "ath%d_diversity", count);
	sprintf(athmac, "ath%d_hwaddr", count);
	sprintf(rxantenna, "ath%d_rxantenna", count);
	sprintf(txantenna, "ath%d_txantenna", count);
	// create base device
	cprintf("configure base interface %d / %s\n", count, dev);
	sprintf(net, "%s_net_mode", dev);
	char *netmode = nvram_default_get(net, "mixed");
	if (!strcmp(netmode, "disabled"))
		return;

	if (has_airtime_fairness(dev)) {
		sprintf(atf, "%s_atf", dev);
		sysprintf("echo %d > /sys/kernel/debug/ieee80211/%s/airtime_flags", nvram_default_matchi(atf, 1, 1) ? 3 : 0, wif);
	}
	// set channelbw ht40 is also 20!
	sprintf(bw, "%s_channelbw", dev);
	if (isath5k) {
		if (nvram_matchi(bw, 5))
			sysprintf("echo 5 > /sys/kernel/debug/ieee80211/%s/ath5k/bwmode", wif);
		else if (nvram_matchi(bw, 10))
			sysprintf("echo 10 > /sys/kernel/debug/ieee80211/%s/ath5k/bwmode", wif);
		else if (nvram_matchi(bw, 40))
			sysprintf("echo 40 > /sys/kernel/debug/ieee80211/%s/ath5k/bwmode", wif);
		else
			sysprintf("echo 20 > /sys/kernel/debug/ieee80211/%s/ath5k/bwmode", wif);
	} else {
		if (nvram_matchi(bw, 5))
			sysprintf("echo 5 > /sys/kernel/debug/ieee80211/%s/ath9k/chanbw", wif);
		else if (nvram_matchi(bw, 10))
			sysprintf("echo 10 > /sys/kernel/debug/ieee80211/%s/ath9k/chanbw", wif);
		else
			sysprintf("echo 20 > /sys/kernel/debug/ieee80211/%s/ath9k/chanbw", wif);
	}
	char wl_intmit[32];
	sprintf(wl_intmit, "%s_intmit", dev);
	char wl_qboost[32];
	sprintf(wl_qboost, "%s_qboost", dev);
	char wl_sifs_trigger_time[32];
	sprintf(wl_sifs_trigger_time, "%s_sifs_trigger_time", dev);
#ifdef HAVE_ATH10K
	if (is_ath10k(dev)) {
		if (has_qboost(dev)) {
			sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath10k/qboost_enable", nvram_default_get(wl_qboost, "0"), wif);
			if (has_qboost_tdma(dev)) {
				sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath10k/sifs_trigger_time", nvram_default_get(wl_sifs_trigger_time, "0"), wif);
			}
		}
		sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath10k/ani_enable", nvram_default_get(wl_intmit, "0"), wif);
	} else
#endif
		sysprintf("echo %s > /sys/kernel/debug/ieee80211/%s/ath9k/ani", nvram_default_get(wl_intmit, "1"), wif);
#ifdef HAVE_REGISTER
	int cpeonly = iscpe();
#else
	int cpeonly = 0;
#endif
	int maxrxchain = mac80211_get_avail_rx_antenna(phy_idx);
	int maxtxchain = mac80211_get_avail_tx_antenna(phy_idx);
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
		mac80211_set_antennas(phy_idx, txchain, rxchain);

	sprintf(wl, "ath%d_mode", count);
	apm = nvram_default_get(wl, "ap");

	if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap") || !strcmp(apm, "sta")
	    || !strcmp(apm, "wet")) {

		eval("iw", wif, "interface", "add", dev, "type", "managed");

		strcpy(primary, dev);
	} else if (!strcmp(apm, "wdssta")) {
		eval("iw", wif, "interface", "add", dev, "type", "managed", "4addr", "on", "mtikwds", "off");
		strcpy(primary, dev);
	} else if (!strcmp(apm, "wdssta_mtik")) {
		eval("iw", wif, "interface", "add", dev, "type", "managed", "4addr", "on", "mtikwds", "on");
		strcpy(primary, dev);
	} else if (!strcmp(apm, "mesh")) {
		char akm[16];
		sprintf(akm, "%s_akm", dev);
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
		char akm[16];
		sprintf(akm, "%s_akm", dev);
		eval("iw", wif, "interface", "add", dev, "type", "ibss");
		if (nvhas(akm, "psk") || nvhas(akm, "psk2") || nvhas(akm, "psk3")) {
			// setup and join does wpa_supplicant
			isadhoc = 1;
		} else {
			cprintf("handle ibss join");
		}
		// still TBD ;-)
		// ifconfig ath0 up
		// iw dev ath0 ibss join AdHocNetworkName 2412
	}

	char macaddr[32];
	// interface is created at this point, so that should work
#if defined(HAVE_MVEBU) || defined(HAVE_IPQ806X)
	getWirelessMac(macaddr, count);
	set_hwaddr(dev, macaddr);
#else
	getMacAddr(dev, macaddr);
#endif
	nvram_set(athmac, macaddr);
	int distance = nvram_default_geti(sens, 2000);	// to meter
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

	cprintf("setup encryption");
	// setup encryption
	int isfirst = 1;
	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wdssta_mtik") && strcmp(apm, "wet") && strcmp(apm, "infra") && strcmp(apm, "mesh") && strcmp(apm, "tdma")) {
		setupHostAP_ath9k(dev, isfirst, 0, 0);
		isfirst = 0;
	} else {
		char *clonename = "def_whwaddr";
		if (!strcmp(apm, "sta"))
			clonename = "def_hwaddr";
		if (nvram_matchi("mac_clone_enable", 1)
		    && nvram_invmatch(clonename, "00:00:00:00:00:00")
		    && nvram_invmatch(clonename, "")) {
			set_hwaddr(dev, nvram_safe_get(clonename));
		}

		setupSupplicant_ath9k(dev, NULL, isadhoc);
	}
	char *vifs = nvram_safe_get(wifivifs);
	int countvaps = 1;
	foreach(var, vifs, next) {
		countvaps++;
	}
	if (countvaps < 4)
		countvaps = 4;
	if (countvaps > vapcount)
		vapcount = countvaps;
	int counter = 1;
	char compr[32];
	if (*vifs) {
		foreach(var, vifs, next) {
			if (nvram_nmatch("disabled", "%s_mode", var))
				continue;
			fprintf(stderr, "setup vifs %s %d\n", var, counter);
			// create the first main hostapd interface when this is repeater mode
			if (!nvram_nmatch("mesh", "%s_mode", var)) {
				if (isfirst)
					sysprintf("iw %s interface add %s.%d type managed", wif, dev, counter);
				setupHostAP_ath9k(dev, isfirst, counter, 0);
			} else {
				char akm[16];
				sprintf(akm, "%s.%d_akm", dev, counter);
				if (nvhas(akm, "psk") || nvhas(akm, "psk2") || nvhas(akm, "psk3"))
					sysprintf("iw %s interface add %s type mp", wif, var);
				else
					sysprintf("iw %s interface add %s type mp mesh_id %s", wif, var, nvram_nget("%s_ssid", var));
				setupSupplicant_ath9k(var, NULL, 0);
			}
			sprintf(compr, "%s_fc_th", var);
			char *threshold = nvram_default_get(compr, "512");	// minimum framesize frequired for compression
			sprintf(compr, "%s_fc", var);

			if (nvram_default_matchi(compr, 1, 0)) {
				load_compressor();
				eval("iw", "dev", var, "set", "compr", "lzo", threshold);
			} else if (nvram_default_matchi(compr, 2, 0)) {
				load_compressor();
				eval("iw", "dev", var, "set", "compr", "lzma", threshold);
			} else if (nvram_default_matchi(compr, 3, 0)) {
				load_compressor();
				eval("iw", "dev", var, "set", "compr", "lz4", threshold);
			} else if (nvram_default_matchi(compr, 4, 0)) {
				load_compressor();
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
	char *threshold = nvram_default_get(compr, "512");	// minimum framesize frequired for compression
	sprintf(compr, "%s_fc", dev);

	if (nvram_default_matchi(compr, 1, 0)) {
		load_compressor();
		eval("iw", "dev", dev, "set", "compr", "lzo", threshold);
	} else if (nvram_default_matchi(compr, 2, 0)) {
		load_compressor();
		eval("iw", "dev", dev, "set", "compr", "lzma", threshold);
	} else if (nvram_default_matchi(compr, 3, 0)) {
		load_compressor();
		eval("iw", "dev", dev, "set", "compr", "lz4", threshold);
	} else if (nvram_default_matchi(compr, 4, 0)) {
		load_compressor();
		eval("iw", "dev", dev, "set", "compr", "zstd", threshold);
	} else {
		eval("iw", "dev", dev, "set", "compr", "off");
	}

}

void get_pairwise(char *prefix, char *pwstring, char *grpstring, int isadhoc, int ismesh);

void setupHostAP_generic_ath9k(char *prefix, FILE * fp, int isrepeater, int aoss)
{
	int channel = 0;
	int channel2 = 0;
	int freq = 0;
	char nfreq[16];
	int freq2 = 0;
	char nfreq2[16];
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
	const char *country = getIsoName(nvram_default_get("ath0_regdomain", "UNITED_STATES"));
	if (!country)
		country = "DE";
	fprintf(fp, "country_code=%s\n", country);
	char *netmode = nvram_nget("%s_net_mode", prefix);
	if (isath5k || !(!strcmp(netmode, "n2-only") || !strcmp(netmode, "n5-only") || !strcmp(netmode, "ac-only") || !strcmp(netmode, "acn-mixed"))) {
		fprintf(fp, "tx_queue_data2_burst=2.0\n");
		fprintf(fp, "wmm_ac_be_txop_limit=64\n");
	} else {
		fprintf(fp, "tx_queue_data2_burst=0\n");
		fprintf(fp, "wmm_ac_be_txop_limit=0\n");
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
	if (nvram_matchi(bw, 2040))
		usebw = 40;
	if (nvram_matchi(bw, 80))
		usebw = 80;
	if (nvram_matchi(bw, 160))
		usebw = 160;
	if (nvram_match(bw, "80+80"))
		usebw = 8080;

	if (has_ac(prefix)) {
		if (strcmp(netmode, "acn-mixed") &&	//
		    strcmp(netmode, "ac-only ") &&	//
		    strcmp(netmode, "mixed")) {
			fprintf(fp, "ieee80211ac=0\n");
		}
	}
	if ((!strcmp(netmode, "ng-only") ||	//
	     !strcmp(netmode, "na-only") ||	//
	     !strcmp(netmode, "n2-only") ||	//
	     !strcmp(netmode, "n5-only") ||	//
	     !strcmp(netmode, "ac-only") ||	//
	     !strcmp(netmode, "acn-mixed") ||	//
	     !strcmp(netmode, "mixed"))
	    && strcmp(akm, "wep")
	    && !aoss) {

		if (strcmp(netmode, "mixed") && strcmp(netmode, "ng-only")
		    && strcmp(netmode, "na-only")) {
			if (!isath5k)
				fprintf(fp, "require_ht=1\n");
		}
		if (!isath5k && !has_ad(prefix)) {
			fprintf(fp, "ieee80211n=1\n");
			if (nvram_matchi(bw, 2040)) {
				fprintf(fp, "dynamic_ht40=1\n");
			} else {
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
		default:
			ht = "HT20";
			break;
		}
	} else {
		ht = "HT20";
	}
	char regdomain[16];
	sprintf(regdomain, "%s_regdomain", prefix);

	if (isrepeater) {
		// for ht40- take second channel otherwise hostapd is unhappy (and does not start)
		if (has_2ghz(prefix)) {
			if (iht == -1) {
				i = 4;	// in 2.4 mhz channel spacing is different
			}
		}
		if (has_5ghz(prefix)) {
			if (iht == -1) {
				i = 1;
			}
		}
		switch (usebw) {
		case 160:
			channel = 100;
			ht = "HT40+";
			iht = 1;
			channeloffset = 14;
			freq = 5500;
			break;
		case 80:
		case 8080:
			channel = 100;
			channel2 = 155;
			ht = "HT40+";
			channeloffset = 6;
			iht = 1;
			freq = 5500;
			break;
		case 40:
			ht = "HT40+";
			if (has_2ghz(prefix)) {
				channel = 6;
				freq = 2437;
			}
			if (has_5ghz(prefix)) {
				channel = 100;
				freq = 5500;
			}
			break;
		case 20:
			ht = "HT20";
		default:
			if (has_2ghz(prefix)) {
				channel = 6;
				freq = 2437;
			}
			if (has_5ghz(prefix)) {
				channel = 100;
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
				channel = 1;
				freq = 53320;
			} else {
				struct mac80211_ac *acs;
				fprintf(stderr, "call mac80211autochannel for interface: %s\n", prefix);
				eval("ifconfig", prefix, "up");
				switch (usebw) {
				case 40:
					acs = mac80211autochannel(prefix, NULL, 2, 1, 0, AUTO_FORCEHT40);
					break;
				case 80:
					acs = mac80211autochannel(prefix, NULL, 2, 1, 0, AUTO_FORCEVHT80);
					break;
				case 160:
				case 8080:
					acs = mac80211autochannel(prefix, NULL, 2, 1, 0, AUTO_FORCEVHT160);
					break;
				default:
					acs = mac80211autochannel(prefix, NULL, 2, 1, 0, AUTO_ALL);
				}
				if (acs != NULL) {
					struct wifi_channels *chan = mac80211_get_channels_simple(prefix, country, usebw, 0xff);
					freq = acs->freq;
					channel = ieee80211_mhz2ieee(freq);
					fprintf(stderr, "mac80211autochannel interface: %s frequency: %d\n", prefix, freq);
					int i = 0;
					while (chan[i].freq != -1) {
						if (chan[i].freq == freq)
							break;
						i++;
					}
					switch (usebw) {
					case 40:
						if (chan[i].luu) {
							ht = "HT40-";
							iht = -1;
						} else if (chan[i].ull) {
							ht = "HT40+";
							iht = 1;
						}
						break;
					case 80:
					case 8080:
						if (chan[i].luu) {
							ht = "HT40-";
							channeloffset = 2;
							iht = -1;
						} else if (chan[i].ulu) {
							ht = "HT40+";
							iht = 1;
							channeloffset = 6;
						} else if (chan[i].lul) {
							ht = "HT40-";
							channeloffset = 6;
							iht = -1;
						} else if (chan[i].ull) {
							ht = "HT40+";
							iht = 1;
							channeloffset = 2;
						}
						break;
					case 160:
						if (chan[i].uuu) {
							ht = "HT40+";
							iht = 1;
							channeloffset = 14;
						} else if (chan[i].uul) {
							ht = "HT40+";
							iht = 1;
							channeloffset = 10;
						} else if (chan[i].ulu) {
							ht = "HT40+";
							iht = 1;
							channeloffset = 6;
						} else if (chan[i].ull) {
							ht = "HT40+";
							iht = 1;
							channeloffset = 2;
						} else if (chan[i].luu) {
							ht = "HT40-";
							iht = -1;
							channeloffset = 2;
						} else if (chan[i].lul) {
							ht = "HT40-";
							iht = -1;
							channeloffset = 6;
						} else if (chan[i].llu) {
							ht = "HT40-";
							iht = -1;
							channeloffset = 10;
						} else if (chan[i].lll) {
							ht = "HT40-";
							iht = -1;
							channeloffset = 14;
						}
						break;
					default:
					case 20:
						ht = "HT20";
						break;
					}
					free_mac80211_ac(acs);
				} else {
					if (has_2ghz(prefix)) {
						channel = 6;
						freq = 2437;
					}
					if (has_5ghz(prefix)) {
						switch (usebw) {
						case 8080:
						case 80:
							channel = 100;
							channel2 = 155;
							ht = "HT40+";
							iht = 1;
							channeloffset = 6;
							freq = 5500;
							break;
						case 160:
							channel = 100;
							ht = "HT40+";
							iht = 1;
							channeloffset = 14;
							freq = 5500;
							break;
						case 40:
							channel = 100;
							ht = "HT40+";
							iht = 1;
							freq = 5500;
							break;
						case 20:
						default:
							ht = "HT20";
							channel = 100;
							freq = 5500;
							break;

						}
					}
				}
			}
		} else {
			channel = ieee80211_mhz2ieee(freq);
			channel2 = ieee80211_mhz2ieee(freq2);
		}
	}
	if (!isath5k && !has_ad(prefix)) {
		char shortgi[32];
		sprintf(shortgi, "%s_shortgi", prefix);
		char greenfield[32];
		sprintf(greenfield, "%s_gf", prefix);
		caps = mac80211_get_caps(prefix, nvram_default_matchi(shortgi, 1, 1) ? 1 : 0, nvram_default_matchi(greenfield, 1, 0) ? 1 : 0);
		if (ht) {
			if (nvram_nmatch("1", "%s_smps", prefix) && has_static_smps(prefix))
				fprintf(fp, "ht_capab=[%s]%s[SMPS-STATIC]\n", ht, caps);
			else if (nvram_nmatch("2", "%s_smps", prefix) && has_dynamic_smps(prefix))
				fprintf(fp, "ht_capab=[%s]%s[SMPS-DYNAMIC]\n", ht, caps);
			else
				fprintf(fp, "ht_capab=[%s]%s\n", ht, caps);
		} else {
			if (nvram_nmatch("1", "%s_smps", prefix) && has_static_smps(prefix))
				fprintf(fp, "ht_capab=%s[SMPS-STATIC]\n", caps);
			else if (nvram_nmatch("2", "%s_smps", prefix) && has_dynamic_smps(prefix))
				fprintf(fp, "ht_capab=%s[SMPS-DYNAMIC]\n", caps);
		}
		free(caps);
	}
	if (has_ac(prefix) && has_5ghz(prefix)) {
		if ((!strcmp(netmode, "mixed") ||	//
		     !strcmp(netmode, "ac-only") || !strcmp(netmode, "acn-mixed"))) {
			char shortgi[32];
			sprintf(shortgi, "%s_shortgi", prefix);
			char mubf[32];
			sprintf(mubf, "%s_mubf", prefix);
			char subf[32];
			sprintf(subf, "%s_subf", prefix);
			caps =
			    mac80211_get_vhtcaps(prefix, nvram_default_matchi(shortgi, 1, 1) ? 1 : 0, (usebw == 80 || usebw == 160 || usebw == 8080) ? 1 : 0, usebw == 160 ? 1 : 0, usebw == 8080 ? 1 : 0,
						 nvram_default_matchi(subf, 1, 0), nvram_default_matchi(mubf, 1, 0));
			if (*caps) {
				fprintf(fp, "vht_capab=%s\n", caps);
				free(caps);
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

				if (!strcmp(netmode, "mixed")) {
					fprintf(fp, "ieee80211ac=1\n");
					fprintf(fp, "ieee80211d=1\n");
					fprintf(fp, "ieee80211h=1\n");
				}
				switch (usebw) {
				case 40:
					fprintf(fp, "vht_oper_chwidth=0\n");
					fprintf(fp, "vht_oper_centr_freq_seg0_idx=%d\n", channel + (2 * iht));
					break;
				case 80:
					fprintf(fp, "vht_oper_chwidth=1\n");
					fprintf(fp, "vht_oper_centr_freq_seg0_idx=%d\n", channel + (channeloffset * iht));
					break;
				case 160:
					fprintf(fp, "vht_oper_chwidth=2\n");
					fprintf(fp, "vht_oper_centr_freq_seg0_idx=%d\n", channel + (channeloffset * iht));
					break;
				case 8080:
					fprintf(fp, "vht_oper_chwidth=3\n");
					fprintf(fp, "vht_oper_centr_freq_seg0_idx=%d\n", channel + (channeloffset * iht));
					fprintf(fp, "vht_oper_centr_freq_seg1_idx=%d\n", channel2);
					break;
				default:
					fprintf(fp, "vht_oper_chwidth=0\n");
					break;

				}
			}

		}

	}

	if (has_ad(prefix)) {
		fprintf(fp, "hw_mode=ad\n");
	} else if (freq < 4000) {
		if (!strcmp(netmode, "b-only")) {
			fprintf(fp, "hw_mode=b\n");
			fprintf(fp, "supported_rates=10 20 55 110\n");
		} else if (!strcmp(netmode, "bg-mixed")) {
			fprintf(fp, "hw_mode=g\n");
			fprintf(fp, "basic_rates=10 20 55 60 110 120 240\n");
			fprintf(fp, "supported_rates=10 20 55 60 90 110 120 180 240 360 480 540\n");
		} else if (!strcmp(netmode, "mixed")) {
			fprintf(fp, "hw_mode=g\n");
			fprintf(fp, "basic_rates=10 20 55 60 110 120 240\n");
			fprintf(fp, "supported_rates=10 20 55 60 90 110 120 180 240 360 480 540\n");
		} else {
			fprintf(fp, "basic_rates=60 120 240\n");
			fprintf(fp, "supported_rates=60 90 120 180 240 360 480 540\n");
			fprintf(fp, "hw_mode=g\n");
		}
	} else {
		fprintf(fp, "hw_mode=a\n");
		fprintf(fp, "basic_rates=60 120 240\n");
		if (!strcmp(netmode, "a-only")) {
			fprintf(fp, "supported_rates=60 90 120 180 240 360 480 540\n");
		}

	}

	fprintf(fp, "channel=%d\n", channel);
	if (!has_ad(prefix))
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
	fprintf(fp, "\n");
}

static void setMacFilter(FILE * fp, char *iface)
{
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
		foreach(var, list, next) {
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
		foreach(var, list, next) {
			fprintf(out, "%s\n", var);
		}
		fclose(out);
	}

}

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
extern void addWPS(FILE * fp, char *prefix, int configured);
extern void setupHS20(FILE * fp, char *prefix);
void setupHostAPPSK(FILE * fp, char *prefix, int isfirst);
void setupHostAP_ath9k(char *maininterface, int isfirst, int vapid, int aoss)
{
#ifdef HAVE_REGISTER
	if (!isregistered())
		return;
#endif
	char psk[32];
	char akm[16];
	char mfp[16];
	char ft[16];
	char fstr[32];
	char preamble[16];
	char uapsd[16];
	FILE *fp = NULL;
	char *ssid;
	char nssid[16];
	char maxassoc[32];
	char ifname[10];
	int isrepeater = 0;
	unsigned char hwbuff[16];
	char macaddr[32];
	char *types;
	char *debug;
	char tmp[256];
	if (isfirst && vapid == 0) {
		sprintf(ifname, "%s", maininterface);
	} else {
		sprintf(ifname, "%s.%d", maininterface, vapid);
		isrepeater = 1;
	}
#ifdef HAVE_WZRHPAG300NH
	if (aoss) {
		if (!strncmp(ifname, "ath0", 4))
			sprintf(ifname, "aossg");
		else
			sprintf(ifname, "aossa");
	}
#else
	if (aoss)
		sprintf(ifname, "aoss");
#endif
	sprintf(akm, "%s_akm", ifname);
	sprintf(ft, "%s_ft", ifname);
	sprintf(mfp, "%s_mfp", ifname);
	sprintf(preamble, "%s_preamble", ifname);
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
		fprintf(stderr, "setup vap %d bss %s\n", vapid, ifname);
		fprintf(fp, "bss=%s\n", ifname);
	}
	char bw[32];
	sprintf(bw, "%s_channelbw", maininterface);
	int usebw = 20;
	if (nvram_matchi(bw, 40))
		usebw = 40;
	if (nvram_matchi(bw, 2040))
		usebw = 40;
	if (nvram_matchi(bw, 80))
		usebw = 80;
	if (nvram_matchi(bw, 160))
		usebw = 160;
	if (nvram_match(bw, "80+80"))
		usebw = 8080;
	if (has_ac(ifname) && has_2ghz(ifname) && usebw < 80) {
		if (nvram_nmatch("1", "%s_turbo_qam", ifname)) {
			char mubf[32];
			sprintf(mubf, "%s_mubf", maininterface);
			char subf[32];
			sprintf(subf, "%s_subf", maininterface);
			char *caps = mac80211_get_vhtcaps(maininterface, 0, 0, 0, 0, nvram_default_matchi(subf, 1, 0), nvram_default_matchi(mubf, 1, 0));
			fprintf(fp, "vht_capab=%s\n", caps);
			//fprintf(fp, "ieee80211ac=1\n");
			//fprintf(fp, "require_vht=1\n");
			fprintf(fp, "vendor_vht=1\n");
			free(caps);
		}

	}
	if (!vapid)
		fprintf(fp, "preamble=%s\n", nvram_default_get(preamble, "0"));
	fprintf(fp, "disassoc_low_ack=1\n");
	char *mode = nvram_nget("%s_mode", ifname);
	if (!strcmp(mode, "wdsap"))
		fprintf(fp, "wds_sta=1\n");
	char wmm[32];
	sprintf(wmm, "%s_wmm", ifname);
	fprintf(fp, "wmm_enabled=%s\n", nvram_default_get(wmm, "1"));
	if (has_uapsd(ifname)) {
		fprintf(fp, "uapsd_advertisement_enabled=%s\n", nvram_default_get(uapsd, "0"));
	}
	if (nvram_matchi("mac_clone_enable", 1)
	    && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00")
	    && nvram_invmatch("def_whwaddr", "")
	    && !strcmp(maininterface, "ath0")) {
		ieee80211_aton(nvram_safe_get("def_whwaddr"), hwbuff);
	} else {
		int i = wl_hwaddr(maininterface, hwbuff);
	}

	if (vapid > 0) {
		int brand = getRouterBrand();
		if (brand == ROUTER_WRT_3200ACM || brand == ROUTER_WRT_32X) {
			hwbuff[0] |= 0x2;
			hwbuff[5] += vapid & 0xf;
		} else {
			hwbuff[0] ^= ((vapid - 1) << 2) | 0x2;
		}

	}
	sprintf(macaddr, "%02X:%02X:%02X:%02X:%02X:%02X", hwbuff[0], hwbuff[1], hwbuff[2], hwbuff[3], hwbuff[4], hwbuff[5]);
//              MAC_ADD(macaddr);
	if (!has_ad(maininterface) && !is_brcmfmac(maininterface)) {
		fprintf(fp, "bssid=%s\n", macaddr);
	}
	char vathmac[16];
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
	int iswpa3_128 = nvhas(akm, "wpa3-128");
	int iswpa3_192 = nvhas(akm, "wpa3-192");
	int iswpa2sha256 = nvhas(akm, "wpa2-sha256");
	int ispsk2sha256 = nvhas(akm, "psk2-sha256");
	int iswep = nvhas(akm, "wep");
	// wep key support
	if (iswep || aoss) {
//              if (!isfirst || aoss)
//                      fprintf(fp, "ieee80211n=0\n");

		if (nvram_nmatch("1", "%s_bridged", ifname))
			fprintf(fp, "bridge=%s\n", getBridge(ifname, tmp));
		if (!aoss) {
			if (!strncmp(ifname, "ath0", 4))
				led_control(LED_SEC0, LED_ON);
			if (!strncmp(ifname, "ath1", 4))
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
	} else if (has_wpa3(ifname) && nvhas(akm, "owe")) {
		if (nvram_nmatch("1", "%s_bridged", ifname))
			fprintf(fp, "bridge=%s\n", getBridge(ifname, tmp));
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
		fprintf(fp, "wpa=2\n");
#ifdef HAVE_80211W
		if (nvram_default_matchi(mfp, -1, 0))
			fprintf(fp, "ieee80211w=1\n");
		if (nvram_default_matchi(mfp, 0, 0))
			fprintf(fp, "ieee80211w=0\n");
		if (nvram_default_matchi(mfp, 1, 0))
			fprintf(fp, "ieee80211w=2\n");
#endif
		fprintf(fp, "wpa_key_mgmt=OWE\n");
		fprintf(fp, "rsn_pairwise=CCMP\n");
	} else if (ispsk || ispsk2 || ispsk3 || iswpa || iswpa2 || iswpa3 || iswpa3_128 || iswpa3_192 || iswpa2sha256 || ispsk2sha256) {
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
		if (!nvram_nmatch("", "%s_radius2_ipaddr", ifname)
		    && !nvram_nmatch("0.0.0.0", "%s_radius2_ipaddr", ifname)
		    && !nvram_nmatch("", "%s_radius2_port", ifname)) {
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
#ifdef HAVE_HOTSPOT20
	setupHS20(fp, ifname);
#endif
	char *v = nvram_nget("%s_config", ifname);
	fprintf(fp, "\n");
	if (v && *v)
		fprintf(fp, "%s", v);
	fprintf(fp, "\n");
	fclose(fp);
}

void addvhtcaps(char *prefix, FILE * fp);

static char *makescanlist(char *value)
{
	char *clone = strdup(value);
	int len = strlen(clone);
	int i;
	char *new = NULL;
/* format list */
	for (i = 0; i < len; i++) {
		if (clone[i] == ';')
			clone[i] = ' ';
		if (clone[i] == ',')
			clone[i] = ' ';
	}
	char *next;
	char var[128];
	foreach(var, clone, next) {
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
			for (i = start; i < end + 5; i += 5) {
				char *old = new;
				if (!new)
					asprintf(&new, "%d", i);
				else {
					asprintf(&new, "%s %d", old, i);
					free(old);
				}

			}
		}
	}
	free(clone);
	return new;
}

void eap_sta_key_mgmt(FILE * fp, char *prefix);
void eap_sta_config(FILE * fp, char *prefix, char *ssidoverride, int addvht);

void setupSupplicant_ath9k(char *prefix, char *ssidoverride, int isadhoc)
{
#ifdef HAVE_REGISTER
	if (!isregistered())
		return;
#endif
	char akm[16];
	int i;
	int freq = 0;
	int freq2 = 0;
	char nfreq[16];
	char nfreq2[16];
	char *cellid;
	char cellidtemp[32];
	char cellidssid[5];
	char mcr[32];
	char ft[16];
	char mfp[16];
	char *mrate;
	sprintf(akm, "%s_akm", prefix);
	sprintf(ft, "%s_ft", prefix);
	sprintf(mfp, "%s_mfp", prefix);
	int ispsk2 = nvhas(akm, "psk2");
	int ispsk = nvhas(akm, "psk");
	int ispsk3 = nvhas(akm, "psk3");
	int ispsk2sha256 = nvhas(akm, "psk2-sha256");
	int isleap = nvhas(akm, "leap");
	int ispeap = nvhas(akm, "peap");
	int istls = nvhas(akm, "tls");
	int isttls = nvhas(akm, "ttls");
	int ismesh = nvram_nmatch("mesh", "%s_mode", prefix);
	if (ispsk)
		nvram_nseti(1, "%s_psk", prefix);
	if (ispsk2)
		nvram_nseti(1, "%s_psk2", prefix);
	if (ispsk2sha256)
		nvram_nseti(1, "%s_psk2-sha256", prefix);
	if (ispsk3)
		nvram_nseti(1, "%s_psk3", prefix);
	check_cryptomod(prefix);
	if (ispsk || ispsk2 || ispsk3 || ispsk2sha256) {
		char fstr[32];
		char psk[16];
		if (!strncmp(prefix, "ath0", 4))
			led_control(LED_SEC0, LED_ON);
		if (!strncmp(prefix, "ath1", 4))
			led_control(LED_SEC1, LED_ON);
		sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
		FILE *fp = fopen(fstr, "wb");
		if (!ismesh) {
			if (isadhoc)
				fprintf(fp, "ap_scan=2\n");
			else if (!ismesh)
				fprintf(fp, "ap_scan=1\n");
		}
		fprintf(fp, "fast_reauth=1\n");
		fprintf(fp, "eapol_version=1\n");
		if (ispsk3)
			fprintf(fp, "sae_groups=19 20 21\n");
		fprintf(fp, "network={\n");
		char *netmode = nvram_nget("%s_net_mode", prefix);
		char *channelbw = nvram_nget("%s_channelbw", prefix);
		if (strcmp(netmode, "ac-only") && strcmp(netmode, "acn-mixed") && strcmp(netmode, "mixed")) {
			fprintf(fp, "\tdisable_vht=1\n");
		}

		if (strcmp(netmode, "n-only") && strcmp(netmode, "n2-only")
		    && strcmp(netmode, "ac-only") && strcmp(netmode, "acn-mixed")
		    && strcmp(netmode, "n5-only") && strcmp(netmode, "na-only")
		    && strcmp(netmode, "ng-only") && strcmp(netmode, "mixed")) {
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
		if (isadhoc || ismesh) {
			char ht[5];
			char sb[32];
			char bw[32];
			if (ismesh)
				fprintf(fp, "\tmode=5\n");
			else
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
			} else if (nvram_match(bw, "40") || nvram_match(bw, "2040") || nvram_match(bw, "80") || nvram_match(bw, "80+80") || nvram_match(bw, "160")) {
				const char *cht = get_channeloffset(prefix, &iht, &channeloffset);
				sprintf(ht, cht + 2);
				fprintf(fp, "\tht40=1\n");
			}
			if (!is_ath5k(prefix))
				// fprintf(fp, "ibss_ht_mode=HT%s\n",ht);
				fprintf(fp, "\thtmode=HT%s\n", ht);

			if (nvram_match(bw, "80") || nvram_match(bw, "80+80") || nvram_match(bw, "160")) {
				fprintf(fp, "\tht40=1\n");
				fprintf(fp, "\tvht=1\n");
			}
			if (nvram_match(bw, "80")) {
				fprintf(fp, "\tmax_oper_chwidth=1\n");
				fprintf(fp, "\tvht_center_freq1=%d\n", freq + (channeloffset * 5));
			}
			if (nvram_match(bw, "80+80")) {
				fprintf(fp, "\tmax_oper_chwidth=3\n");
				fprintf(fp, "\tvht_center_freq1=%d\n", freq + (channeloffset * 5));
				fprintf(fp, "\tvht_center_freq2=%d\n", nvram_geti(nfreq2));	// todo
			}
			if (nvram_match(bw, "160")) {
				fprintf(fp, "\tmax_oper_chwidth=2\n");
				fprintf(fp, "\tvht_center_freq1=%d\n", freq + (channeloffset * 5));
			}

			if (isadhoc) {
				sprintf(cellidtemp, "%s_cellid", prefix);
				cellid = nvram_safe_get(cellidtemp);
				if (*cellid) {
					fprintf(fp, "\tbssid=%s\n", cellid);
				}
#if defined(HAVE_MAKSAT) || defined(HAVE_TMK) || defined(HAVE_BKM)
				else {
					memset(cellidssid, 0, 5);
					strncpy(cellidssid, ssidoverride, 5);
					fprintf(fp, "\tbssid=02:%02x:%02x:%02x:%02x:%02x\n", cellidssid[0], cellidssid[1], cellidssid[2], cellidssid[3], cellidssid[4]);
				}
#endif
			}
		}

		char scanlist[32];
		sprintf(scanlist, "%s_scanlist", prefix);
		char *sl = nvram_default_get(scanlist, "default");
		if (strcmp(sl, "default")) {
			char *scanlist = makescanlist(sl);
			fprintf(fp, "\tscan_freq=%s\n", scanlist);
			free(scanlist);
		}
#ifdef HAVE_UNIWIP
		fprintf(fp, "\tbgscan=\"simple:30:-45:300\"\n");
#endif
		// fprintf (fp, "\tmode=0\n");
		fprintf(fp, "\tscan_ssid=1\n");
		fprintf(fp, "\tkey_mgmt=");
		if (ispsk2 || ispsk)
			fprintf(fp, "WPA-PSK ");
		if (has_wpa3(prefix) && ispsk2sha256)
			fprintf(fp, "WPA-PSK-SHA256 ");
		if (has_wpa3(prefix) && ispsk3)
			fprintf(fp, "SAE ");
#ifdef HAVE_80211R
		if (has_wpa3(prefix) && nvram_matchi(ft, 1) && ispsk3)
			fprintf(fp, "FT-SAE ");
		if (nvram_matchi(ft, 1) && (ispsk2 || ispsk))
			fprintf(fp, "FT-PSK ");
#endif
		fprintf(fp, "\n");
		char pwstring[128] = {
			0, 0
		};
		char grpstring[128] = {
			0, 0
		};
		get_pairwise(prefix, pwstring, grpstring, isadhoc, ismesh);
#ifdef HAVE_80211W
		if (nvram_default_matchi(mfp, 1, 0) || ((ispsk2sha256 || ispsk3) && (!ispsk2 && !ispsk)))
			fprintf(fp, "\tieee80211w=2\n");
		else if (nvram_default_matchi(mfp, -1, 0) || ispsk3) {
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
			fprintf(fp, "\tpairwise=%s\n", &pwstring[1]);
			fprintf(fp, "\tgroup=%s\n", &grpstring[1]);
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
		char extra[32];
		sprintf(extra, "%s_supplicantext", prefix);
		fwritenvram(extra, fp);
		fclose(fp);
	} else if (ispeap || isleap || istls || isttls) {
		char fstr[32];
		char psk[64];
		char ath[64];
		if (!strncmp(prefix, "ath0", 4))
			led_control(LED_SEC0, LED_ON);
		if (!strncmp(prefix, "ath1", 4))
			led_control(LED_SEC1, LED_ON);
		sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
		FILE *fp = fopen(fstr, "wb");
		fprintf(fp, "ap_scan=1\n");
		fprintf(fp, "fast_reauth=1\n");
		fprintf(fp, "eapol_version=1\n");
		// fprintf (fp, "ctrl_interface_group=0\n");
		// fprintf (fp, "ctrl_interface=/var/run/wpa_supplicant\n");
		eap_sta_config(fp, prefix, ssidoverride, 1);

		char extra[32];
		sprintf(extra, "%s_supplicantext", prefix);
		fwritenvram(extra, fp);
		fclose(fp);
	} else if (nvram_match(akm, "disabled") || nvram_match(akm, "wep")) {
		char fstr[32];
		char psk[16];
		if (nvram_match(akm, "wep")) {
			if (!strncmp(prefix, "ath0", 4))
				led_control(LED_SEC0, LED_ON);
			if (!strncmp(prefix, "ath1", 4))
				led_control(LED_SEC1, LED_ON);
		}
		sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
		FILE *fp = fopen(fstr, "wb");
		if (!ismesh)
			fprintf(fp, "ap_scan=1\n");
		fprintf(fp, "network={\n");
		addvhtcaps(prefix, fp);
		if (!ssidoverride)
			ssidoverride = nvram_nget("%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", ssidoverride);
#ifdef HAVE_UNIWIP
		fprintf(fp, "\tbgscan=\"simple:30:-45:300\"\n");
#endif
		if (ismesh) {
			char ht[5];
			char sb[32];
			char bw[32];
			fprintf(fp, "\tmode=5\n");
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
			} else if (nvram_match(bw, "40") || nvram_match(bw, "2040") || nvram_match(bw, "80") || nvram_match(bw, "80+80") || nvram_match(bw, "160")) {
				const char *cht = get_channeloffset(prefix, &iht, &channeloffset);
				sprintf(ht, cht + 2);
				fprintf(fp, "\tht40=1\n");
			}
			if (!is_ath5k(prefix))
				// fprintf(fp, "ibss_ht_mode=HT%s\n",ht);
				fprintf(fp, "\thtmode=HT%s\n", ht);

			if (nvram_match(bw, "80") || nvram_match(bw, "80+80") || nvram_match(bw, "160")) {
				fprintf(fp, "\tht40=1\n");
				fprintf(fp, "\tvht=1\n");
			}
			if (nvram_match(bw, "80")) {
				fprintf(fp, "\tmax_oper_chwidth=1\n");
				fprintf(fp, "\tvht_center_freq1=%d\n", freq + (channeloffset * 5));
			}
			if (nvram_match(bw, "80+80")) {
				fprintf(fp, "\tmax_oper_chwidth=3\n");
				fprintf(fp, "\tvht_center_freq1=%d\n", freq + (channeloffset * 5));
				fprintf(fp, "\tvht_center_freq2=%d\n", nvram_geti(nfreq2));	// todo
			}
			if (nvram_match(bw, "160")) {
				fprintf(fp, "\tmax_oper_chwidth=2\n");
				fprintf(fp, "\tvht_center_freq1=%d\n", freq + (channeloffset * 5));
			}
		} else
			fprintf(fp, "\tscan_ssid=1\n");
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
					fprintf(fp, "wep_key%d=%s\n", i - 1, athkey);	// setup wep
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

}

extern void do_hostapd(char *fstr, char *prefix);
void ath9k_start_supplicant(int count, char *prefix)
{
// erst hostapd starten fuer repeater mode
// dann wpa_supplicant mit link zu hostapd
// dann bridgen und konfiguriren
	char *next;
	char var[80];
	char fstr[32];
	char bridged[32];
	char mode[80];
	char dev[10];
	char power[32];
	char *apm, *vifs;
	char wl[16];
	char ctrliface[32] = "";
	char wifivifs[16];
	char tmp[256];
	char *background = "-B";
	int debug;
	char subinterface[16];
	char net[16];
	char wmode[16];
	int ctrlifneeded = 0;
	char wif[10];
	sprintf(wif, "phy%d", get_ath9k_phy_idx(count));
	sprintf(wl, "%s_mode", prefix);
	sprintf(dev, "%s", prefix);
	sprintf(net, "%s_net_mode", dev);
	char *netmode = nvram_default_get(net, "mixed");
	if (!strcmp(netmode, "disabled"))
		return;
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
#ifdef HAVE_CONFIG_DEBUG_SYSLOG
	if (debug == 1)
		background = "-Bds";
	else if (debug == 2)
		background = "-Bdds";
	else if (debug == 3)
		background = "-Bddds";
#else
	if (debug == 1)
		background = "-Bd";
	else if (debug == 2)
		background = "-Bdd";
	else if (debug == 3)
		background = "-Bddd";
#endif
	int wet = 0;
#ifndef HAVE_RELAYD
	wet = nvram_match(wmode, "wet");
#endif
	char pid[64];
	sprintf(pid, "/var/run/%s_wpa_supplicant.pid", dev);

	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wdssta_mtik")
	    && strcmp(apm, "infra")
	    && strcmp(apm, "mesh")
	    && strcmp(apm, "tdma")
	    && strcmp(apm, "wet")) {
		sprintf(fstr, "/tmp/%s_hostap.conf", dev);
		do_hostapd(fstr, dev);
	} else {
		if (*vifs) {
			int ctrl = 0;
			foreach(var, vifs, next) {
				ctrl++;
				if (nvram_nmatch("ap", "%s_mode", var) || nvram_nmatch("wdsap", "%s_mode", var))
					break;
			}
			if (!nvram_match(wmode, "mesh")) {
				/* do not start hostapd before wpa_supplicant in mesh mode, it will fail to initialize the ap interface once mesh is running */
				sprintf(fstr, "/tmp/%s_hostap.conf", dev);
				do_hostapd(fstr, dev);
			}
			sprintf(ctrliface, "/var/run/hostapd/%s.%d", dev, ctrl);
			sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", dev);
			if (!nvram_match(wmode, "mesh") && !nvram_match(wmode, "infra")) {
				if ((nvram_match(wmode, "wdssta") || nvram_match(wmode, "mesh") || nvram_match(wmode, "wdsta_mtik")
				     || wet)
				    && nvram_matchi(bridged, 1))
					eval("wpa_supplicant", "-P", pid, "-b", getBridge(dev, tmp), background, "-Dnl80211", subinterface, "-H", ctrliface, "-c", fstr);
				else
					eval("wpa_supplicant", "-P", pid, background, "-Dnl80211", subinterface, "-H", ctrliface, "-c", fstr);
			} else {
				/* for mesh mode we dont need ctrl interface since it has a static channel configuration */
				if (nvram_matchi(bridged, 1))
					eval("wpa_supplicant", "-P", pid, "-b", getBridge(dev, tmp), background, "-Dnl80211", subinterface, "-c", fstr);
				else
					eval("wpa_supplicant", "-P", pid, background, "-Dnl80211", subinterface, "-c", fstr);
			}
			if (nvram_match(wmode, "mesh") || nvram_match(wmode, "infra")) {
				/* now start hostapd once wpa_supplicant has been started */
				sprintf(fstr, "/tmp/%s_hostap.conf", dev);
				do_hostapd(fstr, dev);
			}
		} else {
			sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", dev);
			if (nvram_match(wmode, "sta") || nvram_match(wmode, "wdssta") || nvram_match(wmode, "wdssta_mtik") || wet || nvram_match(wmode, "infra") || nvram_match(wmode, "mesh")) {
				if ((nvram_match(wmode, "wdssta") || nvram_match(wmode, "mesh") || nvram_match(wmode, "wdsta_mtik") || wet) && nvram_matchi(bridged, 1)) {
					eval("wpa_supplicant", "-P", pid, "-b", getBridge(dev, tmp), background, "-Dnl80211", subinterface, "-c", fstr);
				} else {
					eval("wpa_supplicant", "-P", pid, background, "-Dnl80211", subinterface, "-c", fstr);
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
		foreach(var, vifs, next) {
			sprintf(mode, "%s_mode", var);
			char *m2 = nvram_safe_get(mode);
			char bridged[32];
			sprintf(bridged, "%s_bridged", var);
			if (!strcmp(m2, "mesh")) {
				sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", var);
				sprintf(subinterface, "-i%s", var);
				if (nvram_matchi(bridged, 1))
					eval("wpa_supplicant", "-b", getBridge(var, tmp), background, "-Dnl80211", subinterface, "-c", fstr);
				else
					eval("wpa_supplicant", background, "-Dnl80211", subinterface, "-c", fstr);

			}

			if (strcmp(m2, "sta")) {
				if (nvram_default_matchi(bridged, 1, 1)) {
					eval("ifconfig", dev, "0.0.0.0", "up");
					br_add_interface(getBridge(var, tmp), var);
				} else {
					eval("ifconfig", var, "mtu", getMTU(var));
					eval("ifconfig", var, "txqueuelen", getTXQ(var));
					eval("ifconfig", var, nvram_nget("%s_ipaddr", var), "netmask", nvram_nget("%s_netmask", var), "up");
				}
			}
		}
	}
	if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap")) {
		int s;
		for (s = 1; s <= 10; s++) {
			char wdsvarname[32] = {
				0
			};
			char wdsdevname[32] = {
				0
			};
			char wdsmacname[32] = {
				0
			};
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
				eval("iw", "dev", wdsdev, "set", "peer", hwaddr);
				eval("ifconfig", wdsdev, "0.0.0.0", "up");
			}
		}
	}
	sysprintf("iw phy %s set txpower fixed %d", wif, nvram_default_geti(power, 16) * 100);
}
#endif
