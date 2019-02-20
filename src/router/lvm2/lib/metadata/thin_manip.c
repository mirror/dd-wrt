/*
 * Copyright (C) 2011-2013 Red Hat, Inc. All rights reserved.
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
#include "lib/activate/activate.h"
#include "lib/locking/locking.h"
#include "lib/mm/memlock.h"
#include "lib/metadata/metadata.h"
#include "lib/metadata/segtype.h"
#include "lib/config/defaults.h"
#include "lib/display/display.h"

/* TODO: drop unused no_update */
int attach_pool_message(struct lv_segment *pool_seg, dm_thin_message_t type,
			struct logical_volume *lv, uint32_t delete_id,
			int no_update)
{
	struct lv_thin_message *tmsg;

	if (!seg_is_thin_pool(pool_seg)) {
		log_error(INTERNAL_ERROR "Cannot attach message to non-pool LV %s.",
			  display_lvname(pool_seg->lv));
		return 0;
	}

	if (pool_has_message(pool_seg, lv, delete_id)) {
		if (lv)
			log_error("Message referring LV %s already queued in pool %s.",
				  display_lvname(lv), display_lvname(pool_seg->lv));
		else
			log_error("Delete for device %u already queued in pool %s.",
				  delete_id, display_lvname(pool_seg->lv));
		return 0;
	}

	if (!(tmsg = dm_pool_alloc(pool_seg->lv->vg->vgmem, sizeof(*tmsg)))) {
		log_error("Failed to allocate memory for message.");
		return 0;
	}

	switch (type) {
	case DM_THIN_MESSAGE_CREATE_SNAP:
	case DM_THIN_MESSAGE_CREATE_THIN:
		tmsg->u.lv = lv;
		break;
	case DM_THIN_MESSAGE_DELETE:
		tmsg->u.delete_id = delete_id;
		break;
	default:
		log_error(INTERNAL_ERROR "Unsupported message type %u.", type);
		return 0;
	}

	tmsg->type = type;

	/* If the 1st message is add in non-read-only mode, modify transaction_id */
	if (!no_update && dm_list_empty(&pool_seg->thin_messages))
		pool_seg->transaction_id++;

	dm_list_add(&pool_seg->thin_messages, &tmsg->list);

	log_debug_metadata("Added %s message.",
			   (type == DM_THIN_MESSAGE_CREATE_SNAP ||
			    type == DM_THIN_MESSAGE_CREATE_THIN) ? "create" :
			   (type == DM_THIN_MESSAGE_DELETE) ? "delete" : "unknown");

	return 1;
}

int attach_thin_external_origin(struct lv_segment *seg,
				struct logical_volume *external_lv)
{
	if (seg->external_lv) {
		log_error(INTERNAL_ERROR "LV %s already has external origin.",
			  display_lvname(seg->lv));
		return 0;
	}

	seg->external_lv = external_lv;

	if (external_lv) {
		if (!add_seg_to_segs_using_this_lv(external_lv, seg))
			return_0;

		external_lv->external_count++;

		if (external_lv->status & LVM_WRITE) {
			log_verbose("Setting logical volume \"%s\" read-only.",
				    display_lvname(external_lv));
			external_lv->status &= ~LVM_WRITE;
		}

		/* FIXME Mark origin read-only?
		if (lv_is_cache(external_lv)) // read-only corigin of cache LV
			seg_lv(first_seg(external_lv), 0)->status &= ~LVM_WRITE;
		 */
	}

	return 1;
}

int detach_thin_external_origin(struct lv_segment *seg)
{
	if (seg->external_lv) {
		if (!lv_is_external_origin(seg->external_lv)) {
			log_error(INTERNAL_ERROR "Inconsitent external origin.");
			return 0;
		}

		if (!remove_seg_from_segs_using_this_lv(seg->external_lv, seg))
			return_0;

		seg->external_lv->external_count--;
		seg->external_lv = NULL;
	}

	return 1;
}

int lv_is_merging_thin_snapshot(const struct logical_volume *lv)
{
	struct lv_segment *seg = first_seg(lv);

	return (seg && seg->status & MERGING) ? 1 : 0;
}

/*
 * Check whether pool has some message queued for LV or for device_id
 * When LV is NULL and device_id is 0 it just checks for any message.
 */
