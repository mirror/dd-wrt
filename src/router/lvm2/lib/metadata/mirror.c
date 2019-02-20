/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2008 Red Hat, Inc. All rights reserved.
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

#include "lib/misc/lib.h"
#include "lib/metadata/metadata.h"
#include "lib/commands/toolcontext.h"
#include "lib/metadata/segtype.h"
#include "lib/display/display.h"
#include "lib/format_text/archiver.h"
#include "lib/activate/activate.h"
#include "lib/metadata/lv_alloc.h"
#include "lib/misc/lvm-string.h"
#include "lib/datastruct/str_list.h"
#include "lib/locking/locking.h"

#include "lib/config/defaults.h"

/* These are necessary for _write_log_header() */
#include "lib/mm/xlate.h"
#define MIRROR_MAGIC 0x4D695272
#define MIRROR_DISK_VERSION 2

/* These are the flags that represent the mirror failure restoration policies */
#define MIRROR_REMOVE		 0
#define MIRROR_ALLOCATE		 1
#define MIRROR_ALLOCATE_ANYWHERE 2

/*
 * Returns true if the lv is temporary mirror layer for resync
 */
int is_temporary_mirror_layer(const struct logical_volume *lv)
{
	if (lv_is_mirror_image(lv) && lv_is_mirrored(lv) && !lv_is_locked(lv))
		return 1;

	return 0;
}

/*
 * Return a temporary LV for resyncing added mirror image.
 * Add other mirror legs to lvs list.
 */
struct logical_volume *find_temporary_mirror(const struct logical_volume *lv)
{
	struct lv_segment *seg;

	if (!lv_is_mirrored(lv))
		return NULL;

	seg = first_seg(lv);

	/* Temporary mirror is always area_num == 0 */
	if (seg_type(seg, 0) == AREA_LV &&
	    is_temporary_mirror_layer(seg_lv(seg, 0)))
		return seg_lv(seg, 0);

	return NULL;
}

/*
 * cluster_mirror_is_available
 *
 * Check if the proper kernel module and log daemon are running.
 * Caller should check for 'vg_is_clustered(lv->vg)' before making
 * this call.
 *
 * Returns: 1 if available, 0 otherwise
 */
int cluster_mirror_is_available(struct cmd_context *cmd)
{
       unsigned attr = 0;
       const struct segment_type *segtype;

       if (!(segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_MIRROR)))
               return_0;

       if (!segtype->ops->target_present)
               return_0;

       if (!segtype->ops->target_present(cmd, NULL, &attr))
               return_0;

       if (!(attr & MIRROR_LOG_CLUSTERED))
               return 0;

       return 1;
}

/*
 * Returns the number of mirrors of the LV
 */
uint32_t lv_mirror_count(const struct logical_volume *lv)
{
	struct lv_segment *seg;
	uint32_t s, mirrors;

	if (!lv_is_mirrored(lv))
		return 1;

	seg = first_seg(lv);

	/* FIXME: RAID10 only supports 2 copies right now */
	if (seg_is_raid10(seg))
		return 2;

	if (lv_is_pvmove(lv))
		return seg->area_count;

	mirrors = 0;

	for (s = 0; s < seg->area_count; s++) {
		if (seg_type(seg, s) != AREA_LV)
			continue;
		if (is_temporary_mirror_layer(seg_lv(seg, s)))
			mirrors += lv_mirror_count(seg_lv(seg, s));
		else
			mirrors++;
	}

	return mirrors ? mirrors : 1;
}

struct lv_segment *find_mirror_seg(struct lv_segment *seg)
{
	struct lv_segment *mirror_seg;

	if (!(mirror_seg = get_only_segment_using_this_lv(seg->lv))) {
		log_error("Failed to find mirror_seg for %s", display_lvname(seg->lv));
		return NULL;
	}

	if (!seg_is_mirrored(mirror_seg)) {
		log_error("LV %s on %s is not a mirror segments.",
			  display_lvname(mirror_seg->lv),
			  display_lvname(seg->lv));
		return NULL;
	}

	return mirror_seg;
}

/*
 * Reduce the region size if necessary to ensure
 * the volume size is a multiple of the region size.
 *
 * For internal use only log only in verbose mode
 */
uint32_t adjusted_mirror_region_size(struct cmd_context *cmd,
				     uint32_t extent_size, uint32_t extents,
				     uint32_t region_size, int internal, int clustered)
{
	uint64_t region_max, region_min;
	uint32_t region_min_pow2;

	region_max = (uint64_t) extents * extent_size;

	if (region_max < UINT32_MAX && region_size > region_max) {
		region_size =  UINT64_C(1) << (31 - clz(region_max));
		if (!internal)
			log_print_unless_silent("Using reduced mirror region size of %s",
						display_size(cmd, region_size));
		else
			log_verbose("Using reduced mirror region size of %s",
				    display_size(cmd, region_size));
	}

#ifdef CMIRROR_REGION_COUNT_LIMIT
	if (clustered) {
		/*
		 * The CPG code used by cluster mirrors can only handle a
		 * payload of < 1MB currently.  (This deficiency is tracked by
		 * http://bugzilla.redhat.com/682771.)  The region size for cluster
		 * mirrors must be restricted in such a way as to limit the
		 * size of the bitmap to < 512kB, because there are two bitmaps
		 * which get sent around during checkpointing while a cluster
		 * mirror starts up.  Ergo, the number of regions must not
		 * exceed 512k * 8.  We also need some room for the other
		 * checkpointing structures as well, so we reduce by another
		 * factor of two.
		 *
		 * This code should be removed when the CPG restriction is
		 * lifted.
		 */
		region_min = region_max / CMIRROR_REGION_COUNT_LIMIT;
		if (region_min > UINT32_MAX / 2) {
			log_error("Can't find proper region size for too big mirror.");
			return 0;
		}
		region_min_pow2 = UINT64_C(1) << (1 + 31 - clz(region_min));

		if (region_size < region_min_pow2) {
			if (internal)
				log_print_unless_silent("Increasing mirror region size from %s to %s",
							display_size(cmd, region_size),
							display_size(cmd, region_min_pow2));
			else
				log_verbose("Increasing mirror region size from %s to %s",
					    display_size(cmd, region_size),
					    display_size(cmd, region_min_pow2));
			region_size = region_min_pow2;
		}
	}
#endif /* CMIRROR_REGION_COUNT_LIMIT */

	return region_size;
}

/*
 * shift_mirror_images
 * @mirrored_seg
 * @mimage:  The position (index) of the image to move to the end
 *
 * When dealing with removal of legs, we often move a 'removable leg'
 * to the back of the 'areas' array.  It is critically important not
 * to simply swap it for the last area in the array.  This would have
 * the affect of reordering the remaining legs - altering position of
 * the primary.  So, we must shuffle all of the areas in the array
 * to maintain their relative position before moving the 'removable
 * leg' to the end.
 *
 * Short illustration of the problem:
 *   - Mirror consists of legs A, B, C and we want to remove A
 *   - We swap A and C and then remove A, leaving C, B
 * This scenario is problematic in failure cases where A dies, because
 * B becomes the primary.  If the above happens, we effectively throw
 * away any changes made between the time of failure and the time of
 * restructuring the mirror.
 *
 * So, any time we want to move areas to the end to be removed, use
 * this function.
 */
int shift_mirror_images(struct lv_segment *mirrored_seg, unsigned mimage)
{
	unsigned i;
	struct lv_segment_area area;

	if (mimage >= mirrored_seg->area_count) {
		log_error("Invalid index (%u) of mirror image supplied "
			  "to shift_mirror_images().", mimage);
		return 0;
	}

	area = mirrored_seg->areas[mimage];

	/* Shift remaining images down to fill the hole */
	for (i = mimage + 1; i < mirrored_seg->area_count; i++)
		mirrored_seg->areas[i-1] = mirrored_seg->areas[i];

	/* Place this one at the end */
	mirrored_seg->areas[i-1] = area;

	return 1;
}

/*
 * This function writes a new header to the mirror log header to the lv
 *
 * Returns: 1 on success, 0 on failure
 */
static int _write_log_header(struct cmd_context *cmd, struct logical_volume *lv)
{
	struct device *dev;
	char name[PATH_MAX];
	struct { /* The mirror log header */
		uint32_t magic;
		uint32_t version;
		uint64_t nr_regions;
	} log_header;

	log_header.magic = xlate32(MIRROR_MAGIC);
	log_header.version = xlate32(MIRROR_DISK_VERSION);
	log_header.nr_regions = xlate64((uint64_t)-1);

	if (dm_snprintf(name, sizeof(name), "%s%s/%s", cmd->dev_dir,
			lv->vg->name, lv->name) < 0) {
		log_error("Device path name too long - log header not written (%s).",
			  display_lvname(lv));
		return 0;
	}

	log_verbose("Writing log header for LV %s to device %s.", display_lvname(lv), name);

	if (!(dev = dev_cache_get(cmd, name, NULL))) {
		log_error("%s: not found: log header not written.", name);
		return 0;
	}

	if (!label_scan_open(dev)) {
		log_error("Failed to open %s/%s to write log header.", lv->vg->name, lv->name);
		return 0;
	}

	dev_set_last_byte(dev, sizeof(log_header));

	if (!dev_write_bytes(dev, UINT64_C(0), sizeof(log_header), &log_header)) {
		dev_unset_last_byte(dev);
		log_error("Failed to write log header to %s.", name);
		return 0;
	}
	dev_unset_last_byte(dev);

	label_scan_invalidate(dev);

	return 1;
}

