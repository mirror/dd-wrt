/*
 * Copyright (C) 2011-2017 Red Hat, Inc. All rights reserved.
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

#include "base/memory/zalloc.h"
#include "lib/misc/lib.h"
#include "lib/metadata/segtype.h"
#include "lib/display/display.h"
#include "lib/format_text/text_export.h"
#include "lib/config/config.h"
#include "lib/datastruct/str_list.h"
#include "lib/activate/targets.h"
#include "lib/misc/lvm-string.h"
#include "lib/activate/activate.h"
#include "lib/metadata/metadata.h"
#include "lib/metadata/lv_alloc.h"

static int _raid_target_present(struct cmd_context *cmd,
				const struct lv_segment *seg __attribute__((unused)),
				unsigned *attributes);

static void _raid_display(const struct lv_segment *seg)
{
	unsigned s;

	for (s = 0; s < seg->area_count; ++s) {
		log_print("  Raid Data LV%2d", s);
		display_stripe(seg, s, "    ");
	}

	if (seg->meta_areas)
		for (s = 0; s < seg->area_count; ++s)
			if (seg_metalv(seg, s))
				log_print("  Raid Metadata LV%2d\t%s", s, seg_metalv(seg, s)->name);

	log_print(" ");
}

static int _raid_text_import_area_count(const struct dm_config_node *sn,
					uint32_t *area_count)
{
	uint32_t stripe_count = 0, device_count = 0;
	int stripe_count_found, device_count_found;

	device_count_found = dm_config_get_uint32(sn, "device_count", &device_count);
	stripe_count_found = dm_config_get_uint32(sn, "stripe_count", &stripe_count);

	if (!device_count_found && !stripe_count_found) {
		log_error("Couldn't read 'device_count' or 'stripe_count' for "
			  "segment '%s'.", dm_config_parent_name(sn));
		return 0;
	}

	if (device_count_found && stripe_count_found) {
		log_error("Only one of 'device_count' and 'stripe_count' allowed for "
			  "segment '%s'.", dm_config_parent_name(sn));
		return 0;
	}

	*area_count = stripe_count + device_count;

	return 1;
}

static int _raid_text_import_areas(struct lv_segment *seg,
				   const struct dm_config_node *sn,
				   const struct dm_config_value *cv)
{
	unsigned int s;
	struct logical_volume *lv;
	const char *seg_name = dm_config_parent_name(sn);

	if (!seg->area_count) {
		log_error("No areas found for segment %s", seg_name);
		return 0;
	}

	for (s = 0; cv && s < seg->area_count; s++, cv = cv->next) {
		if (cv->type != DM_CFG_STRING) {
			log_error("Bad volume name in areas array for segment %s.", seg_name);
			return 0;
		}

		/* Metadata device comes first. */
		if (!(lv = find_lv(seg->lv->vg, cv->v.str))) {
			log_error("Couldn't find volume '%s' for segment '%s'.",
				  cv->v.str ? : "NULL", seg_name);
			return 0;
		}

		if (strstr(lv->name, "_rmeta_")) {
			if (!set_lv_segment_area_lv(seg, s, lv, 0, RAID_META))
				return_0;
			cv = cv->next;
		}

		if (!cv) {
			log_error("Missing data device in areas array for segment %s.", seg_name);
			return 0;
		}

		/* Data device comes second */
		if (!(lv = find_lv(seg->lv->vg, cv->v.str))) {
			log_error("Couldn't find volume '%s' for segment '%s'.",
				  cv->v.str ? : "NULL", seg_name);
			return 0;
		}
		if (!set_lv_segment_area_lv(seg, s, lv, 0, RAID_IMAGE))
			return_0;
	}

	/*
	 * Check we read the correct number of RAID data/meta pairs.
	 */
	if (cv || (s < seg->area_count)) {
		log_error("Incorrect number of areas in area array "
			  "for segment '%s'.", seg_name);
		return 0;
	}

	return 1;
}

