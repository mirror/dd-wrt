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

#include <xfs.h>
#include <sys/stat.h>
#include "init.h"

char *progname = "libxfs";	/* default, changed by each tool */

struct cache *libxfs_icache;	/* global inode cache */
int libxfs_ihash_size;		/* #buckets in icache */

struct cache *libxfs_bcache;	/* global buffer cache */
int libxfs_bhash_size;		/* #buckets in bcache */

int	use_xfs_buf_lock;	/* global flag: use xfs_buf_t locks for MT */

static void manage_zones(int);	/* setup global zones */

/*
 * dev_map - map open devices to fd.
 */
#define MAX_DEVS 10	/* arbitary maximum */
int nextfakedev = -1;	/* device number to give to next fake device */
static struct dev_to_fd {
	dev_t	dev;
	int	fd;
} dev_map[MAX_DEVS]={{0}};

/*
 * Checks whether a given device has a mounted, writable
 * filesystem, returns 1 if it does & fatal (just warns
 * if not fatal, but allows us to proceed).
 *
 * Useful to tools which will produce uncertain results
 * if the filesystem is active - repair, check, logprint.
 */
static int
check_isactive(char *name, char *block, int fatal)
{
	struct stat64	st;

	if (stat64(block, &st) < 0)
		return 0;
	if ((st.st_mode & S_IFMT) != S_IFBLK)
		return 0;
	if (platform_check_ismounted(name, block, &st, 0) == 0)
		return 0;
	return platform_check_iswritable(name, block, &st, fatal);
}

/* libxfs_device_to_fd:
 *     lookup a device number in the device map
 *     return the associated fd
 */
int
libxfs_device_to_fd(dev_t device)
{
	int	d;

	for (d = 0; d < MAX_DEVS; d++)
		if (dev_map[d].dev == device)
			return dev_map[d].fd;

	fprintf(stderr, _("%s: %s: device %lld is not open\n"),
		progname, __FUNCTION__, (long long)device);
	exit(1);
	/* NOTREACHED */
}

/* libxfs_device_open:
 *     open a device and return its device number
 */
dev_t
libxfs_device_open(char *path, int creat, int xflags, int setblksize)
{
	dev_t		dev;
	int		fd, d, flags;
	int		readonly, dio, excl;
	struct stat64	statb;

	readonly = (xflags & LIBXFS_ISREADONLY);
	excl = (xflags & LIBXFS_EXCLUSIVELY) && !creat;
	dio = (xflags & LIBXFS_DIRECT) && !creat && platform_direct_blockdev();

retry:
	flags = (readonly ? O_RDONLY : O_RDWR) | \
		(creat ? (O_CREAT|O_TRUNC) : 0) | \
		(dio ? O_DIRECT : 0) | \
		(excl ? O_EXCL : 0);

	if ((fd = open(path, flags, 0666)) < 0) {
		if (errno == EINVAL && --dio == 0)
			goto retry;
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
		exit(1);
	}

	if (fstat64(fd, &statb) < 0) {
		fprintf(stderr, _("%s: cannot stat %s: %s\n"),
			progname, path, strerror(errno));
		exit(1);
	}

	if (!readonly && setblksize && (statb.st_mode & S_IFMT) == S_IFBLK) {
		if (setblksize == 1)
			/* use the default blocksize */
			(void)platform_set_blocksize(fd, path, statb.st_rdev, XFS_MIN_SECTORSIZE, 0);
		else {
			/* given an explicit blocksize to use */
			if (platform_set_blocksize(fd, path, statb.st_rdev, setblksize, 1))
			    exit(1);
		}
	}

	/*
	 * Get the device number from the stat buf - unless
	 * we're not opening a real device, in which case
	 * choose a new fake device number.
	 */
	dev = (statb.st_rdev) ? (statb.st_rdev) : (nextfakedev--);

	for (d = 0; d < MAX_DEVS; d++)
		if (dev_map[d].dev == dev) {
			fprintf(stderr, _("%s: device %lld is already open\n"),
			    progname, (long long)dev);
			exit(1);
		}

	for (d = 0; d < MAX_DEVS; d++)
		if (!dev_map[d].dev) {
			dev_map[d].dev = dev;
			dev_map[d].fd = fd;

			return dev;
		}

	fprintf(stderr, _("%s: %s: too many open devices\n"),
		progname, __FUNCTION__);
	exit(1);
	/* NOTREACHED */
}

