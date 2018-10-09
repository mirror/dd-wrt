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
#include	"mdadm.h"
#include	"dlink.h"
#include	<sys/mman.h>
#include	<stddef.h>
#include	<stdint.h>
#include	<signal.h>
#include	<sys/wait.h>

#if ! defined(__BIG_ENDIAN) && ! defined(__LITTLE_ENDIAN)
#error no endian defined
#endif
#include	"md_u.h"
#include	"md_p.h"

int restore_backup(struct supertype *st,
		   struct mdinfo *content,
		   int working_disks,
		   int next_spare,
		   char **backup_filep,
		   int verbose)
{
	int i;
	int *fdlist;
	struct mdinfo *dev;
	int err;
	int disk_count = next_spare + working_disks;
	char *backup_file = *backup_filep;

	dprintf("Called restore_backup()\n");
	fdlist = xmalloc(sizeof(int) * disk_count);

	enable_fds(next_spare);
	for (i = 0; i < next_spare; i++)
		fdlist[i] = -1;
	for (dev = content->devs; dev; dev = dev->next) {
		char buf[22];
		int fd;

		sprintf(buf, "%d:%d", dev->disk.major, dev->disk.minor);
		fd = dev_open(buf, O_RDWR);

		if (dev->disk.raid_disk >= 0)
			fdlist[dev->disk.raid_disk] = fd;
		else
			fdlist[next_spare++] = fd;
	}

	if (!backup_file) {
		backup_file = locate_backup(content->sys_name);
		*backup_filep = backup_file;
	}

	if (st->ss->external && st->ss->recover_backup)
		err = st->ss->recover_backup(st, content);
	else
		err = Grow_restart(st, content, fdlist, next_spare,
				   backup_file, verbose > 0);

	while (next_spare > 0) {
		next_spare--;
		if (fdlist[next_spare] >= 0)
			close(fdlist[next_spare]);
	}
	free(fdlist);
	if (err) {
		pr_err("Failed to restore critical section for reshape - sorry.\n");
		if (!backup_file)
			pr_err("Possibly you need to specify a --backup-file\n");
		return 1;
	}

	dprintf("restore_backup() returns status OK.\n");
	return 0;
}

int Grow_Add_device(char *devname, int fd, char *newdev)
{
	/* Add a device to an active array.
	 * Currently, just extend a linear array.
	 * This requires writing a new superblock on the
	 * new device, calling the kernel to add the device,
	 * and if that succeeds, update the superblock on
	 * all other devices.
	 * This means that we need to *find* all other devices.
	 */
	struct mdinfo info;

	dev_t rdev;
	int nfd, fd2;
	int d, nd;
	struct supertype *st = NULL;
	char *subarray = NULL;

	if (md_get_array_info(fd, &info.array) < 0) {
		pr_err("cannot get array info for %s\n", devname);
		return 1;
	}

	if (info.array.level != -1) {
		pr_err("can only add devices to linear arrays\n");
		return 1;
	}

	st = super_by_fd(fd, &subarray);
	if (!st) {
		pr_err("cannot handle arrays with superblock version %d\n",
		       info.array.major_version);
		return 1;
	}

	if (subarray) {
		pr_err("Cannot grow linear sub-arrays yet\n");
		free(subarray);
		free(st);
		return 1;
	}

	nfd = open(newdev, O_RDWR|O_EXCL|O_DIRECT);
	if (nfd < 0) {
		pr_err("cannot open %s\n", newdev);
		free(st);
		return 1;
	}
	if (!fstat_is_blkdev(nfd, newdev, &rdev)) {
		close(nfd);
		free(st);
		return 1;
	}
	/* now check out all the devices and make sure we can read the
	 * superblock */
	for (d=0 ; d < info.array.raid_disks ; d++) {
		mdu_disk_info_t disk;
		char *dv;

		st->ss->free_super(st);

		disk.number = d;
		if (md_get_disk_info(fd, &disk) < 0) {
			pr_err("cannot get device detail for device %d\n", d);
			close(nfd);
			free(st);
			return 1;
		}
		dv = map_dev(disk.major, disk.minor, 1);
		if (!dv) {
			pr_err("cannot find device file for device %d\n", d);
			close(nfd);
			free(st);
			return 1;
		}
		fd2 = dev_open(dv, O_RDWR);
		if (fd2 < 0) {
			pr_err("cannot open device file %s\n", dv);
			close(nfd);
			free(st);
			return 1;
		}

		if (st->ss->load_super(st, fd2, NULL)) {
			pr_err("cannot find super block on %s\n", dv);
			close(nfd);
			close(fd2);
			free(st);
			return 1;
		}
		close(fd2);
	}
	/* Ok, looks good. Lets update the superblock and write it out to
	 * newdev.
	 */

	info.disk.number = d;
	info.disk.major = major(rdev);
	info.disk.minor = minor(rdev);
	info.disk.raid_disk = d;
	info.disk.state = (1 << MD_DISK_SYNC) | (1 << MD_DISK_ACTIVE);
	st->ss->update_super(st, &info, "linear-grow-new", newdev, 0, 0, NULL);

	if (st->ss->store_super(st, nfd)) {
		pr_err("Cannot store new superblock on %s\n", newdev);
		close(nfd);
		return 1;
	}
	close(nfd);

	if (ioctl(fd, ADD_NEW_DISK, &info.disk) != 0) {
		pr_err("Cannot add new disk to this array\n");
		return 1;
	}
	/* Well, that seems to have worked.
	 * Now go through and update all superblocks
	 */

	if (md_get_array_info(fd, &info.array) < 0) {
		pr_err("cannot get array info for %s\n", devname);
		return 1;
	}

	nd = d;
	for (d=0 ; d < info.array.raid_disks ; d++) {
		mdu_disk_info_t disk;
		char *dv;

		disk.number = d;
		if (md_get_disk_info(fd, &disk) < 0) {
			pr_err("cannot get device detail for device %d\n", d);
			return 1;
		}
		dv = map_dev(disk.major, disk.minor, 1);
		if (!dv) {
			pr_err("cannot find device file for device %d\n", d);
			return 1;
		}
		fd2 = dev_open(dv, O_RDWR);
		if (fd2 < 0) {
			pr_err("cannot open device file %s\n", dv);
			return 1;
		}
		if (st->ss->load_super(st, fd2, NULL)) {
			pr_err("cannot find super block on %s\n", dv);
			close(fd);
			close(fd2);
			return 1;
		}
		info.array.raid_disks = nd+1;
		info.array.nr_disks = nd+1;
		info.array.active_disks = nd+1;
		info.array.working_disks = nd+1;

		st->ss->update_super(st, &info, "linear-grow-update", dv,
				     0, 0, NULL);

		if (st->ss->store_super(st, fd2)) {
			pr_err("Cannot store new superblock on %s\n", dv);
			close(fd2);
			return 1;
		}
		close(fd2);
	}

	return 0;
}

int Grow_addbitmap(char *devname, int fd, struct context *c, struct shape *s)
{
	/*
	 * First check that array doesn't have a bitmap
	 * Then create the bitmap
	 * Then add it
	 *
	 * For internal bitmaps, we need to check the version,
	 * find all the active devices, and write the bitmap block
	 * to all devices
	 */
	mdu_bitmap_file_t bmf;
	mdu_array_info_t array;
	struct supertype *st;
	char *subarray = NULL;
	int major = BITMAP_MAJOR_HI;
	unsigned long long bitmapsize, array_size;
	struct mdinfo *mdi;

	/*
	 * We only ever get called if s->bitmap_file is != NULL, so this check
	 * is just here to quiet down static code checkers.
	 */
	if (!s->bitmap_file)
		return 1;

	if (strcmp(s->bitmap_file, "clustered") == 0)
		major = BITMAP_MAJOR_CLUSTERED;

	if (ioctl(fd, GET_BITMAP_FILE, &bmf) != 0) {
		if (errno == ENOMEM)
			pr_err("Memory allocation failure.\n");
		else
			pr_err("bitmaps not supported by this kernel.\n");
		return 1;
	}
	if (bmf.pathname[0]) {
		if (strcmp(s->bitmap_file,"none") == 0) {
			if (ioctl(fd, SET_BITMAP_FILE, -1) != 0) {
				pr_err("failed to remove bitmap %s\n",
					bmf.pathname);
				return 1;
			}
			return 0;
		}
		pr_err("%s already has a bitmap (%s)\n", devname, bmf.pathname);
		return 1;
	}
	if (md_get_array_info(fd, &array) != 0) {
		pr_err("cannot get array status for %s\n", devname);
		return 1;
	}
	if (array.state & (1 << MD_SB_BITMAP_PRESENT)) {
		if (strcmp(s->bitmap_file, "none")==0) {
			array.state &= ~(1 << MD_SB_BITMAP_PRESENT);
			if (md_set_array_info(fd, &array) != 0) {
				if (array.state & (1 << MD_SB_CLUSTERED))
					pr_err("failed to remove clustered bitmap.\n");
				else
					pr_err("failed to remove internal bitmap.\n");
				return 1;
			}
			return 0;
		}
		pr_err("bitmap already present on %s\n", devname);
		return 1;
	}

	if (strcmp(s->bitmap_file, "none") == 0) {
		pr_err("no bitmap found on %s\n", devname);
		return 1;
	}
	if (array.level <= 0) {
		pr_err("Bitmaps not meaningful with level %s\n",
			map_num(pers, array.level)?:"of this array");
		return 1;
	}
	bitmapsize = array.size;
	bitmapsize <<= 1;
	if (get_dev_size(fd, NULL, &array_size) &&
	    array_size > (0x7fffffffULL << 9)) {
		/* Array is big enough that we cannot trust array.size
		 * try other approaches
		 */
		bitmapsize = get_component_size(fd);
	}
	if (bitmapsize == 0) {
		pr_err("Cannot reliably determine size of array to create bitmap - sorry.\n");
		return 1;
	}

	if (array.level == 10) {
		int ncopies;

		ncopies = (array.layout & 255) * ((array.layout >> 8) & 255);
		bitmapsize = bitmapsize * array.raid_disks / ncopies;

		if (strcmp(s->bitmap_file, "clustered") == 0 &&
		    !is_near_layout_10(array.layout)) {
			pr_err("only near layout is supported with clustered raid10\n");
			return 1;
		}
	}

	st = super_by_fd(fd, &subarray);
	if (!st) {
		pr_err("Cannot understand version %d.%d\n",
			array.major_version, array.minor_version);
		return 1;
	}
	if (subarray) {
		pr_err("Cannot add bitmaps to sub-arrays yet\n");
		free(subarray);
		free(st);
		return 1;
	}

	mdi = sysfs_read(fd, NULL, GET_CONSISTENCY_POLICY);
	if (mdi) {
		if (mdi->consistency_policy == CONSISTENCY_POLICY_PPL) {
			pr_err("Cannot add bitmap to array with PPL\n");
			free(mdi);
			free(st);
			return 1;
		}
		free(mdi);
	}

	if (strcmp(s->bitmap_file, "internal") == 0 ||
	    strcmp(s->bitmap_file, "clustered") == 0) {
		int rv;
		int d;
		int offset_setable = 0;
		if (st->ss->add_internal_bitmap == NULL) {
			pr_err("Internal bitmaps not supported with %s metadata\n", st->ss->name);
			return 1;
		}
		st->nodes = c->nodes;
		st->cluster_name = c->homecluster;
		mdi = sysfs_read(fd, NULL, GET_BITMAP_LOCATION);
		if (mdi)
			offset_setable = 1;
		for (d = 0; d < st->max_devs; d++) {
			mdu_disk_info_t disk;
			char *dv;
			int fd2;

			disk.number = d;
			if (md_get_disk_info(fd, &disk) < 0)
				continue;
			if (disk.major == 0 && disk.minor == 0)
				continue;
			if ((disk.state & (1 << MD_DISK_SYNC)) == 0)
				continue;
			dv = map_dev(disk.major, disk.minor, 1);
			if (!dv)
				continue;
			fd2 = dev_open(dv, O_RDWR);
			if (fd2 < 0)
				continue;
			rv = st->ss->load_super(st, fd2, NULL);
			if (!rv) {
				rv = st->ss->add_internal_bitmap(
					st, &s->bitmap_chunk, c->delay,
					s->write_behind, bitmapsize,
					offset_setable, major);
				if (!rv) {
					st->ss->write_bitmap(st, fd2,
							     NodeNumUpdate);
				} else {
					pr_err("failed to create internal bitmap - chunksize problem.\n");
				}
			} else {
				pr_err("failed to load super-block.\n");
			}
			close(fd2);
			if (rv)
				return 1;
		}
		if (offset_setable) {
			st->ss->getinfo_super(st, mdi, NULL);
			if (sysfs_init(mdi, fd, NULL)) {
				pr_err("failed to intialize sysfs.\n");
				free(mdi);
			}
			rv = sysfs_set_num_signed(mdi, NULL, "bitmap/location",
						  mdi->bitmap_offset);
			free(mdi);
		} else {
			if (strcmp(s->bitmap_file, "clustered") == 0)
				array.state |= (1 << MD_SB_CLUSTERED);
			array.state |= (1 << MD_SB_BITMAP_PRESENT);
			rv = md_set_array_info(fd, &array);
		}
		if (rv < 0) {
			if (errno == EBUSY)
				pr_err("Cannot add bitmap while array is resyncing or reshaping etc.\n");
			pr_err("failed to set internal bitmap.\n");
			return 1;
		}
	} else {
		int uuid[4];
		int bitmap_fd;
		int d;
		int max_devs = st->max_devs;

		/* try to load a superblock */
		for (d = 0; d < max_devs; d++) {
			mdu_disk_info_t disk;
			char *dv;
			int fd2;
			disk.number = d;
			if (md_get_disk_info(fd, &disk) < 0)
				continue;
			if ((disk.major==0 && disk.minor == 0) ||
			    (disk.state & (1 << MD_DISK_REMOVED)))
				continue;
			dv = map_dev(disk.major, disk.minor, 1);
			if (!dv)
				continue;
			fd2 = dev_open(dv, O_RDONLY);
			if (fd2 >= 0) {
				if (st->ss->load_super(st, fd2, NULL) == 0) {
					close(fd2);
					st->ss->uuid_from_super(st, uuid);
					break;
				}
				close(fd2);
			}
		}
		if (d == max_devs) {
			pr_err("cannot find UUID for array!\n");
			return 1;
		}
		if (CreateBitmap(s->bitmap_file, c->force, (char*)uuid,
				 s->bitmap_chunk, c->delay, s->write_behind,
				 bitmapsize, major)) {
			return 1;
		}
		bitmap_fd = open(s->bitmap_file, O_RDWR);
		if (bitmap_fd < 0) {
			pr_err("weird: %s cannot be opened\n", s->bitmap_file);
			return 1;
		}
		if (ioctl(fd, SET_BITMAP_FILE, bitmap_fd) < 0) {
			int err = errno;
			if (errno == EBUSY)
				pr_err("Cannot add bitmap while array is resyncing or reshaping etc.\n");
			pr_err("Cannot set bitmap file for %s: %s\n",
				devname, strerror(err));
			return 1;
		}
	}

	return 0;
}

int Grow_consistency_policy(char *devname, int fd, struct context *c, struct shape *s)
{
	struct supertype *st;
	struct mdinfo *sra;
	struct mdinfo *sd;
	char *subarray = NULL;
	int ret = 0;
	char container_dev[PATH_MAX];
	char buf[20];

	if (s->consistency_policy != CONSISTENCY_POLICY_RESYNC &&
	    s->consistency_policy != CONSISTENCY_POLICY_PPL) {
		pr_err("Operation not supported for consistency policy %s\n",
		       map_num(consistency_policies, s->consistency_policy));
		return 1;
	}

	st = super_by_fd(fd, &subarray);
	if (!st)
		return 1;

	sra = sysfs_read(fd, NULL, GET_CONSISTENCY_POLICY|GET_LEVEL|
				   GET_DEVS|GET_STATE);
	if (!sra) {
		ret = 1;
		goto free_st;
	}

	if (s->consistency_policy == CONSISTENCY_POLICY_PPL &&
	    !st->ss->write_init_ppl) {
		pr_err("%s metadata does not support PPL\n", st->ss->name);
		ret = 1;
		goto free_info;
	}

	if (sra->array.level != 5) {
		pr_err("Operation not supported for array level %d\n",
				sra->array.level);
		ret = 1;
		goto free_info;
	}

	if (sra->consistency_policy == (unsigned)s->consistency_policy) {
		pr_err("Consistency policy is already %s\n",
		       map_num(consistency_policies, s->consistency_policy));
		ret = 1;
		goto free_info;
	} else if (sra->consistency_policy != CONSISTENCY_POLICY_RESYNC &&
		   sra->consistency_policy != CONSISTENCY_POLICY_PPL) {
		pr_err("Current consistency policy is %s, cannot change to %s\n",
		       map_num(consistency_policies, sra->consistency_policy),
		       map_num(consistency_policies, s->consistency_policy));
		ret = 1;
		goto free_info;
	}

	if (s->consistency_policy == CONSISTENCY_POLICY_PPL) {
		if (sysfs_get_str(sra, NULL, "sync_action", buf, 20) <= 0) {
			ret = 1;
			goto free_info;
		} else if (strcmp(buf, "reshape\n") == 0) {
			pr_err("PPL cannot be enabled when reshape is in progress\n");
			ret = 1;
			goto free_info;
		}
	}

	if (subarray) {
		char *update;

		if (s->consistency_policy == CONSISTENCY_POLICY_PPL)
			update = "ppl";
		else
			update = "no-ppl";

		sprintf(container_dev, "/dev/%s", st->container_devnm);

		ret = Update_subarray(container_dev, subarray, update, NULL,
				      c->verbose);
		if (ret)
			goto free_info;
	}

	if (s->consistency_policy == CONSISTENCY_POLICY_PPL) {
		struct mdinfo info;

		if (subarray) {
			struct mdinfo *mdi;
			int cfd;

			cfd = open(container_dev, O_RDWR|O_EXCL);
			if (cfd < 0) {
				pr_err("Failed to open %s\n", container_dev);
				ret = 1;
				goto free_info;
			}

			ret = st->ss->load_container(st, cfd, st->container_devnm);
			close(cfd);

			if (ret) {
				pr_err("Cannot read superblock for %s\n",
				       container_dev);
				goto free_info;
			}

			mdi = st->ss->container_content(st, subarray);
			info = *mdi;
			free(mdi);
		}

		for (sd = sra->devs; sd; sd = sd->next) {
			int dfd;
			char *devpath;

			devpath = map_dev(sd->disk.major, sd->disk.minor, 0);
			dfd = dev_open(devpath, O_RDWR);
			if (dfd < 0) {
				pr_err("Failed to open %s\n", devpath);
				ret = 1;
				goto free_info;
			}

			if (!subarray) {
				ret = st->ss->load_super(st, dfd, NULL);
				if (ret) {
					pr_err("Failed to load super-block.\n");
					close(dfd);
					goto free_info;
				}

				ret = st->ss->update_super(st, sra, "ppl",
							   devname,
							   c->verbose, 0, NULL);
				if (ret) {
					close(dfd);
					st->ss->free_super(st);
					goto free_info;
				}
				st->ss->getinfo_super(st, &info, NULL);
			}

			ret |= sysfs_set_num(sra, sd, "ppl_sector",
					     info.ppl_sector);
			ret |= sysfs_set_num(sra, sd, "ppl_size",
					     info.ppl_size);

			if (ret) {
				pr_err("Failed to set PPL attributes for %s\n",
				       sd->sys_name);
				close(dfd);
				st->ss->free_super(st);
				goto free_info;
			}

			ret = st->ss->write_init_ppl(st, &info, dfd);
			if (ret)
				pr_err("Failed to write PPL\n");

			close(dfd);

			if (!subarray)
				st->ss->free_super(st);

			if (ret)
				goto free_info;
		}
	}

	ret = sysfs_set_str(sra, NULL, "consistency_policy",
			    map_num(consistency_policies,
				    s->consistency_policy));
	if (ret)
		pr_err("Failed to change array consistency policy\n");

free_info:
	sysfs_free(sra);
free_st:
	free(st);
	free(subarray);

	return ret;
}

/*
 * When reshaping an array we might need to backup some data.
 * This is written to all spares with a 'super_block' describing it.
 * The superblock goes 4K from the end of the used space on the
 * device.
 * It if written after the backup is complete.
 * It has the following structure.
 */

static struct mdp_backup_super {
	char	magic[16];  /* md_backup_data-1 or -2 */
	__u8	set_uuid[16];
	__u64	mtime;
	/* start/sizes in 512byte sectors */
	__u64	devstart;	/* address on backup device/file of data */
	__u64	arraystart;
	__u64	length;
	__u32	sb_csum;	/* csum of preceeding bytes. */
	__u32   pad1;
	__u64	devstart2;	/* offset in to data of second section */
	__u64	arraystart2;
	__u64	length2;
	__u32	sb_csum2;	/* csum of preceeding bytes. */
	__u8 pad[512-68-32];
} __attribute__((aligned(512))) bsb, bsb2;

