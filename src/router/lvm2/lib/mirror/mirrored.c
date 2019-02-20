/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2015 Red Hat, Inc. All rights reserved.
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
#include "lib/commands/toolcontext.h"
#include "lib/metadata/segtype.h"
#include "lib/display/display.h"
#include "lib/format_text/text_export.h"
#include "lib/format_text/text_import.h"
#include "lib/config/config.h"
#include "lib/misc/lvm-string.h"
#include "lib/activate/targets.h"
#include "lib/activate/activate.h"
#include "lib/datastruct/str_list.h"

#include <sys/utsname.h>

enum {
	MIRR_DISABLED,
	MIRR_RUNNING,
	MIRR_COMPLETED
};

struct mirror_state {
	uint32_t default_region_size;
};

static void _mirrored_display(const struct lv_segment *seg)
{
	const char *size;
	uint32_t s;

	log_print("  Mirrors\t\t%u", seg->area_count);
	log_print("  Mirror size\t\t%u", seg->area_len);
	if (seg->log_lv)
		log_print("  Mirror log volume\t%s", seg->log_lv->name);

	if (seg->region_size) {
		size = display_size(seg->lv->vg->cmd,
				    (uint64_t) seg->region_size);
		log_print("  Mirror region size\t%s", size);
	}

	log_print("  Mirror original:");
	display_stripe(seg, 0, "    ");
	log_print("  Mirror destinations:");
	for (s = 1; s < seg->area_count; s++)
		display_stripe(seg, s, "    ");
	log_print(" ");
}

static int _mirrored_text_import_area_count(const struct dm_config_node *sn, uint32_t *area_count)
{
	if (!dm_config_get_uint32(sn, "mirror_count", area_count)) {
		log_error("Couldn't read 'mirror_count' for "
			  "segment '%s'.", dm_config_parent_name(sn));
		return 0;
	}

	return 1;
}

static int _mirrored_text_import(struct lv_segment *seg, const struct dm_config_node *sn,
			struct dm_hash_table *pv_hash)
{
	const struct dm_config_value *cv;
	const char *logname = NULL;

	if (dm_config_has_node(sn, "extents_moved")) {
		if (dm_config_get_uint32(sn, "extents_moved",
				      &seg->extents_copied))
			seg->status |= PVMOVE;
		else {
			log_error("Couldn't read 'extents_moved' for "
				  "segment %s of logical volume %s.",
				  dm_config_parent_name(sn), seg->lv->name);
			return 0;
		}
	}

	if (dm_config_has_node(sn, "region_size")) {
		if (!dm_config_get_uint32(sn, "region_size",
				      &seg->region_size)) {
			log_error("Couldn't read 'region_size' for "
				  "segment %s of logical volume %s.",
				  dm_config_parent_name(sn), seg->lv->name);
			return 0;
		}
	}

	if (dm_config_get_str(sn, "mirror_log", &logname)) {
		if (!(seg->log_lv = find_lv(seg->lv->vg, logname))) {
			log_error("Unrecognised mirror log in "
				  "segment %s of logical volume %s.",
				  dm_config_parent_name(sn), seg->lv->name);
			return 0;
		}
		seg->log_lv->status |= MIRROR_LOG;
	}

	if (logname && !seg->region_size) {
		log_error("Missing region size for mirror log for "
			  "segment %s of logical volume %s.",
			  dm_config_parent_name(sn), seg->lv->name);
		return 0;
	}

	if (!dm_config_get_list(sn, "mirrors", &cv)) {
		log_error("Couldn't find mirrors array for "
			  "segment %s of logical volume %s.",
			  dm_config_parent_name(sn), seg->lv->name);
		return 0;
	}

	return text_import_areas(seg, sn, cv, pv_hash, MIRROR_IMAGE);
}

