/*
 * cdimage.c
 * Layered data source for CD images in raw mode.
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
 * types
 */

typedef struct cdimage_source {
	SOURCE c;
	u8 off;
} CDIMAGE_SOURCE;

/*
 * helper functions
 */

static SOURCE *init_cdimage_source(SOURCE *foundation, u8 offset);
static int read_block_cdimage(SOURCE *s, u8 pos, void *buf);

/*
 * cd image detection
 */

static unsigned char syncbytes[12] = { 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0 };

int detect_cdimage(SECTION *section, int level)
{
	int mode, off;
	unsigned char *buf;
	SOURCE *s;

	if (get_buffer(section, 0, 2352, (void **)&buf) < 2352)
		return 0;

	/* check sync bytes as signature */
	if (memcmp(buf, syncbytes, 12) != 0)
		return 0;

	/* get mode of the track -- this determines sector layout */
	mode = buf[15];
	if (mode == 1) {
		/* standard data track */
		print_line(level, "Raw CD image, Mode 1");
		off = 16;
	} else if (mode == 2) {
		/* free-form track, assume XA form 1 */
		print_line(level, "Raw CD image, Mode 2, assuming Form 1");
		off = 24;
	} else
		return 0;

	/* create and analyze wrapped source */
	s = init_cdimage_source(section->source, section->pos + off);
	analyze_source(s, level);
	close_source(s);

	/* don't run other analyzers */
	stop_detect();
	return 1;
}

/*
 * initialize the cd image source
 */

static SOURCE *init_cdimage_source(SOURCE *foundation, u8 offset)
{
	CDIMAGE_SOURCE *src;

	src = (CDIMAGE_SOURCE *)malloc(sizeof(CDIMAGE_SOURCE));
	if (src == NULL)
		bailout("Out of memory");
	memset(src, 0, sizeof(CDIMAGE_SOURCE));

	if (foundation->size_known) {
		src->c.size_known = 1;
		src->c.size = ((foundation->size - offset + 304) / 2352) * 2048;
		/* TODO: pass the size in from the SECTION record and use it */
	}
	src->c.blocksize = 2048;
	src->c.foundation = foundation;
	src->c.read_block = read_block_cdimage;
	src->c.close = NULL;
	src->off = offset;

	return (SOURCE *)src;
}

/*
 * raw read
 */

static int read_block_cdimage(SOURCE *s, u8 pos, void *buf)
{
	SOURCE *fs = s->foundation;
	u8 filepos;

	/* translate position */
	filepos = (pos / 2048) * 2352 + ((CDIMAGE_SOURCE *)s)->off;

	/* read from lower layer */
	if (get_buffer_real(fs, filepos, 2048, buf, NULL) < 2048)
		return 0;

	return 1;
}

/* EOF */