/*
 * Initialize mirror log contents
 */
static int _init_mirror_log(struct cmd_context *cmd,
			    struct logical_volume *lock_holder,
			    struct logical_volume *log_lv, int in_sync,
			    struct dm_list *tagsl, int remove_on_failure)
{
	struct dm_str_list *sl;

	if (log_lv != lv_lock_holder(log_lv) || !lv_is_visible(log_lv)) {
		/* Expect fully visible device for init */
		log_error(INTERNAL_ERROR "Log LV %s is not top level LV for initialization.",
			  display_lvname(log_lv));
		return 0;
	}

	if (test_mode()) {
		log_verbose("Test mode: Skipping mirror log initialisation.");
		return 1;
	}

	if (!activation() && in_sync) {
		log_error("Aborting. Unable to create in-sync mirror log "
			  "while activation is disabled.");
		return 0;
	}

	/* Temporary tag mirror log for activation */
	dm_list_iterate_items(sl, tagsl)
		if (!str_list_add(log_lv->vg->vgmem, &log_lv->tags, sl->str)) {
			log_error("Aborting. Unable to tag mirror log.");
			return 0;
		}

	/* store mirror log on disk(s) */
	if (!lock_holder) {
		if (!vg_write(log_lv->vg) || !vg_commit(log_lv->vg))
			return_0;
	} else if (!lv_update_and_reload((struct logical_volume*) lock_holder))
		return_0;

	if (!activate_lv(cmd, log_lv)) {
		log_error("Aborting. Failed to activate mirror log.");
		goto revert_new_lv;
	}

	if (activation()) {
		if (!wipe_lv(log_lv, (struct wipe_params)
			     { .do_zero = 1, .zero_sectors = log_lv->size,
			       .zero_value = in_sync ? -1 : 0 })) {
			log_error("Aborting. Failed to wipe mirror log.");
			goto deactivate_and_revert_new_lv;
		}

		if (!_write_log_header(cmd, log_lv)) {
			log_error("Aborting. Failed to write mirror log header.");
			goto deactivate_and_revert_new_lv;
		}
	}

	if (!deactivate_lv(cmd, log_lv)) {
		log_error("Aborting. Failed to deactivate mirror log. "
			  "Manual intervention required.");
		goto revert_new_lv;
	}

	/* Wait for events following any deactivation before reactivating */
	if (!sync_local_dev_names(cmd)) {
		log_error("Aborting. Failed to sync local devices before initialising mirror log %s.",
			  display_lvname(log_lv));
		goto revert_new_lv;
	}

	/* Remove the temporary tags */
	dm_list_iterate_items(sl, tagsl)
		str_list_del(&log_lv->tags, sl->str);

	return 1;

deactivate_and_revert_new_lv:
	if (!deactivate_lv(cmd, log_lv)) {
		log_error("Unable to deactivate mirror log LV. "
			  "Manual intervention required.");
		return 0;
	}

revert_new_lv:
	dm_list_iterate_items(sl, tagsl)
		str_list_del(&log_lv->tags, sl->str);

	if (remove_on_failure && !lv_remove(log_lv)) {
		log_error("Manual intervention may be required to remove "
			  "abandoned log LV before retrying.");
		return 0;
	}

	if (!vg_write(log_lv->vg) || !vg_commit(log_lv->vg))
		log_error("Manual intervention may be required to "
			  "remove/restore abandoned log LV before retrying.");
	else
		backup(log_lv->vg);

	return 0;
}

/*
 * Activate an LV similarly (i.e. SH or EX) to a given "model" LV
 */
static int _activate_lv_like_model(struct logical_volume *model,
				   struct logical_volume *lv)
{
	/* FIXME: run all cases through lv_active_change when clvm variants are gone. */

	if (vg_is_shared(lv->vg))
		return lv_active_change(lv->vg->cmd, lv, CHANGE_AEY);

	if (!activate_lv(lv->vg->cmd, lv))
		return_0;
	return 1;
}

/*
 * Delete independent/orphan LV, it must acquire lock.
 */
static int _delete_lv(struct logical_volume *mirror_lv, struct logical_volume *lv,
		      int reactivate)
{
	struct cmd_context *cmd = mirror_lv->vg->cmd;
	struct dm_str_list *sl;

	/* Inherit tags - maybe needed for activation */
	if (!str_list_match_list(&mirror_lv->tags, &lv->tags, NULL)) {
		dm_list_iterate_items(sl, &mirror_lv->tags)
			if (!str_list_add(cmd->mem, &lv->tags, sl->str)) {
				log_error("Aborting. Unable to tag.");
				return 0;
			}

		if (!vg_write(mirror_lv->vg) ||
		    !vg_commit(mirror_lv->vg)) {
			log_error("Intermediate VG commit for orphan volume failed.");
			return 0;
		}
	}

	if (reactivate) {
		/* FIXME: the 'model' should be 'mirror_lv' not 'lv', I think. */
		if (!_activate_lv_like_model(lv, lv))
			return_0;

		/* FIXME Is this superfluous now? */
		if (!sync_local_dev_names(cmd)) {
			log_error("Failed to sync local devices when reactivating %s.",
				  display_lvname(lv));
			return 0;
		}

		if (!deactivate_lv(cmd, lv))
			return_0;
	}

	if (!lv_remove(lv))
		return_0;

	return 1;
}

static int _merge_mirror_images(struct logical_volume *lv,
				const struct dm_list *mimages)
{
	uint32_t addition = dm_list_size(mimages);
	struct logical_volume **img_lvs;
	struct lv_list *lvl;
	int i = 0;

	if (!addition)
		return 1;

	img_lvs = alloca(sizeof(*img_lvs) * addition);

	dm_list_iterate_items(lvl, mimages)
		img_lvs[i++] = lvl->lv;

	return lv_add_mirror_lvs(lv, img_lvs, addition,
				 MIRROR_IMAGE, first_seg(lv)->region_size);
}

/* Unlink the relationship between the segment and its log_lv */
struct logical_volume *detach_mirror_log(struct lv_segment *mirrored_seg)
{
	struct logical_volume *log_lv;

	if (!mirrored_seg->log_lv)
		return NULL;

	log_lv = mirrored_seg->log_lv;
	mirrored_seg->log_lv = NULL;
	lv_set_visible(log_lv);
	log_lv->status &= ~MIRROR_LOG;
	if (!remove_seg_from_segs_using_this_lv(log_lv, mirrored_seg))
		return_0;

	return log_lv;
}

/* Check if mirror image LV is removable with regard to given removable_pvs */
int is_mirror_image_removable(struct logical_volume *mimage_lv, void *baton)
{
	struct physical_volume *pv;
	struct lv_segment *seg;
	int pv_found;
	struct pv_list *pvl;
	uint32_t s;
	struct dm_list *removable_pvs = baton;

	if (!baton || dm_list_empty(removable_pvs))
		return 1;

	dm_list_iterate_items(seg, &mimage_lv->segments) {
		for (s = 0; s < seg->area_count; s++) {
			if (seg_type(seg, s) != AREA_PV) {
				/* FIXME Recurse for AREA_LV? */
				/* Structure of seg_lv is unknown.
				 * Not removing this LV for safety. */
				return 0;
			}

			pv = seg_pv(seg, s);

			pv_found = 0;
			dm_list_iterate_items(pvl, removable_pvs) {
				if (id_equal(&pv->id, &pvl->pv->id)) {
					pv_found = 1;
					break;
				}
				if (pvl->pv->dev && pv->dev &&
				    pv->dev->dev == pvl->pv->dev->dev) {
					pv_found = 1;
					break;
				}
			}
			if (!pv_found)
				return 0;
		}
	}

	return 1;
}

/*
 * _move_removable_mimages_to_end
 *
 * We always detach mimage LVs from the end of the areas array.
 * This function will push 'count' mimages to the end of the array
 * based on if their PVs are removable.
 *
 * This is an all or nothing function.  Either the user specifies
 * enough removable PVs to satisfy count, or they don't specify
 * any removable_pvs at all (in which case all PVs in the mirror
 * are considered removable).
 */
static int _move_removable_mimages_to_end(struct logical_volume *lv,
					  uint32_t count,
					  struct dm_list *removable_pvs)
{
	int i;
	struct logical_volume *sub_lv;
	struct lv_segment *mirrored_seg = first_seg(lv);

	if (!removable_pvs)
		return 1;

	for (i = mirrored_seg->area_count - 1; (i >= 0) && count; i--) {
		sub_lv = seg_lv(mirrored_seg, i);

		if (!is_temporary_mirror_layer(sub_lv) &&
		    is_mirror_image_removable(sub_lv, removable_pvs)) {
			if (!shift_mirror_images(mirrored_seg, i))
				return_0;
			count--;
		}
	}

