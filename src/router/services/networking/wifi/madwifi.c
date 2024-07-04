/*
 * madwifi.c
 *
 * Copyright (C) 2005 - 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>
#include <syslog.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>
#include <sha1.h>
#include "wireless.h"
#include <services.h>
#include <wlutils.h>
#include <libbridge.h>

#ifdef HAVE_MADWIFI
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"
//#include <iwlib.h>

static int setsysctrl(const char *dev, const char *control, u_long value)
{
	char val[32];
	sprintf(val, "%li", value);
	writevaproc(val, "/proc/sys/dev/%s/%s", dev, control);

	return 0;
}

static void setdistance(char *device, int distance, int chanbw)
{
	if (distance >= 0)
		setsysctrl(device, "distance", distance);
}
#endif

// returns the number of installed atheros devices/cards

static void deconfigure_single(int count)
{
	char *next;
	char dev[16];
	char var[80];
	char wifivifs[16];
	sprintf(wifivifs, "wlan%d_vifs", count);
	sprintf(dev, "wlan%d", count);
	if (!strncmp(dev, "wlan0", 4)) {
		led_control(LED_SEC0, LED_OFF);
		led_control(LED_WLAN0, LED_OFF);
	}
	if (!strncmp(dev, "wlan1", 4)) {
		led_control(LED_SEC1, LED_OFF);
		led_control(LED_WLAN1, LED_OFF);
	}

	char vifs[128];
	if (is_mac80211(dev)) {
		deconfigure_single_ath9k(count);
		sysprintf("rm -f /tmp/wlan%d_configured", count);
		return;
	}
#ifdef HAVE_MADWIFI
	sprintf(vifs, "%s.1 %s.2 %s.3 %s.4 %s.5 %s.6 %s.7 %s.8 %s.9", dev, dev, dev, dev, dev, dev, dev, dev, dev);
	int s;

	for (s = 1; s <= 10; s++) {
		sprintf(dev, "wlan%d.wds%d", count, s - 1);
		if (ifexists(dev)) {
			br_del_interface("br0", dev);
			eval("ifconfig", dev, "down");
		}
	}
	sprintf(dev, "wlan%d", count);
	if (ifexists(dev)) {
		br_del_interface("br0", dev);
		eval("ifconfig", dev, "down");
	}
	foreach(var, vifs, next)
	{
		if (ifexists(var)) {
			eval("ifconfig", var, "down");
		}
	}
	sprintf(dev, "wlan%d", count);

	if (ifexists(dev))
		eval("wlanconfig", dev, "destroy");

	foreach(var, vifs, next)
	{
		if (ifexists(var)) {
			eval("wlanconfig", var, "destroy");
		}
	}
#endif
	sysprintf("rm -f /tmp/wlan%d_configured", count);
}

void deconfigure_wifi(void)
{
	stop_process("wrt-radauth", "Radius daemon");
	stop_process("hostapd", "hostapd daemon");
	stop_process("wpa_supplicant", "wpa_supplicant daemon");
	sysprintf("rm -f /var/run/ath*"); // delete pid files
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++)
		deconfigure_single(i);
	invalidate_channelcache();
}

static int need_commit = 0;
#ifdef HAVE_MADWIFI

static int getMaxPower(char *ifname)
{
	char buf[128];

	sprintf(buf, "iwlist %s txpower|grep \"Maximum Power:\" > /tmp/.power", ifname);
	system(buf);
	FILE *in = fopen("/tmp/.power", "rb");

	if (in == NULL)
		return 1000;
	char buf2[16];
	int max;

	fscanf(in, "%s %s %d", buf, buf2, &max);
	fclose(in);
	return max;
}

void setupKey(char *prefix)
{
	char akm[16];
	char mode[16];

	sprintf(akm, "%s_akm", prefix);
	sprintf(mode, "%s_mode", prefix);
	if (nvram_match(akm, "wep") && (nvram_match(mode, "ap") || nvram_match(mode, "wdsap") || nvram_match(mode, "adhoc"))) {
		char key[16];
		int cnt = 1;
		int i;
		char bul[8];
		char *authmode = nvram_nget("%s_authmode", prefix);
		if (!strcmp(authmode, "shared"))
			eval("iwpriv", prefix, "authmode", "2");
		else if (!strcmp(authmode, "auto"))
			eval("iwpriv", prefix, "authmode", "4");
		else
			eval("iwpriv", prefix, "authmode", "1");
		for (i = 1; i < 5; i++) {
			char *athkey = nvram_nget("%s_key%d", prefix, i);

			if (athkey != NULL && *athkey) {
				sysprintf("iwconfig %s key [%d] %s", prefix, cnt++, athkey); // setup wep
			}
		}
		sysprintf("iwconfig %s key [%s]", prefix, nvram_nget("%s_key", prefix));
	}
}

void get_pairwise(char *prefix, char *pwstring, char *grpstring, int isadhoc, int ismesh)
{
	char temp_grpstring[256] = { 0, 0 };
	char akm[16];
	sprintf(akm, "%s_akm", prefix);
	int iswpa3_192 = nvhas(akm, "wpa3-192");
	int iswpa3_128 = nvhas(akm, "wpa3-128");
	int iswpa3 = nvhas(akm, "wpa3");
	if (nvram_nmatch("1", "%s_ccmp", prefix)) {
		strspcattach(pwstring, "CCMP");
		if (grpstring) {
#if defined(HAVE_MAKSAT) || defined(HAVE_TMK) || defined(HAVE_BKM)
			if (isadhoc)
				strspcattach(temp_grpstring, "CCMP");
			else
#endif
				if (ismesh)
				strspcattach(temp_grpstring, "CCMP");
			else
				strspcattach(temp_grpstring, "CCMP TKIP");
		}
	}
	if (nvram_nmatch("1", "%s_tkip", prefix)) {
		strspcattach(pwstring, "TKIP");
		if (grpstring)
			strspcattach(temp_grpstring, "TKIP");
	}
	if (nvram_nmatch("1", "%s_ccmp-256", prefix)) {
		strspcattach(pwstring, "CCMP-256");
		if (grpstring) {
			if (ismesh)
				strspcattach(temp_grpstring, "CCMP-256 GCMP-256 GCMP CCMP");
			else
				strspcattach(temp_grpstring, "CCMP-256 GCMP-256 GCMP CCMP TKIP");
		}
	}
	if (nvram_nmatch("1", "%s_gcmp-256", prefix) || iswpa3_192) {
		strspcattach(pwstring, "GCMP-256");
		if (grpstring) {
			if (ismesh)
				strspcattach(temp_grpstring, "GCMP-256 GCMP CCMP");
			else
				strspcattach(temp_grpstring, "GCMP-256 GCMP CCMP TKIP");
		}
	}
	if (nvram_nmatch("1", "%s_gcmp", prefix) || iswpa3_128) {
		strspcattach(pwstring, "GCMP");
		if (grpstring) {
			if (has_ad(prefix))
				strspcattach(temp_grpstring, "GCMP");
			else {
				if (ismesh)
					strspcattach(temp_grpstring, "GCMP CCMP");
				else
					strspcattach(temp_grpstring, "GCMP CCMP TKIP");
			}
		}
	}
	/* remove duplicates and recreate group string */
	if (grpstring) {
		char *next;
		char var[32];
		foreach(var, temp_grpstring, next)
		{
			if (!strhas(grpstring, var)) {
				strspcattach(grpstring, var);
			}
		}
	}
}

void eap_sta_key_mgmt(FILE *fp, char *prefix)
{
	char ft[16];
	char mfp[16];
	char akm[16];
	sprintf(ft, "%s_ft", prefix);
	sprintf(mfp, "%s_mfp", prefix);
	sprintf(akm, "%s_akm", prefix);
	const int _has_wpa3 = has_wpa3(prefix);
	const int iswep = nvhas(akm, "802.1x");
	const int iswpa = nvhas(akm, "wpa");
	const int iswpa2 = nvhas(akm, "wpa2");
	const int ispsk = nvhas(akm, "psk");
	const int ispsk2 = nvhas(akm, "psk2");
	const int iswpa3 = _has_wpa3 ? nvhas(akm, "wpa3") : 0;
	const int iswpa3_192 = _has_wpa3 ? nvhas(akm, "wpa3-192") : 0;
	const int iswpa3_128 = _has_wpa3 ? nvhas(akm, "wpa3-128") : 0;
	const int iswpa2sha256 = _has_wpa3 ? nvhas(akm, "wpa2-sha256") : 0;
	const int iswpa2sha384 = _has_wpa3 ? nvhas(akm, "wpa2-sha384") : 0;
	char pwstring[128] = { 0 };
	char grpstring[128] = { 0 };
	get_pairwise(prefix, pwstring, grpstring, 0, 0);
	if (*pwstring && (iswpa2 || iswpa || iswpa3 || iswpa3_128 || iswpa3_192 || iswpa2sha256|| iswpa2sha384)) {
		fprintf(fp, "\tpairwise=%s\n", pwstring);
		fprintf(fp, "\tgroup=%s\n", grpstring);
	}

	fprintf(fp, "\tkey_mgmt=");
	if (iswpa2 || iswpa || iswpa3)
		fprintf(fp, "WPA-EAP ");
	if (_has_wpa3 && iswpa2sha256)
		fprintf(fp, "WPA-EAP-SHA256 ");
	if (_has_wpa3 && iswpa2sha384)
		fprintf(fp, "WPA-EAP-SHA384 ");
	if (_has_wpa3 && iswpa3_128)
		fprintf(fp, "WPA-EAP-SUITE-B ");
	if (_has_wpa3 && iswpa3_192)
		fprintf(fp, "WPA-EAP-SUITE-B-192 ");
#ifdef HAVE_80211R
	if (nvram_matchi(ft, 1) && (iswpa || iswpa2 || iswpa3 || iswpa3_128 || iswpa2sha256))
		fprintf(fp, "FT-EAP ");
	if (nvram_matchi(ft, 1) && (iswpa3_192 || iswpa2sha384))
		fprintf(fp, "FT-EAP-SHA384 ");
#endif
	if (iswep)
		fprintf(fp, "IEEE8021X ");
	fprintf(fp, "\n");
#ifdef HAVE_80211W
	if ((iswpa2 || iswpa || iswpa3 || iswpa3_128 || iswpa3_192 || iswpa2sha256 || iswpa2sha384)) {
		if (nvram_default_matchi(mfp, 1, 0) ||
		    ((iswpa3_128 || iswpa3_192 || iswpa3 || iswpa2sha256 || iswpa2sha384) && (!iswpa && !iswpa2 && !ispsk && !ispsk2)))
			fprintf(fp, "\tieee80211w=2\n");
		else if (nvram_default_matchi(mfp, -1, 0) || iswpa3_192 || iswpa3_128 || iswpa3 || iswpa2sha256  || iswpa2sha384)
			fprintf(fp, "\tieee80211w=1\n");
		else if (nvram_default_matchi(mfp, 0, 0))
			fprintf(fp, "\tieee80211w=0\n");
	}
#endif
}

#ifndef HAVE_SUPERCHANNEL
int inline issuperchannel(void)
{
	return 0;
}
#else
int issuperchannel(void);
#endif

static int cansuperchannel(char *prefix)
{
	return (issuperchannel() && nvram_nmatch("0", "%s_regulatory", prefix) && nvram_nmatch("ddwrt", "%s_fwtype", prefix));
}

void addvhtcaps(char *prefix, FILE *fp)
{
/* must use integer mask */
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ 0x00000004
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ 0x00000008
#define IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE 0x00000800
#define IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE 0x00001000
#define IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE 0x00080000
#define IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE 0x00100000
#define IEEE80211_VHT_CAP_SHORT_GI_80 0x00000020
#define IEEE80211_VHT_CAP_SHORT_GI_160 0x00000040
#define IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT 13
#define IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK (7 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT)
#define IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT 16
#define IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK (7 << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT)

	unsigned int mask;
	if (is_mt7615(prefix) || is_ath10k(prefix) || is_ath11k(prefix) || is_brcmfmac(prefix) || is_mt7915(prefix) ||
	    is_mt7921(prefix) || is_mt7603(prefix) || is_mt76x0(prefix) || is_mt76x2(prefix)) {
		char *netmode = nvram_nget("%s_net_mode", prefix);
		if (has_ac(prefix) && (!strcmp(netmode, "ac-only") || strcmp(netmode, "ax-only") || !strcmp(netmode, "acn-mixed") ||
				       !strcmp(netmode, "xacn-mixed") || !strcmp(netmode, "mixed") ||
				       (cansuperchannel(prefix) && nvram_nmatch("1", "%s_turbo_qam", prefix)))) {
			char shortgi[32];
			sprintf(shortgi, "%s_shortgi", prefix);
			char mubf[32];
			sprintf(mubf, "%s_mubf", prefix);
			char subf[32];
			sprintf(subf, "%s_subf", prefix);
			char cbw[32];
			sprintf(cbw, "%s_channelbw", prefix);
			mask = 0;
			if (nvram_default_matchi(subf, 0, 0)) {
				mask |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
				mask |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
				mask |= IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK;
				mask |= IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK;
			}
			if (nvram_default_matchi(mubf, 0, 0)) {
				mask |= IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;
				mask |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;
			}

			if (nvram_default_matchi(shortgi, 0, 1)) {
				mask |= IEEE80211_VHT_CAP_SHORT_GI_80;
				mask |= IEEE80211_VHT_CAP_SHORT_GI_160;
			}
			int bw = atoi(nvram_safe_get(cbw));
			if (bw > 0 && bw != 8080 && bw != 2040) {
				if (bw < 160) {
					mask |= IEEE80211_VHT_CAP_SHORT_GI_160;
					mask |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ;
					mask |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
				}
				if (bw < 80) {
					mask |= IEEE80211_VHT_CAP_SHORT_GI_80;
				}
			}
//			if (mask) {
//				fprintf(fp, "\tvht_capa=0\n");
//				fprintf(fp, "\tvht_capa_mask=%d\n", mask);
//			}
		}
	}
#ifdef HAVE_ATH9K
	if (is_mac80211(prefix)) {
		char shortgi[32];
		sprintf(shortgi, "%s_shortgi", prefix);
		if (nvram_matchi(shortgi, 0))
			fprintf(fp, "\tdisable_sgi=1\n");
	}
#endif
}

void addbssid(FILE *fp, char *prefix)
{
	char *bssid = nvram_nget("%s_bssid", prefix);
	char c_bssid[32];
	strncpy(c_bssid, bssid, 31);
	int i;
	int cnt = 0;
	for (i = 0; i < strlen(c_bssid); i++) {
		if (c_bssid[i] == ' ') {
			continue;
		}
		c_bssid[cnt++] = c_bssid[i];
	}
	c_bssid[cnt] = 0;
	if (strlen(c_bssid) == 17 && strcmp(c_bssid, "00:00:00:00:00:00"))
		fprintf(fp, "\tbssid=%s\n", c_bssid);
}

