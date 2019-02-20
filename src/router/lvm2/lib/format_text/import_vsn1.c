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
#include "import-export.h"
#include "lib/display/display.h"
#include "lib/commands/toolcontext.h"
#include "lib/cache/lvmcache.h"
#include "lib/locking/lvmlockd.h"
#include "lib/metadata/lv_alloc.h"
#include "lib/metadata/pv_alloc.h"
#include "lib/metadata/segtype.h"
#include "lib/format_text/text_import.h"
#include "lib/config/defaults.h"
#include "lib/datastruct/str_list.h"

typedef int (*section_fn) (struct format_instance * fid,
			   struct volume_group * vg, const struct dm_config_node * pvn,
			   const struct dm_config_node * vgn,
			   struct dm_hash_table * pv_hash,
			   struct dm_hash_table * lv_hash);

#define _read_int32(root, path, result) \
	dm_config_get_uint32(root, path, (uint32_t *) (result))

#define _read_uint32(root, path, result) \
	dm_config_get_uint32(root, path, (result))

#define _read_uint64(root, path, result) \
	dm_config_get_uint64(root, path, (result))

/*
 * Logs an attempt to read an invalid format file.
 */
static void _invalid_format(const char *str)
{
	log_error("Can't process text format file - %s.", str);
}

/*
 * Checks that the config file contains vg metadata, and that it
 * we recognise the version number,
 */
static int _vsn1_check_version(const struct dm_config_tree *cft)
{
	const struct dm_config_node *cn;
	const struct dm_config_value *cv;

	/*
	 * Check the contents field.
	 */
	if (!(cn = dm_config_find_node(cft->root, CONTENTS_FIELD))) {
		_invalid_format("missing contents field");
		return 0;
	}

	cv = cn->v;
	if (!cv || cv->type != DM_CFG_STRING || strcmp(cv->v.str, CONTENTS_VALUE)) {
		_invalid_format("unrecognised contents field");
		return 0;
	}

	/*
	 * Check the version number.
	 */
	if (!(cn = dm_config_find_node(cft->root, FORMAT_VERSION_FIELD))) {
		_invalid_format("missing version number");
		return 0;
	}

	cv = cn->v;
	if (!cv || cv->type != DM_CFG_INT || cv->v.i != FORMAT_VERSION_VALUE) {
		_invalid_format("unrecognised version number");
		return 0;
	}

	return 1;
}

static int _is_converting(struct logical_volume *lv)
{
	struct lv_segment *seg;

	if (lv_is_mirrored(lv)) {
		seg = first_seg(lv);
		/* Can't use is_temporary_mirror() because the metadata for
		 * seg_lv may not be read in and flags may not be set yet. */
		if (seg_type(seg, 0) == AREA_LV &&
		    strstr(seg_lv(seg, 0)->name, MIRROR_SYNC_LAYER))
			return 1;
	}

	return 0;
}

static int _read_id(struct id *id, const struct dm_config_node *cn, const char *path)
{
	const char *uuid;

	if (!dm_config_get_str(cn, path, &uuid)) {
		log_error("Couldn't find uuid.");
		return 0;
	}

	if (!id_read_format(id, uuid)) {
		log_error("Invalid uuid.");
		return 0;
	}

	return 1;
}

static int _read_flag_config(const struct dm_config_node *n, uint64_t *status, int type)
{
	const struct dm_config_value *cv;
	*status = 0;

	if (!dm_config_get_list(n, "status", &cv)) {
		log_error("Could not find status flags.");
		return 0;
	}

	/* For backward compatible metadata accept both type of flags */
	if (!(read_flags(status, type, STATUS_FLAG | SEGTYPE_FLAG, cv))) {
		log_error("Could not read status flags.");
		return 0;
	}

	if (dm_config_get_list(n, "flags", &cv)) {
		if (!(read_flags(status, type, COMPATIBLE_FLAG, cv))) {
			log_error("Could not read flags.");
			return 0;
		}
	}

	return 1;
}

static int _read_str_list(struct dm_pool *mem, struct dm_list *list, const struct dm_config_value *cv)
{
	if (cv->type == DM_CFG_EMPTY_ARRAY)
		return 1;

	while (cv) {
		if (cv->type != DM_CFG_STRING) {
			log_error("Found an item that is not a string");
			return 0;
		}

		if (!str_list_add(mem, list, dm_pool_strdup(mem, cv->v.str)))
			return_0;

		cv = cv->next;
	}

	return 1;
}

static int _read_pv(struct format_instance *fid,
		    struct volume_group *vg, const struct dm_config_node *pvn,
		    const struct dm_config_node *vgn __attribute__((unused)),
		    struct dm_hash_table *pv_hash,
		    struct dm_hash_table *lv_hash __attribute__((unused)))
{
	struct dm_pool *mem = vg->vgmem;
	struct physical_volume *pv;
	struct pv_list *pvl;
	const struct dm_config_value *cv;
	uint64_t size, ba_start;

	if (!(pvl = dm_pool_zalloc(mem, sizeof(*pvl))) ||
	    !(pvl->pv = dm_pool_zalloc(mem, sizeof(*pvl->pv))))
		return_0;

	pv = pvl->pv;

	/*
	 * Add the pv to the pv hash for quick lookup when we read
	 * the lv segments.
	 */
	if (!dm_hash_insert(pv_hash, pvn->key, pv))
		return_0;

