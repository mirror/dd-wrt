/*
 * atari.c
 * Detection of ATARI ST partition map
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
 * ATARI ST partition map
 */

static char *get_name_for_type(char *type)
{
	if (memcmp(type, "GEM", 3) == 0)
		return "Standard GEMDOS";
	else if (memcmp(type, "BGM", 3) == 0)
		return "Big GEMDOS";
	else if (memcmp(type, "XGM", 3) == 0)
		return "Extended";
	return "Unknown";
}

static int detect_atari_partmap_ext(SECTION *section, u8 extbase, int level);

int detect_atari_partmap(SECTION *section, int level)
{
	unsigned char *buf;
	int i, off, used, flag, flags[4];
	u4 start, size, starts[4], sizes[4];
	char types[4][4], *type;
	u2 atari_csum;
	char s[256], append[64];
	int found = 0;

	/* partition maps only occur at the start of a device */
	if (section->pos != 0)
		return 0;

	if (get_buffer(section, 0, 512, (void **)&buf) < 512)
		return 0;

	/* check checksum */
	atari_csum = 0;
	for (i = 0; i < 512; i += 2)
		atari_csum += get_be_short(buf + i);
	if (atari_csum != 0x1234)
		return 0;

	/* get data and check if it's actually used */
	used = 0;
	for (off = 0x1c6, i = 0; i < 4; i++, off += 12) {
		flags[i] = buf[off];
		get_string(buf + off + 1, 3, types[i]);
		starts[i] = get_be_long(buf + off + 4);
		sizes[i] = get_be_long(buf + off + 8);
		if ((flags[i] & 1) && starts[i] != 0 && sizes[i] != 0)
			used = 1;
	}
	if (!used)
		return 0;

	/* print data and handle each partition */
	print_line(level, "ATARI ST partition map");
	for (i = 0; i < 4; i++) {
		start = starts[i];
		size = sizes[i];
		flag = flags[i];
		type = types[i];
		if ((flag & 1) == 0)
			continue;

		sprintf(append, " from %lu", start);
		if (flag & 0x80)
			strcat(append, ", bootable");
		format_blocky_size(s, size, 512, "sectors", append);
		print_line(level, "Partition %d: %s", i + 1, s);

		print_line(level + 1, "Type \"%s\" (%s)", type,
			   get_name_for_type(type));

		if (memcmp(type, "XGM", 3) == 0) {
			/* extended partition */
			if (level >= 0)
				detect_atari_partmap_ext(section, start,
							 level + 1);
			found = 1;
		} else {
			/* recurse for content detection */
			if (level >= 0)
				analyze_recursive(section, level + 1,
						  (u8)start * 512,
						  (u8)size * 512, 0);
			found = 1;
		}
	}
	return found;
}

static int detect_atari_partmap_ext(SECTION *section, u8 extbase, int level)
{
	unsigned char *buf;
	int extpartnum = 5;
	u8 tablebase, nexttablebase;
	int i, off, flags[4];
	u4 start, size, starts[4], sizes[4];
	char types[4][4], *type;
	char s[256], append[64];
	int found = 0;
	for (tablebase = extbase; tablebase; tablebase = nexttablebase) {
		/* read sector from linked list */
		if (get_buffer(section, tablebase << 9, 512, (void **)&buf) <
		    512)
			return 0;

		/* get data */
		for (off = 0x1c6, i = 0; i < 4; i++, off += 12) {
			flags[i] = buf[off];
			get_string(buf + off + 1, 3, types[i]);
			starts[i] = get_be_long(buf + off + 4);
			sizes[i] = get_be_long(buf + off + 8);
		}

		/* print data and handle each partition */
		nexttablebase = 0;
		for (i = 0; i < 4; i++) {
			start = starts[i];
			size = sizes[i];
			type = types[i];
			if ((flags[i] & 1) == 0)
				continue;

			if (memcmp(type, "XGM", 3) == 0) {
				/* link to next table */

				nexttablebase = extbase + start;
				found = 1;

			} else {
				/* real partition */

				sprintf(append, " from %lu", start);
				format_blocky_size(s, size, 512, "sectors",
						   append);
				print_line(level, "Partition %d: %s",
					   extpartnum, s);
				extpartnum++;

				print_line(level + 1, "Type \"%s\" (%s)", type,
					   get_name_for_type(type));

				/* recurse for content detection */
				if (level >= 0)
					analyze_recursive(section, level + 1,
							  (tablebase + start) *
								  512,
							  (u8)size * 512, 0);
				found = 1;
			}
		}
	}
	return found;
}

/* EOF */
