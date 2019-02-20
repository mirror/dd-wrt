/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
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
#include "lib/metadata/segtype.h"
#include "lib/locking/locking.h"
#include "lib/commands/toolcontext.h"
#include "lib/metadata/lv_alloc.h"
#include "lib/activate/activate.h"

#define SNAPSHOT_MIN_CHUNKS	3       /* Minimum number of chunks in snapshot */

int lv_is_origin(const struct logical_volume *lv)
{
	return lv->origin_count ? 1 : 0;
}

int lv_is_cow(const struct logical_volume *lv)
{
	/* Make sure a merging thin origin isn't confused as a cow LV */
	return (!lv_is_thin_volume(lv) && !lv_is_origin(lv) && lv->snapshot) ? 1 : 0;
}

struct logical_volume *find_cow(const struct logical_volume *snap)
{
	return first_seg(snap)->cow;
}

/*
 * Some kernels have a bug that they may leak space in the snapshot on crash.
 * If the kernel is buggy, we add some extra space.
 */
static uint64_t _cow_extra_chunks(struct cmd_context *cmd, uint64_t n_chunks)
{
	const struct segment_type *segtype;
	unsigned attrs = 0;

	if (activation() &&
	    (segtype = get_segtype_from_string(cmd, SEG_TYPE_NAME_SNAPSHOT)) &&
	    segtype->ops->target_present &&
	    segtype->ops->target_present(cmd, NULL, &attrs) &&
	    (attrs & SNAPSHOT_FEATURE_FIXED_LEAK))
		return 0;

	return (n_chunks + 63) / 64;
}

static uint64_t _cow_max_size(struct cmd_context *cmd, uint64_t origin_size, uint32_t chunk_size)
{
	/* Snapshot disk layout:
	 *    COW is divided into chunks
	 *        1st. chunk is reserved for header
	 *        2nd. chunk is the 1st. metadata chunk
	 *        3rd. chunk is the 1st. data chunk
	 */

	uint64_t origin_chunks = (origin_size + chunk_size - 1) / chunk_size;
	uint64_t chunks_per_metadata_area = (uint64_t)chunk_size << (SECTOR_SHIFT - 4);

	/*
	 * Note: if origin_chunks is divisible by chunks_per_metadata_area, we
	 * need one extra metadata chunk as a terminator.
	 */
	uint64_t metadata_chunks = (origin_chunks + chunks_per_metadata_area) / chunks_per_metadata_area;
	uint64_t n_chunks = 1 + origin_chunks + metadata_chunks;

	return (n_chunks + _cow_extra_chunks(cmd, n_chunks)) * chunk_size;
}

uint32_t cow_max_extents(const struct logical_volume *origin, uint32_t chunk_size)
{
	uint64_t size = _cow_max_size(origin->vg->cmd, origin->size, chunk_size);
	uint32_t extent_size = origin->vg->extent_size;
	uint64_t max_size = (uint64_t) MAX_EXTENT_COUNT * extent_size;

	if (size % extent_size)
		size += extent_size - size % extent_size;

	if (size > max_size)
		size = max_size; /* Origin is too big for 100% snapshot anyway */

	return (uint32_t) (size / extent_size);
}

int cow_has_min_chunks(const struct volume_group *vg, uint32_t cow_extents, uint32_t chunk_size)
{
	if (((uint64_t)vg->extent_size * cow_extents) >= (SNAPSHOT_MIN_CHUNKS * chunk_size))
		return 1;

	log_error("Snapshot volume cannot be smaller than " DM_TO_STRING(SNAPSHOT_MIN_CHUNKS)
		  " chunks (%u extents, %s).", (unsigned)
		  (((uint64_t) SNAPSHOT_MIN_CHUNKS * chunk_size +
		    vg->extent_size - 1) / vg->extent_size),
		  display_size(vg->cmd, (uint64_t) SNAPSHOT_MIN_CHUNKS * chunk_size));

	return 0;
}

int lv_is_cow_covering_origin(const struct logical_volume *lv)
{
	return lv_is_cow(lv) &&
		(lv->size >= _cow_max_size(lv->vg->cmd, origin_from_cow(lv)->size,
					   find_snapshot(lv)->chunk_size));
}

int lv_is_visible(const struct logical_volume *lv)
{
	if (lv_is_historical(lv))
		return 1;

	if (lv_is_snapshot(lv))
		return 0;

	if (lv_is_cow(lv)) {
		if (lv_is_virtual_origin(origin_from_cow(lv)))
			return 1;

		if (lv_is_merging_cow(lv))
			return 0;

		return lv_is_visible(origin_from_cow(lv));
	}

	return lv->status & VISIBLE_LV ? 1 : 0;
}

