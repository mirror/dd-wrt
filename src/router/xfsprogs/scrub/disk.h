// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_DISK_H_
#define XFS_SCRUB_DISK_H_

#define DISK_FLAG_SCSI_VERIFY	0x1
struct disk {
	struct stat	d_sb;
	int		d_fd;
	unsigned int	d_lbalog;
	unsigned int	d_lbasize;	/* bytes */
	unsigned int	d_flags;
	unsigned int	d_blksize;	/* bytes */
	uint64_t	d_size;		/* bytes */
	uint64_t	d_start;	/* bytes */
};

unsigned int disk_heads(struct disk *disk);
struct disk *disk_open(const char *pathname);
int disk_close(struct disk *disk);
ssize_t disk_read_verify(struct disk *disk, void *buf, uint64_t startblock,
		uint64_t blockcount);

#endif /* XFS_SCRUB_DISK_H_ */
