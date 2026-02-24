/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "debug.h"
#include "apf.h"
#include "wiphy.h"
#include "bus.h"
#include "ps.h"

#define MORSE_APF_DBG(_m, _f, _a...)		morse_dbg(FEATURE_ID_APF, _m, _f, ##_a)
#define MORSE_APF_INFO(_m, _f, _a...)		morse_info(FEATURE_ID_APF, _m, _f, ##_a)
#define MORSE_APF_WARN(_m, _f, _a...)		morse_warn(FEATURE_ID_APF, _m, _f, ##_a)
#define MORSE_APF_ERR(_m, _f, _a...)		morse_err(FEATURE_ID_APF, _m, _f, ##_a)

struct nla_policy morse_apf_nla_policy[VENDOR_ATTR_PACKET_FILTER_MAX] = {
	[VENDOR_ATTR_PACKET_FILTER_VERSION] = { .type = NLA_U32},
	[VENDOR_ATTR_PACKET_FILTER_MAX_LENGTH] = { .type = NLA_U32},
	[VENDOR_ATTR_PACKET_FILTER_PROGRAM_OFFSET] = { .type = NLA_U32},
	[VENDOR_ATTR_PACKET_FILTER_PROGRAM] = { .type = NLA_BINARY},
	[VENDOR_ATTR_PACKET_FILTER_PROGRAM_LEN] = { .type = NLA_U32},
};

int morse_vendor_cmd_get_supported_feature_set(struct wiphy *wiphy,
				struct wireless_dev *wdev, const void *data, int data_len)
{
	struct morse *mors = morse_wiphy_to_morse(wiphy);
	u32 feature_set;
	u32 skb_len;
	struct sk_buff *skb;
	int ret;

	skb_len = NLMSG_HDRLEN + sizeof(feature_set);
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, skb_len);
	if (!skb) {
		MORSE_APF_ERR(mors, "%s, vendor reply alloc failed\n", __func__);
		return -ENOMEM;
	}

	feature_set = WIFI_FEATURE_INFRA | WIFI_FEATURE_INFRA_5G;
	ret = nla_put_nohdr(skb, sizeof(feature_set), (uint8_t *)&feature_set);
	if (ret < 0) {
		MORSE_APF_ERR(mors, "%s, Failed to put version attr ret=%d\n", __func__, ret);
		kfree_skb(skb);
		return -EINVAL;
	}

	MORSE_APF_INFO(mors, "%s: sending reply, ret=%d\n", __func__, ret);
	return cfg80211_vendor_cmd_reply(skb);
}

int morse_vendor_cmd_apf_get_capabilities(struct wiphy *wiphy,
					struct wireless_dev *wdev, const void *data, int data_len)
{
	struct morse *mors = morse_wiphy_to_morse(wiphy);
	struct sk_buff *skb;
	u32 version = 0;
	u32 max_len = 0;
	u32 skb_len;
	int ret;
	struct morse_vif *mors_vif = morse_wdev_to_morse_vif(wdev);

	skb_len = NLMSG_HDRLEN + sizeof(version) + NLA_HDRLEN + sizeof(max_len) + NLA_HDRLEN;
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, skb_len);
	if (!skb) {
		MORSE_APF_ERR(mors, "%s, vendor reply alloc failed\n", __func__);
		return -ENOMEM;
	}

	ret = morse_cmd_get_apf_capabilities(mors, mors_vif, &version, &max_len);
	if (ret < 0) {
		MORSE_APF_ERR(mors, "%s, Failed to get apf capabilities ret=%d\n", __func__, ret);
		kfree_skb(skb);
		return -EINVAL;
	}
	mors_vif->apf.enabled = true;
	mors_vif->apf.max_length = max_len;

	ret = nla_put_u32(skb, VENDOR_ATTR_PACKET_FILTER_VERSION, version);
	if (ret < 0) {
		MORSE_APF_ERR(mors, "%s, Failed to put version attr ret=%d\n", __func__, ret);
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = nla_put_u32(skb, VENDOR_ATTR_PACKET_FILTER_MAX_LENGTH, max_len);
	if (ret < 0) {
		MORSE_APF_ERR(mors, "%s, Failed to put max length attr ret=%d\n", __func__, ret);
		kfree_skb(skb);
		return -EINVAL;
	}

	MORSE_APF_INFO(mors, "%s: sending reply\n", __func__);
	return cfg80211_vendor_cmd_reply(skb);
}

int morse_vendor_cmd_apf_set_packet_filter(struct wiphy *wiphy,
			struct wireless_dev *wdev, const void *data, int data_len)
{
	struct morse *mors = morse_wiphy_to_morse(wiphy);
	int ret = 0;
	struct nlattr *vendor_attr[VENDOR_ATTR_PACKET_FILTER_MAX + 1];
	u16 program_len;
	struct morse_vif *mors_vif = morse_wdev_to_morse_vif(wdev);

