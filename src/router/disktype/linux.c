/*
 * linux.c
 * Detection of Linux file systems and boot codes
 *
 * Copyright (c) 2003-2006 Christoph Pfisterer
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
 * ext2/ext3 file system
 */

void detect_ext23(SECTION *section, int level)
{
  unsigned char *buf;
  char s[256];
  u4 blocksize;
  u8 blockcount;

  if (get_buffer(section, 1024, 1024, (void **)&buf) < 1024)
    return;

  if (get_le_short(buf + 56) == 0xEF53) {
    if (get_le_long(buf + 96) & 0x0008)       /* JOURNAL_DEV flag */
      print_line(level, "Ext3 external journal");
    else if (get_le_long(buf + 92) & 0x0004)  /* HAS_JOURNAL flag */
      print_line(level, "Ext3 file system");
    else
      print_line(level, "Ext2 file system");

    get_string(buf + 120, 16, s);
    if (s[0])
      print_line(level + 1, "Volume name \"%s\"", s);

    format_uuid(buf + 104, s);
    print_line(level + 1, "UUID %s", s);

    get_string(buf + 136, 64, s);
    if (s[0])
      print_line(level + 1, "Last mounted at \"%s\"", s);

    blocksize = 1024 << get_le_long(buf + 24);
    blockcount = get_le_long(buf + 4);
    format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
    print_line(level + 1, "Volume size %s", s);

    /* 76 4 s_rev_level */
    /* 62 2 s_minor_rev_level */
    /* 72 4 s_creator_os */
    /* 92 3x4 s_feature_compat, s_feature_incompat, s_feature_ro_compat */
  }
}

/*
 * ReiserFS file system
 */

void detect_reiser(SECTION *section, int level)
{
  unsigned char *buf;
  int i, at, newformat;
  int offsets[3] = { 8, 64, -1 };
  char s[256];
  u8 blockcount;
  u4 blocksize;

  for (i = 0; offsets[i] >= 0; i++) {
    at = offsets[i];
    if (get_buffer(section, at * 1024, 1024, (void **)&buf) < 1024)
      continue;

    /* check signature */
    if (memcmp(buf + 52, "ReIsErFs", 8) == 0) {
      print_line(level, "ReiserFS file system (old 3.5 format, standard journal, starts at %d KiB)", at);
      newformat = 0;
    } else if (memcmp(buf + 52, "ReIsEr2Fs", 9) == 0) {
      print_line(level, "ReiserFS file system (new 3.6 format, standard journal, starts at %d KiB)", at);
      newformat = 1;
    } else if (memcmp(buf + 52, "ReIsEr3Fs", 9) == 0) {
      newformat = get_le_short(buf + 72);
      if (newformat == 0) {
	print_line(level, "ReiserFS file system (old 3.5 format, non-standard journal, starts at %d KiB)", at);
      } else if (newformat == 2) {
	print_line(level, "ReiserFS file system (new 3.6 format, non-standard journal, starts at %d KiB)", at);
	newformat = 1;
      } else {
	print_line(level, "ReiserFS file system (v3 magic, but unknown version %d, starts at %d KiB)", newformat, at);
	continue;
      }
    } else
      continue;

    /* get data */
    blockcount = get_le_long(buf);
    blocksize = get_le_short(buf + 44);
    /* for new format only:
       hashtype = get_le_long(buf + 64);
    */

    /* get label */
    get_string(buf + 100, 16, s);
    if (s[0])
      print_line(level + 1, "Volume name \"%s\"", s);

    format_uuid(buf + 84, s);
    print_line(level + 1, "UUID %s", s);

    /* print size */
    format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
    print_line(level + 1, "Volume size %s", s);

    /* TODO: print hash code */
  }
}

/*
 * Reiser4 file system
 */