int lv_is_merging_cow(const struct logical_volume *cow)
{
	struct lv_segment *snap_seg;

	if (!lv_is_cow(cow))
		return 0;

	snap_seg = find_snapshot(cow);

	/* checks lv_segment's status to see if snapshot is merging */
	return (snap_seg && (snap_seg->status & MERGING)) ? 1 : 0;
}

struct lv_segment *find_snapshot(const struct logical_volume *lv)
{
	return lv->snapshot;
}

/* Given a cow LV, return its origin */
struct logical_volume *origin_from_cow(const struct logical_volume *lv)
{
	if (lv->snapshot)
		return lv->snapshot->origin;
	return NULL;
}

void init_snapshot_seg(struct lv_segment *seg, struct logical_volume *origin,
		       struct logical_volume *cow, uint32_t chunk_size, int merge)
{
	seg->chunk_size = chunk_size;
	seg->origin = origin;
	seg->cow = cow;

	lv_set_hidden(cow);

	cow->snapshot = seg;

	origin->origin_count++;

	/* FIXME Assumes an invisible origin belongs to a sparse device */
	if (!lv_is_visible(origin))
		origin->status |= VIRTUAL_ORIGIN;

	seg->lv->status |= (SNAPSHOT | VIRTUAL);
	if (merge)
		init_snapshot_merge(seg, origin);

	dm_list_add(&origin->snapshot_segs, &seg->origin_list);
}

void init_snapshot_merge(struct lv_segment *snap_seg,
			 struct logical_volume *origin)
{
	snap_seg->status |= MERGING;
	origin->snapshot = snap_seg;
	origin->status |= MERGING;

	if (seg_is_thin_volume(snap_seg)) {
		snap_seg->merge_lv = origin;
		/* Making thin LV invisible with regular log */
		lv_set_hidden(snap_seg->lv);
		return;
	}

	/*
	 * Even though lv_is_visible(snap_seg->lv) returns 0,
	 * the snap_seg->lv (name: snapshotX) is _not_ hidden;
	 * this is part of the lvm2 snapshot fiction.  Must
	 * clear VISIBLE_LV directly (lv_set_visible can't)
	 * - snap_seg->lv->status is used to control whether 'lv'
	 *   (with user provided snapshot LV name) is visible
	 * - this also enables vg_validate() to succeed with
	 *   merge metadata (snap_seg->lv is now "internal")
	 */
	snap_seg->lv->status &= ~VISIBLE_LV;
}

void clear_snapshot_merge(struct logical_volume *origin)
{
	/* clear merge attributes */
	if (origin->snapshot->merge_lv)
		/* Removed thin volume has to be visible */
		lv_set_visible(origin->snapshot->lv);

	origin->snapshot->merge_lv = NULL;
	origin->snapshot->status &= ~MERGING;
	origin->snapshot = NULL;
	origin->status &= ~MERGING;
}

static struct lv_segment *_alloc_snapshot_seg(struct logical_volume *lv)
{
	struct lv_segment *seg;
	const struct segment_type *segtype;

	segtype = get_segtype_from_string(lv->vg->cmd, SEG_TYPE_NAME_SNAPSHOT);
	if (!segtype) {
		log_error("Failed to find snapshot segtype");
		return NULL;
	}

	if (!(seg = alloc_lv_segment(segtype, lv, 0, lv->le_count, 0, 0, 0,
				     NULL, 0, lv->le_count, 0, 0, 0, 0, NULL))) {
		log_error("Couldn't allocate new snapshot segment.");
		return NULL;
	}

	dm_list_add(&lv->segments, &seg->list);

	return seg;
}

int vg_add_snapshot(struct logical_volume *origin,
		    struct logical_volume *cow, union lvid *lvid,
		    uint32_t extent_count, uint32_t chunk_size)
{
	struct logical_volume *snap;
	struct lv_segment *seg;

	/*
	 * Is the cow device already being used ?
	 */
	if (lv_is_cow(cow)) {
		log_error("'%s' is already in use as a snapshot.", cow->name);
		return 0;
	}

	if (cow == origin) {
		log_error("Snapshot and origin LVs must differ.");
		return 0;
	}

	if (!(snap = lv_create_empty("snapshot%d",
				     lvid, LVM_READ | LVM_WRITE | VISIBLE_LV,
				     ALLOC_INHERIT, origin->vg)))
		return_0;

	snap->le_count = extent_count;

	if (!(seg = _alloc_snapshot_seg(snap)))
		return_0;

	init_snapshot_seg(seg, origin, cow, chunk_size, 0);

	return 1;
}