static __u32 bsb_csum(char *buf, int len)
{
	int i;
	int csum = 0;
	for (i = 0; i < len; i++)
		csum = (csum<<3) + buf[0];
	return __cpu_to_le32(csum);
}

static int check_idle(struct supertype *st)
{
	/* Check that all member arrays for this container, or the
	 * container of this array, are idle
	 */
	char *container = (st->container_devnm[0]
			   ? st->container_devnm : st->devnm);
	struct mdstat_ent *ent, *e;
	int is_idle = 1;

	ent = mdstat_read(0, 0);
	for (e = ent ; e; e = e->next) {
		if (!is_container_member(e, container))
			continue;
		/* frozen array is not idle*/
		if (e->percent >= 0 || e->metadata_version[9] == '-') {
			is_idle = 0;
			break;
		}
	}
	free_mdstat(ent);
	return is_idle;
}

static int freeze_container(struct supertype *st)
{
	char *container = (st->container_devnm[0]
			   ? st->container_devnm : st->devnm);

	if (!check_idle(st))
		return -1;

	if (block_monitor(container, 1)) {
		pr_err("failed to freeze container\n");
		return -2;
	}

	return 1;
}

static void unfreeze_container(struct supertype *st)
{
	char *container = (st->container_devnm[0]
			   ? st->container_devnm : st->devnm);

	unblock_monitor(container, 1);
}

static int freeze(struct supertype *st)
{
	/* Try to freeze resync/rebuild on this array/container.
	 * Return -1 if the array is busy,
	 * return -2 container cannot be frozen,
	 * return 0 if this kernel doesn't support 'frozen'
	 * return 1 if it worked.
	 */
	if (st->ss->external)
		return freeze_container(st);
	else {
		struct mdinfo *sra = sysfs_read(-1, st->devnm, GET_VERSION);
		int err;
		char buf[20];

		if (!sra)
			return -1;
		/* Need to clear any 'read-auto' status */
		if (sysfs_get_str(sra, NULL, "array_state", buf, 20) > 0 &&
		    strncmp(buf, "read-auto", 9) == 0)
			sysfs_set_str(sra, NULL, "array_state", "clean");

		err = sysfs_freeze_array(sra);
		sysfs_free(sra);
		return err;
	}
}

static void unfreeze(struct supertype *st)
{
	if (st->ss->external)
		return unfreeze_container(st);
	else {
		struct mdinfo *sra = sysfs_read(-1, st->devnm, GET_VERSION);
		char buf[20];

		if (sra &&
		    sysfs_get_str(sra, NULL, "sync_action", buf, 20) > 0 &&
		    strcmp(buf, "frozen\n") == 0)
			sysfs_set_str(sra, NULL, "sync_action", "idle");
		sysfs_free(sra);
	}
}

static void wait_reshape(struct mdinfo *sra)
{
	int fd = sysfs_get_fd(sra, NULL, "sync_action");
	char action[20];

	if (fd < 0)
		return;

	while (sysfs_fd_get_str(fd, action, 20) > 0 &&
	       strncmp(action, "reshape", 7) == 0)
		sysfs_wait(fd, NULL);
	close(fd);
}

static int reshape_super(struct supertype *st, unsigned long long size,
			 int level, int layout, int chunksize, int raid_disks,
			 int delta_disks, char *backup_file, char *dev,
			 int direction, int verbose)
{
	/* nothing extra to check in the native case */
	if (!st->ss->external)
		return 0;
	if (!st->ss->reshape_super || !st->ss->manage_reshape) {
		pr_err("%s metadata does not support reshape\n",
			st->ss->name);
		return 1;
	}

	return st->ss->reshape_super(st, size, level, layout, chunksize,
				     raid_disks, delta_disks, backup_file, dev,
				     direction, verbose);
}

static void sync_metadata(struct supertype *st)
{
	if (st->ss->external) {
		if (st->update_tail) {
			flush_metadata_updates(st);
			st->update_tail = &st->updates;
		} else
			st->ss->sync_metadata(st);
	}
}

static int subarray_set_num(char *container, struct mdinfo *sra, char *name, int n)
{
	/* when dealing with external metadata subarrays we need to be
	 * prepared to handle EAGAIN.  The kernel may need to wait for
	 * mdmon to mark the array active so the kernel can handle
	 * allocations/writeback when preparing the reshape action
	 * (md_allow_write()).  We temporarily disable safe_mode_delay
	 * to close a race with the array_state going clean before the
	 * next write to raid_disks / stripe_cache_size
	 */
	char safe[50];
	int rc;

	/* only 'raid_disks' and 'stripe_cache_size' trigger md_allow_write */
	if (!container ||
	    (strcmp(name, "raid_disks") != 0 &&
	     strcmp(name, "stripe_cache_size") != 0))
		return sysfs_set_num(sra, NULL, name, n);

	rc = sysfs_get_str(sra, NULL, "safe_mode_delay", safe, sizeof(safe));
	if (rc <= 0)
		return -1;
	sysfs_set_num(sra, NULL, "safe_mode_delay", 0);
	rc = sysfs_set_num(sra, NULL, name, n);
	if (rc < 0 && errno == EAGAIN) {
		ping_monitor(container);
		/* if we get EAGAIN here then the monitor is not active
		 * so stop trying
		 */
		rc = sysfs_set_num(sra, NULL, name, n);
	}
	sysfs_set_str(sra, NULL, "safe_mode_delay", safe);
	return rc;
}

int start_reshape(struct mdinfo *sra, int already_running,
		  int before_data_disks, int data_disks)
{
	int err;
	unsigned long long sync_max_to_set;

	sysfs_set_num(sra, NULL, "suspend_lo", 0x7FFFFFFFFFFFFFFFULL);
	err = sysfs_set_num(sra, NULL, "suspend_hi", sra->reshape_progress);
	err = err ?: sysfs_set_num(sra, NULL, "suspend_lo",
				   sra->reshape_progress);
	if (before_data_disks <= data_disks)
		sync_max_to_set = sra->reshape_progress / data_disks;
	else
		sync_max_to_set = (sra->component_size * data_disks
				   - sra->reshape_progress) / data_disks;
	if (!already_running)
		sysfs_set_num(sra, NULL, "sync_min", sync_max_to_set);
	err = err ?: sysfs_set_num(sra, NULL, "sync_max", sync_max_to_set);
	if (!already_running && err == 0) {
		int cnt = 5;
		do {
			err = sysfs_set_str(sra, NULL, "sync_action",
					    "reshape");
			if (err)
				sleep(1);
		} while (err && errno == EBUSY && cnt-- > 0);
	}
	return err;
}

void abort_reshape(struct mdinfo *sra)
{
	sysfs_set_str(sra, NULL, "sync_action", "idle");
	/*
	 * Prior to kernel commit: 23ddff3792f6 ("md: allow suspend_lo and
	 * suspend_hi to decrease as well as increase.")
	 * you could only increase suspend_{lo,hi} unless the region they
	 * covered was empty.  So to reset to 0, you need to push suspend_lo
	 * up past suspend_hi first.  So to maximize the chance of mdadm
	 * working on all kernels, we want to keep doing that.
	 */
	sysfs_set_num(sra, NULL, "suspend_lo", 0x7FFFFFFFFFFFFFFFULL);
	sysfs_set_num(sra, NULL, "suspend_hi", 0);
	sysfs_set_num(sra, NULL, "suspend_lo", 0);
	sysfs_set_num(sra, NULL, "sync_min", 0);
	// It isn't safe to reset sync_max as we aren't monitoring.
	// Array really should be stopped at this point.
}

int remove_disks_for_takeover(struct supertype *st,
			      struct mdinfo *sra,
			      int layout)
{
	int nr_of_copies;
	struct mdinfo *remaining;
	int slot;

	if (st->ss->external) {
		int rv = 0;
		struct mdinfo *arrays = st->ss->container_content(st, NULL);
		/*
		 * containter_content returns list of arrays in container
		 * If arrays->next is not NULL it means that there are
		 * 2 arrays in container and operation should be blocked
		 */
		if (arrays) {
			if (arrays->next)
				rv = 1;
			sysfs_free(arrays);
			if (rv) {
				pr_err("Error. Cannot perform operation on /dev/%s\n", st->devnm);
				pr_err("For this operation it MUST be single array in container\n");
				return rv;
			}
		}
	}

	if (sra->array.level == 10)
		nr_of_copies = layout & 0xff;
	else if (sra->array.level == 1)
		nr_of_copies = sra->array.raid_disks;
	else
		return 1;

	remaining = sra->devs;
	sra->devs = NULL;
	/* for each 'copy', select one device and remove from the list. */
	for (slot = 0; slot < sra->array.raid_disks; slot += nr_of_copies) {
		struct mdinfo **diskp;
		int found = 0;

		/* Find a working device to keep */
		for (diskp =  &remaining; *diskp ; diskp = &(*diskp)->next) {
			struct mdinfo *disk = *diskp;

			if (disk->disk.raid_disk < slot)
				continue;
			if (disk->disk.raid_disk >= slot + nr_of_copies)
				continue;
			if (disk->disk.state & (1<<MD_DISK_REMOVED))
				continue;
			if (disk->disk.state & (1<<MD_DISK_FAULTY))
				continue;
			if (!(disk->disk.state & (1<<MD_DISK_SYNC)))
				continue;

			/* We have found a good disk to use! */
			*diskp = disk->next;
			disk->next = sra->devs;
			sra->devs = disk;
			found = 1;
			break;
		}
		if (!found)
			break;
	}

	if (slot < sra->array.raid_disks) {
		/* didn't find all slots */
		struct mdinfo **e;
		e = &remaining;
		while (*e)
			e = &(*e)->next;
		*e = sra->devs;
		sra->devs = remaining;
		return 1;
	}

	/* Remove all 'remaining' devices from the array */
	while (remaining) {
		struct mdinfo *sd = remaining;
		remaining = sd->next;

		sysfs_set_str(sra, sd, "state", "faulty");
		sysfs_set_str(sra, sd, "slot", "none");
		/* for external metadata disks should be removed in mdmon */
		if (!st->ss->external)
			sysfs_set_str(sra, sd, "state", "remove");
		sd->disk.state |= (1<<MD_DISK_REMOVED);
		sd->disk.state &= ~(1<<MD_DISK_SYNC);
		sd->next = sra->devs;
		sra->devs = sd;
	}
	return 0;
}

void reshape_free_fdlist(int *fdlist,
			 unsigned long long *offsets,
			 int size)
{
	int i;

	for (i = 0; i < size; i++)
		if (fdlist[i] >= 0)
			close(fdlist[i]);

	free(fdlist);
	free(offsets);
}

int reshape_prepare_fdlist(char *devname,
			   struct mdinfo *sra,
			   int raid_disks,
			   int nrdisks,
			   unsigned long blocks,
			   char *backup_file,
			   int *fdlist,
			   unsigned long long *offsets)
{
	int d = 0;
	struct mdinfo *sd;

	enable_fds(nrdisks);
	for (d = 0; d <= nrdisks; d++)
		fdlist[d] = -1;
	d = raid_disks;
	for (sd = sra->devs; sd; sd = sd->next) {
		if (sd->disk.state & (1<<MD_DISK_FAULTY))
			continue;
		if (sd->disk.state & (1<<MD_DISK_SYNC) &&
		    sd->disk.raid_disk < raid_disks) {
			char *dn = map_dev(sd->disk.major, sd->disk.minor, 1);
			fdlist[sd->disk.raid_disk] = dev_open(dn, O_RDONLY);
			offsets[sd->disk.raid_disk] = sd->data_offset*512;
			if (fdlist[sd->disk.raid_disk] < 0) {
				pr_err("%s: cannot open component %s\n",
				       devname, dn ? dn : "-unknown-");
				d = -1;
				goto release;
			}
		} else if (backup_file == NULL) {
			/* spare */
			char *dn = map_dev(sd->disk.major, sd->disk.minor, 1);
				fdlist[d] = dev_open(dn, O_RDWR);
				offsets[d] = (sd->data_offset + sra->component_size - blocks - 8)*512;
				if (fdlist[d] < 0) {
					pr_err("%s: cannot open component %s\n",
						devname, dn ? dn : "-unknown-");
					d = -1;
					goto release;
				}
				d++;
			}
		}
release:
	return d;
}

int reshape_open_backup_file(char *backup_file,
			     int fd,
			     char *devname,
			     long blocks,
			     int *fdlist,
			     unsigned long long *offsets,
			     char *sys_name,
			     int restart)
{
	/* Return 1 on success, 0 on any form of failure */
	/* need to check backup file is large enough */
	char buf[512];
	struct stat stb;
	unsigned int dev;
	int i;

	*fdlist = open(backup_file, O_RDWR|O_CREAT|(restart ? O_TRUNC : O_EXCL),
		       S_IRUSR | S_IWUSR);
	*offsets = 8 * 512;
	if (*fdlist < 0) {
		pr_err("%s: cannot create backup file %s: %s\n",
			devname, backup_file, strerror(errno));
		return 0;
	}
	/* Guard against backup file being on array device.
	 * If array is partitioned or if LVM etc is in the
	 * way this will not notice, but it is better than
	 * nothing.
	 */
	fstat(*fdlist, &stb);
	dev = stb.st_dev;
	fstat(fd, &stb);
	if (stb.st_rdev == dev) {
		pr_err("backup file must NOT be on the array being reshaped.\n");
		close(*fdlist);
		return 0;
	}

	memset(buf, 0, 512);
	for (i=0; i < blocks + 8 ; i++) {
		if (write(*fdlist, buf, 512) != 512) {
			pr_err("%s: cannot create backup file %s: %s\n",
				devname, backup_file, strerror(errno));
			return 0;
		}
	}
	if (fsync(*fdlist) != 0) {
		pr_err("%s: cannot create backup file %s: %s\n",
			devname, backup_file, strerror(errno));
		return 0;
	}

	if (!restart && strncmp(backup_file, MAP_DIR, strlen(MAP_DIR)) != 0) {
		char *bu = make_backup(sys_name);
		if (symlink(backup_file, bu))
			pr_err("Recording backup file in " MAP_DIR " failed: %s\n",
			       strerror(errno));
		free(bu);
	}

	return 1;
}

unsigned long compute_backup_blocks(int nchunk, int ochunk,
				    unsigned int ndata, unsigned int odata)
{
	unsigned long a, b, blocks;
	/* So how much do we need to backup.
	 * We need an amount of data which is both a whole number of
	 * old stripes and a whole number of new stripes.
	 * So LCM for (chunksize*datadisks).
	 */
	a = (ochunk/512) * odata;
	b = (nchunk/512) * ndata;
	/* Find GCD */
	a = GCD(a, b);
	/* LCM == product / GCD */
	blocks = (ochunk/512) * (nchunk/512) * odata * ndata / a;

	return blocks;
}

