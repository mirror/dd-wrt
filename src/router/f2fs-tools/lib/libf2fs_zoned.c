/**
 * libf2fs_zoned.c
 *
 * Copyright (c) 2016 Western Digital Corporation.
 * Written by: Damien Le Moal <damien.lemoal@wdc.com>
 *
 * Dual licensed under the GPL or LGPL version 2 licenses.
 */
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <libgen.h>

#include <f2fs_fs.h>

#ifdef HAVE_LINUX_BLKZONED_H

void f2fs_get_zoned_model(int i)
{
	struct device_info *dev = c.devices + i;
	char str[128];
	FILE *file;
	int res;

	/* Check that this is a zoned block device */
	snprintf(str, sizeof(str),
		 "/sys/block/%s/queue/zoned",
		 basename(dev->path));
	file = fopen(str, "r");
	if (!file)
		goto not_zoned;

	memset(str, 0, sizeof(str));
	res = fscanf(file, "%s", str);
	fclose(file);

	if (res != 1)
		goto not_zoned;

	if (strcmp(str, "host-aware") == 0) {
		dev->zoned_model = F2FS_ZONED_HA;
		return;
	}
	if (strcmp(str, "host-managed") == 0) {
		dev->zoned_model = F2FS_ZONED_HM;
		return;
	}

not_zoned:
	dev->zoned_model = F2FS_ZONED_NONE;
}

int f2fs_get_zone_blocks(int i)
{
	struct device_info *dev = c.devices + i;
	uint64_t sectors;
	char str[128];
	FILE *file;
	int res;

	/* Get zone size */
	dev->zone_blocks = 0;

	snprintf(str, sizeof(str),
		 "/sys/block/%s/queue/chunk_sectors",
		 basename(dev->path));
	file = fopen(str, "r");
	if (!file)
		return -1;

	memset(str, 0, sizeof(str));
	res = fscanf(file, "%s", str);
	fclose(file);

	if (res != 1)
		return -1;

	sectors = atol(str);
	if (!sectors)
		return -1;

	dev->zone_blocks = sectors >> (F2FS_BLKSIZE_BITS - 9);
	sectors = (sectors << 9) / c.sector_size;

	/*
	 * Total number of zones: there may
	 * be a last smaller runt zone.
	 */
	dev->nr_zones = dev->total_sectors / sectors;
	if (dev->total_sectors % sectors)
		dev->nr_zones++;

	return 0;
}

#define F2FS_REPORT_ZONES_BUFSZ	524288

int f2fs_check_zones(int j)
{
	struct device_info *dev = c.devices + j;
	struct blk_zone_report *rep;
	struct blk_zone *blkz;
	unsigned int i, n = 0;
	u_int64_t total_sectors;
	u_int64_t sector;
	int last_is_conv = 1;
	int ret = -1;

	rep = malloc(F2FS_REPORT_ZONES_BUFSZ);
	if (!rep) {
		ERR_MSG("No memory for report zones\n");
		return -ENOMEM;
	}

	dev->nr_rnd_zones = 0;
	sector = 0;
	total_sectors = (dev->total_sectors * c.sector_size) >> 9;

	while (sector < total_sectors) {

		/* Get zone info */
		memset(rep, 0, F2FS_REPORT_ZONES_BUFSZ);
		rep->sector = sector;
		rep->nr_zones = (F2FS_REPORT_ZONES_BUFSZ - sizeof(struct blk_zone_report))
			/ sizeof(struct blk_zone);

		ret = ioctl(dev->fd, BLKREPORTZONE, rep);
		if (ret != 0) {
			ret = -errno;
			ERR_MSG("ioctl BLKREPORTZONE failed\n");
			goto out;
		}

		if (!rep->nr_zones)
			break;

		blkz = (struct blk_zone *)(rep + 1);
		for (i = 0; i < rep->nr_zones && sector < total_sectors; i++) {

			if (blk_zone_cond(blkz) == BLK_ZONE_COND_READONLY ||
			    blk_zone_cond(blkz) == BLK_ZONE_COND_OFFLINE)
				last_is_conv = 0;
			if (blk_zone_conv(blkz) ||
			    blk_zone_seq_pref(blkz)) {
				if (last_is_conv)
					dev->nr_rnd_zones++;
			} else {
				last_is_conv = 0;
			}

			if (blk_zone_conv(blkz)) {
				DBG(2,
				    "Zone %05u: Conventional, cond 0x%x (%s), sector %llu, %llu sectors\n",
				    n,
				    blk_zone_cond(blkz),
				    blk_zone_cond_str(blkz),
				    blk_zone_sector(blkz),
				    blk_zone_length(blkz));
			} else {
				DBG(2,
				    "Zone %05u: type 0x%x (%s), cond 0x%x (%s), need_reset %d, "
				    "non_seq %d, sector %llu, %llu sectors, wp sector %llu\n",
				    n,
				    blk_zone_type(blkz),
				    blk_zone_type_str(blkz),
				    blk_zone_cond(blkz),
				    blk_zone_cond_str(blkz),
				    blk_zone_need_reset(blkz),
				    blk_zone_non_seq(blkz),
				    blk_zone_sector(blkz),
				    blk_zone_length(blkz),
				    blk_zone_wp_sector(blkz));
			}

			sector = blk_zone_sector(blkz) + blk_zone_length(blkz);
			n++;
			blkz++;
		}
	}

	if (sector != total_sectors) {
		ERR_MSG("Invalid zones: last sector reported is %llu, expected %llu\n",
			(unsigned long long)(sector << 9) / c.sector_size,
			(unsigned long long)dev->total_sectors);
		ret = -1;
		goto out;
	}

	if (n != dev->nr_zones) {
		ERR_MSG("Inconsistent number of zones: expected %u zones, got %u\n",
			dev->nr_zones, n);
		ret = -1;
		goto out;
	}

	if (dev->zoned_model == F2FS_ZONED_HM &&
			!dev->nr_rnd_zones) {
		ERR_MSG("No conventional zone for super block\n");
		ret = -1;
	}
out:
	free(rep);
	return ret;
}

