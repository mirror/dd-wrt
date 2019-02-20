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

struct vgreduce_params {
	int force;
	int fixed;
	int already_consistent;
};

static int _remove_pv(struct volume_group *vg, struct pv_list *pvl, int silent)
{
	char uuid[64] __attribute__((aligned(8)));

	if (vg->pv_count == 1) {
		log_error("Volume Groups must always contain at least one PV.");
		return 0;
	}

	if (!id_write_format(&pvl->pv->id, uuid, sizeof(uuid)))
		return_0;

	log_verbose("Removing PV with UUID %s from VG %s.", uuid, vg->name);

	if (pvl->pv->pe_alloc_count) {
		if (!silent)
			log_error("LVs still present on PV with UUID %s: "
				  "Can't remove from VG %s.", uuid, vg->name);
		return 0;
	}

	vg->free_count -= pvl->pv->pe_count;
	vg->extent_count -= pvl->pv->pe_count;
	del_pvl_from_vgs(vg, pvl);
	free_pv_fid(pvl->pv);

	return 1;
}

static int _consolidate_vg(struct cmd_context *cmd, struct volume_group *vg)
{
	struct pv_list *pvl;
	struct lv_list *lvl;
	int r = 1;

	dm_list_iterate_items(lvl, &vg->lvs)
		if (lv_is_partial(lvl->lv)) {
			log_warn("WARNING: Partial LV %s needs to be repaired "
				 "or removed. ", lvl->lv->name);
			r = 0;
		}

	if (!r) {
		cmd->handles_missing_pvs = 1;
		log_error("There are still partial LVs in VG %s.", vg->name);
		log_error("To remove them unconditionally use: vgreduce --removemissing --force.");
		log_warn("WARNING: Proceeding to remove empty missing PVs.");
	}

	dm_list_iterate_items(pvl, &vg->pvs) {
		if (pvl->pv->dev && !is_missing_pv(pvl->pv))
			continue;
		if (r && !_remove_pv(vg, pvl, 0))
			return_0;
	}

	return r;
}

static int _make_vg_consistent(struct cmd_context *cmd, struct volume_group *vg)
{
	struct lv_list *lvl;
	struct logical_volume *lv;

	cmd->partial_activation = 1;

 restart:
	vg_mark_partial_lvs(vg, 1);

	dm_list_iterate_items(lvl, &vg->lvs) {
		lv = lvl->lv;

		/* Are any segments of this LV on missing PVs? */
		if (lv_is_partial(lv)) {
			if (seg_is_raid(first_seg(lv))) {
				if (!lv_raid_remove_missing(lv))
					return_0;
				goto restart;
			}

			if (lv_is_mirror(lv)) {
				if (!mirror_remove_missing(cmd, lv, 1))
					return_0;
				goto restart;
			}

			if (arg_is_set(cmd, mirrorsonly_ARG) && !lv_is_mirrored(lv)) {
				log_error("Non-mirror-image LV %s found: can't remove.", lv->name);
				continue;
			}

			if (!lv_is_visible(lv))
				continue;

			log_warn("WARNING: Removing partial LV %s.", display_lvname(lv));

			if (!lv_remove_with_dependencies(cmd, lv, DONT_PROMPT, 0))
				return_0;
			goto restart;
		}
	}

	_consolidate_vg(cmd, vg);

	return 1;
}

/* Or take pv_name instead? */
static int _vgreduce_single(struct cmd_context *cmd, struct volume_group *vg,
			    struct physical_volume *pv,
			    struct processing_handle *handle __attribute__((unused)))
{
	int r;

	if (!vg_check_status(vg, EXPORTED_VG | LVM_WRITE | RESIZEABLE_VG))
		return ECMD_FAILED;

	r = vgreduce_single(cmd, vg, pv, 1);
	if (!r)
		return ECMD_FAILED;

	return ECMD_PROCESSED;
}