static int _raid_text_import(struct lv_segment *seg,
			     const struct dm_config_node *sn,
			     struct dm_hash_table *pv_hash)
{
	const struct dm_config_value *cv;
	const struct {
		const char *name;
		uint32_t *var;
	} raid_attr_import[] = {
		{ "region_size",	&seg->region_size },
		{ "stripe_size",	&seg->stripe_size },
		{ "data_copies",	&seg->data_copies },
		{ "writebehind",	&seg->writebehind },
		{ "min_recovery_rate",	&seg->min_recovery_rate },
		{ "max_recovery_rate",	&seg->max_recovery_rate },
		{ "data_offset",	&seg->data_offset },
	}, *aip = raid_attr_import;
	unsigned i;

	for (i = 0; i < DM_ARRAY_SIZE(raid_attr_import); i++, aip++) {
		if (dm_config_has_node(sn, aip->name)) {
			if (!dm_config_get_uint32(sn, aip->name, aip->var)) {
				if (!strcmp(aip->name, "data_copies") ||
				    !strcmp(aip->name, "data_offset")) {
					*aip->var = 0;
					continue;
				}
				log_error("Couldn't read '%s' for segment %s of logical volume %s.",
					  aip->name, dm_config_parent_name(sn), seg->lv->name);
				return 0;
			}

			if (!strcmp(aip->name, "data_offset") && !*aip->var)
				*aip->var = 1;
		}
	}

	if (!dm_config_get_list(sn, seg_is_raid0(seg) ? "raid0_lvs" : "raids", &cv)) {
		log_error("Couldn't find RAID array for "
			  "segment %s of logical volume %s.",
			  dm_config_parent_name(sn), seg->lv->name);
		return 0;
	}

	if (!_raid_text_import_areas(seg, sn, cv)) {
		log_error("Failed to import RAID component pairs.");
		return 0;
	}

	if (seg->data_copies < 2)
		seg->data_copies = lv_raid_data_copies(seg->segtype, seg->area_count);

	if (seg_is_any_raid0(seg))
		seg->area_len /= seg->area_count;

	return 1;
}

static int _raid_text_export_raid0(const struct lv_segment *seg, struct formatter *f)
{
	outf(f, "stripe_count = %u", seg->area_count);

	if (seg->stripe_size)
		outf(f, "stripe_size = %" PRIu32, seg->stripe_size);

	return out_areas(f, seg, seg_is_raid0(seg) ? "raid0_lv" : "raid");
}

static int _raid_text_export_raid(const struct lv_segment *seg, struct formatter *f)
{
	int raid0 = seg_is_any_raid0(seg);

	if (raid0)
		outfc(f, (seg->area_count == 1) ? "# linear" : NULL,
		      "stripe_count = %u", seg->area_count);

	else {
		outf(f, "device_count = %u", seg->area_count);
		if (seg_is_any_raid10(seg) && seg->data_copies > 0)
			outf(f, "data_copies = %" PRIu32, seg->data_copies);
		if (seg->region_size)
			outf(f, "region_size = %" PRIu32, seg->region_size);
	}

	if (seg->stripe_size)
		outf(f, "stripe_size = %" PRIu32, seg->stripe_size);

	if (!raid0) {
		if (seg_is_raid1(seg) && seg->writebehind)
			outf(f, "writebehind = %" PRIu32, seg->writebehind);
		if (seg->min_recovery_rate)
			outf(f, "min_recovery_rate = %" PRIu32, seg->min_recovery_rate);
		if (seg->max_recovery_rate)
			outf(f, "max_recovery_rate = %" PRIu32, seg->max_recovery_rate);
		if (seg->data_offset)
			outf(f, "data_offset = %" PRIu32, seg->data_offset == 1 ? 0 : seg->data_offset);
	}

	return out_areas(f, seg, "raid");
}

static int _raid_text_export(const struct lv_segment *seg, struct formatter *f)
{
	if (seg_is_any_raid0(seg))
		return _raid_text_export_raid0(seg, f);

	return _raid_text_export_raid(seg, f);
}

