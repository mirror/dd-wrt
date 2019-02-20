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

#include "lib/misc/lib.h"
#include "pv_map.h"

#include <assert.h>

/*
 * Areas are maintained in size order, largest first.
 *
 * FIXME Cope with overlap.
 */
static void _insert_area(struct dm_list *head, struct pv_area *a, unsigned reduced)
{
	struct pv_area *pva;
	uint32_t count = reduced ? a->unreserved : a->count;
		
	dm_list_iterate_items(pva, head)
		if (count > pva->count)
			break;

	dm_list_add(&pva->list, &a->list);
	a->map->pe_count += a->count;
}

static void _remove_area(struct pv_area *a)
{
	dm_list_del(&a->list);
	a->map->pe_count -= a->count;
}

static int _create_single_area(struct dm_pool *mem, struct pv_map *pvm,
			       uint32_t start, uint32_t length)
{
	struct pv_area *pva;

	if (!(pva = dm_pool_zalloc(mem, sizeof(*pva))))
		return_0;

	log_debug_alloc("Allowing allocation on %s start PE %" PRIu32 " length %"
			PRIu32, pv_dev_name(pvm->pv), start, length);
	pva->map = pvm;
	pva->start = start;
	pva->count = length;
	pva->unreserved = pva->count;
	_insert_area(&pvm->areas, pva, 0);

	return 1;
}

static int _create_alloc_areas_for_pv(struct dm_pool *mem, struct pv_map *pvm,
				      uint32_t start, uint32_t count)
{
	struct pv_segment *peg;
	uint32_t pe, end, area_len;

	/* Only select extents from start to end inclusive */
	end = start + count - 1;
	if (end > pvm->pv->pe_count - 1)
		end = pvm->pv->pe_count - 1;

	pe = start;

	/* Walk through complete ordered list of device segments */
	dm_list_iterate_items(peg, &pvm->pv->segments) {
		/* pe holds the next extent we want to check */

		/* Beyond the range we're interested in? */
		if (pe > end)
			break;

		/* Skip if we haven't reached the first seg we want yet */
		if (pe > peg->pe + peg->len - 1)
			continue;

		/* Free? */
		if (peg->lvseg)
			goto next;

		/* How much of this peg do we need? */
		area_len = (end >= peg->pe + peg->len - 1) ?
			   peg->len - (pe - peg->pe) : end - pe + 1;

		if (!_create_single_area(mem, pvm, pe, area_len))
			return_0;

      next:
		pe = peg->pe + peg->len;
	}

	return 1;
}

static int _create_all_areas_for_pv(struct dm_pool *mem, struct pv_map *pvm,
				    struct dm_list *pe_ranges)
{
	struct pe_range *aa;

	if (!pe_ranges) {
		/* Use whole PV */
		if (!_create_alloc_areas_for_pv(mem, pvm, UINT32_C(0),
						pvm->pv->pe_count))
			return_0;

		return 1;
	}

	dm_list_iterate_items(aa, pe_ranges) {
		if (!_create_alloc_areas_for_pv(mem, pvm, aa->start,
						aa->count))
			return_0;
	}

	return 1;
}

static int _create_maps(struct dm_pool *mem, struct dm_list *pvs, struct dm_list *pvms)
{
	struct pv_map *pvm, *pvm2;
	struct pv_list *pvl;

	dm_list_iterate_items(pvl, pvs) {
		if (!(pvl->pv->status & ALLOCATABLE_PV) ||
		    (pvl->pv->status & PV_ALLOCATION_PROHIBITED)) {
		    	pvl->pv->status &= ~PV_ALLOCATION_PROHIBITED;
			continue;
		}
		if (is_missing_pv(pvl->pv))
			continue;
		assert(pvl->pv->dev);

		pvm = NULL;

		dm_list_iterate_items(pvm2, pvms)
			if (pvm2->pv->dev == pvl->pv->dev) {
				pvm = pvm2;
				break;
			}

		if (!pvm) {
			if (!(pvm = dm_pool_zalloc(mem, sizeof(*pvm))))
				return_0;

			pvm->pv = pvl->pv;
			dm_list_init(&pvm->areas);
			dm_list_add(pvms, &pvm->list);
		}

		if (!_create_all_areas_for_pv(mem, pvm, pvl->pe_ranges))
			return_0;
	}

	return 1;
}

/*
 * Create list of PV areas available for this particular allocation
 */
struct dm_list *create_pv_maps(struct dm_pool *mem, struct volume_group *vg,
			    struct dm_list *allocatable_pvs)
{
	struct dm_list *pvms;

	if (!(pvms = dm_pool_zalloc(mem, sizeof(*pvms)))) {
		log_error("create_pv_maps alloc failed");
		return NULL;
	}

	dm_list_init(pvms);

	if (!_create_maps(mem, allocatable_pvs, pvms)) {
		log_error("Couldn't create physical volume maps in %s",
			  vg->name);
		dm_pool_free(mem, pvms);
		return NULL;
	}

	return pvms;
}

void consume_pv_area(struct pv_area *pva, uint32_t to_go)
{
	_remove_area(pva);

	assert(to_go <= pva->count);

	if (to_go < pva->count) {
		/* split the area */
		pva->start += to_go;
		pva->count -= to_go;
		pva->unreserved = pva->count;
		_insert_area(&pva->map->areas, pva, 0);
	}
}

/*
 * Remove an area from list and reinsert it based on its new size
 * after a provisional allocation (or reverting one).
 */
void reinsert_changed_pv_area(struct pv_area *pva)
{
	_remove_area(pva);
	_insert_area(&pva->map->areas, pva, 1);
}

uint32_t pv_maps_size(struct dm_list *pvms)
{
	struct pv_map *pvm;
	uint32_t pe_count = 0;

	dm_list_iterate_items(pvm, pvms)
		pe_count += pvm->pe_count;

	return pe_count;
}