void
libxfs_device_close(dev_t dev)
{
	int	d;

	for (d = 0; d < MAX_DEVS; d++)
		if (dev_map[d].dev == dev) {
			int	fd;

			fd = dev_map[d].fd;
			dev_map[d].dev = dev_map[d].fd = 0;

			fsync(fd);
			platform_flush_device(fd, dev);
			close(fd);

			return;
		}

	fprintf(stderr, _("%s: %s: device %lld is not open\n"),
			progname, __FUNCTION__, (long long)dev);
	exit(1);
}

static int
check_open(char *path, int flags, char **rawfile, char **blockfile)
{
	int readonly = (flags & LIBXFS_ISREADONLY);
	int inactive = (flags & LIBXFS_ISINACTIVE);
	int dangerously = (flags & LIBXFS_DANGEROUSLY);
	struct stat64	stbuf;

	if (stat64(path, &stbuf) < 0) {
		perror(path);
		return 0;
	}
	if (!(*rawfile = platform_findrawpath(path))) {
		fprintf(stderr, _("%s: "
				  "can't find a character device matching %s\n"),
			progname, path);
		return 0;
	}
	if (!(*blockfile = platform_findblockpath(path))) {
		fprintf(stderr, _("%s: "
				  "can't find a block device matching %s\n"),
			progname, path);
		return 0;
	}
	if (!readonly && !inactive && platform_check_ismounted(path, *blockfile, NULL, 1))
		return 0;

	if (inactive && check_isactive(path, *blockfile, ((readonly|dangerously)?1:0)))
		return 0;

	return 1;
}

/*
 * libxfs initialization.
 * Caller gets a 0 on failure (and we print a message), 1 on success.
 */
