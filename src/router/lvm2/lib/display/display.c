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
#include "lib/metadata/metadata.h"
#include "lib/display/display.h"
#include "lib/activate/activate.h"
#include "lib/commands/toolcontext.h"
#include "lib/metadata/segtype.h"
#include "lib/config/defaults.h"
#include "lib/misc/lvm-signal.h"

#include <stdarg.h>

static const struct {
	alloc_policy_t alloc;
	const char str[14]; /* must be changed when size extends 13 chars */
	const char repchar;
} _policies[] = {
	{
	ALLOC_CONTIGUOUS, "contiguous", 'c'}, {
	ALLOC_CLING, "cling", 'l'}, {
	ALLOC_CLING_BY_TAGS, "cling_by_tags", 't'}, {	/* Only used in log mesgs */
	ALLOC_NORMAL, "normal", 'n'}, {
	ALLOC_ANYWHERE, "anywhere", 'a'}, {
	ALLOC_INHERIT, "inherit", 'i'}
};

static const int _num_policies = DM_ARRAY_SIZE(_policies);

char alloc_policy_char(alloc_policy_t alloc)
{
	int i;

	for (i = 0; i < _num_policies; i++)
		if (_policies[i].alloc == alloc)
			return _policies[i].repchar;

	return '-';
}

const char *get_alloc_string(alloc_policy_t alloc)
{
	int i;

	for (i = 0; i < _num_policies; i++)
		if (_policies[i].alloc == alloc)
			return _policies[i].str;

	return NULL;
}

alloc_policy_t get_alloc_from_string(const char *str)
{
	int i;

	/* cling_by_tags is part of cling */
	if (!strcmp("cling_by_tags", str))
		return ALLOC_CLING;

	for (i = 0; i < _num_policies; i++)
		if (!strcmp(_policies[i].str, str))
			return _policies[i].alloc;

	/* Special case for old metadata */
	if (!strcmp("next free", str))
		return ALLOC_NORMAL;

	log_error("Unrecognised allocation policy %s", str);
	return ALLOC_INVALID;
}

const char *get_lock_type_string(lock_type_t lock_type)
{
	switch (lock_type) {
	case LOCK_TYPE_INVALID:
		return "invalid";
	case LOCK_TYPE_NONE:
		return "none";
	case LOCK_TYPE_CLVM:
		return "clvm";
	case LOCK_TYPE_DLM:
		return "dlm";
	case LOCK_TYPE_SANLOCK:
		return "sanlock";
	}
	return "invalid";
}

lock_type_t get_lock_type_from_string(const char *str)
{
	if (!str)
		return LOCK_TYPE_NONE;
	if (!strcmp(str, "none"))
		return LOCK_TYPE_NONE;
	if (!strcmp(str, "clvm"))
		return LOCK_TYPE_CLVM;
	if (!strcmp(str, "dlm"))
		return LOCK_TYPE_DLM;
	if (!strcmp(str, "sanlock"))
		return LOCK_TYPE_SANLOCK;
	return LOCK_TYPE_INVALID;
}

static const char *_percent_types[7] = { "NONE", "VG", "FREE", "LV", "PVS", "ORIGIN" };

const char *get_percent_string(percent_type_t def)
{
	return _percent_types[def];
}

static const char *_lv_name(const struct logical_volume *lv)
{
	/* Never try to display names of the internal snapshot structures. */
	if (lv_is_snapshot(lv))
		return find_cow(lv)->name;

	return lv->name;
}

const char *display_lvname(const struct logical_volume *lv)
{
	char *name;
	const char *lv_name = _lv_name(lv);
	int r;

	if ((lv->vg->cmd->display_lvname_idx + NAME_LEN) >= sizeof((lv->vg->cmd->display_buffer)))
		lv->vg->cmd->display_lvname_idx = 0;

	name = lv->vg->cmd->display_buffer + lv->vg->cmd->display_lvname_idx;
	r = dm_snprintf(name, NAME_LEN, "%s/%s", lv->vg->name, lv_name);

	if (r < 0) {
		log_error("Full LV name \"%s/%s\" is too long.", lv->vg->name, lv_name);
		return NULL;
	}

	lv->vg->cmd->display_lvname_idx += r + 1;

	return name;
}

