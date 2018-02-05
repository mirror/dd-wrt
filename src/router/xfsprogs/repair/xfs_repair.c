/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
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
#include "libxlog.h"
#include <sys/resource.h>
#include "xfs_multidisk.h"
#include "avl.h"
#include "avl64.h"
#include "globals.h"
#include "versions.h"
#include "agheader.h"
#include "protos.h"
#include "incore.h"
#include "err_protos.h"
#include "prefetch.h"
#include "threads.h"
#include "progress.h"
#include "dinode.h"
#include "slab.h"
#include "rmap.h"

#define	rounddown(x, y)	(((x)/(y))*(y))

#define		XR_MAX_SECT_SIZE	(64 * 1024)

/*
 * option tables for getsubopt calls
 */

/*
 * -o: user-supplied override options
 */
static char *o_opts[] = {
#define ASSUME_XFS	0
	"assume_xfs",
#define PRE_65_BETA	1
	"fs_is_pre_65_beta",
#define	IHASH_SIZE	2
	"ihash",
#define	BHASH_SIZE	3
	"bhash",
#define	AG_STRIDE	4
	"ag_stride",
#define FORCE_GEO	5
	"force_geometry",
#define PHASE2_THREADS	6
	"phase2_threads",
	NULL
};

/*
 * -c: conversion options
 */
static char *c_opts[] = {
#define CONVERT_LAZY_COUNT	0
	"lazycount",
	NULL
};


static int	bhash_option_used;
static long	max_mem_specified;	/* in megabytes */
static int	phase2_threads = 32;

static void
usage(void)
{
	do_warn(_(
"Usage: %s [options] device\n"
"\n"
"Options:\n"
"  -f           The device is a file\n"
"  -L           Force log zeroing. Do this as a last resort.\n"
"  -l logdev    Specifies the device where the external log resides.\n"
"  -m maxmem    Maximum amount of memory to be used in megabytes.\n"
"  -n           No modify mode, just checks the filesystem for damage.\n"
"  -P           Disables prefetching.\n"
"  -r rtdev     Specifies the device where the realtime section resides.\n"
"  -v           Verbose output.\n"
"  -c subopts   Change filesystem parameters - use xfs_admin.\n"
"  -o subopts   Override default behaviour, refer to man page.\n"
"  -t interval  Reporting interval in seconds.\n"
"  -d           Repair dangerously.\n"
"  -V           Reports version and exits.\n"), progname);
	exit(1);
}

char *
err_string(int err_code)
{
	static char *err_message[XR_BAD_ERR_CODE];
	static int done;

	if (!done) {
		err_message[XR_OK] = _("no error");
		err_message[XR_BAD_MAGIC] = _("bad magic number");
		err_message[XR_BAD_BLOCKSIZE] = _("bad blocksize field");
		err_message[XR_BAD_BLOCKLOG] = _("bad blocksize log field");
		err_message[XR_BAD_VERSION] = _("bad or unsupported version");
		err_message[XR_BAD_INPROGRESS] =
			_("filesystem mkfs-in-progress bit set");
		err_message[XR_BAD_FS_SIZE_DATA] =
			_("inconsistent filesystem geometry information");
		err_message[XR_BAD_INO_SIZE_DATA] =
	_("bad inode size or inconsistent with number of inodes/block"),
		err_message[XR_BAD_SECT_SIZE_DATA] = _("bad sector size");
		err_message[XR_AGF_GEO_MISMATCH] =
	_("AGF geometry info conflicts with filesystem geometry");
		err_message[XR_AGI_GEO_MISMATCH] =
	_("AGI geometry info conflicts with filesystem geometry");
		err_message[XR_SB_GEO_MISMATCH] =
	_("AG superblock geometry info conflicts with filesystem geometry");
		err_message[XR_EOF] = _("attempted to perform I/O beyond EOF");
		err_message[XR_BAD_RT_GEO_DATA] =
	_("inconsistent filesystem geometry in realtime filesystem component");
		err_message[XR_BAD_INO_MAX_PCT] =
			_("maximum indicated percentage of inodes > 100%");
		err_message[XR_BAD_INO_ALIGN] =
			_("inconsistent inode alignment value");
		err_message[XR_INSUFF_SEC_SB] =
	_("not enough secondary superblocks with matching geometry");
		err_message[XR_BAD_SB_UNIT] =
			_("bad stripe unit in superblock");
		err_message[XR_BAD_SB_WIDTH] =
			_("bad stripe width in superblock");
		err_message[XR_BAD_SVN] =
			_("bad shared version number in superblock");
		err_message[XR_BAD_CRC] =
			_("bad CRC in superblock");
		err_message[XR_BAD_DIR_SIZE_DATA] =
			_("inconsistent directory geometry information");
		done = 1;
	}

	if (err_code < XR_OK || err_code >= XR_BAD_ERR_CODE)
		do_abort(_("bad error code - %d\n"), err_code);

	return(err_message[err_code]);
}

