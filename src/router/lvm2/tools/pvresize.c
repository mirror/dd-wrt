/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2005 Red Hat, Inc. All rights reserved.
 * Copyright (C) 2005 Zak Kipling. All rights reserved.
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

struct pvresize_params {
	uint64_t new_size;

	unsigned done;
	unsigned total;
};

static int _pvresize_single(struct cmd_context *cmd,
			    struct volume_group *vg,
			    struct physical_volume *pv,
			    struct processing_handle *handle)
{
	struct pvresize_params *params = (struct pvresize_params *) handle->custom_handle;

	if (!params) {
		log_error(INTERNAL_ERROR "Invalid resize params.");
		return ECMD_FAILED;
	}
	params->total++;

	if (vg && vg_is_exported(vg)) {
		log_error("Volume group %s is exported", vg->name);
		return ECMD_FAILED;
	}

	/*
	 * Needed to change a property on an orphan PV.
	 * i.e. the global lock is only needed for orphans.
	 * Convert sh to ex.
	 */
	if (is_orphan(pv)) {
		if (!lockd_gl(cmd, "ex", 0))
			return_ECMD_FAILED;
		cmd->lockd_gl_disable = 1;
	}

	if (!pv_resize_single(cmd, vg, pv, params->new_size, arg_is_set(cmd, yes_ARG)))
		return_ECMD_FAILED;

	params->done++;

	return ECMD_PROCESSED;
}

int pvresize(struct cmd_context *cmd, int argc, char **argv)
{
	struct pvresize_params params;
	struct processing_handle *handle = NULL;
	int ret;

	if (!argc) {
		log_error("Please supply physical volume(s)");
		ret = EINVALID_CMD_LINE;
		goto out;
	}

	if (arg_sign_value(cmd, setphysicalvolumesize_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error("Physical volume size may not be negative");
		ret = EINVALID_CMD_LINE;
		goto out;
	}

	params.new_size = arg_uint64_value(cmd, setphysicalvolumesize_ARG,
					   UINT64_C(0));

	params.done = 0;
	params.total = 0;

	set_pv_notify(cmd);

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		ret = ECMD_FAILED;
		goto out;
	}

	handle->custom_handle = &params;

	ret = process_each_pv(cmd, argc, argv, NULL, 0, READ_FOR_UPDATE | READ_ALLOW_EXPORTED, handle, _pvresize_single);

	log_print_unless_silent("%d physical volume(s) resized or updated / %d physical volume(s) "
				"not resized", params.done, params.total - params.done);
out:
	destroy_processing_handle(cmd, handle);
	return ret;
}