/* Display percentage with (TODO) configurable precision */
const char *display_percent(struct cmd_context *cmd, dm_percent_t percent)
{
	char *buf;
	int r;

        /* Reusing same  ring buffer we use for displaying LV names */
	if ((cmd->display_lvname_idx + NAME_LEN) >= sizeof((cmd->display_buffer)))
		cmd->display_lvname_idx = 0;

	buf = cmd->display_buffer + cmd->display_lvname_idx;
	/* TODO: Make configurable hardcoded 2 digits */
	r = dm_snprintf(buf, NAME_LEN, "%.2f", dm_percent_to_round_float(percent, 2));

	if (r < 0) {
		log_error("Percentage %d does not fit.", percent);
		return NULL;
	}

	cmd->display_lvname_idx += r + 1;

	return buf;
}

/* Size supplied in sectors */
static const char *_display_size(const struct cmd_context *cmd,
				 uint64_t size, dm_size_suffix_t suffix_type)
{
	return dm_size_to_string(cmd->mem, size, cmd->current_settings.unit_type,
				 cmd->si_unit_consistency, 
				 cmd->current_settings.unit_factor,
				 cmd->current_settings.suffix,
				 suffix_type);
}

const char *display_size_long(const struct cmd_context *cmd, uint64_t size)
{
	return _display_size(cmd, size, DM_SIZE_LONG);
}

const char *display_size_units(const struct cmd_context *cmd, uint64_t size)
{
	return _display_size(cmd, size, DM_SIZE_UNIT);
}

const char *display_size(const struct cmd_context *cmd, uint64_t size)
{
	return _display_size(cmd, size, DM_SIZE_SHORT);
}

void pvdisplay_colons(const struct physical_volume *pv)
{
	char uuid[64] __attribute__((aligned(8)));

	if (!pv)
		return;

	if (!id_write_format(&pv->id, uuid, sizeof(uuid))) {
		stack;
		return;
	}

	log_print("%s:%s:%" PRIu64 ":-1:%" PRIu64 ":%" PRIu64 ":-1:%" PRIu32 ":%u:%u:%u:%s",
		  pv_dev_name(pv), pv_vg_name(pv), pv->size,
		  /* FIXME pv->pv_number, Derive or remove? */
		  pv->status,	/* FIXME Support old or new format here? */
		  pv->status & ALLOCATABLE_PV,	/* FIXME remove? */
		  /* FIXME pv->lv_cur, Remove? */
		  pv->pe_size / 2,
		  pv->pe_count,
		  pv->pe_count - pv->pe_alloc_count,
		  pv->pe_alloc_count, *uuid ? uuid : "none");
}

void pvdisplay_segments(const struct physical_volume *pv)
{
	const struct pv_segment *pvseg;

	if (pv->pe_size)
		log_print("--- Physical Segments ---");

	dm_list_iterate_items(pvseg, &pv->segments) {
		log_print("Physical extent %u to %u:",
			  pvseg->pe, pvseg->pe + pvseg->len - 1);

		if (pvseg_is_allocated(pvseg)) {
			log_print("  Logical volume\t%s%s/%s",
				  pvseg->lvseg->lv->vg->cmd->dev_dir,
				  pvseg->lvseg->lv->vg->name,
				  pvseg->lvseg->lv->name);
			log_print("  Logical extents\t%d to %d",
				  pvseg->lvseg->le, pvseg->lvseg->le +
				  pvseg->lvseg->len - 1);
		} else
			log_print("  FREE");
	}

	log_print(" ");
}