int pool_has_message(const struct lv_segment *seg,
		     const struct logical_volume *lv, uint32_t device_id)
{
	const struct lv_thin_message *tmsg;

	if (!seg_is_thin_pool(seg)) {
		log_error(INTERNAL_ERROR "LV %s is not pool.", display_lvname(seg->lv));
		return 0;
	}

	if (!lv && !device_id)
		return !dm_list_empty(&seg->thin_messages);

	dm_list_iterate_items(tmsg, &seg->thin_messages) {
		switch (tmsg->type) {
		case DM_THIN_MESSAGE_CREATE_SNAP:
		case DM_THIN_MESSAGE_CREATE_THIN:
			if (tmsg->u.lv == lv)
				return 1;
			break;
		case DM_THIN_MESSAGE_DELETE:
			if (tmsg->u.delete_id == device_id)
				return 1;
			break;
		default:
			break;
		}
	}

	return 0;
}

int pool_is_active(const struct logical_volume *lv)
{
	struct lvinfo info;
	const struct seg_list *sl;

	if (!lv_is_thin_pool(lv)) {
		log_error(INTERNAL_ERROR "pool_is_active called with non-pool volume %s.",
			  display_lvname(lv));
		return 0;
	}

	/* On clustered VG, query every related thin pool volume */
	if (vg_is_clustered(lv->vg)) {
		if (lv_is_active(lv))
			return 1;

		dm_list_iterate_items(sl, &lv->segs_using_this_lv)
			if (lv_is_active(sl->seg->lv)) {
				log_debug_activation("Pool's thin volume %s is active.",
						     display_lvname(sl->seg->lv));
				return 1;
			}
	} else if (lv_info(lv->vg->cmd, lv, 1, &info, 0, 0) && info.exists)
		return 1; /* Non clustered VG - just checks for '-tpool' */

	return 0;
}

int thin_pool_feature_supported(const struct logical_volume *lv, int feature)
{
	static unsigned attr = 0U;
	struct lv_segment *seg;

	if (!lv_is_thin_pool(lv)) {
		log_error(INTERNAL_ERROR "LV %s is not thin pool.", display_lvname(lv));
		return 0;
	}

	seg = first_seg(lv);
	if ((attr == 0U) && activation() && seg->segtype &&
	    seg->segtype->ops->target_present &&
	    !seg->segtype->ops->target_present(lv->vg->cmd, NULL, &attr)) {
		log_error("%s: Required device-mapper target(s) not "
			  "detected in your kernel.", lvseg_name(seg));
		return 0;
	}

	return (attr & feature) ? 1 : 0;
}

int pool_metadata_min_threshold(const struct lv_segment *pool_seg)
{
	/*
	 * Hardcoded minimal requirment for thin pool target.
	 *
	 * In the metadata LV there should be minimum from either 4MiB of free space
	 * or at least 25% of free space, which applies when the size of thin pool's
	 * metadata is less than 16MiB.
	 */
	const dm_percent_t meta_min = DM_PERCENT_1 * 25;
	dm_percent_t meta_free = dm_make_percent(((4096 * 1024) >> SECTOR_SHIFT),
						pool_seg->metadata_lv->size);

	if (meta_min < meta_free)
		meta_free = meta_min;

	return DM_PERCENT_100 - meta_free;
}

int pool_below_threshold(const struct lv_segment *pool_seg)
{
	struct cmd_context *cmd = pool_seg->lv->vg->cmd;
	dm_percent_t percent;
	dm_percent_t min_threshold = pool_metadata_min_threshold(pool_seg);
	dm_percent_t threshold = DM_PERCENT_1 *
		find_config_tree_int(cmd, activation_thin_pool_autoextend_threshold_CFG,
				     lv_config_profile(pool_seg->lv));

	/* Data */
	if (!lv_thin_pool_percent(pool_seg->lv, 0, &percent))
		return_0;

	if (percent > threshold || percent >= DM_PERCENT_100) {
		log_debug("Threshold configured for free data space in "
			  "thin pool %s has been reached (%s%% >= %s%%).",
			  display_lvname(pool_seg->lv),
			  display_percent(cmd, percent),
			  display_percent(cmd, threshold));
		return 0;
	}

	/* Metadata */
	if (!lv_thin_pool_percent(pool_seg->lv, 1, &percent))
		return_0;


	if (percent >= min_threshold) {
		log_warn("WARNING: Remaining free space in metadata of thin pool %s "
			 "is too low (%s%% >= %s%%). "
			 "Resize is recommended.",
			 display_lvname(pool_seg->lv),
			 display_percent(cmd, percent),
			 display_percent(cmd, min_threshold));
		return 0;
	}


	if (percent > threshold) {
		log_debug("Threshold configured for free metadata space in "
			  "thin pool %s has been reached (%s%% > %s%%).",
			  display_lvname(pool_seg->lv),
			  display_percent(cmd, percent),
			  display_percent(cmd, threshold));
		return 0;
	}

	return 1;
}

