/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2013 Neil Brown <neilb@suse.de>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 */

#include "mdadm.h"
#include "md_u.h"
#include "md_p.h"
#include <ctype.h>

int Manage_ro(char *devname, int fd, int readonly)
{
	/* switch to readonly or rw
	 *
	 * requires >= 0.90.0
	 * first check that array is runing
	 * use RESTART_ARRAY_RW or STOP_ARRAY_RO
	 *
	 */
	struct mdinfo *mdi;
	int rv = 0;

	/* If this is an externally-managed array, we need to modify the
	 * metadata_version so that mdmon doesn't undo our change.
	 */
	mdi = sysfs_read(fd, NULL, GET_LEVEL|GET_VERSION);
	if (mdi &&
	    mdi->array.major_version == -1 &&
	    is_subarray(mdi->text_version)) {
		char vers[64];
		strcpy(vers, "external:");
		strcat(vers, mdi->text_version);
		if (readonly > 0) {
			int rv;
			/* We set readonly ourselves. */
			vers[9] = '-';
			sysfs_set_str(mdi, NULL, "metadata_version", vers);

			close(fd);
			rv = sysfs_set_str(mdi, NULL, "array_state", "readonly");

			if (rv < 0) {
				pr_err("failed to set readonly for %s: %s\n",
					devname, strerror(errno));

				vers[9] = mdi->text_version[0];
				sysfs_set_str(mdi, NULL, "metadata_version", vers);
				rv = 1;
				goto out;
			}
		} else {
			char *cp;
			/* We cannot set read/write - must signal mdmon */
			vers[9] = '/';
			sysfs_set_str(mdi, NULL, "metadata_version", vers);

			cp = strchr(vers+10, '/');
			if (cp)
				*cp = 0;
			ping_monitor(vers+10);
			if (mdi->array.level <= 0)
				sysfs_set_str(mdi, NULL, "array_state", "active");
		}
		goto out;
	}

	if (!md_array_active(fd)) {
		pr_err("%s does not appear to be active.\n", devname);
		rv = 1;
		goto out;
	}

	if (readonly > 0) {
		if (ioctl(fd, STOP_ARRAY_RO, NULL)) {
			pr_err("failed to set readonly for %s: %s\n",
				devname, strerror(errno));
			rv = 1;
			goto out;
		}
	} else if (readonly < 0) {
		if (ioctl(fd, RESTART_ARRAY_RW, NULL)) {
			pr_err("failed to set writable for %s: %s\n",
				devname, strerror(errno));
			rv = 1;
			goto out;
		}
	}
out:
	sysfs_free(mdi);
	return rv;
}

static void remove_devices(char *devnm, char *path)
{
	/*
	 * Remove names at 'path' - possibly with
	 * partition suffixes - which link to the 'standard'
	 * name for devnm.  These were probably created
	 * by mdadm when the array was assembled.
	 */
	char base[40];
	char *path2;
	char link[1024];
	int n;
	int part;
	char *be;
	char *pe;

	if (!path)
		return;

	sprintf(base, "/dev/%s", devnm);
	be = base + strlen(base);

	path2 = xmalloc(strlen(path)+20);
	strcpy(path2, path);
	pe = path2 + strlen(path2);

	for (part = 0; part < 16; part++) {
		if (part) {
			sprintf(be, "p%d", part);

			if (isdigit(pe[-1]))
				sprintf(pe, "p%d", part);
			else
				sprintf(pe, "%d", part);
		}
		n = readlink(path2, link, sizeof(link));
		if (n > 0 && (int)strlen(base) == n &&
		    strncmp(link, base, n) == 0)
			unlink(path2);
	}
	free(path2);
}

int Manage_run(char *devname, int fd, struct context *c)
{
	/* Run the array.  Array must already be configured
	 *  Requires >= 0.90.0
	 */
	char nm[32], *nmp;

	nmp = fd2devnm(fd);
	if (!nmp) {
		pr_err("Cannot find %s in sysfs!!\n", devname);
		return 1;
	}
	strcpy(nm, nmp);
	return IncrementalScan(c, nm);
}

