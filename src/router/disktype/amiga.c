/*
 * amiga.c
 * Detection of Amiga partition maps and file systems
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
 * Amiga partition type codes
 *
 * Based on the dostypes list of the Ambient file manager
 * as of Apr 02, 2005.
 */

struct dostype {
  char *typecode;
  int isfs;   /* true = native Amiga filesystem (affects printout when */
              /*  found at the start of the boot sector) */
  char *name;
};

struct dostype amiga_dostypes[] = {

  { "DOS\x00", 1, "Amiga OFS file system (non-intl.)" },
  { "DOS\x01", 1, "Amiga FFS file system (non-intl.)" },
  { "DOS\x02", 1, "Amiga OFS file system (intl., no dir cache)" },
  { "DOS\x03", 1, "Amiga FFS file system (intl., no dir cache)" },
  { "DOS\x04", 1, "Amiga OFS file system (intl., dir cache)" },
  { "DOS\x05", 1, "Amiga FFS file system (intl., dir cache)" },
  { "DOS\x06", 1, "Amiga OFS file system (LNFS)" },
  { "DOS\x07", 1, "Amiga FFS file system (LNFS)" },

  { "muFS",    1, "Amiga muFS FFS file system (intl., no dir cache)" },
  { "muF\x00", 1, "Amiga muFS OFS file system (non-intl.)" },
  { "muF\x01", 1, "Amiga muFS FFS file system (non-intl.)" },
  { "muF\x02", 1, "Amiga muFS OFS file system (intl., no dir cache)" },
  { "muF\x03", 1, "Amiga muFS FFS file system (intl., no dir cache)" },
  { "muF\x04", 1, "Amiga muFS OFS file system (intl., dir cache)" },
  { "muF\x05", 1, "Amiga muFS FFS file system (intl., dir cache)" },

  { "SFS\x00", 1, "Amiga Smart File System" },

  { "PFS\x00", 1, "Amiga PFS file system 0" },
  { "PFS\x01", 1, "Amiga PFS file system 1" },
  { "PFS\x02", 1, "Amiga PFS file system 2" },
  { "PFS\x03", 1, "Amiga PFS file system 3" },
  { "PDS\x02", 1, "Amiga PFS file system 2, SCSIdirect" },
  { "PDS\x03", 1, "Amiga PFS file system 3, SCSIdirect" },
  { "muPF",    1, "Amiga PFS file system, multiuser" },

  { "AFS\x00", 1, "Amiga AFS file system" },
  { "AFS\x01", 1, "Amiga AFS file system (experimental)" },

  { "UNI\x00", 0, "Amiga Amix 0" },
  { "UNI\x01", 0, "Amiga Amix 1" },

  { "KICK",    1, "Amiga Kickstart disk" },
  { "BOOU",    1, "Amiga generic boot disk" },
  { "BAD\x00", 0, "Unreadable disk" },
  { "NDOS",    0, "Not a DOS disk" },
  { "resv",    0, "reserved" },

  { "CD00",    0, "CD-ROM High Sierra format" },
  { "CD01",    0, "CD-ROM ISO9660 format" },
  { "CDDA",    0, "CD Audio" },
  { "CDFS",    0, "CD-ROM - Amiga CDrive or AmiCDFS" },
  { "\x66\x2d\xab\xac", 0, "CD-ROM - AsimCDFS" },

  { "NBR\x07", 0, "NetBSD root" },
  { "NBS\x01", 0, "NetBSD swap" },
  { "NBU\x07", 0, "NetBSD other" },

  { "LNX\x00", 0, "Linux native" },
  { "EXT2",    0, "Linux ext2" },
  { "SWAP",    0, "Linux swap" },
  { "SWP\x00", 0, "Linux swap" },
  { "MNX\x00", 0, "Linux minix" },

  { "MAC\x00", 0, "Macintosh HFS" },
  { "MSD\x00", 0, "MS-DOS disk" },
  { "MSH\x00", 0, "MS-DOS PC-Task hardfile" },
  { "BFFS",    0, "Berkeley Fast Filesystem" },

  { NULL, 0, NULL },
};

static char * get_name_for_dostype(const unsigned char *dostype)
{
  int i;

  for (i = 0; amiga_dostypes[i].name; i++)
    if (memcmp(dostype, amiga_dostypes[i].typecode, 4) == 0)
      return amiga_dostypes[i].name;
  return "Unknown";
}