/*
 * Detect overprovisioning and check lvm2 is configured for auto resize.
 *
 * If passed LV is thin volume/pool, check first only this one for overprovisiong.
 * Lots of test combined together.
 * Test is not detecting status of dmeventd, too complex for now...
 */
int pool_check_overprovisioning(const struct logical_volume *lv)
{
	const struct lv_list *lvl;
	const struct seg_list *sl;
	const struct logical_volume *pool_lv = NULL;
	struct cmd_context *cmd = lv->vg->cmd;
	const char *txt = "";
	uint64_t thinsum = 0, poolsum = 0, sz = ~0;
	int threshold, max_threshold = 0;
	int percent, min_percent = 100;
	int more_pools = 0;

	/* When passed thin volume, check related pool first */
	if (lv_is_thin_volume(lv))
		pool_lv = first_seg(lv)->pool_lv;
	else if (lv_is_thin_pool(lv))
		pool_lv = lv;

	if (pool_lv) {
		poolsum += pool_lv->size;
		dm_list_iterate_items(sl, &pool_lv->segs_using_this_lv)
			thinsum += sl->seg->lv->size;

		if (thinsum <= poolsum)
			return 1; /* All thins fit into this thin pool */
	}

	/* Sum all thins and all thin pools in VG */
	dm_list_iterate_items(lvl, &lv->vg->lvs) {
		if (!lv_is_thin_pool(lvl->lv))
			continue;

		threshold = find_config_tree_int(cmd, activation_thin_pool_autoextend_threshold_CFG,
						 lv_config_profile(lvl->lv));
		percent = find_config_tree_int(cmd, activation_thin_pool_autoextend_percent_CFG,
					       lv_config_profile(lvl->lv));
		if (threshold > max_threshold)
			max_threshold = threshold;
		if (percent < min_percent)
			min_percent = percent;

		if (lvl->lv == pool_lv)
			continue; /* Skip iteration for already checked thin pool */

		more_pools++;
		poolsum += lvl->lv->size;
		dm_list_iterate_items(sl, &lvl->lv->segs_using_this_lv)
			thinsum += sl->seg->lv->size;
	}

	if (thinsum <= poolsum)
		return 1; /* All fits for all pools */

	if ((sz = vg_size(lv->vg)) < thinsum)
		/* Thin sum size is above VG size */
		txt = " and the size of whole volume group";
	else if ((sz = vg_free(lv->vg)) < thinsum)
		/* Thin sum size is more then free space in a VG */
		txt = !sz ? "" : " and the amount of free space in volume group";
	else if ((max_threshold > 99) || !min_percent)
		/* There is some free space in VG, but it is not configured
		 * for growing - threshold is 100% or percent is 0% */
		sz = poolsum;
	else
		sz = UINT64_C(~0); /* No warning */

	if (sz != UINT64_C(~0)) {
		log_warn("WARNING: Sum of all thin volume sizes (%s) exceeds the "
			 "size of thin pool%s%s%s (%s).",
			 display_size(cmd, thinsum),
			 more_pools ? "" : " ",
			 more_pools ? "s" : display_lvname(pool_lv),
			 txt,
			 (sz > 0) ? display_size(cmd, sz) : "no free space in volume group");
		if (max_threshold > 99 || !min_percent)
			log_print_unless_silent("WARNING: You have not turned on protection against thin pools running out of space.");
		if (max_threshold > 99)
			log_print_unless_silent("WARNING: Set activation/thin_pool_autoextend_threshold below 100 to trigger automatic extension of thin pools before they get full.");
		if (!min_percent)
			log_print_unless_silent("WARNING: Set activation/thin_pool_autoextend_percent above 0 to specify by how much to extend thin pools reaching the threshold.");
		/* FIXME Also warn if there isn't sufficient free space for one pool extension to occur? */
	}

	return 1;
}