static int _vgreduce_repair_single(struct cmd_context *cmd, const char *vg_name,
		                   struct volume_group *vg, struct processing_handle *handle)
{
	struct vgreduce_params *vp = (struct vgreduce_params *) handle->custom_handle;

	if (!vg_missing_pv_count(vg)) {
		vp->already_consistent = 1;
		return ECMD_PROCESSED;
	}

	if (!archive(vg))
		return_ECMD_FAILED;

	if (vp->force) {
		if (!_make_vg_consistent(cmd, vg))
			return_ECMD_FAILED;
		vp->fixed = 1;
	} else
		vp->fixed = _consolidate_vg(cmd, vg);

	if (!vg_write(vg) || !vg_commit(vg)) {
		log_error("Failed to write out a consistent VG for %s", vg_name);
		return ECMD_FAILED;
	}

	backup(vg);
	return ECMD_PROCESSED;
}

int vgreduce(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle;
	struct vgreduce_params vp = { 0 };
	const char *vg_name;
	int repairing = arg_is_set(cmd, removemissing_ARG);
	int saved_ignore_suspended_devices = ignore_suspended_devices();
	int ret;

	if (!argc && !repairing) {
		log_error("Please give volume group name and "
			  "physical volume paths.");
		return EINVALID_CMD_LINE;
	}
	
	if (!argc) { /* repairing */
		log_error("Please give volume group name.");
		return EINVALID_CMD_LINE;
	}

	if (arg_is_set(cmd, mirrorsonly_ARG) && !repairing) {
		log_error("--mirrorsonly requires --removemissing.");
		return EINVALID_CMD_LINE;
	}

	if (argc == 1 && !arg_is_set(cmd, all_ARG) && !repairing) {
		log_error("Please enter physical volume paths or option -a.");
		return EINVALID_CMD_LINE;
	}

	if (argc > 1 && arg_is_set(cmd, all_ARG)) {
		log_error("Option -a and physical volume paths mutually "
			  "exclusive.");
		return EINVALID_CMD_LINE;
	}

	if (argc > 1 && repairing) {
		log_error("Please only specify the volume group.");
		return EINVALID_CMD_LINE;
	}

	vg_name = skip_dev_dir(cmd, argv[0], NULL);
	argv++;
	argc--;

	/* Needed to change the set of orphan PVs. */
	if (!lockd_gl(cmd, "ex", 0))
		return_ECMD_FAILED;
	cmd->lockd_gl_disable = 1;

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}
	handle->custom_handle = &vp;

	if (!repairing) {
		if (!lock_vol(cmd, VG_ORPHANS, LCK_VG_WRITE, NULL)) {
			log_error("Can't get lock for orphan PVs");
			ret = ECMD_FAILED;
			goto out;
		}

		/* FIXME: Pass private struct through to all these functions */
		/* and update in batch afterwards? */

		ret = process_each_pv(cmd, argc, argv, vg_name, 0,
				      READ_FOR_UPDATE | PROCESS_SKIP_ORPHAN_LOCK,
				      handle, _vgreduce_single);

		unlock_vg(cmd, NULL, VG_ORPHANS);
		goto out;
	}

	/*
	 * VG repair (removemissing)
	 */

	vp.force = arg_count(cmd, force_ARG);

	cmd->handles_missing_pvs = 1;

	init_ignore_suspended_devices(1);

	process_each_vg(cmd, 0, NULL, vg_name, NULL,
			READ_FOR_UPDATE | READ_ALLOW_EXPORTED,
			0, handle, &_vgreduce_repair_single);

	if (vp.already_consistent) {
		log_print_unless_silent("Volume group \"%s\" is already consistent.", vg_name);
		ret = ECMD_PROCESSED;
	} else if (vp.fixed) {
		log_print_unless_silent("Wrote out consistent volume group %s.", vg_name);
		ret = ECMD_PROCESSED;
	} else
		ret = ECMD_FAILED;
out:
	init_ignore_suspended_devices(saved_ignore_suspended_devices);
	destroy_processing_handle(cmd, handle);

	return ret;
}