int Manage_stop(char *devname, int fd, int verbose, int will_retry)
{
	/* Stop the array.  Array must already be configured
	 * 'will_retry' means that error messages are not wanted.
	 */
	int rv = 0;
	struct map_ent *map = NULL;
	struct mdinfo *mdi;
	char devnm[32];
	char container[32];
	int err;
	int count;
	char buf[32];
	unsigned long long rd1, rd2;

	if (will_retry && verbose == 0)
		verbose = -1;

	strcpy(devnm, fd2devnm(fd));
	/* Get EXCL access first.  If this fails, then attempting
	 * to stop is probably a bad idea.
	 */
	mdi = sysfs_read(fd, NULL, GET_LEVEL|GET_COMPONENT|GET_VERSION);
	if (mdi && is_subarray(mdi->text_version)) {
		char *sl;
		strncpy(container, mdi->text_version+1, sizeof(container));
		container[sizeof(container)-1] = 0;
		sl = strchr(container, '/');
		if (sl)
			*sl = 0;
	} else
		container[0] = 0;
	close(fd);
	count = 5;
	while (((fd = ((devname[0] == '/')
		       ?open(devname, O_RDONLY|O_EXCL)
		       :open_dev_flags(devnm, O_RDONLY|O_EXCL))) < 0 ||
		strcmp(fd2devnm(fd), devnm) != 0) && container[0] &&
	       mdmon_running(container) && count) {
		/* Can't open, so something might be wrong.  However it
		 * is a container, so we might be racing with mdmon, so
		 * retry for a bit.
		 */
		if (fd >= 0)
			close(fd);
		flush_mdmon(container);
		count--;
	}
	if (fd < 0 || strcmp(fd2devnm(fd), devnm) != 0) {
		if (fd >= 0)
			close(fd);
		if (verbose >= 0)
			pr_err("Cannot get exclusive access to %s:Perhaps a running process, mounted filesystem or active volume group?\n",
			       devname);
		return 1;
	}
	/* If this is an mdmon managed array, just write 'inactive'
	 * to the array state and let mdmon clear up.
	 */
	if (mdi &&
	    mdi->array.level > 0 &&
	    is_subarray(mdi->text_version)) {
		int err;
		/* This is mdmon managed. */
		close(fd);

		/* As we had an O_EXCL open, any use of the device
		 * which blocks STOP_ARRAY is probably a transient use,
		 * so it is reasonable to retry for a while - 5 seconds.
		 */
		count = 25;
		while (count &&
		       (err = sysfs_set_str(mdi, NULL,
					    "array_state",
					    "inactive")) < 0 &&
		       errno == EBUSY) {
			usleep(200000);
			count--;
		}
		if (err) {
			if (verbose >= 0)
				pr_err("failed to stop array %s: %s\n",
				       devname, strerror(errno));
			rv = 1;
			goto out;
		}

		/* Give monitor a chance to act */
		ping_monitor(mdi->text_version);

		fd = open_dev_excl(devnm);
		if (fd < 0) {
			if (verbose >= 0)
				pr_err("failed to completely stop %s: Device is busy\n",
				       devname);
			rv = 1;
			goto out;
		}
	} else if (mdi &&
		   mdi->array.major_version == -1 &&
		   mdi->array.minor_version == -2 &&
		   !is_subarray(mdi->text_version)) {
		struct mdstat_ent *mds, *m;
		/* container, possibly mdmon-managed.
		 * Make sure mdmon isn't opening it, which
		 * would interfere with the 'stop'
		 */
		ping_monitor(mdi->sys_name);

		/* now check that there are no existing arrays
		 * which are members of this array
		 */
		mds = mdstat_read(0, 0);
		for (m = mds; m; m = m->next)
			if (m->metadata_version &&
			    strncmp(m->metadata_version, "external:", 9)==0 &&
			    metadata_container_matches(m->metadata_version+9,
						       devnm)) {
				if (verbose >= 0)
					pr_err("Cannot stop container %s: member %s still active\n",
					       devname, m->devnm);
				free_mdstat(mds);
				rv = 1;
				goto out;
			}
	}

	/* If the array is undergoing a reshape which changes the number
	 * of devices, then it would be nice to stop it at a point where
	 * it has completed a full number of stripes in both old and
	 * new layouts as this will allow the reshape to be reverted.
	 * So if 'sync_action' is "reshape" and 'raid_disks' shows two
	 * different numbers, then
	 *  - freeze reshape
	 *  - set sync_max to next multiple of both data_disks and
	 *    chunk sizes (or next but one)
	 *  - unfreeze reshape
	 *  - wait on 'sync_completed' for that point to be reached.
	 */
	if (mdi && (mdi->array.level >= 4 && mdi->array.level <= 6) &&
	    sysfs_attribute_available(mdi, NULL, "sync_action") &&
	    sysfs_attribute_available(mdi, NULL, "reshape_direction") &&
	    sysfs_get_str(mdi, NULL, "sync_action", buf, 20) > 0 &&
	    strcmp(buf, "reshape\n") == 0 &&
	    sysfs_get_two(mdi, NULL, "raid_disks", &rd1, &rd2) == 2) {
		unsigned long long position, curr;
		unsigned long long chunk1, chunk2;
		unsigned long long rddiv, chunkdiv;
		unsigned long long sectors;
		unsigned long long sync_max, old_sync_max;
		unsigned long long completed;
		int backwards = 0;
		int delay;
		int scfd;

		delay = 40;
		while (rd1 > rd2 && delay > 0 &&
		       sysfs_get_ll(mdi, NULL, "sync_max", &old_sync_max) == 0) {
			/* must be in the critical section - wait a bit */
			delay -= 1;
			usleep(100000);
		}

		if (sysfs_set_str(mdi, NULL, "sync_action", "frozen") != 0)
			goto done;
		/* Array is frozen */

		rd1 -= mdi->array.level == 6 ? 2 : 1;
		rd2 -= mdi->array.level == 6 ? 2 : 1;
		sysfs_get_str(mdi, NULL, "reshape_direction", buf, sizeof(buf));
		if (strncmp(buf, "back", 4) == 0)
			backwards = 1;
		if (sysfs_get_ll(mdi, NULL, "reshape_position", &position) != 0) {
			/* reshape must have finished now */
			sysfs_set_str(mdi, NULL, "sync_action", "idle");
			goto done;
		}
		sysfs_get_two(mdi, NULL, "chunk_size", &chunk1, &chunk2);
		chunk1 /= 512;
		chunk2 /= 512;
		rddiv = GCD(rd1, rd2);
		chunkdiv = GCD(chunk1, chunk2);
		sectors = (chunk1/chunkdiv) * chunk2 * (rd1/rddiv) * rd2;

		if (backwards) {
			/* Need to subtract 'reshape_position' from
			 * array size to get equivalent of sync_max.
			 * Size calculation based on raid5_size in kernel.
			 */
			unsigned long long size = mdi->component_size;
			size &= ~(chunk1-1);
			size &= ~(chunk2-1);
			/* rd1 must be smaller */
			/* Reshape may have progressed further backwards than
			 * recorded, so target even further back (hence "-1")
			 */
			position = (position / sectors - 1) * sectors;
			/* rd1 is always the conversion factor between 'sync'
			 * position and 'reshape' position.
			 * We read 1 "new" stripe worth of data from where-ever,
			 * and when write out that full stripe.
			 */
			sync_max = size - position/rd1;
		} else {
			/* Reshape will very likely be beyond position, and it may
			 * be too late to stop at '+1', so aim for '+2'
			 */
			position = (position / sectors + 2) * sectors;
			sync_max = position/rd1;
		}
		if (sysfs_get_ll(mdi, NULL, "sync_max", &old_sync_max) < 0)
			old_sync_max = mdi->component_size;
		/* Must not advance sync_max as that could confuse
		 * the reshape monitor */
		if (sync_max < old_sync_max)
			sysfs_set_num(mdi, NULL, "sync_max", sync_max);
		sysfs_set_str(mdi, NULL, "sync_action", "idle");

		/* That should have set things going again.  Now we
		 * wait a little while (3 second max) for sync_completed
		 * to reach the target.
		 * The reshape process can block for 500msec if
		 * the sync speed limit is hit, so we need to wait
		 * a lot longer than that. 1 second is usually
		 * enough.  3 is safe.
		 */
		delay = 3000;
		scfd = sysfs_open(mdi->sys_name, NULL, "sync_completed");
		while (scfd >= 0 && delay > 0 && old_sync_max > 0) {
			unsigned long long max_completed;
			sysfs_get_ll(mdi, NULL, "reshape_position", &curr);
			sysfs_fd_get_str(scfd, buf, sizeof(buf));
			if (strncmp(buf, "none", 4) == 0) {
				/* Either reshape has aborted, or hasn't
				 * quite started yet.  Wait a bit and
				 * check  'sync_action' to see.
				 */
				usleep(10000);
				sysfs_get_str(mdi, NULL, "sync_action", buf, sizeof(buf));
				if (strncmp(buf, "reshape", 7) != 0)
					break;
			}

			if (sysfs_fd_get_two(scfd, &completed,
					     &max_completed) == 2 &&
			    /* 'completed' sometimes reads as max-uulong */
			    completed < max_completed &&
			    (completed > sync_max ||
			     (completed == sync_max && curr != position))) {
				while (completed > sync_max) {
					sync_max += sectors / rd1;
					if (backwards)
						position -= sectors;
					else
						position += sectors;
				}
				if (sync_max < old_sync_max)
					sysfs_set_num(mdi, NULL, "sync_max", sync_max);
			}

			if (!backwards && curr >= position)
				break;
			if (backwards && curr <= position)
				break;
			sysfs_wait(scfd, &delay);
		}
		if (scfd >= 0)
			close(scfd);

	}
done:

	/* As we have an O_EXCL open, any use of the device
	 * which blocks STOP_ARRAY is probably a transient use,
	 * so it is reasonable to retry for a while - 5 seconds.
	 */
	count = 25; err = 0;
	while (count && fd >= 0 &&
	       (err = ioctl(fd, STOP_ARRAY, NULL)) < 0 && errno == EBUSY) {
		usleep(200000);
		count --;
	}
	if (fd >= 0 && err) {
		if (verbose >= 0) {
			pr_err("failed to stop array %s: %s\n",
			       devname, strerror(errno));
			if (errno == EBUSY)
				cont_err("Perhaps a running process, mounted filesystem or active volume group?\n");
		}
		rv = 1;
		goto out;
	}

	if (get_linux_version() < 2006028) {
		/* prior to 2.6.28, KOBJ_CHANGE was not sent when an md array
		 * was stopped, so We'll do it here just to be sure.  Drop any
		 * partitions as well...
		 */
		if (fd >= 0)
			ioctl(fd, BLKRRPART, 0);
		if (mdi)
			sysfs_uevent(mdi, "change");
	}

	if (devnm[0] && use_udev()) {
		struct map_ent *mp = map_by_devnm(&map, devnm);
		remove_devices(devnm, mp ? mp->path : NULL);
	}

	if (verbose >= 0)
		pr_err("stopped %s\n", devname);
	map_lock(&map);
	map_remove(&map, devnm);
	map_unlock(&map);
out:
	sysfs_free(mdi);

	return rv;
}

