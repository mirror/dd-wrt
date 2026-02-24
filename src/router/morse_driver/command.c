/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>

#include "debug.h"
#include "command.h"
#include "skbq.h"
#include "mac.h"
#include "skb_header.h"
#include "watchdog.h"
#include "ps.h"
#include "raw.h"
#include "twt.h"
#include "cac.h"
#include "operations.h"
#include "vendor_ie.h"
#include "ocs.h"
#include "mbssid.h"
#include "mesh.h"
#include "morse_commands.h"
#include "wiphy.h"

#define MM_BA_TIMEOUT (5000)
#define MM_MAX_COMMAND_RETRY 2

/*
 * These timeouts (in msecs) must be kept in sync with the same definitions in the driver.
 */
#define MM_CMD_DEFAULT_TIMEOUT_MS 600
#define MM_CMD_POWERSAVE_TIMEOUT_MS 2000
#define MM_CMD_HEALTH_CHECK_TIMEOUT_MS 1000

enum morse_interface_type {
	MORSE_INTERFACE_TYPE_INVALID = 0,
	MORSE_INTERFACE_TYPE_STA = 1,
	MORSE_INTERFACE_TYPE_AP = 2,
	MORSE_INTERFACE_TYPE_MON = 3,
	MORSE_INTERFACE_TYPE_ADHOC = 4,
	MORSE_INTERFACE_TYPE_MESH = 5,

	MORSE_INTERFACE_TYPE_LAST = MORSE_INTERFACE_TYPE_MESH,
	MORSE_INTERFACE_TYPE_MAX = INT_MAX,
};

struct morse_cmd_resp_cb {
	int ret;
	u32 length;
	struct morse_cmd_resp *dest_resp;
};

/* Set driver to chip command timeout: max to wait (in ms) before failing the command */
static u32 default_cmd_timeout_ms __read_mostly = MM_CMD_DEFAULT_TIMEOUT_MS;
module_param(default_cmd_timeout_ms, uint, 0644);
MODULE_PARM_DESC(default_cmd_timeout_ms, "Default command timeout (in ms)");

static void morse_cmd_init(struct morse *mors, struct morse_cmd_header *hdr,
			   enum morse_cmd_id cmd, u16 vif_id, u16 len)
{
	if (len < sizeof(*hdr)) {
		MORSE_ERR_RATELIMITED(mors, "Invalid cmd len %d\n", len);
		return;
	}

	hdr->message_id = cpu_to_le16(cmd);
	hdr->len = cpu_to_le16(len - sizeof(*hdr));
	hdr->vif_id = cpu_to_le16(vif_id);
}

/**
 * morse_cmd_standby_exit_reason_to_str() - Convert standby exit reason to string
 *
 * @reason: Reason for failure
 *
 * return string representing failure
 */
static const char *morse_cmd_standby_exit_reason_to_str(enum
		morse_cmd_standby_mode_exit_reason reason)
{
	switch (reason) {
	case MORSE_CMD_STANDBY_MODE_EXIT_REASON_NONE:
		return "none";
	case MORSE_CMD_STANDBY_MODE_EXIT_REASON_WAKEUP_FRAME:
		return "wake-up frame";
	case MORSE_CMD_STANDBY_MODE_EXIT_REASON_ASSOCIATE:
		return "associate";
	case MORSE_CMD_STANDBY_MODE_EXIT_REASON_EXT_INPUT:
		return "external input";
	case MORSE_CMD_STANDBY_MODE_EXIT_REASON_WHITELIST_PKT:
		return "whitelist pkt";
	case MORSE_CMD_STANDBY_MODE_EXIT_REASON_TCP_CONNECTION_LOST:
		return "tcp connection lost";
	case MORSE_CMD_STANDBY_MODE_EXIT_REASON_HW_SCAN_NOT_ENABLED:
		return "hw scan not enabled";
	case MORSE_CMD_STANDBY_MODE_EXIT_REASON_HW_SCAN_FAILED_TO_START:
		return "hw scan failed to start";
	default:
		return "unknown";
	}
}

static int morse_cmd_tx(struct morse *mors, struct morse_cmd_resp *resp,
			struct morse_cmd_req *req, u32 length, u32 timeout, const char *func)
{
	int cmd_len;
	int ret = 0;
	u16 host_id;
	int retry = 0;
	unsigned long wait_ret = 0;
	struct sk_buff *skb;
	struct morse_skbq *cmd_q = mors->cfg->ops->skbq_cmd_tc_q(mors);
	struct morse_cmd_resp_cb *resp_cb;
	DECLARE_COMPLETION_ONSTACK(cmd_comp);

	BUILD_BUG_ON(sizeof(struct morse_cmd_resp_cb) > IEEE80211_TX_INFO_DRIVER_DATA_SIZE);

	if (!cmd_q)
		/* No control pageset, not supported by FW */
		return -ENODEV;

	cmd_len = sizeof(*req) + le16_to_cpu(req->hdr.len);
	req->hdr.flags = cpu_to_le16(MORSE_CMD_TYPE_REQ);

	mutex_lock(&mors->cmd_wait);
	mors->cmd_seq++;
	if (mors->cmd_seq > MORSE_CMD_HOST_ID_SEQ_MAX)
		mors->cmd_seq = 1;
	host_id = mors->cmd_seq << MORSE_CMD_HOST_ID_SEQ_SHIFT;

	/* Make sure no one enables PS until the command is responded to or timed out */
	morse_ps_disable(mors);

	do {
		req->hdr.host_id = cpu_to_le16(host_id | retry);

		skb = morse_skbq_alloc_skb(cmd_q, cmd_len);
		if (!skb) {
			ret = -ENOMEM;
			break;
		}

		memcpy(skb->data, req, cmd_len);
		resp_cb = (struct morse_cmd_resp_cb *)IEEE80211_SKB_CB(skb)->driver_data;
		resp_cb->length = length;
		resp_cb->dest_resp = resp;

		MORSE_DBG(mors, "CMD 0x%04x:%04x\n", le16_to_cpu(req->hdr.message_id),
			  le16_to_cpu(req->hdr.host_id));

		mutex_lock(&mors->cmd_lock);
		mors->cmd_comp = &cmd_comp;
		if (retry > 0)
			reinit_completion(&cmd_comp);
		timeout = timeout ? timeout : default_cmd_timeout_ms;
		ret = morse_skbq_skb_tx(cmd_q, &skb, NULL, MORSE_SKB_CHAN_COMMAND);
		mutex_unlock(&mors->cmd_lock);

		if (ret) {
			MORSE_ERR(mors, "morse_skbq_tx fail: %d\n", ret);
			break;
		}

		wait_ret = wait_for_completion_timeout(&cmd_comp, msecs_to_jiffies(timeout));
		mutex_lock(&mors->cmd_lock);
		mors->cmd_comp = NULL;

		if (!wait_ret) {
			MORSE_INFO(mors, "Try:%d Command %04x:%04x timeout after %u ms\n",
				   retry, le16_to_cpu(req->hdr.message_id),
				   le16_to_cpu(req->hdr.host_id), timeout);
			ret = -ETIMEDOUT;
		} else {
			ret = (length && resp) ? le32_to_cpu(resp->status) : resp_cb->ret;

			MORSE_DBG(mors, "Command 0x%04x:%04x status 0x%08x\n",
				  le16_to_cpu(req->hdr.message_id),
				  le16_to_cpu(req->hdr.host_id), ret);
			if (ret)
				MORSE_ERR(mors, "Command 0x%04x:%04x error %d\n",
					  le16_to_cpu(req->hdr.message_id),
					  le16_to_cpu(req->hdr.host_id), ret);
		}
		/* Free the command request */
		spin_lock_bh(&cmd_q->lock);
		morse_skbq_skb_finish(cmd_q, skb, NULL);
		spin_unlock_bh(&cmd_q->lock);
		mutex_unlock(&mors->cmd_lock);

		retry++;
	} while ((ret == -ETIMEDOUT) && retry < MM_MAX_COMMAND_RETRY);

	morse_ps_enable(mors);
	mutex_unlock(&mors->cmd_wait);

	if (ret == -ETIMEDOUT)
		MORSE_ERR(mors, "Command %s %02x:%02x timed out\n",
			  func, le16_to_cpu(req->hdr.message_id), le16_to_cpu(req->hdr.host_id));
	else if (ret != 0)
		MORSE_ERR(mors, "Command %s %02x:%02x failed with rc %d (0x%x)\n",
			  func, le16_to_cpu(req->hdr.message_id),
			  le16_to_cpu(req->hdr.host_id), ret, ret);

	return ret;
}

static int morse_cmd_ocs_req(struct morse_vif *mors_vif, struct morse_cmd_resp_ocs_driver *drv_resp,
			     struct morse_cmd_req_ocs_driver *drv_req)
{
	int ret;
	struct morse_cmd_req_ocs req;
	struct morse *mors = morse_vif_to_morse(mors_vif);

	if (sizeof(struct morse_cmd_resp_ocs) != sizeof(*drv_resp)) {
		MORSE_ERR(mors,
			"%s: OCS req failed - reason: inconsistent response size, firmware: %ld, driver:%ld\n",
			__func__, (unsigned long)sizeof(struct morse_cmd_resp_ocs),
			(unsigned long)sizeof(*drv_resp));
		return -EINVAL;
	}

	/* Prepare request */
	req.hdr = drv_req->hdr;
	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_OCS, 0, sizeof(req));

	req.subcmd = drv_req->subcmd;
	req.config.op_channel_freq_hz = drv_req->config.op_channel_freq_hz;
	req.config.op_channel_bw_mhz = drv_req->config.op_channel_bw_mhz;
	req.config.pri_channel_bw_mhz = drv_req->config.pri_channel_bw_mhz;
	req.config.pri_1mhz_channel_index = drv_req->config.pri_1mhz_channel_index;
	req.config.aid = cpu_to_le16(MORSE_OCS_AID);
	req.config.type = ocs_type;

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)drv_resp,
			   (struct morse_cmd_req *)&req, sizeof(*drv_resp), 0, __func__);
	if (ret)
		return ret;

	return morse_ocs_cmd_post_process(mors_vif, drv_resp, &req);
}

/**
 * morse_cmd_send_wake_action_frame() - Execute command to send wake action frame
 *
 * @mors	Morse chip struct
 * @req		Command from morsectrl
 */
static int morse_cmd_send_wake_action_frame(struct morse *mors, struct morse_cmd_req *req)
{
	struct morse_cmd_req_send_wake_action_frame *cmd_action =
	    (struct morse_cmd_req_send_wake_action_frame *)req;
	return morse_mac_send_vendor_wake_action_frame(mors, cmd_action->dest_addr,
						       cmd_action->payload,
						       le32_to_cpu(cmd_action->payload_size));
}

/**
 * morse_cmd_coredump() - Schedule the restart work, coredump, and wait for
 *                        chip reload.
 *
 * @mors	Morse chip struct
 */
static int morse_cmd_coredump(struct morse *mors)
{
	int ret;
	unsigned long rem;
	/* A core dump typically takes ~30s, applying a x2 buffer for completion */
	const int timeout_ms = 60000;
	DECLARE_COMPLETION_ONSTACK(user_coredump_comp);

	lockdep_assert_held(&mors->lock);

	ret = morse_coredump_new(mors, MORSE_COREDUMP_REASON_USER_REQUEST);
	if (ret)
		return ret;

	mors->user_coredump_comp = &user_coredump_comp;
	set_bit(MORSE_STATE_FLAG_DO_COREDUMP, &mors->state_flags);
	schedule_work(&mors->driver_restart);

	mutex_unlock(&mors->lock);
	rem = wait_for_completion_timeout(&user_coredump_comp, msecs_to_jiffies(timeout_ms));
	mutex_lock(&mors->lock);

	mors->user_coredump_comp = NULL;

	/* A value of zero indicates that wait_for_completion_timeout timed out
	 * waiting for completion
	 */
	return rem ? 0 : -1;
}

static void morse_cmd_cac_cfg_get(struct morse *mors, struct morse_vif *mors_vif,
				  struct morse_cmd_resp_cac *resp)
{
	struct cac_threshold_change_rules rules;
	int i;

	BUILD_BUG_ON(ARRAY_SIZE(resp->rule) < ARRAY_SIZE(rules.rule));