char *analyse_change(char *devname, struct mdinfo *info, struct reshape *re)
{
	/* Based on the current array state in info->array and
	 * the changes in info->new_* etc, determine:
	 *  - whether the change is possible
	 *  - Intermediate level/raid_disks/layout
	 *  - whether a restriping reshape is needed
	 *  - number of sectors in minimum change unit.  This
	 *    will cover a whole number of stripes in 'before' and
	 *    'after'.
	 *
	 * Return message if the change should be rejected
	 *        NULL if the change can be achieved
	 *
	 * This can be called as part of starting a reshape, or
	 * when assembling an array that is undergoing reshape.
	 */
	int near, far, offset, copies;
	int new_disks;
	int old_chunk, new_chunk;
	/* delta_parity records change in number of devices
	 * caused by level change
	 */
	int delta_parity = 0;

	memset(re, 0, sizeof(*re));

	/* If a new level not explicitly given, we assume no-change */
	if (info->new_level == UnSet)
		info->new_level = info->array.level;

	if (info->new_chunk)
		switch (info->new_level) {
		case 0:
		case 4:
		case 5:
		case 6:
		case 10:
			/* chunk size is meaningful, must divide component_size
			 * evenly
			 */
			if (info->component_size % (info->new_chunk/512)) {
				unsigned long long shrink = info->component_size;
				shrink &= ~(unsigned long long)(info->new_chunk/512-1);
				pr_err("New chunk size (%dK) does not evenly divide device size (%lluk)\n",
				       info->new_chunk/1024, info->component_size/2);
				pr_err("After shrinking any filesystem, \"mdadm --grow %s --size %llu\"\n",
				       devname, shrink/2);
				pr_err("will shrink the array so the given chunk size would work.\n");
				return "";
			}
			break;
		default:
			return "chunk size not meaningful for this level";
		}
	else
		info->new_chunk = info->array.chunk_size;

	switch (info->array.level) {
	default:
		return "No reshape is possibly for this RAID level";
	case LEVEL_LINEAR:
		if (info->delta_disks != UnSet)
			return "Only --add is supported for LINEAR, setting --raid-disks is not needed";
		else
			return "Only --add is supported for LINEAR, other --grow options are not meaningful";
	case 1:
		/* RAID1 can convert to RAID1 with different disks, or
		 * raid5 with 2 disks, or
		 * raid0 with 1 disk
		 */
		if (info->new_level > 1 && (info->component_size & 7))
			return "Cannot convert RAID1 of this size - reduce size to multiple of 4K first.";
		if (info->new_level == 0) {
			if (info->delta_disks != UnSet &&
			    info->delta_disks != 0)
				return "Cannot change number of disks with RAID1->RAID0 conversion";
			re->level = 0;
			re->before.data_disks = 1;
			re->after.data_disks = 1;
			return NULL;
		}
		if (info->new_level == 1) {
			if (info->delta_disks == UnSet)
				/* Don't know what to do */
				return "no change requested for Growing RAID1";
			re->level = 1;
			return NULL;
		}
		if (info->array.raid_disks != 2 && info->new_level == 5)
			return "Can only convert a 2-device array to RAID5";
		if (info->array.raid_disks == 2 && info->new_level == 5) {
			re->level = 5;
			re->before.data_disks = 1;
			if (info->delta_disks != UnSet &&
			    info->delta_disks != 0)
				re->after.data_disks = 1 + info->delta_disks;
			else
				re->after.data_disks = 1;
			if (re->after.data_disks < 1)
				return "Number of disks too small for RAID5";

			re->before.layout = ALGORITHM_LEFT_SYMMETRIC;
			info->array.chunk_size = 65536;
			break;
		}
		/* Could do some multi-stage conversions, but leave that to
		 * later.
		 */
		return "Impossibly level change request for RAID1";

	case 10:
		/* RAID10 can be converted from near mode to
		 * RAID0 by removing some devices.
		 * It can also be reshaped if the kernel supports
		 * new_data_offset.
		 */
		switch (info->new_level) {
		case 0:
			if ((info->array.layout & ~0xff) != 0x100)
				return "Cannot Grow RAID10 with far/offset layout";
			/*
			 * number of devices must be multiple of
			 * number of copies
			 */
			if (info->array.raid_disks %
			    (info->array.layout & 0xff))
				return "RAID10 layout too complex for Grow operation";

			new_disks = (info->array.raid_disks /
				     (info->array.layout & 0xff));
			if (info->delta_disks == UnSet)
				info->delta_disks = (new_disks
						     - info->array.raid_disks);

			if (info->delta_disks !=
			    new_disks - info->array.raid_disks)
				return "New number of raid-devices impossible for RAID10";
			if (info->new_chunk &&
			    info->new_chunk != info->array.chunk_size)
				return "Cannot change chunk-size with RAID10 Grow";

			/* looks good */
			re->level = 0;
			re->before.data_disks = new_disks;
			re->after.data_disks = re->before.data_disks;
			return NULL;

		case 10:
			near = info->array.layout & 0xff;
			far = (info->array.layout >> 8) & 0xff;
			offset = info->array.layout & 0x10000;
			if (far > 1 && !offset)
				return "Cannot reshape RAID10 in far-mode";
			copies = near * far;

			old_chunk = info->array.chunk_size * far;

			if (info->new_layout == UnSet)
				info->new_layout = info->array.layout;
			else {
				near = info->new_layout & 0xff;
				far = (info->new_layout >> 8) & 0xff;
				offset = info->new_layout & 0x10000;
				if (far > 1 && !offset)
					return "Cannot reshape RAID10 to far-mode";
				if (near * far != copies)
					return "Cannot change number of copies when reshaping RAID10";
			}
			if (info->delta_disks == UnSet)
				info->delta_disks = 0;
			new_disks = (info->array.raid_disks +
				     info->delta_disks);

			new_chunk = info->new_chunk * far;

			re->level = 10;
			re->before.layout = info->array.layout;
			re->before.data_disks = info->array.raid_disks;
			re->after.layout = info->new_layout;
			re->after.data_disks = new_disks;
			/* For RAID10 we don't do backup but do allow reshape,
			 * so set backup_blocks to INVALID_SECTORS rather than
			 * zero.
			 * And there is no need to synchronise stripes on both
			 * 'old' and  'new'.  So the important
			 * number is the minimum data_offset difference
			 * which is the larger of (offset copies * chunk).
			 */
			re->backup_blocks = INVALID_SECTORS;
			re->min_offset_change = max(old_chunk, new_chunk) / 512;
			if (new_disks < re->before.data_disks &&
			    info->space_after < re->min_offset_change)
				/* Reduce component size by one chunk */
				re->new_size = (info->component_size -
						re->min_offset_change);
			else
				re->new_size = info->component_size;
			re->new_size = re->new_size * new_disks / copies;
			return NULL;

		default:
			return "RAID10 can only be changed to RAID0";
		}
	case 0:
		/* RAID0 can be converted to RAID10, or to RAID456 */
		if (info->new_level == 10) {
			if (info->new_layout == UnSet &&
			    info->delta_disks == UnSet) {
				/* Assume near=2 layout */
				info->new_layout = 0x102;
				info->delta_disks = info->array.raid_disks;
			}
			if (info->new_layout == UnSet) {
				int copies = 1 + (info->delta_disks
						  / info->array.raid_disks);
				if (info->array.raid_disks * (copies-1) !=
				    info->delta_disks)
					return "Impossible number of devices for RAID0->RAID10";
				info->new_layout = 0x100 + copies;
			}
			if (info->delta_disks == UnSet) {
				int copies = info->new_layout & 0xff;
				if (info->new_layout != 0x100 + copies)
					return "New layout impossible for RAID0->RAID10";;
				info->delta_disks = (copies - 1) *
					info->array.raid_disks;
			}
			if (info->new_chunk &&
			    info->new_chunk != info->array.chunk_size)
				return "Cannot change chunk-size with RAID0->RAID10";
			/* looks good */
			re->level = 10;
			re->before.data_disks = (info->array.raid_disks +
						 info->delta_disks);
			re->after.data_disks = re->before.data_disks;
			re->before.layout = info->new_layout;
			return NULL;
		}

		/* RAID0 can also covert to RAID0/4/5/6 by first converting to
		 * a raid4 style layout of the final level.
		 */
		switch (info->new_level) {
		case 4:
			delta_parity = 1;
		case 0:
			re->level = 4;
			re->before.layout = 0;
			break;
		case 5:
			delta_parity = 1;
			re->level = 5;
			re->before.layout = ALGORITHM_PARITY_N;
			if (info->new_layout == UnSet)
				info->new_layout = map_name(r5layout, "default");
			break;
		case 6:
			delta_parity = 2;
			re->level = 6;
			re->before.layout = ALGORITHM_PARITY_N;
			if (info->new_layout == UnSet)
				info->new_layout = map_name(r6layout, "default");
			break;
		default:
			return "Impossible level change requested";
		}
		re->before.data_disks = info->array.raid_disks;
		/* determining 'after' layout happens outside this 'switch' */
		break;

	case 4:
		info->array.layout = ALGORITHM_PARITY_N;
	case 5:
		switch (info->new_level) {
		case 0:
			delta_parity = -1;
		case 4:
			re->level = info->array.level;
			re->before.data_disks = info->array.raid_disks - 1;
			re->before.layout = info->array.layout;
			break;
		case 5:
			re->level = 5;
			re->before.data_disks = info->array.raid_disks - 1;
			re->before.layout = info->array.layout;
			break;
		case 6:
			delta_parity = 1;
			re->level = 6;
			re->before.data_disks = info->array.raid_disks - 1;
			switch (info->array.layout) {
			case ALGORITHM_LEFT_ASYMMETRIC:
				re->before.layout = ALGORITHM_LEFT_ASYMMETRIC_6;
				break;
			case ALGORITHM_RIGHT_ASYMMETRIC:
				re->before.layout = ALGORITHM_RIGHT_ASYMMETRIC_6;
				break;
			case ALGORITHM_LEFT_SYMMETRIC:
				re->before.layout = ALGORITHM_LEFT_SYMMETRIC_6;
				break;
			case ALGORITHM_RIGHT_SYMMETRIC:
				re->before.layout = ALGORITHM_RIGHT_SYMMETRIC_6;
				break;
			case ALGORITHM_PARITY_0:
				re->before.layout = ALGORITHM_PARITY_0_6;
				break;
			case ALGORITHM_PARITY_N:
				re->before.layout = ALGORITHM_PARITY_N_6;
				break;
			default:
				return "Cannot convert an array with this layout";
			}
			break;
		case 1:
			if (info->array.raid_disks != 2)
				return "Can only convert a 2-device array to RAID1";
			if (info->delta_disks != UnSet &&
			    info->delta_disks != 0)
				return "Cannot set raid_disk when converting RAID5->RAID1";
			re->level = 1;
			info->new_chunk = 0;
			return NULL;
		default:
			return "Impossible level change requested";
		}
		break;
	case 6:
		switch (info->new_level) {
		case 4:
		case 5:
			delta_parity = -1;
		case 6:
			re->level = 6;
			re->before.data_disks = info->array.raid_disks - 2;
			re->before.layout = info->array.layout;
			break;
		default:
			return "Impossible level change requested";
		}
		break;
	}

	/* If we reached here then it looks like a re-stripe is
	 * happening.  We have determined the intermediate level
	 * and initial raid_disks/layout and stored these in 're'.
	 *
	 * We need to deduce the final layout that can be atomically
	 * converted to the end state.
	 */
	switch (info->new_level) {
	case 0:
		/* We can only get to RAID0 from RAID4 or RAID5
		 * with appropriate layout and one extra device
		 */
		if (re->level != 4 && re->level != 5)
			return "Cannot covert to RAID0 from this level";

		switch (re->level) {
		case 4:
			re->before.layout = 0;
			re->after.layout = 0;
			break;
		case 5:
			re->after.layout = ALGORITHM_PARITY_N;
			break;
		}
		break;

	case 4:
		/* We can only get to RAID4 from RAID5 */
		if (re->level != 4 && re->level != 5)
			return "Cannot convert to RAID4 from this level";

		switch (re->level) {
		case 4:
			re->after.layout = 0;
			break;
		case 5:
			re->after.layout = ALGORITHM_PARITY_N;
			break;
		}
		break;

	case 5:
		/* We get to RAID5 from RAID5 or RAID6 */
		if (re->level != 5 && re->level != 6)
			return "Cannot convert to RAID5 from this level";

		switch (re->level) {
		case 5:
			if (info->new_layout == UnSet)
				re->after.layout = re->before.layout;
			else
				re->after.layout = info->new_layout;
			break;
		case 6:
			if (info->new_layout == UnSet)
				info->new_layout = re->before.layout;

			/* after.layout needs to be raid6 version of new_layout */
			if (info->new_layout == ALGORITHM_PARITY_N)
				re->after.layout = ALGORITHM_PARITY_N;
			else {
				char layout[40];
				char *ls = map_num(r5layout, info->new_layout);
				int l;
				if (ls) {
					/* Current RAID6 layout has a RAID5
					 * equivalent - good
					 */
					strcat(strcpy(layout, ls), "-6");
					l = map_name(r6layout, layout);
					if (l == UnSet)
						return "Cannot find RAID6 layout to convert to";
				} else {
					/* Current RAID6 has no equivalent.
					 * If it is already a '-6' layout we
					 * can leave it unchanged, else we must
					 * fail
					 */
					ls = map_num(r6layout,
						     info->new_layout);
					if (!ls ||
					    strcmp(ls+strlen(ls)-2, "-6") != 0)
						return "Please specify new layout";
					l = info->new_layout;
				}
				re->after.layout = l;
			}
		}
		break;

	case 6:
		/* We must already be at level 6 */
		if (re->level != 6)
			return "Impossible level change";
		if (info->new_layout == UnSet)
			re->after.layout = info->array.layout;
		else
			re->after.layout = info->new_layout;
		break;
	default:
		return "Impossible level change requested";
	}
	if (info->delta_disks == UnSet)
		info->delta_disks = delta_parity;

	re->after.data_disks =
		(re->before.data_disks + info->delta_disks - delta_parity);

	switch (re->level) {
	case 6:
		re->parity = 2;
		break;
	case 4:
	case 5:
		re->parity = 1;
		break;
	default:
		re->parity = 0;
		break;
	}
	/* So we have a restripe operation, we need to calculate the number
	 * of blocks per reshape operation.
	 */
	re->new_size = info->component_size * re->before.data_disks;
	if (info->new_chunk == 0)
		info->new_chunk = info->array.chunk_size;
	if (re->after.data_disks == re->before.data_disks &&
	    re->after.layout == re->before.layout &&
	    info->new_chunk == info->array.chunk_size) {
		/* Nothing to change, can change level immediately. */
		re->level = info->new_level;
		re->backup_blocks = 0;
		return NULL;
	}
	if (re->after.data_disks == 1 && re->before.data_disks == 1) {
		/* chunk and layout changes make no difference */
		re->level = info->new_level;
		re->backup_blocks = 0;
		return NULL;
	}

	if (re->after.data_disks == re->before.data_disks &&
	    get_linux_version() < 2006032)
		return "in-place reshape is not safe before 2.6.32 - sorry.";

	if (re->after.data_disks < re->before.data_disks &&
	    get_linux_version() < 2006030)
		return "reshape to fewer devices is not supported before 2.6.30 - sorry.";

	re->backup_blocks = compute_backup_blocks(
		info->new_chunk, info->array.chunk_size,
		re->after.data_disks, re->before.data_disks);
	re->min_offset_change = re->backup_blocks / re->before.data_disks;

	re->new_size = info->component_size * re->after.data_disks;
	return NULL;
}

static int set_array_size(struct supertype *st, struct mdinfo *sra,
			  char *text_version)
{
	struct mdinfo *info;
	char *subarray;
	int ret_val = -1;

	if ((st == NULL) || (sra == NULL))
		return ret_val;

	if (text_version == NULL)
		text_version = sra->text_version;
	subarray = strchr(text_version + 1, '/')+1;
	info = st->ss->container_content(st, subarray);
	if (info) {
		unsigned long long current_size = 0;
		unsigned long long new_size = info->custom_array_size/2;

		if (sysfs_get_ll(sra, NULL, "array_size", &current_size) == 0 &&
		    new_size > current_size) {
			if (sysfs_set_num(sra, NULL, "array_size", new_size)
					< 0)
				dprintf("Error: Cannot set array size");
			else {
				ret_val = 0;
				dprintf("Array size changed");
			}
			dprintf_cont(" from %llu to %llu.\n",
				     current_size, new_size);
		}
		sysfs_free(info);
	} else
		dprintf("Error: set_array_size(): info pointer in NULL\n");

	return ret_val;
}

static int reshape_array(char *container, int fd, char *devname,
			 struct supertype *st, struct mdinfo *info,
			 int force, struct mddev_dev *devlist,
			 unsigned long long data_offset,
			 char *backup_file, int verbose, int forked,
			 int restart, int freeze_reshape);
static int reshape_container(char *container, char *devname,
			     int mdfd,
			     struct supertype *st,
			     struct mdinfo *info,
			     int force,
			     char *backup_file, int verbose,
			     int forked, int restart, int freeze_reshape);

int Grow_reshape(char *devname, int fd,
		 struct mddev_dev *devlist,
		 unsigned long long data_offset,
		 struct context *c, struct shape *s)
{
	/* Make some changes in the shape of an array.
	 * The kernel must support the change.
	 *
	 * There are three different changes.  Each can trigger
	 * a resync or recovery so we freeze that until we have
	 * requested everything (if kernel supports freezing - 2.6.30).
	 * The steps are:
	 *  - change size (i.e. component_size)
	 *  - change level
	 *  - change layout/chunksize/ndisks
	 *
	 * The last can require a reshape.  It is different on different
	 * levels so we need to check the level before actioning it.
	 * Some times the level change needs to be requested after the
	 * reshape (e.g. raid6->raid5, raid5->raid0)
	 *
	 */
	struct mdu_array_info_s array;
	int rv = 0;
	struct supertype *st;
	char *subarray = NULL;

	int frozen;
	int changed = 0;
	char *container = NULL;
	int cfd = -1;

	struct mddev_dev *dv;
	int added_disks;

	struct mdinfo info;
	struct mdinfo *sra;

	if (md_get_array_info(fd, &array) < 0) {
		pr_err("%s is not an active md array - aborting\n",
			devname);
		return 1;
	}
	if (s->level != UnSet && s->chunk) {
		pr_err("Cannot change array level in the same operation as changing chunk size.\n");
		return 1;
	}

	if (data_offset != INVALID_SECTORS && array.level != 10 &&
	    (array.level < 4 || array.level > 6)) {
		pr_err("--grow --data-offset not yet supported\n");
		return 1;
	}

	if (s->size > 0 &&
	    (s->chunk || s->level!= UnSet || s->layout_str || s->raiddisks)) {
		pr_err("cannot change component size at the same time as other changes.\n"
			"   Change size first, then check data is intact before making other changes.\n");
		return 1;
	}

	if (s->raiddisks && s->raiddisks < array.raid_disks &&
	    array.level > 1 && get_linux_version() < 2006032 &&
	    !check_env("MDADM_FORCE_FEWER")) {
		pr_err("reducing the number of devices is not safe before Linux 2.6.32\n"
			"       Please use a newer kernel\n");
		return 1;
	}

	if (array.level > 1 && s->size > 1 &&
	    (unsigned long long) (array.chunk_size / 1024) > s->size) {
		pr_err("component size must be larger than chunk size.\n");
		return 1;
	}

	st = super_by_fd(fd, &subarray);
	if (!st) {
		pr_err("Unable to determine metadata format for %s\n", devname);
		return 1;
	}
	if (s->raiddisks > st->max_devs) {
		pr_err("Cannot increase raid-disks on this array beyond %d\n", st->max_devs);
		return 1;
	}
	if (s->level == 0 &&
	    (array.state & (1<<MD_SB_BITMAP_PRESENT)) &&
	    !(array.state & (1<<MD_SB_CLUSTERED))) {
                array.state &= ~(1<<MD_SB_BITMAP_PRESENT);
                if (md_set_array_info(fd, &array)!= 0) {
                        pr_err("failed to remove internal bitmap.\n");
                        return 1;
                }
        }

	/* in the external case we need to check that the requested reshape is
	 * supported, and perform an initial check that the container holds the
	 * pre-requisite spare devices (mdmon owns final validation)
	 */
	if (st->ss->external) {
		int retval;

		if (subarray) {
			container = st->container_devnm;
			cfd = open_dev_excl(st->container_devnm);
		} else {
			container = st->devnm;
			close(fd);
			cfd = open_dev_excl(st->devnm);
			fd = cfd;
		}
		if (cfd < 0) {
			pr_err("Unable to open container for %s\n", devname);
			free(subarray);
			return 1;
		}

		retval = st->ss->load_container(st, cfd, NULL);

		if (retval) {
			pr_err("Cannot read superblock for %s\n", devname);
			free(subarray);
			return 1;
		}

		/* check if operation is supported for metadata handler */
		if (st->ss->container_content) {
			struct mdinfo *cc = NULL;
			struct mdinfo *content = NULL;

			cc = st->ss->container_content(st, subarray);
			for (content = cc; content ; content = content->next) {
				int allow_reshape = 1;

				/* check if reshape is allowed based on metadata
				 * indications stored in content.array.status
				 */
				if (content->array.state &
				    (1 << MD_SB_BLOCK_VOLUME))
					allow_reshape = 0;
				if (content->array.state &
				    (1 << MD_SB_BLOCK_CONTAINER_RESHAPE))
					allow_reshape = 0;
				if (!allow_reshape) {
					pr_err("cannot reshape arrays in container with unsupported metadata: %s(%s)\n",
					       devname, container);
					sysfs_free(cc);
					free(subarray);
					return 1;
				}
				if (content->consistency_policy ==
				    CONSISTENCY_POLICY_PPL) {
					pr_err("Operation not supported when ppl consistency policy is enabled\n");
					sysfs_free(cc);
					free(subarray);
					return 1;
				}
			}
			sysfs_free(cc);
		}
		if (mdmon_running(container))
			st->update_tail = &st->updates;
	}

	added_disks = 0;
	for (dv = devlist; dv; dv = dv->next)
		added_disks++;
	if (s->raiddisks > array.raid_disks &&
	    array.spare_disks + added_disks <
	    (s->raiddisks - array.raid_disks) &&
	    !c->force) {
		pr_err("Need %d spare%s to avoid degraded array, and only have %d.\n"
		       "       Use --force to over-ride this check.\n",
		       s->raiddisks - array.raid_disks,
		       s->raiddisks - array.raid_disks == 1 ? "" : "s",
		       array.spare_disks + added_disks);
		return 1;
	}

	sra = sysfs_read(fd, NULL, GET_LEVEL | GET_DISKS | GET_DEVS |
			 GET_STATE | GET_VERSION);
	if (sra) {
		if (st->ss->external && subarray == NULL) {
			array.level = LEVEL_CONTAINER;
			sra->array.level = LEVEL_CONTAINER;
		}
	} else {
		pr_err("failed to read sysfs parameters for %s\n",
			devname);
		return 1;
	}
	frozen = freeze(st);
	if (frozen < -1) {
		/* freeze() already spewed the reason */
		sysfs_free(sra);
		return 1;
	} else if (frozen < 0) {
		pr_err("%s is performing resync/recovery and cannot be reshaped\n", devname);
		sysfs_free(sra);
		return 1;
	}

	/* ========= set size =============== */
	if (s->size > 0 &&
	    (s->size == MAX_SIZE || s->size != (unsigned)array.size)) {
		unsigned long long orig_size = get_component_size(fd)/2;
		unsigned long long min_csize;
		struct mdinfo *mdi;
		int raid0_takeover = 0;

		if (orig_size == 0)
			orig_size = (unsigned) array.size;

		if (orig_size == 0) {
			pr_err("Cannot set device size in this type of array.\n");
			rv = 1;
			goto release;
		}

		if (reshape_super(st, s->size, UnSet, UnSet, 0, 0, UnSet, NULL,
				  devname, APPLY_METADATA_CHANGES,
				  c->verbose > 0)) {
			rv = 1;
			goto release;
		}
		sync_metadata(st);
		if (st->ss->external) {
			/* metadata can have size limitation
			 * update size value according to metadata information
			 */
			struct mdinfo *sizeinfo =
				st->ss->container_content(st, subarray);
			if (sizeinfo) {
				unsigned long long new_size =
					sizeinfo->custom_array_size/2;
				int data_disks = get_data_disks(
						sizeinfo->array.level,
						sizeinfo->array.layout,
						sizeinfo->array.raid_disks);
				new_size /= data_disks;
				dprintf("Metadata size correction from %llu to %llu (%llu)\n",
					orig_size, new_size,
					new_size * data_disks);
				s->size = new_size;
				sysfs_free(sizeinfo);
			}
		}

		/* Update the size of each member device in case
		 * they have been resized.  This will never reduce
		 * below the current used-size.  The "size" attribute
		 * understands '0' to mean 'max'.
		 */
		min_csize = 0;
		for (mdi = sra->devs; mdi; mdi = mdi->next) {
			sysfs_set_num(sra, mdi, "size",
				      s->size == MAX_SIZE ? 0 : s->size);
			if (array.not_persistent == 0 &&
			    array.major_version == 0 &&
			    get_linux_version() < 3001000) {
				/* Dangerous to allow size to exceed 2TB */
				unsigned long long csize;
				if (sysfs_get_ll(sra, mdi, "size",
						 &csize) == 0) {
					if (csize >= 2ULL*1024*1024*1024)
						csize = 2ULL*1024*1024*1024;
					if ((min_csize == 0 ||
					     (min_csize > csize)))
						min_csize = csize;
				}
			}
		}
		if (min_csize && s->size > min_csize) {
			pr_err("Cannot safely make this array use more than 2TB per device on this kernel.\n");
			rv = 1;
			goto size_change_error;
		}
		if (min_csize && s->size == MAX_SIZE) {
			/* Don't let the kernel choose a size - it will get
			 * it wrong
			 */
			pr_err("Limited v0.90 array to 2TB per device\n");
			s->size = min_csize;
		}
		if (st->ss->external) {
			if (sra->array.level == 0) {
				rv = sysfs_set_str(sra, NULL, "level", "raid5");
				if (!rv) {
					raid0_takeover = 1;
					/* get array parameters after takeover
					 * to change one parameter at time only
					 */
					rv = md_get_array_info(fd, &array);
				}
			}
			/* make sure mdmon is
			 * aware of the new level */
			if (!mdmon_running(st->container_devnm))
				start_mdmon(st->container_devnm);
			ping_monitor(container);
			if (mdmon_running(st->container_devnm) &&
					st->update_tail == NULL)
				st->update_tail = &st->updates;
		}

		if (s->size == MAX_SIZE)
			s->size = 0;
		array.size = s->size;
		if (s->size & ~INT32_MAX) {
			/* got truncated to 32bit, write to
			 * component_size instead
			 */
			if (sra)
				rv = sysfs_set_num(sra, NULL,
						   "component_size", s->size);
			else
				rv = -1;
		} else {
			rv = md_set_array_info(fd, &array);

			/* manage array size when it is managed externally
			 */
			if ((rv == 0) && st->ss->external)
				rv = set_array_size(st, sra, sra->text_version);
		}

		if (raid0_takeover) {
			/* do not recync non-existing parity,
			 * we will drop it anyway
			 */
			sysfs_set_str(sra, NULL, "sync_action", "frozen");
			/* go back to raid0, drop parity disk
			 */
			sysfs_set_str(sra, NULL, "level", "raid0");
			md_get_array_info(fd, &array);
		}

size_change_error:
		if (rv != 0) {
			int err = errno;

			/* restore metadata */
			if (reshape_super(st, orig_size, UnSet, UnSet, 0, 0,
					  UnSet, NULL, devname,
					  ROLLBACK_METADATA_CHANGES,
					  c->verbose) == 0)
				sync_metadata(st);
			pr_err("Cannot set device size for %s: %s\n",
				devname, strerror(err));
			if (err == EBUSY &&
			    (array.state & (1<<MD_SB_BITMAP_PRESENT)))
				cont_err("Bitmap must be removed before size can be changed\n");
			rv = 1;
			goto release;
		}
		if (s->assume_clean) {
			/* This will fail on kernels older than 3.0 unless
			 * a backport has been arranged.
			 */
			if (sra == NULL ||
			    sysfs_set_str(sra, NULL, "resync_start",
					  "none") < 0)
				pr_err("--assume-clean not supported with --grow on this kernel\n");
		}
		md_get_array_info(fd, &array);
		s->size = get_component_size(fd)/2;
		if (s->size == 0)
			s->size = array.size;
		if (c->verbose >= 0) {
			if (s->size == orig_size)
				pr_err("component size of %s unchanged at %lluK\n",
					devname, s->size);
			else
				pr_err("component size of %s has been set to %lluK\n",
					devname, s->size);
		}
		changed = 1;
	} else if (array.level != LEVEL_CONTAINER) {
		s->size = get_component_size(fd)/2;
		if (s->size == 0)
			s->size = array.size;
	}

