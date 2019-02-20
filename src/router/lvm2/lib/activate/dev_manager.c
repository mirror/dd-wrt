/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2018 Red Hat, Inc. All rights reserved.
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
#include "dev_manager.h"
#include "lib/misc/lvm-string.h"
#include "fs.h"
#include "lib/config/defaults.h"
#include "lib/metadata/segtype.h"
#include "lib/display/display.h"
#include "lib/commands/toolcontext.h"
#include "lib/activate/targets.h"
#include "lib/config/config.h"
#include "lib/activate/activate.h"
#include "lib/misc/lvm-exec.h"
#include "lib/datastruct/str_list.h"

#include <limits.h>
#include <dirent.h>

#define MAX_TARGET_PARAMSIZE 50000
#define LVM_UDEV_NOSCAN_FLAG DM_SUBSYSTEM_UDEV_FLAG0
#define CRYPT_TEMP	"CRYPT-TEMP"
#define STRATIS		"stratis-"

typedef enum {
	PRELOAD,
	ACTIVATE,
	DEACTIVATE,
	SUSPEND,
	SUSPEND_WITH_LOCKFS,
	CLEAN
} action_t;

/* This list must match lib/misc/lvm-string.c:build_dm_uuid(). */
const char *uuid_suffix_list[] = { "pool", "cdata", "cmeta", "tdata", "tmeta", "vdata", "vpool", NULL};

struct dlid_list {
	struct dm_list list;
	const char *dlid;
	const struct logical_volume *lv;
};

struct dev_manager {
	struct dm_pool *mem;

	struct cmd_context *cmd;

	void *target_state;
	uint32_t pvmove_mirror_count;
	int flush_required;
	int activation;                 /* building activation tree */
	int suspend;			/* building suspend tree */
	unsigned track_external_lv_deps;
	struct dm_list pending_delete;	/* str_list of dlid(s) with pending delete */
	unsigned track_pending_delete;
	unsigned track_pvmove_deps;

	const char *vg_name;
};

struct lv_layer {
	const struct logical_volume *lv;
	const char *old_name;
	int visible_component;
};

int read_only_lv(const struct logical_volume *lv, const struct lv_activate_opts *laopts, const char *layer)
{
	if (layer && lv_is_cow(lv))
		return 0; /* Keep snapshot's COW volume writable */

	if (lv_is_raid_image(lv) || lv_is_raid_metadata(lv))
		return 0; /* Keep RAID SubLvs writable */

	return (laopts->read_only || !(lv->status & LVM_WRITE));
}

/*
 * Low level device-layer operations.
 *
 * Unless task is DM_DEVICE_TARGET_MSG, also calls dm_task_run()
 */
static struct dm_task *_setup_task_run(int task, struct dm_info *info,
				       const char *name, const char *uuid,
				       uint32_t *event_nr,
				       uint32_t major, uint32_t minor,
				       int with_open_count,
				       int with_flush,
				       int query_inactive)
{
	struct dm_task *dmt;

	if (!(dmt = dm_task_create(task)))
		return_NULL;

	if (name && !dm_task_set_name(dmt, name))
		goto_out;

	if (uuid && *uuid && !dm_task_set_uuid(dmt, uuid))
		goto_out;

	if (event_nr && !dm_task_set_event_nr(dmt, *event_nr))
		goto_out;

	if (major && !dm_task_set_major_minor(dmt, major, minor, 1))
		goto_out;

	if (activation_checks() && !dm_task_enable_checks(dmt))
		goto_out;

	if (query_inactive && !dm_task_query_inactive_table(dmt)) {
		log_error("Failed to set query_inactive_table.");
		goto out;
	}

	if (!with_open_count && !dm_task_no_open_count(dmt))
		log_warn("WARNING: Failed to disable open_count.");

	if (!with_flush && !dm_task_no_flush(dmt))
		log_warn("WARNING: Failed to set no_flush.");

	if (task == DM_DEVICE_TARGET_MSG)
		return dmt; /* TARGET_MSG needs more local tweaking before task_run() */

	if (!dm_task_run(dmt))
		goto_out;

	if (info && !dm_task_get_info(dmt, info))
		goto_out;

	return dmt;

out:
	dm_task_destroy(dmt);

	return NULL;
}

static int _get_segment_status_from_target_params(const char *target_name,
						  const char *params,
						  struct lv_seg_status *seg_status)
{
	const struct lv_segment *seg = seg_status->seg;
	const struct segment_type *segtype = seg->segtype;

	seg_status->type = SEG_STATUS_UNKNOWN; /* Parsing failed */

	/* Switch to snapshot segtype status logic for merging origin */
	/* This is 'dynamic' decision, both states are valid */
	if (lv_is_merging_origin(seg->lv)) {
		if (!strcmp(target_name, TARGET_NAME_SNAPSHOT_ORIGIN)) {
			seg_status->type = SEG_STATUS_NONE;
			return 1; /* Merge has not yet started */
		}
		if (!strcmp(target_name, TARGET_NAME_SNAPSHOT_MERGE) &&
		    !(segtype = get_segtype_from_string(seg->lv->vg->cmd, TARGET_NAME_SNAPSHOT)))
			return_0;
		/* Merging, parse 'snapshot' status of merge progress */
	}

	if (!params) {
		log_warn("WARNING: Cannot find matching %s segment for %s.",
			 segtype->name, display_lvname(seg_status->seg->lv));
		return 0;
	}

	/* Validate target_name segtype from DM table with lvm2 metadata segtype */
	if (!lv_is_locked(seg->lv) &&
	    strcmp(segtype->name, target_name) &&
	    /* If kernel's type isn't an exact match is it compatible? */
	    (!segtype->ops->target_status_compatible ||
	     !segtype->ops->target_status_compatible(target_name))) {
		log_warn(INTERNAL_ERROR "WARNING: Segment type %s found does not match expected type %s for %s.",
			 target_name, segtype->name, display_lvname(seg_status->seg->lv));
		return 0;
	}

	/* TODO: move into segtype method */
	if (segtype_is_cache(segtype)) {
		if (!dm_get_status_cache(seg_status->mem, params, &(seg_status->cache)))
			return_0;
		seg_status->type = SEG_STATUS_CACHE;
	} else if (segtype_is_raid(segtype)) {
		if (!dm_get_status_raid(seg_status->mem, params, &seg_status->raid))
			return_0;
		seg_status->type = SEG_STATUS_RAID;
	} else if (segtype_is_thin_volume(segtype)) {
		if (!dm_get_status_thin(seg_status->mem, params, &seg_status->thin))
			return_0;
		seg_status->type = SEG_STATUS_THIN;
	} else if (segtype_is_thin_pool(segtype)) {
		if (!dm_get_status_thin_pool(seg_status->mem, params, &seg_status->thin_pool))
			return_0;
		seg_status->type = SEG_STATUS_THIN_POOL;
	} else if (segtype_is_snapshot(segtype)) {
		if (!dm_get_status_snapshot(seg_status->mem, params, &seg_status->snapshot))
			return_0;
		seg_status->type = SEG_STATUS_SNAPSHOT;
	} else if (segtype_is_vdo_pool(segtype)) {
		if (!parse_vdo_pool_status(seg_status->mem, seg->lv, params, &seg_status->vdo_pool))
			return_0;
		seg_status->type = SEG_STATUS_VDO_POOL;
	} else
		/*
		 * TODO: Add support for other segment types too!
		 * Status not supported
		 */
		seg_status->type = SEG_STATUS_NONE;

	return 1;
}

typedef enum {
	INFO,	/* DM_DEVICE_INFO ioctl */
	STATUS, /* DM_DEVICE_STATUS ioctl */
} info_type_t;

/* Return length of segment depending on type and reshape_len */
static uint32_t _seg_len(const struct lv_segment *seg)
{
	uint32_t reshape_len = seg_is_raid(seg) ? ((seg->area_count - seg->segtype->parity_devs) * seg->reshape_len) : 0;

	return seg->len - reshape_len;
}

static int _info_run(const char *dlid, struct dm_info *dminfo,
		     uint32_t *read_ahead,
		     struct lv_seg_status *seg_status,
		     int with_open_count, int with_read_ahead,
		     uint32_t major, uint32_t minor)
{
	int r = 0;
	struct dm_task *dmt;
	int dmtask;
	int with_flush; /* TODO: arg for _info_run */
	void *target = NULL;
	uint64_t target_start, target_length, start, length;
	char *target_name, *target_params;

	if (seg_status) {
		dmtask = DM_DEVICE_STATUS;
		with_flush = 0;
	} else {
		dmtask = DM_DEVICE_INFO;
		with_flush = 1; /* doesn't really matter */
	}

	if (!(dmt = _setup_task_run(dmtask, dminfo, NULL, dlid, 0, major, minor,
				    with_open_count, with_flush, 0)))
		return_0;

	if (with_read_ahead && dminfo->exists) {
		if (!dm_task_get_read_ahead(dmt, read_ahead))
			goto_out;
	} else if (read_ahead)
		*read_ahead = DM_READ_AHEAD_NONE;

	/* Query status only for active device */
	if (seg_status && dminfo->exists) {
		start = length = seg_status->seg->lv->vg->extent_size;
		start *= seg_status->seg->le;
		length *= _seg_len(seg_status->seg);

		/* Uses max DM_THIN_MAX_METADATA_SIZE sectors for metadata device */
		if (lv_is_thin_pool_metadata(seg_status->seg->lv) &&
		    (length > DM_THIN_MAX_METADATA_SIZE))
			length = DM_THIN_MAX_METADATA_SIZE;

		/* Uses virtual size with headers for VDO pool device */
		if (lv_is_vdo_pool(seg_status->seg->lv))
			length = get_vdo_pool_virtual_size(seg_status->seg);

		do {
			target = dm_get_next_target(dmt, target, &target_start,
						    &target_length, &target_name, &target_params);

			if ((start == target_start) && (length == target_length))
				break; /* Keep target_params when matching segment is found */

			target_params = NULL; /* Marking this target_params unusable */
		} while (target);

		if (!target_name ||
		    !_get_segment_status_from_target_params(target_name, target_params, seg_status))
			stack;
	}

	r = 1;

      out:
	dm_task_destroy(dmt);

	return r;
}

/*
 * ignore_blocked_mirror_devices
 * @dev
 * @start
 * @length
 * @mirror_status_str
 *
 * When a DM 'mirror' target is created with 'block_on_error' or
 * 'handle_errors', it will block I/O if there is a device failure
 * until the mirror is reconfigured.  Thus, LVM should never attempt
 * to read labels from a mirror that has a failed device.  (LVM
 * commands are issued to repair mirrors; and if LVM is blocked
 * attempting to read a mirror, a circular dependency would be created.)
 *
 * This function is a slimmed-down version of lib/mirror/mirrored.c:
 * _mirrored_transient_status().
 *
 * If a failed device is detected in the status string, then it must be
 * determined if 'block_on_error' or 'handle_errors' was used when
 * creating the mirror.  This info can only be determined from the mirror
 * table.  The 'dev', 'start', 'length' trio allow us to correlate the
 * 'mirror_status_str' with the correct device table in order to check
 * for blocking.
 *
 * Returns: 1 if mirror should be ignored, 0 if safe to use
 */
static int _ignore_blocked_mirror_devices(struct device *dev,
					  uint64_t start, uint64_t length,
					  char *mirror_status_str)
{
	struct dm_pool *mem;
	struct dm_status_mirror *sm;
	unsigned i, check_for_blocking = 0;
	uint64_t s,l;
	char *p, *params, *target_type = NULL;
	void *next = NULL;
	struct dm_task *dmt = NULL;
	int r = 0;
	struct device *tmp_dev;
	char buf[16];

	if (!(mem = dm_pool_create("blocked_mirrors", 128)))
		return_0;

	if (!dm_get_status_mirror(mem, mirror_status_str, &sm))
		goto_out;

	for (i = 0; i < sm->dev_count; ++i)
		if (sm->devs[i].health != DM_STATUS_MIRROR_ALIVE) {
			log_debug_activation("%s: Mirror image %d marked as failed.",
					     dev_name(dev), i);
			check_for_blocking = 1;
		}

	if (!check_for_blocking && sm->log_count) {
		if (sm->logs[0].health != DM_STATUS_MIRROR_ALIVE) {
			log_debug_activation("%s: Mirror log device marked as failed.",
					     dev_name(dev));
			check_for_blocking = 1;
		} else {

			if (dm_snprintf(buf, sizeof(buf), "%u:%u",
					sm->logs[0].major, sm->logs[0].minor) < 0)
				goto_out;

			if (!(tmp_dev = dev_create_file(buf, NULL, NULL, 0)))
				goto_out;

			tmp_dev->dev = MKDEV(sm->logs[0].major, sm->logs[0].minor);
			if (device_is_usable(tmp_dev, (struct dev_usable_check_params)
					     { .check_empty = 1,
					       .check_blocked = 1,
					       .check_suspended = ignore_suspended_devices(),
					       .check_error_target = 1,
					       .check_reserved = 0 }))
				goto out; /* safe to use */
			stack;
		}
	}

	if (!check_for_blocking) {
		r = 1;
		goto out;
	}

	/*
	 * We avoid another system call if we can, but if a device is
	 * dead, we have no choice but to look up the table too.
	 */
	if (!(dmt = _setup_task_run(DM_DEVICE_TABLE, NULL, NULL, NULL, NULL,
				    MAJOR(dev->dev), MINOR(dev->dev), 0, 1, 0)))
		goto_out;

	do {
		next = dm_get_next_target(dmt, next, &s, &l,
					  &target_type, &params);
		if ((s == start) && (l == length) &&
		    target_type && params) {
			if (strcmp(target_type, TARGET_NAME_MIRROR))
				goto_out;

			if (((p = strstr(params, " block_on_error")) &&
			     (p[15] == '\0' || p[15] == ' ')) ||
			    ((p = strstr(params, " handle_errors")) &&
			     (p[14] == '\0' || p[14] == ' '))) {
				log_debug_activation("%s: I/O blocked to mirror device.",
						     dev_name(dev));
				goto out;
			}
		}
	} while (next);

	r = 1;
out:
	if (dmt)
		dm_task_destroy(dmt);

	dm_pool_destroy(mem);

	return r;
}

static int _device_is_suspended(int major, int minor)
{
	struct dm_task *dmt;
	struct dm_info info;

	if (!(dmt = _setup_task_run(DM_DEVICE_INFO, &info,
				    NULL, NULL, NULL,
				    major, minor, 0, 0, 0)))
		return_0;

	dm_task_destroy(dmt);

	return (info.exists && info.suspended);
}

