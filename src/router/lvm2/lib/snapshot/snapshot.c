/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2008 Red Hat, Inc. All rights reserved.
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
#include "lib/metadata/metadata.h"
#include "lib/metadata/segtype.h"
#include "lib/format_text/text_export.h"
#include "lib/config/config.h"
#include "lib/activate/activate.h"
#include "lib/datastruct/str_list.h"

#define SEG_LOG_ERROR(t, p...) \
	log_error(t " segment %s of logical volume %s.", ## p, \
		  dm_config_parent_name(sn), seg->lv->name), 0;

static const char *_snap_target_name(const struct lv_segment *seg,
				     const struct lv_activate_opts *laopts)
{
	if (!laopts->no_merging && (seg->status & MERGING))
		return TARGET_NAME_SNAPSHOT_MERGE;

	return lvseg_name(seg);
}

static int _snap_text_import(struct lv_segment *seg, const struct dm_config_node *sn,
			struct dm_hash_table *pv_hash __attribute__((unused)))
{
	uint32_t chunk_size;
	struct logical_volume *org, *cow;
	const char *org_name = NULL, *cow_name = NULL;
	int merge = 0;

	if (!dm_config_get_uint32(sn, "chunk_size", &chunk_size)) {
		log_error("Couldn't read chunk size for snapshot.");
		return 0;
	}

	if (dm_config_has_node(sn, "merging_store")) {
		if (!(cow_name = dm_config_find_str(sn, "merging_store", NULL)))
			return SEG_LOG_ERROR("Merging store must be a string in");
		merge = 1;
	}

	if (dm_config_has_node(sn, "cow_store")) {
		if (cow_name)
			return SEG_LOG_ERROR("Both snapshot cow and merging storage were specified in");

		if (!(cow_name = dm_config_find_str(sn, "cow_store", NULL)))
			return SEG_LOG_ERROR("Cow store must be a string in");
	}

	if (!cow_name)
		return SEG_LOG_ERROR("Snapshot cow storage not specified in");

	if (!dm_config_has_node(sn, "origin"))
		return SEG_LOG_ERROR("Snapshot origin not specified in");

	if (!(org_name = dm_config_find_str(sn, "origin", NULL)))
		return SEG_LOG_ERROR("Snapshot origin must be a string in");

	if (!(cow = find_lv(seg->lv->vg, cow_name)))
		return SEG_LOG_ERROR("Unknown logical volume %s specified for "
				     "snapshot cow store in", cow_name);

	if (!(org = find_lv(seg->lv->vg, org_name)))
		return SEG_LOG_ERROR("Unknown logical volume %s specified for "
			  "snapshot origin in", org_name);

	init_snapshot_seg(seg, org, cow, chunk_size, merge);

	return 1;
}

static int _snap_text_export(const struct lv_segment *seg, struct formatter *f)
{
	outf(f, "chunk_size = %u", seg->chunk_size);
	outf(f, "origin = \"%s\"", seg->origin->name);

	if (!(seg->status & MERGING))
		outf(f, "cow_store = \"%s\"", seg->cow->name);
	else
		outf(f, "merging_store = \"%s\"", seg->cow->name);

	return 1;
}

#ifdef DEVMAPPER_SUPPORT
static int _snap_target_status_compatible(const char *type)
{
	return (strcmp(type, TARGET_NAME_SNAPSHOT_MERGE) == 0);
}

static int _snap_target_percent(void **target_state __attribute__((unused)),
				dm_percent_t *percent,
				struct dm_pool *mem __attribute__((unused)),
				struct cmd_context *cmd __attribute__((unused)),
				struct lv_segment *seg __attribute__((unused)),
				char *params, uint64_t *total_numerator,
				uint64_t *total_denominator)
{
	struct dm_status_snapshot *s;

	if (!dm_get_status_snapshot(mem, params, &s))
		return_0;

	if (s->invalid)
		*percent = DM_PERCENT_INVALID;
	else if (s->merge_failed)
		*percent = LVM_PERCENT_MERGE_FAILED;
	else {
		*total_numerator += s->used_sectors;
		*total_denominator += s->total_sectors;
		if (s->has_metadata_sectors &&
		    s->used_sectors == s->metadata_sectors)
			*percent = DM_PERCENT_0;
		else if (s->used_sectors == s->total_sectors)
			*percent = DM_PERCENT_100;
		else
			*percent = dm_make_percent(*total_numerator, *total_denominator);
	}

	return 1;
}

