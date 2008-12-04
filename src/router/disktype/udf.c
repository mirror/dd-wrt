/*
 * udf.c
 * Detection of UDF file system
 *
 * UDF support integrated by Aaron Geyer of StorageQuest, Inc.
 *
 * UDF sniffer based on Richard Freeman's work at StorageQuest, Inc.
 *
 * Extensive changes by Christoph Pfisterer
 *
 * Copyright (c) 2003 Aaron Geyer
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
 *
 */

#include "global.h"


static int probe_udf(SECTION *section, int level, int sector_size);
static int validate_tag(unsigned char *buf, u4 sector);

void detect_udf(SECTION *section, int level)
{
  unsigned char *buffer;
  unsigned char sig_bea[7]  = {0x00, 0x42, 0x45, 0x41, 0x30, 0x31, 0x01};
                                   /*   B     E     A     0     1  */
  unsigned char sig_nsr2[7] = {0x00, 0x4e, 0x53, 0x52, 0x30, 0x32, 0x01};
                                   /*   N     S     R     0     2  */
  unsigned char sig_nsr3[7] = {0x00, 0x4e, 0x53, 0x52, 0x30, 0x33, 0x01};
                                   /*   N     S     R     0     3  */
  unsigned char sig_tea[7]  = {0x00, 0x54, 0x45, 0x41, 0x30, 0x31, 0x01};
                                   /*   T     E     A     0     1  */

  int recog_state, sector;
  int detected = 0;
  int probe_result;
  int sizes[] = { 2048, 512, 1024, 4096, -1 };
  int i;

  /* check the Volume Recognition Area (shared with ISO9660) */

  recog_state = 0;
  for (sector = 16; sector < 64; sector++) {
    if (get_buffer(section, sector * 2048, 2048, (void **)&buffer) < 2048)
      return;
    /* empty ID check (end of sequence) */
    if (buffer[2] == buffer[1] &&
	buffer[3] == buffer[1] &&
	buffer[4] == buffer[1] &&
	buffer[5] == buffer[1])
      break;
    if (recog_state == 0 && memcmp(buffer, sig_bea, 7) == 0)
      recog_state = 1;
    if (recog_state == 1 && memcmp(buffer, sig_tea, 7) == 0)
      recog_state = 0;
    if (recog_state == 1 && (memcmp(buffer, sig_nsr2, 7) == 0 ||
			     memcmp(buffer, sig_nsr3, 7) == 0)) {
      detected = 1;
      break;
    }
  }
  if (!detected)
    return;
  /* We now know we should look for an actual UDF file system */

  /*
    The UDF anchor descriptor resides at sector 256 (and some other
    places). Unfortunately, the sector size may vary, and it isn't
    recorded anywhere. The base standard allows any multiple of
    512 bytes, but the various UDF standards restrict this. For now,
    we check the common sizes 512, 1K, 2K, and 4K, and give up else.
  */

  for (i = 0; sizes[i] > 0; i++) {
    probe_result = probe_udf(section, level, sizes[i]);
    if (probe_result > 0)
      return;
  }

  /* We couldn't find the actual file system, but the stuff found
     in the recognition area is worth reporting anyway */
  print_line(level, "UDF recognition sequence, unable to locate anchor descriptor");
}

static int probe_udf(SECTION *section, int level, int sector_size)
{
  unsigned char *buffer;

  u4 count, addr, sect;
  int seen_primary = 0;
  int seen_logical = 0;
  int i;
  char s[256];

  /* first read the Anchor Volume Descriptor Pointer @ sector 256 */
  if (get_buffer(section, 256 * sector_size, 512, (void **)&buffer) < 512)
    return 0;
  if (!validate_tag(buffer, 256))
    return 0;
  /* tag identifier */
  if (get_le_short(buffer) != 2)
    return 0;

  print_line(level, "UDF file system");
  print_line(level + 1, "Sector size %d bytes", sector_size);

  /* get the Volume Descriptor Area */
  count = get_le_long(buffer + 16) / sector_size;
  addr = get_le_long(buffer + 20);

  /* look for a Logical Volume Descriptor */
  for (i = 0; i < count; i++) {
    sect = addr + i;
    if (get_buffer(section, (u8)sect * sector_size, 512, (void **)&buffer) < 512)
      break;
    if (!validate_tag(buffer, sect))
      continue;

    /* switch by tag identifier */
    switch (get_le_short(buffer)) {
    case 1:   /* Primary Volume Descriptor */
      if (!seen_primary) {
	seen_primary = 1;

	if (buffer[24] == 8) {
	  get_string(buffer + 25, 30, s);
	  print_line(level+1, "Volume name \"%s\"", s);
	} else if (buffer[24] == 16) {
	  format_utf16_le(buffer + 25, 30, s);
	  print_line(level+1, "Volume name \"%s\"", s);
	} else {
	  print_line(level+1, "Volume name encoding not supported");
	}

      }
      break;

    case 6:   /* Logical Volume Descriptor */
      if (!seen_logical) {
	seen_logical = 1;

	if (memcmp(buffer + 216+1, "*OSTA UDF Compliant", 19) == 0) {
	  print_line(level+1, "UDF version %x.%02x",
		     (int)buffer[216+25], (int)buffer[216+24]);
	}

      }
      break;
    }
  }

  if (!seen_primary) {
    print_line(level + 1, "Primary Volume Descriptor missing");
  }

  return 1;  /* some problems */
}

static int validate_tag(unsigned char *buf, u4 sector)
{
  int cksum, i;

  /* tag checksum */
  cksum = 0;
  for (i = 0; i < 16; i++)
    if (i != 4)
      cksum += buf[i];
  if ((cksum & 0xFF) != buf[4])
    return 0;

  /* reserverd */
  if (buf[5] != 0)
    return 0;

  /* tag location */
  if (get_le_long(buf + 12) != sector)
    return 0;

  return 1;
}

/* EOF */