int
libxfs_init(libxfs_init_t *a)
{
	char		*blockfile;
	char		curdir[MAXPATHLEN];
	char		*dname;
	char		dpath[25];
	int		fd;
	char		*logname;
	char		logpath[25];
	int		needcd;
	char		*rawfile;
	char		*rtname;
	char		rtpath[25];
	int		rval = 0;
	int		flags;

	dpath[0] = logpath[0] = rtpath[0] = '\0';
	dname = a->dname;
	logname = a->logname;
	rtname = a->rtname;
	a->dfd = a->logfd = a->rtfd = -1;
	a->ddev = a->logdev = a->rtdev = 0;
	a->dbsize = a->lbsize = a->rtbsize = 0;
	a->dsize = a->logBBsize = a->logBBstart = a->rtsize = 0;

	(void)getcwd(curdir,MAXPATHLEN);
	needcd = 0;
	fd = -1;
	flags = (a->isreadonly | a->isdirect);

	radix_tree_init();

	if (a->volname) {
		if(!check_open(a->volname,flags,&rawfile,&blockfile))
			goto done;
		needcd = 1;
		fd = open(rawfile, O_RDONLY);
		dname = a->dname = a->volname;
		a->volname = NULL;
	}
	if (dname) {
		if (dname[0] != '/' && needcd)
			chdir(curdir);
		if (a->disfile) {
			a->ddev= libxfs_device_open(dname, a->dcreat, flags,
						    a->setblksize);
			a->dfd = libxfs_device_to_fd(a->ddev);
		} else {
			if (!check_open(dname, flags, &rawfile, &blockfile))
				goto done;
			a->ddev = libxfs_device_open(rawfile,
					a->dcreat, flags, a->setblksize);
			a->dfd = libxfs_device_to_fd(a->ddev);
			platform_findsizes(rawfile, a->dfd,
						&a->dsize, &a->dbsize);
		}
		needcd = 1;
	} else
		a->dsize = 0;
	if (logname) {
		if (logname[0] != '/' && needcd)
			chdir(curdir);
		if (a->lisfile) {
			a->logdev = libxfs_device_open(logname,
					a->lcreat, flags, a->setblksize);
			a->logfd = libxfs_device_to_fd(a->logdev);
		} else {
			if (!check_open(logname, flags, &rawfile, &blockfile))
				goto done;
			a->logdev = libxfs_device_open(rawfile,
					a->lcreat, flags, a->setblksize);
			a->logfd = libxfs_device_to_fd(a->logdev);
			platform_findsizes(rawfile, a->logfd,
						&a->logBBsize, &a->lbsize);
		}
		needcd = 1;
	} else
		a->logBBsize = 0;
	if (rtname) {
		if (rtname[0] != '/' && needcd)
			chdir(curdir);
		if (a->risfile) {
			a->rtdev = libxfs_device_open(rtname,
					a->rcreat, flags, a->setblksize);
			a->rtfd = libxfs_device_to_fd(a->rtdev);
		} else {
			if (!check_open(rtname, flags, &rawfile, &blockfile))
				goto done;
			a->rtdev = libxfs_device_open(rawfile,
					a->rcreat, flags, a->setblksize);
			a->rtfd = libxfs_device_to_fd(a->rtdev);
			platform_findsizes(rawfile, a->rtfd,
						&a->rtsize, &a->rtbsize);
		}
		needcd = 1;
	} else
		a->rtsize = 0;
	if (a->dsize < 0) {
		fprintf(stderr, _("%s: can't get size for data subvolume\n"),
			progname);
		goto done;
	}
	if (a->logBBsize < 0) {
		fprintf(stderr, _("%s: can't get size for log subvolume\n"),
			progname);
		goto done;
	}
	if (a->rtsize < 0) {
		fprintf(stderr, _("%s: can't get size for realtime subvolume\n"),
			progname);
		goto done;
	}
	if (needcd)
		chdir(curdir);
	if (!libxfs_ihash_size)
		libxfs_ihash_size = LIBXFS_IHASHSIZE(sbp);
	libxfs_icache = cache_init(libxfs_ihash_size, &libxfs_icache_operations);
	if (!libxfs_bhash_size)
		libxfs_bhash_size = LIBXFS_BHASHSIZE(sbp);
	libxfs_bcache = cache_init(libxfs_bhash_size, &libxfs_bcache_operations);
	use_xfs_buf_lock = a->usebuflock;
	manage_zones(0);
	rval = 1;
done:
	if (dpath[0])
		unlink(dpath);
	if (logpath[0])
		unlink(logpath);
	if (rtpath[0])
		unlink(rtpath);
	if (fd >= 0)
		close(fd);
	if (!rval && a->ddev)
		libxfs_device_close(a->ddev);
	if (!rval && a->logdev)
		libxfs_device_close(a->logdev);
	if (!rval && a->rtdev)
		libxfs_device_close(a->rtdev);
	return rval;
}


/*
 * Initialize/destroy all of the zone allocators we use.
 */
static void
manage_zones(int release)
{
	extern kmem_zone_t	*xfs_buf_zone;
	extern kmem_zone_t	*xfs_ili_zone;
	extern kmem_zone_t	*xfs_inode_zone;
	extern kmem_zone_t	*xfs_ifork_zone;
	extern kmem_zone_t	*xfs_dabuf_zone;
	extern kmem_zone_t	*xfs_buf_item_zone;
	extern kmem_zone_t	*xfs_da_state_zone;
	extern kmem_zone_t	*xfs_btree_cur_zone;
	extern kmem_zone_t	*xfs_bmap_free_item_zone;
	extern kmem_zone_t	*xfs_log_item_desc_zone;
	extern void		xfs_dir_startup();

	if (release) {	/* free zone allocation */
		kmem_free(xfs_buf_zone);
		kmem_free(xfs_inode_zone);
		kmem_free(xfs_ifork_zone);
		kmem_free(xfs_dabuf_zone);
		kmem_free(xfs_buf_item_zone);
		kmem_free(xfs_da_state_zone);
		kmem_free(xfs_btree_cur_zone);
		kmem_free(xfs_bmap_free_item_zone);
		kmem_free(xfs_log_item_desc_zone);
		return;
	}
	/* otherwise initialise zone allocation */
	xfs_buf_zone = kmem_zone_init(sizeof(xfs_buf_t), "xfs_buffer");
	xfs_inode_zone = kmem_zone_init(sizeof(xfs_inode_t), "xfs_inode");
	xfs_ifork_zone = kmem_zone_init(sizeof(xfs_ifork_t), "xfs_ifork");
	xfs_dabuf_zone = kmem_zone_init(sizeof(xfs_dabuf_t), "xfs_dabuf");
	xfs_ili_zone = kmem_zone_init(
			sizeof(xfs_inode_log_item_t), "xfs_inode_log_item");
	xfs_buf_item_zone = kmem_zone_init(
			sizeof(xfs_buf_log_item_t), "xfs_buf_log_item");
	xfs_da_state_zone = kmem_zone_init(
			sizeof(xfs_da_state_t), "xfs_da_state");
	xfs_btree_cur_zone = kmem_zone_init(
			sizeof(xfs_btree_cur_t), "xfs_btree_cur");
	xfs_bmap_free_item_zone = kmem_zone_init(
			sizeof(xfs_bmap_free_item_t), "xfs_bmap_free_item");
	xfs_log_item_desc_zone = kmem_zone_init(
			sizeof(struct xfs_log_item_desc), "xfs_log_item_desc");
	xfs_dir_startup();
}