void eap_sta_config(FILE *fp, char *prefix, char *ssidoverride, int addvht)
{
	char ath[64];
	char psk[64];
	char akm[16];
	sprintf(akm, "%s_akm", prefix);
	int isleap = nvhas(akm, "leap");
	int ispeap = nvhas(akm, "peap");
	int istls = nvhas(akm, "tls");
	int isttls = nvhas(akm, "ttls");
	if (istls) {
		fprintf(fp, "network={\n");
		if (addvht)
			addvhtcaps(prefix, fp);
		if (!ssidoverride)
			ssidoverride = nvram_nget("%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", ssidoverride);
#ifdef HAVE_UNIWIP
		fprintf(fp, "\tbgscan=\"simple:30:-45:300\"\n");
#endif
		addbssid(fp, prefix);
		fprintf(fp, "\tscan_ssid=1\n");
		eap_sta_key_mgmt(fp, prefix);

		fprintf(fp, "\teap=TLS\n");
		fprintf(fp, "\tidentity=\"%s\"\n", nvram_prefix_get("tls8021xuser", prefix));
		sprintf(psk, "/tmp/%s", prefix);
		mkdir(psk, 0700);
		sprintf(psk, "/tmp/%s/ca.pem", prefix);
		sprintf(ath, "%s_tls8021xca", prefix);
		write_nvram(psk, ath);
		sprintf(psk, "/tmp/%s/user.pem", prefix);
		sprintf(ath, "%s_tls8021xpem", prefix);
		write_nvram(psk, ath);
		sprintf(psk, "/tmp/%s/user.prv", prefix);
		sprintf(ath, "%s_tls8021xprv", prefix);
		write_nvram(psk, ath);
		fprintf(fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix);
		fprintf(fp, "\tclient_cert=\"/tmp/%s/user.pem\"\n", prefix);
		fprintf(fp, "\tprivate_key=\"/tmp/%s/user.prv\"\n", prefix);
		fprintf(fp, "\tprivate_key_passwd=\"%s\"\n", nvram_prefix_get("tls8021xpasswd", prefix));
		fprintf(fp, "\teapol_flags=3\n");
		if (*(nvram_nget("%s_tls8021xphase2", prefix))) {
			fprintf(fp, "\tphase2=\"%s\"\n", nvram_nget("%s_tls8021xphase2", prefix));
		}
		if (*(nvram_nget("%s_tls8021xanon", prefix))) {
			fprintf(fp, "\tanonymous_identity=\"%s\"\n", nvram_nget("%s_tls8021xanon", prefix));
		}
		if (*(nvram_nget("%s_tls8021xaddopt", prefix))) {
			sprintf(ath, "%s_tls8021xaddopt", prefix);
			fprintf(fp, "\t"); // tab
			fwritenvram(ath, fp);
			fprintf(fp, "\n"); // extra new line at the end
		}
		fprintf(fp, "}\n");
	}

	if (ispeap) {
		fprintf(fp, "network={\n");
		if (addvht)
			addvhtcaps(prefix, fp);
		if (!ssidoverride)
			ssidoverride = nvram_nget("%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", ssidoverride);
#ifdef HAVE_UNIWIP
		fprintf(fp, "\tbgscan=\"simple:30:-45:300\"\n");
#endif
		addbssid(fp, prefix);
		fprintf(fp, "\tscan_ssid=1\n");
		eap_sta_key_mgmt(fp, prefix);
		fprintf(fp, "\teap=PEAP\n");
		fprintf(fp, "\tidentity=\"%s\"\n", nvram_prefix_get("peap8021xuser", prefix));
		fprintf(fp, "\tpassword=\"%s\"\n", nvram_prefix_get("peap8021xpasswd", prefix));
		sprintf(psk, "/tmp/%s", prefix);
		mkdir(psk, 0700);
		sprintf(psk, "/tmp/%s/ca.pem", prefix);
		sprintf(ath, "%s_peap8021xca", prefix);
		if (!nvram_match(ath, "")) {
			write_nvram(psk, ath);
			fprintf(fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix);
		}

		if (*(nvram_nget("%s_peap8021xphase1", prefix))) {
			fprintf(fp, "\tphase1=\"%s\"\n", nvram_nget("%s_peap8021xphase1", prefix));
		} else {
			fprintf(fp, "\tphase1=\"peapver=0\"\n");
		}

		if (*(nvram_nget("%s_peap8021xphase2", prefix))) {
			fprintf(fp, "\tphase2=\"%s\"\n", nvram_nget("%s_peap8021xphase2", prefix));
		}
		if (*(nvram_nget("%s_peap8021xanon", prefix))) {
			fprintf(fp, "\tanonymous_identity=\"%s\"\n", nvram_nget("%s_peap8021xanon", prefix));
		}
		if (*(nvram_nget("%s_peap8021xaddopt", prefix))) {
			sprintf(ath, "%s_peap8021xaddopt", prefix);
			fprintf(fp, "\t"); // tab
			fwritenvram(ath, fp);
			fprintf(fp, "\n"); // extra new line at the end
		}
		fprintf(fp, "}\n");
	}

	if (isttls) {
		fprintf(fp, "network={\n");
		if (addvht)
			addvhtcaps(prefix, fp);
		if (!ssidoverride)
			ssidoverride = nvram_nget("%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", ssidoverride);
#ifdef HAVE_UNIWIP
		fprintf(fp, "\tbgscan=\"simple:30:-45:300\"\n");
#endif
		addbssid(fp, prefix);
		fprintf(fp, "\tscan_ssid=1\n");
		eap_sta_key_mgmt(fp, prefix);
		fprintf(fp, "\teap=TTLS\n");
		fprintf(fp, "\tidentity=\"%s\"\n", nvram_prefix_get("ttls8021xuser", prefix));
		fprintf(fp, "\tpassword=\"%s\"\n", nvram_prefix_get("ttls8021xpasswd", prefix));
		if (*(nvram_nget("%s_ttls8021xca", prefix))) {
			sprintf(psk, "/tmp/%s", prefix);
			mkdir(psk, 0700);
			sprintf(psk, "/tmp/%s/ca.pem", prefix);
			sprintf(ath, "%s_ttls8021xca", prefix);
			write_nvram(psk, ath);
			fprintf(fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix);
		}
		if (*(nvram_nget("%s_ttls8021xphase2", prefix))) {
			fprintf(fp, "\tphase2=\"%s\"\n", nvram_nget("%s_ttls8021xphase2", prefix));
		}
		if (*(nvram_nget("%s_ttls8021xanon", prefix))) {
			fprintf(fp, "\tanonymous_identity=\"%s\"\n", nvram_nget("%s_ttls8021xanon", prefix));
		}
		if (*(nvram_nget("%s_ttls8021xaddopt", prefix))) {
			sprintf(ath, "%s_ttls8021xaddopt", prefix);
			fprintf(fp, "\t"); // tab
			fwritenvram(ath, fp);
			fprintf(fp, "\n"); // extra new line at the end
		}
		fprintf(fp, "}\n");
	}

	if (isleap) {
		fprintf(fp, "network={\n");
		if (addvht)
			addvhtcaps(prefix, fp);
		if (!ssidoverride)
			ssidoverride = nvram_nget("%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", ssidoverride);
#ifdef HAVE_UNIWIP
		fprintf(fp, "\tbgscan=\"simple:30:-45:300\"\n");
#endif
		addbssid(fp, prefix);
		fprintf(fp, "\tscan_ssid=1\n");
		eap_sta_key_mgmt(fp, prefix);
		fprintf(fp, "\teap=LEAP\n");
		fprintf(fp, "\tauth_alg=LEAP\n");
		fprintf(fp, "\tproto=WPA RSN\n");
		fprintf(fp, "\tidentity=\"%s\"\n", nvram_prefix_get("leap8021xuser", prefix));
		fprintf(fp, "\tpassword=\"%s\"\n", nvram_prefix_get("leap8021xpasswd", prefix));
		if (*(nvram_nget("%s_leap8021xphase2", prefix))) {
			fprintf(fp, "\tphase2=\"%s\"\n", nvram_nget("%s_leap8021xphase2", prefix));
		}
		if (*(nvram_nget("%s_leap8021xanon", prefix))) {
			fprintf(fp, "\tanonymous_identity=\"%s\"\n", nvram_nget("%s_leap8021xanon", prefix));
		}
		if (*(nvram_nget("%s_leap8021xaddopt", prefix))) {
			sprintf(ath, "%s_leap8021xaddopt", prefix);
			fprintf(fp, "\t"); // tab
			fwritenvram(ath, fp);
			fprintf(fp, "\n"); // extra new line at the end
		}
		fprintf(fp, "}\n");
	}
}

void check_cryptomod(char *prefix)
{
	insmod("crypto_hash crypto_null aead ctr ccm cmac");
	if (has_wpa3(prefix)) {
		char mfp[16];
		char akm[16];
		sprintf(mfp, "%s_mfp", prefix);
		sprintf(akm, "%s_akm", prefix);
		int w = nvram_default_geti(mfp, 0);

		if (w == 1 || w == -1 || nvhas(akm, "psk3") || nvhas(akm, "owe") || nvhas(akm, "wpa3") || nvhas(akm, "wpa3-192") ||
		    nvhas(akm, "wpa3-128") || nvhas(akm, "wpa2-sha256") || nvhas(akm, "wpa2-sha384") || nvhas(akm, "psk2-sha256"))
			insmod("gf128mul ghash-generic gcm");
	}
}

/*
 * MADWIFI Encryption Setup 
 */
int isregistered(void);

void setupSupplicant(char *prefix, char *ssidoverride)
{
#ifdef HAVE_REGISTER
	if (!isregistered())
		return;
#endif
	char akm[16];
	char bridged[32];
	char wmode[16];
	char *background = "-B";
	char *debug;
	char tmp[256];
	char ft[16];
	char mfp[16];
	char eapol[32];
	sprintf(eapol, "%s_eapol_version", prefix);
	int i;
	debug = nvram_nget("%s_wpa_debug", prefix);
	if (debug != NULL) {
		if (!strcmp(debug, "1"))
			background = "-Bds";
		else if (!strcmp(debug, "2"))
			background = "-Bdds";
		else if (!strcmp(debug, "3"))
			background = "-Bddds";
	}

	char driver[32];
	sprintf(driver, "-Dwext");

	sprintf(ft, "%s_ft", prefix);
	sprintf(mfp, "%s_mfp", prefix);
	sprintf(akm, "%s_akm", prefix);
	sprintf(wmode, "%s_mode", prefix);
	sprintf(bridged, "%s_bridged", prefix);
	const int _has_wpa3 = has_wpa3(prefix);
	const int ispsk2 = nvhas(akm, "psk2");
	const int ispsk = nvhas(akm, "psk");
	const int ispsk3 = _has_wpa3 ? nvhas(akm, "psk3") : 0;
	const int ispsk2sha256 = _has_wpa3 ? nvhas(akm, "psk2-sha256") : 0;
	const int isleap = nvhas(akm, "leap");
	const int ispeap = nvhas(akm, "peap");
	const int istls = nvhas(akm, "tls");
	const int isttls = nvhas(akm, "ttls");
	if (ispsk)
		nvram_nseti(1, "%s_psk", prefix);
	if (ispsk2)
		nvram_nseti(1, "%s_psk2", prefix);
	if (ispsk2sha256)
		nvram_nseti(1, "%s_psk2-sha256", prefix);
	if (ispsk3)
		nvram_nseti(1, "%s_psk3", prefix);
	check_cryptomod(prefix);
	char pid[64];
	sprintf(pid, "/var/run/%s_wpa_supplicant.pid", prefix);

	if (ispsk || ispsk2 || ispsk3 || ispsk2sha256) {
		char fstr[32];
		char psk[16];
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
		if (ispsk3)
			fprintf(fp, "\tsae_groups=19 20 21\n");

		fprintf(fp, "network={\n");
		if (!ssidoverride)
			ssidoverride = nvram_nget("%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", ssidoverride);
		// fprintf (fp, "\tmode=0\n");
		addbssid(fp, prefix);
		fprintf(fp, "\tscan_ssid=1\n");
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
		get_pairwise(prefix, pwstring, grpstring, 0, 0);
#ifdef HAVE_80211W
		if (nvram_default_matchi(mfp, -1, 0))
			fprintf(fp, "\tieee80211w=1\n");
		if (nvram_default_matchi(mfp, 0, 0))
			fprintf(fp, "\tieee80211w=0\n");
		if (nvram_default_matchi(mfp, 1, 0))
			fprintf(fp, "\tieee80211w=2\n");
#endif

		if (!*pwstring) {
			sprintf(psk, "%s_crypto", prefix);
			if (nvram_match(psk, "aes")) {
				nvram_nseti(1, "%s_ccmp", prefix);
				fprintf(fp, "\tpairwise=CCMP\n");
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
		char extra[32];
		sprintf(extra, "%s_supplicantext", prefix);
		fwritenvram(extra, fp);

		fclose(fp);
		sprintf(psk, "-i%s", prefix);
#ifdef HAVE_RELAYD
		if ((nvram_match(wmode, "wdssta") || nvram_match(wmode, "mesh") || nvram_match(wmode, "wdssta_mtik")) &&
		    nvram_matchi(bridged, 1))
			log_eval("wpa_supplicant", "-P", pid, "-b", getBridge(prefix, tmp), background, driver, psk, "-c", fstr);
		else
			log_eval("wpa_supplicant", "-P", pid, background, driver, psk, "-c", fstr);
#else
		if ((nvram_match(wmode, "wdssta") || nvram_match(wmode, "wet") || nvram_match(wmode, "mesh") ||
		     nvram_match(wmode, "wdssta_mtik")) &&
		    nvram_matchi(bridged, 1))
			log_eval("wpa_supplicant", "-P", pid, "-b", getBridge(prefix, tmp), background, driver, psk, "-c", fstr);
		else
			log_eval("wpa_supplicant", "-P", pid, background, driver, psk, "-c", fstr);
#endif
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
		// fprintf (fp, "ctrl_interface=/var/run/wpa_supplicant\n");
		eap_sta_config(fp, prefix, ssidoverride, 0);

		char extra[32];
		sprintf(extra, "%s_supplicantext", prefix);
		fwritenvram(extra, fp);
		fclose(fp);
		sprintf(psk, "-i%s", prefix);
		eval("iwpriv", prefix, "hostroaming", "2");
#ifdef HAVE_RELAYD
		if (nvram_matchi(bridged, 1) &&
		    (nvram_match(wmode, "wdssta") || nvram_match(wmode, "mesh") || nvram_match(wmode, "wdssta_mtik")))
			log_eval("wpa_supplicant", "-P", pid, "-b", nvram_safe_get("lan_ifname"), background, driver, psk, "-c",
				 fstr);
		else
			log_eval("wpa_supplicant", "-P", pid, background, driver, psk, "-c", fstr);
#else
		if (nvram_matchi(bridged, 1) && (nvram_match(wmode, "wdssta") || nvram_match(wmode, "mesh") ||
						 nvram_match(wmode, "wdssta_mtik") || nvram_match(wmode, "wet")))
			log_eval("wpa_supplicant", "-P", pid, "-b", nvram_safe_get("lan_ifname"), background, driver, psk, "-c",
				 fstr);
		else
			log_eval("wpa_supplicant", "-P", pid, background, driver, psk, "-c", fstr);

#endif
	} else if (nvram_match(akm, "disabled") || nvram_match(akm, "wep")) {
		char fstr[32];
		char psk[16];
		if (nvram_match(akm, "wep")) {
			if (!strncmp(prefix, "wlan0", 4))
				led_control(LED_SEC0, LED_ON);
			if (!strncmp(prefix, "wlan1", 4))
				led_control(LED_SEC1, LED_ON);
		}
		sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
		FILE *fp = fopen(fstr, "wb");

		fprintf(fp, "ctrl_interface=/var/run/wpa_supplicant\n");
		fprintf(fp, "ap_scan=1\n");
		// fprintf (fp, "ctrl_interface_group=0\n");
		// fprintf (fp, "ctrl_interface=/var/run/wpa_supplicant\n");

		fprintf(fp, "network={\n");
		if (!ssidoverride)
			ssidoverride = nvram_nget("%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", ssidoverride);
		// fprintf (fp, "\tmode=0\n");
		addbssid(fp, prefix);
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
					fprintf(fp, "wep_key%d=%s\n", cnt++,
						athkey); // setup wep
				}
			}

			fprintf(fp, "wep_tx_keyidx=%s\n", nvram_nget("%s_key", prefix));
		}
		fprintf(fp, "}\n");
		char extra[32];
		sprintf(extra, "%s_supplicantext", prefix);
		fwritenvram(extra, fp);

		fclose(fp);
		sprintf(psk, "-i%s", prefix);
#ifdef HAVE_RELAYD
		if ((nvram_match(wmode, "wdssta") || nvram_match(wmode, "mesh") || nvram_match(wmode, "wdssta_mtik")) &&
		    nvram_matchi(bridged, 1))
			log_eval("wpa_supplicant", "-P", pid, "-b", getBridge(prefix, tmp), background, driver, psk, "-c", fstr);
		else
			log_eval("wpa_supplicant", "-P", pid, background, driver, psk, "-c", fstr);
#else
		if ((nvram_match(wmode, "wdssta") || nvram_match(wmode, "wet") || nvram_match(wmode, "mesh") ||
		     nvram_match(wmode, "wdssta_mtik")) &&
		    nvram_matchi(bridged, 1))
			log_eval("wpa_supplicant", "-P", pid, "-b", getBridge(prefix, tmp), background, driver, psk, "-c", fstr);
		else
			log_eval("wpa_supplicant", "-P", pid, background, driver, psk, "-c", fstr);
#endif
	}
}

