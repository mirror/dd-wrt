/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2009 Red Hat, Inc. All rights reserved.
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

/*
 * Intial sanity checking of recovery-related command-line arguments.
 * These args are: --restorefile, --uuid, and --physicalvolumesize
 *
 * Output arguments:
 * pp: structure allocated by caller, fields written / validated here
 */
static int _pvcreate_restore_params_from_args(struct cmd_context *cmd, int argc,
					      struct pvcreate_params *pp)
{
	pp->restorefile = arg_str_value(cmd, restorefile_ARG, NULL);

	if (arg_is_set(cmd, restorefile_ARG) && !arg_is_set(cmd, uuidstr_ARG)) {
		log_error("--uuid is required with --restorefile");
		return 0;
	}

	if (!arg_is_set(cmd, restorefile_ARG) && arg_is_set(cmd, uuidstr_ARG)) {
		if (!arg_is_set(cmd, norestorefile_ARG) &&
		    find_config_tree_bool(cmd, devices_require_restorefile_with_uuid_CFG, NULL)) {
			log_error("--restorefile is required with --uuid");
			return 0;
		}
	}

	if (arg_is_set(cmd, uuidstr_ARG) && argc != 1) {
		log_error("Can only set uuid on one volume at once");
		return 0;
	}

	if (arg_is_set(cmd, uuidstr_ARG)) {
		pp->uuid_str = arg_str_value(cmd, uuidstr_ARG, "");
		if (!id_read_format(&pp->pva.id, pp->uuid_str))
			return 0;
		pp->pva.idp = &pp->pva.id;
	}

	if (arg_sign_value(cmd, setphysicalvolumesize_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error("Physical volume size may not be negative");
		return 0;
	}
	pp->pva.size = arg_uint64_value(cmd, setphysicalvolumesize_ARG, UINT64_C(0));

	if (arg_is_set(cmd, restorefile_ARG) || arg_is_set(cmd, uuidstr_ARG))
		pp->zero = 0;
	return 1;
}

static int _pvcreate_restore_params_from_backup(struct cmd_context *cmd,
					       struct pvcreate_params *pp)
{
	struct volume_group *vg;
	struct pv_list *existing_pvl;

	/*
	 * When restoring a PV, params need to be read from a backup file.
	 */
	if (!pp->restorefile)
		return 1;

	if (!(vg = backup_read_vg(cmd, NULL, pp->restorefile))) {
		log_error("Unable to read volume group from %s", pp->restorefile);
		return 0;
	}

	if (!(existing_pvl = find_pv_in_vg_by_uuid(vg, &pp->pva.id))) {
		release_vg(vg);
		log_error("Can't find uuid %s in backup file %s",
			  pp->uuid_str, pp->restorefile);
		return 0;
	}

	pp->pva.ba_start = pv_ba_start(existing_pvl->pv);
	pp->pva.ba_size = pv_ba_size(existing_pvl->pv);
	pp->pva.pe_start = pv_pe_start(existing_pvl->pv);
	pp->pva.extent_size = pv_pe_size(existing_pvl->pv);
	pp->pva.extent_count = pv_pe_count(existing_pvl->pv);

	release_vg(vg);
	return 1;
}

int pvcreate(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct pvcreate_params pp;
	int ret;

	/*
	 * Five kinds of pvcreate param values:
	 * 1. defaults
	 * 2. recovery-related command line args
	 * 3. recovery-related args from backup file
	 * 4. normal command line args
	 *    (this also checks some settings from 2 & 3)
	 * 5. argc/argv free args specifying devices
	 */

	pvcreate_params_set_defaults(&pp);

	if (!_pvcreate_restore_params_from_args(cmd, argc, &pp))
		return EINVALID_CMD_LINE;

	if (!_pvcreate_restore_params_from_backup(cmd, &pp))
		return EINVALID_CMD_LINE;

	if (!pvcreate_params_from_args(cmd, &pp))
		return EINVALID_CMD_LINE;

	/*
	 * If --metadatasize was not given with --restorefile, set it to pe_start.
	 * Later code treats this as a maximum size and reduces it to fit.
	 */
	if (!arg_is_set(cmd, metadatasize_ARG) && arg_is_set(cmd, restorefile_ARG))
		pp.pva.pvmetadatasize = pp.pva.pe_start;

	/* FIXME Also needs to check any 2nd metadata area isn't inside the data area! */

	pp.pv_count = argc;
	pp.pv_names = argv;

	/* Check for old md signatures at the end of devices. */
	cmd->use_full_md_check = 1;

	/*
	 * Needed to change the set of orphan PVs.
	 * (disable afterward to prevent process_each_pv from doing
	 * a shared global lock since it's already acquired it ex.)
	 */
	if (!lockd_gl(cmd, "ex", 0))
		return_ECMD_FAILED;
	cmd->lockd_gl_disable = 1;

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	if (!pvcreate_each_device(cmd, handle, &pp))
		ret = ECMD_FAILED;
	else {
		/* pvcreate_each_device returns with orphans locked */
		unlock_vg(cmd, NULL, VG_ORPHANS);
		ret = ECMD_PROCESSED;
	}

	destroy_processing_handle(cmd, handle);
	return ret;
}