	morse_cac_get_rules(mors_vif, &rules, &resp->rule_tot);
	resp->rule_tot = rules.rule_tot;

	memset(&resp->rule, 0, sizeof(resp->rule));

	for (i = 0; i < resp->rule_tot && i < ARRAY_SIZE(resp->rule); i++) {
		struct cac_threshold_change_rule *rule = &rules.rule[i];
		struct morse_cmd_cac_change_rule *cfm_rule = &resp->rule[i];

		cfm_rule->arfs = cpu_to_le16(rule->arfs);
		cfm_rule->threshold_change =
				cpu_to_le16(cac_threshold_val2pc(rule->threshold_change));
	}
}

static int morse_cmd_cac_cfg_set(struct morse *mors, struct morse_vif *mors_vif,
				 struct morse_cmd_req_cac *req)
{
	struct cac_threshold_change_rules rules;
	int i;

	if (req->rule_tot > ARRAY_SIZE(rules.rule))
		return -E2BIG;

	if (req->rule_tot == 0)
		return -EINVAL;

	memset(&rules, 0, sizeof(rules));

	rules.rule_tot = req->rule_tot;

	for (i = 0; i < req->rule_tot; i++) {
		struct cac_threshold_change_rule *rule = &rules.rule[i];

		rule->arfs = le16_to_cpu(req->rule[i].arfs);
		rule->threshold_change =
			cac_threshold_pc2val(le16_to_cpu(req->rule[i].threshold_change));
	}

	morse_cac_set_rules(mors_vif, &rules);

	return 0;
}

static int morse_process_cac_cmd(struct morse *mors, struct morse_vif *mors_vif,
	struct morse_cmd_req *req, struct morse_cmd_resp *resp)
{
	struct morse_cmd_req_cac *cac_req = (struct morse_cmd_req_cac *)req;
	struct morse_cmd_resp_cac *cac_resp = (struct morse_cmd_resp_cac *)resp;

	resp->hdr.len = cpu_to_le16(4);

	switch (cac_req->opcode) {
	case MORSE_CMD_CAC_OP_DISABLE:
		return morse_cac_deinit(mors_vif);
	case MORSE_CMD_CAC_OP_ENABLE:
		return morse_cac_init(mors, mors_vif);
	case MORSE_CMD_CAC_OP_CFG_GET:
		resp->hdr.len = cpu_to_le16(sizeof(*cac_resp) - sizeof(cac_resp->hdr));
		morse_cmd_cac_cfg_get(mors, mors_vif, cac_resp);
		return 0;
	case MORSE_CMD_CAC_OP_CFG_SET:
		return morse_cmd_cac_cfg_set(mors, mors_vif, cac_req);
	default:
		return -EINVAL;
	}
}

static void morse_cmd_resp_init(struct morse_cmd_resp *resp, u16 len, int status)
{
	resp->hdr.len = cpu_to_le16(len);
	resp->status = cpu_to_le32(status);
}

static int morse_cmd_drv(struct morse *mors, struct ieee80211_vif *vif, struct morse_cmd_resp *resp,
			 struct morse_cmd_req *req, u32 length, u32 timeout)
{
	int ret;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);

	switch (le16_to_cpu(req->hdr.message_id)) {
	case MORSE_CMD_ID_SET_STA_TYPE:
		if (mors_vif) {
			struct morse_cmd_req_set_sta_type *req_sta =
				(struct morse_cmd_req_set_sta_type *)req;
			mors->custom_configs.sta_type = req_sta->sta_type;
			ret = 0;
			morse_cmd_resp_init(resp, 4, ret);
		} else {
			ret = -EFAULT;
		}
		break;
	case MORSE_CMD_ID_SET_ENC_MODE:
		if (mors_vif) {
			struct morse_cmd_req_set_enc_mode *req_enc =
				(struct morse_cmd_req_set_enc_mode *)req;
			mors->custom_configs.enc_mode = req_enc->enc_mode;
			ret = 0;
			morse_cmd_resp_init(resp, 4, ret);
		} else {
			ret = -EFAULT;
		}
		break;
	case MORSE_CMD_ID_SET_LISTEN_INTERVAL:
		if (mors_vif) {
			struct morse_cmd_req_set_listen_interval *cmd_li =
			    (struct morse_cmd_req_set_listen_interval *)req;

			mors->custom_configs.listen_interval = le16_to_cpu(cmd_li->listen_interval);

			mors->custom_configs.listen_interval_ovr = true;

			MORSE_DBG(mors, "Listen Interval %d\n",
				  mors->custom_configs.listen_interval);

			ret = 0;
			morse_cmd_resp_init(resp, 4, ret);
		} else {
			ret = -EFAULT;
		}
		break;
	case MORSE_CMD_ID_SET_AMPDU: {
		struct morse_cmd_req_set_ampdu *req_ampdu =
				(struct morse_cmd_req_set_ampdu *)req;
		mors->custom_configs.enable_ampdu = (req_ampdu->ampdu_enabled > 0);
		ret = 0;

		morse_cmd_resp_init(resp, 4, ret);
		break;
	}
	case MORSE_CMD_ID_CONFIG_RAW:
		if (mors_vif) {
			struct morse_cmd_req_config_raw *req_raw =
				(struct morse_cmd_req_config_raw *)req;

			ret = morse_raw_process_cmd(mors_vif, req_raw);

			morse_cmd_resp_init(resp, 4, ret);
		} else {
			ret = -EFAULT;
		}
		break;
	case MORSE_CMD_ID_TEST_BA:
		if (mors_vif) {
			struct ieee80211_sta *sta;
			struct morse_cmd_req_test_ba *test_ba = (struct morse_cmd_req_test_ba *)req;

			ret = -EINVAL;
			if (test_ba->tx) {
				/* Must be held while finding and dereferencing sta */
				rcu_read_lock();
				sta = ieee80211_find_sta(vif, test_ba->addr);
				if (!sta) {
					rcu_read_unlock();
					break;
				}

				if (test_ba->start)
					ret = ieee80211_start_tx_ba_session(sta,
									le32_to_cpu(test_ba->tid),
									MM_BA_TIMEOUT);
				else
					ret = ieee80211_stop_tx_ba_session(sta,
									le32_to_cpu(test_ba->tid));
				rcu_read_unlock();
			} else {
				if (test_ba->start)
					break;
				ieee80211_stop_rx_ba_session(vif,
								le32_to_cpu(test_ba->tid),
								test_ba->addr);
				ret = 0;
			}
		} else {
			ret = -EFAULT;
		}
		break;
	case MORSE_CMD_ID_SET_S1G_OP_CLASS: {
		struct morse_cmd_req_set_s1g_op_class *req_op =
			(struct morse_cmd_req_set_s1g_op_class *)req;
		mors->custom_configs.channel_info.s1g_operating_class = req_op->opclass;
		mors->custom_configs.channel_info.pri_global_operating_class = req_op->prim_opclass;
		ret = 0;

		morse_cmd_resp_init(resp, 4, ret);
		break;
	}
	case MORSE_CMD_ID_SEND_WAKE_ACTION_FRAME:
		ret = morse_cmd_send_wake_action_frame(mors, req);
		morse_cmd_resp_init(resp, 4, ret);
		break;

	case MORSE_CMD_ID_DRIVER_SET_DUTY_CYCLE: {
		struct morse_cmd_req_set_duty_cycle *duty_req =
		    (struct morse_cmd_req_set_duty_cycle *)req;
		mors->custom_configs.duty_cycle = le32_to_cpu(duty_req->config.duty_cycle);
		/*
		 * When a disable duty cycle command is executed via morsectrl it sends a
		 * duty cycle value of 100%. When this happens set the duty cycle value in
		 * custom config as 0. This enables the driver to use the duty cycle value
		 * mentioned in the regdom.
		 */
		if (le32_to_cpu(duty_req->config.duty_cycle) == 10000)
			mors->custom_configs.duty_cycle = 0;
		mors->duty_cycle = le32_to_cpu(duty_req->config.duty_cycle);

		req->hdr.message_id = cpu_to_le16(MORSE_CMD_ID_SET_DUTY_CYCLE);
		ret = morse_cmd_tx(mors, resp, req, le16_to_cpu(resp->hdr.len), 0, __func__);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	}
	case MORSE_CMD_ID_SET_TWT_CONF:
		ret = morse_process_twt_cmd(mors, mors_vif, req);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	case MORSE_CMD_ID_CAC:
		ret = morse_process_cac_cmd(mors, mors_vif, req, resp);
		resp->status = cpu_to_le32(ret);
		break;
	case MORSE_CMD_ID_GET_AVAILABLE_CHANNELS:
		ret = morse_cmd_get_available_channels(mors, resp);
		resp->status = cpu_to_le32(ret);
		break;
	case MORSE_CMD_ID_SET_ECSA_S1G_INFO:
		if (mors_vif) {
			struct morse_cmd_req_set_ecsa_s1g_info *req_ecsa =
				(struct morse_cmd_req_set_ecsa_s1g_info *)req;

			MORSE_INFO(mors, "ECSA channel info   :\n"
				   " * s1g_global_operating_class    : %d\n"
				   " * s1g_primary_bandwidth         : %d\n"
				   " * s1g_operating_frequency       : %u\n"
				   " * s1g_operating_bandwidth       : %d\n"
				   " * s1g_primary_1MHz_chan_idx     : %d\n"
				   " * primary_global_op_class       : %d\n"
				   " * s1g_cap0                      : %d\n",
				   req_ecsa->opclass,
				   req_ecsa->primary_channel_bw_mhz,
				   req_ecsa->operating_channel_freq_hz,
				   req_ecsa->operating_channel_bw_mhz,
				   req_ecsa->prim_1mhz_ch_idx,
				   req_ecsa->prim_opclass,
				   req_ecsa->s1g_cap0);
			mors_vif->ecsa_channel_info.op_chan_freq_hz =
				le32_to_cpu(req_ecsa->operating_channel_freq_hz);
			mors_vif->ecsa_channel_info.op_bw_mhz = req_ecsa->operating_channel_bw_mhz;
			mors_vif->ecsa_channel_info.pri_1mhz_chan_idx =
				req_ecsa->prim_1mhz_ch_idx;
			mors_vif->ecsa_channel_info.pri_bw_mhz = req_ecsa->primary_channel_bw_mhz;
			mors_vif->ecsa_channel_info.s1g_operating_class = req_ecsa->opclass;
			mors_vif->ecsa_channel_info.pri_global_operating_class =
			    req_ecsa->prim_opclass;
			mors_vif->ecsa_channel_info.s1g_cap0 = req_ecsa->s1g_cap0;
			mors_vif->ecsa_channel_info.s1g_cap1 = req_ecsa->s1g_cap1;
			mors_vif->ecsa_channel_info.s1g_cap2 = req_ecsa->s1g_cap2;
			mors_vif->ecsa_channel_info.s1g_cap3 = req_ecsa->s1g_cap3;
			mors_vif->mask_ecsa_info_in_beacon = false;
			ret = 0;
		} else {
			ret = -EFAULT;
		}
		morse_cmd_resp_init(resp, 4, ret);
		break;
	case MORSE_CMD_ID_MBSSID:
		ret = morse_command_process_bssid_info(mors_vif,
						       (struct morse_cmd_req_mbssid *)req);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	case MORSE_CMD_ID_OCS_DRIVER:
		ret = morse_cmd_ocs_req(mors_vif,
					(struct morse_cmd_resp_ocs_driver *)resp,
					(struct morse_cmd_req_ocs_driver *)req);
		resp->status = cpu_to_le32(ret);
		break;
	case MORSE_CMD_ID_SET_MESH_CONFIG:
		ret = morse_cmd_set_mesh_config(mors_vif,
						(struct morse_cmd_req_set_mesh_config *)req,
						NULL);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	case MORSE_CMD_ID_SET_MCBA_CONF:
		ret = morse_cmd_process_mbca_conf(mors_vif,
						  (struct morse_cmd_req_set_mcba_conf *)req);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	case MORSE_CMD_ID_DYNAMIC_PEERING_CONFIG:
		ret = morse_cmd_process_dynamic_peering_conf(mors_vif,
					     (struct morse_cmd_req_dynamic_peering_config *)req);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	case MORSE_CMD_ID_CONFIG_BSS_STATS:
		ret = morse_cmd_process_bss_stats_conf(mors_vif,
					     (struct morse_cmd_req_config_bss_stats *)req);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

int morse_cmd_resp_process(struct morse *mors, struct sk_buff *skb)
{
	int length, ret = -ESRCH;	/* No such process */
	struct morse_skbq *cmd_q = mors->cfg->ops->skbq_cmd_tc_q(mors);
	struct morse_cmd_resp *src_resp = (struct morse_cmd_resp *)(skb->data);
	struct sk_buff *cmd_skb = NULL;
	struct morse_cmd_resp_cb *resp_cb;
	struct morse_cmd_resp *dest_resp;
	struct morse_cmd_req *req;
	u16 message_id = 0;
	u16 host_id = 0;
	u16 resp_message_id = le16_to_cpu(src_resp->hdr.message_id);
	u16 resp_host_id = le16_to_cpu(src_resp->hdr.host_id);
	bool is_late_response = false;

	MORSE_DBG(mors, "EVT 0x%04x:0x%04x\n", resp_message_id, resp_host_id);

	if (!MORSE_CMD_IS_RESP(src_resp)) {
		ret = morse_mac_event_recv(mors, skb);
		goto exit_free;
	}

	mutex_lock(&mors->cmd_lock);

	cmd_skb = morse_skbq_tx_pending(cmd_q);
	if (cmd_skb) {
		morse_skb_remove_hdr_after_sent_to_chip(cmd_skb);
		req = (struct morse_cmd_req *)cmd_skb->data;
		message_id = le16_to_cpu(req->hdr.message_id);
		host_id = le16_to_cpu(req->hdr.host_id);
	}

	/*
	 * If there is no pending command or the sequence ID does not match, this is a late response
	 * for a timed out command which has been cleaned up, so just free up the response.
	 * If a command was retried, the response may be from the retry or from the original
	 * command (late response) but not from both because the firmware will silently drop
	 * a retry if it received the initial request. So a mismatched retry counter is treated
	 * as a matched command and response.
	 */
	if (!cmd_skb || message_id != resp_message_id ||
	    (host_id & MORSE_CMD_HOST_ID_SEQ_MASK) != (resp_host_id & MORSE_CMD_HOST_ID_SEQ_MASK)) {
		MORSE_ERR(mors,
			  "Late response for timed out req 0x%04x:%04x have 0x%04x:%04x 0x%04x\n",
			  resp_message_id, resp_host_id, message_id, host_id, mors->cmd_seq);
		is_late_response = true;
		goto exit;
	}
	if ((host_id & MORSE_CMD_HOST_ID_RETRY_MASK) !=
	    (resp_host_id & MORSE_CMD_HOST_ID_RETRY_MASK))
		MORSE_INFO(mors, "Command retry mismatch 0x%04x:%04x 0x%04x:%04x\n",
			   message_id, host_id, resp_message_id, resp_host_id);

	resp_cb = (struct morse_cmd_resp_cb *)IEEE80211_SKB_CB(cmd_skb)->driver_data;
	length = resp_cb->length;
	dest_resp = resp_cb->dest_resp;
	if (length >= sizeof(struct morse_cmd_resp) && dest_resp) {
		ret = 0;
		length = min_t(int, length, le16_to_cpu(src_resp->hdr.len) +
			       sizeof(struct morse_cmd_header));
		memcpy(dest_resp, src_resp, length);
	} else {
		ret = le32_to_cpu(src_resp->status);
	}

	resp_cb->ret = ret;

exit:
	if (cmd_skb && !is_late_response) {
		/* Complete if not already timed out */
		if (mors->cmd_comp)
			complete(mors->cmd_comp);
	}

	mutex_unlock(&mors->cmd_lock);
exit_free:
	dev_kfree_skb(skb);

	return 0;
}

int morse_cmd_set_channel(struct morse *mors, u32 op_chan_freq_hz,
			  u8 pri_1mhz_chan_idx, u8 op_bw_mhz, u8 pri_bw_mhz, s32 *power_mbm)
{
	int ret;
	struct morse_cmd_req_set_channel req;
	struct morse_cmd_resp_set_channel resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_CHANNEL, 0, sizeof(req));

	/* May be 0xFFFF/0xFFFFFFFF to indicate no change */
	req.op_chan_freq_hz = cpu_to_le32(op_chan_freq_hz);
	req.op_bw_mhz = op_bw_mhz;
	req.pri_bw_mhz = pri_bw_mhz;
	req.pri_1mhz_chan_idx = pri_1mhz_chan_idx;
	/* TODO: add other modes as necessary */
	req.dot11_mode = MORSE_CMD_DOT11_PROTO_MODE_AH;

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);
	if (!ret)
		*power_mbm = QDBM_TO_MBM(le32_to_cpu(resp.power_qdbm));

	return ret;
}

