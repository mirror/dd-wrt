/*
 * beos.c
 * Detection of BeOS file systems
 *
 * Copyright (c) 2003-2006 Christoph Pfisterer
 * Based on a contribution by Shadowcaster
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
 * BeOS BFS (BeFS) file system
 */

int detect_bfs(SECTION *section, int level)
{
	unsigned char *buf;
	char s[256];
	int off, en;
	u4 blocksize;
	u8 blockcount;

	for (off = 0; off <= 512; off += 512) {
		if (get_buffer(section, off, 512, (void **)&buf) < 512)
			continue;

		for (en = 0; en < 2; en++) {
			if (get_ve_long(en, buf + 32) == 0x42465331 && /* magic 1 */
			    get_ve_long(en, buf + 36) == 0x42494745 && /* endianness */
			    get_ve_long(en, buf + 68) == 0xdd121031 && /* magic 2 */
			    get_ve_long(en, buf + 112) == 0x15b6830e) { /* magic 3 */

				print_line(level, "BeOS BFS (BeFS) file system, %s placement, %s", (off == 0) ? "Apple" : "Intel",
					   get_ve_name(en));

				/* get label */
				get_string(buf, 32, s);
				if (s[0])
					print_line(level + 1, "Volume name \"%s\"", s);

				/* get size */
				blocksize = get_ve_long(en, buf + 40);
				blockcount = get_ve_quad(en, buf + 48);
				/* possible further consistency checks: 
				   blocksize must be in ( 512, 1024, 2048, 4096 )
				   long @ 44 is shift number for the block size, i.e.
				   blocksize == 1 << get_ve_long(en, buf + 44)
				 */

				format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
				print_line(level + 1, "Volume size %s", s);

				return 1;
			}
		}
	}
	return 0;
}

/*
 * BeOS boot loader
 */

int detect_beos_loader(SECTION *section, int level)
{
	unsigned char *buf;

	if (section->flags & FLAG_IN_DISKLABEL)
		return 0;

	if (get_buffer(section, 0, 512, (void **)&buf) < 512)
		return 0;

	if (find_memory(buf, 512, "Be Boot Loader", 14) >= 0) {
		print_line(level, "BeOS boot loader");
		return 1;
	}
	if (find_memory(buf, 512, "yT Boot Loader", 14) >= 0) {
		print_line(level, "ZETA/yellowTab boot loader");
		return 1;
	}
	if (find_memory(buf, 512,
			"\x04"
			"beos\x06"
			"system\x05"
			"zbeos",
			18) >= 0) {
		print_line(level, "Haiku boot loader");
		return 1;
	}
	return 0;
}

/* EOF */
