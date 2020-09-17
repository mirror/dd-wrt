// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#include "libxfs.h"
#include "libxlog.h"

#include "logprint.h"

/*
 * Extract a log and write it out to a file
 */

void
xfs_log_copy(
	struct xlog	*log,
	int		fd,
	char		*filename)
{
	int		ofd, r;
	xfs_daddr_t	blkno;
	char		buf[XLOG_HEADER_SIZE];

	if ((ofd = open(filename, O_CREAT|O_EXCL|O_RDWR|O_TRUNC, 0666)) == -1) {
		perror("open");
		exit(1);
	}

	xlog_print_lseek(log, fd, 0, SEEK_SET);
	for (blkno = 0; blkno < log->l_logBBsize; blkno++) {
		r = read(fd, buf, sizeof(buf));
		if (r < 0) {
			fprintf(stderr, _("%s: read error (%lld): %s\n"),
				__FUNCTION__, (long long)blkno,
				strerror(errno));
			continue;
		} else if (r == 0) {
			printf(_("%s: physical end of log at %lld\n"),
				__FUNCTION__, (long long)blkno);
			break;
		} else if (r != sizeof(buf)) {
			fprintf(stderr, _("%s: short read? (%lld)\n"),
					__FUNCTION__, (long long)blkno);
			continue;
		}

		r = write(ofd, buf, sizeof(buf));
		if (r < 0) {
			fprintf(stderr, _("%s: write error (%lld): %s\n"),
				__FUNCTION__, (long long)blkno,
				strerror(errno));
			break;
		} else if (r != sizeof(buf)) {
			fprintf(stderr, _("%s: short write? (%lld)\n"),
				__FUNCTION__, (long long)blkno);
			continue;
		}
	}

	close(ofd);
}