void detect_reiser4(SECTION *section, int level)
{
  unsigned char *buf;
  char s[256];
  int layout_id;
  char layout_name[64];
  u4 blocksize;
  u8 blockcount;

  if (get_buffer(section, 16 * 4096, 1024, (void **)&buf) < 1024)
    return;

  /* check signature */
  if (memcmp(buf, "ReIsEr4", 7) != 0)
    return;

  /* get data from master superblock */
  layout_id = get_le_short(buf + 16);
  blocksize = get_le_short(buf + 18);
  if (layout_id == 0)
    strcpy(layout_name, "4.0 layout");
  else
    sprintf(layout_name, "Unknown layout with ID %d", layout_id);

  format_size(s, blocksize);
  print_line(level, "Reiser4 file system (%s, block size %s)",
	     layout_name, s);

  /* get label and UUID */
  get_string(buf + 36, 16, s);
  if (s[0])
    print_line(level + 1, "Volume name \"%s\"", s);

  format_uuid(buf + 20, s);
  print_line(level + 1, "UUID %s", s);

  if (layout_id == 0) {
    /* read 4.0 superblock */
    if (get_buffer(section, 17 * 4096, 1024, (void **)&buf) < 1024)
      return;
    if (memcmp(buf + 52, "ReIsEr40FoRmAt", 14) != 0) {
      print_line(level + 1, "Superblock for 4.0 format missing");
      return;
    }

    blockcount = get_le_quad(buf);
    format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
    print_line(level + 1, "Volume size %s", s);
  }
}

/*
 * Linux RAID persistent superblock
 */

static char *levels[] = {
  "Multipath",
  "\'HSM\'",
  "\'translucent\'",
  "Linear",
  "RAID0",
  "RAID1",
  NULL, NULL,
  "RAID4(?)",
  "RAID5"
};

void detect_linux_raid(SECTION *section, int level)
{
  unsigned char *buf;
  u8 pos;
  int rlevel, nr_disks, raid_disks, spare;
  u1 uuid[16];
  char s[256];

  /* don't do this if:
   *  - the size is unknown (0)
   *  - the size is too small for the calculation
   *  - it is inefficient to read from the end of the source
   */
  if (section->size < 65536 || section->source->sequential)
    return;

  /* get RAID superblock from the end of the device */
  pos = (section->size & ~65535) - 65536;
  if (get_buffer(section, pos, 4096, (void **)&buf) < 4096)
    return;

  /* signature */
  if (get_le_long(buf) != 0xa92b4efc)
    return;

  print_line(level, "Linux RAID disk, version %lu.%lu.%lu",
	     get_le_long(buf + 4), get_le_long(buf + 8),
	     get_le_long(buf + 12));

  /* get some data */
  rlevel = (int)(long)get_le_long(buf + 28);   /* is signed, actually */
  nr_disks = get_le_long(buf + 36);
  raid_disks = get_le_long(buf + 40);
  spare = nr_disks - raid_disks;

  /* find the name for the personality in the table */
  if (rlevel < -4 || rlevel > 5 || levels[rlevel+4] == NULL) {
    print_line(level + 1, "Unknown RAID level %d using %d regular %d spare disks",
	       rlevel, raid_disks, spare);
  } else {
    print_line(level + 1, "%s set using %d regular %d spare disks",
	       levels[rlevel+4], raid_disks, spare);
  }

  /* get the UUID */
  memcpy(uuid, buf + 5*4, 4);
  memcpy(uuid + 4, buf + 13*4, 3*4);
  format_uuid(uuid, s);
  print_line(level + 1, "RAID set UUID %s", s);
}

/*
 * Linux LVM1
 */

