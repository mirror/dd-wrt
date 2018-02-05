/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "libxfs.h"
#include "libxcmd.h"
#ifdef ENABLE_BLKID
#  include <blkid/blkid.h>
#endif /* ENABLE_BLKID */
#include "xfs_multidisk.h"

#define TERABYTES(count, blog)	((uint64_t)(count) << (40 - (blog)))
#define GIGABYTES(count, blog)	((uint64_t)(count) << (30 - (blog)))
#define MEGABYTES(count, blog)	((uint64_t)(count) << (20 - (blog)))

void
calc_default_ag_geometry(
	int		blocklog,
	uint64_t	dblocks,
	int		multidisk,
	uint64_t	*agsize,
	uint64_t	*agcount)
{
	uint64_t	blocks = 0;
	int		shift = 0;

	/*
	 * First handle the high extreme - the point at which we will
	 * always use the maximum AG size.
	 *
	 * This applies regardless of storage configuration.
	 */
	if (dblocks >= TERABYTES(32, blocklog)) {
		blocks = XFS_AG_MAX_BLOCKS(blocklog);
		goto done;
	}

	/*
	 * For a single underlying storage device over 4TB in size
	 * use the maximum AG size.  Between 128MB and 4TB, just use
	 * 4 AGs and scale up smoothly between min/max AG sizes.
	 */
	if (!multidisk) {
		if (dblocks >= TERABYTES(4, blocklog)) {
			blocks = XFS_AG_MAX_BLOCKS(blocklog);
			goto done;
		} else if (dblocks >= MEGABYTES(128, blocklog)) {
			shift = XFS_NOMULTIDISK_AGLOG;
			goto calc_blocks;
		}
	}

	/*
	 * For the multidisk configs we choose an AG count based on the number
	 * of data blocks available, trying to keep the number of AGs higher
	 * than the single disk configurations. This makes the assumption that
	 * larger filesystems have more parallelism available to them.
	 */
	shift = XFS_MULTIDISK_AGLOG;
	if (dblocks <= GIGABYTES(512, blocklog))
		shift--;
	if (dblocks <= GIGABYTES(8, blocklog))
		shift--;
	if (dblocks < MEGABYTES(128, blocklog))
		shift--;
	if (dblocks < MEGABYTES(64, blocklog))
		shift--;
	if (dblocks < MEGABYTES(32, blocklog))
		shift--;

	/*
	 * If dblocks is not evenly divisible by the number of
	 * desired AGs, round "blocks" up so we don't lose the
	 * last bit of the filesystem. The same principle applies
	 * to the AG count, so we don't lose the last AG!
	 */
calc_blocks:
	ASSERT(shift >= 0 && shift <= XFS_MULTIDISK_AGLOG);
	blocks = dblocks >> shift;
	if (dblocks & xfs_mask32lo(shift)) {
		if (blocks < XFS_AG_MAX_BLOCKS(blocklog))
		    blocks++;
	}
done:
	*agsize = blocks;
	*agcount = dblocks / blocks + (dblocks % blocks != 0);
}

/*
 * Check for existing filesystem or partition table on device.
 * Returns:
 *	 1 for existing fs or partition
 *	 0 for nothing found
 *	-1 for internal error
 */
#ifdef ENABLE_BLKID
int
check_overwrite(
	const char	*device)
{
	const char	*type;
	blkid_probe	pr = NULL;
	int		ret;
	int		fd;
	long long	size;
	int		bsz;

	if (!device || !*device)
		return 0;

	ret = -1; /* will reset on success of all setup calls */

	fd = open(device, O_RDONLY);
	if (fd < 0)
		goto out;
	platform_findsizes((char *)device, fd, &size, &bsz);
	close(fd);

	/* nothing to overwrite on a 0-length device */
	if (size == 0) {
		ret = 0;
		goto out;
	}

	pr = blkid_new_probe_from_filename(device);
	if (!pr)
		goto out;

	ret = blkid_probe_enable_partitions(pr, 1);
	if (ret < 0)
		goto out;

	ret = blkid_do_fullprobe(pr);
	if (ret < 0)
		goto out;

	/*
	 * Blkid returns 1 for nothing found and 0 when it finds a signature,
	 * but we want the exact opposite, so reverse the return value here.
	 *
	 * In addition print some useful diagnostics about what actually is
	 * on the device.
	 */
	if (ret) {
		ret = 0;
		goto out;
	}

	if (!blkid_probe_lookup_value(pr, "TYPE", &type, NULL)) {
		fprintf(stderr,
			_("%s: %s appears to contain an existing "
			"filesystem (%s).\n"), progname, device, type);
	} else if (!blkid_probe_lookup_value(pr, "PTTYPE", &type, NULL)) {
		fprintf(stderr,
			_("%s: %s appears to contain a partition "
			"table (%s).\n"), progname, device, type);
	} else {
		fprintf(stderr,
			_("%s: %s appears to contain something weird "
			"according to blkid\n"), progname, device);
	}
	ret = 1;
out:
	if (pr)
		blkid_free_probe(pr);
	if (ret == -1)
		fprintf(stderr,
			_("%s: probe of %s failed, cannot detect "
			  "existing filesystem.\n"), progname, device);
	return ret;
}