	/* See if there is anything else to do */
	if ((s->level == UnSet || s->level == array.level) &&
	    (s->layout_str == NULL) &&
	    (s->chunk == 0 || s->chunk == array.chunk_size) &&
	    data_offset == INVALID_SECTORS &&
	    (s->raiddisks == 0 || s->raiddisks == array.raid_disks)) {
		/* Nothing more to do */
		if (!changed && c->verbose >= 0)
			pr_err("%s: no change requested\n", devname);
		goto release;
	}

	/* ========= check for Raid10/Raid1 -> Raid0 conversion ===============
	 * current implementation assumes that following conditions must be met:
	 * - RAID10:
	 *	- far_copies == 1
	 *	- near_copies == 2
	 */
	if ((s->level == 0 && array.level == 10 && sra &&
	     array.layout == ((1 << 8) + 2) && !(array.raid_disks & 1)) ||
	    (s->level == 0 && array.level == 1 && sra)) {
		int err;

		err = remove_disks_for_takeover(st, sra, array.layout);
		if (err) {
			dprintf("Array cannot be reshaped\n");
			if (cfd > -1)
				close(cfd);
			rv = 1;
			goto release;
		}
		/* Make sure mdmon has seen the device removal
		 * and updated metadata before we continue with
		 * level change
		 */
		if (container)
			ping_monitor(container);
	}

	memset(&info, 0, sizeof(info));
	info.array = array;
	if (sysfs_init(&info, fd, NULL)) {
		pr_err("failed to intialize sysfs.\n");
		rv = 1;
		goto release;
	}
	strcpy(info.text_version, sra->text_version);
	info.component_size = s->size*2;
	info.new_level = s->level;
	info.new_chunk = s->chunk * 1024;
	if (info.array.level == LEVEL_CONTAINER) {
		info.delta_disks = UnSet;
		info.array.raid_disks = s->raiddisks;
	} else if (s->raiddisks)
		info.delta_disks = s->raiddisks - info.array.raid_disks;
	else
		info.delta_disks = UnSet;
	if (s->layout_str == NULL) {
		info.new_layout = UnSet;
		if (info.array.level == 6 &&
		    (info.new_level == 6 || info.new_level == UnSet) &&
		    info.array.layout >= 16) {
			pr_err("%s has a non-standard layout.  If you wish to preserve this\n", devname);
			cont_err("during the reshape, please specify --layout=preserve\n");
			cont_err("If you want to change it, specify a layout or use --layout=normalise\n");
			rv = 1;
			goto release;
		}
	} else if (strcmp(s->layout_str, "normalise") == 0 ||
		   strcmp(s->layout_str, "normalize") == 0) {
		/* If we have a -6 RAID6 layout, remove the '-6'. */
		info.new_layout = UnSet;
		if (info.array.level == 6 && info.new_level == UnSet) {
			char l[40], *h;
			strcpy(l, map_num(r6layout, info.array.layout));
			h = strrchr(l, '-');
			if (h && strcmp(h, "-6") == 0) {
				*h = 0;
				info.new_layout = map_name(r6layout, l);
			}
		} else {
			pr_err("%s is only meaningful when reshaping a RAID6 array.\n", s->layout_str);
			rv = 1;
			goto release;
		}
	} else if (strcmp(s->layout_str, "preserve") == 0) {
		/* This means that a non-standard RAID6 layout
		 * is OK.
		 * In particular:
		 * - When reshape a RAID6 (e.g. adding a device)
		 *   which is in a non-standard layout, it is OK
		 *   to preserve that layout.
		 * - When converting a RAID5 to RAID6, leave it in
		 *   the XXX-6 layout, don't re-layout.
		 */
		if (info.array.level == 6 && info.new_level == UnSet)
			info.new_layout = info.array.layout;
		else if (info.array.level == 5 && info.new_level == 6) {
			char l[40];
			strcpy(l, map_num(r5layout, info.array.layout));
			strcat(l, "-6");
			info.new_layout = map_name(r6layout, l);
		} else {
			pr_err("%s in only meaningful when reshaping to RAID6\n", s->layout_str);
			rv = 1;
			goto release;
		}
	} else {
		int l = info.new_level;
		if (l == UnSet)
			l = info.array.level;
		switch (l) {
		case 5:
			info.new_layout = map_name(r5layout, s->layout_str);
			break;
		case 6:
			info.new_layout = map_name(r6layout, s->layout_str);
			break;
		case 10:
			info.new_layout = parse_layout_10(s->layout_str);
			break;
		case LEVEL_FAULTY:
			info.new_layout = parse_layout_faulty(s->layout_str);
			break;
		default:
			pr_err("layout not meaningful with this level\n");
			rv = 1;
			goto release;
		}
		if (info.new_layout == UnSet) {
			pr_err("layout %s not understood for this level\n",
				s->layout_str);
			rv = 1;
			goto release;
		}
	}

	if (array.level == LEVEL_FAULTY) {
		if (s->level != UnSet && s->level != array.level) {
			pr_err("cannot change level of Faulty device\n");
			rv =1 ;
		}
		if (s->chunk) {
			pr_err("cannot set chunksize of Faulty device\n");
			rv =1 ;
		}
		if (s->raiddisks && s->raiddisks != 1) {
			pr_err("cannot set raid_disks of Faulty device\n");
			rv =1 ;
		}
		if (s->layout_str) {
			if (md_get_array_info(fd, &array) != 0) {
				dprintf("Cannot get array information.\n");
				goto release;
			}
			array.layout = info.new_layout;
			if (md_set_array_info(fd, &array) != 0) {
				pr_err("failed to set new layout\n");
				rv = 1;
			} else if (c->verbose >= 0)
				printf("layout for %s set to %d\n",
				       devname, array.layout);
		}
	} else if (array.level == LEVEL_CONTAINER) {
		/* This change is to be applied to every array in the
		 * container.  This is only needed when the metadata imposes
		 * restraints of the various arrays in the container.
		 * Currently we only know that IMSM requires all arrays
		 * to have the same number of devices so changing the
		 * number of devices (On-Line Capacity Expansion) must be
		 * performed at the level of the container
		 */
		if (fd > 0) {
			close(fd);
			fd = -1;
		}
		rv = reshape_container(container, devname, -1, st, &info,
				       c->force, c->backup_file, c->verbose,
				       0, 0, 0);
		frozen = 0;
	} else {
		/* get spare devices from external metadata
		 */
		if (st->ss->external) {
			struct mdinfo *info2;

			info2 = st->ss->container_content(st, subarray);
			if (info2) {
				info.array.spare_disks =
					info2->array.spare_disks;
				sysfs_free(info2);
			}
		}

		/* Impose these changes on a single array.  First
		 * check that the metadata is OK with the change. */

		if (reshape_super(st, 0, info.new_level,
				  info.new_layout, info.new_chunk,
				  info.array.raid_disks, info.delta_disks,
				  c->backup_file, devname,
				  APPLY_METADATA_CHANGES, c->verbose)) {
			rv = 1;
			goto release;
		}
		sync_metadata(st);
		rv = reshape_array(container, fd, devname, st, &info, c->force,
				   devlist, data_offset, c->backup_file,
				   c->verbose, 0, 0, 0);
		frozen = 0;
	}
release:
	sysfs_free(sra);
	if (frozen > 0)
		unfreeze(st);
	return rv;
}

/* verify_reshape_position()
 *	Function checks if reshape position in metadata is not farther
 *	than position in md.
 * Return value:
 *	 0 : not valid sysfs entry
 *		it can be caused by not started reshape, it should be started
 *		by reshape array or raid0 array is before takeover
 *	-1 :	error, reshape position is obviously wrong
 *	 1 :	success, reshape progress correct or updated
*/
static int verify_reshape_position(struct mdinfo *info, int level)
{
	int ret_val = 0;
	char buf[40];
	int rv;

	/* read sync_max, failure can mean raid0 array */
	rv = sysfs_get_str(info, NULL, "sync_max", buf, 40);

	if (rv > 0) {
		char *ep;
		unsigned long long position = strtoull(buf, &ep, 0);

		dprintf("Read sync_max sysfs entry is: %s\n", buf);
		if (!(ep == buf || (*ep != 0 && *ep != '\n' && *ep != ' '))) {
			position *= get_data_disks(level,
						   info->new_layout,
						   info->array.raid_disks);
			if (info->reshape_progress < position) {
				dprintf("Corrected reshape progress (%llu) to md position (%llu)\n",
					info->reshape_progress, position);
				info->reshape_progress = position;
				ret_val = 1;
			} else if (info->reshape_progress > position) {
				pr_err("Fatal error: array reshape was not properly frozen (expected reshape position is %llu, but reshape progress is %llu.\n",
				       position, info->reshape_progress);
				ret_val = -1;
			} else {
				dprintf("Reshape position in md and metadata are the same;");
				ret_val = 1;
			}
		}
	} else if (rv == 0) {
		/* for valid sysfs entry, 0-length content
		 * should be indicated as error
		 */
		ret_val = -1;
	}

	return ret_val;
}

static unsigned long long choose_offset(unsigned long long lo,
					unsigned long long hi,
					unsigned long long min,
					unsigned long long max)
{
	/* Choose a new offset between hi and lo.
	 * It must be between min and max, but
	 * we would prefer something near the middle of hi/lo, and also
	 * prefer to be aligned to a big power of 2.
	 *
	 * So we start with the middle, then for each bit,
	 * starting at '1' and increasing, if it is set, we either
	 * add it or subtract it if possible, preferring the option
	 * which is furthest from the boundary.
	 *
	 * We stop once we get a 1MB alignment. As units are in sectors,
	 * 1MB = 2*1024 sectors.
	 */
	unsigned long long choice = (lo + hi) / 2;
	unsigned long long bit = 1;

	for (bit = 1; bit < 2*1024; bit = bit << 1) {
		unsigned long long bigger, smaller;
		if (! (bit & choice))
			continue;
		bigger = choice + bit;
		smaller = choice - bit;
		if (bigger > max && smaller < min)
			break;
		if (bigger > max)
			choice = smaller;
		else if (smaller < min)
			choice = bigger;
		else if (hi - bigger > smaller - lo)
			choice = bigger;
		else
			choice = smaller;
	}
	return choice;
}

static int set_new_data_offset(struct mdinfo *sra, struct supertype *st,
			       char *devname, int delta_disks,
			       unsigned long long data_offset,
			       unsigned long long min,
			       int can_fallback)
{
	struct mdinfo *sd;
	int dir = 0;
	int err = 0;
	unsigned long long before, after;

	/* Need to find min space before and after so same is used
	 * on all devices
	 */
	before = UINT64_MAX;
	after = UINT64_MAX;
	for (sd = sra->devs; sd; sd = sd->next) {
		char *dn;
		int dfd;
		int rv;
		struct supertype *st2;
		struct mdinfo info2;

		if (sd->disk.state & (1<<MD_DISK_FAULTY))
			continue;
		dn = map_dev(sd->disk.major, sd->disk.minor, 0);
		dfd = dev_open(dn, O_RDONLY);
		if (dfd < 0) {
			pr_err("%s: cannot open component %s\n",
				devname, dn ? dn : "-unknown-");
			goto release;
		}
		st2 = dup_super(st);
		rv = st2->ss->load_super(st2,dfd, NULL);
		close(dfd);
		if (rv) {
			free(st2);
			pr_err("%s: cannot get superblock from %s\n",
				devname, dn);
			goto release;
		}
		st2->ss->getinfo_super(st2, &info2, NULL);
		st2->ss->free_super(st2);
		free(st2);
		if (info2.space_before == 0 &&
		    info2.space_after == 0) {
			/* Metadata doesn't support data_offset changes */
			if (!can_fallback)
				pr_err("%s: Metadata version doesn't support data_offset changes\n",
				       devname);
			goto fallback;
		}
		if (before > info2.space_before)
			before = info2.space_before;
		if (after > info2.space_after)
			after = info2.space_after;

		if (data_offset != INVALID_SECTORS) {
			if (dir == 0) {
				if (info2.data_offset == data_offset) {
					pr_err("%s: already has that data_offset\n",
					       dn);
					goto release;
				}
				if (data_offset < info2.data_offset)
					dir = -1;
				else
					dir = 1;
			} else if ((data_offset <= info2.data_offset &&
				    dir == 1) ||
				   (data_offset >= info2.data_offset &&
				    dir == -1)) {
				pr_err("%s: differing data offsets on devices make this --data-offset setting impossible\n",
					dn);
				goto release;
			}
		}
	}
	if (before == UINT64_MAX)
		/* impossible really, there must be no devices */
		return 1;

	for (sd = sra->devs; sd; sd = sd->next) {
		char *dn = map_dev(sd->disk.major, sd->disk.minor, 0);
		unsigned long long new_data_offset;

		if (sd->disk.state & (1<<MD_DISK_FAULTY))
			continue;
		if (delta_disks < 0) {
			/* Don't need any space as array is shrinking
			 * just move data_offset up by min
			 */
			if (data_offset == INVALID_SECTORS)
				new_data_offset = sd->data_offset + min;
			else {
				if (data_offset < sd->data_offset + min) {
					pr_err("--data-offset too small for %s\n",
						dn);
					goto release;
				}
				new_data_offset = data_offset;
			}
		} else if (delta_disks > 0) {
			/* need space before */
			if (before < min) {
				if (can_fallback)
					goto fallback;
				pr_err("Insufficient head-space for reshape on %s\n",
					dn);
				goto release;
			}
			if (data_offset == INVALID_SECTORS)
				new_data_offset = sd->data_offset - min;
			else {
				if (data_offset > sd->data_offset - min) {
					pr_err("--data-offset too large for %s\n",
						dn);
					goto release;
				}
				new_data_offset = data_offset;
			}
		} else {
			if (dir == 0) {
				/* can move up or down.  If 'data_offset'
				 * was set we would have already decided,
				 * so just choose direction with most space.
				 */
				if (before > after)
					dir = -1;
				else
					dir = 1;
			}
			sysfs_set_str(sra, NULL, "reshape_direction",
				      dir == 1 ? "backwards" : "forwards");
			if (dir > 0) {
				/* Increase data offset */
				if (after < min) {
					if (can_fallback)
						goto fallback;
					pr_err("Insufficient tail-space for reshape on %s\n",
						dn);
					goto release;
				}
				if (data_offset != INVALID_SECTORS &&
				    data_offset < sd->data_offset + min) {
					pr_err("--data-offset too small on %s\n",
						dn);
					goto release;
				}
				if (data_offset != INVALID_SECTORS)
					new_data_offset = data_offset;
				else
					new_data_offset = choose_offset(sd->data_offset,
									sd->data_offset + after,
									sd->data_offset + min,
									sd->data_offset + after);
			} else {
				/* Decrease data offset */
				if (before < min) {
					if (can_fallback)
						goto fallback;
					pr_err("insufficient head-room on %s\n",
						dn);
					goto release;
				}
				if (data_offset != INVALID_SECTORS &&
				    data_offset < sd->data_offset - min) {
					pr_err("--data-offset too small on %s\n",
						dn);
					goto release;
				}
				if (data_offset != INVALID_SECTORS)
					new_data_offset = data_offset;
				else
					new_data_offset = choose_offset(sd->data_offset - before,
									sd->data_offset,
									sd->data_offset - before,
									sd->data_offset - min);
			}
		}
		err = sysfs_set_num(sra, sd, "new_offset", new_data_offset);
		if (err < 0 && errno == E2BIG) {
			/* try again after increasing data size to max */
			err = sysfs_set_num(sra, sd, "size", 0);
			if (err < 0 && errno == EINVAL &&
			    !(sd->disk.state & (1<<MD_DISK_SYNC))) {
				/* some kernels have a bug where you cannot
				 * use '0' on spare devices. */
				sysfs_set_num(sra, sd, "size",
					      (sra->component_size + after)/2);
			}
			err = sysfs_set_num(sra, sd, "new_offset",
					    new_data_offset);
		}
		if (err < 0) {
			if (errno == E2BIG && data_offset != INVALID_SECTORS) {
				pr_err("data-offset is too big for %s\n", dn);
				goto release;
			}
			if (sd == sra->devs &&
			    (errno == ENOENT || errno == E2BIG))
				/* Early kernel, no 'new_offset' file,
				 * or kernel doesn't like us.
				 * For RAID5/6 this is not fatal
				 */
				return 1;
			pr_err("Cannot set new_offset for %s\n", dn);
			break;
		}
	}
	return err;
release:
	return -1;
fallback:
	/* Just use a backup file */
	return 1;
}

static int raid10_reshape(char *container, int fd, char *devname,
			  struct supertype *st, struct mdinfo *info,
			  struct reshape *reshape,
			  unsigned long long data_offset,
			  int force, int verbose)
{
	/* Changing raid_disks, layout, chunksize or possibly
	 * just data_offset for a RAID10.
	 * We must always change data_offset.  We change by at least
	 * ->min_offset_change which is the largest of the old and new
	 * chunk sizes.
	 * If raid_disks is increasing, then data_offset must decrease
	 * by at least this copy size.
	 * If raid_disks is unchanged, data_offset must increase or
	 * decrease by at least min_offset_change but preferably by much more.
	 * We choose half of the available space.
	 * If raid_disks is decreasing, data_offset must increase by
	 * at least min_offset_change.  To allow of this, component_size
	 * must be decreased by the same amount.
	 *
	 * So we calculate the required minimum and direction, possibly
	 * reduce the component_size, then iterate through the devices
	 * and set the new_data_offset.
	 * If that all works, we set chunk_size, layout, raid_disks, and start
	 * 'reshape'
	 */
	struct mdinfo *sra;
	unsigned long long min;
	int err = 0;

	sra = sysfs_read(fd, NULL,
			 GET_COMPONENT|GET_DEVS|GET_OFFSET|GET_STATE|GET_CHUNK
		);
	if (!sra) {
		pr_err("%s: Cannot get array details from sysfs\n", devname);
		goto release;
	}
	min = reshape->min_offset_change;

	if (info->delta_disks)
		sysfs_set_str(sra, NULL, "reshape_direction",
			      info->delta_disks < 0 ? "backwards" : "forwards");
	if (info->delta_disks < 0 && info->space_after < min) {
		int rv = sysfs_set_num(sra, NULL, "component_size",
				       (sra->component_size - min)/2);
		if (rv) {
			pr_err("cannot reduce component size\n");
			goto release;
		}
	}
	err = set_new_data_offset(sra, st, devname, info->delta_disks,
				  data_offset, min, 0);
	if (err == 1) {
		pr_err("Cannot set new_data_offset: RAID10 reshape not\n");
		cont_err("supported on this kernel\n");
		err = -1;
	}
	if (err < 0)
		goto release;

	if (!err && sysfs_set_num(sra, NULL, "chunk_size", info->new_chunk) < 0)
		err = errno;
	if (!err && sysfs_set_num(sra, NULL, "layout",
				  reshape->after.layout) < 0)
		err = errno;
	if (!err &&
	    sysfs_set_num(sra, NULL, "raid_disks",
			  info->array.raid_disks + info->delta_disks) < 0)
		err = errno;
	if (!err && sysfs_set_str(sra, NULL, "sync_action", "reshape") < 0)
		err = errno;
	if (err) {
		pr_err("Cannot set array shape for %s\n",
		       devname);
		if (err == EBUSY &&
		    (info->array.state & (1<<MD_SB_BITMAP_PRESENT)))
			cont_err("       Bitmap must be removed before shape can be changed\n");
		goto release;
	}
	sysfs_free(sra);
	return 0;
release:
	sysfs_free(sra);
	return 1;
}

