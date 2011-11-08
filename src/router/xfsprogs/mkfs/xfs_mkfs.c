/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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

#include <xfs/libxfs.h>
#include <ctype.h>
#ifdef ENABLE_BLKID
#include <blkid/blkid.h>
#else
#include <disk/fstyp.h>
#include <disk/volume.h>
#endif
#include "xfs_mkfs.h"

/*
 * Device topology information.
 */
struct fs_topology {
	int	dsunit;		/* stripe unit - data subvolume */
	int	dswidth;	/* stripe width - data subvolume */
	int	rtswidth;	/* stripe width - rt subvolume */
	int	sectorsize;
	int	sectoralign;
};

/*
 * Prototypes for internal functions.
 */
static void conflict(char opt, char *tab[], int oldidx, int newidx);
static void illegal(char *value, char *opt);
static __attribute__((noreturn)) void usage (void);
static __attribute__((noreturn)) void reqval(char opt, char *tab[], int idx);
static void respec(char opt, char *tab[], int idx);
static void unknown(char opt, char *s);
static int  ispow2(unsigned int i);

/*
 * option tables for getsubopt calls
 */
char *bopts[] = {
#define	B_LOG		0
	"log",
#define	B_SIZE		1
	"size",
	NULL
};

char	*dopts[] = {
#define	D_AGCOUNT	0
	"agcount",
#define	D_FILE		1
	"file",
#define	D_NAME		2
	"name",
#define	D_SIZE		3
	"size",
#define D_SUNIT		4
	"sunit",
#define D_SWIDTH	5
	"swidth",
#define D_AGSIZE	6
	"agsize",
#define D_SU		7
	"su",
#define D_SW		8
	"sw",
#define D_SECTLOG	9
	"sectlog",
#define D_SECTSIZE	10
	"sectsize",
#define D_NOALIGN	11
	"noalign",
#define D_RTINHERIT	12
	"rtinherit",
#define D_PROJINHERIT	13
	"projinherit",
#define D_EXTSZINHERIT	14
	"extszinherit",
	NULL
};

char	*iopts[] = {
#define	I_ALIGN		0
	"align",
#define	I_LOG		1
	"log",
#define	I_MAXPCT	2
	"maxpct",
#define	I_PERBLOCK	3
	"perblock",
#define	I_SIZE		4
	"size",
#define	I_ATTR		5
	"attr",
#define	I_PROJID32BIT	6
	"projid32bit",
	NULL
};

char	*lopts[] = {
#define	L_AGNUM		0
	"agnum",
#define	L_INTERNAL	1
	"internal",
#define	L_SIZE		2
	"size",
#define L_VERSION	3
	"version",
#define L_SUNIT		4
	"sunit",
#define L_SU		5
	"su",
#define L_DEV		6
	"logdev",
#define	L_SECTLOG	7
	"sectlog",
#define	L_SECTSIZE	8
	"sectsize",
#define	L_FILE		9
	"file",
#define	L_NAME		10
	"name",
#define	L_LAZYSBCNTR	11
	"lazy-count",
	NULL
};

char	*nopts[] = {
#define	N_LOG		0
	"log",
#define	N_SIZE		1
	"size",
#define	N_VERSION	2
	"version",
	NULL,
};

char	*ropts[] = {
#define	R_EXTSIZE	0
	"extsize",
#define	R_SIZE		1
	"size",
#define	R_DEV		2
	"rtdev",
#define	R_FILE		3
	"file",
#define	R_NAME		4
	"name",
#define R_NOALIGN	5
	"noalign",
	NULL
};

char	*sopts[] = {
#define	S_LOG		0
	"log",
#define	S_SECTLOG	1
	"sectlog",
#define	S_SIZE		2
	"size",
#define	S_SECTSIZE	3
	"sectsize",
	NULL
};

#define TERABYTES(count, blog)	((__uint64_t)(count) << (40 - (blog)))
#define GIGABYTES(count, blog)	((__uint64_t)(count) << (30 - (blog)))
#define MEGABYTES(count, blog)	((__uint64_t)(count) << (20 - (blog)))

/*
 * Use this macro before we have superblock and mount structure
 */
#define	DTOBT(d)	((xfs_drfsbno_t)((d) >> (blocklog - BBSHIFT)))

/*
 * Use this for block reservations needed for mkfs's conditions
 * (basically no fragmentation).
 */
#define	MKFS_BLOCKRES_INODE	\
	((uint)(XFS_IALLOC_BLOCKS(mp) + (XFS_IN_MAXLEVELS(mp) - 1)))
#define	MKFS_BLOCKRES(rb)	\
	((uint)(MKFS_BLOCKRES_INODE + XFS_DA_NODE_MAXDEPTH + \
	(XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK) - 1) + (rb)))

/* amount (in bytes) we zero at the beginning and end of the device to
 * remove traces of other filesystems, raid superblocks, etc.
 */
#define WHACK_SIZE (128 * 1024)

static void
calc_stripe_factors(
	int		dsu,
	int		dsw,
	int		dsectsz,
	int		lsu,
	int		lsectsz,
	int		*dsunit,
	int		*dswidth,
	int		*lsunit)
{
	/* Handle data sunit/swidth options */
	if (*dsunit || *dswidth) {
		if (dsu || dsw) {
			fprintf(stderr,
				_("data su/sw must not be used in "
				"conjunction with data sunit/swidth\n"));
			usage();
		}

		if ((*dsunit && !*dswidth) || (!*dsunit && *dswidth)) {
			fprintf(stderr,
				_("both data sunit and data swidth options "
				"must be specified\n"));
			usage();
		}
	}

	if (dsu || dsw) {
		if (*dsunit || *dswidth) {
			fprintf(stderr,
				_("data sunit/swidth must not be used in "
				"conjunction with data su/sw\n"));
			usage();
		}

		if ((dsu && !dsw) || (!dsu && dsw)) {
			fprintf(stderr,
				_("both data su and data sw options "
				"must be specified\n"));
			usage();
		}

		if (dsu % dsectsz) {
			fprintf(stderr,
				_("data su must be a multiple of the "
				"sector size (%d)\n"), dsectsz);
			usage();
		}

		*dsunit  = (int)BTOBBT(dsu);
		*dswidth = *dsunit * dsw;
	}

	if (*dsunit && (*dswidth % *dsunit != 0)) {
		fprintf(stderr,
			_("data stripe width (%d) must be a multiple of the "
			"data stripe unit (%d)\n"), *dswidth, *dsunit);
		usage();
	}

	/* Handle log sunit options */

	if (*lsunit) {
		if (lsu) {
			fprintf(stderr,
				_("log su should not be used in "
				"conjunction with log sunit\n"));
			usage();
		}
	}

	if (lsu) {
		if (*lsunit) {
			fprintf(stderr,
				_("log sunit should not be used in "
				"conjunction with log su\n"));
			usage();
		}
		*lsunit = (int)BTOBBT(lsu);
	}
}

#ifdef ENABLE_BLKID
/*
 * Check for existing filesystem or partition table on device.
 * Returns:
 *	 1 for existing fs or partition
 *	 0 for nothing found
 *	-1 for internal error
 */
static int
check_overwrite(
	char		*device)
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
	platform_findsizes(device, fd, &size, &bsz);
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

static void blkid_get_topology(const char *device, int *sunit, int *swidth, int *sectorsize)
{
	blkid_topology tp;
	blkid_probe pr;
	unsigned long val;
	struct stat statbuf;

	/* can't get topology info from a file */
	if (!stat(device, &statbuf) && S_ISREG(statbuf.st_mode))
		return;

	pr = blkid_new_probe_from_filename(device);
	if (!pr)
		return;

	tp = blkid_probe_get_topology(pr);
	if (!tp)
		goto out_free_probe;

	/*
	 * Blkid reports the information in terms of bytes, but we want it in
	 * terms of 512 bytes blocks (just to convert it to bytes later..)
	 *
	 * If the reported values are just the normal 512 byte block size
	 * do not bother to report anything.  It will just causes warnings
	 * if people specifier larger stripe units or widths manually.
	 */
	val = blkid_topology_get_minimum_io_size(tp) >> 9;
	if (val > 1)
		*sunit = val;
	val = blkid_topology_get_optimal_io_size(tp) >> 9;
	if (val > 1)
		*swidth = val;
	val = blkid_probe_get_sectorsize(pr);
	*sectorsize = val;

	if (blkid_topology_get_alignment_offset(tp) != 0) {
		fprintf(stderr,
			_("warning: device is not properly aligned %s\n"),
			device);
	}

	blkid_free_probe(pr);
	return;

out_free_probe:
	blkid_free_probe(pr);
	fprintf(stderr,
		_("warning: unable to probe device toplology for device %s\n"),
		device);
}

static void get_topology(libxfs_init_t *xi, struct fs_topology *ft)
{
	if (!xi->disfile) {
		const char *dfile = xi->volname ? xi->volname : xi->dname;

		blkid_get_topology(dfile, &ft->dsunit, &ft->dswidth,
				   &ft->sectorsize);
	}

	if (xi->rtname && !xi->risfile) {
		int dummy;

		blkid_get_topology(xi->rtname, &dummy, &ft->rtswidth, &dummy);
	}
}
#else /* ENABLE_BLKID */
static int
check_overwrite(
	char		*device)
{
	char		*type;

	if (device && *device) {
		if ((type = fstype(device)) != NULL) {
			fprintf(stderr,
				_("%s: %s appears to contain an existing "
				"filesystem (%s).\n"), progname, device, type);
			return 1;
		}
		if ((type = pttype(device)) != NULL) {
			fprintf(stderr,
				_("%s: %s appears to contain a partition "
				"table (%s).\n"), progname, device, type);
			return 1;
		}
	}
	return 0;
}

static void get_topology(libxfs_init_t *xi, struct fs_topology *ft)
{
	char *dfile = xi->volname ? xi->volname : xi->dname;
	int bsz = BBSIZE;

	if (!xi->disfile) {
		int fd;
		long long dummy;

		get_subvol_stripe_wrapper(dfile, SVTYPE_DATA,
				&ft->dsunit, &ft->dswidth, &ft->sectoralign);
		fd = open(dfile, O_RDONLY);
		/* If this fails we just fall back to BBSIZE */
		if (fd >= 0) {
			platform_findsizes(dfile, fd, &dummy, &bsz);
			close(fd);
		}
	}

	ft->sectorsize = bsz;

	if (xi->rtname && !xi->risfile) {
		int dummy1;

		get_subvol_stripe_wrapper(dfile, SVTYPE_RT, &dummy1,
					  &ft->rtswidth, &dummy1);
	}
}
#endif /* ENABLE_BLKID */

static void
fixup_log_stripe_unit(
	int		lsflag,
	int		sunit,
	xfs_drfsbno_t	*logblocks,
	int		blocklog)
{
	__uint64_t	tmp_logblocks;

	/*
	 * Make sure that the log size is a multiple of the stripe unit
	 */
	if ((*logblocks % sunit) != 0) {
		if (!lsflag) {
			tmp_logblocks = ((*logblocks + (sunit - 1))
						/ sunit) * sunit;
			/*
			 * If the log is too large, round down
			 * instead of round up
			 */
			if ((tmp_logblocks > XFS_MAX_LOG_BLOCKS) ||
			    ((tmp_logblocks << blocklog) > XFS_MAX_LOG_BYTES)) {
				tmp_logblocks = (*logblocks / sunit) * sunit;
			}
			*logblocks = tmp_logblocks;
		} else {
			fprintf(stderr, _("log size %lld is not a multiple "
					  "of the log stripe unit %d\n"),
				(long long) *logblocks, sunit);
			usage();
		}
	}
}

