/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2009,2016 Red Hat, Inc. All rights reserved.
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

static int _lv_is_in_vg(struct volume_group *vg, struct logical_volume *lv)
{
	if (!lv || lv->vg != vg)
		return 0;

	return 1;
}

static struct dm_list *_lvh_in_vg(struct logical_volume *lv, struct volume_group *vg)
{
	struct dm_list *lvh;

	dm_list_iterate(lvh, &vg->lvs)
		if (lv == dm_list_item(lvh, struct lv_list)->lv)
			return lvh;

	return NULL;
}

static int _lv_tree_move(struct dm_list *lvh,
			 struct dm_list **lvht,
			 struct volume_group *vg_from,
			 struct volume_group *vg_to)
{
	uint32_t s;
	struct logical_volume *lv = dm_list_item(lvh, struct lv_list)->lv;
	struct lv_segment *seg = first_seg(lv);
	struct dm_list *lvh1;

	/* Update the list pointer refering to the item moving to @vg_to. */
	if (lvh == *lvht)
		*lvht = dm_list_next(lvh, lvh);

	dm_list_move(&vg_to->lvs, lvh);
	lv->vg = vg_to;
	lv->lvid.id[0] = lv->vg->id;

	if (seg)
		for (s = 0; s < seg->area_count; s++)
			if (seg_type(seg, s) == AREA_LV && seg_lv(seg, s)) {
				if ((lvh1 = _lvh_in_vg(seg_lv(seg, s), vg_from))) {
					if (!_lv_tree_move(lvh1, lvht, vg_from, vg_to))
						return 0;
				} else if (!_lvh_in_vg(seg_lv(seg, s), vg_to))
					return 0;
			}

	return 1;
}

static int _move_one_lv(struct volume_group *vg_from,
			struct volume_group *vg_to,
			struct dm_list *lvh,
			struct dm_list **lvht)
{
	struct logical_volume *lv = dm_list_item(lvh, struct lv_list)->lv;
	struct logical_volume *parent_lv;

	if (lv_is_active(lv)) {
		if ((parent_lv = lv_parent(lv)))
			log_error("Logical volume %s (part of %s) must be inactive.", display_lvname(lv), parent_lv->name);
		else
			log_error("Logical volume %s must be inactive.", display_lvname(lv));
		return 0;
	}

	/* Bail out, if any allocations of @lv are still on PVs of @vg_from */
	if (lv_is_on_pvs(lv, &vg_from->pvs)) {
		log_error("Can't split LV %s between "
			  "two Volume Groups", lv->name);
		return 0;
	}

	if (!_lv_tree_move(lvh, lvht, vg_from, vg_to))
		return 0;

	/* Moved pool metadata spare LV */
	if (vg_from->pool_metadata_spare_lv == lv) {
		vg_to->pool_metadata_spare_lv = lv;
		vg_from->pool_metadata_spare_lv = NULL;
	}

	return 1;
}

static int _move_lvs(struct volume_group *vg_from, struct volume_group *vg_to)
{
	struct dm_list *lvh, *lvht;
	struct logical_volume *lv;
	struct lv_segment *seg;
	struct physical_volume *pv;
	struct volume_group *vg_with;
	unsigned s;

	dm_list_iterate_safe(lvh, lvht, &vg_from->lvs) {
		lv = dm_list_item(lvh, struct lv_list)->lv;

		if (lv_is_snapshot(lv))
			continue;

		if (lv_is_raid(lv))
			continue;

		if (lv_is_mirrored(lv))
			continue;

		if (lv_is_thin_pool(lv) ||
		    lv_is_thin_volume(lv))
			continue;

		if (lv_is_cache(lv) || lv_is_cache_pool(lv))
			/* further checks by _move_cache() */
			continue;

		/* Ensure all the PVs used by this LV remain in the same */
		/* VG as each other */
		vg_with = NULL;
		dm_list_iterate_items(seg, &lv->segments) {
			for (s = 0; s < seg->area_count; s++) {
				/* FIXME Check AREA_LV too */
				if (seg_type(seg, s) != AREA_PV)
					continue;

				pv = seg_pv(seg, s);
				if (vg_with) {
					if (!pv_is_in_vg(vg_with, pv)) {
						log_error("Can't split Logical "
							  "Volume %s between "
							  "two Volume Groups",
							  lv->name);
						return 0;
					}
					continue;
				}

				if (pv_is_in_vg(vg_from, pv)) {
					vg_with = vg_from;
					continue;
				}
				if (pv_is_in_vg(vg_to, pv)) {
					vg_with = vg_to;
					continue;
				}
				log_error("Physical Volume %s not found",
					  pv_dev_name(pv));
				return 0;
			}

		}

		if (vg_with == vg_from)
			continue;

		/* Move this LV */
		if (!_move_one_lv(vg_from, vg_to, lvh, &lvht))
			return_0;
	}

	/* FIXME Ensure no LVs contain segs pointing at LVs in the other VG */

	return 1;
}