static void
noval(char opt, char *tbl[], int idx)
{
	do_warn(_("-%c %s option cannot have a value\n"), opt, tbl[idx]);
	usage();
}

static void
respec(char opt, char *tbl[], int idx)
{
	do_warn("-%c ", opt);
	if (tbl)
		do_warn("%s ", tbl[idx]);
	do_warn(_("option respecified\n"));
	usage();
}

static void
unknown(char opt, char *s)
{
	do_warn(_("unknown option -%c %s\n"), opt, s);
	usage();
}

/*
 * sets only the global argument flags and variables
 */
static void
process_args(int argc, char **argv)
{
	char *p;
	int c;

	log_spec = 0;
	fs_is_dirty = 0;
	verbose = 0;
	no_modify = 0;
	dangerously = 0;
	isa_file = 0;
	zap_log = 0;
	dumpcore = 0;
	full_ino_ex_data = 0;
	delete_attr_ok = 1;
	force_geo = 0;
	assume_xfs = 0;
	copied_sunit = 0;
	sb_inoalignmt = 0;
	sb_unit = 0;
	sb_width = 0;
	fs_attributes_allowed = 1;
	fs_attributes2_allowed = 1;
	fs_quotas_allowed = 1;
	fs_aligned_inodes_allowed = 1;
	fs_sb_feature_bits_allowed = 1;
	fs_has_extflgbit_allowed = 1;
	pre_65_beta = 0;
	fs_shared_allowed = 1;
	ag_stride = 0;
	thread_count = 1;
	report_interval = PROG_RPT_DEFAULT;

	/*
	 * XXX have to add suboption processing here
	 * attributes, quotas, nlinks, aligned_inos, sb_fbits
	 */
	while ((c = getopt(argc, argv, "c:o:fl:m:r:LnDvVdPt:")) != EOF)  {
		switch (c) {
		case 'D':
			dumpcore = 1;
			break;
		case 'o':
			p = optarg;
			while (*p != '\0')  {
				char *val;

				switch (getsubopt(&p, o_opts, &val))  {
				case ASSUME_XFS:
					if (val)
						noval('o', o_opts, ASSUME_XFS);
					if (assume_xfs)
						respec('o', o_opts, ASSUME_XFS);
					assume_xfs = 1;
					break;
				case PRE_65_BETA:
					if (val)
						noval('o', o_opts, PRE_65_BETA);
					if (pre_65_beta)
						respec('o', o_opts,
							PRE_65_BETA);
					pre_65_beta = 1;
					break;
				case IHASH_SIZE:
					do_warn(
		_("-o ihash option has been removed and will be ignored\n"));
					break;
				case BHASH_SIZE:
					if (max_mem_specified)
						do_abort(
		_("-o bhash option cannot be used with -m option\n"));
					libxfs_bhash_size = (int)strtol(val, NULL, 0);
					bhash_option_used = 1;
					break;
				case AG_STRIDE:
					ag_stride = (int)strtol(val, NULL, 0);
					break;
				case FORCE_GEO:
					if (val)
						noval('o', o_opts, FORCE_GEO);
					if (force_geo)
						respec('o', o_opts, FORCE_GEO);
					force_geo = 1;
					break;
				case PHASE2_THREADS:
					phase2_threads = (int)strtol(val, NULL, 0);
					break;
				default:
					unknown('o', val);
					break;
				}
			}
			break;
		case 'c':
			p = optarg;
			while (*p) {
				char *val;

				switch (getsubopt(&p, c_opts, &val)) {
				case CONVERT_LAZY_COUNT:
					lazy_count = (int)strtol(val, NULL, 0);
					convert_lazy_count = 1;
					break;
				default:
					unknown('c', val);
					break;
				}
			}
			break;
		case 'l':
			log_name = optarg;
			log_spec = 1;
			break;
		case 'r':
			rt_name = optarg;
			rt_spec = 1;
			break;
		case 'f':
			isa_file = 1;
			break;
		case 'm':
			if (bhash_option_used)
				do_abort(_("-m option cannot be used with "
						"-o bhash option\n"));
			max_mem_specified = strtol(optarg, NULL, 0);
			break;
		case 'L':
			zap_log = 1;
			break;
		case 'n':
			no_modify = 1;
			break;
		case 'd':
			dangerously = 1;
			break;
		case 'v':
			verbose++;
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		case 'P':
			do_prefetch = 0;
			break;
		case 't':
			report_interval = (int)strtol(optarg, NULL, 0);
			break;
		case '?':
			usage();
		}
	}

	if (argc - optind != 1)
		usage();

	if ((fs_name = argv[optind]) == NULL)
		usage();
}

