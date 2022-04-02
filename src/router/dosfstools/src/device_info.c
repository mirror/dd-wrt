/* device_info.c - Collect device information for mkfs.fat

   Copyright (C) 2015 Andreas Bombe <aeb@debian.org>
   Copyright (C) 2018 Pali Rohár <pali.rohar@gmail.com>

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
#include <sys/sysmacros.h>

#ifdef HAVE_LINUX_LOOP_H
#include <linux/loop.h>
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
#include <errno.h>

#include "blkdev.h"
#include "device_info.h"


static const struct device_info device_info_clueless = {
    .type         = TYPE_UNKNOWN,
    .partition    = -1,
    .has_children = -1,
    .geom_heads   = -1,
    .geom_sectors = -1,
    .geom_start   = -1,
    .geom_size    = -1,
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


static void get_block_geometry(struct device_info *info, int fd, dev_t rdev)
{
    unsigned int heads, sectors;
    unsigned long long start;

    if (!blkdev_get_geometry(fd, &heads, &sectors)
	    && heads && sectors) {
	info->geom_heads = heads;
	info->geom_sectors = sectors;
    }

    if (!blkdev_get_start(fd, rdev, &start))
	info->geom_start = start;
}


static void get_sector_size(struct device_info *info, int fd)
{
    int size;

    if (!blkdev_get_sector_size(fd, &size))
	info->sector_size = size;
}


#ifdef __linux__
static void get_block_linux_info(struct device_info *info, int devfd, dev_t rdev)
{
    struct stat st;
    char path[PATH_MAX];
    int fd;
    int blockfd;
    FILE *file;
    DIR *dir;
    struct dirent *d;
    int maj;
    int min;
    long long start;
    int removable;

#ifdef HAVE_LINUX_LOOP_H
    struct loop_info64 lo;
#endif

    maj = major(rdev);
    min = minor(rdev);

    snprintf(path, sizeof(path), "/sys/dev/block/%d:%d", maj, min);
    blockfd = open(path, O_RDONLY | O_DIRECTORY);
    if (blockfd < 0)
        return;

    /* Check if device is partition */
    fd = openat(blockfd, "partition", O_RDONLY);
    if (fd >= 0) {
        file = fdopen(fd, "r");
        if (file) {
            if (fscanf(file, "%d", &info->partition) != 1 || info->partition == 0)
                info->partition = -1;
            fclose(file);
        } else {
            close(fd);
        }
        /* Read total number of sectors of the disk */
        fd = openat(blockfd, "../size", O_RDONLY);
        if (fd >= 0) {
            file = fdopen(fd, "r");
            if (file) {
                if (fscanf(file, "%lld", &info->geom_size) != 1 || info->geom_size == 0)
                    info->geom_size = -1;
                fclose(file);
            } else {
                close(fd);
            }
        }
    } else if (errno == ENOENT && info->geom_start <= 0) {
        info->partition = 0;
        if (info->size > 0 && info->sector_size > 0)
            info->geom_size = info->size / info->sector_size;
    }

    /* Check if device has partition subdevice and therefore has children */
    fd = dup(blockfd);
    if (fd >= 0) {
        dir = fdopendir(fd);
        if (dir) {
            info->has_children = 0;
            errno = 0;
            while ((d = readdir(dir))) {
                if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
                    continue;
                if (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
                    continue;
                snprintf(path, sizeof(path), "%s/partition", d->d_name);
                if (fstatat(blockfd, path, &st, 0) == 0) {
                    if (S_ISREG(st.st_mode)) {
                        start = -1;
                        snprintf(path, sizeof(path), "%s/start", d->d_name);
                        fd = openat(blockfd, path, O_RDONLY);
                        if (fd >= 0) {
                            file = fdopen(fd, "r");
                            if (file) {
                                if (fscanf(file, "%lld", &start) != 1)
                                    start = -1;
                                fclose(file);
                            } else {
                                close(fd);
                            }
                        }
                        /* If subdevice starts at zero offset then it is whole device, so it is not a child */
                        if (start != 0) {
                            info->has_children = 1;
                            break;
                        }
                    }
                } else if (errno != ENOENT) {
                    info->has_children = -1;
                }
                errno = 0;
            }
            if (errno != 0 && info->has_children == 0)
                info->has_children = -1;
            closedir(dir);
        } else {
            close(fd);
        }
    }

    /* Check if device has holders and therefore has children */
    if (info->has_children <= 0) {
        fd = openat(blockfd, "holders", O_RDONLY | O_DIRECTORY);
        if (fd >= 0) {
            dir = fdopendir(fd);
            if (dir) {
                while ((d = readdir(dir))) {
                    if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
                        continue;
                    info->has_children = 1;
                    break;
                }
                closedir(dir);
            } else {
                close(fd);
            }
        }
    }

    /* Check if device is slave of another device and therefore is virtual */
    fd = openat(blockfd, "slaves", O_RDONLY | O_DIRECTORY);
    if (fd >= 0) {
        dir = fdopendir(fd);
        if (dir) {
            while ((d = readdir(dir))) {
                if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
                    continue;
                info->type = TYPE_VIRTUAL;
                break;
            }
            closedir(dir);
        } else {
            close(fd);
        }
    }

#ifdef HAVE_LINUX_LOOP_H
    /* Check if device is loop and detect if is based from regular file or is virtual */
    if (info->type == TYPE_UNKNOWN && info->partition == 0 && ioctl(devfd, LOOP_GET_STATUS64, &lo) == 0) {
        if (lo.lo_offset == 0 && lo.lo_sizelimit == 0 && lo.lo_encrypt_type == LO_CRYPT_NONE &&
            stat((char *)lo.lo_file_name, &st) == 0 && S_ISREG(st.st_mode) &&
            st.st_dev == lo.lo_device && st.st_ino == lo.lo_inode && st.st_size == info->size)
            info->type = TYPE_FILE;
        else
            info->type = TYPE_VIRTUAL;
    }
#endif

    /* Device is neither loop nor virtual, so is either removable or fixed */
    if (info->type == TYPE_UNKNOWN) {
        removable = 0;
        fd = openat(blockfd, "removable", O_RDONLY);
        if (fd >= 0) {
            file = fdopen(fd, "r");
            if (file) {
                if (fscanf(file, "%d", &removable) != 1)
                    removable = 0;
                fclose(file);
            } else {
                close(fd);
            }
        }

        if (removable)
            info->type = TYPE_REMOVABLE;
        else
            info->type = TYPE_FIXED;
    }

    close(blockfd);
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
    get_block_geometry(info, fd, stat.st_rdev);
    get_sector_size(info, fd);

#ifdef __linux__
    get_block_linux_info(info, fd, stat.st_rdev);
#endif

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