static xfs_dfsbno_t
fixup_internal_log_stripe(
	xfs_mount_t	*mp,
	int		lsflag,
	xfs_dfsbno_t	logstart,
	__uint64_t	agsize,
	int		sunit,
	xfs_drfsbno_t	*logblocks,
	int		blocklog,
	int		*lalign)
{
	if ((logstart % sunit) != 0) {
		logstart = ((logstart + (sunit - 1))/sunit) * sunit;
		*lalign = 1;
	}

	fixup_log_stripe_unit(lsflag, sunit, logblocks, blocklog);

	if (*logblocks > agsize - XFS_FSB_TO_AGBNO(mp, logstart)) {
		fprintf(stderr,
			_("Due to stripe alignment, the internal log size "
			"(%lld) is too large.\n"), (long long) *logblocks);
		fprintf(stderr, _("Must fit within an allocation group.\n"));
		usage();
	}
	return logstart;
}

void
validate_log_size(__uint64_t logblocks, int blocklog, int min_logblocks)
{
	if (logblocks < min_logblocks) {
		fprintf(stderr,
	_("log size %lld blocks too small, minimum size is %d blocks\n"),
			(long long)logblocks, min_logblocks);
		usage();
	}
	if (logblocks > XFS_MAX_LOG_BLOCKS) {
		fprintf(stderr,
	_("log size %lld blocks too large, maximum size is %lld blocks\n"),
			(long long)logblocks, XFS_MAX_LOG_BLOCKS);
		usage();
	}
	if ((logblocks << blocklog) > XFS_MAX_LOG_BYTES) {
		fprintf(stderr,
	_("log size %lld bytes too large, maximum size is %lld bytes\n"),
			(long long)(logblocks << blocklog), XFS_MAX_LOG_BYTES);
		usage();
	}
}

static int
calc_default_imaxpct(
	int		blocklog,
	__uint64_t	dblocks)
{
	/*
	 * This returns the % of the disk space that is used for
	 * inodes, it changes relatively to the FS size:
	 *  - over  50 TB, use 1%,
	 *  - 1TB - 50 TB, use 5%,
	 *  - under  1 TB, use XFS_DFL_IMAXIMUM_PCT (25%).
	 */

	if (dblocks < TERABYTES(1, blocklog)) {
		return XFS_DFL_IMAXIMUM_PCT;
	} else if (dblocks < TERABYTES(50, blocklog)) {
		return 5;
	}

	return 1;
}


void
calc_default_ag_geometry(
	int		blocklog,
	__uint64_t	dblocks,
	int		multidisk,
	__uint64_t	*agsize,
	__uint64_t	*agcount)
{
	__uint64_t	blocks = 0;
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
	 * For the remainder we choose an AG size based on the
	 * number of data blocks available, trying to keep the
	 * number of AGs relatively small (especially compared
	 * to the original algorithm).  AG count is calculated
	 * based on the preferred AG size, not vice-versa - the
	 * count can be increased by growfs, so prefer to use
	 * smaller counts at mkfs time.
	 *
	 * For a single underlying storage device between 128MB
	 * and 4TB in size, just use 4 AGs, otherwise scale up
	 * smoothly between min/max AG sizes.
	 */

	if (!multidisk && dblocks >= MEGABYTES(128, blocklog)) {
		if (dblocks >= TERABYTES(4, blocklog)) {
			blocks = XFS_AG_MAX_BLOCKS(blocklog);
			goto done;
		}
		shift = 2;
	} else if (dblocks > GIGABYTES(512, blocklog))
		shift = 5;
	else if (dblocks > GIGABYTES(8, blocklog))
		shift = 4;
	else if (dblocks >= MEGABYTES(128, blocklog))
		shift = 3;
	else if (dblocks >= MEGABYTES(64, blocklog))
		shift = 2;
	else if (dblocks >= MEGABYTES(32, blocklog))
		shift = 1;
	else
		shift = 0;
	/*
	 * If dblocks is not evenly divisible by the number of
	 * desired AGs, round "blocks" up so we don't lose the
	 * last bit of the filesystem. The same principle applies
	 * to the AG count, so we don't lose the last AG!
	 */
	blocks = dblocks >> shift;
	if (dblocks & xfs_mask32lo(shift)) {
		if (blocks < XFS_AG_MAX_BLOCKS(blocklog))
		    blocks++;
	}
done:
	*agsize = blocks;
	*agcount = dblocks / blocks + (dblocks % blocks != 0);
}

static void
validate_ag_geometry(
	int		blocklog,
	__uint64_t	dblocks,
	__uint64_t	agsize,
	__uint64_t	agcount)
{
	if (agsize < XFS_AG_MIN_BLOCKS(blocklog)) {
		fprintf(stderr,
	_("agsize (%lldb) too small, need at least %lld blocks\n"),
			(long long)agsize,
			(long long)XFS_AG_MIN_BLOCKS(blocklog));
		usage();
	}

	if (agsize > XFS_AG_MAX_BLOCKS(blocklog)) {
		fprintf(stderr,
	_("agsize (%lldb) too big, maximum is %lld blocks\n"),
			(long long)agsize,
			(long long)XFS_AG_MAX_BLOCKS(blocklog));
		usage();
	}

	if (agsize > dblocks) {
		fprintf(stderr,
	_("agsize (%lldb) too big, data area is %lld blocks\n"),
			(long long)agsize, (long long)dblocks);
			usage();
	}

	if (agsize < XFS_AG_MIN_BLOCKS(blocklog)) {
		fprintf(stderr,
	_("too many allocation groups for size = %lld\n"),
				(long long)agsize);
		fprintf(stderr, _("need at most %lld allocation groups\n"),
			(long long)(dblocks / XFS_AG_MIN_BLOCKS(blocklog) +
				(dblocks % XFS_AG_MIN_BLOCKS(blocklog) != 0)));
		usage();
	}

	if (agsize > XFS_AG_MAX_BLOCKS(blocklog)) {
		fprintf(stderr,
	_("too few allocation groups for size = %lld\n"), (long long)agsize);
		fprintf(stderr,
	_("need at least %lld allocation groups\n"),
		(long long)(dblocks / XFS_AG_MAX_BLOCKS(blocklog) +
			(dblocks % XFS_AG_MAX_BLOCKS(blocklog) != 0)));
		usage();
	}

	/*
	 * If the last AG is too small, reduce the filesystem size
	 * and drop the blocks.
	 */
	if ( dblocks % agsize != 0 &&
	     (dblocks % agsize < XFS_AG_MIN_BLOCKS(blocklog))) {
		fprintf(stderr,
	_("last AG size %lld blocks too small, minimum size is %lld blocks\n"),
			(long long)(dblocks % agsize),
			(long long)XFS_AG_MIN_BLOCKS(blocklog));
		usage();
	}

	/*
	 * If agcount is too large, make it smaller.
	 */
	if (agcount > XFS_MAX_AGNUMBER + 1) {
		fprintf(stderr,
	_("%lld allocation groups is too many, maximum is %lld\n"),
			(long long)agcount, (long long)XFS_MAX_AGNUMBER + 1);
		usage();
	}
}

static void
zero_old_xfs_structures(
	libxfs_init_t		*xi,
	xfs_sb_t		*new_sb)
{
	void 			*buf;
	xfs_sb_t 		sb;
	__uint32_t		bsize;
	int			i;
	xfs_off_t		off;

	/*
	 * read in existing filesystem superblock, use its geometry
	 * settings and zero the existing secondary superblocks.
	 */
	buf = memalign(libxfs_device_alignment(), new_sb->sb_sectsize);
	if (!buf) {
		fprintf(stderr,
	_("error reading existing superblock -- failed to memalign buffer\n"));
		return;
	}
	memset(buf, 0, new_sb->sb_sectsize);

	if (pread(xi->dfd, buf, new_sb->sb_sectsize, 0) != new_sb->sb_sectsize) {
		fprintf(stderr, _("existing superblock read failed: %s\n"),
			strerror(errno));
		free(buf);
		return;
	}
	libxfs_sb_from_disk(&sb, buf);

	/*
	 * perform same basic superblock validation to make sure we
	 * actually zero secondary blocks
	 */
	if (sb.sb_magicnum != XFS_SB_MAGIC || sb.sb_blocksize == 0)
		goto done;

	for (bsize = 1, i = 0; bsize < sb.sb_blocksize &&
			i < sizeof(sb.sb_blocksize) * NBBY; i++)
		bsize <<= 1;

	if (i < XFS_MIN_BLOCKSIZE_LOG || i > XFS_MAX_BLOCKSIZE_LOG ||
			i != sb.sb_blocklog)
		goto done;

	if (sb.sb_dblocks > ((__uint64_t)sb.sb_agcount * sb.sb_agblocks) ||
			sb.sb_dblocks < ((__uint64_t)(sb.sb_agcount - 1) *
					 sb.sb_agblocks + XFS_MIN_AG_BLOCKS))
		goto done;

	/*
	 * block size and basic geometry seems alright, zero the secondaries.
	 */
	memset(buf, 0, new_sb->sb_sectsize);
	off = 0;
	for (i = 1; i < sb.sb_agcount; i++)  {
		off += sb.sb_agblocks;
		if (pwrite64(xi->dfd, buf, new_sb->sb_sectsize,
					off << sb.sb_blocklog) == -1)
			break;
	}
done:
	free(buf);
}

static void
discard_blocks(dev_t dev, __uint64_t nsectors)
{
	int fd;

	/*
	 * We intentionally ignore errors from the discard ioctl.  It is
	 * not necessary for the mkfs functionality but just an optimization.
	 */
	fd = libxfs_device_to_fd(dev);
	if (fd > 0)
		platform_discard_blocks(fd, 0, nsectors << 9);
}

