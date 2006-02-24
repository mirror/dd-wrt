/*
 * WPA Supplicant - driver interaction with MADWIFI 802.11 driver
 * Copyright (c) 2004, Sam Leffler <sam@errno.com>
 * Copyright (c) 2004-2005, Jouni Malinen <jkmaline@cc.hut.fi>
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

#include "common.h"
#include "driver.h"
#include "driver_wext.h"
#include "eloop.h"
#include "wpa_supplicant.h"
#include "wpa.h"
#include "wireless_copy.h"

#include <include/compat.h>
#include <net80211/ieee80211.h>
#ifdef WME_NUM_AC
/* Assume this is built against BSD branch of madwifi driver. */
#define MADWIFI_BSD
#include <net80211/_ieee80211.h>
#endif /* WME_NUM_AC */
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>

#ifdef IEEE80211_IOCTL_SETWMMPARAMS
/* Assume this is built against madwifi-ng */
#define MADWIFI_NG
#endif /* IEEE80211_IOCTL_SETWMMPARAMS */

struct wpa_driver_madwifi_data {
	void *wext; /* private data for driver_wext */
	void *ctx;
	char ifname[IFNAMSIZ + 1];
	int sock;
};

static int
set80211priv(struct wpa_driver_madwifi_data *drv, int op, void *data, int len,
	     int show_err)
{
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	if (len < IFNAMSIZ) {
		/*
		 * Argument data fits inline; put it there.
		 */
		memcpy(iwr.u.name, data, len);
	} else {
		/*
		 * Argument data too big for inline transfer; setup a
		 * parameter block instead; the kernel will transfer
		 * the data for the driver.
		 */
		iwr.u.data.pointer = data;
		iwr.u.data.length = len;
	}

	if (ioctl(drv->sock, op, &iwr) < 0) {
		if (show_err) {
#ifdef MADWIFI_NG
			int first = IEEE80211_IOCTL_SETPARAM;
			int last = IEEE80211_IOCTL_GETMODE;
			static const char *opnames[] = {
				"ioctl[IEEE80211_IOCTL_SETPARAM]",
				"ioctl[IEEE80211_IOCTL_GETPARAM]",
				"ioctl[IEEE80211_IOCTL_SETKEY]",
				"ioctl[IEEE80211_IOCTL_SETWMMPARAMS]",
				"ioctl[IEEE80211_IOCTL_DELKEY]",
				"ioctl[IEEE80211_IOCTL_GETWMMPARAMS]",
				"ioctl[IEEE80211_IOCTL_SETMLME]",
				"ioctl[IEEE80211_IOCTL_GETCHANINFO]",
				"ioctl[IEEE80211_IOCTL_SETOPTIE]",
				"ioctl[IEEE80211_IOCTL_GETOPTIE]",
				"ioctl[IEEE80211_IOCTL_ADDMAC]",
				NULL,
				"ioctl[IEEE80211_IOCTL_DELMAC]",
				"ioctl[IEEE80211_IOCTL_GETCHANLIST]",
				"ioctl[IEEE80211_IOCTL_SETCHANLIST]",
				NULL,
				"ioctl[IEEE80211_IOCTL_SETMODE]",
				"ioctl[IEEE80211_IOCTL_GETMODE]",
			};
#else /* MADWIFI_NG */
			int first = IEEE80211_IOCTL_SETPARAM;
			int last = IEEE80211_IOCTL_CHANLIST;
			static const char *opnames[] = {
				"ioctl[IEEE80211_IOCTL_SETPARAM]",
				"ioctl[IEEE80211_IOCTL_GETPARAM]",
				"ioctl[IEEE80211_IOCTL_SETKEY]",
				"ioctl[IEEE80211_IOCTL_GETKEY]",
				"ioctl[IEEE80211_IOCTL_DELKEY]",
				NULL,
				"ioctl[IEEE80211_IOCTL_SETMLME]",
				NULL,
				"ioctl[IEEE80211_IOCTL_SETOPTIE]",
				"ioctl[IEEE80211_IOCTL_GETOPTIE]",
				"ioctl[IEEE80211_IOCTL_ADDMAC]",
				NULL,
				"ioctl[IEEE80211_IOCTL_DELMAC]",
				NULL,
				"ioctl[IEEE80211_IOCTL_CHANLIST]",
			};
#endif /* MADWIFI_NG */
			if (first <= op && op <= last)
				perror(opnames[op - first]);
			else
				perror("ioctl[unknown???]");
		}
		return -1;
	}
	return 0;
}