void detect_linux_lvm(SECTION *section, int level)
{
  unsigned char *buf;
  char s[256];
  int minor_version, pv_number;
  u8 pe_size, pe_count, pe_start;

  if (get_buffer(section, 0, 1024, (void **)&buf) < 1024)
    return;

  /* signature */
  if (buf[0] != 'H' || buf[1] != 'M')
    return;
  /* helpful sanity check... */
  if (get_le_long(buf + 36) == 0 || get_le_long(buf + 40) == 0)
    return;

  minor_version = get_le_short(buf + 2);
  print_line(level, "Linux LVM1 volume, version %d%s",
	     minor_version,
	     (minor_version < 1 || minor_version > 2) ? " (unknown)" : "");

  /* volume group name */
  get_string(buf + 172, 128, s);
  print_line(level + 1, "Volume group name \"%s\"", s);

  /* "UUID" of this physical volume */
  format_uuid_lvm(buf + 0x2c, s);
  print_line(level + 1, "PV UUID %s", s);

  /* number of this physical volume */
  pv_number = get_le_long(buf + 432);
  print_line(level + 1, "PV number %d", pv_number);

  /* volume size */
  pe_size = get_le_long(buf + 452);
  pe_count = get_le_long(buf + 456);
  format_blocky_size(s, pe_count, pe_size * 512, "PEs", NULL);
  print_line(level + 1, "Useable size %s", s);

  /* get start of first PE */
  if (minor_version == 1) {
    /* minor format 1: first PE starts after the declared length of the PE tables */
    pe_start = get_le_long(buf + 36) + get_le_long(buf + 40);
  } else if (minor_version == 2) {
    /* minor format 2: a field in the header indicates this */
    pe_start = get_le_long(buf + 464) << 9;
  } else {
    /* unknown minor format */
    pe_start = 0;
  }

  /* try to detect from first PE */
  if (pe_start > 0) {
    analyze_recursive(section, level + 1,
		      pe_start, 0, 0);
    /* TODO: elaborate on this by reading the PE allocation map */
  }
}

/*
 * Linux LVM2
 */

void detect_linux_lvm2(SECTION *section, int level)
{
  unsigned char *buf;
  int at, i;
  char s[256];
  u8 labelsector;
  u4 labeloffset;
  u8 pvsize, mdoffset, mdsize;
  int mda_version;

  for (at = 0; at < 4; at++) {
    if (get_buffer(section, at * 512, 512, (void **)&buf) < 512)
      continue;

    /* check signature */
    if (memcmp(buf, "LABELONE", 8) != 0)
      continue;

    labelsector = get_le_quad(buf + 8);
    labeloffset = get_le_long(buf + 20);

    if (memcmp(buf + 24, "LVM2 001", 8) != 0) {
      get_string(buf + 24, 8, s);
      print_line(level, "LABELONE label at sector %d, unknown type \"%s\"",
		 at, s);
      return;
    }

    print_line(level, "Linux LVM2 volume, version 001");
    print_line(level + 1, "LABELONE label at sector %d",
	       at);

    if (labeloffset >= 512 || labelsector > 256 ||
	labelsector != at) {
      print_line(level + 1, "LABELONE data inconsistent, aborting analysis");
      return;
    }

    /* "UUID" of this physical volume */
    format_uuid_lvm(buf + labeloffset, s);
    print_line(level + 1, "PV UUID %s", s);

    /* raw volume size */
    pvsize = get_le_quad(buf + labeloffset + 32);
    format_size_verbose(s, pvsize);
    print_line(level + 1, "Volume size %s", s);

    /* find first metadata area in list */
    mdoffset = 0;
    for (i = 0; i < 16; i++)
      if (get_le_quad(buf + labeloffset + 40 + i * 16) == 0) {
	i++;
	mdoffset = get_le_quad(buf + labeloffset + 40 + i * 16);
	mdsize = get_le_quad(buf + labeloffset + 40 + i * 16 + 8);
	break;
      }
    if (mdoffset == 0)
      return;

    if (get_buffer(section, mdoffset, mdsize, (void **)&buf) < mdsize)
      return;

    if (memcmp(buf + 4, " LVM2 x[5A%r0N*>", 16) != 0)
      return;
    mda_version = get_le_long(buf + 20);

    print_line(level + 1, "Meta-data version %d", mda_version);

    /* TODO: parse the metadata area (big task...) */

    return;
  }
}