static int _ignore_suspended_snapshot_component(struct device *dev)
{
	struct dm_task *dmt;
	void *next = NULL;
	char *params, *target_type = NULL;
	uint64_t start, length;
	int major1, minor1, major2, minor2;
	int r = 0;

	if (!(dmt = _setup_task_run(DM_DEVICE_TABLE, NULL,
				    NULL, NULL, NULL,
				    MAJOR(dev->dev), MINOR(dev->dev), 0, 1, 0)))
		return_0;

	do {
		next = dm_get_next_target(dmt, next, &start, &length, &target_type, &params);

		if (!target_type)
			continue;

		if (!strcmp(target_type, TARGET_NAME_SNAPSHOT)) {
			if (!params || sscanf(params, "%d:%d %d:%d", &major1, &minor1, &major2, &minor2) != 4) {
				log_warn("WARNING: Incorrect snapshot table found for %d:%d.",
					 (int)MAJOR(dev->dev), (int)MINOR(dev->dev));
				goto out;
			}
			r = r || _device_is_suspended(major1, minor1) || _device_is_suspended(major2, minor2);
		} else if (!strcmp(target_type, TARGET_NAME_SNAPSHOT_ORIGIN)) {
			if (!params || sscanf(params, "%d:%d", &major1, &minor1) != 2) {
				log_warn("WARNING: Incorrect snapshot-origin table found for %d:%d.",
					 (int)MAJOR(dev->dev), (int)MINOR(dev->dev));
				goto out;
			}
			r = r || _device_is_suspended(major1, minor1);
		}
	} while (next);

out:
	dm_task_destroy(dmt);

	return r;
}

static int _ignore_unusable_thins(struct device *dev)
{
	/* TODO make function for thin testing */
	struct dm_pool *mem;
	struct dm_status_thin_pool *status;
	struct dm_task *dmt = NULL;
	void *next = NULL;
	uint64_t start, length;
	char *target_type = NULL;
	char *params;
	int minor, major;
	int r = 0;

	if (!(mem = dm_pool_create("unusable_thins", 128)))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_TABLE, NULL, NULL, NULL, NULL,
				    MAJOR(dev->dev), MINOR(dev->dev), 0, 1, 0)))
		goto_out;

	dm_get_next_target(dmt, next, &start, &length, &target_type, &params);
	if (!params || sscanf(params, "%d:%d", &major, &minor) != 2) {
		log_warn("WARNING: Cannot get thin-pool major:minor for thin device %d:%d.",
			  (int)MAJOR(dev->dev), (int)MINOR(dev->dev));
		goto out;
	}
	dm_task_destroy(dmt);

	if (!(dmt = _setup_task_run(DM_DEVICE_STATUS, NULL, NULL, NULL, NULL,
				    major, minor, 0, 0, 0)))
		goto_out;

	dm_get_next_target(dmt, next, &start, &length, &target_type, &params);
	if (!dm_get_status_thin_pool(mem, params, &status))
		goto_out;

	if (status->read_only || status->out_of_data_space) {
		log_warn("WARNING: %s: Thin's thin-pool needs inspection.",
			 dev_name(dev));
		goto out;
	}

	r = 1;
out:
	if (dmt)
		dm_task_destroy(dmt);

	dm_pool_destroy(mem);

        return r;
}

static int _ignore_invalid_snapshot(const char *params)
{
	struct dm_status_snapshot *s;
	struct dm_pool *mem;
	int r = 0;

	if (!(mem = dm_pool_create("invalid snapshots", 128)))
		return_0;

	if (!dm_get_status_snapshot(mem, params, &s))
		stack;
        else
		r = s->invalid;

	dm_pool_destroy(mem);

	return r;
}

static int _ignore_frozen_raid(struct device *dev, const char *params)
{
	struct dm_status_raid *s;
	struct dm_pool *mem;
	int r = 0;

	if (!(mem = dm_pool_create("frozen raid", 128)))
		return_0;

	if (!dm_get_status_raid(mem, params, &s))
		stack;
	else if (s->sync_action && !strcmp(s->sync_action, "frozen")) {
		log_warn("WARNING: %s frozen raid device (%d:%d) needs inspection.",
			  dev_name(dev), (int)MAJOR(dev->dev), (int)MINOR(dev->dev));
		r = 1;
	}

	dm_pool_destroy(mem);

	return r;
}

/*
 * device_is_usable
 * @dev
 * @check_lv_names
 *
 * A device is considered not usable if it is:
 *     1) An empty device (no targets)
 *     2) A blocked mirror (i.e. a mirror with a failure and block_on_error set)
 *     3) ignore_suspended_devices is set and
 *        a) the device is suspended
 *        b) it is a snapshot origin
 *     4) an error target
 *     5) the LV name is a reserved name.
 *
 * Returns: 1 if usable, 0 otherwise
 */
int device_is_usable(struct device *dev, struct dev_usable_check_params check)
{
	struct dm_task *dmt;
	struct dm_info info;
	const char *name, *uuid;
	uint64_t start, length;
	char *target_type = NULL;
	char *params, *vgname = NULL, *lvname, *layer;
	void *next = NULL;
	int only_error_target = 1;
	int r = 0;

	if (!(dmt = _setup_task_run(DM_DEVICE_STATUS, &info, NULL, NULL, NULL,
				    MAJOR(dev->dev), MINOR(dev->dev), 0, 0, 0)))
		return_0;

	if (!info.exists)
		goto out;

	name = dm_task_get_name(dmt);
	uuid = dm_task_get_uuid(dmt);

	if (check.check_empty && !info.target_count) {
		log_debug_activation("%s: Empty device %s not usable.", dev_name(dev), name);
		goto out;
	}

	if (check.check_suspended && info.suspended) {
		log_debug_activation("%s: Suspended device %s not usable.", dev_name(dev), name);
		goto out;
	}

	/* Check internal lvm devices */
	if (check.check_reserved &&
	    uuid && !strncmp(uuid, UUID_PREFIX, sizeof(UUID_PREFIX) - 1)) {
		if (strlen(uuid) > (sizeof(UUID_PREFIX) + 2 * ID_LEN)) { /* 68 */
			log_debug_activation("%s: Reserved uuid %s on internal LV device %s not usable.",
					     dev_name(dev), uuid, name);
			goto out;
		}

		if (!(vgname = strdup(name)) ||
		    !dm_split_lvm_name(NULL, NULL, &vgname, &lvname, &layer))
			goto_out;

		/* FIXME: fails to handle dev aliases i.e. /dev/dm-5, replace with UUID suffix */
		if (lvname && (is_reserved_lvname(lvname) || *layer)) {
			log_debug_activation("%s: Reserved internal LV device %s/%s%s%s not usable.",
					     dev_name(dev), vgname, lvname, *layer ? "-" : "", layer);
			goto out;
		}
	}

	if (check.check_lv && uuid && !strncmp(uuid, "LVM-", 4)) {
		/* Skip LVs */
		goto out;
	}

	if (check.check_reserved && uuid &&
	    (!strncmp(uuid, CRYPT_TEMP, sizeof(CRYPT_TEMP) - 1) ||
	     !strncmp(uuid, STRATIS, sizeof(STRATIS) - 1))) {
		/* Skip private crypto devices */
		log_debug_activation("%s: Reserved uuid %s on %s device %s not usable.",
				     dev_name(dev), uuid,
				     uuid[0] == 'C' ? "crypto" : "stratis",
				     name);
		goto out;
	}

	/* FIXME Also check for mpath no paths */
	do {
		next = dm_get_next_target(dmt, next, &start, &length,
					  &target_type, &params);

		if (!target_type)
			continue;

		if (check.check_blocked && !strcmp(target_type, TARGET_NAME_MIRROR)) {
			if (ignore_lvm_mirrors()) {
				log_debug_activation("%s: Scanning mirror devices is disabled.", dev_name(dev));
				goto out;
			}
			if (!_ignore_blocked_mirror_devices(dev, start,
							    length, params)) {
				log_debug_activation("%s: Mirror device %s not usable.",
						     dev_name(dev), name);
				goto out;
			}
		}

		/*
		 * FIXME: Snapshot origin could be sitting on top of a mirror
		 * which could be blocking I/O. We should add a check for the
		 * stack here and see if there's blocked mirror underneath.
		 * Currently, mirrors used as origin or snapshot is not
		 * supported anymore and in general using mirrors in a stack
		 * is disabled by default (with a warning that if enabled,
		 * it could cause various deadlocks).
		 * Similar situation can happen with RAID devices where
		 * a RAID device can be snapshotted.
		 * If one of the RAID legs are down and we're doing
		 * lvconvert --repair, there's a time period in which
		 * snapshot components are (besides other devs) suspended.
		 * See also https://bugzilla.redhat.com/show_bug.cgi?id=1219222
		 * for an example where this causes problems.
		 *
		 * This is a quick check for now, but replace it with more
		 * robust and better check that would check the stack
		 * correctly, not just snapshots but any cobimnation possible
		 * in a stack - use proper dm tree to check this instead.
		 */
		if (check.check_suspended &&
		    (!strcmp(target_type, TARGET_NAME_SNAPSHOT) || !strcmp(target_type, TARGET_NAME_SNAPSHOT_ORIGIN)) &&
		    _ignore_suspended_snapshot_component(dev)) {
			log_debug_activation("%s: %s device %s not usable.", dev_name(dev), target_type, name);
			goto out;
		}

		if (!strcmp(target_type, TARGET_NAME_SNAPSHOT) &&
		    _ignore_invalid_snapshot(params)) {
			log_debug_activation("%s: Invalid %s device %s not usable.", dev_name(dev), target_type, name);
			goto out;
		}

		if (!strncmp(target_type, TARGET_NAME_RAID, 4) && _ignore_frozen_raid(dev, params)) {
			log_debug_activation("%s: Frozen %s device %s not usable.",
					     dev_name(dev), target_type, name);
			goto out;
		}

		/* TODO: extend check struct ? */
		if (!strcmp(target_type, TARGET_NAME_THIN) &&
		    !_ignore_unusable_thins(dev)) {
			log_debug_activation("%s: %s device %s not usable.", dev_name(dev), target_type, name);
			goto out;
		}

		if (strcmp(target_type, TARGET_NAME_ERROR))
			only_error_target = 0;
	} while (next);

	/* Skip devices consisting entirely of error targets. */
	/* FIXME Deal with device stacked above error targets? */
	if (check.check_error_target && only_error_target) {
		log_debug_activation("%s: Error device %s not usable.",
				     dev_name(dev), name);
		goto out;
	}

	/* FIXME Also check dependencies? */

	r = 1;

      out:
	free(vgname);
	dm_task_destroy(dmt);
	return r;
}

/*
 * If active LVs were activated by a version of LVM2 before 2.02.00 we must
 * perform additional checks to find them because they do not have the LVM-
 * prefix on their dm uuids.
 * As of 2.02.150, we've chosen to disable this compatibility arbitrarily if
 * we're running kernel version 3 or above.
 */
#define MIN_KERNEL_MAJOR 3

static int _original_uuid_format_check_required(struct cmd_context *cmd)
{
	static int _kernel_major = 0;

	if (!_kernel_major) {
		if ((sscanf(cmd->kernel_vsn, "%d", &_kernel_major) == 1) &&
		    (_kernel_major >= MIN_KERNEL_MAJOR))
			log_debug_activation("Skipping checks for old devices without " UUID_PREFIX
					     " dm uuid prefix (kernel vsn %d >= %d).", _kernel_major, MIN_KERNEL_MAJOR);
		else
			_kernel_major = -1;
	}

	return (_kernel_major == -1);
}

static int _info(struct cmd_context *cmd,
		 const char *name, const char *dlid,
		 int with_open_count, int with_read_ahead,
		 struct dm_info *dminfo, uint32_t *read_ahead,
		 struct lv_seg_status *seg_status)
{
	char old_style_dlid[sizeof(UUID_PREFIX) + 2 * ID_LEN];
	const char *suffix, *suffix_position;
	unsigned i = 0;

	log_debug_activation("Getting device info for %s [%s].", name, dlid);

	/* Check for dlid */
	if (!_info_run(dlid, dminfo, read_ahead, seg_status,
		       with_open_count, with_read_ahead, 0, 0))
		return_0;

	if (dminfo->exists)
		return 1;

	/* Check for original version of dlid before the suffixes got added in 2.02.106 */
	if ((suffix_position = rindex(dlid, '-'))) {
		while ((suffix = uuid_suffix_list[i++])) {
			if (strcmp(suffix_position + 1, suffix))
				continue;

			(void) strncpy(old_style_dlid, dlid, sizeof(old_style_dlid));
			old_style_dlid[sizeof(old_style_dlid) - 1] = '\0';
			if (!_info_run(old_style_dlid, dminfo, read_ahead, seg_status,
				       with_open_count, with_read_ahead, 0, 0))
				return_0;
			if (dminfo->exists)
				return 1;
		}
	}

	/* Must we still check for the pre-2.02.00 dm uuid format? */
	if (!_original_uuid_format_check_required(cmd))
		return 1;

	/* Check for dlid before UUID_PREFIX was added */
	if (!_info_run(dlid + sizeof(UUID_PREFIX) - 1, dminfo, read_ahead, seg_status,
		       with_open_count, with_read_ahead, 0, 0))
		return_0;

	return 1;
}

static int _info_by_dev(uint32_t major, uint32_t minor, struct dm_info *info)
{
	return _info_run(NULL, info, NULL, 0, 0, 0, major, minor);
}

int dev_manager_info(struct cmd_context *cmd,
		     const struct logical_volume *lv, const char *layer,
		     int with_open_count, int with_read_ahead,
		     struct dm_info *dminfo, uint32_t *read_ahead,
		     struct lv_seg_status *seg_status)
{
	char *dlid, *name;
	int r = 0;

	if (!(name = dm_build_dm_name(cmd->mem, lv->vg->name, lv->name, layer)))
		return_0;

	if (!(dlid = build_dm_uuid(cmd->mem, lv, layer)))
		goto_out;

	if (!(r = _info(cmd, name, dlid, with_open_count, with_read_ahead,
			dminfo, read_ahead, seg_status)))
		stack;
out:
	dm_pool_free(cmd->mem, name);

	return r;
}

static const struct dm_info *_cached_dm_info(struct dm_pool *mem,
					     struct dm_tree *dtree,
					     const struct logical_volume *lv,
					     const char *layer)
{
	char *dlid;
	const struct dm_tree_node *dnode;
	const struct dm_info *dinfo = NULL;

	if (!(dlid = build_dm_uuid(mem, lv, layer)))
		return_NULL;

	if (!(dnode = dm_tree_find_node_by_uuid(dtree, dlid)))
		goto out;

	if (!(dinfo = dm_tree_node_get_info(dnode))) {
		log_warn("WARNING: Cannot get info from tree node for %s.",
			 display_lvname(lv));
		goto out;
	}

	if (!dinfo->exists)
		dinfo = NULL;
out:
	dm_pool_free(mem, dlid);

	return dinfo;
}