static int _mirrored_text_export(const struct lv_segment *seg, struct formatter *f)
{
	outf(f, "mirror_count = %u", seg->area_count);
	if (seg->status & PVMOVE)
		outsize(f, (uint64_t) seg->extents_copied * seg->lv->vg->extent_size,
			"extents_moved = %" PRIu32, seg->extents_copied);
	if (seg->log_lv)
		outf(f, "mirror_log = \"%s\"", seg->log_lv->name);
	if (seg->region_size)
		outf(f, "region_size = %" PRIu32, seg->region_size);

	return out_areas(f, seg, "mirror");
}

#ifdef DEVMAPPER_SUPPORT
static int _block_on_error_available = 0;

static struct mirror_state *_mirrored_init_target(struct dm_pool *mem,
					 struct cmd_context *cmd)
{
	struct mirror_state *mirr_state;

	if (!(mirr_state = dm_pool_alloc(mem, sizeof(*mirr_state)))) {
		log_error("struct mirr_state allocation failed");
		return NULL;
	}

	mirr_state->default_region_size = get_default_region_size(cmd);

	return mirr_state;
}

static int _mirrored_target_percent(void **target_state,
				    dm_percent_t *percent,
				    struct dm_pool *mem,
				    struct cmd_context *cmd,
				    struct lv_segment *seg, char *params,
				    uint64_t *total_numerator,
				    uint64_t *total_denominator)
{
	struct dm_status_mirror *sm;

	if (!*target_state)
		*target_state = _mirrored_init_target(mem, cmd);

	if (!dm_get_status_mirror(mem, params, &sm))
		return_0;

	*total_numerator += sm->insync_regions;
	*total_denominator += sm->total_regions;

	if (seg)
		seg->extents_copied = seg->area_len * sm->insync_regions / sm->total_regions;

	*percent = dm_make_percent(sm->insync_regions, sm->total_regions);

	dm_pool_free(mem, sm);

	return 1;
}

static int _mirrored_transient_status(struct dm_pool *mem, struct lv_segment *seg, char *params)
{
	struct dm_status_mirror *sm;
	struct logical_volume *log;
	struct logical_volume *lv = seg->lv;
	int failed = 0, r = 0;
	unsigned i, j;
	struct lvinfo info;

	log_very_verbose("Mirrored transient status: \"%s\"", params);

	if (!dm_get_status_mirror(mem, params, &sm))
		return_0;

	if (sm->dev_count != seg->area_count) {
		log_error("Active mirror has a wrong number of mirror images!");
		log_error("Metadata says %u, kernel says %u.",
			  seg->area_count, sm->dev_count);
		goto out;
	}

	if (!strcmp(sm->log_type, "disk")) {
		log = first_seg(lv)->log_lv;
		if (!lv_info(lv->vg->cmd, log, 0, &info, 0, 0)) {
			log_error("Check for existence of mirror log %s failed.",
				  display_lvname(log));
			goto out;
		}
		log_debug_activation("Found mirror log at %d:%d", info.major, info.minor);
		if (info.major != (int)sm->logs[0].major ||
		    info.minor != (int)sm->logs[0].minor) {
			log_error("Mirror log mismatch. Metadata says %d:%d, kernel says %u:%u.",
				  info.major, info.minor,
				  sm->logs[0].major, sm->logs[0].minor);
			goto out;
		}
		log_very_verbose("Status of log (%d:%d): %c.",
				 info.major, info.minor,
				 sm->logs[0].health);
		if (sm->logs[0].health != DM_STATUS_MIRROR_ALIVE) {
			log->status |= PARTIAL_LV;
			++failed;
		}
	}

	for (i = 0; i < seg->area_count; ++i) {
		if (!lv_info(lv->vg->cmd, seg_lv(seg, i), 0, &info, 0, 0)) {
			log_error("Check for existence of mirror image %s failed.",
				  seg_lv(seg, i)->name);
			goto out;
		}
		log_debug_activation("Found mirror image at %d:%d", info.major, info.minor);
		for (j = 0; j < sm->dev_count; ++j)
			if (info.major == (int)sm->devs[j].major &&
			    info.minor == (int)sm->devs[j].minor) {
				log_very_verbose("Status of image %d: %c.",
						 i, sm->devs[j].health);
				if (sm->devs[j].health != DM_STATUS_MIRROR_ALIVE) {
					seg_lv(seg, i)->status |= PARTIAL_LV;
					++failed;
				}
				break;
			}
		if (j == sm->dev_count) {
			log_error("Failed to find image %d (%d:%d).",
				  i, info.major, info.minor);
			goto out;
		}
	}

	/* update PARTIAL_LV flags across the VG */
	if (failed)
		vg_mark_partial_lvs(lv->vg, 0);

	r = 1;
out:
	dm_pool_free(mem, sm);

	return r;
}