/*
 * Get the bitmap and summary inodes into the mount structure
 * at mount time.
 */
static int
rtmount_inodes(xfs_mount_t *mp)
{
	int		error;
	xfs_sb_t	*sbp;

	sbp = &mp->m_sb;
	if (sbp->sb_rbmino == NULLFSINO)
		return 0;
	error = libxfs_iget(mp, NULL, sbp->sb_rbmino, 0, &mp->m_rbmip, 0);
	if (error) {
		fprintf(stderr,
			_("%s: cannot read realtime bitmap inode (%d)\n"),
			progname, error);
		return error;
	}
	ASSERT(mp->m_rbmip != NULL);
	ASSERT(sbp->sb_rsumino != NULLFSINO);
	error = libxfs_iget(mp, NULL, sbp->sb_rsumino, 0, &mp->m_rsumip, 0);
	if (error) {
		libxfs_iput(mp->m_rbmip, 0);
		fprintf(stderr,
			_("%s: cannot read realtime summary inode (%d)\n"),
			progname, error);
		return error;
	}
	ASSERT(mp->m_rsumip != NULL);
	return 0;
}

/*
 * Initialize realtime fields in the mount structure.
 */
static int
rtmount_init(
	xfs_mount_t	*mp,	/* file system mount structure */
	int		flags)
{
	xfs_buf_t	*bp;	/* buffer for last block of subvolume */
	xfs_daddr_t	d;	/* address of last block of subvolume */
	xfs_sb_t	*sbp;	/* filesystem superblock copy in mount */

	sbp = &mp->m_sb;
	if (sbp->sb_rblocks == 0)
		return 0;
	if (mp->m_rtdev == 0 && !(flags & LIBXFS_MOUNT_DEBUGGER)) {
		fprintf(stderr, _("%s: filesystem has a realtime subvolume\n"),
			progname);
		return -1;
	}
	mp->m_rsumlevels = sbp->sb_rextslog + 1;
	mp->m_rsumsize =
		(uint)sizeof(xfs_suminfo_t) * mp->m_rsumlevels *
		sbp->sb_rbmblocks;
	mp->m_rsumsize = roundup(mp->m_rsumsize, sbp->sb_blocksize);
	mp->m_rbmip = mp->m_rsumip = NULL;

	/*
	 * Allow debugger to be run without the realtime device present.
	 */
	if (flags & LIBXFS_MOUNT_DEBUGGER)
		return 0;

	/*
	 * Check that the realtime section is an ok size.
	 */
	d = (xfs_daddr_t)XFS_FSB_TO_BB(mp, mp->m_sb.sb_rblocks);
	if (XFS_BB_TO_FSB(mp, d) != mp->m_sb.sb_rblocks) {
		fprintf(stderr, _("%s: realtime init - %llu != %llu\n"),
			progname, (unsigned long long) XFS_BB_TO_FSB(mp, d),
			(unsigned long long) mp->m_sb.sb_rblocks);
		return -1;
	}
	bp = libxfs_readbuf(mp->m_rtdev,
			d - XFS_FSB_TO_BB(mp, 1), XFS_FSB_TO_BB(mp, 1), 0);
	if (bp == NULL) {
		fprintf(stderr, _("%s: realtime size check failed\n"),
			progname);
		return -1;
	}
	libxfs_putbuf(bp);
	return 0;
}