void supplicant_main(int argc, char *argv[])
{
	setupSupplicant(argv[1], argv[2]);
}
#endif
void do_hostapd(char *fstr, char *prefix)
{
	char fname[32];
	int debug;
	FILE *fp;
	int pid;

	sprintf(fname, "/var/run/%s_hostapd.pid", prefix);

	fp = fopen(fname, "rb");
	if (fp) {
		fscanf(fp, "%d", &pid);
		fclose(fp);
		if (pid > 0)
			kill(pid, SIGTERM);
	}
	char *argv[] = { "hostapd", "-B", "-P", fname, NULL, NULL, NULL, NULL, NULL };
	int argc = 4;
	debug = nvram_ngeti("%s_wpa_debug", prefix);
	char file[64];
	if (debug > 0 && debug < 4) {
		if (debug == 1)
			argv[argc++] = "-d";
		else if (debug == 2)
			argv[argc++] = "-dd";
		else if (debug == 3)
			argv[argc++] = "-ddd";
		argv[argc++] = "-f";
		sprintf(file, "/tmp/%s_debug", prefix);
		argv[argc++] = file;
	}

	argv[argc++] = fstr;

	_log_evalpid(argv, NULL, 0, NULL);
}

static void checkhostapd(char *ifname, int force)
{
	int pid = 0;
	int sup = 0;
	char fname[32];
	FILE *fp = NULL;
	if (nvram_nmatch("mesh", "%s_mode", ifname) || nvram_nmatch("sta", "%s_mode", ifname) ||
	    nvram_nmatch("wdssta", "%s_mode", ifname) || nvram_nmatch("wdssta_mtik", "%s_mode", ifname) ||
	    nvram_nmatch("infra", "%s_mode", ifname))
		sup = 1;
	if (sup) {
		sprintf(fname, "/var/run/%s_wpa_supplicant.pid", ifname);
		fp = fopen(fname, "rb");
	} else {
		sprintf(fname, "/var/run/%s_hostapd.pid", ifname);
		fp = fopen(fname, "rb");
	}

	if (!fp && force == 2) {
		force = 1;
	}
	if (fp || force == 1) {
		if (fp) {
			fscanf(fp, "%d", &pid);
			fclose(fp);
		}
		if (pid > 0 || force == 1) {
			int needrestart = 0;
			if (force == 1) {
				needrestart = 1;
				if (pid > 0)
					kill(pid, SIGKILL);
				fp = NULL;
			} else {
				char checkname[32];
				sprintf(checkname, "/proc/%d/cmdline", pid);
				fp = fopen(checkname, "rb");
				needrestart = 0;
			}
			if (!fp) {
				needrestart = 1;
			} else {
				char cmdline[128];
				fscanf(fp, "%s", cmdline);
				if (strncmp(cmdline, "hostapd", 7) && strncmp(cmdline, "wpa_supplicant", 14))
					needrestart = 1;
				fclose(fp);
			}
			if (needrestart) {
				char fstr[32];
				if (sup)
					sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", ifname);
				else
					sprintf(fstr, "/tmp/%s_hostap.conf", ifname);
				if (force == 1) {
					dd_loginfo(sup ? "wpa_supplicant" : "hostapd",
						   "daemon on %s with pid %d is forced to be restarted....\n", ifname, pid);
				} else {
					dd_loginfo(sup ? "wpa_supplicant" : "hostapd",
						   "daemon on %s with pid %d died, restarting....\n", ifname, pid);
				}
				if (sup) {
					start_deconfigurewifi();
					start_configurewifi();
					start_wan_boot();
					return;
				}
				do_hostapd(fstr, ifname);
				char *next;
				char var[80];
				if (!nvram_nmatch("sta", "%s_mode", ifname)) {
					char bridged[32];
					sprintf(bridged, "%s_bridged", ifname);
					if (nvram_matchi(bridged, 0)) {
						eval("ifconfig", ifname, "mtu", getMTU(ifname));
						eval("ifconfig", ifname, "txqueuelen", getTXQ(ifname));
						eval("ifconfig", ifname, nvram_nget("%s_ipaddr", ifname), "netmask",
						     nvram_nget("%s_netmask", ifname), "up");
					}
				}
				char *vifs = nvram_nget("%s_vifs", ifname);
				foreach(var, vifs, next)
				{
					char bridged[32];
					sprintf(bridged, "%s_bridged", var);
					if (nvram_matchi(bridged, 0)) {
						eval("ifconfig", var, "mtu", getMTU(var));
						eval("ifconfig", var, "txqueuelen", getTXQ(var));
						eval("ifconfig", var, nvram_nget("%s_ipaddr", var), "netmask",
						     nvram_nget("%s_netmask", var), "up");
					}
				}
			}
		}
	}
}

static void s_checkhostapd(int force)
{
	char *next, *vifs;
	char wifivifs[32];
	char var[80];
	int c = getdevicecount();
	char athname[32];
	int i;
	for (i = 0; i < c; i++) {
		sprintf(athname, "wlan%d", i);
		if (!nvram_nmatch("disabled", "%s_net_mode", athname)) {
			//okay, these modes might run hostapd and may cause troubles if the radius gets unavailable
			checkhostapd(athname, force);
			if (!is_mac80211(athname)) {
				sprintf(wifivifs, "%s_vifs", athname);
				vifs = nvram_safe_get(wifivifs);
				if (vifs != NULL && *vifs) {
					foreach(var, vifs, next)
					{
						checkhostapd(var, force);
					}
				}
			}
		}
	}
}

void start_checkhostapd(void)
{
	s_checkhostapd(0);
}

void start_restarthostapd(void)
{
	s_checkhostapd(1);
}

void start_restarthostapd_ifneeded(void)
{
	s_checkhostapd(2);
}

#ifdef HAVE_WPS
//loaned from hostapd
void get_uuid(char *uuid_str)
{
	unsigned char mac[6];
	get_ether_hwaddr("eth0", mac);

	const unsigned char *addr[2];
	unsigned int len[2];
	unsigned char hash[20];
	unsigned char nsid[16] = { 0x52, 0x64, 0x80, 0xf8, 0xc9, 0x9b, 0x4b, 0xe5, 0xa6, 0x55, 0x58, 0xed, 0x5f, 0x5d, 0x60, 0x84 };
	unsigned char bin[16];
	sha1_ctx_t ctx;

	addr[0] = nsid;
	len[0] = sizeof(nsid);
	addr[1] = (unsigned char *)mac;
	len[1] = 6;

	sha1_begin(&ctx);
	sha1_hash(addr[0], len[0], &ctx);
	sha1_hash(addr[1], len[1], &ctx);
	sha1_end(hash, &ctx);
	memcpy(bin, hash, 16);

	/* Version: 5 = named-based version using SHA-1 */
	bin[6] = (5 << 4) | (bin[6] & 0x0f);

	/* Variant specified in RFC 4122 */
	bin[8] = 0x80 | (bin[8] & 0x3f);

	sprintf(uuid_str,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-"
		"%02x%02x-%02x%02x%02x%02x%02x%02x",
		bin[0], bin[1], bin[2], bin[3], bin[4], bin[5], bin[6], bin[7], bin[8], bin[9], bin[10], bin[11], bin[12], bin[13],
		bin[14], bin[15]);
}

#endif

#ifdef HAVE_HOTSPOT20
void setupHS20(FILE *fp, char *prefix)
{
	if (nvram_nmatch("1", "%s_hs20", prefix)) {
		fprintf(fp, "hs20=1\n");
		if (nvram_nmatch("1", "%s_disable_dgaf", prefix))
			fprintf(fp, "disable_dgaf=1\n");
		else
			fprintf(fp, "disable_dgaf=0\n");

		if (nvram_nmatch("1", "%s_osen", prefix))
			fprintf(fp, "osen=1\n");
		else
			fprintf(fp, "osen=0\n");

		if (nvram_nmatch("", "%s_anqp_domain_id", prefix))
			fprintf(fp, "anqp_domain_id=1234\n");
		else
			fprintf(fp, "anqp_domain_id=%s\n", nvram_nget("%s_anqp_domain_id", prefix));

		if (nvram_nmatch("", "%s_hs20_deauth_req_timeout", prefix))
			fprintf(fp, "hs20_deauth_req_timeout=60\n");
		else
			fprintf(fp, "hs20_deauth_req_timeout=%s\n", nvram_nget("%s_hs20_deauth_req_timeout", prefix));

		int i;
		for (i = 0; i < 10; i++) {
			if (nvram_nmatch("", "%s_hs20_oper_friendly_name%d", prefix, i))
				continue;
			fprintf(fp, "hs20_oper_friendly_name=%s\n", nvram_nget("%s_hs20_oper_friendly_name%d", prefix, i));
		}

		for (i = 0; i < 10; i++) {
			if (nvram_nmatch("", "%s_hs20_conn_capab%d", prefix, i))
				continue;
			fprintf(fp, "hs20_conn_capab=%s\n", nvram_nget("%s_hs20_conn_capab%d", prefix, i));
		}
		if (nvram_nmatch("", "%s_hs20_wan_metrics", prefix))
			fprintf(fp, "hs20_wan_metrics=01:50000:50000:80:240:0\n");
		else
			fprintf(fp, "hs20_wan_metrics=%s\n", nvram_nget("%s_hs20_wan_metrics", prefix));

		if (nvram_nmatch("", "%s_hs20_operating_class", prefix))
			fprintf(fp, "hs20_operating_class=5173\n");
		else
			fprintf(fp, "hs20_operating_class=%s\n", nvram_nget("%s_hs20_operating_class", prefix));

		if (!nvram_nmatch("", "%s_osu_ssid", prefix))
			fprintf(fp, "osu_ssid=\"%s\"\n", nvram_nget("%s_osu_ssid", prefix));

		if (nvram_nmatch("", "%s_osu_server_uri", prefix, i))
			fprintf(fp, "osu_server_uri=%s\n", nvram_nget("%s_osu_server_uri", prefix));

		for (i = 0; i < 10; i++) {
			if (nvram_nmatch("", "%s_osu_friendly_name%d", prefix, i))
				continue;
			fprintf(fp, "osu_friendly_name=%s\n", nvram_nget("%s_osu_friendly_name%d", prefix, i));
		}
		if (nvram_nmatch("", "%s_nai", prefix, i))
			fprintf(fp, "osu_nai=%s\n", nvram_nget("%s_osu_nai", prefix));

		if (nvram_nmatch("", "%s_osu_method_list", prefix, i))
			fprintf(fp, "osu_method_list=%s\n", nvram_nget("%s_osu_method_list", prefix));
		for (i = 0; i < 10; i++) {
			if (nvram_nmatch("", "%s_osu_service_desc%d", prefix, i))
				continue;
			fprintf(fp, "osu_service_desc=%s\n", nvram_nget("%s_osu_service_desc%d", prefix, i));
		}
	}
}
#endif

