/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "morse.h"
#include "command.h"

#define MORSE_OCS_AID		(AID_LIMIT + 1)	/* Use an unused AID */

int morse_ocs_cmd_post_process(struct morse_vif *mors_vif,
			       const struct morse_cmd_resp_ocs_driver *resp,
			       const struct morse_cmd_req_ocs *req);
int morse_evt_ocs_done(struct morse_vif *mors_vif, struct morse_cmd_evt_ocs_done *evt);
