/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2017 Red Hat, Inc. All rights reserved.
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
#include "lib/config/defaults.h"
#include "lib/metadata/metadata.h"
#include "lib/metadata/lv_alloc.h"
#include "lib/metadata/pv_alloc.h"
#include "lib/datastruct/str_list.h"
#include "lib/metadata/segtype.h"

/*
 * Attempt to merge two adjacent segments.
 * Currently only supports striped segments on AREA_PV.
 * Returns success if successful, in which case 'first'
 * gets adjusted to contain both areas.
 */
static int _merge(struct lv_segment *first, struct lv_segment *second)
{
	if (!first || !second || first->segtype != second->segtype ||
	    !first->segtype->ops->merge_segments)
		return 0;

	return first->segtype->ops->merge_segments(first, second);
}

int lv_merge_segments(struct logical_volume *lv)
{
	struct dm_list *segh, *t;
	struct lv_segment *seg, *current, *prev = NULL;

	/*
	 * Don't interfere with pvmoves as they rely upon two LVs
	 * having a matching segment structure.
	 */

	if (lv_is_locked(lv) || lv_is_pvmove(lv))
		return 1;

	if (lv_is_mirror_image(lv) &&
	    (seg = get_only_segment_using_this_lv(lv)) &&
	    (lv_is_locked(seg->lv) || lv_is_pvmove(seg->lv)))
		return 1;

	dm_list_iterate_safe(segh, t, &lv->segments) {
		current = dm_list_item(segh, struct lv_segment);

		if (_merge(prev, current))
			dm_list_del(&current->list);
		else
			prev = current;
	}

	return 1;
}

#define ERROR_MAX 100
#define inc_error_count \
	if (error_count++ > ERROR_MAX)	\
		goto out

#define seg_error(msg) do { \
		log_error("LV %s, segment %u invalid: %s for %s segment.", \
			  seg->lv->name, seg_count, (msg), lvseg_name(seg)); \
		if ((*error_count)++ > ERROR_MAX) \
			return; \
	} while (0)

/*
 * RAID segment property checks.
 *
 * Checks in here shall catch any
 * bogus segment structure setup.
 */
#define raid_seg_error(msg) do { \
	log_error("LV %s invalid: %s for %s segment", \
		  seg->lv->name, (msg), lvseg_name(seg)); \
	if ((*error_count)++ > ERROR_MAX) \
		return; \
} while (0)

#define raid_seg_error_val(msg, val) do { \
	log_error("LV %s invalid: %s (is %u) for %s segment", \
		  seg->lv->name, (msg), (val), lvseg_name(seg)); \
	if ((*error_count)++ > ERROR_MAX) \
		return; \
} while(0)

/* Check segment LV for reshape flags. */
static int _check_raid_seg_reshape_flags(struct lv_segment *seg)
{
	return ((seg->lv->status & LV_RESHAPE) ||
		(seg->lv->status & LV_RESHAPE_DELTA_DISKS_MINUS) ||
		(seg->lv->status & LV_RESHAPE_DELTA_DISKS_PLUS));
}

/* Check raid0 segment properties in @seg */
static void _check_raid0_seg(struct lv_segment *seg, int *error_count)
{
	if (seg_is_raid0_meta(seg) &&
	    !seg->meta_areas)
		raid_seg_error("no meta areas");
	if (!seg_is_raid0_meta(seg) &&
	    seg->meta_areas)
		raid_seg_error("meta areas");
	if (!seg->stripe_size)
		raid_seg_error("zero stripe size");
	if (!is_power_of_2(seg->stripe_size))
		raid_seg_error_val("non power of 2 stripe size", seg->stripe_size);
	if (seg->region_size)
		raid_seg_error_val("non-zero region_size", seg->region_size);
	if (seg->writebehind)
		raid_seg_error_val("non-zero write behind", seg->writebehind);
	if (seg->min_recovery_rate)
		raid_seg_error_val("non-zero min recovery rate", seg->min_recovery_rate);
	if (seg->max_recovery_rate)
		raid_seg_error_val("non-zero max recovery rate", seg->max_recovery_rate);
	if ((seg->lv->status & LV_RESHAPE_DATA_OFFSET) || seg->data_offset > 1)
		raid_seg_error_val("data_offset", seg->data_offset);
	if (_check_raid_seg_reshape_flags(seg))
		raid_seg_error("reshape");
}