static int _raid_add_target_line(struct dev_manager *dm __attribute__((unused)),
				 struct dm_pool *mem __attribute__((unused)),
				 struct cmd_context *cmd __attribute__((unused)),
				 void **target_state __attribute__((unused)),
				 struct lv_segment *seg,
				 const struct lv_activate_opts *laopts __attribute__((unused)),
				 struct dm_tree_node *node, uint64_t len,
				 uint32_t *pvmove_mirror_count __attribute__((unused)))
{
	int delta_disks = 0, delta_disks_minus = 0, delta_disks_plus = 0, data_offset = 0;
	uint32_t s;
	uint64_t flags = 0;
	uint64_t rebuilds[RAID_BITMAP_SIZE] = { 0 };
	uint64_t writemostly[RAID_BITMAP_SIZE] = { 0 };
	struct dm_tree_node_raid_params_v2 params = { 0 };
	unsigned attrs;

	if (seg_is_raid4(seg)) {
		if (!_raid_target_present(cmd, NULL, &attrs) ||
		    !(attrs & RAID_FEATURE_RAID4)) {
			log_error("RAID target does not support RAID4 for LV %s.",
				  display_lvname(seg->lv));
			return 0;
		}
	}

	if (!seg->area_count) {
		log_error(INTERNAL_ERROR "_raid_add_target_line called "
			  "with no areas for %s.", seg->lv->name);
		return 0;
	}

	/*
	 * 253 device restriction imposed by kernel due to MD and dm-raid bitfield limitation in superblock.
	 * It is not strictly a userspace limitation.
	 */
	if (seg->area_count > DEFAULT_RAID_MAX_IMAGES) {
		log_error("Unable to handle more than %u devices in a "
			  "single RAID array", DEFAULT_RAID_MAX_IMAGES);
		return 0;
	}

	if (!seg_is_any_raid0(seg)) {
		if (!seg->region_size) {
			log_error("Missing region size for raid segment in %s.",
				  seg_lv(seg, 0)->name);
			return 0;
		}

		for (s = 0; s < seg->area_count; s++) {
			uint64_t status = seg_lv(seg, s)->status;

			if (status & LV_REBUILD)
				rebuilds[s/64] |= 1ULL << (s%64);

			if (status & LV_RESHAPE_DELTA_DISKS_PLUS) {
				delta_disks++;
				delta_disks_plus++;
			} else if (status & LV_RESHAPE_DELTA_DISKS_MINUS) {
				delta_disks--;
				delta_disks_minus++;
			}

			if (delta_disks_plus && delta_disks_minus) {
				log_error(INTERNAL_ERROR "Invalid request for delta disks minus and delta disks plus!");
				return 0;
			}

			if (status & LV_WRITEMOSTLY)
				writemostly[s/64] |= 1ULL << (s%64);
		}

		data_offset = seg->data_offset;

		if (mirror_in_sync())
			flags = DM_NOSYNC;
	}

	params.raid_type = lvseg_name(seg);

	if (seg->segtype->parity_devs) {
		/* RAID 4/5/6 */
		params.mirrors = 1;
		params.stripes = seg->area_count - seg->segtype->parity_devs;
	} else if (seg_is_any_raid0(seg)) {
		params.mirrors = 1;
		params.stripes = seg->area_count;
	} else if (seg_is_any_raid10(seg)) {
		params.data_copies = seg->data_copies;
		params.stripes = seg->area_count;
	} else {
		/* RAID 1 */
		params.mirrors = seg->data_copies;
		params.stripes = 1;
		params.writebehind = seg->writebehind;
		memcpy(params.writemostly, writemostly, sizeof(params.writemostly));
	}

	/* RAID 0 doesn't have a bitmap, thus no region_size, rebuilds etc. */
	if (!seg_is_any_raid0(seg)) {
		params.region_size = seg->region_size;
		memcpy(params.rebuilds, rebuilds, sizeof(params.rebuilds));
		params.min_recovery_rate = seg->min_recovery_rate;
		params.max_recovery_rate = seg->max_recovery_rate;
		params.delta_disks = delta_disks;
		params.data_offset = data_offset;
	}

	params.stripe_size = seg->stripe_size;
	params.flags = flags;

	if (!dm_tree_node_add_raid_target_with_params_v2(node, len, &params))
		return_0;

	return add_areas_line(dm, seg, node, 0u, seg->area_count);
}

