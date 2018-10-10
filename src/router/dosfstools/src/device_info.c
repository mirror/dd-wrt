/* device_info.c - Collect device information for mkfs.fat

   Copyright (C) 2015 Andreas Bombe <aeb@debian.org>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#ifdef HAVE_UDEV
#include <libudev.h>
#endif

#if HAVE_DECL_GETMNTENT
#include <paths.h>
#include <mntent.h>
#endif

#if HAVE_DECL_GETMNTINFO
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif

#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "blkdev.h"
#include "device_info.h"


static const struct device_info device_info_clueless = {
    .type         = TYPE_UNKNOWN,
    .partition    = -1,
    .has_children = -1,
    .geom_heads   = -1,
    .geom_sectors = -1,
    .geom_start   = -1,
    .sector_size  = -1,
    .size         = -1,
};


int device_info_verbose;


static void get_block_device_size(struct device_info *info, int fd)
{
    unsigned long long bytes;

    if (!blkdev_get_size(fd, &bytes) && bytes != 0)
	info->size = bytes;
}


static void get_block_geometry(struct device_info *info, int fd)
{
    unsigned int heads, sectors, start;

    if (!blkdev_get_geometry(fd, &heads, &sectors)
	    && heads && sectors) {
	info->geom_heads = heads;
	info->geom_sectors = sectors;
    }

    if (!blkdev_get_start(fd, &start))
	info->geom_start = start;
}


static void get_sector_size(struct device_info *info, int fd)
{
    int size;

    if (!blkdev_get_sector_size(fd, &size))
	info->sector_size = size;
}


static int udev_fill_info(struct device_info *info, struct stat *stat);

#ifdef HAVE_UDEV
static int udev_fill_info(struct device_info *info, struct stat *stat)
{
    struct udev *ctx;
    struct udev_device *dev, *parent;
    struct udev_enumerate *uenum;
    const char *attr;
    char holders_path[PATH_MAX + 1];
    DIR *holders_dir;
    struct dirent *dir_entry;
    unsigned long number;
    char *endptr;

    if (device_info_verbose >= 3)
	printf("udev_fill_info()\n");

    ctx = udev_new();
    if (!ctx) {
	if (device_info_verbose)
	    printf("no udev library context\n");
	return -1;
    }

    dev = udev_device_new_from_devnum(ctx, 'b', stat->st_rdev);
    if (!dev) {
	if (device_info_verbose)
	    printf("no udev context\n");
	udev_unref(ctx);
	return -1;
    }

    /*
     * first, look for for dependent devices (partitions or virtual mappings on
     * this device)
     */
    if (device_info_verbose >= 3)
	printf("looking for dependent devices\n");

    uenum = udev_enumerate_new(ctx);
    if (uenum) {
	struct udev_list_entry *entry;
	if (udev_enumerate_add_match_parent(uenum, dev) >= 0 &&
		udev_enumerate_scan_devices(uenum) >= 0) {
	    entry = udev_enumerate_get_list_entry(uenum);
	    if (entry) {
		/*
		 * the list of children includes the parent device, so make
		 * sure that has_children is -1 to end up with the correct
		 * count
		 */
		info->has_children = -1;

		while (entry) {
		    if (device_info_verbose >= 2)
			printf("child-or-self: %s\n", udev_list_entry_get_name(entry));
		    entry = udev_list_entry_get_next(entry);
		    info->has_children++;
		}
	    } else
		info->has_children = 0;
	}
	udev_enumerate_unref(uenum);
    }

    /* see if the holders directory in sysfs exists and has entries */
    if (device_info_verbose >= 2)
	printf("syspath: %s\n", udev_device_get_syspath(dev));
    if (info->has_children < 1 || device_info_verbose >= 3) {
	snprintf(holders_path, PATH_MAX, "%s/holders",
		udev_device_get_syspath(dev));
	holders_path[PATH_MAX] = 0;

	if (info->has_children < 0)
	    info->has_children = 0;

	holders_dir = opendir(holders_path);
	if (holders_dir) {
	    dir_entry = readdir(holders_dir);
	    while (dir_entry) {
		if (dir_entry->d_reclen && dir_entry->d_name[0] != '.') {
		    if (device_info_verbose >= 2)
			printf("holder: %s\n", dir_entry->d_name);

		    info->has_children++;

		    /* look up and print every holder when very verbose */
		    if (device_info_verbose < 3)
			break;
		}
		dir_entry = readdir(holders_dir);
	    }

	    closedir(holders_dir);
	}
    }

    /*
     * block devices on real hardware have either other block devices
     * (in the case of partitions) or the actual hardware as parent
     */
    parent = udev_device_get_parent(dev);

    if (!parent) {
	if (device_info_verbose >= 3)
	    printf("no parent found, therefore virtual device\n");
	info->type = TYPE_VIRTUAL;
	info->partition = 0;
	udev_device_unref(dev);
	return 0;
    }

    attr = udev_device_get_sysattr_value(dev, "removable");
    if (device_info_verbose >= 3) {
	if (attr)
	    printf("attribute \"removable\" is \"%s\"\n", attr);
	else
	    printf("attribute \"removable\" not found\n");
    }
    if (attr && !strcmp(attr, "1"))
	info->type = TYPE_REMOVABLE;
    else
	info->type = TYPE_FIXED;

    attr = udev_device_get_sysattr_value(dev, "partition");
    if (attr) {
	if (device_info_verbose >= 3)
	    printf("attribute \"partition\" is \"%s\"\n", attr);

	number = strtoul(attr, &endptr, 10);
	if (!*endptr)
	    info->partition = number;
    } else {
	printf("attribute \"partition\" not found\n");
	if (info->type != TYPE_VIRTUAL && parent) {
	    /* partitions have other block devices as parent */
	    attr = udev_device_get_subsystem(parent);
	    if (attr) {
		if (device_info_verbose >= 3)
		    printf("parent subsystem is \"%s\"\n", attr);

		if (!strcmp(attr, "block"))
		    /* we don't know the partition number, use 1 */
		    info->partition = 1;
		else
		    info->partition = 0;
	    }
	}
    }

    udev_device_unref(dev);
    udev_unref(ctx);
    return 0;
}
#else  /* HAVE_UDEV */
static int udev_fill_info(struct device_info *info, struct stat *stat)
{
    /* prevent "unused parameter" warning */
    (void)stat;
    (void)info;

    return -1;
}
#endif