	return !count;
}

static int _mirrored_lv_in_sync(struct logical_volume *lv)
{
	dm_percent_t sync_percent;

	if (!lv_mirror_percent(lv->vg->cmd, lv, 0, &sync_percent,
			       NULL)) {
		log_error("Unable to determine mirror sync status of %s.", display_lvname(lv));
		return 0;
	}

	return (sync_percent == DM_PERCENT_100) ? 1 : 0;
}

/*
 * Split off 'split_count' legs from a mirror
 *
 * Returns: 0 on error, 1 on success
 */
static int _split_mirror_images(struct logical_volume *lv,
				const char *split_name,
				uint32_t split_count,
				struct dm_list *removable_pvs)
{
	uint32_t i;
	struct logical_volume *sub_lv = NULL;
	struct logical_volume *new_lv = NULL;
	struct logical_volume *detached_log_lv = NULL;
	struct lv_segment *mirrored_seg = first_seg(lv);
	struct dm_list split_images;
	struct lv_list *lvl;
	struct cmd_context *cmd = lv->vg->cmd;
	char layer_name[NAME_LEN], format[NAME_LEN];
	int act;

	if (!lv_is_mirrored(lv)) {
		log_error("Unable to split non-mirrored LV %s.",
			  display_lvname(lv));
		return 0;
	}

	if (!split_count) {
		log_error(INTERNAL_ERROR "split_count is zero!");
		return 0;
	}

	log_verbose("Detaching %d images from mirror %s.",
		    split_count, display_lvname(lv));

	if (!_move_removable_mimages_to_end(lv, split_count, removable_pvs)) {
		/*
		 * FIXME: Allow incomplete specification of removable PVs?
		 *
		 * I am forcing the user to either specify no
		 * removable PVs or all of them.  Should we allow
		 * them to just specify some - making us pick the rest?
		 */
		log_error("Insufficient removable PVs given to satisfy request.");
		return 0;
	}

	/*
	 * Step 1:
	 *   Remove the images from the mirror.
	 *   Make them visible, independent LVs (don't change names yet).
	 *   Track them in a list for later instantiation.
	 */
	dm_list_init(&split_images);
	for (i = 0; i < split_count; i++) {
		mirrored_seg->area_count--;
		sub_lv = seg_lv(mirrored_seg, mirrored_seg->area_count);

		sub_lv->status &= ~MIRROR_IMAGE;
		if (!release_lv_segment_area(mirrored_seg, mirrored_seg->area_count, mirrored_seg->area_len))
			return_0;

		log_very_verbose("LV %s assigned to be split.", display_lvname(sub_lv));

		if (!new_lv) {
			lv_set_visible(sub_lv);
			new_lv = sub_lv;
			continue;
		}

		/* If there is more than one image being split, add to list */
		lvl = dm_pool_alloc(lv->vg->vgmem, sizeof(*lvl));
		if (!lvl) {
			log_error("lv_list alloc failed.");
			return 0;
		}
		lvl->lv = sub_lv;
		dm_list_add(&split_images, &lvl->list);
	}

	new_lv->name = dm_pool_strdup(lv->vg->vgmem, split_name);
	if (!new_lv->name) {
		log_error("Unable to rename newly split LV.");
		return 0;
	}

	if (lv->vg->lock_type && !strcmp(lv->vg->lock_type, "dlm"))
		new_lv->lock_args = lv->lock_args;

	if (!dm_list_empty(&split_images)) {
		/*
		 * A number of images have been split and
		 * a new mirror layer must be formed
		 */

		if (!insert_layer_for_lv(cmd, new_lv, 0, "_mimage_%d")) {
			log_error("Failed to build new mirror, %s.",
				  display_lvname(new_lv));
			return 0;
		}

		first_seg(new_lv)->region_size = mirrored_seg->region_size;

		dm_list_iterate_items(lvl, &split_images) {
			sub_lv = lvl->lv;

			if (dm_snprintf(format, sizeof(format), "%s_mimage_%%d",
					new_lv->name) < 0) {
				log_error("Failed to build new image name for %s.",
					  display_lvname(new_lv));
				return 0;
			}
			if (!generate_lv_name(lv->vg, format, layer_name, sizeof(layer_name))) {
				log_error("Failed to generate new image names for %s.",
					  display_lvname(new_lv));
				return 0;
			}
			if (!(sub_lv->name = dm_pool_strdup(lv->vg->vgmem, layer_name))) {
				log_error("Unable to allocate memory.");
				return 0;
			}
		}

		if (!_merge_mirror_images(new_lv, &split_images)) {
			log_error("Failed to group split images into new mirror.");
			return 0;
		}

		/*
		 * We don't allow splitting a mirror that is not in-sync,
		 * so we can bring the newly split mirror up without a
		 * resync.  (It will be a 'core' log mirror after all.)
		 */
		init_mirror_in_sync(1);
	}

	sub_lv = NULL;

	/*
	 * If no more mirrors, remove mirror layer.
	 * The sub_lv is removed entirely later - leaving
	 * only the top-level (now linear) LV.
	 */
	if (mirrored_seg->area_count == 1) {
		sub_lv = seg_lv(mirrored_seg, 0);
		sub_lv->status &= ~MIRROR_IMAGE;
		lv_set_visible(sub_lv);
		detached_log_lv = detach_mirror_log(mirrored_seg);
		if (!remove_layer_from_lv(lv, sub_lv))
			return_0;
		lv->status &= ~(MIRROR | MIRRORED | LV_NOTSYNCED);
	}

	/*
	 * Suspend and resume the mirror - this includes all
	 * the sub-LVs and soon-to-be-split sub-LVs
	 */
	if (!lv_update_and_reload(lv))
		return_0;

	act = lv_is_active(lv_lock_holder(lv));

	if (act && (!deactivate_lv(cmd, new_lv) || !_activate_lv_like_model(lv, new_lv))) {
		log_error("Failed to rename newly split LV in the kernel");
		return 0;
	}

	/* Remove original mirror layer if it has been converted to linear */
	if (sub_lv && !_delete_lv(lv, sub_lv, act))
		return_0;

	/* Remove the log if it has been converted to linear */
	if (detached_log_lv && !_delete_lv(lv, detached_log_lv, act))
		return_0;

	return 1;
}

/*
 * Remove num_removed images from mirrored_seg
 *
 * Arguments:
 *   num_removed:   the requested (maximum) number of mirrors to be removed
 *   removable_pvs: if not NULL and list not empty, only mirrors using PVs
 *                  in this list will be removed
 *   remove_log:    if non-zero, log_lv will be removed
 *                  (even if it's 0, log_lv will be removed if there is no
 *                   mirror remaining after the removal)
 *   collapse:      if non-zero, instead of removing, remove the temporary
 *                  mirror layer and merge mirrors to the original LV.
 *                  removable_pvs should be NULL and num_removed should be
 *                  seg->area_count - 1.
 *   removed:       if non NULL, the number of removed mirror images is set
 *                  as a result
 *
 * If collapse is non-zero, <removed> is guaranteed to be equal to num_removed.
 *
 * Return values:
 *   Failure (0) means something unexpected has happend and
 *   the caller should abort.
 *   Even if no mirror was removed (e.g. no LV matches to 'removable_pvs'),
 *   returns success (1).
 */