static void get_space_after(int fd, struct supertype *st, struct mdinfo *info)
{
	struct mdinfo *sra, *sd;
	/* Initialisation to silence compiler warning */
	unsigned long long min_space_before = 0, min_space_after = 0;
	int first = 1;

	sra = sysfs_read(fd, NULL, GET_DEVS);
	if (!sra)
		return;
	for (sd = sra->devs; sd; sd = sd->next) {
		char *dn;
		int dfd;
		struct supertype *st2;
		struct mdinfo info2;

		if (sd->disk.state & (1<<MD_DISK_FAULTY))
			continue;
		dn = map_dev(sd->disk.major, sd->disk.minor, 0);
		dfd = dev_open(dn, O_RDONLY);
		if (dfd < 0)
			break;
		st2 = dup_super(st);
		if (st2->ss->load_super(st2,dfd, NULL)) {
			close(dfd);
			free(st2);
			break;
		}
		close(dfd);
		st2->ss->getinfo_super(st2, &info2, NULL);
		st2->ss->free_super(st2);
		free(st2);
		if (first ||
		    min_space_before > info2.space_before)
			min_space_before = info2.space_before;
		if (first ||
		    min_space_after > info2.space_after)
			min_space_after = info2.space_after;
		first = 0;
	}
	if (sd == NULL && !first) {
		info->space_after = min_space_after;
		info->space_before = min_space_before;
	}
	sysfs_free(sra);
}

static void update_cache_size(char *container, struct mdinfo *sra,
			      struct mdinfo *info,
			      int disks, unsigned long long blocks)
{
	/* Check that the internal stripe cache is
	 * large enough, or it won't work.
	 * It must hold at least 4 stripes of the larger
	 * chunk size
	 */
	unsigned long cache;
	cache = max(info->array.chunk_size, info->new_chunk);
	cache *= 4; /* 4 stripes minimum */
	cache /= 512; /* convert to sectors */
	/* make sure there is room for 'blocks' with a bit to spare */
	if (cache < 16 + blocks / disks)
		cache = 16 + blocks / disks;
	cache /= (4096/512); /* Convert from sectors to pages */

	if (sra->cache_size < cache)
		subarray_set_num(container, sra, "stripe_cache_size",
				 cache+1);
}

static int impose_reshape(struct mdinfo *sra,
			  struct mdinfo *info,
			  struct supertype *st,
			  int fd,
			  int restart,
			  char *devname, char *container,
			  struct reshape *reshape)
{
	struct mdu_array_info_s array;

	sra->new_chunk = info->new_chunk;

	if (restart) {
		/* for external metadata checkpoint saved by mdmon can be lost
		 * or missed /due to e.g. crash/. Check if md is not during
		 * restart farther than metadata points to.
		 * If so, this means metadata information is obsolete.
		 */
		if (st->ss->external)
			verify_reshape_position(info, reshape->level);
		sra->reshape_progress = info->reshape_progress;
	} else {
		sra->reshape_progress = 0;
		if (reshape->after.data_disks < reshape->before.data_disks)
			/* start from the end of the new array */
			sra->reshape_progress = (sra->component_size
						 * reshape->after.data_disks);
	}

	md_get_array_info(fd, &array);
	if (info->array.chunk_size == info->new_chunk &&
	    reshape->before.layout == reshape->after.layout &&
	    st->ss->external == 0) {
		/* use SET_ARRAY_INFO but only if reshape hasn't started */
		array.raid_disks = reshape->after.data_disks + reshape->parity;
		if (!restart && md_set_array_info(fd, &array) != 0) {
			int err = errno;

			pr_err("Cannot set device shape for %s: %s\n",
			       devname, strerror(errno));

			if (err == EBUSY &&
			    (array.state & (1<<MD_SB_BITMAP_PRESENT)))
				cont_err("Bitmap must be removed before shape can be changed\n");

			goto release;
		}
	} else if (!restart) {
		/* set them all just in case some old 'new_*' value
		 * persists from some earlier problem.
		 */
		int err = 0;
		if (sysfs_set_num(sra, NULL, "chunk_size", info->new_chunk) < 0)
			err = errno;
		if (!err && sysfs_set_num(sra, NULL, "layout",
					  reshape->after.layout) < 0)
			err = errno;
		if (!err && subarray_set_num(container, sra, "raid_disks",
					     reshape->after.data_disks +
					     reshape->parity) < 0)
			err = errno;
		if (err) {
			pr_err("Cannot set device shape for %s\n", devname);

			if (err == EBUSY &&
			    (array.state & (1<<MD_SB_BITMAP_PRESENT)))
				cont_err("Bitmap must be removed before shape can be changed\n");
			goto release;
		}
	}
	return 0;
release:
	return -1;
}

static int impose_level(int fd, int level, char *devname, int verbose)
{
	char *c;
	struct mdu_array_info_s array;
	struct mdinfo info;

	if (sysfs_init(&info, fd, NULL)) {
		pr_err("failed to intialize sysfs.\n");
		return  1;
	}

	md_get_array_info(fd, &array);
	if (level == 0 && (array.level >= 4 && array.level <= 6)) {
		/* To convert to RAID0 we need to fail and
		 * remove any non-data devices. */
		int found = 0;
		int d;
		int data_disks = array.raid_disks - 1;
		if (array.level == 6)
			data_disks -= 1;
		if (array.level == 5 && array.layout != ALGORITHM_PARITY_N)
			return -1;
		if (array.level == 6 && array.layout != ALGORITHM_PARITY_N_6)
			return -1;
		sysfs_set_str(&info, NULL,"sync_action", "idle");
		/* First remove any spares so no recovery starts */
		for (d = 0, found = 0;
		     d < MAX_DISKS && found < array.nr_disks; d++) {
			mdu_disk_info_t disk;
			disk.number = d;
			if (md_get_disk_info(fd, &disk) < 0)
				continue;
			if (disk.major == 0 && disk.minor == 0)
				continue;
			found++;
			if ((disk.state & (1 << MD_DISK_ACTIVE)) &&
			    disk.raid_disk < data_disks)
				/* keep this */
				continue;
			ioctl(fd, HOT_REMOVE_DISK,
			      makedev(disk.major, disk.minor));
		}
		/* Now fail anything left */
		md_get_array_info(fd, &array);
		for (d = 0, found = 0;
		     d < MAX_DISKS && found < array.nr_disks; d++) {
			mdu_disk_info_t disk;
			disk.number = d;
			if (md_get_disk_info(fd, &disk) < 0)
				continue;
			if (disk.major == 0 && disk.minor == 0)
				continue;
			found++;
			if ((disk.state & (1 << MD_DISK_ACTIVE)) &&
			    disk.raid_disk < data_disks)
				/* keep this */
				continue;
			ioctl(fd, SET_DISK_FAULTY,
			      makedev(disk.major, disk.minor));
			hot_remove_disk(fd, makedev(disk.major, disk.minor), 1);
		}
	}
	c = map_num(pers, level);
	if (c) {
		int err = sysfs_set_str(&info, NULL, "level", c);
		if (err) {
			err = errno;
			pr_err("%s: could not set level to %s\n",
				devname, c);
			if (err == EBUSY &&
			    (array.state & (1<<MD_SB_BITMAP_PRESENT)))
				cont_err("Bitmap must be removed before level can be changed\n");
			return err;
		}
		if (verbose >= 0)
			pr_err("level of %s changed to %s\n", devname, c);
	}
	return 0;
}

int sigterm = 0;
static void catch_term(int sig)
{
	sigterm = 1;
}

static int continue_via_systemd(char *devnm)
{
	int skipped, i, pid, status;
	char pathbuf[1024];
	/* In a systemd/udev world, it is best to get systemd to
	 * run "mdadm --grow --continue" rather than running in the
	 * background.
	 */
	switch(fork()) {
	case  0:
		/* FIXME yuk. CLOSE_EXEC?? */
		skipped = 0;
		for (i = 3; skipped < 20; i++)
			if (close(i) < 0)
				skipped++;
			else
				skipped = 0;

		/* Don't want to see error messages from
		 * systemctl.  If the service doesn't exist,
		 * we fork ourselves.
		 */
		close(2);
		open("/dev/null", O_WRONLY);
		snprintf(pathbuf, sizeof(pathbuf),
			 "mdadm-grow-continue@%s.service", devnm);
		status = execl("/usr/bin/systemctl", "systemctl", "restart",
			       pathbuf, NULL);
		status = execl("/bin/systemctl", "systemctl", "restart",
			       pathbuf, NULL);
		exit(1);
	case -1: /* Just do it ourselves. */
		break;
	default: /* parent - good */
		pid = wait(&status);
		if (pid >= 0 && status == 0)
			return 1;
	}
	return 0;
}

static int reshape_array(char *container, int fd, char *devname,
			 struct supertype *st, struct mdinfo *info,
			 int force, struct mddev_dev *devlist,
			 unsigned long long data_offset,
			 char *backup_file, int verbose, int forked,
			 int restart, int freeze_reshape)
{
	struct reshape reshape;
	int spares_needed;
	char *msg;
	int orig_level = UnSet;
	int odisks;
	int delayed;

	struct mdu_array_info_s array;
	char *c;

	struct mddev_dev *dv;
	int added_disks;

	int *fdlist = NULL;
	unsigned long long *offsets = NULL;
	int d;
	int nrdisks;
	int err;
	unsigned long blocks;
	unsigned long long array_size;
	int done;
	struct mdinfo *sra = NULL;
	char buf[20];

	/* when reshaping a RAID0, the component_size might be zero.
	 * So try to fix that up.
	 */
	if (md_get_array_info(fd, &array) != 0) {
		dprintf("Cannot get array information.\n");
		goto release;
	}
	if (array.level == 0 && info->component_size == 0) {
		get_dev_size(fd, NULL, &array_size);
		info->component_size = array_size / array.raid_disks;
	}

	if (array.level == 10)
		/* Need space_after info */
		get_space_after(fd, st, info);

	if (info->reshape_active) {
		int new_level = info->new_level;
		info->new_level = UnSet;
		if (info->delta_disks > 0)
			info->array.raid_disks -= info->delta_disks;
		msg = analyse_change(devname, info, &reshape);
		info->new_level = new_level;
		if (info->delta_disks > 0)
			info->array.raid_disks += info->delta_disks;
		if (!restart)
			/* Make sure the array isn't read-only */
			ioctl(fd, RESTART_ARRAY_RW, 0);
	} else
		msg = analyse_change(devname, info, &reshape);
	if (msg) {
		/* if msg == "", error has already been printed */
		if (msg[0])
			pr_err("%s\n", msg);
		goto release;
	}
	if (restart && (reshape.level != info->array.level ||
			reshape.before.layout != info->array.layout ||
			reshape.before.data_disks + reshape.parity !=
			info->array.raid_disks - max(0, info->delta_disks))) {
		pr_err("reshape info is not in native format - cannot continue.\n");
		goto release;
	}

	if (st->ss->external && restart && (info->reshape_progress == 0) &&
	    !((sysfs_get_str(info, NULL, "sync_action",
			     buf, sizeof(buf)) > 0) &&
	      (strncmp(buf, "reshape", 7) == 0))) {
		/* When reshape is restarted from '0', very begin of array
		 * it is possible that for external metadata reshape and array
		 * configuration doesn't happen.
		 * Check if md has the same opinion, and reshape is restarted
		 * from 0. If so, this is regular reshape start after reshape
		 * switch in metadata to next array only.
		 */
		if ((verify_reshape_position(info, reshape.level) >= 0) &&
		    (info->reshape_progress == 0))
			restart = 0;
	}
	if (restart) {
		/*
		 * reshape already started. just skip to monitoring
		 * the reshape
		 */
		if (reshape.backup_blocks == 0)
			return 0;
		if (restart & RESHAPE_NO_BACKUP)
			return 0;

		/* Need 'sra' down at 'started:' */
		sra = sysfs_read(fd, NULL,
				 GET_COMPONENT|GET_DEVS|GET_OFFSET|GET_STATE|
				 GET_CHUNK|GET_CACHE);
		if (!sra) {
			pr_err("%s: Cannot get array details from sysfs\n",
			       devname);
			goto release;
		}

		if (!backup_file)
			backup_file = locate_backup(sra->sys_name);

		goto started;
	}
	/* The container is frozen but the array may not be.
	 * So freeze the array so spares don't get put to the wrong use
	 * FIXME there should probably be a cleaner separation between
	 * freeze_array and freeze_container.
	 */
	sysfs_freeze_array(info);
	/* Check we have enough spares to not be degraded */
	added_disks = 0;
	for (dv = devlist; dv ; dv=dv->next)
		added_disks++;
	spares_needed = max(reshape.before.data_disks,
			    reshape.after.data_disks) +
		reshape.parity - array.raid_disks;

	if (!force && info->new_level > 1 && info->array.level > 1 &&
	    spares_needed > info->array.spare_disks + added_disks) {
		pr_err("Need %d spare%s to avoid degraded array, and only have %d.\n"
		       "       Use --force to over-ride this check.\n",
		       spares_needed,
		       spares_needed == 1 ? "" : "s",
		       info->array.spare_disks + added_disks);
		goto release;
	}
	/* Check we have enough spares to not fail */
	spares_needed = max(reshape.before.data_disks,
			    reshape.after.data_disks)
		- array.raid_disks;
	if ((info->new_level > 1 || info->new_level == 0) &&
	    spares_needed > info->array.spare_disks +added_disks) {
		pr_err("Need %d spare%s to create working array, and only have %d.\n",
		       spares_needed, spares_needed == 1 ? "" : "s",
		       info->array.spare_disks + added_disks);
		goto release;
	}

	if (reshape.level != array.level) {
		int err = impose_level(fd, reshape.level, devname, verbose);
		if (err)
			goto release;
		info->new_layout = UnSet; /* after level change,
					   * layout is meaningless */
		orig_level = array.level;
		sysfs_freeze_array(info);

		if (reshape.level > 0 && st->ss->external) {
			/* make sure mdmon is aware of the new level */
			if (mdmon_running(container))
				flush_mdmon(container);

			if (!mdmon_running(container))
				start_mdmon(container);
			ping_monitor(container);
			if (mdmon_running(container) && st->update_tail == NULL)
				st->update_tail = &st->updates;
		}
	}
	/* ->reshape_super might have chosen some spares from the
	 * container that it wants to be part of the new array.
	 * We can collect them with ->container_content and give
	 * them to the kernel.
	 */
	if (st->ss->reshape_super && st->ss->container_content) {
		char *subarray = strchr(info->text_version+1, '/')+1;
		struct mdinfo *info2 =
			st->ss->container_content(st, subarray);
		struct mdinfo *d;

		if (info2) {
			if (sysfs_init(info2, fd, st->devnm)) {
				pr_err("unable to initialize sysfs for %s\n",
				       st->devnm);
				free(info2);
				goto release;
			}
			/* When increasing number of devices, we need to set
			 * new raid_disks before adding these, or they might
			 * be rejected.
			 */
			if (reshape.backup_blocks &&
			    reshape.after.data_disks >
			    reshape.before.data_disks)
				subarray_set_num(container, info2, "raid_disks",
						 reshape.after.data_disks +
						 reshape.parity);
			for (d = info2->devs; d; d = d->next) {
				if (d->disk.state == 0 &&
				    d->disk.raid_disk >= 0) {
					/* This is a spare that wants to
					 * be part of the array.
					 */
					add_disk(fd, st, info2, d);
				}
			}
			sysfs_free(info2);
		}
	}
	/* We might have been given some devices to add to the
	 * array.  Now that the array has been changed to the right
	 * level and frozen, we can safely add them.
	 */
	if (devlist) {
		if (Manage_subdevs(devname, fd, devlist, verbose, 0, NULL, 0))
			goto release;
	}

	if (reshape.backup_blocks == 0 && data_offset != INVALID_SECTORS)
		reshape.backup_blocks = reshape.before.data_disks * info->array.chunk_size/512;
	if (reshape.backup_blocks == 0) {
		/* No restriping needed, but we might need to impose
		 * some more changes: layout, raid_disks, chunk_size
		 */
		/* read current array info */
		if (md_get_array_info(fd, &array) != 0) {
			dprintf("Cannot get array information.\n");
			goto release;
		}
		/* compare current array info with new values and if
		 * it is different update them to new */
		if (info->new_layout != UnSet &&
		    info->new_layout != array.layout) {
			array.layout = info->new_layout;
			if (md_set_array_info(fd, &array) != 0) {
				pr_err("failed to set new layout\n");
				goto release;
			} else if (verbose >= 0)
				printf("layout for %s set to %d\n",
				       devname, array.layout);
		}
		if (info->delta_disks != UnSet && info->delta_disks != 0 &&
		    array.raid_disks !=
		    (info->array.raid_disks + info->delta_disks)) {
			array.raid_disks += info->delta_disks;
			if (md_set_array_info(fd, &array) != 0) {
				pr_err("failed to set raid disks\n");
				goto release;
			} else if (verbose >= 0) {
				printf("raid_disks for %s set to %d\n",
				       devname, array.raid_disks);
			}
		}
		if (info->new_chunk != 0 &&
		    info->new_chunk != array.chunk_size) {
			if (sysfs_set_num(info, NULL,
					  "chunk_size", info->new_chunk) != 0) {
				pr_err("failed to set chunk size\n");
				goto release;
			} else if (verbose >= 0)
				printf("chunk size for %s set to %d\n",
				       devname, array.chunk_size);
		}
		unfreeze(st);
		return 0;
	}

	/*
	 * There are three possibilities.
	 * 1/ The array will shrink.
	 *    We need to ensure the reshape will pause before reaching
	 *    the 'critical section'.  We also need to fork and wait for
	 *    that to happen.  When it does we
	 *       suspend/backup/complete/unfreeze
	 *
	 * 2/ The array will not change size.
	 *    This requires that we keep a backup of a sliding window
	 *    so that we can restore data after a crash.  So we need
	 *    to fork and monitor progress.
	 *    In future we will allow the data_offset to change, so
	 *    a sliding backup becomes unnecessary.
	 *
	 * 3/ The array will grow. This is relatively easy.
	 *    However the kernel's restripe routines will cheerfully
	 *    overwrite some early data before it is safe.  So we
	 *    need to make a backup of the early parts of the array
	 *    and be ready to restore it if rebuild aborts very early.
	 *    For externally managed metadata, we still need a forked
	 *    child to monitor the reshape and suspend IO over the region
	 *    that is being reshaped.
	 *
	 *    We backup data by writing it to one spare, or to a
	 *    file which was given on command line.
	 *
	 * In each case, we first make sure that storage is available
	 * for the required backup.
	 * Then we:
	 *   -  request the shape change.
	 *   -  fork to handle backup etc.
	 */
	/* Check that we can hold all the data */
	get_dev_size(fd, NULL, &array_size);
	if (reshape.new_size < (array_size/512)) {
		pr_err("this change will reduce the size of the array.\n"
		       "       use --grow --array-size first to truncate array.\n"
		       "       e.g. mdadm --grow %s --array-size %llu\n",
		       devname, reshape.new_size/2);
		goto release;
	}

	if (array.level == 10) {
		/* Reshaping RAID10 does not require any data backup by
		 * user-space.  Instead it requires that the data_offset
		 * is changed to avoid the need for backup.
		 * So this is handled very separately
		 */
		if (restart)
			/* Nothing to do. */
			return 0;
		return raid10_reshape(container, fd, devname, st, info,
				      &reshape, data_offset, force, verbose);
	}
	sra = sysfs_read(fd, NULL,
			 GET_COMPONENT|GET_DEVS|GET_OFFSET|GET_STATE|GET_CHUNK|
			 GET_CACHE);
	if (!sra) {
		pr_err("%s: Cannot get array details from sysfs\n",
			devname);
		goto release;
	}

	if (!backup_file)
		switch(set_new_data_offset(sra, st, devname,
					   reshape.after.data_disks - reshape.before.data_disks,
					   data_offset,
					   reshape.min_offset_change, 1)) {
	case -1:
		goto release;
	case 0:
		/* Updated data_offset, so it's easy now */
		update_cache_size(container, sra, info,
				  min(reshape.before.data_disks,
				      reshape.after.data_disks),
				  reshape.backup_blocks);

		/* Right, everything seems fine. Let's kick things off.
		 */
		sync_metadata(st);

		if (impose_reshape(sra, info, st, fd, restart,
				   devname, container, &reshape) < 0)
			goto release;
		if (sysfs_set_str(sra, NULL, "sync_action", "reshape") < 0) {
			struct mdinfo *sd;
			if (errno != EINVAL) {
				pr_err("Failed to initiate reshape!\n");
				goto release;
			}
			/* revert data_offset and try the old way */
			for (sd = sra->devs; sd; sd = sd->next) {
				sysfs_set_num(sra, sd, "new_offset",
					      sd->data_offset);
				sysfs_set_str(sra, NULL, "reshape_direction",
					      "forwards");
			}
			break;
		}
		if (info->new_level == reshape.level)
			return 0;
		/* need to adjust level when reshape completes */
		switch(fork()) {
		case -1: /* ignore error, but don't wait */
			return 0;
		default: /* parent */
			return 0;
		case 0:
			map_fork();
			break;
		}
		close(fd);
		wait_reshape(sra);
		fd = open_dev(sra->sys_name);
		if (fd >= 0)
			impose_level(fd, info->new_level, devname, verbose);
		return 0;
	case 1: /* Couldn't set data_offset, try the old way */
		if (data_offset != INVALID_SECTORS) {
			pr_err("Cannot update data_offset on this array\n");
			goto release;
		}
		break;
	}

started:
	/* Decide how many blocks (sectors) for a reshape
	 * unit.  The number we have so far is just a minimum
	 */
	blocks = reshape.backup_blocks;
	if (reshape.before.data_disks ==
	    reshape.after.data_disks) {
		/* Make 'blocks' bigger for better throughput, but
		 * not so big that we reject it below.
		 * Try for 16 megabytes
		 */
		while (blocks * 32 < sra->component_size && blocks < 16*1024*2)
			blocks *= 2;
	} else
		pr_err("Need to backup %luK of critical section..\n", blocks/2);

	if (blocks >= sra->component_size/2) {
		pr_err("%s: Something wrong - reshape aborted\n", devname);
		goto release;
	}

	/* Now we need to open all these devices so we can read/write.
	 */
	nrdisks = max(reshape.before.data_disks,
		      reshape.after.data_disks) + reshape.parity
		+ sra->array.spare_disks;
	fdlist = xcalloc((1+nrdisks), sizeof(int));
	offsets = xcalloc((1+nrdisks), sizeof(offsets[0]));

	odisks = reshape.before.data_disks + reshape.parity;
	d = reshape_prepare_fdlist(devname, sra, odisks, nrdisks, blocks,
				   backup_file, fdlist, offsets);
	if (d < odisks) {
		goto release;
	}
	if ((st->ss->manage_reshape == NULL) ||
	    (st->ss->recover_backup == NULL)) {
		if (backup_file == NULL) {
			if (reshape.after.data_disks <=
			    reshape.before.data_disks) {
				pr_err("%s: Cannot grow - need backup-file\n",
				       devname);
				pr_err(" Please provide one with \"--backup=...\"\n");
				goto release;
			} else if (d == odisks) {
				pr_err("%s: Cannot grow - need a spare or backup-file to backup critical section\n", devname);
				goto release;
			}
		} else {
			if (!reshape_open_backup_file(backup_file, fd, devname,
						      (signed)blocks,
						      fdlist+d, offsets+d,
						      sra->sys_name, restart)) {
				goto release;
			}
			d++;
		}
	}

	update_cache_size(container, sra, info,
			  min(reshape.before.data_disks,
			      reshape.after.data_disks), blocks);

	/* Right, everything seems fine. Let's kick things off.
	 * If only changing raid_disks, use ioctl, else use
	 * sysfs.
	 */
	sync_metadata(st);

	if (impose_reshape(sra, info, st, fd, restart,
			   devname, container, &reshape) < 0)
		goto release;

	err = start_reshape(sra, restart, reshape.before.data_disks,
			    reshape.after.data_disks);
	if (err) {
		pr_err("Cannot %s reshape for %s\n",
		       restart ? "continue" : "start", devname);
		goto release;
	}
	if (restart)
		sysfs_set_str(sra, NULL, "array_state", "active");
	if (freeze_reshape) {
		free(fdlist);
		free(offsets);
		sysfs_free(sra);
		pr_err("Reshape has to be continued from location %llu when root filesystem has been mounted.\n",
			sra->reshape_progress);
		return 1;
	}

	if (!forked && !check_env("MDADM_NO_SYSTEMCTL"))
		if (continue_via_systemd(container ?: sra->sys_name)) {
			free(fdlist);
			free(offsets);
			sysfs_free(sra);
			return 0;
		}

	/* Now we just need to kick off the reshape and watch, while
	 * handling backups of the data...
	 * This is all done by a forked background process.
	 */
	switch(forked ? 0 : fork()) {
	case -1:
		pr_err("Cannot run child to monitor reshape: %s\n",
			strerror(errno));
		abort_reshape(sra);
		goto release;
	default:
		free(fdlist);
		free(offsets);
		sysfs_free(sra);
		return 0;
	case 0:
		map_fork();
		break;
	}

	/* If another array on the same devices is busy, the
	 * reshape will wait for them.  This would mean that
	 * the first section that we suspend will stay suspended
	 * for a long time.  So check on that possibility
	 * by looking for "DELAYED" in /proc/mdstat, and if found,
	 * wait a while
	 */
	do {
		struct mdstat_ent *mds, *m;
		delayed = 0;
		mds = mdstat_read(1, 0);
		for (m = mds; m; m = m->next)
			if (strcmp(m->devnm, sra->sys_name) == 0) {
				if (m->resync && m->percent == RESYNC_DELAYED)
					delayed = 1;
				if (m->resync == 0)
					/* Haven't started the reshape thread
					 * yet, wait a bit
					 */
					delayed = 2;
				break;
			}
		free_mdstat(mds);
		if (delayed == 1 && get_linux_version() < 3007000) {
			pr_err("Reshape is delayed, but cannot wait carefully with this kernel.\n"
			       "       You might experience problems until other reshapes complete.\n");
			delayed = 0;
		}
		if (delayed)
			mdstat_wait(30 - (delayed-1) * 25);
	} while (delayed);
	mdstat_close();
	close(fd);
	if (check_env("MDADM_GROW_VERIFY"))
		fd = open(devname, O_RDONLY | O_DIRECT);
	else
		fd = -1;
	mlockall(MCL_FUTURE);

	signal(SIGTERM, catch_term);

	if (st->ss->external) {
		/* metadata handler takes it from here */
		done = st->ss->manage_reshape(
			fd, sra, &reshape, st, blocks,
			fdlist, offsets, d - odisks, fdlist + odisks,
			offsets + odisks);
	} else
		done = child_monitor(
			fd, sra, &reshape, st, blocks, fdlist, offsets,
			d - odisks, fdlist + odisks, offsets + odisks);

	free(fdlist);
	free(offsets);

	if (backup_file && done) {
		char *bul;
		bul = make_backup(sra->sys_name);
		if (bul) {
			char buf[1024];
			int l = readlink(bul, buf, sizeof(buf) - 1);
			if (l > 0) {
				buf[l]=0;
				unlink(buf);
			}
			unlink(bul);
			free(bul);
		}
		unlink(backup_file);
	}
	if (!done) {
		abort_reshape(sra);
		goto out;
	}

	if (!st->ss->external &&
	    !(reshape.before.data_disks != reshape.after.data_disks &&
	      info->custom_array_size) && info->new_level == reshape.level &&
	    !forked) {
		/* no need to wait for the reshape to finish as
		 * there is nothing more to do.
		 */
		sysfs_free(sra);
		exit(0);
	}
	wait_reshape(sra);

	if (st->ss->external) {
		/* Re-load the metadata as much could have changed */
		int cfd = open_dev(st->container_devnm);
		if (cfd >= 0) {
			flush_mdmon(container);
			st->ss->free_super(st);
			st->ss->load_container(st, cfd, container);
			close(cfd);
		}
	}

	/* set new array size if required customer_array_size is used
	 * by this metadata.
	 */
	if (reshape.before.data_disks != reshape.after.data_disks &&
	    info->custom_array_size)
		set_array_size(st, info, info->text_version);

	if (info->new_level != reshape.level) {
		if (fd < 0)
			fd = open(devname, O_RDONLY);
		impose_level(fd, info->new_level, devname, verbose);
		close(fd);
		if (info->new_level == 0)
			st->update_tail = NULL;
	}
out:
	sysfs_free(sra);
	if (forked)
		return 0;
	unfreeze(st);
	exit(0);

release:
	free(fdlist);
	free(offsets);
	if (orig_level != UnSet && sra) {
		c = map_num(pers, orig_level);
		if (c && sysfs_set_str(sra, NULL, "level", c) == 0)
			pr_err("aborting level change\n");
	}
	sysfs_free(sra);
	if (!forked)
		unfreeze(st);
	return 1;
}