int lv_has_target_type(struct dm_pool *mem, const struct logical_volume *lv,
		       const char *layer, const char *target_type)
{
	int r = 0;
	char *dlid;
	struct dm_task *dmt;
	struct dm_info info;
	void *next = NULL;
	uint64_t start, length;
	char *type = NULL;
	char *params = NULL;

	if (!(dlid = build_dm_uuid(mem, lv, layer)))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_STATUS, &info, NULL, dlid, 0, 0, 0, 0, 0, 0)))
		goto_bad;

	if (!info.exists)
		goto_out;

	/* If there is a preloaded table, use that in preference. */
	if (info.inactive_table) {
		dm_task_destroy(dmt);

		if (!(dmt = _setup_task_run(DM_DEVICE_STATUS, &info, NULL, dlid, 0, 0, 0, 0, 0, 1)))
			goto_bad;

		if (!info.exists || !info.inactive_table)
			goto_out;
	}

	do {
		next = dm_get_next_target(dmt, next, &start, &length,
					  &type, &params);
		if (type && !strncmp(type, target_type, strlen(target_type))) {
			r = 1;
			break;
		}
	} while (next);

out:
	dm_task_destroy(dmt);
bad:
	dm_pool_free(mem, dlid);

	return r;
}

static int _thin_lv_has_device_id(struct dm_pool *mem, const struct logical_volume *lv,
				  const char *layer, unsigned device_id)
{
	char *dlid;
	struct dm_task *dmt;
	struct dm_info info;
	void *next = NULL;
	uint64_t start, length;
	char *type = NULL;
	char *params = NULL;
	unsigned id = ~0;

	if (!(dlid = build_dm_uuid(mem, lv, layer)))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_TABLE, &info, NULL, dlid, 0, 0, 0, 0, 1, 0)))
		goto_bad;

	if (!info.exists)
		goto_out;

	/* If there is a preloaded table, use that in preference. */
	if (info.inactive_table) {
		dm_task_destroy(dmt);

		if (!(dmt = _setup_task_run(DM_DEVICE_TABLE, &info, NULL, dlid, 0, 0, 0, 0, 1, 1)))
			goto_bad;

		if (!info.exists || !info.inactive_table)
			goto_out;
	}

	(void) dm_get_next_target(dmt, next, &start, &length, &type, &params);

	if (!type || strcmp(type, TARGET_NAME_THIN))
		goto_out;

	if (!params || sscanf(params, "%*u:%*u %u", &id) != 1)
		goto_out;

	log_debug_activation("%soaded thin volume %s with id %u is %smatching id %u.",
			     info.inactive_table  ? "Prel" : "L",
			     display_lvname(lv), id,
			     (device_id != id) ? "not " : "", device_id);
out:
	dm_task_destroy(dmt);
bad:
	dm_pool_free(mem, dlid);

	return (device_id == id);
}

int add_linear_area_to_dtree(struct dm_tree_node *node, uint64_t size, uint32_t extent_size,
			     int use_linear_target, const char *vgname, const char *lvname)
{
	uint32_t page_size;

	/*
	 * Use striped or linear target?
	 */
	if (!use_linear_target) {
		page_size = lvm_getpagesize() >> SECTOR_SHIFT;

		/*
		 * We'll use the extent size as the stripe size.
		 * Extent size and page size are always powers of 2.
		 * The striped target requires that the stripe size is
		 * divisible by the page size.
		 */
		if (extent_size >= page_size) {
			/* Use striped target */
			if (!dm_tree_node_add_striped_target(node, size, extent_size))
				return_0;
			return 1;
		}

		/* Some exotic cases are unsupported by striped. */
		log_warn("WARNING: Using linear target for %s/%s: Striped requires extent size "
			 "(" FMTu32 " sectors) >= page size (" FMTu32 ").",
			 vgname, lvname, extent_size, page_size);
	}

	/*
	 * Use linear target.
	 */
	if (!dm_tree_node_add_linear_target(node, size))
		return_0;

	return 1;
}

static dm_percent_range_t _combine_percent(dm_percent_t a, dm_percent_t b,
					   uint32_t numerator, uint32_t denominator)
{
	if (a == LVM_PERCENT_MERGE_FAILED || b == LVM_PERCENT_MERGE_FAILED)
		return LVM_PERCENT_MERGE_FAILED;

	if (a == DM_PERCENT_INVALID || b == DM_PERCENT_INVALID)
		return DM_PERCENT_INVALID;

	if (a == DM_PERCENT_100 && b == DM_PERCENT_100)
		return DM_PERCENT_100;

	if (a == DM_PERCENT_0 && b == DM_PERCENT_0)
		return DM_PERCENT_0;

	return (dm_percent_range_t) dm_make_percent(numerator, denominator);
}

static int _percent_run(struct dev_manager *dm, const char *name,
			const char *dlid,
			const char *target_type, int wait,
			const struct logical_volume *lv, dm_percent_t *overall_percent,
			uint32_t *event_nr, int fail_if_percent_unsupported)
{
	int r = 0;
	struct dm_task *dmt;
	struct dm_info info;
	void *next = NULL;
	uint64_t start, length;
	char *type = NULL;
	char *params = NULL;
	const struct dm_list *segh = lv ? &lv->segments : NULL;
	struct lv_segment *seg = NULL;
	int first_time = 1;
	dm_percent_t percent = DM_PERCENT_INVALID;
	uint64_t total_numerator = 0, total_denominator = 0;
	struct segment_type *segtype;

	*overall_percent = percent;

	if (!(segtype = get_segtype_from_string(dm->cmd, target_type)))
		return_0;

	if (!(dmt = _setup_task_run(wait ? DM_DEVICE_WAITEVENT : DM_DEVICE_STATUS, &info,
				    name, dlid, event_nr, 0, 0, 0, 0, 0)))
		return_0;

	if (!info.exists)
		goto_out;

	if (event_nr)
		*event_nr = info.event_nr;

	do {
		next = dm_get_next_target(dmt, next, &start, &length, &type,
					  &params);
		if (lv) {
			if (!(segh = dm_list_next(&lv->segments, segh))) {
				log_error("Number of segments in active LV %s "
					  "does not match metadata.",
					  display_lvname(lv));
				goto out;
			}
			seg = dm_list_item(segh, struct lv_segment);
		}

		if (!type || !params)
			continue;

		if (strcmp(type, target_type)) {
			/* If kernel's type isn't an exact match is it compatible? */
			if (!segtype->ops->target_status_compatible ||
			    !segtype->ops->target_status_compatible(type))
				continue;
		}

		if (!segtype->ops->target_percent)
			continue;

		if (!segtype->ops->target_percent(&dm->target_state,
						  &percent, dm->mem,
						  dm->cmd, seg, params,
						  &total_numerator,
						  &total_denominator))
			goto_out;

		if (first_time) {
			*overall_percent = percent;
			first_time = 0;
		} else
			*overall_percent =
				_combine_percent(*overall_percent, percent,
						 total_numerator, total_denominator);
	} while (next);

	if (lv && dm_list_next(&lv->segments, segh)) {
		log_error("Number of segments in active LV %s does not "
			  "match metadata.", display_lvname(lv));
		goto out;
	}

	if (first_time) {
		/* above ->target_percent() was not executed! */
		/* FIXME why return PERCENT_100 et. al. in this case? */
		*overall_percent = DM_PERCENT_100;
		if (fail_if_percent_unsupported)
			goto_out;
	}

	log_debug_activation("LV percent: %s",
			     display_percent(dm->cmd, *overall_percent));
	r = 1;

      out:
	dm_task_destroy(dmt);
	return r;
}

static int _percent(struct dev_manager *dm, const char *name, const char *dlid,
		    const char *target_type, int wait,
		    const struct logical_volume *lv, dm_percent_t *percent,
		    uint32_t *event_nr, int fail_if_percent_unsupported)
{
	if (dlid && *dlid) {
		if (_percent_run(dm, NULL, dlid, target_type, wait, lv, percent,
				 event_nr, fail_if_percent_unsupported))
			return 1;

		if (_original_uuid_format_check_required(dm->cmd) &&
		    _percent_run(dm, NULL, dlid + sizeof(UUID_PREFIX) - 1,
				 target_type, wait, lv, percent,
				 event_nr, fail_if_percent_unsupported))
			return 1;
	}

	if (name && _percent_run(dm, name, NULL, target_type, wait, lv, percent,
				 event_nr, fail_if_percent_unsupported))
		return 1;

	return_0;
}

/* FIXME Merge with the percent function */
int dev_manager_transient(struct dev_manager *dm, const struct logical_volume *lv)
{
	int r = 0;
	struct dm_task *dmt;
	struct dm_info info;
	void *next = NULL;
	uint64_t start, length;
	char *type = NULL;
	char *params = NULL;
	char *dlid = NULL;
	const char *layer = lv_layer(lv);
	const struct dm_list *segh = &lv->segments;
	struct lv_segment *seg = NULL;

	if (!(dlid = build_dm_uuid(dm->mem, lv, layer)))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_STATUS, &info, NULL, dlid, NULL, 0, 0, 0, 0, 0)))
		return_0;

	if (!info.exists)
		goto_out;

	do {
		next = dm_get_next_target(dmt, next, &start, &length, &type,
					  &params);

		if (!(segh = dm_list_next(&lv->segments, segh))) {
		    log_error("Number of segments in active LV %s "
			      "does not match metadata.", display_lvname(lv));
		    goto out;
		}
		seg = dm_list_item(segh, struct lv_segment);

		if (!type || !params)
			continue;

		if (!seg) {
			log_error(INTERNAL_ERROR "Segment is not selected.");
			goto out;
		}

		if (seg->segtype->ops->check_transient_status &&
		    !seg->segtype->ops->check_transient_status(dm->mem, seg, params))
			goto_out;

	} while (next);

	if (dm_list_next(&lv->segments, segh)) {
		log_error("Number of segments in active LV %s does not "
			  "match metadata.", display_lvname(lv));
		goto out;
	}

	r = 1;

      out:
	dm_task_destroy(dmt);
	return r;
}

/*
 * dev_manager implementation.
 */
struct dev_manager *dev_manager_create(struct cmd_context *cmd,
				       const char *vg_name,
				       unsigned track_pvmove_deps)
{
	struct dm_pool *mem;
	struct dev_manager *dm;

	if (!(mem = dm_pool_create("dev_manager", 16 * 1024)))
		return_NULL;

	if (!(dm = dm_pool_zalloc(mem, sizeof(*dm))))
		goto_bad;

	dm->cmd = cmd;
	dm->mem = mem;
	dm->vg_name = vg_name;

	/*
	 * When we manipulate (normally suspend/resume) the PVMOVE
	 * device directly, there's no need to touch the LVs above.
	 */
	dm->track_pvmove_deps = track_pvmove_deps;

	dm->target_state = NULL;

	dm_udev_set_sync_support(cmd->current_settings.udev_sync);

	dm_list_init(&dm->pending_delete);

	return dm;

      bad:
	dm_pool_destroy(mem);

	return NULL;
}

void dev_manager_destroy(struct dev_manager *dm)
{
	dm_pool_destroy(dm->mem);
}

void dev_manager_release(void)
{
	dm_lib_release();
}

void dev_manager_exit(void)
{
	dm_lib_exit();
}

int dev_manager_snapshot_percent(struct dev_manager *dm,
				 const struct logical_volume *lv,
				 dm_percent_t *percent)
{
	const struct logical_volume *snap_lv;
	char *name;
	const char *dlid;
	int fail_if_percent_unsupported = 0;

	if (lv_is_merging_origin(lv)) {
		/*
		 * Set 'fail_if_percent_unsupported', otherwise passing
		 * unsupported LV types to _percent will lead to a default
		 * successful return with percent_range as PERCENT_100.
		 * - For a merging origin, this will result in a polldaemon
		 *   that runs infinitely (because completion is PERCENT_0)
		 * - We unfortunately don't yet _know_ if a snapshot-merge
		 *   target is active (activation is deferred if dev is open);
		 *   so we can't short-circuit origin devices based purely on
		 *   existing LVM LV attributes.
		 */
		fail_if_percent_unsupported = 1;
	}

	if (lv_is_merging_cow(lv)) {
		/* must check percent of origin for a merging snapshot */
		snap_lv = origin_from_cow(lv);
	} else
		snap_lv = lv;

	/*
	 * Build a name for the top layer.
	 */
	if (!(name = dm_build_dm_name(dm->mem, snap_lv->vg->name, snap_lv->name, NULL)))
		return_0;

	if (!(dlid = build_dm_uuid(dm->mem, snap_lv, NULL)))
		return_0;

	/*
	 * Try and get some info on this device.
	 */
	if (!_percent(dm, name, dlid, TARGET_NAME_SNAPSHOT, 0, NULL, percent,
		      NULL, fail_if_percent_unsupported))
		return_0;

	/* If the snapshot isn't available, percent will be -1 */
	return 1;
}

/* FIXME Merge with snapshot_percent, auto-detecting target type */
/* FIXME Cope with more than one target */
int dev_manager_mirror_percent(struct dev_manager *dm,
			       const struct logical_volume *lv, int wait,
			       dm_percent_t *percent, uint32_t *event_nr)
{
	char *name;
	const char *dlid;
	const char *target_type = first_seg(lv)->segtype->name;
	const char *layer = lv_layer(lv);

	/*
	 * Build a name for the top layer.
	 */
	if (!(name = dm_build_dm_name(dm->mem, lv->vg->name, lv->name, layer)))
		return_0;

	if (!(dlid = build_dm_uuid(dm->mem, lv, layer)))
		return_0;

	log_debug_activation("Getting device %s status percentage for %s.",
			     target_type, name);

	if (!_percent(dm, name, dlid, target_type, wait, lv, percent, event_nr, 0))
		return_0;

	return 1;
}

int dev_manager_raid_status(struct dev_manager *dm,
			    const struct logical_volume *lv,
			    struct dm_status_raid **status)
{
	int r = 0;
	const char *dlid;
	struct dm_task *dmt;
	struct dm_info info;
	uint64_t start, length;
	char *type = NULL;
	char *params = NULL;
	const char *layer = lv_layer(lv);

	if (!(dlid = build_dm_uuid(dm->mem, lv, layer)))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_STATUS, &info, NULL, dlid, 0, 0, 0, 0, 0, 0)))
		return_0;

	if (!info.exists)
		goto_out;

	dm_get_next_target(dmt, NULL, &start, &length, &type, &params);

	if (!type || strcmp(type, TARGET_NAME_RAID)) {
		log_error("Expected %s segment type but got %s instead.",
			  TARGET_NAME_RAID, type ? type : "NULL");
		goto out;
	}

	/* FIXME Check there's only one target */

	if (!dm_get_status_raid(dm->mem, params, status))
		goto_out;

	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}

