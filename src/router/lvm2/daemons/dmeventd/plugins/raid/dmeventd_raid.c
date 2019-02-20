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
#include "lib/config/defaults.h"

/* Hold enough elements for the mximum number of RAID images */
#define	RAID_DEVS_ELEMS	((DEFAULT_RAID_MAX_IMAGES + 63) / 64)

struct dso_state {
	struct dm_pool *mem;
	char cmd_lvconvert[512];
	uint64_t raid_devs[RAID_DEVS_ELEMS];
	int failed;
	int warned;
};

DM_EVENT_LOG_FN("raid")

/* FIXME Reformat to 80 char lines. */

static int _process_raid_event(struct dso_state *state, char *params, const char *device)
{
	struct dm_status_raid *status;
	const char *d;
	int dead = 0, r = 1;
	uint32_t dev;

	if (!dm_get_status_raid(state->mem, params, &status)) {
		log_error("Failed to process status line for %s.", device);
		return 0;
	}

	d = status->dev_health;
	while ((d = strchr(d, 'D'))) {
		dev = (uint32_t)(d - status->dev_health);

		if (!(state->raid_devs[dev / 64] & (UINT64_C(1) << (dev % 64)))) {
			state->raid_devs[dev / 64] |= (UINT64_C(1) << (dev % 64));
			log_warn("WARNING: Device #%u of %s array, %s, has failed.",
				 dev, status->raid_type, device);
		}

		d++;
		dead = 1;
	}

	/*
	 * if we are converting from non-RAID to RAID (e.g. linear -> raid1)
	 * and too many original devices die, such that we cannot continue
	 * the "recover" operation, the sync action will go to "idle", the
	 * unsynced devs will remain at 'a', and the original devices will
	 * NOT SWITCH TO 'D', but will remain at 'A' - hoping to be revived.
	 *
	 * This is simply the way the kernel works...
	 */
	if (!strcmp(status->sync_action, "idle") &&
	    (status->dev_health[0] == 'a') &&
	    (status->insync_regions < status->total_regions)) {
		log_error("Primary sources for new RAID, %s, have failed.",
			  device);
		dead = 1; /* run it through LVM repair */
	}

	if (dead) {
		if (status->insync_regions < status->total_regions) {
			if (!state->warned) {
				state->warned = 1;
				log_warn("WARNING: waiting for resynchronization to finish "
					 "before initiating repair on RAID device %s.", device);
			}

			goto out; /* Not yet done syncing with accessible devices */
		}

		if (state->failed)
			goto out; /* already reported */

		state->failed = 1;

		/* if repair goes OK, report success even if lvscan has failed */
		if (!dmeventd_lvm2_run_with_lock(state->cmd_lvconvert)) {
			log_error("Repair of RAID device %s failed.", device);
			r = 0;
		}
	} else {
		state->failed = 0;
		if (status->insync_regions == status->total_regions)
			memset(&state->raid_devs, 0, sizeof(state->raid_devs));
		log_info("%s array, %s, is %s in-sync.",
			 status->raid_type, device,
			 (status->insync_regions == status->total_regions) ? "now" : "not");
	}
out:
	dm_pool_free(state->mem, status);

	return r;
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

		if (strcmp(target_type, "raid")) {
			log_info("%s has non-raid portion.", device);
			continue;
		}

		if (!_process_raid_event(state, params, device))
			log_error("Failed to process event for %s.",
				  device);
	} while (next);
}

int register_device(const char *device,
		    const char *uuid __attribute__((unused)),
		    int major __attribute__((unused)),
		    int minor __attribute__((unused)),
		    void **user)
{
	struct dso_state *state;

	if (!dmeventd_lvm2_init_with_pool("raid_state", state))
		goto_bad;

	if (!dmeventd_lvm2_command(state->mem, state->cmd_lvconvert, sizeof(state->cmd_lvconvert),
				   "lvconvert --repair --use-policies", device))
		goto_bad;

	*user = state;

	log_info("Monitoring RAID device %s for events.", device);

	return 1;
bad:
	log_error("Failed to monitor RAID %s.", device);

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
	log_info("No longer monitoring RAID device %s for events.",
		 device);

	return 1;
}