/* FIXME Include label fields */
void pvdisplay_full(const struct cmd_context *cmd,
		    const struct physical_volume *pv,
		    void *handle __attribute__((unused)))
{
	char uuid[64] __attribute__((aligned(8)));
	const char *size;

	uint32_t pe_free;
	uint64_t data_size, pvsize, unusable;

	if (!pv)
		return;

	if (!id_write_format(&pv->id, uuid, sizeof(uuid))) {
		stack;
		return;
	}

	log_print("--- %sPhysical volume ---", pv->pe_size ? "" : "NEW ");
	log_print("PV Name               %s", pv_dev_name(pv));
	log_print("VG Name               %s%s",
		  is_orphan(pv) ? "" : pv->vg_name,
		  pv->status & EXPORTED_VG ? " (exported)" : "");

	data_size = (uint64_t) pv->pe_count * pv->pe_size;
	if (pv->size > data_size + pv->pe_start) {
		pvsize = pv->size;
		unusable = pvsize - data_size;
	} else {
		pvsize = data_size + pv->pe_start;
		unusable = pvsize - pv->size;
	}

	size = display_size(cmd, pvsize);
	if (data_size)
		log_print("PV Size               %s / not usable %s",	/*  [LVM: %s]", */
			  size, display_size(cmd, unusable));
	else
		log_print("PV Size               %s", size);

	/* PV number not part of LVM2 design
	   log_print("PV#                   %u", pv->pv_number);
	 */

	pe_free = pv->pe_count - pv->pe_alloc_count;
	if (pv->pe_count && (pv->status & ALLOCATABLE_PV))
		log_print("Allocatable           yes %s",
			  (!pe_free && pv->pe_count) ? "(but full)" : "");
	else
		log_print("Allocatable           NO");

	/* LV count is no longer available when displaying PV
	   log_print("Cur LV                %u", vg->lv_count);
	 */

	if (cmd->si_unit_consistency)
		log_print("PE Size               %s", display_size(cmd, (uint64_t) pv->pe_size));
	else
		log_print("PE Size (KByte)       %" PRIu32, pv->pe_size / 2);

	log_print("Total PE              %u", pv->pe_count);
	log_print("Free PE               %" PRIu32, pe_free);
	log_print("Allocated PE          %u", pv->pe_alloc_count);
	log_print("PV UUID               %s", *uuid ? uuid : "none");
	log_print(" ");
}

int pvdisplay_short(const struct cmd_context *cmd __attribute__((unused)),
		    const struct volume_group *vg __attribute__((unused)),
		    const struct physical_volume *pv,
		    void *handle __attribute__((unused)))
{
	char uuid[64] __attribute__((aligned(8)));

	if (!pv)
		return_0;

	if (!id_write_format(&pv->id, uuid, sizeof(uuid)))
		return_0;

	log_print("PV Name               %s     ", pv_dev_name(pv));
	/* FIXME  pv->pv_number); */
	log_print("PV UUID               %s", *uuid ? uuid : "none");
	log_print("PV Status             %sallocatable",
		  (pv->status & ALLOCATABLE_PV) ? "" : "NOT ");
	log_print("Total PE / Free PE    %u / %u",
		  pv->pe_count, pv->pe_count - pv->pe_alloc_count);

	log_print(" ");

	return 1; /* ECMD_PROCESSED */
}

void lvdisplay_colons(const struct logical_volume *lv)
{
	int inkernel;
	struct lvinfo info;
	inkernel = lv_info(lv->vg->cmd, lv, 0, &info, 1, 0) && info.exists;

	log_print("%s%s/%s:%s:%" PRIu64 ":%d:-1:%d:%" PRIu64 ":%d:-1:%d:%d:%d:%d",
		  lv->vg->cmd->dev_dir,
		  lv->vg->name,
		  lv->name,
		  lv->vg->name,
		  ((lv->status & (LVM_READ | LVM_WRITE)) >> 8) |
		  ((inkernel && info.read_only) ? 4 : 0), inkernel ? 1 : 0,
		  /* FIXME lv->lv_number,  */
		  inkernel ? info.open_count : 0, lv->size, lv->le_count,
		  /* FIXME Add num allocated to struct! lv->lv_allocated_le, */
		  (lv->alloc == ALLOC_CONTIGUOUS ? 2 : 0), lv->read_ahead,
		  inkernel ? info.major : -1, inkernel ? info.minor : -1);
}

static int _lvdisplay_historical_full(struct cmd_context *cmd,
				      const struct logical_volume *lv)
{
	char uuid[64] __attribute__((aligned(8)));
	int lvm1compat = find_config_tree_bool(cmd, global_lvdisplay_shows_full_device_path_CFG, NULL);
	struct historical_logical_volume *hlv = lv->this_glv->historical;

	if (!id_write_format(&hlv->lvid.id[1], uuid, sizeof(uuid)))
		return_0;

	log_print("--- Historical Logical volume ---");

	if (lvm1compat)
		/* /dev/vgname/lvname doen't actually exist for historical devices */
		log_print("LV Name                %s%s/%s",
			  hlv->vg->cmd->dev_dir, hlv->vg->name, hlv->name);
	else
		log_print("LV Name                %s%s", HISTORICAL_LV_PREFIX, hlv->name);

	log_print("VG Name                %s", hlv->vg->name);
	log_print("LV UUID                %s", uuid);
	log_print("LV Creation time       %s", lv_creation_time_dup(cmd->mem, lv, 1));
	log_print("LV Removal time        %s", lv_removal_time_dup(cmd->mem, lv, 1));

	log_print(" ");
	return 1;
}