int morse_cmd_get_current_channel(struct morse *mors, u32 *op_chan_freq_hz,
				  u8 *pri_1mhz_chan_idx, u8 *op_bw_mhz, u8 *pri_bw_mhz)
{
	struct morse_cmd_req_get_channel req;
	struct morse_cmd_resp_get_channel resp;
	int ret;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_CHANNEL, 0, sizeof(req));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp, (struct morse_cmd_req *)&req,
			   sizeof(resp), 0, __func__);
	if (ret)
		return ret;

	*op_chan_freq_hz = le32_to_cpu(resp.op_chan_freq_hz);
	*pri_1mhz_chan_idx = resp.pri_1mhz_chan_idx;
	*op_bw_mhz = resp.op_chan_bw_mhz;
	*pri_bw_mhz = resp.pri_chan_bw_mhz;

	return 0;
}

int morse_cmd_set_txpower(struct morse *mors, s32 *out_power_mbm, int txpower_mbm)
{
	int ret;
	struct morse_cmd_req_set_txpower req;
	struct morse_cmd_resp_set_txpower resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_TXPOWER, 0, sizeof(req));

	req.power_qdbm = cpu_to_le32(MBM_TO_QDBM(txpower_mbm));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);
	if (ret == 0)
		*out_power_mbm = QDBM_TO_MBM(le32_to_cpu(resp.power_qdbm));

	return ret;
}

int morse_cmd_get_max_txpower(struct morse *mors, s32 *out_power_mbm)
{
	int ret;
	struct morse_cmd_req_get_max_txpower req;
	struct morse_cmd_resp_get_max_txpower resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_MAX_TXPOWER, 0, sizeof(req));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);
	if (ret == 0)
		*out_power_mbm = QDBM_TO_MBM(le32_to_cpu(resp.power_qdbm));

	return ret;
}

int morse_cmd_set_ps(struct morse *mors, bool enabled, bool enable_dynamic_ps_offload)
{
	int ret;
	struct morse_cmd_req_config_ps req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_CONFIG_PS, 0, sizeof(req));

	req.enabled = (u8)enabled;
	req.dynamic_ps_offload = (u8)enable_dynamic_ps_offload;

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0,
			   default_cmd_timeout_ms > MM_CMD_POWERSAVE_TIMEOUT_MS ?
			   default_cmd_timeout_ms : MM_CMD_POWERSAVE_TIMEOUT_MS, __func__);

	return ret;
}

int morse_cmd_config_beacon_timer(struct morse *mors, struct morse_vif *morse_vif, bool enabled)
{
	int ret;
	struct morse_cmd_req_bss_beacon_config req;
	struct morse_cmd_resp_bss_beacon_config resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_BSS_BEACON_CONFIG, morse_vif->id, sizeof(req));
	req.enable = enabled;

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_store_pv1_hc_data(struct morse *mors, struct morse_vif *mors_vif,
				struct ieee80211_sta *sta, u8 *a3, u8 *a4, bool is_store_in_rx)
{
	int ret;
	struct morse_cmd_req_pv1_store_hc req;
	struct morse_cmd_resp_pv1_store_hc resp;
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;

	memset(&req, 0, sizeof(req));
	if (a3 || a4)
		req.opcode = MORSE_CMD_PV1_HC_OPCODE_A3_A4;

	if (a3) {
		req.pv1_hc_store |= MORSE_PV1_CMD_STORE_A3;
		memcpy(req.a3, a3, sizeof(req.a3));
	}

	if (a4) {
		req.pv1_hc_store |= MORSE_PV1_CMD_STORE_A4;
		memcpy(req.a4, a4, sizeof(req.a4));
	}

	if (is_store_in_rx)
		req.pv1_hc_store |= MORSE_PV1_CMD_STORE_RX;

	memcpy(req.sta_addr, mors_sta->addr, sizeof(req.sta_addr));

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_PV1_STORE_HC, mors_vif->id, sizeof(req));
	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);

	return ret;
}

int morse_cmd_add_if(struct morse *mors, u16 *vif_id, const u8 *addr, enum nl80211_iftype type)
{
	int ret;
	struct morse_cmd_req_add_interface req;
	struct morse_cmd_resp_add_interface resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_ADD_INTERFACE, 0, sizeof(req));

	switch (type) {
	case NL80211_IFTYPE_STATION:
		req.interface_type = cpu_to_le32(MORSE_CMD_INTERFACE_TYPE_STA);
		break;
	case NL80211_IFTYPE_ADHOC:
		req.interface_type = cpu_to_le32(MORSE_CMD_INTERFACE_TYPE_ADHOC);
		break;
	case NL80211_IFTYPE_AP:
		req.interface_type = cpu_to_le32(MORSE_CMD_INTERFACE_TYPE_AP);
		break;
	case NL80211_IFTYPE_MONITOR:
		req.interface_type = cpu_to_le32(MORSE_CMD_INTERFACE_TYPE_MON);
		break;
	case NL80211_IFTYPE_MESH_POINT:
		req.interface_type = cpu_to_le32(MORSE_CMD_INTERFACE_TYPE_MESH);
		break;
	default:
		return -EOPNOTSUPP;
	}

	memcpy(req.addr.octet, addr, sizeof(req.addr.octet));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);
	if (ret == 0)
		*vif_id = le16_to_cpu(resp.hdr.vif_id);

	return ret;
}

int morse_cmd_rm_if(struct morse *mors, u16 vif_id)
{
	int ret;
	struct morse_cmd_req_remove_interface req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_REMOVE_INTERFACE, vif_id, sizeof(req));

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_cfg_bss(struct morse *mors, u16 vif_id, u16 beacon_int, u16 dtim_period, u32 cssid)
{
	int ret;
	struct morse_cmd_req_bss_config req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_BSS_CONFIG, vif_id, sizeof(req));

	req.beacon_interval_tu = cpu_to_le16(beacon_int);
	req.cssid = cpu_to_le32(cssid);
	req.dtim_period = cpu_to_le16(dtim_period);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_sta_state(struct morse *mors, struct morse_vif *mors_vif,
			u16 aid, struct ieee80211_sta *sta, enum ieee80211_sta_state state)
{
	int ret;
	struct morse_cmd_req_set_sta_state req;
	struct morse_cmd_resp_set_sta_state resp;
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;

	memset(&req, 0, sizeof(req));
	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_STA_STATE, mors_vif->id, sizeof(req));

	memcpy(req.sta_addr, sta->addr, sizeof(req.sta_addr));
	req.aid = cpu_to_le16(aid);
	req.state = cpu_to_le16(state);
	req.uapsd_queues = sta->uapsd_queues;
	if (mors_vif->enable_pv1 && mors_sta->pv1_frame_support)
		req.flags = cpu_to_le32(MORSE_CMD_STA_FLAG_S1G_PV1);

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);

	return ret;
}

int morse_cmd_disable_key(struct morse *mors, struct morse_vif *mors_vif,
			  u16 aid, struct ieee80211_key_conf *key)
{
	int ret;
	struct morse_cmd_req_disable_key req;

	MORSE_DBG(mors, "%s Disabling key for vif (%d):\n"
		  "\tkey->hw_key_idx: %d\n"
		  "\taid (optional): %d\n", __func__, mors_vif->id, key->hw_key_idx, aid);

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_DISABLE_KEY, mors_vif->id, sizeof(req));

	req.aid = cpu_to_le32(aid);
	req.key_idx = key->hw_key_idx;
	req.key_type = cpu_to_le32((key->flags & IEEE80211_KEY_FLAG_PAIRWISE) ?
	    MORSE_CMD_TEMPORAL_KEY_TYPE_PTK : MORSE_CMD_TEMPORAL_KEY_TYPE_GTK);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_install_key(struct morse *mors, struct morse_vif *mors_vif,
			  u16 aid, struct ieee80211_key_conf *key, enum morse_cmd_key_cipher cipher,
			  enum morse_cmd_aes_key_len length)
{
	int ret;
	struct morse_cmd_req_install_key req;
	struct morse_cmd_resp_install_key resp;