static int _add_log(struct dm_pool *mem, struct lv_segment *seg,
		    const struct lv_activate_opts *laopts,
		    struct dm_tree_node *node, uint32_t area_count, uint32_t region_size)
{
	unsigned clustered = 0;
	char *log_dlid = NULL;
	uint32_t log_flags = 0;

	if (seg->lv->vg->lock_type && !strcmp(seg->lv->vg->lock_type, "dlm")) {
		/*
		 * If shared lock was used due to -asy, then we set clustered
		 * to use a clustered mirror log with cmirrod.
		 */
		if (seg->lv->vg->cmd->lockd_lv_sh)
			clustered = 1;
	}

	if (seg->log_lv) {
		/* If disk log, use its UUID */
		if (!(log_dlid = build_dm_uuid(mem, seg->log_lv, NULL))) {
			log_error("Failed to build uuid for log LV %s.",
				  seg->log_lv->name);
			return 0;
		}
	} else {
		/* If core log, use mirror's UUID and set DM_CORELOG flag */
		if (!(log_dlid = build_dm_uuid(mem, seg->lv, lv_is_pvmove(seg->lv) ? "pvmove" : NULL))) {
			log_error("Failed to build uuid for mirror LV %s.",
				  seg->lv->name);
			return 0;
		}
		log_flags |= DM_CORELOG;
	}

	if (mirror_in_sync() && !(seg->status & PVMOVE))
		log_flags |= DM_NOSYNC;

	if (_block_on_error_available && !(seg->status & PVMOVE)) {
		if (dmeventd_monitor_mode() == 0) {
			log_warn_suppress(seg->lv->vg->cmd->mirror_warn_printed,
					  "WARNING: Mirror %s without monitoring will not react on failures.",
					  display_lvname(seg->lv));
			seg->lv->vg->cmd->mirror_warn_printed = 1; /* Do not print this more then once */
		} else
			log_flags |= DM_BLOCK_ON_ERROR;
	}

	return dm_tree_node_add_mirror_target_log(node, region_size, clustered, log_dlid, area_count, log_flags);
}