/*
 * Validate given external origin could be used with thin pool
 */
int pool_supports_external_origin(const struct lv_segment *pool_seg, const struct logical_volume *external_lv)
{
	uint32_t csize = pool_seg->chunk_size;

	if (((external_lv->size < csize) || (external_lv->size % csize)) &&
	    !thin_pool_feature_supported(pool_seg->lv, THIN_FEATURE_EXTERNAL_ORIGIN_EXTEND)) {
		log_error("Can't use \"%s\" as external origin with \"%s\" pool. "
			  "Size %s is not a multiple of pool's chunk size %s.",
			  display_lvname(external_lv), display_lvname(pool_seg->lv),
			  display_size(external_lv->vg->cmd, external_lv->size),
			  display_size(external_lv->vg->cmd, csize));
		return 0;
	}

	return 1;
}

struct logical_volume *find_pool_lv(const struct logical_volume *lv)
{
	struct lv_segment *seg;

	if (!(seg = first_seg(lv))) {
		log_error("LV %s has no segment.", display_lvname(lv));
		return NULL;
	}

	if (!(seg = find_pool_seg(seg)))
		return_NULL;

	return seg->lv;
}

/*
 * Find a free device_id for given thin_pool segment.
 *
 * \return
 * Free device id, or 0 if free device_id is not found.
 *
 * FIXME: Improve naive search and keep the value cached
 * and updated during VG lifetime (so no const for lv_segment)
 */
uint32_t get_free_pool_device_id(struct lv_segment *thin_pool_seg)
{
	uint32_t max_id = 0;
	struct seg_list *sl;

	if (!seg_is_thin_pool(thin_pool_seg)) {
		log_error(INTERNAL_ERROR
			  "Segment in %s is not a thin pool segment.",
			  display_lvname(thin_pool_seg->lv));
		return 0;
	}

	dm_list_iterate_items(sl, &thin_pool_seg->lv->segs_using_this_lv)
		if (sl->seg->device_id > max_id)
			max_id = sl->seg->device_id;

	if (++max_id > DM_THIN_MAX_DEVICE_ID) {
		/* FIXME Find empty holes instead of aborting! */
		log_error("Cannot find free device_id.");
		return 0;
	}

	log_debug_metadata("Found free pool device_id %u.", max_id);

	return max_id;
}

static int _check_pool_create(const struct logical_volume *lv)
{
	const struct lv_thin_message *lmsg;
	struct lvinfo info;

	dm_list_iterate_items(lmsg, &first_seg(lv)->thin_messages) {
		if (lmsg->type != DM_THIN_MESSAGE_CREATE_THIN)
			continue;
		/* When creating new thin LV, check for size would be needed */
		if (!lv_info(lv->vg->cmd, lv, 1, &info, 0, 0) ||
		    !info.exists) {
			log_error("Pool %s needs to be locally active for threshold check.",
				  display_lvname(lv));
			return 0;
		}
		if (!pool_below_threshold(first_seg(lv))) {
			log_error("Free space in pool %s is above threshold, new volumes are not allowed.",
				  display_lvname(lv));
			return 0;
		}
		break;
	}

	return 1;
}