void addWPS(FILE *fp, char *prefix, int configured)
{
#ifdef HAVE_WPS
	char *config_methods;
	asprintf(&config_methods, "label keypad");
	fprintf(fp, "ctrl_interface=/var/run/hostapd\n"); // for cli
	if (!strcmp(prefix, "wlan0") || !strcmp(prefix, "wlan1")) {
		fprintf(fp, "eap_server=1\n");
		if (nvram_matchi("wps_enabled", 1)) {
			config_methods = (char *)realloc(config_methods, strlen(config_methods) + sizeof(" push_button"));
			strcat(config_methods, " push_button");
		}
		//# WPS configuration (AP configured, do not allow external WPS Registrars)
		if (nvram_matchi("wps_forcerelease", 1)) {
			nvram_seti("wps_status", 0);
			nvram_async_commit();
			fprintf(fp, "wps_state=1\n");
		} else {
			if (configured) {
				if (nvram_match("wps_status", "")) {
					nvram_seti("wps_status", 1);
					nvram_async_commit();
				}
			} else {
				if (nvram_match("wps_status", "")) {
					nvram_seti("wps_status", 0);
					nvram_async_commit();
				}
			}

			if (nvram_matchi("wps_status", 0)) {
				nvram_seti("wps_status", 0);
				fprintf(fp, "wps_state=1\n");
			} else {
				nvram_seti("wps_status", 1);
				fprintf(fp, "wps_state=2\n");
			}
		}
		if (nvram_matchi("wps_registrar", 1)) {
			fprintf(fp, "ap_setup_locked=0\n");
			fprintf(fp, "upnp_iface=%s\n", nvram_safe_get("lan_ifname"));
			fprintf(fp, "model_description=Wireless Access Point\n");
			//# If UUID is not configured, it will be generated based on local MAC address.
			char uuid[64];
			get_uuid(uuid);
			fprintf(fp, "uuid=%s\n", uuid);
			//# In case of external registrar add conf for non-conforming Windows 7 / Vista Clients
			fprintf(fp, "pbc_in_m1=1\n");
		} else
			fprintf(fp, "ap_setup_locked=1\n");
//              fprintf(fp, "ap_pin=%s\n",nvram_safe_get("pincode"));
#ifdef HAVE_WZRHPAG300NH
		fprintf(fp, "dualband=1\n");
#endif
		fprintf(fp, "wps_pin_requests=/var/run/hostapd.pin-req\n");
		fprintf(fp, "device_name=%s\n", nvram_safe_get("router_name"));
		fprintf(fp, "manufacturer=DD-WRT\n");
		fprintf(fp, "model_name=%s\n", nvram_safe_get("DD_BOARD"));
		fprintf(fp, "model_number=0\n");
		fprintf(fp, "serial_number=12345\n");
		fprintf(fp, "device_type=6-0050F204-1\n");
		fprintf(fp, "os_version=01020300\n");
#ifdef HAVE_BUFFALO
		fprintf(fp, "friendly_name=BUFFALO %s\n", nvram_safe_get("DD_BOARD"));
#else
		fprintf(fp, "friendly_name=DD-WRT WPS Access Point\n");
#endif
		fprintf(fp, "config_methods=%s\n", config_methods);
		//      "config_methods=label display push_button keypad\n");
	}
	free(config_methods);
#endif
}

void start_ses_led_control(void)
{
	char ath[32];
	char net[32];
	char *next;
	char var[80];
	char akm[16];
	int c = getdevicecount();
	int i;
	led_control(LED_SEC0, LED_OFF);
	led_control(LED_SEC1, LED_OFF);

	for (i = 0; i < c; i++) {
		sprintf(ath, "wlan%d", i);
		sprintf(net, "%s_net_mode", ath);
		if (nvram_match(net, "disabled"))
			continue;
		if (nvram_nmatch("ap", "%s_mode", ath) || nvram_nmatch("wdsap", "%s_mode", ath)) {
			sprintf(akm, "%s_akm", ath);
			if (nvhas(akm, "psk") || nvhas(akm, "psk2") || nvhas(akm, "psk3") || nvhas(akm, "owe") ||
			    nvhas(akm, "psk2-sha256") || nvhas(akm, "wpa") || nvhas(akm, "wpa2") || nvhas(akm, "wpa3") ||
			    nvhas(akm, "wpa3-128") || nvhas(akm, "wpa3-192") || nvhas(akm, "wpa2-sha256") || nvhas(akm, "wpa2-sha384") ||
			    nvram_match(akm, "wep")) {
				if (!strncmp(ath, "wlan0", 4))
					led_control(LED_SEC0, LED_ON);
				if (!strncmp(ath, "wlan1", 4))
					led_control(LED_SEC1, LED_ON);
			}
		}
		char *vifs = nvram_nget("wlan%d_vifs", i);

		if (vifs != NULL)
			foreach(var, vifs, next)
			{
				sprintf(akm, "%s_akm", var);
				if (nvhas(akm, "psk") || nvhas(akm, "psk2") || nvhas(akm, "psk3") || nvhas(akm, "owe") ||
				    nvhas(akm, "psk2-sha256") || nvhas(akm, "wpa") || nvhas(akm, "wpa2") || nvhas(akm, "wpa3") ||
				    nvhas(akm, "wpa3-128") || nvhas(akm, "wpa3-192") || nvhas(akm, "wpa2-sha256") || nvhas(akm, "wpa2-sha384") ||
				    nvram_match(akm, "wep")) {
					if (!strncmp(var, "wlan0", 4))
						led_control(LED_SEC0, LED_ON);
					if (!strncmp(var, "wlan1", 4))
						led_control(LED_SEC1, LED_ON);
				}
			}
	}
}

extern char *hostapd_eap_get_types(void);

void setupHostAPPSK(FILE *fp, char *prefix, int isfirst)
{
	char akm[16];
	char mfp[16];
	char mbo[16];
	char ft[16];
	char tmp[256];
	char rekey[32];
	char *debug;
	char *types;
	char eapol[32];
	sprintf(eapol, "%s_eapol_version", prefix);

	sprintf(akm, "%s_akm", prefix);
	sprintf(ft, "%s_ft", prefix);
	sprintf(mfp, "%s_mfp", prefix);
	sprintf(mbo, "%s_mbo", prefix);

	const int _has_wpa3 = has_wpa3(prefix);
	const int ispsk2 = nvhas(akm, "psk2");
	const int ispsk = nvhas(akm, "psk");
	const int ispsk3 = _has_wpa3 ? nvhas(akm, "psk3") : 0;
	const int isowe = nvhas(akm, "owe");
	const int iswpa = nvhas(akm, "wpa");
	const int iswpa2 = nvhas(akm, "wpa2");
	const int iswpa3 = _has_wpa3 ? nvhas(akm, "wpa3") : 0;
	const int iswpa3_192 = _has_wpa3 ? nvhas(akm, "wpa3-192") : 0;
	const int iswpa3_128 = _has_wpa3 ? nvhas(akm, "wpa3-128") : 0;
	const int iswpa2sha256 = _has_wpa3 ? nvhas(akm, "wpa2-sha256") : 0;
	const int iswpa2sha384 = _has_wpa3 ? nvhas(akm, "wpa2-sha384") : 0;
	const int ispsk2sha256 = _has_wpa3 ? nvhas(akm, "psk2-sha256") : 0;
	const int iswep = nvhas(akm, "wep");

	if (!strncmp(prefix, "wlan0", 4))
		led_control(LED_SEC0, LED_ON);
	if (!strncmp(prefix, "wlan1", 4))
		led_control(LED_SEC1, LED_ON);
	// sprintf(buf, "rsn_preauth_interfaces=%s\n", "br0");
	if (nvram_nmatch("1", "%s_bridged", prefix))
		fprintf(fp, "bridge=%s\n", getBridge(prefix, tmp));
	fprintf(fp, "logger_syslog=-1\n");
	debug = nvram_nget("%s_wpa_debug", prefix);
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
	// fprintf (fp, "eap_server=0\n");
	// fprintf (fp, "own_ip_addr=127.0.0.1\n");
	fprintf(fp, "eapol_version=%s\n", nvram_default_get(eapol, "1"));
	fprintf(fp, "eapol_key_index_workaround=0\n");
	char eap_key_retries[32];
	sprintf(eap_key_retries, "%s_disable_eapol_key_retries", prefix);
	if (nvram_default_matchi(eap_key_retries, 1, 0)) {
		fprintf(fp, "wpa_disable_eapol_key_retries=1\n");
	}
	int wpamask = 0;
	if (ispsk || iswpa)
		wpamask |= 1;
	if (ispsk2 || ispsk3 || isowe || iswpa2 || iswpa3 || iswpa3_192 || iswpa3_128 || iswpa2sha256 || iswpa2sha384 || ispsk2sha256)
		wpamask |= 2;
	fprintf(fp, "wpa=%d\n", wpamask);
	if (ispsk)
		nvram_nseti(1, "%s_psk", prefix);
	if (ispsk2)
		nvram_nseti(1, "%s_psk2", prefix);
	if (ispsk2sha256)
		nvram_nseti(1, "%s_psk2-sha256", prefix);
	if (ispsk3)
		nvram_nseti(1, "%s_psk3", prefix);
	if (isowe)
		nvram_nseti(1, "%s_owe", prefix);
	if (iswpa)
		nvram_nseti(1, "%s_wpa", prefix);
	if (iswpa2)
		nvram_nseti(1, "%s_wpa2", prefix);
	if (iswpa2sha256)
		nvram_nseti(1, "%s_wpa2-sha256", prefix);
	if (iswpa2sha384)
		nvram_nseti(1, "%s_wpa2-sha384", prefix);
	if (iswpa3)
		nvram_nseti(1, "%s_wpa3", prefix);
	if (iswpa3_128)
		nvram_nseti(1, "%s_wpa3-128", prefix);
	if (iswpa3_192)
		nvram_nseti(1, "%s_wpa3-192", prefix);
#ifdef HAVE_80211W
	if ((iswpa3 || iswpa3_128 || iswpa3_192 || iswpa2sha256  || iswpa2sha384 || ispsk2sha256 || isowe || ispsk3) &&
	    (!ispsk && !ispsk2 && !iswpa && !iswpa2)) {
		fprintf(fp, "ieee80211w=2\n");
		if (ispsk3 || iswpa3 || iswpa3_192 || iswpa3_128 || isowe)
			fprintf(fp, "sae_require_mfp=1\n");
	} else if (ispsk3 || ispsk2sha256 || isowe && (!ispsk && !ispsk2 && !iswpa && !iswpa2)) {
		fprintf(fp, "ieee80211w=1\n");
		if (ispsk3 || iswpa3 || iswpa3_192 || iswpa3_128 || isowe)
			fprintf(fp, "sae_require_mfp=1\n");
	} else if (nvram_default_matchi(mfp, 1, 0)) {
		fprintf(fp, "ieee80211w=2\n");
		if (ispsk3 || iswpa3 || iswpa3_192 || iswpa3_128 || isowe)
			fprintf(fp, "sae_require_mfp=1\n");
	} else if (nvram_default_matchi(mfp, -1, 0) || ispsk3 || isowe || iswpa3 || iswpa3_192 || iswpa3_128 || ispsk2sha256 ||
		   iswpa2sha256 || iswpa2sha384) {
		fprintf(fp, "ieee80211w=1\n");
		if (ispsk3 || iswpa3 || iswpa3_192 || iswpa3_128 || isowe)
			fprintf(fp, "sae_require_mfp=1\n");
	} else if (nvram_default_matchi(mbo, 1, 0))
		fprintf(fp, "ieee80211w=1\n");
	else if (nvram_default_matchi(mfp, 0, 0))
		fprintf(fp, "ieee80211w=0\n");
#endif
	if (ispsk3 && !ispsk && !ispsk2 && !ispsk2sha256) {
		char *sae_key = nvram_nget("%s_sae_key", prefix);
		fprintf(fp, "sae_password=%s\n", sae_key);
	} else if (ispsk || ispsk2 || ispsk2sha256 || ispsk3) {
		if (strlen(nvram_nget("%s_wpa_psk", prefix)) == 64)
			fprintf(fp, "wpa_psk=%s\n", nvram_nget("%s_wpa_psk", prefix));
		else
			fprintf(fp, "wpa_passphrase=%s\n", nvram_nget("%s_wpa_psk", prefix));
	}
	fprintf(fp, "wpa_key_mgmt=");
	if (ispsk2 || ispsk)
		fprintf(fp, "WPA-PSK ");
	if (_has_wpa3 && ispsk3)
		fprintf(fp, "SAE ");
	if (_has_wpa3 && isowe)
		fprintf(fp, "OWE ");
	if (_has_wpa3 && ispsk2sha256)
		fprintf(fp, "WPA-PSK-SHA256 ");
	if (iswpa2 || iswpa || iswpa3)
		fprintf(fp, "WPA-EAP ");
	if (_has_wpa3 && iswpa2sha256)
		fprintf(fp, "WPA-EAP-SHA256 ");
	if (_has_wpa3 && iswpa2sha384)
		fprintf(fp, "WPA-EAP-SHA384 ");
	if (_has_wpa3 && iswpa3_128)
		fprintf(fp, "WPA-EAP-SUITE-B ");
	if (_has_wpa3 && iswpa3_192)
		fprintf(fp, "WPA-EAP-SUITE-B-192 ");
#ifdef HAVE_80211R
	if (_has_wpa3 && nvram_matchi(ft, 1) && ispsk3)
		fprintf(fp, "FT-SAE ");
	if (nvram_matchi(ft, 1) && (ispsk2 || ispsk || ispsk2sha256))
		fprintf(fp, "FT-PSK ");
	if (nvram_matchi(ft, 1) && (iswpa3_192))
		fprintf(fp, "FT-EAP-SHA384 ");
	if (nvram_matchi(ft, 1) && (iswpa || iswpa2 || iswpa3 || iswpa2sha256 || iswpa2sha384 || iswpa3_128))
		fprintf(fp, "FT-EAP ");
#endif
	fprintf(fp, "\n");
	if (_has_wpa3 && isowe) {
		fprintf(fp, "owe_transition_ifname=%s\n", nvram_nget("%s_owe_ifname", prefix));
		fprintf(fp, "owe_groups=19 20 21\n");
	}
	if (_has_wpa3 && ispsk3)
		fprintf(fp, "sae_groups=19 20 21\n");
#ifdef HAVE_80211R
	if (nvram_matchi(ft, 1) &&
	    (ispsk3 || ispsk || ispsk2 || ispsk2sha256 || iswpa || iswpa2 || iswpa3 || iswpa2sha256  || iswpa2sha384 || iswpa3_128 || iswpa3_192)) {
		char dl[32];
		fprintf(fp, "nas_identifier=%s\n", nvram_nget("%s_nas", prefix));
		fprintf(fp, "mobility_domain=%s\n", nvram_nget("%s_domain", prefix));
		sprintf(dl, "%s_ft_over_ds", prefix);
		fprintf(fp, "ft_over_ds=%d\n", nvram_default_geti(dl, 0));
		fprintf(fp, "ft_psk_generate_local=1\n");
		fprintf(fp, "pmk_r1_push=1\n");
		sprintf(dl, "%s_deadline", prefix);
		fprintf(fp, "reassociation_deadline=%d\n", nvram_default_geti(dl, 1000));
		// todo. add key holders
	}
#endif
	if (iswpa || iswpa2 || iswpa3 || iswpa3_128 || iswpa3_192 || iswpa2sha256 || iswpa2sha384) {
		fprintf(fp, "ieee8021x=1\n");
		char local_ip[32];
		sprintf(local_ip, "%s_local_ip", prefix);
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
		sprintf(retry, "%s_radius_retry", prefix);
		fprintf(fp, "radius_retry_primary_interval=%s\n", nvram_default_get(retry, "600"));
		types = hostapd_eap_get_types();
		fprintf(fp, "%s", types);
		free(types);
		fprintf(fp, "auth_server_addr=%s\n", nvram_nget("%s_radius_ipaddr", prefix));
		fprintf(fp, "auth_server_port=%s\n", nvram_nget("%s_radius_port", prefix));
		fprintf(fp, "auth_server_shared_secret=%s\n", nvram_nget("%s_radius_key", prefix));
		char check[64];
		sprintf(check, "%s_radius2_ipaddr", prefix);
		nvram_default_get(check, "0.0.0.0");
		if (!nvram_nmatch("", "%s_radius2_ipaddr", prefix) && !nvram_nmatch("0.0.0.0", "%s_radius2_ipaddr", prefix) &&
		    !nvram_nmatch("", "%s_radius2_port", prefix)) {
			fprintf(fp, "auth_server_addr=%s\n", nvram_nget("%s_radius2_ipaddr", prefix));
			fprintf(fp, "auth_server_port=%s\n", nvram_nget("%s_radius2_port", prefix));
			fprintf(fp, "auth_server_shared_secret=%s\n", nvram_nget("%s_radius2_key", prefix));
		}
		if (nvram_nmatch("1", "%s_acct", prefix)) {
			fprintf(fp, "acct_server_addr=%s\n", nvram_nget("%s_acct_ipaddr", prefix));
			fprintf(fp, "acct_server_port=%s\n", nvram_nget("%s_acct_port", prefix));
			fprintf(fp, "acct_server_shared_secret=%s\n", nvram_nget("%s_acct_key", prefix));
		}
	}
	char pwstring[128] = { 0 };

	get_pairwise(prefix, pwstring, NULL, 0, 0);

	if (!*pwstring) {
		char psk[32];
		sprintf(psk, "%s_crypto", prefix);
		if (nvram_match(psk, "aes")) {
			nvram_nseti(1, "%s_ccmp", prefix);
			fprintf(fp, "wpa_pairwise=CCMP\n");
		}
		if (nvram_match(psk, "ccmp-256")) {
			nvram_nseti(1, "%s_ccmp-256", prefix);
			fprintf(fp, "wpa_pairwise=CCMP-256\n");
		}
		if (nvram_match(psk, "gcmp")) {
			nvram_nseti(1, "%s_gcmp", prefix);
			fprintf(fp, "wpa_pairwise=GCMP\n");
		}
		if (nvram_match(psk, "gcmp-256")) {
			nvram_nseti(1, "%s_gcmp-256", prefix);
			fprintf(fp, "wpa_pairwise=GCMP-256\n");
		}
		if (nvram_match(psk, "tkip")) {
			nvram_nseti(1, "%s_tkip", prefix);
			if (!isfirst)
				fprintf(fp, "ieee80211n=0\n");
			fprintf(fp, "wpa_pairwise=TKIP\n");
		}
		if (nvram_match(psk, "tkip+aes")) {
			nvram_nseti(1, "%s_ccmp", prefix);
			nvram_nseti(1, "%s_tkip", prefix);
			fprintf(fp, "wpa_pairwise=TKIP CCMP\n");
		}
	} else {
		fprintf(fp, "wpa_pairwise=%s\n", pwstring);
		if (iswpa3_192)
			fprintf(fp, "group_mgmt_cipher=BIP-GMAC-256\n");
		else if (iswpa3_128)
			fprintf(fp, "group_mgmt_cipher=BIP-GMAC-128\n");
#ifdef HAVE_80211W
		else if (nvram_default_matchi(mfp, -1, 0) || nvram_default_matchi(mfp, 1, 0) || ispsk3 || isowe || iswpa3 ||
			 ispsk2sha256 || iswpa2sha256 || iswpa2sha384)
			fprintf(fp, "group_mgmt_cipher=AES-128-CMAC\n");
#endif
	}
	if (ispsk3 || iswpa3 || iswpa3_192 || iswpa3_128 || isowe) {
		fprintf(fp, "okc=1\n");
	} else {
		fprintf(fp, "okc=0\n");
		fprintf(fp, "disable_pmksa_caching=1\n");
	}
	sprintf(rekey, "%s_wpa_strict_rekey", prefix);
	fprintf(fp, "wpa_group_rekey=%s\n", nvram_nget("%s_wpa_gtk_rekey", prefix));
	fprintf(fp, "wpa_strict_rekey=%d\n", nvram_default_geti(rekey, 0));
	if (ispsk3 || ispsk || ispsk2 || ispsk2sha256)
		addWPS(fp, prefix, 1);
}

