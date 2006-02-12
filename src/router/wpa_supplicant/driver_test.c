/*
 * WPA Supplicant - testing driver interface
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
#include <string.h>

#include "common.h"
#include "driver.h"
#include "wpa_supplicant.h"
#include "l2_packet.h"
#include "eloop.h"


struct wpa_driver_test_data {
	void *ctx;
};


static int wpa_driver_test_set_wpa(void *priv, int enabled)
{
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __func__, enabled);
	return 0;
}


static void wpa_driver_test_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	wpa_printf(MSG_DEBUG, "Scan timeout - try to get results");
	wpa_supplicant_event(timeout_ctx, EVENT_SCAN_RESULTS, NULL);
}


static int wpa_driver_test_scan(void *priv, const u8 *ssid, size_t ssid_len)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s: priv=%p", __func__, priv);
	eloop_register_timeout(1, 0, wpa_driver_test_scan_timeout, drv,
			       drv->ctx);
	return 0;
}


static int wpa_driver_test_get_scan_results(void *priv,
					    struct wpa_scan_result *results,
					    size_t max_size)
{
	return 0;
}


static int wpa_driver_test_set_key(void *priv, wpa_alg alg, const u8 *addr,
				   int key_idx, int set_tx,
				   const u8 *seq, size_t seq_len,
				   const u8 *key, size_t key_len)
{
	wpa_printf(MSG_DEBUG, "%s: priv=%p alg=%d key_idx=%d set_tx=%d",
		   __func__, priv, alg, key_idx, set_tx);
	if (addr) {
		wpa_printf(MSG_DEBUG, "   addr=" MACSTR, MAC2STR(addr));
	}
	if (seq) {
		wpa_hexdump(MSG_DEBUG, "   seq", seq, seq_len);
	}
	if (key) {
		wpa_hexdump(MSG_DEBUG, "   key", key, key_len);
	}
	return 0;
}


static int wpa_driver_test_associate(
	void *priv, struct wpa_driver_associate_params *params)
{
	wpa_printf(MSG_DEBUG, "%s: priv=%p freq=%d pairwise_suite=%d "
		   "group_suite=%d key_mgmt_suite=%d auth_alg=%d mode=%d",
		   __func__, priv, params->freq, params->pairwise_suite,
		   params->group_suite, params->key_mgmt_suite,
		   params->auth_alg, params->mode);
	if (params->bssid) {
		wpa_printf(MSG_DEBUG, "   bssid=" MACSTR,
			   MAC2STR(params->bssid));
	}
	if (params->ssid) {
		wpa_hexdump_ascii(MSG_DEBUG, "   ssid",
				  params->ssid, params->ssid_len);
	}
	if (params->wpa_ie) {
		wpa_hexdump(MSG_DEBUG, "   wpa_ie",
			    params->wpa_ie, params->wpa_ie_len);
	}

	return 0;
}


static void * wpa_driver_test_init(void *ctx, const char *ifname)
{
	struct wpa_driver_test_data *drv;

	drv = malloc(sizeof(*drv));
	if (drv == NULL)
		return NULL;
	memset(drv, 0, sizeof(*drv));
	drv->ctx = ctx;

	return drv;
}


static void wpa_driver_test_deinit(void *priv)
{
	struct wpa_driver_test_data *drv = priv;
	free(drv);
}


struct wpa_driver_ops wpa_driver_test_ops = {
	.name = "test",
	.desc = "wpa_supplicant test driver",
	.init = wpa_driver_test_init,
	.deinit = wpa_driver_test_deinit,
	.set_wpa = wpa_driver_test_set_wpa,
	.scan = wpa_driver_test_scan,
	.get_scan_results = wpa_driver_test_get_scan_results,
	.set_key = wpa_driver_test_set_key,
	.associate = wpa_driver_test_associate,
};