/*
 * Linux swap area
 */

void detect_linux_swap(SECTION *section, int level)
{
  int i, en, pagesize;
  unsigned char *buf;
  u4 version, pages;
  char s[256];
  int pagesizes[] = { 4096, 8192, 0 };

  for (i = 0; pagesizes[i]; i++) {
    pagesize = pagesizes[i];

    if (get_buffer(section, pagesize - 512, 512, (void **)&buf) != 512)
      break;  /* assumes page sizes increase through the loop */

    if (memcmp((char *)buf + 512 - 10, "SWAP-SPACE", 10) == 0) {
      print_line(level, "Linux swap, version 1, %d KiB pages",
		 pagesize >> 10);
    }
    if (memcmp((char *)buf + 512 - 10, "SWAPSPACE2", 10) == 0) {
      if (get_buffer(section, 1024, 512, (void **)&buf) != 512)
	break;  /* really shouldn't happen */

      for (en = 0; en < 2; en++) {
	version = get_ve_long(en, buf);
	if (version >= 1 && version < 10)
	  break;
      }
      if (en < 2) {
	print_line(level, "Linux swap, version 2, subversion %d, %d KiB pages, %s",
		   (int)version, pagesize >> 10, get_ve_name(en));
	if (version == 1) {
	  pages = get_ve_long(en, buf + 4) - 1;
	  format_blocky_size(s, pages, pagesize, "pages", NULL);
	  print_line(level + 1, "Swap size %s", s);
	}
      } else {
	print_line(level, "Linux swap, version 2, illegal subversion, %d KiB pages",
		   pagesize >> 10);
      }
    }
  }
}

/*
 * various file systems
 */

void detect_linux_misc(SECTION *section, int level)
{
  int magic, fill, off, en;
  unsigned char *buf;
  char s[256];
  u8 size, blocks, blocksize;

  fill = get_buffer(section, 0, 2048, (void **)&buf);
  if (fill < 512)
    return;

  /* minix file system */
  if (fill >= 2048) {
    int version = 0, namesize = 14;

    magic = get_le_short(buf + 1024 + 16);
    if (magic == 0x137F)
      version = 1;
    if (magic == 0x138F) {
      version = 1;
      namesize = 30;
    }
    if (magic == 0x2468)
      version = 2;
    if (magic == 0x2478) {
      version = 2;
      namesize = 30;
    }
    if (version) {
      print_line(level, "Minix file system (v%d, %d chars)",
		 version, namesize);
      if (version == 1)
	blocks = get_le_short(buf + 1024 + 2);
      else
	blocks = get_le_long(buf + 1024 + 20);
      blocks = (blocks - get_le_short(buf + 1024 + 8))
	<< get_le_short(buf + 1024 + 10);
      format_blocky_size(s, blocks, 1024, "blocks", NULL);
      print_line(level + 1, "Volume size %s", s);
    }
  }

  /* Linux romfs */
  if (memcmp(buf, "-rom1fs-", 8) == 0) {
    size = get_be_long(buf + 8);
    print_line(level, "Linux romfs");
    print_line(level+1, "Volume name \"%.300s\"", (char *)(buf + 16));
    format_size_verbose(s, size);
    print_line(level+1, "Volume size %s", s);
  }

  /* Linux cramfs */
  for (off = 0; off <= 512; off += 512) {
    if (fill < off + 512)
      break;
    for (en = 0; en < 2; en++) {
      if (get_ve_long(en, buf + off) == 0x28cd3d45) {
	print_line(level, "Linux cramfs, starts sector %d, %s",
		   off >> 9, get_ve_name(en));

	get_string(buf + off + 48, 16, s);
	print_line(level + 1, "Volume name \"%s\"", s);

	size = get_ve_long(en, buf + off + 4);
	blocks = get_ve_long(en, buf + off + 40);
	format_size_verbose(s, size);
	print_line(level + 1, "Compressed size %s", s);
	format_blocky_size(s, blocks, 4096, "blocks", " -assumed-");
	print_line(level + 1, "Data size %s", s);
      }
    }
  }

  /* Linux squashfs */
  for (en = 0; en < 2; en++) {
    if (get_ve_long(en, buf) == 0x73717368) {
      int major, minor;

      major = get_ve_short(en, buf + 28);
      minor = get_ve_short(en, buf + 30);
      print_line(level, "Linux squashfs, version %d.%d, %s",
		 major, minor, get_ve_name(en));

      if (major > 2)
	size = get_ve_quad(en, buf + 63);
      else
	size = get_ve_long(en, buf + 8);
      if (major > 1)
	blocksize = get_ve_long(en, buf + 51);
      else
	blocksize = get_ve_short(en, buf + 32);

      format_size_verbose(s, size);
      print_line(level + 1, "Compressed size %s", s);
      format_size(s, blocksize);
      print_line(level + 1, "Block size %s", s);
    }
  }
}

