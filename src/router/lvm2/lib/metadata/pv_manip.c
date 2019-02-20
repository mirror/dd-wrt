/*
 * Copyright (C) 2003 Sistina Software, Inc. All rights reserved.
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
#include "lib/metadata/pv_alloc.h"
#include "lib/commands/toolcontext.h"
#include "lib/locking/locking.h"
#include "lib/config/defaults.h"
#include "lib/display/display.h"
#include "lib/format_text/archiver.h"

static struct pv_segment *_alloc_pv_segment(struct dm_pool *mem,
					    struct physical_volume *pv,
					    uint32_t pe, uint32_t len,
					    struct lv_segment *lvseg,
					    uint32_t lv_area)
{
	struct pv_segment *peg;

	if (!(peg = dm_pool_zalloc(mem, sizeof(*peg)))) {
		log_error("pv_segment allocation failed");
		return NULL;
	}

	peg->pv = pv;
	peg->pe = pe;
	peg->len = len;
	peg->lvseg = lvseg;
	peg->lv_area = lv_area;

	dm_list_init(&peg->list);

	return peg;
}

int alloc_pv_segment_whole_pv(struct dm_pool *mem, struct physical_volume *pv)
{
	struct pv_segment *peg;

	if (!pv->pe_count)
		return 1;

	/* FIXME Cope with holes in PVs */
	if (!(peg = _alloc_pv_segment(mem, pv, 0, pv->pe_count, NULL, 0)))
		return_0;

	dm_list_add(&pv->segments, &peg->list);

	return 1;
}

int peg_dup(struct dm_pool *mem, struct dm_list *peg_new, struct dm_list *peg_old)
{
	struct pv_segment *peg, *pego;

	dm_list_init(peg_new);

	dm_list_iterate_items(pego, peg_old) {
		if (!(peg = _alloc_pv_segment(mem, pego->pv, pego->pe,
					      pego->len, pego->lvseg,
					      pego->lv_area)))
			return_0;
		dm_list_add(peg_new, &peg->list);
	}

	return 1;
}

/* Find segment at a given physical extent in a PV */
static struct pv_segment *_find_peg_by_pe(const struct physical_volume *pv,
					  uint32_t pe)
{
	struct pv_segment *pvseg;

	/* search backwards to optimise mostly used last segment split */
	dm_list_iterate_back_items(pvseg, &pv->segments)
		if (pe >= pvseg->pe && pe < pvseg->pe + pvseg->len)
			return pvseg;

	return NULL;
}

/*
 * Split peg at given extent.
 * Second part is always not allocated to a LV and returned.
 */
static struct pv_segment *_pv_split_segment(struct dm_pool *mem,
					    struct physical_volume *pv,
					    struct pv_segment *peg,
					    uint32_t pe)
{
	struct pv_segment *peg_new;

	if (!(peg_new = _alloc_pv_segment(mem, peg->pv, pe,
					  peg->len + peg->pe - pe,
					  NULL, 0)))
		return_NULL;

	peg->len = peg->len - peg_new->len;

	dm_list_add_h(&peg->list, &peg_new->list);

	if (peg->lvseg) {
		peg->pv->pe_alloc_count -= peg_new->len;
		peg->lvseg->lv->vg->free_count += peg_new->len;
	}

	return peg_new;
}

/*
 * Ensure there is a PV segment boundary at the given extent.
 */
int pv_split_segment(struct dm_pool *mem,
		     struct physical_volume *pv, uint32_t pe,
		     struct pv_segment **pvseg_allocated)
{
	struct pv_segment *pvseg, *pvseg_new = NULL;

	if (pe == pv->pe_count)
		goto out;

	if (!(pvseg = _find_peg_by_pe(pv, pe))) {
		log_error("Segment with extent %" PRIu32 " in PV %s not found",
			  pe, pv_dev_name(pv));
		return 0;
	}

	/* This is a peg start already */
	if (pe == pvseg->pe) {
		pvseg_new = pvseg;
		goto out;
	}

	if (!(pvseg_new = _pv_split_segment(mem, pv, pvseg, pe)))
		return_0;
out:
	if (pvseg_allocated)
		*pvseg_allocated = pvseg_new;

	return 1;
}

static struct pv_segment _null_pv_segment = {
	.pv = NULL,
	.pe = 0,
};