int dev_manager_raid_message(struct dev_manager *dm,
			     const struct logical_volume *lv,
			     const char *msg)
{
	int r = 0;
	const char *dlid;
	struct dm_task *dmt;
	const char *layer = lv_layer(lv);

	if (!lv_is_raid(lv)) {
		log_error(INTERNAL_ERROR "%s is not a RAID logical volume.",
			  display_lvname(lv));
		return 0;
	}

	/* These are the supported RAID messages for dm-raid v1.9.0 */
	if (strcmp(msg, "idle") &&
	    strcmp(msg, "frozen") &&
	    strcmp(msg, "resync") &&
	    strcmp(msg, "recover") &&
	    strcmp(msg, "check") &&
	    strcmp(msg, "repair")) {
		log_error(INTERNAL_ERROR "Unknown RAID message: %s.", msg);
		return 0;
	}

	if (!(dlid = build_dm_uuid(dm->mem, lv, layer)))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_TARGET_MSG, NULL, NULL, dlid, 0, 0, 0, 0, 1, 0)))
		return_0;

	if (!dm_task_set_message(dmt, msg))
		goto_out;

	if (!dm_task_run(dmt))
		goto_out;

	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}

int dev_manager_cache_status(struct dev_manager *dm,
			     const struct logical_volume *lv,
			     struct lv_status_cache **status)
{
	int r = 0;
	const char *dlid;
	struct dm_task *dmt;
	struct dm_info info;
	uint64_t start, length;
	char *type = NULL;
	char *params = NULL;
	struct dm_status_cache *c;

	if (!(dlid = build_dm_uuid(dm->mem, lv, lv_layer(lv))))
		return_0;

	if (!(*status = dm_pool_zalloc(dm->mem, sizeof(struct lv_status_cache))))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_STATUS, &info, NULL, dlid, 0, 0, 0, 0, 0, 0)))
		return_0;

	if (!info.exists)
		goto_out;

	dm_get_next_target(dmt, NULL, &start, &length, &type, &params);

	if (!type || strcmp(type, TARGET_NAME_CACHE)) {
		log_error("Expected %s segment type but got %s instead.",
			  TARGET_NAME_CACHE, type ? type : "NULL");
		goto out;
	}

	/*
	 * FIXME:
	 * ->target_percent() API is able to transfer only a single value.
	 * Needs to be able to pass whole structure.
	 */
	if (!dm_get_status_cache(dm->mem, params, &c))
		goto_out;

	(*status)->cache = c;
	(*status)->mem = dm->mem; /* User has to destroy this mem pool later */
	if (c->fail || c->error) {
		(*status)->data_usage =
			(*status)->metadata_usage =
			(*status)->dirty_usage = DM_PERCENT_INVALID;
	} else {
		(*status)->data_usage = dm_make_percent(c->used_blocks,
							c->total_blocks);
		(*status)->metadata_usage = dm_make_percent(c->metadata_used_blocks,
							    c->metadata_total_blocks);
		(*status)->dirty_usage = (c->used_blocks) ?
			dm_make_percent(c->dirty_blocks,
					c->used_blocks) : DM_PERCENT_0;
	}
	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}

int dev_manager_thin_pool_status(struct dev_manager *dm,
				 const struct logical_volume *lv,
				 struct dm_status_thin_pool **status,
				 int flush)
{
	const char *dlid;
	struct dm_task *dmt;
	struct dm_info info;
	uint64_t start, length;
	char *type = NULL;
	char *params = NULL;
	int r = 0;

	/* Build dlid for the thin pool layer */
	if (!(dlid = build_dm_uuid(dm->mem, lv, lv_layer(lv))))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_STATUS, &info, NULL, dlid, 0, 0, 0, 0, flush, 0)))
		return_0;

	if (!info.exists)
		goto_out;

	dm_get_next_target(dmt, NULL, &start, &length, &type, &params);

	/* FIXME Check for thin and check there's exactly one target */

	if (!dm_get_status_thin_pool(dm->mem, params, status))
		goto_out;

	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}

int dev_manager_thin_pool_percent(struct dev_manager *dm,
				  const struct logical_volume *lv,
				  int metadata, dm_percent_t *percent)
{
	char *name;
	const char *dlid;
	const char *layer = lv_layer(lv);

	/* Build a name for the top layer */
	if (!(name = dm_build_dm_name(dm->mem, lv->vg->name, lv->name, layer)))
		return_0;

	if (!(dlid = build_dm_uuid(dm->mem, lv, layer)))
		return_0;

	log_debug_activation("Getting device status percentage for %s.", name);

	if (!(_percent(dm, name, dlid, TARGET_NAME_THIN_POOL, 0,
		       (metadata) ? lv : NULL, percent, NULL, 1)))
		return_0;

	return 1;
}

int dev_manager_thin_percent(struct dev_manager *dm,
			     const struct logical_volume *lv,
			     int mapped, dm_percent_t *percent)
{
	char *name;
	const char *dlid;
	const char *layer = lv_layer(lv);

	/* Build a name for the top layer */
	if (!(name = dm_build_dm_name(dm->mem, lv->vg->name, lv->name, layer)))
		return_0;

	if (!(dlid = build_dm_uuid(dm->mem, lv, layer)))
		return_0;

	log_debug_activation("Getting device status percentage for %s", name);

	if (!(_percent(dm, name, dlid, TARGET_NAME_THIN, 0,
		       (mapped) ? NULL : lv, percent, NULL, 1)))
		return_0;

	return 1;
}

int dev_manager_thin_device_id(struct dev_manager *dm,
			       const struct logical_volume *lv,
			       uint32_t *device_id)
{
	const char *dlid;
	struct dm_task *dmt;
	struct dm_info info;
	uint64_t start, length;
	char *params, *target_type = NULL;
	int r = 0;

	/* Build dlid for the thin layer */
	if (!(dlid = build_dm_uuid(dm->mem, lv, lv_layer(lv))))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_TABLE, &info, NULL, dlid, 0, 0, 0, 0, 1, 0)))
		return_0;

	if (!info.exists)
		goto_out;

	if (dm_get_next_target(dmt, NULL, &start, &length,
			       &target_type, &params)) {
		log_error("More then one table line found for %s.",
			  display_lvname(lv));
		goto out;
	}

	if (!target_type || strcmp(target_type, TARGET_NAME_THIN)) {
		log_error("Unexpected target type %s found for thin %s.",
			  target_type, display_lvname(lv));
		goto out;
	}

	if (!params || sscanf(params, "%*u:%*u %u", device_id) != 1) {
		log_error("Cannot parse table like parameters %s for %s.",
			  params, display_lvname(lv));
		goto out;
	}

	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}

int dev_manager_vdo_pool_status(struct dev_manager *dm,
				const struct logical_volume *lv,
				int flush,
				char **vdo_params,
				struct lv_status_vdo **vdo_status)
{
	struct lv_status_vdo *status;
	const char *dlid;
	struct dm_info info;
	uint64_t start, length;
	struct dm_task *dmt = NULL;
	char *type = NULL;
	char *params = NULL;
	int r = 0;

	*vdo_params = NULL;
	*vdo_status = NULL;

	if (!(status = dm_pool_zalloc(dm->mem, sizeof(struct lv_status_vdo)))) {
		log_error("Cannot allocate VDO status structure.");
		return 0;
	}

	if (!(dlid = build_dm_uuid(dm->mem, lv, lv_layer(lv))))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_STATUS, &info, NULL, dlid, 0, 0, 0, 0, flush, 0)))
		return_0;

	if (!info.exists)
		goto_out;

	if (dm_get_next_target(dmt, NULL, &start, &length, &type, &params)) {
		log_error("More then one table line found for %s.",
			  display_lvname(lv));
		goto out;
	}

	if (!type || strcmp(type, TARGET_NAME_VDO)) {
		log_error("Expected %s segment type but got %s instead.",
			  TARGET_NAME_VDO, type ? type : "NULL");
		goto out;
	}

	if (!(*vdo_params = dm_pool_strdup(dm->mem, params))) {
		log_error("Cannot duplicate VDO status params.");
		goto out;
	}

	status->mem = dm->mem;
	*vdo_status =  status;

	r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}


/*************************/
/*  NEW CODE STARTS HERE */
/*************************/

static int _dev_manager_lv_mknodes(const struct logical_volume *lv)
{
	char *name;

	if (!(name = dm_build_dm_name(lv->vg->cmd->mem, lv->vg->name,
				      lv->name, NULL)))
		return_0;

	return fs_add_lv(lv, name);
}

static int _dev_manager_lv_rmnodes(const struct logical_volume *lv)
{
	return fs_del_lv(lv);
}

static int _lv_has_mknode(const struct logical_volume *lv)
{
	return (lv_is_visible(lv) &&
		(!lv_is_thin_pool(lv) || lv_is_new_thin_pool(lv)));
}

int dev_manager_mknodes(const struct logical_volume *lv)
{
	struct dm_info dminfo;
	struct dm_task *dmt;
	char *name;
	int r = 0;

	if (!(name = dm_build_dm_name(lv->vg->cmd->mem, lv->vg->name, lv->name, NULL)))
		return_0;

	if (!(dmt = _setup_task_run(DM_DEVICE_MKNODES, &dminfo, name, NULL, 0, 0, 0, 0, 0, 0)))
		return_0;

	if (dminfo.exists) {
		/* read-only component LV is also made visible */
		if (_lv_has_mknode(lv) || (dminfo.read_only && lv_is_component(lv)))
			r = _dev_manager_lv_mknodes(lv);
	} else
		r = _dev_manager_lv_rmnodes(lv);

	dm_task_destroy(dmt);

	return r;
}

#ifdef UDEV_SYNC_SUPPORT
/*
 * Until the DM_UEVENT_GENERATED_FLAG was introduced in kernel patch
 * 856a6f1dbd8940e72755af145ebcd806408ecedd
 * some operations could not be performed by udev, requiring our fallback code.
 */
static int _dm_driver_has_stable_udev_support(void)
{
	char vsn[80];
	unsigned maj, min, patchlevel;

	return driver_version(vsn, sizeof(vsn)) &&
	       (sscanf(vsn, "%u.%u.%u", &maj, &min, &patchlevel) == 3) &&
	       (maj == 4 ? min >= 18 : maj > 4);
}

static int _check_udev_fallback(struct cmd_context *cmd)
{
	struct config_info *settings = &cmd->current_settings;

	if (settings->udev_fallback != -1)
		goto out;

	/*
	 * Use udev fallback automatically in case udev
	 * is disabled via DM_DISABLE_UDEV environment
	 * variable or udev rules are switched off.
	 */
	settings->udev_fallback = !settings->udev_rules ? 1 :
		find_config_tree_bool(cmd, activation_verify_udev_operations_CFG, NULL);

	/* Do not rely fully on udev if the udev support is known to be incomplete. */
	if (!settings->udev_fallback && !_dm_driver_has_stable_udev_support()) {
		log_very_verbose("Kernel driver has incomplete udev support so "
				 "LVM will check and perform some operations itself.");
		settings->udev_fallback = 1;
	}
out:
	return settings->udev_fallback;
}

#else /* UDEV_SYNC_SUPPORT */

static int _check_udev_fallback(struct cmd_context *cmd)
{
	/* We must use old node/symlink creation code if not compiled with udev support at all! */
	return cmd->current_settings.udev_fallback = 1;
}

#endif /* UDEV_SYNC_SUPPORT */

static uint16_t _get_udev_flags(struct dev_manager *dm, const struct logical_volume *lv,
				const char *layer, int noscan, int temporary,
				int visible_component)
{
	uint16_t udev_flags = 0;

	/*
	 * Instruct also libdevmapper to disable udev
	 * fallback in accordance to LVM2 settings.
	 */
	if (!_check_udev_fallback(dm->cmd))
		udev_flags |= DM_UDEV_DISABLE_LIBRARY_FALLBACK;

	/*
	 * Is this top-level and visible device?
	 * If not, create just the /dev/mapper content.
	 */
	/* FIXME: add target's method for this */
	if (lv_is_new_thin_pool(lv) || visible_component)
		/* New thin-pool is regular LV with -tpool UUID suffix. */
		udev_flags |= DM_UDEV_DISABLE_DISK_RULES_FLAG |
		              DM_UDEV_DISABLE_OTHER_RULES_FLAG;
	else if (layer || !lv_is_visible(lv) || lv_is_thin_pool(lv))
		udev_flags |= DM_UDEV_DISABLE_SUBSYSTEM_RULES_FLAG |
			      DM_UDEV_DISABLE_DISK_RULES_FLAG |
			      DM_UDEV_DISABLE_OTHER_RULES_FLAG;
	/*
	 * There's no need for other udev rules to touch special LVs with
	 * reserved names. We don't need to populate /dev/disk here either.
	 * Even if they happen to be visible and top-level.
	 */
	else if (is_reserved_lvname(lv->name))
		udev_flags |= DM_UDEV_DISABLE_DISK_RULES_FLAG |
			      DM_UDEV_DISABLE_OTHER_RULES_FLAG;

	/*
	 * Snapshots and origins could have the same rule applied that will
	 * give symlinks exactly the same name (e.g. a name based on
	 * filesystem UUID). We give preference to origins to make such
	 * naming deterministic (e.g. symlinks in /dev/disk/by-uuid).
	 */
	if (lv_is_cow(lv))
		udev_flags |= DM_UDEV_LOW_PRIORITY_FLAG;

	/*
	 * Finally, add flags to disable /dev/mapper and /dev/<vgname> content
	 * to be created by udev if it is requested by user's configuration.
	 * This is basically an explicit fallback to old node/symlink creation
	 * without udev.
	 */
	if (!dm->cmd->current_settings.udev_rules)
		udev_flags |= DM_UDEV_DISABLE_DM_RULES_FLAG |
			      DM_UDEV_DISABLE_SUBSYSTEM_RULES_FLAG;

	/*
	 * LVM subsystem specific flags.
	 */
	if (noscan)
		udev_flags |= DM_SUBSYSTEM_UDEV_FLAG0;

	if (temporary)
		udev_flags |= DM_UDEV_DISABLE_DISK_RULES_FLAG |
			      DM_UDEV_DISABLE_OTHER_RULES_FLAG;

	return udev_flags;
}

static int _add_lv_to_dtree(struct dev_manager *dm, struct dm_tree *dtree,
			    const struct logical_volume *lv, int origin_only);