int lvdisplay_full(struct cmd_context *cmd,
		   const struct logical_volume *lv,
		   void *handle __attribute__((unused)))
{
	struct lvinfo info;
	int inkernel, snap_active = 0;
	char uuid[64] __attribute__((aligned(8)));
	const char *access_str;
	struct lv_segment *snap_seg = NULL, *mirror_seg = NULL;
	struct lv_segment *seg = NULL;
	int lvm1compat;
	dm_percent_t snap_percent;
	int thin_data_active = 0, thin_metadata_active = 0;
	dm_percent_t thin_data_percent, thin_metadata_percent;
	int thin_active = 0;
	dm_percent_t thin_percent;
	struct lv_status_cache *cache_status = NULL;
	struct lv_status_vdo *vdo_status = NULL;

	if (lv_is_historical(lv))
		return _lvdisplay_historical_full(cmd, lv);

	if (!id_write_format(&lv->lvid.id[1], uuid, sizeof(uuid)))
		return_0;

	inkernel = lv_info(cmd, lv, 0, &info, 1, 1) && info.exists;

	if ((lv->status & LVM_WRITE) && inkernel && info.read_only)
		access_str = "read/write (activated read only)";
	else if (lv->status & LVM_WRITE)
		access_str = "read/write";
	else
		access_str = "read only";

	log_print("--- Logical volume ---");

	lvm1compat = find_config_tree_bool(cmd, global_lvdisplay_shows_full_device_path_CFG, NULL);

	if (lvm1compat)
		/* /dev/vgname/lvname doen't actually exist for internal devices */
		log_print("LV Name                %s%s/%s",
			  lv->vg->cmd->dev_dir, lv->vg->name, lv->name);
	else if (lv_is_visible(lv)) {
		/* Thin pool does not have /dev/vg/name link */
		if (!lv_is_thin_pool(lv))
			log_print("LV Path                %s%s/%s",
				  lv->vg->cmd->dev_dir,
				  lv->vg->name, lv->name);
		log_print("LV Name                %s", lv->name);
	} else
		log_print("Internal LV Name       %s", lv->name);

	log_print("VG Name                %s", lv->vg->name);
	log_print("LV UUID                %s", uuid);
	log_print("LV Write Access        %s", access_str);
	log_print("LV Creation host, time %s, %s",
		  lv_host_dup(cmd->mem, lv), lv_creation_time_dup(cmd->mem, lv, 1));

	if (lv_is_origin(lv)) {
		log_print("LV snapshot status     source of");

		dm_list_iterate_items_gen(snap_seg, &lv->snapshot_segs,
				       origin_list) {
			if (inkernel &&
			    (snap_active = lv_snapshot_percent(snap_seg->cow,
							       &snap_percent)))
				if (snap_percent == DM_PERCENT_INVALID)
					snap_active = 0;
			if (lvm1compat)
				log_print("                       %s%s/%s [%s]",
					  lv->vg->cmd->dev_dir, lv->vg->name,
					  snap_seg->cow->name,
					  snap_active ? "active" : "INACTIVE");
			else
				log_print("                       %s [%s]",
					  snap_seg->cow->name,
					  snap_active ? "active" : "INACTIVE");
		}
		snap_seg = NULL;
	} else if ((snap_seg = find_snapshot(lv))) {
		if (inkernel &&
		    (snap_active = lv_snapshot_percent(snap_seg->cow,
						       &snap_percent)))
			if (snap_percent == DM_PERCENT_INVALID)
				snap_active = 0;

		if (lvm1compat)
			log_print("LV snapshot status     %s destination for %s%s/%s",
				  snap_active ? "active" : "INACTIVE",
				  lv->vg->cmd->dev_dir, lv->vg->name,
				  snap_seg->origin->name);
		else
			log_print("LV snapshot status     %s destination for %s",
				  snap_active ? "active" : "INACTIVE",
				  snap_seg->origin->name);
	}

	if (lv_is_thin_volume(lv)) {
		seg = first_seg(lv);
		log_print("LV Pool name           %s", seg->pool_lv->name);
		if (seg->origin)
			log_print("LV Thin origin name    %s",
				  seg->origin->name);
		if (seg->external_lv)
			log_print("LV External origin name %s",
				  seg->external_lv->name);
		if (seg->merge_lv)
			log_print("LV merging to          %s",
				  seg->merge_lv->name);
		if (inkernel)
			thin_active = lv_thin_percent(lv, 0, &thin_percent);
		if (lv_is_merging_origin(lv))
			log_print("LV merged with         %s",
				  find_snapshot(lv)->lv->name);
	} else if (lv_is_thin_pool(lv)) {
		if (lv_info(cmd, lv, 1, &info, 1, 1) && info.exists) {
			thin_data_active = lv_thin_pool_percent(lv, 0, &thin_data_percent);
			thin_metadata_active = lv_thin_pool_percent(lv, 1, &thin_metadata_percent);
		}
		/* FIXME: display thin_pool targets transid for activated LV as well */
		seg = first_seg(lv);
		log_print("LV Pool metadata       %s", seg->metadata_lv->name);
		log_print("LV Pool data           %s", seg_lv(seg, 0)->name);
	} else if (lv_is_cache_origin(lv)) {
		if ((seg = get_only_segment_using_this_lv(lv)))
			log_print("LV origin of Cache LV  %s", seg->lv->name);
	} else if (lv_is_cache(lv)) {
		seg = first_seg(lv);
		if (inkernel && !lv_cache_status(lv, &cache_status))
                        return_0;
		log_print("LV Cache pool name     %s", seg->pool_lv->name);
		log_print("LV Cache origin name   %s", seg_lv(seg, 0)->name);
	} else if (lv_is_cache_pool(lv)) {
		seg = first_seg(lv);
		log_print("LV Pool metadata       %s", seg->metadata_lv->name);
		log_print("LV Pool data           %s", seg_lv(seg, 0)->name);
	} else if (lv_is_vdo_pool(lv)) {
		seg = first_seg(lv);
		log_print("LV VDO Pool data       %s", seg_lv(seg, 0)->name);
		if (inkernel && lv_vdo_pool_status(lv, 0, &vdo_status)) { /* FIXME: flush option? */
			log_print("LV VDO Pool usage      %s%%",
				  display_percent(cmd, vdo_status->usage));
			log_print("LV VDO Pool saving     %s%%",
				  display_percent(cmd, vdo_status->saving));
			log_print("LV VDO Operating mode  %s",
				  get_vdo_operating_mode_name(vdo_status->vdo->operating_mode));
			log_print("LV VDO Index state     %s",
				  get_vdo_index_state_name(vdo_status->vdo->index_state));
			log_print("LV VDO Compression st  %s",
				  get_vdo_compression_state_name(vdo_status->vdo->compression_state));
			log_print("LV VDO Used size       %s",
				  display_size(cmd, vdo_status->vdo->used_blocks * DM_VDO_BLOCK_SIZE));
			dm_pool_destroy(vdo_status->mem);
		}
	} else if (lv_is_vdo(lv)) {
		seg = first_seg(lv);
		log_print("LV VDO Pool name       %s", seg_lv(seg, 0)->name);
	}

	if (inkernel && info.suspended)
		log_print("LV Status              suspended");
	else if (activation())
		log_print("LV Status              %savailable",
			  inkernel ? "" : "NOT ");

/********* FIXME lv_number
    log_print("LV #                   %u", lv->lv_number + 1);
************/

	if (inkernel)
		log_print("# open                 %u", info.open_count);

	log_print("LV Size                %s",
		  display_size(cmd,
			       snap_seg ? snap_seg->origin->size : lv->size));

	if (cache_status) {
		log_print("Cache used blocks      %s%%",
			  display_percent(cmd, cache_status->data_usage));
		log_print("Cache metadata blocks  %s%%",
			  display_percent(cmd, cache_status->metadata_usage));
		log_print("Cache dirty blocks     %s%%",
			  display_percent(cmd, cache_status->dirty_usage));
		log_print("Cache read hits/misses " FMTu64 " / " FMTu64,
			  cache_status->cache->read_hits,
			  cache_status->cache->read_misses);
		log_print("Cache wrt hits/misses  " FMTu64 " / " FMTu64,
			  cache_status->cache->write_hits,
			  cache_status->cache->write_misses);
		log_print("Cache demotions        " FMTu64,
			  cache_status->cache->demotions);
		log_print("Cache promotions       " FMTu64,
			  cache_status->cache->promotions);

		dm_pool_destroy(cache_status->mem);
	}

	if (thin_data_active)
		log_print("Allocated pool data    %s%%",
			  display_percent(cmd, thin_data_percent));

	if (thin_metadata_active)
		log_print("Allocated metadata     %s%%",
			  display_percent(cmd, thin_metadata_percent));

	if (thin_active)
		log_print("Mapped size            %s%%",
			  display_percent(cmd, thin_percent));

	log_print("Current LE             %u",
		  snap_seg ? snap_seg->origin->le_count : lv->le_count);

	if (snap_seg) {
		log_print("COW-table size         %s",
			  display_size(cmd, (uint64_t) lv->size));
		log_print("COW-table LE           %u", lv->le_count);

		if (snap_active)
			log_print("Allocated to snapshot  %s%%",
				  display_percent(cmd, snap_percent));

		log_print("Snapshot chunk size    %s",
			  display_size(cmd, (uint64_t) snap_seg->chunk_size));
	}

	if (lv_is_mirrored(lv)) {
		mirror_seg = first_seg(lv);
		log_print("Mirrored volumes       %" PRIu32, mirror_seg->area_count);
		if (lv_is_converting(lv))
			log_print("LV type        Mirror undergoing conversion");
	}

	log_print("Segments               %u", dm_list_size(&lv->segments));

/********* FIXME Stripes & stripesize for each segment
	log_print("Stripe size            %s", display_size(cmd, (uint64_t) lv->stripesize));
***********/

	log_print("Allocation             %s", get_alloc_string(lv->alloc));
	if (lv->read_ahead == DM_READ_AHEAD_AUTO)
		log_print("Read ahead sectors     auto");
	else if (lv->read_ahead == DM_READ_AHEAD_NONE)
		log_print("Read ahead sectors     0");
	else
		log_print("Read ahead sectors     %u", lv->read_ahead);

	if (inkernel && lv->read_ahead != info.read_ahead)
		log_print("- currently set to     %u", info.read_ahead);

	if (lv->status & FIXED_MINOR) {
		if (lv->major >= 0)
			log_print("Persistent major       %d", lv->major);
		log_print("Persistent minor       %d", lv->minor);
	}

	if (inkernel)
		log_print("Block device           %d:%d", info.major,
			  info.minor);

	log_print(" ");

	return 1; /* ECMD_PROCESSED */
}