int vg_remove_snapshot(struct logical_volume *cow)
{
	int merging_snapshot = 0;
	struct logical_volume *origin = origin_from_cow(cow);
	int is_origin_active = lv_is_active(origin);

	if (is_origin_active &&
	    lv_is_virtual_origin(origin)) {
		if (!deactivate_lv(origin->vg->cmd, origin)) {
			log_error("Failed to deactivate logical volume \"%s\"",
				  origin->name);
			return 0;
		}
		is_origin_active = 0;
	}

	dm_list_del(&cow->snapshot->origin_list);
	origin->origin_count--;

	if (lv_is_merging_origin(origin) &&
	    (find_snapshot(origin) == find_snapshot(cow))) {
		clear_snapshot_merge(origin);
		/*
		 * preload origin IFF "snapshot-merge" target is active
		 * - IMPORTANT: avoids preload if inactivate merge is pending
		 */
		if (lv_has_target_type(origin->vg->vgmem, origin, NULL,
				       TARGET_NAME_SNAPSHOT_MERGE)) {
			/*
			 * preload origin to:
			 * - allow proper release of -cow
			 * - avoid allocations with other devices suspended
			 *   when transitioning from "snapshot-merge" to
			 *   "snapshot-origin after a merge completes.
			 */
			merging_snapshot = 1;
		}
	}

	if (!lv_remove(cow->snapshot->lv)) {
		log_error("Failed to remove internal snapshot LV %s",
			  cow->snapshot->lv->name);
		return 0;
	}

	cow->snapshot = NULL;
	lv_set_visible(cow);

	if (!vg_write(origin->vg))
		return_0;

	/* Skip call suspend, if device is not active */
	if (is_origin_active && !suspend_lv(origin->vg->cmd, origin)) {
		log_error("Failed to refresh %s without snapshot.",
			  origin->name);
		vg_revert(origin->vg);
		return 0;
	}
	if (!vg_commit(origin->vg))
		return_0;

	if (is_origin_active) {
		/*
		 * If the snapshot was active and the COW LV is taken away
		 * the LV lock on cluster has to be grabbed, so use
		 * activate_lv() which resumes suspend cow device.
		 */
		if (!merging_snapshot && !activate_lv(cow->vg->cmd, cow)) {
			log_error("Failed to activate %s.", cow->name);
			return 0;
		}

		if (!resume_lv(origin->vg->cmd, origin)) {
			log_error("Failed to resume %s.", origin->name);
			return 0;
		}

		/*
		 * For merged snapshot and clustered VG activate cow LV so
		 * the following call to deactivate_lv() can clean-up table
		 * entries. For this clustered lock need to be held.
		 */
		if (vg_is_clustered(cow->vg) &&
		    merging_snapshot && !activate_lv(cow->vg->cmd, cow)) {
			log_error("Failed to activate %s.", cow->name);
			return 0;
		}
	}

	return 1;
}

/* Check if given LV is usable as snapshot origin LV */
int validate_snapshot_origin(const struct logical_volume *origin_lv)
{
	const char *err = NULL; /* For error string */

	if (lv_is_cow(origin_lv))
		err = "snapshots";
	else if (lv_is_locked(origin_lv))
		err = "locked volumes";
	else if (lv_is_pvmove(origin_lv))
		err = "pvmoved volumes";
	else if (!lv_is_visible(origin_lv))
		err = "hidden volumes";
	else if (lv_is_merging_origin(origin_lv))
		err = "an origin that has a merging snapshot";
	else if (lv_is_cache_type(origin_lv) && !lv_is_cache(origin_lv))
		err = "cache type volumes";
	else if (lv_is_thin_type(origin_lv) && !lv_is_thin_volume(origin_lv))
		err = "thin pool type volumes";
	else if (lv_is_mirror_type(origin_lv)) {
		if (!lv_is_mirror(origin_lv))
			err = "mirror subvolumes";
		else {
			log_warn("WARNING: Snapshots of mirrors can deadlock under rare device failures.");
			log_warn("WARNING: Consider using the raid1 mirror type to avoid this.");
			log_warn("WARNING: See global/mirror_segtype_default in lvm.conf.");
		}
	} else if (lv_is_raid_type(origin_lv) && !lv_is_raid(origin_lv))
		err = "raid subvolumes";

	if (err) {
		log_error("Snapshots of %s are not supported.", err);
		return 0;
	}

	return 1;
}