static struct mddev_dev *add_one(struct mddev_dev *dv, char *name, char disp)
{
	struct mddev_dev *new;
	new = xmalloc(sizeof(*new));
	memset(new, 0, sizeof(*new));
	new->devname = xstrdup(name);
	new->disposition = disp;
	new->next = dv->next;
	dv->next = new;
	return new;
}

static void add_faulty(struct mddev_dev *dv, int fd, char disp)
{
	mdu_array_info_t array;
	mdu_disk_info_t disk;
	int remaining_disks;
	int i;

	if (md_get_array_info(fd, &array) != 0)
		return;

	remaining_disks = array.nr_disks;
	for (i = 0; i < MAX_DISKS && remaining_disks > 0; i++) {
		char buf[40];
		disk.number = i;
		if (md_get_disk_info(fd, &disk) != 0)
			continue;
		if (disk.major == 0 && disk.minor == 0)
			continue;
		remaining_disks--;
		if ((disk.state & 1) == 0) /* not faulty */
			continue;
		sprintf(buf, "%d:%d", disk.major, disk.minor);
		dv = add_one(dv, buf, disp);
	}
}

static void add_detached(struct mddev_dev *dv, int fd, char disp)
{
	mdu_array_info_t array;
	mdu_disk_info_t disk;
	int remaining_disks;
	int i;

	if (md_get_array_info(fd, &array) != 0)
		return;

	remaining_disks = array.nr_disks;
	for (i = 0; i < MAX_DISKS && remaining_disks > 0; i++) {
		char buf[40];
		int sfd;
		disk.number = i;
		if (md_get_disk_info(fd, &disk) != 0)
			continue;
		if (disk.major == 0 && disk.minor == 0)
			continue;
		remaining_disks--;
		if (disp == 'f' && (disk.state & 1) != 0) /* already faulty */
			continue;
		sprintf(buf, "%d:%d", disk.major, disk.minor);
		sfd = dev_open(buf, O_RDONLY);
		if (sfd >= 0) {
			/* Not detached */
			close(sfd);
			continue;
		}
		if (errno != ENXIO)
			/* Probably not detached */
			continue;
		dv = add_one(dv, buf, disp);
	}
}

static void add_set(struct mddev_dev *dv, int fd, char set_char)
{
	mdu_array_info_t array;
	mdu_disk_info_t disk;
	int remaining_disks;
	int copies, set;
	int i;

	if (md_get_array_info(fd, &array) != 0)
		return;
	if (array.level != 10)
		return;
	copies = ((array.layout & 0xff) *
		  ((array.layout >> 8) & 0xff));
	if (array.raid_disks % copies)
		return;

	remaining_disks = array.nr_disks;
	for (i = 0; i < MAX_DISKS && remaining_disks > 0; i++) {
		char buf[40];
		disk.number = i;
		if (md_get_disk_info(fd, &disk) != 0)
			continue;
		if (disk.major == 0 && disk.minor == 0)
			continue;
		remaining_disks--;
		set = disk.raid_disk % copies;
		if (set_char != set + 'A')
			continue;
		sprintf(buf, "%d:%d", disk.major, disk.minor);
		dv = add_one(dv, buf, dv->disposition);
	}
}

int attempt_re_add(int fd, int tfd, struct mddev_dev *dv,
		   struct supertype *dev_st, struct supertype *tst,
		   unsigned long rdev,
		   char *update, char *devname, int verbose,
		   mdu_array_info_t *array)
{
	struct mdinfo mdi;
	int duuid[4];
	int ouuid[4];

	dev_st->ss->getinfo_super(dev_st, &mdi, NULL);
	dev_st->ss->uuid_from_super(dev_st, ouuid);
	if (tst->sb)
		tst->ss->uuid_from_super(tst, duuid);
	else
		/* Assume uuid matches: kernel will check */
		memcpy(duuid, ouuid, sizeof(ouuid));
	if ((mdi.disk.state & (1<<MD_DISK_ACTIVE)) &&
	    !(mdi.disk.state & (1<<MD_DISK_FAULTY)) &&
	    memcmp(duuid, ouuid, sizeof(ouuid))==0) {
		/* Looks like it is worth a
		 * try.  Need to make sure
		 * kernel will accept it
		 * though.
		 */
		mdu_disk_info_t disc;
		/* re-add doesn't work for version-1 superblocks
		 * before 2.6.18 :-(
		 */
		if (array->major_version == 1 &&
		    get_linux_version() <= 2006018)
			goto skip_re_add;
		disc.number = mdi.disk.number;
		if (md_get_disk_info(fd, &disc) != 0 ||
		    disc.major != 0 || disc.minor != 0)
			goto skip_re_add;
		disc.major = major(rdev);
		disc.minor = minor(rdev);
		disc.number = mdi.disk.number;
		disc.raid_disk = mdi.disk.raid_disk;
		disc.state = mdi.disk.state;
		if (array->state & (1 << MD_SB_CLUSTERED)) {
			/* extra flags are needed when adding to a cluster as
			 * there are two cases to distinguish
			 */
			if (dv->disposition == 'c')
				disc.state |= (1 << MD_DISK_CANDIDATE);
			else
				disc.state |= (1 << MD_DISK_CLUSTER_ADD);
		}
		if (dv->writemostly == FlagSet)
			disc.state |= 1 << MD_DISK_WRITEMOSTLY;
		if (dv->writemostly == FlagClear)
			disc.state &= ~(1 << MD_DISK_WRITEMOSTLY);
		if (dv->failfast == FlagSet)
			disc.state |= 1 << MD_DISK_FAILFAST;
		if (dv->failfast == FlagClear)
			disc.state &= ~(1 << MD_DISK_FAILFAST);
		remove_partitions(tfd);
		if (update || dv->writemostly != FlagDefault ||
		    dv->failfast != FlagDefault) {
			int rv = -1;
			tfd = dev_open(dv->devname, O_RDWR);
			if (tfd < 0) {
				pr_err("failed to open %s for superblock update during re-add\n", dv->devname);
				return -1;
			}

			if (dv->writemostly == FlagSet)
				rv = dev_st->ss->update_super(
					dev_st, NULL, "writemostly",
					devname, verbose, 0, NULL);
			if (dv->writemostly == FlagClear)
				rv = dev_st->ss->update_super(
					dev_st, NULL, "readwrite",
					devname, verbose, 0, NULL);
			if (dv->failfast == FlagSet)
				rv = dev_st->ss->update_super(
					dev_st, NULL, "failfast",
					devname, verbose, 0, NULL);
			if (dv->failfast == FlagClear)
				rv = dev_st->ss->update_super(
					dev_st, NULL, "nofailfast",
					devname, verbose, 0, NULL);
			if (update)
				rv = dev_st->ss->update_super(
					dev_st, NULL, update,
					devname, verbose, 0, NULL);
			if (rv == 0)
				rv = dev_st->ss->store_super(dev_st, tfd);
			close(tfd);
			if (rv != 0) {
				pr_err("failed to update superblock during re-add\n");
				return -1;
			}
		}
		/* don't even try if disk is marked as faulty */
		errno = 0;
		if (ioctl(fd, ADD_NEW_DISK, &disc) == 0) {
			if (verbose >= 0)
				pr_err("re-added %s\n", dv->devname);
			return 1;
		}
		if (errno == ENOMEM || errno == EROFS) {
			pr_err("add new device failed for %s: %s\n",
			       dv->devname, strerror(errno));
			if (dv->disposition == 'M')
				return 0;
			return -1;
		}
	}
skip_re_add:
	return 0;
}

