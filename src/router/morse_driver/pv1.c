/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <net/mac80211.h>
#include <linux/crc32.h>
#include <linux/ieee80211.h>
#include <linux/bitfield.h>

#include "morse.h"
#include "mac.h"
#include "pv1.h"
#include "debug.h"

/**
 * morse_pv1_retrieve_tx_bpn - Retrieve the PV1 Tx BPN for QoS Data & MGMT frames per TID
 *
 * @mors_vif:  Morse VIF
 * @sta:      Peer STA
 * @seq_num:  Sequence number of Tx frame
 * @tid:      TID of the Tx frame
 * @is_mgmt:  Flag to notify if management frame or data frame
 *
 * Return: BPN on success, else relevant error code
 */
static int32_t morse_pv1_retrieve_tx_bpn(struct morse_vif *mors_vif,
			struct ieee80211_sta *sta, u16 seq_num, u8 tid, bool is_mgmt)
{
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;
	struct morse_sta_pv1 *pv1_ctx = &mors_sta->tx_pv1_ctx;
	u8 ptid;

	if (!mors_vif->enable_pv1 || !mors_sta->pv1_frame_support)
		return -EPERM;

	BUILD_BUG_ON(ARRAY_SIZE(pv1_ctx->bpn) < (IEEE80211_NUM_PTIDS));
	BUILD_BUG_ON(ARRAY_SIZE(pv1_ctx->last_seq_num) < (IEEE80211_NUM_PTIDS));
	/* MGMT BPN stored in the last entry of the array */
	ptid = (is_mgmt) ? IEEE80211_NUM_PTIDS : TID_TO_PTID(tid);

	if (seq_num < pv1_ctx->last_seq_num[ptid])
		pv1_ctx->bpn[ptid]++;

	pv1_ctx->last_seq_num[ptid] = seq_num;

	return pv1_ctx->bpn[ptid];
}

/**
 * morse_mac_process_hc_request - Process Header Compression Request frame and
 *                  schedules response
 *
 * @mors_vif:     Virtual interface
 * @ie_data:      Header Compression IE data
 * @sta:          Peer STA from which the request was received
 * @dialog_token: Dialog token from request action frame
 *
 * @note:         The RCU lock must be held when calling this function.
 */
static void morse_mac_process_hc_request(struct morse *mors, struct morse_vif *mors_vif,
			struct dot11ah_pv1_header_compression *ie_data, struct ieee80211_sta *sta,
			u8 dialog_token)
{
	u8 header_compression_ctrl = ie_data->header_compression_control;
	bool store_a3 = (header_compression_ctrl & DOT11AH_PV1_HEADER_COMPRESSION_STORE_A3);
	bool store_a4 = (header_compression_ctrl & DOT11AH_PV1_HEADER_COMPRESSION_STORE_A4);
	u8 *ptr = (uint8_t *)ie_data->variable;
	struct morse_pv1_hc_request *rx_request;

	mutex_lock(&mors_vif->pv1.lock);

	rx_request = &mors_vif->pv1.rx_request;
	mors_vif->pv1.fw_stored_response_status = false;
	rx_request->a1_a3_differ = store_a3;
	rx_request->a2_a4_differ = store_a4;
	rx_request->action_dialog_token = dialog_token;
	memcpy(mors_vif->pv1.rx_pv1_sta_addr, sta->addr, ETH_ALEN);

	if (store_a3) {
		memcpy(rx_request->header_compression_a3, ptr,
		       sizeof(rx_request->header_compression_a3));
		ptr += sizeof(rx_request->header_compression_a3);
	}

	if (store_a4) {
		memcpy(rx_request->header_compression_a4, ptr,
		       sizeof(rx_request->header_compression_a4));
	}

	schedule_work(&mors_vif->pv1.hc_req_work);

	mutex_unlock(&mors_vif->pv1.lock);
}

/**
 * morse_mac_process_hc_response - Process Header Compression Response frame on RX
 *
 * @mgmt:        Received response frame
 * @mors_vif:    Virtual interface
 * @hc_ie_data:  Response IE data
 * @sta:         Peer STA from which the response was received
 *
 * @note:        The RCU lock must be held when calling this function.
 */
static void morse_mac_process_hc_response(struct morse_dot11ah_s1g_action *mgmt,
		struct ieee80211_vif *vif, struct dot11ah_pv1_header_compression *hc_ie_data,
		struct ieee80211_sta *sta)
{
	struct morse_sta *mors_sta;
	struct morse_sta_pv1 *resp_status;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);

	mors_sta = (struct morse_sta *)sta->drv_priv;

	if (!mors_sta || !mors_sta->pv1_frame_support)
		return;

	resp_status = &mors_sta->tx_pv1_ctx;
	if (!resp_status)
		return;

	resp_status->a3_stored = hc_ie_data->header_compression_control &
				DOT11AH_PV1_HEADER_COMPRESSION_STORE_A3;
	resp_status->a4_stored = hc_ie_data->header_compression_control &
				DOT11AH_PV1_HEADER_COMPRESSION_STORE_A4;

	mutex_lock(&mors_vif->pv1.lock);

	if (mors_vif->pv1.tx_request.action_in_progress) {
		mors_vif->pv1.tx_request.action_in_progress = false;
		mors_vif->pv1.hc_response_timeout = jiffies;

		if (resp_status->a3_stored)
			memcpy(resp_status->stored_a3, mors_vif->pv1.tx_request.stored_a3,
			       sizeof(resp_status->stored_a3));
		else
			memset(resp_status->stored_a3, 0, sizeof(resp_status->stored_a3));

		if (resp_status->a4_stored)
			memcpy(resp_status->stored_a4, mors_vif->pv1.tx_request.stored_a4,
			       sizeof(resp_status->stored_a4));
		else
			memset(resp_status->stored_a4, 0, sizeof(resp_status->stored_a4));
	}

	memcpy(mors_vif->pv1.tx_pv1_sta_addr, sta->addr, ETH_ALEN);

	if (resp_status->a3_stored || resp_status->a4_stored)
		schedule_work(&mors_vif->pv1.hc_resp_work);

	mutex_unlock(&mors_vif->pv1.lock);
}

