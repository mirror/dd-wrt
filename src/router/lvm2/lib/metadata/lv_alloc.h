/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.  
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

#ifndef _LVM_LV_ALLOC_H
#define _LVM_LV_ALLOC_H

#include "lib/metadata/metadata-exported.h"

struct lv_segment *alloc_lv_segment(const struct segment_type *segtype,
				    struct logical_volume *lv,
				    uint32_t le, uint32_t len,
				    uint32_t reshape_len,
				    uint64_t status,
				    uint32_t stripe_size,
				    struct logical_volume *log_lv,
				    uint32_t area_count,
				    uint32_t area_len,
				    uint32_t data_copies,
				    uint32_t chunk_size,
				    uint32_t region_size,
				    uint32_t extents_copied,
				    struct lv_segment *pvmove_source_seg);

int set_lv_segment_area_pv(struct lv_segment *seg, uint32_t area_num,
			   struct physical_volume *pv, uint32_t pe);
int set_lv_segment_area_lv(struct lv_segment *seg, uint32_t area_num,
			   struct logical_volume *lv, uint32_t le,
			   uint64_t status);
int move_lv_segment_area(struct lv_segment *seg_to, uint32_t area_to,
			 struct lv_segment *seg_from, uint32_t area_from);
int release_lv_segment_area(struct lv_segment *seg, uint32_t s,
			    uint32_t area_reduction);
int release_and_discard_lv_segment_area(struct lv_segment *seg, uint32_t s, uint32_t area_reduction);

struct alloc_handle;
struct alloc_handle *allocate_extents(struct volume_group *vg,
				      struct logical_volume *lv,
				      const struct segment_type *segtype,
				      uint32_t stripes,
				      uint32_t mirrors, uint32_t log_count,
				      uint32_t region_size, uint32_t extents,
				      struct dm_list *allocatable_pvs,
				      alloc_policy_t alloc, int approx_alloc,
				      struct dm_list *parallel_areas);

int lv_add_segment(struct alloc_handle *ah,
		   uint32_t first_area, uint32_t num_areas,
		   struct logical_volume *lv,
		   const struct segment_type *segtype,
		   uint32_t stripe_size,
		   uint64_t status,
		   uint32_t region_size);

int lv_add_mirror_areas(struct alloc_handle *ah,
			struct logical_volume *lv, uint32_t le,
			uint32_t region_size);
int lv_add_segmented_mirror_image(struct alloc_handle *ah,
				  struct logical_volume *lv, uint32_t le,
				  uint32_t region_size);
int lv_add_mirror_lvs(struct logical_volume *lv,
		      struct logical_volume **sub_lvs,
		      uint32_t num_extra_areas,
		      uint64_t status, uint32_t region_size);

int lv_add_log_segment(struct alloc_handle *ah, uint32_t first_area,
		       struct logical_volume *log_lv, uint64_t status);
int lv_add_virtual_segment(struct logical_volume *lv, uint64_t status,
			   uint32_t extents, const struct segment_type *segtype);

void alloc_destroy(struct alloc_handle *ah);

struct dm_list *build_parallel_areas_from_lv(struct logical_volume *lv,
					     unsigned use_pvmove_parent_lv,
					     unsigned create_single_list);

#endif