/* Check RAID @seg for non-zero, power of 2 region size and min recovery rate <= max */
static void _check_raid_region_recovery(struct lv_segment *seg, int *error_count)
{
	if (!seg->region_size)
		raid_seg_error("zero region_size");
	if (!is_power_of_2(seg->region_size))
		raid_seg_error_val("non power of 2 region size", seg->region_size);
	/* min/max recovery rate may be zero but min may not be larger than max if set */
	if (seg->max_recovery_rate &&
	    seg->min_recovery_rate > seg->max_recovery_rate)
		raid_seg_error_val("min recovery larger than max recovery", seg->min_recovery_rate);
}

/* Check raid1 segment properties in @seg */
static void _check_raid1_seg(struct lv_segment *seg, int *error_count)
{
	if (!seg->meta_areas)
		raid_seg_error("no meta areas");
	if (seg->stripe_size)
		raid_seg_error_val("non-zero stripe size", seg->stripe_size);
	if ((seg->lv->status & LV_RESHAPE_DATA_OFFSET) || seg->data_offset > 1)
		raid_seg_error_val("data_offset", seg->data_offset);
	if (_check_raid_seg_reshape_flags(seg))
		raid_seg_error("reshape");
	_check_raid_region_recovery(seg, error_count);
}

/* Check raid4/5/6/10 segment properties in @seg */
static void _check_raid45610_seg(struct lv_segment *seg, int *error_count)
{
	/* Checks applying to any raid4/5/6/10 */
	/*
	 * Allow raid4 + raid5_n to get activated w/o metadata.
	 *
	 * This is mandatory during conversion between them,
	 * because switching the dedicated parity SubLVs
	 * beginning <-> end changes the roles of all SubLVs
	 * which the kernel would reject.
	 */
	if (!(seg_is_raid4(seg) || seg_is_raid5_n(seg)) && !seg->meta_areas)
		raid_seg_error("no meta areas");
	if (!seg->stripe_size)
		raid_seg_error("zero stripe size");
	if (!is_power_of_2(seg->stripe_size))
		raid_seg_error_val("non power of 2 stripe size", seg->stripe_size);
	_check_raid_region_recovery(seg, error_count);
	/* END: checks applying to any raid4/5/6/10 */

	if (seg->data_offset > 1) {
		if (seg->lv->status & LV_RESHAPE_DATA_OFFSET) {
			if (seg->data_offset & (seg->lv->vg->extent_size - 1))
				raid_seg_error_val("data_offset", seg->data_offset);
		} else
			raid_seg_error_val("data_offset", seg->data_offset);
	}

	/* Specific checks per raid level */
	if (seg_is_raid4(seg) ||
	    seg_is_any_raid5(seg)) {
		/*
		 * To allow for takeover between the MD raid1 and
		 * raid4/5 personalities, exactly 2 areas (i.e. DataLVs)
		 * can be mirrored by all raid1, raid4 and raid5 personalities.
		 * Hence allow a minimum of 2 areas.
		 */
		if (seg->area_count < 2)
			raid_seg_error_val("minimum 2 areas required", seg->area_count);
	} else if (seg_is_any_raid6(seg)) {
		/*
		 * FIXME: MD raid6 supports a minimum of 4 areas.
		 *	  LVM requests a minimum of 5 due to easier
		 *	  processing of SubLVs to replace.
		 *
		 *	  Once that obstacle got removed, allow for a minimum of 4.
		 */
		if (seg->area_count < 5)
			raid_seg_error_val("minimum 5 areas required", seg->area_count);
	} else if (seg_is_raid10(seg)) {
		/*
		 * FIXME: raid10 area_count minimum has to change to 2 once we
		 *	  support data_copies and odd numbers of stripes
		 */
		if (seg->area_count < 4)
			raid_seg_error_val("minimum 4 areas required", seg->area_count);
		if (seg->writebehind)
			raid_seg_error_val("non-zero writebehind", seg->writebehind);
	}
}

/* Check any non-RAID segment struct members in @seg and increment @error_count for any bogus ones */
static void _check_non_raid_seg_members(struct lv_segment *seg, int *error_count)
{
	if (seg->origin) /* snap and thin */
		raid_seg_error("non-zero origin LV");
	if (seg->cow) /* snap */
		raid_seg_error("non-zero cow LV");
	if (!dm_list_empty(&seg->origin_list)) /* snap */
		raid_seg_error("non-zero origin_list");
	/* .... more members? */
}

