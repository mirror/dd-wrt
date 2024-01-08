/*
 * cdrom.c
 * Detection of file systems for CD-ROM and similar media
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
 * sub-functions
 */

static int dump_boot_catalog(SECTION *section, u8 pos, int level);

/*
 * ISO9660 file system
 */

int detect_iso(SECTION *section, int level)
{
	char s[256], t[256];
	int i, sector, type;
	u4 blocksize;
	u8 blocks, bcpos;
	unsigned char *buf;
	int found = 0;
	/* get the volume descriptor */
	if (get_buffer(section, 32768, 2048, (void **)&buf) < 2048)
		return 0;

	/* check signature */
	if (memcmp(buf, "\001CD001", 6) != 0)
		return 0;

	print_line(level, "ISO9660 file system");

	/* read Volume ID and other info */
	get_padded_string(buf + 40, 32, ' ', s);
	print_line(level + 1, "Volume name \"%s\"", s);

	get_padded_string(buf + 318, 128, ' ', s);
	if (s[0])
		print_line(level + 1, "Publisher   \"%s\"", s);

	get_padded_string(buf + 446, 128, ' ', s);
	if (s[0])
		print_line(level + 1, "Preparer    \"%s\"", s);

	get_padded_string(buf + 574, 128, ' ', s);
	if (s[0])
		print_line(level + 1, "Application \"%s\"", s);

	/* some other interesting facts */
	blocks = get_le_long(buf + 80);
	blocksize = get_le_short(buf + 128);
	format_blocky_size(s, blocks, blocksize, "blocks", NULL);
	print_line(level + 1, "Data size %s", s);

	for (sector = 17;; sector++) {
		/* get next descriptor */
		if (get_buffer(section, sector * 2048, 2048, (void **)&buf) <
		    2048)
			return 0;

		/* check signature */
		if (memcmp(buf + 1, "CD001", 5) != 0) {
			print_line(level + 1, "Signature missing in sector %d",
				   sector);
			return 0;
		}
		type = buf[0];
		if (type == 255)
			break;

		switch (type) {
		case 0: /* El Torito */
			/* check signature */
			if (memcmp(buf + 7, "EL TORITO SPECIFICATION", 23) !=
			    0) {
				print_line(level + 1,
					   "Boot record of unknown format");
				break;
			}

			bcpos = get_le_long(buf + 0x47);
			print_line(level + 1,
				   "El Torito boot record, catalog at %llu",
				   bcpos);

			/* boot catalog */
			dump_boot_catalog(section, bcpos * 2048, level + 2);
			found = 1;
			break;

		case 1: /* Primary Volume Descriptor */
			print_line(level + 1,
				   "Additional Primary Volume Descriptor");
			found = 1;
			break;

		case 2: /* Supplementary Volume Descriptor, Joliet */
			/* read Volume ID */
			format_utf16_be(buf + 40, 32, t);
			for (i = strlen(t) - 1; i >= 0 && t[i] == ' '; i--)
				t[i] = 0;
			print_line(level + 1,
				   "Joliet extension, volume name \"%s\"", t);
			found = 1;
			break;

		case 3: /* Volume Partition Descriptor */
			print_line(level + 1, "Volume Partition Descriptor");
			found = 1;
			break;

		default:
			print_line(level + 1, "Descriptor type %d at sector %d",
				   type, sector);
			break;
		}
	}
	return found;
}

/*
 * El Torito boot catalog
 */

static char *media_types[16] = { "non-emulated",     "1.2M floppy",
				 "1.44M floppy",     "2.88M floppy",
				 "hard disk",	     "reserved type 5",
				 "reserved type 6",  "reserved type 7",
				 "reserved type 8",  "reserved type 9",
				 "reserved type 10", "reserved type 11",
				 "reserved type 12", "reserved type 13",
				 "reserved type 14", "reserved type 15" };

static char *get_name_for_eltorito_platform(int id)
{
	if (id == 0)
		return "x86";
	if (id == 1)
		return "PowerPC";
	if (id == 2)
		return "Macintosh";
	if (id == 0xEF)
		return "EFI";
	return "unknown";
}