static int _raid_target_status_compatible(const char *type)
{
	return (strstr(type, "raid") != NULL);
}

static void _raid_destroy(struct segment_type *segtype)
{
	free((void *) segtype->dso);
	free(segtype);
}

#ifdef DEVMAPPER_SUPPORT
static int _raid_target_percent(void **target_state,
				dm_percent_t *percent,
				struct dm_pool *mem,
				struct cmd_context *cmd,
				struct lv_segment *seg, char *params,
				uint64_t *total_numerator,
				uint64_t *total_denominator)
{
	struct dm_status_raid *sr;

	if (!dm_get_status_raid(mem, params, &sr))
		return_0;

	*total_numerator += sr->insync_regions;
	*total_denominator += sr->total_regions;

	if (seg)
		seg->extents_copied = (uint64_t) seg->area_len
			* dm_make_percent(sr->insync_regions , sr->total_regions) / DM_PERCENT_100;

	*percent = dm_make_percent(sr->insync_regions, sr->total_regions);

	dm_pool_free(mem, sr);

	return 1;
}

static int _raid_transient_status(struct dm_pool *mem,
				  struct lv_segment *seg,
				  char *params)
{
	int failed = 0, r = 0;
	unsigned i;
	struct lvinfo info;
	struct logical_volume *lv;
	struct dm_status_raid *sr;

	log_debug("Raid transient status %s.", params);

	if (!dm_get_status_raid(mem, params, &sr))
		return_0;

	if (sr->dev_count != seg->area_count) {
		log_error("Active raid has a wrong number of raid images!");
		log_error("Metadata says %u, kernel says %u.",
			  seg->area_count, sr->dev_count);
		goto out;
	}

	if (seg->meta_areas)
		for (i = 0; i < seg->area_count; ++i) {
			lv = seg_metalv(seg, i);
			if (!lv_info(lv->vg->cmd, lv, 0, &info, 0, 0)) {
				log_error("Check for existence of raid meta %s failed.",
					  display_lvname(lv));
				goto out;
			}
		}

	for (i = 0; i < seg->area_count; ++i) {
		lv = seg_lv(seg, i);
		if (!lv_info(lv->vg->cmd, lv, 0, &info, 0, 0)) {
			log_error("Check for existence of raid image %s failed.",
				  display_lvname(lv));
			goto out;
		}
		if (sr->dev_health[i] == 'D') {
			lv->status |= PARTIAL_LV;
			++failed;
		}
	}

	/* Update PARTIAL_LV flags across the VG */
	if (failed)
		vg_mark_partial_lvs(lv->vg, 0);

	r = 1;
out:
	dm_pool_free(mem, sr);

	return r;
}

/* Define raid feature based on the tuple(major, minor, patchlevel) of raid target */
struct raid_feature {
	uint32_t maj;
	uint32_t min;
	uint32_t patchlevel;
	unsigned raid_feature;
	const char *feature;
};

/* Return true if tuple(@maj, @min, @patchlevel) is greater/equal to @*feature members */
static int _check_feature(const struct raid_feature *feature, uint32_t maj, uint32_t min, uint32_t patchlevel)
{
	return (maj > feature->maj) ||
	       (maj == feature->maj && min > feature->min) ||
	       (maj == feature->maj && min == feature->min && patchlevel >= feature->patchlevel);
}