static void _check_raid_sublvs(struct lv_segment *seg, int *error_count)
{
	unsigned s;

	for (s = 0; s < seg->area_count; s++) {
		if (seg_type(seg, s) != AREA_LV)
			raid_seg_error("no raid image SubLV");

		if ((seg_lv(seg, s)->status & LVM_WRITE) &&
		    !(seg->lv->status & LV_ACTIVATION_SKIP) &&
		    lv_is_visible(seg_lv(seg, s)))
			raid_seg_error("visible raid image LV");

		if (!seg_is_raid_with_meta(seg) || !seg->meta_areas)
			continue;

		if (seg_metatype(seg, s) != AREA_LV)
			raid_seg_error("no raid meta SubLV");
		else if (!(seg->lv->status & LV_ACTIVATION_SKIP) &&
			 lv_is_visible(seg_metalv(seg, s)))
			raid_seg_error("visible raid meta LV");
	}
}

/*
 * Check RAID segment struct members of @seg for acceptable
 * properties and increment @error_count for any bogus ones.
 */
static void _check_raid_seg(struct lv_segment *seg, int *error_count)
{
	uint32_t area_len, s;

	/* General checks applying to all RAIDs */
	if (!seg->area_count)
		raid_seg_error("zero area count");

	if (!seg->areas)
		raid_seg_error("zero areas");

	if (seg->extents_copied > seg->len)
		raid_seg_error_val("extents_copied too large", seg->extents_copied);

	/* Default < 10, change once raid1 split shift and rename SubLVs works! */
	if (seg_is_raid1(seg)) {
		if (seg->area_count > DEFAULT_RAID1_MAX_IMAGES) {
			log_error("LV %s invalid: maximum supported areas %u (is %u) for %s segment",
			  	seg->lv->name, DEFAULT_RAID1_MAX_IMAGES, seg->area_count, lvseg_name(seg));
				if ((*error_count)++ > ERROR_MAX)
					return;
		}
	} else if (seg->area_count > DEFAULT_RAID_MAX_IMAGES) {
		log_error("LV %s invalid: maximum supported areas %u (is %u) for %s segment",
		  	seg->lv->name, DEFAULT_RAID_MAX_IMAGES, seg->area_count, lvseg_name(seg));
			if ((*error_count)++ > ERROR_MAX)
				return;
	}

	/* FIXME: should we check any non-RAID segment struct members at all? */
	_check_non_raid_seg_members(seg, error_count);

	/* Check for any DataLV flaws like non-existing ones or size variations */
	for (area_len = s = 0; s < seg->area_count; s++) {
		if (seg_type(seg, s) != AREA_LV)
			raid_seg_error("no DataLV");
		if (!lv_is_raid_image(seg_lv(seg, s)))
			raid_seg_error("DataLV without RAID image flag");
		if (area_len &&
		    area_len != seg_lv(seg, s)->le_count) {
				raid_seg_error_val("DataLV size variations",
		    				   seg_lv(seg, s)->le_count);
		} else
			area_len = seg_lv(seg, s)->le_count;
	}

	/* Check for any MetaLV flaws like non-existing ones or size variations */
	if (seg->meta_areas)
		for (area_len = s = 0; s < seg->area_count; s++) {
			if (seg_metatype(seg, s) == AREA_UNASSIGNED)
				continue;

			if (seg_metatype(seg, s) != AREA_LV) {
				raid_seg_error("no MetaLV");
				continue;
			}

			if (!lv_is_raid_metadata(seg_metalv(seg, s)))
				raid_seg_error("MetaLV without RAID metadata flag");
			if (area_len &&
			    area_len != seg_metalv(seg, s)->le_count) {
				raid_seg_error_val("MetaLV size variations",
		    				   seg_metalv(seg, s)->le_count);
			} else
				area_len = seg_metalv(seg, s)->le_count;
		}
	/* END: general checks applying to all RAIDs */

	/* Specific segment type checks from here on */
	if (seg_is_any_raid0(seg))
		_check_raid0_seg(seg, error_count);
	else if (seg_is_raid1(seg))
		_check_raid1_seg(seg, error_count);
	else if (seg_is_raid4(seg) ||
		 seg_is_any_raid5(seg) ||
		 seg_is_any_raid6(seg) ||
		 seg_is_raid10(seg))
		_check_raid45610_seg(seg, error_count);
	else
		raid_seg_error("bogus RAID segment type");

	_check_raid_sublvs(seg, error_count);
}
/* END: RAID segment property checks. */