int
main(
	int			argc,
	char			**argv)
{
	__uint64_t		agcount;
	xfs_agf_t		*agf;
	xfs_agi_t		*agi;
	xfs_agnumber_t		agno;
	__uint64_t		agsize;
	xfs_alloc_rec_t		*arec;
	int			attrversion;
	int			projid32bit;
	struct xfs_btree_block	*block;
	int			blflag;
	int			blocklog;
	unsigned int		blocksize;
	int			bsflag;
	int			bsize;
	xfs_buf_t		*buf;
	int			c;
	int			daflag;
	int			dasize;
	xfs_drfsbno_t		dblocks;
	char			*dfile;
	int			dirblocklog;
	int			dirblocksize;
	int			dirversion;
	char			*dsize;
	int			dsu;
	int			dsw;
	int			dsunit;
	int			dswidth;
	int			force_overwrite;
	struct fsxattr		fsx;
	int			iaflag;
	int			ilflag;
	int			imaxpct;
	int			imflag;
	int			inodelog;
	int			inopblock;
	int			ipflag;
	int			isflag;
	int			isize;
	char			*label = NULL;
	int			laflag;
	int			lalign;
	int			ldflag;
	int			liflag;
	xfs_agnumber_t		logagno;
	xfs_drfsbno_t		logblocks;
	char			*logfile;
	int			loginternal;
	char			*logsize;
	xfs_dfsbno_t		logstart;
	int			logversion;
	int			lvflag;
	int			lsflag;
	int			lsectorlog;
	int			lsectorsize;
	int			lslflag;
	int			lssflag;
	int			lsu;
	int			lsunit;
	int			max_tr_res;
	int			min_logblocks;
	xfs_mount_t		*mp;
	xfs_mount_t		mbuf;
	xfs_extlen_t		nbmblocks;
	int			nlflag;
	int			nodsflag;
	int			norsflag;
	xfs_alloc_rec_t		*nrec;
	int			nsflag;
	int			nvflag;
	int			nci;
	int			Nflag;
	int			discard = 1;
	char			*p;
	char			*protofile;
	char			*protostring;
	int			qflag;
	xfs_drfsbno_t		rtblocks;
	xfs_extlen_t		rtextblocks;
	xfs_drtbno_t		rtextents;
	char			*rtextsize;
	char			*rtfile;
	char			*rtsize;
	xfs_sb_t		*sbp;
	int			sectorlog;
	unsigned int		sectorsize;
	__uint64_t		sector_mask;
	int			slflag;
	int			ssflag;
	__uint64_t		tmp_agsize;
	uuid_t			uuid;
	int			worst_freelist;
	libxfs_init_t		xi;
	struct fs_topology	ft;
	int			lazy_sb_counters;

	progname = basename(argv[0]);
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	attrversion = 2;
	projid32bit = 0;
	blflag = bsflag = slflag = ssflag = lslflag = lssflag = 0;
	blocklog = blocksize = 0;
	sectorlog = lsectorlog = XFS_MIN_SECTORSIZE_LOG;
	sectorsize = lsectorsize = XFS_MIN_SECTORSIZE;
	agsize = daflag = dasize = dblocks = 0;
	ilflag = imflag = ipflag = isflag = 0;
	liflag = laflag = lsflag = ldflag = lvflag = 0;
	loginternal = 1;
	logversion = 2;
	logagno = logblocks = rtblocks = rtextblocks = 0;
	Nflag = nlflag = nsflag = nvflag = nci = 0;
	dirblocklog = dirblocksize = 0;
	dirversion = XFS_DFL_DIR_VERSION;
	qflag = 0;
	imaxpct = inodelog = inopblock = isize = 0;
	iaflag = XFS_IFLAG_ALIGN;
	dfile = logfile = rtfile = NULL;
	dsize = logsize = rtsize = rtextsize = protofile = NULL;
	dsu = dsw = dsunit = dswidth = lalign = lsu = lsunit = 0;
	nodsflag = norsflag = 0;
	force_overwrite = 0;
	worst_freelist = 0;
	lazy_sb_counters = 1;
	memset(&fsx, 0, sizeof(fsx));

	memset(&xi, 0, sizeof(xi));
	xi.isdirect = LIBXFS_DIRECT;
	xi.isreadonly = LIBXFS_EXCLUSIVELY;

	while ((c = getopt(argc, argv, "b:d:i:l:L:n:KNp:qr:s:CfV")) != EOF) {
		switch (c) {
		case 'C':
		case 'f':
			force_overwrite = 1;
			break;
		case 'b':
			p = optarg;
			while (*p != '\0') {
				char	*value;

				switch (getsubopt(&p, (constpp)bopts, &value)) {
				case B_LOG:
					if (!value || *value == '\0')
						reqval('b', bopts, B_LOG);
					if (blflag)
						respec('b', bopts, B_LOG);
					if (bsflag)
						conflict('b', bopts, B_SIZE,
							 B_LOG);
					blocklog = atoi(value);
					if (blocklog <= 0)
						illegal(value, "b log");
					blocksize = 1 << blocklog;
					blflag = 1;
					break;
				case B_SIZE:
					if (!value || *value == '\0')
						reqval('b', bopts, B_SIZE);
					if (bsflag)
						respec('b', bopts, B_SIZE);
					if (blflag)
						conflict('b', bopts, B_LOG,
							 B_SIZE);
					blocksize = cvtnum(
						blocksize, sectorsize, value);
					if (blocksize <= 0 ||
					    !ispow2(blocksize))
						illegal(value, "b size");
					blocklog = libxfs_highbit32(blocksize);
					bsflag = 1;
					break;
				default:
					unknown('b', value);
				}
			}
			break;
		case 'd':
			p = optarg;
			while (*p != '\0') {
				char	*value;

				switch (getsubopt(&p, (constpp)dopts, &value)) {
				case D_AGCOUNT:
					if (!value || *value == '\0')
						reqval('d', dopts, D_AGCOUNT);
					if (daflag)
						respec('d', dopts, D_AGCOUNT);
					agcount = (__uint64_t)
						strtoul(value, NULL, 10);
					if ((__int64_t)agcount <= 0)
						illegal(value, "d agcount");
					daflag = 1;
					break;
				case D_AGSIZE:
					if (!value || *value == '\0')
						reqval('d', dopts, D_AGSIZE);
					if (dasize)
						respec('d', dopts, D_AGSIZE);
					agsize = cvtnum(
						blocksize, sectorsize, value);
					dasize = 1;
					break;
				case D_FILE:
					if (!value || *value == '\0')
						value = "1";
					xi.disfile = atoi(value);
					if (xi.disfile < 0 || xi.disfile > 1)
						illegal(value, "d file");
					if (xi.disfile && !Nflag)
						xi.dcreat = 1;
					break;
				case D_NAME:
					if (!value || *value == '\0')
						reqval('d', dopts, D_NAME);
					if (xi.dname)
						respec('d', dopts, D_NAME);
					xi.dname = value;
					break;
				case D_SIZE:
					if (!value || *value == '\0')
						reqval('d', dopts, D_SIZE);
					if (dsize)
						respec('d', dopts, D_SIZE);
					dsize = value;
					break;
				case D_SUNIT:
					if (!value || *value == '\0')
						reqval('d', dopts, D_SUNIT);
					if (dsunit)
						respec('d', dopts, D_SUNIT);
					if (nodsflag)
						conflict('d', dopts, D_NOALIGN,
							 D_SUNIT);
					if (!isdigits(value)) {
						fprintf(stderr,
	_("%s: Specify data sunit in 512-byte blocks, no unit suffix\n"),
							progname);
						exit(1);
					}
					dsunit = cvtnum(0, 0, value);
					break;
				case D_SWIDTH:
					if (!value || *value == '\0')
						reqval('d', dopts, D_SWIDTH);
					if (dswidth)
						respec('d', dopts, D_SWIDTH);
					if (nodsflag)
						conflict('d', dopts, D_NOALIGN,
							 D_SWIDTH);
					if (!isdigits(value)) {
						fprintf(stderr,
	_("%s: Specify data swidth in 512-byte blocks, no unit suffix\n"),
							progname);
						exit(1);
					}
					dswidth = cvtnum(0, 0, value);
					break;
				case D_SU:
					if (!value || *value == '\0')
						reqval('d', dopts, D_SU);
					if (dsu)
						respec('d', dopts, D_SU);
					if (nodsflag)
						conflict('d', dopts, D_NOALIGN,
							 D_SU);
					dsu = cvtnum(
						blocksize, sectorsize, value);
					break;
				case D_SW:
					if (!value || *value == '\0')
						reqval('d', dopts, D_SW);
					if (dsw)
						respec('d', dopts, D_SW);
					if (nodsflag)
						conflict('d', dopts, D_NOALIGN,
							 D_SW);
					if (!isdigits(value)) {
						fprintf(stderr,
		_("%s: Specify data sw as multiple of su, no unit suffix\n"),
							progname);
						exit(1);
					}
					dsw = cvtnum(0, 0, value);
					break;
				case D_NOALIGN:
					if (dsu)
						conflict('d', dopts, D_SU,
							 D_NOALIGN);
					if (dsunit)
						conflict('d', dopts, D_SUNIT,
							 D_NOALIGN);
					if (dsw)
						conflict('d', dopts, D_SW,
							 D_NOALIGN);
					if (dswidth)
						conflict('d', dopts, D_SWIDTH,
							 D_NOALIGN);
					nodsflag = 1;
					break;
				case D_SECTLOG:
					if (!value || *value == '\0')
						reqval('d', dopts, D_SECTLOG);
					if (slflag)
						respec('d', dopts, D_SECTLOG);
					if (ssflag)
						conflict('d', dopts, D_SECTSIZE,
							 D_SECTLOG);
					sectorlog = atoi(value);
					if (sectorlog <= 0)
						illegal(value, "d sectlog");
					sectorsize = 1 << sectorlog;
					slflag = 1;
					break;
				case D_SECTSIZE:
					if (!value || *value == '\0')
						reqval('d', dopts, D_SECTSIZE);
					if (ssflag)
						respec('d', dopts, D_SECTSIZE);
					if (slflag)
						conflict('d', dopts, D_SECTLOG,
							 D_SECTSIZE);
					sectorsize = cvtnum(
						blocksize, sectorsize, value);
					if (sectorsize <= 0 ||
					    !ispow2(sectorsize))
						illegal(value, "d sectsize");
					sectorlog =
						libxfs_highbit32(sectorsize);
					ssflag = 1;
					break;
				case D_RTINHERIT:
					fsx.fsx_xflags |= \
						XFS_DIFLAG_RTINHERIT;
					break;
				case D_PROJINHERIT:
					if (!value || *value == '\0')
						reqval('d', dopts, D_PROJINHERIT);
					fsx.fsx_projid = atoi(value);
					fsx.fsx_xflags |= \
						XFS_DIFLAG_PROJINHERIT;
					break;
				case D_EXTSZINHERIT:
					if (!value || *value == '\0')
						reqval('d', dopts, D_EXTSZINHERIT);
					fsx.fsx_extsize = atoi(value);
					fsx.fsx_xflags |= \
						XFS_DIFLAG_EXTSZINHERIT;
					break;
				default:
					unknown('d', value);
				}
			}
			break;
		case 'i':
			p = optarg;
			while (*p != '\0') {
				char	*value;

				switch (getsubopt(&p, (constpp)iopts, &value)) {
				case I_ALIGN:
					if (!value || *value == '\0')
						value = "1";
					iaflag = atoi(value);
					if (iaflag < 0 || iaflag > 1)
						illegal(value, "i align");
					break;
				case I_LOG:
					if (!value || *value == '\0')
						reqval('i', iopts, I_LOG);
					if (ilflag)
						respec('i', iopts, I_LOG);
					if (ipflag)
						conflict('i', iopts, I_PERBLOCK,
							 I_LOG);
					if (isflag)
						conflict('i', iopts, I_SIZE,
							 I_LOG);
					inodelog = atoi(value);
					if (inodelog <= 0)
						illegal(value, "i log");
					isize = 1 << inodelog;
					ilflag = 1;
					break;
				case I_MAXPCT:
					if (!value || *value == '\0')
						reqval('i', iopts, I_MAXPCT);
					if (imflag)
						respec('i', iopts, I_MAXPCT);
					imaxpct = atoi(value);
					if (imaxpct < 0 || imaxpct > 100)
						illegal(value, "i maxpct");
					imflag = 1;
					break;
				case I_PERBLOCK:
					if (!value || *value == '\0')
						reqval('i', iopts, I_PERBLOCK);
					if (ilflag)
						conflict('i', iopts, I_LOG,
							 I_PERBLOCK);
					if (ipflag)
						respec('i', iopts, I_PERBLOCK);
					if (isflag)
						conflict('i', iopts, I_SIZE,
							 I_PERBLOCK);
					inopblock = atoi(value);
					if (inopblock <
						XFS_MIN_INODE_PERBLOCK ||
					    !ispow2(inopblock))
						illegal(value, "i perblock");
					ipflag = 1;
					break;
				case I_SIZE:
					if (!value || *value == '\0')
						reqval('i', iopts, I_SIZE);
					if (ilflag)
						conflict('i', iopts, I_LOG,
							 I_SIZE);
					if (ipflag)
						conflict('i', iopts, I_PERBLOCK,
							 I_SIZE);
					if (isflag)
						respec('i', iopts, I_SIZE);
					isize = cvtnum(0, 0, value);
					if (isize <= 0 || !ispow2(isize))
						illegal(value, "i size");
					inodelog = libxfs_highbit32(isize);
					isflag = 1;
					break;
				case I_ATTR:
					if (!value || *value == '\0')
						reqval('i', iopts, I_ATTR);
					c = atoi(value);
					if (c < 0 || c > 2)
						illegal(value, "i attr");
					attrversion = c;
					break;
				case I_PROJID32BIT:
					if (!value || *value == '\0')
						value = "0";
					c = atoi(value);
					if (c < 0 || c > 1)
						illegal(value, "i projid32bit");
					projid32bit = c;
					break;
				default:
					unknown('i', value);
				}
			}
			break;
		case 'l':
			p = optarg;
			while (*p != '\0') {
				char	*value;

				switch (getsubopt(&p, (constpp)lopts, &value)) {
				case L_AGNUM:
					if (!value || *value == '\0')
						reqval('l', lopts, L_AGNUM);
					if (laflag)
						respec('l', lopts, L_AGNUM);
					if (ldflag)
						conflict('l', lopts, L_AGNUM, L_DEV);
					logagno = atoi(value);
					laflag = 1;
					break;
				case L_FILE:
					if (!value || *value == '\0')
						value = "1";
					if (loginternal)
						conflict('l', lopts, L_INTERNAL,
							 L_FILE);
					xi.lisfile = atoi(value);
					if (xi.lisfile < 0 || xi.lisfile > 1)
						illegal(value, "l file");
					if (xi.lisfile)
						xi.lcreat = 1;
					break;
				case L_INTERNAL:
					if (!value || *value == '\0')
						value = "1";
					if (ldflag)
						conflict('l', lopts, L_INTERNAL, L_DEV);
					if (xi.lisfile)
						conflict('l', lopts, L_FILE,
							 L_INTERNAL);
					if (liflag)
						respec('l', lopts, L_INTERNAL);
					loginternal = atoi(value);
					if (loginternal < 0 || loginternal > 1)
						illegal(value, "l internal");
					liflag = 1;
					break;
				case L_SU:
					if (!value || *value == '\0')
						reqval('l', lopts, L_SU);
					if (lsu)
						respec('l', lopts, L_SU);
					lsu = cvtnum(
						blocksize, sectorsize, value);
					break;
				case L_SUNIT:
					if (!value || *value == '\0')
						reqval('l', lopts, L_SUNIT);
					if (lsunit)
						respec('l', lopts, L_SUNIT);
					if (!isdigits(value)) {
						fprintf(stderr,
		_("Specify log sunit in 512-byte blocks, no size suffix\n"));
						usage();
					}
					lsunit = cvtnum(0, 0, value);
					break;
				case L_NAME:
				case L_DEV:
					if (laflag)
						conflict('l', lopts, L_AGNUM, L_DEV);
					if (liflag)
						conflict('l', lopts, L_INTERNAL, L_DEV);
					if (!value || *value == '\0')
						reqval('l', lopts, L_NAME);
					if (xi.logname)
						respec('l', lopts, L_NAME);
					ldflag = 1;
					loginternal = 0;
					logfile = value;
					xi.logname = value;
					break;
				case L_VERSION:
					if (!value || *value == '\0')
						reqval('l', lopts, L_VERSION);
					if (lvflag)
						respec('l', lopts, L_VERSION);
					logversion = atoi(value);
					if (logversion < 1 || logversion > 2)
						illegal(value, "l version");
					lvflag = 1;
					break;
				case L_SIZE:
					if (!value || *value == '\0')
						reqval('l', lopts, L_SIZE);
					if (logsize)
						respec('l', lopts, L_SIZE);
					logsize = value;
					lsflag = 1;
					break;
				case L_SECTLOG:
					if (!value || *value == '\0')
						reqval('l', lopts, L_SECTLOG);
					if (lslflag)
						respec('l', lopts, L_SECTLOG);
					if (lssflag)
						conflict('l', lopts, L_SECTSIZE,
							 L_SECTLOG);
					lsectorlog = atoi(value);
					if (lsectorlog <= 0)
						illegal(value, "l sectlog");
					lsectorsize = 1 << lsectorlog;
					lslflag = 1;
					break;
				case L_SECTSIZE:
					if (!value || *value == '\0')
						reqval('l', lopts, L_SECTSIZE);
					if (lssflag)
						respec('l', lopts, L_SECTSIZE);
					if (lslflag)
						conflict('l', lopts, L_SECTLOG,
							 L_SECTSIZE);
					lsectorsize = cvtnum(
						blocksize, sectorsize, value);
					if (lsectorsize <= 0 ||
					    !ispow2(lsectorsize))
						illegal(value, "l sectsize");
					lsectorlog =
						libxfs_highbit32(lsectorsize);
					lssflag = 1;
					break;
				case L_LAZYSBCNTR:
					if (!value || *value == '\0')
						reqval('l', lopts,
								L_LAZYSBCNTR);
					c = atoi(value);
					if (c < 0 || c > 1)
						illegal(value, "l lazy-count");
					lazy_sb_counters = c;
					break;
				default:
					unknown('l', value);
				}
			}
			break;
		case 'L':
			if (strlen(optarg) > sizeof(sbp->sb_fname))
				illegal(optarg, "L");
			label = optarg;
			break;
		case 'n':
			p = optarg;
			while (*p != '\0') {
				char	*value;

				switch (getsubopt(&p, (constpp)nopts, &value)) {
				case N_LOG:
					if (!value || *value == '\0')
						reqval('n', nopts, N_LOG);
					if (nlflag)
						respec('n', nopts, N_LOG);
					if (nsflag)
						conflict('n', nopts, N_SIZE,
							 N_LOG);
					dirblocklog = atoi(value);
					if (dirblocklog <= 0)
						illegal(value, "n log");
					dirblocksize = 1 << dirblocklog;
					nlflag = 1;
					break;
				case N_SIZE:
					if (!value || *value == '\0')
						reqval('n', nopts, N_SIZE);
					if (nsflag)
						respec('n', nopts, N_SIZE);
					if (nlflag)
						conflict('n', nopts, N_LOG,
							 N_SIZE);
					dirblocksize = cvtnum(
						blocksize, sectorsize, value);
					if (dirblocksize <= 0 ||
					    !ispow2(dirblocksize))
						illegal(value, "n size");
					dirblocklog =
						libxfs_highbit32(dirblocksize);
					nsflag = 1;
					break;
				case N_VERSION:
					if (!value || *value == '\0')
						reqval('n', nopts, N_VERSION);
					if (nvflag)
						respec('n', nopts, N_VERSION);
					if (!strcasecmp(value, "ci")) {
						nci = 1; /* ASCII CI mode */
					} else {
						dirversion = atoi(value);
						if (dirversion != 2)
							illegal(value,
								"n version");
					}
					nvflag = 1;
					break;
				default:
					unknown('n', value);
				}
			}
			break;
		case 'N':
			Nflag = 1;
			break;
		case 'K':
			discard = 0;
			break;
		case 'p':
			if (protofile)
				respec('p', NULL, 0);
			protofile = optarg;
			break;
		case 'q':
			qflag = 1;
			break;
		case 'r':
			p = optarg;
			while (*p != '\0') {
				char	*value;

				switch (getsubopt(&p, (constpp)ropts, &value)) {
				case R_EXTSIZE:
					if (!value || *value == '\0')
						reqval('r', ropts, R_EXTSIZE);
					if (rtextsize)
						respec('r', ropts, R_EXTSIZE);
					rtextsize = value;
					break;
				case R_FILE:
					if (!value || *value == '\0')
						value = "1";
					xi.risfile = atoi(value);
					if (xi.risfile < 0 || xi.risfile > 1)
						illegal(value, "r file");
					if (xi.risfile)
						xi.rcreat = 1;
					break;
				case R_NAME:
				case R_DEV:
					if (!value || *value == '\0')
						reqval('r', ropts, R_NAME);
					if (xi.rtname)
						respec('r', ropts, R_NAME);
					xi.rtname = value;
					break;
				case R_SIZE:
					if (!value || *value == '\0')
						reqval('r', ropts, R_SIZE);
					if (rtsize)
						respec('r', ropts, R_SIZE);
					rtsize = value;
					break;
				case R_NOALIGN:
					norsflag = 1;
					break;
				default:
					unknown('r', value);
				}
			}
			break;
		case 's':
			p = optarg;
			while (*p != '\0') {
				char	*value;

				switch (getsubopt(&p, (constpp)sopts, &value)) {
				case S_LOG:
				case S_SECTLOG:
					if (!value || *value == '\0')
						reqval('s', sopts, S_SECTLOG);
					if (slflag || lslflag)
						respec('s', sopts, S_SECTLOG);
					if (ssflag || lssflag)
						conflict('s', sopts, S_SECTSIZE,
							 S_SECTLOG);
					sectorlog = atoi(value);
					if (sectorlog <= 0)
						illegal(value, "s sectlog");
					lsectorlog = sectorlog;
					sectorsize = 1 << sectorlog;
					lsectorsize = sectorsize;
					lslflag = slflag = 1;
					break;
				case S_SIZE:
				case S_SECTSIZE:
					if (!value || *value == '\0')
						reqval('s', sopts, S_SECTSIZE);
					if (ssflag || lssflag)
						respec('s', sopts, S_SECTSIZE);
					if (slflag || lslflag)
						conflict('s', sopts, S_SECTLOG,
							 S_SECTSIZE);
					sectorsize = cvtnum(
						blocksize, sectorsize, value);
					if (sectorsize <= 0 ||
					    !ispow2(sectorsize))
						illegal(value, "s sectsize");
					lsectorsize = sectorsize;
					sectorlog =
						libxfs_highbit32(sectorsize);
					lsectorlog = sectorlog;
					lssflag = ssflag = 1;
					break;
				default:
					unknown('s', value);
				}
			}
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		case '?':
			unknown(optopt, "");
		}
	}
	if (argc - optind > 1) {
		fprintf(stderr, _("extra arguments\n"));
		usage();
	} else if (argc - optind == 1) {
		dfile = xi.volname = argv[optind];
		if (xi.dname) {
			fprintf(stderr,
				_("cannot specify both %s and -d name=%s\n"),
				xi.volname, xi.dname);
			usage();
		}
	} else
		dfile = xi.dname;

	/*
	 * Blocksize and sectorsize first, other things depend on them
	 * For RAID4/5/6 we want to align sector size and block size,
	 * so we need to start with the device geometry extraction too.
	 */
	if (!blflag && !bsflag) {
		blocklog = XFS_DFL_BLOCKSIZE_LOG;
		blocksize = 1 << XFS_DFL_BLOCKSIZE_LOG;
	}
	if (blocksize < XFS_MIN_BLOCKSIZE || blocksize > XFS_MAX_BLOCKSIZE) {
		fprintf(stderr, _("illegal block size %d\n"), blocksize);
		usage();
	}

	memset(&ft, 0, sizeof(ft));
	get_topology(&xi, &ft);

	if (ft.sectoralign) {
		/*
		 * Older Linux software RAID versions want the sector size
		 * to match the block size to avoid switching I/O sizes.
		 * For the legacy libdisk case we thus set the sector size to
		 * match the block size.  For systems using libblkid we assume
		 * that the kernel is recent enough to not require this and
		 * ft.sectoralign will never be set.
		 */
		sectorsize = blocksize;
	} else if (!ssflag) {
		/*
		 * Unless specified manually on the command line use the
		 * advertised sector size of the device.
		 */
		sectorsize = ft.sectorsize ? ft.sectorsize : XFS_MIN_SECTORSIZE;
	}

	if (ft.sectoralign || !ssflag) {
		sectorlog = libxfs_highbit32(sectorsize);
		if (loginternal) {
			lsectorsize = sectorsize;
			lsectorlog = sectorlog;
		}
	}

	if (sectorsize < XFS_MIN_SECTORSIZE ||
	    sectorsize > XFS_MAX_SECTORSIZE || sectorsize > blocksize) {
		fprintf(stderr, _("illegal sector size %d\n"), sectorsize);
		usage();
	}
	if (sectorsize < ft.sectorsize) {
		fprintf(stderr, _("illegal sector size %d; hw sector is %d\n"),
			sectorsize, ft.sectorsize);
		usage();
	}
	if (lsectorsize < XFS_MIN_SECTORSIZE ||
	    lsectorsize > XFS_MAX_SECTORSIZE || lsectorsize > blocksize) {
		fprintf(stderr, _("illegal log sector size %d\n"), lsectorsize);
		usage();
	} else if (lsectorsize > XFS_MIN_SECTORSIZE && !lsu && !lsunit) {
		lsu = blocksize;
		logversion = 2;
	}

	if (nsflag || nlflag) {
		if (dirblocksize < blocksize ||
					dirblocksize > XFS_MAX_BLOCKSIZE) {
			fprintf(stderr, _("illegal directory block size %d\n"),
				dirblocksize);
			usage();
		}
	} else {
		if (blocksize < (1 << XFS_MIN_REC_DIRSIZE))
			dirblocklog = XFS_MIN_REC_DIRSIZE;
		else
			dirblocklog = blocklog;
		dirblocksize = 1 << dirblocklog;
	}

	if (daflag && dasize) {
		fprintf(stderr,
	_("both -d agcount= and agsize= specified, use one or the other\n"));
		usage();
	}

	if (xi.disfile && (!dsize || !xi.dname)) {
		fprintf(stderr,
	_("if -d file then -d name and -d size are required\n"));
		usage();
	}
	if (dsize) {
		__uint64_t dbytes;

		dbytes = cvtnum(blocksize, sectorsize, dsize);
		if (dbytes % XFS_MIN_BLOCKSIZE) {
			fprintf(stderr,
			_("illegal data length %lld, not a multiple of %d\n"),
				(long long)dbytes, XFS_MIN_BLOCKSIZE);
			usage();
		}
		dblocks = (xfs_drfsbno_t)(dbytes >> blocklog);
		if (dbytes % blocksize)
			fprintf(stderr, _("warning: "
	"data length %lld not a multiple of %d, truncated to %lld\n"),
				(long long)dbytes, blocksize,
				(long long)(dblocks << blocklog));
	}
	if (ipflag) {
		inodelog = blocklog - libxfs_highbit32(inopblock);
		isize = 1 << inodelog;
	} else if (!ilflag && !isflag) {
		inodelog = XFS_DINODE_DFL_LOG;
		isize = 1 << inodelog;
	}
	if (xi.lisfile && (!logsize || !xi.logname)) {
		fprintf(stderr,
		_("if -l file then -l name and -l size are required\n"));
		usage();
	}
	if (logsize) {
		__uint64_t logbytes;

		logbytes = cvtnum(blocksize, sectorsize, logsize);
		if (logbytes % XFS_MIN_BLOCKSIZE) {
			fprintf(stderr,
			_("illegal log length %lld, not a multiple of %d\n"),
				(long long)logbytes, XFS_MIN_BLOCKSIZE);
			usage();
		}
		logblocks = (xfs_drfsbno_t)(logbytes >> blocklog);
		if (logbytes % blocksize)
			fprintf(stderr,
	_("warning: log length %lld not a multiple of %d, truncated to %lld\n"),
				(long long)logbytes, blocksize,
				(long long)(logblocks << blocklog));
	}
	if (xi.risfile && (!rtsize || !xi.rtname)) {
		fprintf(stderr,
		_("if -r file then -r name and -r size are required\n"));
		usage();
	}
	if (rtsize) {
		__uint64_t rtbytes;

		rtbytes = cvtnum(blocksize, sectorsize, rtsize);
		if (rtbytes % XFS_MIN_BLOCKSIZE) {
			fprintf(stderr,
			_("illegal rt length %lld, not a multiple of %d\n"),
				(long long)rtbytes, XFS_MIN_BLOCKSIZE);
			usage();
		}
		rtblocks = (xfs_drfsbno_t)(rtbytes >> blocklog);
		if (rtbytes % blocksize)
			fprintf(stderr,
	_("warning: rt length %lld not a multiple of %d, truncated to %lld\n"),
				(long long)rtbytes, blocksize,
				(long long)(rtblocks << blocklog));
	}
	/*
	 * If specified, check rt extent size against its constraints.
	 */
	if (rtextsize) {
		__uint64_t rtextbytes;

		rtextbytes = cvtnum(blocksize, sectorsize, rtextsize);
		if (rtextbytes % blocksize) {
			fprintf(stderr,
		_("illegal rt extent size %lld, not a multiple of %d\n"),
				(long long)rtextbytes, blocksize);
			usage();
		}
		if (rtextbytes > XFS_MAX_RTEXTSIZE) {
			fprintf(stderr,
				_("rt extent size %s too large, maximum %d\n"),
				rtextsize, XFS_MAX_RTEXTSIZE);
			usage();
		}
		if (rtextbytes < XFS_MIN_RTEXTSIZE) {
			fprintf(stderr,
				_("rt extent size %s too small, minimum %d\n"),
				rtextsize, XFS_MIN_RTEXTSIZE);
			usage();
		}
		rtextblocks = (xfs_extlen_t)(rtextbytes >> blocklog);
	} else {
		/*
		 * If realtime extsize has not been specified by the user,
		 * and the underlying volume is striped, then set rtextblocks
		 * to the stripe width.
		 */
		int		rswidth;
		__uint64_t	rtextbytes;

		rswidth = 0;

		if (!norsflag && !xi.risfile && !(!rtsize && xi.disfile))
			rswidth = ft.rtswidth;
		else
			rswidth = 0;

		/* check that rswidth is a multiple of fs blocksize */
		if (!norsflag && rswidth && !(BBTOB(rswidth) % blocksize)) {
			rswidth = DTOBT(rswidth);
			rtextbytes = rswidth << blocklog;
			if (XFS_MIN_RTEXTSIZE <= rtextbytes &&
			    (rtextbytes <= XFS_MAX_RTEXTSIZE)) {
				rtextblocks = rswidth;
			}
		}
		if (!rtextblocks) {
			rtextblocks = (blocksize < XFS_MIN_RTEXTSIZE) ?
					XFS_MIN_RTEXTSIZE >> blocklog : 1;
		}
	}
	ASSERT(rtextblocks);

	/*
	 * Check some argument sizes against mins, maxes.
	 */
	if (isize > blocksize / XFS_MIN_INODE_PERBLOCK ||
	    isize < XFS_DINODE_MIN_SIZE ||
	    isize > XFS_DINODE_MAX_SIZE) {
		int	maxsz;

		fprintf(stderr, _("illegal inode size %d\n"), isize);
		maxsz = MIN(blocksize / XFS_MIN_INODE_PERBLOCK,
			    XFS_DINODE_MAX_SIZE);
		if (XFS_DINODE_MIN_SIZE == maxsz)
			fprintf(stderr,
			_("allowable inode size with %d byte blocks is %d\n"),
				blocksize, XFS_DINODE_MIN_SIZE);
		else
			fprintf(stderr,
	_("allowable inode size with %d byte blocks is between %d and %d\n"),
				blocksize, XFS_DINODE_MIN_SIZE, maxsz);
		usage();
	}

	/* if lsu or lsunit was specified, automatically use v2 logs */
	if ((lsu || lsunit) && logversion == 1) {
		fprintf(stderr,
			_("log stripe unit specified, using v2 logs\n"));
		logversion = 2;
	}

	calc_stripe_factors(dsu, dsw, sectorsize, lsu, lsectorsize,
				&dsunit, &dswidth, &lsunit);

	xi.setblksize = sectorsize;

	/*
	 * Initialize.  This will open the log and rt devices as well.
	 */
	if (!libxfs_init(&xi))
		usage();
	if (!xi.ddev) {
		fprintf(stderr, _("no device name given in argument list\n"));
		usage();
	}

	/*
	 * Ok, Linux only has a 1024-byte resolution on device _size_,
	 * and the sizes below are in basic 512-byte blocks,
	 * so if we have (size % 2), on any partition, we can't get
	 * to the last 512 bytes.  The same issue exists for larger
	 * sector sizes - we cannot write past the last sector.
	 *
	 * So, we reduce the size (in basic blocks) to a perfect
	 * multiple of the sector size, or 1024, whichever is larger.
	 */

	sector_mask = (__uint64_t)-1 << (MAX(sectorlog, 10) - BBSHIFT);
	xi.dsize &= sector_mask;
	xi.rtsize &= sector_mask;
	xi.logBBsize &= (__uint64_t)-1 << (MAX(lsectorlog, 10) - BBSHIFT);

	if (!force_overwrite) {
		if (check_overwrite(dfile) ||
		    check_overwrite(logfile) ||
		    check_overwrite(xi.rtname)) {
			fprintf(stderr,
			_("%s: Use the -f option to force overwrite.\n"),
				progname);
			exit(1);
		}
	}

	if (discard) {
		discard_blocks(xi.ddev, xi.dsize);
		if (xi.rtdev)
			discard_blocks(xi.rtdev, xi.rtsize);
		if (xi.logdev && xi.logdev != xi.ddev)
			discard_blocks(xi.logdev, xi.logBBsize);
	}

	if (!liflag && !ldflag)
		loginternal = xi.logdev == 0;
	if (xi.logname)
		logfile = xi.logname;
	else if (loginternal)
		logfile = _("internal log");
	else if (xi.volname && xi.logdev)
		logfile = _("volume log");
	else if (!ldflag) {
		fprintf(stderr, _("no log subvolume or internal log\n"));
		usage();
	}
	if (xi.rtname)
		rtfile = xi.rtname;
	else
	if (xi.volname && xi.rtdev)
		rtfile = _("volume rt");
	else if (!xi.rtdev)
		rtfile = _("none");
	if (dsize && xi.dsize > 0 && dblocks > DTOBT(xi.dsize)) {
		fprintf(stderr,
			_("size %s specified for data subvolume is too large, "
			"maximum is %lld blocks\n"),
			dsize, (long long)DTOBT(xi.dsize));
		usage();
	} else if (!dsize && xi.dsize > 0)
		dblocks = DTOBT(xi.dsize);
	else if (!dsize) {
		fprintf(stderr, _("can't get size of data subvolume\n"));
		usage();
	}
	if (dblocks < XFS_MIN_DATA_BLOCKS) {
		fprintf(stderr,
	_("size %lld of data subvolume is too small, minimum %d blocks\n"),
			(long long)dblocks, XFS_MIN_DATA_BLOCKS);
		usage();
	}

	if (loginternal && xi.logdev) {
		fprintf(stderr,
			_("can't have both external and internal logs\n"));
		usage();
	} else if (loginternal && sectorsize != lsectorsize) {
		fprintf(stderr,
	_("data and log sector sizes must be equal for internal logs\n"));
		usage();
	}

	if (xi.dbsize > sectorsize) {
		fprintf(stderr, _(
"Warning: the data subvolume sector size %u is less than the sector size \n\
reported by the device (%u).\n"),
			sectorsize, xi.dbsize);
	}
	if (!loginternal && xi.lbsize > lsectorsize) {
		fprintf(stderr, _(
"Warning: the log subvolume sector size %u is less than the sector size\n\
reported by the device (%u).\n"),
			lsectorsize, xi.lbsize);
	}
	if (rtsize && xi.rtsize > 0 && xi.rtbsize > sectorsize) {
		fprintf(stderr, _(
"Warning: the realtime subvolume sector size %u is less than the sector size\n\
reported by the device (%u).\n"),
			sectorsize, xi.rtbsize);
	}

	max_tr_res = max_trans_res(dirversion,
				   sectorlog, blocklog, inodelog, dirblocklog);
	ASSERT(max_tr_res);
	min_logblocks = max_tr_res * XFS_MIN_LOG_FACTOR;
	min_logblocks = MAX(XFS_MIN_LOG_BLOCKS, min_logblocks);
	if (!logsize && dblocks >= (1024*1024*1024) >> blocklog)
		min_logblocks = MAX(min_logblocks, XFS_MIN_LOG_BYTES>>blocklog);
	if (logsize && xi.logBBsize > 0 && logblocks > DTOBT(xi.logBBsize)) {
		fprintf(stderr,
_("size %s specified for log subvolume is too large, maximum is %lld blocks\n"),
			logsize, (long long)DTOBT(xi.logBBsize));
		usage();
	} else if (!logsize && xi.logBBsize > 0) {
		logblocks = DTOBT(xi.logBBsize);
	} else if (logsize && !xi.logdev && !loginternal) {
		fprintf(stderr,
			_("size specified for non-existent log subvolume\n"));
		usage();
	} else if (loginternal && logsize && logblocks >= dblocks) {
		fprintf(stderr, _("size %lld too large for internal log\n"),
			(long long)logblocks);
		usage();
	} else if (!loginternal && !xi.logdev) {
		logblocks = 0;
	} else if (loginternal && !logsize) {
		/*
		 * With a 2GB max log size, default to maximum size
		 * at 4TB. This keeps the same ratio from the older
		 * max log size of 128M at 256GB fs size. IOWs,
		 * the ratio of fs size to log size is 2048:1.
		 */
		logblocks = (dblocks << blocklog) / 2048;
		logblocks = logblocks >> blocklog;
		logblocks = MAX(min_logblocks, logblocks);
		logblocks = MAX(logblocks,
				MAX(XFS_DFL_LOG_SIZE,
					max_tr_res * XFS_DFL_LOG_FACTOR));
		logblocks = MIN(logblocks, XFS_MAX_LOG_BLOCKS);
		if ((logblocks << blocklog) > XFS_MAX_LOG_BYTES) {
			logblocks = XFS_MAX_LOG_BYTES >> blocklog;
		}
	}
	validate_log_size(logblocks, blocklog, min_logblocks);

	if (rtsize && xi.rtsize > 0 && rtblocks > DTOBT(xi.rtsize)) {
		fprintf(stderr,
			_("size %s specified for rt subvolume is too large, "
			"maximum is %lld blocks\n"),
			rtsize, (long long)DTOBT(xi.rtsize));
		usage();
	} else if (!rtsize && xi.rtsize > 0)
		rtblocks = DTOBT(xi.rtsize);
	else if (rtsize && !xi.rtdev) {
		fprintf(stderr,
			_("size specified for non-existent rt subvolume\n"));
		usage();
	}
	if (xi.rtdev) {
		rtextents = rtblocks / rtextblocks;
		nbmblocks = (xfs_extlen_t)howmany(rtextents, NBBY * blocksize);
	} else {
		rtextents = rtblocks = 0;
		nbmblocks = 0;
	}

	if (dasize) {		/* User-specified AG size */
		/*
		 * Check specified agsize is a multiple of blocksize.
		 */
		if (agsize % blocksize) {
			fprintf(stderr,
		_("agsize (%lld) not a multiple of fs blk size (%d)\n"),
				(long long)agsize, blocksize);
			usage();
		}
		agsize /= blocksize;
		agcount = dblocks / agsize + (dblocks % agsize != 0);

	} else if (daflag)	/* User-specified AG count */
		agsize = dblocks / agcount + (dblocks % agcount != 0);
	else
		calc_default_ag_geometry(blocklog, dblocks,
				ft.dsunit | ft.dswidth, &agsize, &agcount);

	if (!nodsflag) {
		if (dsunit) {
			if (ft.dsunit && ft.dsunit != dsunit) {
				fprintf(stderr,
					_("%s: Specified data stripe unit %d "
					"is not the same as the volume stripe "
					"unit %d\n"),
					progname, dsunit, ft.dsunit);
			}
			if (ft.dswidth && ft.dswidth != dswidth) {
				fprintf(stderr,
					_("%s: Specified data stripe width %d "
					"is not the same as the volume stripe "
					"width %d\n"),
					progname, dswidth, ft.dswidth);
			}
		} else {
			dsunit = ft.dsunit;
			dswidth = ft.dswidth;
			nodsflag = 1;
		}
	} /* else dsunit & dswidth can't be set if nodsflag is set */

	/*
	 * If dsunit is a multiple of fs blocksize, then check that is a
	 * multiple of the agsize too
	 */
	if (dsunit && !(BBTOB(dsunit) % blocksize) &&
	    dswidth && !(BBTOB(dswidth) % blocksize)) {

		/* convert from 512 byte blocks to fs blocksize */
		dsunit = DTOBT(dsunit);
		dswidth = DTOBT(dswidth);

		/*
		 * agsize is not a multiple of dsunit
		 */
		if ((agsize % dsunit) != 0) {
			/*
			 * Round up to stripe unit boundary. Also make sure
			 * that agsize is still larger than
			 * XFS_AG_MIN_BLOCKS(blocklog)
		 	 */
			tmp_agsize = ((agsize + (dsunit - 1))/ dsunit) * dsunit;
			/*
			 * Round down to stripe unit boundary if rounding up
			 * created an AG size that is larger than the AG max.
			 */
			if (tmp_agsize > XFS_AG_MAX_BLOCKS(blocklog))
				tmp_agsize = ((agsize) / dsunit) * dsunit;

			if ((tmp_agsize >= XFS_AG_MIN_BLOCKS(blocklog)) &&
			    (tmp_agsize <= XFS_AG_MAX_BLOCKS(blocklog))) {
				agsize = tmp_agsize;
				if (!daflag)
					agcount = dblocks/agsize +
						(dblocks % agsize != 0);
				if (dasize)
					fprintf(stderr,
				_("agsize rounded to %lld, swidth = %d\n"),
						(long long)agsize, dswidth);
			} else {
				if (nodsflag) {
					dsunit = dswidth = 0;
				} else {
					fprintf(stderr,
_("Allocation group size (%lld) is not a multiple of the stripe unit (%d)\n"),
						(long long)agsize, dsunit);
					exit(1);
				}
			}
		}
		if (dswidth && ((agsize % dswidth) == 0) && (agcount > 1)) {
			/* This is a non-optimal configuration because all AGs
			 * start on the same disk in the stripe.  Changing
			 * the AG size by one sunit will guarantee that this
			 * does not happen.
			 */
			tmp_agsize = agsize - dsunit;
			if (tmp_agsize < XFS_AG_MIN_BLOCKS(blocklog)) {
				tmp_agsize = agsize + dsunit;
				if (dblocks < agsize) {
					/* oh well, nothing to do */
					tmp_agsize = agsize;
				}
			}
			if (daflag || dasize) {
				fprintf(stderr, _(
"Warning: AG size is a multiple of stripe width.  This can cause performance\n\
problems by aligning all AGs on the same disk.  To avoid this, run mkfs with\n\
an AG size that is one stripe unit smaller, for example %llu.\n"),
					(unsigned long long)tmp_agsize);
			} else {
				agsize = tmp_agsize;
				agcount = dblocks/agsize + (dblocks % agsize != 0);
				/*
				 * If the last AG is too small, reduce the
				 * filesystem size and drop the blocks.
				 */
				if ( dblocks % agsize != 0 &&
				    (dblocks % agsize <
				    XFS_AG_MIN_BLOCKS(blocklog))) {
					dblocks = (xfs_drfsbno_t)((agcount - 1) * agsize);
					agcount--;
					ASSERT(agcount != 0);
				}
			}
		}
	} else {
		if (nodsflag)
			dsunit = dswidth = 0;
		else {
			fprintf(stderr,
				_("%s: Stripe unit(%d) or stripe width(%d) is "
				"not a multiple of the block size(%d)\n"),
				progname, BBTOB(dsunit), BBTOB(dswidth),
				blocksize);
			exit(1);
		}
	}

	/*
	 * If the last AG is too small, reduce the filesystem size
	 * and drop the blocks.
	 */
	if ( dblocks % agsize != 0 &&
	     (dblocks % agsize < XFS_AG_MIN_BLOCKS(blocklog))) {
		ASSERT(!daflag);
		dblocks = (xfs_drfsbno_t)((agcount - 1) * agsize);
		agcount--;
		ASSERT(agcount != 0);
	}

	validate_ag_geometry(blocklog, dblocks, agsize, agcount);

	if (!imflag)
		imaxpct = calc_default_imaxpct(blocklog, dblocks);

	/*
	 * check that log sunit is modulo fsblksize or default it to dsunit.
	 */

	if (lsunit) {
		if ((BBTOB(lsunit) % blocksize != 0)) {
			fprintf(stderr,
	_("log stripe unit (%d) must be a multiple of the block size (%d)\n"),
			BBTOB(lsunit), blocksize);
			exit(1);
		}
		/* convert from 512 byte blocks to fs blocks */
		lsunit = DTOBT(lsunit);
	} else if (logversion == 2 && loginternal && dsunit) {
		/* lsunit and dsunit now in fs blocks */
		lsunit = dsunit;
	}

	if (logversion == 2 && (lsunit * blocksize) > 256 * 1024) {
		fprintf(stderr,
	_("log stripe unit (%d bytes) is too large (maximum is 256KiB)\n"),
			(lsunit * blocksize));
		lsunit = (32 * 1024) >> blocklog;
		fprintf(stderr, _("log stripe unit adjusted to 32KiB\n"));
	}

	protostring = setup_proto(protofile);
	bsize = 1 << (blocklog - BBSHIFT);
	mp = &mbuf;
	sbp = &mp->m_sb;
	memset(mp, 0, sizeof(xfs_mount_t));
	sbp->sb_blocklog = (__uint8_t)blocklog;
	sbp->sb_sectlog = (__uint8_t)sectorlog;
	sbp->sb_agblklog = (__uint8_t)libxfs_log2_roundup((unsigned int)agsize);
	mp->m_blkbb_log = sbp->sb_blocklog - BBSHIFT;
	mp->m_sectbb_log = sbp->sb_sectlog - BBSHIFT;

	if (loginternal) {
		/*
		 * Readjust the log size to fit within an AG if it was sized
		 * automatically.
		 */
		if (!logsize) {
			logblocks = MIN(logblocks,
					agsize - XFS_PREALLOC_BLOCKS(mp));
		}
		if (logblocks > agsize - XFS_PREALLOC_BLOCKS(mp)) {
			fprintf(stderr,
	_("internal log size %lld too large, must fit in allocation group\n"),
				(long long)logblocks);
			usage();
		}
		if (laflag) {
			if (logagno >= agcount) {
				fprintf(stderr,
		_("log ag number %d too large, must be less than %lld\n"),
					logagno, (long long)agcount);
				usage();
			}
		} else
			logagno = (xfs_agnumber_t)(agcount / 2);

		logstart = XFS_AGB_TO_FSB(mp, logagno, XFS_PREALLOC_BLOCKS(mp));
		/*
		 * Align the logstart at stripe unit boundary.
		 */
		if (lsunit) {
			logstart = fixup_internal_log_stripe(mp,
					lsflag, logstart, agsize, lsunit,
					&logblocks, blocklog, &lalign);
		} else if (dsunit) {
			logstart = fixup_internal_log_stripe(mp,
					lsflag, logstart, agsize, dsunit,
					&logblocks, blocklog, &lalign);
		}
	} else {
		logstart = 0;
		if (lsunit)
			fixup_log_stripe_unit(lsflag, lsunit,
					&logblocks, blocklog);
	}
	validate_log_size(logblocks, blocklog, min_logblocks);

	if (!qflag || Nflag) {
		printf(_(
		   "meta-data=%-22s isize=%-6d agcount=%lld, agsize=%lld blks\n"
		   "         =%-22s sectsz=%-5u attr=%u, projid32bit=%u\n"
		   "data     =%-22s bsize=%-6u blocks=%llu, imaxpct=%u\n"
		   "         =%-22s sunit=%-6u swidth=%u blks\n"
		   "naming   =version %-14u bsize=%-6u ascii-ci=%d\n"
		   "log      =%-22s bsize=%-6d blocks=%lld, version=%d\n"
		   "         =%-22s sectsz=%-5u sunit=%d blks, lazy-count=%d\n"
		   "realtime =%-22s extsz=%-6d blocks=%lld, rtextents=%lld\n"),
			dfile, isize, (long long)agcount, (long long)agsize,
			"", sectorsize, attrversion, projid32bit,
			"", blocksize, (long long)dblocks, imaxpct,
			"", dsunit, dswidth,
			dirversion, dirblocksize, nci,
			logfile, 1 << blocklog, (long long)logblocks,
			logversion, "", lsectorsize, lsunit, lazy_sb_counters,
			rtfile, rtextblocks << blocklog,
			(long long)rtblocks, (long long)rtextents);
		if (Nflag)
			exit(0);
	}

	if (label)
		strncpy(sbp->sb_fname, label, sizeof(sbp->sb_fname));
	sbp->sb_magicnum = XFS_SB_MAGIC;
	sbp->sb_blocksize = blocksize;
	sbp->sb_dblocks = dblocks;
	sbp->sb_rblocks = rtblocks;
	sbp->sb_rextents = rtextents;
	platform_uuid_generate(&uuid);
	platform_uuid_copy(&sbp->sb_uuid, &uuid);
	sbp->sb_logstart = logstart;
	sbp->sb_rootino = sbp->sb_rbmino = sbp->sb_rsumino = NULLFSINO;
	sbp->sb_rextsize = rtextblocks;
	sbp->sb_agblocks = (xfs_agblock_t)agsize;
	sbp->sb_agcount = (xfs_agnumber_t)agcount;
	sbp->sb_rbmblocks = nbmblocks;
	sbp->sb_logblocks = (xfs_extlen_t)logblocks;
	sbp->sb_sectsize = (__uint16_t)sectorsize;
	sbp->sb_inodesize = (__uint16_t)isize;
	sbp->sb_inopblock = (__uint16_t)(blocksize / isize);
	sbp->sb_sectlog = (__uint8_t)sectorlog;
	sbp->sb_inodelog = (__uint8_t)inodelog;
	sbp->sb_inopblog = (__uint8_t)(blocklog - inodelog);
	sbp->sb_rextslog =
		(__uint8_t)(rtextents ?
			libxfs_highbit32((unsigned int)rtextents) : 0);
	sbp->sb_inprogress = 1;	/* mkfs is in progress */
	sbp->sb_imax_pct = imaxpct;
	sbp->sb_icount = 0;
	sbp->sb_ifree = 0;
	sbp->sb_fdblocks = dblocks - agcount * XFS_PREALLOC_BLOCKS(mp) -
		(loginternal ? logblocks : 0);
	sbp->sb_frextents = 0;	/* will do a free later */
	sbp->sb_uquotino = sbp->sb_gquotino = 0;
	sbp->sb_qflags = 0;
	sbp->sb_unit = dsunit;
	sbp->sb_width = dswidth;
	sbp->sb_dirblklog = dirblocklog - blocklog;
	if (logversion == 2) {	/* This is stored in bytes */
		lsunit = (lsunit == 0) ? 1 : XFS_FSB_TO_B(mp, lsunit);
		sbp->sb_logsunit = lsunit;
	} else
		sbp->sb_logsunit = 0;
	if (iaflag) {
		sbp->sb_inoalignmt = XFS_INODE_BIG_CLUSTER_SIZE >> blocklog;
		iaflag = sbp->sb_inoalignmt != 0;
	} else
		sbp->sb_inoalignmt = 0;
	if (lsectorsize != BBSIZE || sectorsize != BBSIZE) {
		sbp->sb_logsectlog = (__uint8_t)lsectorlog;
		sbp->sb_logsectsize = (__uint16_t)lsectorsize;
	} else {
		sbp->sb_logsectlog = 0;
		sbp->sb_logsectsize = 0;
	}
	sbp->sb_features2 = XFS_SB_VERSION2_MKFS(lazy_sb_counters,
					attrversion == 2, projid32bit == 1, 0);
	sbp->sb_versionnum = XFS_SB_VERSION_MKFS(iaflag, dsunit != 0,
					logversion == 2, attrversion == 1,
					(sectorsize != BBSIZE ||
							lsectorsize != BBSIZE),
					nci, sbp->sb_features2 != 0);
	/*
	 * Due to a structure alignment issue, sb_features2 ended up in one
	 * of two locations, the second "incorrect" location represented by
	 * the sb_bad_features2 field. To avoid older kernels mounting
	 * filesystems they shouldn't, set both field to the same value.
	 */
	sbp->sb_bad_features2 = sbp->sb_features2;

	if (force_overwrite)
		zero_old_xfs_structures(&xi, sbp);

	/*
	 * Zero out the beginning of the device, to obliterate any old
	 * filesystem signatures out there.  This should take care of
	 * swap (somewhere around the page size), jfs (32k),
	 * ext[2,3] and reiserfs (64k) - and hopefully all else.
	 */
	buf = libxfs_getbuf(xi.ddev, 0, BTOBB(WHACK_SIZE));
	memset(XFS_BUF_PTR(buf), 0, WHACK_SIZE);
	libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);
	libxfs_purgebuf(buf);

	/* OK, now write the superblock */
	buf = libxfs_getbuf(xi.ddev, XFS_SB_DADDR, XFS_FSS_TO_BB(mp, 1));
	memset(XFS_BUF_PTR(buf), 0, sectorsize);
	libxfs_sb_to_disk((void *)XFS_BUF_PTR(buf), sbp, XFS_SB_ALL_BITS);
	libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);

	/*
	 * If the data area is a file, then grow it out to its final size
	 * so that the reads for the end of the device in the mount code
	 * will succeed.
	 */
	if (xi.disfile && ftruncate64(xi.dfd, dblocks * blocksize) < 0) {
		fprintf(stderr, _("%s: Growing the data section failed\n"),
			progname);
		exit(1);
	}

	/*
	 * Zero out the end of the device, to obliterate any
	 * old MD RAID (or other) metadata at the end of the device.
 	 * (MD sb is ~64k from the end, take out a wider swath to be sure)
	 */
	if (!xi.disfile) {
		buf = libxfs_getbuf(xi.ddev, (xi.dsize - BTOBB(WHACK_SIZE)),
				    BTOBB(WHACK_SIZE));
		memset(XFS_BUF_PTR(buf), 0, WHACK_SIZE);
		libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);
		libxfs_purgebuf(buf);
	}

	/*
	 * Zero the log if there is one.
	 */
	if (loginternal)
		xi.logdev = xi.ddev;
	if (xi.logdev)
		libxfs_log_clear(xi.logdev, XFS_FSB_TO_DADDR(mp, logstart),
			(xfs_extlen_t)XFS_FSB_TO_BB(mp, logblocks),
			&sbp->sb_uuid, logversion, lsunit, XLOG_FMT);

	mp = libxfs_mount(mp, sbp, xi.ddev, xi.logdev, xi.rtdev, 1);
	if (mp == NULL) {
		fprintf(stderr, _("%s: filesystem failed to initialize\n"),
			progname);
		exit(1);
	}

	for (agno = 0; agno < agcount; agno++) {
		/*
		 * Superblock.
		 */
		buf = libxfs_getbuf(xi.ddev,
				XFS_AG_DADDR(mp, agno, XFS_SB_DADDR),
				XFS_FSS_TO_BB(mp, 1));
		memset(XFS_BUF_PTR(buf), 0, sectorsize);
		libxfs_sb_to_disk((void *)XFS_BUF_PTR(buf), sbp, XFS_SB_ALL_BITS);
		libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);

		/*
		 * AG header block: freespace
		 */
		buf = libxfs_getbuf(mp->m_dev,
				XFS_AG_DADDR(mp, agno, XFS_AGF_DADDR(mp)),
				XFS_FSS_TO_BB(mp, 1));
		agf = XFS_BUF_TO_AGF(buf);
		memset(agf, 0, sectorsize);
		if (agno == agcount - 1)
			agsize = dblocks - (xfs_drfsbno_t)(agno * agsize);
		agf->agf_magicnum = cpu_to_be32(XFS_AGF_MAGIC);
		agf->agf_versionnum = cpu_to_be32(XFS_AGF_VERSION);
		agf->agf_seqno = cpu_to_be32(agno);
		agf->agf_length = cpu_to_be32(agsize);
		agf->agf_roots[XFS_BTNUM_BNOi] = cpu_to_be32(XFS_BNO_BLOCK(mp));
		agf->agf_roots[XFS_BTNUM_CNTi] = cpu_to_be32(XFS_CNT_BLOCK(mp));
		agf->agf_levels[XFS_BTNUM_BNOi] = cpu_to_be32(1);
		agf->agf_levels[XFS_BTNUM_CNTi] = cpu_to_be32(1);
		agf->agf_flfirst = 0;
		agf->agf_fllast = cpu_to_be32(XFS_AGFL_SIZE(mp) - 1);
		agf->agf_flcount = 0;
		nbmblocks = (xfs_extlen_t)(agsize - XFS_PREALLOC_BLOCKS(mp));
		agf->agf_freeblks = cpu_to_be32(nbmblocks);
		agf->agf_longest = cpu_to_be32(nbmblocks);
		if (loginternal && agno == logagno) {
			be32_add_cpu(&agf->agf_freeblks, -logblocks);
			agf->agf_longest = cpu_to_be32(agsize -
				XFS_FSB_TO_AGBNO(mp, logstart) - logblocks);
		}
		if (XFS_MIN_FREELIST(agf, mp) > worst_freelist)
			worst_freelist = XFS_MIN_FREELIST(agf, mp);
		libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);

		/*
		 * AG header block: inodes
		 */
		buf = libxfs_getbuf(mp->m_dev,
				XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)),
				XFS_FSS_TO_BB(mp, 1));
		agi = XFS_BUF_TO_AGI(buf);
		memset(agi, 0, sectorsize);
		agi->agi_magicnum = cpu_to_be32(XFS_AGI_MAGIC);
		agi->agi_versionnum = cpu_to_be32(XFS_AGI_VERSION);
		agi->agi_seqno = cpu_to_be32(agno);
		agi->agi_length = cpu_to_be32((xfs_agblock_t)agsize);
		agi->agi_count = 0;
		agi->agi_root = cpu_to_be32(XFS_IBT_BLOCK(mp));
		agi->agi_level = cpu_to_be32(1);
		agi->agi_freecount = 0;
		agi->agi_newino = cpu_to_be32(NULLAGINO);
		agi->agi_dirino = cpu_to_be32(NULLAGINO);
		for (c = 0; c < XFS_AGI_UNLINKED_BUCKETS; c++)
			agi->agi_unlinked[c] = cpu_to_be32(NULLAGINO);
		libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);

		/*
		 * BNO btree root block
		 */
		buf = libxfs_getbuf(mp->m_dev,
				XFS_AGB_TO_DADDR(mp, agno, XFS_BNO_BLOCK(mp)),
				bsize);
		block = XFS_BUF_TO_BLOCK(buf);
		memset(block, 0, blocksize);
		block->bb_magic = cpu_to_be32(XFS_ABTB_MAGIC);
		block->bb_level = 0;
		block->bb_numrecs = cpu_to_be16(1);
		block->bb_u.s.bb_leftsib = cpu_to_be32(NULLAGBLOCK);
		block->bb_u.s.bb_rightsib = cpu_to_be32(NULLAGBLOCK);
		arec = XFS_ALLOC_REC_ADDR(mp, block, 1);
		arec->ar_startblock = cpu_to_be32(XFS_PREALLOC_BLOCKS(mp));
		if (loginternal && agno == logagno) {
			if (lalign) {
				/*
				 * Have to insert two records
				 * Insert pad record for stripe align of log
				 */
				arec->ar_blockcount = cpu_to_be32(
					XFS_FSB_TO_AGBNO(mp, logstart) -
					be32_to_cpu(arec->ar_startblock));
				nrec = arec + 1;
				/*
				 * Insert record at start of internal log
				 */
				nrec->ar_startblock = cpu_to_be32(
					be32_to_cpu(arec->ar_startblock) +
					be32_to_cpu(arec->ar_blockcount));
				arec = nrec;
				be16_add_cpu(&block->bb_numrecs, 1);
			}
			/*
			 * Change record start to after the internal log
			 */
			be32_add_cpu(&arec->ar_startblock, logblocks);
		}
		arec->ar_blockcount = cpu_to_be32(agsize - 
					be32_to_cpu(arec->ar_startblock));
		libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);

		/*
		 * CNT btree root block
		 */
		buf = libxfs_getbuf(mp->m_dev,
				XFS_AGB_TO_DADDR(mp, agno, XFS_CNT_BLOCK(mp)),
				bsize);
		block = XFS_BUF_TO_BLOCK(buf);
		memset(block, 0, blocksize);
		block->bb_magic = cpu_to_be32(XFS_ABTC_MAGIC);
		block->bb_level = 0;
		block->bb_numrecs = cpu_to_be16(1);
		block->bb_u.s.bb_leftsib = cpu_to_be32(NULLAGBLOCK);
		block->bb_u.s.bb_rightsib = cpu_to_be32(NULLAGBLOCK);
		arec = XFS_ALLOC_REC_ADDR(mp, block, 1);
		arec->ar_startblock = cpu_to_be32(XFS_PREALLOC_BLOCKS(mp));
		if (loginternal && agno == logagno) {
			if (lalign) {
				arec->ar_blockcount = cpu_to_be32(
					XFS_FSB_TO_AGBNO(mp, logstart) -
					be32_to_cpu(arec->ar_startblock));
				nrec = arec + 1;
				nrec->ar_startblock = cpu_to_be32(
					be32_to_cpu(arec->ar_startblock) +
					be32_to_cpu(arec->ar_blockcount));
				arec = nrec;
				be16_add_cpu(&block->bb_numrecs, 1);
			}
			be32_add_cpu(&arec->ar_startblock, logblocks);
		}
		arec->ar_blockcount = cpu_to_be32(agsize - 
					be32_to_cpu(arec->ar_startblock));
		libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);

		/*
		 * INO btree root block
		 */
		buf = libxfs_getbuf(mp->m_dev,
				XFS_AGB_TO_DADDR(mp, agno, XFS_IBT_BLOCK(mp)),
				bsize);
		block = XFS_BUF_TO_BLOCK(buf);
		memset(block, 0, blocksize);
		block->bb_magic = cpu_to_be32(XFS_IBT_MAGIC);
		block->bb_level = 0;
		block->bb_numrecs = 0;
		block->bb_u.s.bb_leftsib = cpu_to_be32(NULLAGBLOCK);
		block->bb_u.s.bb_rightsib = cpu_to_be32(NULLAGBLOCK);
		libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);
	}

	/*
	 * Touch last block, make fs the right size if it's a file.
	 */
	buf = libxfs_getbuf(mp->m_dev,
		(xfs_daddr_t)XFS_FSB_TO_BB(mp, dblocks - 1LL), bsize);
	memset(XFS_BUF_PTR(buf), 0, blocksize);
	libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);

	/*
	 * Make sure we can write the last block in the realtime area.
	 */
	if (mp->m_rtdev && rtblocks > 0) {
		buf = libxfs_getbuf(mp->m_rtdev,
				XFS_FSB_TO_BB(mp, rtblocks - 1LL), bsize);
		memset(XFS_BUF_PTR(buf), 0, blocksize);
		libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);
	}

	/*
	 * BNO, CNT free block list
	 */
	for (agno = 0; agno < agcount; agno++) {
		xfs_alloc_arg_t	args;
		xfs_trans_t	*tp;

		memset(&args, 0, sizeof(args));
		args.tp = tp = libxfs_trans_alloc(mp, 0);
		args.mp = mp;
		args.agno = agno;
		args.alignment = 1;
		args.pag = xfs_perag_get(mp,agno);
		if ((c = libxfs_trans_reserve(tp, worst_freelist, 0, 0, 0, 0)))
			res_failed(c);
		libxfs_alloc_fix_freelist(&args, 0);
		xfs_perag_put(args.pag);
		libxfs_trans_commit(tp, 0);
	}

	/*
	 * Allocate the root inode and anything else in the proto file.
	 */
	mp->m_rootip = NULL;
	parse_proto(mp, &fsx, &protostring);

	/*
	 * Protect ourselves against possible stupidity
	 */
	if (XFS_INO_TO_AGNO(mp, mp->m_sb.sb_rootino) != 0) {
		fprintf(stderr,
			_("%s: root inode created in AG %u, not AG 0\n"),
			progname, XFS_INO_TO_AGNO(mp, mp->m_sb.sb_rootino));
		exit(1);
	}

	/*
	 * Write out multiple secondary superblocks with rootinode field set
	 */
	if (mp->m_sb.sb_agcount > 1) {
		/*
		 * the last superblock
		 */
		buf = libxfs_readbuf(mp->m_dev,
				XFS_AGB_TO_DADDR(mp, mp->m_sb.sb_agcount-1,
					XFS_SB_DADDR),
				XFS_FSS_TO_BB(mp, 1),
				LIBXFS_EXIT_ON_FAILURE);
		XFS_BUF_TO_SBP(buf)->sb_rootino = cpu_to_be64(
							mp->m_sb.sb_rootino);
		libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);
		/*
		 * and one in the middle for luck
		 */
		if (mp->m_sb.sb_agcount > 2) {
			buf = libxfs_readbuf(mp->m_dev,
				XFS_AGB_TO_DADDR(mp, (mp->m_sb.sb_agcount-1)/2,
					XFS_SB_DADDR),
				XFS_FSS_TO_BB(mp, 1),
				LIBXFS_EXIT_ON_FAILURE);
			XFS_BUF_TO_SBP(buf)->sb_rootino = cpu_to_be64(
							mp->m_sb.sb_rootino);
			libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);
		}
	}

	/*
	 * Dump all inodes and buffers before marking us all done.
	 * Need to drop references to inodes we still hold, first.
	 */
	libxfs_rtmount_destroy(mp);
	libxfs_icache_purge();
	libxfs_bcache_purge();

	/*
	 * Mark the filesystem ok.
	 */
	buf = libxfs_getsb(mp, LIBXFS_EXIT_ON_FAILURE);
	(XFS_BUF_TO_SBP(buf))->sb_inprogress = 0;
	libxfs_writebuf(buf, LIBXFS_EXIT_ON_FAILURE);

	libxfs_umount(mp);
	if (xi.rtdev)
		libxfs_device_close(xi.rtdev);
	if (xi.logdev && xi.logdev != xi.ddev)
		libxfs_device_close(xi.logdev);
	libxfs_device_close(xi.ddev);

	return 0;
}