	MORSE_DBG(mors, "%s Installing key for vif (%d):\n"
		  "\tkey->idx: %d\n"
		  "\tkey->cipher: 0x%08x\n"
		  "\tkey->pn: %lld\n"
		  "\tkey->len: %d\n"
		  "\tkey->flags: 0x%08x\n"
		  "\taid (optional): %d\n",
		  __func__,
		  mors_vif->id,
		  key->keyidx,
		  key->cipher, (u64)atomic64_read(&key->tx_pn), key->keylen, key->flags, aid);

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_INSTALL_KEY, mors_vif->id, sizeof(req));

	req.pn = cpu_to_le64(atomic64_read(&key->tx_pn));
	req.aid = cpu_to_le32(aid);
	req.cipher = cipher;
	req.key_length = length;
	req.key_type = (key->flags & IEEE80211_KEY_FLAG_PAIRWISE) ?
	    MORSE_CMD_TEMPORAL_KEY_TYPE_PTK : MORSE_CMD_TEMPORAL_KEY_TYPE_GTK;

	req.key_idx = key->keyidx;
	memcpy(&req.key[0], &key->key[0], sizeof(req.key));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);

	if (ret == 0) {
		key->hw_key_idx = resp.key_idx;
		MORSE_DBG(mors, "%s Installed key @ hw index: %d\n", __func__, resp.key_idx);
	}

	return ret;
}

int morse_cmd_get_version(struct morse *mors)
{
	int ret;
	struct morse_cmd_req_get_version req;
	struct morse_cmd_resp_get_version *resp = NULL;
	/* Allocate space for the resp, the max version str len and a null character */
	u32 max_version_resp_size = sizeof(*resp) +
				    sizeof(resp->version[0]) * MORSE_CMD_MAX_VERSION_LEN + 1;

	memset(&mors->sw_ver, 0, sizeof(mors->sw_ver));

	/* we have to kmalloc otherwise we are bigger than stack allows */
	resp = kmalloc(max_version_resp_size, GFP_KERNEL);
	if (!resp)
		return -ENOSPC;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_VERSION, 0, sizeof(req));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)resp,
			   (struct morse_cmd_req *)&req, max_version_resp_size, 0, __func__);
	if (ret == 0) {
		int major;
		int minor;
		int patch;

		resp->version[le32_to_cpu(resp->length)] = '\0';
		MORSE_INFO(mors,
			   "Morse Driver Version: %s, Morse FW Version: %s\n",
			   DRV_VERSION, resp->version);

		if (sscanf(resp->version, "rel_%d_%d_%d", &major, &minor, &patch) == 3) {
			MORSE_WARN_ON_ONCE(FEATURE_ID_DEFAULT, major > __UINT8_MAX__);
			MORSE_WARN_ON_ONCE(FEATURE_ID_DEFAULT, minor > __UINT8_MAX__);
			MORSE_WARN_ON_ONCE(FEATURE_ID_DEFAULT, patch > __UINT8_MAX__);
			mors->sw_ver.major = major;
			mors->sw_ver.minor = minor;
			mors->sw_ver.patch = patch;
		}

		/* Keep the firmware version string for coredump creation */
		morse_coredump_set_fw_version_str(mors, resp->version);
	}

	kfree(resp);

	return ret;
}

int morse_cmd_get_disabled_channels(struct morse *mors,
				    struct morse_cmd_resp_get_disabled_channels *resp,
				    uint resp_len)
{
	struct morse_cmd_req req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_DISABLED_CHANNELS, 0, sizeof(req));

	return morse_cmd_tx(mors, (struct morse_cmd_resp *)resp, &req, resp_len, 0, __func__);
}

int morse_cmd_cfg_scan(struct morse *mors, bool enabled, bool is_survey)
{
	int ret;
	struct morse_cmd_req_scan_config req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SCAN_CONFIG, 0, sizeof(req));

	req.enabled = enabled;
	req.is_survey = is_survey;

	if (!enabled && is_survey)
		MORSE_WARN(mors,
			"%s: internal error - survey requested with scanning disabled",
			__func__);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_get_channel_usage(struct morse *mors, struct morse_survey_rx_usage_record *record)
{
	int ret;
	struct morse_cmd_req_get_channel_usage req;
	struct morse_cmd_resp_get_channel_usage resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_CHANNEL_USAGE, 0, sizeof(req));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);
	if (ret == 0) {
		record->time_listen = le64_to_cpu(resp.time_listen);
		record->time_rx = le64_to_cpu(resp.busy_time);
		record->freq_hz = le32_to_cpu(resp.freq_hz);
		record->bw_mhz = resp.bw_mhz;
		record->noise = resp.noise;
	}

	return ret;
}

static void morse_set_dtim_cts_to_self(bool enable, struct morse_vif *mors_vif)
{
	if (enable)
		MORSE_OPS_SET(&mors_vif->operations, DTIM_CTS_TO_SELF);
	else
		MORSE_OPS_CLEAR(&mors_vif->operations, DTIM_CTS_TO_SELF);
}

int morse_cmd_update_beacon_vendor_ie_oui_filter(struct morse *mors, struct morse_vif *mors_vif)
{
	struct morse_cmd_req_update_oui_filter req;
	struct vendor_ie_oui_filter_list_item *item;
	u8 idx = 0;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_UPDATE_OUI_FILTER, mors_vif->id, sizeof(req));

	spin_lock_bh(&mors_vif->vendor_ie.lock);
	list_for_each_entry(item, &mors_vif->vendor_ie.oui_filter_list, list) {
		if (item->mgmt_type_mask & MORSE_VENDOR_IE_TYPE_BEACON)
			memcpy(req.ouis[idx++], item->oui, sizeof(req.ouis[idx]));

		if (idx >= ARRAY_SIZE(req.ouis))
			break;
	}
	spin_unlock_bh(&mors_vif->vendor_ie.lock);

	req.n_ouis = idx;

	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);
}

int morse_cmd_cfg_multicast_filter(struct morse *mors, struct morse_vif *mors_vif)
{
	struct morse_cmd_req_mcast_filter *req;
	struct mcast_filter *filter = mors->mcast_filter;
	u16 filter_list_len = sizeof(filter->addr_list[0]) * filter->count;
	u16 alloc_len = filter_list_len + sizeof(*req);
	int ret = 0;

	req = kmalloc(alloc_len, GFP_KERNEL);

	morse_cmd_init(mors, &req->hdr, MORSE_CMD_ID_MCAST_FILTER, mors_vif->id, alloc_len);

	req->count = filter->count;
	memcpy(req->hw_addr, filter->addr_list, filter_list_len);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)req, 0, 0, __func__);
	kfree(req);
	return ret;
}

static int morse_cmd_vendor_set_channel(struct morse *mors, int cmd_len,
					struct morse_cmd_resp_set_channel *drv_resp,
					struct morse_cmd_req_set_channel *drv_req)
{
	int ret;
	struct morse_channel_info *stored_info = &mors->custom_configs.default_bw_info;
	const struct morse_dot11ah_channel *chan_s1g;

	drv_resp->hdr.message_id = drv_req->hdr.message_id;

	/* Send SET_CHANNEL command without reg_tx_power_set as it is undefined in firmware */
	drv_req->hdr.len = cpu_to_le16(sizeof(*drv_req) - sizeof(drv_req->hdr)
			  - sizeof(drv_req->reg_tx_power_set));
	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)drv_resp,
			   (struct morse_cmd_req *)drv_req, sizeof(*drv_resp), 0, __func__);
	if (ret)
		return ret;

	/* Store info */
	if (le32_to_cpu(drv_req->op_chan_freq_hz) != MORSE_CMD_CHANNEL_FREQ_NOT_SET)
		stored_info->op_chan_freq_hz = le32_to_cpu(drv_req->op_chan_freq_hz);

	if (drv_req->pri_1mhz_chan_idx != MORSE_CMD_CHANNEL_IDX_NOT_SET)
		stored_info->pri_1mhz_chan_idx = drv_req->pri_1mhz_chan_idx;

	if (drv_req->op_bw_mhz != MORSE_CMD_CHANNEL_BW_NOT_SET)
		stored_info->op_bw_mhz = drv_req->op_bw_mhz;

	if (drv_req->pri_bw_mhz != MORSE_CMD_CHANNEL_BW_NOT_SET)
		stored_info->pri_bw_mhz = drv_req->pri_bw_mhz;

	/* Validate that primary does not exceed operating */
	stored_info->pri_bw_mhz = (stored_info->op_bw_mhz == 1) ? 1 : stored_info->pri_bw_mhz;

	mors->tx_power_mbm = QDBM_TO_MBM(le32_to_cpu(drv_resp->power_qdbm));

	MORSE_INFO(mors,
		   "%s%s: f:%d o:%d p:%d i:%d power:%d mBm\n",
		   __func__, mors->in_scan ? " (scanning)" : "",
		   le32_to_cpu(drv_req->op_chan_freq_hz), drv_req->op_bw_mhz,
		   drv_req->pri_bw_mhz, drv_req->pri_1mhz_chan_idx,
		   mors->tx_power_mbm);

	if (!drv_req->reg_tx_power_set)
		return 0;

	/* Update txpower using S1G max values if possible */
	chan_s1g = morse_dot11ah_s1g_freq_to_s1g(le32_to_cpu(drv_req->op_chan_freq_hz),
				drv_req->op_bw_mhz);
	if (chan_s1g)
		morse_mac_set_txpower(mors, chan_s1g->ch.max_reg_power);

	return 0;
}

/**
 * morse_cmd_vendor_dhcp_offload() - Handle DHCP offload command being sent to chip
 *
 * @mors:	Global Morse struct
 * @resp:	Response for the command
 * @req:	Command request
 *
 * @return  Error code on failure, 0 otherwise
 */
static int morse_cmd_vendor_dhcp_offload(struct morse *mors, struct morse_cmd_resp_vendor *resp,
					 const struct morse_cmd_req_vendor *req)
{
	int ret;
	struct morse_cmd_req_dhcp_offload *dhcp_req =
		(struct morse_cmd_req_dhcp_offload *)req;
	struct morse_cmd_resp_dhcp_offload *dhcp_resp =
		(struct morse_cmd_resp_dhcp_offload *)resp;

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)resp,
				   (struct morse_cmd_req *)req, sizeof(*resp), 0, __func__);

	/* A successful DHCP offload enable command may require a state update */
	if (ret == 0 && le32_to_cpu(dhcp_req->opcode) == MORSE_CMD_DHCP_OPCODE_ENABLE) {
		if (le32_to_cpu(dhcp_resp->retcode) == MORSE_CMD_DHCP_RETCODE_SUCCESS) {
			mors->custom_configs.enable_dhcpc_offload = true;
			MORSE_DBG(mors, "%s: dhcp client hw offload enabled\n", __func__);
		}
	}
	return ret;
}

static int morse_cmd_vendor_force_power_mode(struct morse *mors, struct morse_cmd_resp_vendor *resp,
					     const struct morse_cmd_req_vendor *req)
{
	int ret;
	struct morse_cmd_req_force_power_mode *force_pm =
		(struct morse_cmd_req_force_power_mode *)req;

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)resp, (struct morse_cmd_req *)req,
			   sizeof(*resp), 0, __func__);

	if (!ret && le32_to_cpu(force_pm->mode) == MORSE_CMD_POWER_MODE_HIBERNATE) {
		/* Once forced into hibernation only a digital reset can recover the device */
		pr_warn("%s has been forced into hibernation\n",
			mors->cfg->get_hw_version(mors->chip_id));
		morse_watchdog_pause(mors);
		set_bit(MORSE_STATE_FLAG_CHIP_UNRESPONSIVE, &mors->state_flags);
	}

	return ret;
}