static void _check_lv_segment(struct logical_volume *lv, struct lv_segment *seg,
			      unsigned seg_count, int *error_count)
{
	struct lv_segment *seg2;

	if (lv_is_mirror_image(lv) &&
	    (!(seg2 = find_mirror_seg(seg)) || !seg_is_mirrored(seg2)))
		seg_error("mirror image is not mirrored");

	if (seg_is_cache(seg)) {
		if (!lv_is_cache(lv))
			seg_error("is not flagged as cache LV");

		if (!seg->pool_lv) {
			seg_error("is missing cache pool LV");
		} else if (!lv_is_cache_pool(seg->pool_lv))
			seg_error("is not referencing cache pool LV");
	} else { /* !cache */
		if (seg->cleaner_policy)
			seg_error("sets cleaner_policy");
	}

	if (seg_is_cache_pool(seg)) {
		if (!dm_list_empty(&seg->lv->segs_using_this_lv)) {
			switch (seg->cache_metadata_format) {
			case CACHE_METADATA_FORMAT_2:
			case CACHE_METADATA_FORMAT_1:
				break;
			default:
				seg_error("has invalid cache metadata format");
			}
			switch (seg->cache_mode) {
			case CACHE_MODE_WRITETHROUGH:
			case CACHE_MODE_WRITEBACK:
			case CACHE_MODE_PASSTHROUGH:
				break;
			default:
				seg_error("has invalid cache's feature flag");
			}
			if (!seg->policy_name)
				seg_error("is missing cache policy name");
		}
		if (!validate_cache_chunk_size(lv->vg->cmd, seg->chunk_size))
			seg_error("has invalid chunk size.");
		if (seg->lv->status & LV_METADATA_FORMAT) {
			if (seg->cache_metadata_format != CACHE_METADATA_FORMAT_2)
				seg_error("sets METADATA_FORMAT flag");
		} else if (seg->cache_metadata_format == CACHE_METADATA_FORMAT_2)
			seg_error("is missing METADATA_FORMAT flag");
	} else { /* !cache_pool */
		if (seg->cache_metadata_format)
			seg_error("sets cache metadata format");
		if (seg->cache_mode)
			seg_error("sets cache mode");
		if (seg->policy_name)
			seg_error("sets policy name");
		if (seg->policy_settings)
			seg_error("sets policy settings");
		if (seg->lv->status & LV_METADATA_FORMAT)
			seg_error("sets METADATA_FORMAT flag");
	}

	if (!seg_can_error_when_full(seg) && lv_is_error_when_full(lv))
		seg_error("does not support flag ERROR_WHEN_FULL.");

	if (seg_is_mirrored(seg)) {
		/* Check mirror log - which is attached to the mirrored seg */
		if (seg->log_lv) {
			if (!lv_is_mirror_log(seg->log_lv))
				seg_error("log LV is not a mirror log");

			if (!(seg2 = first_seg(seg->log_lv)) || (find_mirror_seg(seg2) != seg))
				seg_error("log LV does not point back to mirror segment");
		}
		if (seg_is_mirror(seg)) {
			if (!seg->region_size)
				seg_error("region size is zero");
			else if (seg->region_size > seg->lv->size)
				seg_error("region size is bigger then LV itself");
			else if (!is_power_of_2(seg->region_size))
				seg_error("region size is non power of 2");
		}
	} else { /* !mirrored */
		if (seg->log_lv) {
			if (lv_is_raid_image(lv))
				seg_error("log LV is not a mirror log or a RAID image");
		}
	}

	if (seg_is_raid(seg))
		_check_raid_seg(seg, error_count);
	else if (!lv_is_raid_type(lv) &&
		 _check_raid_seg_reshape_flags(seg))
		seg_error("reshape");

	if (seg_is_pool(seg)) {
		if ((seg->area_count != 1) || (seg_type(seg, 0) != AREA_LV)) {
			seg_error("is missing a pool data LV");
		} else if (!(seg2 = first_seg(seg_lv(seg, 0))) || (find_pool_seg(seg2) != seg))
			seg_error("data LV does not refer back to pool LV");

		if (!seg->metadata_lv) {
			seg_error("is missing a pool metadata LV");
		} else if (!(seg2 = first_seg(seg->metadata_lv)) || (find_pool_seg(seg2) != seg))
			seg_error("metadata LV does not refer back to pool LV");
	} else { /* !thin_pool && !cache_pool */
		if (seg->metadata_lv)
			seg_error("must not have pool metadata LV set");
	}

	if (seg_is_thin_pool(seg)) {
		if (!lv_is_thin_pool(lv))
			seg_error("is not flagged as thin pool LV");

		if (lv_is_thin_volume(lv))
			seg_error("is a thin volume that must not contain thin pool segment");

		if (!validate_thin_pool_chunk_size(lv->vg->cmd, seg->chunk_size))
			seg_error("has invalid chunk size.");

		if (seg->zero_new_blocks != THIN_ZERO_YES &&
                    seg->zero_new_blocks != THIN_ZERO_NO)
			seg_error("zero_new_blocks is invalid");
	} else { /* !thin_pool */
		if (seg->zero_new_blocks != THIN_ZERO_UNSELECTED)
			seg_error("sets zero_new_blocks");
		if (seg->discards != THIN_DISCARDS_UNSELECTED)
			seg_error("sets discards");
		if (!dm_list_empty(&seg->thin_messages))
			seg_error("sets thin_messages list");
	}

	if (seg_is_thin_volume(seg)) {
		if (!lv_is_thin_volume(lv))
			seg_error("is not flagged as thin volume LV");

		if (lv_is_thin_pool(lv))
			seg_error("is a thin pool that must not contain thin volume segment");

		if (!seg->pool_lv) {
			seg_error("is missing thin pool LV");
		} else if (!lv_is_thin_pool(seg->pool_lv))
			seg_error("is not referencing thin pool LV");

		if (seg->device_id > DM_THIN_MAX_DEVICE_ID)
			seg_error("has too large device id");

		if (seg->external_lv &&
		    !lv_is_external_origin(seg->external_lv))
			seg_error("external LV is not flagged as a external origin LV");

		if (seg->merge_lv) {
			if (!lv_is_thin_volume(seg->merge_lv))
				seg_error("merge LV is not flagged as a thin LV");

			if (!lv_is_merging_origin(seg->merge_lv))
				seg_error("merge LV is not flagged as merging");
		}
	} else { /* !thin */
		if (seg->device_id)
			seg_error("sets device_id");
		if (seg->external_lv)
			seg_error("sets external LV");
		if (seg->merge_lv)
			seg_error("sets merge LV");
		if (seg->indirect_origin)
			seg_error("sets indirect_origin LV");
	}

	if (seg_is_vdo_pool(seg)) {
		if (!lv_is_vdo_pool(lv))
			seg_error("is not flagged as VDO pool LV");
		if ((seg->area_count != 1) || (seg_type(seg, 0) != AREA_LV)) {
			seg_error("is missing a VDO pool data LV");
		} else if (!lv_is_vdo_pool_data(seg_lv(seg, 0)))
			seg_error("is not VDO pool data LV");
	} else { /* !VDO pool */
		if (seg->vdo_pool_header_size)
			seg_error("sets vdo_pool_header_size");
		if (seg->vdo_pool_virtual_extents)
			seg_error("sets vdo_pool_virtual_extents");
	}

	if (seg_is_vdo(seg)) {
		if (!lv_is_vdo(lv))
			seg_error("is not flagged as VDO LV");
		if (!seg_lv(seg, 0))
			seg_error("is missing VDO pool LV");
		else if (!lv_is_vdo_pool(seg_lv(seg, 0)))
			seg_error("is not referencing VDO pool LV");
	}

	/* Some multi-seg vars excluded here */
	if (!seg_is_cache(seg) &&
	    !seg_is_thin_volume(seg)) {
		if (seg->pool_lv)
			seg_error("sets pool LV");
	}

	if (!seg_is_pool(seg) &&
	    /* FIXME: format_pool/import_export.c  _add_linear_seg() sets chunk_size */
	    !seg_is_linear(seg) &&
	    !seg_is_snapshot(seg)) {
		if (seg->chunk_size)
			seg_error("sets chunk_size");
	}

	if (!seg_is_thin_pool(seg) &&
	    !seg_is_thin_volume(seg)) {
		if (seg->transaction_id)
			seg_error("sets transaction_id");
	}

	if (!seg_unknown(seg)) {
		if (seg->segtype_private)
			seg_error("set segtype_private");
	}
}

