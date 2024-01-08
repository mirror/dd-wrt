/*
 * cdaccess.c
 * System-dependent code to access extended CD information
 *
 * Based on a contribution by Erik Andersen
 *
 * Copyright (c) 2003 Christoph Pfisterer
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 */

#include "global.h"

#ifdef USE_IOCTL_LINUX
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#define DO_CDACCESS 1
#endif /* USE_IOCTL_LINUX */

#ifdef DO_CDACCESS

/*
 * CDDB stuff
 */

static int cddb_sum(int n);

#define LBA_TO_SECS(lba) (((lba) + 150) / 75)

/*
 * CD structure:
 * Tries to get the CD TOC using out-of-band means (i.e. ioctl()),
 * prints the info, and analyzes just the data tracks. Quite
 * system-dependent, but layed out for porability.
 */

int analyze_cdaccess(int fd, SOURCE *s, int level)
{
	int i;
	int first, last, ntracks;
	int cksum, totaltime, seconds;
	u4 diskid;
	u1 ctrl[100];
	u4 lba[100], length;
	char human_readable_size[256];
#ifdef USE_IOCTL_LINUX
	struct cdrom_tochdr tochdr;
	struct cdrom_tocentry tocentry;
#endif

	/* read TOC header */
#ifdef USE_IOCTL_LINUX
	if (ioctl(fd, CDROMREADTOCHDR, &tochdr) < 0) {
		return 0;
	}
	first = tochdr.cdth_trk0;
	last = tochdr.cdth_trk1;
#endif

	ntracks = last + 1 - first;
	if (ntracks > 99) /* natural limit */
		return 0;

	/* read per-track data from TOC */
	for (i = 0; i <= ntracks; i++) {
#ifdef USE_IOCTL_LINUX
		if (i == ntracks)
			tocentry.cdte_track = CDROM_LEADOUT;
		else
			tocentry.cdte_track = first + i;
		tocentry.cdte_format = CDROM_LBA;
		if (ioctl(fd, CDROMREADTOCENTRY, &tocentry) < 0) {
			//printf("CDROMREADTOCENTRY: %s: ", strerror(errno));
			return 0;
		}
		ctrl[i] = tocentry.cdte_ctrl;
		lba[i] = tocentry.cdte_addr.lba;
#endif
	}

	/* System-dependent code ends here. From now on, we use the data
	   in first, last, ntracks, ctrl[], and lba[]. We also assume
	   all systems treat actual data access the same way... */

	/* calculate CDDB disk id */
	cksum = 0;
	for (i = 0; i < ntracks; i++) {
		cksum += cddb_sum(LBA_TO_SECS(lba[i]));
	}
	totaltime = LBA_TO_SECS(lba[ntracks]) - LBA_TO_SECS(lba[0]);
	diskid = (u4)(cksum % 0xff) << 24 | (u4)totaltime << 8 | (u4)ntracks;

	/* print disk info */
	print_line(level, "CD-ROM, %d track%s, CDDB disk ID %08lX", ntracks,
		   (ntracks != 1) ? "s" : "", diskid);

	/* Loop over each track */
	for (i = 0; i < ntracks; i++) {
		/* length of track in sectors */
		length = lba[i + 1] - lba[i];

		if ((ctrl[i] & 0x4) == 0) {
			/* Audio track, one sector holds 2352 actual data bytes */
			seconds = length / 75;
			format_size(human_readable_size, (u8)length * 2352);
			print_line(
				level,
				"Track %d: Audio track, %s, %3d min %02d sec",
				first + i, human_readable_size, seconds / 60,
				seconds % 60);

		} else {
			/* Data track, one sector holds 2048 actual data bytes */
			format_size(human_readable_size, length * 2048);
			print_line(level, "Track %d: Data track, %s", first + i,
				   human_readable_size);

			/* NOTE: we adjust the length to stay clear of padding or
			   post-gap stuff */
			analyze_source_special(s, level + 1, (u8)lba[i] * 2048,
					       (u8)(length - 250) * 2048);
		}
	}

	return 1;
}

/*
 * helper function: calculate cddb disk id
 */

static int cddb_sum(int n)
{
	/* a number like 2344 becomes 2+3+4+4 (13) */
	int ret = 0;

	while (n > 0) {
		ret = ret + (n % 10);
		n = n / 10;
	}

	return ret;
}

#else /* DO_CDACCESS */

/*
 * the system is not supported, so use a dummy function
 */

int analyze_cdaccess(int fd, SOURCE *s, int level)
{
	return 0;
}

#endif /* DO_CDACCESS */

/* EOF */
