/*
 * Copyright (C) 2016 Red Hat, Inc. All rights reserved.
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
#include "lib/cache/lvmcache.h"
#include "lib/filters/filter.h"

struct vgimportclone_params {
	unsigned done;
	unsigned total;

	int import_vg;
	int found_args;
	struct dm_list arg_import;
	const char *base_vgname;
	const char *old_vgname;
	const char *new_vgname;
};

struct vgimportclone_device {
	struct dm_list list;
	struct device *dev;
	unsigned found_in_vg : 1;
};

static int _vgimportclone_pv_single(struct cmd_context *cmd, struct volume_group *vg,
				    struct physical_volume *pv, struct processing_handle *handle)
{
	struct vgimportclone_params *vp = (struct vgimportclone_params *) handle->custom_handle;
	struct vgimportclone_device *vd;

	if (vg && is_orphan_vg(vg->name)) {
		log_error("Cannot import clone of orphan PV %s.", dev_name(pv->dev));
		return ECMD_FAILED;
	}

	if (!(vd = dm_pool_zalloc(cmd->mem, sizeof(*vd)))) {
		log_error("alloc failed.");
		return ECMD_FAILED;
	}

	vd->dev = pv->dev;
	dm_list_add(&vp->arg_import, &vd->list);

	log_debug("vgimportclone dev %s VG %s found to import",
		  dev_name(vd->dev), vg ? vg->name : "<none>");

	vp->found_args++;

	return ECMD_PROCESSED;
}

static int _vgimportclone_vg_single(struct cmd_context *cmd, const char *vg_name,
		                    struct volume_group *vg, struct processing_handle *handle)
{
	char uuid[64] __attribute__((aligned(8)));
	struct vgimportclone_params *vp = (struct vgimportclone_params *) handle->custom_handle;
	struct vgimportclone_device *vd;
	struct pv_list *pvl, *new_pvl;
	struct lv_list *lvl;
	int devs_used_for_lv = 0;
	int found;

	if (vg_is_exported(vg) && !vp->import_vg) {
		log_error("VG %s is exported, use the --import option.", vg->name);
		goto bad;
	}

	if (vg_status(vg) & PARTIAL_VG) {
		log_error("VG %s is partial, it must be complete.", vg->name);
		goto bad;
	}

	/*
	 * N.B. lvs_in_vg_activated() is not smart enough to distinguish
	 * between LVs that are active in the original VG vs the cloned VG
	 * that's being imported, so check DEV_USED_FOR_LV.
	 */
	dm_list_iterate_items(pvl, &vg->pvs) {
		if (pvl->pv->dev->flags & DEV_USED_FOR_LV) {
			log_error("Device %s has active LVs, deactivate first.", dev_name(pvl->pv->dev));
			devs_used_for_lv++;
		}
	}

	if (devs_used_for_lv)
		goto_bad;

	/*
	 * The arg_import list must match the PVs in VG.
	 */

	dm_list_iterate_items(pvl, &vg->pvs) {
		found = 0;

		dm_list_iterate_items(vd, &vp->arg_import) {
			if (pvl->pv->dev != vd->dev)
				continue;
			vd->found_in_vg = 1;
			found = 1;
			break;
		}

		if (!found) {
			if (!id_write_format(&pvl->pv->id, uuid, sizeof(uuid)))
				goto_bad;

			/* all PVs in the VG must be imported together, pvl is missing from args. */
			log_error("PV with UUID %s is part of VG %s, but is not included in the devices to import.",
				   uuid, vg->name);
			log_error("All PVs in the VG must be imported together.");
			goto bad;
		}
	}

	dm_list_iterate_items(vd, &vp->arg_import) {
		if (!vd->found_in_vg) {
			/* device arg is not in the VG. */
			log_error("Device %s was not found in VG %s.", dev_name(vd->dev), vg->name);
			log_error("The devices to import must match the devices in the VG.");
			goto bad;
		}
	}

	/*
	 * Write changes.
	 */

	if (!archive(vg))
		return_ECMD_FAILED;

	if (vp->import_vg)
		vg->status &= ~EXPORTED_VG;

	if (!id_create(&vg->id))
		goto_bad;

	/* Low level vg_write code needs old_name to be set! */
	vg->old_name = vg->name;

	if (!(vg->name = dm_pool_strdup(vg->vgmem, vp->new_vgname)))
		goto_bad;

	/* A duplicate of a shared VG is imported as a new local VG. */
	vg->lock_type = NULL;
	vg->lock_args = NULL;
	vg->system_id = cmd->system_id ? dm_pool_strdup(vg->vgmem, cmd->system_id) : NULL;

	dm_list_iterate_items(pvl, &vg->pvs) {
		if (!(new_pvl = dm_pool_zalloc(vg->vgmem, sizeof(*new_pvl))))
			goto_bad;

		new_pvl->pv = pvl->pv;

		if (!(pvl->pv->vg_name = dm_pool_strdup(vg->vgmem, vp->new_vgname)))
			goto_bad;

		if (vp->import_vg)
			new_pvl->pv->status &= ~EXPORTED_VG;

		/* Low level pv_write code needs old_id to be set! */
		memcpy(&new_pvl->pv->old_id, &new_pvl->pv->id, sizeof(new_pvl->pv->id));

		if (!id_create(&new_pvl->pv->id))
			goto_bad;

		dm_list_add(&vg->pv_write_list, &new_pvl->list);
	}

	dm_list_iterate_items(lvl, &vg->lvs) {
		memcpy(&lvl->lv->lvid, &vg->id, sizeof(vg->id));
		lvl->lv->lock_args = NULL;
	}

	if (!vg_write(vg) || !vg_commit(vg))
		goto_bad;

	return ECMD_PROCESSED;
