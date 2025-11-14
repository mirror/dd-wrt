/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/**
 * @file ecm_classifier_wifi_public.h
 *	ECM Wi-Fi classifier subsystem.
 */

#ifndef __ECM_CLASSIFIER_WIFI_PUBLIC_H__
#define __ECM_CLASSIFIER_WIFI_PUBLIC_H__

#define ECM_CLASSIFIER_WIFI_MLO_PARAM_VALID		0x1

/**
 * ecm_classifier_wifi_metadata_info
 *
 * @dest_dev: Destination netdevice
 * @dest_mac: Destination peer mac address
 * @out_ppe_ds_node_id: destination node id for ppe-ds
 */
struct ecm_classifier_wifi_metadata_info {
	struct net_device *dest_dev;
	uint8_t *dest_mac;
	uint8_t out_ppe_ds_node_id;
};

/**
 * ecm_classifier_wifi_metadata
 *
 * @valid_params_flag: Flags indicating valid params
 * @wifi_mdata: Contains wifi metadata info
 */
struct ecm_classifier_wifi_metadata {
	uint32_t valid_params_flag;
	struct ecm_classifier_wifi_metadata_info wifi_mdata;
};

/**
 * Callback to which WLAN driver will register to get wifi metadata from ECM frontend.
 */
typedef uint32_t (*ecm_classifier_wifi_get_info_callback_t)(struct ecm_classifier_wifi_metadata *wifi_info);

/**
 * Data structure for WiFi callbacks.
 */
struct ecm_classifier_wifi_callbacks {
	ecm_classifier_wifi_get_info_callback_t get_wifi_metadata;
};

/**
 * Registers Wi-Fi callbacks.
 *
 * @param 	wifi_cb 	Wi-Fi callback pointer.
 *
 * @return
 * 0 if success, error value if fails.
 */
int ecm_classifier_wifi_callback_register(struct ecm_classifier_wifi_callbacks *wifi_cb);

/**
 * Unregisters Wifi callbacks.
 *
 * @return
 * None.
 */
void ecm_classifier_wifi_callback_unregister(void);

#endif /* __ECM_CLASSIFIER_WIFI_PUBLIC_H__ */
