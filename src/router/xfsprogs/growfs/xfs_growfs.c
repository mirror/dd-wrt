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
#include <xfs/path.h>

/*
 * When growing a filesystem, this is the most significant
 * bits we'll accept in the resulting inode numbers
 * without warning the user.
 */

#define XFS_MAX_INODE_SIG_BITS 32

static void
usage(void)
{
	fprintf(stderr, _(
"Usage: %s [options] mountpoint\n\n\
Options:\n\
	-d          grow data/metadata section\n\
	-l          grow log section\n\
	-r          grow realtime section\n\
	-n          don't change anything, just show geometry\n\
	-I          allow inode numbers to exceed %d significant bits\n\
	-i          convert log from external to internal format\n\
	-t          alternate location for mount table (/etc/mtab)\n\
	-x          convert log from internal to external format\n\
	-D size     grow data/metadata section to size blks\n\
	-L size     grow/shrink log section to size blks\n\
	-R size     grow realtime section to size blks\n\
	-e size     set realtime extent size to size blks\n\
	-m imaxpct  set inode max percent to imaxpct\n\
	-V          print version information\n"),
		progname, XFS_MAX_INODE_SIG_BITS);
	exit(2);
}

void
report_info(
	xfs_fsop_geom_t	geo,
	char		*mntpoint,
	int		isint,
	char		*logname,
	char		*rtname,
	int		lazycount,
	int		dirversion,
	int		logversion,
	int		attrversion,
	int		cimode)
{
	printf(_(
	    "meta-data=%-22s isize=%-6u agcount=%u, agsize=%u blks\n"
	    "         =%-22s sectsz=%-5u attr=%u\n"
	    "data     =%-22s bsize=%-6u blocks=%llu, imaxpct=%u\n"
	    "         =%-22s sunit=%-6u swidth=%u blks\n"
	    "naming   =version %-14u bsize=%-6u ascii-ci=%d\n"
	    "log      =%-22s bsize=%-6u blocks=%u, version=%u\n"
	    "         =%-22s sectsz=%-5u sunit=%u blks, lazy-count=%u\n"
	    "realtime =%-22s extsz=%-6u blocks=%llu, rtextents=%llu\n"),

		mntpoint, geo.inodesize, geo.agcount, geo.agblocks,
		"", geo.sectsize, attrversion,
		"", geo.blocksize, (unsigned long long)geo.datablocks,
			geo.imaxpct,
		"", geo.sunit, geo.swidth,
  		dirversion, geo.dirblocksize, cimode,
		isint ? _("internal") : logname ? logname : _("external"),
			geo.blocksize, geo.logblocks, logversion,
		"", geo.logsectsize, geo.logsunit / geo.blocksize, lazycount,
		!geo.rtblocks ? _("none") : rtname ? rtname : _("external"),
		geo.rtextsize * geo.blocksize, (unsigned long long)geo.rtblocks,
			(unsigned long long)geo.rtextents);
}

