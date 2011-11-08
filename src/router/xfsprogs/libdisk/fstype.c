/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fstype.h"

/*
 * From mount(8) source by Andries Brouwer.  Hacked for XFS by mkp.
 * Recent sync's to mount source:
 *	- util-linux-2.10o ... 06 Sep 00
 *	- util-linux-2.10r ... 06 Dec 00
 *	- util-linux-2.11g ... 02 Jul 01
 *	- util-linux-2.11u ... 24 Aug 02
 *	- util-linux-2.11z ... 13 May 03
 */

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

/* Most file system types can be recognized by a `magic' number
   in the superblock.  Note that the order of the tests is
   significant: by coincidence a filesystem can have the
   magic numbers for several file system types simultaneously.
   For example, the romfs magic lives in the 1st sector;
   xiafs does not touch the 1st sector and has its magic in
   the 2nd sector; ext2 does not touch the first two sectors. */

static inline unsigned short
swapped(unsigned short a) {
     return (a>>8) | (a<<8);
}

/*
    Probes the device and attempts to determine the type of filesystem
    contained within.

    Original routine by <jmorriso@bogomips.ww.ubc.ca>; made into a function
    for mount(8) by Mike Grupenhoff <kashmir@umiacs.umd.edu>.
    Corrected the test for xiafs - aeb
    Read the superblock only once - aeb
    Added a very weak heuristic for vfat - aeb
    Added iso9660, minix-v2, romfs, qnx4, udf, vxfs, swap - aeb
    Added a test for high sierra (iso9660) - quinlan@bucknell.edu
    Added ufs from a patch by jj. But maybe there are several types of ufs?
    Added ntfs from a patch by Richard Russon.
    Added xfs - 2000-03-21 Martin K. Petersen <mkp@linuxcare.com>
    Added cramfs, hfs, hpfs, adfs - Sepp Wijnands <mrrazz@garbage-coderz.net>
    Added ext3 - Andrew Morton
    Added jfs - Christoph Hellwig
    Added sysv - Tim Launchbury
    Added udf - Bryce Nesbitt
    Added gfs/gfs2, btrfs - Eric Sandeen
*/

/*
 * udf magic - I find that trying to mount garbage as an udf fs
 * causes a very large kernel delay, almost killing the machine.
 * So, we do not try udf unless there is positive evidence that it
 * might work. Strings below taken from ECMA 167.
 */
/*
 * It seems that before udf 2.00 the volume descriptor was not well
 * defined.  For 2.00 you're supposed to keep scanning records until
 * you find one NOT in this list.  (See ECMA 2/8.3.1).
 */
static char
*udf_magic[] = { "BEA01", "BOOT2", "CD001", "CDW02", "NSR02",
		 "NSR03", "TEA01" };


static int
may_be_udf(const char *id) {
    char **m;

    for (m = udf_magic; m - udf_magic < SIZE(udf_magic); m++)
       if (!strncmp(*m, id, 5))
	  return 1;
    return 0;
}

/* we saw "CD001" - may be iso9660 or udf - Bryce Nesbitt */
static int
is_really_udf(int fd) {
	int j, bs;
	struct iso_volume_descriptor isosb;

	/* determine the block size by scanning in 2K increments
	   (block sizes larger than 2K will be null padded) */
	for (bs = 1; bs < 16; bs++) {
		lseek(fd, bs*2048+32768, SEEK_SET);
		if (read(fd, (char *)&isosb, sizeof(isosb)) != sizeof(isosb))
			return 0;
		if (isosb.id[0])
			break;
	}

	/* Scan up to another 64 blocks looking for additional VSD's */
	for (j = 1; j < 64; j++) {
		if (j > 1) {
			lseek(fd, j*bs*2048+32768, SEEK_SET);
			if (read(fd, (char *)&isosb, sizeof(isosb))
			    != sizeof(isosb))
				return 0;
		}
		/* If we find NSR0x then call it udf:
		   NSR01 for UDF 1.00
		   NSR02 for UDF 1.50
		   NSR03 for UDF 2.00 */
		if (!strncmp(isosb.id, "NSR0", 4))
			return 1;
		if (!may_be_udf(isosb.id))
			return 0;
	}

	return 0;
}