void __attribute__((noreturn))
do_error(char const *msg, ...)
{
	va_list args;

	fprintf(stderr, _("\nfatal error -- "));

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	if (dumpcore)
		abort();
	exit(1);
}

/*
 * like do_error, only the error is internal, no system
 * error so no oserror processing
 */
void __attribute__((noreturn))
do_abort(char const *msg, ...)
{
	va_list args;

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	if (dumpcore)
		abort();
	exit(1);
}

void
do_warn(char const *msg, ...)
{
	va_list args;

	fs_is_dirty = 1;

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
}

/* no formatting */

void
do_log(char const *msg, ...)
{
	va_list args;

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
}

static void
calc_mkfs(xfs_mount_t *mp)
{
	xfs_agblock_t	fino_bno;
	int		do_inoalign;

	do_inoalign = mp->m_sinoalign;

	/*
	 * Pre-calculate the geometry of ag 0. We know what it looks like
	 * because we know what mkfs does: 2 allocation btree roots (by block
	 * and by size), the inode allocation btree root, the free inode
	 * allocation btree root (if enabled) and some number of blocks to
	 * prefill the agfl.
	 *
	 * Because the current shape of the btrees may differ from the current
	 * shape, we open code the mkfs freelist block count here. mkfs creates
	 * single level trees, so the calculation is pertty straight forward for
	 * the trees that use the AGFL.
	 */
	bnobt_root = howmany(4 * mp->m_sb.sb_sectsize, mp->m_sb.sb_blocksize);
	bcntbt_root = bnobt_root + 1;
	inobt_root = bnobt_root + 2;
	fino_bno = inobt_root + (2 * min(2, mp->m_ag_maxlevels)) + 1;
	if (xfs_sb_version_hasfinobt(&mp->m_sb))
		fino_bno++;
	if (xfs_sb_version_hasrmapbt(&mp->m_sb)) {
		fino_bno += min(2, mp->m_rmap_maxlevels); /* agfl blocks */
		fino_bno++;
	}
	if (xfs_sb_version_hasreflink(&mp->m_sb))
		fino_bno++;

	/*
	 * If the log is allocated in the first allocation group we need to
	 * add the number of blocks used by the log to the above calculation.
	 *
	 * This can happens with filesystems that only have a single
	 * allocation group, or very odd geometries created by old mkfs
	 * versions on very small filesystems.
	 */
	if (mp->m_sb.sb_logstart &&
	    XFS_FSB_TO_AGNO(mp, mp->m_sb.sb_logstart) == 0) {

		/*
		 * XXX(hch): verify that sb_logstart makes sense?
		 */
		 fino_bno += mp->m_sb.sb_logblocks;
	}

	/*
	 * ditto the location of the first inode chunks in the fs ('/')
	 */
	if (xfs_sb_version_hasdalign(&mp->m_sb) && do_inoalign)  {
		first_prealloc_ino = XFS_OFFBNO_TO_AGINO(mp, roundup(fino_bno,
					mp->m_sb.sb_unit), 0);
	} else if (xfs_sb_version_hasalign(&mp->m_sb) &&
					mp->m_sb.sb_inoalignmt > 1)  {
		first_prealloc_ino = XFS_OFFBNO_TO_AGINO(mp,
					roundup(fino_bno,
						mp->m_sb.sb_inoalignmt),
					0);
	} else  {
		first_prealloc_ino = XFS_OFFBNO_TO_AGINO(mp, fino_bno, 0);
	}

	ASSERT(mp->m_ialloc_blks > 0);

	if (mp->m_ialloc_blks > 1)
		last_prealloc_ino = first_prealloc_ino + XFS_INODES_PER_CHUNK;
	else
		last_prealloc_ino = XFS_OFFBNO_TO_AGINO(mp, fino_bno + 1, 0);

	/*
	 * now the first 3 inodes in the system
	 */
	if (mp->m_sb.sb_rootino != first_prealloc_ino)  {
		do_warn(
_("sb root inode value %" PRIu64 " %sinconsistent with calculated value %u\n"),
			mp->m_sb.sb_rootino,
			(mp->m_sb.sb_rootino == NULLFSINO ? "(NULLFSINO) ":""),
			first_prealloc_ino);

		if (!no_modify)
			do_warn(
		_("resetting superblock root inode pointer to %u\n"),
				first_prealloc_ino);
		else
			do_warn(
		_("would reset superblock root inode pointer to %u\n"),
				first_prealloc_ino);

		/*
		 * just set the value -- safe since the superblock
		 * doesn't get flushed out if no_modify is set
		 */
		mp->m_sb.sb_rootino = first_prealloc_ino;
	}

	if (mp->m_sb.sb_rbmino != first_prealloc_ino + 1)  {
		do_warn(
_("sb realtime bitmap inode %" PRIu64 " %sinconsistent with calculated value %u\n"),
			mp->m_sb.sb_rbmino,
			(mp->m_sb.sb_rbmino == NULLFSINO ? "(NULLFSINO) ":""),
			first_prealloc_ino + 1);

		if (!no_modify)
			do_warn(
		_("resetting superblock realtime bitmap ino pointer to %u\n"),
				first_prealloc_ino + 1);
		else
			do_warn(
		_("would reset superblock realtime bitmap ino pointer to %u\n"),
				first_prealloc_ino + 1);

		/*
		 * just set the value -- safe since the superblock
		 * doesn't get flushed out if no_modify is set
		 */
		mp->m_sb.sb_rbmino = first_prealloc_ino + 1;
	}

	if (mp->m_sb.sb_rsumino != first_prealloc_ino + 2)  {
		do_warn(
_("sb realtime summary inode %" PRIu64 " %sinconsistent with calculated value %u\n"),
			mp->m_sb.sb_rsumino,
			(mp->m_sb.sb_rsumino == NULLFSINO ? "(NULLFSINO) ":""),
			first_prealloc_ino + 2);

		if (!no_modify)
			do_warn(
		_("resetting superblock realtime summary ino pointer to %u\n"),
				first_prealloc_ino + 2);
		else
			do_warn(
		_("would reset superblock realtime summary ino pointer to %u\n"),
				first_prealloc_ino + 2);

		/*
		 * just set the value -- safe since the superblock
		 * doesn't get flushed out if no_modify is set
		 */
		mp->m_sb.sb_rsumino = first_prealloc_ino + 2;
	}

}

