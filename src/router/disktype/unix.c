/*
 * unix.c
 * Detection of general Unix file systems
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
 * JFS (for Linux)
 */

void detect_jfs(SECTION *section, int level)
{
  unsigned char *buf;
  int version;
  char s[256];
  u4 blocksize;
  u8 blockcount;

  if (get_buffer(section, 32768, 512, (void **)&buf) < 512)
    return;

  /* check signature */
  if (memcmp(buf, "JFS1", 4) != 0)
    return;

  /* tell the user */
  version = get_le_long(buf + 4);
  print_line(level, "JFS file system, version %d", version);

  get_string(buf + 101, 11, s);
  print_line(level + 1, "Volume name \"%s\"", s);

  blocksize = get_le_long(buf + 24);
  blockcount = get_le_quad(buf + 8);
  format_blocky_size(s, blockcount, blocksize, "h/w blocks", NULL);
  print_line(level + 1, "Volume size %s", s);
}

/*
 * XFS
 */

void detect_xfs(SECTION *section, int level)
{
  unsigned char *buf;
  u4 raw_version, blocksize;
  u8 blockcount;
  int version;
  char s[256];

  if (get_buffer(section, 0, 512, (void **)&buf) < 512)
    return;

  /* check signature */
  if (get_be_long(buf) != 0x58465342)
    return;

  /* tell the user */
  raw_version = get_be_short(buf + 0x64);
  version = raw_version & 0x000f;
  print_line(level, "XFS file system, version %d", version);

  get_string(buf + 0x6c, 12, s);
  print_line(level + 1, "Volume name \"%s\"", s);

  format_uuid(buf + 32, s);
  print_line(level + 1, "UUID %s", s);

  blocksize = get_be_long(buf + 4);
  blockcount = get_be_quad(buf + 8);
  format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
  print_line(level + 1, "Volume size %s", s);
}

/*
 * UFS file system from various BSD strains
 */

void detect_ufs(SECTION *section, int level)
{
  unsigned char *buf;
  int i, at, en, namelen, offsets[5] = { 0, 8, 64, 256, -1 };
  u4 magic;
  char s[256];

  /* NextStep/OpenStep apparently can move the superblock further into
     the device. Linux uses steps of 8 blocks (of the applicable block
     size) to scan for it. We only scan at the canonical location for now. */
  /* Possible block sizes: 512 (old formats), 1024 (most), 2048 (CD media) */

  for (i = 0; offsets[i] >= 0; i++) {
    at = offsets[i];
    if (get_buffer(section, at * 1024, 1536, (void **)&buf) < 1536)
      break;

    for (en = 0; en < 2; en++) {
      magic = get_ve_long(en, buf + 1372);

      if (magic == 0x00011954) {
	print_line(level, "UFS file system, %d KiB offset, %s",
		   at, get_ve_name(en));
      } else if (magic == 0x00095014) {
	print_line(level, "UFS file system, %d KiB offset, long file names, %s",
		   at, get_ve_name(en));
      } else if (magic == 0x00195612) {
	print_line(level, "UFS file system, %d KiB offset, fs_featurebits, %s",
		   at, get_ve_name(en));
      } else if (magic == 0x05231994) {
	print_line(level, "UFS file system, %d KiB offset, fs_featurebits, >4GB support, %s",
		   at, get_ve_name(en));
      } else if (magic == 0x19540119) {
	print_line(level, "UFS2 file system, %d KiB offset, %s",
		   at, get_ve_name(en));
      } else
	continue;

      /* volume name by FreeBSD convention */
      get_string(buf + 680, 32, s);
      if (s[0])
	print_line(level + 1, "Volume name \"%s\" (in superblock)", s);

      /* last mount point */
      get_string(buf + 212, 255, s);  /* actually longer, but varies */
      if (s[0])
	print_line(level + 1, "Last mounted at \"%s\"", s);

      /* volume name by Darwin convention */
      if (get_buffer(section, 7 * 1024, 1024, (void **)&buf) == 1024) {
	if (get_ve_long(en, buf) == 0x4c41424c &&  /* "LABL" */
	    get_ve_long(en, buf + 8) == 1) {       /* version 1 */
	  namelen = get_ve_short(en, buf + 16);
	  get_string(buf + 18, namelen, s);  /* automatically limits to 255 */
	  print_line(level + 1, "Volume name \"%s\" (in label v%lu)",
		     s, get_ve_long(en, buf + 8));
	}
      }

      return;
    }
  }
}

/*
 * System V file system
 */