	if (!(pvn = pvn->child)) {
		log_error("Empty pv section.");
		return 0;
	}

	if (!_read_id(&pv->id, pvn, "id")) {
		log_error("Couldn't read uuid for physical volume.");
		return 0;
	}

        pv->is_labelled = 1; /* All format_text PVs are labelled. */

	/*
	 * Convert the uuid into a device.
	 */
	if (!(pv->dev = lvmcache_device_from_pvid(fid->fmt->cmd, &pv->id, &pv->label_sector))) {
		char buffer[64] __attribute__((aligned(8)));

		if (!id_write_format(&pv->id, buffer, sizeof(buffer)))
			buffer[0] = '\0';

		if (fid->fmt->cmd && !fid->fmt->cmd->pvscan_cache_single)
			log_error_once("Couldn't find device with uuid %s.", buffer);
		else
			log_debug_metadata("Couldn't find device with uuid %s.", buffer);
	}

	if (!(pv->vg_name = dm_pool_strdup(mem, vg->name)))
		return_0;

	memcpy(&pv->vgid, &vg->id, sizeof(vg->id));

	if (!_read_flag_config(pvn, &pv->status, PV_FLAGS)) {
		log_error("Couldn't read status flags for physical volume.");
		return 0;
	}

	if (!pv->dev)
		pv->status |= MISSING_PV;

	if ((pv->status & MISSING_PV) && pv->dev && pv_mda_used_count(pv) == 0) {
		pv->status &= ~MISSING_PV;
		log_info("Recovering a previously MISSING PV %s with no MDAs.",
			 pv_dev_name(pv));
	}

	/* Late addition */
	if (dm_config_has_node(pvn, "dev_size") &&
	    !_read_uint64(pvn, "dev_size", &pv->size)) {
		log_error("Couldn't read dev size for physical volume.");
		return 0;
	}

	if (!_read_uint64(pvn, "pe_start", &pv->pe_start)) {
		log_error("Couldn't read extent start value (pe_start) "
			  "for physical volume.");
		return 0;
	}

	if (!_read_int32(pvn, "pe_count", &pv->pe_count)) {
		log_error("Couldn't find extent count (pe_count) for "
			  "physical volume.");
		return 0;
	}

	/* Bootloader area is not compulsory - just log_debug for the record if found. */
	ba_start = size = 0;
	_read_uint64(pvn, "ba_start", &ba_start);
	_read_uint64(pvn, "ba_size", &size);
	if (ba_start && size) {
		log_debug_metadata("Found bootloader area specification for PV %s "
			  "in metadata: ba_start=%" PRIu64 ", ba_size=%" PRIu64 ".",
			  pv_dev_name(pv), ba_start, size);
		pv->ba_start = ba_start;
		pv->ba_size = size;
	} else if ((!ba_start && size) || (ba_start && !size)) {
		log_error("Found incomplete bootloader area specification "
			  "for PV %s in metadata.", pv_dev_name(pv));
		return 0;
	}

	dm_list_init(&pv->tags);
	dm_list_init(&pv->segments);

	/* Optional tags */
	if (dm_config_get_list(pvn, "tags", &cv) &&
	    !(_read_str_list(mem, &pv->tags, cv))) {
		log_error("Couldn't read tags for physical volume %s in %s.",
			  pv_dev_name(pv), vg->name);
		return 0;
	}

	pv->pe_size = vg->extent_size;

	pv->pe_alloc_count = 0;
	pv->pe_align = 0;
	pv->fmt = fid->fmt;

	/* Fix up pv size if missing or impossibly large */
	if ((!pv->size || pv->size > (1ULL << 62)) && pv->dev) {
		if (!dev_get_size(pv->dev, &pv->size)) {
			log_error("%s: Couldn't get size.", pv_dev_name(pv));
			return 0;
		}
		log_verbose("Fixing up missing size (%s) "
			    "for PV %s", display_size(fid->fmt->cmd, pv->size),
			    pv_dev_name(pv));
		size = pv->pe_count * (uint64_t) vg->extent_size + pv->pe_start;
		if (size > pv->size)
			log_warn("WARNING: Physical Volume %s is too large "
				 "for underlying device", pv_dev_name(pv));
	}

	if (!alloc_pv_segment_whole_pv(mem, pv))
		return_0;

	vg->extent_count += pv->pe_count;
	vg->free_count += pv->pe_count;
	add_pvl_to_vgs(vg, pvl);

	return 1;
}

static void _insert_segment(struct logical_volume *lv, struct lv_segment *seg)
{
	struct lv_segment *comp;

	dm_list_iterate_items(comp, &lv->segments) {
		if (comp->le > seg->le) {
			dm_list_add(&comp->list, &seg->list);
			return;
		}
	}

	lv->le_count += seg->len;
	dm_list_add(&lv->segments, &seg->list);
}

static int _read_segment(struct logical_volume *lv, const struct dm_config_node *sn,
			 struct dm_hash_table *pv_hash)
{
	struct dm_pool *mem = lv->vg->vgmem;
	uint32_t area_count = 0u;
	struct lv_segment *seg;
	const struct dm_config_node *sn_child = sn->child;
	const struct dm_config_value *cv;
	uint32_t area_extents, start_extent, extent_count, reshape_count, data_copies;
	struct segment_type *segtype;
	const char *segtype_str;
	char *segtype_with_flags;