static int morse_cmd_vendor_standby(struct morse *mors,
				struct morse_cmd_resp_vendor *resp,
				const struct morse_cmd_req_vendor *req)
{
	int ret;
	struct morse_cmd_req_standby_mode *standby_mode =
		(struct morse_cmd_req_standby_mode *)req;
	struct morse_cmd_resp_standby_mode *standby_mode_resp =
		(struct morse_cmd_resp_standby_mode *)resp;

	if (le32_to_cpu(standby_mode->cmd) == MORSE_CMD_STANDBY_MODE_ENTER) {
		/* Validate hw scan config prior to entering standby */
		if (hw_scan_is_supported(mors)) {
			if (!mors->hw_scan.params)
				return -EINVAL;
			else if (!hw_scan_is_idle(mors))
				return -EBUSY;
			else if (!morse_mac_is_sta_vif_associated(mors->hw_scan.params->vif) &&
				 !hw_scan_saved_config_has_ssid(mors))
				return -EINVAL;

			ret = morse_cmd_hw_scan(mors, mors->hw_scan.params, true, NULL);

			if (ret)
				goto exit;
		}
	}

	/* Clear the flag that blocks host to firmware TX, before sending standby exit
	 * command to firmware or else the exit command itself will get blocked.
	 */
	if (le32_to_cpu(standby_mode->cmd) == MORSE_CMD_STANDBY_MODE_EXIT &&
		test_bit(MORSE_STATE_FLAG_HOST_TO_CHIP_CMD_BLOCKED, &mors->state_flags))
		clear_bit(MORSE_STATE_FLAG_HOST_TO_CHIP_CMD_BLOCKED, &mors->state_flags);

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)resp,
			(struct morse_cmd_req *)req, sizeof(*resp), 0, __func__);

	if (ret)
		goto exit;

	if (le32_to_cpu(standby_mode->cmd) == MORSE_CMD_STANDBY_MODE_ENTER) {
		if (hw_scan_is_supported(mors)) {
			set_bit(MORSE_STATE_FLAG_HOST_TO_CHIP_TX_BLOCKED, &mors->state_flags);
			set_bit(MORSE_STATE_FLAG_HOST_TO_CHIP_CMD_BLOCKED, &mors->state_flags);
		}
		morse_watchdog_pause(mors);
	} else if (le32_to_cpu(standby_mode->cmd) == MORSE_CMD_STANDBY_MODE_EXIT) {
		clear_bit(MORSE_STATE_FLAG_HOST_TO_CHIP_TX_BLOCKED, &mors->state_flags);
		clear_bit(MORSE_STATE_FLAG_HOST_TO_CHIP_CMD_BLOCKED, &mors->state_flags);
		morse_watchdog_resume(mors);
	}

	if (!ret && le32_to_cpu(standby_mode->cmd) == MORSE_CMD_STANDBY_MODE_EXIT) {
		u16 vif_id = le16_to_cpu(standby_mode_resp->hdr.vif_id);
		struct morse_cmd_standby_mode_exit *exit = &standby_mode_resp->info;
		bool valid_response = (le16_to_cpu(standby_mode_resp->hdr.len) ==
				(sizeof(*standby_mode_resp) - sizeof(standby_mode_resp->hdr)));
		struct ieee80211_vif *vif = morse_get_vif_from_vif_id(mors, vif_id);

		if (valid_response)
			MORSE_ERR(mors, "%s: Standby exited - reason: '%s', STA state %d\n",
				__func__, morse_cmd_standby_exit_reason_to_str(exit->reason),
				exit->sta_state);

		if (vif && vif->type == NL80211_IFTYPE_STATION &&
		    morse_mac_is_sta_vif_associated(vif) &&
		    valid_response && exit->sta_state < IEEE80211_STA_ASSOC)
			ieee80211_connection_loss(vif);
	}

exit:
	return ret;
}

/**
 * morse_cmd_get_set_non_tim_mode() - Get or Set the non-TIM mode
 *
 * @vif: Pointer to virtual interface
 * @resp: Response for the command
 * @req: Command request to get or set
 * @is_set_cmd: Value indicating set/get command, 1 for set command and 0 for get command.
 *
 * @return Error code on failure, 0 otherwise
 */
static int morse_cmd_get_set_non_tim_mode(struct ieee80211_vif *vif,
					   struct morse_cmd_resp_get_set_generic_param *resp,
					   const struct morse_cmd_req_get_set_generic_param *req,
					   bool is_set_cmd)
{
	int ret = 0;
	struct morse_vif *mors_vif;

	if (!vif) {
		ret = -EFAULT;
		goto exit;
	}

	if (vif->type != NL80211_IFTYPE_STATION && vif->type != NL80211_IFTYPE_AP) {
		ret = -EFAULT;
		goto exit;
	}
	mors_vif = ieee80211_vif_to_morse_vif(vif);

	if (is_set_cmd) {
		/* Do not allow disabling non-TIM mode when STAs are associated */
		if (vif->type == NL80211_IFTYPE_AP &&
				le32_to_cpu(req->value) == 0 && mors_vif->ap->num_stas) {
			ret = -EPERM;
			goto exit;
		}
		mors_vif->enable_non_tim_mode = (le32_to_cpu(req->value) == 1);
		if (mors_vif->enable_non_tim_mode &&
			MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, NON_TIM))
			mors_vif->s1g_cap_ie.capab_info[4] |= S1G_CAP4_NON_TIM;
		else
			mors_vif->s1g_cap_ie.capab_info[4] &= ~S1G_CAP4_NON_TIM;
	} else {
		resp->value = cpu_to_le32(mors_vif->enable_non_tim_mode);
	}

exit:
	return ret;
}

/**
 * morse_cmd_vendor_get_params() - Get the parameter value
 *
 * @mors: Global Morse struct
 * @vif: Pointer to virtual interface
 * @req: Command request to get
 * @resp: Response for the command
 *
 * @return Error code on failure, 0 otherwise
 */
static int morse_cmd_vendor_get_params(struct morse *mors, struct ieee80211_vif *vif,
				       struct morse_cmd_resp_get_set_generic_param *resp,
				       const struct morse_cmd_req_get_set_generic_param *req)
{
	int ret = 0;

	switch (le32_to_cpu(req->param_id)) {
	case MORSE_CMD_PARAM_ID_NON_TIM_MODE:
		ret = morse_cmd_get_set_non_tim_mode(vif, resp, req, false);
		break;
	case MORSE_CMD_PARAM_ID_HOME_CHANNEL_DWELL_MS:
		resp->value = cpu_to_le32(mors->hw_scan.home_dwell_ms);
		break;
	default:
		resp->value = cpu_to_le32(-1);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * morse_cmd_vendor_set_params() - Set the parameter value
 *
 * @mors: Global Morse struct
 * @vif: Pointer to virtual interface
 * @req: Command request to set
 * @resp: Response for the command
 *
 * @return Error code on failure, 0 otherwise
 */
static int morse_cmd_vendor_set_params(struct morse *mors, struct ieee80211_vif *vif,
				       struct morse_cmd_resp_get_set_generic_param *resp,
				       const struct morse_cmd_req_get_set_generic_param *req)
{
	int ret = 0;

	switch (le32_to_cpu(req->param_id)) {
	case MORSE_CMD_PARAM_ID_NON_TIM_MODE:
		ret = morse_cmd_get_set_non_tim_mode(vif, resp, req, true);
		break;
	case MORSE_CMD_PARAM_ID_HOME_CHANNEL_DWELL_MS:
		mors->hw_scan.home_dwell_ms = le32_to_cpu(req->value);
		MORSE_INFO(mors, "Set home channel dwell to %d ms\n", mors->hw_scan.home_dwell_ms);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * morse_cmd_vendor_get_set_params() - Get or set parameter value in driver or firmware
 *
 * @mors:	Global Morse struct
 * @vif:	Pointer to virtual interface
 * @req:	Command request to get or set
 * @resp:	Response for the command
 */
static int morse_cmd_vendor_get_set_params(struct morse *mors, struct ieee80211_vif *vif,
					   struct morse_cmd_resp_get_set_generic_param *resp,
					   const struct morse_cmd_req_get_set_generic_param *req)
{
	int ret = 0;
	bool is_driver_param = false;

	switch (le32_to_cpu(req->param_id)) {
	case MORSE_CMD_PARAM_ID_NON_TIM_MODE:
	case MORSE_CMD_PARAM_ID_HOME_CHANNEL_DWELL_MS:
		is_driver_param = true;
		if (le32_to_cpu(req->action) == MORSE_CMD_PARAM_ACTION_GET)
			ret = morse_cmd_vendor_get_params(mors, vif, resp, req);
		else if (le32_to_cpu(req->action) == MORSE_CMD_PARAM_ACTION_SET)
			ret = morse_cmd_vendor_set_params(mors, vif, resp, req);
		break;
	default:
		/* Get or set command for the firmware */
		ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)resp,
			(struct morse_cmd_req *)req, sizeof(*resp), 0, __func__);
		break;
	}

	if (!ret && is_driver_param) {
		resp->hdr.len = cpu_to_le16(sizeof(*resp) - sizeof(resp->hdr));
		resp->hdr.host_id = req->hdr.host_id;
		resp->status = cpu_to_le32(ret);
	}

	return ret;
}

/**
 * morse_cmd_vendor_postprocess() - Post process vendor command after it was handled by the chip.
 */
static void morse_cmd_vendor_postprocess(struct morse *mors, struct morse_vif *mors_vif,
					 const struct morse_cmd_req_vendor *req,
					 struct morse_cmd_resp_vendor *resp)
{
	u16 message_id = le16_to_cpu(req->hdr.message_id);
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);

	switch (message_id) {
	case MORSE_CMD_ID_SET_CONTROL_RESPONSE:
		{
			struct morse_cmd_req_set_control_response *cr_req =
			    (struct morse_cmd_req_set_control_response *)req;

			if (mors_vif) {
				if (cr_req->direction)
					mors_vif->ctrl_resp_in_1mhz_en =
						cr_req->control_response_1mhz_en;
				else
					mors_vif->ctrl_resp_out_1mhz_en =
						cr_req->control_response_1mhz_en;
			}
		}
		break;
	case MORSE_CMD_ID_SET_BSS_COLOR:
		{
			struct morse_cmd_req_set_bss_color *color_cmd =
			    (struct morse_cmd_req_set_bss_color *)req;

			if (mors_vif)
				mors_vif->bss_color = color_cmd->bss_color;
		}
		break;
	case MORSE_CMD_ID_SET_LONG_SLEEP_CONFIG:
		{
			struct morse_cmd_req_set_long_sleep_config *long_sleep_cmd =
			    (struct morse_cmd_req_set_long_sleep_config *)req;

			if (long_sleep_cmd->enabled)
				(void)morse_watchdog_pause(mors);
			else
				(void)morse_watchdog_resume(mors);
		}
		break;
	case MORSE_CMD_ID_CTS_SELF_PS:
		{
			struct morse_cmd_req_cts_self_ps *cts_self_ps =
			    (struct morse_cmd_req_cts_self_ps *)req;

			WARN_ON_ONCE(!mors_vif);
			if (mors_vif && vif->type == NL80211_IFTYPE_AP)
				morse_set_dtim_cts_to_self(cts_self_ps->enable, mors_vif);
		}
		break;
	case MORSE_CMD_ID_GET_SET_GENERIC_PARAM:
		{
			struct morse_cmd_req_get_set_generic_param *get_set_cmd =
			    (struct morse_cmd_req_get_set_generic_param *)req;
			struct morse_cmd_resp_get_set_generic_param *get_set_resp =
			    (struct morse_cmd_resp_get_set_generic_param *)resp;

			if (le32_to_cpu(get_set_cmd->param_id) ==
					MORSE_CMD_PARAM_ID_EXTRA_ACK_TIMEOUT_ADJUST_US) {
				if (get_set_cmd->action == MORSE_CMD_PARAM_ACTION_SET)
					mors->extra_ack_timeout_us =
						le32_to_cpu(get_set_cmd->value);
				else if (le32_to_cpu(get_set_cmd->action) ==
					MORSE_CMD_PARAM_ACTION_GET)
					mors->extra_ack_timeout_us =
						le32_to_cpu(get_set_resp->value);
			}
		}
		break;
	case MORSE_CMD_ID_GET_TSF:
		{
			struct morse_cmd_resp_get_tsf *get_tsf_resp =
				(struct morse_cmd_resp_get_tsf *)resp;

			if (mors_vif)
				MORSE_DBG(mors, "vif[%u] tsf:%llu (hw clock:%llu)\n",
					  mors_vif->id, get_tsf_resp->now_tsf,
					  get_tsf_resp->now_chip_ts);
		}
		break;
	}
}

/* Handlers for driver commands which are common between softmac and fullmac. */
static int morse_cmd_drv_common(struct morse *mors, struct morse_vif *mors_vif,
				struct morse_cmd_resp *resp, struct morse_cmd_req *req)
{
	int ret;

	switch (le16_to_cpu(req->hdr.message_id)) {
	case MORSE_CMD_ID_COREDUMP:
		ret = morse_cmd_coredump(mors);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	case MORSE_CMD_ID_GET_HW_VERSION:
		ret = morse_cmd_get_hw_version(mors, resp);
		resp->status = cpu_to_le32(ret);
		break;
	case MORSE_CMD_ID_VENDOR_IE_CONFIG:
		ret = morse_vendor_ie_handle_config_cmd(mors_vif,
							(struct morse_cmd_req_vendor_ie_config *)
							req);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	default:
		ret = -ENOTSUPP;
	}
	return ret;
}

static int morse_cmd_drv_fullmac(struct morse *mors, struct morse_vif *mors_vif,
				 struct morse_cmd_resp *resp, struct morse_cmd_req *req)
{
	int ret;

	switch (le16_to_cpu(req->hdr.message_id)) {
	case MORSE_CMD_ID_SET_AMPDU:
		ret = morse_cmd_tx(mors, resp, req, sizeof(*resp), 0, __func__);
		break;
	case MORSE_CMD_ID_DRIVER_SET_DUTY_CYCLE:
		req->hdr.message_id = cpu_to_le16(MORSE_CMD_ID_SET_DUTY_CYCLE);
		ret = morse_cmd_tx(mors, resp, req, le16_to_cpu(resp->hdr.len), 0, __func__);
		morse_cmd_resp_init(resp, 4, ret);
		break;
	default:
		/* TODO: Implement required driver cmd for FullMAC */
		ret = -ENOTSUPP;
		morse_cmd_resp_init(resp, 4, ret);
	}

	return ret;
}

int morse_cmd_vendor(struct morse *mors, struct morse_vif *mors_vif,
		     const struct morse_cmd_req_vendor *req, int cmd_len,
		     struct morse_cmd_resp_vendor *resp, int *resp_len)
{
	int ret;
	u16 message_id;

	resp->hdr.message_id = req->hdr.message_id;
	message_id = le16_to_cpu(req->hdr.message_id);

	if (message_id >= MORSE_CMD_ID_DRIVER_START &&
	    message_id <= MORSE_CMD_ID_DRIVER_END) {
		ret = morse_cmd_drv_common(mors, mors_vif, (struct morse_cmd_resp *)resp,
					   (struct morse_cmd_req *)req);
		if (ret == -ENOTSUPP && is_fullmac_mode()) {
			ret = morse_cmd_drv_fullmac(mors, mors_vif, (struct morse_cmd_resp *)resp,
						    (struct morse_cmd_req *)req);
		} else if (ret == -ENOTSUPP) {
			struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);

			ret = morse_cmd_drv(mors, vif, (struct morse_cmd_resp *)resp,
					    (struct morse_cmd_req *)req, sizeof(*resp), 0);
			if (ret)
				MORSE_ERR(mors, "%s error %d\n", __func__, ret);
		}
	} else if (!is_fullmac_mode() && message_id == MORSE_CMD_ID_SET_CHANNEL) {
		ret = morse_cmd_vendor_set_channel(mors, cmd_len,
				(struct morse_cmd_resp_set_channel *)resp,
				(struct morse_cmd_req_set_channel *)req);
	} else if (message_id == MORSE_CMD_ID_STANDBY_MODE) {
		ret = morse_cmd_vendor_standby(mors, resp, req);
	} else if (message_id == MORSE_CMD_ID_FORCE_POWER_MODE) {
		ret = morse_cmd_vendor_force_power_mode(mors, resp, req);
	} else if (!is_fullmac_mode() && message_id == MORSE_CMD_ID_GET_SET_GENERIC_PARAM) {
		struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);

		ret = morse_cmd_vendor_get_set_params(mors, vif,
			(struct morse_cmd_resp_get_set_generic_param *)resp,
			(struct morse_cmd_req_get_set_generic_param *)req);
	} else if (message_id == MORSE_CMD_ID_DHCP_OFFLOAD) {
		ret = morse_cmd_vendor_dhcp_offload(mors, resp, req);
	} else {
		ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)resp,
				   (struct morse_cmd_req *)req, sizeof(*resp), 0, __func__);
	}
	if (ret) {
		resp->hdr.host_id = req->hdr.host_id;
		resp->status = cpu_to_le32(ret);
		*resp_len = sizeof(struct morse_cmd_resp);
		goto exit;
	}
	*resp_len = le16_to_cpu(resp->hdr.len) + sizeof(struct morse_cmd_header);

	if (!is_fullmac_mode())
		morse_cmd_vendor_postprocess(mors, mors_vif, req, resp);