int Manage_add(int fd, int tfd, struct mddev_dev *dv,
	       struct supertype *tst, mdu_array_info_t *array,
	       int force, int verbose, char *devname,
	       char *update, unsigned long rdev, unsigned long long array_size,
	       int raid_slot)
{
	unsigned long long ldsize;
	struct supertype *dev_st;
	int j;
	mdu_disk_info_t disc;

	if (!get_dev_size(tfd, dv->devname, &ldsize)) {
		if (dv->disposition == 'M')
			return 0;
		else
			return -1;
	}

	if (tst->ss == &super0 && ldsize > 4ULL*1024*1024*1024*1024) {
		/* More than 4TB is wasted on v0.90 */
		if (!force) {
			pr_err("%s is larger than %s can effectively use.\n"
			       "       Add --force is you really want to add this device.\n",
			       dv->devname, devname);
			return -1;
		}
		pr_err("%s is larger than %s can effectively use.\n"
		       "       Adding anyway as --force was given.\n",
		       dv->devname, devname);
	}
	if (!tst->ss->external && array->major_version == 0) {
		if (ioctl(fd, HOT_ADD_DISK, rdev)==0) {
			if (verbose >= 0)
				pr_err("hot added %s\n",
				       dv->devname);
			return 1;
		}

		pr_err("hot add failed for %s: %s\n",
		       dv->devname, strerror(errno));
		return -1;
	}

	if (array->not_persistent == 0 || tst->ss->external) {

		/* need to find a sample superblock to copy, and
		 * a spare slot to use.
		 * For 'external' array (well, container based),
		 * We can just load the metadata for the array->
		 */
		int array_failed;
		if (tst->sb)
			/* already loaded */;
		else if (tst->ss->external) {
			tst->ss->load_container(tst, fd, NULL);
		} else for (j = 0; j < tst->max_devs; j++) {
				char *dev;
				int dfd;
				disc.number = j;
				if (md_get_disk_info(fd, &disc))
					continue;
				if (disc.major==0 && disc.minor==0)
					continue;
				if ((disc.state & 4)==0) /* sync */
					continue;
				/* Looks like a good device to try */
				dev = map_dev(disc.major, disc.minor, 1);
				if (!dev)
					continue;
				dfd = dev_open(dev, O_RDONLY);
				if (dfd < 0)
					continue;
				if (tst->ss->load_super(tst, dfd,
							NULL)) {
					close(dfd);
					continue;
				}
				close(dfd);
				break;
			}
		/* FIXME this is a bad test to be using */
		if (!tst->sb && (dv->disposition != 'a' &&
				 dv->disposition != 'S')) {
			/* we are re-adding a device to a
			 * completely dead array - have to depend
			 * on kernel to check
			 */
		} else if (!tst->sb) {
			pr_err("cannot load array metadata from %s\n", devname);
			return -1;
		}

		/* Make sure device is large enough */
		if (dv->disposition != 'j' &&  /* skip size check for Journal */
		    tst->sb &&
		    tst->ss->avail_size(tst, ldsize/512, INVALID_SECTORS) <
		    array_size) {
			if (dv->disposition == 'M')
				return 0;
			pr_err("%s not large enough to join array\n",
			       dv->devname);
			return -1;
		}

		/* Possibly this device was recently part of
		 * the array and was temporarily removed, and
		 * is now being re-added.  If so, we can
		 * simply re-add it.
		 */

		if (array->not_persistent == 0) {
			dev_st = dup_super(tst);
			dev_st->ss->load_super(dev_st, tfd, NULL);
			if (dev_st->sb && dv->disposition != 'S') {
				int rv;

				rv = attempt_re_add(fd, tfd, dv, dev_st, tst,
						    rdev, update, devname,
						    verbose, array);
				dev_st->ss->free_super(dev_st);
				if (rv)
					return rv;
			}
		}
		if (dv->disposition == 'M') {
			if (verbose > 0)
				pr_err("--re-add for %s to %s is not possible\n",
				       dv->devname, devname);
			return 0;
		}
		if (dv->disposition == 'A') {
			pr_err("--re-add for %s to %s is not possible\n",
			       dv->devname, devname);
			return -1;
		}
		if (array->active_disks < array->raid_disks) {
			char *avail = xcalloc(array->raid_disks, 1);
			int d;
			int found = 0;

			for (d = 0; d < MAX_DISKS && found < array->nr_disks; d++) {
				disc.number = d;
				if (md_get_disk_info(fd, &disc))
					continue;
				if (disc.major == 0 && disc.minor == 0)
					continue;
				if (!(disc.state & (1<<MD_DISK_SYNC)))
					continue;
				avail[disc.raid_disk] = 1;
				found++;
			}
			array_failed = !enough(array->level, array->raid_disks,
					       array->layout, 1, avail);
			free(avail);
		} else
			array_failed = 0;
		if (array_failed) {
			pr_err("%s has failed so using --add cannot work and might destroy\n",
			       devname);
			pr_err("data on %s.  You should stop the array and re-assemble it.\n",
			       dv->devname);
			return -1;
		}
	} else {
		/* non-persistent. Must ensure that new drive
		 * is at least array->size big.
		 */
		if (ldsize/512 < array_size) {
			pr_err("%s not large enough to join array\n",
			       dv->devname);
			return -1;
		}
	}
	/* committed to really trying this device now*/
	remove_partitions(tfd);

	/* in 2.6.17 and earlier, version-1 superblocks won't
	 * use the number we write, but will choose a free number.
	 * we must choose the same free number, which requires
	 * starting at 'raid_disks' and counting up
	 */
	for (j = array->raid_disks; j < tst->max_devs; j++) {
		disc.number = j;
		if (md_get_disk_info(fd, &disc))
			break;
		if (disc.major==0 && disc.minor==0)
			break;
		if (disc.state & 8) /* removed */
			break;
	}
	disc.major = major(rdev);
	disc.minor = minor(rdev);
	if (raid_slot < 0)
		disc.number = j;
	else
		disc.number = raid_slot;
	disc.state = 0;

	/* only add journal to array that supports journaling */
	if (dv->disposition == 'j') {
		struct mdinfo *mdp;

		mdp = sysfs_read(fd, NULL, GET_ARRAY_STATE);
		if (!mdp) {
			pr_err("%s unable to read array state.\n", devname);
			return -1;
		}

		if (mdp->array_state != ARRAY_READONLY) {
			sysfs_free(mdp);
			pr_err("%s is not readonly, cannot add journal.\n", devname);
			return -1;
		}

		sysfs_free(mdp);

		disc.raid_disk = 0;
	}

	if (array->not_persistent==0) {
		int dfd;
		if (dv->disposition == 'j')
			disc.state |= (1 << MD_DISK_JOURNAL) | (1 << MD_DISK_SYNC);
		if (dv->writemostly == FlagSet)
			disc.state |= 1 << MD_DISK_WRITEMOSTLY;
		if (dv->failfast == FlagSet)
			disc.state |= 1 << MD_DISK_FAILFAST;
		dfd = dev_open(dv->devname, O_RDWR | O_EXCL|O_DIRECT);
		if (tst->ss->add_to_super(tst, &disc, dfd,
					  dv->devname, INVALID_SECTORS))
			return -1;
		if (tst->ss->write_init_super(tst))
			return -1;
	} else if (dv->disposition == 'A') {
		/*  this had better be raid1.
		 * As we are "--re-add"ing we must find a spare slot
		 * to fill.
		 */
		char *used = xcalloc(array->raid_disks, 1);
		for (j = 0; j < tst->max_devs; j++) {
			mdu_disk_info_t disc2;
			disc2.number = j;
			if (md_get_disk_info(fd, &disc2))
				continue;
			if (disc2.major==0 && disc2.minor==0)
				continue;
			if (disc2.state & 8) /* removed */
				continue;
			if (disc2.raid_disk < 0)
				continue;
			if (disc2.raid_disk > array->raid_disks)
				continue;
			used[disc2.raid_disk] = 1;
		}
		for (j = 0 ; j < array->raid_disks; j++)
			if (!used[j]) {
				disc.raid_disk = j;
				disc.state |= (1<<MD_DISK_SYNC);
				break;
			}
		free(used);
	}

	if (array->state & (1 << MD_SB_CLUSTERED)) {
		if (dv->disposition == 'c')
			disc.state |= (1 << MD_DISK_CANDIDATE);
		else
			disc.state |= (1 << MD_DISK_CLUSTER_ADD);
	}

	if (dv->writemostly == FlagSet)
		disc.state |= (1 << MD_DISK_WRITEMOSTLY);
	if (dv->failfast == FlagSet)
		disc.state |= (1 << MD_DISK_FAILFAST);
	if (tst->ss->external) {
		/* add a disk
		 * to an external metadata container */
		struct mdinfo new_mdi;
		struct mdinfo *sra;
		int container_fd;
		char devnm[32];
		int dfd;

		strcpy(devnm, fd2devnm(fd));

		container_fd = open_dev_excl(devnm);
		if (container_fd < 0) {
			pr_err("add failed for %s: could not get exclusive access to container\n",
			       dv->devname);
			tst->ss->free_super(tst);
			return -1;
		}

		Kill(dv->devname, NULL, 0, -1, 0);
		dfd = dev_open(dv->devname, O_RDWR | O_EXCL|O_DIRECT);
		if (mdmon_running(tst->container_devnm))
			tst->update_tail = &tst->updates;
		if (tst->ss->add_to_super(tst, &disc, dfd,
					  dv->devname, INVALID_SECTORS)) {
			close(dfd);
			close(container_fd);
			return -1;
		}
		if (tst->update_tail)
			flush_metadata_updates(tst);
		else
			tst->ss->sync_metadata(tst);

		sra = sysfs_read(container_fd, NULL, 0);
		if (!sra) {
			pr_err("add failed for %s: sysfs_read failed\n",
			       dv->devname);
			close(container_fd);
			tst->ss->free_super(tst);
			return -1;
		}
		sra->array.level = LEVEL_CONTAINER;
		/* Need to set data_offset and component_size */
		tst->ss->getinfo_super(tst, &new_mdi, NULL);
		new_mdi.disk.major = disc.major;
		new_mdi.disk.minor = disc.minor;
		new_mdi.recovery_start = 0;
		/* Make sure fds are closed as they are O_EXCL which
		 * would block add_disk */
		tst->ss->free_super(tst);
		if (sysfs_add_disk(sra, &new_mdi, 0) != 0) {
			pr_err("add new device to external metadata failed for %s\n", dv->devname);
			close(container_fd);
			sysfs_free(sra);
			return -1;
		}
		ping_monitor(devnm);
		sysfs_free(sra);
		close(container_fd);
	} else {
		tst->ss->free_super(tst);
		if (ioctl(fd, ADD_NEW_DISK, &disc)) {
			if (dv->disposition == 'j')
				pr_err("Failed to hot add %s as journal, "
				       "please try restart %s.\n", dv->devname, devname);
			else
				pr_err("add new device failed for %s as %d: %s\n",
				       dv->devname, j, strerror(errno));
			return -1;
		}
		if (dv->disposition == 'j') {
			pr_err("Journal added successfully, making %s read-write\n", devname);
			if (Manage_ro(devname, fd, -1))
				pr_err("Failed to make %s read-write\n", devname);
		}

	}
	if (verbose >= 0)
		pr_err("added %s\n", dv->devname);
	return 1;
}

