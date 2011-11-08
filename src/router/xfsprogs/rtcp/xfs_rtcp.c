/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
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

#include <libxfs.h>

int rtcp(char *, char *, int);
int xfsrtextsize(char *path);

int pflag;
char *progname;

void
usage()
{
	fprintf(stderr, _("%s [-e extsize] [-p] source target\n"), progname);
	exit(2);
}

int
main(int argc, char **argv)
{
	register int	c, i, r, errflg = 0;
	struct stat64	s2;
	int		extsize = - 1;

	progname = basename(argv[0]);
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	while ((c = getopt(argc, argv, "pe:V")) != EOF) {
		switch (c) {
		case 'e':
			extsize = atoi(optarg);
			break;
		case 'p':
			pflag = 1;
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		default:
			errflg++;
		}
	}

	/*
	 * Check for sufficient arguments or a usage error.
	 */
	argc -= optind;
	argv  = &argv[optind];

	if (argc < 2) {
		fprintf(stderr, _("%s: must specify files to copy\n"),
			progname);
		errflg++;
	}

	if (errflg)
		usage();

	/*
	 * If there is more than a source and target,
	 * the last argument (the target) must be a directory
	 * which really exists.
	 */
	if (argc > 2) {
		if (stat64(argv[argc-1], &s2) < 0) {
			fprintf(stderr, _("%s: stat64 of %s failed\n"),
				progname, argv[argc-1]);
			exit(2);
		}

		if (!S_ISDIR(s2.st_mode)) {
			fprintf(stderr,
				_("%s: final argument is not directory\n"),
				progname);
			usage();
		}
	}

	/*
	 * Perform a multiple argument rtcp by
	 * multiple invocations of rtcp().
	 */
	r = 0;
	for (i = 0; i < argc-1; i++)
		r += rtcp(argv[i], argv[argc-1], extsize);

	/*
	 * Show errors by nonzero exit code.
	 */
	exit(r?2:0);
}