static void format_dostype(char *buf, const unsigned char *dostype)
{
  int i;
  unsigned char c;
  char *p;

  p = buf;
  for (i = 0; i < 4; i++) {
    c = dostype[i];
    if (c < 10) {
      *p++ = '\\';
      *p++ = '0' + c;
    } else if (c < 32) {
      sprintf(p, "0x%02x", (int)c);
      p = strchr(p, 0);
    } else {
      *p++ = (char)c;
    }
  }
  *p = 0;
}

/*
 * Amiga "Rigid Disk" partition map
 */

void detect_amiga_partmap(SECTION *section, int level)
{
  int i, off, found;
  unsigned char *buf;
  char s[256], append[64];
  u4 blocksize, part_ptr;
  u8 cylsize, start, size;

  for (off = 0, found = 0; off < 16; off++) {
    if (get_buffer(section, off * 512, 512, (void **)&buf) < 512)
      break;

    if (memcmp(buf, "RDSK", 4) == 0) {
      found = 1;
      break;
    }
  }
  if (!found)
    return;

  if (off == 0)
    print_line(level, "Amiga Rigid Disk partition map");
  else
    print_line(level, "Amiga Rigid Disk partition map at sector %d", off);

  /* get device block size (?) */
  blocksize = get_be_long(buf + 16);
  if (blocksize < 256 || (blocksize & (blocksize-1))) {
    print_line(level+1, "Illegal block size %lu", blocksize);
    return;
  } else if (blocksize != 512) {
    print_line(level+1, "Unusual block size %lu, not sure this will work...", blocksize);
  }
  /* TODO: get geometry data for later use */

  /* walk the partition list */
  part_ptr = get_be_long(buf + 28);
  for (i = 1; part_ptr != 0xffffffffUL; i++) {
    if (get_buffer(section, (u8)part_ptr * 512, 256,
		   (void **)&buf) < 256) {
      print_line(level, "Partition %d: Can't read partition info block");
      break;
    }

    /* check signature */
    if (memcmp(buf, "PART", 4) != 0) {
      print_line(level, "Partition %d: Invalid signature");
      break;
    }

    /* get "next" pointer for next iteration */
    part_ptr = get_be_long(buf + 16);

    /* get sizes */
    cylsize = (u8)get_be_long(buf + 140) * (u8)get_be_long(buf + 148);
    start = get_be_long(buf + 164) * cylsize;
    size = (get_be_long(buf + 168) + 1 - get_be_long(buf + 164)) * cylsize;

    snprintf(append, 63, " from %llu", start);
    format_blocky_size(s, size, 512, "sectors", append);
    print_line(level, "Partition %d: %s",
               i, s);

    /* get name */
    get_pstring(buf + 36, s);
    if (s[0])
      print_line(level + 1, "Drive name \"%s\"", s);

    /* show dos type */
    format_dostype(s, buf + 192);
    print_line(level + 1, "Type \"%s\" (%s)", s,
               get_name_for_dostype(buf + 192));

    /* detect contents */
    if (size > 0 && start > 0) {
      analyze_recursive(section, level + 1,
			start * 512, size * 512, 0);
    }
  }
}

/*
 * Amiga file system
 */

void detect_amiga_fs(SECTION *section, int level)
{
  unsigned char *buf;
  int i, isfs;
  char s[256], *typename;

  if (get_buffer(section, 0, 512, (void **)&buf) < 512)
    return;

  /* look for one of the signatures */
  isfs = 0;
  typename = NULL;
  for (i = 0; amiga_dostypes[i].name; i++)
    if (memcmp(buf, amiga_dostypes[i].typecode, 4) == 0) {
      isfs = amiga_dostypes[i].isfs;
      typename = amiga_dostypes[i].name;
      break;
    }
  if (typename == NULL)
    return;

  if (isfs) {

    print_line(level, "%s", typename);

    format_dostype(s, buf);
    print_line(level + 1, "Type \"%s\"", s);

    if (section->size == 512*11*2*80) {
      print_line(level+1, "Size matches DD floppy");
    } else if (section->size == 512*22*2*80) {
      print_line(level+1, "Size matches HD floppy");
    }

  } else {

    format_dostype(s, buf);
    print_line(level, "Amiga type code \"%s\" (%s)", s, typename);

  }
}

/* EOF */