/*
 * v5 superblock metadata track the LSN of last modification and thus require
 * that the current LSN is always moving forward. The current LSN is reset if
 * the log has been cleared, which puts the log behind parts of the filesystem
 * on-disk and can disrupt log recovery.
 *
 * We have tracked the maximum LSN of every piece of metadata that has been read
 * in via the read verifiers. Compare the max LSN with the log and if the log is
 * behind, bump the cycle number and reformat the log.
 */
static void
format_log_max_lsn(
	struct xfs_mount	*mp)
{
	struct xlog		*log = mp->m_log;
	int			max_cycle;
	int			max_block;
	int			new_cycle;
	xfs_daddr_t		logstart;
	xfs_daddr_t		logblocks;
	int			logversion;

	if (!xfs_sb_version_hascrc(&mp->m_sb))
		return;

	/*
	 * If the log is ahead of the highest metadata LSN we've seen, we're
	 * safe and there's nothing to do.
	 */
	max_cycle = CYCLE_LSN(libxfs_max_lsn);
	max_block = BLOCK_LSN(libxfs_max_lsn);
	if (max_cycle < log->l_curr_cycle ||
	    (max_cycle == log->l_curr_cycle && max_block < log->l_curr_block))
		return;

	/*
	 * Going to the next cycle should be sufficient but we bump by a few
	 * counts to help cover any metadata LSNs we could have missed.
	 */
	new_cycle = max_cycle + 3;
	logstart = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart);
	logblocks = XFS_FSB_TO_BB(mp, mp->m_sb.sb_logblocks);
	logversion = xfs_sb_version_haslogv2(&mp->m_sb) ? 2 : 1;

	do_warn(_("Maximum metadata LSN (%d:%d) is ahead of log (%d:%d).\n"),
		max_cycle, max_block, log->l_curr_cycle, log->l_curr_block);

	if (no_modify) {
		do_warn(_("Would format log to cycle %d.\n"), new_cycle);
		return;
	}

	do_warn(_("Format log to cycle %d.\n"), new_cycle);
	libxfs_log_clear(log->l_dev, NULL, logstart, logblocks,
			 &mp->m_sb.sb_uuid, logversion, mp->m_sb.sb_logsunit,
			 XLOG_FMT, new_cycle, true);
}