/*
 * Move the hidden / internal "snapshotN" LVs.from 'vg_from' to 'vg_to'.
 */
static int _move_snapshots(struct volume_group *vg_from,
			   struct volume_group *vg_to)
{
	struct dm_list *lvh, *lvht;
	struct logical_volume *lv;
	struct lv_segment *seg;
	int cow_from = 0;
	int origin_from = 0;

	dm_list_iterate_safe(lvh, lvht, &vg_from->lvs) {
		lv = dm_list_item(lvh, struct lv_list)->lv;

		if (!lv_is_snapshot(lv))
			continue;

		dm_list_iterate_items(seg, &lv->segments) {
			cow_from = _lv_is_in_vg(vg_from, seg->cow);
			origin_from = _lv_is_in_vg(vg_from, seg->origin);

			if (cow_from && origin_from)
				continue;
			if ((!cow_from && origin_from) ||
			     (cow_from && !origin_from)) {
				log_error("Can't split snapshot %s between"
					  " two Volume Groups", seg->cow->name);
				return 0;
			}

			/*
			 * At this point, the cow and origin should already be
			 * in vg_to.
			 */
			if (_lv_is_in_vg(vg_to, seg->cow) &&
			    _lv_is_in_vg(vg_to, seg->origin)) {
				if (!_move_one_lv(vg_from, vg_to, lvh, &lvht))
					return_0;
			}
		}

	}

	return 1;
}

static int _move_mirrors(struct volume_group *vg_from,
			 struct volume_group *vg_to)
{
	struct dm_list *lvh, *lvht;
	struct logical_volume *lv;
	struct lv_segment *seg, *log_seg;
	unsigned s, seg_in, log_in;

	dm_list_iterate_safe(lvh, lvht, &vg_from->lvs) {
		lv = dm_list_item(lvh, struct lv_list)->lv;

		if (lv_is_raid(lv))
			continue;

		if (!lv_is_mirrored(lv))
			continue;

		/* Ignore, if no allocations on PVs of @vg_to */
		if (!lv_is_on_pvs(lv, &vg_to->pvs))
			continue;

		seg = first_seg(lv);

		seg_in = 0;
		for (s = 0; s < seg->area_count; s++)
			if (_lv_is_in_vg(vg_to, seg_lv(seg, s)))
				seg_in++;

		log_in = !seg->log_lv;
		if (seg->log_lv) {
			log_seg = first_seg(seg->log_lv);
			if (seg_is_mirrored(log_seg)) {
				log_in = 1;

				/* Ensure each log dev is in vg_to */
				for (s = 0; s < log_seg->area_count; s++)
					log_in = log_in &&
						_lv_is_in_vg(vg_to,
							     seg_lv(log_seg, s));
			} else
				log_in = _lv_is_in_vg(vg_to, seg->log_lv);
		}

		if ((seg_in && seg_in < seg->area_count) ||
		    (seg_in && seg->log_lv && !log_in) ||
		    (!seg_in && seg->log_lv && log_in)) {
			log_error("Can't split mirror %s between "
				  "two Volume Groups", lv->name);
			return 0;
		}

		if (seg_in == seg->area_count && log_in) {
			if (!_move_one_lv(vg_from, vg_to, lvh, &lvht))
				return_0;
		}
	}

	return 1;
}

/*
 * Check for any RAID LVs with allocations on PVs of @vg_to.
 *
 * If these don't have any allocations on PVs of @vg_from,
 * move their whole lv stack across to @vg_to including the
 * top-level RAID LV.
 */
