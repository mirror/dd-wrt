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
#include	"md_p.h"
#include	"md_u.h"
#include	<ctype.h>
#include	<dirent.h>

static int cmpstringp(const void *p1, const void *p2)
{
	return strcmp(* (char * const *) p1, * (char * const *) p2);
}

static int add_device(const char *dev, char ***p_devices,
		      int *p_max_devices, int n_devices)
{
	if (n_devices + 1 >= *p_max_devices) {
		*p_max_devices += 16;
		*p_devices = xrealloc(*p_devices, *p_max_devices *
				      sizeof(**p_devices));
		if (!*p_devices) {
			*p_max_devices = 0;
			return 0;
		}
	};
	(*p_devices)[n_devices] = xstrdup(dev);
	return n_devices + 1;
}

int Detail(char *dev, struct context *c)
{
	/*
	 * Print out details for an md array
	 */
	int fd = open(dev, O_RDONLY);
	mdu_array_info_t array;
	mdu_disk_info_t *disks;
	int next;
	int d;
	time_t atime;
	char *str;
	char **devices = NULL;
	int max_devices = 0, n_devices = 0;
	int spares = 0;
	struct stat stb;
	int failed = 0;
	struct supertype *st;
	char *subarray = NULL;
	int max_disks = MD_SB_DISKS; /* just a default */
	struct mdinfo *info = NULL;
	struct mdinfo *sra;
	struct mdinfo *subdev;
	char *member = NULL;
	char *container = NULL;

	int rv = c->test ? 4 : 1;
	int avail_disks = 0;
	char *avail = NULL;
	int external;
	int inactive;
	int is_container = 0;

	if (fd < 0) {
		pr_err("cannot open %s: %s\n",
			dev, strerror(errno));
		return rv;
	}
	sra = sysfs_read(fd, NULL, GET_VERSION | GET_DEVS |
			GET_ARRAY_STATE | GET_STATE);
	if (!sra) {
		if (md_get_array_info(fd, &array)) {
			pr_err("%s does not appear to be an md device\n", dev);
			close(fd);
			return rv;
		}
	}
	external = (sra != NULL && sra->array.major_version == -1 &&
		    sra->array.minor_version == -2);
	inactive = (sra != NULL && !md_array_is_active(sra));
	st = super_by_fd(fd, &subarray);
	if (md_get_array_info(fd, &array)) {
		if (errno == ENODEV) {
			if (sra->array.major_version == -1 &&
			    sra->array.minor_version == -1 &&
			    sra->devs == NULL) {
				pr_err("Array associated with md device %s does not exist.\n",
				       dev);
				close(fd);
				sysfs_free(sra);
				return rv;
			}
			array = sra->array;
		} else {
			pr_err("cannot get array detail for %s: %s\n",
			       dev, strerror(errno));
			close(fd);
			return rv;
		}
	}

	if (array.raid_disks == 0 && external)
		is_container = 1;
	if (fstat(fd, &stb) != 0 && !S_ISBLK(stb.st_mode))
		stb.st_rdev = 0;
	rv = 0;

	if (st)
		max_disks = st->max_devs;

	if (subarray) {
		/* This is a subarray of some container.
		 * We want the name of the container, and the member
		 */
		dev_t devid = devnm2devid(st->container_devnm);
		int cfd, err;

		member = subarray;
		container = map_dev_preferred(major(devid), minor(devid),
					      1, c->prefer);
		cfd = open_dev(st->container_devnm);
		if (cfd >= 0) {
			err = st->ss->load_container(st, cfd, NULL);
			close(cfd);
			if (err == 0)
				info = st->ss->container_content(st, subarray);
		}
	}

	/* try to load a superblock. Try sra->devs first, then try ioctl */
	if (st && !info)
		for (d = 0, subdev = sra ? sra->devs : NULL;
		     d < max_disks || subdev;
		     subdev ? (void)(subdev = subdev->next) : (void)(d++)){
		mdu_disk_info_t disk;
		char *dv;
		int fd2;
		int err;

		if (subdev)
			disk = subdev->disk;
		else {
			disk.number = d;
			if (md_get_disk_info(fd, &disk) < 0)
				continue;
			if (d >= array.raid_disks &&
			    disk.major == 0 && disk.minor == 0)
				continue;
		}

		if (array.raid_disks > 0 &&
		    (disk.state & (1 << MD_DISK_ACTIVE)) == 0)
			continue;

		dv = map_dev(disk.major, disk.minor, 1);
		if (!dv)
			continue;

		fd2 = dev_open(dv, O_RDONLY);
		if (fd2 < 0)
			continue;

		if (st->sb)
			st->ss->free_super(st);

		err = st->ss->load_super(st, fd2, NULL);
		close(fd2);
		if (err)
			continue;
		if (info)
			free(info);
		if (subarray)
			info = st->ss->container_content(st, subarray);
		else {
			info = xmalloc(sizeof(*info));
			st->ss->getinfo_super(st, info, NULL);
		}
		if (!info)
			continue;

		if (array.raid_disks != 0 && /* container */
		    (info->array.ctime != array.ctime ||
		     info->array.level != array.level)) {
			st->ss->free_super(st);
			continue;
		}
		/* some formats (imsm) have free-floating-spares
		 * with a uuid of uuid_zero, they don't
		 * have very good info about the rest of the
		 * container, so keep searching when
		 * encountering such a device.  Otherwise, stop
		 * after the first successful call to
		 * ->load_super.
		 */
		if (memcmp(uuid_zero,
			   info->uuid,
			   sizeof(uuid_zero)) == 0) {
			st->ss->free_super(st);
			continue;
		}
		break;
	}

	/* Ok, we have some info to print... */
	str = map_num(pers, array.level);

	if (c->export) {
		if (array.raid_disks) {
			if (str)
				printf("MD_LEVEL=%s\n", str);
			printf("MD_DEVICES=%d\n", array.raid_disks);
		} else {
			if (is_container)
				printf("MD_LEVEL=container\n");
			printf("MD_DEVICES=%d\n", array.nr_disks);
		}
		if (container) {
			printf("MD_CONTAINER=%s\n", container);
			printf("MD_MEMBER=%s\n", member);
		} else {
			if (sra && sra->array.major_version < 0)
				printf("MD_METADATA=%s\n", sra->text_version);
			else
				printf("MD_METADATA=%d.%d\n",
				       array.major_version,
				       array.minor_version);
		}

		if (st && st->sb && info) {
			char nbuf[64];
			struct map_ent *mp, *map = NULL;

			fname_from_uuid(st, info, nbuf, ':');
			printf("MD_UUID=%s\n", nbuf + 5);
			mp = map_by_uuid(&map, info->uuid);
			if (mp && mp->path &&
			    strncmp(mp->path, "/dev/md/", 8) == 0) {
				printf("MD_DEVNAME=");
				print_escape(mp->path + 8);
				putchar('\n');
			}

			if (st->ss->export_detail_super)
				st->ss->export_detail_super(st);
			map_free(map);
		} else {
			struct map_ent *mp, *map = NULL;
			char nbuf[64];
			mp = map_by_devnm(&map, fd2devnm(fd));
			if (mp) {
				__fname_from_uuid(mp->uuid, 0, nbuf, ':');
				printf("MD_UUID=%s\n", nbuf+5);
			}
			if (mp && mp->path &&
			    strncmp(mp->path, "/dev/md/", 8) == 0) {
				printf("MD_DEVNAME=");
				print_escape(mp->path+8);
				putchar('\n');
			}
			map_free(map);
		}
		if (sra) {
			struct mdinfo *mdi;
			for (mdi  = sra->devs; mdi; mdi = mdi->next) {
				char *path;
				char *sysdev = xstrdup(mdi->sys_name + 1);
				char *cp;

				path = map_dev(mdi->disk.major,
					       mdi->disk.minor, 0);
				for (cp = sysdev; *cp; cp++)
					if (!isalnum(*cp))
						*cp = '_';

				if (mdi->disk.raid_disk >= 0)
					printf("MD_DEVICE_%s_ROLE=%d\n",
					       sysdev,
					       mdi->disk.raid_disk);
				else
					printf("MD_DEVICE_%s_ROLE=spare\n",
					       sysdev);
				if (path)
					printf("MD_DEVICE_%s_DEV=%s\n",
					       sysdev, path);
			}
		}
		goto out;
	}

	disks = xmalloc(max_disks * 2 * sizeof(mdu_disk_info_t));
	for (d = 0; d < max_disks * 2; d++) {
		disks[d].state = (1 << MD_DISK_REMOVED);
		disks[d].major = disks[d].minor = 0;
		disks[d].number = -1;
		disks[d].raid_disk = d / 2;
	}

	next = array.raid_disks * 2;
	if (inactive) {
		struct mdinfo *mdi;
		for (mdi = sra->devs; mdi; mdi = mdi->next) {
			disks[next++] = mdi->disk;
			disks[next - 1].number = -1;
		}
	} else for (d = 0; d < max_disks; d++) {
		mdu_disk_info_t disk;
		disk.number = d;
		if (md_get_disk_info(fd, &disk) < 0) {
			if (d < array.raid_disks)
				pr_err("cannot get device detail for device %d: %s\n",
					d, strerror(errno));
			continue;
		}
		if (disk.major == 0 && disk.minor == 0)
			continue;
		if (disk.raid_disk >= 0 && disk.raid_disk < array.raid_disks &&
		    disks[disk.raid_disk * 2].state == (1 << MD_DISK_REMOVED) &&
		    ((disk.state & (1 << MD_DISK_JOURNAL)) == 0))
			disks[disk.raid_disk * 2] = disk;
		else if (disk.raid_disk >= 0 &&
			 disk.raid_disk < array.raid_disks &&
			 disks[disk.raid_disk * 2 + 1].state ==
			 (1 << MD_DISK_REMOVED) &&
			 !(disk.state & (1 << MD_DISK_JOURNAL)))
			disks[disk.raid_disk * 2 + 1] = disk;
		else if (next < max_disks * 2)
			disks[next++] = disk;
	}

	avail = xcalloc(array.raid_disks, 1);

	for (d = 0; d < array.raid_disks; d++) {

		if ((disks[d*2].state & (1<<MD_DISK_SYNC)) ||
		    (disks[d*2+1].state & (1<<MD_DISK_SYNC))) {
			avail_disks ++;
			avail[d] = 1;
		} else
			rv |= !! c->test;
	}

	if (c->brief) {
		mdu_bitmap_file_t bmf;
		if (inactive && !is_container)
			printf("INACTIVE-ARRAY %s", dev);
		else
			printf("ARRAY %s", dev);
		if (c->verbose > 0) {
			if (array.raid_disks)
				printf(" level=%s num-devices=%d",
				       str ? str : "-unknown-",
				       array.raid_disks);
			else if (is_container)
				printf(" level=container num-devices=%d",
				       array.nr_disks);
			else
				printf(" num-devices=%d", array.nr_disks);
		}
		if (container) {
			printf(" container=%s", container);
			printf(" member=%s", member);
		} else {
			if (sra && sra->array.major_version < 0)
				printf(" metadata=%s", sra->text_version);
			else
				printf(" metadata=%d.%d", array.major_version,
				       array.minor_version);
		}

		/* Only try GET_BITMAP_FILE for 0.90.01 and later */
		if (ioctl(fd, GET_BITMAP_FILE, &bmf) == 0 && bmf.pathname[0]) {
			printf(" bitmap=%s", bmf.pathname);
		}
	} else {
		mdu_bitmap_file_t bmf;
		unsigned long long larray_size;
		struct mdstat_ent *ms = mdstat_read(0, 0);
		struct mdstat_ent *e;
		char *devnm;

		devnm = stat2devnm(&stb);
		for (e = ms; e; e = e->next)
			if (strcmp(e->devnm, devnm) == 0)
				break;
		if (!get_dev_size(fd, NULL, &larray_size))
			larray_size = 0;

		printf("%s:\n", dev);

		if (container)
			printf("         Container : %s, member %s\n",
			       container, member);
		else {
			if (sra && sra->array.major_version < 0)
				printf("           Version : %s\n",
				       sra->text_version);
			else
				printf("           Version : %d.%d\n",
				       array.major_version,
				       array.minor_version);
		}

		atime = array.ctime;
		if (atime)
			printf("     Creation Time : %.24s\n", ctime(&atime));
		if (is_container)
			str = "container";
		if (str)
			printf("        Raid Level : %s\n", str);
		if (larray_size)
			printf("        Array Size : %llu%s\n",
			       (larray_size >> 10),
			       human_size(larray_size));
		if (array.level >= 1) {
			if (sra)
				array.major_version = sra->array.major_version;
			if (array.major_version != 0 &&
			    (larray_size >= 0xFFFFFFFFULL|| array.size == 0)) {
				unsigned long long dsize;

				dsize = get_component_size(fd);
				if (dsize > 0)
					printf("     Used Dev Size : %llu%s\n",
					       dsize/2,
					 human_size((long long)dsize<<9));
				else
					printf("     Used Dev Size : unknown\n");
			} else
				printf("     Used Dev Size : %lu%s\n",
				       (unsigned long)array.size,
				       human_size((unsigned long long)
						  array.size << 10));
		}
		if (array.raid_disks)
			printf("      Raid Devices : %d\n", array.raid_disks);
		printf("     Total Devices : %d\n", array.nr_disks);
		if (!container &&
		    ((sra == NULL && array.major_version == 0) ||
		     (sra && sra->array.major_version == 0)))
			printf("   Preferred Minor : %d\n", array.md_minor);
		if (sra == NULL || sra->array.major_version >= 0)
			printf("       Persistence : Superblock is %spersistent\n",
			       array.not_persistent ? "not " : "");
		printf("\n");
		/* Only try GET_BITMAP_FILE for 0.90.01 and later */
		if (ioctl(fd, GET_BITMAP_FILE, &bmf) == 0 && bmf.pathname[0]) {
			printf("     Intent Bitmap : %s\n", bmf.pathname);
			printf("\n");
		} else if (array.state & (1<<MD_SB_BITMAP_PRESENT))
			printf("     Intent Bitmap : Internal\n\n");
		atime = array.utime;
		if (atime)
			printf("       Update Time : %.24s\n", ctime(&atime));
		if (array.raid_disks) {
			static char *sync_action[] = {
				", recovering",  ", resyncing",
				", reshaping",   ", checking"  };
			char *st;
			if (avail_disks == array.raid_disks)
				st = "";
			else if (!enough(array.level, array.raid_disks,
					 array.layout, 1, avail))
				st = ", FAILED";
			else
				st = ", degraded";

			printf("             State : %s%s%s%s%s%s \n",
			       (array.state & (1 << MD_SB_CLEAN)) ?
			       "clean" : "active", st,
			       (!e || (e->percent < 0 &&
				       e->percent != RESYNC_PENDING &&
				       e->percent != RESYNC_DELAYED)) ?
			       "" : sync_action[e->resync],
			       larray_size ? "": ", Not Started",
			       (e && e->percent == RESYNC_DELAYED) ?
			       " (DELAYED)": "",
			       (e && e->percent == RESYNC_PENDING) ?
			       " (PENDING)": "");
		} else if (inactive && !is_container) {
			printf("             State : inactive\n");
		}
		if (array.raid_disks)
			printf("    Active Devices : %d\n", array.active_disks);
		if (array.working_disks > 0)
			printf("   Working Devices : %d\n",
			       array.working_disks);
		if (array.raid_disks) {
			printf("    Failed Devices : %d\n", array.failed_disks);
			printf("     Spare Devices : %d\n", array.spare_disks);
		}
		printf("\n");
		if (array.level == 5) {
			str = map_num(r5layout, array.layout);
			printf("            Layout : %s\n",
			       str ? str : "-unknown-");
		}
		if (array.level == 6) {
			str = map_num(r6layout, array.layout);
			printf("            Layout : %s\n",
			       str ? str : "-unknown-");
		}
		if (array.level == 10) {
			printf("            Layout :");
			print_r10_layout(array.layout);
			printf("\n");
		}
		switch (array.level) {
		case 0:
		case 4:
		case 5:
		case 10:
		case 6:
			if (array.chunk_size)
				printf("        Chunk Size : %dK\n\n",
				       array.chunk_size/1024);
			break;
		case -1:
			printf("          Rounding : %dK\n\n",
			       array.chunk_size/1024);
			break;
		default:
			break;
		}

		if (array.raid_disks) {
			struct mdinfo *mdi;

			mdi = sysfs_read(fd, NULL, GET_CONSISTENCY_POLICY);
			if (mdi) {
				char *policy = map_num(consistency_policies,
						       mdi->consistency_policy);
				sysfs_free(mdi);
				if (policy)
					printf("Consistency Policy : %s\n\n",
					       policy);
			}
		}

		if (e && e->percent >= 0) {
			static char *sync_action[] = {
				"Rebuild", "Resync", "Reshape", "Check"};
			printf("    %7s Status : %d%% complete\n",
			       sync_action[e->resync], e->percent);
		}

		if ((st && st->sb) && (info && info->reshape_active)) {
#if 0
This is pretty boring
			printf("     Reshape pos'n : %llu%s\n",
			       (unsigned long long) info->reshape_progress << 9,
			       human_size((unsigned long long)
					  info->reshape_progress << 9));
#endif
			if (info->delta_disks != 0)
				printf("     Delta Devices : %d, (%d->%d)\n",
				       info->delta_disks,
				       array.raid_disks - info->delta_disks,
				       array.raid_disks);
			if (info->new_level != array.level) {
				str = map_num(pers, info->new_level);
				printf("         New Level : %s\n",
				       str ? str : "-unknown-");
			}
			if (info->new_level != array.level ||
			    info->new_layout != array.layout) {
				if (info->new_level == 5) {
					str = map_num(r5layout,
						      info->new_layout);
					printf("        New Layout : %s\n",
					       str ? str : "-unknown-");
				}
				if (info->new_level == 6) {
					str = map_num(r6layout,
						      info->new_layout);
					printf("        New Layout : %s\n",
					       str ? str : "-unknown-");
				}
				if (info->new_level == 10) {
					printf("        New Layout : near=%d, %s=%d\n",
					       info->new_layout & 255,
					       (info->new_layout & 0x10000) ?
					       "offset" : "far",
					       (info->new_layout >> 8) & 255);
				}
			}
			if (info->new_chunk != array.chunk_size)
				printf("     New Chunksize : %dK\n",
				       info->new_chunk/1024);
			printf("\n");
		} else if (e && e->percent >= 0)
			printf("\n");
		free_mdstat(ms);

		if (st && st->sb)
			st->ss->detail_super(st, c->homehost);

		if (array.raid_disks == 0 && sra &&
		    sra->array.major_version == -1 &&
		    sra->array.minor_version == -2 &&
		    sra->text_version[0] != '/') {
			/* This looks like a container.  Find any active arrays
			 * That claim to be a member.
			 */
			DIR *dir = opendir("/sys/block");
			struct dirent *de;

			printf("     Member Arrays :");

			while (dir && (de = readdir(dir)) != NULL) {
				char path[287];
				char vbuf[1024];
				int nlen = strlen(sra->sys_name);
				dev_t devid;
				if (de->d_name[0] == '.')
					continue;
				sprintf(path,
					"/sys/block/%s/md/metadata_version",
					de->d_name);
				if (load_sys(path, vbuf, sizeof(vbuf)) < 0)
					continue;
				if (strncmp(vbuf, "external:", 9) ||
				    !is_subarray(vbuf + 9) ||
				    strncmp(vbuf + 10, sra->sys_name, nlen) ||
				    vbuf[10 + nlen] != '/')
					continue;
				devid = devnm2devid(de->d_name);
				printf(" %s",
				       map_dev_preferred(major(devid),
							 minor(devid), 1,
							 c->prefer));
			}
			if (dir)
				closedir(dir);
			printf("\n\n");
		}

		if (array.raid_disks)
			printf("    Number   Major   Minor   RaidDevice State\n");
		else
			printf("    Number   Major   Minor   RaidDevice\n");
	}
	free(info);

	for (d = 0; d < max_disks * 2; d++) {
		char *dv;
		mdu_disk_info_t disk = disks[d];

		if (d >= array.raid_disks * 2 &&
		    disk.major == 0 && disk.minor == 0)
			continue;
		if ((d & 1) && disk.major == 0 && disk.minor == 0)
			continue;
		if (!c->brief) {
			if (d == array.raid_disks*2)
				printf("\n");
			if (disk.number < 0 && disk.raid_disk < 0)
				printf("       -   %5d    %5d        -     ",
				       disk.major, disk.minor);
			else if (disk.raid_disk < 0 ||
				 disk.state & (1 << MD_DISK_JOURNAL))
				printf("   %5d   %5d    %5d        -     ",
				       disk.number, disk.major, disk.minor);
			else if (disk.number < 0)
				printf("       -   %5d    %5d    %5d     ",
				       disk.major, disk.minor, disk.raid_disk);
			else
				printf("   %5d   %5d    %5d    %5d     ",
				       disk.number, disk.major, disk.minor,
				       disk.raid_disk);
		}
		if (!c->brief && array.raid_disks) {
			if (disk.state & (1 << MD_DISK_FAULTY)) {
				printf(" faulty");
				if (disk.raid_disk < array.raid_disks &&
				    disk.raid_disk >= 0)
					failed++;
			}
			if (disk.state & (1 << MD_DISK_ACTIVE))
				printf(" active");
			if (disk.state & (1 << MD_DISK_SYNC)) {
				printf(" sync");
				if (array.level == 10 &&
				    (array.layout & ~0x1FFFF) == 0) {
					int nc = array.layout & 0xff;
					int fc = (array.layout >> 8) & 0xff;
					int copies = nc*fc;
					if (fc == 1 &&
					    array.raid_disks % copies == 0 &&
					    copies <= 26) {
						/* We can divide the devices
						   into 'sets' */
						int set;
						set = disk.raid_disk % copies;
						printf(" set-%c", set + 'A');
					}
				}
			}
			if (disk.state & (1 << MD_DISK_REMOVED))
				printf(" removed");
			if (disk.state & (1 << MD_DISK_WRITEMOSTLY))
				printf(" writemostly");
			if (disk.state & (1 << MD_DISK_FAILFAST))
				printf(" failfast");
			if (disk.state & (1 << MD_DISK_JOURNAL))
				printf(" journal");
			if ((disk.state &
			     ((1 << MD_DISK_ACTIVE) | (1 << MD_DISK_SYNC) |
			      (1 << MD_DISK_REMOVED) | (1 << MD_DISK_FAULTY) |
			      (1 << MD_DISK_JOURNAL))) == 0) {
				printf(" spare");
				if (disk.raid_disk < array.raid_disks &&
				    disk.raid_disk >= 0)
					printf(" rebuilding");
			}
		}
		if (disk.state == 0)
			spares++;
		dv = map_dev_preferred(disk.major, disk.minor, 0, c->prefer);
		if (dv != NULL) {
			if (c->brief)
				n_devices = add_device(dv, &devices,
						       &max_devices, n_devices);
			else
				printf("   %s", dv);
		}
		if (!c->brief)
			printf("\n");
	}
	if (spares && c->brief && array.raid_disks)
		printf(" spares=%d", spares);
	if (c->brief && st && st->sb)
		st->ss->brief_detail_super(st);
	if (st)
		st->ss->free_super(st);

	if (c->brief && c->verbose > 0 && devices) {
		qsort(devices, n_devices, sizeof(*devices), cmpstringp);
		printf("\n   devices=%s", devices[0]);
		for (d = 1; d < n_devices; d++)
			printf(",%s", devices[d]);
	}
	if (c->brief)
		printf("\n");
	if (c->test &&
	    !enough(array.level, array.raid_disks, array.layout, 1, avail))
		rv = 2;

	free(disks);
out:
	close(fd);
	free(subarray);
	free(avail);
	for (d = 0; d < n_devices; d++)
		free(devices[d]);
	free(devices);
	sysfs_free(sra);
	return rv;
}