bad:
	return ECMD_FAILED;
}

int vgimportclone(struct cmd_context *cmd, int argc, char **argv)
{
	struct vgimportclone_params vp = { 0 };
	struct processing_handle *handle = NULL;
	struct dm_list vgnameids_on_system;     /* vgnameid_list */
	struct vgnameid_list *vgnl;
	struct vgimportclone_device *vd;
	struct lvmcache_info *info;
	const char *vgname;
	char base_vgname[NAME_LEN] = { 0 };
	char tmp_vgname[NAME_LEN] = { 0 };
	unsigned int vgname_count;
	int ret = ECMD_FAILED;

	if (!argc) {
		log_error("PV names required.");
		return EINVALID_CMD_LINE;
	}

	dm_list_init(&vgnameids_on_system);
	dm_list_init(&vp.arg_import);

	set_pv_notify(cmd);

	vp.import_vg = arg_is_set(cmd, import_ARG);

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}
	handle->custom_handle = &vp;

	if (!lock_vol(cmd, VG_GLOBAL, LCK_VG_WRITE, NULL)) {
		log_error("Unable to obtain global lock.");
		destroy_processing_handle(cmd, handle);
		return ECMD_FAILED;
	}

	if (!lockd_gl(cmd, "ex", 0))
		goto_out;
	cmd->lockd_gl_disable = 1;

	/*
	 * Find the devices being imported which are named on the command line.
	 * They may be in the list of unchosen duplicates.
	 */

	log_debug("Finding devices to import.");
	cmd->cname->flags |= ENABLE_DUPLICATE_DEVS;
	process_each_pv(cmd, argc, argv, NULL, 0, READ_ALLOW_EXPORTED, handle, _vgimportclone_pv_single);

	if (vp.found_args != argc) {
		log_error("Failed to find all devices.");
		goto out;
	}

	/*
	 * Find the VG name of the PVs being imported, save as old_vgname.
	 * N.B. If vd->dev is a duplicate, then it may not match info->dev.
	 */

	dm_list_iterate_items(vd, &vp.arg_import) {
		if (!(info = lvmcache_info_from_pvid(vd->dev->pvid, NULL, 0))) {
			log_error("Failed to find PVID for device %s in lvmcache.", dev_name(vd->dev));
			goto out;
		}

		if (!(vgname = lvmcache_vgname_from_info(info))) {
			log_error("Failed to find VG name for device %s in lvmcache.", dev_name(vd->dev));
			goto out;
		}

		if (!vp.old_vgname) {
			if (!(vp.old_vgname = dm_pool_strdup(cmd->mem, vgname)))
				goto_out;
		} else {
			if (strcmp(vp.old_vgname, vgname)) {
				log_error("Devices must be from the same VG.");
				goto out;
			}
		}
	}

	/*
	 * Pick a new VG name, save as new_vgname.  The new name begins with
	 * the basevgname or old_vgname, plus a $i suffix, if necessary, to
	 * make it unique.  This requires comparing the old_vgname with all the
	 * VG names on the system.
	 */

	if (arg_is_set(cmd, basevgname_ARG)) {
		vgname = arg_str_value(cmd, basevgname_ARG, "");
		if (dm_snprintf(base_vgname, sizeof(base_vgname), "%s", vgname) < 0) {
			log_error("Base vg name %s is too long.", vgname);
			goto out;
		}
		(void) dm_strncpy(tmp_vgname, base_vgname, NAME_LEN);
		vgname_count = 0;
	} else {
		if (dm_snprintf(base_vgname, sizeof(base_vgname), "%s", vp.old_vgname) < 0) {
			log_error(INTERNAL_ERROR "Old vg name %s is too long.", vp.old_vgname);
			goto out;
		}
		if (dm_snprintf(tmp_vgname, sizeof(tmp_vgname), "%s1", vp.old_vgname) < 0) {
			log_error("Temporary vg name %s1 is too long.", vp.old_vgname);
			goto out;
		}
		vgname_count = 1;
	}

	if (!get_vgnameids(cmd, &vgnameids_on_system, NULL, 0))
		goto_out;

