/*
 * archives.c
 * Detection of (Unix) archive formats
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

/*
 * detection of various archive headers
 *
 * Most of this is based on the tapetype program by Doug Merritt,
 * posted to the net in 1992. Some parts are augmented with stuff from
 * /etc/magic.
 */

int detect_archive(SECTION *section, int level)
{
	int fill, i, stored_sum, sum, en;
	u4 magic;
	unsigned char *buf;

	fill = get_buffer(section, 0, 512, (void **)&buf);
	if (fill < 512)
		return 0;

	/* tar archives */
	sum = 0;
	for (i = 0; i < 148; i++)
		sum += (char)buf[i];
	for (; i < 156; i++)
		sum += ' ';
	for (; i < 512; i++)
		sum += (char)buf[i];
	stored_sum = 0;
	for (i = 148; i < 156; i++) {
		if (buf[i] == 0)
			break;
		else if (buf[i] >= '0' && buf[i] <= '7')
			stored_sum = (stored_sum * 8) + (buf[i] - '0');
		else if (buf[i] != ' ') {
			stored_sum =
				-1; /* make it mismatch, since this is an error */
			break;
		}
	}
	if (sum == stored_sum) {
		if (memcmp((char *)buf + 257, "ustar  \0", 8) == 0) {
			print_line(level, "GNU tar archive");
			return 1;
		} else if (memcmp((char *)buf + 257, "ustar\0", 6) == 0) {
			print_line(level, "POSIX tar archive");
			return 1;
		} else {
			print_line(level, "Pre-POSIX tar archive");
			return 1;
		}
	}

	/* cpio */
	if (get_le_short(buf) == 070707) {
		print_line(level, "cpio archive, little-endian binary");
		return 1;
	} else if (get_be_short(buf) == 070707) {
		print_line(level, "cpio archive, big-endian binary");
		return 1;
	} else if (memcmp(buf, "07070", 5) == 0) {
		print_line(level, "cpio archive, ascii");
		return 1;
	}

	/* bar */
	if (memcmp(buf + 65, "\x56\0", 2) == 0) {
		print_line(level, "bar archive");
		return 1;
	}

	/* dump */
	for (en = 0; en < 2; en++) {
		magic = get_ve_long(en, buf + 24);

		if (magic == 60011) {
			print_line(level,
				   "dump: 4.1BSD (or older) or Sun OFS, %s",
				   get_ve_name(en));
			return 1;
		} else if (magic == 60012) {
			print_line(
				level,
				"dump: 4.2BSD (or newer) without IDC or Sun NFS, %s",
				get_ve_name(en));
			return 1;
		} else if (magic == 60013) {
			print_line(level,
				   "dump: 4.2BSD (or newer) with IDC, %s",
				   get_ve_name(en));
			return 1;
		} else if (magic == 60014) {
			print_line(level, "dump: Convex Storage Manager, %s",
				   get_ve_name(en));
			return 1;
		}
	}
	return 0;
}

/* EOF */