static int _remove_mirror_images(struct logical_volume *lv,
				 uint32_t num_removed,
				 int (*is_removable)(struct logical_volume *, void *),
				 void *removable_baton,
				 unsigned remove_log, unsigned collapse,
				 uint32_t *removed, int preferred_only)
{
	uint32_t m;
	int32_t s;
	struct logical_volume *sub_lv;
	struct logical_volume *detached_log_lv = NULL;
	struct logical_volume *temp_layer_lv = NULL;
	struct lv_segment *seg, *pvmove_seg, *mirrored_seg = first_seg(lv);
	uint32_t old_area_count = mirrored_seg->area_count;
	uint32_t new_area_count = mirrored_seg->area_count;
	struct lv_list *lvl;
	struct dm_list tmp_orphan_lvs;
	uint32_t orig_removed = num_removed;
	int reactivate;

	if (removed)
		*removed = 0;

	log_very_verbose("Reducing mirror set %s from " FMTu32 " to " FMTu32
			 " image(s)%s.", display_lvname(lv),
			 old_area_count, old_area_count - num_removed,
			 remove_log ? " and no log volume" : "");

	if (collapse && (old_area_count - num_removed != 1)) {
		log_error("Incompatible parameters to _remove_mirror_images.");
		return 0;
	}

	num_removed = 0;

	/* Move removable_pvs to end of array */
	for (s = mirrored_seg->area_count - 1;
	     s >= 0 && old_area_count - new_area_count < orig_removed;
	     s--) {
		sub_lv = seg_lv(mirrored_seg, s);
		if (!(is_temporary_mirror_layer(sub_lv) && lv_mirror_count(sub_lv) != 1)) {
			if (!is_removable) {
				log_error(INTERNAL_ERROR "_remove_mirror_images called incorrectly with is_removable undefined.");
				return 0;
			}
			if (!is_removable(sub_lv, removable_baton))
				continue;
			/*
			 * Check if the user is trying to pull the
			 * primary mirror image when the mirror is
			 * not in-sync.
			 */
			if ((s == 0) && !_mirrored_lv_in_sync(lv) &&
			    !(lv_is_partial(lv))) {
				log_error("Unable to remove primary mirror image while mirror volume "
					  "%s is not in-sync.", display_lvname(lv));
				return 0;
			}
			if (!shift_mirror_images(mirrored_seg, s))
				return_0;
			--new_area_count;
			++num_removed;
		}
	}

	if (!preferred_only)
		num_removed = orig_removed;

	/*
	 * If removable_pvs were specified, then they have been shifted
	 * to the end to ensure they are removed.  The remaining balance
	 * of images left to remove will be taken from the unspecified.
	 */
	new_area_count = old_area_count - num_removed;

	if (num_removed && old_area_count == new_area_count)
		return 1;

	/* Remove mimage LVs from the segment */
	dm_list_init(&tmp_orphan_lvs);
	for (m = new_area_count; m < mirrored_seg->area_count; m++) {
		seg_lv(mirrored_seg, m)->status &= ~MIRROR_IMAGE;
		lv_set_visible(seg_lv(mirrored_seg, m));
		if (!(lvl = dm_pool_alloc(lv->vg->cmd->mem, sizeof(*lvl)))) {
			log_error("lv_list alloc failed.");
			return 0;
		}
		lvl->lv = seg_lv(mirrored_seg, m);
		dm_list_add(&tmp_orphan_lvs, &lvl->list);
		if (!release_lv_segment_area(mirrored_seg, m, mirrored_seg->area_len))
			return_0;
	}
	mirrored_seg->area_count = new_area_count;

	/* If no more mirrors, remove mirror layer */
	/* As an exceptional case, if the lv is temporary layer,
	 * leave the LV as mirrored and let the lvconvert completion
	 * to remove the layer. */
	if (new_area_count == 1 && !is_temporary_mirror_layer(lv)) {
		temp_layer_lv = seg_lv(mirrored_seg, 0);
		temp_layer_lv->status &= ~MIRROR_IMAGE;
		lv_set_visible(temp_layer_lv);
		detached_log_lv = detach_mirror_log(mirrored_seg);
		if (!remove_layer_from_lv(lv, temp_layer_lv))
			return_0;
		if (collapse && !_merge_mirror_images(lv, &tmp_orphan_lvs)) {
			log_error("Failed to add mirror images.");
			return 0;
		}
		/*
		 * No longer a mirror? Even though new_area_count was 1,
		 * _merge_mirror_images may have resulted into lv being still a
		 * mirror. Fix up the flags if we only have one image left.
		 */
		if (lv_mirror_count(lv) == 1)
			lv->status &= ~(MIRROR | MIRRORED | LV_NOTSYNCED);

		mirrored_seg = first_seg(lv);
		if (remove_log && !detached_log_lv)
			detached_log_lv = detach_mirror_log(mirrored_seg);

		if (lv_is_pvmove(lv))
			dm_list_iterate_items(pvmove_seg, &lv->segments)
				pvmove_seg->status |= PVMOVE;
	} else if (new_area_count == 0) {
		log_very_verbose("All mimages of %s are gone.", display_lvname(lv));

		/* All mirror images are gone.
		 * It can happen for vgreduce --removemissing. */
		detached_log_lv = detach_mirror_log(mirrored_seg);
		lv->status &= ~(MIRROR | MIRRORED | LV_NOTSYNCED);
		if (!replace_lv_with_error_segment(lv))
			return_0;
	} else if (remove_log)
		detached_log_lv = detach_mirror_log(mirrored_seg);

	/*
	 * The log may be removed due to repair.  If the log
	 * happens to be a mirrored log, then there is a special
	 * case we need to consider.  One of the images of a
	 * mirrored log can fail followed shortly afterwards by
	 * a failure of the second.  This means that the top-level
	 * mirror is waiting for writes to the log to finish, but
	 * they never will unless the mirrored log can be repaired
	 * or replaced with an error target.  Since both the devices
	 * have failed, we must replace with error target - it is
	 * the only way to release the pending writes.
	 */
	if (detached_log_lv && lv_is_mirrored(detached_log_lv) &&
	    lv_is_partial(detached_log_lv)) {
		seg = first_seg(detached_log_lv);

		log_very_verbose("%s being removed due to failures.",
				 display_lvname(detached_log_lv));

		/*
		 * We are going to replace the mirror with an
		 * error segment, but before we do, we must remember
		 * all of the LVs that must be deleted later (i.e.
		 * the sub-lv's)
		 */
		for (m = 0; m < seg->area_count; m++) {
			if (!(lvl = dm_pool_alloc(lv->vg->cmd->mem,
						  sizeof(*lvl))))
				return_0;

			seg_lv(seg, m)->status &= ~MIRROR_IMAGE;
			lv_set_visible(seg_lv(seg, m));
			lvl->lv = seg_lv(seg, m);
			dm_list_add(&tmp_orphan_lvs, &lvl->list);
		}

		if (!replace_lv_with_error_segment(detached_log_lv)) {
			log_error("Failed error target substitution for %s.",
				  display_lvname(detached_log_lv));
			return 0;
		}

		if (!lv_update_and_reload(detached_log_lv))
			return_0;
	}

	/*
	 * To successfully remove these unwanted LVs we need to
	 * remove the LVs from the mirror set, commit that metadata
	 * then deactivate and remove them fully.
	 */
	if (!lv_update_and_reload_origin(mirrored_seg->lv))
		return_0;

	/* Save or delete the 'orphan' LVs */
	reactivate = lv_is_active(lv_lock_holder(lv));
	if (!collapse) {
		dm_list_iterate_items(lvl, &tmp_orphan_lvs)
			if (!_delete_lv(lv, lvl->lv, reactivate))
				return_0;
	}

	if (temp_layer_lv && !_delete_lv(lv, temp_layer_lv, reactivate))
		return_0;

	if (detached_log_lv && !_delete_lv(lv, detached_log_lv, reactivate))
		return_0;

	/* Mirror with only 1 area is 'in sync'. */
	if (new_area_count == 1 && is_temporary_mirror_layer(lv)) {
		detached_log_lv = detach_mirror_log(mirrored_seg);
		if (!_init_mirror_log(lv->vg->cmd,
				      (struct logical_volume*)lv_lock_holder(mirrored_seg->lv),
				      detached_log_lv,
				      1, &lv->tags, 0)) {
			/* As a result, unnecessary sync may run after
			 * collapsing. But safe.*/
			log_error("Failed to initialize log device %s.",
				  display_lvname(detached_log_lv));
			return 0;
		}
		if (!attach_mirror_log(mirrored_seg, detached_log_lv))
			return_0;
	}

	if (removed)
		*removed = old_area_count - new_area_count;

	log_very_verbose(FMTu32 " image(s) removed from %s.",
			 old_area_count - new_area_count,
			 display_lvname(lv));

	return 1;
}

/*
 * Remove the number of mirror images from the LV
 */
int remove_mirror_images(struct logical_volume *lv, uint32_t num_mirrors,
			 int (*is_removable)(struct logical_volume *, void *),
			 void *removable_baton, unsigned remove_log)
{
	uint32_t num_removed, removed_once, r;
	uint32_t existing_mirrors = lv_mirror_count(lv);
	struct logical_volume *next_lv = lv;

	int preferred_only = 1;
	int retries = 0;

	num_removed = existing_mirrors - num_mirrors;

	/* num_removed can be 0 if the function is called just to remove log */
	do {
		if (num_removed < first_seg(next_lv)->area_count)
			removed_once = num_removed;
		else
			removed_once = first_seg(next_lv)->area_count - 1;

		if (!_remove_mirror_images(next_lv, removed_once,
					   is_removable, removable_baton,
					   remove_log, 0, &r, preferred_only))
			return_0;

		if (r < removed_once || !removed_once) {
			/* Some mirrors are removed from the temporary mirror,
			 * but the temporary layer still exists.
			 * Down the stack and retry for remainder. */
			next_lv = find_temporary_mirror(next_lv);
			if (!next_lv) {
				preferred_only = 0;
				next_lv = lv;
			}
		}

		num_removed -= r;

		/*
		 * if there are still images to be removed, try again; this is
		 * required since some temporary layers may have been reduced
		 * to 1, at which point they are made removable, just like
		 * normal images
		 */
		if (!next_lv && !preferred_only && !retries && num_removed) {
			++retries;
			preferred_only = 1;
		}

	} while (next_lv && num_removed);

	if (num_removed) {
		if (num_removed == existing_mirrors - num_mirrors)
			log_error("No mirror images found using specified PVs.");
		else {
			log_error("%u images are removed out of requested %u.",
				  existing_mirrors - lv_mirror_count(lv),
				  existing_mirrors - num_mirrors);
		}
		return 0;
	}

	return 1;
}