void display_stripe(const struct lv_segment *seg, uint32_t s, const char *pre)
{
	switch (seg_type(seg, s)) {
	case AREA_PV:
		/* FIXME Re-check the conditions for 'Missing' */
		log_print("%sPhysical volume\t%s", pre,
			  seg_pv(seg, s) ?
			  pv_dev_name(seg_pv(seg, s)) :
			    "Missing");

		if (seg_pv(seg, s))
			log_print("%sPhysical extents\t%d to %d", pre,
				  seg_pe(seg, s),
				  seg_pe(seg, s) + seg->area_len - 1);
		break;
	case AREA_LV:
		log_print("%sLogical volume\t%s", pre,
			  seg_lv(seg, s) ?
			  seg_lv(seg, s)->name : "Missing");

		if (seg_lv(seg, s))
			log_print("%sLogical extents\t%d to %d", pre,
				  seg_le(seg, s),
				  seg_le(seg, s) + seg->area_len - 1);
		break;
	case AREA_UNASSIGNED:
		log_print("%sUnassigned area", pre);
	}
}

int lvdisplay_segments(const struct logical_volume *lv)
{
	const struct lv_segment *seg;

	log_print("--- Segments ---");

	dm_list_iterate_items(seg, &lv->segments) {
		log_print("%s extents %u to %u:",
			  lv_is_virtual(lv) ? "Virtual" : "Logical",
			  seg->le, seg->le + seg->len - 1);

		log_print("  Type\t\t%s", lvseg_name(seg));

		if (seg->segtype->ops->target_monitored)
			log_print("  Monitoring\t\t%s",
				  lvseg_monitor_dup(lv->vg->cmd->mem, seg));

		if (seg->segtype->ops->display)
			seg->segtype->ops->display(seg);
	}

	log_print(" ");
	return 1;
}

