/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2010 Red Hat, Inc. All rights reserved.
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
#include "lib/cache/lvmcache.h"

/*
 * FIXME: Check for valid handle before dereferencing field or log error?
 */
#define pv_field(handle, field)	((handle)->field)

char *pv_fmt_dup(const struct physical_volume *pv)
{
	if (!pv->fmt)
		return NULL;
	return dm_pool_strdup(pv->vg->vgmem, pv->fmt->name);
}

char *pv_name_dup(struct dm_pool *mem, const struct physical_volume *pv)
{
	return dm_pool_strdup(mem ? mem : pv->vg->vgmem, dev_name(pv->dev));
}

/*
 * Gets/Sets for external LVM library
 */
struct id pv_id(const struct physical_volume *pv)
{
	return pv_field(pv, id);
}

char *pv_uuid_dup(struct dm_pool *mem, const struct physical_volume *pv)
{
	return id_format_and_copy(mem ? mem : pv->vg->vgmem, &pv->id);
}

char *pv_tags_dup(const struct physical_volume *pv)
{
	return tags_format_and_copy(pv->vg->vgmem, &pv->tags);
}

const struct format_type *pv_format_type(const struct physical_volume *pv)
{
	return pv_field(pv, fmt);
}

struct id pv_vgid(const struct physical_volume *pv)
{
	return pv_field(pv, vgid);
}

struct device *pv_dev(const struct physical_volume *pv)
{
	return pv_field(pv, dev);
}

const char *pv_vg_name(const struct physical_volume *pv)
{
	/* Avoid exposing internal orphan names to users */
	return (!is_orphan(pv)) ? pv_field(pv, vg_name) : "";
}

const char *pv_dev_name(const struct physical_volume *pv)
{
	return dev_name(pv_dev(pv));
}

uint64_t pv_size(const struct physical_volume *pv)
{
	return pv_field(pv, size);
}

uint64_t pv_dev_size(const struct physical_volume *pv)
{
	uint64_t size;

	if (!dev_get_size(pv->dev, &size))
		size = 0;

	return size;
}

uint64_t pv_size_field(const struct physical_volume *pv)
{
	uint64_t size;

	if (!pv->pe_count)
		size = pv->size;
	else
		size = (uint64_t) pv->pe_count * pv->pe_size;

	return size;
}

uint64_t pv_free(const struct physical_volume *pv)
{
	uint64_t freespace;

	if (!pv->vg || is_orphan_vg(pv->vg->name))
		freespace = pv->size;
	else
		freespace = (uint64_t)
			(pv->pe_count - pv->pe_alloc_count) * pv->pe_size;

	return freespace;
}

uint64_t pv_status(const struct physical_volume *pv)
{
	return pv_field(pv, status);
}

uint32_t pv_pe_size(const struct physical_volume *pv)
{
	return pv_field(pv, pe_size);
}

uint64_t pv_ba_start(const struct physical_volume *pv)
{
	return pv_field(pv, ba_start);
}

uint64_t pv_ba_size(const struct physical_volume *pv)
{
	return pv_field(pv, ba_size);
}

uint64_t pv_pe_start(const struct physical_volume *pv)
{
	return pv_field(pv, pe_start);
}

uint32_t pv_pe_count(const struct physical_volume *pv)
{
	return pv_field(pv, pe_count);
}

uint32_t pv_pe_alloc_count(const struct physical_volume *pv)
{
	return pv_field(pv, pe_alloc_count);
}

uint32_t pv_mda_count(const struct physical_volume *pv)
{
	struct lvmcache_info *info;

	info = lvmcache_info_from_pvid((const char *)&pv->id.uuid, pv->dev, 0);

	return info ? lvmcache_mda_count(info) : UINT64_C(0);
}

static int _count_unignored(struct metadata_area *mda, void *baton)
{
	uint32_t *count = baton;

	if (!mda_is_ignored(mda))
		(*count) ++;

	return 1;
}