int Manage_remove(struct supertype *tst, int fd, struct mddev_dev *dv,
		  int sysfd, unsigned long rdev, int force, int verbose, char *devname)
{
	int lfd = -1;
	int err;

	if (tst->ss->external) {
		/* To remove a device from a container, we must
		 * check that it isn't in use in an array.
		 * This involves looking in the 'holders'
		 * directory - there must be just one entry,
		 * the container.
		 * To ensure that it doesn't get used as a
		 * hot spare while we are checking, we
		 * get an O_EXCL open on the container
		 */
		int ret;
		char devnm[32];
		strcpy(devnm, fd2devnm(fd));
		lfd = open_dev_excl(devnm);
		if (lfd < 0) {
			pr_err("Cannot get exclusive access  to container - odd\n");
			return -1;
		}
		/* We may not be able to check on holders in
		 * sysfs, either because we don't have the dev num
		 * (rdev == 0) or because the device has been detached
		 * and the 'holders' directory no longer exists
		 * (ret == -1).  In that case, assume it is OK to
		 * remove.
		 */
		if (rdev == 0)
			ret = -1;
		else {
			/*
			 * The drive has already been set to 'faulty', however
			 * monitor might not have had time to process it and the
			 * drive might still have an entry in the 'holders'
			 * directory. Try a few times to avoid a false error
			 */
			int count = 20;

			do {
				ret = sysfs_unique_holder(devnm, rdev);
				if (ret < 2)
					break;
				usleep(100 * 1000);	/* 100ms */
			} while (--count > 0);

			if (ret == 0) {
				pr_err("%s is not a member, cannot remove.\n",
					dv->devname);
				close(lfd);
				return -1;
			}
			if (ret >= 2) {
				pr_err("%s is still in use, cannot remove.\n",
					dv->devname);
				close(lfd);
				return -1;
			}
		}
	}
	/* FIXME check that it is a current member */
	if (sysfd >= 0) {
		/* device has been removed and we don't know
		 * the major:minor number
		 */
		err = sys_hot_remove_disk(sysfd, force);
	} else {
		err = hot_remove_disk(fd, rdev, force);
		if (err && errno == ENODEV) {
			/* Old kernels rejected this if no personality
			 * is registered */
			struct mdinfo *sra = sysfs_read(fd, NULL, GET_DEVS);
			struct mdinfo *dv = NULL;
			if (sra)
				dv = sra->devs;
			for ( ; dv ; dv=dv->next)
				if (dv->disk.major == (int)major(rdev) &&
				    dv->disk.minor == (int)minor(rdev))
					break;
			if (dv)
				err = sysfs_set_str(sra, dv,
						    "state", "remove");
			else
				err = -1;
			sysfs_free(sra);
		}
	}
	if (err) {
		pr_err("hot remove failed for %s: %s\n",	dv->devname,
		       strerror(errno));
		if (lfd >= 0)
			close(lfd);
		return -1;
	}
	if (tst->ss->external) {
		/*
		 * Before dropping our exclusive open we make an
		 * attempt at preventing mdmon from seeing an
		 * 'add' event before reconciling this 'remove'
		 * event.
		 */
		char *devnm = fd2devnm(fd);

		if (!devnm) {
			pr_err("unable to get container name\n");
			return -1;
		}

		ping_manager(devnm);
	}
	if (lfd >= 0)
		close(lfd);
	if (verbose >= 0)
		pr_err("hot removed %s from %s\n",
		       dv->devname, devname);
	return 1;
}

