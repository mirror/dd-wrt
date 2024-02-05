// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "libfrog/paths.h"
#include "xfs_scrub.h"
#include "common.h"
#include "filemap.h"

/*
 * These routines provide a simple interface to query the block
 * mappings of the fork of a given inode via GETBMAPX and call a
 * function to iterate each mapping result.
 */

#define BMAP_NR		2048

/*
 * Iterate all the extent block mappings between the key and fork end.
 * Returns 0 or a positive error number.
 */
int
scrub_iterate_filemaps(
	struct scrub_ctx	*ctx,
	int			fd,
	int			whichfork,
	struct file_bmap	*key,
	scrub_bmap_iter_fn	fn,
	void			*arg)
{
	struct fsxattr		fsx;
	struct getbmapx		*map;
	struct getbmapx		*p;
	struct file_bmap	bmap;
	xfs_off_t		new_off;
	int			getxattr_type;
	int			i;
	int			ret;

	map = calloc(BMAP_NR, sizeof(struct getbmapx));
	if (!map)
		return errno;

	map->bmv_offset = BTOBB(key->bm_offset);
	map->bmv_block = BTOBB(key->bm_physical);
	if (key->bm_length == 0)
		map->bmv_length = ULLONG_MAX;
	else
		map->bmv_length = BTOBB(key->bm_length);
	map->bmv_iflags = BMV_IF_PREALLOC | BMV_IF_NO_HOLES;
	switch (whichfork) {
	case XFS_ATTR_FORK:
		getxattr_type = XFS_IOC_FSGETXATTRA;
		map->bmv_iflags |= BMV_IF_ATTRFORK;
		break;
	case XFS_COW_FORK:
		map->bmv_iflags |= BMV_IF_COWFORK;
		getxattr_type = FS_IOC_FSGETXATTR;
		break;
	case XFS_DATA_FORK:
		getxattr_type = FS_IOC_FSGETXATTR;
		break;
	default:
		abort();
	}

	ret = ioctl(fd, getxattr_type, &fsx);
	if (ret < 0) {
		ret = errno;
		goto out;
	}

	if (fsx.fsx_nextents == 0)
		goto out;

	map->bmv_count = min(fsx.fsx_nextents + 1, BMAP_NR);

	while ((ret = ioctl(fd, XFS_IOC_GETBMAPX, map)) == 0) {
		for (i = 0, p = &map[i + 1]; i < map->bmv_entries; i++, p++) {
			bmap.bm_offset = BBTOB(p->bmv_offset);
			bmap.bm_physical = BBTOB(p->bmv_block);
			bmap.bm_length = BBTOB(p->bmv_length);
			bmap.bm_flags = p->bmv_oflags;
			ret = fn(ctx, fd, whichfork, &fsx, &bmap, arg);
			if (ret)
				goto out;
			if (scrub_excessive_errors(ctx))
				goto out;
		}

		if (map->bmv_entries == 0)
			break;
		p = map + map->bmv_entries;
		if (p->bmv_oflags & BMV_OF_LAST)
			break;

		new_off = p->bmv_offset + p->bmv_length;
		map->bmv_length -= new_off - map->bmv_offset;
		map->bmv_offset = new_off;
	}
	if (ret < 0)
		ret = errno;

	/*
	 * Pre-reflink filesystems don't know about CoW forks, so don't
	 * be too surprised if it fails.
	 */
	if (whichfork == XFS_COW_FORK && ret == EINVAL)
		ret = 0;
out:
	free(map);
	return ret;
}