static int _check_holder(struct dev_manager *dm, struct dm_tree *dtree,
			 const struct logical_volume *lv, uint32_t major,
			 const char *d_name)
{
	const char *default_uuid_prefix = dm_uuid_prefix();
	const size_t default_uuid_prefix_len = strlen(default_uuid_prefix);
	const char *name;
	const char *uuid;
	struct dm_info info;
	struct dm_task *dmt;
	struct logical_volume *lv_det;
	union lvid id;
	int dev, r = 0;

	errno = 0;
	dev = strtoll(d_name + 3, NULL, 10);
	if (errno) {
		log_error("Failed to parse dm device minor number from %s.", d_name);
		return 0;
	}

	if (!(dmt = _setup_task_run(DM_DEVICE_INFO, &info, NULL, NULL, NULL,
				    major, dev, 0, 0, 0)))
		return_0;

	if (info.exists) {
		uuid = dm_task_get_uuid(dmt);
		name = dm_task_get_name(dmt);

		log_debug_activation("Checking holder of %s  %s (" FMTu32 ":" FMTu32 ") %s.",
				     display_lvname(lv), uuid, info.major, info.minor,
				     name);

		/* Skip common uuid prefix */
		if (!strncmp(default_uuid_prefix, uuid, default_uuid_prefix_len))
			uuid += default_uuid_prefix_len;

		if (!strncmp(uuid, (char*)&lv->vg->id, sizeof(lv->vg->id)) &&
		    !dm_tree_find_node_by_uuid(dtree, uuid)) {
			/* trims any UUID suffix (i.e. -cow) */
			(void) dm_strncpy((char*)&id, uuid, 2 * sizeof(struct id) + 1);

			/* If UUID is not yet in dtree, look for matching LV */
			if (!(lv_det = find_lv_in_vg_by_lvid(lv->vg, &id))) {
				log_error("Cannot find holder with device name %s in VG %s.",
					  name, lv->vg->name);
				goto out;
			}

			if (lv_is_cow(lv_det))
				lv_det = origin_from_cow(lv_det);
			log_debug_activation("Found holder %s of %s.",
					     display_lvname(lv_det),
					     display_lvname(lv));
			if (!_add_lv_to_dtree(dm, dtree, lv_det, 0))
				goto_out;
		}
	}

        r = 1;
out:
	dm_task_destroy(dmt);

	return r;
}

/*
 * Add exiting devices which holds given LV device open.
 * This is used in case when metadata already do not contain information
 * i.e. PVMOVE is being finished and final table is going to be resumed.
 */
static int _add_holders_to_dtree(struct dev_manager *dm, struct dm_tree *dtree,
				 const struct logical_volume *lv, struct dm_info *info)
{
	const char *sysfs_dir = dm_sysfs_dir();
	char sysfs_path[PATH_MAX];
	struct dirent *dirent;
	DIR *d;
	int r = 0;

	/* Sysfs path of holders */
	if (dm_snprintf(sysfs_path, sizeof(sysfs_path), "%sblock/dm-" FMTu32
			"/holders", sysfs_dir, info->minor) < 0) {
		log_error("sysfs_path dm_snprintf failed.");
		return 0;
	}

	if (!(d = opendir(sysfs_path))) {
		log_sys_error("opendir", sysfs_path);
		return 0;
	}

	while ((dirent = readdir(d)))
		/* Expects minor is added to 'dm-' prefix */
		if (!strncmp(dirent->d_name, "dm-", 3) &&
		    !_check_holder(dm, dtree, lv, info->major, dirent->d_name))
			goto_out;

	r = 1;
out:
	if (closedir(d))
		log_sys_debug("closedir", "holders");

	return r;
}

static int _add_dev_to_dtree(struct dev_manager *dm, struct dm_tree *dtree,
			     const struct logical_volume *lv, const char *layer)
{
	char *dlid, *name;
	struct dm_info info, info2;

	if (!(name = dm_build_dm_name(dm->mem, lv->vg->name, lv->name, layer)))
		return_0;

	if (!(dlid = build_dm_uuid(dm->mem, lv, layer)))
		return_0;

	if (!_info(dm->cmd, name, dlid, 1, 0, &info, NULL, NULL))
		return_0;

	/*
	 * For top level volumes verify that existing device match
	 * requested major/minor and that major/minor pair is available for use
	 */
	if (!layer && lv->major != -1 && lv->minor != -1) {
		/*
		 * FIXME compare info.major with lv->major if multiple major support
		 */
		if (info.exists && ((int) info.minor != lv->minor)) {
			log_error("Volume %s (%" PRIu32 ":%" PRIu32")"
				  " differs from already active device "
				  "(%" PRIu32 ":%" PRIu32").",
				  display_lvname(lv), lv->major, lv->minor,
				  info.major, info.minor);
			return 0;
		}
		if (!info.exists && _info_by_dev(lv->major, lv->minor, &info2) &&
		    info2.exists) {
			log_error("The requested major:minor pair "
				  "(%" PRIu32 ":%" PRIu32") is already used.",
				  lv->major, lv->minor);
			return 0;
		}
	}

	if (info.exists && !dm_tree_add_dev_with_udev_flags(dtree, info.major, info.minor,
							    _get_udev_flags(dm, lv, layer,
									    0, 0, 0))) {
		log_error("Failed to add device (%" PRIu32 ":%" PRIu32") to dtree.",
			  info.major, info.minor);
		return 0;
	}

	if (info.exists && dm->track_pending_delete) {
		log_debug_activation("Tracking pending delete for %s (%s).",
				     display_lvname(lv), dlid);
		if (!str_list_add(dm->mem, &dm->pending_delete, dlid))
			return_0;
	}

	/*
	 * Find holders of existing active LV where name starts with 'pvmove',
	 * but it's not anymore PVMOVE LV and also it's not PVMOVE _mimage
	 */
	if (info.exists && !lv_is_pvmove(lv) &&
	    !strchr(lv->name, '_') && !strncmp(lv->name, "pvmove", 6))
		if (!_add_holders_to_dtree(dm, dtree, lv, &info))
			return_0;

	return 1;
}

struct pool_cb_data {
	struct dev_manager *dm;
	const struct logical_volume *pool_lv;

	int skip_zero;  /* to skip zeroed device header (check first 64B) */
	int exec;       /* which binary to call */
	int opts;
	struct {
		unsigned maj;
		unsigned min;
		unsigned patch;
	} version;
	const char *global;
};

/*
 * Simple version of check function calling 'tool -V'
 *
 * Returns 1 if the tool's version is equal or better to given.
 * Otherwise it returns 0.
 */
static int _check_tool_version(struct cmd_context *cmd, const char *tool,
			       unsigned maj, unsigned min, unsigned patch)
{
	const char *argv[] = { tool, "-V", NULL };
	struct pipe_data pdata;
	FILE *f;
	char buf[128] = { 0 };
	char *nl;
	unsigned v_maj, v_min, v_patch;
	int ret = 0;

	if (!(f = pipe_open(cmd, argv, 0, &pdata))) {
		log_warn("WARNING: Cannot read output from %s.", argv[0]);
	} else {
		if (fgets(buf, sizeof(buf) - 1, f) &&
		    (sscanf(buf, "%u.%u.%u", &v_maj, &v_min, &v_patch) == 3)) {
			if ((v_maj > maj) ||
			    ((v_maj == maj) &&
			     ((v_min > min) ||
			      (v_min == min && v_patch >= patch))))
				ret = 1;

			if ((nl = strchr(buf, '\n')))
				nl[0] = 0; /* cut newline away */

			log_verbose("Found version of %s %s is %s then requested %u.%u.%u.",
				    argv[0], buf, ret ? "better" : "older", maj, min, patch);
		} else
			log_warn("WARNING: Cannot parse output '%s' from %s.", buf, argv[0]);

		(void) pipe_close(&pdata);
	}

	return ret;
}

static int _pool_callback(struct dm_tree_node *node,
			  dm_node_callback_t type, void *cb_data)
{
	int ret, status = 0, fd;
	const struct dm_config_node *cn;
	const struct dm_config_value *cv;
	const struct pool_cb_data *data = cb_data;
	const struct logical_volume *pool_lv = data->pool_lv;
	const struct logical_volume *mlv = first_seg(pool_lv)->metadata_lv;
	long buf[64 / sizeof(long)]; /* buffer for short disk header (64B) */
	int args = 0;
	char *mpath;
	const char *argv[19] = { /* Max supported 15 args */
		find_config_tree_str_allow_empty(pool_lv->vg->cmd, data->exec, NULL)
	};

	if (!*argv[0]) /* *_check tool is unconfigured/disabled with "" setting */
		return 1;

	if (!(mpath = lv_dmpath_dup(data->dm->mem, mlv))) {
		log_error("Failed to build device path for checking pool metadata %s.",
			  display_lvname(mlv));
		return 0;
	}

	if (data->skip_zero) {
		if ((fd = open(mpath, O_RDONLY)) < 0) {
			log_sys_error("open", mpath);
			return 0;
		}
		/* let's assume there is no problem to read 64 bytes */
		if (read(fd, buf, sizeof(buf)) < (int)sizeof(buf)) {
			log_sys_error("read", mpath);
			if (close(fd))
				log_sys_error("close", mpath);
			return 0;
		}
		for (ret = 0; ret < (int) DM_ARRAY_SIZE(buf); ++ret)
			if (buf[ret])
				break;

		if (close(fd))
			log_sys_error("close", mpath);

		if (ret == (int) DM_ARRAY_SIZE(buf)) {
			log_debug_activation("Metadata checking skipped, detected empty disk header on %s.",
					     mpath);
			return 1;
		}
	}

	if (!(cn = find_config_tree_array(mlv->vg->cmd, data->opts, NULL))) {
		log_error(INTERNAL_ERROR "Unable to find configuration for pool check options.");
		return 0;
	}

	for (cv = cn->v; cv && args < 16; cv = cv->next) {
		if (cv->type != DM_CFG_STRING) {
			log_error("Invalid string in config file: "
				  "global/%s_check_options.",
				  data->global);
			return 0;
		}
		argv[++args] = cv->v.str;
	}

	if (args == 16) {
		log_error("Too many options for %s command.", argv[0]);
		return 0;
	}

	argv[++args] = mpath;

	if (!(ret = exec_cmd(pool_lv->vg->cmd, (const char * const *)argv,
			     &status, 0))) {
		if (status == ENOENT) {
			log_warn("WARNING: Check is skipped, please install recommended missing binary %s!",
				 argv[0]);
			return 1;
		}

		if ((data->version.maj || data->version.min || data->version.patch) &&
		    !_check_tool_version(pool_lv->vg->cmd, argv[0],
					 data->version.maj, data->version.min, data->version.patch)) {
			log_warn("WARNING: Check is skipped, please upgrade installed version of %s!",
				 argv[0]);
			return 1;
		}
		switch (type) {
		case DM_NODE_CALLBACK_PRELOADED:
			log_err_once("Check of pool %s failed (status:%d). "
				     "Manual repair required!",
				     display_lvname(pool_lv), status);
			break;
		default:
			log_warn("WARNING: Integrity check of metadata for pool "
				 "%s failed.", display_lvname(pool_lv));
		}
		/*
		 * FIXME: What should we do here??
		 *
		 * Maybe mark the node, so it's not activating
		 * as pool but as error/linear and let the
		 * dm tree resolve the issue.
		 */
	}

	return ret;
}

static int _pool_register_callback(struct dev_manager *dm,
				   struct dm_tree_node *node,
				   const struct logical_volume *lv)
{
	struct pool_cb_data *data;

	/* Do not skip metadata of testing even for unused thin pools */
#if 0
	/* Skip metadata testing for unused thin pool. */
	if (lv_is_thin_pool(lv) &&
	    (!first_seg(lv)->transaction_id ||
	     ((first_seg(lv)->transaction_id == 1) &&
	      pool_has_message(first_seg(lv), NULL, 0))))
		return 1;
#endif

	if (!(data = dm_pool_zalloc(dm->mem, sizeof(*data)))) {
		log_error("Failed to allocated path for callback.");
		return 0;
	}

	data->dm = dm;

	if (lv_is_thin_pool(lv)) {
		data->pool_lv = lv;
		data->skip_zero = 1;
		data->exec = global_thin_check_executable_CFG;
		data->opts = global_thin_check_options_CFG;
		data->global = "thin";
	} else if (lv_is_cache(lv)) { /* cache pool */
		data->pool_lv = first_seg(lv)->pool_lv;
		data->skip_zero = 1; /* cheap read-error detection */
		data->exec = global_cache_check_executable_CFG;
		data->opts = global_cache_check_options_CFG;
		data->global = "cache";
		if (first_seg(first_seg(lv)->pool_lv)->cache_metadata_format > 1) {
			data->version.maj = 0;
			data->version.min = 7;
		}
	} else {
		log_error(INTERNAL_ERROR "Registering unsupported pool callback.");
		return 0;
	}

	dm_tree_node_set_callback(node, _pool_callback, data);

	return 1;
}

/* Declaration to resolve suspend tree and message passing for thin-pool */
static int _add_target_to_dtree(struct dev_manager *dm,
				struct dm_tree_node *dnode,
				struct lv_segment *seg,
				struct lv_activate_opts *laopts);
/*
 * Add LV and any known dependencies
 */