static int _no_removable_images(struct logical_volume *lv __attribute__((unused)),
				void *baton __attribute__((unused))) {
	return 0;
}

/*
 * Collapsing temporary mirror layers.
 *
 * When mirrors are added to already-mirrored LV, a temporary mirror layer
 * is inserted at the top of the stack to reduce resync work.
 * The function will remove the intermediate layer and collapse the stack
 * as far as mirrors are in-sync.
 *
 * The function is destructive: to remove intermediate mirror layers,
 * VG metadata commits and suspend/resume are necessary.
 */
int collapse_mirrored_lv(struct logical_volume *lv)
{
	struct logical_volume *tmp_lv;
	struct lv_segment *mirror_seg;

	while ((tmp_lv = find_temporary_mirror(lv))) {
		mirror_seg = find_mirror_seg(first_seg(tmp_lv));
		if (!mirror_seg) {
			log_error("Failed to find mirrored LV for %s.",
				  display_lvname(tmp_lv));
			return 0;
		}

		if (!_mirrored_lv_in_sync(mirror_seg->lv)) {
			log_verbose("Not collapsing %s: out-of-sync.",
				    display_lvname(mirror_seg->lv));
			return 1;
		}

		if (!_remove_mirror_images(mirror_seg->lv,
					   mirror_seg->area_count - 1,
					   _no_removable_images, NULL, 0, 1, NULL, 0)) {
			log_error("Failed to release mirror images");
			return 0;
		}
	}

	return 1;
}

#if 0
/* FIXME: reconfigure_mirror_images: remove this code? */
static int _get_mirror_fault_policy(struct cmd_context *cmd __attribute__((unused)),
				   int log_policy)
{
	const char *policy = NULL;
/*
	if (log_policy)
		policy = find_config_tree_str(cmd, activation_mirror_log_fault_policy_CFG);
	else {
		policy = find_config_tree_str(cmd, activation_mirror_image_fault_policy_CFG);
		if (!policy)
			policy = find_config_tree_str(cmd, activation_mirror_device_fault_policy_CFG);
	}
*/
	if (!strcmp(policy, "remove"))
		return MIRROR_REMOVE;
	else if (!strcmp(policy, "allocate"))
		return MIRROR_ALLOCATE;
	else if (!strcmp(policy, "allocate_anywhere"))
		return MIRROR_ALLOCATE_ANYWHERE;

	if (log_policy)
		log_error("Bad activation/mirror_log_fault_policy");
	else
		log_error("Bad activation/mirror_device_fault_policy");

	return MIRROR_REMOVE;
}

static int _get_mirror_log_fault_policy(struct cmd_context *cmd)
{
	return _get_mirror_fault_policy(cmd, 1);
}

static int _get_mirror_device_fault_policy(struct cmd_context *cmd)
{
	return _get_mirror_fault_policy(cmd, 0);
}

/*
 * replace_mirror_images
 * @mirrored_seg: segment (which may be linear now) to restore
 * @num_mirrors: number of copies we should end up with
 * @replace_log: replace log if not present
 * @in_sync: was the original mirror in-sync?
 *
 * in_sync will be set to 0 if new mirror devices are being added
 * In other words, it is only useful if the log (and only the log)
 * is being restored.
 *
 * Returns: 0 on failure, 1 on reconfig, -1 if no reconfig done
 */