int update_pool_lv(struct logical_volume *lv, int activate)
{
	int monitored;
	int ret = 1;

	if (!lv_is_thin_pool(lv)) {
		log_error(INTERNAL_ERROR "Updated LV %s is not pool.", display_lvname(lv));
		return 0;
	}

	if (dm_list_empty(&(first_seg(lv)->thin_messages)))
		return 1; /* No messages */

	if (activate) {
		/* If the pool is not active, do activate deactivate */
		monitored = dmeventd_monitor_mode();
		init_dmeventd_monitor(DMEVENTD_MONITOR_IGNORE);
		if (!lv_is_active(lv)) {
			/*
			 * FIXME:
			 *   Rewrite activation code to handle whole tree of thinLVs
			 *   as this version has major problem when it does not know
			 *   which Node has pool active.
			 */
			if (!activate_lv(lv->vg->cmd, lv)) {
				init_dmeventd_monitor(monitored);
				return_0;
			}
			if (!lv_is_active(lv)) {
				init_dmeventd_monitor(monitored);
				log_error("Cannot activate thin pool %s, perhaps skipped in lvm.conf volume_list?",
					  display_lvname(lv));
				return 0;
			}
		} else
			activate = 0; /* Was already active */

		if (!(ret = _check_pool_create(lv)))
			stack; /* Safety guard, needs local presence of thin-pool target */
		else {
			if (!(ret = suspend_lv_origin(lv->vg->cmd, lv)))
				/* Send messages */
				log_error("Failed to suspend %s with queued messages.", display_lvname(lv));

			/* Even failing suspend needs resume */
			if (!resume_lv_origin(lv->vg->cmd, lv)) {
				log_error("Failed to resume %s.", display_lvname(lv));
				ret = 0;
			}
		}

		if (activate &&
		    !deactivate_lv(lv->vg->cmd, lv)) {
			log_error("Failed to deactivate %s.", display_lvname(lv));
			ret = 0;
		}
		init_dmeventd_monitor(monitored);

		/* Unlock memory if possible */
		memlock_unlock(lv->vg->cmd);

		if (!ret)
			return_0;
	}

	dm_list_init(&(first_seg(lv)->thin_messages));

	if (!vg_write(lv->vg) || !vg_commit(lv->vg))
		return_0;

	return ret;
}

static uint64_t _estimate_size(uint32_t data_extents, uint32_t extent_size, uint64_t size)
{
	/*
	 * nr_pool_blocks = data_size / metadata_size
	 * chunk_size = nr_pool_blocks * 64b / sector_size
	 */
	return (uint64_t) data_extents * extent_size / (size * (SECTOR_SIZE / UINT64_C(64)));
}

/* Estimate thin pool metadata size from data size and chunks size (in sector units) */
static uint64_t _estimate_metadata_size(uint32_t data_extents, uint32_t extent_size, uint32_t chunk_size)
{
	return _estimate_size(data_extents, extent_size, chunk_size);
}

/* Estimate maximal supportable thin pool data size for given chunk_size */
static uint64_t _estimate_max_data_size(uint32_t chunk_size)
{
	return  chunk_size * (DEFAULT_THIN_POOL_MAX_METADATA_SIZE * 2) * SECTOR_SIZE / UINT64_C(64);
}

/* Estimate thin pool chunk size from data and metadata size (in sector units) */
static uint32_t _estimate_chunk_size(uint32_t data_extents, uint32_t extent_size,
				     uint64_t metadata_size, int attr)
{
	uint32_t chunk_size = _estimate_size(data_extents, extent_size, metadata_size);

	if (attr & THIN_FEATURE_BLOCK_SIZE) {
		/* Round up to 64KB */
		chunk_size += DM_THIN_MIN_DATA_BLOCK_SIZE - 1;
		chunk_size &= ~(uint32_t)(DM_THIN_MIN_DATA_BLOCK_SIZE - 1);
	} else {
		/* Round up to nearest power of 2 */
		chunk_size--;
		chunk_size |= chunk_size >> 1;
		chunk_size |= chunk_size >> 2;
		chunk_size |= chunk_size >> 4;
		chunk_size |= chunk_size >> 8;
		chunk_size |= chunk_size >> 16;
		chunk_size++;
	}

	if (chunk_size < DM_THIN_MIN_DATA_BLOCK_SIZE)
		chunk_size = DM_THIN_MIN_DATA_BLOCK_SIZE;
	else if (chunk_size > DM_THIN_MAX_DATA_BLOCK_SIZE)
		chunk_size = DM_THIN_MAX_DATA_BLOCK_SIZE;

	return chunk_size;
}