static int _add_lv_to_dtree(struct dev_manager *dm, struct dm_tree *dtree,
			    const struct logical_volume *lv, int origin_only)
{
	uint32_t s;
	struct seg_list *sl;
	struct dm_list *snh;
	struct lv_segment *seg;
	struct dm_tree_node *node;
	const char *uuid;
	const struct logical_volume *plv;

	if (lv_is_pvmove(lv) && (dm->track_pvmove_deps == 2))
		return 1; /* Avoid rechecking of already seen pvmove LV */

	if (lv_is_cache_pool(lv)) {
		if (!dm_list_empty(&lv->segs_using_this_lv)) {
			if (!_add_lv_to_dtree(dm, dtree, seg_lv(first_seg(lv), 0), 0))
				return_0;
			if (!_add_lv_to_dtree(dm, dtree, first_seg(lv)->metadata_lv, 0))
				return_0;
			/* Cache pool does not have a real device node */
			return 1;
		}
		/* Unused cache pool is activated as metadata */
	}

	if (!origin_only && !_add_dev_to_dtree(dm, dtree, lv, NULL))
		return_0;

	/* FIXME Can we avoid doing this every time? */
	/* Reused also for lv_is_external_origin(lv) */
	if (!_add_dev_to_dtree(dm, dtree, lv, "real"))
		return_0;

	if (!origin_only && !_add_dev_to_dtree(dm, dtree, lv, "cow"))
		return_0;

	if (origin_only && lv_is_thin_volume(lv)) {
		if (!_add_dev_to_dtree(dm, dtree, lv, lv_layer(lv)))
			return_0;
#if 0
		/* ? Use origin_only to avoid 'deep' thin pool suspend ? */
		/* FIXME Implement dm_tree_node_skip_childrens optimisation */
		if (!(uuid = build_dm_uuid(dm->mem, lv, lv_layer(lv))))
			return_0;
		if ((node = dm_tree_find_node_by_uuid(dtree, uuid)))
			dm_tree_node_skip_childrens(node, 1);
#endif
	}

	if (origin_only && dm->activation && dm->track_external_lv_deps &&
	    lv_is_external_origin(lv)) {
		/* Find possible users of external origin lv */
		dm->track_external_lv_deps = 0; /* avoid recursion */
		dm_list_iterate_items(sl, &lv->segs_using_this_lv)
			/* Match only external_lv users */
			if ((sl->seg->external_lv == lv) &&
			    !_add_lv_to_dtree(dm, dtree, sl->seg->lv, 1))
				return_0;
		dm->track_external_lv_deps = 1;
	}

	if (lv_is_thin_pool(lv)) {
		/*
		 * For both origin_only and !origin_only
		 * skips test for -tpool-real and tpool-cow
		 */
		if (!_add_dev_to_dtree(dm, dtree, lv, lv_layer(lv)))
			return_0;

		/*
		 * TODO: change API and move this code
		 * Could be easier to handle this in _add_dev_to_dtree()
		 * and base this according to info.exists ?
		 */
		if (!dm->activation) {
			if (!(uuid = build_dm_uuid(dm->mem, lv, lv_layer(lv))))
				return_0;
			if ((node = dm_tree_find_node_by_uuid(dtree, uuid))) {
				if (origin_only) {
					struct lv_activate_opts laopts = {
						.origin_only = 1,
						.send_messages = 1 /* Node with messages */
					};
					/*
					 * Add some messsages if right node exist in the table only
					 * when building SUSPEND tree for origin-only thin-pool.
					 *
					 * TODO: Fix call of '_add_target_to_dtree()' to add message
					 * to thin-pool node as we already know the pool node exists
					 * in the table. Any better/cleaner API way ?
					 *
					 * Probably some 'new' target method to add messages for any node?
					 */
					if (dm->suspend &&
					    !dm_list_empty(&(first_seg(lv)->thin_messages)) &&
					    !_add_target_to_dtree(dm, node, first_seg(lv), &laopts))
						return_0;
				} else {
					/* Setup callback for non-activation partial tree */
					/* Activation gets own callback when needed */
					/* TODO: extend _cached_dm_info() to return dnode */
					if (!_pool_register_callback(dm, node, lv))
						return_0;
				}
			}
		}
	}

	if (lv_is_cache(lv)) {
		if (!origin_only && !dm->activation && !dm->track_pending_delete) {
			/* Setup callback for non-activation partial tree */
			/* Activation gets own callback when needed */
			/* TODO: extend _cached_dm_info() to return dnode */
			if (!(uuid = build_dm_uuid(dm->mem, lv, lv_layer(lv))))
				return_0;
			if ((node = dm_tree_find_node_by_uuid(dtree, uuid)) &&
			    !_pool_register_callback(dm, node, lv))
				return_0;
		}
	}

	/* Add any snapshots of this LV */
	if (!origin_only && lv_is_origin(lv))
		dm_list_iterate(snh, &lv->snapshot_segs)
			if (!_add_lv_to_dtree(dm, dtree, dm_list_struct_base(snh, struct lv_segment, origin_list)->cow, 0))
				return_0;

	if (dm->activation && !origin_only && lv_is_merging_origin(lv) &&
	    !_add_lv_to_dtree(dm, dtree, find_snapshot(lv)->lv, 1))
		return_0;

	/* Add any LVs referencing a PVMOVE LV unless told not to. */
	if ((dm->track_pvmove_deps == 1) && lv_is_pvmove(lv)) {
		dm->track_pvmove_deps = 2; /* Mark as already seen */
		dm_list_iterate_items(sl, &lv->segs_using_this_lv) {
			/* If LV is snapshot COW - whole snapshot needs reload */
			plv = lv_is_cow(sl->seg->lv) ? origin_from_cow(sl->seg->lv) : sl->seg->lv;
			if (!_add_lv_to_dtree(dm, dtree, plv, 0))
				return_0;
		}
		dm->track_pvmove_deps = 1;
	}

	if (!dm->track_pending_delete)
		dm_list_iterate_items(sl, &lv->segs_using_this_lv) {
			if (lv_is_pending_delete(sl->seg->lv)) {
				/* LV is referenced by 'cache pending delete LV */
				dm->track_pending_delete = 1;
				if (!_add_lv_to_dtree(dm, dtree, sl->seg->lv, origin_only))
					return_0;
				dm->track_pending_delete = 0;
			}
		}

	/* Add any LVs used by segments in this LV */
	dm_list_iterate_items(seg, &lv->segments) {
		if (seg->external_lv && dm->track_external_lv_deps &&
		    !_add_lv_to_dtree(dm, dtree, seg->external_lv, 1)) /* stack */
			return_0;
		if (seg->log_lv &&
		    !_add_lv_to_dtree(dm, dtree, seg->log_lv, 0))
			return_0;
		if (seg->metadata_lv &&
		    !_add_lv_to_dtree(dm, dtree, seg->metadata_lv, 0))
			return_0;
		if (seg->pool_lv &&
		    (lv_is_cache_pool(seg->pool_lv) || dm->track_external_lv_deps) &&
		    /* When activating and not origin_only detect linear 'overlay' over pool */
		    !_add_lv_to_dtree(dm, dtree, seg->pool_lv, dm->activation ? origin_only : 1))
			return_0;

		for (s = 0; s < seg->area_count; s++) {
			if (seg_type(seg, s) == AREA_LV && seg_lv(seg, s) &&
			    /* origin only for cache without pending delete */
			    (!dm->track_pending_delete || !lv_is_cache(lv)) &&
			    !_add_lv_to_dtree(dm, dtree, seg_lv(seg, s), 0))
				return_0;
			if (seg_is_raid_with_meta(seg) && seg->meta_areas && seg_metalv(seg, s) &&
			    !_add_lv_to_dtree(dm, dtree, seg_metalv(seg, s), 0))
				return_0;
		}

		/* When activating, detect merging LV presence */
		if (dm->activation && seg->merge_lv &&
		    !_add_lv_to_dtree(dm, dtree, seg->merge_lv, 1))
			return_0;
	}

	return 1;
}

static struct dm_tree *_create_partial_dtree(struct dev_manager *dm, const struct logical_volume *lv, int origin_only)
{
	struct dm_tree *dtree;

	if (!(dtree = dm_tree_create())) {
		log_debug_activation("Partial dtree creation failed for %s.",
				     display_lvname(lv));
		return NULL;
	}

	dm_tree_set_optional_uuid_suffixes(dtree, &uuid_suffix_list[0]);

	if (!_add_lv_to_dtree(dm, dtree, lv, (lv_is_origin(lv) || lv_is_thin_volume(lv) || lv_is_thin_pool(lv)) ? origin_only : 0))
		goto_bad;

	return dtree;

bad:
	dm_tree_free(dtree);
	return NULL;
}

static char *_add_error_or_zero_device(struct dev_manager *dm, struct dm_tree *dtree,
				       struct lv_segment *seg, int s, int use_zero)
{
	char *dlid, *name;
	char errid[32];
	struct dm_tree_node *node;
	struct lv_segment *seg_i;
	struct dm_info info;
	int segno = -1, i = 0;
	uint64_t size = (uint64_t) _seg_len(seg) * seg->lv->vg->extent_size;

	dm_list_iterate_items(seg_i, &seg->lv->segments) {
		if (seg == seg_i) {
			segno = i;
			break;
		}
		++i;
	}

	if (segno < 0) {
		log_error(INTERNAL_ERROR "_add_error_or_zero_device called with bad segment.");
		return NULL;
	}

	sprintf(errid, "missing_%d_%d", segno, s);

	if (!(dlid = build_dm_uuid(dm->mem, seg->lv, errid)))
		return_NULL;

	if (!(name = dm_build_dm_name(dm->mem, seg->lv->vg->name,
				      seg->lv->name, errid)))
		return_NULL;

	if (!_info(dm->cmd, name, dlid, 1, 0, &info, NULL, NULL))
		return_NULL;

	if (!info.exists) {
		/* Create new node */
		if (!(node = dm_tree_add_new_dev(dtree, name, dlid, 0, 0, 0, 0, 0)))
			return_NULL;

		if (use_zero) {
			if (!dm_tree_node_add_zero_target(node, size))
				return_NULL;
		} else
			if (!dm_tree_node_add_error_target(node, size))
				return_NULL;
	} else {
		/* Already exists */
		if (!dm_tree_add_dev(dtree, info.major, info.minor)) {
			log_error("Failed to add device (%" PRIu32 ":%" PRIu32") to dtree.",
				  info.major, info.minor);
			return NULL;
		}
	}

	return dlid;
}

static int _add_error_area(struct dev_manager *dm, struct dm_tree_node *node,
			   struct lv_segment *seg, int s)
{
	char *dlid;
	uint64_t extent_size = seg->lv->vg->extent_size;
	int use_zero = !strcmp(dm->cmd->stripe_filler, TARGET_NAME_ZERO) ? 1 : 0;

	if (!strcmp(dm->cmd->stripe_filler, TARGET_NAME_ERROR) || use_zero) {
		/*
		 * FIXME, the tree pointer is first field of dm_tree_node, but
		 * we don't have the struct definition available.
		 */
		struct dm_tree **tree = (struct dm_tree **) node;
		if (!(dlid = _add_error_or_zero_device(dm, *tree, seg, s, use_zero)))
			return_0;
		if (!dm_tree_node_add_target_area(node, NULL, dlid, extent_size * seg_le(seg, s)))
			return_0;
	} else
		if (!dm_tree_node_add_target_area(node, dm->cmd->stripe_filler, NULL, UINT64_C(0)))
			return_0;

	return 1;
}

int add_areas_line(struct dev_manager *dm, struct lv_segment *seg,
		   struct dm_tree_node *node, uint32_t start_area,
		   uint32_t areas)
{
	uint64_t extent_size = seg->lv->vg->extent_size;
	uint32_t s;
	char *dlid;
	struct stat info;
	const char *name;
	unsigned num_error_areas = 0;
	unsigned num_existing_areas = 0;

	/* FIXME Avoid repeating identical stat in dm_tree_node_add_target_area */
	for (s = start_area; s < areas; s++) {
		if ((seg_type(seg, s) == AREA_PV &&
		     (!seg_pvseg(seg, s) || !seg_pv(seg, s) || !seg_dev(seg, s) ||
		       !(name = dev_name(seg_dev(seg, s))) || !*name ||
		       stat(name, &info) < 0 || !S_ISBLK(info.st_mode))) ||
		    (seg_type(seg, s) == AREA_LV && !seg_lv(seg, s))) {
			if (!seg->lv->vg->cmd->partial_activation) {
				if (!seg->lv->vg->cmd->degraded_activation ||
				    !lv_is_raid_type(seg->lv)) {
					log_error("Aborting.  LV %s is now incomplete "
						  "and '--activationmode partial' was not specified.",
						  display_lvname(seg->lv));
					return 0;
				}
			}
			if (!_add_error_area(dm, node, seg, s))
				return_0;
			num_error_areas++;
		} else if (seg_type(seg, s) == AREA_PV) {
			if (!dm_tree_node_add_target_area(node, dev_name(seg_dev(seg, s)), NULL,
				    (seg_pv(seg, s)->pe_start + (extent_size * seg_pe(seg, s)))))
				return_0;
			num_existing_areas++;
		} else if (seg_is_raid(seg)) {
			/*
			 * RAID can handle unassigned areas.  It simple puts
			 * '- -' in for the metadata/data device pair.  This
			 * is a valid way to indicate to the RAID target that
			 * the device is missing.
			 *
			 * If an image is marked as VISIBLE_LV and !LVM_WRITE,
			 * it means the device has temporarily been extracted
			 * from the array.  It may come back at a future date,
			 * so the bitmap must track differences.  Again, '- -'
			 * is used in the CTR table.
			 */
			if ((seg_type(seg, s) == AREA_UNASSIGNED) ||
			    (lv_is_visible(seg_lv(seg, s)) &&
			     !(seg_lv(seg, s)->status & LVM_WRITE))) {
				/* One each for metadata area and data area */
				if (!dm_tree_node_add_null_area(node, 0) ||
				    !dm_tree_node_add_null_area(node, 0))
					return_0;
				continue;
			}

			if (seg->meta_areas && seg_metalv(seg, s)) {
				if (!(dlid = build_dm_uuid(dm->mem, seg_metalv(seg, s), NULL)))
					return_0;
				if (!dm_tree_node_add_target_area(node, NULL, dlid, extent_size * seg_metale(seg, s)))
					return_0;
			} else if (!dm_tree_node_add_null_area(node, 0))
				return_0;

			if (!(dlid = build_dm_uuid(dm->mem, seg_lv(seg, s), NULL)))
				return_0;
			if (!dm_tree_node_add_target_area(node, NULL, dlid, extent_size * seg_le(seg, s)))
				return_0;
		} else if (seg_type(seg, s) == AREA_LV) {

			if (!(dlid = build_dm_uuid(dm->mem, seg_lv(seg, s), NULL)))
				return_0;
			if (!dm_tree_node_add_target_area(node, NULL, dlid, extent_size * seg_le(seg, s)))
				return_0;
		} else {
			log_error(INTERNAL_ERROR "Unassigned area found in LV %s.",
				  display_lvname(seg->lv));
			return 0;
		}
	}

        if (num_error_areas) {
		/* Thins currently do not support partial activation */
		if (lv_is_thin_type(seg->lv)) {
			log_error("Cannot activate %s: pool incomplete.",
				  display_lvname(seg->lv));
			return 0;
		}
	}

	return 1;
}

static int _add_layer_target_to_dtree(struct dev_manager *dm,
				      struct dm_tree_node *dnode,
				      const struct logical_volume *lv)
{
	const char *layer_dlid;

	if (!(layer_dlid = build_dm_uuid(dm->mem, lv, lv_layer(lv))))
		return_0;

	/* Add linear mapping over layered LV */
	if (!add_linear_area_to_dtree(dnode, lv->size, lv->vg->extent_size,
				      lv->vg->cmd->use_linear_target,
				      lv->vg->name, lv->name) ||
	    !dm_tree_node_add_target_area(dnode, NULL, layer_dlid, 0))
		return_0;

	return 1;
}

static int _add_origin_target_to_dtree(struct dev_manager *dm,
				       struct dm_tree_node *dnode,
				       const struct logical_volume *lv)
{
	const char *real_dlid;

	if (!(real_dlid = build_dm_uuid(dm->mem, lv, "real")))
		return_0;

	if (!dm_tree_node_add_snapshot_origin_target(dnode, lv->size, real_dlid))
		return_0;

	return 1;
}