int Detail_Platform(struct superswitch *ss, int scan, int verbose, int export, char *controller_path)
{
	/* display platform capabilities for the given metadata format
	 * 'scan' in this context means iterate over all metadata types
	 */
	int i;
	int err = 1;

	if (ss && export && ss->export_detail_platform)
		err = ss->export_detail_platform(verbose, controller_path);
	else if (ss && ss->detail_platform)
		err = ss->detail_platform(verbose, 0, controller_path);
	else if (ss) {
		if (verbose > 0)
			pr_err("%s metadata is platform independent\n",
				ss->name ? : "[no name]");
	} else if (!scan) {
		if (verbose > 0)
			pr_err("specify a metadata type or --scan\n");
	}

	if (!scan)
		return err;

	err = 0;
	for (i = 0; superlist[i]; i++) {
		struct superswitch *meta = superlist[i];

		if (meta == ss)
			continue;
		if (verbose > 0)
			pr_err("checking metadata %s\n",
				meta->name ? : "[no name]");
		if (!meta->detail_platform) {
			if (verbose > 0)
				pr_err("%s metadata is platform independent\n",
					meta->name ? : "[no name]");
		} else if (export && meta->export_detail_platform) {
			err |= meta->export_detail_platform(verbose, controller_path);
		} else
			err |= meta->detail_platform(verbose, 0, controller_path);
	}

	return err;
}