void detect_sysv(SECTION *section, int level)
{
  unsigned char *buf;
  int i, at, en, offsets[5] = { 512, 1024, -1 };
  s4 blocksize_code;
  char s[256];

  for (i = 0; offsets[i] >= 0; i++) {
    at = offsets[i];
    if (get_buffer(section, at, 1024, (void **)&buf) < 1024)
      break;

    for (en = 0; en < 2; en++) {
      if (get_ve_long(en, buf + 1016) == 0x2b5544) {
	blocksize_code = get_ve_long(en, buf + 1020);
	s[0] = 0;
	if (blocksize_code == 1)
	  strcpy(s, "512 byte blocks");
	else if (blocksize_code == 2)
	  strcpy(s, "1 KiB blocks");
	else if (blocksize_code == 3)
	  strcpy(s, "2 KiB blocks");
	else
	  snprintf(s, 255, "unknown block size code %d", (int)blocksize_code);

	print_line(level, "XENIX file system (SysV variant), %s, %s",
		   get_ve_name(en), s);
	return;
      }

      if (get_ve_long(en, buf + 504) == 0xfd187e20) {
	blocksize_code = get_ve_long(en, buf + 508);
	s[0] = 0;
	if (blocksize_code == 1)
	  strcpy(s, "512 byte blocks");
	else if (blocksize_code == 2)
	  strcpy(s, "1 KiB blocks");
	else
	  snprintf(s, 255, "unknown block size code %d", (int)blocksize_code);

	print_line(level, "SysV file system, %s, %s",
		   get_ve_name(en), s);
	return;
      }
    }
  }
}

/*
 * BSD disklabel
 */

static char * bsdtype_names[] = {
  "Unused",
  "swap",
  "Sixth Edition",
  "Seventh Edition",
  "System V",
  "V7 with 1 KiB blocks",
  "Eighth Edition, 4 KiB blocks",
  "4.2BSD fast file system",
  "ext2 or MS-DOS",
  "4.4BSD log-structured file system",
  "\"Other\"",
  "HPFS",
  "ISO9660",
  "bootstrap",
  "AmigaDOS fast file system",
  "Macintosh HFS",
  "Digital Unix AdvFS",
};

static char * get_name_for_bsdtype(int type)
{
  if (type >= 0 && type <= 16)
    return bsdtype_names[type];
  return "Unknown";
}

void detect_bsd_disklabel(SECTION *section, int level)
{
  unsigned char *buf;
  int i, off, partcount, types[16], min_offset_valid, did_recurse;
  u4 starts[16], sizes[16];
  u4 sectsize, nsectors, ntracks, ncylinders, secpercyl, secperunit;
  u8 offset, min_offset, base_offset;
  char s[256], append[64], pn;

  if (section->flags & FLAG_IN_DISKLABEL)
    return;

  if (get_buffer(section, 512, 512, (void **)&buf) < 512)
    return;

  if (get_le_long(buf)       != 0x82564557 ||
      get_le_long(buf + 132) != 0x82564557)
    return;

  sectsize = get_le_long(buf + 40);
  nsectors = get_le_long(buf + 44);
  ntracks = get_le_long(buf + 48);
  ncylinders = get_le_long(buf + 52);
  secpercyl = get_le_long(buf + 56);
  secperunit = get_le_long(buf + 60);

  partcount = get_le_short(buf + 138);

  if (partcount <= 8) {
    print_line(level, "BSD disklabel (at sector 1), %d partitions", partcount);
  } else if (partcount > 8 && partcount <= 16) {
    print_line(level, "BSD disklabel (at sector 1), %d partitions (more than usual, but valid)",
	       partcount);
  } else if (partcount > 16) {
    print_line(level, "BSD disklabel (at sector 1), %d partitions (broken, limiting to 16)",
	       partcount);
    partcount = 16;
  }
  if (sectsize != 512) {
    print_line(level + 1, "Unusual sector size %d bytes, your mileage may vary");
  }

  min_offset = 0;
  min_offset_valid = 0;
  for (i = 0, off = 148; i < partcount; i++, off += 16) {
    starts[i] = get_le_long(buf + off + 4);
    sizes[i] = get_le_long(buf + off);
    types[i] = buf[off + 12];

    if (types[i] != 0 || i == 2) {
      offset = (u8)starts[i] * 512;
      if (!min_offset_valid || offset < min_offset) {
	min_offset = offset;
	min_offset_valid = 1;
      }
    }
  }
  /* if min_offset_valid is still 0, the default of min_offset=0 is okay */

  if (section->pos == min_offset) {
    /* either min_offset is zero, or we were analyzing the whole disk */
    base_offset = section->pos;
  } else if (section->pos == 0) {
    /* are we analyzing the slice alone? */
    print_line(level + 1, "Adjusting offsets for disklabel in a DOS partition at sector %llu", min_offset >> 9);
    base_offset = min_offset;
  } else if (min_offset == 0) {
    /* assume relative offsets after all */
    base_offset = 0;
  } else {
    print_line(level + 1, "Warning: Unable to adjust offsets, your mileage may vary");
    base_offset = section->pos;
  }

  /* loop over partitions: print and analyze */
  did_recurse = 0;
  for (i = 0; i < partcount; i++) {
    pn = 'a' + i;
    if (types[i] == 0 && i != 2)
      continue;

    sprintf(append, " from %lu", starts[i]);
    format_blocky_size(s, sizes[i], 512, "sectors", append);
    print_line(level, "Partition %c: %s",
	       pn, s);

    print_line(level + 1, "Type %d (%s)",
	       types[i], get_name_for_bsdtype(types[i]));

    if (types[i] == 0 || sizes[i] == 0)
      continue;

    offset = (u8)starts[i] * 512;
    if (offset < base_offset) {
      print_line(level + 1, "(Illegal start offset, no detection)");
    } else if (offset == base_offset) {
      print_line(level + 1, "Includes the disklabel and boot code");

      /* recurse for content detection, but carefully */
      analyze_recursive(section, level + 1,
			offset - base_offset, (u8)sizes[i] * 512,
			FLAG_IN_DISKLABEL);
      did_recurse = 1;
    } else {
      /* recurse for content detection */
      analyze_recursive(section, level + 1,
			offset - base_offset, (u8)sizes[i] * 512,
			0);
    }
  }

  if (did_recurse)
    stop_detect();  /* don't run other detectors; we already did that
		       for an overlapping partition. */
}