int get_default_allocation_thin_pool_chunk_size(struct cmd_context *cmd, struct profile *profile,
						uint32_t *chunk_size, int *chunk_size_calc_method)
{
	const char *str;

	if (!(str = find_config_tree_str(cmd, allocation_thin_pool_chunk_size_policy_CFG, profile))) {
		log_error(INTERNAL_ERROR "Cannot find configuration.");
		return 0;
	}

	if (!strcasecmp(str, "generic")) {
		*chunk_size = DEFAULT_THIN_POOL_CHUNK_SIZE * 2;
		*chunk_size_calc_method = THIN_CHUNK_SIZE_CALC_METHOD_GENERIC;
	} else if (!strcasecmp(str, "performance")) {
		*chunk_size = DEFAULT_THIN_POOL_CHUNK_SIZE_PERFORMANCE * 2;
		*chunk_size_calc_method = THIN_CHUNK_SIZE_CALC_METHOD_PERFORMANCE;
	} else {
		log_error("Thin pool chunk size calculation policy \"%s\" is unrecognised.", str);
		return 0;
	}

	return 1;
}

int update_thin_pool_params(struct cmd_context *cmd,
			    struct profile *profile,
			    uint32_t extent_size,
			    const struct segment_type *segtype,
			    unsigned attr,
			    uint32_t pool_data_extents,
			    uint32_t *pool_metadata_extents,
			    int *chunk_size_calc_method, uint32_t *chunk_size,
			    thin_discards_t *discards, thin_zero_t *zero_new_blocks)
{
	uint64_t pool_metadata_size = (uint64_t) *pool_metadata_extents * extent_size;
	uint32_t estimate_chunk_size;
	uint64_t max_pool_data_size;
	const char *str;

	if (!*chunk_size &&
	    find_config_tree_node(cmd, allocation_thin_pool_chunk_size_CFG, profile))
		*chunk_size = find_config_tree_int(cmd, allocation_thin_pool_chunk_size_CFG, profile) * 2;

	if (*chunk_size && !(attr & THIN_FEATURE_BLOCK_SIZE) &&
	    !is_power_of_2(*chunk_size)) {
		log_error("Chunk size must be a power of 2 for this thin target version.");
		return 0;
	}

	if ((*discards == THIN_DISCARDS_UNSELECTED) &&
	    find_config_tree_node(cmd, allocation_thin_pool_discards_CFG, profile)) {
		if (!(str = find_config_tree_str(cmd, allocation_thin_pool_discards_CFG, profile))) {
			log_error(INTERNAL_ERROR "Could not find configuration.");
			return 0;
		}
		if (!set_pool_discards(discards, str))
			return_0;
	}

	if ((*zero_new_blocks == THIN_ZERO_UNSELECTED) &&
	    find_config_tree_node(cmd, allocation_thin_pool_zero_CFG, profile))
		*zero_new_blocks = find_config_tree_bool(cmd, allocation_thin_pool_zero_CFG, profile)
			? THIN_ZERO_YES : THIN_ZERO_NO;

	if (!pool_metadata_size) {
		if (!*chunk_size) {
			if (!get_default_allocation_thin_pool_chunk_size(cmd, profile,
									 chunk_size,
									 chunk_size_calc_method))
				return_0;

			pool_metadata_size = _estimate_metadata_size(pool_data_extents, extent_size, *chunk_size);

			/* Check if we should eventually use bigger chunk size */
			while ((pool_metadata_size >
				(DEFAULT_THIN_POOL_OPTIMAL_METADATA_SIZE * 2)) &&
			       (*chunk_size < DM_THIN_MAX_DATA_BLOCK_SIZE)) {
				*chunk_size <<= 1;
				pool_metadata_size >>= 1;
			}
			log_verbose("Setting chunk size to %s.",
				    display_size(cmd, *chunk_size));
		} else {
			pool_metadata_size = _estimate_metadata_size(pool_data_extents, extent_size, *chunk_size);

			if (pool_metadata_size > (DEFAULT_THIN_POOL_MAX_METADATA_SIZE * 2)) {
				/* Suggest bigger chunk size */
				estimate_chunk_size =
					_estimate_chunk_size(pool_data_extents, extent_size,
							     (DEFAULT_THIN_POOL_MAX_METADATA_SIZE * 2), attr);
				log_warn("WARNING: Chunk size is too small for pool, suggested minimum is %s.",
					 display_size(cmd, estimate_chunk_size));
			}
		}

		/* Round up to extent size silently */
		if (pool_metadata_size % extent_size)
			pool_metadata_size += extent_size - pool_metadata_size % extent_size;
	} else {
		estimate_chunk_size = _estimate_chunk_size(pool_data_extents, extent_size,
							   pool_metadata_size, attr);

		/* Check to eventually use bigger chunk size */
		if (!*chunk_size) {
			*chunk_size = estimate_chunk_size;
			log_verbose("Setting chunk size %s.", display_size(cmd, *chunk_size));
		} else if (*chunk_size < estimate_chunk_size) {
			/* Suggest bigger chunk size */
			log_warn("WARNING: Chunk size is smaller then suggested minimum size %s.",
				 display_size(cmd, estimate_chunk_size));
		}
	}

	max_pool_data_size = _estimate_max_data_size(*chunk_size);
	if ((max_pool_data_size / extent_size) < pool_data_extents) {
		log_error("Selected chunk size %s cannot address more then %s of thin pool data space.",
			  display_size(cmd, *chunk_size), display_size(cmd, max_pool_data_size));
		return 0;
	}

	log_print_unless_silent("Thin pool volume with chunk size %s can address at most %s of data.",
				display_size(cmd, *chunk_size), display_size(cmd, max_pool_data_size));

	if (!validate_thin_pool_chunk_size(cmd, *chunk_size))
		return_0;

	if (pool_metadata_size > (2 * DEFAULT_THIN_POOL_MAX_METADATA_SIZE)) {
		pool_metadata_size = 2 * DEFAULT_THIN_POOL_MAX_METADATA_SIZE;
		if (*pool_metadata_extents)
			log_warn("WARNING: Maximum supported pool metadata size is %s.",
				 display_size(cmd, pool_metadata_size));
	} else if (pool_metadata_size < (2 * DEFAULT_THIN_POOL_MIN_METADATA_SIZE)) {
		pool_metadata_size = 2 * DEFAULT_THIN_POOL_MIN_METADATA_SIZE;
		if (*pool_metadata_extents)
			log_warn("WARNING: Minimum supported pool metadata size is %s.",
				 display_size(cmd, pool_metadata_size));
	}

	if (!(*pool_metadata_extents =
	      extents_from_size(cmd, pool_metadata_size, extent_size)))
		return_0;

	if ((uint64_t) *chunk_size > (uint64_t) pool_data_extents * extent_size) {
		log_error("Size of %s data volume cannot be smaller than chunk size %s.",
			  segtype->name, display_size(cmd, *chunk_size));
		return 0;
	}

	if ((*discards == THIN_DISCARDS_UNSELECTED) &&
	    !set_pool_discards(discards, DEFAULT_THIN_POOL_DISCARDS))
		return_0;

	if (*zero_new_blocks == THIN_ZERO_UNSELECTED) {
		*zero_new_blocks = (DEFAULT_THIN_POOL_ZERO) ? THIN_ZERO_YES : THIN_ZERO_NO;
		log_verbose("%s pool zeroing on default.", (*zero_new_blocks == THIN_ZERO_YES) ?
			    "Enabling" : "Disabling");
	}

	if ((*zero_new_blocks == THIN_ZERO_YES) &&
	    (*chunk_size >= DEFAULT_THIN_POOL_CHUNK_SIZE_PERFORMANCE * 2)) {
		log_warn("WARNING: Pool zeroing and %s large chunk size slows down thin provisioning.",
			 display_size(cmd, *chunk_size));
		log_warn("WARNING: Consider disabling zeroing (-Zn) or using smaller chunk size (<%s).",
			 display_size(cmd, DEFAULT_THIN_POOL_CHUNK_SIZE_PERFORMANCE * 2));
	}

	log_verbose("Preferred pool metadata size %s.",
		    display_size(cmd, (uint64_t)*pool_metadata_extents * extent_size));

	return 1;
}