static void
conflict(
	char		opt,
	char		*tab[],
	int		oldidx,
	int		newidx)
{
	fprintf(stderr, _("Cannot specify both -%c %s and -%c %s\n"),
		opt, tab[oldidx], opt, tab[newidx]);
	usage();
}


static void
illegal(
	char		*value,
	char		*opt)
{
	fprintf(stderr, _("Illegal value %s for -%s option\n"), value, opt);
	usage();
}

static int
ispow2(
	unsigned int	i)
{
	return (i & (i - 1)) == 0;
}

static void __attribute__((noreturn))
reqval(
	char		opt,
	char		*tab[],
	int		idx)
{
	fprintf(stderr, _("-%c %s option requires a value\n"), opt, tab[idx]);
	usage();
}

static void
respec(
	char		opt,
	char		*tab[],
	int		idx)
{
	fprintf(stderr, "-%c ", opt);
	if (tab)
		fprintf(stderr, "%s ", tab[idx]);
	fprintf(stderr, _("option respecified\n"));
	usage();
}

static void
unknown(
	char		opt,
	char		*s)
{
	fprintf(stderr, _("unknown option -%c %s\n"), opt, s);
	usage();
}

/*
 * isdigits -- returns 1 if string contains nothing but [0-9], 0 otherwise
 */