/*
 * Core dir v1 mount code for allowing reading of these dirs.
 */
static void
libxfs_dirv1_mount(
	xfs_mount_t	*mp)
{
	mp->m_dir_node_ents = mp->m_attr_node_ents =
		(XFS_LBSIZE(mp) - (uint)sizeof(xfs_da_node_hdr_t)) /
		(uint)sizeof(xfs_da_node_entry_t);
	mp->m_dir_magicpct = (XFS_LBSIZE(mp) * 37) / 100;
	mp->m_dirblksize = mp->m_sb.sb_blocksize;
	mp->m_dirblkfsbs = 1;
}

static int
libxfs_initialize_perag(
	xfs_mount_t	*mp,
	xfs_agnumber_t	agcount,
	xfs_agnumber_t	*maxagi)
{
	xfs_agnumber_t	index, max_metadata;
	xfs_agnumber_t	first_initialised = 0;
	xfs_perag_t	*pag;
	xfs_agino_t	agino;
	xfs_ino_t	ino;
	xfs_sb_t	*sbp = &mp->m_sb;
	int		error = -ENOMEM;

	/*
	 * Walk the current per-ag tree so we don't try to initialise AGs
	 * that already exist (growfs case). Allocate and insert all the
	 * AGs we don't find ready for initialisation.
	 */
	for (index = 0; index < agcount; index++) {
		pag = xfs_perag_get(mp, index);
		if (pag) {
			xfs_perag_put(pag);
			continue;
		}
		if (!first_initialised)
			first_initialised = index;

		pag = kmem_zalloc(sizeof(*pag), KM_MAYFAIL);
		if (!pag)
			goto out_unwind;
		pag->pag_agno = index;
		pag->pag_mount = mp;

		if (radix_tree_insert(&mp->m_perag_tree, index, pag)) {
			error = -EEXIST;
			goto out_unwind;
		}
	}

	/*
	 * If we mount with the inode64 option, or no inode overflows
	 * the legacy 32-bit address space clear the inode32 option.
	 */
	agino = XFS_OFFBNO_TO_AGINO(mp, sbp->sb_agblocks - 1, 0);
	ino = XFS_AGINO_TO_INO(mp, agcount - 1, agino);

	if ((mp->m_flags & XFS_MOUNT_SMALL_INUMS) && ino > XFS_MAXINUMBER_32)
		mp->m_flags |= XFS_MOUNT_32BITINODES;
	else
		mp->m_flags &= ~XFS_MOUNT_32BITINODES;

	if (mp->m_flags & XFS_MOUNT_32BITINODES) {
		/*
		 * Calculate how much should be reserved for inodes to meet
		 * the max inode percentage.
		 */
		if (mp->m_maxicount) {
			__uint64_t	icount;

			icount = sbp->sb_dblocks * sbp->sb_imax_pct;
			do_div(icount, 100);
			icount += sbp->sb_agblocks - 1;
			do_div(icount, sbp->sb_agblocks);
			max_metadata = icount;
		} else {
			max_metadata = agcount;
		}

		for (index = 0; index < agcount; index++) {
			ino = XFS_AGINO_TO_INO(mp, index, agino);
			if (ino > XFS_MAXINUMBER_32) {
				index++;
				break;
			}

			pag = xfs_perag_get(mp, index);
			pag->pagi_inodeok = 1;
			if (index < max_metadata)
				pag->pagf_metadata = 1;
			xfs_perag_put(pag);
		}
	} else {
		for (index = 0; index < agcount; index++) {
			pag = xfs_perag_get(mp, index);
			pag->pagi_inodeok = 1;
			xfs_perag_put(pag);
		}
	}

	if (maxagi)
		*maxagi = index;
	return 0;

out_unwind:
	kmem_free(pag);
	for (; index > first_initialised; index--) {
		pag = radix_tree_delete(&mp->m_perag_tree, index);
		kmem_free(pag);
	}
	return error;
}

/*
 * Mount structure initialization, provides a filled-in xfs_mount_t
 * such that the numerous XFS_* macros can be used.  If dev is zero,
 * no IO will be performed (no size checks, read root inodes).
 */
