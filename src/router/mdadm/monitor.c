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

#include "mdadm.h"
#include "mdmon.h"
#include <sys/syscall.h>
#include <sys/select.h>
#include <signal.h>

static char *array_states[] = {
	"clear", "inactive", "suspended", "readonly", "read-auto",
	"clean", "active", "write-pending", "active-idle", NULL };
static char *sync_actions[] = {
	"idle", "reshape", "resync", "recover", "check", "repair", NULL
};

enum bb_action {
	RECORD_BB = 1,
	COMPARE_BB,
};

static int write_attr(char *attr, int fd)
{
	return write(fd, attr, strlen(attr));
}

static void add_fd(fd_set *fds, int *maxfd, int fd)
{
	struct stat st;
	if (fd < 0)
		return;
	if (fstat(fd, &st) == -1) {
		dprintf("Invalid fd %d\n", fd);
		return;
	}
	if (st.st_nlink == 0) {
		dprintf("fd %d was deleted\n", fd);
		return;
	}
	if (fd > *maxfd)
		*maxfd = fd;
	FD_SET(fd, fds);
}

static int read_attr(char *buf, int len, int fd)
{
	int n;

	if (fd < 0) {
		buf[0] = 0;
		return 0;
	}
	lseek(fd, 0, 0);
	n = read(fd, buf, len - 1);

	if (n <= 0) {
		buf[0] = 0;
		return 0;
	}
	buf[n] = 0;
	if (buf[n-1] == '\n')
		buf[n-1] = 0;
	return n;
}

static void read_resync_start(int fd, unsigned long long *v)
{
	char buf[30];
	int n;

	n = read_attr(buf, 30, fd);
	if (n <= 0) {
		dprintf("Failed to read resync_start (%d)\n", fd);
		return;
	}
	if (strncmp(buf, "none", 4) == 0)
		*v = MaxSector;
	else
		*v = strtoull(buf, NULL, 10);
}

static unsigned long long read_sync_completed(int fd)
{
	unsigned long long val;
	char buf[50];
	int n;
	char *ep;

	n = read_attr(buf, 50, fd);

	if (n <= 0)
		return 0;
	buf[n] = 0;
	val = strtoull(buf, &ep, 0);
	if (ep == buf || (*ep != 0 && *ep != '\n' && *ep != ' '))
		return 0;
	return val;
}

static enum array_state read_state(int fd)
{
	char buf[20];
	int n = read_attr(buf, 20, fd);

	if (n <= 0)
		return bad_word;
	return (enum array_state) sysfs_match_word(buf, array_states);
}

static enum sync_action read_action( int fd)
{
	char buf[20];
	int n = read_attr(buf, 20, fd);

	if (n <= 0)
		return bad_action;
	return (enum sync_action) sysfs_match_word(buf, sync_actions);
}

int read_dev_state(int fd)
{
	char buf[100];
	int n = read_attr(buf, sizeof(buf), fd);
	char *cp;
	int rv = 0;

	if (n <= 0)
		return 0;

	cp = buf;
	while (cp) {
		if (sysfs_attr_match(cp, "faulty"))
			rv |= DS_FAULTY;
		if (sysfs_attr_match(cp, "in_sync"))
			rv |= DS_INSYNC;
		if (sysfs_attr_match(cp, "write_mostly"))
			rv |= DS_WRITE_MOSTLY;
		if (sysfs_attr_match(cp, "spare"))
			rv |= DS_SPARE;
		if (sysfs_attr_match(cp, "blocked"))
			rv |= DS_BLOCKED;
		cp = strchr(cp, ',');
		if (cp)
			cp++;
	}
	return rv;
}