static int
set80211param(struct wpa_driver_madwifi_data *drv, int op, int arg,
	      int show_err)
{
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.mode = op;
	memcpy(iwr.u.name+sizeof(__u32), &arg, sizeof(arg));

	if (ioctl(drv->sock, IEEE80211_IOCTL_SETPARAM, &iwr) < 0) {
		if (show_err) 
			perror("ioctl[IEEE80211_IOCTL_SETPARAM]");
		return -1;
	}
	return 0;
}

static int
wpa_driver_madwifi_set_wpa_ie(struct wpa_driver_madwifi_data *drv,
			      const u8 *wpa_ie, size_t wpa_ie_len)
{
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	/* NB: SETOPTIE is not fixed-size so must not be inlined */
	iwr.u.data.pointer = (void *) wpa_ie;
	iwr.u.data.length = wpa_ie_len;

	if (ioctl(drv->sock, IEEE80211_IOCTL_SETOPTIE, &iwr) < 0) {
		perror("ioctl[IEEE80211_IOCTL_SETOPTIE]");
		return -1;
	}
	return 0;
}

static int
wpa_driver_madwifi_del_key(struct wpa_driver_madwifi_data *drv, int key_idx,
			   const u8 *addr)
{
	struct ieee80211req_del_key wk;

	wpa_printf(MSG_DEBUG, "%s: keyidx=%d", __FUNCTION__, key_idx);
	memset(&wk, 0, sizeof(wk));
	wk.idk_keyix = key_idx;
	if (addr != NULL)
		memcpy(wk.idk_macaddr, addr, IEEE80211_ADDR_LEN);

	return set80211priv(drv, IEEE80211_IOCTL_DELKEY, &wk, sizeof(wk), 1);
}

static int
wpa_driver_madwifi_set_key(void *priv, wpa_alg alg,
			   const u8 *addr, int key_idx, int set_tx,
			   const u8 *seq, size_t seq_len,
			   const u8 *key, size_t key_len)
{
	struct wpa_driver_madwifi_data *drv = priv;
	struct ieee80211req_key wk;
	char *alg_name;
	u_int8_t cipher;

	if (alg == WPA_ALG_NONE)
		return wpa_driver_madwifi_del_key(drv, key_idx, addr);

	switch (alg) {
	case WPA_ALG_WEP:
		if (addr == NULL || memcmp(addr, "\xff\xff\xff\xff\xff\xff",
					   ETH_ALEN) == 0) {
			/*
			 * madwifi did not seem to like static WEP key
			 * configuration with IEEE80211_IOCTL_SETKEY, so use
			 * Linux wireless extensions ioctl for this.
			 */
			return wpa_driver_wext_set_key(drv->wext, alg, addr,
						       key_idx, set_tx,
						       seq, seq_len,
						       key, key_len);
		}
		alg_name = "WEP";
		cipher = IEEE80211_CIPHER_WEP;
		break;
	case WPA_ALG_TKIP:
		alg_name = "TKIP";
		cipher = IEEE80211_CIPHER_TKIP;
		break;
	case WPA_ALG_CCMP:
		alg_name = "CCMP";
		cipher = IEEE80211_CIPHER_AES_CCM;
		break;
	default:
		wpa_printf(MSG_DEBUG, "%s: unknown/unsupported algorithm %d",
			__FUNCTION__, alg);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: alg=%s key_idx=%d set_tx=%d seq_len=%lu "
		   "key_len=%lu", __FUNCTION__, alg_name, key_idx, set_tx,
		   (unsigned long) seq_len, (unsigned long) key_len);

	if (seq_len > sizeof(u_int64_t)) {
		wpa_printf(MSG_DEBUG, "%s: seq_len %lu too big",
			   __FUNCTION__, (unsigned long) seq_len);
		return -2;
	}
	if (key_len > sizeof(wk.ik_keydata)) {
		wpa_printf(MSG_DEBUG, "%s: key length %lu too big",
			   __FUNCTION__, (unsigned long) key_len);
		return -3;
	}

	memset(&wk, 0, sizeof(wk));
	wk.ik_type = cipher;
	wk.ik_flags = IEEE80211_KEY_RECV;
	if (addr == NULL ||
	    memcmp(addr, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0)
		wk.ik_flags |= IEEE80211_KEY_GROUP;
	if (set_tx) {
		wk.ik_flags |= IEEE80211_KEY_XMIT | IEEE80211_KEY_DEFAULT;
		memcpy(wk.ik_macaddr, addr, IEEE80211_ADDR_LEN);
	} else
		memset(wk.ik_macaddr, 0, IEEE80211_ADDR_LEN);
	wk.ik_keyix = key_idx;
	wk.ik_keylen = key_len;
	memcpy(&wk.ik_keyrsc, seq, seq_len);
	memcpy(wk.ik_keydata, key, key_len);

	return set80211priv(drv, IEEE80211_IOCTL_SETKEY, &wk, sizeof(wk), 1);
}

static int
wpa_driver_madwifi_set_countermeasures(void *priv, int enabled)
{
	struct wpa_driver_madwifi_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __FUNCTION__, enabled);
	return set80211param(drv, IEEE80211_PARAM_COUNTERMEASURES, enabled, 1);
}


