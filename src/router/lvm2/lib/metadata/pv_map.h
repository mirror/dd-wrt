/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
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

#ifndef _LVM_PV_MAP_H
#define _LVM_PV_MAP_H

#include "lib/metadata/metadata.h"

/*
 * The in core rep. only stores a mapping from
 * logical extents to physical extents against an
 * lv.  Sometimes, when allocating a new lv for
 * instance, it is useful to have the inverse
 * mapping available.
 */

struct pv_area {
	struct pv_map *map;
	uint32_t start;
	uint32_t count;

	/* Number of extents unreserved during a single allocation pass. */
	uint32_t unreserved;

	struct dm_list list;		/* pv_map.areas */
};

/*
 * When building up a potential group of "parallel" extent ranges during
 * an allocation attempt, track the maximum number of extents that may
 * need to be used as a particular parallel area.  Several of these
 * structs may reference the same pv_area, but 'used' may differ between
 * them.  The sum of all the 'used' variables referring to the same
 * pv_area may not exceed that area's count, so we cannot allocate the
 * same extents twice.
 */
struct pv_area_used {
	struct pv_area *pva;
	uint32_t used;
};

struct pv_map {
	struct physical_volume *pv;
	struct dm_list areas;		/* struct pv_areas */
	uint32_t pe_count;		/* Total number of PEs */

	struct dm_list list;
};

/*
 * Find intersection between available_pvs and free space in VG
 */
struct dm_list *create_pv_maps(struct dm_pool *mem, struct volume_group *vg,
			    struct dm_list *allocatable_pvs);

void consume_pv_area(struct pv_area *pva, uint32_t to_go);
void reinsert_changed_pv_area(struct pv_area *pva);

uint32_t pv_maps_size(struct dm_list *pvms);

#endif