static int _mirrored_add_target_line(struct dev_manager *dm, struct dm_pool *mem,
				     struct cmd_context *cmd, void **target_state,
				     struct lv_segment *seg,
				     const struct lv_activate_opts *laopts,
				     struct dm_tree_node *node, uint64_t len,
				     uint32_t *pvmove_mirror_count)
{
	struct mirror_state *mirr_state;
	uint32_t area_count = seg->area_count;
	unsigned start_area = 0u;
	int mirror_status = MIRR_RUNNING;
	uint32_t region_size;
	int r;

	if (!*target_state &&
	    !(*target_state = _mirrored_init_target(mem, cmd)))
                return_0;

	mirr_state = *target_state;

	/*
	 * Mirror segment could have only 1 area temporarily
	 * if the segment is under conversion.
	 */
 	if (seg->area_count == 1)
		mirror_status = MIRR_DISABLED;

	/*
	 * For pvmove, only have one mirror segment RUNNING at once.
	 * Segments before this are COMPLETED and use 2nd area.
	 * Segments after this are DISABLED and use 1st area.
	 */
	if (seg->status & PVMOVE) {
		if (seg->extents_copied == seg->area_len) {
			mirror_status = MIRR_COMPLETED;
			start_area = 1;
		} else if ((*pvmove_mirror_count)++) {
			mirror_status = MIRR_DISABLED;
			area_count = 1;
		}
		/* else MIRR_RUNNING */
	}

	if (mirror_status != MIRR_RUNNING) {
		if (!add_linear_area_to_dtree(node, len, seg->lv->vg->extent_size,
					      cmd->use_linear_target,
					      seg->lv->vg->name, seg->lv->name))
			return_0;
		goto done;
	}

	if (!(seg->status & PVMOVE)) {
		if (!seg->region_size) {
			log_error("Missing region size for mirror segment.");
			return 0;
		}
		region_size = seg->region_size;

	} else if (!(region_size = adjusted_mirror_region_size(cmd,
							       seg->lv->vg->extent_size,
							       seg->area_len,
							       mirr_state->default_region_size, 1,
							       vg_is_clustered(seg->lv->vg))))
		return_0;

	if (!dm_tree_node_add_mirror_target(node, len))
		return_0;

	if ((r = _add_log(mem, seg, laopts, node, area_count, region_size)) <= 0) {
		stack;
		return r;
	}

      done:
	return add_areas_line(dm, seg, node, start_area, area_count);
}

static int _mirrored_target_present(struct cmd_context *cmd,
				    const struct lv_segment *seg,
				    unsigned *attributes)
{
	static int _mirrored_checked = 0;
	static int _mirrored_present = 0;
	static unsigned _mirror_attributes = 0;
	uint32_t maj, min, patchlevel;
	unsigned maj2, min2, patchlevel2;
	char vsn[80];

	if (!activation())
		return 0;

	if (!_mirrored_checked) {
		_mirrored_checked = 1;

		if (!(_mirrored_present = target_present(cmd, TARGET_NAME_MIRROR, 1)))
			return 0;

		/*
		 * block_on_error available as "block_on_error" log
		 * argument with mirror target >= 1.1 and <= 1.11
		 * or with 1.0 in RHEL4U3 driver >= 4.5
		 *
		 * block_on_error available as "handle_errors" mirror
		 * argument with mirror target >= 1.12.
		 *
		 * libdm-deptree.c is smart enough to handle the differences
		 * between block_on_error and handle_errors for all
		 * mirror target versions >= 1.1
		 */
		/* FIXME Move this into libdevmapper */

		if (target_version(TARGET_NAME_MIRROR, &maj, &min, &patchlevel) &&
		    maj == 1 &&
		    ((min >= 1) ||
		     (min == 0 && driver_version(vsn, sizeof(vsn)) &&
		      sscanf(vsn, "%u.%u.%u", &maj2, &min2, &patchlevel2) == 3 &&
		      maj2 == 4 && min2 == 5 && patchlevel2 == 0)))	/* RHEL4U3 */
			_block_on_error_available = 1;

#ifdef CMIRRORD_PIDFILE
		/*
		 * The cluster mirror log daemon must be running,
		 * otherwise, the kernel module will fail to make
		 * contact.
		 */
		if (cmirrord_is_running()) {
			struct utsname uts;
			unsigned kmaj, kmin, krel;
			/*
			 * The dm-log-userspace module was added to the
			 * 2.6.31 kernel.
			 */
			if (!uname(&uts) &&
			    (sscanf(uts.release, "%u.%u.%u", &kmaj, &kmin, &krel) == 3) &&
			    KERNEL_VERSION(kmaj, kmin, krel) < KERNEL_VERSION(2, 6, 31)) {
				if (module_present(cmd, MODULE_NAME_LOG_CLUSTERED))
				_mirror_attributes |= MIRROR_LOG_CLUSTERED;
			} else if (module_present(cmd, MODULE_NAME_LOG_USERSPACE))
				_mirror_attributes |= MIRROR_LOG_CLUSTERED;

			if (!(_mirror_attributes & MIRROR_LOG_CLUSTERED))
				log_verbose("Cluster mirror log module is not available.");
		} else
			log_verbose("Cluster mirror log daemon is not running.");
#else
		log_verbose("Cluster mirror log daemon not included in build.");
#endif
	}

	/*
	 * Check only for modules if atttributes requested and no previous check.
	 * FIXME: Fails incorrectly if cmirror was built into kernel.
	 */
	if (attributes)
		*attributes = _mirror_attributes;

	return _mirrored_present;
}

