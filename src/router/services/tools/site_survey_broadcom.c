/*g
 * site_survey_broadcom.c
 *
 * Copyright (C) 2006 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#define sys_restart() kill(1, SIGHUP)

int write_site_survey(void);
static int local_open_site_survey(void);
int write_site_survey(void);

struct site_survey_list *site_survey_lists;

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
	return (((uint8 *)bytes)[1] << 8) + ((uint8 *)bytes)[0];
}

static bool wlu_is_wpa_ie(uint8 **wpaie, uint8 **tlvs, uint *tlvs_len)
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

static int wl_rsn_ie_parse_info(uint8 *rsn_buf, uint len, rsn_parse_info_t *rsn)
{
	uint16 count;

	bzero(rsn, sizeof(rsn_parse_info_t));

	/* version */
	if (len < sizeof(uint16))
		return 1;

	rsn->version = ltoh16_ua(rsn_buf);
	len -= sizeof(uint16);
	rsn_buf += sizeof(uint16);

	/* Multicast Suite */
	if (len < sizeof(wpa_suite_mcast_t))
		return 0;

	rsn->mcast = (wpa_suite_mcast_t *)rsn_buf;
	len -= sizeof(wpa_suite_mcast_t);
	rsn_buf += sizeof(wpa_suite_mcast_t);

	/* Unicast Suite */
	if (len < sizeof(uint16))
		return 0;

	count = ltoh16_ua(rsn_buf);

	if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
		return 1;

	rsn->ucast = (wpa_suite_ucast_t *)rsn_buf;
	len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
	rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

	/* AKM Suite */
	if (len < sizeof(uint16))
		return 0;

	count = ltoh16_ua(rsn_buf);

	if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
		return 1;

	rsn->akm = (wpa_suite_auth_key_mgmt_t *)rsn_buf;
	len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
	rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

	/* Capabilites */
	if (len < sizeof(uint16))
		return 0;

	rsn->capabilities = rsn_buf;

	return 0;
}

static void wl_rsn_ie_dump(bcm_tlv_t *ie, char *sum)
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
		wpa = (wpa_ie_fixed_t *)ie;
		err = wl_rsn_ie_parse_info((uint8 *)&wpa->version, wpa->length - WPA_IE_OUITYPE_LEN, &rsn_info);
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
			case 8:
				strcat(sum, "Group-GCMP-128 ");
				break;
			case 9:
				strcat(sum, "Group-GCMP-256 ");
				break;
			case 10:
				strcat(sum, "Group-CCMP-256 ");
				break;
			default:
				sprintf(sum, "Unknown-%s(#%d) ", rsn ? "WPA2" : "WPA", rsn_info.mcast->type);
				break;
			}
		} else {
			sprintf(sum, "%s Unknown-%02X:%02X:%02X(#%d) ", sum, rsn_info.mcast->oui[0], rsn_info.mcast->oui[1],
				rsn_info.mcast->oui[2], rsn_info.mcast->type);
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
					strcat(sum, "NONE ");
					break;
				case WPA_CIPHER_WEP_40:
					strcat(sum, "WEP40 ");
					break;
				case WPA_CIPHER_WEP_104:
					strcat(sum, "WEP104 ");
					break;
				case WPA_CIPHER_TKIP:
					strcat(sum, "TKIP ");
					break;
				case WPA_CIPHER_AES_OCB:
					strcat(sum, "OCB ");
					break;
				case WPA_CIPHER_AES_CCM:
					strcat(sum, "CCMP ");
					break;
				case 6:
					strcat(sum, "AES-128-CMAC ");
					break;
				case 7:
					strcat(sum, "NO-GROUP ");
					break;
				case 8:
					strcat(sum, "GCMP-128 ");
					break;
				case 9:
					strcat(sum, "GCMP-256 ");
					break;
				case 10:
					strcat(sum, "CCMP-256 ");
					break;
				case 11:
					strcat(sum, "AES-128-GMAC ");
					break;
				case 12:
					strcat(sum, "AES-256-GMAC ");
					break;
				case 13:
					strcat(sum, "AES-256-CMAC ");
					break;
				default:
					sprintf(sum, "WPA-Unknown-%s(#%d) ", rsn ? "WPA2" : "WPA", suite->type);
					break;
				}
			} else {
				sprintf(sum, "%s Unknown-%02X:%02X:%02X(#%d) ", sum, suite->oui[0], suite->oui[1], suite->oui[2],
					suite->type);
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
						strcat(sum, "PSK2 ");
					else
						strcat(sum, "PSK ");
					break;
				case 3:
					strcat(sum, "FT/EAP ");
					break;
				case 4:
					strcat(sum, "FT/PSK ");
					break;
				case 5:
					strcat(sum, "EAP/SHA-256 ");
					break;
				case 6:
					strcat(sum, "PSK/SHA-256 ");
					break;
				case 7:
					strcat(sum, "TDLS/TPK ");
					break;
				case 8:
					strcat(sum, "SAE/PSK3 ");
					break;
				case 9:
					strcat(sum, "FT/SAE ");
					break;
				case 11:
					strcat(sum, "EAP/SUITE-B ");
					break;
				case 12:
					strcat(sum, "EAP/SUITE-B-192 ");
					break;
				case 14:
					strcat(sum, "FILS/SHA256 ");
					break;
				case 15:
					strcat(sum, "FILS/SHA384 ");
					break;
				case 16:
					strcat(sum, "FT-FILS/SHA256 ");
					break;
				case 17:
					strcat(sum, "FT-FILS/SHA384 ");
					break;
				case 18:
					strcat(sum, "OWE ");
					break;
				default:
					sprintf(sum, "Unknown-%s(#%d) ", rsn ? "WPA2" : "WPA", suite->type);
					break;
				}
			} else {
				sprintf(sum, "%s Unknown-%02X:%02X:%02X(#%d) ", sum, suite->oui[0], suite->oui[1], suite->oui[2],
					suite->type);
			}
		}
	}
}