void vgdisplay_extents(const struct volume_group *vg __attribute__((unused)))
{
}

void vgdisplay_full(const struct volume_group *vg)
{
	uint32_t access_str;
	uint32_t active_pvs;
	char uuid[64] __attribute__((aligned(8)));

	active_pvs = vg->pv_count - vg_missing_pv_count(vg);

	log_print("--- Volume group ---");
	log_print("VG Name               %s", vg->name);
	log_print("System ID             %s", (vg->system_id && *vg->system_id) ? vg->system_id : "");
	log_print("Format                %s", vg->fid->fmt->name);
	log_print("Metadata Areas        %d", vg_mda_count(vg));
	log_print("Metadata Sequence No  %d", vg->seqno);
	access_str = vg->status & (LVM_READ | LVM_WRITE);
	log_print("VG Access             %s%s%s%s",
		  access_str == (LVM_READ | LVM_WRITE) ? "read/write" : "",
		  access_str == LVM_READ ? "read" : "",
		  access_str == LVM_WRITE ? "write" : "",
		  access_str == 0 ? "error" : "");
	log_print("VG Status             %s%sresizable",
		  vg_is_exported(vg) ? "exported/" : "",
		  vg_is_resizeable(vg) ? "" : "NOT ");
	/* vg number not part of LVM2 design
	   log_print ("VG #                  %u\n", vg->vg_number);
	 */
	if (vg_is_clustered(vg)) {
		log_print("Clustered             yes");
		log_print("Shared                %s",
			  vg->status & SHARED ? "yes" : "no");
	}

	log_print("MAX LV                %u", vg->max_lv);
	log_print("Cur LV                %u", vg_visible_lvs(vg));
	log_print("Open LV               %u", lvs_in_vg_opened(vg));
/****** FIXME Max LV Size
      log_print ( "MAX LV Size           %s",
               ( s1 = display_size ( LVM_LV_SIZE_MAX(vg))));
      free ( s1);
*********/
	log_print("Max PV                %u", vg->max_pv);
	log_print("Cur PV                %u", vg->pv_count);
	log_print("Act PV                %u", active_pvs);

	log_print("VG Size               %s",
		  display_size(vg->cmd,
			       (uint64_t) vg->extent_count * vg->extent_size));

	log_print("PE Size               %s",
		  display_size(vg->cmd, vg->extent_size));

	log_print("Total PE              %u", vg->extent_count);

	log_print("Alloc PE / Size       %u / %s",
		  vg->extent_count - vg->free_count,
		  display_size(vg->cmd,
			       (uint64_t) (vg->extent_count - vg->free_count) *
			       vg->extent_size));

	log_print("Free  PE / Size       %u / %s", vg->free_count,
		  display_size(vg->cmd, vg_free(vg)));

	if (!id_write_format(&vg->id, uuid, sizeof(uuid))) {
		stack;
		return;
	}

	log_print("VG UUID               %s", uuid);
	log_print(" ");
}