#  ifdef DMEVENTD
/* FIXME Cache this */
static int _target_registered(struct lv_segment *seg, int *pending, int *monitored)
{
	return target_registered_with_dmeventd(seg->lv->vg->cmd, seg->segtype->dso,
					       seg->lv, pending, monitored);
}

/* FIXME This gets run while suspended and performs banned operations. */
static int _target_set_events(struct lv_segment *seg, int evmask, int set)
{
	return target_register_events(seg->lv->vg->cmd, seg->segtype->dso,
				      seg->lv, evmask, set, 0);
}

static int _target_monitor_events(struct lv_segment *seg, int events)
{
	return _target_set_events(seg, events, 1);
}

static int _target_unmonitor_events(struct lv_segment *seg, int events)
{
	return _target_set_events(seg, events, 0);
}

#  endif /* DMEVENTD */

static int _mirrored_modules_needed(struct dm_pool *mem,
				    const struct lv_segment *seg,
				    struct dm_list *modules)
{
	if (seg->log_lv &&
	    !list_segment_modules(mem, first_seg(seg->log_lv), modules))
		return_0;

	if (!str_list_add(mem, modules, MODULE_NAME_MIRROR)) {
		log_error("mirror string list allocation failed");
		return 0;
	}

	return 1;
}
#endif /* DEVMAPPER_SUPPORT */

static void _mirrored_destroy(struct segment_type *segtype)
{
	free((void *) segtype->dso);
	free(segtype);
}

static struct segtype_handler _mirrored_ops = {
	.display = _mirrored_display,
	.text_import_area_count = _mirrored_text_import_area_count,
	.text_import = _mirrored_text_import,
	.text_export = _mirrored_text_export,
#ifdef DEVMAPPER_SUPPORT
	.add_target_line = _mirrored_add_target_line,
	.target_percent = _mirrored_target_percent,
	.target_present = _mirrored_target_present,
	.check_transient_status = _mirrored_transient_status,
	.modules_needed = _mirrored_modules_needed,
#  ifdef DMEVENTD
	.target_monitored = _target_registered,
	.target_monitor_events = _target_monitor_events,
	.target_unmonitor_events = _target_unmonitor_events,
#  endif	/* DMEVENTD */
#endif
	.destroy = _mirrored_destroy,
};

#ifdef MIRRORED_INTERNAL
struct segment_type *init_mirrored_segtype(struct cmd_context *cmd)
#else				/* Shared */
struct segment_type *init_segtype(struct cmd_context *cmd);
struct segment_type *init_segtype(struct cmd_context *cmd)
#endif
{
	struct segment_type *segtype = zalloc(sizeof(*segtype));

	if (!segtype)
		return_NULL;

	segtype->ops = &_mirrored_ops;
	segtype->name = SEG_TYPE_NAME_MIRROR;
	segtype->flags = SEG_MIRROR | SEG_AREAS_MIRRORED;

#ifdef DEVMAPPER_SUPPORT
#  ifdef DMEVENTD
	segtype->dso = get_monitor_dso_path(cmd, dmeventd_mirror_library_CFG);

	if (segtype->dso)
		segtype->flags |= SEG_MONITORED;
#  endif	/* DMEVENTD */
#endif

	log_very_verbose("Initialised segtype: %s", segtype->name);

	return segtype;
}