int process_ubb(struct active_array *a, struct mdinfo *mdi, const unsigned long
		long sector, const int length, const char *buf,
		const int buf_len)
{
	struct superswitch *ss = a->container->ss;

	/*
	 * record bad block in metadata first, then acknowledge it to the driver
	 * via sysfs file
	 */
	if ((ss->record_bad_block(a, mdi->disk.raid_disk, sector, length)) &&
	    (write(mdi->bb_fd, buf, buf_len) == buf_len))
		return 1;

	/*
	 * failed to store or acknowledge bad block, switch of bad block support
	 * to get it out of blocked state
	 */
	sysfs_set_str(&a->info, mdi, "state", "-external_bbl");
	return -1;
}

int compare_bb(struct active_array *a, struct mdinfo *mdi, const unsigned long
	       long sector, const unsigned int length, void *arg)
{
	struct superswitch *ss = a->container->ss;
	struct md_bb *bb = (struct md_bb *) arg;
	int record = 1;
	int i;

	for (i = 0; i < bb->count; i++) {
		unsigned long long start = bb->entries[i].sector;
		unsigned long long len = bb->entries[i].length;

		/*
		 * bad block in metadata exactly matches bad block in kernel
		 * list, just remove it from a list
		 */
		if ((start == sector) && (len == length)) {
			if (i < bb->count - 1)
				bb->entries[i] = bb->entries[bb->count - 1];
			bb->count -= 1;
			record = 0;
			break;
		}
		/*
		 * bad block in metadata spans bad block in kernel list,
		 * clear it and record new bad block
		 */
		if ((sector >= start) && (sector + length <= start + len)) {
			ss->clear_bad_block(a, mdi->disk.raid_disk, start, len);
			break;
		}
	}

	/* record all bad blocks not in metadata list */
	if (record && (ss->record_bad_block(a, mdi->disk.raid_disk, sector,
					     length) <= 0)) {
		sysfs_set_str(&a->info, mdi, "state", "-external_bbl");
		return -1;
	}

	return 1;
}

static int read_bb_file(int fd, struct active_array *a, struct mdinfo *mdi,
			enum bb_action action, void *arg)
{
	char buf[30];
	int n = 0;
	int ret = 0;
	int read_again = 0;
	int off = 0;
	int pos = 0;
	int preserve_pos = (action == RECORD_BB ? 0 : 1);

	if (lseek(fd, 0, SEEK_SET) == (off_t) -1)
		return -1;

	do {
		read_again = 0;
		n = read(fd, buf + pos, sizeof(buf) - 1 - pos);
		if (n < 0)
			return -1;
		n += pos;

		buf[n] = '\0';
		off = 0;

		while (off < n) {
			unsigned long long sector;
			int length;
			char newline;
			int consumed;
			int matched;
			int rc;

			/* kernel sysfs file format: "sector length\n" */
			matched = sscanf(buf + off, "%llu %d%c%n", &sector,
					 &length, &newline, &consumed);
			if ((matched != 3) && (off > 0)) {
				/* truncated entry, read again */
				if (preserve_pos) {
					pos = sizeof(buf) - off - 1;
					memmove(buf, buf + off, pos);
				} else {
					if (lseek(fd, 0, SEEK_SET) ==
					    (off_t) -1)
						return -1;
				}
				read_again = 1;
				break;
			}
			if (matched != 3)
				return -1;
			if (newline != '\n')
				return -1;
			if (length <= 0)
				return -1;

			if (action == RECORD_BB)
				rc = process_ubb(a, mdi, sector, length,
						  buf + off, consumed);
			else if (action == COMPARE_BB)
				rc = compare_bb(a, mdi, sector, length, arg);
			else
				rc = -1;

			if (rc < 0)
				return rc;
			ret += rc;
			off += consumed;
		}
	} while (read_again);

	return ret;
}

static int process_dev_ubb(struct active_array *a, struct mdinfo *mdi)
{
	return read_bb_file(mdi->ubb_fd, a, mdi, RECORD_BB, NULL);
}