int
isdigits(
	char		*str)
{
	int		i;
	int		n = strlen(str);

	for (i = 0; i < n; i++) {
		if (!isdigit((int)str[i]))
			return 0;
	}
	return 1;
}

long long
cvtnum(
	unsigned int	blocksize,
	unsigned int	sectorsize,
	char		*s)
{
	long long	i;
	char		*sp;

	i = strtoll(s, &sp, 0);
	if (i == 0 && sp == s)
		return -1LL;
	if (*sp == '\0')
		return i;

	if (*sp == 'b' && sp[1] == '\0') {
		if (blocksize)
			return i * blocksize;
		fprintf(stderr, _("blocksize not available yet.\n"));
		usage();
	}
	if (*sp == 's' && sp[1] == '\0') {
		if (sectorsize)
			return i * sectorsize;
		return i * BBSIZE;
	}
	if (*sp == 'k' && sp[1] == '\0')
		return 1024LL * i;
	if (*sp == 'm' && sp[1] == '\0')
		return 1024LL * 1024LL * i;
	if (*sp == 'g' && sp[1] == '\0')
		return 1024LL * 1024LL * 1024LL * i;
	if (*sp == 't' && sp[1] == '\0')
		return 1024LL * 1024LL * 1024LL * 1024LL * i;
	if (*sp == 'p' && sp[1] == '\0')
		return 1024LL * 1024LL * 1024LL * 1024LL * 1024LL * i;
	if (*sp == 'e' && sp[1] == '\0')
		return 1024LL * 1024LL * 1024LL * 1024LL * 1024LL * 1024LL * i;
	return -1LL;
}