/*
 * mkfs increases the AG count for "multidisk" configurations, we want
 * to target these for an increase in thread count. Hence check the superlock
 * geometry information to determine if mkfs considered this a multidisk
 * configuration.
 */
static bool
is_multidisk_filesystem(
	struct xfs_mount	*mp)
{
	struct xfs_sb		*sbp = &mp->m_sb;

	/* High agcount filesystems are always considered "multidisk" */
	if (sbp->sb_agcount >= XFS_MULTIDISK_AGCOUNT)
		return true;

	/*
	 * If it doesn't have a sunit/swidth, mkfs didn't consider it a
	 * multi-disk array, so we don't either.
	 */
	if (!sbp->sb_unit)
		return false;

	ASSERT(sbp->sb_width);
	return true;
}

/*
 * if the sector size of the filesystem we are trying to repair is
 * smaller than that of the underlying filesystem (i.e. we are repairing
 * an image), the we have to turn off direct IO because we cannot do IO
 * smaller than the host filesystem's sector size.
 */
static void
check_fs_vs_host_sectsize(
	struct xfs_sb	*sb)
{
	int	fd;
	long	old_flags;
	struct xfs_fsop_geom_v1 geom = { 0 };

	fd = libxfs_device_to_fd(x.ddev);

	if (ioctl(fd, XFS_IOC_FSGEOMETRY_V1, &geom) < 0) {
		do_log(_("Cannot get host filesystem geometry.\n"
	"Repair may fail if there is a sector size mismatch between\n"
	"the image and the host filesystem.\n"));
		geom.sectsize = BBSIZE;
	}

	if (sb->sb_sectsize < geom.sectsize) {
		old_flags = fcntl(fd, F_GETFL, 0);
		if (fcntl(fd, F_SETFL, old_flags & ~O_DIRECT) < 0) {
			do_warn(_(
	"Sector size on host filesystem larger than image sector size.\n"
	"Cannot turn off direct IO, so exiting.\n"));
			exit(1);
		}
	}
}