void morse_mac_process_pv1_action_frame(struct morse_dot11ah_s1g_action *mgmt,
					struct morse *mors, struct ieee80211_vif *vif)
{
	struct ieee80211_sta *sta;
	struct morse_sta *mors_sta;
	u8 *hc_ie = (uint8_t *)mgmt->u.pv1_action.variable;
	u8 dialog_token = mgmt->u.pv1_action.dialog_token;
	/* Point to the Header Compression IE data excluding 2 bytes of IE
	 * header (IE id + IE length)
	 */
	struct dot11ah_pv1_header_compression *hc_ie_data =
		(struct dot11ah_pv1_header_compression *)hc_ie + 2;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	u8 header_compression_ctrl = hc_ie_data->header_compression_control;
	bool is_response =
		(header_compression_ctrl & DOT11AH_PV1_HEADER_COMPRESSION_REQ_RESPONSE);

	if (!mors_vif->enable_pv1)
		return;

	if (mgmt->u.pv1_action.action_code != WLAN_S1G_HEADER_COMPRESSION)
		return;

	/* Must be held while finding and dereferencing sta */
	rcu_read_lock();

	sta = ieee80211_find_sta(vif, mgmt->sa);
	if (!sta)
		goto exit;

	mors_sta = (struct morse_sta *)sta->drv_priv;
	if (!mors_sta || !mors_sta->pv1_frame_support)
		goto exit;

	if (is_response)
		morse_mac_process_hc_response(mgmt, vif, hc_ie_data, sta);
	else
		morse_mac_process_hc_request(mors, mors_vif, hc_ie_data, sta, dialog_token);
exit:
	rcu_read_unlock();
}

void morse_pv1_a3_a4_check(struct morse_vif *mors_vif, struct ieee80211_sta *pubsta,
					      struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct morse_pv1_hc_request *tx_request = &mors_vif->pv1.tx_request;

	tx_request->a4_included = ieee80211_has_a4(hdr->frame_control);
	tx_request->a1_a3_differ = false;
	tx_request->a2_a4_differ = false;

	if (is_broadcast_ether_addr(hdr->addr3) ||
	    is_multicast_ether_addr(hdr->addr3)) {
		tx_request->a1_a3_differ = true;
		return;
	}

	if (memcmp(hdr->addr1, hdr->addr3, ETH_ALEN)) {
		tx_request->a1_a3_differ = true;
		memcpy(tx_request->header_compression_a3, hdr->addr3,
		       sizeof(tx_request->header_compression_a3));
	}

	if (tx_request->a4_included && memcmp(hdr->addr2, hdr->addr4, ETH_ALEN)) {
		tx_request->a2_a4_differ = true;
		memcpy(tx_request->header_compression_a4, hdr->addr4,
		       sizeof(tx_request->header_compression_a4));
	}
}

void morse_mac_send_pv1_hc_action_frame(struct morse *mors,
			struct ieee80211_vif *vif, struct ieee80211_sta *sta,
			struct sk_buff *skb_data, bool is_response)
{
	struct sk_buff *skb;
	struct morse_dot11ah_s1g_action *action;
	struct morse_skb_tx_info tx_info = { 0 };
	struct morse_vif *mors_vif;
	struct morse_sta *mors_sta;
	struct morse_skbq *mq;
	int ie_len;
	struct dot11ah_ies_mask *ies_mask = NULL;
	int ret;
	struct morse_sta_pv1 *sta_resp_status;
	struct morse_pv1_hc_request *tx;
	struct morse_pv1_hc_request *rx;
	struct ieee80211_hdr *hdr;
	u32 timeout;

	int frame_len = sizeof(*action);

	hdr = skb_data ? (struct ieee80211_hdr *)skb_data->data : NULL;

	if (!is_response && hdr &&
	    (is_broadcast_ether_addr(hdr->addr3) ||
		 is_multicast_ether_addr(hdr->addr3)))
		return;

	if (!sta)
		return;

	mors_sta = (struct morse_sta *)sta->drv_priv;
	mors_vif = ieee80211_vif_to_morse_vif(vif);

	tx = &mors_vif->pv1.tx_request;
	rx = &mors_vif->pv1.rx_request;
	sta_resp_status = &mors_sta->tx_pv1_ctx;
	timeout = jiffies - mors_vif->pv1.hc_response_timeout;

	/* The action frames are sent based on:
	 * If Response - always send as the requestor is waiting for response
	 * If Request
	 * 1. Previous action is in progress and the response wait period has timed out
	 * 2. Both A3 and A4 does not differ and previously stored A3 and A3 has some address
	 * 3. A3 differ and stored address does not match with the current A3 in data
	 * 4. A4 differ and stored address does not match with the current A4 in data
	 */
	if (!is_response) {
		if (tx->action_in_progress && timeout < HC_RESPONSE_TIMEOUT)
			return;

		if ((!tx->a1_a3_differ && !tx->a2_a4_differ) &&
		    (!sta_resp_status->a3_stored && !sta_resp_status->a4_stored))
			return;

		if (tx->a1_a3_differ &&
		    sta_resp_status->a3_stored &&
			!memcmp(tx->header_compression_a3, sta_resp_status->stored_a3, ETH_ALEN))
			return;

		if (tx->a2_a4_differ &&
		    sta_resp_status->a4_stored &&
			!memcmp(tx->header_compression_a4, sta_resp_status->stored_a4, ETH_ALEN))
			return;
	}