static int
wpa_driver_madwifi_set_drop_unencrypted(void *priv, int enabled)
{
	struct wpa_driver_madwifi_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __FUNCTION__, enabled);
	return set80211param(drv, IEEE80211_PARAM_DROPUNENCRYPTED, enabled, 1);
}

static int
wpa_driver_madwifi_deauthenticate(void *priv, const u8 *addr, int reason_code)
{
	struct wpa_driver_madwifi_data *drv = priv;
	struct ieee80211req_mlme mlme;

	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);
	mlme.im_op = IEEE80211_MLME_DEAUTH;
	mlme.im_reason = reason_code;
	memcpy(mlme.im_macaddr, addr, IEEE80211_ADDR_LEN);
	return set80211priv(drv, IEEE80211_IOCTL_SETMLME, &mlme, sizeof(mlme), 1);
}

static int
wpa_driver_madwifi_disassociate(void *priv, const u8 *addr, int reason_code)
{
	struct wpa_driver_madwifi_data *drv = priv;
	struct ieee80211req_mlme mlme;

	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);
	mlme.im_op = IEEE80211_MLME_DISASSOC;
	mlme.im_reason = reason_code;
	memcpy(mlme.im_macaddr, addr, IEEE80211_ADDR_LEN);
	return set80211priv(drv, IEEE80211_IOCTL_SETMLME, &mlme, sizeof(mlme), 1);
}

static int
wpa_driver_madwifi_associate(void *priv,
			     struct wpa_driver_associate_params *params)
{
	struct wpa_driver_madwifi_data *drv = priv;
	struct ieee80211req_mlme mlme;
	int ret = 0, privacy = 1;

	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);

	/*
	 * NB: Don't need to set the freq or cipher-related state as
	 *     this is implied by the bssid which is used to locate
	 *     the scanned node state which holds it.  The ssid is
	 *     needed to disambiguate an AP that broadcasts multiple
	 *     ssid's but uses the same bssid.
	 */
	/* XXX error handling is wrong but unclear what to do... */
	if (wpa_driver_madwifi_set_wpa_ie(drv, params->wpa_ie,
					  params->wpa_ie_len) < 0)
		ret = -1;

	if (params->pairwise_suite == CIPHER_NONE &&
	    params->group_suite == CIPHER_NONE &&
	    params->key_mgmt_suite == KEY_MGMT_NONE &&
	    params->wpa_ie_len == 0)
		privacy = 0;

	if (set80211param(drv, IEEE80211_PARAM_PRIVACY, privacy, 1) < 0)
		ret = -1;

	if (params->wpa_ie_len &&
	    set80211param(drv, IEEE80211_PARAM_WPA,
			  params->wpa_ie[0] == RSN_INFO_ELEM ? 2 : 1, 1) < 0)
		ret = -1;

	if (params->bssid == NULL) {
		/* ap_scan=2 mode - driver takes care of AP selection and
		 * roaming */
		/* FIX: this does not seem to work; would probably need to
		 * change something in the driver */
		if (set80211param(drv, IEEE80211_PARAM_ROAMING, 0, 1) < 0)
			ret = -1;
	}

	if (wpa_driver_wext_set_ssid(drv->wext, params->ssid,
				     params->ssid_len) < 0)
		ret = -1;
	if (params->bssid) {
		if (set80211param(drv, IEEE80211_PARAM_ROAMING, 2, 1) < 0)
			ret = -1;
		memset(&mlme, 0, sizeof(mlme));
		mlme.im_op = IEEE80211_MLME_ASSOC;
		memcpy(mlme.im_macaddr, params->bssid, IEEE80211_ADDR_LEN);
		if (set80211priv(drv, IEEE80211_IOCTL_SETMLME, &mlme,
				 sizeof(mlme), 1) < 0)
			ret = -1;
	}

	return ret;
}

static int
wpa_driver_madwifi_set_auth_alg(void *priv, int auth_alg)
{
	struct wpa_driver_madwifi_data *drv = priv;
	int authmode;

	if ((auth_alg & AUTH_ALG_OPEN_SYSTEM) &&
	    (auth_alg & AUTH_ALG_SHARED_KEY))
		authmode = IEEE80211_AUTH_AUTO;
	else if (auth_alg & AUTH_ALG_SHARED_KEY)
		authmode = IEEE80211_AUTH_SHARED;
	else
		authmode = IEEE80211_AUTH_OPEN;

	return set80211param(drv, IEEE80211_PARAM_AUTHMODE, authmode, 1);
}

