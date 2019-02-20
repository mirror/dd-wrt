/*
 * Copyright (C) 2005-2017 Red Hat, Inc. All rights reserved.
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
#include "daemons/dmeventd/plugins/lvm2/dmeventd_lvm.h"
#include "daemons/dmeventd/libdevmapper-event.h"
#include "lib/activate/activate.h"

/* FIXME Reformat to 80 char lines. */

#define ME_IGNORE    0
#define ME_INSYNC    1
#define ME_FAILURE   2

struct dso_state {
	struct dm_pool *mem;
	char cmd_lvconvert[512];
};

DM_EVENT_LOG_FN("mirr")

static void _process_status_code(dm_status_mirror_health_t health,
				 uint32_t major, uint32_t minor,
				 const char *dev_type, int *r)
{
	/*
	 *    A => Alive - No failures
	 *    D => Dead - A write failure occurred leaving mirror out-of-sync
	 *    F => Flush failed.
	 *    S => Sync - A sychronization failure occurred, mirror out-of-sync
	 *    R => Read - A read failure occurred, mirror data unaffected
	 *    U => Unclassified failure (bug)
	 */ 
	switch (health) {
	case DM_STATUS_MIRROR_ALIVE:
		return;
	case DM_STATUS_MIRROR_FLUSH_FAILED:
		log_error("%s device %u:%u flush failed.",
			  dev_type, major, minor);
		*r = ME_FAILURE;
		break;
	case DM_STATUS_MIRROR_SYNC_FAILED:
		log_error("%s device %u:%u sync failed.",
			  dev_type, major, minor);
		break;
	case DM_STATUS_MIRROR_READ_FAILED:
		log_error("%s device %u:%u read failed.",
			  dev_type, major, minor);
		break;
	default:
		log_error("%s device %u:%u has failed (%c).",
			  dev_type, major, minor, (char)health);
		*r = ME_FAILURE;
		break;
	}
}

static int _get_mirror_event(struct dso_state *state, char *params)
{
	int r = ME_INSYNC;
	unsigned i;
	struct dm_status_mirror *ms;

	if (!dm_get_status_mirror(state->mem, params, &ms)) {
		log_error("Unable to parse mirror status string.");
		return ME_IGNORE;
	}

	/* Check for bad mirror devices */
	for (i = 0; i < ms->dev_count; ++i)
		_process_status_code(ms->devs[i].health,
				     ms->devs[i].major, ms->devs[i].minor,
				     i ? "Secondary mirror" : "Primary mirror", &r);

	/* Check for bad disk log device */
	for (i = 0; i < ms->log_count; ++i)
		_process_status_code(ms->logs[i].health,
				     ms->logs[i].major, ms->logs[i].minor,
				     "Log", &r);

	/* Ignore if not in-sync */
	if ((r == ME_INSYNC) && (ms->insync_regions != ms->total_regions))
		r = ME_IGNORE;

	dm_pool_free(state->mem, ms);

	return r;
}

static int _remove_failed_devices(const char *cmd_lvconvert, const char *device)
{
	/* if repair goes OK, report success even if lvscan has failed */
	if (!dmeventd_lvm2_run_with_lock(cmd_lvconvert)) {
		log_error("Repair of mirrored device %s failed.", device);
		return 0;
	}

	log_info("Repair of mirrored device %s finished successfully.", device);

	return 1;
}

void process_event(struct dm_task *dmt,
		   enum dm_event_mask event __attribute__((unused)),
		   void **user)
{
	struct dso_state *state = *user;
	void *next = NULL;
	uint64_t start, length;
	char *target_type = NULL;
	char *params;
	const char *device = dm_task_get_name(dmt);

	do {
		next = dm_get_next_target(dmt, next, &start, &length,
					  &target_type, &params);

		if (!target_type) {
			log_info("%s mapping lost.", device);
			continue;
		}

		if (strcmp(target_type, TARGET_NAME_MIRROR)) {
			log_info("%s has unmirrored portion.", device);
			continue;
		}

		switch(_get_mirror_event(state, params)) {
		case ME_INSYNC:
			/* FIXME: all we really know is that this
			   _part_ of the device is in sync
			   Also, this is not an error
			*/
			log_notice("%s is now in-sync.", device);
			break;
		case ME_FAILURE:
			log_error("Device failure in %s.", device);
			if (!_remove_failed_devices(state->cmd_lvconvert, device))
				/* FIXME Why are all the error return codes unused? Get rid of them? */
				log_error("Failed to remove faulty devices in %s.",
					  device);
			/* Should check before warning user that device is now linear
			else
				log_notice("%s is now a linear device.",
					   device);
			*/
			break;
		case ME_IGNORE:
			break;
		default:
			/* FIXME Provide value then! */
			log_warn("WARNING: %s received unknown event.", device);
		}
	} while (next);
}

int register_device(const char *device,
		    const char *uuid __attribute__((unused)),
		    int major __attribute__((unused)),
		    int minor __attribute__((unused)),
		    void **user)
{
	struct dso_state *state;

	if (!dmeventd_lvm2_init_with_pool("mirror_state", state))
		goto_bad;

        /* CANNOT use --config as this disables cached content */
	if (!dmeventd_lvm2_command(state->mem, state->cmd_lvconvert, sizeof(state->cmd_lvconvert),
				   "lvconvert --repair --use-policies", device))
		goto_bad;

	*user = state;

	log_info("Monitoring mirror device %s for events.", device);

	return 1;
bad:
	log_error("Failed to monitor mirror %s.", device);

	if (state)
		dmeventd_lvm2_exit_with_pool(state);

	return 0;
}

int unregister_device(const char *device,
		      const char *uuid __attribute__((unused)),
		      int major __attribute__((unused)),
		      int minor __attribute__((unused)),
		      void **user)
{
	struct dso_state *state = *user;

	dmeventd_lvm2_exit_with_pool(state);
	log_info("No longer monitoring mirror device %s for events.",
		 device);

	return 1;
}