void vgdisplay_colons(const struct volume_group *vg)
{
	uint32_t active_pvs;
	const char *access_str;
	char uuid[64] __attribute__((aligned(8)));

	active_pvs = vg->pv_count - vg_missing_pv_count(vg);

	switch (vg->status & (LVM_READ | LVM_WRITE)) {
		case LVM_READ | LVM_WRITE:
			access_str = "r/w";
			break;
		case LVM_READ:
			access_str = "r";
			break;
		case LVM_WRITE:
			access_str = "w";
			break;
		default:
			access_str = "";
	}

	if (!id_write_format(&vg->id, uuid, sizeof(uuid))) {
		stack;
		return;
	}

	log_print("%s:%s:%" PRIu64 ":-1:%u:%u:%u:-1:%u:%u:%u:%" PRIu64 ":%" PRIu32
		  ":%u:%u:%u:%s",
		vg->name,
		access_str,
		vg->status,
		/* internal volume group number; obsolete */
		vg->max_lv,
		vg_visible_lvs(vg),
		lvs_in_vg_opened(vg),
		/* FIXME: maximum logical volume size */
		vg->max_pv,
		vg->pv_count,
		active_pvs,
		(uint64_t) vg->extent_count * (vg->extent_size / 2),
		vg->extent_size / 2,
		vg->extent_count,
		vg->extent_count - vg->free_count,
		vg->free_count,
		uuid[0] ? uuid : "none");
}

void vgdisplay_short(const struct volume_group *vg)
{
	log_print("\"%s\" %-9s [%-9s used / %s free]", vg->name,
/********* FIXME if "open" print "/used" else print "/idle"???  ******/
		  display_size(vg->cmd,
			       (uint64_t) vg->extent_count * vg->extent_size),
		  display_size(vg->cmd,
			       ((uint64_t) vg->extent_count -
				vg->free_count) * vg->extent_size),
		  display_size(vg->cmd, vg_free(vg)));
}

void display_formats(const struct cmd_context *cmd)
{
	const struct format_type *fmt;

	dm_list_iterate_items(fmt, &cmd->formats) {
		log_print("%s", fmt->name);
	}
}