static int _replace_mirror_images(struct lv_segment *mirrored_seg,
				 uint32_t num_mirrors,
				 int log_policy, int in_sync)
{
	int r = -1;
	struct logical_volume *lv = mirrored_seg->lv;

	/* FIXME: Use lvconvert rather than duplicating its code */

	if (mirrored_seg->area_count < num_mirrors) {
		log_warn("WARNING: Failed to replace mirror device in %s.",
			 display_lvname(mirrored_seg->lv);

		if ((mirrored_seg->area_count > 1) && !mirrored_seg->log_lv)
			log_warn("WARNING: Use 'lvconvert -m %d %s --corelog' to replace failed devices.",
				 num_mirrors - 1, display_lvname(lv));
		else
			log_warn("WARNING: Use 'lvconvert -m %d %s' to replace failed devices.",
				 num_mirrors - 1, display_lvname(lv));
		r = 0;

		/* REMEMBER/FIXME: set in_sync to 0 if a new mirror device was added */
		in_sync = 0;
	}

	/*
	 * FIXME: right now, we ignore the allocation policy specified to
	 * allocate the new log.
	 */
	if ((mirrored_seg->area_count > 1) && !mirrored_seg->log_lv &&
	    (log_policy != MIRROR_REMOVE)) {
		log_warn("WARNING: Failed to replace mirror log device in %s.",
			 display_lvname(lv));

		log_warn("WARNING: Use 'lvconvert -m %d %s' to replace failed devices.",
			 mirrored_seg->area_count - 1 , display_lvname(lv));
		r = 0;
	}

	return r;
}

int reconfigure_mirror_images(struct lv_segment *mirrored_seg, uint32_t num_mirrors,
			      struct dm_list *removable_pvs, unsigned remove_log)
{
	int r;
	int in_sync;
	int log_policy, dev_policy;
	uint32_t old_num_mirrors = mirrored_seg->area_count;
	int had_log = (mirrored_seg->log_lv) ? 1 : 0;

	/* was the mirror in-sync before problems? */
	in_sync = _mirrored_lv_in_sync(mirrored_seg->lv);

	/*
	 * While we are only removing devices, we can have sync set.
	 * Setting this is only useful if we are moving to core log
	 * otherwise the disk log will contain the sync information
	 */
	init_mirror_in_sync(in_sync);

	r = _remove_mirror_images(mirrored_seg->lv, old_num_mirrors - num_mirrors,
				  is_mirror_image_removable, removable_pvs,
				  remove_log, 0, NULL, 0);
	if (!r)
		/* Unable to remove bad devices */
		return 0;

	log_warn("WARNING: Bad device removed from mirror volume %s.",
		  display_lvname(mirrored_seg->lv));

	log_policy = _get_mirror_log_fault_policy(mirrored_seg->lv->vg->cmd);
	dev_policy = _get_mirror_device_fault_policy(mirrored_seg->lv->vg->cmd);

	r = _replace_mirror_images(mirrored_seg,
				  (dev_policy != MIRROR_REMOVE) ?
				  old_num_mirrors : num_mirrors,
				  log_policy, in_sync);

	if (!r)
		/* Failed to replace device(s) */
		log_warn("WARNING: Unable to find substitute device for mirror volume %s.",
			 display_lvname(mirrored_seg->lv));
	else if (r > 0)
		/* Success in replacing device(s) */
		log_warn("WARNING: Mirror volume %s restored - substitute for failed device found.",
			 display_lvname(mirrored_seg->lv));
	else
		/* Bad device removed, but not replaced because of policy */
		if (mirrored_seg->area_count == 1) {
			log_warn("WARNING: Mirror volume %s converted to linear due to device failure.",
				  display_lvname(mirrored_seg->lv);
		} else if (had_log && !mirrored_seg->log_lv) {
			log_warn("WARNING: Mirror volume %s disk log removed due to device failure.",
				  display_lvname(mirrored_seg->lv));
		}
	/*
	 * If we made it here, we at least removed the bad device.
	 * Consider this success.
	 */
	return 1;
}
#endif

static int _create_mimage_lvs(struct alloc_handle *ah,
			      uint32_t num_mirrors,
			      uint32_t stripes,
			      uint32_t stripe_size,
			      struct logical_volume *lv,
			      struct logical_volume **img_lvs,
			      int log)
{
	uint32_t m, first_area;
	char img_name[NAME_LEN];

	if (dm_snprintf(img_name, sizeof(img_name), "%s_mimage_%%d", lv->name) < 0) {
		log_error("Failed to build new mirror image name for %s.",
			  display_lvname(lv));
		return 0;
	}

	for (m = 0; m < num_mirrors; m++) {
		if (!(img_lvs[m] = lv_create_empty(img_name,
						   NULL, LVM_READ | LVM_WRITE,
						   ALLOC_INHERIT, lv->vg))) {
			log_error("Aborting. Failed to create mirror image LV. "
				  "Remove new LV and retry.");
			return 0;
		}

		if (log) {
			first_area = m * stripes + (log - 1);

			if (!lv_add_log_segment(ah, first_area, img_lvs[m], 0)) {
				log_error("Failed to add mirror image segment"
					  " to %s. Remove new LV and retry.",
					  display_lvname(img_lvs[m]));
				return 0;
			}
		} else {
			if (!lv_add_segment(ah, m * stripes, stripes, img_lvs[m],
					    get_segtype_from_string(lv->vg->cmd, SEG_TYPE_NAME_STRIPED),
					    stripe_size, 0, 0)) {
				log_error("Aborting. Failed to add mirror image segment "
					  "to %s. Remove new LV and retry.",
					  display_lvname(img_lvs[m]));
				return 0;
			}
		}
	}

	return 1;
}

/*
 * Remove mirrors from each segment.
 * 'new_mirrors' is the number of mirrors after the removal. '0' for linear.
 * If 'status_mask' is non-zero, the removal happens only when all segments
 * has the status bits on.
 */
int remove_mirrors_from_segments(struct logical_volume *lv,
				 uint32_t new_mirrors, uint64_t status_mask)
{
	struct lv_segment *seg;
	uint32_t s;

	/* Check the segment params are compatible */
	dm_list_iterate_items(seg, &lv->segments) {
		if (!seg_is_mirrored(seg)) {
			log_error("Segment is not mirrored: %s:" FMTu32,
				  display_lvname(lv), seg->le);
			return 0;
		}
		if ((seg->status & status_mask) != status_mask) {
			log_error("Segment status does not match: %s:" FMTu32
				  " status:0x" FMTx64 "/0x" FMTx64,
				  display_lvname(lv), seg->le,
				  seg->status, status_mask);
			return 0;
		}
	}

	/* Convert the segments */
	dm_list_iterate_items(seg, &lv->segments) {
		if (!new_mirrors && seg->extents_copied == seg->area_len) {
			if (!move_lv_segment_area(seg, 0, seg, 1))
				return_0;
		}

		for (s = new_mirrors + 1; s < seg->area_count; s++)
			if (!release_and_discard_lv_segment_area(seg, s, seg->area_len))
				return_0;

		seg->area_count = new_mirrors + 1;

		if (!new_mirrors)
			seg->segtype = get_segtype_from_string(lv->vg->cmd, SEG_TYPE_NAME_STRIPED);
	}

	return 1;
}

const char *get_pvmove_pvname_from_lv_mirr(const struct logical_volume *lv_mirr)
{
	struct lv_segment *seg;

	dm_list_iterate_items(seg, &lv_mirr->segments) {
		if (!seg_is_mirrored(seg))
			continue;
		if (seg_type(seg, 0) == AREA_PV)
			return dev_name(seg_dev(seg, 0));
		if (seg_type(seg, 0) == AREA_LV)
			return dev_name(seg_dev(first_seg(seg_lv(seg, 0)), 0));
	}

	return NULL;
}

/*
 * Find first pvmove LV referenced by a segment of an LV.
 */
const struct logical_volume *find_pvmove_lv_in_lv(const struct logical_volume *lv)
{
	const struct lv_segment *seg;
	uint32_t s;

	if (lv_is_pvmove(lv))
		return lv;

	dm_list_iterate_items(seg, &lv->segments) {
		for (s = 0; s < seg->area_count; s++) {
			if (seg_type(seg, s) != AREA_LV)
				continue;
			if (lv_is_pvmove(seg_lv(seg, s)))
				return seg_lv(seg, s);
		}
	}

	return NULL;
}

const char *get_pvmove_pvname_from_lv(const struct logical_volume *lv)
{
	const struct logical_volume *pvmove_lv;

	pvmove_lv = find_pvmove_lv_in_lv(lv);

	if (pvmove_lv)
		return get_pvmove_pvname_from_lv_mirr(pvmove_lv);

	return NULL;
}

struct logical_volume *find_pvmove_lv(struct volume_group *vg,
				      struct device *dev,
				      uint64_t lv_type)
{
	struct lv_list *lvl;
	struct logical_volume *lv;
	struct lv_segment *seg;

	/* Loop through all LVs */
	dm_list_iterate_items(lvl, &vg->lvs) {
		lv = lvl->lv;

		if (!(lv->status & lv_type))
			continue;

		/*
		 * If this is an atomic pvmove, the first
		 * segment will be a mirror containing
		 * mimages (i.e. AREA_LVs)
		 */
		if (seg_type(first_seg(lv), 0) == AREA_LV) {
			seg = first_seg(lv);            /* the mirror segment */
			seg = first_seg(seg_lv(seg, 0)); /* mimage_0 segment0 */
			if (seg_dev(seg, 0) != dev)
				continue;
			return lv;
		}

		/*
		 * If this is a normal pvmove, check all the segments'
		 * first areas for the requested device
		 */
		dm_list_iterate_items(seg, &lv->segments) {
			if (seg_type(seg, 0) != AREA_PV)
				continue;
			if (seg_dev(seg, 0) != dev)
				continue;

			return lv;
		}
	}

	return NULL;
}

struct dm_list *lvs_using_lv(struct cmd_context *cmd, struct volume_group *vg,
			  struct logical_volume *lv)
{
	struct dm_list *lvs;
	struct logical_volume *lv1;
	struct lv_list *lvl, *lvl1;
	struct lv_segment *seg;
	uint32_t s;

	if (!(lvs = dm_pool_alloc(cmd->mem, sizeof(*lvs)))) {
		log_error("lvs list alloc failed.");
		return NULL;
	}

	dm_list_init(lvs);

	/* Loop through all LVs except the one supplied */
	dm_list_iterate_items(lvl1, &vg->lvs) {
		lv1 = lvl1->lv;
		if (lv1 == lv)
			continue;

		/* Find whether any segment points at the supplied LV */
		dm_list_iterate_items(seg, &lv1->segments) {
			for (s = 0; s < seg->area_count; s++) {
				if (seg_type(seg, s) != AREA_LV ||
				    seg_lv(seg, s) != lv)
					continue;
				if (!(lvl = dm_pool_alloc(cmd->mem, sizeof(*lvl)))) {
					log_error("lv_list alloc failed.");
					return NULL;
				}
				lvl->lv = lv1;
				dm_list_add(lvs, &lvl->list);
				goto next_lv;
			}
		}
	      next_lv:
		;
	}

	return lvs;
}

/*
 * Fixup mirror pointers after single-pass segment import
 */
int fixup_imported_mirrors(struct volume_group *vg)
{
	struct lv_list *lvl;
	struct lv_segment *seg;

	dm_list_iterate_items(lvl, &vg->lvs) {
		dm_list_iterate_items(seg, &lvl->lv->segments) {
			if (seg->segtype !=
			    get_segtype_from_string(vg->cmd, SEG_TYPE_NAME_MIRROR))
				continue;

			if (seg->log_lv && !add_seg_to_segs_using_this_lv(seg->log_lv, seg))
				return_0;
		}
	}

	return 1;
}

static int _add_mirrors_that_preserve_segments(struct logical_volume *lv,
					       uint32_t flags,
					       uint32_t mirrors,
					       uint32_t region_size,
					       struct dm_list *allocatable_pvs,
					       alloc_policy_t alloc)
{
	struct cmd_context *cmd = lv->vg->cmd;
	struct alloc_handle *ah;
	const struct segment_type *segtype;
	struct dm_list *parallel_areas;
	uint32_t adjusted_region_size;
	int r = 1;

	if (!(parallel_areas = build_parallel_areas_from_lv(lv, 1, 0)))
		return_0;

	if (!(segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_MIRROR)))
		return_0;

	if (!(adjusted_region_size = adjusted_mirror_region_size(cmd,
								lv->vg->extent_size,
								lv->le_count,
								region_size, 1,
								vg_is_clustered(lv->vg))))
		return_0;

	if (!(ah = allocate_extents(lv->vg, NULL, segtype, 1, mirrors, 0, 0,
				    lv->le_count, allocatable_pvs, alloc, 0,
				    parallel_areas))) {
		log_error("Unable to allocate mirror extents for %s.",
			  display_lvname(lv));
		return 0;
	}

	if (flags & MIRROR_BY_SEG) {
		if (!lv_add_mirror_areas(ah, lv, 0, adjusted_region_size)) {
			log_error("Failed to add mirror areas to %s.",
				  display_lvname(lv));
			r = 0;
		}
	} else if (flags & MIRROR_BY_SEGMENTED_LV) {
		if (!lv_add_segmented_mirror_image(ah, lv, 0,
						   adjusted_region_size)) {
			log_error("Failed to add mirror areas to %s.",
				  display_lvname(lv));
			r = 0;
		}
	} else {
		log_error(INTERNAL_ERROR "Unknown mirror flag.");
		r = 0;
	}
	alloc_destroy(ah);
	return r;
}

/*
 * Add mirrors to "linear" or "mirror" segments
 */
int add_mirrors_to_segments(struct cmd_context *cmd, struct logical_volume *lv,
			    uint32_t mirrors, uint32_t region_size,
			    struct dm_list *allocatable_pvs, alloc_policy_t alloc)
{
	return _add_mirrors_that_preserve_segments(lv, MIRROR_BY_SEG,
						   mirrors, region_size,
						   allocatable_pvs, alloc);
}

/*
 * Convert mirror log
 *
 * FIXME: Can't handle segment-by-segment mirror (like pvmove)
 */
int remove_mirror_log(struct cmd_context *cmd,
		      struct logical_volume *lv,
		      struct dm_list *removable_pvs,
		      int force)
{
	dm_percent_t sync_percent;

	/* Unimplemented features */
	if (dm_list_size(&lv->segments) != 1) {
		log_error("Multiple-segment mirror is not supported.");
		return 0;
	}

	/* Had disk log, switch to core. */
	if (lv_is_active(lv)) {
		if (!lv_mirror_percent(cmd, lv, 0, &sync_percent,
				       NULL)) {
			log_error("Unable to determine mirror sync status.");
			return 0;
		}
	} else if (force || yes_no_prompt("Full resync required to convert inactive "
					  "mirror volume %s to core log. "
					  "Proceed? [y/n]: ", display_lvname(lv)) == 'y')
		sync_percent = 0;
	else {
		log_error("Logical volume %s NOT converted.", display_lvname(lv));
		return 0;
	}

	if (sync_percent == DM_PERCENT_100)
		init_mirror_in_sync(1);
	else {
		/* A full resync will take place */
		lv->status &= ~LV_NOTSYNCED;
		init_mirror_in_sync(0);
	}

	if (!remove_mirror_images(lv, lv_mirror_count(lv),
				  is_mirror_image_removable, removable_pvs, 1U))
		return_0;

	return 1;
}

static struct logical_volume *_create_mirror_log(struct logical_volume *lv,
						 struct alloc_handle *ah,
						 alloc_policy_t alloc,
						 const char *lv_name,
						 const char *suffix)
{
	struct logical_volume *log_lv;
	char log_name[NAME_LEN];

	if (dm_snprintf(log_name, sizeof(log_name), "%s%s", lv_name, suffix) < 0) {
		log_error("Failed to build new mirror log name for %s.", lv_name);
		return NULL;
	}

	if (!(log_lv = lv_create_empty(log_name, NULL,
				       VISIBLE_LV | LVM_READ | LVM_WRITE,
				       alloc, lv->vg)))
		return_NULL;

	if (!lv_add_log_segment(ah, 0, log_lv, MIRROR_LOG))
		return_NULL;

	return log_lv;
}

/*
 * Returns: 1 on success, 0 on error
 */
static int _form_mirror(struct cmd_context *cmd, struct alloc_handle *ah,
			struct logical_volume *lv,
			uint32_t mirrors, uint32_t stripes,
			uint32_t stripe_size, uint32_t region_size, int log)
{
	struct logical_volume **img_lvs;

	/*
	 * insert a mirror layer
	 */
	if (dm_list_size(&lv->segments) != 1 ||
	    seg_type(first_seg(lv), 0) != AREA_LV)
		if (!insert_layer_for_lv(cmd, lv, 0, "_mimage_%d"))
			return_0;

	/*
	 * create mirror image LVs
	 */
	img_lvs = alloca(sizeof(*img_lvs) * mirrors);

	if (!_create_mimage_lvs(ah, mirrors, stripes, stripe_size, lv, img_lvs, log))
		return_0;

	if (!lv_add_mirror_lvs(lv, img_lvs, mirrors,
			       /* Pass through MIRRORED & LOCKED status flag
				* TODO: Any other would be needed ?? */
			       MIRROR_IMAGE | (lv->status & (MIRRORED | LOCKED)),
			       region_size)) {
		log_error("Aborting. Failed to add mirror segment. "
			  "Remove new LV and retry.");
		return 0;
	}

	return 1;
}

static struct logical_volume *_set_up_mirror_log(struct cmd_context *cmd,
						 struct alloc_handle *ah,
						 struct logical_volume *lv,
						 uint32_t log_count,
						 uint32_t region_size,
						 alloc_policy_t alloc,
						 int in_sync)
{
	struct logical_volume *log_lv;
	const char *suffix, *lv_name;
	char *tmp_name;
	size_t len;
	struct lv_segment *seg;

	init_mirror_in_sync(in_sync);

	/* Mirror log name is lv_name + suffix, determined as the following:
	 *   1. suffix is:
	 *        o "_mlog" for the original mirror LV.
	 *        o "_mlogtmp_%d" for temporary mirror LV,
	 *   2. lv_name is:
	 *        o lv->name, if the log is temporary
	 *        o otherwise, the top-level LV name
	 */
	seg = first_seg(lv);
	if (seg_type(seg, 0) == AREA_LV &&
	    strstr(seg_lv(seg, 0)->name, MIRROR_SYNC_LAYER)) {
		lv_name = lv->name;
		suffix = "_mlogtmp_%d";
	} else if ((lv_name = strstr(lv->name, MIRROR_SYNC_LAYER))) {
		len = lv_name - lv->name;
		tmp_name = alloca(len + 1);
		tmp_name[len] = '\0';
		lv_name = strncpy(tmp_name, lv->name, len);
		suffix = "_mlog";
	} else {
		lv_name = lv->name;
		suffix = "_mlog";
	}

	if (!(log_lv = _create_mirror_log(lv, ah, alloc, lv_name, suffix))) {
		log_error("Failed to create mirror log.");
		return NULL;
	}

	if (log_count > 1) {
		/* Kernel requires a mirror to be at least 1 region large. */
		if (region_size > log_lv->size) {
			region_size = UINT64_C(1) << (31 - clz(log_lv->size));
			log_debug("Adjusting region_size to %s for mirrored log.",
				  display_size(cmd, (uint64_t)region_size));
		}

		if (!_form_mirror(cmd, ah, log_lv, log_count-1, 1, 0, region_size, 2)) {
			log_error("Failed to form mirrored log.");
			return NULL;
		}
	}

	if (!_init_mirror_log(cmd, NULL, log_lv, in_sync, &lv->tags, 1)) {
		log_error("Failed to initialise mirror log.");
		return NULL;
	}

	return log_lv;
}

int attach_mirror_log(struct lv_segment *seg, struct logical_volume *log_lv)
{
	seg->log_lv = log_lv;
	log_lv->status |= MIRROR_LOG;
	lv_set_hidden(log_lv);
	return add_seg_to_segs_using_this_lv(log_lv, seg);
}

/* Prepare disk mirror log for raid1->mirror conversion */
struct logical_volume *prepare_mirror_log(struct logical_volume *lv,
					  int in_sync, uint32_t region_size,
					  struct dm_list *allocatable_pvs,
					  alloc_policy_t alloc)
{
	struct cmd_context *cmd = lv->vg->cmd;
	const struct segment_type *segtype;
	struct dm_list *parallel_areas;
	struct alloc_handle *ah;
	struct logical_volume *log_lv;

	if (!(parallel_areas = build_parallel_areas_from_lv(lv, 0, 0)))
		return_NULL;

	if (!(segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_MIRROR)))
		return_NULL;

	/* Allocate destination extents */
	if (!(ah = allocate_extents(lv->vg, NULL, segtype,
				    0, 0, 1, region_size,
				    lv->le_count, allocatable_pvs,
				    alloc, 0, parallel_areas))) {
		log_error("Unable to allocate extents for mirror log.");
		return NULL;
	}

	if (!(log_lv = _create_mirror_log(lv, ah, alloc, lv->name, "_mlog"))) {
		log_error("Failed to create mirror log.");
		goto out;
	}

	if (!_init_mirror_log(cmd, NULL, log_lv, in_sync, &lv->tags, 1)) {
		log_error("Failed to initialise mirror log.");
		log_lv = NULL;
		goto out;
	}
out:
	alloc_destroy(ah);

	return log_lv;
}