struct pv_segment *assign_peg_to_lvseg(struct physical_volume *pv,
				       uint32_t pe, uint32_t area_len,
				       struct lv_segment *seg,
				       uint32_t area_num)
{
	struct pv_segment *peg = NULL;

	/* Missing format1 PV */
	if (!pv)
		return &_null_pv_segment;

	if (!pv_split_segment(seg->lv->vg->vgmem, pv, pe, &peg) ||
	    !pv_split_segment(seg->lv->vg->vgmem, pv, pe + area_len, NULL))
		return_NULL;

	if (!peg) {
		log_error("Missing PV segment on %s at %u.",
			  pv_dev_name(pv), pe);
		return NULL;
	}

	peg->lvseg = seg;
	peg->lv_area = area_num;

	peg->pv->pe_alloc_count += area_len;
	peg->lvseg->lv->vg->free_count -= area_len;

	return peg;
}

int discard_pv_segment(struct pv_segment *peg, uint32_t discard_area_reduction)
{
	uint64_t discard_offset_sectors;
	uint64_t pe_start = peg->pv->pe_start;
	char uuid[64] __attribute__((aligned(8)));

	if (!peg->lvseg) {
		log_error("discard_pv_segment with unallocated segment: "
			  "%s PE %" PRIu32, pv_dev_name(peg->pv), peg->pe);
		return 0;
	}

	/*
	 * Only issue discards if enabled in lvm.conf and both
	 * the device and kernel (>= 2.6.35) supports discards.
	 */
	if (!find_config_tree_bool(peg->pv->fmt->cmd, devices_issue_discards_CFG, NULL))
		return 1;
 
	/* Missing PV? */
	if (is_missing_pv(peg->pv) || !peg->pv->dev) {
		if (!id_write_format(&peg->pv->id, uuid, sizeof(uuid)))
			return_0;

		log_verbose("Skipping discard on missing device with uuid %s.", uuid);

		return 1;
	}

	if (!dev_discard_max_bytes(peg->pv->fmt->cmd->dev_types, peg->pv->dev) ||
	    !dev_discard_granularity(peg->pv->fmt->cmd->dev_types, peg->pv->dev))
		return 1;

	discard_offset_sectors = (peg->pe + peg->lvseg->area_len - discard_area_reduction) *
				 (uint64_t) peg->pv->vg->extent_size + pe_start;
	if (!discard_offset_sectors) {
		/*
		 * pe_start=0 and the PV's first extent contains the label.
		 * Must skip past the first extent.
		 */
		discard_offset_sectors = peg->pv->vg->extent_size;
		discard_area_reduction--;
	}

	log_debug_alloc("Discarding %" PRIu32 " extents offset %" PRIu64 " sectors on %s.",
			discard_area_reduction, discard_offset_sectors, dev_name(peg->pv->dev));
	if (discard_area_reduction &&
	    !dev_discard_blocks(peg->pv->dev, discard_offset_sectors << SECTOR_SHIFT,
				discard_area_reduction * (uint64_t) peg->pv->vg->extent_size * SECTOR_SIZE))
		return_0;

	return 1;
}

static int _merge_free_pv_segment(struct pv_segment *peg)
{
	struct dm_list *l;
	struct pv_segment *merge_peg;

	if (peg->lvseg) {
		log_error(INTERNAL_ERROR
			  "_merge_free_pv_seg called on a"
			  " segment that is not free.");
		return 0;
	}

	/*
	 * FIXME:
	 * Should we free the list element once it is deleted
	 * from the list?  I think not.  It is likely part of
	 * a mempool.
	 */
	/* Attempt to merge with Free space before */
	if ((l = dm_list_prev(&peg->pv->segments, &peg->list))) {
		merge_peg = dm_list_item(l, struct pv_segment);
		if (!merge_peg->lvseg) {
			merge_peg->len += peg->len;
			dm_list_del(&peg->list);
			peg = merge_peg;
		}
	}

	/* Attempt to merge with Free space after */
	if ((l = dm_list_next(&peg->pv->segments, &peg->list))) {
		merge_peg = dm_list_item(l, struct pv_segment);
		if (!merge_peg->lvseg) {
			peg->len += merge_peg->len;
			dm_list_del(&merge_peg->list);
		}
	}

	return 1;
}

/*
 * release_pv_segment
 * @peg
 * @area_reduction
 *
 * WARNING: When release_pv_segment is called, the freed space may be
 *          merged into the 'pv_segment's before and after it in the
 *          list if they are also free.  Thus, any iterators of the
 *          'pv->segments' list that call this function must be aware
 *          that the list can change in a way that is unsafe even for
 *          *_safe iterators.  Restart the iterator in these cases.
 *
 * Returns: 1 on success, 0 on failure
 */