/*
 * FreeBSD boot loader (?)
 */

void detect_bsd_loader(SECTION *section, int level)
{
  unsigned char *buf;

  if (section->flags & FLAG_IN_DISKLABEL)
    return;

  if (get_buffer(section, 0, 512, (void **)&buf) == 512) {
    if (get_le_short(buf + 0x1b0) == 0xbb66) {
      print_line(level, "FreeBSD boot manager (i386 boot0 at sector 0)");
    } else if (get_le_long(buf + 0x1f6) == 0 &&
	       get_le_long(buf + 0x1fa) == 50000 &&
	       get_le_short(buf + 0x1fe) == 0xaa55) {
      print_line(level, "FreeBSD boot loader (i386 boot1 at sector 0)");
    }
  }

  if (get_buffer(section, 1024, 512, (void **)&buf) == 512) {
    if (memcmp(buf + 2, "BTX", 3) == 0) {
      print_line(level, "FreeBSD boot loader (i386 boot2/BTX %d.%02d at sector 2)",
		 (int)buf[5], (int)buf[6]);
    }
  }
}

/*
 * Solaris SPARC disklabel
 */

void detect_solaris_disklabel(SECTION *section, int level)
{
  unsigned char *buf;
  int i, off1, off2, types[8], did_recurse;
  u4 sizes[8];
  u8 starts[8], cylsize, offset;
  char s[256], append[256], pn;

  if (section->flags & FLAG_IN_DISKLABEL)
    return;

  if (get_buffer(section, 0, 512, (void **)&buf) < 512)
    return;

  if (get_be_short(buf + 508) != 0xDABE)
    return;

  print_line(level, "Solaris SPARC disklabel");

  cylsize = (u8)get_be_short(buf + 436) * (u8)get_be_short(buf + 438);
  for (i = 0, off1 = 142, off2 = 444; i < 8; i++, off1 += 4, off2 += 8) {
    types[i] = get_be_short(buf + off1);
    starts[i] = get_be_long(buf + off2) * cylsize;
    sizes[i] = get_be_long(buf + off2 + 4);
  }

  /* loop over partitions: print and analyze */
  did_recurse = 0;
  for (i = 0; i < 8; i++) {
    pn = '0' + i;
    if (sizes[i] == 0)
      continue;

    sprintf(append, " from %llu", starts[i]);
    format_blocky_size(s, sizes[i], 512, "sectors", append);
    print_line(level, "Partition %c: %s", pn, s);

    print_line(level + 1, "Type %d",
               types[i]);

    offset = starts[i] * 512;
    if (offset == 0) {
      print_line(level + 1, "Includes the disklabel");

      /* recurse for content detection, but carefully */
      analyze_recursive(section, level + 1,
                        offset, (u8)sizes[i] * 512,
                        FLAG_IN_DISKLABEL);
      did_recurse = 1;
    } else {
      /* recurse for content detection */
      analyze_recursive(section, level + 1,
                        offset, (u8)sizes[i] * 512,
                        0);
    }
  }

  if (did_recurse)
    stop_detect();  /* don't run other detectors; we already did that
                       for the first partition, which overlaps with
                       the disklabel itself. */
}