exit:
	return ret;
}

int morse_hw_cmd_vendor(struct morse *mors,
			const struct morse_cmd_req_vendor *req, int cmd_len,
			struct morse_cmd_resp_vendor *resp, int *resp_len)
{
	int ret;

	resp->hdr.message_id = req->hdr.message_id;

	if (le16_to_cpu(req->hdr.message_id) == MORSE_CMD_ID_COREDUMP) {
		ret = morse_cmd_drv_common(mors, NULL, (struct morse_cmd_resp *)resp,
					   (struct morse_cmd_req *)req);
	} else {
		/* Command not supported yet */
		ret = -ENOTSUPP;
	}
	if (ret) {
		resp->hdr.host_id = req->hdr.host_id;
		resp->status = cpu_to_le32(ret);
		*resp_len = sizeof(struct morse_cmd_resp);
		goto exit;
	}
	*resp_len = le16_to_cpu(resp->hdr.len) + sizeof(struct morse_cmd_header);

exit:
	if (ret)
		MORSE_ERR(mors, "%s: command id:%d failed (ret:%d)\n",
			__func__, le16_to_cpu(req->hdr.message_id), ret);
	return ret;
}

/* Sets the control response frame bandwidth for the given vif */
int morse_cmd_set_cr_bw(struct morse *mors, struct morse_vif *mors_vif, u8 direction, u8 cr_1mhz_en)
{
	int ret;
	struct morse_cmd_req_set_control_response req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_CONTROL_RESPONSE, mors_vif->id,
		       sizeof(req));

	req.control_response_1mhz_en = cr_1mhz_en;
	req.direction = direction;

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_cfg_qos(struct morse *mors, struct morse_queue_params *params)
{
	int ret;
	struct morse_cmd_req_set_qos_params req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_QOS_PARAMS, 0, sizeof(req));

	req.uapsd = params->uapsd;
	req.queue_idx = params->aci;
	req.aifs_slot_count = params->aifs;
	req.contention_window_min = cpu_to_le16(params->cw_min);
	req.contention_window_max = cpu_to_le16(params->cw_max);
	req.max_txop_usec = cpu_to_le32(params->txop);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_set_bss_color(struct morse *mors, struct morse_vif *mors_vif, u8 color)
{
	int ret;
	struct morse_cmd_req_set_bss_color req;
	struct morse_cmd_resp_set_bss_color resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_BSS_COLOR, mors_vif->id, sizeof(req));

	req.bss_color = color;

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);

	return ret;
}

int morse_cmd_health_check(struct morse *mors)
{
	int ret;
	struct morse_cmd_req_health_check req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_HEALTH_CHECK, 0, sizeof(req));

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0,
			   default_cmd_timeout_ms > MM_CMD_HEALTH_CHECK_TIMEOUT_MS ?
			   default_cmd_timeout_ms : MM_CMD_HEALTH_CHECK_TIMEOUT_MS, __func__);

	return ret;
}

int morse_cmd_arp_offload_update_ip_table(struct morse *mors, u16 vif_id,
					  int arp_addr_count, __be32 *arp_addr_list)
{
	int ret = 0;
	int i;
	struct morse_cmd_req_arp_offload req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_ARP_OFFLOAD, vif_id, sizeof(req));

	memset(req.ip_table, 0, sizeof(req.ip_table));

	for (i = 0; i < arp_addr_count && i < ARRAY_SIZE(req.ip_table); i++)
		req.ip_table[i] = arp_addr_list[i];

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_get_capabilities(struct morse *mors, u16 vif_id, struct morse_caps *capabilities)
{
	int ret = 0;
	int i;
	struct morse_cmd_req_get_capabilities req;
	struct morse_cmd_resp_get_capabilities rsp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_CAPABILITIES, vif_id, sizeof(req));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&rsp,
			   (struct morse_cmd_req *)&req, sizeof(rsp), 0, __func__);
	if (ret != 0)
		return ret;

	capabilities->ampdu_mss = rsp.capabilities.ampdu_mss;
	capabilities->morse_mmss_offset = rsp.morse_mmss_offset;
	capabilities->beamformee_sts_capability = rsp.capabilities.beamformee_sts_capability;
	capabilities->maximum_ampdu_length_exponent =
	    rsp.capabilities.maximum_ampdu_length_exponent;
	capabilities->number_sounding_dimensions = rsp.capabilities.number_sounding_dimensions;
	for (i = 0; i < FW_CAPABILITIES_FLAGS_WIDTH; i++)
		capabilities->flags[i] = le32_to_cpu(rsp.capabilities.flags[i]);

	return ret;
}

int morse_cmd_config_non_tim_mode(struct morse *mors, bool enable, u16 vif_id)
{
	int ret = 0;
	struct morse_cmd_req_get_set_generic_param req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_SET_GENERIC_PARAM, vif_id, sizeof(req));
	req.param_id = cpu_to_le32(MORSE_CMD_PARAM_ID_NON_TIM_MODE);
	req.action = cpu_to_le32(MORSE_CMD_PARAM_ACTION_SET);
	req.value = cpu_to_le32(enable);
	req.flags = 0;

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_enable_li_sleep(struct morse *mors, u16 listen_interval, u16 vif_id)
{
	int ret;
	struct morse_cmd_req_li_sleep req;

	if (listen_interval == 0)
		return -EINVAL;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_LI_SLEEP, vif_id, sizeof(req));
	req.listen_interval = cpu_to_le32(listen_interval);
	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_dhcpc_enable(struct morse *mors, u16 vif_id)
{
	int ret;
	struct morse_cmd_req_dhcp_offload req;
	struct morse_cmd_resp_dhcp_offload resp;

	if (vif_id == (u16)-1)
		return -ENODEV;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_DHCP_OFFLOAD, vif_id, sizeof(req));

	req.opcode = cpu_to_le32(MORSE_CMD_DHCP_OPCODE_ENABLE);

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);

	if (ret == 0) {
		if (le32_to_cpu(resp.retcode) == MORSE_CMD_DHCP_RETCODE_SUCCESS) {
			MORSE_INFO(mors, "In chip DHCP client enabled\n");
		} else if (le32_to_cpu(resp.retcode) == MORSE_CMD_DHCP_RETCODE_ALREADY_ENABLED) {
			/* Client is already enabled, trigger a lease update. */
			MORSE_INFO(mors, "DHCP client already enabled, forcing lease update\n");
			req.opcode = cpu_to_le32(MORSE_CMD_DHCP_OPCODE_SEND_LEASE_UPDATE);
			ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
					   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);
		} else {
			MORSE_WARN(mors, "Error enabling in-chip DHCP client %d\n",
				   le32_to_cpu(resp.retcode));
		}
	}

	return ret;
}