static int _add_snapshot_merge_target_to_dtree(struct dev_manager *dm,
					       struct dm_tree_node *dnode,
					       const struct logical_volume *lv)
{
	const char *origin_dlid, *cow_dlid, *merge_dlid;
	struct lv_segment *merging_snap_seg = find_snapshot(lv);

	if (!lv_is_merging_origin(lv)) {
		log_error(INTERNAL_ERROR "LV %s is not merging snapshot.",
			  display_lvname(lv));
		return 0;
	}

	if (!(origin_dlid = build_dm_uuid(dm->mem, lv, "real")))
		return_0;

	if (!(cow_dlid = build_dm_uuid(dm->mem, merging_snap_seg->cow, "cow")))
		return_0;

	if (!(merge_dlid = build_dm_uuid(dm->mem, merging_snap_seg->cow, NULL)))
		return_0;

	if (!dm_tree_node_add_snapshot_merge_target(dnode, lv->size, origin_dlid,
						    cow_dlid, merge_dlid,
						    merging_snap_seg->chunk_size))
		return_0;

	return 1;
}

static int _add_snapshot_target_to_dtree(struct dev_manager *dm,
					 struct dm_tree_node *dnode,
					 const struct logical_volume *lv,
					 struct lv_activate_opts *laopts)
{
	const char *origin_dlid;
	const char *cow_dlid;
	struct lv_segment *snap_seg;
	uint64_t size;

	if (!(snap_seg = find_snapshot(lv))) {
		log_error("Couldn't find snapshot for '%s'.",
			  display_lvname(lv));
		return 0;
	}

	if (!(origin_dlid = build_dm_uuid(dm->mem, snap_seg->origin, "real")))
		return_0;

	if (!(cow_dlid = build_dm_uuid(dm->mem, snap_seg->cow, "cow")))
		return_0;

	size = (uint64_t) snap_seg->len * snap_seg->origin->vg->extent_size;

	if (!laopts->no_merging && lv_is_merging_cow(lv)) {
		/* cow is to be merged so load the error target */
		if (!dm_tree_node_add_error_target(dnode, size))
			return_0;
	}
	else if (!dm_tree_node_add_snapshot_target(dnode, size, origin_dlid,
						   cow_dlid, 1, snap_seg->chunk_size))
		return_0;

	return 1;
}

static int _add_target_to_dtree(struct dev_manager *dm,
				struct dm_tree_node *dnode,
				struct lv_segment *seg,
				struct lv_activate_opts *laopts)
{
	uint64_t extent_size = seg->lv->vg->extent_size;

	if (!seg->segtype->ops->add_target_line) {
		log_error(INTERNAL_ERROR "_emit_target cannot handle "
			  "segment type %s.", lvseg_name(seg));
		return 0;
	}

	return seg->segtype->ops->add_target_line(dm, dm->mem, dm->cmd,
						  &dm->target_state, seg,
						  laopts, dnode,
						  extent_size * _seg_len(seg),
						  &dm->pvmove_mirror_count);
}

static int _add_new_lv_to_dtree(struct dev_manager *dm, struct dm_tree *dtree,
				const struct logical_volume *lv,
				struct lv_activate_opts *laopts,
				const char *layer);

static int _add_new_external_lv_to_dtree(struct dev_manager *dm,
					 struct dm_tree *dtree,
					 struct logical_volume *external_lv,
					 struct lv_activate_opts *laopts)
{
	struct seg_list *sl;

	/* Do not want to recursively add externals again */
	if (!dm->track_external_lv_deps)
		return 1;

	/*
	 * Any LV can have only 1 external origin, so we will
	 * process all LVs related to this LV, and we want to
	 * skip repeated invocation of external lv processing
	 */
	dm->track_external_lv_deps = 0;

	log_debug_activation("Adding external origin LV %s and all active users.",
			     display_lvname(external_lv));

	if (!_add_new_lv_to_dtree(dm, dtree, external_lv, laopts,
				  lv_layer(external_lv)))
		return_0;

	/*
	 * Add all ACTIVE LVs using this external origin LV. This is
	 * needed because of conversion of thin which could have been
	 * also an old-snapshot to external origin.
	 */
	dm_list_iterate_items(sl, &external_lv->segs_using_this_lv)
		if ((sl->seg->external_lv == external_lv) &&
		    /* Add only active layered devices (also avoids loop) */
		    _cached_dm_info(dm->mem, dtree, sl->seg->lv,
				    lv_layer(sl->seg->lv)) &&
		    !_add_new_lv_to_dtree(dm, dtree, sl->seg->lv,
					  laopts, lv_layer(sl->seg->lv)))
			return_0;

	log_debug_activation("Finished adding external origin LV %s and all active users.",
			     display_lvname(external_lv));

	dm->track_external_lv_deps = 1;

	return 1;
}

static int _add_segment_to_dtree(struct dev_manager *dm,
				 struct dm_tree *dtree,
				 struct dm_tree_node *dnode,
				 struct lv_segment *seg,
				 struct lv_activate_opts *laopts,
				 const char *layer)
{
	uint32_t s;
	struct lv_segment *seg_present;
	const struct segment_type *segtype;
	const char *target_name;

	/* Ensure required device-mapper targets are loaded */
	seg_present = find_snapshot(seg->lv) ? : seg;
	segtype = seg_present->segtype;

	target_name = (segtype->ops->target_name ?
		       segtype->ops->target_name(seg_present, laopts) :
		       segtype->name);

	log_debug_activation("Checking kernel supports %s segment type for %s%s%s",
			     target_name, display_lvname(seg->lv),
			     layer ? "-" : "", layer ? : "");

	if (segtype->ops->target_present &&
	    !segtype->ops->target_present(seg_present->lv->vg->cmd,
					  seg_present, NULL)) {
		log_error("Can't process LV %s: %s target support missing "
			  "from kernel?", display_lvname(seg->lv), target_name);
		return 0;
	}

	/* Add external origin layer */
	if (seg->external_lv &&
	    !_add_new_external_lv_to_dtree(dm, dtree, seg->external_lv, laopts))
		return_0;

	/* Add mirror log */
	if (seg->log_lv &&
	    !_add_new_lv_to_dtree(dm, dtree, seg->log_lv, laopts, NULL))
		return_0;

	/* Add pool metadata */
	if (seg->metadata_lv &&
	    !_add_new_lv_to_dtree(dm, dtree, seg->metadata_lv, laopts, NULL))
		return_0;

	/* Add pool layer */
	if (seg->pool_lv && !laopts->origin_only &&
	    !_add_new_lv_to_dtree(dm, dtree, seg->pool_lv, laopts,
				  lv_layer(seg->pool_lv)))
		return_0;

	/* Add any LVs used by this segment */
	for (s = 0; s < seg->area_count; ++s) {
		if ((seg_type(seg, s) == AREA_LV) &&
		    /* do not bring up tracked image */
		    !lv_is_raid_image_with_tracking(seg_lv(seg, s)) &&
		    /* origin only for cache without pending delete */
		    (!dm->track_pending_delete || !seg_is_cache(seg)) &&
		    !_add_new_lv_to_dtree(dm, dtree, seg_lv(seg, s),
					  laopts, NULL))
			return_0;
		if (seg_is_raid_with_meta(seg) && seg->meta_areas && seg_metalv(seg, s) &&
		    !lv_is_raid_image_with_tracking(seg_lv(seg, s)) &&
		    !_add_new_lv_to_dtree(dm, dtree, seg_metalv(seg, s),
					  laopts, NULL))
			return_0;
	}

	if (dm->track_pending_delete) {
		/* Replace target and all its used devs with error mapping */
		log_debug_activation("Using error for pending delete %s.",
				     display_lvname(seg->lv));
		if (!dm_tree_node_add_error_target(dnode, (uint64_t)seg->lv->vg->extent_size * _seg_len(seg)))
			return_0;
	} else if (!_add_target_to_dtree(dm, dnode, seg, laopts))
		return_0;

	return 1;
}

static int _add_new_lv_to_dtree(struct dev_manager *dm, struct dm_tree *dtree,
				const struct logical_volume *lv, struct lv_activate_opts *laopts,
				const char *layer)
{
	struct lv_segment *seg;
	struct lv_layer *lvlayer;
	struct seg_list *sl;
	struct dm_list *snh;
	struct dm_tree_node *dnode;
	const struct dm_info *dinfo;
	char *name, *dlid;
	uint32_t max_stripe_size = UINT32_C(0);
	uint32_t read_ahead = lv->read_ahead;
	uint32_t read_ahead_flags = UINT32_C(0);
	int save_pending_delete = dm->track_pending_delete;
	int merge_in_progress = 0;

	log_debug_activation("Adding new LV %s%s%s to dtree", display_lvname(lv),
			     layer ? "-" : "", layer ? : "");
	/* LV with pending delete is never put new into a table */
	if (lv_is_pending_delete(lv) && !_cached_dm_info(dm->mem, dtree, lv, NULL))
		return 1; /* Replace with error only when already exists */

	if (lv_is_cache_pool(lv) &&
	    !dm_list_empty(&lv->segs_using_this_lv)) {
		/* cache pool is 'meta' LV and does not have a real device node */
		if (!_add_new_lv_to_dtree(dm, dtree, seg_lv(first_seg(lv), 0), laopts, NULL))
			return_0;
		if (!_add_new_lv_to_dtree(dm, dtree, first_seg(lv)->metadata_lv, laopts, NULL))
			return_0;
		return 1;
	}

	/* FIXME Seek a simpler way to lay out the snapshot-merge tree. */

	if (!layer && lv_is_merging_origin(lv)) {
		seg = find_snapshot(lv);
		/*
		 * Prevent merge if merge isn't currently possible:
		 * either origin or merging snapshot are open
		 * - for old snaps use "snapshot-merge" if it is already in use
		 * - open_count is always retrieved (as of dm-ioctl 4.7.0)
		 *   so just use the tree's existing nodes' info
		 */
		if ((dinfo = _cached_dm_info(dm->mem, dtree, lv, NULL))) {
			/* Merging origin LV is present, check if mergins is already running. */
			if ((seg_is_thin_volume(seg) && _thin_lv_has_device_id(dm->mem, lv, NULL, seg->device_id)) ||
			    (!seg_is_thin_volume(seg) && lv_has_target_type(dm->mem, lv, NULL, TARGET_NAME_SNAPSHOT_MERGE))) {
				log_debug_activation("Merging of snapshot volume %s to origin %s is in progress.",
						     display_lvname(seg->lv), display_lvname(seg->lv));
				merge_in_progress = 1; /* Merge is already running */
			} /* Merge is not yet running, so check if it can be started */
			else if (laopts->resuming) {
				log_debug_activation("Postponing pending snapshot merge for origin %s, "
						     "merge was not started before suspend.",
						     display_lvname(lv));
				laopts->no_merging = 1; /* Cannot be reloaded in suspend */
			} /* Non-resuming merge requires origin to be unused */
			else if (dinfo->open_count) {
				log_debug_activation("Postponing pending snapshot merge for origin %s, "
						     "origin volume is opened.",
						     display_lvname(lv));
				laopts->no_merging = 1;
			}
		}

		/* If merge would be still undecided, look as snapshot */
		if (!merge_in_progress && !laopts->no_merging &&
		    (dinfo = _cached_dm_info(dm->mem, dtree,
					     seg_is_thin_volume(seg) ?
					     seg->lv : seg->cow, NULL))) {
			if (seg_is_thin_volume(seg)) {
				/* Active thin snapshot prevents merge */
				log_debug_activation("Postponing pending snapshot merge for origin volume %s, "
						     "merging thin snapshot volume %s is active.",
						     display_lvname(lv), display_lvname(seg->lv));
				laopts->no_merging = 1;
			} else if (dinfo->open_count) {
				log_debug_activation("Postponing pending snapshot merge for origin volume %s, "
						     "merging snapshot volume %s is opened.",
						     display_lvname(lv), display_lvname(seg->lv));
				laopts->no_merging = 1;
			}
		}
	}

	if (!(name = dm_build_dm_name(dm->mem, lv->vg->name, lv->name, layer)))
		return_0;

	/* Even unused thin-pool still needs to get layered  UUID -suffix */
	if (!layer && lv_is_new_thin_pool(lv))
		layer = lv_layer(lv);

	if (!(dlid = build_dm_uuid(dm->mem, lv, layer)))
		return_0;

	/* We've already processed this node if it already has a context ptr */
	if ((dnode = dm_tree_find_node_by_uuid(dtree, dlid)) &&
	    dm_tree_node_get_context(dnode))
		return 1;

	if (!(lvlayer = dm_pool_alloc(dm->mem, sizeof(*lvlayer)))) {
		log_error("_add_new_lv_to_dtree: pool alloc failed for %s %s.",
			  display_lvname(lv), layer);
		return 0;
	}

	lvlayer->lv = lv;
	lvlayer->visible_component = (laopts->component_lv == lv) ? 1 : 0;

	/*
	 * Add LV to dtree.
	 * If we're working with precommitted metadata, clear any
	 * existing inactive table left behind.
	 * Major/minor settings only apply to the visible layer.
	 */
	/* FIXME Move the clear from here until later, so we can leave
	 * identical inactive tables untouched. (For pvmove.)
	 */
	if (!(dnode = dm_tree_add_new_dev_with_udev_flags(dtree, name, dlid,
					     layer ? UINT32_C(0) : (uint32_t) lv->major,
					     layer ? UINT32_C(0) : (uint32_t) lv->minor,
					     read_only_lv(lv, laopts, layer),
					     ((lv->vg->status & PRECOMMITTED) | laopts->revert) ? 1 : 0,
					     lvlayer,
					     _get_udev_flags(dm, lv, layer, laopts->noscan, laopts->temporary,
							     lvlayer->visible_component))))
		return_0;

	/* Store existing name so we can do rename later */
	lvlayer->old_name = dm_tree_node_get_name(dnode);

	/* Create table */
	dm->pvmove_mirror_count = 0u;

	if (lv_is_pending_delete(lv))
		/* Handle LVs with pending delete */
		/* Fow now used only by cache segtype, TODO snapshots */
		dm->track_pending_delete = 1;

	/* This is unused cache-pool - make metadata accessible */
	if (lv_is_cache_pool(lv))
		lv = first_seg(lv)->metadata_lv;

	/* If this is a snapshot origin, add real LV */
	/* If this is a snapshot origin + merging snapshot, add cow + real LV */
	/* Snapshot origin could be also external origin */
	if (lv_is_origin(lv) && !layer) {
		if (!_add_new_lv_to_dtree(dm, dtree, lv, laopts, "real"))
			return_0;
		if (!laopts->no_merging && lv_is_merging_origin(lv)) {
			if (!_add_new_lv_to_dtree(dm, dtree,
						  find_snapshot(lv)->cow, laopts, "cow"))
				return_0;
			/*
			 * Must also add "real" LV for use when
			 * snapshot-merge target is added
			 */
			if (!_add_snapshot_merge_target_to_dtree(dm, dnode, lv))
				return_0;
		} else if (!_add_origin_target_to_dtree(dm, dnode, lv))
			return_0;

		/* Add any snapshots of this LV */
		dm_list_iterate(snh, &lv->snapshot_segs)
			if (!_add_new_lv_to_dtree(dm, dtree,
						  dm_list_struct_base(snh, struct lv_segment,
								      origin_list)->cow,
						  laopts, NULL))
				return_0;
	} else if (lv_is_cow(lv) && !layer) {
		if (!_add_new_lv_to_dtree(dm, dtree, lv, laopts, "cow"))
			return_0;
		if (!_add_snapshot_target_to_dtree(dm, dnode, lv, laopts))
			return_0;
	} else if (!layer && ((lv_is_thin_pool(lv) && !lv_is_new_thin_pool(lv)) ||
			      lv_is_external_origin(lv))) {
		/* External origin or 'used' Thin pool is using layer */
		if (!_add_new_lv_to_dtree(dm, dtree, lv, laopts, lv_layer(lv)))
			return_0;
		if (!_add_layer_target_to_dtree(dm, dnode, lv))
			return_0;
	} else {
		/* Add 'real' segments for LVs */
		dm_list_iterate_items(seg, &lv->segments) {
			if (!_add_segment_to_dtree(dm, dtree, dnode, seg, laopts, layer))
				return_0;
			if (max_stripe_size < seg->stripe_size * seg->area_count)
				max_stripe_size = seg->stripe_size * seg->area_count;
		}
	}

	/* Setup thin pool callback */
	if (lv_is_thin_pool(lv) && layer &&
	    !_pool_register_callback(dm, dnode, lv))
		return_0;

	if (lv_is_cache(lv) &&
	    /* Register callback only for layer activation or non-layered cache LV */
	    (layer || !lv_layer(lv)) &&
	    /* Register callback when metadata LV is NOT already active */
	    !_cached_dm_info(dm->mem, dtree, first_seg(first_seg(lv)->pool_lv)->metadata_lv, NULL) &&
	    !_pool_register_callback(dm, dnode, lv))
		return_0;

	if (read_ahead == DM_READ_AHEAD_AUTO) {
		/* we need RA at least twice a whole stripe - see the comment in md/raid0.c */
		read_ahead = max_stripe_size * 2;
		/* FIXME: layered device read-ahead */
		if (!read_ahead)
			lv_calculate_readahead(lv, &read_ahead);
		read_ahead_flags = DM_READ_AHEAD_MINIMUM_FLAG;
	}

	dm_tree_node_set_read_ahead(dnode, read_ahead, read_ahead_flags);

	/* Add any LVs referencing a PVMOVE LV unless told not to */
	if (dm->track_pvmove_deps && lv_is_pvmove(lv))
		dm_list_iterate_items(sl, &lv->segs_using_this_lv)
			if (!_add_new_lv_to_dtree(dm, dtree, sl->seg->lv, laopts, NULL))
				return_0;

	dm->track_pending_delete = save_pending_delete; /* restore */

	return 1;
}