/*
 * Verify that an LV's segments are consecutive, complete and don't overlap.
 */
int check_lv_segments(struct logical_volume *lv, int complete_vg)
{
	struct lv_segment *seg, *seg2;
	uint32_t le = 0;
	unsigned seg_count = 0, seg_found, external_lv_found = 0;
	uint32_t data_rimage_count, s;
	struct seg_list *sl;
	struct glv_list *glvl;
	int error_count = 0;

	dm_list_iterate_items(seg, &lv->segments) {
		seg_count++;

		if (seg->lv != lv) {
			log_error("LV %s invalid: segment %u is referencing different LV.",
				  lv->name, seg_count);
			inc_error_count;
		}

		if (seg->le != le) {
			log_error("LV %s invalid: segment %u should begin at "
				  "LE %" PRIu32 " (found %" PRIu32 ").",
				  lv->name, seg_count, le, seg->le);
			inc_error_count;
		}

		data_rimage_count = seg->area_count - seg->segtype->parity_devs;
		/* FIXME: raid varies seg->area_len? */
		if (seg->len != seg->area_len &&
		    seg->len != seg->area_len * data_rimage_count) {
			log_error("LV %s: segment %u with len=%u "
				  " has inconsistent area_len %u",
				  lv->name, seg_count, seg->len, seg->area_len);
			inc_error_count;
		}

		if (seg_is_snapshot(seg)) {
			if (seg->cow && seg->cow == seg->origin) {
				log_error("LV %s: segment %u has same LV %s for "
					  "both origin and snapshot",
					  lv->name, seg_count, seg->cow->name);
				inc_error_count;
			}
		}

		if (complete_vg)
			_check_lv_segment(lv, seg, seg_count, &error_count);

		for (s = 0; s < seg->area_count; s++) {
			if (seg_type(seg, s) == AREA_UNASSIGNED) {
				log_error("LV %s: segment %u has unassigned "
					  "area %u.",
					  lv->name, seg_count, s);
				inc_error_count;
			} else if (seg_type(seg, s) == AREA_PV) {
				if (!seg_pvseg(seg, s) ||
				    seg_pvseg(seg, s)->lvseg != seg ||
				    seg_pvseg(seg, s)->lv_area != s) {
					log_error("LV %s: segment %u has "
						  "inconsistent PV area %u",
						  lv->name, seg_count, s);
					inc_error_count;
				}
			} else {
				if (!seg_lv(seg, s) ||
				    seg_lv(seg, s)->vg != lv->vg ||
				    seg_lv(seg, s) == lv) {
					log_error("LV %s: segment %u has "
						  "inconsistent LV area %u",
						  lv->name, seg_count, s);
					inc_error_count;
				}

				if (complete_vg && seg_lv(seg, s) &&
				    lv_is_mirror_image(seg_lv(seg, s)) &&
				    (!(seg2 = find_seg_by_le(seg_lv(seg, s),
							    seg_le(seg, s))) ||
				     find_mirror_seg(seg2) != seg)) {
					log_error("LV %s: segment %u mirror "
						  "image %u missing mirror ptr",
						  lv->name, seg_count, s);
					inc_error_count;
				}

/* FIXME I don't think this ever holds?
				if (seg_le(seg, s) != le) {
					log_error("LV %s: segment %u has "
						  "inconsistent LV area %u "
						  "size",
						  lv->name, seg_count, s);
					inc_error_count;
				}
 */
				seg_found = 0;
				dm_list_iterate_items(sl, &seg_lv(seg, s)->segs_using_this_lv)
					if (sl->seg == seg)
						seg_found++;

				if (!seg_found) {
					log_error("LV %s segment %u uses LV %s,"
						  " but missing ptr from %s to %s",
						  lv->name, seg_count,
						  seg_lv(seg, s)->name,
						  seg_lv(seg, s)->name, lv->name);
					inc_error_count;
				} else if (seg_found > 1) {
					log_error("LV %s has duplicated links "
						  "to LV %s segment %u",
						  seg_lv(seg, s)->name,
						  lv->name, seg_count);
					inc_error_count;
				}
			}

			if (complete_vg &&
			    seg_is_mirrored(seg) && !seg_is_raid(seg) &&
			    seg_type(seg, s) == AREA_LV &&
			    seg_lv(seg, s)->le_count != seg->area_len) {
				log_error("LV %s: mirrored LV segment %u has "
					  "wrong size %u (should be %u).",
					  lv->name, s, seg_lv(seg, s)->le_count,
					  seg->area_len);
				inc_error_count;
			}
		}

		le += seg->len;
	}

	if (le != lv->le_count) {
		log_error("LV %s: inconsistent LE count %u != %u",
			  lv->name, le, lv->le_count);
		inc_error_count;
	}

	if (!le) {
		log_error("LV %s: has no segment.", lv->name);
		inc_error_count;
	}

	dm_list_iterate_items(sl, &lv->segs_using_this_lv) {
		seg = sl->seg;
		seg_found = 0;
		for (s = 0; s < seg->area_count; s++) {
			if (seg_type(seg, s) != AREA_LV)
				continue;
			if (lv == seg_lv(seg, s))
				seg_found++;
			if (seg->meta_areas && seg_is_raid_with_meta(seg) && (lv == seg_metalv(seg, s)))
				seg_found++;
		}
		if (seg->log_lv == lv)
			seg_found++;
		if (seg->metadata_lv == lv || seg->pool_lv == lv)
			seg_found++;
		if (seg_is_thin_volume(seg) && (seg->origin == lv || seg->external_lv == lv))
			seg_found++;

		if (!seg_found) {
			log_error("LV %s is used by LV %s:%" PRIu32 "-%" PRIu32
				  ", but missing ptr from %s to %s",
				  lv->name, seg->lv->name, seg->le,
				  seg->le + seg->len - 1,
				  seg->lv->name, lv->name);
			inc_error_count;
		} else if (seg_found != sl->count) {
			log_error("Reference count mismatch: LV %s has %u "
				  "links to LV %s:%" PRIu32 "-%" PRIu32
				  ", which has %u links",
				  lv->name, sl->count, seg->lv->name, seg->le,
				  seg->le + seg->len - 1, seg_found);
			inc_error_count;
		}

		seg_found = 0;
		dm_list_iterate_items(seg2, &seg->lv->segments)
			if (seg == seg2) {
				seg_found++;
				break;
			}

		if (!seg_found) {
			log_error("LV segment %s:%" PRIu32 "-%" PRIu32
				  " is incorrectly listed as being used by LV %s",
				  seg->lv->name, seg->le, seg->le + seg->len - 1,
				  lv->name);
			inc_error_count;
		}

		/* Validation of external origin counter */
		if (seg->external_lv == lv)
			external_lv_found++;
	}

	dm_list_iterate_items(glvl, &lv->indirect_glvs) {
		if (glvl->glv->is_historical) {
			if (glvl->glv->historical->indirect_origin != lv->this_glv) {
				log_error("LV %s is indirectly used by historical LV %s"
					  "but that historical LV does not point back to LV %s",
					   lv->name, glvl->glv->historical->name, lv->name);
				inc_error_count;
			}
		} else {
			if (!(seg = first_seg(glvl->glv->live)) ||
			    seg->indirect_origin != lv->this_glv) {
				log_error("LV %s is indirectly used by LV %s"
					  "but that LV does not point back to LV %s",
					  lv->name, glvl->glv->live->name, lv->name);
				inc_error_count;
			}
		}
	}

	/* Check LV flags match first segment type */
	if (complete_vg) {
		if ((seg_count != 1) &&
		    (lv_is_cache(lv) ||
		     lv_is_cache_pool(lv) ||
		     lv_is_raid(lv) ||
		     lv_is_snapshot(lv) ||
		     lv_is_thin_pool(lv) ||
		     lv_is_thin_volume(lv))) {
			log_error("LV %s must have exactly one segment.",
				  lv->name);
			inc_error_count;
		}

		if (lv_is_pool_data(lv) &&
		    (!(seg2 = first_seg(lv)) || !(seg2 = find_pool_seg(seg2)) ||
		     seg2->area_count != 1 || seg_type(seg2, 0) != AREA_LV ||
		     seg_lv(seg2, 0) != lv)) {
			log_error("LV %s: segment 1 pool data LV does not point back to same LV",
				  lv->name);
			inc_error_count;
		}

		if (lv_is_thin_pool_metadata(lv) && !strstr(lv->name, "_tmeta")) {
			log_error("LV %s: thin pool metadata LV does not use _tmeta.",
				  lv->name);
			inc_error_count;
		} else if (lv_is_cache_pool_metadata(lv) && !strstr(lv->name, "_cmeta")) {
			log_error("LV %s: cache pool metadata LV does not use _cmeta.",
				  lv->name);
			inc_error_count;
		}

		if (lv_is_external_origin(lv)) {
			if (lv->external_count != external_lv_found) {
				log_error("LV %s: external origin count does not match.",
					  lv->name);
				inc_error_count;
			}
			if (lv->status & LVM_WRITE) {
				log_error("LV %s: external origin cant't be writable.",
					  lv->name);
				inc_error_count;
			}
		}
	}

out:
	return !error_count;
}

