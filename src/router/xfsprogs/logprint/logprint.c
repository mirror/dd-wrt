/*
 * Copyright (c) 2000-2004 Silicon Graphics, Inc.
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

#include "logprint.h"
#include <sys/types.h>
#include <sys/stat.h>

#define OP_PRINT	0
#define OP_PRINT_TRANS	1
#define OP_DUMP		2
#define OP_COPY		3

int	print_data;
int	print_only_data;
int	print_inode;
int	print_quota;
int	print_buffer;
int	print_overwrite;
int     print_no_data;
int     print_no_print;
int     print_exit = 1; /* -e is now default. specify -c to override */
int	print_operation = OP_PRINT;

void
usage(void)
{
	fprintf(stderr, _("Usage: %s [options...] <device>\n\n\
Options:\n\
    -c	            try to continue if error found in log\n\
    -C <filename>   copy the log from the filesystem to filename\n\
    -d	            dump the log in log-record format\n\
    -f	            specified device is actually a file\n\
    -l <device>     filename of external log\n\
    -n	            don't try and interpret log data\n\
    -o	            print buffer data in hex\n\
    -s <start blk>  block # to start printing\n\
    -v              print \"overwrite\" data\n\
    -t	            print out transactional view\n\
	-b          in transactional view, extract buffer info\n\
	-i          in transactional view, extract inode info\n\
	-q          in transactional view, extract quota info\n\
    -D              print only data; no decoding\n\
    -V              print version information\n"),
	progname);
	exit(1);
}

int
logstat(xfs_mount_t *mp)
{
	int		fd;
	char		buf[BBSIZE];
	xfs_sb_t	*sb;

	/* On Linux we always read the superblock of the
	 * filesystem. We need this to get the length of the
	 * log. Otherwise we end up seeking forever. -- mkp
	 */
	if ((fd = open(x.dname, O_RDONLY)) == -1) {
		fprintf(stderr, _("    Can't open device %s: %s\n"),
			x.dname, strerror(errno));
		exit(1);
	}
	lseek64(fd, 0, SEEK_SET);
	if (read(fd, buf, sizeof(buf)) != sizeof(buf)) {
		fprintf(stderr, _("    read of XFS superblock failed\n"));
		exit(1);
	}
	close (fd);

	if (!x.disfile) {
		/*
		 * Conjure up a mount structure
		 */
		sb = &mp->m_sb;
		libxfs_sb_from_disk(sb, (xfs_dsb_t *)buf);
		mp->m_blkbb_log = sb->sb_blocklog - BBSHIFT;

		x.logBBsize = XFS_FSB_TO_BB(mp, sb->sb_logblocks);
		x.logBBstart = XFS_FSB_TO_DADDR(mp, sb->sb_logstart);
		if (!x.logname && sb->sb_logstart == 0) {
			fprintf(stderr, _("    external log device not specified\n\n"));
			usage();
			/*NOTREACHED*/
		}
	} else {
		struct stat	s;

		stat(x.dname, &s);
		x.logBBsize = s.st_size >> 9;
		x.logBBstart = 0;
	}


	if (x.logname && *x.logname) {    /* External log */
		if ((fd = open(x.logname, O_RDONLY)) == -1) {
			fprintf(stderr, _("Can't open file %s: %s\n"),
				x.logname, strerror(errno));
			exit(1);
		}
		close(fd);
	} else {                            /* Internal log */
		x.logdev = x.ddev;
	}

	return 0;
}

int
main(int argc, char **argv)
{
	int		print_start = -1;
	int		c;
	int             logfd;
	char		*copy_file = NULL;
	xlog_t	        log = {0};
	xfs_mount_t	mount;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	progname = basename(argv[0]);
	while ((c = getopt(argc, argv, "bC:cdefl:iqnors:tDVv")) != EOF) {
		switch (c) {
			case 'D':
				print_only_data++;
				print_data++;
				break;
			case 'b':
				print_buffer++;
				break;
			case 'c':
			    /* default is to stop on error.
			     * -c turns this off.
			     */
				print_exit = 0;
				break;
			case 'e':
			    /* -e is now default
			     */
				print_exit++;
				break;
			case 'C':
				print_operation = OP_COPY;
				copy_file = optarg;
				break;
			case 'd':
				print_operation = OP_DUMP;
				break;
			case 'f':
				print_skip_uuid++;
				x.disfile = 1;
				break;
			case 'l':
				x.logname = optarg;
				x.lisfile = 1;
				break;
			case 'i':
				print_inode++;
				break;
			case 'q':
				print_quota++;
				break;
			case 'n':
				print_no_data++;
				break;
			case 'o':
				print_data++;
				break;
			case 's':
				print_start = atoi(optarg);
				break;
			case 't':
				print_operation = OP_PRINT_TRANS;
				break;
			case 'v':
				print_overwrite++;
				break;
			case 'V':
				printf(_("%s version %s\n"), progname, VERSION);
				exit(0);
			case '?':
				usage();
		}
	}

	if (argc - optind != 1)
		usage();

	x.dname = argv[optind];

	if (x.dname == NULL)
		usage();

	x.isreadonly = LIBXFS_ISINACTIVE;
	printf(_("xfs_logprint:\n"));
	if (!libxfs_init(&x))
		exit(1);

	logstat(&mount);

	logfd = (x.logfd < 0) ? x.dfd : x.logfd;

	printf(_("    data device: 0x%llx\n"), (unsigned long long)x.ddev);

	if (x.logname) {
		printf(_("    log file: \"%s\" "), x.logname);
	} else {
		printf(_("    log device: 0x%llx "), (unsigned long long)x.logdev);
	}

	printf(_("daddr: %lld length: %lld\n\n"),
		(long long)x.logBBstart, (long long)x.logBBsize);

	ASSERT(x.logBBsize <= INT_MAX);

	log.l_dev         = x.logdev;
	log.l_logsize     = BBTOB(x.logBBsize);
	log.l_logBBstart  = x.logBBstart;
	log.l_logBBsize   = x.logBBsize;
	log.l_mp          = &mount;

	switch (print_operation) {
	case OP_PRINT:
		xfs_log_print(&log, logfd, print_start);
		break;
	case OP_PRINT_TRANS:
		xfs_log_print_trans(&log, print_start);
		break;
	case OP_DUMP:
		xfs_log_dump(&log, logfd, print_start);
		break;
	case OP_COPY:
		xfs_log_copy(&log, logfd, copy_file);
		break;
	}
	exit(0);
}