/* mdfd handle is passed to be closed in child process (after fork).
 */
int reshape_container(char *container, char *devname,
		      int mdfd,
		      struct supertype *st,
		      struct mdinfo *info,
		      int force,
		      char *backup_file, int verbose,
		      int forked, int restart, int freeze_reshape)
{
	struct mdinfo *cc = NULL;
	int rv = restart;
	char last_devnm[32] = "";

	/* component_size is not meaningful for a container,
	 * so pass '0' meaning 'no change'
	 */
	if (!restart &&
	    reshape_super(st, 0, info->new_level,
			  info->new_layout, info->new_chunk,
			  info->array.raid_disks, info->delta_disks,
			  backup_file, devname, APPLY_METADATA_CHANGES,
			  verbose)) {
		unfreeze(st);
		return 1;
	}

	sync_metadata(st);

	/* ping monitor to be sure that update is on disk
	 */
	ping_monitor(container);

	if (!forked && !freeze_reshape && !check_env("MDADM_NO_SYSTEMCTL"))
		if (continue_via_systemd(container))
			return 0;

	switch (forked ? 0 : fork()) {
	case -1: /* error */
		perror("Cannot fork to complete reshape\n");
		unfreeze(st);
		return 1;
	default: /* parent */
		if (!freeze_reshape)
			printf("%s: multi-array reshape continues in background\n", Name);
		return 0;
	case 0: /* child */
		map_fork();
		break;
	}

	/* close unused handle in child process
	 */
	if (mdfd > -1)
		close(mdfd);

	while(1) {
		/* For each member array with reshape_active,
		 * we need to perform the reshape.
		 * We pick the first array that needs reshaping and
		 * reshape it.  reshape_array() will re-read the metadata
		 * so the next time through a different array should be
		 * ready for reshape.
		 * It is possible that the 'different' array will not
		 * be assembled yet.  In that case we simple exit.
		 * When it is assembled, the mdadm which assembles it
		 * will take over the reshape.
		 */
		struct mdinfo *content;
		int fd;
		struct mdstat_ent *mdstat;
		char *adev;
		dev_t devid;

		sysfs_free(cc);

		cc = st->ss->container_content(st, NULL);

		for (content = cc; content ; content = content->next) {
			char *subarray;
			if (!content->reshape_active)
				continue;

			subarray = strchr(content->text_version+1, '/')+1;
			mdstat = mdstat_by_subdev(subarray, container);
			if (!mdstat)
				continue;
			if (mdstat->active == 0) {
				pr_err("Skipping inactive array %s.\n",
				       mdstat->devnm);
				free_mdstat(mdstat);
				mdstat = NULL;
				continue;
			}
			break;
		}
		if (!content)
			break;

		devid = devnm2devid(mdstat->devnm);
		adev = map_dev(major(devid), minor(devid), 0);
		if (!adev)
			adev = content->text_version;

		fd = open_dev(mdstat->devnm);
		if (fd < 0) {
			pr_err("Device %s cannot be opened for reshape.\n",
			       adev);
			break;
		}

		if (strcmp(last_devnm, mdstat->devnm) == 0) {
			/* Do not allow for multiple reshape_array() calls for
			 * the same array.
			 * It can happen when reshape_array() returns without
			 * error, when reshape is not finished (wrong reshape
			 * starting/continuation conditions).  Mdmon doesn't
			 * switch to next array in container and reentry
			 * conditions for the same array occur.
			 * This is possibly interim until the behaviour of
			 * reshape_array is resolved().
			 */
			printf("%s: Multiple reshape execution detected for device  %s.\n", Name, adev);
			close(fd);
			break;
		}
		strcpy(last_devnm, mdstat->devnm);

		if (sysfs_init(content, fd, mdstat->devnm)) {
			pr_err("Unable to initialize sysfs for %s\n",
			       mdstat->devnm);
			rv = 1;
			break;
		}

		if (mdmon_running(container))
			flush_mdmon(container);

		rv = reshape_array(container, fd, adev, st,
				   content, force, NULL, INVALID_SECTORS,
				   backup_file, verbose, 1, restart,
				   freeze_reshape);
		close(fd);

		if (freeze_reshape) {
			sysfs_free(cc);
			exit(0);
		}

		restart = 0;
		if (rv)
			break;

		if (mdmon_running(container))
			flush_mdmon(container);
	}
	if (!rv)
		unfreeze(st);
	sysfs_free(cc);
	exit(0);
}

/*
 * We run a child process in the background which performs the following
 * steps:
 *   - wait for resync to reach a certain point
 *   - suspend io to the following section
 *   - backup that section
 *   - allow resync to proceed further
 *   - resume io
 *   - discard the backup.
 *
 * When are combined in slightly different ways in the three cases.
 * Grow:
 *   - suspend/backup/allow/wait/resume/discard
 * Shrink:
 *   - allow/wait/suspend/backup/allow/wait/resume/discard
 * same-size:
 *   - wait/resume/discard/suspend/backup/allow
 *
 * suspend/backup/allow always come together
 * wait/resume/discard do too.
 * For the same-size case we have two backups to improve flow.
 *
 */