static int _raid_target_present(struct cmd_context *cmd,
				const struct lv_segment *seg __attribute__((unused)),
				unsigned *attributes)
{
	/* List of features with their kernel target version */
	const struct raid_feature _features[] = {
		{ 1, 3, 0, RAID_FEATURE_RAID10, SEG_TYPE_NAME_RAID10 },
		{ 1, 7, 0, RAID_FEATURE_RAID0, SEG_TYPE_NAME_RAID0 },
		{ 1, 9, 0, RAID_FEATURE_SHRINK, "shrinking" },
		{ 1, 9, 0, RAID_FEATURE_NEW_DEVICES_ACCEPT_REBUILD, "rebuild+emptymeta" },
		{ 1, 12, 0, RAID_FEATURE_RESHAPE, "reshaping" },
	};

	static int _raid_checked = 0;
	static int _raid_present = 0;
	static unsigned _raid_attrs = 0;
	uint32_t maj, min, patchlevel;
	unsigned i;

	if (!activation())
		return 0;

	if (!_raid_checked) {
		_raid_checked = 1;

		if (!(_raid_present = target_present(cmd, TARGET_NAME_RAID, 1)))
			return 0;

		if (!target_version("raid", &maj, &min, &patchlevel))
			return_0;

		for (i = 0; i < DM_ARRAY_SIZE(_features); ++i)
			if (_check_feature(_features + i, maj, min, patchlevel))
				_raid_attrs |= _features[i].raid_feature;
			else
				log_very_verbose("Target raid does not support %s.",
						 _features[i].feature);

		/*
		 * Seperate check for proper raid4 mapping supported
		 *
		 * If we get more of these range checks, avoid them
		 * altogether by enhancing 'struct raid_feature'
		 * and _check_feature() to handle them.
		 */
		if (!(maj == 1 && (min == 8 || (min == 9 && patchlevel == 0))))
			_raid_attrs |= RAID_FEATURE_RAID4;
		else
			log_very_verbose("Target raid does not support %s.",
					 SEG_TYPE_NAME_RAID4);
	}

	if (attributes)
		*attributes = _raid_attrs;

	return _raid_present;
}

static int _raid_modules_needed(struct dm_pool *mem,
				const struct lv_segment *seg __attribute__((unused)),
				struct dm_list *modules)
{
	if (!str_list_add(mem, modules, MODULE_NAME_RAID)) {
		log_error("raid module string list allocation failed");
		return 0;
	}

	return 1;
}

#  ifdef DMEVENTD
static int _raid_target_monitored(struct lv_segment *seg, int *pending, int *monitored)
{
	return target_registered_with_dmeventd(seg->lv->vg->cmd, seg->segtype->dso,
					       seg->lv, pending, monitored);
}

static int _raid_set_events(struct lv_segment *seg, int evmask, int set)
{
	return target_register_events(seg->lv->vg->cmd, seg->segtype->dso,
				      seg->lv, evmask, set, 0);
}

static int _raid_target_monitor_events(struct lv_segment *seg, int events)
{
	return _raid_set_events(seg, events, 1);
}

static int _raid_target_unmonitor_events(struct lv_segment *seg, int events)
{
	return _raid_set_events(seg, events, 0);
}
#  endif /* DMEVENTD */
#endif /* DEVMAPPER_SUPPORT */

static struct segtype_handler _raid_ops = {
	.display = _raid_display,
	.text_import_area_count = _raid_text_import_area_count,
	.text_import = _raid_text_import,
	.text_export = _raid_text_export,
	.add_target_line = _raid_add_target_line,
	.target_status_compatible = _raid_target_status_compatible,
#ifdef DEVMAPPER_SUPPORT
	.target_percent = _raid_target_percent,
	.target_present = _raid_target_present,
	.check_transient_status = _raid_transient_status,
	.modules_needed = _raid_modules_needed,
#  ifdef DMEVENTD
	.target_monitored = _raid_target_monitored,
	.target_monitor_events = _raid_target_monitor_events,
	.target_unmonitor_events = _raid_target_unmonitor_events,
#  endif        /* DMEVENTD */
#endif
	.destroy = _raid_destroy,
};

