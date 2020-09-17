// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#ifdef HAVE_SG_IO
# include <scsi/sg.h>
#endif
#ifdef HAVE_HDIO_GETGEO
# include <linux/hdreg.h>
#endif
#include "platform_defs.h"
#include "libfrog/util.h"
#include "libfrog/paths.h"
#include "xfs_scrub.h"
#include "common.h"
#include "disk.h"
#include "platform_defs.h"

#ifndef BLKROTATIONAL
# define BLKROTATIONAL	_IO(0x12, 126)
#endif

/*
 * Disk Abstraction
 *
 * These routines help us to discover the geometry of a block device,
 * estimate the amount of concurrent IOs that we can send to it, and
 * abstract the process of performing read verification of disk blocks.
 */

/* Figure out how many disk heads are available. */
static unsigned int
__disk_heads(
	struct disk		*disk)
{
	int			iomin;
	int			ioopt;
	int			nproc = platform_nproc();
	unsigned short		rot;
	int			error;

	/* If it's not a block device, throw all the CPUs at it. */
	if (!S_ISBLK(disk->d_sb.st_mode))
		return nproc;

	/* Non-rotational device?  Throw all the CPUs at the problem. */
	rot = 1;
	error = ioctl(disk->d_fd, BLKROTATIONAL, &rot);
	if (error == 0 && rot == 0)
		return nproc;

	/*
	 * Sometimes we can infer the number of devices from the
	 * min/optimal IO sizes.
	 */
	iomin = ioopt = 0;
	if (ioctl(disk->d_fd, BLKIOMIN, &iomin) == 0 &&
	    ioctl(disk->d_fd, BLKIOOPT, &ioopt) == 0 &&
	    iomin > 0 && ioopt > 0) {
		return min(nproc, max(1, ioopt / iomin));
	}

	/* Rotating device?  I guess? */
	return 2;
}

/* Figure out how many disk heads are available. */
unsigned int
disk_heads(
	struct disk		*disk)
{
	if (force_nr_threads)
		return force_nr_threads;
	return __disk_heads(disk);
}

/*
 * Execute a SCSI VERIFY(16) to verify disk contents.
 * For devices that support this command, this can sharply reduce the
 * runtime of the data block verification phase if the storage device's
 * internal bandwidth exceeds its link bandwidth.  However, it only
 * works if we're talking to a raw SCSI device, and only if we trust the
 * firmware.
 */
#ifdef HAVE_SG_IO
# define SENSE_BUF_LEN		64
# define VERIFY16_CMDLEN	16
# define VERIFY16_CMD		0x8F

# ifndef SG_FLAG_Q_AT_TAIL
#  define SG_FLAG_Q_AT_TAIL	0x10
# endif
static int
disk_scsi_verify(
	struct disk		*disk,
	uint64_t		startblock, /* lba */
	uint64_t		blockcount) /* lba */
{
	struct sg_io_hdr	iohdr;
	unsigned char		cdb[VERIFY16_CMDLEN];
	unsigned char		sense[SENSE_BUF_LEN];
	uint64_t		llba;
	uint64_t		veri_len = blockcount;
	int			error;

	assert(!debug_tweak_on("XFS_SCRUB_NO_SCSI_VERIFY"));

	llba = startblock + (disk->d_start >> BBSHIFT);

	/* Borrowed from sg_verify */
	cdb[0] = VERIFY16_CMD;
	cdb[1] = 0; /* skip PI, DPO, and byte check. */
	cdb[2] = (llba >> 56) & 0xff;
	cdb[3] = (llba >> 48) & 0xff;
	cdb[4] = (llba >> 40) & 0xff;
	cdb[5] = (llba >> 32) & 0xff;
	cdb[6] = (llba >> 24) & 0xff;
	cdb[7] = (llba >> 16) & 0xff;
	cdb[8] = (llba >> 8) & 0xff;
	cdb[9] = llba & 0xff;
	cdb[10] = (veri_len >> 24) & 0xff;
	cdb[11] = (veri_len >> 16) & 0xff;
	cdb[12] = (veri_len >> 8) & 0xff;
	cdb[13] = veri_len & 0xff;
	cdb[14] = 0;
	cdb[15] = 0;
	memset(sense, 0, SENSE_BUF_LEN);

	/* v3 SG_IO */
	memset(&iohdr, 0, sizeof(iohdr));
	iohdr.interface_id = 'S';
	iohdr.dxfer_direction = SG_DXFER_NONE;
	iohdr.cmdp = cdb;
	iohdr.cmd_len = VERIFY16_CMDLEN;
	iohdr.sbp = sense;
	iohdr.mx_sb_len = SENSE_BUF_LEN;
	iohdr.flags |= SG_FLAG_Q_AT_TAIL;
	iohdr.timeout = 30000; /* 30s */

	error = ioctl(disk->d_fd, SG_IO, &iohdr);
	if (error < 0)
		return error;

	dbg_printf("VERIFY(16) fd %d lba %"PRIu64" len %"PRIu64" info %x "
			"status %d masked %d msg %d host %d driver %d "
			"duration %d resid %d\n",
			disk->d_fd, startblock, blockcount, iohdr.info,
			iohdr.status, iohdr.masked_status, iohdr.msg_status,
			iohdr.host_status, iohdr.driver_status, iohdr.duration,
			iohdr.resid);

	if (iohdr.info & SG_INFO_CHECK) {
		dbg_printf("status: msg %x host %x driver %x\n",
				iohdr.msg_status, iohdr.host_status,
				iohdr.driver_status);
		errno = EIO;
		return -1;
	}

	return blockcount << BBSHIFT;
}
#else
# define disk_scsi_verify(...)		(ENOTTY)
#endif /* HAVE_SG_IO */