static int morse_cmd_twt_agreement_req(struct morse *mors,
				       struct morse_twt_agreement_data *agreement,
				       u16 iface_id, bool validate_only)
{
	int ret = 0;
	struct morse_cmd_req_twt_agreement_install *req;
	enum morse_cmd_id message_id;

	req = kmalloc(sizeof(*req) + MORSE_CMD_DOT11_TWT_AGREEMENT_MAX_LEN, GFP_KERNEL);

	if (validate_only)
		message_id = MORSE_CMD_ID_TWT_AGREEMENT_VALIDATE;
	else
		message_id = MORSE_CMD_ID_TWT_AGREEMENT_INSTALL;
	morse_cmd_init(mors, &req->hdr, message_id, iface_id,
		       sizeof(*req) + MORSE_CMD_DOT11_TWT_AGREEMENT_MAX_LEN);

	req->flow_id = (le16_to_cpu(agreement->params.req_type) & IEEE80211_TWT_REQTYPE_FLOWID) >>
	    IEEE80211_TWT_REQTYPE_FLOWID_OFFSET;
	req->agreement_len = morse_twt_initialise_agreement(agreement, req->agreement);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)req, 0, 0, __func__);
	kfree(req);
	return ret;
}

int morse_cmd_twt_agreement_validate_req(struct morse *mors,
					 struct morse_twt_agreement_data *agreement, u16 iface_id)
{
	return morse_cmd_twt_agreement_req(mors, agreement, iface_id, true);
}

int morse_cmd_twt_agreement_install_req(struct morse *mors,
					struct morse_twt_agreement_data *agreement, u16 iface_id)
{
	return morse_cmd_twt_agreement_req(mors, agreement, iface_id, false);
}

int morse_cmd_twt_remove_req(struct morse *mors,
			     struct morse_cmd_req_twt_agreement_remove *twt_remove_req,
			     u16 iface_id)
{
	morse_cmd_init(mors, &twt_remove_req->hdr, MORSE_CMD_ID_TWT_AGREEMENT_REMOVE,
		       iface_id, sizeof(*twt_remove_req));

	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)twt_remove_req, 0, 0, __func__);
}

int morse_cmd_cfg_ibss(struct morse *mors, u16 vif_id,
		       const u8 *bssid, bool ibss_creator, bool stop_ibss)
{
	int ret;
	struct morse_cmd_req_ibss_config req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_IBSS_CONFIG, vif_id, sizeof(req));

	/* If stop_ibss is set, other parameters are ignored and cleared in target */
	memcpy(req.ibss_bssid, bssid, sizeof(req.ibss_bssid));

	if (stop_ibss) {
		req.ibss_cfg_opcode = MORSE_CMD_IBSS_CONFIG_OPCODE_STOP;
	} else {
		if (ibss_creator)
			req.ibss_cfg_opcode = MORSE_CMD_IBSS_CONFIG_OPCODE_CREATE;
		else
			req.ibss_cfg_opcode = MORSE_CMD_IBSS_CONFIG_OPCODE_JOIN;
	}

	req.ibss_probe_filtering = enable_ibss_probe_filtering;

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_cfg_offset_tsf(struct morse *mors, u16 vif_id, s64 offset_tsf)
{
	int ret;
	struct morse_cmd_req_set_offset_tsf req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_OFFSET_TSF, vif_id, sizeof(req));

	req.offset_tsf = cpu_to_le64(offset_tsf);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_set_duty_cycle(struct morse *mors, enum morse_cmd_duty_cycle_mode mode,
			     int duty_cycle, bool omit_ctrl_resp)
{
	int ret;
	struct morse_cmd_req_set_duty_cycle req;
	struct morse_cmd_resp_get_duty_cycle resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_DUTY_CYCLE, 0, sizeof(req));

	req.config.duty_cycle = cpu_to_le32(duty_cycle);
	req.config.omit_control_responses = omit_ctrl_resp ? 1 : 0;
	req.config_ext.mode = mode;
	req.set_cfgs =
	    MORSE_CMD_DUTY_CYCLE_SET_CFG_DUTY_CYCLE |
	    MORSE_CMD_DUTY_CYCLE_SET_CFG_OMIT_CONTROL_RESP | MORSE_CMD_DUTY_CYCLE_SET_CFG_EXT;

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);

	return ret;
}

int morse_cmd_set_mpsw(struct morse *mors, int min, int max, int window)
{
	int ret;
	struct morse_cmd_req_mpsw_config req;
	struct morse_cmd_resp_mpsw_config resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_MPSW_CONFIG, 0, sizeof(req));

	req.config.airtime_max_us = cpu_to_le32(max);
	req.config.airtime_min_us = cpu_to_le32(min);
	req.config.packet_space_window_length_us = cpu_to_le32(window);
	req.config.enable = (max > 0 && min > 0);
	req.set_cfgs = MORSE_CMD_SET_MPSW_CFG_AIRTIME_BOUNDS |
	    MORSE_CMD_SET_MPSW_CFG_PKT_SPC_WIN_LEN | MORSE_CMD_SET_MPSW_CFG_ENABLED;

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);

	return ret;
}

int morse_cmd_get_available_channels(struct morse *mors, struct morse_cmd_resp *resp)
{
	struct morse_cmd_resp_get_available_channels *channel_resp =
	    (struct morse_cmd_resp_get_available_channels *)resp;
	int num_entries;

	/* morse_channel and morse_cmd_channel_info should have the same memory layout */
	BUILD_BUG_ON(sizeof(struct morse_channel) != sizeof(channel_resp->channels[0]));

	num_entries =
		morse_dot11ah_fill_channel_list((struct morse_channel *)channel_resp->channels);

	if (num_entries < 0) {
		resp->hdr.len = cpu_to_le16(4);
		return num_entries;
	}

	channel_resp->num_channels = cpu_to_le32(num_entries);

	resp->hdr.len = cpu_to_le16(sizeof(*channel_resp) - sizeof(channel_resp->hdr) -
			sizeof(channel_resp->channels) +
			(num_entries * sizeof(channel_resp->channels[0])));

	return 0;
}

int morse_cmd_get_hw_version(struct morse *mors, struct morse_cmd_resp *resp)
{
	struct morse_cmd_resp_get_hw_version *hw_resp =
		(struct morse_cmd_resp_get_hw_version *)resp;
	struct morse_hw_cfg *cfg = mors->cfg;
	const char *hw_version = "n/a";
	int ret;

	if (!cfg)
		return -ENXIO;

	memset(&hw_resp->hw_version, 0x0, sizeof(hw_resp->hw_version));

	if (mors->cfg->get_hw_version)
		hw_version = mors->cfg->get_hw_version(mors->chip_id);

	ret = strscpy(hw_resp->hw_version, hw_version, sizeof(hw_resp->hw_version));

	if (ret == -E2BIG)
		MORSE_WARN(mors, "Hardware version string truncated\n");

	resp->hdr.len = cpu_to_le16(sizeof(*hw_resp) - sizeof(hw_resp->hdr));

	return 0;
}

int morse_cmd_set_frag_threshold(struct morse *mors, u32 frag_threshold)
{
	int ret;
	struct morse_cmd_req_get_set_generic_param req = {
		.param_id = cpu_to_le32(MORSE_CMD_PARAM_ID_FRAGMENT_THRESHOLD),
		.action = cpu_to_le32(MORSE_CMD_PARAM_ACTION_SET),
		.value = cpu_to_le32(frag_threshold),
	};
	struct morse_cmd_resp_get_set_generic_param resp;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_SET_GENERIC_PARAM, 0, sizeof(req));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&resp,
			   (struct morse_cmd_req *)&req, sizeof(resp), 0, __func__);

	return ret;
}

int morse_cmd_cfg_mesh(struct morse *mors, struct morse_vif *mors_vif, bool stop_mesh,
		       bool mesh_beaconing)
{
	int ret;
	struct morse_cmd_req_mesh_config req;
	struct morse_mesh *mesh = mors_vif->mesh;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_MESH_CONFIG, mors_vif->id, sizeof(req));

	if (stop_mesh)
		req.mesh_cfg_opcode = MORSE_CMD_MESH_CONFIG_OPCODE_STOP;
	else
		req.mesh_cfg_opcode = MORSE_CMD_MESH_CONFIG_OPCODE_START;

	req.enable_beaconing = mesh_beaconing;
	req.mbca_config = mesh->mbca.config;
	if (mesh_beaconing) {
		req.min_beacon_gap_ms = mesh->mbca.min_beacon_gap_ms;
		req.tbtt_adj_timer_interval_ms =
			cpu_to_le16(mesh->mbca.tbtt_adj_interval_ms);
		req.mbss_start_scan_duration_ms =
			cpu_to_le16(mesh->mbca.mbss_start_scan_duration_ms);
	}
	MORSE_INFO(mors, "%s: cfg=0x%02x, gap=%u, tbtt interval=%u start scan duration=%u\n",
		   __func__,
		   mesh->mbca.config, mesh->mbca.min_beacon_gap_ms, mesh->mbca.tbtt_adj_interval_ms,
		   mesh->mbca.mbss_start_scan_duration_ms);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_ack_timeout_adjust(struct morse *mors, u16 vif_id, u32 timeout_us)
{
	int ret;
	struct morse_cmd_req_get_set_generic_param req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_SET_GENERIC_PARAM, vif_id, sizeof(req));

	req.param_id = cpu_to_le32(MORSE_CMD_PARAM_ID_EXTRA_ACK_TIMEOUT_ADJUST_US);
	req.action = cpu_to_le32(MORSE_CMD_PARAM_ACTION_SET);
	req.value = cpu_to_le32(timeout_us);
	req.flags = 0;

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	/* Store ack timeout adjust as it's used in a vendor WAR */
	if (ret == 0)
		mors->extra_ack_timeout_us = timeout_us;

	return ret;
}

int morse_cmd_pv1_set_rx_ampdu_state(struct morse_vif *mors_vif, u8 *sta_addr, u8 tid,
		u16 buf_size, bool ba_session_enable)
{
	int ret;
	struct morse_cmd_req_pv1_rx_ampdu_config req;
	struct morse *mors = morse_vif_to_morse(mors_vif);

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_PV1_RX_AMPDU_CONFIG,
			mors_vif->id, sizeof(req));

	req.tid = tid;
	req.buf_size = cpu_to_le16(buf_size);
	req.ba_session_enable = ba_session_enable;
	memcpy(req.addr, sta_addr, ETH_ALEN);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_configure_page_slicing(struct morse_vif *mors_vif, bool enable)
{
	int ret;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct morse_cmd_req_page_slicing_config req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_PAGE_SLICING_CONFIG,
		mors_vif->id, sizeof(req));

	req.enable = enable;

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_hw_scan(struct morse *mors, struct morse_hw_scan_params *params, bool store,
		      struct cfg80211_sched_scan_request *sched_req)
{
	int ret;
	struct morse_cmd_req_hw_scan *req;
	size_t cmd_size;
	u8 *buf;
	u32 flags = 0;

	cmd_size = morse_hw_scan_get_command_size(params, sched_req);
	cmd_size = ROUND_BYTES_TO_WORD(cmd_size);

	req = kzalloc(cmd_size, GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	buf = req->variable;

	if (store)
		flags = MORSE_CMD_HW_SCAN_FLAGS_STORE;
	else if (params->operation == MORSE_HW_SCAN_OP_START)
		flags |= MORSE_CMD_HW_SCAN_FLAGS_START;
	else if (params->operation == MORSE_HW_SCAN_OP_STOP)
		flags |= MORSE_CMD_HW_SCAN_FLAGS_ABORT;

	if (params->operation == MORSE_HW_SCAN_OP_SCHED_START)
		flags |= MORSE_CMD_HW_SCAN_FLAGS_SCHED_START;
	else if (params->operation == MORSE_HW_SCAN_OP_SCHED_STOP)
		flags |= MORSE_CMD_HW_SCAN_FLAGS_SCHED_STOP;

	if (params->survey)
		flags |= MORSE_CMD_HW_SCAN_FLAGS_SURVEY;

	if (params->use_1mhz_probes)
		flags |= MORSE_CMD_HW_SCAN_FLAGS_1MHZ_PROBES;

	if (params->operation == MORSE_HW_SCAN_OP_START ||
			params->operation == MORSE_HW_SCAN_OP_SCHED_START) {
		req->dwell_time_ms = cpu_to_le32(params->dwell_time_ms);
		buf = morse_hw_scan_insert_tlvs(params, buf, sched_req);
	}

	req->flags = cpu_to_le32(flags);
	morse_cmd_init(mors, &req->hdr, MORSE_CMD_ID_HW_SCAN, 0, buf - (u8 *)req);
	morse_hw_scan_dump_scan_cmd(mors, req);
	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)req, 0, 0, __func__);
	kfree(req);

	return ret;
}