int Manage_replace(struct supertype *tst, int fd, struct mddev_dev *dv,
		   unsigned long rdev, int verbose, char *devname)
{
	struct mdinfo *mdi, *di;
	if (tst->ss->external) {
		pr_err("--replace only supported for native metadata (0.90 or 1.x)\n");
		return -1;
	}
	/* Need to find the device in sysfs and add 'want_replacement' to the
	 * status.
	 */
	mdi = sysfs_read(fd, NULL, GET_DEVS);
	if (!mdi || !mdi->devs) {
		pr_err("Cannot find status of %s to enable replacement - strange\n",
		       devname);
		return -1;
	}
	for (di = mdi->devs; di; di = di->next)
		if (di->disk.major == (int)major(rdev) &&
		    di->disk.minor == (int)minor(rdev))
			break;
	if (di) {
		int rv;
		if (di->disk.raid_disk < 0) {
			pr_err("%s is not active and so cannot be replaced.\n",
			       dv->devname);
			sysfs_free(mdi);
			return -1;
		}
		rv = sysfs_set_str(mdi, di,
				   "state", "want_replacement");
		if (rv) {
			sysfs_free(mdi);
			pr_err("Failed to request replacement for %s\n",
			       dv->devname);
			return -1;
		}
		if (verbose >= 0)
			pr_err("Marked %s (device %d in %s) for replacement\n",
			       dv->devname, di->disk.raid_disk, devname);
		/* If there is a matching 'with', we need to tell it which
		 * raid disk
		 */
		while (dv && dv->disposition != 'W')
			dv = dv->next;
		if (dv) {
			dv->disposition = 'w';
			dv->used = di->disk.raid_disk;
		}
		return 1;
	}
	sysfs_free(mdi);
	pr_err("%s not found in %s so cannot --replace it\n",
	       dv->devname, devname);
	return -1;
}

int Manage_with(struct supertype *tst, int fd, struct mddev_dev *dv,
		unsigned long rdev, int verbose, char *devname)
{
	struct mdinfo *mdi, *di;
	/* try to set 'slot' for 'rdev' in 'fd' to 'dv->used' */
	mdi = sysfs_read(fd, NULL, GET_DEVS|GET_STATE);
	if (!mdi || !mdi->devs) {
		pr_err("Cannot find status of %s to enable replacement - strange\n",
		       devname);
		return -1;
	}
	for (di = mdi->devs; di; di = di->next)
		if (di->disk.major == (int)major(rdev) &&
		    di->disk.minor == (int)minor(rdev))
			break;
	if (di) {
		int rv;
		if (di->disk.state & (1<<MD_DISK_FAULTY)) {
			pr_err("%s is faulty and cannot be a replacement\n",
			       dv->devname);
			sysfs_free(mdi);
			return -1;
		}
		if (di->disk.raid_disk >= 0) {
			pr_err("%s is active and cannot be a replacement\n",
			       dv->devname);
			sysfs_free(mdi);
			return -1;
		}
		rv = sysfs_set_num(mdi, di,
				   "slot", dv->used);
		if (rv) {
			sysfs_free(mdi);
			pr_err("Failed to set %s as preferred replacement.\n",
			       dv->devname);
			return -1;
		}
		if (verbose >= 0)
			pr_err("Marked %s in %s as replacement for device %d\n",
			       dv->devname, devname, dv->used);
		return 1;
	}
	sysfs_free(mdi);
	pr_err("%s not found in %s so cannot make it preferred replacement\n",
	       dv->devname, devname);
	return -1;
}