xfs_mount_t *
libxfs_mount(
	xfs_mount_t	*mp,
	xfs_sb_t	*sb,
	dev_t		dev,
	dev_t		logdev,
	dev_t		rtdev,
	int		flags)
{
	xfs_daddr_t	d;
	xfs_buf_t	*bp;
	xfs_sb_t	*sbp;
	int		error;

	mp->m_dev = dev;
	mp->m_rtdev = rtdev;
	mp->m_logdev = logdev;
	mp->m_flags = (LIBXFS_MOUNT_32BITINODES|LIBXFS_MOUNT_32BITINOOPT);
	mp->m_sb = *sb;
	INIT_RADIX_TREE(&mp->m_perag_tree, GFP_KERNEL);
	sbp = &(mp->m_sb);

	xfs_mount_common(mp, sb);

	xfs_alloc_compute_maxlevels(mp);
	xfs_bmap_compute_maxlevels(mp, XFS_DATA_FORK);
	xfs_bmap_compute_maxlevels(mp, XFS_ATTR_FORK);
	xfs_ialloc_compute_maxlevels(mp);

	if (sbp->sb_imax_pct) {
		/* Make sure the maximum inode count is a multiple of the
		 * units we allocate inodes in.
		 */
		mp->m_maxicount = (sbp->sb_dblocks * sbp->sb_imax_pct) / 100;
		mp->m_maxicount = ((mp->m_maxicount / mp->m_ialloc_blks) *
				  mp->m_ialloc_blks)  << sbp->sb_inopblog;
	} else
		mp->m_maxicount = 0;

	mp->m_inode_cluster_size = XFS_INODE_BIG_CLUSTER_SIZE;

	/*
	 * Set whether we're using stripe alignment.
	 */
	if (xfs_sb_version_hasdalign(&mp->m_sb)) {
		mp->m_dalign = sbp->sb_unit;
		mp->m_swidth = sbp->sb_width;
	}

	/*
	 * Set whether we're using inode alignment.
	 */
	if (xfs_sb_version_hasalign(&mp->m_sb) &&
	    mp->m_sb.sb_inoalignmt >=
	    XFS_B_TO_FSBT(mp, mp->m_inode_cluster_size))
		mp->m_inoalign_mask = mp->m_sb.sb_inoalignmt - 1;
	else
		mp->m_inoalign_mask = 0;
	/*
	 * If we are using stripe alignment, check whether
	 * the stripe unit is a multiple of the inode alignment
	 */
	if (mp->m_dalign && mp->m_inoalign_mask &&
					!(mp->m_dalign & mp->m_inoalign_mask))
		mp->m_sinoalign = mp->m_dalign;
	else
		mp->m_sinoalign = 0;

	/*
	 * Check that the data (and log if separate) are an ok size.
	 */
	d = (xfs_daddr_t) XFS_FSB_TO_BB(mp, mp->m_sb.sb_dblocks);
	if (XFS_BB_TO_FSB(mp, d) != mp->m_sb.sb_dblocks) {
		fprintf(stderr, _("%s: size check failed\n"), progname);
		if (!(flags & LIBXFS_MOUNT_DEBUGGER))
			return NULL;
	}

	/* Initialize the appropriate directory manager */
	if (xfs_sb_version_hasdirv2(sbp))
		xfs_dir_mount(mp);
	else {
		fprintf(stderr, _("%s: WARNING - filesystem uses v1 dirs,"
				"limited functionality provided.\n"), progname);
		libxfs_dirv1_mount(mp);
	}

	/* Initialize cached values for the attribute manager */
	mp->m_attr_magicpct = (mp->m_sb.sb_blocksize * 37) / 100;
	if (xfs_sb_version_hasattr2(&mp->m_sb))
		mp->m_flags |= LIBXFS_MOUNT_ATTR2;

	/* Initialize the precomputed transaction reservations values */
	xfs_trans_init(mp);

	if (dev == 0)	/* maxtrres, we have no device so leave now */
		return mp;

	bp = libxfs_readbuf(mp->m_dev,
			d - XFS_FSS_TO_BB(mp, 1), XFS_FSS_TO_BB(mp, 1),
			!(flags & LIBXFS_MOUNT_DEBUGGER));
	if (!bp) {
		fprintf(stderr, _("%s: data size check failed\n"), progname);
		if (!(flags & LIBXFS_MOUNT_DEBUGGER))
			return NULL;
	} else
		libxfs_putbuf(bp);

	if (mp->m_logdev && mp->m_logdev != mp->m_dev) {
		d = (xfs_daddr_t) XFS_FSB_TO_BB(mp, mp->m_sb.sb_logblocks);
		if ( (XFS_BB_TO_FSB(mp, d) != mp->m_sb.sb_logblocks) ||
		     (!(bp = libxfs_readbuf(mp->m_logdev,
					d - XFS_FSB_TO_BB(mp, 1),
					XFS_FSB_TO_BB(mp, 1),
					!(flags & LIBXFS_MOUNT_DEBUGGER)))) ) {
			fprintf(stderr, _("%s: log size checks failed\n"),
					progname);
			if (!(flags & LIBXFS_MOUNT_DEBUGGER))
				return NULL;
		}
		if (bp)
			libxfs_putbuf(bp);
	}

	/* Initialize realtime fields in the mount structure */
	if (rtmount_init(mp, flags)) {
		fprintf(stderr, _("%s: realtime device init failed\n"),
			progname);
			return NULL;
	}

	error = libxfs_initialize_perag(mp, sbp->sb_agcount, &mp->m_maxagi);
	if (error) {
		fprintf(stderr, _("%s: perag init failed\n"),
			progname);
		exit(1);
	}

	/*
	 * mkfs calls mount before the root inode is allocated.
	 */
	if ((flags & LIBXFS_MOUNT_ROOTINOS) && sbp->sb_rootino != NULLFSINO) {
		error = libxfs_iget(mp, NULL, sbp->sb_rootino, 0,
				&mp->m_rootip, 0);
		if (error) {
			fprintf(stderr, _("%s: cannot read root inode (%d)\n"),
				progname, error);
			if (!(flags & LIBXFS_MOUNT_DEBUGGER))
				return NULL;
		}
		ASSERT(mp->m_rootip != NULL);
	}
	if ((flags & LIBXFS_MOUNT_ROOTINOS) && rtmount_inodes(mp)) {
		if (mp->m_rootip)
			libxfs_iput(mp->m_rootip, 0);
		return NULL;
	}

	/*
	 * mkfs calls mount before the AGF/AGI structures are written.
	 */
	if ((flags & LIBXFS_MOUNT_ROOTINOS) && sbp->sb_rootino != NULLFSINO &&
	    xfs_sb_version_haslazysbcount(&mp->m_sb)) {
		error = xfs_initialize_perag_data(mp, sbp->sb_agcount);
		if (error) {
			fprintf(stderr, _("%s: cannot init perag data (%d)\n"),
				progname, error);
			return NULL;
		}
	}

	return mp;
}