	if (!sn_child) {
		log_error("Empty segment section.");
		return 0;
	}

	if (!_read_int32(sn_child, "start_extent", &start_extent)) {
		log_error("Couldn't read 'start_extent' for segment '%s' "
			  "of logical volume %s.", sn->key, lv->name);
		return 0;
	}

	if (!_read_int32(sn_child, "extent_count", &extent_count)) {
		log_error("Couldn't read 'extent_count' for segment '%s' "
			  "of logical volume %s.", sn->key, lv->name);
		return 0;
	}

	if (!_read_int32(sn_child, "reshape_count", &reshape_count))
		reshape_count = 0;

	if (!_read_int32(sn_child, "data_copies", &data_copies))
		data_copies = 1;

	segtype_str = SEG_TYPE_NAME_STRIPED;

	if (!dm_config_get_str(sn_child, "type", &segtype_str)) {
		log_error("Segment type must be a string.");
		return 0;
	}

        /* Locally duplicate to parse out status flag bits */
	if (!(segtype_with_flags = dm_pool_strdup(mem, segtype_str))) {
		log_error("Cannot duplicate segtype string.");
		return 0;
	}

	if (!read_segtype_lvflags(&lv->status, segtype_with_flags)) {
		log_error("Couldn't read segtype for logical volume %s.",
			  display_lvname(lv));
	       return 0;
	}

	if (!(segtype = get_segtype_from_string(lv->vg->cmd, segtype_with_flags)))
		return_0;

	/* Can drop temporary string here as nothing has allocated from VGMEM meanwhile */
	dm_pool_free(mem, segtype_with_flags);

	if (segtype->ops->text_import_area_count &&
	    !segtype->ops->text_import_area_count(sn_child, &area_count))
		return_0;

	area_extents = segtype->parity_devs ?
		       raid_rimage_extents(segtype, extent_count, area_count - segtype->parity_devs, data_copies) : extent_count;
	if (!(seg = alloc_lv_segment(segtype, lv, start_extent,
				     extent_count, reshape_count, 0, 0, NULL, area_count,
				     area_extents, data_copies, 0, 0, 0, NULL))) {
		log_error("Segment allocation failed");
		return 0;
	}

	if (seg->segtype->ops->text_import &&
	    !seg->segtype->ops->text_import(seg, sn_child, pv_hash))
		return_0;

	/* Optional tags */
	if (dm_config_get_list(sn_child, "tags", &cv) &&
	    !(_read_str_list(mem, &seg->tags, cv))) {
		log_error("Couldn't read tags for a segment of %s/%s.",
			  lv->vg->name, lv->name);
		return 0;
	}

	/*
	 * Insert into correct part of segment list.
	 */
	_insert_segment(lv, seg);

	if (seg_is_mirror(seg))
		lv->status |= MIRROR;

	if (seg_is_mirrored(seg))
		lv->status |= MIRRORED;

	if (seg_is_raid(seg))
		lv->status |= RAID;

	if (seg_is_virtual(seg))
		lv->status |= VIRTUAL;

	if (!seg_is_raid(seg) && _is_converting(lv))
		lv->status |= CONVERTING;

	return 1;
}

int text_import_areas(struct lv_segment *seg, const struct dm_config_node *sn,
		      const struct dm_config_value *cv, struct dm_hash_table *pv_hash,
		      uint64_t status)
{
	unsigned int s;
	struct logical_volume *lv1;
	struct physical_volume *pv;
	const char *seg_name = dm_config_parent_name(sn);

	if (!seg->area_count) {
		log_error("Zero areas not allowed for segment %s", seg_name);
		return 0;
	}

	for (s = 0; cv && s < seg->area_count; s++, cv = cv->next) {

		/* first we read the pv */
		if (cv->type != DM_CFG_STRING) {
			log_error("Bad volume name in areas array for segment %s.", seg_name);
			return 0;
		}

		if (!cv->next) {
			log_error("Missing offset in areas array for segment %s.", seg_name);
			return 0;
		}

		if (cv->next->type != DM_CFG_INT) {
			log_error("Bad offset in areas array for segment %s.", seg_name);
			return 0;
		}

		/* FIXME Cope if LV not yet read in */
		if ((pv = dm_hash_lookup(pv_hash, cv->v.str))) {
			if (!set_lv_segment_area_pv(seg, s, pv, (uint32_t) cv->next->v.i))
				return_0;
		} else if ((lv1 = find_lv(seg->lv->vg, cv->v.str))) {
			if (!set_lv_segment_area_lv(seg, s, lv1,
						    (uint32_t) cv->next->v.i,
						    status))
				return_0;
		} else {
			log_error("Couldn't find volume '%s' "
				  "for segment '%s'.",
				  cv->v.str ? : "NULL", seg_name);
			return 0;
		}

		cv = cv->next;
	}

	/*
	 * Check we read the correct number of stripes.
	 */
	if (cv || (s < seg->area_count)) {
		log_error("Incorrect number of areas in area array "
			  "for segment '%s'.", seg_name);
		return 0;
	}

	return 1;
}

static int _read_segments(struct logical_volume *lv, const struct dm_config_node *lvn,
			  struct dm_hash_table *pv_hash)
{
	const struct dm_config_node *sn;
	int count = 0, seg_count;