static int
wpa_driver_madwifi_scan(void *priv, const u8 *ssid, size_t ssid_len)
{
	struct wpa_driver_madwifi_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);

	/* set desired ssid before scan */
	/* FIX: scan should not break the current association, so using
	 * set_ssid may not be the best way of doing this.. */
	if (wpa_driver_wext_set_ssid(drv->wext, ssid, ssid_len) < 0)
		ret = -1;

	if (ioctl(drv->sock, SIOCSIWSCAN, &iwr) < 0) {
		perror("ioctl[SIOCSIWSCAN]");
		ret = -1;
	}

	/*
	 * madwifi delivers a scan complete event so no need to poll, but
	 * register a backup timeout anyway to make sure that we recover even
	 * if the driver does not send this event for any reason. This timeout
	 * will only be used if the event is not delivered (event handler will
	 * cancel the timeout).
	 */
	eloop_register_timeout(30, 0, wpa_driver_wext_scan_timeout, drv->wext,
			       drv->ctx);

	return ret;
}

static int wpa_driver_madwifi_get_bssid(void *priv, u8 *bssid)
{
	struct wpa_driver_madwifi_data *drv = priv;
	return wpa_driver_wext_get_bssid(drv->wext, bssid);
}


static int wpa_driver_madwifi_get_ssid(void *priv, u8 *ssid)
{
	struct wpa_driver_madwifi_data *drv = priv;
	return wpa_driver_wext_get_ssid(drv->wext, ssid);
}


static int wpa_driver_madwifi_get_scan_results(void *priv,
					    struct wpa_scan_result *results,
					    size_t max_size)
{
	struct wpa_driver_madwifi_data *drv = priv;
	return wpa_driver_wext_get_scan_results(drv->wext, results, max_size);
}


static void * wpa_driver_madwifi_init(void *ctx, const char *ifname)
{
	struct wpa_driver_madwifi_data *drv;

	drv = malloc(sizeof(*drv));
	if (drv == NULL)
		return NULL;
	memset(drv, 0, sizeof(*drv));
	drv->wext = wpa_driver_wext_init(ctx, ifname);
	if (drv->wext == NULL)
		goto fail;

	drv->ctx = ctx;
	strncpy(drv->ifname, ifname, sizeof(drv->ifname));
	drv->sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->sock < 0)
		goto fail2;

	if (set80211param(drv, IEEE80211_PARAM_ROAMING, 2, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to set wpa_supplicant-based "
			   "roaming", __FUNCTION__);
		goto fail3;
	}

	if (set80211param(drv, IEEE80211_PARAM_WPA, 3, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to enable WPA support",
			   __FUNCTION__);
		goto fail3;
	}

	return drv;

fail3:
	close(drv->sock);
fail2:
	wpa_driver_wext_deinit(drv->wext);
fail:
	free(drv);
	return NULL;
}


static void wpa_driver_madwifi_deinit(void *priv)
{
	struct wpa_driver_madwifi_data *drv = priv;

	if (wpa_driver_madwifi_set_wpa_ie(drv, NULL, 0) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to clear WPA IE",
			   __FUNCTION__);
	}
	if (set80211param(drv, IEEE80211_PARAM_ROAMING, 0, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to enable driver-based "
			   "roaming", __FUNCTION__);
	}
	if (set80211param(drv, IEEE80211_PARAM_PRIVACY, 0, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to disable forced Privacy "
			   "flag", __FUNCTION__);
	}
	if (set80211param(drv, IEEE80211_PARAM_WPA, 0, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to disable WPA",
			   __FUNCTION__);
	}

	wpa_driver_wext_deinit(drv->wext);

	close(drv->sock);
	free(drv);
}


const struct wpa_driver_ops wpa_driver_madwifi_ops = {
	.name			= "madwifi",
	.desc			= "MADWIFI 802.11 support (Atheros, etc.)",
	.get_bssid		= wpa_driver_madwifi_get_bssid,
	.get_ssid		= wpa_driver_madwifi_get_ssid,
	.set_key		= wpa_driver_madwifi_set_key,
	.init			= wpa_driver_madwifi_init,
	.deinit			= wpa_driver_madwifi_deinit,
	.set_countermeasures	= wpa_driver_madwifi_set_countermeasures,
	.set_drop_unencrypted	= wpa_driver_madwifi_set_drop_unencrypted,
	.scan			= wpa_driver_madwifi_scan,
	.get_scan_results	= wpa_driver_madwifi_get_scan_results,
	.deauthenticate		= wpa_driver_madwifi_deauthenticate,
	.disassociate		= wpa_driver_madwifi_disassociate,
	.associate		= wpa_driver_madwifi_associate,
	.set_auth_alg		= wpa_driver_madwifi_set_auth_alg,
};
