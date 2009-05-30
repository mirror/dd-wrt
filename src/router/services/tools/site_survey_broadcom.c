/*
 * site_survey_broadcom.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <utils.h>
#include <bcmutils.h>

#define sys_restart() kill(1, SIGHUP)
#define SITE_SURVEY_DB	"/tmp/site_survey"
#define SITE_SURVEY_NUM	256

int write_site_survey(void);
static int open_site_survey(void);
int write_site_survey(void);

struct site_survey_list {
	uint8 SSID[33];
	unsigned char BSSID[18];
	uint8 channel;		/* Channel no. */
	int16 RSSI;		/* receive signal strength (in dBm) */
	int16 phy_noise;	/* noise (in dBm) */
	uint16 beacon_period;	/* units are Kusec */
	uint16 capability;	/* Capability information */
	// unsigned char athcaps;
	unsigned char ENCINFO[128];	/* encryption info */
	uint rate_count;	/* # rates in this set */
	uint8 dtim_period;	/* DTIM period */
} site_survey_lists[SITE_SURVEY_NUM];

/* 802.11i/WPA RSN IE parsing utilities */
typedef struct {
	uint16 version;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *akm;
	uint8 *capabilities;
} rsn_parse_info_t;

static int wlu_bcmp(const void *b1, const void *b2, int len)
{
	return (memcmp(b1, b2, len));
}

static INLINE uint16 ltoh16_ua(void *bytes)
{
	return (((uint8 *) bytes)[1] << 8) + ((uint8 *) bytes)[0];
}

static bool wlu_is_wpa_ie(uint8 ** wpaie, uint8 ** tlvs, uint * tlvs_len)
{
	uint8 *ie = *wpaie;

	/* If the contents match the WPA_OUI and type=1 */
	if ((ie[1] >= 6) && !wlu_bcmp(&ie[2], WPA_OUI "\x01", 4)) {
		return TRUE;
	}

	/* point to the next ie */
	ie += ie[1] + 2;
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)(ie - *tlvs);
	/* update the pointer to the start of the buffer */
	*tlvs = ie;

	return FALSE;
}

static int
wl_rsn_ie_parse_info(uint8 * rsn_buf, uint len, rsn_parse_info_t * rsn)
{
	uint16 count;

	memset(rsn, 0, sizeof(rsn_parse_info_t));

	/* version */
	if (len < sizeof(uint16))
		return 1;

	rsn->version = ltoh16_ua(rsn_buf);
	len -= sizeof(uint16);
	rsn_buf += sizeof(uint16);

	/* Multicast Suite */
	if (len < sizeof(wpa_suite_mcast_t))
		return 0;

	rsn->mcast = (wpa_suite_mcast_t *) rsn_buf;
	len -= sizeof(wpa_suite_mcast_t);
	rsn_buf += sizeof(wpa_suite_mcast_t);

	/* Unicast Suite */
	if (len < sizeof(uint16))
		return 0;

	count = ltoh16_ua(rsn_buf);

	if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
		return 1;

	rsn->ucast = (wpa_suite_ucast_t *) rsn_buf;
	len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
	rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

	/* AKM Suite */
	if (len < sizeof(uint16))
		return 0;

	count = ltoh16_ua(rsn_buf);

	if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
		return 1;

	rsn->akm = (wpa_suite_auth_key_mgmt_t *) rsn_buf;
	len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
	rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

	/* Capabilites */
	if (len < sizeof(uint16))
		return 0;

	rsn->capabilities = rsn_buf;

	return 0;
}