int
main(int argc, char **argv)
{
	xfs_mount_t	*temp_mp;
	xfs_mount_t	*mp;
	xfs_dsb_t	*dsb;
	xfs_buf_t	*sbp;
	xfs_mount_t	xfs_m;
	struct xlog	log = {0};
	char		*msgbuf;
	struct xfs_sb	psb;
	int		rval;

	progname = basename(argv[0]);
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	dinode_bmbt_translation_init();

	temp_mp = &xfs_m;
	setbuf(stdout, NULL);

	process_args(argc, argv);
	xfs_init(&x);

	msgbuf = malloc(DURATION_BUF_SIZE);

	timestamp(PHASE_START, 0, NULL);
	timestamp(PHASE_END, 0, NULL);

	/* -f forces this, but let's be nice and autodetect it, as well. */
	if (!isa_file) {
		int		fd = libxfs_device_to_fd(x.ddev);
		struct stat	statbuf;

		if (fstat(fd, &statbuf) < 0)
			do_warn(_("%s: couldn't stat \"%s\"\n"),
				progname, fs_name);
		else if (S_ISREG(statbuf.st_mode))
			isa_file = 1;
	}

	if (isa_file) {
		/* Best effort attempt to validate fs vs host sector size */
		rval = get_sb(&psb, 0, XFS_MAX_SECTORSIZE, 0);
		if (rval == XR_OK)
			check_fs_vs_host_sectsize(&psb);
	}

	/* do phase1 to make sure we have a superblock */
	phase1(temp_mp);
	timestamp(PHASE_END, 1, NULL);

	if (no_modify && primary_sb_modified)  {
		do_warn(_("Primary superblock would have been modified.\n"
			  "Cannot proceed further in no_modify mode.\n"
			  "Exiting now.\n"));
		exit(1);
	}

	rval = get_sb(&psb, 0, XFS_MAX_SECTORSIZE, 0);
	if (rval != XR_OK) {
		do_warn(_("Primary superblock bad after phase 1!\n"
			  "Exiting now.\n"));
		exit(1);
	}

	/*
	 * Now that we have completely validated the superblock, geometry may
	 * have changed; re-check geometry vs the host filesystem geometry
	 */
	if (isa_file)
		check_fs_vs_host_sectsize(&psb);

	/*
	 * Prepare the mount structure. Point the log reference to our local
	 * copy so it's available to the various phases. The log bits are
	 * initialized in phase 2.
	 */
	memset(&xfs_m, 0, sizeof(xfs_mount_t));
	mp = libxfs_mount(&xfs_m, &psb, x.ddev, x.logdev, x.rtdev, 0);

	if (!mp)  {
		fprintf(stderr,
			_("%s: cannot repair this filesystem.  Sorry.\n"),
			progname);
		exit(1);
	}
	mp->m_log = &log;

	/* Spit out function & line on these corruption macros */
	if (verbose > 2)
		mp->m_flags |= LIBXFS_MOUNT_WANT_CORRUPTED;

	/*
	 * set XFS-independent status vars from the mount/sb structure
	 */
	glob_agcount = mp->m_sb.sb_agcount;

	chunks_pblock = mp->m_sb.sb_inopblock / XFS_INODES_PER_CHUNK;
	max_symlink_blocks = libxfs_symlink_blocks(mp, XFS_SYMLINK_MAXLEN);
	inodes_per_cluster = MAX(mp->m_sb.sb_inopblock,
			mp->m_inode_cluster_size >> mp->m_sb.sb_inodelog);

	/*
	 * Automatic striding for high agcount filesystems.
	 *
	 * More AGs indicates that the filesystem is either large or can handle
	 * more IO parallelism. Either way, we should try to process multiple
	 * AGs at a time in such a configuration to try to saturate the
	 * underlying storage and speed the repair process. Only do this if
	 * prefetching is enabled.
	 *
	 * Given mkfs defaults for 16AGs for "multidisk" configurations, we want
	 * to target these for an increase in thread count. Hence a stride value
	 * of 15 is chosen to ensure we get at least 2 AGs being scanned at once
	 * on such filesystems.
	 *
	 * Limit the maximum thread count based on the available CPU power that
	 * is available. If we use too many threads, we might run out of memory
	 * and CPU power before we run out of IO concurrency. We limit to 8
	 * threads/CPU as this is enough threads to saturate a CPU on fast
	 * devices, yet few enough that it will saturate but won't overload slow
	 * devices.
	 *
	 * Multidisk filesystems can handle more IO parallelism so we should try
	 * to process multiple AGs at a time in such a configuration to try to
	 * saturate the underlying storage and speed the repair process. Only do
	 * this if prefetching is enabled.
	 */
	if (!ag_stride && do_prefetch && is_multidisk_filesystem(mp)) {
		/*
		 * For small agcount multidisk systems, just double the
		 * parallelism. For larger AG count filesystems (32 and above)
		 * use more parallelism, and linearly increase the parallelism
		 * with the number of AGs.
		 */
		ag_stride = min(glob_agcount, XFS_MULTIDISK_AGCOUNT / 2) - 1;
	}

	if (ag_stride) {
		int max_threads = platform_nproc() * 8;

		thread_count = (glob_agcount + ag_stride - 1) / ag_stride;
		while (thread_count > max_threads) {
			ag_stride *= 2;
			thread_count = (glob_agcount + ag_stride - 1) /
								ag_stride;
		}
		if (thread_count > 0)
			thread_init();
		else {
			thread_count = 1;
			ag_stride = 0;
		}
	}

	if (ag_stride && report_interval) {
		init_progress_rpt();
		if (msgbuf) {
			do_log(_("        - reporting progress in intervals of %s\n"),
			duration(report_interval, msgbuf));
		}
	}

	/*
	 * Adjust libxfs cache sizes based on system memory,
	 * filesystem size and inode count.
	 *
	 * We'll set the cache size based on 3/4s the memory minus
	 * space used by the inode AVL tree and block usage map.
	 *
	 * Inode AVL tree space is approximately 4 bytes per inode,
	 * block usage map is currently 1 byte for 2 blocks.
	 *
	 * We assume most blocks will be inode clusters.
	 *
	 * Calculations are done in kilobyte units.
	 */

	if (!bhash_option_used || max_mem_specified) {
		unsigned long 	mem_used;
		unsigned long	max_mem;
		struct rlimit	rlim;

		libxfs_bcache_purge();
		cache_destroy(libxfs_bcache);

		mem_used = (mp->m_sb.sb_icount >> (10 - 2)) +
					(mp->m_sb.sb_dblocks >> (10 + 1)) +
					50000;	/* rough estimate of 50MB overhead */
		max_mem = max_mem_specified ? max_mem_specified * 1024 :
						libxfs_physmem() * 3 / 4;

		if (getrlimit(RLIMIT_AS, &rlim) != -1 &&
					rlim.rlim_cur != RLIM_INFINITY) {
			rlim.rlim_cur = rlim.rlim_max;
			setrlimit(RLIMIT_AS, &rlim);
			/* use approximately 80% of rlimit to avoid overrun */
			max_mem = MIN(max_mem, rlim.rlim_cur / 1280);
		} else
			max_mem = MIN(max_mem, (LONG_MAX >> 10) + 1);

		if (verbose > 1)
			do_log(
	_("        - max_mem = %lu, icount = %" PRIu64 ", imem = %" PRIu64 ", dblock = %" PRIu64 ", dmem = %" PRIu64 "\n"),
				max_mem, mp->m_sb.sb_icount,
				mp->m_sb.sb_icount >> (10 - 2),
				mp->m_sb.sb_dblocks,
				mp->m_sb.sb_dblocks >> (10 + 1));

		if (max_mem <= mem_used) {
			if (max_mem_specified) {
				do_abort(
	_("Required memory for repair is greater that the maximum specified\n"
	  "with the -m option. Please increase it to at least %lu.\n"),
					mem_used / 1024);
			}
			do_log(
	_("Memory available for repair (%luMB) may not be sufficient.\n"
	  "At least %luMB is needed to repair this filesystem efficiently\n"
	  "If repair fails due to lack of memory, please\n"),
				max_mem / 1024, mem_used / 1024);
			if (do_prefetch)
				do_log(
	_("turn prefetching off (-P) to reduce the memory footprint.\n"));
			else
				do_log(
	_("increase system RAM and/or swap space to at least %luMB.\n"),
			mem_used * 2 / 1024);

			max_mem = mem_used;
		}

		max_mem -= mem_used;
		if (max_mem >= (1 << 30))
			max_mem = 1 << 30;
		libxfs_bhash_size = max_mem / (HASH_CACHE_RATIO *
				(mp->m_inode_cluster_size >> 10));
		if (libxfs_bhash_size < 512)
			libxfs_bhash_size = 512;

		if (verbose)
			do_log(_("        - block cache size set to %d entries\n"),
				libxfs_bhash_size * HASH_CACHE_RATIO);

		libxfs_bcache = cache_init(0, libxfs_bhash_size,
						&libxfs_bcache_operations);
	}

	/*
	 * calculate what mkfs would do to this filesystem
	 */
	calc_mkfs(mp);

	/*
	 * initialize block alloc map
	 */
	init_bmaps(mp);
	incore_ino_init(mp);
	incore_ext_init(mp);
	rmaps_init(mp);

	/* initialize random globals now that we know the fs geometry */
	inodes_per_block = mp->m_sb.sb_inopblock;

	if (parse_sb_version(&mp->m_sb))  {
		do_warn(
	_("Found unsupported filesystem features.  Exiting now.\n"));
		return(1);
	}

	/* make sure the per-ag freespace maps are ok so we can mount the fs */
	phase2(mp, phase2_threads);
	timestamp(PHASE_END, 2, NULL);

	if (do_prefetch)
		init_prefetch(mp);

	phase3(mp, phase2_threads);
	timestamp(PHASE_END, 3, NULL);

	phase4(mp);
	timestamp(PHASE_END, 4, NULL);

	if (no_modify)
		printf(_("No modify flag set, skipping phase 5\n"));
	else {
		phase5(mp);
	}
	timestamp(PHASE_END, 5, NULL);

	/*
	 * Done with the block usage maps, toss them...
	 */
	rmaps_free(mp);
	free_bmaps(mp);

	if (!bad_ino_btree)  {
		phase6(mp);
		timestamp(PHASE_END, 6, NULL);

		phase7(mp, phase2_threads);
		timestamp(PHASE_END, 7, NULL);
	} else  {
		do_warn(
_("Inode allocation btrees are too corrupted, skipping phases 6 and 7\n"));
	}

	if (lost_quotas && !have_uquotino && !have_gquotino && !have_pquotino) {
		if (!no_modify)  {
			do_warn(
_("Warning:  no quota inodes were found.  Quotas disabled.\n"));
		} else  {
			do_warn(
_("Warning:  no quota inodes were found.  Quotas would be disabled.\n"));
		}
	} else if (lost_quotas)  {
		if (!no_modify)  {
			do_warn(
_("Warning:  quota inodes were cleared.  Quotas disabled.\n"));
		} else  {
			do_warn(
_("Warning:  quota inodes would be cleared.  Quotas would be disabled.\n"));
		}
	} else  {
		if (lost_uquotino)  {
			if (!no_modify)  {
				do_warn(
_("Warning:  user quota information was cleared.\n"
  "User quotas can not be enforced until limit information is recreated.\n"));
			} else  {
				do_warn(
_("Warning:  user quota information would be cleared.\n"
  "User quotas could not be enforced until limit information was recreated.\n"));
			}
		}

		if (lost_gquotino)  {
			if (!no_modify)  {
				do_warn(
_("Warning:  group quota information was cleared.\n"
  "Group quotas can not be enforced until limit information is recreated.\n"));
			} else  {
				do_warn(
_("Warning:  group quota information would be cleared.\n"
  "Group quotas could not be enforced until limit information was recreated.\n"));
			}
		}

		if (lost_pquotino)  {
			if (!no_modify)  {
				do_warn(
_("Warning:  project quota information was cleared.\n"
  "Project quotas can not be enforced until limit information is recreated.\n"));
			} else  {
				do_warn(
_("Warning:  project quota information would be cleared.\n"
  "Project quotas could not be enforced until limit information was recreated.\n"));
			}
		}
	}

	if (ag_stride && report_interval)
		stop_progress_rpt();

	if (no_modify)  {
		/*
		 * Warn if the current LSN is problematic and the log requires a
		 * reformat.
		 */
		format_log_max_lsn(mp);

		do_log(
	_("No modify flag set, skipping filesystem flush and exiting.\n"));
		if (verbose)
			summary_report();
		if (fs_is_dirty)
			return(1);

		return(0);
	}

	/*
	 * Clear the quota flags if they're on.
	 */
	sbp = libxfs_getsb(mp, 0);
	if (!sbp)
		do_error(_("couldn't get superblock\n"));

	dsb = XFS_BUF_TO_SBP(sbp);

	if (be16_to_cpu(dsb->sb_qflags) & XFS_ALL_QUOTA_CHKD) {
		do_warn(_("Note - quota info will be regenerated on next "
			"quota mount.\n"));
		dsb->sb_qflags &= cpu_to_be16(~XFS_ALL_QUOTA_CHKD);
	}

	if (copied_sunit) {
		do_warn(
_("Note - stripe unit (%d) and width (%d) were copied from a backup superblock.\n"
  "Please reset with mount -o sunit=<value>,swidth=<value> if necessary\n"),
			be32_to_cpu(dsb->sb_unit), be32_to_cpu(dsb->sb_width));
	}

	libxfs_writebuf(sbp, 0);

	/*
	 * Done. Flush all cached buffers and inodes first to ensure all
	 * verifiers are run (where we discover the max metadata LSN), reformat
	 * the log if necessary and unmount.
	 */
	libxfs_bcache_flush();
	format_log_max_lsn(mp);
	libxfs_umount(mp);

	if (x.rtdev)
		libxfs_device_close(x.rtdev);
	if (x.logdev && x.logdev != x.ddev)
		libxfs_device_close(x.logdev);
	libxfs_device_close(x.ddev);

	if (verbose)
		summary_report();
	do_log(_("done\n"));

	if (dangerously && !no_modify)
		do_warn(
_("Repair of readonly mount complete.  Immediate reboot encouraged.\n"));

	pftrace_done();

	free(msgbuf);

	return (0);
}
