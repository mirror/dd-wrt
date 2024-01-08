/*
 * vpc.c
 * Layered data source for Virtual PC hard disk images.
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

typedef struct vhd_chunk {
	int present;
	u8 off;
	u1 bitmap[1];
} VHD_CHUNK;

typedef struct vhd_source {
	SOURCE c;
	u8 off;
	u4 chunk_size;
	u4 chunk_count;
	u4 *raw_map;
	VHD_CHUNK **chunks;
} VHD_SOURCE;

/*
 * helper functions
 */

static SOURCE *init_vhd_source(SECTION *section, int level, u8 total_size, u8 sparse_offset);
static int read_block_vhd(SOURCE *s, u8 pos, void *buf);
static void close_vhd(SOURCE *s);

/*
 * cd image detection
 */

int detect_vhd(SECTION *section, int level)
{
	unsigned char *buf;
	int found, type;
	u8 sparse_offset, total_size;
	char s[256];
	SOURCE *src;

	found = 0;

	/* check for info block at the beginning */
	if (get_buffer(section, 0, 511, (void **)&buf) < 511)
		return 0;
	if (memcmp(buf, "conectix", 8) == 0) {
		found = 1;
	}

	/* check for info block at the end if possible */
	if (!found && section->size > 1024 && !section->source->sequential) {
		if (get_buffer(section, section->size - 511, 511, (void **)&buf) < 511)
			return 0;
		if (memcmp(buf, "conectix", 8) == 0) {
			found = 1;
		}
	}

	if (!found)
		return 0;
	/* okay, now buf points to the info block, wherever it was found */

	type = get_be_long(buf + 0x3c);
	total_size = get_be_quad(buf + 0x28); /* copy at 0x30 ... ??? */

	if (type == 2) {
		print_line(level, "Connectix Virtual PC hard disk image, fixed size");
	} else if (type == 3) {
		print_line(level, "Connectix Virtual PC hard disk image, dynamic size");
	} else if (type == 4) {
		print_line(level, "Connectix Virtual PC hard disk image, differential");
	} else {
		print_line(level, "Connectix Virtual PC hard disk image, unknown type %d", type);
	}
	format_size_verbose(s, total_size);
	print_line(level + 1, "Disk size %s", s);

	if (type == 3) {
		/* dynamically sized, set up a mapping data source */
		sparse_offset = get_be_quad(buf + 16);

		src = init_vhd_source(section, level, total_size, sparse_offset);

		if (src != NULL) {
			/* analyze it */
			analyze_source(src, level);
			close_source(src);
		}
	}

	if (type == 3 || type == 4)
		stop_detect();
	return found;
}

/*
 * initialize the mapping source
 */

static SOURCE *init_vhd_source(SECTION *section, int level, u8 total_size, u8 sparse_offset)
{
	VHD_SOURCE *vs;
	unsigned char *buf;
	u8 map_offset;
	u4 map_size;
	char s[256];

	/* allocate and init source structure */
	vs = (VHD_SOURCE *)malloc(sizeof(VHD_SOURCE));
	if (vs == NULL)
		bailout("Out of memory");
	memset(vs, 0, sizeof(VHD_SOURCE));

	vs->c.size_known = 1;
	vs->c.size = total_size;
	vs->c.blocksize = 512;
	vs->c.foundation = section->source;
	vs->c.read_block = read_block_vhd;
	vs->c.close = close_vhd;
	vs->off = section->pos;

	/* read sparse information block */
	if (get_buffer(section, sparse_offset, 512, (void **)&buf) < 512) {
		print_line(level + 1, "Error reading the sparse image info block");
		goto errorexit;
	}
	map_offset = get_be_quad(buf + 16);
	vs->chunk_count = get_be_long(buf + 28);
	vs->chunk_size = get_be_long(buf + 32);

	format_size(s, vs->chunk_size);
	print_line(level + 1, "Dynamic sizing uses %lu chunks of %s", vs->chunk_count, s);

	if ((u8)vs->chunk_count * vs->chunk_size < total_size) {
		print_line(level + 1, "Error: Sparse parameters don't match total size");
		goto errorexit;
	}
	if (vs->chunk_size < 4096) {
		print_line(level + 1, "Error: Sparse chunk size too small (%lu bytes)", vs->chunk_size);
		goto errorexit;
	}
	if (vs->chunk_size > 2 * 1024 * 1024) {
		/* written-to bitmap wouldn't fit in one sector */
		print_line(level + 1, "Error: Sparse chunk size too large (%lu bytes)", vs->chunk_size);
		goto errorexit;
	}

	/* allocate further data structures */
	map_size = vs->chunk_count * 4;
	vs->raw_map = (u4 *)malloc(map_size);
	if (vs->raw_map == NULL)
		bailout("Out of memory");
	vs->chunks = (VHD_CHUNK **)malloc(vs->chunk_count * sizeof(VHD_CHUNK *));
	if (vs->chunks == NULL)
		bailout("Out of memory");
	memset(vs->chunks, 0, vs->chunk_count * sizeof(VHD_CHUNK *));

	/* read the chunk map */
	if (get_buffer_real(section->source, vs->off + map_offset, map_size, (void *)vs->raw_map, NULL) < map_size) {
		print_line(level + 1, "Error reading the sparse image map");
		goto errorexit;
	}

	return (SOURCE *)vs;

errorexit:
	close_vhd((SOURCE *)vs);
	free(vs);
	return NULL;
}