static void blkid_get_topology(
	const char	*device,
	int		*sunit,
	int		*swidth,
	int		*lsectorsize,
	int		*psectorsize,
	int		force_overwrite)
{

	blkid_topology tp;
	blkid_probe pr;
	unsigned long val;
	struct stat statbuf;

	/* can't get topology info from a file */
	if (!stat(device, &statbuf) && S_ISREG(statbuf.st_mode)) {
		fprintf(stderr,
	_("%s: Warning: trying to probe topology of a file %s!\n"),
			progname, device);
		return;
	}

	pr = blkid_new_probe_from_filename(device);
	if (!pr)
		return;

	tp = blkid_probe_get_topology(pr);
	if (!tp)
		goto out_free_probe;

	val = blkid_topology_get_logical_sector_size(tp);
	*lsectorsize = val;
	val = blkid_topology_get_physical_sector_size(tp);
	*psectorsize = val;
	val = blkid_topology_get_minimum_io_size(tp);
	*sunit = val;
	val = blkid_topology_get_optimal_io_size(tp);
	*swidth = val;

	/*
	 * If the reported values are the same as the physical sector size
	 * do not bother to report anything.  It will only cause warnings
	 * if people specify larger stripe units or widths manually.
	 */
	if (*sunit == *psectorsize || *swidth == *psectorsize) {
		*sunit = 0;
		*swidth = 0;
	}

	/*
	 * Blkid reports the information in terms of bytes, but we want it in
	 * terms of 512 bytes blocks (only to convert it to bytes later..)
	 */
	*sunit = *sunit >> 9;
	*swidth = *swidth >> 9;

	if (blkid_topology_get_alignment_offset(tp) != 0) {
		fprintf(stderr,
			_("warning: device is not properly aligned %s\n"),
			device);

		if (!force_overwrite) {
			fprintf(stderr,
				_("Use -f to force usage of a misaligned device\n"));

			exit(EXIT_FAILURE);
		}
		/* Do not use physical sector size if the device is misaligned */
		*psectorsize = *lsectorsize;
	}

	blkid_free_probe(pr);
	return;

out_free_probe:
	blkid_free_probe(pr);
	fprintf(stderr,
		_("warning: unable to probe device topology for device %s\n"),
		device);
}
#else /* ifdef ENABLE_BLKID */
/*
 * Without blkid, we can't do a good check for signatures.
 * So instead of some messy attempts, just disable any checks
 * and always return 'nothing found'.
 */
#  warning BLKID is disabled, so signature detection and block device\
 access are not working!
int
check_overwrite(
	const char	*device)
{
	return 1;
}

static void blkid_get_topology(
	const char	*device,
	int		*sunit,
	int		*swidth,
	int		*lsectorsize,
	int		*psectorsize,
	int		force_overwrite)
{
	/*
	 * Shouldn't make any difference (no blkid = no block device access),
	 * but make sure this dummy replacement returns with at least some
	 * sanity.
	 */
	*lsectorsize = *psectorsize = 512;
}

#endif /* ENABLE_BLKID */

void get_topology(
	libxfs_init_t		*xi,
	struct fs_topology	*ft,
	int			force_overwrite)
{
	struct stat statbuf;
	char *dfile = xi->volname ? xi->volname : xi->dname;

	/*
	 * If our target is a regular file, use platform_findsizes
	 * to try to obtain the underlying filesystem's requirements
	 * for direct IO; we'll set our sector size to that if possible.
	 */
	if (xi->disfile ||
	    (!stat(dfile, &statbuf) && S_ISREG(statbuf.st_mode))) {
		int fd;
		int flags = O_RDONLY;
		long long dummy;

		/* with xi->disfile we may not have the file yet! */
		if (xi->disfile)
			flags |= O_CREAT;

		fd = open(dfile, flags, 0666);
		if (fd >= 0) {
			platform_findsizes(dfile, fd, &dummy, &ft->lsectorsize);
			close(fd);
			ft->psectorsize = ft->lsectorsize;
		} else
			ft->psectorsize = ft->lsectorsize = BBSIZE;
	} else {
		blkid_get_topology(dfile, &ft->dsunit, &ft->dswidth,
				   &ft->lsectorsize, &ft->psectorsize,
				   force_overwrite);
	}

	if (xi->rtname && !xi->risfile) {
		int sunit, lsectorsize, psectorsize;

		blkid_get_topology(xi->rtname, &sunit, &ft->rtswidth,
				   &lsectorsize, &psectorsize, force_overwrite);
	}
}