/* Test the availability of the kernel scrub ioctl. */
static bool
disk_can_scsi_verify(
	struct disk		*disk)
{
	int			error;

	if (debug_tweak_on("XFS_SCRUB_NO_SCSI_VERIFY"))
		return false;

	error = disk_scsi_verify(disk, 0, 1);
	return error == 0;
}

/* Open a disk device and discover its geometry. */
struct disk *
disk_open(
	const char		*pathname)
{
#ifdef HAVE_HDIO_GETGEO
	struct hd_geometry	bdgeo;
#endif
	struct disk		*disk;
	bool			suspicious_disk = false;
	int			error;

	disk = calloc(1, sizeof(struct disk));
	if (!disk)
		return NULL;

	disk->d_fd = open(pathname, O_RDONLY | O_DIRECT | O_NOATIME);
	if (disk->d_fd < 0)
		goto out_free;

	/* Try to get LBA size. */
	error = ioctl(disk->d_fd, BLKSSZGET, &disk->d_lbasize);
	if (error)
		disk->d_lbasize = 512;
	disk->d_lbalog = log2_roundup(disk->d_lbasize);

	/* Obtain disk's stat info. */
	error = fstat(disk->d_fd, &disk->d_sb);
	if (error)
		goto out_close;

	/* Determine bdev size, block size, and offset. */
	if (S_ISBLK(disk->d_sb.st_mode)) {
		error = ioctl(disk->d_fd, BLKGETSIZE64, &disk->d_size);
		if (error)
			disk->d_size = 0;
		error = ioctl(disk->d_fd, BLKBSZGET, &disk->d_blksize);
		if (error)
			disk->d_blksize = 0;
#ifdef HAVE_HDIO_GETGEO
		error = ioctl(disk->d_fd, HDIO_GETGEO, &bdgeo);
		if (!error) {
			/*
			 * dm devices will pass through ioctls, which means
			 * we can't use SCSI VERIFY unless the start is 0.
			 * Most dm devices don't set geometry (unlike scsi
			 * and nvme) so use a zeroed out CHS to screen them
			 * out.
			 */
			if (bdgeo.start != 0 &&
			    (unsigned long long)bdgeo.heads * bdgeo.sectors *
					bdgeo.sectors == 0)
				suspicious_disk = true;
			disk->d_start = bdgeo.start << BBSHIFT;
		} else
#endif
			disk->d_start = 0;
	} else {
		disk->d_size = disk->d_sb.st_size;
		disk->d_blksize = disk->d_sb.st_blksize;
		disk->d_start = 0;
	}

	/* Can we issue SCSI VERIFY? */
	if (!suspicious_disk && disk_can_scsi_verify(disk))
		disk->d_flags |= DISK_FLAG_SCSI_VERIFY;

	return disk;
out_close:
	close(disk->d_fd);
out_free:
	free(disk);
	return NULL;
}

/* Close a disk device. */
int
disk_close(
	struct disk		*disk)
{
	int			error = 0;

	if (disk->d_fd >= 0)
		error = close(disk->d_fd);
	disk->d_fd = -1;
	free(disk);
	return error;
}

#define BTOLBAT(d, bytes)	((uint64_t)(bytes) >> (d)->d_lbalog)
#define LBASIZE(d)		(1ULL << (d)->d_lbalog)
#define BTOLBA(d, bytes)	(((uint64_t)(bytes) + LBASIZE(d) - 1) >> (d)->d_lbalog)

/* Simulate disk errors. */
static int
disk_simulate_read_error(
	struct disk		*disk,
	uint64_t		start,
	uint64_t		*length)
{
	static int64_t		interval;
	uint64_t		start_interval;

	/* Simulated disk errors are disabled. */
	if (interval < 0)
		return 0;

	/* Figure out the disk read error interval. */
	if (interval == 0) {
		char		*p;

		/* Pretend there's bad media every so often, in bytes. */
		p = getenv("XFS_SCRUB_DISK_ERROR_INTERVAL");
		if (p == NULL) {
			interval = -1;
			return 0;
		}
		interval = strtoull(p, NULL, 10);
		interval &= ~((1U << disk->d_lbalog) - 1);
	}
	if (interval <= 0) {
		interval = -1;
		return 0;
	}

	/*
	 * We simulate disk errors by pretending that there are media errors at
	 * predetermined intervals across the disk.  If a read verify request
	 * crosses one of those intervals we shorten it so that the next read
	 * will start on an interval threshold.  If the read verify request
	 * starts on an interval threshold, we send back EIO as if it had
	 * failed.
	 */
	if ((start % interval) == 0) {
		dbg_printf("fd %d: simulating disk error at %"PRIu64".\n",
				disk->d_fd, start);
		return EIO;
	}

	start_interval = start / interval;
	if (start_interval != (start + *length) / interval) {
		*length = ((start_interval + 1) * interval) - start;
		dbg_printf(
"fd %d: simulating short read at %"PRIu64" to length %"PRIu64".\n",
				disk->d_fd, start, *length);
	}

	return 0;
}

/* Read-verify an extent of a disk device. */
ssize_t
disk_read_verify(
	struct disk		*disk,
	void			*buf,
	uint64_t		start,
	uint64_t		length)
{
	if (debug) {
		int		ret;

		ret = disk_simulate_read_error(disk, start, &length);
		if (ret) {
			errno = ret;
			return -1;
		}

		/* Don't actually issue the IO */
		if (getenv("XFS_SCRUB_DISK_VERIFY_SKIP"))
			return length;
	}

	/* Convert to logical block size. */
	if (disk->d_flags & DISK_FLAG_SCSI_VERIFY)
		return disk_scsi_verify(disk, BTOLBAT(disk, start),
				BTOLBA(disk, length));

	return pread(disk->d_fd, buf, length, start);
}