void
libxfs_rtmount_destroy(xfs_mount_t *mp)
{
	if (mp->m_rsumip)
		libxfs_iput(mp->m_rsumip, 0);
	if (mp->m_rbmip)
		libxfs_iput(mp->m_rbmip, 0);
	mp->m_rsumip = mp->m_rbmip = NULL;
}

/*
 * Release any resource obtained during a mount.
 */
void
libxfs_umount(xfs_mount_t *mp)
{
	struct xfs_perag	*pag;
	int			agno;

	libxfs_rtmount_destroy(mp);
	libxfs_icache_purge();
	libxfs_bcache_purge();

	for (agno = 0; agno < mp->m_maxagi; agno++) {
		pag = radix_tree_delete(&mp->m_perag_tree, agno);
		kmem_free(pag);
	}
}

/*
 * Release any global resources used by libxfs.
 */
void
libxfs_destroy(void)
{
	manage_zones(1);
	cache_destroy(libxfs_icache);
	cache_destroy(libxfs_bcache);
}

int
libxfs_device_alignment(void)
{
	return platform_align_blockdev();
}

void
libxfs_report(FILE *fp)
{
	time_t t;
	char *c;

	cache_report(fp, "libxfs_icache", libxfs_icache);
	cache_report(fp, "libxfs_bcache", libxfs_bcache);

	t = time(NULL);
	c = asctime(localtime(&t));
	fprintf(fp, "%s", c);
}

int
libxfs_nproc(void)
{
	return platform_nproc();
}

unsigned long
libxfs_physmem(void)
{
	return platform_physmem();
}