	ies_mask = morse_dot11ah_ies_mask_alloc();
	if (!ies_mask)
		return;

	ie_len = morse_dot11ah_insert_pv1_hc_ie(vif, ies_mask, is_response);
	mq = mors->cfg->ops->skbq_mgmt_tc_q(mors);

	skb = morse_skbq_alloc_skb(mq, frame_len + ie_len);
	if (!skb)
		goto end;

	action = (struct morse_dot11ah_s1g_action *)skb->data;
	memset(action, 0, sizeof(*action));

	if (sta && sta->mfp) {
		if (is_sw_crypto_mode()) {
			MORSE_ERR_RATELIMITED(mors,
			      "Can't send protected action frame with software encryption\n");
			morse_mac_skb_free(mors, skb);
			goto end;
		}
		action->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
						    IEEE80211_STYPE_ACTION |
						    IEEE80211_FCTL_PROTECTED);
	} else {
		action->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
	}
	action->category = WLAN_CATEGORY_S1G_PROTECTED;
	memcpy(action->da, mors_sta->addr, ETH_ALEN);
	memcpy(action->sa, vif->addr, ETH_ALEN);
	memcpy(action->bssid, vif->bss_conf.bssid, ETH_ALEN);
	action->u.pv1_action.action_code = WLAN_S1G_HEADER_COMPRESSION;
	action->u.pv1_action.dialog_token = is_response ?
			rx->action_dialog_token : ++tx->action_dialog_token;

	if (ies_mask->ies[WLAN_EID_HEADER_COMPRESSION].ptr) {
		morse_dot11_insert_ie(action->u.pv1_action.variable,
				ies_mask->ies[WLAN_EID_HEADER_COMPRESSION].ptr,
				WLAN_EID_HEADER_COMPRESSION,
				ies_mask->ies[WLAN_EID_HEADER_COMPRESSION].len);
	}

	IEEE80211_SKB_CB(skb)->control.vif = vif;
	morse_mac_fill_tx_info(mors, &tx_info, skb, vif,
		mors->custom_configs.channel_info.op_bw_mhz, sta);

	ret = morse_skbq_skb_tx(mq, &skb, &tx_info, MORSE_SKB_CHAN_MGMT);
	if (ret) {
		MORSE_ERR(mors, "%s failed\n", __func__);
		morse_mac_skb_free(mors, skb);
	}
	tx->action_in_progress = true;
	mors_vif->pv1.hc_response_timeout = jiffies;

end:
	morse_dot11ah_ies_mask_free(ies_mask);
}

/**
 * morse_prepare_pv1_frame_ctrl  - Prepare PV1 frame control for PV1 data
 *
 * @mors_vif:     Valid AP/STA VIF
 * @morse_sta:   A pointer to STA object
 * @hdr:         PV0 MAC Header
 */
static u16 morse_prepare_pv1_frame_ctrl(struct morse_vif *mors_vif,
			struct morse_sta *mors_sta, struct ieee80211_hdr *hdr)
{
	struct morse_sta_pv1 *pv1_sta = &mors_sta->tx_pv1_ctx;
	struct morse_pv1_hc_request *tx_request = &mors_vif->pv1.tx_request;
	u8 *qos_ctrl = ieee80211_get_qos_ctl(hdr);
	u16 pv1_fc = DOT11_PV1_PROTOCOL_VERSION & IEEE80211_PV1_FCTL_VERS;
	u16 tid = qos_ctrl[0] & IEEE80211_QOS_CTL_TID_MASK;

	pv1_fc |= (tid << DOT11_MAC_PV1_STYPE_OFFSET) & IEEE80211_PV1_FCTL_STYPE;

	if (pv1_sta->a3_stored || pv1_sta->a4_stored ||
	    tx_request->a1_a3_differ || tx_request->a2_a4_differ)
		pv1_fc |= IEEE80211_PV1_FCTL_FTYPE & DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA_SID;
	else
		pv1_fc |= IEEE80211_PV1_FCTL_FTYPE & DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA;

	if (ieee80211_has_fromds(hdr->frame_control))
		pv1_fc |= (IEEE80211_PV1_FCTL_FROMDS);

	if (ieee80211_has_morefrags(hdr->frame_control))
		pv1_fc |= IEEE80211_PV1_FCTL_MOREFRAGS;

	if (ieee80211_has_pm(hdr->frame_control))
		pv1_fc |= IEEE80211_PV1_FCTL_PM;

	if (ieee80211_has_moredata(hdr->frame_control))
		pv1_fc |= IEEE80211_PV1_FCTL_MOREDATA;

	if (ieee80211_has_protected(hdr->frame_control))
		pv1_fc |= IEEE80211_PV1_FCTL_PROTECTED;

	if (*qos_ctrl & IEEE80211_QOS_CTL_EOSP)
		pv1_fc |= IEEE80211_PV1_FCTL_END_SP;

	return pv1_fc;
}