uint32_t pv_mda_used_count(const struct physical_volume *pv)
{
	struct lvmcache_info *info;
	uint32_t used_count=0;

	info = lvmcache_info_from_pvid((const char *)&pv->id.uuid, pv->dev, 0);
	if (!info)
		return 0;
	lvmcache_foreach_mda(info, _count_unignored, &used_count);

	return used_count;
}

/**
 * is_orphan - Determine whether a pv is an orphan based on its vg_name
 * @pv: handle to the physical volume
 */
int is_orphan(const struct physical_volume *pv)
{
	return is_orphan_vg(pv_field(pv, vg_name));
}

/**
 * is_pv - Determine whether a pv is a real pv or dummy one
 * @pv: handle to device
 */
int is_pv(const struct physical_volume *pv)
{
	return (pv_field(pv, vg_name) ? 1 : 0);
}

int is_missing_pv(const struct physical_volume *pv)
{
	return pv_field(pv, status) & MISSING_PV ? 1 : 0;
}

int is_used_pv(const struct physical_volume *pv)
{
	struct lvmcache_info *info;
	uint32_t ext_flags;

	if (!pv->fmt)
		return 0;

	if (!is_orphan(pv))
		return 1;

	if (!(pv->fmt->features & FMT_PV_FLAGS))
		return 0;

	if (!(info = lvmcache_info_from_pvid((const char *)&pv->id, pv->dev, 0))) {
		log_error("Failed to find cached info for PV %s.", pv_dev_name(pv));
		return -1;
	}

	ext_flags = lvmcache_ext_flags(info);

	return ext_flags & PV_EXT_USED ? 1 : 0;
}

char *pv_attr_dup(struct dm_pool *mem, const struct physical_volume *pv)
{
	char *repstr;
	int used = is_used_pv(pv);
	int duplicate = lvmcache_dev_is_unchosen_duplicate(pv->dev);

	if (!(repstr = dm_pool_zalloc(mem, 4))) {
		log_error("dm_pool_alloc failed");
		return NULL;
	}

	/*
	 * An allocatable PV is always used, so we don't need to show 'u'.
	 */
	if (duplicate)
		repstr[0] = 'd';
	else if (pv->status & ALLOCATABLE_PV)
		repstr[0] = 'a';
	else if (used > 0)
		repstr[0] = 'u';
	else
		repstr[0] = '-';

	repstr[1] = (pv->status & EXPORTED_VG) ? 'x' : '-';
	repstr[2] = (pv->status & MISSING_PV) ? 'm' : '-';

	return repstr;
}

uint64_t pv_mda_size(const struct physical_volume *pv)
{
	struct lvmcache_info *info;
	uint64_t min_mda_size = 0;
	const char *pvid = (const char *)(&pv->id.uuid);

	/* PVs could have 2 mdas of different sizes (rounding effect) */
	if ((info = lvmcache_info_from_pvid(pvid, pv->dev, 0)))
		min_mda_size = lvmcache_smallest_mda_size(info);
	return min_mda_size;
}

static int _pv_mda_free(struct metadata_area *mda, void *baton)
{
	uint64_t mda_free;
	uint64_t *freespace = baton;

	if (!mda->ops->mda_free_sectors)
		return 1;

	mda_free = mda->ops->mda_free_sectors(mda);
	if (mda_free < *freespace)
		*freespace = mda_free;

	return 1;
}

uint64_t lvmcache_info_mda_free(struct lvmcache_info *info)
{
	uint64_t freespace = UINT64_MAX;

	if (info)
		lvmcache_foreach_mda(info, _pv_mda_free, &freespace);

	if (freespace == UINT64_MAX)
		freespace = UINT64_C(0);

	return freespace;
}

uint64_t pv_mda_free(const struct physical_volume *pv)
{
	const char *pvid = (const char *)&pv->id.uuid;
	struct lvmcache_info *info;

	if ((info = lvmcache_info_from_pvid(pvid, pv->dev, 0)))
		return lvmcache_info_mda_free(info);

	return 0;
}