/*
 * Split the supplied segment at the supplied logical extent
 * NB Use LE numbering that works across stripes PV1: 0,2,4 PV2: 1,3,5 etc.
 */
static int _lv_split_segment(struct logical_volume *lv, struct lv_segment *seg,
			     uint32_t le)
{
	struct lv_segment *split_seg;
	uint32_t s;
	uint32_t offset = le - seg->le;
	uint32_t area_offset;

	if (!seg_can_split(seg)) {
		log_error("Unable to split the %s segment at LE %" PRIu32
			  " in LV %s", lvseg_name(seg), le, lv->name);
		return 0;
	}

	/* Clone the existing segment */
	if (!(split_seg = alloc_lv_segment(seg->segtype,
					   seg->lv, seg->le, seg->len, seg->reshape_len,
					   seg->status, seg->stripe_size,
					   seg->log_lv,
					   seg->area_count, seg->area_len, seg->data_copies,
					   seg->chunk_size, seg->region_size,
					   seg->extents_copied, seg->pvmove_source_seg))) {
		log_error("Couldn't allocate cloned LV segment.");
		return 0;
	}

	if (!str_list_dup(lv->vg->vgmem, &split_seg->tags, &seg->tags)) {
		log_error("LV segment tags duplication failed");
		return 0;
	}

	/* In case of a striped segment, the offset has to be / stripes */
	area_offset = offset;
	if (seg_is_striped(seg))
		area_offset /= seg->area_count;

	split_seg->area_len -= area_offset;
	seg->area_len = area_offset;

	split_seg->len -= offset;
	seg->len = offset;

	split_seg->le = seg->le + seg->len;

	/* Adjust the PV mapping */
	for (s = 0; s < seg->area_count; s++) {
		seg_type(split_seg, s) = seg_type(seg, s);

		/* Split area at the offset */
		switch (seg_type(seg, s)) {
		case AREA_LV:
			if (!set_lv_segment_area_lv(split_seg, s, seg_lv(seg, s),
						    seg_le(seg, s) + seg->area_len, 0))
				return_0;
			log_debug_alloc("Split %s:%u[%u] at %u: %s LE %u", lv->name,
					seg->le, s, le, seg_lv(seg, s)->name,
					seg_le(split_seg, s));
			break;

		case AREA_PV:
			if (!(seg_pvseg(split_seg, s) =
			     assign_peg_to_lvseg(seg_pv(seg, s),
						 seg_pe(seg, s) +
						     seg->area_len,
						 seg_pvseg(seg, s)->len -
						     seg->area_len,
						 split_seg, s)))
				return_0;
			log_debug_alloc("Split %s:%u[%u] at %u: %s PE %u", lv->name,
					seg->le, s, le,
					dev_name(seg_dev(seg, s)),
					seg_pe(split_seg, s));
			break;

		case AREA_UNASSIGNED:
			log_error("Unassigned area %u found in segment", s);
			return 0;
		}
	}

	/* Add split off segment to the list _after_ the original one */
	dm_list_add_h(&seg->list, &split_seg->list);

	return 1;
}

/*
 * Ensure there's a segment boundary at the given logical extent
 */
int lv_split_segment(struct logical_volume *lv, uint32_t le)
{
	struct lv_segment *seg;

	if (!(seg = find_seg_by_le(lv, le))) {
		log_error("Segment with extent %" PRIu32 " in LV %s not found",
			  le, lv->name);
		return 0;
	}

	/* This is a segment start already */
	if (le == seg->le)
		return 1;

	if (!_lv_split_segment(lv, seg, le))
		return_0;

	if (!vg_validate(lv->vg))
		return_0;

	return 1;
}
