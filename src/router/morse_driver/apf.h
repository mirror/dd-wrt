#ifndef _MORSE_APF_H_
#define _MORSE_APF_H_

/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <net/mac80211.h>
#include <net/netlink.h>

#include "mac.h"

/** Basic infrastructure mode */
#define WIFI_FEATURE_INFRA              ((uint64_t)0x1)
/** Support for 5 GHz Band */
#define WIFI_FEATURE_INFRA_5G           ((uint64_t)0x2)

/**
 * Packet Filter attributes
 */
enum packet_filter_attr {
	VENDOR_ATTR_PACKET_FILTER_VERSION = 0,
	VENDOR_ATTR_PACKET_FILTER_MAX_LENGTH,
	VENDOR_ATTR_PACKET_FILTER_PROGRAM_OFFSET,
	VENDOR_ATTR_PACKET_FILTER_PROGRAM,
	VENDOR_ATTR_PACKET_FILTER_PROGRAM_LEN,
	VENDOR_ATTR_PACKET_FILTER_MAX
};

/**
 * Reference to the nla_policy defined in apf.c to validate vendor attributes.
 */
extern struct nla_policy morse_apf_nla_policy[VENDOR_ATTR_PACKET_FILTER_MAX];

/**
 * morse_vendor_cmd_get_supported_feature_set() - Get supported feature set
 * @wiphy: Pointer to wiphy structure
 * @wdev: Pointer to wireless device structure
 * @data: Pointer to the data to be passed.
 * @data_len: Length of the data
 *
 * Return: Return 0 on Success or Error code on Failure.
 */
int morse_vendor_cmd_get_supported_feature_set(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len);

/**
 * morse_vendor_cmd_apf_get_capabilities() - Get APF capabilities
 *
 * @wiphy: Pointer to wiphy structure
 * @wdev: Pointer to wireless device structure
 * @data: Pointer to the data to be passed.
 * @data_len: Length of the data
 *
 * Return: Return 0 on Success or Error code on Failure.
 */
int morse_vendor_cmd_apf_get_capabilities(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len);

/**
 * morse_vendor_cmd_apf_set_packet_filter() - Program the packet filter
 *
 * @wiphy: Pointer to wiphy structure
 * @wdev: Pointer to wireless device structure
 * @data: Pointer to the packet filter byte code
 * @data_len: Length of the packet filter byte code
 *
 * Return: Return 0 on Success or Error code on Failure.
 */
int morse_vendor_cmd_apf_set_packet_filter(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len);

/**
 * morse_vendor_cmd_apf_read_packet_filter_data() - Get APF capabilities
 *
 * @wiphy: Pointer to wiphy structure
 * @wdev: Pointer to wireless device structure
 * @data: Pointer to the host memory.
 * @data_len: Length of the data
 *
 * Return: Return 0 on Success or Error code on Failure.
 */
int morse_vendor_cmd_apf_read_packet_filter_data(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len);

#endif /* !_MORSE_APF_H_ */
