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

static struct volume_group *_vgmerge_vg_read(struct cmd_context *cmd,
					     const char *vg_name)
{
	struct volume_group *vg;
	log_verbose("Checking for volume group \"%s\"", vg_name);
	vg = vg_read_for_update(cmd, vg_name, NULL, 0, 0);
	if (vg_read_error(vg)) {
		release_vg(vg);
		return NULL;
	}

	if (vg_is_shared(vg)) {
		log_error("vgmerge not allowed for lock_type %s", vg->lock_type);
		unlock_and_release_vg(cmd, vg, vg_name);
		return NULL;
	}

	return vg;
}

/* Select bigger pool metadata spare volume */
static int _vgmerge_select_pool_metadata_spare(struct cmd_context *cmd,
					       struct volume_group *vg_to,
					       struct volume_group *vg_from)
{
	struct volume_group *svg;

	if (!vg_to->pool_metadata_spare_lv ||
	    !vg_from->pool_metadata_spare_lv)
		return 1; /* no problem */

	/* Drop smaller pool metadata spare */
	svg = (vg_to->pool_metadata_spare_lv->le_count <
	       vg_from->pool_metadata_spare_lv->le_count) ? vg_to : vg_from;
	vg_remove_pool_metadata_spare(svg);

	/* Re-test lv name compatibility */
	if (!vgs_are_compatible(cmd, vg_from, vg_to))
		return_0;

	return 1;
}

static int _vgmerge_single(struct cmd_context *cmd, const char *vg_name_to,
			   const char *vg_name_from)
{
	struct pv_list *pvl, *tpvl;
	struct volume_group *vg_to, *vg_from;
	struct lv_list *lvl1, *lvl2;
	int r = ECMD_FAILED;
	int lock_vg_from_first = 0;

	if (!strcmp(vg_name_to, vg_name_from)) {
		log_error("Duplicate volume group name \"%s\"", vg_name_from);
		return ECMD_FAILED;
	}

	lvmcache_label_scan(cmd);

	if (strcmp(vg_name_to, vg_name_from) > 0)
		lock_vg_from_first = 1;

	if (lock_vg_from_first) {
		if (!(vg_from = _vgmerge_vg_read(cmd, vg_name_from)))
			return_ECMD_FAILED;
		if (!(vg_to = _vgmerge_vg_read(cmd, vg_name_to))) {
			unlock_and_release_vg(cmd, vg_from, vg_name_from);
			return_ECMD_FAILED;
		}
	} else {
		if (!(vg_to = _vgmerge_vg_read(cmd, vg_name_to)))
			return_ECMD_FAILED;

		if (!(vg_from = _vgmerge_vg_read(cmd, vg_name_from))) {
			unlock_and_release_vg(cmd, vg_to, vg_name_to);
			return_ECMD_FAILED;
		}
	}

	if (!vgs_are_compatible(cmd, vg_from, vg_to))
		goto_bad;

	/* FIXME List arg: vg_show_with_pv_and_lv(vg_to); */

	if (!archive(vg_from) || !archive(vg_to))
		goto_bad;

	if (!_vgmerge_select_pool_metadata_spare(cmd, vg_to, vg_from))
		goto_bad;

	/* Merge volume groups */
	dm_list_iterate_items_safe(pvl, tpvl, &vg_from->pvs) {
		del_pvl_from_vgs(vg_from, pvl);
		add_pvl_to_vgs(vg_to, pvl);
		pvl->pv->vg_name = dm_pool_strdup(cmd->mem, vg_to->name);
		/* Mark the VGs that still hold metadata for the old VG */
		log_debug_metadata("Marking PV %s as moved to VG %s", dev_name(pvl->pv->dev), vg_to->name);
		pvl->pv->status |= PV_MOVED_VG;
	}

	/* Fix up LVIDs */
	dm_list_iterate_items(lvl1, &vg_to->lvs) {
		union lvid *lvid1 = &lvl1->lv->lvid;
		char uuid[64] __attribute__((aligned(8)));

		dm_list_iterate_items(lvl2, &vg_from->lvs) {
			union lvid *lvid2 = &lvl2->lv->lvid;

			if (id_equal(&lvid1->id[1], &lvid2->id[1])) {
				if (!id_create(&lvid2->id[1])) {
					log_error("Failed to generate new "
						  "random LVID for %s",
						  lvl2->lv->name);
					goto bad;
				}
				if (!id_write_format(&lvid2->id[1], uuid,
						     sizeof(uuid)))
					goto_bad;

				log_verbose("Changed LVID for %s to %s",
					    lvl2->lv->name, uuid);
			}
		}
	}

	dm_list_iterate_items(lvl1, &vg_from->lvs) {
		lvl1->lv->vg = vg_to;
		lvl1->lv->lvid.id[0] = lvl1->lv->vg->id;
	}

	while (!dm_list_empty(&vg_from->lvs)) {
		struct dm_list *lvh = vg_from->lvs.n;

		dm_list_move(&vg_to->lvs, lvh);
	}

	while (!dm_list_empty(&vg_from->fid->metadata_areas_in_use)) {
		struct dm_list *mdah = vg_from->fid->metadata_areas_in_use.n;

		dm_list_move(&vg_to->fid->metadata_areas_in_use, mdah);
	}

	while (!dm_list_empty(&vg_from->fid->metadata_areas_ignored)) {
		struct dm_list *mdah = vg_from->fid->metadata_areas_ignored.n;

		dm_list_move(&vg_to->fid->metadata_areas_ignored, mdah);
	}

	if (!vg_to->pool_metadata_spare_lv)
		vg_to->pool_metadata_spare_lv =
			vg_from->pool_metadata_spare_lv;

	vg_to->extent_count += vg_from->extent_count;
	vg_to->free_count += vg_from->free_count;

	/* Flag up that some PVs have moved from another VG */
	vg_to->old_name = vg_from->name;

	/* store it on disks */
	log_verbose("Writing out updated volume group");
	if (!vg_write(vg_to) || !vg_commit(vg_to))
		goto_bad;

	/* FIXME Remove /dev/vgfrom */

	backup(vg_to);
	log_print_unless_silent("Volume group \"%s\" successfully merged into \"%s\"",
				vg_from->name, vg_to->name);
	r = ECMD_PROCESSED;
bad:
	/*
	 * Note: as vg_to is referencing moved elements from vg_from
	 * the order of release_vg calls is mandatory.
	 */
	unlock_and_release_vg(cmd, vg_to, vg_name_to);
	unlock_and_release_vg(cmd, vg_from, vg_name_from);

	return r;
}

int vgmerge(struct cmd_context *cmd, int argc, char **argv)
{
	const char *vg_name_to, *vg_name_from;
	int opt = 0;
	int ret = 0, ret_max = 0;

	if (argc < 2) {
		log_error("Please enter 2 or more volume groups to merge");
		return EINVALID_CMD_LINE;
	}

	/* Needed change the global VG namespace. */
	if (!lockd_gl(cmd, "ex", LDGL_UPDATE_NAMES))
		return ECMD_FAILED;

	vg_name_to = skip_dev_dir(cmd, argv[0], NULL);
	argc--;
	argv++;

	for (; opt < argc; opt++) {
		vg_name_from = skip_dev_dir(cmd, argv[opt], NULL);

		ret = _vgmerge_single(cmd, vg_name_to, vg_name_from);
		if (ret > ret_max)
			ret_max = ret;
	}

	return ret_max;
}