/**
 * morse_prepare_pv1_sid_header - Prepare PV1 MAC header with SID (QoS Data Type 0)
 *
 * @vif:     Valid AP or STA VIF
 * @sta:     Pointer to peer STA context
 * @pv1_hdr: Pointer to PV1 header to be replaced with PV0 header at TX
 * @pv0_hdr: Pointer to PV0 header to fill details like address, sequence control
 *           to PV1 header
 * @fc:      PV1 frame control to be filled in PV1 mac header
 *
 * @retun: PV1 SID Header length
 */
static int morse_prepare_pv1_sid_header(struct ieee80211_vif *vif,
		struct ieee80211_sta *sta, struct dot11ah_mac_pv1_hdr *pv1_hdr,
		struct ieee80211_hdr *pv0_hdr, u16 fc)
{
	struct dot11ah_mac_pv1_qos_data_sid_hdr *sid_header =
				(struct dot11ah_mac_pv1_qos_data_sid_hdr *)pv1_hdr;
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;
	struct morse_sta_pv1 *pv1_sta = &mors_sta->tx_pv1_ctx;
	u8 *tmp = sid_header->variable;
	int pv1_header_length = sizeof(*sid_header);
	__le16 sid = 0;

	if (vif->type == NL80211_IFTYPE_STATION)
		sid = cpu_to_le16(morse_mac_sta_aid(vif) & DOT11_MAC_PV1_SID_AID_MASK);
	else if (vif->type == NL80211_IFTYPE_AP)
		sid = cpu_to_le16(sta->aid & DOT11_MAC_PV1_SID_AID_MASK);

	/* Add A3 only if A3 is not stored or the stored address
	 * does not match with latest A3
	 */
	if (!pv1_sta->a3_stored ||
	    memcmp(pv1_sta->stored_a3, pv0_hdr->addr3, ETH_ALEN)) {
		sid |= cpu_to_le16(DOT11_MAC_PV1_SID_A3_PRESENT);
		memcpy(tmp, pv0_hdr->addr3, ETH_ALEN);
		tmp += ETH_ALEN;
	}

	/* Add A4 only if TX frame has A4 included and is not stored
	 * or stored address does not match with latest A4
	 */
	if ((ieee80211_has_a4(pv0_hdr->frame_control) &&
	     (!pv1_sta->a4_stored ||
			memcmp(pv1_sta->stored_a4, pv0_hdr->addr4, ETH_ALEN)))) {
		sid |= cpu_to_le16(DOT11_MAC_PV1_SID_A4_PRESENT);
		memcpy(tmp, pv0_hdr->addr4, ETH_ALEN);
		tmp += ETH_ALEN;
	}

	if (fc & IEEE80211_PV1_FCTL_FROMDS) {
		sid_header->u.from_ds.addr1_sid = sid;
		memcpy(sid_header->u.from_ds.addr2, pv0_hdr->addr2, ETH_ALEN);
	} else {
		memcpy(sid_header->u.to_ds.addr1, pv0_hdr->addr1, ETH_ALEN);
		sid_header->u.to_ds.addr2_sid = sid;
	}

	sid_header->sequence_ctrl = pv0_hdr->seq_ctrl;
	pv1_header_length += (tmp - sid_header->variable);

	return pv1_header_length;
}

/**
 * morse_prepare_pv1_qos_header - Prepare PV1 MAC header where both A1 and A2 has address
 *                                (QoS Data Type 3)
 *
 *@pv1_hdr:   Pointer to PV1 header to be replaced with PV0 header in TX
 *@pv0_hdr:   Pointer to PV0 header to fill details like A1, A2 to PV1 header
 *
 * @return:  PV1 QoS Data header length
 */
static int morse_prepare_pv1_qos_header(struct dot11ah_mac_pv1_hdr *pv1_hdr,
		struct ieee80211_hdr *pv0_hdr)
{
	struct dot11ah_mac_pv1_qos_data_hdr *qos_hdr =
			(struct dot11ah_mac_pv1_qos_data_hdr *)pv1_hdr;

	memcpy(qos_hdr->addr1, pv0_hdr->addr1, ETH_ALEN);
	memcpy(qos_hdr->addr2, pv0_hdr->addr2, ETH_ALEN);
	qos_hdr->sequence_ctrl = pv0_hdr->seq_ctrl;

	return sizeof(*qos_hdr);
}

/**
 * morse_prepare_pv1_mac_header - Function to derive PV1 Header. Derives type of QoS
 *                     from frame control and passes control to the function that prepare
 *                     mac address for specific to QoS type
 *
 * @vif:     Valid AP/STA VIF
 * @sta:     Pointer to peer STA context
 * @hdr:     Pointer to PV1 MAC header
 * @pv0_hdr: Pointer to PV0 MAC header
 * @fc:      PV1 Frame control
 *
 * @return:  PV1 header size
 */
static int morse_prepare_pv1_mac_header(struct ieee80211_vif *vif,
		struct ieee80211_sta *sta, struct dot11ah_mac_pv1_hdr *pv1_hdr,
		struct ieee80211_hdr *pv0_hdr, u16 fc)
{
	pv1_hdr->frame_ctrl = cpu_to_le16(fc);

	switch (fc & IEEE80211_PV1_FCTL_FTYPE) {
	case DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA_SID:
		return morse_prepare_pv1_sid_header(vif, sta, pv1_hdr, pv0_hdr, fc);
	case DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA:
		return morse_prepare_pv1_qos_header(pv1_hdr, pv0_hdr);
	default:
		break;
	}

	return sizeof(*pv1_hdr);
}

