/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ocs.h"
#include "raw.h"
#include "vendor.h"
#include "debug.h"

#define MORSE_OCS_DURATION	(32767)
#define MORSE_OCS_RAW_IDX	(RAW_INTERNAL_ID_OFFSET + 1)

/* Does needed post processing after sending the OCS command to the FW. For now, this function
 * configures OCS-specific RAW assignment if enabled
 */
int morse_ocs_cmd_post_process(struct morse_vif *mors_vif,
			       const struct morse_cmd_resp_ocs_driver *drv_resp,
			       const struct morse_cmd_req_ocs *req)
{
	struct morse_raw *raw;
	struct morse_raw_config *config;

	if (!mors_vif->ap)
		return -EFAULT;

	raw = &mors_vif->ap->raw;

	if (ocs_type != MORSE_CMD_OCS_TYPE_RAW || le32_to_cpu(req->subcmd) !=
		MORSE_CMD_OCS_SUBCMD_CONFIG || le32_to_cpu(drv_resp->status))
		return 0;

	mutex_lock(&raw->lock);

	config = morse_raw_create_or_find_by_id(raw, MORSE_OCS_RAW_IDX);

	if (!config) {
		mutex_unlock(&raw->lock);
		return -ENOMEM;
	}

	/* check if its been initialised */
	if (config->slot_definition.slot_duration_us != MORSE_OCS_DURATION) {
		config->type = IEEE80211_S1G_RPS_RAW_TYPE_GENERIC;
		config->start_time_us = 0;
		config->start_aid = le16_to_cpu(req->config.aid);
		config->end_aid = config->start_aid;
		config->start_aid_idx = -1;
		config->end_aid_idx = -1;
		config->slot_definition.num_slots = 1;
		config->slot_definition.slot_duration_us = MORSE_OCS_DURATION;
	}
	/* Set the dynamic beacon index value to default to identify PRAW config as static */
	config->dynamic.insert_at_idx = U16_MAX;

	/* Enable RAW config */
	morse_raw_activate_config(raw, config);

	if (!morse_raw_is_enabled(mors_vif))
		morse_raw_enable(raw);

	mutex_unlock(&raw->lock);

	/* Update RPS IE with new configuration. */
	morse_raw_trigger_update(mors_vif, false);

	return 0;
}

int morse_evt_ocs_done(struct morse_vif *mors_vif, struct morse_cmd_evt_ocs_done *evt)
{
	struct morse_raw *raw;
	int ret;

	if (!mors_vif->ap)
		return -EFAULT;

	raw = &mors_vif->ap->raw;

	if (ocs_type == MORSE_CMD_OCS_TYPE_RAW) {
		struct morse_raw_config *config;

		mutex_lock(&raw->lock);
		config = morse_raw_find_config_by_id(raw, MORSE_OCS_RAW_IDX);
		morse_raw_deactivate_config(raw, config);
		mutex_unlock(&raw->lock);

		/* Update RPS IE with new configuration. */
		morse_raw_trigger_update(mors_vif, false);
	}

	ret = morse_vendor_send_ocs_done_event(morse_vif_to_ieee80211_vif(mors_vif), evt);

	return ret;
}