int progress_reshape(struct mdinfo *info, struct reshape *reshape,
		     unsigned long long backup_point,
		     unsigned long long wait_point,
		     unsigned long long *suspend_point,
		     unsigned long long *reshape_completed, int *frozen)
{
	/* This function is called repeatedly by the reshape manager.
	 * It determines how much progress can safely be made and allows
	 * that progress.
	 * - 'info' identifies the array and particularly records in
	 *    ->reshape_progress the metadata's knowledge of progress
	 *      This is a sector offset from the start of the array
	 *      of the next array block to be relocated.  This number
	 *      may increase from 0 or decrease from array_size, depending
	 *      on the type of reshape that is happening.
	 *    Note that in contrast, 'sync_completed' is a block count of the
	 *    reshape so far.  It gives the distance between the start point
	 *    (head or tail of device) and the next place that data will be
	 *    written.  It always increases.
	 * - 'reshape' is the structure created by analyse_change
	 * - 'backup_point' shows how much the metadata manager has backed-up
	 *   data.  For reshapes with increasing progress, it is the next address
	 *   to be backed up, previous addresses have been backed-up.  For
	 *   decreasing progress, it is the earliest address that has been
	 *   backed up - later address are also backed up.
	 *   So addresses between reshape_progress and backup_point are
	 *   backed up providing those are in the 'correct' order.
	 * - 'wait_point' is an array address.  When reshape_completed
	 *   passes this point, progress_reshape should return.  It might
	 *   return earlier if it determines that ->reshape_progress needs
	 *   to be updated or further backup is needed.
	 * - suspend_point is maintained by progress_reshape and the caller
	 *   should not touch it except to initialise to zero.
	 *   It is an array address and it only increases in 2.6.37 and earlier.
	 *   This makes it difficult to handle reducing reshapes with
	 *   external metadata.
	 *   However:  it is similar to backup_point in that it records the
	 *     other end of a suspended region from  reshape_progress.
	 *     it is moved to extend the region that is safe to backup and/or
	 *     reshape
	 * - reshape_completed is read from sysfs and returned.  The caller
	 *   should copy this into ->reshape_progress when it has reason to
	 *   believe that the metadata knows this, and any backup outside this
	 *   has been erased.
	 *
	 * Return value is:
	 *   1 if more data from backup_point - but only as far as suspend_point,
	 *     should be backed up
	 *   0 if things are progressing smoothly
	 *  -1 if the reshape is finished because it is all done,
	 *  -2 if the reshape is finished due to an error.
	 */

	int advancing = (reshape->after.data_disks
			 >= reshape->before.data_disks);
	unsigned long long need_backup; /* All data between start of array and
					 * here will at some point need to
					 * be backed up.
					 */
	unsigned long long read_offset, write_offset;
	unsigned long long write_range;
	unsigned long long max_progress, target, completed;
	unsigned long long array_size = (info->component_size
					 * reshape->before.data_disks);
	int fd;
	char buf[20];

	/* First, we unsuspend any region that is now known to be safe.
	 * If suspend_point is on the 'wrong' side of reshape_progress, then
	 * we don't have or need suspension at the moment.  This is true for
	 * native metadata when we don't need to back-up.
	 */
	if (advancing) {
		if (info->reshape_progress <= *suspend_point)
			sysfs_set_num(info, NULL, "suspend_lo",
				      info->reshape_progress);
	} else {
		/* Note: this won't work in 2.6.37 and before.
		 * Something somewhere should make sure we don't need it!
		 */
		if (info->reshape_progress >= *suspend_point)
			sysfs_set_num(info, NULL, "suspend_hi",
				      info->reshape_progress);
	}

	/* Now work out how far it is safe to progress.
	 * If the read_offset for ->reshape_progress is less than
	 * 'blocks' beyond the write_offset, we can only progress as far
	 * as a backup.
	 * Otherwise we can progress until the write_offset for the new location
	 * reaches (within 'blocks' of) the read_offset at the current location.
	 * However that region must be suspended unless we are using native
	 * metadata.
	 * If we need to suspend more, we limit it to 128M per device, which is
	 * rather arbitrary and should be some time-based calculation.
	 */
	read_offset = info->reshape_progress / reshape->before.data_disks;
	write_offset = info->reshape_progress / reshape->after.data_disks;
	write_range = info->new_chunk/512;
	if (reshape->before.data_disks == reshape->after.data_disks)
		need_backup = array_size;
	else
		need_backup = reshape->backup_blocks;
	if (advancing) {
		if (read_offset < write_offset + write_range)
			max_progress = backup_point;
		else
			max_progress =
				read_offset * reshape->after.data_disks;
	} else {
		if (read_offset > write_offset - write_range)
			/* Can only progress as far as has been backed up,
			 * which must be suspended */
			max_progress = backup_point;
		else if (info->reshape_progress <= need_backup)
			max_progress = backup_point;
		else {
			if (info->array.major_version >= 0)
				/* Can progress until backup is needed */
				max_progress = need_backup;
			else {
				/* Can progress until metadata update is required */
				max_progress =
					read_offset * reshape->after.data_disks;
				/* but data must be suspended */
				if (max_progress < *suspend_point)
					max_progress = *suspend_point;
			}
		}
	}

	/* We know it is safe to progress to 'max_progress' providing
	 * it is suspended or we are using native metadata.
	 * Consider extending suspend_point 128M per device if it
	 * is less than 64M per device beyond reshape_progress.
	 * But always do a multiple of 'blocks'
	 * FIXME this is too big - it takes to long to complete
	 * this much.
	 */
	target = 64*1024*2 * min(reshape->before.data_disks,
				 reshape->after.data_disks);
	target /= reshape->backup_blocks;
	if (target < 2)
		target = 2;
	target *= reshape->backup_blocks;

	/* For externally managed metadata we always need to suspend IO to
	 * the area being reshaped so we regularly push suspend_point forward.
	 * For native metadata we only need the suspend if we are going to do
	 * a backup.
	 */
	if (advancing) {
		if ((need_backup > info->reshape_progress ||
		     info->array.major_version < 0) &&
		    *suspend_point < info->reshape_progress + target) {
			if (need_backup < *suspend_point + 2 * target)
				*suspend_point = need_backup;
			else if (*suspend_point + 2 * target < array_size)
				*suspend_point += 2 * target;
			else
				*suspend_point = array_size;
			sysfs_set_num(info, NULL, "suspend_hi", *suspend_point);
			if (max_progress > *suspend_point)
				max_progress = *suspend_point;
		}
	} else {
		if (info->array.major_version >= 0) {
			/* Only need to suspend when about to backup */
			if (info->reshape_progress < need_backup * 2 &&
			    *suspend_point > 0) {
				*suspend_point = 0;
				sysfs_set_num(info, NULL, "suspend_lo", 0);
				sysfs_set_num(info, NULL, "suspend_hi",
					      need_backup);
			}
		} else {
			/* Need to suspend continually */
			if (info->reshape_progress < *suspend_point)
				*suspend_point = info->reshape_progress;
			if (*suspend_point + target < info->reshape_progress)
				/* No need to move suspend region yet */;
			else {
				if (*suspend_point >= 2 * target)
					*suspend_point -= 2 * target;
				else
					*suspend_point = 0;
				sysfs_set_num(info, NULL, "suspend_lo",
					      *suspend_point);
			}
			if (max_progress < *suspend_point)
				max_progress = *suspend_point;
		}
	}

	/* now set sync_max to allow that progress. sync_max, like
	 * sync_completed is a count of sectors written per device, so
	 * we find the difference between max_progress and the start point,
	 * and divide that by after.data_disks to get a sync_max
	 * number.
	 * At the same time we convert wait_point to a similar number
	 * for comparing against sync_completed.
	 */
	/* scale down max_progress to per_disk */
	max_progress /= reshape->after.data_disks;
	/*
	 * Round to chunk size as some kernels give an erroneously
	 * high number
	 */
	max_progress /= info->new_chunk/512;
	max_progress *= info->new_chunk/512;
	/* And round to old chunk size as the kernel wants that */
	max_progress /= info->array.chunk_size/512;
	max_progress *= info->array.chunk_size/512;
	/* Limit progress to the whole device */
	if (max_progress > info->component_size)
		max_progress = info->component_size;
	wait_point /= reshape->after.data_disks;
	if (!advancing) {
		/* switch from 'device offset' to 'processed block count' */
		max_progress = info->component_size - max_progress;
		wait_point = info->component_size - wait_point;
	}

	if (!*frozen)
		sysfs_set_num(info, NULL, "sync_max", max_progress);

	/* Now wait.  If we have already reached the point that we were
	 * asked to wait to, don't wait at all, else wait for any change.
	 * We need to select on 'sync_completed' as that is the place that
	 * notifications happen, but we are really interested in
	 * 'reshape_position'
	 */
	fd = sysfs_get_fd(info, NULL, "sync_completed");
	if (fd < 0)
		goto check_progress;

	if (sysfs_fd_get_ll(fd, &completed) < 0)
		goto check_progress;

	while (completed < max_progress && completed < wait_point) {
		/* Check that sync_action is still 'reshape' to avoid
		 * waiting forever on a dead array
		 */
		char action[20];
		if (sysfs_get_str(info, NULL, "sync_action", action, 20) <= 0 ||
		    strncmp(action, "reshape", 7) != 0)
			break;
		/* Some kernels reset 'sync_completed' to zero
		 * before setting 'sync_action' to 'idle'.
		 * So we need these extra tests.
		 */
		if (completed == 0 && advancing &&
		    strncmp(action, "idle", 4) == 0 &&
		    info->reshape_progress > 0)
			break;
		if (completed == 0 && !advancing &&
		    strncmp(action, "idle", 4) == 0 &&
		    info->reshape_progress <
		    (info->component_size * reshape->after.data_disks))
			break;
		sysfs_wait(fd, NULL);
		if (sysfs_fd_get_ll(fd, &completed) < 0)
			goto check_progress;
	}
	/* Some kernels reset 'sync_completed' to zero,
	 * we need to have real point we are in md.
	 * So in that case, read 'reshape_position' from sysfs.
	 */
	if (completed == 0) {
		unsigned long long reshapep;
		char action[20];
		if (sysfs_get_str(info, NULL, "sync_action", action, 20) > 0 &&
		    strncmp(action, "idle", 4) == 0 &&
		    sysfs_get_ll(info, NULL,
				 "reshape_position", &reshapep) == 0)
			*reshape_completed = reshapep;
	} else {
		/* some kernels can give an incorrectly high
		 * 'completed' number, so round down */
		completed /= (info->new_chunk/512);
		completed *= (info->new_chunk/512);
		/* Convert 'completed' back in to a 'progress' number */
		completed *= reshape->after.data_disks;
		if (!advancing)
			completed = (info->component_size
				     * reshape->after.data_disks
				     - completed);
		*reshape_completed = completed;
	}

	close(fd);

	/* We return the need_backup flag.  Caller will decide
	 * how much - a multiple of ->backup_blocks up to *suspend_point
	 */
	if (advancing)
		return need_backup > info->reshape_progress;
	else
		return need_backup >= info->reshape_progress;

check_progress:
	/* if we couldn't read a number from sync_completed, then
	 * either the reshape did complete, or it aborted.
	 * We can tell which by checking for 'none' in reshape_position.
	 * If it did abort, then it might immediately restart if it
	 * it was just a device failure that leaves us degraded but
	 * functioning.
	 */
	if (sysfs_get_str(info, NULL, "reshape_position", buf,
			  sizeof(buf)) < 0 || strncmp(buf, "none", 4) != 0) {
		/* The abort might only be temporary.  Wait up to 10
		 * seconds for fd to contain a valid number again.
		 */
		int wait = 10000;
		int rv = -2;
		unsigned long long new_sync_max;
		while (fd >= 0 && rv < 0 && wait > 0) {
			if (sysfs_wait(fd, &wait) != 1)
				break;
			switch (sysfs_fd_get_ll(fd, &completed)) {
			case 0:
				/* all good again */
				rv = 1;
				/* If "sync_max" is no longer max_progress
				 * we need to freeze things
				 */
				sysfs_get_ll(info, NULL, "sync_max",
					     &new_sync_max);
				*frozen = (new_sync_max != max_progress);
				break;
			case -2: /* read error - abort */
				wait = 0;
				break;
			}
		}
		if (fd >= 0)
			close(fd);
		return rv; /* abort */
	} else {
		/* Maybe racing with array shutdown - check state */
		if (fd >= 0)
			close(fd);
		if (sysfs_get_str(info, NULL, "array_state", buf,
				  sizeof(buf)) < 0 ||
		    strncmp(buf, "inactive", 8) == 0 ||
		    strncmp(buf, "clear",5) == 0)
			return -2; /* abort */
		return -1; /* complete */
	}
}

/* FIXME return status is never checked */
static int grow_backup(struct mdinfo *sra,
		unsigned long long offset, /* per device */
		unsigned long stripes, /* per device, in old chunks */
		int *sources, unsigned long long *offsets,
		int disks, int chunk, int level, int layout,
		int dests, int *destfd, unsigned long long *destoffsets,
		int part, int *degraded,
		char *buf)
{
	/* Backup 'blocks' sectors at 'offset' on each device of the array,
	 * to storage 'destfd' (offset 'destoffsets'), after first
	 * suspending IO.  Then allow resync to continue
	 * over the suspended section.
	 * Use part 'part' of the backup-super-block.
	 */
	int odata = disks;
	int rv = 0;
	int i;
	unsigned long long ll;
	int new_degraded;
	//printf("offset %llu\n", offset);
	if (level >= 4)
		odata--;
	if (level == 6)
		odata--;

	/* Check that array hasn't become degraded, else we might backup the wrong data */
	if (sysfs_get_ll(sra, NULL, "degraded", &ll) < 0)
		return -1; /* FIXME this error is ignored */
	new_degraded = (int)ll;
	if (new_degraded != *degraded) {
		/* check each device to ensure it is still working */
		struct mdinfo *sd;
		for (sd = sra->devs ; sd ; sd = sd->next) {
			if (sd->disk.state & (1<<MD_DISK_FAULTY))
				continue;
			if (sd->disk.state & (1<<MD_DISK_SYNC)) {
				char sbuf[100];

				if (sysfs_get_str(sra, sd, "state",
						  sbuf, sizeof(sbuf)) < 0 ||
				    strstr(sbuf, "faulty") ||
				    strstr(sbuf, "in_sync") == NULL) {
					/* this device is dead */
					sd->disk.state = (1<<MD_DISK_FAULTY);
					if (sd->disk.raid_disk >= 0 &&
					    sources[sd->disk.raid_disk] >= 0) {
						close(sources[sd->disk.raid_disk]);
						sources[sd->disk.raid_disk] = -1;
					}
				}
			}
		}
		*degraded = new_degraded;
	}
	if (part) {
		bsb.arraystart2 = __cpu_to_le64(offset * odata);
		bsb.length2 = __cpu_to_le64(stripes * (chunk/512) * odata);
	} else {
		bsb.arraystart = __cpu_to_le64(offset * odata);
		bsb.length = __cpu_to_le64(stripes * (chunk/512) * odata);
	}
	if (part)
		bsb.magic[15] = '2';
	for (i = 0; i < dests; i++)
		if (part)
			lseek64(destfd[i], destoffsets[i] +
				__le64_to_cpu(bsb.devstart2)*512, 0);
		else
			lseek64(destfd[i], destoffsets[i], 0);

	rv = save_stripes(sources, offsets, disks, chunk, level, layout,
			  dests, destfd, offset * 512 * odata,
			  stripes * chunk * odata, buf);

	if (rv)
		return rv;
	bsb.mtime = __cpu_to_le64(time(0));
	for (i = 0; i < dests; i++) {
		bsb.devstart = __cpu_to_le64(destoffsets[i]/512);

		bsb.sb_csum = bsb_csum((char*)&bsb,
				       ((char*)&bsb.sb_csum)-((char*)&bsb));
		if (memcmp(bsb.magic, "md_backup_data-2", 16) == 0)
			bsb.sb_csum2 = bsb_csum((char*)&bsb,
						((char*)&bsb.sb_csum2)-((char*)&bsb));

		rv = -1;
		if ((unsigned long long)lseek64(destfd[i],
						destoffsets[i] - 4096, 0) !=
		    destoffsets[i] - 4096)
			break;
		if (write(destfd[i], &bsb, 512) != 512)
			break;
		if (destoffsets[i] > 4096) {
			if ((unsigned long long)lseek64(destfd[i], destoffsets[i]+stripes*chunk*odata, 0) !=
			    destoffsets[i]+stripes*chunk*odata)
				break;
			if (write(destfd[i], &bsb, 512) != 512)
				break;
		}
		fsync(destfd[i]);
		rv = 0;
	}

	return rv;
}

/* in 2.6.30, the value reported by sync_completed can be
 * less that it should be by one stripe.
 * This only happens when reshape hits sync_max and pauses.
 * So allow wait_backup to either extent sync_max further
 * than strictly necessary, or return before the
 * sync has got quite as far as we would really like.
 * This is what 'blocks2' is for.
 * The various caller give appropriate values so that
 * every works.
 */
/* FIXME return value is often ignored */
static int forget_backup(int dests, int *destfd,
			 unsigned long long *destoffsets,
			 int part)
{
	/*
	 * Erase backup 'part' (which is 0 or 1)
	 */
	int i;
	int rv;

	if (part) {
		bsb.arraystart2 = __cpu_to_le64(0);
		bsb.length2 = __cpu_to_le64(0);
	} else {
		bsb.arraystart = __cpu_to_le64(0);
		bsb.length = __cpu_to_le64(0);
	}
	bsb.mtime = __cpu_to_le64(time(0));
	rv = 0;
	for (i = 0; i < dests; i++) {
		bsb.devstart = __cpu_to_le64(destoffsets[i]/512);
		bsb.sb_csum = bsb_csum((char*)&bsb,
				       ((char*)&bsb.sb_csum)-((char*)&bsb));
		if (memcmp(bsb.magic, "md_backup_data-2", 16) == 0)
			bsb.sb_csum2 = bsb_csum((char*)&bsb,
						((char*)&bsb.sb_csum2)-((char*)&bsb));
		if ((unsigned long long)lseek64(destfd[i], destoffsets[i]-4096, 0) !=
		    destoffsets[i]-4096)
			rv = -1;
		if (rv == 0 && write(destfd[i], &bsb, 512) != 512)
			rv = -1;
		fsync(destfd[i]);
	}
	return rv;
}

static void fail(char *msg)
{
	int rv;
	rv = (write(2, msg, strlen(msg)) != (int)strlen(msg));
	rv |= (write(2, "\n", 1) != 1);
	exit(rv ? 1 : 2);
}

static char *abuf, *bbuf;
static unsigned long long abuflen;
static void validate(int afd, int bfd, unsigned long long offset)
{
	/* check that the data in the backup against the array.
	 * This is only used for regression testing and should not
	 * be used while the array is active
	 */
	if (afd < 0)
		return;
	lseek64(bfd, offset - 4096, 0);
	if (read(bfd, &bsb2, 512) != 512)
		fail("cannot read bsb");
	if (bsb2.sb_csum != bsb_csum((char*)&bsb2,
				     ((char*)&bsb2.sb_csum)-((char*)&bsb2)))
		fail("first csum bad");
	if (memcmp(bsb2.magic, "md_backup_data", 14) != 0)
		fail("magic is bad");
	if (memcmp(bsb2.magic, "md_backup_data-2", 16) == 0 &&
	    bsb2.sb_csum2 != bsb_csum((char*)&bsb2,
				      ((char*)&bsb2.sb_csum2)-((char*)&bsb2)))
		fail("second csum bad");

	if (__le64_to_cpu(bsb2.devstart)*512 != offset)
		fail("devstart is wrong");

	if (bsb2.length) {
		unsigned long long len = __le64_to_cpu(bsb2.length)*512;

		if (abuflen < len) {
			free(abuf);
			free(bbuf);
			abuflen = len;
			if (posix_memalign((void**)&abuf, 4096, abuflen) ||
			    posix_memalign((void**)&bbuf, 4096, abuflen)) {
				abuflen = 0;
				/* just stop validating on mem-alloc failure */
				return;
			}
		}

		lseek64(bfd, offset, 0);
		if ((unsigned long long)read(bfd, bbuf, len) != len) {
			//printf("len %llu\n", len);
			fail("read first backup failed");
		}
		lseek64(afd, __le64_to_cpu(bsb2.arraystart)*512, 0);
		if ((unsigned long long)read(afd, abuf, len) != len)
			fail("read first from array failed");
		if (memcmp(bbuf, abuf, len) != 0) {
#if 0
			int i;
			printf("offset=%llu len=%llu\n",
			       (unsigned long long)__le64_to_cpu(bsb2.arraystart)*512, len);
			for (i=0; i<len; i++)
				if (bbuf[i] != abuf[i]) {
					printf("first diff byte %d\n", i);
					break;
				}
#endif
			fail("data1 compare failed");
		}
	}
	if (bsb2.length2) {
		unsigned long long len = __le64_to_cpu(bsb2.length2)*512;

		if (abuflen < len) {
			free(abuf);
			free(bbuf);
			abuflen = len;
			abuf = xmalloc(abuflen);
			bbuf = xmalloc(abuflen);
		}

		lseek64(bfd, offset+__le64_to_cpu(bsb2.devstart2)*512, 0);
		if ((unsigned long long)read(bfd, bbuf, len) != len)
			fail("read second backup failed");
		lseek64(afd, __le64_to_cpu(bsb2.arraystart2)*512, 0);
		if ((unsigned long long)read(afd, abuf, len) != len)
			fail("read second from array failed");
		if (memcmp(bbuf, abuf, len) != 0)
			fail("data2 compare failed");
	}
}

int child_monitor(int afd, struct mdinfo *sra, struct reshape *reshape,
		  struct supertype *st, unsigned long blocks,
		  int *fds, unsigned long long *offsets,
		  int dests, int *destfd, unsigned long long *destoffsets)
{
	/* Monitor a reshape where backup is being performed using
	 * 'native' mechanism - either to a backup file, or
	 * to some space in a spare.
	 */
	char *buf;
	int degraded = -1;
	unsigned long long speed;
	unsigned long long suspend_point, array_size;
	unsigned long long backup_point, wait_point;
	unsigned long long reshape_completed;
	int done = 0;
	int increasing = reshape->after.data_disks >=
		reshape->before.data_disks;
	int part = 0; /* The next part of the backup area to fill.  It
		       * may already be full, so we need to check */
	int level = reshape->level;
	int layout = reshape->before.layout;
	int data = reshape->before.data_disks;
	int disks = reshape->before.data_disks + reshape->parity;
	int chunk = sra->array.chunk_size;
	struct mdinfo *sd;
	unsigned long stripes;
	int uuid[4];
	int frozen = 0;

	/* set up the backup-super-block.  This requires the
	 * uuid from the array.
	 */
	/* Find a superblock */
	for (sd = sra->devs; sd; sd = sd->next) {
		char *dn;
		int devfd;
		int ok;
		if (sd->disk.state & (1<<MD_DISK_FAULTY))
			continue;
		dn = map_dev(sd->disk.major, sd->disk.minor, 1);
		devfd = dev_open(dn, O_RDONLY);
		if (devfd < 0)
			continue;
		ok = st->ss->load_super(st, devfd, NULL);
		close(devfd);
		if (ok == 0)
			break;
	}
	if (!sd) {
		pr_err("Cannot find a superblock\n");
		return 0;
	}

	memset(&bsb, 0, 512);
	memcpy(bsb.magic, "md_backup_data-1", 16);
	st->ss->uuid_from_super(st, uuid);
	memcpy(bsb.set_uuid, uuid, 16);
	bsb.mtime = __cpu_to_le64(time(0));
	bsb.devstart2 = blocks;

	stripes = blocks / (sra->array.chunk_size/512) /
		reshape->before.data_disks;

	if (posix_memalign((void**)&buf, 4096, disks * chunk))
		/* Don't start the 'reshape' */
		return 0;
	if (reshape->before.data_disks == reshape->after.data_disks) {
		sysfs_get_ll(sra, NULL, "sync_speed_min", &speed);
		sysfs_set_num(sra, NULL, "sync_speed_min", 200000);
	}

	if (increasing) {
		array_size = sra->component_size * reshape->after.data_disks;
		backup_point = sra->reshape_progress;
		suspend_point = 0;
	} else {
		array_size = sra->component_size * reshape->before.data_disks;
		backup_point = reshape->backup_blocks;
		suspend_point = array_size;
	}

	while (!done) {
		int rv;

		/* Want to return as soon the oldest backup slot can
		 * be released as that allows us to start backing up
		 * some more, providing suspend_point has been
		 * advanced, which it should have.
		 */
		if (increasing) {
			wait_point = array_size;
			if (part == 0 && __le64_to_cpu(bsb.length) > 0)
				wait_point = (__le64_to_cpu(bsb.arraystart) +
					      __le64_to_cpu(bsb.length));
			if (part == 1 && __le64_to_cpu(bsb.length2) > 0)
				wait_point = (__le64_to_cpu(bsb.arraystart2) +
					      __le64_to_cpu(bsb.length2));
		} else {
			wait_point = 0;
			if (part == 0 && __le64_to_cpu(bsb.length) > 0)
				wait_point = __le64_to_cpu(bsb.arraystart);
			if (part == 1 && __le64_to_cpu(bsb.length2) > 0)
				wait_point = __le64_to_cpu(bsb.arraystart2);
		}

		reshape_completed = sra->reshape_progress;
		rv = progress_reshape(sra, reshape,
				      backup_point, wait_point,
				      &suspend_point, &reshape_completed,
				      &frozen);
		/* external metadata would need to ping_monitor here */
		sra->reshape_progress = reshape_completed;

		/* Clear any backup region that is before 'here' */
		if (increasing) {
			if (__le64_to_cpu(bsb.length) > 0 &&
			    reshape_completed >= (__le64_to_cpu(bsb.arraystart) +
						  __le64_to_cpu(bsb.length)))
				forget_backup(dests, destfd,
					      destoffsets, 0);
			if (__le64_to_cpu(bsb.length2) > 0 &&
			    reshape_completed >= (__le64_to_cpu(bsb.arraystart2) +
						  __le64_to_cpu(bsb.length2)))
				forget_backup(dests, destfd,
					      destoffsets, 1);
		} else {
			if (__le64_to_cpu(bsb.length) > 0 &&
			    reshape_completed <= (__le64_to_cpu(bsb.arraystart)))
				forget_backup(dests, destfd,
					      destoffsets, 0);
			if (__le64_to_cpu(bsb.length2) > 0 &&
			    reshape_completed <= (__le64_to_cpu(bsb.arraystart2)))
				forget_backup(dests, destfd,
					      destoffsets, 1);
		}
		if (sigterm)
			rv = -2;
		if (rv < 0) {
			if (rv == -1)
				done = 1;
			break;
		}
		if (rv == 0 && increasing && !st->ss->external) {
			/* No longer need to monitor this reshape */
			sysfs_set_str(sra, NULL, "sync_max", "max");
			done = 1;
			break;
		}

		while (rv) {
			unsigned long long offset;
			unsigned long actual_stripes;
			/* Need to backup some data.
			 * If 'part' is not used and the desired
			 * backup size is suspended, do a backup,
			 * then consider the next part.
			 */
			/* Check that 'part' is unused */
			if (part == 0 && __le64_to_cpu(bsb.length) != 0)
				break;
			if (part == 1 && __le64_to_cpu(bsb.length2) != 0)
				break;

			offset = backup_point / data;
			actual_stripes = stripes;
			if (increasing) {
				if (offset + actual_stripes * (chunk/512) >
				    sra->component_size)
					actual_stripes = ((sra->component_size - offset)
							  / (chunk/512));
				if (offset + actual_stripes * (chunk/512) >
				    suspend_point/data)
					break;
			} else {
				if (offset < actual_stripes * (chunk/512))
					actual_stripes = offset / (chunk/512);
				offset -= actual_stripes * (chunk/512);
				if (offset < suspend_point/data)
					break;
			}
			if (actual_stripes == 0)
				break;
			grow_backup(sra, offset, actual_stripes, fds, offsets,
				    disks, chunk, level, layout, dests, destfd,
				    destoffsets, part, &degraded, buf);
			validate(afd, destfd[0], destoffsets[0]);
			/* record where 'part' is up to */
			part = !part;
			if (increasing)
				backup_point += actual_stripes * (chunk/512) * data;
			else
				backup_point -= actual_stripes * (chunk/512) * data;
		}
	}

	/* FIXME maybe call progress_reshape one more time instead */
	/* remove any remaining suspension */
	sysfs_set_num(sra, NULL, "suspend_lo", 0x7FFFFFFFFFFFFFFFULL);
	sysfs_set_num(sra, NULL, "suspend_hi", 0);
	sysfs_set_num(sra, NULL, "suspend_lo", 0);
	sysfs_set_num(sra, NULL, "sync_min", 0);

	if (reshape->before.data_disks == reshape->after.data_disks)
		sysfs_set_num(sra, NULL, "sync_speed_min", speed);
	free(buf);
	return done;
}