/*
 * mapping read
 */

static int read_block_vhd(SOURCE *s, u8 pos, void *buf)
{
	VHD_SOURCE *vs = (VHD_SOURCE *)s;
	SOURCE *fs = s->foundation;
	u4 chunk, chunk_start_sector, sector;
	u8 chunk_disk_off, sector_pos;
	unsigned char *filebuf;
	int present;

	chunk = (u4)(pos / vs->chunk_size);
	if (chunk >= vs->chunk_count)
		return 0;

	if (vs->chunks[chunk] == NULL) {
		/* create data structure for the chunk */

		chunk_start_sector = get_be_long(vs->raw_map + chunk);
		/* NOTE: raw_map is a u4*, so C does the arithmetics for us */

		if (chunk_start_sector == 0xffffffff) {
			present = 0;
		} else {
			chunk_disk_off = vs->off + (u8)chunk_start_sector * 512;
			if (get_buffer_real(fs, chunk_disk_off, 512, NULL, (void **)&filebuf) < 512)
				present = 0;
			else
				present = 1;
		}

		if (!present) {
			vs->chunks[chunk] = (VHD_CHUNK *)malloc(sizeof(VHD_CHUNK));
			if (vs->chunks[chunk] == NULL)
				bailout("Out of memory");
			vs->chunks[chunk]->present = 0;
		} else {
			vs->chunks[chunk] = (VHD_CHUNK *)malloc(sizeof(VHD_CHUNK) + 512);
			if (vs->chunks[chunk] == NULL)
				bailout("Out of memory");
			vs->chunks[chunk]->present = 1;
			vs->chunks[chunk]->off = chunk_disk_off + 512;
			memcpy(vs->chunks[chunk]->bitmap, filebuf, 512);
		}
	}

	if (!vs->chunks[chunk]->present) {
		/* whole chunk is missing */
		memset(buf, 0, 512);
		return 1;
	}

	sector = (u4)((pos - (u8)chunk * vs->chunk_size) / 512);
	if (vs->chunks[chunk]->bitmap[sector >> 3] & (128 >> (sector & 7))) {
		/* sector is present and in use */
		sector_pos = vs->chunks[chunk]->off + (u8)sector * 512;
		if (get_buffer_real(fs, sector_pos, 512, buf, NULL) < 512)
			return 0;
	} else {
		/* sector has not been written to (although it's present on disk) */
		memset(buf, 0, 512);
	}
	return 1;
}

/*
 * cleanup
 */

static void close_vhd(SOURCE *s)
{
	VHD_SOURCE *vs = (VHD_SOURCE *)s;
	u4 chunk;

	/* free raw chunk map */
	if (vs->raw_map != NULL)
		free(vs->raw_map);

	/* free chunk info data */
	if (vs->chunks != NULL) {
		for (chunk = 0; chunk < vs->chunk_count; chunk++) {
			if (vs->chunks[chunk] != NULL)
				free(vs->chunks[chunk]);
		}
		free(vs->chunks);
	}
}

/* EOF */