int
main(int argc, char **argv)
{
	int			aflag;	/* fake flag, do all pieces */
	int			c;	/* current option character */
	long long		ddsize;	/* device size in 512-byte blocks */
	int			dflag;	/* -d flag */
	int			attrversion;/* attribute version number */
	int			dirversion; /* directory version number */
	int			logversion; /* log version number */
	long long		dlsize;	/* device size in 512-byte blocks */
	long long		drsize;	/* device size in 512-byte blocks */
	long long		dsize;	/* new data size in fs blocks */
	int			error;	/* we have hit an error */
	long			esize;	/* new rt extent size */
	int			ffd;	/* mount point file descriptor */
	xfs_fsop_geom_t		geo;	/* current fs geometry */
	int			iflag;	/* -i flag */
	int			isint;	/* log is currently internal */
	int			lflag;	/* -l flag */
	long long		lsize;	/* new log size in fs blocks */
	int			maxpct;	/* -m flag value */
	int			mflag;	/* -m flag */
	int			nflag;	/* -n flag */
	xfs_fsop_geom_t		ngeo;	/* new fs geometry */
	int			rflag;	/* -r flag */
	long long		rsize;	/* new rt size in fs blocks */
	int			ci;	/* ASCII case-insensitive fs */
	int			lazycount; /* lazy superblock counters */
	int			xflag;	/* -x flag */
	char			*fname;	/* mount point name */
	char			*datadev; /* data device name */
	char			*logdev;  /*  log device name */
	char			*rtdev;	/*   RT device name */
	fs_path_t		*fs;	/* mount point information */
	libxfs_init_t		xi;	/* libxfs structure */

	progname = basename(argv[0]);
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	maxpct = esize = 0;
	dsize = lsize = rsize = 0LL;
	aflag = dflag = iflag = lflag = mflag = nflag = rflag = xflag = 0;
	ci = 0;

	while ((c = getopt(argc, argv, "dD:e:ilL:m:np:rR:t:xV")) != EOF) {
		switch (c) {
		case 'D':
			dsize = strtoll(optarg, NULL, 10);
			/* fall through */
		case 'd':
			dflag = 1;
			break;
		case 'e':
			esize = atol(optarg);
			rflag = 1;
			break;
		case 'i':
			lflag = iflag = 1;
			break;
		case 'L':
			lsize = strtoll(optarg, NULL, 10);
			/* fall through */
		case 'l':
			lflag = 1;
			break;
		case 'm':
			mflag = 1;
			maxpct = atoi(optarg);
			break;
		case 'n':
			nflag = 1;
			break;
		case 'p':
			progname = optarg;
			break;
		case 'R':
			rsize = strtoll(optarg, NULL, 10);
			/* fall through */
		case 'r':
			rflag = 1;
			break;
		case 't':
			mtab_file = optarg;
			break;
		case 'x':
			lflag = xflag = 1;
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		case '?':
		default:
			usage();
		}
	}
	if (argc - optind != 1)
		usage();
	if (iflag && xflag)
		usage();
	if (dflag + lflag + rflag == 0)
		aflag = 1;

	fs_table_initialise(0, NULL, 0, NULL);
	fs = fs_table_lookup(argv[optind], FS_MOUNT_POINT);
	if (!fs) {
		fprintf(stderr, _("%s: %s is not a mounted XFS filesystem\n"),
			progname, argv[optind]);
		return 1;
	}

	fname = fs->fs_dir;
	datadev = fs->fs_name;
	logdev = fs->fs_log;
	rtdev = fs->fs_rt;

	ffd = open(fname, O_RDONLY);
	if (ffd < 0) {
		perror(fname);
		return 1;
	}

	if (!platform_test_xfs_fd(ffd)) {
		fprintf(stderr, _("%s: specified file "
			"[\"%s\"] is not on an XFS filesystem\n"),
			progname, fname);
		exit(1);
	}

	/* get the current filesystem size & geometry */
	if (xfsctl(fname, ffd, XFS_IOC_FSGEOMETRY, &geo) < 0) {
		/*
		 * OK, new xfsctl barfed - back off and try earlier version
		 * as we're probably running an older kernel version.
		 * Only field added in the v2 geometry xfsctl is "logsunit"
		 * so we'll zero that out for later display (as zero).
		 */
		geo.logsunit = 0;
		if (xfsctl(fname, ffd, XFS_IOC_FSGEOMETRY_V1, &geo) < 0) {
			fprintf(stderr, _(
				"%s: cannot determine geometry of filesystem"
				" mounted at %s: %s\n"),
				progname, fname, strerror(errno));
			exit(1);
		}
	}
	isint = geo.logstart > 0;
	lazycount = geo.flags & XFS_FSOP_GEOM_FLAGS_LAZYSB ? 1 : 0;
	dirversion = geo.flags & XFS_FSOP_GEOM_FLAGS_DIRV2 ? 2 : 1;
	logversion = geo.flags & XFS_FSOP_GEOM_FLAGS_LOGV2 ? 2 : 1;
	attrversion = geo.flags & XFS_FSOP_GEOM_FLAGS_ATTR2 ? 2 : \
			(geo.flags & XFS_FSOP_GEOM_FLAGS_ATTR ? 1 : 0);
	ci = geo.flags & XFS_FSOP_GEOM_FLAGS_DIRV2CI ? 1 : 0;
	if (nflag) {
		report_info(geo, datadev, isint, logdev, rtdev,
				lazycount, dirversion, logversion,
				attrversion, ci);
		exit(0);
	}

	/*
	 * Need root access from here on (using raw devices)...
	 */

	memset(&xi, 0, sizeof(xi));
	xi.dname = datadev;
	xi.logname = logdev;
	xi.rtname = rtdev;
	xi.isreadonly = LIBXFS_ISREADONLY;

	if (!libxfs_init(&xi))
		usage();

	/* check we got the info for all the sections we are trying to modify */
	if (!xi.ddev) {
		fprintf(stderr, _("%s: failed to access data device for %s\n"),
			progname, fname);
		exit(1);
	}
	if (lflag && !isint && !xi.logdev) {
		fprintf(stderr, _("%s: failed to access external log for %s\n"),
			progname, fname);
		exit(1);
	}
	if (rflag && !xi.rtdev) {
		fprintf(stderr,
			_("%s: failed to access realtime device for %s\n"),
			progname, fname);
		exit(1);
	}

	report_info(geo, datadev, isint, logdev, rtdev,
			lazycount, dirversion, logversion,
			attrversion, ci);

	ddsize = xi.dsize;
	dlsize = ( xi.logBBsize? xi.logBBsize :
			geo.logblocks * (geo.blocksize / BBSIZE) );
	drsize = xi.rtsize;

	/*
	 * Ok, Linux only has a 1024-byte resolution on device _size_,
	 * and the sizes below are in basic 512-byte blocks,
	 * so if we have (size % 2), on any partition, we can't get
	 * to the last 512 bytes.  Just chop it down by a block.
	 */

	ddsize -= (ddsize % 2);
	dlsize -= (dlsize % 2);
	drsize -= (drsize % 2);

	error = 0;
	if (dflag | aflag) {
		xfs_growfs_data_t	in;

		if (!mflag)
			maxpct = geo.imaxpct;
		if (!dsize)
			dsize = ddsize / (geo.blocksize / BBSIZE);
		else if (dsize > ddsize / (geo.blocksize / BBSIZE)) {
			fprintf(stderr, _(
				"data size %lld too large, maximum is %lld\n"),
				(long long)dsize,
				(long long)(ddsize/(geo.blocksize/BBSIZE)));
			error = 1;
		}

		if (!error && dsize < geo.datablocks) {
			fprintf(stderr, _("data size %lld too small,"
				" old size is %lld\n"),
				(long long)dsize, (long long)geo.datablocks);
			error = 1;
		} else if (!error &&
			   dsize == geo.datablocks && maxpct == geo.imaxpct) {
			if (dflag)
				fprintf(stderr, _(
					"data size unchanged, skipping\n"));
			if (mflag)
				fprintf(stderr, _(
					"inode max pct unchanged, skipping\n"));
		} else if (!error && !nflag) {
			in.newblocks = (__u64)dsize;
			in.imaxpct = (__u32)maxpct;
			if (xfsctl(fname, ffd, XFS_IOC_FSGROWFSDATA, &in) < 0) {
				if (errno == EWOULDBLOCK)
					fprintf(stderr, _(
				 "%s: growfs operation in progress already\n"),
						progname);
				else
					fprintf(stderr, _(
				"%s: XFS_IOC_FSGROWFSDATA xfsctl failed: %s\n"),
						progname, strerror(errno));
				error = 1;
			}
		}
	}

	if (!error && (rflag | aflag)) {
		xfs_growfs_rt_t	in;

		if (!esize)
			esize = (__u32)geo.rtextsize;
		if (!rsize)
			rsize = drsize / (geo.blocksize / BBSIZE);
		else if (rsize > drsize / (geo.blocksize / BBSIZE)) {
			fprintf(stderr, _(
			"realtime size %lld too large, maximum is %lld\n"),
				rsize, drsize / (geo.blocksize / BBSIZE));
			error = 1;
		}
		if (!error && rsize < geo.rtblocks) {
			fprintf(stderr, _(
			"realtime size %lld too small, old size is %lld\n"),
				(long long)rsize, (long long)geo.rtblocks);
			error = 1;
		} else if (!error && rsize == geo.rtblocks) {
			if (rflag)
				fprintf(stderr, _(
					"realtime size unchanged, skipping\n"));
		} else if (!error && !nflag) {
			in.newblocks = (__u64)rsize;
			in.extsize = (__u32)esize;
			if (xfsctl(fname, ffd, XFS_IOC_FSGROWFSRT, &in) < 0) {
				if (errno == EWOULDBLOCK)
					fprintf(stderr, _(
				"%s: growfs operation in progress already\n"),
						progname);
				else if (errno == ENOSYS)
					fprintf(stderr, _(
				"%s: realtime growth not implemented\n"),
						progname);
				else
					fprintf(stderr, _(
				"%s: XFS_IOC_FSGROWFSRT xfsctl failed: %s\n"),
						progname, strerror(errno));
				error = 1;
			}
		}
	}

	if (!error && (lflag | aflag)) {
		xfs_growfs_log_t	in;

		if (!lsize)
			lsize = dlsize / (geo.blocksize / BBSIZE);
		if (iflag)
			in.isint = 1;
		else if (xflag)
			in.isint = 0;
		else
			in.isint = xi.logBBsize == 0;
		if (lsize == geo.logblocks && (in.isint == isint)) {
			if (lflag)
				fprintf(stderr,
					_("log size unchanged, skipping\n"));
		} else if (!nflag) {
			in.newblocks = (__u32)lsize;
			if (xfsctl(fname, ffd, XFS_IOC_FSGROWFSLOG, &in) < 0) {
				if (errno == EWOULDBLOCK)
					fprintf(stderr,
				_("%s: growfs operation in progress already\n"),
						progname);
				else if (errno == ENOSYS)
					fprintf(stderr,
				_("%s: log growth not supported yet\n"),
						progname);
				else
					fprintf(stderr,
				_("%s: XFS_IOC_FSGROWFSLOG xfsctl failed: %s\n"),
						progname, strerror(errno));
				error = 1;
			}
		}
	}

	if (xfsctl(fname, ffd, XFS_IOC_FSGEOMETRY_V1, &ngeo) < 0) {
		fprintf(stderr, _("%s: XFS_IOC_FSGEOMETRY xfsctl failed: %s\n"),
			progname, strerror(errno));
		exit(1);
	}
	if (geo.datablocks != ngeo.datablocks)
		printf(_("data blocks changed from %lld to %lld\n"),
			(long long)geo.datablocks, (long long)ngeo.datablocks);
	if (geo.imaxpct != ngeo.imaxpct)
		printf(_("inode max percent changed from %d to %d\n"),
			geo.imaxpct, ngeo.imaxpct);
	if (geo.logblocks != ngeo.logblocks)
		printf(_("log blocks changed from %d to %d\n"),
			geo.logblocks, ngeo.logblocks);
	if ((geo.logstart == 0) != (ngeo.logstart == 0))
		printf(_("log changed from %s to %s\n"),
			geo.logstart ? _("internal") : _("external"),
			ngeo.logstart ? _("internal") : _("external"));
	if (geo.rtblocks != ngeo.rtblocks)
		printf(_("realtime blocks changed from %lld to %lld\n"),
			(long long)geo.rtblocks, (long long)ngeo.rtblocks);
	if (geo.rtextsize != ngeo.rtextsize)
		printf(_("realtime extent size changed from %d to %d\n"),
			geo.rtextsize, ngeo.rtextsize);
	exit(error);
}