/**
 * morse_convert_pv0_to_pv1 - Convert PV0 to PV1 frame
 *
 * @mors:    The global Morse structure
 * @mors_vif: Morse vif structure
 * @sta:     Peer STA to which frame is being transmitted
 * @skb:     Tx data SKB
 *
 * @return: 0 on success, error on failure
 */
static int morse_convert_pv0_to_pv1(struct morse *mors, struct morse_vif *mors_vif,
		struct ieee80211_sta *sta, struct sk_buff *skb)
{
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(skb)->data;
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	u16 pv0_fc = le16_to_cpu(hdr->frame_control);
	u8 tid = skb->priority & IEEE80211_QOS_CTL_TAG1D_MASK;
	u16 seq_num = IEEE80211_SEQ_TO_SN(le16_to_cpu(hdr->seq_ctrl));
	bool is_mgmt = ieee80211_is_mgmt(hdr->frame_control);
	u8 pv1_header_buf[DOT11_PV1_MAC_HEADER_SIZE_MAX] = {0};
	struct dot11ah_mac_pv1_hdr *pv1_mac_header =
			(struct dot11ah_mac_pv1_hdr *)pv1_header_buf;
	bool is_protected = pv0_fc & IEEE80211_FCTL_PROTECTED;
	int bpn = 0;
	u16 pv1_fc;
	int pv1_header_length;
	int pv0_hdr_len;
	int headroom_required;

	pv0_hdr_len = (ieee80211_get_qos_ctl(hdr) - (u8 *)hdr) + IEEE80211_QOS_CTL_LEN;
	pv1_fc = morse_prepare_pv1_frame_ctrl(mors_vif, mors_sta, hdr);
	pv1_header_length = morse_prepare_pv1_mac_header(vif, sta, pv1_mac_header, hdr, pv1_fc);
	headroom_required = (skb->len - pv0_hdr_len) + pv1_header_length;

	if (is_protected) {
		/* Calculate BPN to insert in PV1 frame body */
		bpn = morse_pv1_retrieve_tx_bpn(mors_vif, sta, seq_num, tid, is_mgmt);
		if (bpn < 0) {
			MORSE_ERR_RATELIMITED(mors,
					      "%s: Failed to retrieve BPN for PV1 frame\n",
						  __func__);
			return -EINVAL;
		}
		headroom_required += BPN_LEN;
	}

	if (headroom_required > skb->len + skb_headroom(skb)) {
		MORSE_ERR_RATELIMITED(mors,
					    "%s: TX SKB not does not have sufficient headroom allocated, has %d, expected %d",
						__func__,
						skb->len + skb_headroom(skb),
						headroom_required);
		return -EFAULT;
	}

	/* Purge PV0 header, now SKB points to data without header */
	skb_pull(skb, pv0_hdr_len);

	/* Add BPN at the starting of Data */
	if (is_protected)
		memcpy(skb_push(skb, BPN_LEN), &bpn, BPN_LEN);

	/* Push the PV1 header and there we have PV1 data frame */
	memcpy(skb_push(skb, pv1_header_length), pv1_mac_header, pv1_header_length);
	return 0;
}

/**
 * morse_prepare_pv0_frame_ctrl -  Prepare PV0 frame control for PV0 data
 *
 * @pv1_fc:  PV1 frame control from incoming SKB
 *
 * @return:  PV0 frame control
 */
static u16 morse_prepare_pv0_frame_ctrl(u16 pv1_fc)
{
	u16 pv0_fc = IEEE80211_FTYPE_DATA | IEEE80211_STYPE_QOS_DATA;

	if (pv1_fc & IEEE80211_PV1_FCTL_FROMDS)
		pv0_fc |= IEEE80211_FCTL_FROMDS;
	else
		pv0_fc |= IEEE80211_FCTL_TODS;

	if (pv1_fc & IEEE80211_PV1_FCTL_MOREFRAGS)
		pv0_fc |= IEEE80211_FCTL_MOREFRAGS;

	if (pv1_fc & IEEE80211_PV1_FCTL_PM)
		pv0_fc |= IEEE80211_FCTL_PM;

	if (pv1_fc & IEEE80211_PV1_FCTL_MOREDATA)
		pv0_fc |= IEEE80211_FCTL_MOREDATA;

	if (pv1_fc & IEEE80211_PV1_FCTL_PROTECTED)
		pv0_fc |= IEEE80211_FCTL_PROTECTED;

	return pv0_fc;
}

/**
 * morse_convert_pv1_to_pv0_qos_ctrl - Prepare QoS Control filed in PV0 header
 *
 * @qos:  Pointer to QoS control
 * @fc:   PV1 Frame control
 */
static void morse_convert_pv1_to_pv0_qos_ctrl(u8 *qos, u16 fc)
{
	u16 tid = (fc & IEEE80211_PV1_FCTL_STYPE) >> DOT11_MAC_PV1_STYPE_OFFSET;

	qos[0] = tid & IEEE80211_QOS_CTL_TID_MASK;

	if (fc & IEEE80211_PV1_FCTL_END_SP)
		qos[0] |= IEEE80211_QOS_CTL_EOSP;

	if (fc & IEEE80211_PV1_FCTL_ACK_POLICY)
		qos[0] |= IEEE80211_QOS_CTL_ACK_POLICY_NOACK;
}

struct pv1_find_sta_aid_iter {
	const struct ieee80211_vif *on_vif;
	u8 aid;
	struct ieee80211_sta *out_sta;
};