static uint8 *wlu_parse_tlvs(uint8 *tlv_buf, int buflen, uint key)
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

struct ieee80211_mtik_ie_data {
	unsigned char data1[2]; /* unknown yet 0x011e */
	unsigned char flags; /* 4(100) - wds, 1(1) - nstream, 8(1000) - pooling, 0 - none */
	unsigned char data2[3]; /* unknown yet fill with zero */
	unsigned char version[4]; /* little endian version. Use 0x1f660902 */
	unsigned char pad1; /* a kind of padding, 0xff */
	unsigned char namelen; /* length of radio name. Change with caution. 0x0f is safe value */
	unsigned char radioname[15]; /* Radio name */
	unsigned char pad2[5]; /* unknown. fill with zero */
} __attribute__((packed));

struct ieee80211_mtik_ie {
	unsigned char id; /* IEEE80211_ELEMID_VENDOR */
	unsigned char len; /* length in bytes */
	unsigned char oui[3]; /* 0x00, 0x50, 0xf2 */
	unsigned char type; /* OUI type */
	unsigned short version; /* spec revision */
	struct ieee80211_mtik_ie_data iedata;
} __attribute__((packed));

struct aironet_ie {
	unsigned char id; /* IEEE80211_ELEMID_VENDOR */
	unsigned char len; /* length in bytes */
	unsigned char load;
	unsigned char hops;
	unsigned char device;
	unsigned char refresh_rate;
	unsigned short cwmin;
	unsigned short cwmax;
	unsigned char flags;
	unsigned char distance;
	char name[16]; /* AP or Client's machine name */
	unsigned short num_assoc; /* number of clients associated */
	unsigned short radiotype;
} __attribute__((packed));

static unsigned char brcm_oui[3] = { 0x00, 0x10, 0x18 };
static unsigned char mtik_oui[3] = { 0x00, 0x0c, 0x42 };