static int check_for_cleared_bb(struct active_array *a, struct mdinfo *mdi)
{
	struct superswitch *ss = a->container->ss;
	struct md_bb *bb;
	int i;

	/*
	 * Get a list of bad blocks for an array, then read list of
	 * acknowledged bad blocks from kernel and compare it against metadata
	 * list, clear all bad blocks remaining in metadata list
	 */
	bb = ss->get_bad_blocks(a, mdi->disk.raid_disk);
	if (!bb)
		return -1;

	if (read_bb_file(mdi->bb_fd, a, mdi, COMPARE_BB, bb) < 0)
		return -1;

	for (i = 0; i < bb->count; i++) {
		unsigned long long sector = bb->entries[i].sector;
		int length = bb->entries[i].length;

		ss->clear_bad_block(a, mdi->disk.raid_disk, sector, length);
	}

	return 0;
}

static void signal_manager(void)
{
	/* tgkill(getpid(), mon_tid, SIGUSR1); */
	int pid = getpid();
	syscall(SYS_tgkill, pid, mgr_tid, SIGUSR1);
}

/* Monitor a set of active md arrays - all of which share the
 * same metadata - and respond to events that require
 * metadata update.
 *
 * New arrays are detected by another thread which allocates
 * required memory and attaches the data structure to our list.
 *
 * Events:
 *  Array stops.
 *    This is detected by array_state going to 'clear' or 'inactive'.
 *    while we thought it was active.
 *    Response is to mark metadata as clean and 'clear' the array(??)
 *  write-pending
 *    array_state if 'write-pending'
 *    We mark metadata as 'dirty' then set array to 'active'.
 *  active_idle
 *    Either ignore, or mark clean, then mark metadata as clean.
 *
 *  device fails
 *    detected by rd-N/state reporting "faulty"
 *    mark device as 'failed' in metadata, let the kernel release the
 *    device by writing '-blocked' to rd/state, and finally write 'remove' to
 *    rd/state.  Before a disk can be replaced it must be failed and removed
 *    from all container members, this will be preemptive for the other
 *    arrays... safe?
 *
 *  sync completes
 *    sync_action was 'resync' and becomes 'idle' and resync_start becomes
 *    MaxSector
 *    Notify metadata that sync is complete.
 *
 *  recovery completes
 *    sync_action changes from 'recover' to 'idle'
 *    Check each device state and mark metadata if 'faulty' or 'in_sync'.
 *
 *  deal with resync
 *    This only happens on finding a new array... mdadm will have set
 *    'resync_start' to the correct value.  If 'resync_start' indicates that an
 *    resync needs to occur set the array to the 'active' state rather than the
 *    initial read-auto state.
 *
 *
 *
 * We wait for a change (poll/select) on array_state, sync_action, and
 * each rd-X/state file.
 * When we get any change, we check everything.  So read each state file,
 * then decide what to do.
 *
 * The core action is to write new metadata to all devices in the array.
 * This is done at most once on any wakeup.
 * After that we might:
 *   - update the array_state
 *   - set the role of some devices.
 *   - request a sync_action
 *
 */