uint64_t pv_used(const struct physical_volume *pv)
{
	uint64_t used;

	if (!pv->pe_count)
		used = 0LL;
	else
		used = (uint64_t) pv->pe_alloc_count * pv->pe_size;

	return used;
}

struct _pv_mda_set_ignored_baton {
	unsigned mda_ignored;
	struct dm_list *mdas_in_use, *mdas_ignored, *mdas_to_change;
};

static int _pv_mda_set_ignored_one(struct metadata_area *mda, void *baton)
{
	struct _pv_mda_set_ignored_baton *b = baton;
	struct metadata_area *vg_mda, *tmda;

	if (mda_is_ignored(mda) && !b->mda_ignored) {
		/* Changing an ignored mda to one in_use requires moving it */
		dm_list_iterate_items_safe(vg_mda, tmda, b->mdas_ignored)
			if (mda_locns_match(mda, vg_mda)) {
				mda_set_ignored(vg_mda, b->mda_ignored);
				dm_list_move(b->mdas_in_use, &vg_mda->list);
			}
	}

	dm_list_iterate_items_safe(vg_mda, tmda, b->mdas_in_use)
		if (mda_locns_match(mda, vg_mda))
			/* Don't move mda: needs writing to disk. */
			mda_set_ignored(vg_mda, b->mda_ignored);

	mda_set_ignored(mda, b->mda_ignored);

	return 1;
}

unsigned pv_mda_set_ignored(const struct physical_volume *pv, unsigned mda_ignored)
{
	struct lvmcache_info *info;
	struct _pv_mda_set_ignored_baton baton;
	struct metadata_area *mda;

	if (!(info = lvmcache_info_from_pvid((const char *)&pv->id.uuid, pv->dev, 0)))
		return_0;

	baton.mda_ignored = mda_ignored;
	baton.mdas_in_use = &pv->fid->metadata_areas_in_use;
	baton.mdas_ignored = &pv->fid->metadata_areas_ignored;
	baton.mdas_to_change = baton.mda_ignored ? baton.mdas_in_use : baton.mdas_ignored;

	if (is_orphan(pv)) {
		dm_list_iterate_items(mda, baton.mdas_to_change)
			mda_set_ignored(mda, baton.mda_ignored);
		return 1;
	}

	/*
	 * Do not allow disabling of the the last PV in a VG.
	 */
	if (pv_mda_used_count(pv) == vg_mda_used_count(pv->vg)) {
		log_error("Cannot disable all metadata areas in volume group %s.",
			  pv->vg->name);
		return 0;
	}

	/*
	 * Non-orphan case is more complex.
	 * If the PV's mdas are ignored, and we wish to un-ignore,
	 * we clear the bit and move them from the ignored mda list to the
	 * in_use list, ensuring the new state will get written to disk
	 * in the vg_write() path.
	 * If the PV's mdas are not ignored, and we are setting
	 * them to ignored, we set the bit but leave them on the in_use
	 * list, ensuring the new state will get written to disk in the
	 * vg_write() path.
	 */
	/* FIXME: Try not to update the cache here! Also, try to iterate over
	 *	  PV mdas only using the format instance's index somehow
	 * 	  (i.e. try to avoid using mda_locn_match call). */

	lvmcache_foreach_mda(info, _pv_mda_set_ignored_one, &baton);

	return 1;
}

struct label *pv_label(const struct physical_volume *pv)
{
	struct lvmcache_info *info =
		lvmcache_info_from_pvid((const char *)&pv->id.uuid, pv->dev, 0);

	if (info)
		return lvmcache_get_label(info);

	/* process_each_pv() may create dummy PVs that have no label */
	if (pv->vg && pv->dev)
		log_error(INTERNAL_ERROR "PV %s unexpectedly not in cache.",
			  dev_name(pv->dev));

	return NULL;
}