static void wl_rsn_ie_dump(bcm_tlv_t * ie, char *sum)
{
	int i;
	int rsn;
	wpa_ie_fixed_t *wpa = NULL;
	rsn_parse_info_t rsn_info;
	wpa_suite_t *suite;
	uint8 std_oui[3];
	int unicast_count = 0;
	int akm_count = 0;
	uint16 capabilities;
	uint cntrs;
	int err;

	if (ie->id == DOT11_MNG_RSN_ID) {
		rsn = TRUE;
		memcpy(std_oui, WPA2_OUI, WPA_OUI_LEN);
		err = wl_rsn_ie_parse_info(ie->data, ie->len, &rsn_info);
	} else {
		rsn = FALSE;
		memcpy(std_oui, WPA_OUI, WPA_OUI_LEN);
		wpa = (wpa_ie_fixed_t *) ie;
		err =
		    wl_rsn_ie_parse_info((uint8 *) & wpa->version,
					 wpa->length - WPA_IE_OUITYPE_LEN,
					 &rsn_info);
	}
	if (err || rsn_info.version != WPA_VERSION) {
		strcat(sum, "WEP ");
		return;
	}

	/* Check for multicast suite */
	if (rsn_info.mcast) {
		if (!wlu_bcmp(rsn_info.mcast->oui, std_oui, 3)) {
			switch (rsn_info.mcast->type) {
			case WPA_CIPHER_NONE:
				break;
			case WPA_CIPHER_WEP_40:
				strcat(sum, "Group-WEP64 ");
				break;
			case WPA_CIPHER_WEP_104:
				strcat(sum, "Group-WEP128 ");
				break;
			case WPA_CIPHER_TKIP:
				strcat(sum, "Group-TKIP ");
				break;
			case WPA_CIPHER_AES_OCB:
				strcat(sum, "Group-AES-OCB ");
				break;
			case WPA_CIPHER_AES_CCM:
				strcat(sum, "Group-AES-CCMP ");
				break;
			default:
				sprintf(sum, "Unknown-%s(#%d) ",
					rsn ? "WPA2" : "WPA",
					rsn_info.mcast->type);
				break;
			}
		} else {
			sprintf(sum, "%s Unknown-%02X:%02X:%02X(#%d) ",
				sum, rsn_info.mcast->oui[0],
				rsn_info.mcast->oui[1], rsn_info.mcast->oui[2],
				rsn_info.mcast->type);
		}
	}

	/* Check for unicast suite(s) */
	if (rsn_info.ucast) {
		unicast_count = ltoh16_ua(&rsn_info.ucast->count);
		for (i = 0; i < unicast_count; i++) {
			suite = &rsn_info.ucast->list[i];
			if (!wlu_bcmp(suite->oui, std_oui, 3)) {
				switch (suite->type) {
				case WPA_CIPHER_NONE:
					strcat(sum, "Pair-NONE ");
					break;
				case WPA_CIPHER_WEP_40:
					strcat(sum, "Pair-WEP64 ");
					break;
				case WPA_CIPHER_WEP_104:
					strcat(sum, "Pair-WEP128 ");
					break;
				case WPA_CIPHER_TKIP:
					strcat(sum, "Pair-TKIP ");
					break;
				case WPA_CIPHER_AES_OCB:
					strcat(sum, "Pair-AES-OCB ");
					break;
				case WPA_CIPHER_AES_CCM:
					strcat(sum, "Pair-AES-CCMP ");
					break;
				default:
					sprintf(sum, "WPA-Unknown-%s(#%d) ",
						rsn ? "WPA2" : "WPA",
						suite->type);
					break;
				}
			} else {
				sprintf(sum, "%s Unknown-%02X:%02X:%02X(#%d) ",
					sum, suite->oui[0], suite->oui[1],
					suite->oui[2], suite->type);
			}
		}
		printf("\n");
	}
	/* Authentication Key Management */
	if (rsn_info.akm) {
		akm_count = ltoh16_ua(&rsn_info.akm->count);
		for (i = 0; i < akm_count; i++) {
			suite = &rsn_info.akm->list[i];
			if (!wlu_bcmp(suite->oui, std_oui, 3)) {
				switch (suite->type) {
				case RSN_AKM_NONE:
					strcat(sum, "None ");
					break;
				case RSN_AKM_UNSPECIFIED:
					if (rsn)
						strcat(sum, "WPA2 ");
					else
						strcat(sum, "WPA ");
					break;
				case RSN_AKM_PSK:
					if (rsn)
						strcat(sum, "WPA2-PSK ");
					else
						strcat(sum, "WPA-PSK ");
					break;
				default:
					sprintf(sum, "Unknown-%s(#%d)  ",
						rsn ? "WPA2" : "WPA",
						suite->type);
					break;
				}
			} else {
				sprintf(sum, "%s Unknown-%02X:%02X:%02X(#%d)  ",
					sum, suite->oui[0], suite->oui[1],
					suite->oui[2], suite->type);
			}
		}
	}
}

static uint8 *wlu_parse_tlvs(uint8 * tlv_buf, int buflen, uint key)
{
	uint8 *cp;
	int totlen;

	cp = tlv_buf;
	totlen = buflen;

	/* find tagged parameter */
	while (totlen >= 2) {
		uint tag;
		int len;

		tag = *cp;
		len = *(cp + 1);

		/* validate remaining totlen */
		if ((tag == key) && (totlen >= (len + 2)))
			return (cp);

		cp += (len + 2);
		totlen -= (len + 2);
	}

	return NULL;
}