/*
 * If any spare contains md_back_data-1 which is recent wrt mtime,
 * write that data into the array and update the super blocks with
 * the new reshape_progress
 */
int Grow_restart(struct supertype *st, struct mdinfo *info, int *fdlist,
		 int cnt, char *backup_file, int verbose)
{
	int i, j;
	int old_disks;
	unsigned long long *offsets;
	unsigned long long  nstripe, ostripe;
	int ndata, odata;

	odata = info->array.raid_disks - info->delta_disks - 1;
	if (info->array.level == 6)
		odata--; /* number of data disks */
	ndata = info->array.raid_disks - 1;
	if (info->new_level == 6)
		ndata--;

	old_disks = info->array.raid_disks - info->delta_disks;

	if (info->delta_disks <= 0)
		/* Didn't grow, so the backup file must have
		 * been used
		 */
		old_disks = cnt;
	for (i=old_disks-(backup_file?1:0); i<cnt; i++) {
		struct mdinfo dinfo;
		int fd;
		int bsbsize;
		char *devname, namebuf[20];
		unsigned long long lo, hi;

		/* This was a spare and may have some saved data on it.
		 * Load the superblock, find and load the
		 * backup_super_block.
		 * If either fail, go on to next device.
		 * If the backup contains no new info, just return
		 * else restore data and update all superblocks
		 */
		if (i == old_disks-1) {
			fd = open(backup_file, O_RDONLY);
			if (fd<0) {
				pr_err("backup file %s inaccessible: %s\n",
					backup_file, strerror(errno));
				continue;
			}
			devname = backup_file;
		} else {
			fd = fdlist[i];
			if (fd < 0)
				continue;
			if (st->ss->load_super(st, fd, NULL))
				continue;

			st->ss->getinfo_super(st, &dinfo, NULL);
			st->ss->free_super(st);

			if (lseek64(fd,
				    (dinfo.data_offset + dinfo.component_size - 8) <<9,
				    0) < 0) {
				pr_err("Cannot seek on device %d\n", i);
				continue; /* Cannot seek */
			}
			sprintf(namebuf, "device-%d", i);
			devname = namebuf;
		}
		if (read(fd, &bsb, sizeof(bsb)) != sizeof(bsb)) {
			if (verbose)
				pr_err("Cannot read from %s\n", devname);
			continue; /* Cannot read */
		}
		if (memcmp(bsb.magic, "md_backup_data-1", 16) != 0 &&
		    memcmp(bsb.magic, "md_backup_data-2", 16) != 0) {
			if (verbose)
				pr_err("No backup metadata on %s\n", devname);
			continue;
		}
		if (bsb.sb_csum != bsb_csum((char*)&bsb, ((char*)&bsb.sb_csum)-((char*)&bsb))) {
			if (verbose)
				pr_err("Bad backup-metadata checksum on %s\n",
				       devname);
			continue; /* bad checksum */
		}
		if (memcmp(bsb.magic, "md_backup_data-2", 16) == 0 &&
		    bsb.sb_csum2 != bsb_csum((char*)&bsb, ((char*)&bsb.sb_csum2)-((char*)&bsb))) {
			if (verbose)
				pr_err("Bad backup-metadata checksum2 on %s\n",
				       devname);
			continue; /* Bad second checksum */
		}
		if (memcmp(bsb.set_uuid,info->uuid, 16) != 0) {
			if (verbose)
				pr_err("Wrong uuid on backup-metadata on %s\n",
				       devname);
			continue; /* Wrong uuid */
		}

		/*
		 * array utime and backup-mtime should be updated at
		 * much the same time, but it seems that sometimes
		 * they aren't... So allow considerable flexability in
		 * matching, and allow this test to be overridden by
		 * an environment variable.
		 */
		if(time_after(info->array.utime, (unsigned int)__le64_to_cpu(bsb.mtime) + 2*60*60) ||
		   time_before(info->array.utime, (unsigned int)__le64_to_cpu(bsb.mtime) - 10*60)) {
			if (check_env("MDADM_GROW_ALLOW_OLD")) {
				pr_err("accepting backup with timestamp %lu for array with timestamp %lu\n",
					(unsigned long)__le64_to_cpu(bsb.mtime),
					(unsigned long)info->array.utime);
			} else {
				pr_err("too-old timestamp on backup-metadata on %s\n", devname);
				pr_err("If you think it is should be safe, try 'export MDADM_GROW_ALLOW_OLD=1'\n");
				continue; /* time stamp is too bad */
			}
		}

		if (bsb.magic[15] == '1') {
			if (bsb.length == 0)
				continue;
			if (info->delta_disks >= 0) {
				/* reshape_progress is increasing */
				if (__le64_to_cpu(bsb.arraystart)
				    + __le64_to_cpu(bsb.length)
				    < info->reshape_progress) {
				nonew:
					if (verbose)
						pr_err("backup-metadata found on %s but is not needed\n", devname);
					continue; /* No new data here */
				}
			} else {
				/* reshape_progress is decreasing */
				if (__le64_to_cpu(bsb.arraystart) >=
				    info->reshape_progress)
					goto nonew; /* No new data here */
			}
		} else {
			if (bsb.length == 0 && bsb.length2 == 0)
				continue;
			if (info->delta_disks >= 0) {
				/* reshape_progress is increasing */
				if ((__le64_to_cpu(bsb.arraystart)
				     + __le64_to_cpu(bsb.length)
				     < info->reshape_progress) &&
				    (__le64_to_cpu(bsb.arraystart2)
				     + __le64_to_cpu(bsb.length2)
				     < info->reshape_progress))
					goto nonew; /* No new data here */
			} else {
				/* reshape_progress is decreasing */
				if (__le64_to_cpu(bsb.arraystart) >=
				    info->reshape_progress &&
				    __le64_to_cpu(bsb.arraystart2) >=
				    info->reshape_progress)
					goto nonew; /* No new data here */
			}
		}
		if (lseek64(fd, __le64_to_cpu(bsb.devstart)*512, 0)< 0) {
		second_fail:
			if (verbose)
				pr_err("Failed to verify secondary backup-metadata block on %s\n",
				       devname);
			continue; /* Cannot seek */
		}
		/* There should be a duplicate backup superblock 4k before here */
		if (lseek64(fd, -4096, 1) < 0 ||
		    read(fd, &bsb2, sizeof(bsb2)) != sizeof(bsb2))
			goto second_fail; /* Cannot find leading superblock */
		if (bsb.magic[15] == '1')
			bsbsize = offsetof(struct mdp_backup_super, pad1);
		else
			bsbsize = offsetof(struct mdp_backup_super, pad);
		if (memcmp(&bsb2, &bsb, bsbsize) != 0)
			goto second_fail; /* Cannot find leading superblock */

		/* Now need the data offsets for all devices. */
		offsets = xmalloc(sizeof(*offsets)*info->array.raid_disks);
		for(j=0; j<info->array.raid_disks; j++) {
			if (fdlist[j] < 0)
				continue;
			if (st->ss->load_super(st, fdlist[j], NULL))
				/* FIXME should be this be an error */
				continue;
			st->ss->getinfo_super(st, &dinfo, NULL);
			st->ss->free_super(st);
			offsets[j] = dinfo.data_offset * 512;
		}
		printf("%s: restoring critical section\n", Name);

		if (restore_stripes(fdlist, offsets, info->array.raid_disks,
				    info->new_chunk, info->new_level,
				    info->new_layout, fd,
				    __le64_to_cpu(bsb.devstart)*512,
				    __le64_to_cpu(bsb.arraystart)*512,
				    __le64_to_cpu(bsb.length)*512, NULL)) {
			/* didn't succeed, so giveup */
			if (verbose)
				pr_err("Error restoring backup from %s\n",
					devname);
			free(offsets);
			return 1;
		}

		if (bsb.magic[15] == '2' &&
		    restore_stripes(fdlist, offsets, info->array.raid_disks,
				    info->new_chunk, info->new_level,
				    info->new_layout, fd,
				    __le64_to_cpu(bsb.devstart)*512 +
				    __le64_to_cpu(bsb.devstart2)*512,
				    __le64_to_cpu(bsb.arraystart2)*512,
				    __le64_to_cpu(bsb.length2)*512, NULL)) {
			/* didn't succeed, so giveup */
			if (verbose)
				pr_err("Error restoring second backup from %s\n",
					devname);
			free(offsets);
			return 1;
		}

		free(offsets);

		/* Ok, so the data is restored. Let's update those superblocks. */

		lo = hi = 0;
		if (bsb.length) {
			lo = __le64_to_cpu(bsb.arraystart);
			hi = lo + __le64_to_cpu(bsb.length);
		}
		if (bsb.magic[15] == '2' && bsb.length2) {
			unsigned long long lo1, hi1;
			lo1 = __le64_to_cpu(bsb.arraystart2);
			hi1 = lo1 + __le64_to_cpu(bsb.length2);
			if (lo == hi) {
				lo = lo1;
				hi = hi1;
			} else if (lo < lo1)
				hi = hi1;
			else
				lo = lo1;
		}
		if (lo < hi && (info->reshape_progress < lo ||
				info->reshape_progress > hi))
			/* backup does not affect reshape_progress*/ ;
		else if (info->delta_disks >= 0) {
			info->reshape_progress = __le64_to_cpu(bsb.arraystart) +
				__le64_to_cpu(bsb.length);
			if (bsb.magic[15] == '2') {
				unsigned long long p2;

				p2 = __le64_to_cpu(bsb.arraystart2) +
					__le64_to_cpu(bsb.length2);
				if (p2 > info->reshape_progress)
					info->reshape_progress = p2;
			}
		} else {
			info->reshape_progress = __le64_to_cpu(bsb.arraystart);
			if (bsb.magic[15] == '2') {
				unsigned long long p2;

				p2 = __le64_to_cpu(bsb.arraystart2);
				if (p2 < info->reshape_progress)
					info->reshape_progress = p2;
			}
		}
		for (j=0; j<info->array.raid_disks; j++) {
			if (fdlist[j] < 0)
				continue;
			if (st->ss->load_super(st, fdlist[j], NULL))
				continue;
			st->ss->getinfo_super(st, &dinfo, NULL);
			dinfo.reshape_progress = info->reshape_progress;
			st->ss->update_super(st, &dinfo, "_reshape_progress",
					     NULL,0, 0, NULL);
			st->ss->store_super(st, fdlist[j]);
			st->ss->free_super(st);
		}
		return 0;
	}
	/* Didn't find any backup data, try to see if any
	 * was needed.
	 */
	if (info->delta_disks < 0) {
		/* When shrinking, the critical section is at the end.
		 * So see if we are before the critical section.
		 */
		unsigned long long first_block;
		nstripe = ostripe = 0;
		first_block = 0;
		while (ostripe >= nstripe) {
			ostripe += info->array.chunk_size / 512;
			first_block = ostripe * odata;
			nstripe = first_block / ndata / (info->new_chunk/512) *
				(info->new_chunk/512);
		}

		if (info->reshape_progress >= first_block)
			return 0;
	}
	if (info->delta_disks > 0) {
		/* See if we are beyond the critical section. */
		unsigned long long last_block;
		nstripe = ostripe = 0;
		last_block = 0;
		while (nstripe >= ostripe) {
			nstripe += info->new_chunk / 512;
			last_block = nstripe * ndata;
			ostripe = last_block / odata / (info->array.chunk_size/512) *
				(info->array.chunk_size/512);
		}

		if (info->reshape_progress >= last_block)
			return 0;
	}
	/* needed to recover critical section! */
	if (verbose)
		pr_err("Failed to find backup of critical section\n");
	return 1;
}

int Grow_continue_command(char *devname, int fd,
			  char *backup_file, int verbose)
{
	int ret_val = 0;
	struct supertype *st = NULL;
	struct mdinfo *content = NULL;
	struct mdinfo array;
	char *subarray = NULL;
	struct mdinfo *cc = NULL;
	struct mdstat_ent *mdstat = NULL;
	int cfd = -1;
	int fd2;

	dprintf("Grow continue from command line called for %s\n", devname);

	st = super_by_fd(fd, &subarray);
	if (!st || !st->ss) {
		pr_err("Unable to determine metadata format for %s\n", devname);
		return 1;
	}
	dprintf("Grow continue is run for ");
	if (st->ss->external == 0) {
		int d;
		int cnt = 5;
		dprintf_cont("native array (%s)\n", devname);
		if (md_get_array_info(fd, &array.array) < 0) {
			pr_err("%s is not an active md array - aborting\n",
			       devname);
			ret_val = 1;
			goto Grow_continue_command_exit;
		}
		content = &array;
		sysfs_init(content, fd, NULL);
		/* Need to load a superblock.
		 * FIXME we should really get what we need from
		 * sysfs
		 */
		do {
			for (d = 0; d < MAX_DISKS; d++) {
				mdu_disk_info_t disk;
				char *dv;
				int err;
				disk.number = d;
				if (md_get_disk_info(fd, &disk) < 0)
					continue;
				if (disk.major == 0 && disk.minor == 0)
					continue;
				if ((disk.state & (1 << MD_DISK_ACTIVE)) == 0)
					continue;
				dv = map_dev(disk.major, disk.minor, 1);
				if (!dv)
					continue;
				fd2 = dev_open(dv, O_RDONLY);
				if (fd2 < 0)
					continue;
				err = st->ss->load_super(st, fd2, NULL);
				close(fd2);
				if (err)
					continue;
				break;
			}
			if (d == MAX_DISKS) {
				pr_err("Unable to load metadata for %s\n",
				       devname);
				ret_val = 1;
				goto Grow_continue_command_exit;
			}
			st->ss->getinfo_super(st, content, NULL);
			if (!content->reshape_active)
				sleep(3);
			else
				break;
		} while (cnt-- > 0);
	} else {
		char *container;

		if (subarray) {
			dprintf_cont("subarray (%s)\n", subarray);
			container = st->container_devnm;
			cfd = open_dev_excl(st->container_devnm);
		} else {
			container = st->devnm;
			close(fd);
			cfd = open_dev_excl(st->devnm);
			dprintf_cont("container (%s)\n", container);
			fd = cfd;
		}
		if (cfd < 0) {
			pr_err("Unable to open container for %s\n", devname);
			ret_val = 1;
			goto Grow_continue_command_exit;
		}

		/* find in container array under reshape
		 */
		ret_val = st->ss->load_container(st, cfd, NULL);
		if (ret_val) {
			pr_err("Cannot read superblock for %s\n", devname);
			ret_val = 1;
			goto Grow_continue_command_exit;
		}

		cc = st->ss->container_content(st, subarray);
		for (content = cc; content ; content = content->next) {
			char *array_name;
			int allow_reshape = 1;

			if (content->reshape_active == 0)
				continue;
			/* The decision about array or container wide
			 * reshape is taken in Grow_continue based
			 * content->reshape_active state, therefore we
			 * need to check_reshape based on
			 * reshape_active and subarray name
			 */
			if (content->array.state & (1<<MD_SB_BLOCK_VOLUME))
				allow_reshape = 0;
			if (content->reshape_active == CONTAINER_RESHAPE &&
			    (content->array.state
			     & (1<<MD_SB_BLOCK_CONTAINER_RESHAPE)))
				allow_reshape = 0;

			if (!allow_reshape) {
				pr_err("cannot continue reshape of an array in container with unsupported metadata: %s(%s)\n",
				       devname, container);
				ret_val = 1;
				goto Grow_continue_command_exit;
			}

			array_name = strchr(content->text_version+1, '/')+1;
			mdstat = mdstat_by_subdev(array_name, container);
			if (!mdstat)
				continue;
			if (mdstat->active == 0) {
				pr_err("Skipping inactive array %s.\n",
				       mdstat->devnm);
				free_mdstat(mdstat);
				mdstat = NULL;
				continue;
			}
			break;
		}
		if (!content) {
			pr_err("Unable to determine reshaped array for %s\n", devname);
			ret_val = 1;
			goto Grow_continue_command_exit;
		}
		fd2 = open_dev(mdstat->devnm);
		if (fd2 < 0) {
			pr_err("cannot open (%s)\n", mdstat->devnm);
			ret_val = 1;
			goto Grow_continue_command_exit;
		}

		if (sysfs_init(content, fd2, mdstat->devnm)) {
			pr_err("Unable to initialize sysfs for %s, Grow cannot continue.\n",
			       mdstat->devnm);
			ret_val = 1;
			close(fd2);
			goto Grow_continue_command_exit;
		}

		close(fd2);

		/* start mdmon in case it is not running
		 */
		if (!mdmon_running(container))
			start_mdmon(container);
		ping_monitor(container);

		if (mdmon_running(container))
			st->update_tail = &st->updates;
		else {
			pr_err("No mdmon found. Grow cannot continue.\n");
			ret_val = 1;
			goto Grow_continue_command_exit;
		}
	}

	/* verify that array under reshape is started from
	 * correct position
	 */
	if (verify_reshape_position(content, content->array.level) < 0) {
		ret_val = 1;
		goto Grow_continue_command_exit;
	}

	/* continue reshape
	 */
	ret_val = Grow_continue(fd, st, content, backup_file, 1, 0);

Grow_continue_command_exit:
	if (cfd > -1)
		close(cfd);
	st->ss->free_super(st);
	free_mdstat(mdstat);
	sysfs_free(cc);
	free(subarray);

	return ret_val;
}

int Grow_continue(int mdfd, struct supertype *st, struct mdinfo *info,
		  char *backup_file, int forked, int freeze_reshape)
{
	int ret_val = 2;

	if (!info->reshape_active)
		return ret_val;

	if (st->ss->external) {
		int cfd = open_dev(st->container_devnm);

		if (cfd < 0)
			return 1;

		st->ss->load_container(st, cfd, st->container_devnm);
		close(cfd);
		ret_val = reshape_container(st->container_devnm, NULL, mdfd,
					    st, info, 0, backup_file, 0,
					    forked, 1 | info->reshape_active,
					    freeze_reshape);
	} else
		ret_val = reshape_array(NULL, mdfd, "array", st, info, 1,
					NULL, INVALID_SECTORS, backup_file,
					0, forked, 1 | info->reshape_active,
					freeze_reshape);

	return ret_val;
}

char *make_backup(char *name)
{
	char *base = "backup_file-";
	int len;
	char *fname;

	len = strlen(MAP_DIR) + 1 + strlen(base) + strlen(name)+1;
	fname = xmalloc(len);
	sprintf(fname, "%s/%s%s", MAP_DIR, base, name);
	return fname;
}

char *locate_backup(char *name)
{
	char *fl = make_backup(name);
	struct stat stb;

	if (stat(fl, &stb) == 0 && S_ISREG(stb.st_mode))
		return fl;

	free(fl);
	return NULL;
}