static void pv1_find_sta_by_aid(void *data, struct ieee80211_sta *sta)
{
	struct pv1_find_sta_aid_iter *iter_data = data;
	struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;

	if (!msta) {
		MORSE_WARN_ON(FEATURE_ID_MESH, 1);
		return;
	}

	if (msta->vif != iter_data->on_vif)
		return;

	MORSE_WARN_ON(FEATURE_ID_DEFAULT, iter_data->out_sta); /* already found */
	if (sta->aid == iter_data->aid)
		iter_data->out_sta = sta;
}

struct ieee80211_sta *morse_pv1_find_sta(struct ieee80211_vif *vif,
				struct dot11ah_mac_pv1_hdr *pv1_hdr)
{
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct morse *mors = morse_vif_to_morse(mors_vif);
	u16 pv1_fc = le16_to_cpu(pv1_hdr->frame_ctrl);
	u16 pv1_fc_type = le16_to_cpu(pv1_hdr->frame_ctrl) & IEEE80211_PV1_FCTL_FTYPE;
	struct ieee80211_sta *sta = NULL;
	u16 sid;
	u16 aid;

	if (pv1_fc_type == DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA_SID) {
		struct dot11ah_mac_pv1_qos_data_sid_hdr *sid_header =
				(struct dot11ah_mac_pv1_qos_data_sid_hdr *)pv1_hdr;

		if (pv1_fc & IEEE80211_PV1_FCTL_FROMDS)
			sid = le16_to_cpu(sid_header->u.from_ds.addr1_sid);
		else
			sid = le16_to_cpu(sid_header->u.to_ds.addr2_sid);
		aid = sid & DOT11_MAC_PV1_SID_AID_MASK;

		if (vif->type == NL80211_IFTYPE_AP) {
			struct pv1_find_sta_aid_iter data = {
				.on_vif = vif,
				.aid = aid,
				.out_sta = NULL,
			};

			ieee80211_iterate_stations_atomic(mors->hw, pv1_find_sta_by_aid, &data);
			sta = data.out_sta;
		} else if (vif->type == NL80211_IFTYPE_STATION) {
			sta = ieee80211_find_sta(vif, vif->bss_conf.bssid);
		}
	} else if (pv1_fc_type == DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA) {
		struct dot11ah_mac_pv1_qos_data_hdr *qos_data_hdr =
				(struct dot11ah_mac_pv1_qos_data_hdr *)pv1_hdr;

		sta = ieee80211_find_sta(vif, qos_data_hdr->addr1);
	}

	return sta;
}

/**
 * morse_prepare_pv0_mac_header - Prepare PV0 mac header from PV1 data.
 *
 * @mors_vif:   Valid AP/STA iface
 * @pv0_hdr:   Pointer to PV0 mac header to be filled in the frame
 * @pv1_hdr:   Pointer to PV1 header of RX packet
 * @pv1_fc:    PV1 frame control to derive type of RX PV1 packet
 * @pv0_fc:    PV0 frame control to be filled in PV0 header
 *
 * @retun:   Length of PV1 mac header that need to be purged
 */
static int morse_prepare_pv0_mac_header(struct morse_vif *mors_vif,
		struct ieee80211_hdr *pv0_hdr, struct dot11ah_mac_pv1_hdr *pv1_hdr,
		u16 pv1_fc, u16 pv0_fc)
{
	struct morse_sta *msta;
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	u16 pv1_fc_type = pv1_fc & IEEE80211_PV1_FCTL_FTYPE;
	struct ieee80211_sta *sta = NULL;
	struct morse_sta_pv1 *stored_status = NULL;
	int pv1_hdr_size = sizeof(*pv1_hdr);

	if (pv1_fc_type == DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA_SID) {
		struct dot11ah_mac_pv1_qos_data_sid_hdr *sid_header =
				(struct dot11ah_mac_pv1_qos_data_sid_hdr *)pv1_hdr;
		u16 sid;
		u8 *tmp = sid_header->variable;

		/* Must be held while finding and dereferencing sta */
		rcu_read_lock();

		/* Find STA interface that have stored PV1 context */
		sta = morse_pv1_find_sta(vif, pv1_hdr);

		if (sta) {
			msta = (struct morse_sta *)sta->drv_priv;
			stored_status = &msta->rx_pv1_ctx;
		}

		pv1_hdr_size = sizeof(*sid_header);
		if (pv1_fc & IEEE80211_PV1_FCTL_FROMDS) {
			sid = le16_to_cpu(sid_header->u.from_ds.addr1_sid);
			memcpy(pv0_hdr->addr2, sid_header->u.from_ds.addr2, sizeof(pv0_hdr->addr2));
			memcpy(pv0_hdr->addr1, vif->addr, sizeof(pv0_hdr->addr1));
		} else {
			sid = le16_to_cpu(sid_header->u.to_ds.addr2_sid);
			memcpy(pv0_hdr->addr1, sid_header->u.to_ds.addr1, sizeof(pv0_hdr->addr1));
			if (sta)
				memcpy(pv0_hdr->addr2, sta->addr, sizeof(pv0_hdr->addr2));
		}

		/* Take A3 from RX frame only if A3 present is indicated,
		 * else use the stored address from PV1 context saved with the RX of
		 * Header Compression action frame.
		 */
		if (sid & DOT11_MAC_PV1_SID_A3_PRESENT) {
			memcpy(pv0_hdr->addr3, tmp, sizeof(pv0_hdr->addr3));
			tmp += sizeof(pv0_hdr->addr3);
			pv1_hdr_size += sizeof(pv0_hdr->addr3);
		} else if (stored_status && stored_status->a3_stored) {
			memcpy(pv0_hdr->addr3, stored_status->stored_a3, sizeof(pv0_hdr->addr3));
		}

		/* Take A3 from RX frame only if A4 present is indicated,
		 * else use the stored address from PV1 context saved with the RX of
		 * Header Compression action frame.
		 */
		if (sid & DOT11_MAC_PV1_SID_A4_PRESENT) {
			memcpy(pv0_hdr->addr4, tmp, sizeof(pv0_hdr->addr4));
			pv1_hdr_size += sizeof(pv0_hdr->addr4);
			pv0_fc |= IEEE80211_FCTL_TODS | IEEE80211_FCTL_FROMDS;
		} else if (stored_status && stored_status->a4_stored) {
			memcpy(pv0_hdr->addr4, stored_status->stored_a4, sizeof(pv0_hdr->addr4));
			pv0_fc |= IEEE80211_FCTL_TODS | IEEE80211_FCTL_FROMDS;
		}

		rcu_read_unlock();

		pv0_hdr->seq_ctrl = sid_header->sequence_ctrl;
	} else if (pv1_fc_type == DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA) {
		struct dot11ah_mac_pv1_qos_data_hdr *qos_data_hdr =
				(struct dot11ah_mac_pv1_qos_data_hdr *)pv1_hdr;

		pv1_hdr_size = sizeof(*qos_data_hdr);

		/* When both address present instead of SID expecting A1 and A3 is same */
		memcpy(pv0_hdr->addr1, qos_data_hdr->addr1, sizeof(pv0_hdr->addr1));
		memcpy(pv0_hdr->addr2, qos_data_hdr->addr2, sizeof(pv0_hdr->addr2));
		memcpy(pv0_hdr->addr3, qos_data_hdr->addr1, sizeof(pv0_hdr->addr3));

		pv0_hdr->seq_ctrl = qos_data_hdr->sequence_ctrl;
	}

	pv0_hdr->frame_control = cpu_to_le16(pv0_fc);

	return pv1_hdr_size;
}