int get_device_info(int fd, struct device_info *info)
{
    struct stat stat;
    int ret;

    *info = device_info_clueless;

    ret = fstat(fd, &stat);
    if (ret < 0) {
	perror("fstat on target failed");
	return -1;
    }

    if (S_ISREG(stat.st_mode)) {
	/* there is nothing more to discover for an image file */
	info->type = TYPE_FILE;
	info->partition = 0;
	info->size = stat.st_size;
	return 0;
    }

    if (!S_ISBLK(stat.st_mode)) {
	/* neither regular file nor block device? not usable */
	info->type = TYPE_BAD;
	return 0;
    }

    get_block_device_size(info, fd);
    get_block_geometry(info, fd);
    get_sector_size(info, fd);

    /* use udev information if available */
    udev_fill_info(info, &stat);

    return 0;
}


int is_device_mounted(const char *path)
{
#if HAVE_DECL_GETMNTENT
    FILE *f;
    struct mntent *mnt;

    if ((f = setmntent(_PATH_MOUNTED, "r")) == NULL)
	return 0;
    while ((mnt = getmntent(f)) != NULL)
	if (strcmp(path, mnt->mnt_fsname) == 0)
	    return 1;
    endmntent(f);
    return 0;
#endif

#if HAVE_DECL_GETMNTINFO
    struct statfs *stat;
    int count, i;

    count = getmntinfo(&stat, 0);
    for (i = 0; i < count; i++)
	if (!strcmp(path, stat[i].f_mntfromname))
	    return 1;
    return 0;
#endif

    (void)path; /* prevent unused parameter warning */
    return 0;
}