int add_mirror_log(struct cmd_context *cmd, struct logical_volume *lv,
		   uint32_t log_count, uint32_t region_size,
		   struct dm_list *allocatable_pvs, alloc_policy_t alloc)
{
	struct alloc_handle *ah;
	const struct segment_type *segtype;
	struct dm_list *parallel_areas;
	dm_percent_t sync_percent;
	int in_sync;
	struct logical_volume *log_lv;
	unsigned old_log_count;
	int r = 0;

	if (dm_list_size(&lv->segments) != 1) {
		log_error("Multiple-segment mirror is not supported.");
		return 0;
	}

	log_lv = first_seg(lv)->log_lv;
	old_log_count = (log_lv) ? lv_mirror_count(log_lv) : 0;
	if (old_log_count == log_count) {
		log_verbose("Mirror %s already has a %s log.", display_lvname(lv),
			    !log_count ? "core" :
			    (log_count == 1) ? "disk" : "mirrored");
		return 1;
	}

	if (log_count > 1) {
		log_err("Log type \"mirrored\" is DEPRECATED. Use RAID1 LV or disk log instead.");
		return 0;
	}

	if (!(parallel_areas = build_parallel_areas_from_lv(lv, 0, 0)))
		return_0;

	if (!(segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_MIRROR)))
		return_0;

	if (activation() && segtype->ops->target_present &&
	    !segtype->ops->target_present(cmd, NULL, NULL)) {
		log_error("%s: Required device-mapper target(s) not "
			  "detected in your kernel.", segtype->name);
		return 0;
	}

	/* allocate destination extents */
	if (!(ah = allocate_extents(lv->vg, NULL, segtype,
				    0, 0, log_count - old_log_count, region_size,
				    lv->le_count, allocatable_pvs,
				    alloc, 0, parallel_areas))) {
		log_error("Unable to allocate extents for mirror log.");
		return 0;
	}

	if (old_log_count) {
		/* Converting from disk to mirrored log */
		if (!_form_mirror(cmd, ah, log_lv, log_count - 1, 1, 0,
				  region_size, 1)) {
			log_error("Failed to convert mirror log");
			return 0;
		}
		r = 1;
		goto out;
	}

	/* check sync status */
	if (mirror_in_sync() ||
	    (lv_mirror_percent(cmd, lv, 0, &sync_percent, NULL) &&
	     (sync_percent == DM_PERCENT_100)))
		in_sync = 1;
	else
		in_sync = 0;

	if (!(log_lv = _set_up_mirror_log(cmd, ah, lv, log_count,
					  region_size, alloc, in_sync)))
		goto_out;

	if (!attach_mirror_log(first_seg(lv), log_lv))
		goto_out;

	r = 1;