/**
 * morse_prepare_ccmp_header - Prepare CCMP header from BPN of RX PV1 packet
 *
 * @skb:   SKB pointing to BPN
 */
static void morse_prepare_ccmp_header(struct sk_buff *skb)
{
	if (!skb)
		return;

	/* Packet coming from firmware has bpn filled in reverse order,
	 * i.e, data[0] == PN[7], reorder and frame CCMP header
	 */
	skb->data[7] = skb->data[0];
	skb->data[6] = skb->data[1];
	skb->data[0] = skb->data[5];
	skb->data[1] = skb->data[4];
	skb->data[4] = skb->data[3];
	skb->data[5] = skb->data[2];
	skb->data[2] = 0;
	skb->data[3] = 0;
}

int morse_mac_convert_pv1_to_pv0(struct morse *mors, struct morse_vif *mors_vif,
				 struct sk_buff *skb,
				 const struct morse_skb_rx_status *hdr_rx_status,
				 struct dot11ah_mac_pv1_hdr *pv1_hdr)
{
	u16 pv0_fc;
	int pv1_hdr_size;
	__le16 qos_ctrl;
	struct ieee80211_hdr pv0_hdr = {0};
	int pv0_hdr_size = sizeof(pv0_hdr);
	u16 pv1_fc = le16_to_cpu(pv1_hdr->frame_ctrl);
	bool is_protected = pv1_fc & IEEE80211_PV1_FCTL_PROTECTED;
	u32 flags = le32_to_cpu(hdr_rx_status->flags);
	int headroom_required;

	if (!mors_vif->enable_pv1)
		return -EINVAL;

	pv0_fc = morse_prepare_pv0_frame_ctrl(pv1_fc);
	morse_convert_pv1_to_pv0_qos_ctrl((u8 *)&qos_ctrl, pv1_fc);
	pv1_hdr_size = morse_prepare_pv0_mac_header(mors_vif, &pv0_hdr, pv1_hdr, pv1_fc, pv0_fc);
	headroom_required = (skb->len - pv1_hdr_size) + pv0_hdr_size + IEEE80211_QOS_CTL_LEN;

	if (headroom_required > skb->len + skb_headroom(skb)) {
		MORSE_ERR_RATELIMITED(mors,
				      "%s: RX SKB not does not have sufficient headroom allocated, has %d, expected %d",
					  __func__,
					  skb->len + skb_headroom(skb),
					  headroom_required);
		return -EFAULT;
	}

	/* Purge PV1 header, now skb points to data without header for open
	 * and to CCMP header for SAE
	 */
	skb_pull(skb, pv1_hdr_size);

	if (is_protected && (flags & MORSE_RX_STATUS_FLAGS_DECRYPTED))
		morse_prepare_ccmp_header(skb);

	/* Add QoS control for both open and SAE */
	memcpy(skb_push(skb, IEEE80211_QOS_CTL_LEN), &qos_ctrl, IEEE80211_QOS_CTL_LEN);

	if (!ieee80211_has_a4(cpu_to_le16(pv0_fc)))
		pv0_hdr_size -= sizeof(pv0_hdr.addr4);

	/* Add PV0 header to SKB pointer */
	memcpy(skb_push(skb, pv0_hdr_size), &pv0_hdr, pv0_hdr_size);

	return 0;
}

bool morse_is_pv1_protected_frame(struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	u16 fc = le16_to_cpu(hdr->frame_control);

	if (!morse_dot11ah_is_pv1_qos_data(fc))
		return false;

	return (fc & IEEE80211_PV1_FCTL_PROTECTED);
}

/**
 * morse_pv1_process_hc_req_work - Header Compression response workqueue handler
 *
 * @work:    Workqueue struct
 */