#ifdef HAVE_MADWIFI

void setupHostAP(char *prefix, char *driver, int iswan)
{
#ifdef HAVE_REGISTER
	if (!isregistered())
		return;
#endif
	char psk[32];
	char akm[16];
	char fstr[32];
	char tmp[256];
	char *types;
	char mfp[16];
	char ft[16];

	sprintf(akm, "%s_akm", prefix);
	sprintf(ft, "%s_ft", prefix);
	sprintf(mfp, "%s_mfp", prefix);
	if (nvram_match(akm, "8021X"))
		return;
	int ispsk2 = nvhas(akm, "psk2");
	int ispsk2sha256 = nvhas(akm, "psk2-sha256");
	int ispsk = nvhas(akm, "psk");
	int ispsk3 = nvhas(akm, "psk3");
	int iswpa = nvhas(akm, "wpa");
	int iswpa2 = nvhas(akm, "wpa2");
	int iswpa2sha256 = nvhas(akm, "wpa2-sha256");
	int iswpa2sha384 = nvhas(akm, "wpa2-sha384");
	int iswpa3 = nvhas(akm, "wpa3");
	int iswpa3_192 = nvhas(akm, "wpa3-192");
	int iswpa3_128 = nvhas(akm, "wpa3-128");
	int iswep = nvhas(akm, "wep");
	check_cryptomod(prefix);

	// wep key support
	if (iswep) {
		if (!strncmp(prefix, "wlan0", 4))
			led_control(LED_SEC0, LED_ON);
		if (!strncmp(prefix, "wlan1", 4))
			led_control(LED_SEC1, LED_ON);
		sprintf(fstr, "/tmp/%s_hostap.conf", prefix);
		FILE *fp = fopen(fstr, "wb");
		fprintf(fp, "interface=%s\n", prefix);
		if (nvram_nmatch("1", "%s_bridged", prefix))
			fprintf(fp, "bridge=%s\n", getBridge(prefix, tmp));
		fprintf(fp, "driver=%s\n", driver);
		fprintf(fp, "logger_syslog=-1\n");
		fprintf(fp, "logger_syslog_level=2\n");
		fprintf(fp, "logger_stdout=-1\n");
		fprintf(fp, "logger_stdout_level=2\n");
		fprintf(fp, "debug=0\n");
		char *authmode = nvram_nget("%s_authmode", prefix);
		if (!strcmp(authmode, "shared"))
			fprintf(fp, "auth_algs=2\n");
		else if (!strcmp(authmode, "auto"))
			fprintf(fp, "auth_algs=3\n");
		else
			fprintf(fp, "auth_algs=1\n");
		int i;
		for (i = 1; i < 5; i++) {
			char *athkey = nvram_nget("%s_key%d", prefix, i);
			if (athkey != NULL && *athkey) {
				fprintf(fp, "wep_key%d=%s\n", i - 1, athkey);
			}
		}
		fprintf(fp, "wep_default_key=%d\n", atoi(nvram_nget("%s_key", prefix)) - 1);
		addWPS(fp, prefix, 1);
		fclose(fp);
		do_hostapd(fstr, prefix);

	} else if (ispsk || ispsk2 || ispsk3 || iswpa || iswpa2 || iswpa3 || iswpa3_128 || iswpa3_192 || iswpa2sha256 || iswpa2sha384 ||
		   ispsk2sha256) {
		sprintf(fstr, "/tmp/%s_hostap.conf", prefix);
		FILE *fp = fopen(fstr, "wb");
		fprintf(fp, "interface=%s\n", prefix);
		fprintf(fp, "driver=%s\n", driver);
		setupHostAPPSK(fp, prefix, 1);

		// fprintf (fp, "jumpstart_p1=1\n");
#ifdef HAVE_HOTSPOT20
		setupHS20(fp, prefix);
#endif
		char *v = nvram_nget("%s_config", prefix);
		fprintf(fp, "\n");
		if (v && *v)
			fprintf(fp, "%s", v);
		fclose(fp);
		do_hostapd(fstr, prefix);

	} else if (nvram_match(akm, "radius")) {
		// wrt-radauth $IFNAME $server $port $share $override $mackey $maxun
		// &
		char *ifname = prefix;
		char *server = nvram_nget("%s_radius_ipaddr", prefix);
		char *port = nvram_nget("%s_radius_port", prefix);
		char *share = nvram_nget("%s_radius_key", prefix);
		char exec[64];
		char type[32];

		sprintf(type, "%s_radmactype", prefix);
		char *pragma = "";

		if (nvram_default_matchi(type, 0, 0))
			pragma = "-n1 ";
		if (nvram_matchi(type, 1))
			pragma = "-n2 ";
		if (nvram_matchi(type, 2))
			pragma = "-n3 ";
		if (nvram_matchi(type, 3))
			pragma = "";
		sysprintf("wrt-radauth %s %s %s %s %s 1 1 0 &", pragma, prefix, server, port, share);
	} else {
		eval("iwconfig", prefix, "key", "off");
	}
}
#endif
void start_hostapdwan(void)
{
	/*	char ath[32];
	char *next;
	char var[80];
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		sprintf(ath, "wlan%d", i);
		if (nvram_nmatch("ap", "%s_mode", ath)
		    || nvram_nmatch("wdsap", "%s_mode", ath)) {
			setupHostAP(ath, "madwifi", 1);
		}
		char *vifs = nvram_nget("wlan%d_vifs", i);

		if (vifs != NULL)
			foreach(var, vifs, next) {
			setupHostAP(var, "madwifi", 1);
			}
	}
*/
}

#ifdef HAVE_MADWIFI
#define SIOCSSCANLIST (SIOCDEVPRIVATE + 6)
static void set_scanlist(char *dev, char *wif)
{
	char var[32];
	char *next;
	struct iwreq iwr;
	char scanlist[32];
	char list[64];

	sprintf(scanlist, "%s_scanlist", dev);
	char *sl = nvram_default_get(scanlist, "default");
	int c = 0;

	eval("iwpriv", dev, "setscanlist", "-ALL");
	if (*sl && strcmp(sl, "default")) {
		foreach(var, sl, next)
		{
			sprintf(list, "+%s", var);
			eval("iwpriv", dev, "setscanlist", list);
		}
	} else {
		eval("iwpriv", dev, "setscanlist", "+ALL");
	}
}

static void set_rate(char *dev, char *priv)
{
	char rate[32];
	char maxrate[32];
	char net[32];
	char bw[32];
	char xr[32];

	sprintf(bw, "%s_channelbw", dev);
	sprintf(net, "%s_net_mode", dev);
	sprintf(rate, "%s_minrate", dev);
	sprintf(maxrate, "%s_maxrate", dev);
	sprintf(xr, "%s_xr", dev);
	char *r = nvram_default_get(rate, "0");
	char *mr = nvram_default_get(maxrate, "0");

#ifdef HAVE_WHRAG108
	char *netmode;

	if (!strcmp(dev, "wlan0"))
		netmode = nvram_default_get(net, "a-only");
	else
		netmode = nvram_default_get(net, "mixed");
#else
	char *netmode = nvram_default_get(net, "mixed");
#endif

	if (nvram_matchi(bw, 20) && nvram_matchi(xr, 0))
		if (atof(r) == 27.0f || atof(r) == 1.5f || atof(r) == 2.0f || atof(r) == 3.0f || atof(r) == 4.5f ||
		    atof(r) == 9.0f || atof(r) == 13.5f) {
			nvram_seti(rate, 0);
			r = "0";
		}
	if (nvram_matchi(bw, 40))
		if (atof(r) == 27.0f || atof(r) == 1.5f || atof(r) == 2.0f || atof(r) == 3.0f || atof(r) == 4.5f ||
		    atof(r) == 9.0f || atof(r) == 13.5f) {
			nvram_seti(rate, 0);
			r = "0";
		}
	if (nvram_matchi(bw, 10))
		if (atof(r) > 27.0f || atof(r) == 1.5f || atof(r) == 2.0f || atof(r) == 13.5f) {
			nvram_seti(rate, 0);
			r = "0";
		}
	if (nvram_matchi(bw, 5))
		if (atof(r) > 13.5) {
			nvram_seti(rate, 0);
			r = "0";
		}
	if (nvram_matchi(bw, 2))
		if (atof(r) > 6.75) {
			nvram_seti(rate, 0);
			r = "0";
		}
	if (!strcmp(netmode, "b-only"))
		eval("iwconfig", priv, "rate", "11M", "auto");
	// else {
	// sysprintf("iwconfig %s rate 54M auto", priv);
	// }
	if (atol(mr) > 0)
		eval("iwpriv", priv, "maxrate", mr);
	if (atoi(r) > 0)
		eval("iwpriv", priv, "minrate", r);
}

static void set_netmode(char *wif, char *dev, char *use)
{
	char net[16];
	char mode[16];
	char xr[16];
	char comp[32];
	char ff[16];
	char bw[16];

	sprintf(mode, "%s_mode", dev);
	sprintf(net, "%s_net_mode", dev);
	sprintf(bw, "%s_channelbw", dev);
	sprintf(xr, "%s_xr", dev);
	//    sprintf( comp, "%s_compression", dev );
	sprintf(ff, "%s_ff", dev);
#ifdef HAVE_WHRAG108
	char *netmode;

	if (!strcmp(dev, "wlan0"))
		netmode = nvram_default_get(net, "a-only");
	else
		netmode = nvram_default_get(net, "mixed");
#else
	char *netmode = nvram_default_get(net, "mixed");
#endif
	// fprintf (stderr, "set netmode of %s to %s\n", net, netmode);
	cprintf("configure net mode %s\n", netmode);

	{
#ifdef HAVE_WHRAG108
		if (!strncmp(use, "wlan0", 4)) {
			eval("iwpriv", use, "mode", "1");
		} else
#endif
#ifdef HAVE_TW6600
			if (!strncmp(use, "wlan0", 4)) {
			eval("iwpriv", use, "mode", "1");
		} else
#endif
		{
			eval("iwpriv", use, "xr", "0");
			if (!strcmp(netmode, "mixed"))
				eval("iwpriv", use, "mode", "0");
			if (!strcmp(netmode, "b-only"))
				eval("iwpriv", use, "mode", "2");
			if (!strcmp(netmode, "g-only")) {
				eval("iwpriv", use, "mode", "3");
				eval("iwpriv", use, "pureg", "1");
			}
			if (!strcmp(netmode, "ng-only")) {
				eval("iwpriv", use, "mode", "7");
			}
			if (!strcmp(netmode, "na-only")) {
				eval("iwpriv", use, "mode", "6");
			}
			if (!strcmp(netmode, "bg-mixed")) {
				eval("iwpriv", use, "mode", "3");
			}

			if (!strcmp(netmode, "a-only"))
				eval("iwpriv", use, "mode", "1");
		}
	}
	if (nvram_default_matchi(bw, 40, 20)) {
		{
			if (!strcmp(netmode, "g-only")) {
				eval("iwpriv", use, "mode", "6");
			}
			if (!strcmp(netmode, "a-only")) {
				eval("iwpriv", use, "mode", "5");
			}
		}
	} else {
		if (nvram_matchi(xr, 1)) {
			eval("iwpriv", use, "xr", "1");
		} else {
			eval("iwpriv", use, "xr", "0");
		}
	}
	//    if( nvram_default_matchi( comp, 1, 0 ) )
	//      sysprintf("iwpriv %s compression 1",use);
	//    else
	//      sysprintf("iwpriv %s compression 0",use);

	if (nvram_default_matchi(ff, 1, 0))
		eval("iwpriv", use, "ff", "1");
	else
		eval("iwpriv", use, "ff", "0");
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
	if (nvram_nmatch("None", "%s_protmode", use))
		eval("iwpriv", use, "protmode", "0");
	if (nvram_nmatch("CTS", "%s_protmode", use))
		eval("iwpriv", use, "protmode", "1");
	if (nvram_nmatch("RTS/CTS", "%s_protmode", use))
		eval("iwpriv", use, "protmode", "2");
}