#define ARRAY_DIRTY 1
#define ARRAY_BUSY 2
static int read_and_act(struct active_array *a, fd_set *fds)
{
	unsigned long long sync_completed;
	int check_degraded = 0;
	int check_reshape = 0;
	int deactivate = 0;
	struct mdinfo *mdi;
	int ret = 0;
	int count = 0;
	struct timeval tv;

	a->next_state = bad_word;
	a->next_action = bad_action;

	a->curr_state = read_state(a->info.state_fd);
	a->curr_action = read_action(a->action_fd);
	if (a->curr_state != clear)
		/*
		 * In "clear" state, resync_start may wrongly be set to "0"
		 * when the kernel called md_clean but didn't remove the
		 * sysfs attributes yet
		 */
		read_resync_start(a->resync_start_fd, &a->info.resync_start);
	sync_completed = read_sync_completed(a->sync_completed_fd);
	for (mdi = a->info.devs; mdi ; mdi = mdi->next) {
		mdi->next_state = 0;
		mdi->curr_state = 0;
		if (mdi->state_fd >= 0) {
			read_resync_start(mdi->recovery_fd,
					  &mdi->recovery_start);
			mdi->curr_state = read_dev_state(mdi->state_fd);
		}
		/*
		 * If array is blocked and metadata handler is able to handle
		 * BB, check if you can acknowledge them to md driver. If
		 * successful, clear faulty state and unblock the array.
		 */
		if ((mdi->curr_state & DS_BLOCKED) &&
		    a->container->ss->record_bad_block &&
		    (process_dev_ubb(a, mdi) > 0)) {
			mdi->next_state |= DS_UNBLOCK;
		}
		if (FD_ISSET(mdi->bb_fd, fds))
			check_for_cleared_bb(a, mdi);
	}

	gettimeofday(&tv, NULL);
	dprintf("(%d): %ld.%06ld state:%s prev:%s action:%s prev: %s start:%llu\n",
		a->info.container_member,
		tv.tv_sec, tv.tv_usec,
		array_states[a->curr_state],
		array_states[a->prev_state],
		sync_actions[a->curr_action],
		sync_actions[a->prev_action],
		a->info.resync_start
		);

	if ((a->curr_state == bad_word || a->curr_state <= inactive) &&
	    a->prev_state > inactive) {
		/* array has been stopped */
		a->container->ss->set_array_state(a, 1);
		a->next_state = clear;
		deactivate = 1;
	}
	if (a->curr_state == write_pending) {
		a->container->ss->set_array_state(a, 0);
		a->next_state = active;
		ret |= ARRAY_DIRTY;
	}
	if (a->curr_state == active_idle) {
		/* Set array to 'clean' FIRST, then mark clean
		 * in the metadata
		 */
		a->next_state = clean;
		ret |= ARRAY_DIRTY;
	}
	if (a->curr_state == clean) {
		a->container->ss->set_array_state(a, 1);
	}
	if (a->curr_state == active ||
	    a->curr_state == suspended)
		ret |= ARRAY_DIRTY;
	if (a->curr_state == readonly) {
		/* Well, I'm ready to handle things.  If readonly
		 * wasn't requested, transition to read-auto.
		 */
		char buf[64];
		read_attr(buf, sizeof(buf), a->metadata_fd);
		if (strncmp(buf, "external:-", 10) == 0) {
			/* explicit request for readonly array.  Leave it alone */
			;
		} else {
			if (a->container->ss->set_array_state(a, 2))
				a->next_state = read_auto; /* array is clean */
			else {
				a->next_state = active; /* Now active for recovery etc */
				ret |= ARRAY_DIRTY;
			}
		}
	}

	if (!deactivate &&
	    a->curr_action == idle &&
	    a->prev_action == resync) {
		/* A resync has finished.  The endpoint is recorded in
		 * 'sync_start'.  We don't update the metadata
		 * until the array goes inactive or readonly though.
		 * Just check if we need to fiddle spares.
		 */
		a->container->ss->set_array_state(a, a->curr_state <= clean);
		check_degraded = 1;
	}

	if (!deactivate &&
	    a->curr_action == idle &&
	    a->prev_action == recover) {
		/* A recovery has finished.  Some disks may be in sync now,
		 * and the array may no longer be degraded
		 */
		for (mdi = a->info.devs ; mdi ; mdi = mdi->next) {
			a->container->ss->set_disk(a, mdi->disk.raid_disk,
						   mdi->curr_state);
			if (! (mdi->curr_state & DS_INSYNC))
				check_degraded = 1;
			count++;
		}
		if (count != a->info.array.raid_disks)
			check_degraded = 1;
	}

	if (!deactivate &&
	    a->curr_action == reshape &&
	    a->prev_action != reshape)
		/* reshape was requested by mdadm.  Need to see if
		 * new devices have been added.  Manager does that
		 * when it sees check_reshape
		 */
		check_reshape = 1;

	/* Check for failures and if found:
	 * 1/ Record the failure in the metadata and unblock the device.
	 *    FIXME update the kernel to stop notifying on failed drives when
	 *    the array is readonly and we have cleared 'blocked'
	 * 2/ Try to remove the device if the array is writable, or can be
	 *    made writable.
	 */
	for (mdi = a->info.devs ; mdi ; mdi = mdi->next) {
		if (mdi->curr_state & DS_FAULTY) {
			a->container->ss->set_disk(a, mdi->disk.raid_disk,
						   mdi->curr_state);
			check_degraded = 1;
			if (mdi->curr_state & DS_BLOCKED)
				mdi->next_state |= DS_UNBLOCK;
			if (a->curr_state == read_auto) {
				a->container->ss->set_array_state(a, 0);
				a->next_state = active;
			}
			if (a->curr_state > readonly)
				mdi->next_state |= DS_REMOVE;
		}
	}

	/* Check for recovery checkpoint notifications.  We need to be a
	 * minimum distance away from the last checkpoint to prevent
	 * over checkpointing.  Note reshape checkpointing is handled
	 * in the second branch.
	 */
	if (sync_completed > a->last_checkpoint &&
	    sync_completed - a->last_checkpoint > a->info.component_size >> 4 &&
	    a->curr_action > reshape) {
		/* A (non-reshape) sync_action has reached a checkpoint.
		 * Record the updated position in the metadata
		 */
		a->last_checkpoint = sync_completed;
		a->container->ss->set_array_state(a, a->curr_state <= clean);
	} else if ((a->curr_action == idle && a->prev_action == reshape) ||
		   (a->curr_action == reshape &&
		    sync_completed > a->last_checkpoint)) {
		/* Reshape has progressed or completed so we need to
		 * update the array state - and possibly the array size
		 */
		if (sync_completed != 0)
			a->last_checkpoint = sync_completed;
		/* We might need to update last_checkpoint depending on
		 * the reason that reshape finished.
		 * if array reshape is really finished:
		 *        set check point to the end, this allows
		 *        set_array_state() to finalize reshape in metadata
		 * if reshape if broken: do not set checkpoint to the end
		 *        this allows for reshape restart from checkpoint
		 */
		if ((a->curr_action != reshape) &&
		    (a->prev_action == reshape)) {
			char buf[40];
			if ((sysfs_get_str(&a->info, NULL,
					  "reshape_position",
					  buf,
					  sizeof(buf)) >= 0) &&
			     strncmp(buf, "none", 4) == 0)
				a->last_checkpoint = a->info.component_size;
		}
		a->container->ss->set_array_state(a, a->curr_state <= clean);
		a->last_checkpoint = sync_completed;
	}

	if (sync_completed > a->last_checkpoint)
		a->last_checkpoint = sync_completed;

	if (sync_completed >= a->info.component_size)
		a->last_checkpoint = 0;

	a->container->ss->sync_metadata(a->container);
	dprintf("(%d): state:%s action:%s next(", a->info.container_member,
		array_states[a->curr_state], sync_actions[a->curr_action]);

	/* Effect state changes in the array */
	if (a->next_state != bad_word) {
		dprintf_cont(" state:%s", array_states[a->next_state]);
		write_attr(array_states[a->next_state], a->info.state_fd);
	}
	if (a->next_action != bad_action) {
		write_attr(sync_actions[a->next_action], a->action_fd);
		dprintf_cont(" action:%s", sync_actions[a->next_action]);
	}
	for (mdi = a->info.devs; mdi ; mdi = mdi->next) {
		if (mdi->next_state & DS_UNBLOCK) {
			dprintf_cont(" %d:-blocked", mdi->disk.raid_disk);
			write_attr("-blocked", mdi->state_fd);
		}

		if ((mdi->next_state & DS_REMOVE) && mdi->state_fd >= 0) {
			int remove_result;

			/* The kernel may not be able to immediately remove the
			 * disk.  In that case we wait a little while and
			 * try again.
			 */
			remove_result = write_attr("remove", mdi->state_fd);
			if (remove_result > 0) {
				dprintf_cont(" %d:removed", mdi->disk.raid_disk);
				close(mdi->state_fd);
				close(mdi->recovery_fd);
				close(mdi->bb_fd);
				close(mdi->ubb_fd);
				mdi->state_fd = -1;
			} else
				ret |= ARRAY_BUSY;
		}
		if (mdi->next_state & DS_INSYNC) {
			write_attr("+in_sync", mdi->state_fd);
			dprintf_cont(" %d:+in_sync", mdi->disk.raid_disk);
		}
	}
	dprintf_cont(" )\n");

	/* move curr_ to prev_ */
	a->prev_state = a->curr_state;

	a->prev_action = a->curr_action;

	for (mdi = a->info.devs; mdi ; mdi = mdi->next) {
		mdi->prev_state = mdi->curr_state;
		mdi->next_state = 0;
	}

	if (check_degraded || check_reshape) {
		/* manager will do the actual check */
		if (check_degraded)
			a->check_degraded = 1;
		if (check_reshape)
			a->check_reshape = 1;
		signal_manager();
	}

	if (deactivate)
		a->container = NULL;

	return ret;
}