static void morse_pv1_process_hc_req_work(struct work_struct *work)
{
	struct morse_pv1 *pv1 = container_of(work, struct morse_pv1, hc_req_work);
	struct morse_vif *mors_vif = container_of(pv1, struct morse_vif, pv1);
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	struct ieee80211_sta *sta;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct morse_pv1_hc_request *rx_request = &mors_vif->pv1.rx_request;
	struct morse_sta_pv1 *req_status;
	struct morse_sta *mors_sta = NULL;
	u8 *a3 = NULL;
	u8 *a4 = NULL;
	bool store_a3;
	bool store_a4;

	/* Must be held while finding and dereferencing sta */
	rcu_read_lock();

	sta = ieee80211_find_sta(vif, mors_vif->pv1.rx_pv1_sta_addr);
	if (sta)
		mors_sta = (struct morse_sta *)sta->drv_priv;
	if (!mors_sta || !mors_sta->pv1_frame_support) {
		rcu_read_unlock();
		return;
	}

	req_status = &mors_sta->rx_pv1_ctx;
	memset(req_status, 0, sizeof(*req_status));

	store_a3 = rx_request->a1_a3_differ;
	store_a4 = rx_request->a2_a4_differ;

	mutex_lock(&mors->lock);

	if (store_a3)
		a3 = (u8 *)rx_request->header_compression_a3;

	if (store_a4)
		a4 = (u8 *)rx_request->header_compression_a4;

	if (morse_cmd_store_pv1_hc_data(mors, mors_vif, sta, a3, a4, true)) {
		rx_request->a1_a3_differ = false;
		rx_request->a2_a4_differ = false;
	} else {
		mors_vif->pv1.fw_stored_response_status = true;
		if (a3) {
			req_status->a3_stored = true;
			memcpy(req_status->stored_a3, a3, sizeof(req_status->stored_a3));
		}

		if (a4) {
			req_status->a4_stored = true;
			memcpy(req_status->stored_a4, a4, sizeof(req_status->stored_a4));
		}
	}

	morse_mac_send_pv1_hc_action_frame(mors, vif, sta, NULL, 1);

	mutex_unlock(&mors->lock);

	rcu_read_unlock();
}

/**
 * morse_pv1_process_hc_resp_work - Header Compression response workqueue handler
 *
 * @work:    Workqueue struct
 */
static void morse_pv1_process_hc_resp_work(struct work_struct *work)
{
	struct morse_pv1 *pv1 = container_of(work, struct morse_pv1, hc_resp_work);
	struct morse_vif *mors_vif = container_of(pv1, struct morse_vif, pv1);
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	struct ieee80211_sta *sta;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct morse_sta_pv1 *resp_status;
	struct morse_sta *mors_sta = NULL;
	u8 *a3 = NULL;
	u8 *a4 = NULL;

	/* Must be held while finding and dereferencing sta */
	rcu_read_lock();

	sta = ieee80211_find_sta(vif, mors_vif->pv1.tx_pv1_sta_addr);
	if (sta)
		mors_sta = (struct morse_sta *)sta->drv_priv;
	if (!mors_sta || !mors_sta->pv1_frame_support) {
		rcu_read_unlock();
		return;
	}

	resp_status = &mors_sta->tx_pv1_ctx;

	a3 = (u8 *)resp_status->stored_a3;
	a4 = (u8 *)resp_status->stored_a4;

	if (a3 || a4)
		morse_cmd_store_pv1_hc_data(mors, mors_vif, sta, a3, a4, false);

	rcu_read_unlock();
}

int morse_mac_convert_pv0_to_pv1(struct morse *mors, struct morse_vif *mors_vif,
		struct ieee80211_sta *sta, struct sk_buff *skb)
{
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(skb)->data;
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	u16 pv0_fc = le16_to_cpu(hdr->frame_control);

	if (!mors_sta || mors_sta->state < IEEE80211_STA_ASSOC)
		return -EFAULT;

	if (!ieee80211_is_data_qos(hdr->frame_control))
		return -EINVAL;

	if ((pv0_fc & IEEE80211_FCTL_PROTECTED) && !info->control.hw_key) {
		MORSE_ERR_RATELIMITED(mors,
				      "%s: Failed to convert protected PV0 frame to PV1\n",
					  __func__);
		return -EINVAL;
	}

	morse_pv1_a3_a4_check(mors_vif, sta, skb);
	morse_mac_send_pv1_hc_action_frame(mors, vif, sta, skb, 0);
	return morse_convert_pv0_to_pv1(mors, mors_vif, sta, skb);
}

void mors_pv1_init_vif(struct morse_vif *mors_vif)
{
	if (!mors_vif || !mors_vif->enable_pv1)
		return;

	mors_vif->pv1.hc_response_timeout = 0;
	memset(&mors_vif->pv1.tx_request, 0, sizeof(mors_vif->pv1.tx_request));
	memset(&mors_vif->pv1.rx_request, 0, sizeof(mors_vif->pv1.rx_request));
	mutex_init(&mors_vif->pv1.lock);
	INIT_WORK(&mors_vif->pv1.hc_req_work, morse_pv1_process_hc_req_work);
	INIT_WORK(&mors_vif->pv1.hc_resp_work, morse_pv1_process_hc_resp_work);
}

void morse_pv1_finish_vif(struct morse_vif *mors_vif)
{
	if (!mors_vif || !mors_vif->enable_pv1)
		return;

	cancel_work_sync(&mors_vif->pv1.hc_req_work);
	cancel_work_sync(&mors_vif->pv1.hc_resp_work);
}