	for (sn = lvn; sn; sn = sn->sib) {

		/*
		 * All sub-sections are assumed to be segments.
		 */
		if (!sn->v) {
			if (!_read_segment(lv, sn, pv_hash))
				return_0;

			count++;
		}
		/* FIXME Remove this restriction */
		if (lv_is_snapshot(lv) && count > 1) {
			log_error("Only one segment permitted for snapshot");
			return 0;
		}
	}

	if (!_read_int32(lvn, "segment_count", &seg_count)) {
		log_error("Couldn't read segment count for logical volume %s.",
			  lv->name);
		return 0;
	}

	if (seg_count != count) {
		log_error("segment_count and actual number of segments "
			  "disagree for logical volume %s.", lv->name);
		return 0;
	}

	/*
	 * Check there are no gaps or overlaps in the lv.
	 */
	if (!check_lv_segments(lv, 0))
		return_0;

	/*
	 * Merge segments in case someones been editing things by hand.
	 */
	if (!lv_merge_segments(lv))
		return_0;

	return 1;
}

static int _read_lvnames(struct format_instance *fid __attribute__((unused)),
			 struct volume_group *vg, const struct dm_config_node *lvn,
			 const struct dm_config_node *vgn __attribute__((unused)),
			 struct dm_hash_table *pv_hash __attribute__((unused)),
			 struct dm_hash_table *lv_hash)
{
	struct dm_pool *mem = vg->vgmem;
	struct logical_volume *lv;
	const char *str;
	const struct dm_config_value *cv;
	const char *hostname;
	uint64_t timestamp = 0, lvstatus;

	if (!(lv = alloc_lv(mem)))
		return_0;

	if (!link_lv_to_vg(vg, lv))
		return_0;

	if (!(lv->name = dm_pool_strdup(mem, lvn->key)))
		return_0;

	log_debug_metadata("Importing logical volume %s.", display_lvname(lv));

	if (!(lvn = lvn->child)) {
		log_error("Empty logical volume section for %s.",
			  display_lvname(lv));
		return 0;
	}

	if (!_read_flag_config(lvn, &lvstatus, LV_FLAGS)) {
		log_error("Couldn't read status flags for logical volume %s.",
			  display_lvname(lv));
		return 0;
	}

	if (lvstatus & LVM_WRITE_LOCKED) {
		lvstatus |= LVM_WRITE;
		lvstatus &= ~LVM_WRITE_LOCKED;
	}
	lv->status = lvstatus;

	if (dm_config_has_node(lvn, "creation_time")) {
		if (!_read_uint64(lvn, "creation_time", &timestamp)) {
			log_error("Invalid creation_time for logical volume %s.",
				  display_lvname(lv));
			return 0;
		}
		if (!dm_config_get_str(lvn, "creation_host", &hostname)) {
			log_error("Couldn't read creation_host for logical volume %s.",
				  display_lvname(lv));
			return 0;
		}
	} else if (dm_config_has_node(lvn, "creation_host")) {
		log_error("Missing creation_time for logical volume %s.",
			  display_lvname(lv));
		return 0;
	}

	/*
	 * The LV lock_args string is generated in lvmlockd, and the content
	 * depends on the lock_type.
	 *
	 * lock_type dlm does not use LV lock_args, so the LV lock_args field
	 * is just set to "dlm".
	 *
	 * lock_type sanlock uses the LV lock_args field to save the
	 * location on disk of that LV's sanlock lock.  The disk name is
	 * specified in the VG lock_args.  The lock_args string begins
	 * with a version number, e.g. 1.0.0, followed by a colon, followed
	 * by a number.  The number is the offset on disk where sanlock is
	 * told to find the LV's lock.
	 * e.g. lock_args = 1.0.0:70254592
	 * means that the lock is located at offset 70254592.
	 *
	 * The lvmlockd code for each specific lock manager also validates
	 * the lock_args before using it to access the lock manager.
	 */
	if (dm_config_get_str(lvn, "lock_args", &str)) {
		if (!(lv->lock_args = dm_pool_strdup(mem, str)))
			return_0;
	}

	if (dm_config_get_str(lvn, "allocation_policy", &str)) {
		lv->alloc = get_alloc_from_string(str);
		if (lv->alloc == ALLOC_INVALID) {
			log_warn("WARNING: Ignoring unrecognised allocation policy %s for LV %s.",
				 str, display_lvname(lv));
			lv->alloc = ALLOC_INHERIT;
		}
	} else
		lv->alloc = ALLOC_INHERIT;

	if (dm_config_get_str(lvn, "profile", &str)) {
		log_debug_metadata("Adding profile configuration %s for LV %s.",
				   str, display_lvname(lv));
		if (!(lv->profile = add_profile(vg->cmd, str, CONFIG_PROFILE_METADATA))) {
			log_error("Failed to add configuration profile %s for LV %s.",
				  str, display_lvname(lv));
			return 0;
		}
	}

	if (!_read_int32(lvn, "read_ahead", &lv->read_ahead))
		/* If not present, choice of auto or none is configurable */
		lv->read_ahead = vg->cmd->default_settings.read_ahead;
	else {
		switch (lv->read_ahead) {
		case 0:
			lv->read_ahead = DM_READ_AHEAD_AUTO;
			break;
		case UINT32_C(-1):
			lv->read_ahead = DM_READ_AHEAD_NONE;
			break;
		default:
			;
		}
	}

