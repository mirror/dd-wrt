/*
 * apple.c
 * Detection of Apple partition maps and file systems
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
 * Apple partition map detection
 */

void detect_apple_partmap(SECTION *section, int level)
{
  int i, magic, count;
  char s[256], append[64];
  unsigned char *buf;
  u8 start, size;

  /* partition maps only occur at the start of a device */
  /*  disabled to allow for APM maps in El Torito entries
  if (section->pos != 0)
    return;
  */

  /*
    if (buf[off] == 0x45 && buf[off+1] == 0x52) {
      print_line(level, "HFS driver description map at sector %d", i);
    }
  */

  if (get_buffer(section, 512, 512, (void **)&buf) < 512)
    return;

  magic = get_be_short(buf);
  if (magic == 0x5453) {
    print_line(level, "Old-style Apple partition map");
    return;
  }
  if (magic != 0x504D)
    return;

  /* get partition count and print info */
  count = get_be_long(buf + 4);
  print_line(level, "Apple partition map, %d entries", count);

  for (i = 1; i <= count; i++) {
    /* read the right sector */
    /* NOTE: the previous run through the loop might have called
     *  get_buffer indirectly, invalidating our old pointer */
    if (i > 1 && get_buffer(section, i * 512, 512, (void **)&buf) < 512)
      return;

    /* check signature */
    if (get_be_short(buf) != 0x504D) {
      print_line(level, "Partition %d: invalid signature, skipping", i);
      continue;
    }

    /* get position and size */
    start = get_be_long(buf + 8);
    size = get_be_long(buf + 12);
    sprintf(append, " from %llu", start);
    format_blocky_size(s, size, 512, "sectors", append);
    print_line(level, "Partition %d: %s",
	       i, s);

    /* get type */
    get_string(buf + 48, 32, s);
    print_line(level+1, "Type \"%s\"", s);

    /* recurse for content detection */
    if (start > count && size > 0) {  /* avoid recursion on self */
      analyze_recursive(section, level + 1,
			start * 512, size * 512, 0);
    }
  }
}

/*
 * Apple volume formats: MFS, HFS, HFS Plus
 */

void detect_apple_volume(SECTION *section, int level)
{
  char s[256], t[514];
  unsigned char *buf;
  u2 magic, version, volnamelen;
  u4 blocksize, blockstart;
  u8 blockcount, offset;
  u8 catalogstart, cataloglength;
  u4 firstleafnode, nodesize;

  if (get_buffer(section, 1024, 512, (void **)&buf) < 512)
    return;

  magic = get_be_short(buf);
  version = get_be_short(buf + 2);

  if (magic == 0xD2D7) {
    print_line(level, "MFS file system");

  } else if (magic == 0x4244) {
    print_line(level, "HFS file system");
    blockcount = get_be_short(buf + 18);
    blocksize = get_be_long(buf + 20);
    blockstart = get_be_short(buf + 28);

    get_pstring(buf + 36, s);
    format_ascii(s, t);
    print_line(level + 1, "Volume name \"%s\"", t);

    format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
    print_line(level + 1, "Volume size %s", s);

    if (get_be_short(buf + 0x7c) == 0x482B) {
      print_line(level, "HFS wrapper for HFS Plus");

      offset = (u8)get_be_short(buf + 0x7e) * blocksize +
	(u8)blockstart * 512;
      /* TODO: size */

      analyze_recursive(section, level + 1,
			offset, 0, 0);
    }

  } else if (magic == 0x482B) {
    print_line(level, "HFS Plus file system");

    blocksize = get_be_long(buf + 40);
    blockcount = get_be_long(buf + 44);

    format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
    print_line(level + 1, "Volume size %s", s);

    /* To read the volume name, we have to parse some structures...
       This code makes many assumptions which are usually true,
       but don't have to be. */

    /* get catalog file location on disk */
    /* ASSUMPTION: This reads the location of the first extent
       of the catalog file. If the catalog file is fragmented, we'll
       be working with only the first fragment, which may not include
       the node we're looking for. */
    catalogstart = get_be_long(buf + 288) * blocksize;
    cataloglength = get_be_long(buf + 292) * blocksize;
    /* limit to actual length (byte count instead of block count) */
    if (cataloglength > get_be_quad(buf + 272))
      cataloglength = get_be_quad(buf + 272);

    /* read header node of B-tree (4096 is the minimum node size) */
    if (get_buffer(section, catalogstart, 4096, (void **)&buf) < 4096)
      return;
    firstleafnode = get_be_long(buf + 24);
    nodesize = get_be_short(buf + 32);
    if (nodesize < 4096)
      return;  /* illegal value */

    /* read first lead node */
    if ((firstleafnode + 1) * nodesize > cataloglength)
      return;  /* the location is beyond the end of the catalog */
    if (get_buffer(section, catalogstart + firstleafnode * nodesize,
		   nodesize, (void **)&buf) < nodesize)
      return;

    /* the first record in this leaf node should be for parent id 1 */
    if (buf[8] != 0xff)
      return;  /* not a leaf node */
    if (get_be_short(buf + 14) <= 6)
      return;  /* key of first record is too short to contain a name */
    if (get_be_long(buf + 16) != 1)
      return;  /* parent folder id is not "root parent" */
    volnamelen = get_be_short(buf + 20);
    format_utf16_be(buf + 22, volnamelen * 2, t);
    print_line(level + 1, "Volume name \"%s\"", t);
  }
}

/*
 * Apple UDIF disk images
 */

void detect_udif(SECTION *section, int level)
{
  u8 pos;
  unsigned char *buf;

  if (section->size < 1024 || section->source->sequential)
    return;

  pos = section->size - 512;
  if (get_buffer(section, pos, 512, (void **)&buf) < 512)
    return;

  if (memcmp(buf, "koly", 4) == 0) {
    print_line(level, "Apple UDIF disk image, content detection may or may not work...");
  }
}

/* EOF */