/*static void set_compression( int count )
{
    char comp[32];
    char wif[32];

    sprintf( wif, "wifi%d", count );
    sprintf( comp, "wlan%d_compression", count );
    if( nvram_default_matchi( comp, 1, 0 ) )
	setsysctrl( wif, "compression", 1 );
    else
	setsysctrl( wif, "compression", 0 );
}
*/
static void setMacFilter(char *iface)
{
	char *next;
	char var[32];

	eval("iwpriv", iface, "maccmd", "3");

	char nvvar[32];

	sprintf(nvvar, "%s_macmode", iface);
	if (nvram_match(nvvar, "deny")) {
		eval("iwpriv", iface, "maccmd", "2");
		char nvlist[32];

		sprintf(nvlist, "%s_maclist", iface);

		foreach(var, nvram_safe_get(nvlist), next)
		{
			eval("iwpriv", iface, "addmac", var);
		}
	} else if (nvram_match(nvvar, "allow")) {
		eval("iwpriv", iface, "maccmd", "1");

		char nvlist[32];

		sprintf(nvlist, "%s_maclist", iface);

		foreach(var, nvram_safe_get(nvlist), next)
		{
			eval("iwpriv", iface, "addmac", var);
		}
	}
}
#endif
#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)
int iscpe(void);

static void configure_single(int count)
{
	char *next;
	char var[80];
	char mode[80];
	int cnt = 0;
	char dev[10];
	char wif[10];
	char wl[16];
	char channel[16];
	char ssid[16];
	char net[16];
	char wifivifs[16];
	char broadcast[16];
	char power[32];
	char sens[32];
	char basedev[16];
	char diversity[32];
	char rxantenna[32];
	char txantenna[32];
	char athmac[19];
	char maxassoc[32];
	char wl_poll[32];
	static int vapcount = 0;
	char inact[36];
	char inact_tick[40];
	char tmp[256];
	if (count == 0)
		vapcount = 0;

	sprintf(wif, "wifi%d", count);
	sprintf(dev, "wlan%d", count);
	if (!strncmp(dev, "wlan0", 4))
		led_control(LED_SEC0, LED_OFF);
	if (!strncmp(dev, "wlan1", 4))
		led_control(LED_SEC1, LED_OFF);
	if (is_mac80211(dev)) {
		configure_single_ath9k(count);
		ath9k_start_supplicant(count, dev);
		sysprintf("touch /tmp/wlan%d_configured", count);
		return;
	}
#ifdef HAVE_MADWIFI

	sprintf(wifivifs, "wlan%d_vifs", count);
	sprintf(wl, "wlan%d_mode", count);
#ifdef HAVE_REGISTER
	int cpeonly = iscpe();
#else
	int cpeonly = 0;
#endif
	if (cpeonly && nvram_match(wl, "ap")) {
		nvram_set(wl, "sta");
	}
	sprintf(channel, "wlan%d_channel", count);
	sprintf(power, "wlan%d_txpwrdbm", count);
	sprintf(sens, "wlan%d_distance", count);
	sprintf(diversity, "wlan%d_diversity", count);
	sprintf(txantenna, "wlan%d_txantenna", count);
	sprintf(rxantenna, "wlan%d_rxantenna", count);
	sprintf(athmac, "wlan%d_hwaddr", count);

	// create base device
	cprintf("configure base interface %d\n", count);
	sprintf(net, "%s_net_mode", dev);
	if (nvram_match(net, "disabled")) {
		sysprintf("touch /tmp/wlan%d_configured", count);
		return;
	}
	//    set_compression( count );
	// create wds interface(s)
	int s;

	char *apm;
	int vif = 0;
	sprintf(wl_poll, "%s_pollingmode", dev);

	setsysctrl(wif, "pollingmode", nvram_default_geti(wl_poll, 0));

	char *vifs = nvram_safe_get(wifivifs);
	int countvaps = 1;
	foreach(var, vifs, next)
	{
		countvaps++;
	}
	if (countvaps < 4)
		countvaps = 4;
	if (countvaps > vapcount)
		vapcount = countvaps;

	setsysctrl(wif, "maxvaps", vapcount);
	char primary[32] = { 0 };
	// create original primary interface
	apm = nvram_default_get(wl, "ap");
	if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap")) {
		eval("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "ap");
		strcpy(primary, dev);
	}

	if (vifs != NULL)
		foreach(var, vifs, next)
		{
			sprintf(mode, "%s_mode", var);
			char *vapm = nvram_default_get(mode, "ap");
			// create device
			if (*mode) {
				if (!strcmp(vapm, "wet") || !strcmp(vapm, "sta") || !strcmp(vapm, "wdssta") ||
				    !strcmp(vapm, "wdssta_mtik"))
					eval("wlanconfig", var, "create", "wlandev", wif, "wlanmode", "sta", "nosbeacon");
				else if (!strcmp(vapm, "ap") || !strcmp(vapm, "wdsap"))
					eval("wlanconfig", var, "create", "wlandev", wif, "wlanmode", "ap");
				else
					eval("wlanconfig", var, "create", "wlandev", wif, "wlanmode", "adhoc", "nosbeacon");
				vif = 1;
				if (!*primary)
					strcpy(primary, var);
				char vathmac[16];

				sprintf(vathmac, "%s_hwaddr", var);
				char vmacaddr[32];

				getMacAddr(var, vmacaddr, sizeof(vmacaddr));
				nvram_set(vathmac, vmacaddr);
			}
		}

	if (strcmp(apm, "ap") && strcmp(apm, "wdsap")) {
		if (!strcmp(apm, "wet") || !strcmp(apm, "wdssta") || !strcmp(apm, "wdssta_mtik") || !strcmp(apm, "sta")) {
			if (vif)
				eval("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "sta", "nosbeacon");
			else
				eval("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "sta");

		} else if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap"))
			eval("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "ap");
		else
			eval("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "adhoc", "nosbeacon");

		if (!*primary)
			strcpy(primary, dev);
	}
#if 0
#endif
	cprintf("detect maxpower\n");
	apm = nvram_default_get(wl, "ap");
	char maxp[16];

	vifs = nvram_safe_get(wifivifs);
	// fprintf(stderr,"vifs %s\n",vifs);
	char *useif = NULL;
	char copyvap[64];

	if (vifs != NULL)
		foreach(var, vifs, next)
		{
			// fprintf(stderr,"vifs %s, %s\n",vifs, var);
			if (!useif) {
				strcpy(copyvap, var);
				useif = copyvap;
			}
		}
	// config net mode
	if (useif)
		set_netmode(wif, dev, useif);
	set_netmode(wif, dev, dev);

	char bcn[32];
	sprintf(bcn, "%s_bcn", dev);
	eval("iwpriv", dev, "bintval", nvram_default_get(bcn, "100"));

	char dtim[32];
	sprintf(dtim, "%s_dtim", dev);
	eval("iwpriv", dev, "dtim_period", nvram_default_get(dtim, "2"));

	char wmm[32];

	sprintf(wmm, "%s_wmm", dev);
	if (nvram_nmatch("1", "%s_pollingmode", dev))
		eval("iwpriv", dev, "wmm", "1");
	else
		eval("iwpriv", dev, "wmm", nvram_default_get(wmm, "0"));
	char doth[32];

	sprintf(doth, "%s_doth", dev);
#ifdef HAVE_BUFFALO
	eval("iwpriv", dev, "doth", nvram_default_get(doth, "1"));
#else
	eval("iwpriv", dev, "doth", nvram_default_get(doth, "0"));
#endif
	int disablescan = 0;

	set_scanlist(dev, wif);
	if (useif)
		set_netmode(wif, dev, useif);
	set_netmode(wif, dev, dev);
	setRTS(dev);

	char macaddr[32];

	getMacAddr(dev, macaddr, sizeof(macaddr));
	nvram_set(athmac, macaddr);

	cprintf("adjust sensitivity\n");

	int distance = nvram_default_geti(sens, 500); // to meter
	if (nvram_nmatch("1", "%s_pollingmode", var)) {
		setdistance(wif, 100000, 20);
	} else {
		if (distance > 0) {
			setsysctrl(wif, "dynack_count", 0);
			char *chanbw = nvram_nget("%s_channelbw", dev);

			setdistance(wif, distance,
				    atoi(chanbw)); // sets the receiver
			// sensitivity
		} else {
			setsysctrl(wif, "distance", 100000);
			setsysctrl(wif, "dynack_count", 20);
		}
	}
	char wl_intmit[32];
	char wl_noise_immunity[32];
	char wl_ofdm_weak_det[32];
	char wl_csma[32];

	sprintf(wl_intmit, "%s_intmit", dev);
	sprintf(wl_noise_immunity, "%s_noise_immunity", dev);
	sprintf(wl_ofdm_weak_det, "%s_ofdm_weak_det", dev);
	sprintf(wl_csma, "%s_csma", dev);

	setsysctrl(wif, "csma", nvram_default_geti(wl_csma, 1));
	setsysctrl(wif, "intmit", nvram_default_geti(wl_intmit, -1));
	int level = nvram_default_geti(wl_noise_immunity, 4);
	if (level < 0)
		level = 4;
	setsysctrl(wif, "noise_immunity", level);
	setsysctrl(wif, "ofdm_weak_det", nvram_default_geti(wl_ofdm_weak_det, 1));

	if (isEMP(dev)) //check this only if the current installed card is usually a emp card. this is made to prevent card destruction
	{
		if (nvram_nmatch("1", "%s_cardtype", dev)) {
			setsysctrl(wif, "powerfix",
				   7); //increase outputpower by 7 dbm, we will do this in future for a and b band separate
		}
	}

	int enable = 1;
	int disable = 0;

#ifdef HAVE_NS5
	int gpio = 1;
#endif
#ifdef HAVE_NS3
	int gpio = 1;
#endif
#ifdef HAVE_LC5
	int gpio = 1;
#endif
#ifdef HAVE_NS2
	int gpio = 7;
#endif
#ifdef HAVE_LC2
	enable = 0; // swap it
	disable = 1;
	int gpio = 2;
#endif

#if defined(HAVE_NS2) || defined(HAVE_NS5) || defined(HAVE_LC2) || defined(HAVE_LC5) || defined(HAVE_NS3)
	int tx = nvram_default_geti(txantenna, 0);
	setsysctrl(wif, "diversity", 0);
	switch (tx) {
	case 0: // vertical
		setsysctrl(wif, "rxantenna", 2);
		setsysctrl(wif, "txantenna", 2);
		set_gpio(gpio, enable);
		break;
	case 1: // horizontal
		setsysctrl(wif, "rxantenna", 1);
		setsysctrl(wif, "txantenna", 1);
		set_gpio(gpio, enable);
		break;
	case 2: // external
		setsysctrl(wif, "rxantenna", 1);
		setsysctrl(wif, "txantenna", 1);
		set_gpio(gpio, disable);
		break;
	case 3: // adaptive
		setsysctrl(wif, "diversity", 1);
		setsysctrl(wif, "rxantenna", 0);
		setsysctrl(wif, "txantenna", 0);
		set_gpio(gpio, enable);
		break;
	}
#else

#if defined(HAVE_PICO2) || defined(HAVE_PICO2HP) || defined(HAVE_PICO5)
	int rx = 1;
	int tx = 1;
	int diva = 0; //atoi( nvram_default_get( diversity, "0" ) );
//#elif defined(HAVE_EOC5610)
//      int rx = nvram_default_geti(txantenna, "1"));
//      int tx = nvram_default_geti(txantenna, "1"));
//      int diva = 0;           //atoi( nvram_default_get( diversity, "0" ) );
//      int rx = 1;
//      int tx = 0;             // fix to internal path, since both antennas use the same connector. so only the switch matters
//      int diva = 1;           //1;// atoi( nvram_default_get( diversity, "0" ) );
//#elif defined(HAVE_EOC1650)
//      int rx = 2;             //atoi( nvram_default_get( txantenna, "2" ) ); // secondary antenna output is the internal antenna and should be used as default value
//      int tx = 2;             //atoi( nvram_default_get( txantenna, "2" ) );
//      int rx = 1;
//      int tx = 0;             // fix to internal path, since both antennas use the same connector. so only the switch matters
//      int diva = 1;           //1;// atoi( nvram_default_get( diversity, "0" ) );
#else
	int rx = nvram_default_geti(rxantenna, 1);
	int tx = nvram_default_geti(txantenna, 1);
	int diva = nvram_default_geti(diversity, 0);
#endif
	setsysctrl(wif, "diversity", diva);
	setsysctrl(wif, "rxantenna", rx);
	setsysctrl(wif, "txantenna", tx);
#endif
	// setup vif interfaces first
	char chanshift_s[32];

	sprintf(chanshift_s, "%s_chanshift", dev);
	char *chanshift = nvram_default_get(chanshift_s, "0");

	sprintf(maxassoc, "%s_maxassoc", dev);
	eval("iwpriv", dev, "maxassoc", nvram_default_get(maxassoc, "256"));

	switch (atoi(chanshift)) {
	case 15:
		eval("iwpriv", dev, "channelshift", "-3");
		break;
	case 10:
		eval("iwpriv", dev, "channelshift", "-2");
		break;
	case 5:
		eval("iwpriv", dev, "channelshift", "-1");
		break;
	case 0:
		eval("iwpriv", dev, "channelshift", "0");
		break;
	case -5:
		eval("iwpriv", dev, "channelshift", "1");
		break;
	case -10:
		eval("iwpriv", dev, "channelshift", "2");
		break;
	case -15:
		eval("iwpriv", dev, "channelshift", "3");
		break;
	default:
		eval("iwpriv", dev, "channelshift", "0");
		break;
	}
	if (!strcmp(apm, "wdssta") || !strcmp(apm, "wdsap") || !strcmp(apm, "wdssta_mtik"))
		eval("iwpriv", dev, "wds", "1");

	if (!strcmp(apm, "wdsap"))
		eval("iwpriv", dev, "wdssep", "1");
	else
		eval("iwpriv", dev, "wdssep", "0");

	vifs = nvram_safe_get(wifivifs);
	if (vifs != NULL)
		foreach(var, vifs, next)
		{
			sprintf(net, "%s_net_mode", var);
			if (nvram_match(net, "disabled"))
				continue;
			sprintf(ssid, "%s_ssid", var);
			sprintf(mode, "%s_mode", var);
			sprintf(maxassoc, "%s_maxassoc", var);
			eval("iwpriv", var, "maxassoc", nvram_default_get(maxassoc, "256"));
			switch (atoi(chanshift)) {
			case 15:
				eval("iwpriv", var, "channelshift", "-3");
				break;
			case 10:
				eval("iwpriv", var, "channelshift", "-2");
				break;
			case 5:
				eval("iwpriv", var, "channelshift", "-1");
				break;
			case 0:
				eval("iwpriv", var, "channelshift", "0");
				break;
			case -5:
				eval("iwpriv", var, "channelshift", "1");
				break;
			case -10:
				eval("iwpriv", var, "channelshift", "2");
				break;
			case -15:
				eval("iwpriv", var, "channelshift", "3");
				break;
			default:
				eval("iwpriv", var, "channelshift", "0");
				break;
			}
			char *mvap = nvram_default_get(mode, "ap");
			set_scanlist(dev, wif);
			setRTS(var);
			eval("iwpriv", var, "bgscan", "0");
			if (strcmp(mvap, "sta") && strcmp(mvap, "wdssta") && strcmp(mvap, "wdssta_mtik") && strcmp(mvap, "wet")) {
				cprintf("set channel\n");
				char *ch = nvram_default_get(channel, "0");

				if (strcmp(ch, "0") == 0) {
					eval("iwconfig", var, "channel", "0");
				} else {
					char s_ch[32];
					sprintf(s_ch, "%sM", ch);
					eval("iwconfig", var, "freq", s_ch);
				}
			}
#ifdef HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "default_vap"));
#else
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "maksat_vap"));
#endif
#elif defined(HAVE_SANSFIL)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "SANSFIL_vap"));
#elif defined(HAVE_TRIMAX)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "m2m_vap"));
#elif defined(HAVE_WIKINGS)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "Excel Networks_vap"));
#elif defined(HAVE_ESPOD)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "ESPOD Technologies_vap"));
#elif defined(HAVE_NEXTMEDIA)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "nextmedia_vap"));
#elif defined(HAVE_TMK)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "KMT_vap"));
#elif defined(HAVE_BKM)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "BKM_vap"));
#elif defined(HAVE_ERC)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "ERC_vap"));
#elif defined(HAVE_CORENET)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "corenet.ap"));
#elif defined(HAVE_ONNET_BLANK)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "Enterprise WIFI_vap"));
#elif defined(HAVE_ONNET)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "OTAi_vap"));
#elif defined(HAVE_KORENRON)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "WBR2000_vap"));
#elif defined(HAVE_HOBBIT)
			eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "HQ-NDS-AIR"));
#else
#ifdef HAVE_REGISTER
			if (!isregistered())
				eval("iwconfig", var, "essid", "--", "need_activation");
			else
#endif
				eval("iwconfig", var, "essid", "--", nvram_default_get(ssid, "dd-wrt_vap"));
#endif
			cprintf("set broadcast flag vif %s\n",
				var); // hide ssid
			sprintf(broadcast, "%s_closed", var);
			eval("iwpriv", var, "hide_ssid", nvram_default_get(broadcast, "0"));
			sprintf(wmm, "%s_wmm", var);
			if (nvram_nmatch("1", "%s_pollingmode", var))
				eval("iwpriv", var, "wmm", "1");
			else
				eval("iwpriv", var, "wmm", nvram_default_get(wmm, "0"));
			char isolate[32];

			sprintf(isolate, "%s_ap_isolate", var);
			if (nvram_default_matchi(isolate, 1, 0))
				eval("iwpriv", var, "ap_bridge", "0");
			if (!strcmp(mvap, "wdssta") || !strcmp(mvap, "wdsap") || !strcmp(mvap, "wdssta_mtik"))
				eval("iwpriv", var, "wds", "1");
			eval("iwpriv", var, "addmtikie", "1");

#ifdef HAVE_BONDING
			int isBond(char *ifname);

			if (!strcmp(mvap, "wdsap") && !isBond(var))
#else
			if (!strcmp(mvap, "wdsap"))
#endif
				eval("iwpriv", var, "wdssep", "1");
			else
				eval("iwpriv", var, "wdssep", "0");

			// removed hostroaming 0 due to excessive tests and driver research
			// sysprintf("iwpriv %s hostroaming 0", var);
			cnt++;
		}

	eval("iwpriv", dev, "addmtikie", "1");

	char isolate[32];

	sprintf(isolate, "%s_ap_isolate", dev);
	if (nvram_default_matchi(isolate, 1, 0))
		eval("iwpriv", dev, "ap_bridge", "0");

	sprintf(ssid, "wlan%d_ssid", count);
	sprintf(broadcast, "wlan%d_closed", count);
	if (!strcmp(apm, "infra")) {
		char *cellid;
		char cellidtemp[32];
		sprintf(cellidtemp, "wlan%d_cellid", count);
		cellid = nvram_safe_get(cellidtemp);
		if (*cellid) {
			eval("iwconfig", dev, "ap", cellid);
		}
#if defined(HAVE_MAKSAT) || defined(HAVE_TMK) || defined(HAVE_BKM)
		else {
			char cellidtemp[5];
			bzero(cellidtemp, 5);
			strncpy(cellidtemp, nvram_safe_get(ssid), 5);
			sysprintf("iwconfig %s ap 02:%02x:%02x:%02x:%02x:%02x", dev, cellidtemp[0], cellidtemp[1], cellidtemp[2],
				  cellidtemp[3], cellidtemp[4]);
		}
#endif
	}

	bzero(var, 80);

	cprintf("set ssid\n");
#ifdef HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "default"));
#else
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "maksat"));
#endif
#elif defined(HAVE_TRIMAX)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "M2M"));
#elif defined(HAVE_WIKINGS)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "Excel Networks"));
#elif defined(HAVE_ESPOD)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "ESPOD Technologies"));
#elif defined(HAVE_NEXTMEDIA)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "nextmedia"));
#elif defined(HAVE_TMK)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "KMT"));
#elif defined(HAVE_BKM)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "BKM"));
#elif defined(HAVE_ERC)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "ERC"));
#elif defined(HAVE_CORENET)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "corenet.ap"));
#elif defined(HAVE_KORENRON)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "WBR2000"));
#elif defined(HAVE_HOBBIT)
	eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "HQ-NDS-AIR"));