	/* Optional tags */
	if (dm_config_get_list(lvn, "tags", &cv) &&
	    !(_read_str_list(mem, &lv->tags, cv))) {
		log_error("Couldn't read tags for logical volume %s.",
			  display_lvname(lv));
		return 0;
	}

	if (!dm_hash_insert(lv_hash, lv->name, lv))
		return_0;

	if (timestamp && !lv_set_creation(lv, hostname, timestamp))
		return_0;

	if (!lv_is_visible(lv) && strstr(lv->name, "_pmspare")) {
		if (vg->pool_metadata_spare_lv) {
			log_error("Couldn't use another pool metadata spare "
				  "logical volume %s.", display_lvname(lv));
			return 0;
		}
		log_debug_metadata("Logical volume %s is pool metadata spare.",
				   display_lvname(lv));
		lv->status |= POOL_METADATA_SPARE;
		vg->pool_metadata_spare_lv = lv;
	}

	if (!lv_is_visible(lv) && !strcmp(lv->name, LOCKD_SANLOCK_LV_NAME)) {
		log_debug_metadata("Logical volume %s is sanlock lv.",
				   display_lvname(lv));
		lv->status |= LOCKD_SANLOCK_LV;
		vg->sanlock_lv = lv;
	}

	return 1;
}

static int _read_historical_lvnames(struct format_instance *fid __attribute__((unused)),
				     struct volume_group *vg, const struct dm_config_node *hlvn,
				     const struct dm_config_node *vgn __attribute__((unused)),
				     struct dm_hash_table *pv_hash __attribute__((unused)),
				     struct dm_hash_table *lv_hash __attribute__((unused)))
{
	struct dm_pool *mem = vg->vgmem;
	struct generic_logical_volume *glv;
	struct glv_list *glvl;
	const char *str;
	uint64_t timestamp;

	if (!(glv = dm_pool_zalloc(mem, sizeof(struct generic_logical_volume))) ||
	    !(glv->historical = dm_pool_zalloc(mem, sizeof(struct historical_logical_volume))) ||
	    !(glvl = dm_pool_zalloc(mem, sizeof(struct glv_list)))) {
		log_error("Removed logical volume structure allocation failed");
		goto bad;
	}

	glv->is_historical = 1;
	glv->historical->vg = vg;
	dm_list_init(&glv->historical->indirect_glvs);

	if (!(glv->historical->name = dm_pool_strdup(mem, hlvn->key)))
		goto_bad;

	if (!(hlvn = hlvn->child)) {
		log_error("Empty removed logical volume section.");
		goto bad;
	}

	if (!_read_id(&glv->historical->lvid.id[1], hlvn, "id")) {
		log_error("Couldn't read uuid for removed logical volume %s in vg %s.",
			  glv->historical->name, vg->name);
		return 0;
	}
	memcpy(&glv->historical->lvid.id[0], &glv->historical->vg->id, sizeof(glv->historical->lvid.id[0]));

	if (dm_config_get_str(hlvn, "name", &str)) {
		if (!(glv->historical->name = dm_pool_strdup(mem, str)))
			goto_bad;
	}

	if (dm_config_has_node(hlvn, "creation_time")) {
		if (!_read_uint64(hlvn, "creation_time", &timestamp)) {
			log_error("Invalid creation_time for removed logical volume %s.", str);
			goto bad;
		}
		glv->historical->timestamp = timestamp;
	}

	if (dm_config_has_node(hlvn, "removal_time")) {
		if (!_read_uint64(hlvn, "removal_time", &timestamp)) {
			log_error("Invalid removal_time for removed logical volume %s.", str);
			goto bad;
		}
		glv->historical->timestamp_removed = timestamp;
	}

	glvl->glv = glv;
	dm_list_add(&vg->historical_lvs, &glvl->list);

	return 1;
bad:
	if (glv)
		dm_pool_free(mem, glv);
	return 0;
}

static int _read_historical_lvnames_interconnections(struct format_instance *fid __attribute__((unused)),
						 struct volume_group *vg, const struct dm_config_node *hlvn,
						 const struct dm_config_node *vgn __attribute__((unused)),
						 struct dm_hash_table *pv_hash __attribute__((unused)),
						 struct dm_hash_table *lv_hash __attribute__((unused)))
{
	struct dm_pool *mem = vg->vgmem;
	const char *historical_lv_name, *origin_name = NULL;
	struct generic_logical_volume *glv, *origin_glv, *descendant_glv;
	struct logical_volume *tmp_lv;
	struct glv_list *glvl = NULL;
	const struct dm_config_value *descendants = NULL;

	historical_lv_name = hlvn->key;
	hlvn = hlvn->child;

	if (!(glv = find_historical_glv(vg, historical_lv_name, 0, NULL))) {
		log_error("Unknown historical logical volume %s/%s%s",
			  vg->name, HISTORICAL_LV_PREFIX, historical_lv_name);
		goto bad;
	}

	if (dm_config_has_node(hlvn, "origin")) {
		if (!dm_config_get_str(hlvn, "origin", &origin_name)) {
			log_error("Couldn't read origin for historical logical "
				  "volume %s/%s%s", vg->name, HISTORICAL_LV_PREFIX, historical_lv_name);
			goto bad;
		}
	}