int morse_cmd_set_slow_clock_mode(struct morse *mors, enum morse_cmd_slow_clock_mode mode)
{
	struct morse_cmd_req_get_set_generic_param req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_SET_GENERIC_PARAM, 0, sizeof(req));
	req.param_id = cpu_to_le32(MORSE_CMD_PARAM_ID_SLOW_CLOCK_MODE);
	req.action = cpu_to_le32(MORSE_CMD_PARAM_ACTION_SET);
	req.flags = 0;
	req.value = cpu_to_le32(mode);

	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);
}

int morse_cmd_set_country(struct morse *mors, const char *country_code)
{
	struct morse_cmd_req_get_set_generic_param req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_SET_GENERIC_PARAM, 0, sizeof(req));
	req.param_id = cpu_to_le32(MORSE_CMD_PARAM_ID_COUNTRY);
	req.action = cpu_to_le32(MORSE_CMD_PARAM_ACTION_SET);
	req.flags = 0;
	req.value = cpu_to_le32(country_code[0] | country_code[1] << 8);

	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);
}

int morse_cmd_set_rate_control(struct morse *mors)
{
	struct morse_cmd_req_set_rate_control req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_RATE_CONTROL, 0, sizeof(req));
	req.mcs10_mode = morse_mac_get_mcs10_mode();
	req.mcs_mask = cpu_to_le16(morse_mac_get_mcs_mask());
	req.enable_sgi_rc = mors->custom_configs.enable_sgi_rc;

	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);
}

int morse_cmd_set_fixed_transmission_rate(struct morse *mors, s32 bandwidth_mhz, s32 mcs_index,
					s8 use_sgi, s8 nss_idx)
{
	struct morse_cmd_req_set_transmission_rate req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_TRANSMISSION_RATE, 0, sizeof(req));

	req.enabled = 1;
	req.bandwidth_mhz = cpu_to_le32(bandwidth_mhz);
	req.mcs_index = cpu_to_le32(mcs_index);
	req.nss_idx = nss_idx;
	req.use_sgi = use_sgi;
	req.tx_80211ah_format = cpu_to_le32(-1);
	req.use_traveling_pilots = -1;
	req.use_stbc = -1;
	req.use_ldpc = -1;

	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);
}

int morse_cmd_set_rts_threshold(struct morse *mors, u32 rts_threshold)
{
	struct morse_cmd_req_get_set_generic_param req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_SET_GENERIC_PARAM, 0, sizeof(req));
	req.param_id = cpu_to_le32(MORSE_CMD_PARAM_ID_RTS_THRESHOLD);
	req.action = cpu_to_le32(MORSE_CMD_PARAM_ACTION_SET);
	req.flags = 0;
	req.value = cpu_to_le32(rts_threshold);

	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);
}

int morse_cmd_start_scan(struct morse *mors,
			 u8 n_ssids,
			 const u8 *ssid, size_t ssid_len,
			 const u8 *extra_ies, size_t extra_ies_len, u32 dwell_time_ms)
{
	int ret;
	struct morse_cmd_req_start_scan *req;
	size_t max_ies_size = sizeof(req->extra_ies[0]) * MORSE_CMD_SCAN_EXTRA_IES_MAX_LEN;
	size_t req_size = sizeof(*req) + max_ies_size;

	req = kzalloc(req_size, GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	morse_cmd_init(mors, &req->hdr, MORSE_CMD_ID_START_SCAN, 0, req_size);

	req->dwell_time_ms = cpu_to_le32(dwell_time_ms);
	if (extra_ies_len) {
		extra_ies_len = min(max_ies_size, extra_ies_len);
		memcpy(req->extra_ies, extra_ies, extra_ies_len);
		req->extra_ies_len = cpu_to_le16(extra_ies_len);
	}
	req->n_ssids = n_ssids;
	if (ssid_len) {
		ssid_len = min(sizeof(req->ssid), ssid_len);
		memcpy(req->ssid, ssid, ssid_len);
		req->ssid_len = ssid_len;
	}

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)req, 0, 0, __func__);

	kfree(req);

	return ret;
}

int morse_cmd_abort_scan(struct morse *mors)
{
	struct morse_cmd_req_abort_scan req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_ABORT_SCAN, 0, sizeof(req));

	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);
}

static int morse_cmd_connect_deprecated(struct morse *mors,
					const struct morse_wiphy_connect_params *params)
{
	struct morse_cmd_req_connect_deprecated *req;
	int ret;

	if (params->roam)
		return -EOPNOTSUPP;

	req = kzalloc(sizeof(*req) + params->extra_assoc_ies_len, GFP_KERNEL);

	morse_cmd_init(mors, &req->hdr, MORSE_CMD_ID_CONNECT_DEPRECATED, 0,
	    sizeof(*req) + params->extra_assoc_ies_len);

	req->auth_type = params->auth_type;

	if (params->ssid_len < 1 || params->ssid_len > sizeof(req->ssid)) {
		ret = -EINVAL;
		goto out;
	}
	memcpy(req->ssid, params->ssid, params->ssid_len);
	req->ssid_len = params->ssid_len;

	if (params->sae_pwd_len > sizeof(req->sae_pwd)) {
		ret = -EINVAL;
		goto out;
	}
	memcpy(req->sae_pwd, params->sae_pwd, params->sae_pwd_len);
	req->sae_pwd_len = params->sae_pwd_len;

	if (params->extra_assoc_ies_len > MORSE_CMD_EXTRA_ASSOC_IES_MAX_LEN) {
		ret = -EINVAL;
		goto out;
	}

	if (params->extra_assoc_ies_len) {
		memcpy(req->extra_assoc_ies, params->extra_assoc_ies, params->extra_assoc_ies_len);
		req->extra_assoc_ies_len = cpu_to_le16(params->extra_assoc_ies_len);
	}

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)req, 0, 0, __func__);

out:
	kfree(req);
	return ret;
}

int morse_cmd_connect(struct morse *mors, const struct morse_wiphy_connect_params *params)
{
	struct morse_cmd_req_connect *req;
	size_t req_len;
	u32 flags;
	int ret;

	flags = 0;
	if (params->roam)
		flags |= MORSE_CMD_CONNECT_FLAG_ROAM;

	req_len = morse_wiphy_connect_get_command_size(params);
	req = kzalloc(req_len, GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	morse_cmd_init(mors, &req->hdr, MORSE_CMD_ID_CONNECT, 0, req_len);
	req->flags = cpu_to_le32(flags);
	morse_wiphy_connect_insert_tlvs(req->variable, params);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)req, 0, 0, __func__);

	kfree(req);

	if (ret == MORSE_RET_CMD_NOT_HANDLED)
		/* Fall back to the original command for older firmware. */
		return morse_cmd_connect_deprecated(mors, params);

	return ret;
}

int morse_cmd_disconnect(struct morse *mors)
{
	struct morse_cmd_req_disconnect req;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_DISCONNECT, 0, sizeof(req));

	return morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);
}

int morse_cmd_get_connection_state(struct morse *mors, s8 *signal,
				   u32 *connected_time_s, u8 *dtim_period,
				   u16 *beacon_interval_tu)
{
	struct morse_cmd_req_get_connection_state req;
	struct morse_cmd_resp_get_connection_state cfm;
	s16 signal_from_chip;
	int ret;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_CONNECTION_STATE, 0, sizeof(req));

	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)&cfm, (struct morse_cmd_req *)&req,
			   sizeof(cfm), 0, __func__);
	if (ret)
		return ret;

	/* The chip gives us a signal indication in dBm as int16_t. */
	signal_from_chip = (s16)le16_to_cpu(cfm.rssi);

	if (signal_from_chip >= S8_MIN && signal_from_chip <= S8_MAX)
		*signal = signal_from_chip;
	else
		*signal = 0;

	*connected_time_s = le32_to_cpu(cfm.connected_time_s);

	if (le16_to_cpu(cfm.dtim_period) <= U8_MAX)
		*dtim_period = le16_to_cpu(cfm.dtim_period);
	else
		*dtim_period = U8_MAX;

	*beacon_interval_tu = le16_to_cpu(cfm.beacon_interval_tu);

	return 0;
}

int morse_cmd_set_cqm_rssi(struct morse *mors, u16 vif_id, s32 cqm_rssi_thold, u32 cqm_rssi_hyst)
{
	struct morse_cmd_req_set_cqm_rssi req;
	int ret;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_SET_CQM_RSSI, 0, sizeof(req));
	req.threshold = cpu_to_le32(cqm_rssi_thold);
	req.hysteresis = cpu_to_le32(cqm_rssi_hyst);

	ret = morse_cmd_tx(mors, NULL, (struct morse_cmd_req *)&req, 0, 0, __func__);

	return ret;
}

int morse_cmd_get_apf_capabilities(struct morse *mors, struct morse_vif *mors_vif,
								   u32 *version, u32 *max_len)
{
	int ret;
	struct morse_cmd_req_get_apf_capabilities req;
	struct morse_cmd_resp_get_apf_capabilities *resp;

	resp = kzalloc(sizeof(*resp), GFP_KERNEL);
	if (!resp)
		return -ENOMEM;

	morse_cmd_init(mors, &req.hdr, MORSE_CMD_ID_GET_APF_CAPABILITIES, 0, sizeof(req));
	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)resp, (struct morse_cmd_req *)&req,
					   sizeof(struct morse_cmd_resp_get_apf_capabilities), 0,
					   __func__);
	if (ret) {
		MORSE_ERR(mors, "%s, morse_cmd_tx failed, ret=%d\n", __func__, ret);
		ret = -EINVAL;
		goto exit;
	}
	MORSE_INFO(mors, "%s: APF capabilities: ver=%d, max_len=%d\n", __func__,
			   resp->version, resp->max_length);

	if (!resp->version || !resp->max_length) {
		MORSE_ERR(mors, "%s Invalid APF version or length\n", __func__);
		*max_len = 0;
		ret = -EINVAL;
		goto exit;
	}
	*version = resp->version;
	*max_len = le32_to_cpu(resp->max_length);
exit:
	kfree(resp);
	return ret;
}

int morse_cmd_read_write_apf(struct morse *mors, struct morse_vif *mors_vif, bool write,
			      u16 program_len, u8 *program, u32 offset)
{
	struct morse_cmd_req_read_write_apf *req;
	struct morse_cmd_resp_read_write_apf *resp;
	int ret;
	u16 req_len = sizeof(*req);
	u16 resp_len = sizeof(*resp);

	if (write)
		req_len += program_len;
	else
		resp_len += program_len;

	req_len = ROUND_BYTES_TO_WORD(req_len);
	resp_len = ROUND_BYTES_TO_WORD(resp_len);

	req = kzalloc(req_len, GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	resp = kzalloc(resp_len, GFP_KERNEL);
	if (!resp) {
		kfree(req);
		return -ENOMEM;
	}

	req->write = write;
	if (write) {
		req->program_length = cpu_to_le16(program_len);
		memcpy(req->program, program, program_len);
	} else {
		req->offset = cpu_to_le32(offset);
		req->program_length = cpu_to_le16(program_len);
	}
	morse_cmd_init(mors, &req->hdr, MORSE_CMD_ID_READ_WRITE_APF, 0, req_len);
	ret = morse_cmd_tx(mors, (struct morse_cmd_resp *)resp, (struct morse_cmd_req *)req,
					   resp_len, 0, __func__);
	if (ret) {
		MORSE_ERR(mors, "%s, morse_cmd_tx failed, ret=%d\n", __func__, ret);
		ret = -EINVAL;
	} else if (!write) {
		memcpy(program, resp->program, program_len);
		MORSE_INFO(mors, "%s:read %u bytes\n", __func__, resp->program_length);
	}
	MORSE_INFO(mors, "%s: write=%d, program_len=%d, offset=%d\n", __func__,
			   write, program_len, offset);

	kfree(req);
	kfree(resp);
	return ret;
}