static int _move_raids(struct volume_group *vg_from,
		       struct volume_group *vg_to)
{
	struct dm_list *lvh, *lvht;
	struct logical_volume *lv;

	dm_list_iterate_safe(lvh, lvht, &vg_from->lvs) {
		lv = dm_list_item(lvh, struct lv_list)->lv;

		if (!lv_is_raid(lv))
			continue;

		/* Ignore, if no allocations on PVs of @vg_to */
		if (!lv_is_on_pvs(lv, &vg_to->pvs))
			continue;
 
		/* If allocations are on PVs of @vg_to -> move RAID LV stack across */
		if (!_move_one_lv(vg_from, vg_to, lvh, &lvht))
			return_0;
	}

	return 1;
}

static int _move_thins(struct volume_group *vg_from,
		       struct volume_group *vg_to)
{
	struct dm_list *lvh, *lvht;
	struct logical_volume *lv, *data_lv;
	struct lv_segment *seg;

	dm_list_iterate_safe(lvh, lvht, &vg_from->lvs) {
		lv = dm_list_item(lvh, struct lv_list)->lv;

		if (lv_is_thin_volume(lv)) {
			seg = first_seg(lv);
			data_lv = seg_lv(first_seg(seg->pool_lv), 0);

			/* Ignore, if no allocations on PVs of @vg_to */
			if (!lv_is_on_pvs(data_lv, &vg_to->pvs) &&
			    (seg->external_lv && !lv_is_on_pvs(seg->external_lv, &vg_to->pvs)))
				continue;

			if ((_lv_is_in_vg(vg_to, data_lv) ||
			     _lv_is_in_vg(vg_to, seg->external_lv))) {
				if (_lv_is_in_vg(vg_from, seg->external_lv) ||
				    _lv_is_in_vg(vg_from, data_lv)) {
					log_error("Can't split external origin %s "
						  "and pool %s between two Volume Groups.",
						  display_lvname(seg->external_lv),
						  display_lvname(seg->pool_lv));
					return 0;
				}
				if (!_move_one_lv(vg_from, vg_to, lvh, &lvht))
					return_0;
			}
		} else if (lv_is_thin_pool(lv)) {
			seg = first_seg(lv);
			data_lv = seg_lv(seg, 0);

			/* Ignore, if no allocations on PVs of @vg_to */
			if (!lv_is_on_pvs(data_lv, &vg_to->pvs))
				continue;

			if (_lv_is_in_vg(vg_to, data_lv) ||
			    _lv_is_in_vg(vg_to, seg->metadata_lv)) {
				if (_lv_is_in_vg(vg_from, seg->metadata_lv) ||
				    _lv_is_in_vg(vg_from, data_lv)) {
					log_error("Can't split pool data and metadata %s "
						  "between two Volume Groups.",
						  lv->name);
					return 0;
				}
				if (!_move_one_lv(vg_from, vg_to, lvh, &lvht))
					return_0;
			}
		}
	}

	return 1;
}

static int _move_cache(struct volume_group *vg_from,
		       struct volume_group *vg_to)
{
	int is_moving;
	struct dm_list *lvh, *lvht;
	struct logical_volume *lv, *data, *meta, *orig;
	struct lv_segment *seg, *cache_seg;

