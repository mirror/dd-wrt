/*
 * WPA Supplicant - driver interaction with Linux Prism54.org driver
 * Copyright (c) 2003-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 * Copyright (c) 2004, Luis R. Rodriguez <mcgrof@ruslug.rutgers.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "wireless_copy.h"
#include "common.h"
#include "driver.h"
#include "driver_wext.h"
#include "driver_hostap.h"
#include "l2_packet.h"
#include "wpa_supplicant.h"

struct wpa_driver_prism54_data {
	void *wext; /* private data for driver_wext */
	void *ctx;
	char ifname[IFNAMSIZ + 1];
	int sock;
};

#define PRISM54_SET_WPA    		SIOCIWFIRSTPRIV+12
#define PRISM54_HOSTAPD    		SIOCIWFIRSTPRIV+25
#define PRISM54_DROP_UNENCRYPTED	SIOCIWFIRSTPRIV+26

static void show_set_key_error(struct prism2_hostapd_param *);

static int hostapd_ioctl_prism54(struct wpa_driver_prism54_data *drv,
				 struct prism2_hostapd_param *param,
				 int len, int show_err)
{
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) param;
	iwr.u.data.length = len;

	if (ioctl(drv->sock, PRISM54_HOSTAPD, &iwr) < 0) {
		int ret = errno;
		if (show_err) 
			perror("ioctl[PRISM54_HOSTAPD]");
		return ret;
	}

	return 0;
}


static int wpa_driver_prism54_set_wpa_ie(struct wpa_driver_prism54_data *drv,
					 const u8 *wpa_ie,
					 size_t wpa_ie_len)
{
	struct prism2_hostapd_param *param;
	int res;
	size_t blen = PRISM2_HOSTAPD_GENERIC_ELEMENT_HDR_LEN + wpa_ie_len;
	if (blen < sizeof(*param))
		blen = sizeof(*param);

	param = (struct prism2_hostapd_param *) malloc(blen);
	if (param == NULL)
		return -1;
	
	memset(param, 0, blen);
	param->cmd = PRISM2_HOSTAPD_SET_GENERIC_ELEMENT;
	param->u.generic_elem.len = wpa_ie_len;
	memcpy(param->u.generic_elem.data, wpa_ie, wpa_ie_len);
	res = hostapd_ioctl_prism54(drv, param, blen, 1);

	free(param);

	return res;
}


/* This is called at wpa_supplicant daemon init time */
static int wpa_driver_prism54_set_wpa(void *priv, int enabled)
{
	struct wpa_driver_prism54_data *drv = priv;
	struct prism2_hostapd_param *param;
	int res;
	size_t blen = PRISM2_HOSTAPD_GENERIC_ELEMENT_HDR_LEN;
	if (blen < sizeof(*param))
		blen = sizeof(*param);

	param = (struct prism2_hostapd_param *) malloc(blen);
	if (param == NULL)
		return -1;

	memset(param, 0, blen);
	param->cmd = PRISM54_SET_WPA;
	param->u.generic_elem.len = 0;
	res = hostapd_ioctl_prism54(drv, param, blen, 1);

	free(param);

	return res;
}