static const struct raid_type {
	const char name[12];
	unsigned parity;
	uint64_t extra_flags;
} _raid_types[] = {
	{ SEG_TYPE_NAME_RAID0,      0, SEG_RAID0 | SEG_AREAS_STRIPED },
	{ SEG_TYPE_NAME_RAID0_META, 0, SEG_RAID0_META | SEG_AREAS_STRIPED },
	{ SEG_TYPE_NAME_RAID1,      0, SEG_RAID1 | SEG_AREAS_MIRRORED },
	{ SEG_TYPE_NAME_RAID10,     0, SEG_RAID10 | SEG_AREAS_MIRRORED },
	{ SEG_TYPE_NAME_RAID10_NEAR,0, SEG_RAID10_NEAR | SEG_AREAS_MIRRORED },
	{ SEG_TYPE_NAME_RAID4,      1, SEG_RAID4 },
	{ SEG_TYPE_NAME_RAID5,      1, SEG_RAID5 },
	{ SEG_TYPE_NAME_RAID5_N,    1, SEG_RAID5_N },
	{ SEG_TYPE_NAME_RAID5_LA,   1, SEG_RAID5_LA },
	{ SEG_TYPE_NAME_RAID5_LS,   1, SEG_RAID5_LS },
	{ SEG_TYPE_NAME_RAID5_RA,   1, SEG_RAID5_RA },
	{ SEG_TYPE_NAME_RAID5_RS,   1, SEG_RAID5_RS },
	{ SEG_TYPE_NAME_RAID6,      2, SEG_RAID6 },
	{ SEG_TYPE_NAME_RAID6_N_6,  2, SEG_RAID6_N_6 },
	{ SEG_TYPE_NAME_RAID6_NC,   2, SEG_RAID6_NC },
	{ SEG_TYPE_NAME_RAID6_NR,   2, SEG_RAID6_NR },
	{ SEG_TYPE_NAME_RAID6_ZR,   2, SEG_RAID6_ZR },
	{ SEG_TYPE_NAME_RAID6_LS_6, 2, SEG_RAID6_LS_6 },
	{ SEG_TYPE_NAME_RAID6_RS_6, 2, SEG_RAID6_RS_6 },
	{ SEG_TYPE_NAME_RAID6_LA_6, 2, SEG_RAID6_LA_6 },
	{ SEG_TYPE_NAME_RAID6_RA_6, 2, SEG_RAID6_RA_6 }
};

static struct segment_type *_init_raid_segtype(struct cmd_context *cmd,
					       const struct raid_type *rt,
					       const char *dso,
					       uint64_t monitored)
{
	struct segment_type *segtype = zalloc(sizeof(*segtype));

	if (!segtype) {
		log_error("Failed to allocate memory for %s segtype",
			  rt->name);
		return NULL;
	}

	segtype->ops = &_raid_ops;
	segtype->name = rt->name;
	segtype->flags = SEG_RAID | SEG_ONLY_EXCLUSIVE | rt->extra_flags;

	/* Never monitor raid0 or raid0_meta LVs */
	if (!segtype_is_any_raid0(segtype) &&
	    dso && (dso = strdup(dso))) {
		segtype->dso = dso;
		segtype->flags |= monitored;
	}

	segtype->parity_devs = rt->parity;

	log_very_verbose("Initialised segtype: %s", segtype->name);

	return segtype;
}

#ifdef RAID_INTERNAL /* Shared */
int init_raid_segtypes(struct cmd_context *cmd, struct segtype_library *seglib)
#else
int init_multiple_segtypes(struct cmd_context *cmd, struct segtype_library *seglib);

int init_multiple_segtypes(struct cmd_context *cmd, struct segtype_library *seglib)
#endif
{
	struct segment_type *segtype;
	char *dso = NULL;
	unsigned i;
	uint64_t monitored = 0;
	int r = 1;

#ifdef DEVMAPPER_SUPPORT
#  ifdef DMEVENTD
	dso = get_monitor_dso_path(cmd, dmeventd_raid_library_CFG);

	if (dso)
		monitored = SEG_MONITORED;
#  endif
#endif

	for (i = 0; i < DM_ARRAY_SIZE(_raid_types); ++i)
		if ((segtype = _init_raid_segtype(cmd, &_raid_types[i], dso, monitored)) &&
		    !lvm_register_segtype(seglib, segtype)) {
			/* segtype is already destroyed */
			stack;
			r = 0;
			break;
		}

	free(dso);

	return r;
}