	dm_list_iterate_safe(lvh, lvht, &vg_from->lvs) {
		lv = dm_list_item(lvh, struct lv_list)->lv;
		data = meta = orig = NULL;
		seg = first_seg(lv);

		if (!lv_is_cache(lv) && !lv_is_cache_pool(lv))
			continue;

		/*
		 * FIXME: The code seems to move cache LVs fine, but it
		 *        hasn't been well tested and it causes problems
		 *        when just splitting PVs that don't contain
		 *        cache LVs.
		 * Waiting for next release before fixing and enabling.
		 */
		log_error("Unable to split VG while it contains cache LVs");
		return 0;

		/* NOTREACHED */

		if (lv_is_cache(lv)) {
			orig = seg_lv(seg, 0);
			data = seg_lv(first_seg(seg->pool_lv), 0);
			meta = first_seg(seg->pool_lv)->metadata_lv;
			/* Ensure all components are coming along */
			is_moving = _lv_is_in_vg(vg_to, orig);
		} else {
			if (!dm_list_empty(&seg->lv->segs_using_this_lv) &&
			    !(cache_seg = get_only_segment_using_this_lv(seg->lv)))
				return_0;
			orig = seg_lv(cache_seg, 0);
			data = seg_lv(seg, 0);
			meta = seg->metadata_lv;

			if (_lv_is_in_vg(vg_to, data) ||
			    _lv_is_in_vg(vg_to, meta))
				is_moving = 1;
		}

		if (!lv_is_on_pvs(data, &vg_to->pvs))
			continue;

		if (!lv_is_on_pvs(meta, &vg_to->pvs))
			continue;

		if (orig && (_lv_is_in_vg(vg_to, orig) != is_moving)) {
			log_error("Can't split %s and its origin (%s)"
				  " into separate VGs", lv->name, orig->name);
			return 0;
		}

		if (data && (_lv_is_in_vg(vg_to, data) != is_moving)) {
			log_error("Can't split %s and its cache pool"
				  " data LV (%s) into separate VGs",
				  lv->name, data->name);
			return 0;
		}

		if (meta && (_lv_is_in_vg(vg_to, meta) != is_moving)) {
			log_error("Can't split %s and its cache pool"
				  " metadata LV (%s) into separate VGs",
				  lv->name, meta->name);
			return 0;
		}
		if (!_move_one_lv(vg_from, vg_to, lvh, &lvht))
			return_0;
	}

	return 1;
}

/*
 * Create or open the destination of the vgsplit operation.
 * Returns
 * - non-NULL: VG handle w/VG lock held
 * - NULL: no VG lock held
 */
static struct volume_group *_vgsplit_to(struct cmd_context *cmd,
					const char *vg_name_to,
					int *existing_vg)
{
	struct volume_group *vg_to = NULL;

	log_verbose("Checking for new volume group \"%s\"", vg_name_to);
	/*
	 * First try to create a new VG.  If we cannot create it,
	 * and we get FAILED_EXIST (we will not be holding a lock),
	 * a VG must already exist with this name.  We then try to
	 * read the existing VG - the vgsplit will be into an existing VG.
	 *
	 * Otherwise, if the lock was successful, it must be the case that
	 * we obtained a WRITE lock and could not find the vgname in the
	 * system.  Thus, the split will be into a new VG.
	 */
	vg_to = vg_lock_and_create(cmd, vg_name_to);
	if (vg_read_error(vg_to) == FAILED_LOCKING) {
		log_error("Can't get lock for %s", vg_name_to);
		release_vg(vg_to);
		return NULL;
	}
	if (vg_read_error(vg_to) == FAILED_EXIST) {
		*existing_vg = 1;
		release_vg(vg_to);
		vg_to = vg_read_for_update(cmd, vg_name_to, NULL, 0, 0);

		if (vg_read_error(vg_to)) {
			release_vg(vg_to);
			return_NULL;
		}

	} else if (vg_read_error(vg_to) == SUCCESS) {
		*existing_vg = 0;
	}
	return vg_to;
}

/*
 * Open the source of the vgsplit operation.
 * Returns
 * - non-NULL: VG handle w/VG lock held
 * - NULL: no VG lock held
 */
static struct volume_group *_vgsplit_from(struct cmd_context *cmd,
					  const char *vg_name_from)
{
	struct volume_group *vg_from;

	log_verbose("Checking for volume group \"%s\"", vg_name_from);

	vg_from = vg_read_for_update(cmd, vg_name_from, NULL, 0, 0);
	if (vg_read_error(vg_from)) {
		release_vg(vg_from);
		return NULL;
	}

	if (vg_is_shared(vg_from)) {
		log_error("vgsplit not allowed for lock_type %s", vg_from->lock_type);
		unlock_and_release_vg(cmd, vg_from, vg_name_from);
		return NULL;
	}

	return vg_from;
}

/*
 * Has the user given an option related to a new vg as the split destination?
 */
static int _new_vg_option_specified(struct cmd_context *cmd)
{
	return(arg_is_set(cmd, clustered_ARG) ||
	       arg_is_set(cmd, alloc_ARG) ||
	       arg_is_set(cmd, maxphysicalvolumes_ARG) ||
	       arg_is_set(cmd, maxlogicalvolumes_ARG) ||
	       arg_is_set(cmd, vgmetadatacopies_ARG));
}