static struct mdinfo *
find_device(struct active_array *a, int major, int minor)
{
	struct mdinfo *mdi;

	for (mdi = a->info.devs ; mdi ; mdi = mdi->next)
		if (mdi->disk.major == major && mdi->disk.minor == minor)
			return mdi;

	return NULL;
}

static void reconcile_failed(struct active_array *aa, struct mdinfo *failed)
{
	struct active_array *a;
	struct mdinfo *victim;

	for (a = aa; a; a = a->next) {
		if (!a->container || a->to_remove)
			continue;
		victim = find_device(a, failed->disk.major, failed->disk.minor);
		if (!victim)
			continue;

		if (!(victim->curr_state & DS_FAULTY))
			write_attr("faulty", victim->state_fd);
	}
}

#ifdef DEBUG
static void dprint_wake_reasons(fd_set *fds)
{
	int i;
	char proc_path[256];
	char link[256];
	char *basename;
	int rv;

	fprintf(stderr, "monitor: wake ( ");
	for (i = 0; i < FD_SETSIZE; i++) {
		if (FD_ISSET(i, fds)) {
			sprintf(proc_path, "/proc/%d/fd/%d",
				(int) getpid(), i);

			rv = readlink(proc_path, link, sizeof(link) - 1);
			if (rv < 0) {
				fprintf(stderr, "%d:unknown ", i);
				continue;
			}
			link[rv] = '\0';
			basename = strrchr(link, '/');
			fprintf(stderr, "%d:%s ",
				i, basename ? ++basename : link);
		}
	}
	fprintf(stderr, ")\n");
}
#endif