/*
 * various boot code signatures
 */

void detect_linux_loader(SECTION *section, int level)
{
  int fill, executable, id;
  unsigned char *buf;

  if (section->flags & FLAG_IN_DISKLABEL)
    return;

  fill = get_buffer(section, 0, 2048, (void **)&buf);
  if (fill < 512)
    return;

  executable = (get_le_short(buf + 510) == 0xaa55) ? 1 : 0;

  /* boot sector stuff */
  if (executable && (memcmp(buf + 2, "LILO", 4) == 0 ||
		     memcmp(buf + 6, "LILO", 4) == 0))
    print_line(level, "LILO boot loader");
  if (executable && memcmp(buf + 3, "SYSLINUX", 8) == 0)
    print_line(level, "SYSLINUX boot loader");
  if (fill >= 1024 && find_memory(buf, fill, "ISOLINUX", 8) >= 0)
    print_line(level, "ISOLINUX boot loader");

  /* we know GRUB a little better... */
  if (executable &&
      find_memory(buf, 512, "Geom\0Hard Disk\0Read\0 Error\0", 27) >= 0) {
    if (buf[0x3e] == 3) {
      print_line(level, "GRUB boot loader, compat version %d.%d, boot drive 0x%02x",
		 (int)buf[0x3e], (int)buf[0x3f], (int)buf[0x40]);
    } else if (executable && buf[0x1bc] == 2 && buf[0x1bd] <= 2) {
      id = buf[0x3e];
      if (id == 0x10) {
	print_line(level, "GRUB boot loader, compat version %d.%d, normal version",
		   (int)buf[0x1bc], (int)buf[0x1bd]);
      } else if (id == 0x20) {
	print_line(level, "GRUB boot loader, compat version %d.%d, LBA version",
		   (int)buf[0x1bc], (int)buf[0x1bd]);
      } else {
	print_line(level, "GRUB boot loader, compat version %d.%d",
		   (int)buf[0x1bc], (int)buf[0x1bd]);
      }
    } else {
      print_line(level, "GRUB boot loader, unknown compat version %d",
		 buf[0x3e]);
    }
  }

  /* Linux kernel loader */
  if (fill >= 1024 && memcmp(buf + 512 + 2, "HdrS", 4) == 0) {
    print_line(level, "Linux kernel build-in loader");
  }

  /* Debian install floppy splitter */
  /* (not exactly boot code, but should be detected before gzip/tar) */
  if (memcmp(buf, "Floppy split ", 13) == 0) {
    char *name = (char *)buf + 32;
    char *number = (char *)buf + 164;
    char *total = (char *)buf + 172;
    print_line(level, "Debian floppy split, name \"%s\", disk %s of %s",
	       name, number, total);
  }
}

/* EOF */