out:
	alloc_destroy(ah);
	return r;
}

/*
 * Convert "linear" LV to "mirror".
 */
int add_mirror_images(struct cmd_context *cmd, struct logical_volume *lv,
		      uint32_t mirrors, uint32_t stripes,
		      uint32_t stripe_size, uint32_t region_size,
		      struct dm_list *allocatable_pvs, alloc_policy_t alloc,
		      uint32_t log_count)
{
	struct alloc_handle *ah;
	const struct segment_type *segtype;
	struct dm_list *parallel_areas;
	struct logical_volume *log_lv = NULL;

	/*
	 * allocate destination extents
	 */

	if (!(parallel_areas = build_parallel_areas_from_lv(lv, 0, 0)))
		return_0;

	if (!(segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_MIRROR)))
		return_0;

	if (!(ah = allocate_extents(lv->vg, NULL, segtype,
				    stripes, mirrors, log_count, region_size, lv->le_count,
				    allocatable_pvs, alloc, 0, parallel_areas))) {
		log_error("Unable to allocate extents for mirror(s).");
		return 0;
	}

	/*
	 * create and initialize mirror log
	 */
	if (log_count &&
	    !(log_lv = _set_up_mirror_log(cmd, ah, lv, log_count,
					  (region_size > lv->vg->extent_size) ?
					  lv->vg->extent_size : region_size,
					  alloc, mirror_in_sync()))) {
		stack;
		goto out_remove_images;
	}

	/* The log initialization involves vg metadata commit.
	   So from here on, if failure occurs, the log must be explicitly
	   removed and the updated vg metadata should be committed. */

	if (!_form_mirror(cmd, ah, lv, mirrors, stripes, stripe_size, region_size, 0))
		goto out_remove_log;

	if (log_count && !attach_mirror_log(first_seg(lv), log_lv))
		stack;

	alloc_destroy(ah);
	return 1;

  out_remove_log:
	if (log_lv) {
		if (!lv_remove(log_lv) ||
		    !vg_write(log_lv->vg) ||
		    !vg_commit(log_lv->vg))
			log_error("Manual intervention may be required to remove "
				  "abandoned log LV before retrying.");
		else
			backup(log_lv->vg);
	}
  out_remove_images:
	alloc_destroy(ah);
	return 0;
}

/*
 * Generic interface for adding mirror and/or mirror log.
 * 'mirror' is the number of mirrors to be added.
 * 'pvs' is either allocatable pvs.
 */
int lv_add_mirrors(struct cmd_context *cmd, struct logical_volume *lv,
		   uint32_t mirrors, uint32_t stripes, uint32_t stripe_size,
		   uint32_t region_size, uint32_t log_count,
		   struct dm_list *pvs, alloc_policy_t alloc, uint32_t flags)
{
	if (!mirrors && !log_count) {
		log_error("No conversion is requested.");
		return 0;
	}

	if (lv->vg->lock_type && !strcmp(lv->vg->lock_type, "dlm") && cmd->lockd_lv_sh) {
		if (!cluster_mirror_is_available(cmd)) {
			log_error("Shared cluster mirrors are not available.");
			return 0;
		}

		if (log_count > 1) {
			log_error("Log type, \"mirrored\", is unavailable to cluster mirrors.");
			return 0;
		}
	}

	/* For corelog mirror, activation code depends on
	 * the global mirror_in_sync status. As we are adding
	 * a new mirror, it should be set as 'out-of-sync'
	 * so that the sync starts. */
	/* However, MIRROR_SKIP_INIT_SYNC even overrides it. */
	if (flags & MIRROR_SKIP_INIT_SYNC)
		init_mirror_in_sync(1);
	else if (!log_count)
		init_mirror_in_sync(0);

	if (flags & MIRROR_BY_SEG) {
		if (log_count) {
			log_error("Persistent log is not supported on "
				  "segment-by-segment mirroring.");
			return 0;
		}
		if (stripes > 1) {
			log_error("Striped-mirroring is not supported on "
				  "segment-by-segment mirroring.");
			return 0;
		}

		return _add_mirrors_that_preserve_segments(lv, MIRROR_BY_SEG,
							   mirrors, region_size,
							   pvs, alloc);
	}

	if (flags & MIRROR_BY_SEGMENTED_LV) {
		if (stripes > 1) {
			log_error("Striped-mirroring is not supported on "
				  "segment-by-segment mirroring.");
			return 0;
		}

		return _add_mirrors_that_preserve_segments(lv, MIRROR_BY_SEGMENTED_LV,
							   mirrors, region_size,
							   pvs, alloc);
	}

	if (flags & MIRROR_BY_LV) {
		if (!mirrors)
			return add_mirror_log(cmd, lv, log_count,
					      region_size, pvs, alloc);
		return add_mirror_images(cmd, lv, mirrors,
					 stripes, stripe_size, region_size,
					 pvs, alloc, log_count);
	}

	log_error("Unsupported mirror conversion type.");

	return 0;
}

int lv_split_mirror_images(struct logical_volume *lv, const char *split_name,
			   uint32_t split_count, struct dm_list *removable_pvs)
{
	int historical;

	if (lv_name_is_used_in_vg(lv->vg, split_name, &historical)) {
		log_error("%sLogical Volume \"%s\" already exists in "
			  "volume group \"%s\".", historical ? "historical " : "",
			  split_name, lv->vg->name);
		return 0;
	}

	/* Can't split a mirror that is not in-sync... unless force? */
	if (!_mirrored_lv_in_sync(lv)) {
		log_error("Unable to split mirror %s that is not in-sync.",
			  display_lvname(lv));
		return 0;
	}

	/*
	 * FIXME: Generate default name when not supplied.
	 *
	 * If we were going to generate a default name, we would
	 * do it here.  Better to wait for a decision on the form
	 * of the default name when '--track_deltas' (the ability
	 * to merge a split leg back in and only copy the changes)
	 * is being implemented.  For now, we force the user to
	 * come up with a name for their LV.
	 */
	if (!_split_mirror_images(lv, split_name, split_count, removable_pvs))
		return_0;

	return 1;
}

/*
 * Generic interface for removing mirror and/or mirror log.
 * 'mirror' is the number of mirrors to be removed.
 * 'pvs' is removable pvs.
 */
int lv_remove_mirrors(struct cmd_context *cmd __attribute__((unused)),
		      struct logical_volume *lv,
		      uint32_t mirrors, uint32_t log_count,
		      int (*is_removable)(struct logical_volume *, void *),
		      void *removable_baton,
		      uint64_t status_mask)
{
	uint32_t new_mirrors;
	struct lv_segment *seg;

	if (!mirrors && !log_count) {
		log_error("No conversion is requested.");
		return 0;
	}

	seg = first_seg(lv);
	if (!seg_is_mirrored(seg)) {
		log_error("Not a mirror segment.");
		return 0;
	}

	if (lv_mirror_count(lv) <= mirrors) {
		log_error("Removing more than existing: %d <= %d.",
			  seg->area_count, mirrors);
		return 0;
	}
	new_mirrors = lv_mirror_count(lv) - mirrors - 1;

	/* MIRROR_BY_LV */
	if (seg_type(seg, 0) == AREA_LV &&
	    lv_is_mirror_image(seg_lv(seg, 0)))
		return remove_mirror_images(lv, new_mirrors + 1,
					    is_removable, removable_baton,
					    log_count ? 1U : 0);

	/* MIRROR_BY_SEG */
	if (log_count) {
		log_error("Persistent log is not supported on "
			  "segment-by-segment mirroring.");
		return 0;
	}
	return remove_mirrors_from_segments(lv, new_mirrors, status_mask);
}

int set_mirror_log_count(int *log_count, const char *mirrorlog)
{
	if (!strcmp("core", mirrorlog))
		*log_count = MIRROR_LOG_CORE;
	else if (!strcmp("disk", mirrorlog))
		*log_count = MIRROR_LOG_DISK;
	else if (!strcmp("mirrored", mirrorlog))
		*log_count = MIRROR_LOG_MIRRORED;
	else {
		log_error("Mirror log type \"%s\" is unknown.", mirrorlog);
		return 0;
	}

	return 1;
}

const char *get_mirror_log_name(int log_count)
{
	switch (log_count) {
	case MIRROR_LOG_CORE: return "core";
	case MIRROR_LOG_DISK: return "disk";
	case MIRROR_LOG_MIRRORED: return "mirrored";
	default:
		log_error(INTERNAL_ERROR "Unknown mirror log count %d.", log_count);
		return NULL;
	}
}