static int wpa_driver_prism54_set_key(void *priv, wpa_alg alg,
				      const u8 *addr, int key_idx, int set_tx,
				      const u8 *seq, size_t seq_len,
				      const u8 *key, size_t key_len)
{
	struct wpa_driver_prism54_data *drv = priv;
	struct prism2_hostapd_param *param;
	u8 *buf;
	size_t blen;
	int ret = 0;
	char *alg_name;

	switch (alg) {
	case WPA_ALG_NONE:
		alg_name = "none";
		return -1;
		break;
	case WPA_ALG_WEP:
		alg_name = "WEP";
		return -1;
		break;
	case WPA_ALG_TKIP:
		alg_name = "TKIP";
		break;
	case WPA_ALG_CCMP:
		alg_name = "CCMP";
		return -1;
		break;
	default:
		return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: alg=%s key_idx=%d set_tx=%d seq_len=%lu "
		   "key_len=%lu", __FUNCTION__, alg_name, key_idx, set_tx,
		   (unsigned long) seq_len, (unsigned long) key_len);

	if (seq_len > 8)
		return -2;

	blen = sizeof(*param) + key_len;
	buf = malloc(blen);
	if (buf == NULL)
		return -1;
	memset(buf, 0, blen);

	param = (struct prism2_hostapd_param *) buf;
	param->cmd = PRISM2_SET_ENCRYPTION;
	/* TODO: In theory, STA in client mode can use five keys; four default
	 * keys for receiving (with keyidx 0..3) and one individual key for
	 * both transmitting and receiving (keyidx 0) _unicast_ packets. Now,
	 * keyidx 0 is reserved for this unicast use and default keys can only
	 * use keyidx 1..3 (i.e., default key with keyidx 0 is not supported).
	 * This should be fine for more or less all cases, but for completeness
	 * sake, the driver could be enhanced to support the missing key. */
#if 0
	if (addr == NULL)
		memset(param->sta_addr, 0xff, ETH_ALEN);
	else
		memcpy(param->sta_addr, addr, ETH_ALEN);
#else
	memset(param->sta_addr, 0xff, ETH_ALEN);
#endif
	strncpy((char *) param->u.crypt.alg, alg_name,
		HOSTAP_CRYPT_ALG_NAME_LEN);
	param->u.crypt.flags = set_tx ? HOSTAP_CRYPT_FLAG_SET_TX_KEY : 0;
	param->u.crypt.idx = key_idx;
	memcpy(param->u.crypt.seq, seq, seq_len);
	param->u.crypt.key_len = key_len;
	memcpy((u8 *) (param + 1), key, key_len);

	if (hostapd_ioctl_prism54(drv, param, blen, 1)) {
		wpa_printf(MSG_WARNING, "Failed to set encryption.");
		show_set_key_error(param);
		ret = -1;
	}
	free(buf);

	return ret;
}


static int wpa_driver_prism54_set_countermeasures(void *priv,
						 int enabled)
{
	/* FIX */
	printf("wpa_driver_prism54_set_countermeasures - not yet "
	       "implemented\n");
	return 0;
}


static int wpa_driver_prism54_set_drop_unencrypted(void *priv,
						  int enabled)
{
	struct wpa_driver_prism54_data *drv = priv;
	struct prism2_hostapd_param *param;
	int res;
	size_t blen = PRISM2_HOSTAPD_GENERIC_ELEMENT_HDR_LEN;
	if (blen < sizeof(*param))
		blen = sizeof(*param);

	param = (struct prism2_hostapd_param *) malloc(blen);
	if (param == NULL)
		return -1;

	memset(param, 0, blen);
	param->cmd = PRISM54_DROP_UNENCRYPTED;
	param->u.generic_elem.len = 0;
	res = hostapd_ioctl_prism54(drv, param, blen, 1);

	free(param);

	return res;
}


static int wpa_driver_prism54_deauthenticate(void *priv, const u8 *addr,
					     int reason_code)
{
	/* FIX */
	printf("wpa_driver_prism54_deauthenticate - not yet implemented\n");
	return 0;
}


static int wpa_driver_prism54_disassociate(void *priv, const u8 *addr,
					   int reason_code)
{
	/* FIX */
	printf("wpa_driver_prism54_disassociate - not yet implemented\n");
	return 0;
}


static int
wpa_driver_prism54_associate(void *priv,
			     struct wpa_driver_associate_params *params)
{
	struct wpa_driver_prism54_data *drv = priv;
	int ret = 0;

	if (wpa_driver_prism54_set_wpa_ie(drv, params->wpa_ie,
					  params->wpa_ie_len) < 0)
		ret = -1;
	if (wpa_driver_wext_set_freq(drv->wext, params->freq) < 0)
		ret = -1;
	if (wpa_driver_wext_set_ssid(drv->wext, params->ssid,
				     params->ssid_len) < 0)
		ret = -1;
	if (wpa_driver_wext_set_bssid(drv->wext, params->bssid) < 0)
		ret = -1;

	return ret;
}