	if (dm_config_has_node(hlvn, "descendants")) {
		if (!dm_config_get_list(hlvn, "descendants", &descendants)) {
			log_error("Couldn't get descendants list for historical logical "
				  "volume %s/%s%s", vg->name, HISTORICAL_LV_PREFIX, historical_lv_name);
			goto bad;
		}
		if (descendants->type == DM_CFG_EMPTY_ARRAY) {
			log_error("Found empty descendants list for historical logical "
				  "volume %s/%s%s", vg->name, HISTORICAL_LV_PREFIX, historical_lv_name);
			goto bad;
		}
	}

	if (!origin_name && !descendants)
		/* no interconnections */
		return 1;

	if (origin_name) {
		if (!(glvl = dm_pool_zalloc(mem, sizeof(struct glv_list)))) {
			log_error("Failed to allocate list item for historical logical "
				  "volume %s/%s%s", vg->name, HISTORICAL_LV_PREFIX, historical_lv_name);
			goto bad;
		}
		glvl->glv = glv;

		if (!strncmp(origin_name, HISTORICAL_LV_PREFIX, strlen(HISTORICAL_LV_PREFIX))) {
			if (!(origin_glv = find_historical_glv(vg, origin_name + strlen(HISTORICAL_LV_PREFIX), 0, NULL))) {
				log_error("Unknown origin %s for historical logical volume %s/%s%s",
					  origin_name, vg->name, HISTORICAL_LV_PREFIX, historical_lv_name);
				goto bad;
			}
		} else {
			if (!(tmp_lv = find_lv(vg, origin_name))) {
				log_error("Unknown origin %s for historical logical volume %s/%s%s",
					  origin_name, vg->name, HISTORICAL_LV_PREFIX, historical_lv_name);
				goto bad;
			}

			if (!(origin_glv = get_or_create_glv(mem, tmp_lv, NULL)))
				goto bad;
		}

		glv->historical->indirect_origin = origin_glv;
		if (origin_glv->is_historical)
			dm_list_add(&origin_glv->historical->indirect_glvs, &glvl->list);
		else
			dm_list_add(&origin_glv->live->indirect_glvs, &glvl->list);
	}

	if (descendants) {
		do {
			if (descendants->type != DM_CFG_STRING) {
				log_error("Descendant value for historical logical volume %s/%s%s "
					  "is not a string.", vg->name, HISTORICAL_LV_PREFIX, historical_lv_name);
				goto bad;
			}

			if (!(tmp_lv = find_lv(vg, descendants->v.str))) {
				log_error("Failed to find descendant %s for historical LV %s.",
					  descendants->v.str, historical_lv_name);
				goto bad;
			}

			if (!(descendant_glv = get_or_create_glv(mem, tmp_lv, NULL)))
				goto bad;

			if (!add_glv_to_indirect_glvs(mem, glv, descendant_glv))
				goto bad;

			descendants = descendants->next;
		} while (descendants);
	}

	return 1;
bad:
	if (glvl)
		dm_pool_free(mem, glvl);
	return 0;
}

static int _read_lvsegs(struct format_instance *fid,
			struct volume_group *vg, const struct dm_config_node *lvn,
			const struct dm_config_node *vgn __attribute__((unused)),
			struct dm_hash_table *pv_hash,
			struct dm_hash_table *lv_hash)
{
	struct logical_volume *lv;

	if (!(lv = dm_hash_lookup(lv_hash, lvn->key))) {
		log_error("Lost logical volume reference %s", lvn->key);
		return 0;
	}

	if (!(lvn = lvn->child)) {
		log_error("Empty logical volume section.");
		return 0;
	}

	/* FIXME: read full lvid */
	if (!_read_id(&lv->lvid.id[1], lvn, "id")) {
		log_error("Couldn't read uuid for logical volume %s.",
			  display_lvname(lv));
		return 0;
	}

	memcpy(&lv->lvid.id[0], &lv->vg->id, sizeof(lv->lvid.id[0]));

	if (!_read_segments(lv, lvn, pv_hash))
		return_0;

	lv->size = (uint64_t) lv->le_count * (uint64_t) vg->extent_size;
	lv->minor = -1;
	lv->major = -1;

	if (lv->status & FIXED_MINOR) {
		if (!_read_int32(lvn, "minor", &lv->minor)) {
			log_error("Couldn't read minor number for logical volume %s.",
				  display_lvname(lv));
			return 0;
		}

		if (!dm_config_has_node(lvn, "major"))
			/* If major is missing, pick default */
			lv->major = vg->cmd->dev_types->device_mapper_major;
		else if (!_read_int32(lvn, "major", &lv->major)) {
			log_warn("WARNING: Couldn't read major number for logical "
				 "volume %s.", display_lvname(lv));
			lv->major = vg->cmd->dev_types->device_mapper_major;
		}

		if (!validate_major_minor(vg->cmd, fid->fmt, lv->major, lv->minor)) {
			log_warn("WARNING: Ignoring invalid major, minor number for "
				 "logical volume %s.", display_lvname(lv));
			lv->major = lv->minor = -1;
		}
	}

	return 1;
}

static int _read_sections(struct format_instance *fid,
			  const char *section, section_fn fn,
			  struct volume_group *vg, const struct dm_config_node *vgn,
			  struct dm_hash_table *pv_hash,
			  struct dm_hash_table *lv_hash,
			  int optional)
{
	const struct dm_config_node *n;

	if (!dm_config_get_section(vgn, section, &n)) {
		if (!optional) {
			log_error("Couldn't find section '%s'.", section);
			return 0;
		}

		return 1;
	}

	for (n = n->child; n; n = n->sib) {
		if (!fn(fid, vg, n, vgn, pv_hash, lv_hash))
			return_0;
	}

	return 1;
}

