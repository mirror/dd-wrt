/*
 * detect.c
 * Detection dispatching
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
 * external detection functions
 */

/* in amiga.c */
int detect_amiga_partmap(SECTION *section, int level);
int detect_amiga_fs(SECTION *section, int level);

/* in apple.c */
int detect_apple_partmap(SECTION *section, int level);
int detect_apple_volume(SECTION *section, int level);
int detect_udif(SECTION *section, int level);

/* in atari.c */
int detect_atari_partmap(SECTION *section, int level);

/* in dos.c */
int detect_dos_partmap(SECTION *section, int level);
int detect_gpt_partmap(SECTION *section, int level);
int detect_fat(SECTION *section, int level);
int detect_exfat(SECTION *section, int level);
int detect_ntfs(SECTION *section, int level);
int detect_hpfs(SECTION *section, int level);
int detect_apfs_volume(SECTION *section, int level);
int detect_dos_loader(SECTION *section, int level);

/* in cdrom.c */
int detect_iso(SECTION *section, int level);
int detect_cdrom_misc(SECTION *section, int level);

/* in udf.c */
int detect_udf(SECTION *section, int level);

/* in linux.c */
int detect_ext234(SECTION *section, int level);
int detect_btrfs(SECTION *section, int level);
int detect_zfs(SECTION *section, int level);
int detect_f2fs(SECTION *section, int level);
int detect_reiser(SECTION *section, int level);
int detect_reiser4(SECTION *section, int level);
int detect_linux_raid(SECTION *section, int level);
int detect_linux_md(SECTION *section, int level);
int detect_linux_lvm(SECTION *section, int level);
int detect_linux_lvm2(SECTION *section, int level);
int detect_linux_swap(SECTION *section, int level);
int detect_linux_misc(SECTION *section, int level);
int detect_linux_loader(SECTION *section, int level);

/* in unix.c */
int detect_jfs(SECTION *section, int level);
int detect_xfs(SECTION *section, int level);
int detect_ufs(SECTION *section, int level);
int detect_sysv(SECTION *section, int level);
int detect_bsd_disklabel(SECTION *section, int level);
int detect_bsd_loader(SECTION *section, int level);
int detect_solaris_disklabel(SECTION *section, int level);
int detect_solaris_vtoc(SECTION *section, int level);
int detect_qnx(SECTION *section, int level);
int detect_vxfs(SECTION *section, int level);

/* in beos.c */
int detect_bfs(SECTION *section, int level);
int detect_beos_loader(SECTION *section, int level);

/* in compressed.c */
int detect_compressed(SECTION *section, int level);

/* in cdimage.c */
int detect_cdimage(SECTION *section, int level);

/* in vpc.c */
int detect_vhd(SECTION *section, int level);

/* in cloop.c */
int detect_cloop(SECTION *section, int level);

/* in archives.c */
int detect_archive(SECTION *section, int level);

/* in blank.c */
int detect_blank(SECTION *section, int level);

/*
 * list of detectors
 */

DETECTOR detectors[] = {
	/* 1: disk image formats */
	detect_vhd, /* may stop */
	detect_cdimage, /* may stop */
	detect_cloop, detect_udif,
	/* 2: boot code */
	detect_linux_loader, detect_bsd_loader, detect_dos_loader, detect_beos_loader,
	/* 3: partition tables */
	detect_bsd_disklabel, /* may stop, recurses with FLAG_IN_DISKLABEL */
	detect_solaris_disklabel, /* may stop, recurses with FLAG_IN_DISKLABEL */
	detect_solaris_vtoc, detect_amiga_partmap, detect_apple_partmap, detect_atari_partmap, detect_dos_partmap,
	detect_gpt_partmap,
	/* 4: file systems */
	detect_amiga_fs, detect_apple_volume, detect_fat, detect_exfat, detect_ntfs, detect_hpfs, detect_apfs_volume, detect_udf,
	detect_cdrom_misc, detect_iso, detect_ext234, detect_btrfs, detect_f2fs, detect_reiser, detect_reiser4, detect_linux_md,
	detect_linux_raid, detect_linux_lvm, detect_linux_lvm2, detect_linux_swap, detect_linux_misc, detect_jfs, detect_xfs,
	detect_ufs, detect_sysv, detect_qnx, detect_vxfs, detect_bfs,
	/* 5: file formats */
	detect_archive, detect_compressed, /* this is down here because of boot disks */
	detect_zfs,
	/* 6: blank formatted disk */
	detect_blank,

	NULL
};

/*
 * internal stuff
 */

static void detect(SECTION *section, int level);

static int stop_flag = 0;

/*
 * analyze a given source
 */

void analyze_source(SOURCE *s, int level)
{
	SECTION section;

	/* Allow custom analyzing using special info available to the
	   data source implementation. The analyze() function must either
	   call through to analyze_source_special() or return zero. */
	if (s->analyze != NULL) {
		if ((*s->analyze)(s, level))
			return;
	}

	section.source = s;
	section.pos = 0;
	section.size = s->size_known ? s->size : 0;
	section.flags = 0;

	detect(&section, level);
}

/*
 * analyze part of a given source
 */

void analyze_source_special(SOURCE *s, int level, u8 pos, u8 size)
{
	SECTION section;

	section.source = s;
	section.pos = pos;
	section.size = size;
	section.flags = 0;

	detect(&section, level);
}

/*
 * recursively analyze a portion of a SECTION
 */

void analyze_recursive(SECTION *section, int level, u8 rel_pos, u8 size, int flags)
{
	SOURCE *s;
	SECTION rs;

	/* sanity */
	if (rel_pos == 0 && (flags & FLAG_IN_DISKLABEL) == 0)
		return;
	s = section->source;
	if (s->size_known && (section->pos + rel_pos >= s->size))
		return;

	rs.source = s;
	rs.pos = section->pos + rel_pos;
	rs.size = size;
	rs.flags = section->flags | flags;

	detect(&rs, level);
}

/*
 * detection dispatching
 */

static void detect(SECTION *section, int level)
{
	int i;

	/* run the modularized detectors */
	for (i = 0; detectors[i] && !stop_flag; i++)
		(*detectors[i])(section, level);
	stop_flag = 0;
}

/*
 * break the detection loop
 */

void stop_detect(void)
{
	stop_flag = 1;
}

/* EOF */
