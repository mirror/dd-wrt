/*
 * mdmon - monitor external metadata arrays
 *
 * Copyright (C) 2007-2009 Neil Brown <neilb@suse.de>
 * Copyright (C) 2007-2009 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

extern const char Name[];

enum array_state { clear, inactive, suspended, readonly, read_auto,
		   clean, active, write_pending, active_idle, bad_word};

enum sync_action { idle, reshape, resync, recover, check, repair, bad_action };

struct active_array {
	struct mdinfo info;
	struct supertype *container;
	struct active_array *next, *replaces;
	int to_remove;

	int action_fd;
	int resync_start_fd;
	int metadata_fd; /* for monitoring rw/ro status */
	int sync_completed_fd; /* for checkpoint notification events */
	int safe_mode_delay_fd;
	unsigned long long last_checkpoint; /* sync_completed fires for many
					     * reasons this field makes sure the
					     * kernel has made progress before
					     * moving the checkpoint.  It is
					     * cleared by the metadata handler
					     * when it determines recovery is
					     * terminated.
					     */

	enum array_state prev_state, curr_state, next_state;
	enum sync_action prev_action, curr_action, next_action;

	int check_degraded; /* flag set by mon, read by manage */
	int check_reshape; /* flag set by mon, read by manage */
};

/*
 * Metadata updates are handled by the monitor thread,
 * as it has exclusive access to the metadata.
 * When the manager want to updates metadata, either
 * for it's own reason (e.g. committing a spare) or
 * on behalf of mdadm, it creates a metadata_update
 * structure and queues it to the monitor.
 * Updates are created and processed by code under the
 * superswitch.  All common code sees them as opaque
 * blobs.
 */
extern struct metadata_update *update_queue, *update_queue_handled;

#define MD_MAJOR 9

extern struct active_array *container;
extern struct active_array *discard_this;
extern struct active_array *pending_discard;
extern struct md_generic_cmd *active_cmd;

void remove_pidfile(char *devname);
void do_monitor(struct supertype *container);
void do_manager(struct supertype *container);
extern int sigterm;

int read_dev_state(int fd);
int is_container_member(struct mdstat_ent *mdstat, char *container);

struct mdstat_ent *mdstat_read(int hold, int start);

extern int exit_now, manager_ready;
extern int mon_tid, mgr_tid;
extern int monitor_loop_cnt;

/* helper routine to determine resync completion since MaxSector is a
 * moving target
 */
static inline int is_resync_complete(struct mdinfo *array)
{
	unsigned long long sync_size = 0;
	int ncopies, l;
	switch(array->array.level) {
	case 1:
	case 4:
	case 5:
	case 6:
		sync_size = array->component_size;
		break;
	case 10:
		l = array->array.layout;
		ncopies = (l & 0xff) * ((l >> 8) & 0xff);
		sync_size = array->component_size * array->array.raid_disks;
		sync_size /= ncopies;
		break;
	}
	return array->resync_start >= sync_size;
}