static int _snap_target_present(struct cmd_context *cmd,
				const struct lv_segment *seg,
				unsigned *attributes)
{
	static int _snap_checked = 0;
	static int _snap_merge_checked = 0;
	static int _snap_present = 0;
	static int _snap_merge_present = 0;
	static unsigned _snap_attrs = 0;
	uint32_t maj, min, patchlevel;

	if (!activation())
		return 0;

	if (!_snap_checked) {
		_snap_checked = 1;

		if (!(_snap_present = target_present(cmd, TARGET_NAME_SNAPSHOT, 1) &&
		      target_present(cmd, TARGET_NAME_SNAPSHOT_ORIGIN, 0)))
			return 0;

		if (target_version(TARGET_NAME_SNAPSHOT, &maj, &min, &patchlevel) &&
		    (maj > 1 ||
		     (maj == 1 && (min >= 12 || (min == 10 && patchlevel >= 2)))))
			_snap_attrs |= SNAPSHOT_FEATURE_FIXED_LEAK;
		else
			log_very_verbose("Target snapshot may leak metadata.");
	}

	if (attributes)
		*attributes = _snap_attrs;

	/* TODO: test everything at once */
	if (_snap_present && seg && (seg->status & MERGING)) {
		if (!_snap_merge_checked) {
			_snap_merge_present = target_present(cmd, TARGET_NAME_SNAPSHOT_MERGE, 0);
			_snap_merge_checked = 1;
		}
		return _snap_merge_present;
	}

	return _snap_present;
}

#  ifdef DMEVENTD
/* FIXME Cache this */
static int _target_registered(struct lv_segment *seg, int *pending, int *monitored)
{
	return target_registered_with_dmeventd(seg->lv->vg->cmd,
					       seg->segtype->dso,
					       seg->cow, pending, monitored);
}

/* FIXME This gets run while suspended and performs banned operations. */
static int _target_set_events(struct lv_segment *seg, int evmask, int set)
{
	/* FIXME Make timeout (10) configurable */
	return target_register_events(seg->lv->vg->cmd, seg->segtype->dso,
				      seg->cow, evmask, set, 10);
}

static int _target_register_events(struct lv_segment *seg,
				   int events)
{
	return _target_set_events(seg, events, 1);
}

static int _target_unregister_events(struct lv_segment *seg,
				     int events)
{
	return _target_set_events(seg, events, 0);
}

#  endif /* DMEVENTD */

static int _snap_modules_needed(struct dm_pool *mem,
				const struct lv_segment *seg __attribute__((unused)),
				struct dm_list *modules)
{
	if (!str_list_add(mem, modules, MODULE_NAME_SNAPSHOT)) {
		log_error("snapshot string list allocation failed");
		return 0;
	}

	return 1;
}
#endif /* DEVMAPPER_SUPPORT */

static void _snap_destroy(struct segment_type *segtype)
{
	free((void *) segtype->dso);
	free(segtype);
}

static struct segtype_handler _snapshot_ops = {
	.target_name = _snap_target_name,
	.text_import = _snap_text_import,
	.text_export = _snap_text_export,
#ifdef DEVMAPPER_SUPPORT
	.target_status_compatible = _snap_target_status_compatible,
	.target_percent = _snap_target_percent,
	.target_present = _snap_target_present,
	.modules_needed = _snap_modules_needed,
#  ifdef DMEVENTD
	.target_monitored = _target_registered,
	.target_monitor_events = _target_register_events,
	.target_unmonitor_events = _target_unregister_events,
#  endif	/* DMEVENTD */
#endif
	.destroy = _snap_destroy,
};

#ifdef SNAPSHOT_INTERNAL
struct segment_type *init_snapshot_segtype(struct cmd_context *cmd)
#else				/* Shared */
struct segment_type *init_segtype(struct cmd_context *cmd);
struct segment_type *init_segtype(struct cmd_context *cmd)
#endif
{
	struct segment_type *segtype = zalloc(sizeof(*segtype));

	if (!segtype)
		return_NULL;

	segtype->ops = &_snapshot_ops;
	segtype->name = SEG_TYPE_NAME_SNAPSHOT;
	segtype->flags = SEG_SNAPSHOT | SEG_CANNOT_BE_ZEROED | SEG_ONLY_EXCLUSIVE;

#ifdef DEVMAPPER_SUPPORT
#  ifdef DMEVENTD
	segtype->dso = get_monitor_dso_path(cmd, dmeventd_snapshot_library_CFG);

	if (segtype->dso)
		segtype->flags |= SEG_MONITORED;
#  endif	/* DMEVENTD */
#endif
	log_very_verbose("Initialised segtype: %s", segtype->name);

	return segtype;
}