static struct volume_group *_read_vg(struct format_instance *fid,
				     const struct dm_config_tree *cft,
				     unsigned allow_lvmetad_extensions)
{
	const struct dm_config_node *vgn;
	const struct dm_config_value *cv;
	const char *str, *format_str, *system_id;
	struct volume_group *vg;
	struct dm_hash_table *pv_hash = NULL, *lv_hash = NULL;
	uint64_t vgstatus;

	/* skip any top-level values */
	for (vgn = cft->root; (vgn && vgn->v); vgn = vgn->sib)
		;

	if (!vgn) {
		log_error("Couldn't find volume group in file.");
		return NULL;
	}

	if (!(vg = alloc_vg("read_vg", fid->fmt->cmd, vgn->key)))
		return_NULL;

	/*
	 * The pv hash memorises the pv section names -> pv
	 * structures.
	 */
	if (!(pv_hash = dm_hash_create(64))) {
		log_error("Couldn't create pv hash table.");
		goto bad;
	}

	/*
	 * The lv hash memorises the lv section names -> lv
	 * structures.
	 */
	if (!(lv_hash = dm_hash_create(1024))) {
		log_error("Couldn't create lv hash table.");
		goto bad;
	}

	vgn = vgn->child;

	/* A backup file might be a backup of a different format */
	if (dm_config_get_str(vgn, "format", &format_str) &&
	    !(vg->original_fmt = get_format_by_name(fid->fmt->cmd, format_str))) {
		log_error("Unrecognised format %s for volume group %s.", format_str, vg->name);
		goto bad;
	}

	if (dm_config_get_str(vgn, "lock_type", &str)) {
		if (!(vg->lock_type = dm_pool_strdup(vg->vgmem, str)))
			goto bad;
	}

	/*
	 * The VG lock_args string is generated in lvmlockd, and the content
	 * depends on the lock_type.  lvmlockd begins the lock_args string
	 * with a version number, e.g. 1.0.0, followed by a colon, followed
	 * by a string that depends on the lock manager.  The string after
	 * the colon is information needed to use the lock manager for the VG.
	 *
	 * For sanlock, the string is the name of the internal LV used to store
	 * sanlock locks.  lvmlockd needs to know where the locks are located
	 * so it can pass that location to sanlock which needs to access the locks.
	 * e.g. lock_args = 1.0.0:lvmlock
	 * means that the locks are located on the the LV "lvmlock".
	 *
	 * For dlm, the string is the dlm cluster name.  lvmlockd needs to use
	 * a dlm lockspace in this cluster to use the VG.
	 * e.g. lock_args = 1.0.0:foo
	 * means that the host needs to be a member of the cluster "foo".
	 *
	 * The lvmlockd code for each specific lock manager also validates
	 * the lock_args before using it to access the lock manager.
	 */
	if (dm_config_get_str(vgn, "lock_args", &str)) {
		if (!(vg->lock_args = dm_pool_strdup(vg->vgmem, str)))
			goto bad;
	}

	if (!_read_id(&vg->id, vgn, "id")) {
		log_error("Couldn't read uuid for volume group %s.", vg->name);
		goto bad;
	}

	if (!_read_int32(vgn, "seqno", &vg->seqno)) {
		log_error("Couldn't read 'seqno' for volume group %s.",
			  vg->name);
		goto bad;
	}

	if (!_read_flag_config(vgn, &vgstatus, VG_FLAGS)) {
		log_error("Error reading flags of volume group %s.",
			  vg->name);
		goto bad;
	}

	if (dm_config_get_str(vgn, "system_id", &system_id)) {
		if (!(vg->system_id = dm_pool_strdup(vg->vgmem, system_id))) {
			log_error("Failed to allocate memory for system_id in _read_vg.");
			goto bad;
		}
	}

	if (vgstatus & LVM_WRITE_LOCKED) {
		vgstatus |= LVM_WRITE;
		vgstatus &= ~LVM_WRITE_LOCKED;
	}
	vg->status = vgstatus;

	if (!_read_int32(vgn, "extent_size", &vg->extent_size)) {
		log_error("Couldn't read extent size for volume group %s.",
			  vg->name);
		goto bad;
	}

	/*
	 * 'extent_count' and 'free_count' get filled in
	 * implicitly when reading in the pv's and lv's.
	 */

	if (!_read_int32(vgn, "max_lv", &vg->max_lv)) {
		log_error("Couldn't read 'max_lv' for volume group %s.",
			  vg->name);
		goto bad;
	}

	if (!_read_int32(vgn, "max_pv", &vg->max_pv)) {
		log_error("Couldn't read 'max_pv' for volume group %s.",
			  vg->name);
		goto bad;
	}

	if (dm_config_get_str(vgn, "allocation_policy", &str)) {
		vg->alloc = get_alloc_from_string(str);
		if (vg->alloc == ALLOC_INVALID) {
			log_warn("WARNING: Ignoring unrecognised allocation policy %s for VG %s", str, vg->name);
			vg->alloc = ALLOC_NORMAL;
		}
	}

	if (dm_config_get_str(vgn, "profile", &str)) {
		log_debug_metadata("Adding profile configuration %s for VG %s.", str, vg->name);
		vg->profile = add_profile(vg->cmd, str, CONFIG_PROFILE_METADATA);
		if (!vg->profile) {
			log_error("Failed to add configuration profile %s for VG %s", str, vg->name);
			goto bad;
		}
	}

	if (!_read_uint32(vgn, "metadata_copies", &vg->mda_copies)) {
		vg->mda_copies = DEFAULT_VGMETADATACOPIES;
	}

	if (!_read_sections(fid, "physical_volumes", _read_pv, vg,
			    vgn, pv_hash, lv_hash, 0)) {
		log_error("Couldn't find all physical volumes for volume "
			  "group %s.", vg->name);
		goto bad;
	}

	/* Optional tags */
	if (dm_config_get_list(vgn, "tags", &cv) &&
	    !(_read_str_list(vg->vgmem, &vg->tags, cv))) {
		log_error("Couldn't read tags for volume group %s.", vg->name);
		goto bad;
	}

	if (!_read_sections(fid, "logical_volumes", _read_lvnames, vg,
			    vgn, pv_hash, lv_hash, 1)) {
		log_error("Couldn't read all logical volume names for volume "
			  "group %s.", vg->name);
		goto bad;
	}

	if (!_read_sections(fid, "historical_logical_volumes", _read_historical_lvnames, vg,
			    vgn, pv_hash, lv_hash, 1)) {
		log_error("Couldn't read all historical logical volumes for volume "
			  "group %s.", vg->name);
		goto bad;
	}

	if (!_read_sections(fid, "logical_volumes", _read_lvsegs, vg,
			    vgn, pv_hash, lv_hash, 1)) {
		log_error("Couldn't read all logical volumes for "
			  "volume group %s.", vg->name);
		goto bad;
	}

	if (!_read_sections(fid, "historical_logical_volumes", _read_historical_lvnames_interconnections,
			    vg, vgn, pv_hash, lv_hash, 1)) {
		log_error("Couldn't read all removed logical volume interconnections "
			  "for volume group %s.", vg->name);
		goto bad;
	}

	if (!fixup_imported_mirrors(vg)) {
		log_error("Failed to fixup mirror pointers after import for "
			  "volume group %s.", vg->name);
		goto bad;
	}

	dm_hash_destroy(pv_hash);
	dm_hash_destroy(lv_hash);

	vg_set_fid(vg, fid);

	/*
	 * Finished.
	 */
	return vg;

      bad:
	if (pv_hash)
		dm_hash_destroy(pv_hash);

	if (lv_hash)
		dm_hash_destroy(lv_hash);

	release_vg(vg);
	return NULL;
}