void display_segtypes(const struct cmd_context *cmd)
{
	const struct segment_type *segtype;

	dm_list_iterate_items(segtype, &cmd->segtypes) {
		log_print("%s", segtype->name);
	}
}

void display_tags(const struct cmd_context *cmd)
{
	const struct dm_str_list *sl;

	dm_list_iterate_items(sl, &cmd->tags) {
		log_print("%s", sl->str);
	}
}

void display_name_error(name_error_t name_error)
{
	switch(name_error) {
	case NAME_VALID:
		/* Valid name */
		break;
	case NAME_INVALID_EMPTY:
		log_error("Name is zero length.");
		break;
	case NAME_INVALID_HYPHEN:
		log_error("Name cannot start with hyphen.");
		break;
	case NAME_INVALID_DOTS:
		log_error("Name starts with . or .. and has no "
			  "following character(s).");
		break;
	case NAME_INVALID_CHARSET:
		log_error("Name contains invalid character, valid set includes: "
			  "[a-zA-Z0-9.-_+].");
		break;
	case NAME_INVALID_LENGTH:
		/* Report that name length - 1 to accommodate nul*/
		log_error("Name length exceeds maximum limit of %d.", (NAME_LEN - 1));
		break;
	default:
		log_error(INTERNAL_ERROR "Unknown error %d on name validation.", name_error);
		break;
	}
}

/*
 * Prompt for y or n from stdin.
 * Defaults to 'no' in silent mode.
 * All callers should support --yes and/or --force to override this.
 *
 * Accepted are either _yes[] or _no[] strings or just their outset.
 * When running without 'tty' stdin is printed to stderr.
 * 'Yes' is accepted ONLY with '\n'.
 */
char yes_no_prompt(const char *prompt, ...)
{
	/* Lowercase Yes/No strings */
	static const char _yes[] = "yes";
	static const char _no[] = "no";
	const char *answer = NULL;
	int c = silent_mode() ? EOF : 0;
	int i = 0, ret = 0, sig = 0;
	char buf[12];
	va_list ap;

	sigint_allow();

	for (;;) {
		if (!ret) {
			/* Show prompt */
			va_start(ap, prompt);
			vfprintf(stderr, prompt, ap);
			va_end(ap);
			fflush(stderr);

			if (c == EOF)
				break;

			i = 0;
			answer = NULL;
		}

	nextchar:
		if ((sig = sigint_caught()))
			break;	/* Check if already interrupted before getchar() */

		if ((c = getchar()) == EOF) {
			/* SIGNAL or no chars on stdin (missing '\n') or ^D */
			if (!i)
				break; /* Just shown prompt,-> print [n]\n */

			goto invalid; /* Note:  c holds EOF */
		}

		if ((i < (sizeof(buf) - 4)) && isprint(c))
			buf[i++] = c;

		c = tolower(c);

		if ((ret > 0) && (c == answer[0]))
			answer++;	/* Matching, next char */
		else if (c == '\n') {
			if (feof(stdin))
				fputc('\n', stderr);
			if (ret > 0)
				break;	/* Answered */
	invalid:
			if (i >= (sizeof(buf) - 4)) {
				/* '...'  for missing input */
				i = sizeof(buf) - 1;
				buf[i - 1] = buf[i - 2] = buf[i - 3] = '.';
			}
			buf[i] = 0;
			log_warn("WARNING: Invalid input '%s'.", buf);
			ret = 0;	/* Otherwise refresh prompt */
		} else if (!ret && (c == _yes[0])) {
			ret = 'y';
			answer = _yes + 1;	/* Expecting 'Yes' */
		} else if (!ret && (c == _no[0])) {
			ret = 'n';
			answer = _no + 1;	/* Expecting 'No' */
		} else if (!ret && isspace(c)) {
			/* Ignore any whitespace before */
			--i;
			goto nextchar;
		} else if ((ret > 0) && isspace(c)) {
			/* Ignore any whitespace after */
			while (*answer)
				answer++; /* jump to end-of-word */
		} else
			ret = -1;	/* Read till '\n' and refresh */
	}

	sigint_restore();

	/* For other then Yes answer check there is really no interrupt */
	if (sig || sigint_caught()) {
		stack;
		ret = 'n';
	} else if (c == EOF) {
		fputs("[n]\n", stderr);
		ret = 'n';
	} else
		/* Not knowing if it's terminal, makes this hard.... */
		log_verbose("Accepted input: [%c]", ret);

	return ret;
}