/* FIXME: symlinks should be created/destroyed at the same time
 * as the kernel devices but we can't do that from within libdevmapper
 * at present so we must walk the tree twice instead. */

/*
 * Create LV symlinks for children of supplied root node.
 */
static int _create_lv_symlinks(struct dev_manager *dm, struct dm_tree_node *root)
{
	void *handle = NULL;
	struct dm_tree_node *child;
	struct lv_layer *lvlayer;
	char *old_vgname, *old_lvname, *old_layer;
	char *new_vgname, *new_lvname, *new_layer;
	const char *name;
	int r = 1;

	/* Nothing to do if udev fallback is disabled. */
	if (!_check_udev_fallback(dm->cmd)) {
		fs_set_create();
		return 1;
	}

	while ((child = dm_tree_next_child(&handle, root, 0))) {
		if (!(lvlayer = dm_tree_node_get_context(child)))
			continue;

		/* Detect rename */
		name = dm_tree_node_get_name(child);

		if (name && lvlayer->old_name && *lvlayer->old_name && strcmp(name, lvlayer->old_name)) {
			if (!dm_split_lvm_name(dm->mem, lvlayer->old_name, &old_vgname, &old_lvname, &old_layer)) {
				log_error("_create_lv_symlinks: Couldn't split up old device name %s.", lvlayer->old_name);
				return 0;
			}
			if (!dm_split_lvm_name(dm->mem, name, &new_vgname, &new_lvname, &new_layer)) {
				log_error("_create_lv_symlinks: Couldn't split up new device name %s.", name);
				return 0;
			}
			if (!fs_rename_lv(lvlayer->lv, name, old_vgname, old_lvname))
				r = 0;
			continue;
		}
		if (_lv_has_mknode(lvlayer->lv) || lvlayer->visible_component) {
			if (!_dev_manager_lv_mknodes(lvlayer->lv))
				r = 0;
			continue;
		}
		if (!_dev_manager_lv_rmnodes(lvlayer->lv))
			r = 0;
	}

	return r;
}

/*
 * Remove LV symlinks for children of supplied root node.
 */
static int _remove_lv_symlinks(struct dev_manager *dm, struct dm_tree_node *root)
{
	void *handle = NULL;
	struct dm_tree_node *child;
	char *vgname, *lvname, *layer;
	int r = 1;

	/* Nothing to do if udev fallback is disabled. */
	if (!_check_udev_fallback(dm->cmd))
		return 1;

	while ((child = dm_tree_next_child(&handle, root, 0))) {
		if (!dm_split_lvm_name(dm->mem, dm_tree_node_get_name(child), &vgname, &lvname, &layer)) {
			r = 0;
			continue;
		}

		if (!*vgname)
			continue;

		/* only top level layer has symlinks */
		if (*layer)
			continue;

		fs_del_lv_byname(dm->cmd->dev_dir, vgname, lvname,
				 dm->cmd->current_settings.udev_rules);
	}

	return r;
}

static int _clean_tree(struct dev_manager *dm, struct dm_tree_node *root, const char *non_toplevel_tree_dlid)
{
	void *handle = NULL;
	struct dm_tree_node *child;
	char *vgname, *lvname, *layer;
	const char *name, *uuid;
	struct dm_str_list *dl;

	/* Deactivate any tracked pending delete nodes */
	dm_list_iterate_items(dl, &dm->pending_delete) {
		log_debug_activation("Deleting tracked UUID %s.", dl->str);
		if (!dm_tree_deactivate_children(root, dl->str, strlen(dl->str)))
			return_0;
	}

	while ((child = dm_tree_next_child(&handle, root, 0))) {
		if (!(name = dm_tree_node_get_name(child)))
			continue;

		if (!(uuid = dm_tree_node_get_uuid(child)))
			continue;

		if (!dm_split_lvm_name(dm->mem, name, &vgname, &lvname, &layer)) {
			log_error("_clean_tree: Couldn't split up device name %s.", name);
			return 0;
		}

		/* Not meant to be top level? */
		if (!*layer)
			continue;

		/* If operation was performed on a partial tree, don't remove it */
		if (non_toplevel_tree_dlid && !strcmp(non_toplevel_tree_dlid, uuid))
			continue;

		if (!dm_tree_deactivate_children(root, uuid, strlen(uuid)))
			return_0;
	}

	return 1;
}

static int _tree_action(struct dev_manager *dm, const struct logical_volume *lv,
			struct lv_activate_opts *laopts, action_t action)
{
	static const char _action_names[][24] = {
		"PRELOAD", "ACTIVATE", "DEACTIVATE", "SUSPEND", "SUSPEND_WITH_LOCKFS", "CLEAN"
	};
	const size_t DLID_SIZE = ID_LEN + sizeof(UUID_PREFIX) - 1;
	struct dm_tree *dtree;
	struct dm_tree_node *root;
	char *dlid;
	int r = 0;

	if (action < DM_ARRAY_SIZE(_action_names))
		log_debug_activation("Creating %s%s tree for %s.",
				     _action_names[action],
				     (laopts->origin_only) ? " origin-only" : "",
				     display_lvname(lv));

	/* Some LV cannot be used for top level tree */
	/* TODO: add more.... */
	if (lv_is_cache_pool(lv) && !dm_list_empty(&lv->segs_using_this_lv)) {
		log_error(INTERNAL_ERROR "Cannot create tree for %s.",
			  display_lvname(lv));
		return 0;
	}
	/* Some targets may build bigger tree for activation */
	dm->activation = ((action == PRELOAD) || (action == ACTIVATE));
	dm->suspend = (action == SUSPEND_WITH_LOCKFS) || (action == SUSPEND);
	dm->track_external_lv_deps = 1;

	if (!(dtree = _create_partial_dtree(dm, lv, laopts->origin_only)))
		return_0;

	if (!(root = dm_tree_find_node(dtree, 0, 0))) {
		log_error("Lost dependency tree root node.");
		goto out_no_root;
	}

	/* Restore fs cookie */
	dm_tree_set_cookie(root, fs_get_cookie());

	if (!(dlid = build_dm_uuid(dm->mem, lv, laopts->origin_only ? lv_layer(lv) : NULL)))
		goto_out;

	/* Only process nodes with uuid of "LVM-" plus VG id. */
	switch(action) {
	case CLEAN:
		if (retry_deactivation())
			dm_tree_retry_remove(root);
		/* Deactivate any unused non-toplevel nodes */
		if (!_clean_tree(dm, root, laopts->origin_only ? dlid : NULL))
			goto_out;
		break;
	case DEACTIVATE:
		if (retry_deactivation())
			dm_tree_retry_remove(root);
		/* Deactivate LV and all devices it references that nothing else has open. */
		if (!dm_tree_deactivate_children(root, dlid, DLID_SIZE))
			goto_out;
		if (!_remove_lv_symlinks(dm, root))
			log_warn("Failed to remove all device symlinks associated with %s.",
				 display_lvname(lv));
		break;
	case SUSPEND:
		dm_tree_skip_lockfs(root);
		if (!dm->flush_required)
			dm_tree_use_no_flush_suspend(root);
		/* Fall through */
	case SUSPEND_WITH_LOCKFS:
		if (!dm_tree_suspend_children(root, dlid, DLID_SIZE))
			goto_out;
		break;
	case PRELOAD:
	case ACTIVATE:
		/* Add all required new devices to tree */
		if (!_add_new_lv_to_dtree(dm, dtree, lv, laopts,
					  (lv_is_origin(lv) && laopts->origin_only) ? "real" :
					  (lv_is_thin_pool(lv) && laopts->origin_only) ? "tpool" : NULL))
			goto_out;

		/* Preload any devices required before any suspensions */
		if (!dm_tree_preload_children(root, dlid, DLID_SIZE))
			goto_out;

		if ((dm_tree_node_size_changed(root) < 0))
			dm->flush_required = 1;
		/* Currently keep the code require flush for any
		 * non 'thin pool/volume' and  size increase */
		else if (!lv_is_thin_volume(lv) &&
			 !lv_is_thin_pool(lv) &&
			 dm_tree_node_size_changed(root))
			dm->flush_required = 1;

		if (action == ACTIVATE) {
			if (!dm_tree_activate_children(root, dlid, DLID_SIZE))
				goto_out;
			if (!_create_lv_symlinks(dm, root))
				log_warn("Failed to create symlinks for %s.",
					 display_lvname(lv));
		}

		break;
	default:
		log_error(INTERNAL_ERROR "_tree_action: Action %u not supported.", action);
		goto out;
	}
	r = 1;

out:
	/* Save fs cookie for udev settle, do not wait here */
	fs_set_cookie(dm_tree_get_cookie(root));
out_no_root:
	dm_tree_free(dtree);

	return r;
}

/* origin_only may only be set if we are resuming (not activating) an origin LV */
int dev_manager_activate(struct dev_manager *dm, const struct logical_volume *lv,
			 struct lv_activate_opts *laopts)
{
	if (!_tree_action(dm, lv, laopts, ACTIVATE))
		return_0;

	if (!_tree_action(dm, lv, laopts, CLEAN))
		return_0;

	return 1;
}

/* origin_only may only be set if we are resuming (not activating) an origin LV */
int dev_manager_preload(struct dev_manager *dm, const struct logical_volume *lv,
			struct lv_activate_opts *laopts, int *flush_required)
{
	dm->flush_required = *flush_required;

	if (!_tree_action(dm, lv, laopts, PRELOAD))
		return_0;

	*flush_required = dm->flush_required;

	return 1;
}

int dev_manager_deactivate(struct dev_manager *dm, const struct logical_volume *lv)
{
	struct lv_activate_opts laopts = { 0 };

	if (!_tree_action(dm, lv, &laopts, DEACTIVATE))
		return_0;

	return 1;
}

int dev_manager_suspend(struct dev_manager *dm, const struct logical_volume *lv,
			struct lv_activate_opts *laopts, int lockfs, int flush_required)
{
	dm->flush_required = flush_required;

	if (!_tree_action(dm, lv, laopts, lockfs ? SUSPEND_WITH_LOCKFS : SUSPEND))
		return_0;

	return 1;
}

/*
 * Does device use VG somewhere in its construction?
 * Returns 1 if uncertain.
 */
int dev_manager_device_uses_vg(struct device *dev,
			       struct volume_group *vg)
{
	struct dm_tree *dtree;
	struct dm_tree_node *root;
	char dlid[sizeof(UUID_PREFIX) + sizeof(struct id) - 1] __attribute__((aligned(8)));
	int r = 1;

	if (!(dtree = dm_tree_create())) {
		log_error("Failed to create partial dtree.");
		return r;
	}

	dm_tree_set_optional_uuid_suffixes(dtree, &uuid_suffix_list[0]);

	if (!dm_tree_add_dev(dtree, (uint32_t) MAJOR(dev->dev), (uint32_t) MINOR(dev->dev))) {
		log_error("Failed to add device %s (%" PRIu32 ":%" PRIu32") to dtree.",
			  dev_name(dev), (uint32_t) MAJOR(dev->dev), (uint32_t) MINOR(dev->dev));
		goto out;
	}

	memcpy(dlid, UUID_PREFIX, sizeof(UUID_PREFIX) - 1);
	memcpy(dlid + sizeof(UUID_PREFIX) - 1, &vg->id.uuid[0], sizeof(vg->id));

	if (!(root = dm_tree_find_node(dtree, 0, 0))) {
		log_error("Lost dependency tree root node.");
		goto out;
	}

	if (dm_tree_children_use_uuid(root, dlid, sizeof(UUID_PREFIX) + sizeof(vg->id) - 1))
		goto_out;

	r = 0;

out:
	dm_tree_free(dtree);

	return r;
}
