/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
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

/*
 * Extract a log and write it out to a file
 */

void
xfs_log_copy(
	xlog_t		*log,
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