static int
may_be_swap(const char *s) {
	return (strncmp(s-10, "SWAP-SPACE", 10) == 0 ||
		strncmp(s-10, "SWAPSPACE2", 10) == 0);
}

/* rather weak necessary condition */
static int
may_be_adfs(const struct adfs_super_block *sb) {
	char *p;
	int sum;

	p = (char *)sb->s_checksum;
	sum = 0;
	while(--p != (char *)sb)
		sum = (sum >> 8) + (sum & 0xff) + *p;

	return (sum & 0xff) == sb->s_checksum[0];
}

static int is_reiserfs_magic_string (struct reiserfs_super_block * rs)
{
    return (!strncmp (rs->s_magic, REISERFS_SUPER_MAGIC_STRING, 
		      strlen ( REISERFS_SUPER_MAGIC_STRING)) ||
	    !strncmp (rs->s_magic, REISER2FS_SUPER_MAGIC_STRING, 
		      strlen ( REISER2FS_SUPER_MAGIC_STRING)));
}

char *
fstype(const char *device) {
    int fd;
    char *type = NULL;
    union {
	struct minix_super_block ms;
	struct ext_super_block es;
	struct ext2_super_block e2s;
	struct vxfs_super_block vs;
	struct hfs_super_block hs;
    } sb;			/* stuff at 1024 */
    union {
	struct xiafs_super_block xiasb;
	char romfs_magic[8];
	char qnx4fs_magic[10];	/* ignore first 4 bytes */
	unsigned int bfs_magic;
	struct ntfs_super_block ntfssb;
	struct fat_super_block fatsb;
	struct xfs_super_block xfsb;
	struct cramfs_super_block cramfssb;
    } xsb;
    struct ufs_super_block ufssb;
    union {
	struct iso_volume_descriptor iso;
	struct hs_volume_descriptor hs;
    } isosb;
    struct reiserfs_super_block reiserfssb;	/* block 64 or 8 */
    struct jfs_super_block jfssb;		/* block 32 */
    struct hpfs_super_block hpfssb;
    struct adfs_super_block adfssb;
    struct sysv_super_block svsb;
    struct gfs2_sb gfs2sb;
    struct btrfs_super_block btrfssb;
    struct stat statbuf;

    /* opening and reading an arbitrary unknown path can have
       undesired side effects - first check that `device' refers
       to a block device or ordinary file */
    if (stat (device, &statbuf) ||
	!(S_ISBLK(statbuf.st_mode) || S_ISREG(statbuf.st_mode)))
      return NULL;

    fd = open(device, O_RDONLY);
    if (fd < 0)
      return NULL;

    /* do seeks and reads in disk order, otherwise a very short
       partition may cause a failure because of read error */

    if (!type) {
	 /* block 0 */
	 if (lseek(fd, 0, SEEK_SET) != 0
	     || read(fd, (char *) &xsb, sizeof(xsb)) != sizeof(xsb))
	      goto try_iso9660;
	 /* Gyorgy Kovesdi: none of my photocds has a readable block 0 */

	 if (xiafsmagic(xsb.xiasb) == _XIAFS_SUPER_MAGIC)
	      type = "xiafs";
	 else if(!strncmp(xsb.romfs_magic, "-rom1fs-", 8))
	      type = "romfs";
	 else if(!strncmp(xsb.xfsb.s_magic, XFS_SUPER_MAGIC, 4))
	      type = "xfs";
	 else if(!strncmp(xsb.qnx4fs_magic+4, "QNX4FS", 6))
	      type = "qnx4";
	 else if(xsb.bfs_magic == 0x1badface)
	      type = "bfs";
	 else if(!strncmp(xsb.ntfssb.s_magic, NTFS_SUPER_MAGIC,
			  sizeof(xsb.ntfssb.s_magic)))
	      type = "ntfs";
	 else if(cramfsmagic(xsb.cramfssb) == CRAMFS_SUPER_MAGIC ||
		 cramfsmagic(xsb.cramfssb) == CRAMFS_SUPER_MAGIC_BE)
	      type = "cramfs";
	 else if ((!strncmp(xsb.fatsb.s_os, "MSDOS", 5) ||
		   !strncmp(xsb.fatsb.s_os, "MSWIN", 5) ||
		   !strncmp(xsb.fatsb.s_os, "MTOOL", 5) ||
		   !strncmp(xsb.fatsb.s_os, "mkdosfs", 7) ||
		   !strncmp(xsb.fatsb.s_os, "kmkdosfs", 8) ||
		   /* Michal Svec: created by fdformat, old msdos utility for
		      formatting large (1.7) floppy disks. */
		   !strncmp(xsb.fatsb.s_os, "CH-FOR18", 8))
		  && (!strncmp(xsb.fatsb.s_fs, "FAT12   ", 8) ||
		      !strncmp(xsb.fatsb.s_fs, "FAT16   ", 8) ||
		      !strncmp(xsb.fatsb.s_fs2, "FAT32   ", 8)))
	      type = "vfat";	/* only guessing - might as well be fat or umsdos */
    }

    if (!type) {
	    /* sector 1 */
	    if (lseek(fd, 512 , SEEK_SET) != 512
		|| read(fd, (char *) &svsb, sizeof(svsb)) != sizeof(svsb))
		    goto io_error;
	    if (sysvmagic(svsb) == SYSV_SUPER_MAGIC )
		    type = "sysv";
    }

    if (!type) {
	/* block 1 */
	if (lseek(fd, 1024, SEEK_SET) != 1024 ||
	    read(fd, (char *) &sb, sizeof(sb)) != sizeof(sb))
		goto io_error;

	/* ext2 has magic in little-endian on disk, so "swapped" is
	   superfluous; however, there have existed strange byteswapped
	   PPC ext2 systems */
	if (ext2magic(sb.e2s) == EXT2_SUPER_MAGIC ||
	    ext2magic(sb.e2s) == EXT2_PRE_02B_MAGIC ||
	    ext2magic(sb.e2s) == swapped(EXT2_SUPER_MAGIC)) {
		type = "ext2";

	     /* maybe even ext3? */
	     if ((assemble4le(sb.e2s.s_feature_compat)
		  & EXT3_FEATURE_COMPAT_HAS_JOURNAL) &&
		 assemble4le(sb.e2s.s_journal_inum) != 0)
		     type = "ext3";	/* "ext3,ext2" */
	}

	else if (minixmagic(sb.ms) == MINIX_SUPER_MAGIC ||
		 minixmagic(sb.ms) == MINIX_SUPER_MAGIC2 ||
		 minixmagic(sb.ms) == swapped(MINIX_SUPER_MAGIC2) ||
		 minixmagic(sb.ms) == MINIX2_SUPER_MAGIC ||
		 minixmagic(sb.ms) == MINIX2_SUPER_MAGIC2)
		type = "minix";

	else if (extmagic(sb.es) == EXT_SUPER_MAGIC)
		type = "ext";

	else if (vxfsmagic(sb.vs) == VXFS_SUPER_MAGIC)
		type = "vxfs";

	else if (hfsmagic(sb.hs) == swapped(HFS_SUPER_MAGIC) ||
		(hfsmagic(sb.hs) == swapped(HFSPLUS_SUPER_MAGIC) &&
		 hfsversion(sb.hs) == swapped(HFSPLUS_SUPER_VERSION)))
		type = "hfs";
    }

    if (!type) {
	/* block 3 */
        if (lseek(fd, 0xc00, SEEK_SET) != 0xc00
            || read(fd, (char *) &adfssb, sizeof(adfssb)) != sizeof(adfssb))
             goto io_error;

	/* only a weak test */
        if (may_be_adfs(&adfssb)
            && (adfsblksize(adfssb) >= 8 &&
                adfsblksize(adfssb) <= 10))
             type = "adfs";
    }

    if (!type) {
	 int mag;

	 /* block 8 */
	 if (lseek(fd, 8192, SEEK_SET) != 8192
	     || read(fd, (char *) &ufssb, sizeof(ufssb)) != sizeof(ufssb))
	      goto io_error;

	 mag = ufsmagic(ufssb);
	 if (mag == UFS_SUPER_MAGIC_LE || mag == UFS_SUPER_MAGIC_BE)
	      type = "ufs";
    }

    if (!type) {
	/* block 8 */
	if (lseek(fd, REISERFS_OLD_DISK_OFFSET_IN_BYTES, SEEK_SET) !=
				REISERFS_OLD_DISK_OFFSET_IN_BYTES
	    || read(fd, (char *) &reiserfssb, sizeof(reiserfssb)) !=
		sizeof(reiserfssb))
	    goto io_error;
	if (is_reiserfs_magic_string(&reiserfssb))
	    type = "reiserfs";
    }

    if (!type) {
	/* block 8 */
        if (lseek(fd, 0x2000, SEEK_SET) != 0x2000
            || read(fd, (char *) &hpfssb, sizeof(hpfssb)) != sizeof(hpfssb))
             goto io_error;

        if (hpfsmagic(hpfssb) == HPFS_SUPER_MAGIC)
             type = "hpfs";
    }

    if (!type) {
	 /* block 32 */
	 if (lseek(fd, JFS_SUPER1_OFF, SEEK_SET) != JFS_SUPER1_OFF
	     || read(fd, (char *) &jfssb, sizeof(jfssb)) != sizeof(jfssb))
	      goto io_error;
	 if (!strncmp(jfssb.s_magic, JFS_MAGIC, 4))
	      type = "jfs";
    }

    if (!type) {
	 /* block 32 */
    try_iso9660:
	 if (lseek(fd, 0x8000, SEEK_SET) != 0x8000
	     || read(fd, (char *) &isosb, sizeof(isosb)) != sizeof(isosb))
	      goto io_error;

	 if (strncmp(isosb.hs.id, HS_STANDARD_ID, sizeof(isosb.hs.id)) == 0) {
		 /* "CDROM" */
		 type = "iso9660";
	 } else if (strncmp(isosb.iso.id, ISO_STANDARD_ID,
			  sizeof(isosb.iso.id)) == 0) {
		 /* CD001 */
		 type = "iso9660";
		 if (is_really_udf(fd))
			 type = "udf";
	 } else if (may_be_udf(isosb.iso.id))
		 type = "udf";
    }

    if (!type) {
	/* block 64 */
	if (lseek(fd, REISERFS_DISK_OFFSET_IN_BYTES, SEEK_SET) !=
		REISERFS_DISK_OFFSET_IN_BYTES
	    || read(fd, (char *) &reiserfssb, sizeof(reiserfssb)) !=
		sizeof(reiserfssb))
	    goto io_error;
	if (is_reiserfs_magic_string(&reiserfssb))
	    type = "reiserfs";
    }

    if (!type) {
	/* block 64 */
	if (lseek(fd, GFS_SUPERBLOCK_OFFSET, SEEK_SET) != GFS_SUPERBLOCK_OFFSET
	    || read(fd, (char *) &gfs2sb, sizeof(gfs2sb)) != sizeof(gfs2sb))
	    goto io_error;
	if (gfsmagic(gfs2sb)) {
		if (gfsformat(gfs2sb) == GFS_FORMAT_FS &&
		    gfsmultiformat(gfs2sb) == GFS_FORMAT_MULTI)
			type = "gfs";
		else if (gfsformat(gfs2sb) == GFS2_FORMAT_FS &&
			 gfsmultiformat(gfs2sb) == GFS2_FORMAT_MULTI)
			type = "gfs2";
	}
    }

    if (!type) {
	/* block 64 */
	if (lseek(fd, BTRFS_SUPER_INFO_OFFSET, SEEK_SET) != BTRFS_SUPER_INFO_OFFSET 
	    || read(fd, (char *) &btrfssb, sizeof(btrfssb)) != sizeof(btrfssb))
	    goto io_error;
	if (!strncmp((char *)(btrfssb.magic), BTRFS_MAGIC,
                    sizeof(btrfssb.magic))) {
		type = "btrfs";
	}
    }

    if (!type) {
	    /* perhaps the user tries to mount the swap space
	       on a new disk; warn her before she does mkfs on it */
	    int pagesize = getpagesize();
	    int rd;
	    char buf[128 * 1024];	/* 64k is current max pagesize */

	    if (pagesize > sizeof(buf))
		    abort();

	    rd = pagesize;
	    if (rd < 8192)
		    rd = 8192;
	    if (rd > sizeof(buf))
		    rd = sizeof(buf);
	    if (lseek(fd, 0, SEEK_SET) != 0
		|| read(fd, buf, rd) != rd)
		    goto io_error;
	    if (may_be_swap(buf+pagesize) ||
		may_be_swap(buf+4096) || may_be_swap(buf+8192))
		    type = "swap";
    }

    close (fd);
    return(type);

io_error:
    close(fd);
    return NULL;
}
