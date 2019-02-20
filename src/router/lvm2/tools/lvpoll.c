/*
 * Copyright (C) 2014-2015 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "tools.h"

#include "lib/lvmpolld/polldaemon.h"
#include "pvmove_poll.h"
#include "lvconvert_poll.h"
#include "daemons/lvmpolld/polling_ops.h"

static struct poll_functions _pvmove_fns = {
	.poll_progress = poll_mirror_progress,
	.update_metadata = pvmove_update_metadata,
	.finish_copy = pvmove_finish
};

static struct poll_functions _convert_fns = {
	.poll_progress = poll_mirror_progress,
	.finish_copy = lvconvert_mirror_finish
};

static struct poll_functions _merge_fns = {
	.poll_progress = poll_merge_progress,
	.finish_copy = lvconvert_merge_finish
};

static struct poll_functions _thin_merge_fns = {
	.poll_progress = poll_thin_merge_progress,
	.finish_copy = lvconvert_merge_finish
};

static int _set_daemon_parms(struct cmd_context *cmd, struct daemon_parms *parms)
{
	const char *poll_oper = arg_str_value(cmd, polloperation_ARG, "");

	parms->interval = arg_uint_value(cmd, interval_ARG, 0);
	parms->aborting = arg_is_set(cmd, abort_ARG);
	parms->progress_display = 1;
	parms->wait_before_testing = (arg_sign_value(cmd, interval_ARG, SIGN_NONE) == SIGN_PLUS);

	if (!strcmp(poll_oper, PVMOVE_POLL)) {
		parms->progress_title = "Moved";
		parms->lv_type = PVMOVE;
		parms->poll_fns = &_pvmove_fns;
	} else if (!strcmp(poll_oper, CONVERT_POLL)) {
		parms->progress_title = "Converted";
		parms->poll_fns = &_convert_fns;
	} else if (!strcmp(poll_oper, MERGE_POLL)) {
		parms->progress_title = "Merged";
		parms->poll_fns = &_merge_fns;
	} else if (!strcmp(poll_oper, MERGE_THIN_POLL)) {
		parms->progress_title = "Merged";
		parms->poll_fns = &_thin_merge_fns;
	} else {
		log_error("Unknown polling operation %s", poll_oper);
		return 0;
	}

	cmd->handles_missing_pvs = arg_is_set(cmd, handlemissingpvs_ARG);

	return 1;
}

static int _poll_lv(struct cmd_context *cmd, const char *lv_name)
{
	struct daemon_parms parms = { 0 };
	struct poll_operation_id id = {
		.display_name = skip_dev_dir(cmd, lv_name, NULL)
	};

	if (!id.display_name)
		return_EINVALID_CMD_LINE;

	id.lv_name = id.display_name;

	if (!validate_lvname_param(cmd, &id.vg_name, &id.lv_name))
		return_EINVALID_CMD_LINE;

	if (!_set_daemon_parms(cmd, &parms))
		return_EINVALID_CMD_LINE;

	return wait_for_single_lv(cmd, &id, &parms) ? ECMD_PROCESSED : ECMD_FAILED;
}

int lvpoll(struct cmd_context *cmd, int argc, char **argv)
{
	if (!arg_is_set(cmd, polloperation_ARG)) {
		log_error("--polloperation parameter is mandatory");
		return EINVALID_CMD_LINE;
	}

	if (arg_sign_value(cmd, interval_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error("Argument to --interval cannot be negative");
		return EINVALID_CMD_LINE;
	}

	if (!argc) {
		log_error("Provide LV name");
		return EINVALID_CMD_LINE;
	}

	return _poll_lv(cmd, argv[0]);
}