int release_pv_segment(struct pv_segment *peg, uint32_t area_reduction)
{
	struct dm_list *l;
	struct pv_segment *merge_peg;

	if (!peg->lvseg) {
		log_error("release_pv_segment with unallocated segment: "
			  "%s PE %" PRIu32, pv_dev_name(peg->pv), peg->pe);
		return 0;
	}

	if (peg->lvseg->area_len == area_reduction) {
		peg->pv->pe_alloc_count -= area_reduction;
		peg->lvseg->lv->vg->free_count += area_reduction;

		peg->lvseg = NULL;
		peg->lv_area = 0;

		return _merge_free_pv_segment(peg);
	}

	if (!pv_split_segment(peg->lvseg->lv->vg->vgmem,
			      peg->pv, peg->pe + peg->lvseg->area_len -
			      area_reduction, NULL))
		return_0;

	/* The segment after 'peg' now holds free space, try to merge it */
	if ((l = dm_list_next(&peg->pv->segments, &peg->list))) {
		merge_peg = dm_list_item(l, struct pv_segment);
		return _merge_free_pv_segment(merge_peg);
	}

	return 1;
}

/*
 * Only for use by lv_segment merging routines.
 */
void merge_pv_segments(struct pv_segment *peg1, struct pv_segment *peg2)
{
	peg1->len += peg2->len;

	dm_list_del(&peg2->list);
}

/*
 * Calculate the overlap, in extents, between a struct pv_segment and
 * a struct pe_range.
 */
static uint32_t _overlap_pe(const struct pv_segment *pvseg,
			    const struct pe_range *per)
{
	uint32_t start;
	uint32_t end;

	start = max(pvseg->pe, per->start);
	end = min(pvseg->pe + pvseg->len, per->start + per->count);

	if (end < start)
		return 0;

	return end - start;
}

/*
 * Returns: number of free PEs in a struct pv_list
 */
uint32_t pv_list_extents_free(const struct dm_list *pvh)
{
	struct pv_list *pvl;
	struct pe_range *per;
	uint32_t extents = 0;
	struct pv_segment *pvseg;

	dm_list_iterate_items(pvl, pvh) {
		if (!pvl->pe_ranges) {
			log_warn(INTERNAL_ERROR "WARNING: PV %s is without initialized PE ranges.", dev_name(pvl->pv->dev));
			continue;
		}
		dm_list_iterate_items(per, pvl->pe_ranges) {
			dm_list_iterate_items(pvseg, &pvl->pv->segments) {
				if (!pvseg_is_allocated(pvseg))
					extents += _overlap_pe(pvseg, per);
			}
		}
	}

	return extents;
}

/*
 * Check all pv_segments in VG for consistency
 */
int check_pv_segments(struct volume_group *vg)
{
	struct physical_volume *pv;
	struct pv_list *pvl;
	struct pv_segment *peg;
	unsigned s, segno;
	uint32_t start_pe, alloced;
	uint32_t pv_count = 0, free_count = 0, extent_count = 0;
	int ret = 1;

	dm_list_iterate_items(pvl, &vg->pvs) {
		pv = pvl->pv;
		segno = 0;
		start_pe = 0;
		alloced = 0;
		pv_count++;

		dm_list_iterate_items(peg, &pv->segments) {
			s = peg->lv_area;

			/* FIXME Remove this next line eventually */
			log_debug_alloc("%s %u: %6u %6u: %s(%u:%u)",
					pv_dev_name(pv), segno++, peg->pe, peg->len,
					peg->lvseg ? peg->lvseg->lv->name : "NULL",
					peg->lvseg ? peg->lvseg->le : 0, s);
			/* FIXME Add details here on failure instead */
			if (start_pe != peg->pe) {
				log_error("Gap in pvsegs: %u, %u",
					  start_pe, peg->pe);
				ret = 0;
			}
			if (peg->lvseg) {
				if (seg_type(peg->lvseg, s) != AREA_PV) {
					log_error("Wrong lvseg area type");
					ret = 0;
				}
				if (seg_pvseg(peg->lvseg, s) != peg) {
					log_error("Inconsistent pvseg pointers");
					ret = 0;
				}
				if (peg->lvseg->area_len != peg->len) {
					log_error("Inconsistent length: %u %u",
						  peg->len,
						  peg->lvseg->area_len);
					ret = 0;
				}
				alloced += peg->len;
			}
			start_pe += peg->len;
		}

		if (start_pe != pv->pe_count) {
			log_error("PV segment pe_count mismatch: %u != %u",
				  start_pe, pv->pe_count);
			ret = 0;
		}

		if (alloced != pv->pe_alloc_count) {
			log_error("PV segment pe_alloc_count mismatch: "
				  "%u != %u", alloced, pv->pe_alloc_count);
			ret = 0;
		}

		extent_count += start_pe;
		free_count += (start_pe - alloced);
	}

	if (pv_count != vg->pv_count) {
		log_error("PV segment VG pv_count mismatch: %u != %u",
			  pv_count, vg->pv_count);
		ret = 0;
	}

	if (free_count != vg->free_count) {
		log_error("PV segment VG free_count mismatch: %u != %u",
			  free_count, vg->free_count);
		ret = 0;
	}

	if (extent_count != vg->extent_count) {
		log_error("PV segment VG extent_count mismatch: %u != %u",
			  extent_count, vg->extent_count);
		ret = 0;
	}

	return ret;
}