#else
#ifdef HAVE_REGISTER
	if (!isregistered())
		eval("iwconfig", dev, "essid", "--", "need_activation");
	else
#endif
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "dd-wrt"));
#endif
	cprintf("set broadcast flag\n"); // hide ssid
	eval("iwpriv", dev, "hide_ssid", nvram_default_get(broadcast, "0"));
	eval("iwpriv", dev, "bgscan", "0");
	apm = nvram_default_get(wl, "ap");

	char preamble[32];

	sprintf(preamble, "%s_preamble", dev);
	if (nvram_default_matchi(preamble, 1, 1)) {
		eval("iwpriv", dev, "shpreamble", "1");
	} else
		eval("iwpriv", dev, "shpreamble", "0");

	if (strcmp(apm, "sta") == 0 || strcmp(apm, "infra") == 0 || strcmp(apm, "wet") == 0 || strcmp(apm, "wdssta") == 0 ||
	    strcmp(apm, "wdssta_mtik") == 0) {
		cprintf("set ssid\n");
#ifdef HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "default"));
#else
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "maksat"));
#endif
#elif defined(HAVE_TRIMAX)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "M2M"));
#elif defined(HAVE_WIKINGS)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "Excel Networks"));
#elif defined(HAVE_ESPOD)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "ESPOD Technologies"));
#elif defined(HAVE_NEXTMEDIA)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "nextmedia"));
#elif defined(HAVE_TMK)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "KMT"));
#elif defined(HAVE_BKM)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "BKM"));
#elif defined(HAVE_ERC)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "ERC"));
#elif defined(HAVE_CORENET)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "corenet.ap"));
#elif defined(HAVE_KORENRON)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "WBR2000"));
#elif defined(HAVE_HOBBIT)
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "HQ-NDS-AIR"));
#else
		eval("iwconfig", dev, "essid", "--", nvram_default_get(ssid, "dd-wrt"));
#endif
	}

	cprintf("adjust power\n");

	int newpower = nvram_default_geti(power, 16);
	char s_dbm[32];
	sprintf(s_dbm, "%ddBm", newpower);
	eval("iwconfig", dev, "txpower", s_dbm);

	cprintf("done()\n");

	cprintf("setup encryption");
	// @todo ifup
	// netconfig

	/*
	 * set_rate (dev);
	 */
	set_rate(dev, dev);

	set_netmode(wif, dev, dev);

	setMacFilter(dev);
	//      setupKey(dev);
	if (vifs != NULL && *vifs) {
		foreach(var, vifs, next)
		{
			setMacFilter(var);
			//                      setupKey(var);
		}
	}

	apm = nvram_default_get(wl, "ap");
	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wdssta_mtik") && strcmp(apm, "wet")) {
		cprintf("set channel\n");
		char *ch = nvram_default_get(channel, "0");

		if (strcmp(ch, "0") == 0) {
			eval("iwconfig", dev, "channel", "0");
		} else {
			char s_ch[32];
			sprintf(s_ch, "%sM", ch);
			eval("iwconfig", dev, "freq", s_ch);
		}
	}
	// set inact inact tick (order is important!)
	sprintf(inact_tick, "%s_inact_tick", dev);
	sprintf(inact, "%s_inact", dev);
#ifdef HAVE_MAKSAT
	eval("iwpriv", dev, "inact_tick", nvram_default_get(inact_tick, "1"));
	eval("iwpriv", dev, "inact", nvram_default_get(inact, "15"));
#else
	eval("iwpriv", dev, "inact_tick", nvram_default_get(inact_tick, "15"));
	eval("iwpriv", dev, "inact", nvram_default_get(inact, "300"));
#endif

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

	// vif netconfig
	vifs = nvram_safe_get(wifivifs);
	if (vifs != NULL && *vifs) {
		foreach(var, vifs, next)
		{
			sprintf(mode, "%s_mode", var);
			char *m2 = nvram_default_get(mode, "ap");

			sprintf(inact_tick, "%s_inact_tick", var);
			sprintf(inact, "%s_inact", var);

#ifdef HAVE_MAKSAT
			eval("iwpriv", var, "inact_tick", nvram_default_get(inact_tick, "1"));
			eval("iwpriv", var, "inact", nvram_default_get(inact, "15"));
#else
			eval("iwpriv", var, "inact_tick", nvram_default_get(inact_tick, "15"));
			eval("iwpriv", var, "inact", nvram_default_get(inact, "300"));
#endif
			if (strcmp(m2, "sta")) {
				char bridged[32];

				sprintf(bridged, "%s_bridged", var);
				if (nvram_default_matchi(bridged, 1, 1)) {
					eval("ifconfig", var, "0.0.0.0", "up");
					br_add_interface(getBridge(var, tmp), var);
				} else {
					char ip[32];
					char mask[32];

					sprintf(ip, "%s_ipaddr", var);
					sprintf(mask, "%s_netmask", var);
					eval("ifconfig", var, "mtu", getMTU(var));
					eval("ifconfig", var, "txqueuelen", getTXQ(var));
					eval("ifconfig", var, nvram_safe_get(ip), "netmask", nvram_safe_get(mask), "up");
				}
			}
		}
	}
	// setup encryption
	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wdssta_mtik") && strcmp(apm, "wet"))
		setupHostAP(dev, "madwifi", 0);
	else
		setupSupplicant(dev, NULL);

	vifs = nvram_safe_get(wifivifs);
	if (vifs != NULL)
		foreach(var, vifs, next)
		{
			sprintf(mode, "%s_mode", var);
			char *vapm = nvram_default_get(mode, "ap");
			if (strcmp(vapm, "sta") && strcmp(vapm, "wdssta") && strcmp(vapm, "wdssta_mtik") && strcmp(vapm, "wet"))
				setupHostAP(var, "madwifi", 0);
			else
				setupSupplicant(var, NULL);
		}

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
			eval("iwpriv", primary, "wds_add", hwaddr);
			set_rate(dev, primary);
		}
	}

	for (s = 1; s <= 10; s++) {
		char wdsvarname[32] = { 0 };
		char wdsdevname[32] = { 0 };
		char wdsmacname[32] = { 0 };
		char *wdsdev;
		char *hwaddr;

		sprintf(wdsvarname, "%s_wds%d_enable", dev, (11 - s));
		sprintf(wdsdevname, "%s_wds%d_if", dev, (11 - s));
		sprintf(wdsmacname, "%s_wds%d_hwaddr", dev, (11 - s));
		wdsdev = nvram_safe_get(wdsdevname);
		if (!*wdsdev)
			continue;
		if (nvram_matchi(wdsvarname, 0))
			continue;
		hwaddr = nvram_safe_get(wdsmacname);
		if (*hwaddr) {
			eval("ifconfig", wdsdev, "0.0.0.0", "up");
		}
	}
	// adhoc interface is stuck sometimes.. don't know why yet, this helps
	if (!strcmp(apm, "infra")) {
		eval("ifconfig", dev, "0.0.0.0", "down");
		sleep(1);
		eval("ifconfig", dev, "0.0.0.0", "up");
	}
#endif
	sysprintf("touch /tmp/wlan%d_configured", count);
}

void start_vifs(void)
{
	char *next;
	char var[80];
	char *vifs;
	char mode[32];
	char *m;
	char wifivifs[32];
	int c = getdevicecount();
	int count = 0;
	char tmp[256];

	for (count = 0; count < c; count++) {
		sprintf(wifivifs, "wlan%d_vifs", count);
		vifs = nvram_safe_get(wifivifs);
		if (vifs != NULL && *vifs) {
			foreach(var, vifs, next)
			{
				setMacFilter(var);

				sprintf(mode, "%s_mode", var);
				m = nvram_default_get(mode, "ap");

				if (strcmp(m, "sta")) {
					char bridged[32];

					sprintf(bridged, "%s_bridged", var);
					if (nvram_default_matchi(bridged, 1, 1)) {
						eval("ifconfig", var, "0.0.0.0", "up");
						br_add_interface(getBridge(var, tmp), var);
					} else {
						char ip[32];
						char mask[32];
						sprintf(ip, "%s_ipaddr", var);
						sprintf(mask, "%s_netmask", var);
						eval("ifconfig", var, "mtu", getMTU(var));
						eval("ifconfig", var, "txqueuelen", getTXQ(var));
						eval("ifconfig", var, nvram_safe_get(ip), "netmask", nvram_safe_get(mask), "up");
					}
				}
			}
		}
	}
}

