/*
 * Copyright (c) 2000-2002 Silicon Graphics, Inc.
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

/*
 * Estimate space of an XFS filesystem
 */
#include <xfs/libxfs.h>
#include <sys/stat.h>
#include <ftw.h>

unsigned long long
cvtnum(char *s)
{
	unsigned long long i;
	char *sp;

	i = strtoll(s, &sp, 0);
	if (i == 0 && sp == s)
		return 0LL;
	if (*sp == '\0')
		return i;
	if (*sp =='k' && sp[1] == '\0')
		return 1024LL * i;
	if (*sp =='m' && sp[1] == '\0')
		return 1024LL * 1024LL * i;
	if (*sp =='g' && sp[1] == '\0')
		return 1024LL * 1024LL * 1024LL * i;
	return 0LL;
}

int ffn(const char *, const struct stat64 *, int, struct FTW *);

#define BLOCKSIZE	4096
#define INODESIZE	256
#define PERDIRENTRY	\
	(sizeof(xfs_dir_leaf_entry_t) + sizeof(xfs_dir_leaf_name_t))
#define LOGSIZE		1000

#define FBLOCKS(n)	((n)/blocksize)
#define RFBYTES(n)	((n) - (FBLOCKS(n) * blocksize))

unsigned long long dirsize=0;		/* bytes */
unsigned long long logsize=LOGSIZE*BLOCKSIZE;	/* bytes */
unsigned long long fullblocks=0;	/* FS blocks */
unsigned long long isize=0;		/* inodes bytes */
unsigned long long blocksize=BLOCKSIZE;
unsigned long long nslinks=0;		/* number of symbolic links */
unsigned long long nfiles=0;		/* number of regular files */
unsigned long long ndirs=0;		/* number of directories */
unsigned long long nspecial=0;		/* number of special files */
unsigned long long verbose=0;		/* verbose mode TRUE/FALSE */

int __debug = 0;
int ilog = 0;
int elog = 0;

void
usage(char *progname)
{
	fprintf(stderr,
		_("Usage: %s [opts] directory [directory ...]\n"
		"\t-b blocksize (fundamental filesystem blocksize)\n"
		"\t-i logsize (internal log size)\n"
		"\t-e logsize (external log size)\n"
		"\t-v prints more verbose messages\n"
		"\t-h prints this usage message\n\n"
	"Note:\tblocksize may have 'k' appended to indicate x1024\n"
	"\tlogsize may also have 'm' appended to indicate (1024 x 1024)\n"),
		basename(progname));
	exit(1);
}

int
main(int argc, char **argv)
{
	unsigned long long est;
	extern int optind;
	extern char *optarg;
	char dname[40];
	int c;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	while ((c = getopt (argc, argv, "b:hdve:i:V")) != EOF) {
		switch (c) {
		case 'b':
			blocksize=cvtnum(optarg);
			if (blocksize <= 0LL) {
				fprintf(stderr, _("blocksize %llu too small\n"),
					blocksize);
				usage(argv[0]);
			}
			else if (blocksize > 64LL * 1024LL) {
				fprintf(stderr, _("blocksize %llu too large\n"),
					blocksize);
				usage(argv[0]);
			}
			break;
		case 'i':
			if (elog) {
				fprintf(stderr, _("already have external log "
					"noted, can't have both\n"));
				usage(argv[0]);
			}
			logsize=cvtnum(optarg);
			ilog++;
			break;
		case 'e':
			if (ilog) {
				fprintf(stderr, _("already have internal log "
					"noted, can't have both\n"));
				usage(argv[0]);
			}
			logsize=cvtnum(optarg);
			elog++;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'd':
			__debug++;
			break;
		case 'V':
			printf(_("%s version %s\n"), basename(argv[0]), VERSION);
			exit(0);
		default:
		case 'h':
			usage(argv[0]);
		}
	}

	if (optind == argc)
		usage(argv[0]);

	if (!elog && !ilog) {
		ilog=1;
		logsize=LOGSIZE * blocksize;
	}
	if (verbose)
		printf(_("directory                               bsize   blocks    megabytes    logsize\n"));

	for ( ; optind < argc; optind++) {
		dirsize=0LL;		/* bytes */
		fullblocks=0LL;		/* FS blocks */
		isize=0LL;		/* inodes bytes */
		nslinks=0LL;		/* number of symbolic links */
		nfiles=0LL;		/* number of regular files */
		ndirs=0LL;		/* number of directories */
		nspecial=0LL;		/* number of special files */

		nftw64(argv[optind], ffn, 40, FTW_PHYS | FTW_MOUNT);

		if (__debug) {
			printf(_("dirsize=%llu\n"), dirsize);
			printf(_("fullblocks=%llu\n"), fullblocks);
			printf(_("isize=%llu\n"), isize);

			printf(_("%llu regular files\n"), nfiles);
			printf(_("%llu symbolic links\n"), nslinks);
			printf(_("%llu directories\n"), ndirs);
			printf(_("%llu special files\n"), nspecial);
		}

		est = FBLOCKS(isize) + 8	/* blocks for inodes */
			+ FBLOCKS(dirsize) + 1	/* blocks for directories */
			+ fullblocks  		/* blocks for file contents */
			+ (8 * 16)	/* fudge for overhead blks (per ag) */
			+ FBLOCKS(isize / INODESIZE); /* 1 byte/inode for map */

		if (ilog)
			est += (logsize / blocksize);

		if (!verbose) {
			printf(_("%s will take about %.1f megabytes\n"),
				argv[optind],
				(double)est*(double)blocksize/(1024.0*1024.0));
		} else {
			/* avoid running over 39 characters in field */
			strncpy(dname, argv[optind], 40);
			dname[39] = '\0';
			printf(_("%-39s %5llu %8llu %10.1fMB %10llu\n"),
			dname, blocksize, est,
			(double)est*(double)blocksize/(1024.0*1024.0), logsize);
		}

		if (!verbose && elog) {
			printf(_("\twith the external log using %llu blocks "),
			logsize/blocksize);
			printf(_("or about %.1f megabytes\n"),
			(double)logsize/(1024.0*1024.0));
		}
	}
	return 0;
}

int
ffn(const char *path, const struct stat64 *stb, int flags, struct FTW *f)
{
	/* cases are in most-encountered to least-encountered order */
	dirsize+=PERDIRENTRY+strlen(path);
	isize+=INODESIZE;
	switch (S_IFMT & stb->st_mode) {
	case S_IFREG:			/* regular files */
		fullblocks+=FBLOCKS(stb->st_blocks * 512 + blocksize-1);
		if (stb->st_blocks * 512 < stb->st_size)
			fullblocks++;	/* add one bmap block here */
		nfiles++;
		break;
	case S_IFLNK:			/* symbolic links */
		if (stb->st_size >= (INODESIZE - (sizeof(xfs_dinode_t)+4)))
			fullblocks+=FBLOCKS(stb->st_size + blocksize-1);
		nslinks++;
		break;
	case S_IFDIR:			/* directories */
		dirsize+=blocksize;	/* fudge upwards */
		if (stb->st_size >= blocksize)
			dirsize+=blocksize;
		ndirs++;
		break;
	case S_IFIFO:			/* named pipes */
	case S_IFCHR:			/* Character Special device */
	case S_IFBLK:			/* Block Special device */
	case S_IFSOCK:			/* socket */
		nspecial++;
		break;
	}
	return 0;
}