int monitor_loop_cnt;

static int wait_and_act(struct supertype *container, int nowait)
{
	fd_set rfds;
	int maxfd = 0;
	struct active_array **aap = &container->arrays;
	struct active_array *a, **ap;
	int rv;
	struct mdinfo *mdi;
	static unsigned int dirty_arrays = ~0; /* start at some non-zero value */

	FD_ZERO(&rfds);

	for (ap = aap ; *ap ;) {
		a = *ap;
		/* once an array has been deactivated we want to
		 * ask the manager to discard it.
		 */
		if (!a->container || a->to_remove) {
			if (discard_this) {
				ap = &(*ap)->next;
				continue;
			}
			*ap = a->next;
			a->next = NULL;
			discard_this = a;
			signal_manager();
			continue;
		}

		add_fd(&rfds, &maxfd, a->info.state_fd);
		add_fd(&rfds, &maxfd, a->action_fd);
		add_fd(&rfds, &maxfd, a->sync_completed_fd);
		for (mdi = a->info.devs ; mdi ; mdi = mdi->next) {
			add_fd(&rfds, &maxfd, mdi->state_fd);
			add_fd(&rfds, &maxfd, mdi->bb_fd);
			add_fd(&rfds, &maxfd, mdi->ubb_fd);
		}

		ap = &(*ap)->next;
	}

	if (manager_ready && (*aap == NULL || (sigterm && !dirty_arrays))) {
		/* No interesting arrays, or we have been told to
		 * terminate and everything is clean.  Lets see about
		 * exiting.  Note that blocking at this point is not a
		 * problem as there are no active arrays, there is
		 * nothing that we need to be ready to do.
		 */
		int fd;
		if (sigterm)
			fd = open_dev_excl(container->devnm);
		else
			fd = open_dev_flags(container->devnm, O_RDONLY|O_EXCL);
		if (fd >= 0 || errno != EBUSY) {
			/* OK, we are safe to leave */
			if (sigterm && !dirty_arrays)
				dprintf("caught sigterm, all clean... exiting\n");
			else
				dprintf("no arrays to monitor... exiting\n");
			if (!sigterm)
				/* On SIGTERM, someone (the take-over mdmon) will
				 * clean up
				 */
				remove_pidfile(container->devnm);
			exit_now = 1;
			signal_manager();
			close(fd);
			exit(0);
		}
	}

	if (!nowait) {
		sigset_t set;
		struct timespec ts;
		ts.tv_sec = 24*3600;
		ts.tv_nsec = 0;
		if (*aap == NULL || container->retry_soon) {
			/* just waiting to get O_EXCL access */
			ts.tv_sec = 0;
			ts.tv_nsec = 20000000ULL;
		}
		sigprocmask(SIG_UNBLOCK, NULL, &set);
		sigdelset(&set, SIGUSR1);
		monitor_loop_cnt |= 1;
		rv = pselect(maxfd+1, NULL, NULL, &rfds, &ts, &set);
		monitor_loop_cnt += 1;
		if (rv == -1) {
			if (errno == EINTR) {
				rv = 0;
				FD_ZERO(&rfds);
				dprintf("monitor: caught signal\n");
			} else
				dprintf("monitor: error %d in pselect\n",
					errno);
		}
		#ifdef DEBUG
		else
			dprint_wake_reasons(&rfds);
		#endif
		container->retry_soon = 0;
	}

	if (update_queue) {
		struct metadata_update *this;

		for (this = update_queue; this ; this = this->next)
			container->ss->process_update(container, this);

		update_queue_handled = update_queue;
		update_queue = NULL;
		signal_manager();
		container->ss->sync_metadata(container);
	}

	rv = 0;
	dirty_arrays = 0;
	for (a = *aap; a ; a = a->next) {

		if (a->replaces && !discard_this) {
			struct active_array **ap;
			for (ap = &a->next; *ap && *ap != a->replaces;
			     ap = & (*ap)->next)
				;
			if (*ap)
				*ap = (*ap)->next;
			discard_this = a->replaces;
			a->replaces = NULL;
			/* FIXME check if device->state_fd need to be cleared?*/
			signal_manager();
		}
		if (a->container && !a->to_remove) {
			int ret = read_and_act(a, &rfds);
			rv |= 1;
			dirty_arrays += !!(ret & ARRAY_DIRTY);
			/* when terminating stop manipulating the array after it
			 * is clean, but make sure read_and_act() is given a
			 * chance to handle 'active_idle'
			 */
			if (sigterm && !(ret & ARRAY_DIRTY))
				a->container = NULL; /* stop touching this array */
			if (ret & ARRAY_BUSY)
				container->retry_soon = 1;
		}
	}

	/* propagate failures across container members */
	for (a = *aap; a ; a = a->next) {
		if (!a->container || a->to_remove)
			continue;
		for (mdi = a->info.devs ; mdi ; mdi = mdi->next)
			if (mdi->curr_state & DS_FAULTY)
				reconcile_failed(*aap, mdi);
	}

	return rv;
}

void do_monitor(struct supertype *container)
{
	int rv;
	int first = 1;
	do {
		rv = wait_and_act(container, first);
		first = 0;
	} while (rv >= 0);
}