static void wl_dump_wpa_rsn_ies(uint8 *cp, uint len, struct site_survey_list *list)
{
	uint8 *parse = cp;
	uint8 *parse2 = cp;
	uint parse_len = len;
	uint8 *wpaie;
	uint8 *rsnie;
	uint8 *customie;
	struct aironet_ie *aironet;
	static char sum[128] = { 0 };
	bzero(sum, sizeof(sum));

	strcpy(list->ENCINFO, "WEP");

	while ((wpaie = wlu_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
		if (wlu_is_wpa_ie(&wpaie, &parse, &parse_len))
			break;
	if (wpaie)
		wl_rsn_ie_dump((bcm_tlv_t *)wpaie, sum);

	rsnie = wlu_parse_tlvs(cp, len, DOT11_MNG_RSN_ID);
	if (rsnie)
		wl_rsn_ie_dump((bcm_tlv_t *)rsnie, sum);
	if (wpaie || rsnie) {
		if (*sum > 0)
			sum[strlen(sum)] = 0;
		strcpy(list->ENCINFO, sum);
	}
	parse = cp;
	while ((customie = wlu_parse_tlvs(parse, len, 221))) {
		if (customie[1] >= 4 && !memcmp(&customie[2], brcm_oui, 3)) {
			if (customie[5] == 2) {
				list->numsta = customie[6];
				if (customie[8] & 0x80)
					list->extcap = CAP_DWDS;
				break;
			}
		}
		if (customie[1] >= 4 && !memcmp(&customie[2], mtik_oui, 3)) {
			struct ieee80211_mtik_ie *ie = customie;
			if (ie->iedata.namelen <= 15) {
				memcpy(list->radioname, ie->iedata.radioname, ie->iedata.namelen);
			}
			if (ie->iedata.flags & 0x4) {
				list->extcap = CAP_MTIKWDS;
			}
		}
		len -= (customie + customie[1] + 2) - parse;
		parse = customie + customie[1] + 2;
	}
	parse = cp;
	aironet = (struct aironet_ie *)wlu_parse_tlvs(parse, len, 133);
	if (aironet) {
		list->numsta = aironet->num_assoc;
		memcpy(list->radioname, aironet->name, 15);
	}
}

static void getEncInfo(wl_bss_info_t *bi, struct site_survey_list *list)
{
	if (bi->capability & DOT11_CAP_PRIVACY) {
		if (bi->ie_length) {
			wl_dump_wpa_rsn_ies((uint8 *)(((uint8 *)bi) + bi->ie_offset), bi->ie_length, list);
		} else
			strcpy(list->ENCINFO, "WEP");
	} else
		strcpy(list->ENCINFO, "Open");
}

static int get_mcs_max(const unsigned char *mcs)
{
	unsigned int mcs_bit, prev_bit = -2, prev_cont = 0;

	for (mcs_bit = 0; mcs_bit < 16 * 8; mcs_bit++) {
		unsigned int mcs_octet = mcs_bit / 8;
		unsigned int MCS_RATE_BIT = 1 << mcs_bit % 8;
		bool mcs_rate_idx_set;

		mcs_rate_idx_set = !!(mcs[mcs_octet] & MCS_RATE_BIT);

		if (!mcs_rate_idx_set)
			break;

		if (prev_bit != mcs_bit - 1) {
			prev_cont = 0;
		} else if (!prev_cont) {
			prev_cont = 1;
		}

		prev_bit = mcs_bit;
	}
	if (prev_cont) {
		if (prev_bit == 7)
			return 150;
		if (prev_bit == 15)
			return 300;
		if (prev_bit == 23)
			return 450;
		if (prev_bit == 31)
			return 600;
	}
	return 0;
}

int get_legacy(unsigned char *rates, int count)
{
	int r, b, i;
	int maxrate = 0;
	for (i = 0; i < count; i++) {
		r = rates[i] & 0x7f;
		//b = rates[i] & 0x80;
		if (r == 0)
			break;
		if (r > maxrate)
			maxrate = r;
	}
	return maxrate / 2;
}

int site_survey_main(int argc, char *argv[])
{
	site_survey_lists = calloc(sizeof(struct site_survey_list) * SITE_SURVEY_NUM, 1);

	char tmp[32];
	sprintf(tmp, "%s_ifname", nvram_safe_get("wifi_display"));
	char *name = nvram_safe_get(tmp);

	unsigned char buf[10000];
	wl_scan_results_t *scan_res = (wl_scan_results_t *)buf;
	wl_bss_info_t *bss_info;
	unsigned char mac[20];
	int i;
	char *dev = name;

	unlink(SITE_SURVEY_DB);
	int ap = 0, oldap = 0;
	wl_scan_params_t params;

	bzero(&params, sizeof(params));

	/*
	 * use defaults (same parameters as wl scan) 
	 */

	memset(&params.bssid, 0xff, sizeof(params.bssid));
	if (argc > 1) {
		params.ssid.SSID_len = strlen(argv[1]);
		strcpy(params.ssid.SSID, argv[1]);
	}
	params.bss_type = DOT11_BSSTYPE_ANY;
	params.scan_type = 0;
	params.nprobes = -1;
	params.active_time = -1;
	params.passive_time = -1;
	params.home_time = -1;
	params.channel_num = 0;

	/*
	 * can only scan in STA mode 
	 */
	if (wl_ioctl(dev, WLC_SCAN, &params, 64) < 0) {
		fprintf(stderr, "scan failed\n");
		return -1;
	}
	int count = 10;
	int ret = 0;
	while ((count--) > 0) //scan for max 5 seconds
	{
		usleep(1000 * 1000);

		bzero(buf, sizeof(buf));
		scan_res->buflen = sizeof(buf);
		ret = wl_ioctl(dev, WLC_SCAN_RESULTS, buf, WLC_IOCTL_MAXLEN);
		if (!ret)
			break;
	}
	if (ret < 0) {
		fprintf(stderr, "scan failed with errorcode %d\n", ret);
	}

	fprintf(stderr, "buflen=[%d] version=[%d] count=[%d]\n", scan_res->buflen, scan_res->version, scan_res->count);

	if (scan_res->count == 0) {
		goto endss;
	}

	bss_info = &scan_res->bss_info[0];
	for (i = 0; i < scan_res->count; i++) {
		strcpy(site_survey_lists[i].SSID, bss_info->SSID);
		strcpy(site_survey_lists[i].BSSID, ether_etoa(bss_info->BSSID.octet, mac));
		site_survey_lists[i].channel = bss_info->chanspec & 0xff;
		switch ((bss_info->chanspec & 0x3800)) {
		case 0:
		case 0x800:
		case 0x1000:
			site_survey_lists[i].channel = bss_info->chanspec & 0xff;
			break;
		case 0x0C00: // for older version
		case 0x1800:
			site_survey_lists[i].channel = bss_info->ctl_ch | 0x2000;
			break;
		case 0x2000:
		case 0x2800:
		case 0x3000:
			site_survey_lists[i].channel = bss_info->ctl_ch;
		}

#ifdef WL_CHANSPEC_BW_80
		switch (bss_info->chanspec & 0x3800) {
		case WL_CHANSPEC_BW_80:
			site_survey_lists[i].channel |= 0x1100;
			break;
		case WL_CHANSPEC_BW_8080:
			site_survey_lists[i].channel |= 0x1200;
			break;
		case WL_CHANSPEC_BW_160:
			site_survey_lists[i].channel |= 0x1200;
			break;
		}
#endif
		site_survey_lists[i].frequency = ieee80211_ieee2mhz(site_survey_lists[i].channel & 0xff);

		site_survey_lists[i].RSSI = bss_info->RSSI;
		site_survey_lists[i].phy_noise = bss_info->phy_noise;
		site_survey_lists[i].beacon_period = bss_info->beacon_period;
		site_survey_lists[i].capability = bss_info->capability;
		site_survey_lists[i].rate_count = get_mcs_max(bss_info->basic_mcs);
		if (!site_survey_lists[i].rate_count)
			site_survey_lists[i].rate_count = get_legacy(bss_info->rateset.rates, bss_info->rateset.count);

		site_survey_lists[i].dtim_period = bss_info->dtim_period;
		site_survey_lists[i].numsta = -1;
		getEncInfo(bss_info, &site_survey_lists[i]);

		bss_info = (wl_bss_info_t *)((uint32)bss_info + bss_info->length);
	}
	write_site_survey();
	local_open_site_survey();
	// modded by ascott and fractal, may 17th, 2012 to show "hidden" SSIDS
	for (i = 0; i < SITE_SURVEY_NUM && site_survey_lists[i].frequency; i++) {
		if (site_survey_lists[i].SSID[0] == 0) {
			strcpy(site_survey_lists[i].SSID, "hidden");
		}
		fprintf(stderr,
			"[%2d] SSID[%20s] BSSID[%s] channel[%2d] frequency[%4d] numsta[%d] rssi[%d] noise[%d] beacon[%d] cap[%x] dtim[%d] rate[%d] enc[%s]\n",
			i, site_survey_lists[i].SSID, site_survey_lists[i].BSSID, site_survey_lists[i].channel & 0xff,
			site_survey_lists[i].frequency, site_survey_lists[i].numsta, site_survey_lists[i].RSSI,
			site_survey_lists[i].phy_noise, site_survey_lists[i].beacon_period, site_survey_lists[i].capability,
			site_survey_lists[i].dtim_period, site_survey_lists[i].rate_count, site_survey_lists[i].ENCINFO);
	}

endss:

	C_led(0);
	eval("wl", "-i", name, "up");
	free(site_survey_lists);
	return 0;
}

int write_site_survey(void)
{
	FILE *fp;

	if ((fp = fopen(SITE_SURVEY_DB, "w"))) {
		fwrite(&site_survey_lists[0], sizeof(struct site_survey_list) * SITE_SURVEY_NUM, 1, fp);
		fclose(fp);
		return FALSE;
	}
	return TRUE;
}

static int local_open_site_survey(void)
{
	FILE *fp;

	bzero(site_survey_lists, sizeof(site_survey_lists) * SITE_SURVEY_NUM);

	if ((fp = fopen(SITE_SURVEY_DB, "r"))) {
		fread(&site_survey_lists[0], sizeof(struct site_survey_list) * SITE_SURVEY_NUM, 1, fp);
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}

#ifdef TEST

void main(int argc, char *argv[])
{
	site_survey_main(argc, argv);
}
#endif