	if (!data || !mors_vif->apf.enabled) {
		MORSE_APF_ERR(mors, "%s Null data or APF not enabled, apf enabled=%d\n", __func__,
					  mors_vif->apf.enabled);
		return -EINVAL;
	}

	nla_parse(vendor_attr, VENDOR_ATTR_PACKET_FILTER_MAX, data, data_len,
			  morse_apf_nla_policy, NULL);
	program_len = nla_get_u32(vendor_attr[VENDOR_ATTR_PACKET_FILTER_PROGRAM_LEN]);
	MORSE_APF_INFO(mors, "%s: Program_len=%d, apf enabled=%d\n", __func__, program_len,
				   mors_vif->apf.enabled);

	if (!program_len || program_len > mors_vif->apf.max_length) {
		MORSE_APF_ERR(mors, "%s Invalid program length=%d, apf max_length=%d\n", __func__,
					  program_len, mors_vif->apf.max_length);
		return -EINVAL;
	}

	if (!vendor_attr[VENDOR_ATTR_PACKET_FILTER_PROGRAM]) {
		MORSE_APF_ERR(mors, "%s no filter data", __func__);
		ret = -EINVAL;
	} else {
		u8 *program;

		program = nla_data(vendor_attr[VENDOR_ATTR_PACKET_FILTER_PROGRAM]);
		if (!program) {
			MORSE_APF_ERR(mors, "%s, NULL program\n", __func__);
			return -EINVAL;
		}

		ret = morse_cmd_read_write_apf(mors, mors_vif, true, program_len, program, 0);
		if (ret < 0)
			MORSE_APF_ERR(mors, "%s: Failed to update APF memory\n", __func__);
	}
	return ret;
}

int morse_vendor_cmd_apf_read_packet_filter_data(struct wiphy *wiphy,
					struct wireless_dev *wdev, const void *data, int data_len)
{
	struct morse *mors = morse_wiphy_to_morse(wiphy);
	u32 skb_len;
	u16 read_len;
	u32 offset;
	struct nlattr *vendor_attr[VENDOR_ATTR_PACKET_FILTER_MAX + 1];
	struct sk_buff *skb;
	int ret = 0;
	u8 *program;
	struct morse_vif *mors_vif = morse_wdev_to_morse_vif(wdev);

	if (!data_len || !mors_vif->apf.enabled) {
		MORSE_APF_ERR(mors, "%s data len zero or APF not enabled, apf enabled=%d\n",
					  __func__, mors_vif->apf.enabled);
		return -EINVAL;
	}

	nla_parse(vendor_attr, VENDOR_ATTR_PACKET_FILTER_MAX, data, data_len, NULL, NULL);

	read_len = nla_get_u32(vendor_attr[VENDOR_ATTR_PACKET_FILTER_PROGRAM_LEN]);
	offset = nla_get_u32(vendor_attr[VENDOR_ATTR_PACKET_FILTER_PROGRAM_OFFSET]);
	MORSE_APF_INFO(mors, "%s: offset=%d, read_len=%d, apf.enabled=%d", __func__, offset,
				   read_len, mors_vif->apf.enabled);

	if (!read_len || read_len > mors_vif->apf.max_length ||
		(offset + read_len) > mors_vif->apf.max_length) {
		MORSE_APF_ERR(mors, "%s, Invalid read_len=%d or offset=%d, apf.max_length=%d\n",
					  __func__, read_len, offset, mors_vif->apf.max_length);
		return -EINVAL;
	}

	program = kzalloc(read_len, GFP_KERNEL);
	if (!program) {
		MORSE_APF_ERR(mors, "%s, program alloc failed\n", __func__);
		return -EINVAL;
	}

	ret = morse_cmd_read_write_apf(mors, mors_vif, false, read_len, program, offset);
	if (ret < 0) {
		ret = -EINVAL;
		goto exit;
	}

	skb_len = NLMSG_HDRLEN + read_len + NLA_HDRLEN;
	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, skb_len);
	if (!skb) {
		ret = -ENOMEM;
		goto exit;
	}

	ret = nla_put(skb, VENDOR_ATTR_PACKET_FILTER_PROGRAM, read_len, program);
	if (ret < 0) {
		MORSE_APF_ERR(mors, "%s, Failed to put data attr ret=%d\n", __func__, ret);
		kfree_skb(skb);
		ret = -EINVAL;
		goto exit;
	}

	MORSE_APF_INFO(mors, "%s: sending reply, ret=%d\n", __func__, ret);
	ret = cfg80211_vendor_cmd_reply(skb);

exit:
	kfree(program);
	return ret;
}