static void show_set_key_error(struct prism2_hostapd_param *param)
{
	switch (param->u.crypt.err) {
	case HOSTAP_CRYPT_ERR_UNKNOWN_ALG:
		wpa_printf(MSG_INFO, "Unknown algorithm '%s'.",
			   param->u.crypt.alg);
		wpa_printf(MSG_INFO, "You may need to load kernel module to "
			   "register that algorithm.");
		wpa_printf(MSG_INFO, "E.g., 'modprobe hostap_crypt_wep' for "
			   "WEP.");
		break;
	case HOSTAP_CRYPT_ERR_UNKNOWN_ADDR:
		wpa_printf(MSG_INFO, "Unknown address " MACSTR ".",
			   MAC2STR(param->sta_addr));
		break;
	case HOSTAP_CRYPT_ERR_CRYPT_INIT_FAILED:
		wpa_printf(MSG_INFO, "Crypt algorithm initialization failed.");
		break;
	case HOSTAP_CRYPT_ERR_KEY_SET_FAILED:
		wpa_printf(MSG_INFO, "Key setting failed.");
		break;
	case HOSTAP_CRYPT_ERR_TX_KEY_SET_FAILED:
		wpa_printf(MSG_INFO, "TX key index setting failed.");
		break;
	case HOSTAP_CRYPT_ERR_CARD_CONF_FAILED:
		wpa_printf(MSG_INFO, "Card configuration failed.");
		break;
	}
}


static int wpa_driver_prism54_get_bssid(void *priv, u8 *bssid)
{
	struct wpa_driver_prism54_data *drv = priv;
	return wpa_driver_wext_get_bssid(drv->wext, bssid);
}


static int wpa_driver_prism54_get_ssid(void *priv, u8 *ssid)
{
	struct wpa_driver_prism54_data *drv = priv;
	return wpa_driver_wext_get_ssid(drv->wext, ssid);
}


static int wpa_driver_prism54_scan(void *priv, const u8 *ssid, size_t ssid_len)
{
	struct wpa_driver_prism54_data *drv = priv;
	return wpa_driver_wext_scan(drv->wext, ssid, ssid_len);
}


static int wpa_driver_prism54_get_scan_results(void *priv,
					    struct wpa_scan_result *results,
					    size_t max_size)
{
	struct wpa_driver_prism54_data *drv = priv;
	return wpa_driver_wext_get_scan_results(drv->wext, results, max_size);
}


static void * wpa_driver_prism54_init(void *ctx, const char *ifname)
{
	struct wpa_driver_prism54_data *drv;

	drv = malloc(sizeof(*drv));
	if (drv == NULL)
		return NULL;
	memset(drv, 0, sizeof(*drv));
	drv->wext = wpa_driver_wext_init(ctx, ifname);
	if (drv->wext == NULL) {
		free(drv);
		return NULL;
	}

	drv->ctx = ctx;
	strncpy(drv->ifname, ifname, sizeof(drv->ifname));
	drv->sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->sock < 0) {
		wpa_driver_wext_deinit(drv->wext);
		free(drv);
		return NULL;
	}

	return drv;
}


static void wpa_driver_prism54_deinit(void *priv)
{
	struct wpa_driver_prism54_data *drv = priv;
	wpa_driver_wext_deinit(drv->wext);
	close(drv->sock);
	free(drv);
}


const struct wpa_driver_ops wpa_driver_prism54_ops = {
	.name = "prism54",
	.desc = "Prism54.org driver (Intersil Prism GT/Duette/Indigo)",
	.get_bssid = wpa_driver_prism54_get_bssid,
	.get_ssid = wpa_driver_prism54_get_ssid,
	.set_wpa = wpa_driver_prism54_set_wpa,
	.set_key = wpa_driver_prism54_set_key,
	.set_countermeasures = wpa_driver_prism54_set_countermeasures,
	.set_drop_unencrypted = wpa_driver_prism54_set_drop_unencrypted,
	.scan = wpa_driver_prism54_scan,
	.get_scan_results = wpa_driver_prism54_get_scan_results,
	.deauthenticate = wpa_driver_prism54_deauthenticate,
	.disassociate = wpa_driver_prism54_disassociate,
	.associate = wpa_driver_prism54_associate,
	.init = wpa_driver_prism54_init,
	.deinit = wpa_driver_prism54_deinit,
};