int f2fs_reset_zones(int j)
{
	struct device_info *dev = c.devices + j;
	struct blk_zone_report *rep;
	struct blk_zone *blkz;
	struct blk_zone_range range;
	u_int64_t total_sectors;
	u_int64_t sector;
	unsigned int i;
	int ret = -1;

	rep = malloc(F2FS_REPORT_ZONES_BUFSZ);
	if (!rep) {
		ERR_MSG("No memory for report zones\n");
		return -1;
	}

	sector = 0;
	total_sectors = (dev->total_sectors * c.sector_size) >> 9;
	while (sector < total_sectors) {

		/* Get zone info */
		memset(rep, 0, F2FS_REPORT_ZONES_BUFSZ);
		rep->sector = sector;
		rep->nr_zones = (F2FS_REPORT_ZONES_BUFSZ - sizeof(struct blk_zone_report))
			/ sizeof(struct blk_zone);

		ret = ioctl(dev->fd, BLKREPORTZONE, rep);
		if (ret != 0) {
			ret = -errno;
			ERR_MSG("ioctl BLKREPORTZONES failed\n");
			goto out;
		}

		if (!rep->nr_zones)
			break;

		blkz = (struct blk_zone *)(rep + 1);
		for (i = 0; i < rep->nr_zones && sector < total_sectors; i++) {
			if (blk_zone_seq(blkz) &&
			    !blk_zone_empty(blkz)) {
				/* Non empty sequential zone: reset */
				range.sector = blk_zone_sector(blkz);
				range.nr_sectors = blk_zone_length(blkz);
				ret = ioctl(dev->fd, BLKRESETZONE, &range);
				if (ret != 0) {
					ret = -errno;
					ERR_MSG("ioctl BLKRESETZONE failed\n");
					goto out;
				}
			}
			sector = blk_zone_sector(blkz) + blk_zone_length(blkz);
			blkz++;
		}
	}
out:
	free(rep);
	if (!ret)
		MSG(0, "Info: Discarded %"PRIu64" MB\n", (sector << 9) >> 20);
	return ret;
}

#else

void f2fs_get_zoned_model(int i)
{
	struct device_info *dev = c.devices + i;

	c.zoned_mode = 0;
	dev->zoned_model = F2FS_ZONED_NONE;
}

int f2fs_get_zone_blocks(int i)
{
	struct device_info *dev = c.devices + i;

	c.zoned_mode = 0;
	dev->nr_zones = 0;
	dev->zone_blocks = 0;
	dev->zoned_model = F2FS_ZONED_NONE;

	return 0;
}

int f2fs_check_zones(int i)
{
	ERR_MSG("%d: Zoned block devices are not supported\n", i);
	return -1;
}

int f2fs_reset_zones(int i)
{
	ERR_MSG("%d: Zoned block devices are not supported\n", i);
	return -1;
}

#endif