void stop_vifs(void)
{
	char *next;
	char var[80];
	char *vifs;
	char mode[32];
	char *m;
	char wifivifs[32];
	int c = getdevicecount();
	int count = 0;

	for (count = 0; count < c; count++) {
		sprintf(wifivifs, "wlan%d_vifs", count);
		vifs = nvram_safe_get(wifivifs);
		if (vifs != NULL && *vifs) {
			foreach(var, vifs, next)
			{
				eval("ifconfig", var, "down");
			}
		}
	}
}

void start_duallink(void)
{
	if (nvram_match("duallink", "master")) {
		sysprintf("ip route flush table 100");
		sysprintf("ip route flush table 200");
		sysprintf("ip route del fwmark 1 table 200");
		sysprintf("iptables -t mangle -F PREROUTING");
		sysprintf("ip route add %s/%s dev wlan0 src %s table 100", nvram_safe_get("wlan0_ipaddr"),
			  nvram_safe_get("wlan0_netmask"), nvram_safe_get("wlan0_ipaddr"));
		sysprintf("ip route default via %s table 100", nvram_safe_get("wlan0_duallink_parent"));
		sysprintf("ip route add %s/%s dev wlan0 src %s table 200", nvram_safe_get("wlan1_ipaddr"),
			  nvram_safe_get("wlan1_netmask"), nvram_safe_get("wlan1_ipaddr"));
		sysprintf("ip route default via %s table 200", nvram_safe_get("wlan1_duallink_parent"));
		sysprintf("iptables -t mangle -A PREROUTING -i br0 -j MARK --set-mark 1");
		sysprintf("ip rule add fwmark 1 table 200");
	}
	if (nvram_match("duallink", "slave")) {
		sysprintf("ip route flush table 100");
		sysprintf("ip route flush table 200");
		sysprintf("ip route del fwmark 1 table 100");
		sysprintf("iptables -t mangle -F PREROUTING");
		sysprintf("ip route add %s/%s dev wlan0 src %s table 100", nvram_safe_get("wlan0_ipaddr"),
			  nvram_safe_get("wlan0_netmask"), nvram_safe_get("wlan0_ipaddr"));
		sysprintf("ip route default via %s table 100", nvram_safe_get("wlan0_duallink_parent"));
		sysprintf("ip route add %s/%s dev wlan0 src %s table 200", nvram_safe_get("wlan1_ipaddr"),
			  nvram_safe_get("wlan1_netmask"), nvram_safe_get("wlan1_ipaddr"));
		sysprintf("ip route default via %s table 200", nvram_safe_get("wlan1_duallink_parent"));
		sysprintf("iptables -t mangle -A PREROUTING -i br0 -j MARK --set-mark 1");
		sysprintf("ip rule add fwmark 1 table 100");
	}
}

extern void adjust_regulatory(int count);

void configure_wifi(void) // madwifi implementation for atheros based
	// cards
{
	if (nvram_match("sfe", "1")) {
		sysprintf("echo 0 > /proc/sys/dev/nss/general/redirect");
	} else if (nvram_match("sfe", "2")) {
		sysprintf("echo 0 > /proc/sys/dev/nss/general/redirect");
	} else if (nvram_match("sfe", "3")) { // ecm nss
		sysprintf("echo 1 > /proc/sys/dev/nss/general/redirect");
	} else if (nvram_match("sfe", "4")) { // ecm sfe
		sysprintf("echo 0 > /proc/sys/dev/nss/general/redirect");
	} else if (nvram_match("sfe", "5")) { // ecm sfe & nss
		sysprintf("echo 1 > /proc/sys/dev/nss/general/redirect");
	} else {
		sysprintf("echo 0 > /proc/sys/dev/nss/general/redirect");
	}

	invalidate_channelcache();
#ifdef HAVE_NLD
	eval("/usr/sbin/nldstop.sh");
#endif
	deconfigure_wifi();
	int c = getdevicecount();
	int i;
	char tmp[256];
	char fwtype_use[32];
	char changestring[128] = { 0 };
	char cmpstring[128] = { 0 };
	int changed = 0;
#ifdef HAVE_ATH9K
	char dev[32];
	int hasath9k = 0;
	for (i = 0; i < c; i++) {
		sprintf(dev, "wlan%d", i);
		if (is_mac80211(dev)) {
			hasath9k = 1;
			break;
		}
	}
	if (hasath9k) {
		char regdomain[16];
		char *country;
		sprintf(regdomain, "wlan0_regdomain");
		country = nvram_default_get(regdomain, "UNITED_STATES");
		eval("iw", "reg", "set", "00");
		char *iso = getIsoName(country);
		if (!iso)
			iso = "DE";
		eval("iw", "reg", "set", iso);
#if defined(HAVE_ONNET) && defined(HAVE_ATH10K_CT)
		if (nvram_geti("ath10k-ct") != nvram_geti("wlan10k-ct_bak")) {
			fprintf(stderr, "Switching ATH10K driver, rebooting now...\n");
			eval("reboot");
		}
#endif
	}
#endif
	for (i = 0; i < c; i++) {
		adjust_regulatory(i);
	}
	for (i = 0; i < c; i++) {
		sysprintf("rm -f /tmp/wlan%d_configured", (c - 1) - i);
		sprintf(fwtype_use, "wlan%d_fwtype_use", (c - 1) - i);
		strcat(changestring, nvram_safe_get(fwtype_use));
		configure_single((c - 1) - i);
		strcat(cmpstring, nvram_safe_get(fwtype_use));
	}
#ifdef HAVE_ATH9K
	if (hasath9k) {
		char regdomain[16];
		char *country;
		sprintf(regdomain, "wlan0_regdomain");
		country = nvram_default_get(regdomain, "UNITED_STATES");
		eval("iw", "reg", "set", "00");
		char *iso = getIsoName(country);
		if (!iso)
			iso = "DE";
		eval("iw", "reg", "set", iso);
#if defined(HAVE_ONNET) && defined(HAVE_ATH10K_CT)
		if (nvram_geti("ath10k-ct") != nvram_geti("wlan10k-ct_bak")) {
			fprintf(stderr, "Switching ATH10K driver, rebooting now...\n");
			eval("reboot");
		}
#endif
	}
#endif
	for (i = 0; i < c; i++) {
		adjust_regulatory(i);
	}
#ifdef HAVE_ATH9K
	for (i = 0; i < c; i++) {
		/* reset tx power */
		char power[32];
		sprintf(power, "wlan%d_txpwrdbm", i);
		char pw[32];
		char phy[32];
		sprintf(phy, "phy%d", i);
		sprintf(pw, "%d", nvram_default_geti(power, 16) * 100);
		eval("iw", "phy", phy, "set", "txpower", "fixed", pw);
	}
#endif
#ifdef HAVE_ATH10K
	//      fprintf(stderr, "first attempt \"%s\", second attempt \"%s\"\n", changestring, cmpstring);
	if (strcmp(changestring, cmpstring)) {
		/* we only need todo this if firmware has changed */
		/* this sucks, we take it as workaround */
		deconfigure_wifi();
		for (i = 0; i < c; i++) {
			sysprintf("rm -f /tmp/wlan%d_configured", (c - 1) - i);
			configure_single((c - 1) - i);
		}
#ifdef HAVE_ATH9K
		if (hasath9k) {
			char regdomain[16];
			char *country;
			sprintf(regdomain, "wlan0_regdomain");
			country = nvram_default_get(regdomain, "UNITED_STATES");
			eval("iw", "reg", "set", "00");
			char *iso = getIsoName(country);
			if (!iso)
				iso = "DE";
			eval("iw", "reg", "set", iso);
#if defined(HAVE_ONNET) && defined(HAVE_ATH10K_CT)
			if (nvram_geti("ath10k-ct") != nvram_geti("wlan10k-ct_bak")) {
				fprintf(stderr, "Switching ATH10K driver, rebooting now...\n");
				eval("reboot");
			}
#endif
		}
#endif
		for (i = 0; i < c; i++) {
			adjust_regulatory(i);
		}
#ifdef HAVE_ATH9K
		for (i = 0; i < c; i++) {
			/* reset tx power */
			char power[32];
			sprintf(power, "wlan%d_txpwrdbm", i);
			char pw[32];
			char phy[32];
			sprintf(phy, "phy%d", i);
			sprintf(pw, "%d", nvram_default_geti(power, 16) * 100);
			eval("iw", "phy", phy, "set", "txpower", "fixed", pw);
		}
#endif
	}
#endif
	invalidate_channelcache();
#if 0
	int dead = 10 * 60;	// after 30 seconds, we can assume that something is hanging
	while (dead--) {
		int cnf = 0;
		for (i = 0; i < c; i++) {
			char path[42];
			sprintf(path, "/tmp/wlan%d_configured", i);
			FILE *check = fopen(path, "rb");
			if (check) {
				cnf++;
				fclose(check);
			}
		}
//              fprintf(stderr, "waiting for %d interfaces, %d finished\n", c, cnf);
		if (cnf == c)
			break;
		usleep(100 * 1000);	// wait 100 ms
	}
#endif
#ifdef HAVE_NLD
#ifdef HAVE_REGISTER
	if (registered_has_cap(21))
#endif
	{
		eval("/usr/sbin/nldstart.sh");
		dd_loginfo("nld:", "startup\n");
	}
#endif
#if defined(HAVE_TMK) || defined(HAVE_BKM)
	eval("/sbin/r1x_startup.sh");
#endif
#ifdef HAVE_WPS
	nvram_unset("wps_forcerelease");
#endif
#ifdef HAVE_AOSS
	if (nvram_matchi("aoss_success", 1))
		led_control(LED_SES, LED_ON);
#endif

	if (need_commit) {
		nvram_async_commit();
		need_commit = 0;
	}
	eval("killall", "-9", "roaming_daemon");
	if (getSTA() || getWET()) {
#ifdef HAVE_ATH9K
		// disable for now, till fixed
		if (0)
#endif
			eval("roaming_daemon");
	}

	int cnt = getdevicecount();
	int s;

	for (c = 0; c < cnt; c++) {
		char br1enable[32];
		char br1ipaddr[32];
		char br1netmask[32];

		sprintf(br1enable, "wlan%d_br1_enable", c);
		sprintf(br1ipaddr, "wlan%d_br1_ipaddr", c);
		sprintf(br1netmask, "wlan%d_br1_netmask", c);
		if (!nvram_exists(br1enable))
			nvram_seti(br1enable, 0);
		if (!nvram_exists(br1ipaddr))
			nvram_set(br1ipaddr, "0.0.0.0");
		if (!nvram_exists(br1netmask))
			nvram_set(br1netmask, "255.255.255.0");
		if (nvram_matchi(br1enable, 1)) {
			ifconfig("br1", 0, 0, 0);

			// eval ("ifconfig", "br1", "down");
			br_del_bridge("br1");
			br_add_bridge("br1");
			char word[256];

			br_set_stp_state("br1", getBridgeSTP("br1", word));
			br_set_bridge_max_age("br1", getBridgeMaxAge("br1"));
			br_set_bridge_forward_delay("br1", getBridgeForwardDelay("br1"));

			/*
			 * Bring up and configure br1 interface 
			 */
			if (nvram_invmatch(br1ipaddr, "0.0.0.0")) {
				ifconfig("br1", IFUP, nvram_safe_get(br1ipaddr), nvram_safe_get(br1netmask));

				br_set_stp_state("br1", getBridgeSTP("br1", word));
			}
		}
	}
	for (c = 0; c < cnt; c++) {
		for (s = 1; s <= MAX_WDS_DEVS; s++) {
			char wdsvarname[32] = { 0 };
			char wdsdevname[32] = { 0 };
			char *dev;

			char br1enable[32];

			sprintf(wdsvarname, "wlan%d_wds%d_enable", c, s);
			sprintf(wdsdevname, "wlan%d_wds%d_if", c, s);
			sprintf(br1enable, "wlan%d_br1_enable", c);
			if (!nvram_exists(wdsvarname))
				nvram_seti(wdsvarname, 0);
			dev = nvram_safe_get(wdsdevname);
			if (!*dev)
				continue;
			ifconfig(dev, 0, 0, 0);

			// eval ("ifconfig", dev, "down");
			if (nvram_matchi(wdsvarname, 1)) {
				char *wdsip;
				char *wdsnm;
				char wdsbc[32] = { 0 };
				wdsip = nvram_nget("wlan%d_wds%d_ipaddr", c, s);
				wdsnm = nvram_nget("wlan%d_wds%d_netmask", c, s);

				snprintf(wdsbc, 31, "%s", wdsip);
				get_broadcast(wdsbc, sizeof(wdsbc), wdsnm);
				eval("ifconfig", dev, wdsip, "broadcast", wdsbc, "netmask", wdsnm, "up");
			} else if (nvram_matchi(wdsvarname, 2) && nvram_matchi(br1enable, 1)) {
				eval("ifconfig", dev, "up");
				br_add_interface("br1", dev);
			} else if (nvram_matchi(wdsvarname, 3)) {
				ifconfig(dev, IFUP, 0, 0);
				br_add_interface(getBridge(dev, tmp), dev);
			}
		}
	}

	char eabuf[32];
#ifdef HAVE_RB500
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_X86
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_LAGUNA
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_VENTANA
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_IPQ6018
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_NEWPORT
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_EROUTER
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_XSCALE
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_MAGICBOX
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_RB600
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_FONERA
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_LS2
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_SOLO51
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_LS5
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_WHRAG108
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_PB42
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_LSX
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_DANUBE
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_STORM
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_OPENRISC
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_ADM5120
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_TW6600
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
#ifdef HAVE_CA8
	nvram_set("wl0_hwaddr", get_hwaddr("wlan0", eabuf));
#endif
	reset_hwaddr(nvram_safe_get("lan_ifname"));
	eval("startservice", "resetleds", "-f");
	sysprintf("echo 1 > /sys/kernel/debug/ieee80211/phy0/ath11k/ext_rx_stats");
	sysprintf("echo 1 > /sys/kernel/debug/ieee80211/phy1/ath11k/ext_rx_stats");
	sysprintf("echo 1 > /sys/kernel/debug/ieee80211/phy2/ath11k/ext_rx_stats");
}

void start_deconfigurewifi(void)
{
	deconfigure_wifi();
}

void start_configurewifi(void)
{
	configure_wifi();
}