static int _reduce_pv(struct physical_volume *pv, struct volume_group *vg,
		      uint32_t old_pe_count, uint32_t new_pe_count)
{
	struct pv_segment *peg, *pegt;

	if (new_pe_count < pv->pe_alloc_count) {
		log_error("%s: cannot resize to %" PRIu32 " extents "
			  "as %" PRIu32 " are allocated.",
			  pv_dev_name(pv), new_pe_count,
			  pv->pe_alloc_count);
		return 0;
	}

	/* Check PEs to be removed are not already allocated */
	dm_list_iterate_items(peg, &pv->segments) {
 		if (peg->pe + peg->len <= new_pe_count)
			continue;

		if (peg->lvseg) {
			log_error("%s: cannot resize to %" PRIu32 " extents as "
				  "later ones are allocated.",
				  pv_dev_name(pv), new_pe_count);
			return 0;
		}
	}

	if (!pv_split_segment(vg->vgmem, pv, new_pe_count, NULL))
		return_0;

	dm_list_iterate_items_safe(peg, pegt, &pv->segments) {
 		if (peg->pe + peg->len > new_pe_count)
			dm_list_del(&peg->list);
	}

	pv->pe_count = new_pe_count;

	vg->extent_count -= (old_pe_count - new_pe_count);
	vg->free_count -= (old_pe_count - new_pe_count);

	return 1;
}

static int _extend_pv(struct physical_volume *pv, struct volume_group *vg,
		      uint32_t old_pe_count, uint32_t new_pe_count)
{
	struct pv_segment *peg;

	if ((uint64_t) new_pe_count * pv->pe_size > pv->size ) {
		log_error("%s: cannot resize to %" PRIu32 " extents as there "
			  "is only room for %" PRIu64 ".", pv_dev_name(pv),
			  new_pe_count, pv->size / pv->pe_size);
		return 0;
	}

	if (!(peg = _alloc_pv_segment(pv->fmt->cmd->mem, pv,
				      old_pe_count,
				      new_pe_count - old_pe_count,
				      NULL, 0)))
		return_0;

	dm_list_add(&pv->segments, &peg->list);

	pv->pe_count = new_pe_count;

	vg->extent_count += (new_pe_count - old_pe_count);
	vg->free_count += (new_pe_count - old_pe_count);

	return 1;
}

/*
 * Resize a PV in a VG, adding or removing segments as needed.
 * New size must fit within pv->size.
 */