static char *wl_dump_wpa_rsn_ies(uint8 * cp, uint len)
{
	uint8 *parse = cp;
	uint parse_len = len;
	uint8 *wpaie;
	uint8 *rsnie;
	static char sum[128] = { 0 };
	memset(sum, 0, sizeof(sum));

	while ((wpaie = wlu_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
		if (wlu_is_wpa_ie(&wpaie, &parse, &parse_len))
			break;
	if (wpaie)
		wl_rsn_ie_dump((bcm_tlv_t *) wpaie, sum);

	rsnie = wlu_parse_tlvs(cp, len, DOT11_MNG_RSN_ID);
	if (rsnie)
		wl_rsn_ie_dump((bcm_tlv_t *) rsnie, sum);
	if (wpaie || rsnie)
		return sum;

	return "WEP";
}

static char *getEncInfo(wl_bss_info_t * bi)
{
	if (bi->capability & DOT11_CAP_PRIVACY) {
		if (bi->ie_length)
			return
			    wl_dump_wpa_rsn_ies((uint8 *) (((uint8 *) bi) +
							   bi->ie_offset),
						bi->ie_length);
		else
			return "WEP";
	} else
		return "Open";
}

int site_survey_main(int argc, char *argv[])
{
	char tmp[32];
	sprintf(tmp, "%s_ifname", nvram_safe_get("wifi_display"));
	char *name = nvram_safe_get(tmp);

	unsigned char buf[10000];
	wl_scan_results_t *scan_res = (wl_scan_results_t *) buf;
	wl_bss_info_t *bss_info;
	unsigned char mac[20];
	int i;
	char *dev = name;

	unlink(SITE_SURVEY_DB);
	int ap = 0, oldap = 0;
	wl_scan_params_t params;

	memset(&params, 0, sizeof(params));

	/*
	 * use defaults (same parameters as wl scan) 
	 */

	memset(&params.bssid, 0xff, sizeof(params.bssid));
	if (argc > 1) {
		params.ssid.SSID_len = strlen(argv[1]);
		strcpy(params.ssid.SSID, argv[1]);
	}
	params.bss_type = DOT11_BSSTYPE_ANY;
	params.scan_type = -1;
	params.nprobes = -1;
	params.active_time = -1;
	params.passive_time = -1;
	params.home_time = -1;

	/*
	 * can only scan in STA mode 
	 */
	if (wl_ioctl(dev, WLC_SCAN, &params, 64) < 0) {
		fprintf(stderr, "scan failed\n");
		return -1;
	}
	sleep(1);
	bzero(buf, sizeof(buf));
	scan_res->buflen = sizeof(buf);

	if (wl_ioctl(dev, WLC_SCAN_RESULTS, buf, WLC_IOCTL_MAXLEN) < 0) {
		fprintf(stderr, "scan results failed\n");
		return -1;
	}

	fprintf(stderr, "buflen=[%d] version=[%d] count=[%d]\n",
		scan_res->buflen, scan_res->version, scan_res->count);

	if (scan_res->count == 0) {
		cprintf("Can't find any wireless device\n");
		goto endss;
	}

	bss_info = &scan_res->bss_info[0];
	for (i = 0; i < scan_res->count; i++) {
		strcpy(site_survey_lists[i].SSID, bss_info->SSID);
		strcpy(site_survey_lists[i].BSSID,
		       ether_etoa(bss_info->BSSID.octet, mac));
#ifndef HAVE_RB500
		site_survey_lists[i].channel = bss_info->chanspec & 0xff;
#endif
		site_survey_lists[i].RSSI = bss_info->RSSI;
		site_survey_lists[i].phy_noise = bss_info->phy_noise;
		site_survey_lists[i].beacon_period = bss_info->beacon_period;
		site_survey_lists[i].capability = bss_info->capability;
		site_survey_lists[i].rate_count = bss_info->rateset.count;
		site_survey_lists[i].dtim_period = bss_info->dtim_period;
		strcpy(site_survey_lists[i].ENCINFO, getEncInfo(bss_info));

		bss_info =
		    (wl_bss_info_t *) ((uint32) bss_info + bss_info->length);
	}
	write_site_survey();
	open_site_survey();
	for (i = 0; i < SITE_SURVEY_NUM && site_survey_lists[i].SSID[0]; i++) {
		fprintf(stderr,
			"[%2d] SSID[%20s] BSSID[%s] channel[%2d] rssi[%d] noise[%d] beacon[%d] cap[%x] dtim[%d] rate[%d] enc[%s]\n",
			i, site_survey_lists[i].SSID,
			site_survey_lists[i].BSSID,
			site_survey_lists[i].channel, site_survey_lists[i].RSSI,
			site_survey_lists[i].phy_noise,
			site_survey_lists[i].beacon_period,
			site_survey_lists[i].capability,
			site_survey_lists[i].dtim_period,
			site_survey_lists[i].rate_count,
			site_survey_lists[i].ENCINFO);
	}

endss:

	C_led(0);
	eval("wl", "-i", name, "up");
	return 0;
}

int write_site_survey(void)
{
	FILE *fp;

	if ((fp = fopen(SITE_SURVEY_DB, "w"))) {
		fwrite(&site_survey_lists[0], sizeof(site_survey_lists), 1, fp);
		fclose(fp);
		return FALSE;
	}
	return TRUE;
}

static int open_site_survey(void)
{
	FILE *fp;

	bzero(site_survey_lists, sizeof(site_survey_lists));

	if ((fp = fopen(SITE_SURVEY_DB, "r"))) {
		fread(&site_survey_lists[0], sizeof(site_survey_lists), 1, fp);
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}