static void _read_desc(struct dm_pool *mem,
		       const struct dm_config_tree *cft, time_t *when, char **desc)
{
	const char *str;
	unsigned int u = 0u;

	if (!dm_config_get_str(cft->root, "description", &str))
		str = "";

	*desc = dm_pool_strdup(mem, str);

	(void) dm_config_get_uint32(cft->root, "creation_time", &u);
	*when = u;
}

/*
 * It is used to read vgsummary information about a VG
 * before locking and reading the VG via vg_read().
 * read_vgsummary: read VG metadata before VG is locked
 *                 and save the data in struct vgsummary
 * read_vg: read VG metadata after VG is locked
 *          and save the data in struct volume_group
 * FIXME: why are these separate?
 */
static int _read_vgsummary(const struct format_type *fmt, const struct dm_config_tree *cft, 
			   struct lvmcache_vgsummary *vgsummary)
{
	const struct dm_config_node *vgn;
	struct dm_pool *mem = fmt->cmd->mem;
	const char *str;

	if (!dm_config_get_str(cft->root, "creation_host", &str))
		str = "";

	if (!(vgsummary->creation_host = dm_pool_strdup(mem, str)))
		return_0;

	/* skip any top-level values */
	for (vgn = cft->root; (vgn && vgn->v); vgn = vgn->sib) ;

	if (!vgn) {
		log_error("Couldn't find volume group in file.");
		return 0;
	}

	if (!(vgsummary->vgname = dm_pool_strdup(mem, vgn->key)))
		return_0;

	vgn = vgn->child;

	if (!_read_id(&vgsummary->vgid, vgn, "id")) {
		log_error("Couldn't read uuid for volume group %s.", vgsummary->vgname);
		return 0;
	}

	if (!_read_flag_config(vgn, &vgsummary->vgstatus, VG_FLAGS)) {
		log_error("Couldn't find status flags for volume group %s.",
			  vgsummary->vgname);
		return 0;
	}

	if (dm_config_get_str(vgn, "system_id", &str) &&
	    (!(vgsummary->system_id = dm_pool_strdup(mem, str))))
		return_0;

	if (dm_config_get_str(vgn, "lock_type", &str) &&
	    (!(vgsummary->lock_type = dm_pool_strdup(mem, str))))
		return_0;

	if (!_read_int32(vgn, "seqno", &vgsummary->seqno)) {
		log_error("Couldn't read seqno for volume group %s.",
			  vgsummary->vgname);
		return 0;
	}

	return 1;
}

static struct text_vg_version_ops _vsn1_ops = {
	.check_version = _vsn1_check_version,
	.read_vg = _read_vg,
	.read_desc = _read_desc,
	.read_vgsummary = _read_vgsummary
};

struct text_vg_version_ops *text_vg_vsn1_init(void)
{
	return &_vsn1_ops;
}