static void __attribute__((noreturn))
usage( void )
{
	fprintf(stderr, _("Usage: %s\n\
/* blocksize */		[-b log=n|size=num]\n\
/* data subvol */	[-d agcount=n,agsize=n,file,name=xxx,size=num,\n\
			    (sunit=value,swidth=value|su=num,sw=num),\n\
			    sectlog=n|sectsize=num\n\
/* inode size */	[-i log=n|perblock=n|size=num,maxpct=n,attr=0|1|2,\n\
			    projid32bit=0|1]\n\
/* log subvol */	[-l agnum=n,internal,size=num,logdev=xxx,version=n\n\
			    sunit=value|su=num,sectlog=n|sectsize=num,\n\
			    lazy-count=0|1]\n\
/* label */		[-L label (maximum 12 characters)]\n\
/* naming */		[-n log=n|size=num,version=2|ci]\n\
/* prototype file */	[-p fname]\n\
/* quiet */		[-q]\n\
/* realtime subvol */	[-r extsize=num,size=num,rtdev=xxx]\n\
/* sectorsize */	[-s log=n|size=num]\n\
/* version */		[-V]\n\
			devicename\n\
<devicename> is required unless -d name=xxx is given.\n\
<num> is xxx (bytes), xxxs (sectors), xxxb (fs blocks), xxxk (xxx KiB),\n\
      xxxm (xxx MiB), xxxg (xxx GiB), xxxt (xxx TiB) or xxxp (xxx PiB).\n\
<value> is xxx (512 byte blocks).\n"),
		progname);
	exit(1);
}