/*
 * Solaris x86 vtoc
 */

static char * vtoctype_names[] = {
  "Unused",
  "Boot",
  "Root",
  "Swap",
  "Usr",
  "Overlap",
  "Stand",
  "Var",
  "Home",
  "Alternate sector",
  "Cache"
};

static char * get_name_for_vtoctype(int type)
{
  if (type >= 0 && type <= 10)
    return vtoctype_names[type];
  return "Unknown";
}

void detect_solaris_vtoc(SECTION *section, int level)
{
  unsigned char *buf;
  int i, off, partcount, sectorsize, types[16], did_recurse;
  u4 starts[16], sizes[16];
  u4 version;
  u8 offset;
  char s[256], append[64];

  if (section->flags & FLAG_IN_DISKLABEL)
    return;

  if (get_buffer(section, 512, 512, (void **)&buf) < 512)
    return;

  if (get_le_long(buf + 12) != 0x600DDEEE)
    return;
  version = get_le_long(buf + 16);
  if (version != 1) {
    print_line(level, "Solaris x86 disklabel, unknown version %lu", version);
    return;
  }
  partcount = get_le_short(buf + 30);
  if (partcount > 16) {
    print_line(level, "Solaris x86 disklabel, version 1, %d partitions (limiting to 16)",
	       partcount);
    partcount = 16;
  } else {
    print_line(level, "Solaris x86 disklabel, version 1, %d partitions",
	       partcount);
  }

  sectorsize = get_le_short(buf + 28);
  if (sectorsize != 512)
    print_line(level + 1, "Unusual sector size %d bytes, your mileage may vary",
	       sectorsize);

  get_string(buf + 20, 8, s);
  if (s[0])
    print_line(level + 1, "Volume name \"%s\"", s);

  for (i = 0, off = 72; i < partcount; i++, off += 12) {
    types[i] = get_le_short(buf + off);
    starts[i] = get_le_long(buf + off + 4);
    sizes[i] = get_le_long(buf + off + 8);
  }

  /* loop over partitions: print and analyze */
  did_recurse = 0;
  for (i = 0; i < partcount; i++) {
    if (sizes[i] == 0)
      continue;

    sprintf(append, " from %lu", starts[i]);
    format_blocky_size(s, sizes[i], 512, "sectors", append);
    print_line(level, "Partition %d: %s",
	       i, s);

    print_line(level + 1, "Type %d (%s)",
	       types[i], get_name_for_vtoctype(types[i]));

    offset = (u8)starts[i] * 512;
    if (offset == 0) {
      print_line(level + 1, "Includes the disklabel");

      /* recurse for content detection, but carefully */
      analyze_recursive(section, level + 1,
			offset, (u8)sizes[i] * 512,
			FLAG_IN_DISKLABEL);
      did_recurse = 1;
    } else {
      /* recurse for content detection */
      analyze_recursive(section, level + 1,
			offset, (u8)sizes[i] * 512,
			0);
    }
  }

  if (did_recurse)
    stop_detect();  /* don't run other detectors; we already did that
		       for an overlapping partition. */
}

/*
 * QNX4 file system
 */

void detect_qnx(SECTION *section, int level)
{
  unsigned char *buf;

  if (get_buffer(section, 512, 512, (void **)&buf) < 512)
    return;

  /* check signature */
  if (get_le_long(buf) != 0x0000002f)
    return;
  /* NOTE: This is actually the string "/", the file name of the root
     directory. QNX4 fs does not have a real superblock, just an
     aggregate of 4 inodes for certain special files. */

  /* tell the user */
  print_line(level, "QNX4 file system");
}

/*
 * Veritas VxFS
 */

void detect_vxfs(SECTION *section, int level)
{
  unsigned char *buf;
  int en, version;
  u4 blocksize, blockcount;
  char s[256];

  if (get_buffer(section, 1024, 1024, (void **)&buf) < 1024)
    return;

  /* check signature */
  for (en = 0; en < 2; en++) {
    if (get_ve_long(en, buf) == 0xA501FCF5) {
      version = get_ve_long(en, buf + 4);
      print_line(level, "Veritas VxFS file system, version %d, %s",
		 version, get_ve_name(en));

      blocksize = get_ve_long(en, buf + 32);
      blockcount = get_ve_long(en, buf + 36);
      format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
      print_line(level + 1, "Volume size %s", s);
    }
  }
}

/* EOF */