static int _pv_resize(struct physical_volume *pv, struct volume_group *vg, uint64_t size)
{
	uint32_t old_pe_count, new_pe_count = 0;

	if (size < pv_min_size()) {
		log_error("Size must exceed minimum of %" PRIu64 " sectors on PV %s.",
			   pv_min_size(), pv_dev_name(pv));
		return 0;
	}

	if (size < pv_pe_start(pv)) {
		log_error("Size must exceed physical extent start "
			  "of %" PRIu64 " sectors on PV %s.",
			  pv_pe_start(pv), pv_dev_name(pv));
		return 0;
	}

	old_pe_count = pv->pe_count;

	if (!pv->fmt->ops->pv_resize(pv->fmt, pv, vg, size)) {
		log_error("Format specific resize of PV %s failed.",
			   pv_dev_name(pv));
		return 0;
	}

	/* pv->pe_count is 0 now! We need to recalculate! */

	/* If there's a VG, calculate new PE count value. */
	/* Don't do for orphan VG */
	if (vg && !is_orphan_vg(vg->name)) {
		/* FIXME: Maybe PE calculation should go into pv->fmt->resize?
		          (like it is for pv->fmt->setup) */
		if (!(new_pe_count = pv_size(pv) / vg->extent_size)) {
			log_error("Size must leave space for at least one physical "
				  "extent of %" PRIu32 " sectors on PV %s.",
				   pv_pe_size(pv), pv_dev_name(pv));
			return 0;
		}

		if (new_pe_count == old_pe_count) {
			pv->pe_count = old_pe_count;
			log_verbose("No change to size of physical volume %s.",
				    pv_dev_name(pv));
			return 1;
		}

		log_verbose("Resizing physical volume %s from %" PRIu32
			    " to %" PRIu32 " extents.",
			    pv_dev_name(pv), old_pe_count, new_pe_count);

		if (new_pe_count > old_pe_count)
			return _extend_pv(pv, vg, old_pe_count, new_pe_count);

		return _reduce_pv(pv, vg, old_pe_count, new_pe_count);
	}

	return 1;
}

int pv_resize_single(struct cmd_context *cmd,
		     struct volume_group *vg,
		     struct physical_volume *pv,
		     const uint64_t new_size,
		     int yes)
{
	uint64_t size = 0;
	int r = 0;
	const char *pv_name = pv_dev_name(pv);
	const char *vg_name = pv->vg_name;
	int vg_needs_pv_write = 0;

	if (!archive(vg))
		goto out;

	if (!(pv->fmt->features & FMT_RESIZE_PV)) {
		log_error("Physical volume %s format does not support resizing.",
			  pv_name);
		goto out;
	}

	/* Get new size */
	if (!dev_get_size(pv_dev(pv), &size)) {
		log_error("%s: Couldn't get size.", pv_name);
		goto out;
	}

	if (new_size) {
		if (new_size > size) {
			log_warn("WARNING: %s: Overriding real size %s. You could lose data.",
				 pv_name, display_size(cmd, (uint64_t) size));
			if (!yes && yes_no_prompt("%s: Requested size %s exceeds real size %s. Proceed?  [y/n]: ",
						  pv_name, display_size(cmd, new_size),
						  display_size(cmd, size)) == 'n') {
				log_error("Physical Volume %s not resized.", pv_name);
				goto out;
			}

		}  else if (new_size < size)
			if (!yes && yes_no_prompt("%s: Requested size %s is less than real size %s. Proceed?  [y/n]: ",
						  pv_name, display_size(cmd, new_size),
						  display_size(cmd, size)) == 'n') {
				log_error("Physical Volume %s not resized.", pv_name);
				goto out;
			}

		if (new_size == size)
			log_verbose("%s: Size is already %s (%" PRIu64 " sectors).",
				    pv_name, display_size(cmd, new_size), new_size);
		else
			log_warn("WARNING: %s: Pretending size is %" PRIu64 " not %" PRIu64 " sectors.",
				 pv_name, new_size, size);
		size = new_size;
	}

	log_verbose("Resizing volume \"%s\" to %" PRIu64 " sectors.",
		    pv_name, size);

	if (!_pv_resize(pv, vg, size))
		goto_out;

	log_verbose("Updating physical volume \"%s\"", pv_name);

	/* Write PV label only if this an orphan PV or it has 2nd mda. */
	if ((is_orphan_vg(vg_name) ||
	     (vg_needs_pv_write = (fid_get_mda_indexed(vg->fid,
			(const char *) &pv->id, ID_LEN, 1) != NULL))) &&
	    !pv_write(cmd, pv, 1)) {
		log_error("Failed to store physical volume \"%s\"",
			  pv_name);
		goto out;
	}

	if (!is_orphan_vg(vg_name)) {
		if (!vg_write(vg) || !vg_commit(vg)) {
			log_error("Failed to store physical volume \"%s\" in "
				  "volume group \"%s\"", pv_name, vg_name);
			goto out;
		}
		backup(vg);
	}

	log_print_unless_silent("Physical volume \"%s\" changed", pv_name);
	r = 1;

out:
	if (!r && vg_needs_pv_write)
		log_error("Use pvcreate and vgcfgrestore "
			  "to repair from archived metadata.");
	return r;
}