int Manage_subdevs(char *devname, int fd,
		   struct mddev_dev *devlist, int verbose, int test,
		   char *update, int force)
{
	/* Do something to each dev.
	 * devmode can be
	 *  'a' - add the device
	 *	   try HOT_ADD_DISK
	 *         If that fails EINVAL, try ADD_NEW_DISK
	 *  'S' - add the device as a spare - don't try re-add
	 *  'j' - add the device as a journal device
	 *  'A' - re-add the device
	 *  'r' - remove the device: HOT_REMOVE_DISK
	 *        device can be 'faulty' or 'detached' in which case all
	 *	  matching devices are removed.
	 *  'f' - set the device faulty SET_DISK_FAULTY
	 *        device can be 'detached' in which case any device that
	 *	  is inaccessible will be marked faulty.
	 *  'R' - mark this device as wanting replacement.
	 *  'W' - this device is added if necessary and activated as
	 *        a replacement for a previous 'R' device.
	 * -----
	 *  'w' - 'W' will be changed to 'w' when it is paired with
	 *        a 'R' device.  If a 'W' is found while walking the list
	 *        it must be unpaired, and is an error.
	 *  'M' - this is created by a 'missing' target.  It is a slight
	 *        variant on 'A'
	 *  'F' - Another variant of 'A', where the device was faulty
	 *        so must be removed from the array first.
	 *  'c' - confirm the device as found (for clustered environments)
	 *
	 * For 'f' and 'r', the device can also be a kernel-internal
	 * name such as 'sdb'.
	 */
	mdu_array_info_t array;
	unsigned long long array_size;
	struct mddev_dev *dv;
	int tfd = -1;
	struct supertype *tst;
	char *subarray = NULL;
	int sysfd = -1;
	int count = 0; /* number of actions taken */
	struct mdinfo info;
	struct mdinfo devinfo;
	int frozen = 0;
	int busy = 0;
	int raid_slot = -1;

	if (sysfs_init(&info, fd, NULL)) {
		pr_err("sysfs not availabile for %s\n", devname);
		goto abort;
	}

	if (md_get_array_info(fd, &array)) {
		pr_err("Cannot get array info for %s\n", devname);
		goto abort;
	}
	/* array.size is only 32 bits and may be truncated.
	 * So read from sysfs if possible, and record number of sectors
	 */

	array_size = get_component_size(fd);
	if (array_size <= 0)
		array_size = array.size * 2;

	tst = super_by_fd(fd, &subarray);
	if (!tst) {
		pr_err("unsupport array - version %d.%d\n",
			array.major_version, array.minor_version);
		goto abort;
	}

	for (dv = devlist; dv; dv = dv->next) {
		dev_t rdev = 0; /* device to add/remove etc */
		int rv;
		int mj,mn;

		raid_slot = -1;
		if (dv->disposition == 'c') {
			rv = parse_cluster_confirm_arg(dv->devname,
						       &dv->devname,
						       &raid_slot);
			if (rv) {
				pr_err("Could not get the devname of cluster\n");
				goto abort;
			}
		}

		if (strcmp(dv->devname, "failed") == 0 ||
		    strcmp(dv->devname, "faulty") == 0) {
			if (dv->disposition != 'A' && dv->disposition != 'r') {
				pr_err("%s only meaningful with -r or --re-add, not -%c\n",
					dv->devname, dv->disposition);
				goto abort;
			}
			add_faulty(dv, fd, (dv->disposition == 'A'
					    ? 'F' : 'r'));
			continue;
		}
		if (strcmp(dv->devname, "detached") == 0) {
			if (dv->disposition != 'r' && dv->disposition != 'f') {
				pr_err("%s only meaningful with -r of -f, not -%c\n",
					dv->devname, dv->disposition);
				goto abort;
			}
			add_detached(dv, fd, dv->disposition);
			continue;
		}

		if (strcmp(dv->devname, "missing") == 0) {
			struct mddev_dev *add_devlist;
			struct mddev_dev **dp;
			if (dv->disposition == 'c') {
				rv = ioctl(fd, CLUSTERED_DISK_NACK, NULL);
				break;
			}

			if (dv->disposition != 'A') {
				pr_err("'missing' only meaningful with --re-add\n");
				goto abort;
			}
			add_devlist = conf_get_devs();
			if (add_devlist == NULL) {
				pr_err("no devices to scan for missing members.\n");
				continue;
			}
			for (dp = &add_devlist; *dp; dp = & (*dp)->next)
				/* 'M' (for 'missing') is like 'A' without errors */
				(*dp)->disposition = 'M';
			*dp = dv->next;
			dv->next = add_devlist;
			continue;
		}

		if (strncmp(dv->devname, "set-", 4) == 0 &&
		    strlen(dv->devname) == 5) {
			int copies;

			if (dv->disposition != 'r' &&
			    dv->disposition != 'f') {
				pr_err("'%s' only meaningful with -r or -f\n",
				       dv->devname);
				goto abort;
			}
			if (array.level != 10) {
				pr_err("'%s' only meaningful with RAID10 arrays\n",
				       dv->devname);
				goto abort;
			}
			copies = ((array.layout & 0xff) *
				  ((array.layout >> 8) & 0xff));
			if (array.raid_disks % copies != 0 ||
			    dv->devname[4] < 'A' ||
			    dv->devname[4] >= 'A' + copies ||
			    copies > 26) {
				pr_err("'%s' not meaningful with this array\n",
				       dv->devname);
				goto abort;
			}
			add_set(dv, fd, dv->devname[4]);
			continue;
		}

		if (strchr(dv->devname, '/') == NULL &&
		    strchr(dv->devname, ':') == NULL &&
		    strlen(dv->devname) < 50) {
			/* Assume this is a kernel-internal name like 'sda1' */
			int found = 0;
			char dname[55];
			if (dv->disposition != 'r' && dv->disposition != 'f') {
				pr_err("%s only meaningful with -r or -f, not -%c\n",
					dv->devname, dv->disposition);
				goto abort;
			}

			sprintf(dname, "dev-%s", dv->devname);
			sysfd = sysfs_open(fd2devnm(fd), dname, "block/dev");
			if (sysfd >= 0) {
				char dn[20];
				if (sysfs_fd_get_str(sysfd, dn, 20) > 0 &&
				    sscanf(dn, "%d:%d", &mj,&mn) == 2) {
					rdev = makedev(mj,mn);
					found = 1;
				}
				close(sysfd);
				sysfd = -1;
			}
			if (!found) {
				sysfd = sysfs_open(fd2devnm(fd), dname, "state");
				if (sysfd < 0) {
					pr_err("%s does not appear to be a component of %s\n",
						dv->devname, devname);
					goto abort;
				}
			}
		} else if ((dv->disposition == 'r' ||
			    dv->disposition == 'f') &&
			   get_maj_min(dv->devname, &mj, &mn)) {
			/* for 'fail' and 'remove', the device might
			 * not exist.
			 */
			rdev = makedev(mj, mn);
		} else {
			tfd = dev_open(dv->devname, O_RDONLY);
			if (tfd >= 0) {
				fstat_is_blkdev(tfd, dv->devname, &rdev);
				close(tfd);
			} else {
				int open_err = errno;
				if (!stat_is_blkdev(dv->devname, &rdev)) {
					if (dv->disposition == 'M')
						/* non-fatal. Also improbable */
						continue;
					goto abort;
				}
				if (dv->disposition == 'r')
					/* Be happy, the stat worked, that is
					 * enough for --remove
					 */
					;
				else {
					if (dv->disposition == 'M')
						/* non-fatal */
						continue;
					pr_err("Cannot open %s: %s\n",
					       dv->devname, strerror(open_err));
					goto abort;
				}
			}
		}
		switch(dv->disposition){
		default:
			pr_err("internal error - devmode[%s]=%d\n",
				dv->devname, dv->disposition);
			goto abort;
		case 'a':
		case 'S': /* --add-spare */
		case 'j': /* --add-journal */
		case 'A':
		case 'M': /* --re-add missing */
		case 'F': /* --re-add faulty  */
		case 'c': /* --cluster-confirm */
			/* add the device */
			if (subarray) {
				pr_err("Cannot add disks to a \'member\' array, perform this operation on the parent container\n");
				goto abort;
			}

			/* Let's first try to write re-add to sysfs */
			if (rdev != 0 &&
			    (dv->disposition == 'A' || dv->disposition == 'F')) {
				sysfs_init_dev(&devinfo, rdev);
				if (sysfs_set_str(&info, &devinfo, "state", "re-add") == 0) {
					pr_err("re-add %s to %s succeed\n",
						dv->devname, info.sys_name);
					break;
				}
			}

			if (dv->disposition == 'F')
				/* Need to remove first */
				hot_remove_disk(fd, rdev, force);
			/* Make sure it isn't in use (in 2.6 or later) */
			tfd = dev_open(dv->devname, O_RDONLY|O_EXCL);
			if (tfd >= 0) {
				/* We know no-one else is using it.  We'll
				 * need non-exclusive access to add it, so
				 * do that now.
				 */
				close(tfd);
				tfd = dev_open(dv->devname, O_RDONLY);
			}
			if (tfd < 0) {
				if (dv->disposition == 'M')
					continue;
				pr_err("Cannot open %s: %s\n",
					dv->devname, strerror(errno));
				goto abort;
			}
			if (!frozen) {
				if (sysfs_freeze_array(&info) == 1)
					frozen = 1;
				else
					frozen = -1;
			}
			rv = Manage_add(fd, tfd, dv, tst, &array,
					force, verbose, devname, update,
					rdev, array_size, raid_slot);
			close(tfd);
			tfd = -1;
			if (rv < 0)
				goto abort;
			if (rv > 0)
				count++;
			break;

		case 'r':
			/* hot remove */
			if (subarray) {
				pr_err("Cannot remove disks from a \'member\' array, perform this operation on the parent container\n");
				rv = -1;
			} else
				rv = Manage_remove(tst, fd, dv, sysfd,
						   rdev, verbose, force,
						   devname);
			if (sysfd >= 0)
				close(sysfd);
			sysfd = -1;
			if (rv < 0)
				goto abort;
			if (rv > 0)
				count++;
			break;

		case 'f': /* set faulty */
			/* FIXME check current member */
			if ((sysfd >= 0 && write(sysfd, "faulty", 6) != 6) ||
			    (sysfd < 0 && ioctl(fd, SET_DISK_FAULTY,
						rdev))) {
				if (errno == EBUSY)
					busy = 1;
				pr_err("set device faulty failed for %s:  %s\n",
					dv->devname, strerror(errno));
				if (sysfd >= 0)
					close(sysfd);
				goto abort;
			}
			if (sysfd >= 0)
				close(sysfd);
			sysfd = -1;
			count++;
			if (verbose >= 0)
				pr_err("set %s faulty in %s\n",
					dv->devname, devname);
			break;
		case 'R': /* Mark as replaceable */
			if (subarray) {
				pr_err("Cannot replace disks in a \'member\' array, perform this operation on the parent container\n");
				rv = -1;
			} else {
				if (!frozen) {
					if (sysfs_freeze_array(&info) == 1)
						frozen = 1;
					else
						frozen = -1;
				}
				rv = Manage_replace(tst, fd, dv,
						    rdev, verbose,
						    devname);
			}
			if (rv < 0)
				goto abort;
			if (rv > 0)
				count++;
			break;
		case 'W': /* --with device that doesn't match */
			pr_err("No matching --replace device for --with %s\n",
			       dv->devname);
			goto abort;
		case 'w': /* --with device which was matched */
			rv = Manage_with(tst, fd, dv,
					 rdev, verbose, devname);
			if (rv < 0)
				goto abort;
			break;
		}
	}
	if (frozen > 0)
		sysfs_set_str(&info, NULL, "sync_action","idle");
	if (test && count == 0)
		return 2;
	return 0;

abort:
	if (frozen > 0)
		sysfs_set_str(&info, NULL, "sync_action","idle");
	return !test && busy ? 2 : 1;
}