static int dump_boot_catalog(SECTION *section, u8 pos, int level)
{
	unsigned char *buf;
	int bootable, media, platform, system_type;
	u4 start, preload;
	int entry, maxentry, off;
	char s[256];
	int found = 0;

	/* get boot catalog */
	if (get_buffer(section, pos, 2048, (void **)&buf) < 2048)
		return 0;

	/* check validation entry (must be first) */
	if (buf[0] != 0x01 || buf[30] != 0x55 || buf[31] != 0xAA) {
		print_line(level, "Validation entry missing");
		return 0;
	}
	/* TODO: check checksum of the validation entry */
	platform = buf[1];
	/* TODO: ID string at bytes 0x04 - 0x1B */

	maxentry = 2;
	for (entry = 1; entry < maxentry + 1; entry++) {
		if ((entry & 63) == 0) {
			/* get the next CD sector */
			if (get_buffer(section, pos + (entry / 64) * 2048, 2048,
				       (void **)&buf) < 2048)
				return 0;
		}
		off = (entry * 32) % 2048;

		if (entry >= maxentry) {
			if (buf[off] ==
			    0x88) /* more bootable entries without proper section headers */
				maxentry++;
			else
				break;
		}

		if (entry == 1) {
			if (!(buf[off] == 0x88 || buf[off] == 0x00)) {
				print_line(level,
					   "Initial/Default entry missing");
				break;
			}
			if (buf[off + 32] == 0x90 || buf[off + 32] == 0x91)
				maxentry = 3;
		}

		if (buf[off] == 0x88 || buf[off] == 0x00) { /* boot entry */
			bootable = (buf[off] == 0x88) ? 1 : 0;
			media = buf[off + 1] & 15;
			system_type = buf[off + 4];
			start = get_le_long(buf + off + 8);
			preload = get_le_short(buf + off + 6);

			/* print and analyze further */
			format_size(s, preload * 512);
			print_line(level,
				   "%s %s image, starts at %lu, preloads %s",
				   bootable ? "Bootable" : "Non-bootable",
				   media_types[media], start, s);
			print_line(
				level + 1,
				"Platform 0x%02X (%s), System Type 0x%02X (%s)",
				platform,
				get_name_for_eltorito_platform(platform),
				system_type, get_name_for_mbrtype(system_type));
			if (level >= 0 && start > 0) {
				analyze_recursive(section, level + 1,
						  (u8)start * 2048, 0, 0);
				/* TODO: calculate size in some way */
			}
			found = 1;

		} else if (buf[off] == 0x44) { /* entry extension */
			maxentry++; /* doesn't count against the entry count from the last section header */

		} else if (buf[off] == 0x90 ||
			   buf[off] == 0x91) { /* section header */
			platform = buf[off + 1];
			maxentry = entry + 1 + get_le_short(buf + off + 2) +
				   ((buf[off] == 0x90) ? 1 : 0);
			/* TODO: ID string at bytes 0x04 - 0x1F */

		} else {
			print_line(level, "Unknown entry type 0x%02X",
				   buf[off]);
			break;
		}
	}
	return found;
}

/*
 * Various CD file systems
 */

int detect_cdrom_misc(SECTION *section, int level)
{
	unsigned char *buf;

	/* get first sector */
	if (get_buffer(section, 0, 2048, (void **)&buf) < 2048)
		return 0;

	/* Sega Dreamcast signature */
	if (memcmp(buf, "SEGA SEGAKATANA SEGA ENTERPRISES", 32) == 0) {
		print_line(level, "Sega Dreamcast signature");
		return 1;
	}

	/* 3DO filesystem */
	if (memcmp(buf, "\x01\x5a\x5a\x5a\x5a\x5a\x01\x00", 8) == 0 &&
	    memcmp(buf + 0x28, "CD-ROM", 6) == 0) {
		print_line(level, "3DO CD-ROM file system");
		return 1;
	}

	/* get sector 32 */
	if (get_buffer(section, 32 * 2048, 2048, (void **)&buf) < 2048)
		return 0;

	/* Xbox DVD file system */
	if (memcmp(buf, "MICROSOFT*XBOX*MEDIA", 20) == 0 &&
	    memcmp(buf + 0x7ec, "MICROSOFT*XBOX*MEDIA", 20) == 0) {
		print_line(level, "Xbox DVD file system");
		return 1;
	}
	return 0;
}

/* EOF */