int
rtcp( char *source, char *target, int fextsize)
{
	int		fromfd, tofd, readct, writect, iosz, reopen;
	int		remove = 0, rtextsize;
	char		*sp, *fbuf, *ptr;
	char		tbuf[ PATH_MAX ];
	struct stat64	s1, s2;
	struct fsxattr	fsxattr;
	struct dioattr	dioattr;

	/*
	 * While source or target have trailing /, remove them
	 * unless only "/".
	 */
	sp = source + strlen(source);
	if (sp) {
		while (*--sp == '/' && sp > source)
			*sp = '\0';
	}
	sp = target + strlen(target);
	if (sp) {
		while (*--sp == '/' && sp > target)
			*sp = '\0';
	}

	if ( stat64(source, &s1) ) {
		fprintf(stderr, _("%s: failed stat64 on %s: %s\n"),
			progname, source, strerror(errno));
		return( -1);
	}

	/*
	 * check for a realtime partition
	 */
	snprintf(tbuf, sizeof(tbuf), "%s", target);
	if ( stat64(target, &s2) ) {
		if (!S_ISDIR(s2.st_mode)) {
			/* take out target file name */
			if ((ptr = strrchr(tbuf, '/')) != NULL)
				*ptr = '\0';
			else
				snprintf(tbuf, sizeof(tbuf), ".");
		}
	}

	if ( (rtextsize = xfsrtextsize( tbuf ))  <= 0 ) {
		fprintf(stderr,
			_("%s: %s filesystem has no realtime partition\n"),
			progname, tbuf);
		return( -1 );
	}

	/*
	 * check if target is a directory
	 */
	snprintf(tbuf, sizeof(tbuf), "%s", target);
	if ( !stat64(target, &s2) ) {
		if (S_ISDIR(s2.st_mode)) {
			snprintf(tbuf, sizeof(tbuf), "%s/%s", target,
				basename(source));
		}
	}

	if ( stat64(tbuf, &s2) ) {
		/*
		 * create the file if it does not exist
		 */
		if ( (tofd = open(tbuf, O_RDWR|O_CREAT|O_DIRECT, 0666)) < 0 ) {
			fprintf(stderr, _("%s: open of %s failed: %s\n"),
				progname, tbuf, strerror(errno));
			return( -1 );
		}
		remove = 1;

		/*
		 * mark the file as a realtime file
		 */
		fsxattr.fsx_xflags = XFS_XFLAG_REALTIME;
		if (fextsize != -1 )
			fsxattr.fsx_extsize = fextsize;
		else
			fsxattr.fsx_extsize = 0;

		if ( xfsctl(tbuf, tofd, XFS_IOC_FSSETXATTR, &fsxattr) ) {
			fprintf(stderr,
				_("%s: set attributes on %s failed: %s\n"),
				progname, tbuf, strerror(errno));
			close( tofd );
			unlink( tbuf );
			return( -1 );
		}
	} else {
		/*
		 * open existing file
		 */
		if ( (tofd = open(tbuf, O_RDWR|O_DIRECT)) < 0 ) {
			fprintf(stderr, _("%s: open of %s failed: %s\n"),
				progname, tbuf, strerror(errno));
			return( -1 );
		}

		if ( xfsctl(tbuf, tofd, XFS_IOC_FSGETXATTR, &fsxattr) ) {
			fprintf(stderr,
				_("%s: get attributes of %s failed: %s\n"),
				progname, tbuf, strerror(errno));
			close( tofd );
			return( -1 );
		}

		/*
		 * check if the existing file is already a realtime file
		 */
		if ( !(fsxattr.fsx_xflags & XFS_XFLAG_REALTIME) ) {
			fprintf(stderr, _("%s: %s is not a realtime file.\n"),
				progname, tbuf);
			return( -1 );
		}

		/*
		 * check for matching extent size
		 */
		if ( (fextsize != -1) && (fsxattr.fsx_extsize != fextsize) ) {
			fprintf(stderr, _("%s: %s file extent size is %d, "
					"instead of %d.\n"),
				progname, tbuf, fsxattr.fsx_extsize, fextsize);
			return( -1 );
		}
	}

	/*
	 * open the source file
	 */
	reopen = 0;
	if ( (fromfd = open(source, O_RDONLY|O_DIRECT)) < 0 ) {
		fprintf(stderr, _("%s: open of %s source failed: %s\n"),
			progname, source, strerror(errno));
		close( tofd );
		if (remove)
			unlink( tbuf );
		return( -1 );
	}

	fsxattr.fsx_xflags = 0;
	fsxattr.fsx_extsize = 0;
	if ( xfsctl(source, fromfd, XFS_IOC_FSGETXATTR, &fsxattr) ) {
		reopen = 1;
	} else {
		if (! (fsxattr.fsx_xflags & XFS_XFLAG_REALTIME) ){
			fprintf(stderr, _("%s: %s is not a realtime file.\n"),
				progname, source);
			reopen = 1;
		}
	}

	if (reopen) {
		close( fromfd );
		if ( (fromfd = open(source, O_RDONLY )) < 0 ) {
			fprintf(stderr, _("%s: open of %s source failed: %s\n"),
				progname, source, strerror(errno));
			close( tofd );
			if (remove)
				unlink( tbuf );
			return( -1 );
		}
	}

	/*
	 * get direct I/O parameters
	 */
	if ( xfsctl(tbuf, tofd, XFS_IOC_DIOINFO, &dioattr) ) {
		fprintf(stderr,
			_("%s: couldn't get direct I/O information: %s\n"),
			progname, strerror(errno));
		close( fromfd );
		close( tofd );
		if ( remove )
			unlink( tbuf );
		return( -1 );
	}

	if ( rtextsize % dioattr.d_miniosz ) {
		fprintf(stderr, _("%s: extent size %d not a multiple of %d.\n"),
			progname, rtextsize, dioattr.d_miniosz);
		close( fromfd );
		close( tofd );
		if ( remove )
			unlink( tbuf );
		return( -1 );
	}

	/*
	 * Check that the source file size is a multiple of the
	 * file system block size.
	 */
	if ( s1.st_size % dioattr.d_miniosz ) {
		printf(_("The size of %s is not a multiple of %d.\n"),
			source, dioattr.d_miniosz);
		if ( pflag ) {
			printf(_("%s will be padded to %lld bytes.\n"),
				tbuf, (long long)
				(((s1.st_size / dioattr.d_miniosz) + 1)  *
					dioattr.d_miniosz) );

		} else {
			printf(_("Use the -p option to pad %s to a "
				"size which is a multiple of %d bytes.\n"),
				tbuf, dioattr.d_miniosz);
			close( fromfd );
			close( tofd );
			if ( remove )
				unlink( tbuf );
			return( -1 );
		}
	}

	iosz =  dioattr.d_miniosz;
	fbuf = memalign( dioattr.d_mem, iosz);
	memset(fbuf, 0, iosz);

	/*
	 * read the entire source file
	 */
	while ( ( readct = read( fromfd, fbuf, iosz) ) != 0 ) {
		/*
		 * if there is a read error - break
		 */
		if (readct < 0 ) {
			break;
		}

		/*
		 * if there is a short read, pad to a block boundary
		 */
		if ( readct != iosz ) {
			if ( (readct % dioattr.d_miniosz)  != 0 )  {
				readct = ( (readct/dioattr.d_miniosz) + 1 ) *
					 dioattr.d_miniosz;
			}
		}

		/*
		 * write to target file
		 */
		writect = write( tofd, fbuf, readct);

		if ( writect != readct ) {
			fprintf(stderr, _("%s: write error: %s\n"),
				progname, strerror(errno));
			close(fromfd);
			close(tofd);
			free( fbuf );
			return( -1 );
		}

		memset( fbuf, 0, iosz);
	}

	close(fromfd);
	close(tofd);
	free( fbuf );
	return( 0 );
}

/*
 * Determine the realtime extent size of the XFS file system
 */
int
xfsrtextsize( char *path)
{
	int fd, rval, rtextsize;
	xfs_fsop_geom_v1_t geo;

	fd = open( path, O_RDONLY );
	if ( fd < 0 ) {
		fprintf(stderr, _("%s: could not open %s: %s\n"),
			progname, path, strerror(errno));
		return -1;
	}
	rval = xfsctl( path, fd, XFS_IOC_FSGEOMETRY_V1, &geo );
	close(fd);
	if ( rval < 0 )
		return -1;

	rtextsize = geo.rtextsize * geo.blocksize;

	return rtextsize;
}