retry_name:
	dm_list_iterate_items(vgnl, &vgnameids_on_system) {
		if (!strcmp(vgnl->vg_name, tmp_vgname)) {
			vgname_count++;
			if (dm_snprintf(tmp_vgname, sizeof(tmp_vgname), "%s%u", base_vgname, vgname_count) < 0) {
				log_error("Failed to generated temporary vg name, %s%u is too long.", base_vgname, vgname_count);
				goto out;
			}
			goto retry_name;
		}
	}

	if (!(vp.new_vgname = dm_pool_strdup(cmd->mem, tmp_vgname)))
		goto_out;
	log_debug("Using new VG name %s.", vp.new_vgname);

	/*
	 * Create a device filter so that we are only working with the devices
	 * in arg_import.  With the original devs hidden (that arg_import were
	 * cloned from), we can read and write the cloned PVs and VG without
	 * touching the original PVs/VG.
	 */

	init_internal_filtering(1);
	dm_list_iterate_items(vd, &vp.arg_import)
		internal_filter_allow(cmd->mem, vd->dev);
	lvmcache_destroy(cmd, 1, 0);

	log_debug("Changing VG %s to %s.", vp.old_vgname, vp.new_vgname);

	if (!lock_vol(cmd, vp.new_vgname, LCK_VG_WRITE, NULL)) {
		log_error("Can't get lock for new VG name %s", vp.new_vgname);
		goto out;
	}

	/*
	 * Trying to lock the duplicated VG would conflict with the original,
	 * and it's not needed because the new VG will be imported as a local VG.
	 */
	cmd->lockd_vg_disable = 1;

	ret = process_each_vg(cmd, 0, NULL, vp.old_vgname, NULL, READ_FOR_UPDATE | READ_ALLOW_EXPORTED, 0, handle, _vgimportclone_vg_single);

	unlock_vg(cmd, NULL, vp.new_vgname);
out:
	unlock_vg(cmd, NULL, VG_GLOBAL);
	internal_filter_clear();
	init_internal_filtering(0);
	destroy_processing_handle(cmd, handle);

	return ret;
}