int set_pool_discards(thin_discards_t *discards, const char *str)
{
	if (!strcasecmp(str, "passdown"))
		*discards = THIN_DISCARDS_PASSDOWN;
	else if (!strcasecmp(str, "nopassdown"))
		*discards = THIN_DISCARDS_NO_PASSDOWN;
	else if (!strcasecmp(str, "ignore"))
		*discards = THIN_DISCARDS_IGNORE;
	else {
		log_error("Thin pool discards type \"%s\" is unknown.", str);
		return 0;
	}

	return 1;
}

const char *get_pool_discards_name(thin_discards_t discards)
{
	switch (discards) {
	case THIN_DISCARDS_PASSDOWN:
                return "passdown";
	case THIN_DISCARDS_NO_PASSDOWN:
		return "nopassdown";
	case THIN_DISCARDS_IGNORE:
		return "ignore";
	default:
		log_error(INTERNAL_ERROR "Unknown discards type encountered.");
		return "unknown";
	}
}

int lv_is_thin_origin(const struct logical_volume *lv, unsigned int *snap_count)
{
	struct seg_list *segl;
	int r = 0;

	if (snap_count)
		*snap_count = 0;

	if (lv_is_thin_volume(lv))
		dm_list_iterate_items(segl, &lv->segs_using_this_lv)
			if (segl->seg->origin == lv) {
				r = 1;

				if (!snap_count)
					break;/* not interested in number of snapshots */

				(*snap_count)++;
			}

	return r;
}