int autodetect(void)
{
	/* Open any md device, and issue the RAID_AUTORUN ioctl */
	int rv = 1;
	int fd = dev_open("9:0", O_RDONLY);
	if (fd >= 0) {
		if (ioctl(fd, RAID_AUTORUN, 0) == 0)
			rv = 0;
		close(fd);
	}
	return rv;
}

int Update_subarray(char *dev, char *subarray, char *update, struct mddev_ident *ident, int verbose)
{
	struct supertype supertype, *st = &supertype;
	int fd, rv = 2;

	memset(st, 0, sizeof(*st));

	fd = open_subarray(dev, subarray, st, verbose < 0);
	if (fd < 0)
		return 2;

	if (!st->ss->update_subarray) {
		if (verbose >= 0)
			pr_err("Operation not supported for %s metadata\n",
			       st->ss->name);
		goto free_super;
	}

	if (mdmon_running(st->devnm))
		st->update_tail = &st->updates;

	rv = st->ss->update_subarray(st, subarray, update, ident);

	if (rv) {
		if (verbose >= 0)
			pr_err("Failed to update %s of subarray-%s in %s\n",
				update, subarray, dev);
	} else if (st->update_tail)
		flush_metadata_updates(st);
	else
		st->ss->sync_metadata(st);

	if (rv == 0 && strcmp(update, "name") == 0 && verbose >= 0)
		pr_err("Updated subarray-%s name from %s, UUIDs may have changed\n",
		       subarray, dev);

 free_super:
	st->ss->free_super(st);
	close(fd);

	return rv;
}

/* Move spare from one array to another If adding to destination array fails
 * add back to original array.
 * Returns 1 on success, 0 on failure */
int move_spare(char *from_devname, char *to_devname, dev_t devid)
{
	struct mddev_dev devlist;
	char devname[20];

	/* try to remove and add */
	int fd1 = open(to_devname, O_RDONLY);
	int fd2 = open(from_devname, O_RDONLY);

	if (fd1 < 0 || fd2 < 0) {
		if (fd1>=0) close(fd1);
		if (fd2>=0) close(fd2);
		return 0;
	}

	devlist.next = NULL;
	devlist.used = 0;
	devlist.writemostly = FlagDefault;
	devlist.failfast = FlagDefault;
	devlist.devname = devname;
	sprintf(devname, "%d:%d", major(devid), minor(devid));

	devlist.disposition = 'r';
	if (Manage_subdevs(from_devname, fd2, &devlist, -1, 0, NULL, 0) == 0) {
		devlist.disposition = 'a';
		if (Manage_subdevs(to_devname, fd1, &devlist, -1, 0, NULL, 0) == 0) {
			/* make sure manager is aware of changes */
			ping_manager(to_devname);
			ping_manager(from_devname);
			close(fd1);
			close(fd2);
			return 1;
		}
		else Manage_subdevs(from_devname, fd2, &devlist, -1, 0, NULL, 0);
	}
	close(fd1);
	close(fd2);
	return 0;
}