int vgsplit(struct cmd_context *cmd, int argc, char **argv)
{
	struct vgcreate_params vp_new;
	struct vgcreate_params vp_def;
	const char *vg_name_from, *vg_name_to;
	struct volume_group *vg_to = NULL, *vg_from = NULL;
	int opt;
	int existing_vg = 0;
	int r = ECMD_FAILED;
	const char *lv_name;
	int lock_vg_from_first = 1;

	if ((arg_is_set(cmd, name_ARG) + argc) < 3) {
		log_error("Existing VG, new VG and either physical volumes "
			  "or logical volume required.");
		return EINVALID_CMD_LINE;
	}

	if (arg_is_set(cmd, name_ARG) && (argc > 2)) {
		log_error("A logical volume name cannot be given with "
			  "physical volumes.");
		return ECMD_FAILED;
	}

	/* Needed change the global VG namespace. */
	if (!lockd_gl(cmd, "ex", LDGL_UPDATE_NAMES))
		return_ECMD_FAILED;

	if (arg_is_set(cmd, name_ARG))
		lv_name = arg_value(cmd, name_ARG);
	else
		lv_name = NULL;

	vg_name_from = skip_dev_dir(cmd, argv[0], NULL);
	vg_name_to = skip_dev_dir(cmd, argv[1], NULL);
	argc -= 2;
	argv += 2;

	if (!strcmp(vg_name_to, vg_name_from)) {
		log_error("Duplicate volume group name \"%s\"", vg_name_from);
		return ECMD_FAILED;
	}

	lvmcache_label_scan(cmd);

	if (strcmp(vg_name_to, vg_name_from) < 0)
		lock_vg_from_first = 0;

	if (lock_vg_from_first) {
		if (!(vg_from = _vgsplit_from(cmd, vg_name_from)))
			return_ECMD_FAILED;
		/*
		 * Set metadata format of original VG.
		 * NOTE: We must set the format before calling vg_lock_and_create()
		 * since vg_lock_and_create() calls the per-format constructor.
		 */
		cmd->fmt = vg_from->fid->fmt;

		if (!(vg_to = _vgsplit_to(cmd, vg_name_to, &existing_vg))) {
			unlock_and_release_vg(cmd, vg_from, vg_name_from);
			return_ECMD_FAILED;
		}
	} else {
		if (!(vg_to = _vgsplit_to(cmd, vg_name_to, &existing_vg)))
			return_ECMD_FAILED;

		if (!(vg_from = _vgsplit_from(cmd, vg_name_from))) {
			unlock_and_release_vg(cmd, vg_to, vg_name_to);
			return_ECMD_FAILED;
		}

		if (cmd->fmt != vg_from->fid->fmt) {
			/* In this case we don't know the vg_from->fid->fmt */
			log_error("Unable to set new VG metadata type based on "
				  "source VG format - use -M option.");
			goto bad;
		}
	}

	if (existing_vg) {
		if (_new_vg_option_specified(cmd)) {
			log_error("Volume group \"%s\" exists, but new VG "
				    "option specified", vg_name_to);
			goto bad;
		}
		if (!vgs_are_compatible(cmd, vg_from,vg_to))
			goto_bad;
	} else {
		if (!vgcreate_params_set_defaults(cmd, &vp_def, vg_from)) {
			r = EINVALID_CMD_LINE;
			goto_bad;
		}
		vp_def.vg_name = vg_name_to;
		if (!vgcreate_params_set_from_args(cmd, &vp_new, &vp_def)) {
			r = EINVALID_CMD_LINE;
			goto_bad;
		}

		if (!vgcreate_params_validate(cmd, &vp_new)) {
			r = EINVALID_CMD_LINE;
			goto_bad;
		}

		if (!vg_set_extent_size(vg_to, vp_new.extent_size) ||
		    !vg_set_max_lv(vg_to, vp_new.max_lv) ||
		    !vg_set_max_pv(vg_to, vp_new.max_pv) ||
		    !vg_set_alloc_policy(vg_to, vp_new.alloc) ||
		    !vg_set_system_id(vg_to, vp_new.system_id) ||
		    !vg_set_mda_copies(vg_to, vp_new.vgmetadatacopies))
			goto_bad;
	}

	/* Archive vg_from before changing it */
	if (!archive(vg_from))
		goto_bad;

	/* Move PVs across to new structure */
	for (opt = 0; opt < argc; opt++) {
		dm_unescape_colons_and_at_signs(argv[opt], NULL, NULL);
		if (!move_pv(vg_from, vg_to, argv[opt]))
			goto_bad;
	}

	/* If an LV given on the cmdline, move used_by PVs */
	if (lv_name && !move_pvs_used_by_lv(vg_from, vg_to, lv_name))
		goto_bad;

	/*
	 * First move any required RAID LVs across recursively.
	 * Reject if they get split between VGs.
	 *
	 * This moves the whole LV stack across, thus _move_lvs() below
	 * ain't hit any of their MetaLVs/DataLVs any more but'll still
	 * work for all other type specific moves following it.
	 */
	if (!(_move_raids(vg_from, vg_to)))
		goto_bad;

	/* Move required sub LVs across, checking consistency */
	if (!(_move_lvs(vg_from, vg_to)))
		goto_bad;

	/* Move required mirrors across */
	if (!(_move_mirrors(vg_from, vg_to)))
		goto_bad;

	/* Move required pools across */
	if (!(_move_thins(vg_from, vg_to)))
		goto_bad;

	/* Move required cache LVs across */
	if (!(_move_cache(vg_from, vg_to)))
		goto_bad;

	/* Move required snapshots across */
	if (!(_move_snapshots(vg_from, vg_to)))
		goto_bad;

	/* Split metadata areas and check if both vgs have at least one area */
	if (!(vg_split_mdas(cmd, vg_from, vg_to)) && vg_from->pv_count) {
		log_error("Cannot split: Nowhere to store metadata for new Volume Group");
		goto bad;
	}

	/* Set proper name for all PVs in new VG */
	if (!vg_rename(cmd, vg_to, vg_name_to))
		goto_bad;

	/* Set old VG name so the metadata operations recognise that the PVs are in an existing VG */
	vg_to->old_name = vg_from->name;

	/* store it on disks */
	log_verbose("Writing out updated volume groups");

	/*
	 * First, write out the new VG as EXPORTED.  We do this first in case
	 * there is a crash - we will still have the new VG information, in an
	 * exported state.  Recovery after this point would importing and removal
	 * of the new VG and redoing the vgsplit.
	 * FIXME: recover automatically or instruct the user?
	 */
	vg_to->status |= EXPORTED_VG;

	if (!archive(vg_to))
		goto_bad;

	if (!vg_write(vg_to) || !vg_commit(vg_to))
		goto_bad;

	backup(vg_to);

	/*
	 * Next, write out the updated old VG.  If we crash after this point,
	 * recovery is a vgimport on the new VG.
	 * FIXME: recover automatically or instruct the user?
	 */
	if (vg_from->pv_count) {
		if (!vg_write(vg_from) || !vg_commit(vg_from))
			goto_bad;

		backup(vg_from);
	}

	/*
	 * Finally, remove the EXPORTED flag from the new VG and write it out.
	 * We need to unlock vg_to because vg_read_for_update wants to lock it.
	 */
	if (!test_mode()) {
		unlock_vg(cmd, NULL, vg_name_to);
		release_vg(vg_to);
		vg_to = vg_read_for_update(cmd, vg_name_to, NULL,
					   READ_ALLOW_EXPORTED, 0);
		if (vg_read_error(vg_to)) {
			log_error("Volume group \"%s\" became inconsistent: "
				  "please fix manually", vg_name_to);
			goto bad;
		}
	}

	vg_to->status &= ~EXPORTED_VG;

	if (!vg_write(vg_to) || !vg_commit(vg_to))
		goto_bad;

	backup(vg_to);

	log_print_unless_silent("%s volume group \"%s\" successfully split from \"%s\"",
				existing_vg ? "Existing" : "New",
				vg_to->name, vg_from->name);

	r = ECMD_PROCESSED;

bad:
	/*
	 * vg_to references elements moved from vg_from
	 * so vg_to has to be freed first.
	 */
	unlock_and_release_vg(cmd, vg_to, vg_name_to);
	unlock_and_release_vg(cmd, vg_from, vg_name_from);

	return r;
}