int lv_is_thin_snapshot(const struct logical_volume *lv)
{
	struct lv_segment *seg;

	if (!lv_is_thin_volume(lv))
		return 0;

	if ((seg = first_seg(lv)) && (seg->origin || seg->external_lv))
		return 1;

	return 0;
}

/*
 * Explict check of new thin pool for usability
 *
 * Allow use of thin pools by external apps. When lvm2 metadata has
 * transaction_id == 0 for a new thin pool, it will explicitely validate
 * the pool is still unused.
 *
 * To prevent lvm2 to create thin volumes in externally used thin pools
 * simply increment its transaction_id.
 */
int check_new_thin_pool(const struct logical_volume *pool_lv)
{
	struct cmd_context *cmd = pool_lv->vg->cmd;
	uint64_t transaction_id;

	/* For transaction_id check LOCAL activation is required */
	if (!activate_lv(cmd, pool_lv)) {
		log_error("Aborting. Failed to locally activate thin pool %s.",
			  display_lvname(pool_lv));
		return 0;
	}

	/* With volume lists, check pool really is locally active */
	if (!lv_thin_pool_transaction_id(pool_lv, &transaction_id)) {
		log_error("Cannot read thin pool %s transaction id locally, perhaps skipped in lvm.conf volume_list?",
			  display_lvname(pool_lv));
		return 0;
	}

	/* Require pool to have same transaction_id as new  */
	if (first_seg(pool_lv)->transaction_id != transaction_id) {
		log_error("Cannot use thin pool %s with transaction id "
			  FMTu64 " for thin volumes. "
			  "Expected transaction id %" PRIu64 ".",
			  display_lvname(pool_lv), transaction_id,
			  first_seg(pool_lv)->transaction_id);
		return 0;
	}

	log_verbose("Deactivating public thin pool %s.",
		    display_lvname(pool_lv));

	/* Prevent any 'race' with in-use thin pool and always deactivate */
	if (!deactivate_lv(pool_lv->vg->cmd, pool_lv)) {
		log_error("Aborting. Could not deactivate thin pool %s.",
			  display_lvname(pool_lv));
		return 0;
	}

	return 1;
}

int validate_thin_pool_chunk_size(struct cmd_context *cmd, uint32_t chunk_size)
{
	const uint32_t min_size = DM_THIN_MIN_DATA_BLOCK_SIZE;
	const uint32_t max_size = DM_THIN_MAX_DATA_BLOCK_SIZE;
	int r = 1;

	if ((chunk_size < min_size) || (chunk_size > max_size)) {
		log_error("Thin pool chunk size %s is not in the range %s to %s.",
			  display_size(cmd, chunk_size),
			  display_size(cmd, min_size),
			  display_size(cmd, max_size));
		r = 0;
	}

	if (chunk_size & (min_size - 1)) {
		log_error("Thin pool chunk size %s must be a multiple of %s.",
			  display_size(cmd, chunk_size),
			  display_size(cmd, min_size));
		r = 0;
	}

	return r;
}
